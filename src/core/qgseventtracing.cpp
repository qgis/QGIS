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

#include <QFile>
#include <QThread>

/// @cond PRIVATE

struct TraceItem
{
  QgsEventTracing::EventType type;
  int threadId;
  int timestamp;
  QString category;
  QString name;
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
    char t = item.type == Begin ? 'B' : ( item.type == End ? 'E' : 'I' );
    QString msg = QString( "  {\"cat\": \"%1\", \"pid\": 1, \"tid\": %2, \"ts\": %3, \"ph\": \"%4\", \"name\": \"%5\" }" )
                  .arg( item.category ).arg( item.threadId ).arg( item.timestamp ).arg( t ).arg( item.name );
    f.write( msg.toUtf8() );
  }

  f.write( "\n]\n}\n" );
  f.close();
  return true;
}

void QgsEventTracing::addEvent( QgsEventTracing::EventType type, const QString &category, const QString &name )
{
  if ( !sIsTracing )
    return;

  sTraceEventsMutex()->lock();
  TraceItem item;
  item.type = type;
  item.timestamp = sTracingTimer()->nsecsElapsed() / 1000;
  item.threadId = reinterpret_cast<qint64>( QThread::currentThreadId() );
  item.category = category;
  item.name = name;
  sTraceEvents()->append( item );
  sTraceEventsMutex()->unlock();
}

///@endcond
