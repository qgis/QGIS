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
#include "qgsconfig.h"

#include "qgsfeedback.h"

#include <QThread>
#if defined(QGISDEBUG) || defined(AGGRESSIVE_SAFE_MODE)
#include <QDebug>
#include <QMutex>
#endif
#include <QSemaphore>
#include <QCoreApplication>
#include <memory>

#ifdef __clang_analyzer__
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS do {} while(false);
#elif defined(AGGRESSIVE_SAFE_MODE)
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS if ( QThread::currentThread() != thread() ) {qFatal( "%s", QStringLiteral("%2 (%1:%3) is run from a different thread than the object %4 lives in [0x%5 vs 0x%6]" ).arg( QString( __FILE__ ), QString( __FUNCTION__ ), QString::number( __LINE__  ), objectName() ).arg( reinterpret_cast< qint64 >( QThread::currentThread() ), 0, 16 ).arg( reinterpret_cast< qint64 >( thread() ), 0, 16 ).toLocal8Bit().constData() ); }
#elif defined(QGISDEBUG)
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS if ( QThread::currentThread() != thread() ) {qWarning() << QStringLiteral("%2 (%1:%3) is run from a different thread than the object %4 lives in [0x%5 vs 0x%6]" ).arg( QString( __FILE__ ), QString( __FUNCTION__ ), QString::number( __LINE__  ), objectName() ).arg( reinterpret_cast< qint64 >( QThread::currentThread() ), 0, 16 ).arg( reinterpret_cast< qint64 >( thread() ), 0, 16 ).toLocal8Bit().constData(); }
#else
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS do {} while(false);
#endif

// !!DO NOT USE THIS FOR NEW CODE !!
// This is in place to keep legacy code running and should be removed in the future.
#ifdef __clang_analyzer__
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL do {} while(false);
#elif defined(QGISDEBUG)
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL if ( QThread::currentThread() != thread() ) { const QString location = QStringLiteral("%1 (%2:%3)").arg( QString( __FUNCTION__ ) ,QString( __FILE__ ), QString::number( __LINE__  ) ); QgsThreadingUtils::sEmittedWarningMutex.lock(); if ( !QgsThreadingUtils::sEmittedWarnings.contains( location ) ) { qWarning() << QStringLiteral("%1 is run from a different thread than the object %2 lives in [0x%3 vs 0x%4]" ).arg( location, objectName() ).arg( reinterpret_cast< qint64 >( QThread::currentThread() ), 0, 16 ).arg( reinterpret_cast< qint64 >( thread() ), 0, 16 ).toLocal8Bit().constData(); QgsThreadingUtils::sEmittedWarnings.insert( location ); } QgsThreadingUtils::sEmittedWarningMutex.unlock(); }
#else
#define QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL do {} while(false);
#endif

#ifdef __clang_analyzer__
#define QGIS_CHECK_QOBJECT_THREAD_EQUALITY(other) do {} while(false);(void)other;
#elif defined(AGGRESSIVE_SAFE_MODE)
#define QGIS_CHECK_QOBJECT_THREAD_EQUALITY(other) if ( other->thread() != thread() ) {qFatal( "%s", QStringLiteral("%2 (%1:%3) Object %4 is from a different thread than the object %5 lives in [0x%6 vs 0x%7]" ).arg( QString( __FILE__ ), QString( __FUNCTION__ ), QString::number( __LINE__  ), other->objectName(), objectName() ).arg( reinterpret_cast< qint64 >( QThread::currentThread() ), 0, 16 ).arg( reinterpret_cast< qint64 >( thread() ), 0, 16 ).toLocal8Bit().constData() ); }
#elif defined(QGISDEBUG)
#define QGIS_CHECK_QOBJECT_THREAD_EQUALITY(other) if ( other->thread() != thread() ) {qWarning() << QStringLiteral("%2 (%1:%3) Object %4 is from a different thread than the object %5 lives in [0x%6 vs 0x%7]" ).arg( QString( __FILE__ ), QString( __FUNCTION__ ), QString::number( __LINE__  ), other->objectName(), objectName() ).arg( reinterpret_cast< qint64 >( QThread::currentThread() ), 0, 16 ).arg( reinterpret_cast< qint64 >( thread() ), 0, 16 ).toLocal8Bit().constData(); }
#else
#define QGIS_CHECK_QOBJECT_THREAD_EQUALITY(other) do {} while(false);(void)other;
#endif


/**
 * \ingroup core
 * \brief Temporarily moves a QObject to the current thread, then resets it back to nullptr thread on destruction.
 *
 * \since QGIS 3.32
 */
class QgsScopedAssignObjectToCurrentThread
{
  public:

    /**
     * Assigns \a object to the current thread.
     *
     * If \a object is already assigned to the current thread, no action will be taken.
     *
     * \warning \a object must be assigned to the nullptr thread or the current thread, or this class will assert.
     */
    QgsScopedAssignObjectToCurrentThread( QObject *object )
      : mObject( object )
    {
      Q_ASSERT_X( mObject->thread() == nullptr || mObject->thread() == QThread::currentThread(), "QgsScopedAssignObjectToCurrentThread", "QObject was already assigned to a different thread!" );
      if ( mObject->thread() != QThread::currentThread() )
        mObject->moveToThread( QThread::currentThread() );
    }

    ~QgsScopedAssignObjectToCurrentThread()
    {
      mObject->moveToThread( nullptr );
    }

    QgsScopedAssignObjectToCurrentThread( const QgsScopedAssignObjectToCurrentThread &other ) = delete;
    QgsScopedAssignObjectToCurrentThread &operator =( const QgsScopedAssignObjectToCurrentThread & ) = delete;

  private:
    QObject *mObject = nullptr;
};

/**
 * \ingroup core
 * \brief Provides threading utilities for QGIS.
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
          // until it's safe to continue.
          semaphoreMainThreadReady.acquire();
          semaphoreWorkerThreadReady.acquire();

          const std::function<void()> waitFunc = [&semaphoreMainThreadReady, &semaphoreWorkerThreadReady]()
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
    }
#if defined(QGISDEBUG)
    //! Contains a set of already emitted thread related warnings, to avoid spamming with multiple duplicate warnings
    static QSet< QString > sEmittedWarnings;
    //! Mutex protecting sEmittedWarnings
    static QMutex sEmittedWarningMutex;
#endif

};


#endif
