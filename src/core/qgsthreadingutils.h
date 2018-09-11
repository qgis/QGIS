/***************************************************************************
               qgsthreadingutils.h
                     --------------------------------------
               Date                 : 11.9.2018
               Copyright            : (C) 2018 by Matthias Kuhn
               email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTHREADINGUTILS_H
#define QGSTHREADINGUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"

#include <QThread>

/**
 * \ingroup core
 * Provides threading utilities for QGIS.
 */
class CORE_EXPORT QgsThreadingUtils
{
  public:

    /**
     * Guarantees that \a func is executed on the main thread. If this is called
     * from another thread, the other thread will be blocked until the function
     * has been executed.
     * This is useful to quickly access information from objects that live on the
     * main thread and copying this information into worker threads. Avoid running
     * expensive code inside \a func.
     *
     * \note Only works with Qt >= 5.10, earlier versions will execute the code
     *       in the worker thread.
     *
     * \since QGIS 3.4
     */
    template <typename Func>
    static void runOnMainThread( const Func &func )
    {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
      // Make sure we only deal with the vector layer on the main thread where it lives.
      // Anything else risks a crash.
      if ( QThread::currentThread() == qApp->thread() )
        func();
      else
        QMetaObject::invokeMethod( qApp, func, Qt::BlockingQueuedConnection );
#else
      func();
#endif
    }

};


#endif
