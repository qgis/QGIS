/***************************************************************************
  qgseventtracing.h
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

#ifndef QGSEVENTTRACING_H
#define QGSEVENTTRACING_H

#include "qgis_core.h"

#define SIP_NO_FILE

#include <QMutex>
#include <QElapsedTimer>
#include <QString>
#include <QVector>

/// @cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//


/**
 * A utility class that provides event tracing functionality. When tracing
 * is enabled, events from different threads can be recorded and stored
 * in a JSON file. This helps understanding what is going on in multi-threaded
 * environment of QGIS and trace some performance issues that are otherwise
 * not that easy to spot in profiler output.
 *
 * Created traces can be viewed in Chrome/Chromium browser - simply set
 * the URL to about:tracing and load the JSON file. There is also a tool
 * called trace2html that turns the JSON file into a standalone HTML page
 * that can be viewed anywhere.
 *
 * Trace viewer project is hosted here (created by Chrome developers):
 * https://github.com/catapult-project/catapult/tree/master/tracing
 *
 * Event trace format specification:
 * https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/preview
 *
 * The intended usage:
 * 1. call startTracing()
 * 2. repeatedly call addEvent()
 * 3. call stopTracing() and writeTrace() to export the data to JSON
 *
 * \note not available in Python bindings
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsEventTracing
{
  public:
    //! Type of the event that is being stored
    enum EventType
    {
      Begin,    //!< Marks start of a duration event - should be paired with "End" event type
      End,      //!< Marks end of a durection event - should be paired with "Begin" event type
      Instant,  //!< Marks an instant event (which does not have any duration)
    };

    /**
     * Starts tracing and clears buffers. Returns true on success (false if tracing is already running).
     */
    static bool startTracing();

    /**
     * Stops tracing. Returns true on success (false if tracing is already stopped).
     */
    static bool stopTracing();

    /**
     * Write captured trace to a JSON file. It is only possible to write trace when tracing has been stopped already.
     */
    static bool writeTrace( const QString &fileName );

    /**
     * Adds an event to the trace. Does nothing if tracing is not started.
     * \note This method is thread-safe: it can be run from any thread.
     */
    static void addEvent( EventType type, const QString &category, const QString &name );

    /**
     * ScopedEvent can be used to trace a single function duration - the constructor adds a "begin" event
     * and the destructor adds "end" event of the same name and category.
     */
    class ScopedEvent
    {
      public:
        ScopedEvent( const QString &category, const QString &name ): mCat( category ), mName( name ) { addEvent( Begin, mCat, mName ); }
        ~ScopedEvent() { addEvent( End, mCat, mName ); }
      private:
        QString mCat, mName;
    };

};

/// @endcond

#endif // QGSEVENTTRACING_H
