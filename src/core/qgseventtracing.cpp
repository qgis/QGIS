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

#include "qgseventtracing.h"

#include <QCoreApplication>
#include <QFile>
#include <QThread>

/// @cond PRIVATE

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
    case QgsEventTracing::Begin: return 'B';
    case QgsEventTracing::End: return 'E';
    case QgsEventTracing::Instant: return 'i';
    case QgsEventTracing::AsyncBegin: return 'b';
    case QgsEventTracing::AsyncEnd: return 'e';
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
    QString msg = QStringLiteral( "  {\"cat\": \"%1\", \"pid\": 1, \"tid\": %2, \"ts\": %3, \"ph\": \"%4\", \"name\": \"%5\"" )
                  .arg( item.category ).arg( item.threadId ).arg( item.timestamp ).arg( t ).arg( item.name );

    // for instant events we always set them as global (currently not supporting instant events at thread scope)
    if ( item.type == Instant )
      msg += QLatin1String( ", \"s\": \"g\"" );

    // async events also need to have ID associated
    if ( item.type == AsyncBegin || item.type == AsyncEnd )
      msg += QStringLiteral( ", \"id\": \"%1\"" ).arg( item.id );

    msg += " }";

    f.write( msg.toUtf8() );
  }

  f.write( "\n]\n}\n" );
  f.close();
  return true;
}

void QgsEventTracing::addEvent( QgsEventTracing::EventType type, const QString &category, const QString &name, const QString &id )
{
  if ( !sIsTracing )
    return;

  sTraceEventsMutex()->lock();
  TraceItem item;
  item.type = type;
  item.timestamp = sTracingTimer()->nsecsElapsed() / 1000;
  if ( QThread::currentThread() == QCoreApplication::instance()->thread() )
    item.threadId = 0;  // to make it show up first
  else
    item.threadId = static_cast<uint>( reinterpret_cast<quint64>( QThread::currentThreadId() ) );
  item.category = category;
  item.name = name;
  item.id = id;
  sTraceEvents()->append( item );
  sTraceEventsMutex()->unlock();
}

///@endcond
