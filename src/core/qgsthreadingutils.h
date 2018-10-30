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

#include "qgsfeedback.h"

#include <QThread>
#include <QSemaphore>
#include <memory>

/**
 * \ingroup core
 * Provides threading utilities for QGIS.
 *
 * \since QGIS 3.4
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
     * If a \a feedback is provided, it will observe if the feedback is canceled.
     * In case the feedback is canceled before the main thread started to run the
     * function, it will return without executing the function.
     *
     * \note Only works with Qt >= 5.10, earlier versions will execute the code
     *       in the worker thread.
     *
     * \since QGIS 3.4
     */
    template <typename Func>
    static bool runOnMainThread( const Func &func, QgsFeedback *feedback = nullptr )
    {
#if QT_VERSION >= QT_VERSION_CHECK( 5, 10, 0 )
      // Make sure we only deal with the vector layer on the main thread where it lives.
      // Anything else risks a crash.
      if ( QThread::currentThread() == qApp->thread() )
      {
        func();
        return true;
      }
      else
      {
        if ( feedback )
        {
          // This semaphore will block the worker thread until the main thread is ready.
          // Ready means the event to execute the waitFunc has arrived in the event loop
          // and is being executed.
          QSemaphore semaphoreMainThreadReady( 1 );

          // This semaphore will block the main thread until the worker thread is ready.
          // Once the main thread is executing the waitFunc, it will wait for this semaphore
          // to be released. This way we can make sure that
          QSemaphore semaphoreWorkerThreadReady( 1 );

          // Acquire both semaphores. We want the main thread and the current thread to be blocked
          // until it's save to continue.
          semaphoreMainThreadReady.acquire();
          semaphoreWorkerThreadReady.acquire();

          std::function<void()> waitFunc = [&semaphoreMainThreadReady, &semaphoreWorkerThreadReady]()
          {
            // This function is executed on the main thread. As soon as it's executed
            // it will tell the worker thread that the main thread is blocked by releasing
            // the semaphore.
            semaphoreMainThreadReady.release();

            // ... and wait for the worker thread to release its semaphore
            semaphoreWorkerThreadReady.acquire();
          };

          QMetaObject::invokeMethod( qApp, waitFunc, Qt::QueuedConnection );

          // while we are in the event queue for the main thread and not yet
          // being executed, check all 100 ms if the feedback is canceled.
          while ( !semaphoreMainThreadReady.tryAcquire( 1, 100 ) )
          {
            if ( feedback->isCanceled() )
            {
              semaphoreWorkerThreadReady.release();
              return false;
            }
          }

          // finally, the main thread is blocked and we are (most likely) not canceled.
          // let's do the real work!!
          func();

          // work done -> tell the main thread he may continue
          semaphoreWorkerThreadReady.release();
          return true;
        }
        QMetaObject::invokeMethod( qApp, func, Qt::BlockingQueuedConnection );
        return true;
      }
#else
      Q_UNUSED( feedback )
      func();
      return true;
#endif
    }

};


#endif
