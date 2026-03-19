/***************************************************************************
  qgseventtracing.cpp
  --------------------------------------
  Date                 : October 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"
#include "qgseventtracing.h"

#include <future>

#include "qgslogger.h"

#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QThread>
#include <qmutex.h>

#ifdef HAVE_TRACY
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#endif

using namespace Qt::StringLiterals;

/// @cond PRIVATE

using namespace Qt::StringLiterals;

struct TraceItem
{
    QgsEventTracing::EventType type;
    uint threadId;
    qint64 timestamp;
    QString category;
    QString name;
    QString id;
};

//! Whether we are tracing right now
static bool sIsTracing = false;
//! High-precision timer to measure the elapsed time
Q_GLOBAL_STATIC( QElapsedTimer, sTracingTimer )
//! Buffer of captured events in the current tracing session
Q_GLOBAL_STATIC( QVector<TraceItem>, sTraceEvents )
//! Mutex to protect the buffer from being written to from multiple threads
Q_GLOBAL_STATIC( QMutex, sTraceEventsMutex )


bool QgsEventTracing::startTracing()
{
  if ( sIsTracing )
    return false;

  sIsTracing = true;
  sTraceEventsMutex()->lock();
  sTracingTimer()->start();
  sTraceEvents()->clear();
  sTraceEvents()->reserve( 1000 );
  sTraceEventsMutex()->unlock();
  return true;
}

bool QgsEventTracing::stopTracing()
{
  if ( !sIsTracing )
    return false;

  sIsTracing = false;
  sTracingTimer()->invalidate();
  return false;
}

bool QgsEventTracing::isTracingEnabled()
{
  return sIsTracing;
}

static char _eventTypeToChar( QgsEventTracing::EventType type )
{
  switch ( type )
  {
    case QgsEventTracing::Begin:
      return 'B';
    case QgsEventTracing::End:
      return 'E';
    case QgsEventTracing::Instant:
      return 'i';
    case QgsEventTracing::AsyncBegin:
      return 'b';
    case QgsEventTracing::AsyncEnd:
      return 'e';
  }
  return '?';
}

bool QgsEventTracing::writeTrace( const QString &fileName )
{
  if ( sIsTracing )
    return false;

  QFile f( fileName );
  if ( !f.open( QIODevice::WriteOnly ) )
    return false;

  f.write( "{\n\"traceEvents\": [\n" );

  bool first = true;
  for ( const auto &item : *sTraceEvents() )
  {
    if ( !first )
      f.write( ",\n" );
    else
      first = false;
    const char t = _eventTypeToChar( item.type );
    QString msg = u"  {\"cat\": \"%1\", \"pid\": 1, \"tid\": %2, \"ts\": %3, \"ph\": \"%4\", \"name\": \"%5\""_s.arg( item.category ).arg( item.threadId ).arg( item.timestamp ).arg( t ).arg( item.name );

    // for instant events we always set them as global (currently not supporting instant events at thread scope)
    if ( item.type == Instant )
      msg += ", \"s\": \"g\""_L1;

    // async events also need to have ID associated
    if ( item.type == AsyncBegin || item.type == AsyncEnd )
      msg += u", \"id\": \"%1\""_s.arg( item.id );

    msg += " }";

    f.write( msg.toUtf8() );
  }

  f.write( "\n]\n}\n" );
  f.close();
  return true;
}

// Utility type for storing Tracy zones based on this key
struct NameAndId
{
    QString name;
    QString id;

    bool operator==( const NameAndId &other ) const { return name == other.name && id == other.id; }

    friend size_t qHash( const NameAndId &key, size_t seed = 0 ) { return qHash( key.name, seed ) ^ qHash( key.id, seed ); }
};

struct TracyZoneState
{
    TracyCZoneCtx zoneCtx;
    std::optional<qsizetype> asyncDummyId;
};

struct TracyZoneDummyThread
{
    QByteArray nameHandle;
    bool occupied;
};

void QgsEventTracing::addEvent( QgsEventTracing::EventType type, const QString &category, const QString &name, const QString &id )
{
#ifdef HAVE_TRACY
  static QMutex zonesRegistryLock;
  static QHash<NameAndId, TracyZoneState> zonesByNameAndId;
  // Tracy needs zones within a single thread to be properly nested (like XML
  // tags), which doesn't work for async events that cross threads. To work
  // around this, we create dummy threads as needed and fill them
  // top-to-bottom.
  static QList<TracyZoneDummyThread> asyncDummyThreads;
  QString zoneNameStr = u"[%1] %2"_s.arg( category, name );
  QByteArray zoneName = ( zoneNameStr ).toLocal8Bit();
  NameAndId nameAndId { zoneName, id };

  std::optional<qsizetype> usingDummyThread = std::nullopt;
  switch ( type )
  {
    case AsyncBegin:
    {
      QMutexLocker<QMutex> lock( &zonesRegistryLock );
      for ( qsizetype i = 0; i < asyncDummyThreads.size(); i++ )
      {
        TracyZoneDummyThread &dummyThread = asyncDummyThreads[i];
        if ( !dummyThread.occupied )
        {
          // Unoccupied dummy thread, use it
          TracyCFiberEnter( dummyThread.nameHandle.data() );
          usingDummyThread = i;
          dummyThread.occupied = true;
          break;
        }
      }
      if ( !usingDummyThread )
      {
        // No unoccupied thread, make one
        asyncDummyThreads.append( {
          u"Async %1"_s.arg( asyncDummyThreads.size(), 3, 10, '0' ).toLocal8Bit(),
          true,
        } );
        TracyCFiberEnter( asyncDummyThreads.last().nameHandle.data() );
        usingDummyThread = asyncDummyThreads.size() - 1;
      }
      [[fallthrough]];
    }
    case Begin:
    {
      QMutexLocker<QMutex> lock( &zonesRegistryLock );
      if ( zonesByNameAndId.contains( nameAndId ) )
      {
        QgsDebugError( u"Tried to re-begin zone! (name: '%1', id: '%1')"_s.arg( zoneNameStr, id ) );
        return;
      }
      // Use dummy values for source location, since we don't have it
      uint64_t srcloc = ___tracy_alloc_srcloc_name( 0, "", 0, "", 0, zoneName.constData(), zoneName.size(), 0 );
      TracyCZoneCtx zone = ___tracy_emit_zone_begin_alloc( srcloc, true );
      if ( id.size() )
      {
        QByteArray extraText = id.toLocal8Bit();
        TracyCZoneText( zone, extraText.data(), extraText.size() );
      }
      zonesByNameAndId[nameAndId] = { zone, usingDummyThread };
      break;
    }
    case AsyncEnd:
    case End:
    {
      QMutexLocker<QMutex> lock( &zonesRegistryLock );
      auto zoneIt = zonesByNameAndId.constFind( nameAndId );
      if ( zoneIt == zonesByNameAndId.end() )
      {
        QgsDebugError( u"Tried to end unstarted zone! "_s + zoneNameStr );
        return;
      }

      if ( zoneIt->asyncDummyId )
      {
        TracyZoneDummyThread &dummyThread = asyncDummyThreads[*zoneIt->asyncDummyId];
        TracyCFiberEnter( dummyThread.nameHandle.data() );
        dummyThread.occupied = false;
        usingDummyThread = true;
      }

      ___tracy_emit_zone_end( zoneIt->zoneCtx );
      zonesByNameAndId.erase( zoneIt );
      break;
    }
    case Instant:
      TracyMessageC( zoneName.constData(), zoneName.size(), 0x4444EE );
      break;
  }
  if ( usingDummyThread )
  {
    TracyCFiberLeave;
  }
#endif

  QgsEventTracing::addEventToQgisTrace( type, category, name, id );
}

void QgsEventTracing::addEventToQgisTrace( QgsEventTracing::EventType type, const QString &category, const QString &name, const QString &id )
{
  if ( !sIsTracing )
    return;

  sTraceEventsMutex()->lock();
  TraceItem item;
  item.type = type;
  item.timestamp = sTracingTimer()->nsecsElapsed() / 1000;
  if ( QThread::currentThread() == QCoreApplication::instance()->thread() )
    item.threadId = 0; // to make it show up first
  else
    item.threadId = static_cast<uint>( reinterpret_cast<quint64>( QThread::currentThreadId() ) );
  item.category = category;
  item.name = name;
  item.id = id;
  sTraceEvents()->append( item );
  sTraceEventsMutex()->unlock();
}

size_t QgsEventTracing::ScopedEvent::sNextId = 0;

///@endcond
