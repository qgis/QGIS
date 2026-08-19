/***************************************************************************
               qgsfutureutils.h
                     --------------------------------------
               Date                 : August 2026
               Copyright            : (C) 2026 by David Koňařík
               email                : dvdkon at konarici dot cz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFUTUREUTILS_H
#define QGSFUTUREUTILS_H

#include <tuple>

#include "qgslogger.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFuture>
#include <QFutureWatcher>
#include <QThread>

#define SIP_NO_FILE

/// @cond private

/**
 * Helper object to coordinate the lifetimes of objects needed for
 * QgsFutureUtils::combine() below.
 * Sadly QObjects can't be templated, so it can't hold the QPromise<T>,
 * hence the callback (and some ugly code in combine()).
 *
 * Will wait until all the futures are finished, then call allFinishedCallback
 * once, then delete itself.
 */
class CORE_EXPORT QgsCombinedFutureHelper : public QObject
{
    Q_OBJECT
  public:
    QgsCombinedFutureHelper( QList<QFuture<void>> futures, std::function<void()> allFinishedCallback );

  private:
    QList<QFutureWatcher<void> *> mWatchers;
    std::function<void()> mAllFinishedCallback;
    bool mFired = false;

  private slots:
    void onOneFinished();
};

/// @endcond

/**
 * \ingroup core
 * \brief Provides utilities for handling QFutures.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsFutureUtils
{
  public:
    /**
     * Waits for the future to become finished without blocking the main
     * thread's event loop.
     *
     * \since QGIS 4.4
     */
    static void waitForFinished( QFuture<void> future );

    /**
     * Returns a future that will finish at the same time base will. Useful
     * since you can only attach one continuation to a single future.
     *
     * \since QGIS 4.4
     */
    static QFuture<void> clone( QFuture<void> base );

    /**
     * Converts multiple futures into a single future that will finish when all
     * its parts have finished. Like QtFuture::whenAll, but more ergonomic.
     *
     * \since QGIS 4.4
     */
    template<typename... Ts> static QFuture<std::tuple<Ts...>> combine( QFuture<Ts>... futures )
    {
      auto combinedPromise = new QPromise<std::tuple<Ts...>>;
      combinedPromise->start();
      new QgsCombinedFutureHelper( { futures... }, [combinedPromise, futures...]() mutable {
        bool allValid = ( true && ... && futures.isValid() );
        if ( !allValid )
          QgsDebugError( "Part of combined future not valid after finishing!" );
        else
        {
          try
          {
            std::tuple<Ts...> result { ( futures.takeResult() )... };
            combinedPromise->addResult( result );
          }
          catch ( QException &e )
          {
            combinedPromise->setException( e );
          }
        }
        combinedPromise->finish();
        delete combinedPromise;
      } );
      return combinedPromise->future();
    }
};

#endif // QGSFUTUREUTILS_H
