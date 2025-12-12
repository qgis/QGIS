/***************************************************************************
    qgsthreadedfeaturedownloader.cpp
    --------------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsthreadedfeaturedownloader.h"

#include "qgsapplication.h"
#include "qgsbackgroundcachedshareddata.h"
#include "qgsfeaturedownloader.h"

#include <QMutexLocker>
#include <QThread>

#include "moc_qgsthreadedfeaturedownloader.cpp"

QgsThreadedFeatureDownloader::QgsThreadedFeatureDownloader( QgsBackgroundCachedSharedData *shared )
  : mShared( shared )
  , mRequestMadeFromMainThread( QThread::currentThread() == QApplication::instance()->thread() )
{
}

QgsThreadedFeatureDownloader::~QgsThreadedFeatureDownloader()
{
  stop();
}

void QgsThreadedFeatureDownloader::stop()
{
  if ( mDownloader )
  {
    mDownloader->stop();
    wait();
    delete mDownloader;
    mDownloader = nullptr;
  }
}

void QgsThreadedFeatureDownloader::startAndWait()
{
  start();

  QMutexLocker locker( &mWaitMutex );
  while ( !mDownloader )
  {
    mWaitCond.wait( &mWaitMutex );
  }
}

void QgsThreadedFeatureDownloader::run()
{
  // We need to construct it in the run() method (i.e. in the new thread)
  mDownloader = new QgsFeatureDownloader();
  mDownloader->setImpl( mShared->newFeatureDownloaderImpl( mDownloader, mRequestMadeFromMainThread ) );
  {
    QMutexLocker locker( &mWaitMutex );
    mWaitCond.wakeOne();
  }
  mDownloader->run( true, /* serialize features */
                    mShared->requestLimit() /* user max features */ );
}
