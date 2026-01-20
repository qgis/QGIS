/***************************************************************************
    qgsfeaturedownloaderimpl.cpp
    ----------------------------
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

#include "qgsfeaturedownloaderimpl.h"

#include "qgsapplication.h"
#include "qgsbackgroundcachedshareddata.h"
#include "qgsfeaturedownloader.h"
#include "qgsfeaturedownloaderprogresstask.h"

#include <QThread>
#include <QVector>

QgsFeatureDownloaderImpl::QgsFeatureDownloaderImpl( QgsBackgroundCachedSharedData *shared, QgsFeatureDownloader *downloader )
  : mSharedBase( shared )
  , mDownloader( downloader )
{
  // Needed because used by a signal
  qRegisterMetaType<QVector<QgsFeatureUniqueIdPair>>( "QVector<QgsFeatureUniqueIdPair>" );
}

QgsFeatureDownloaderImpl::~QgsFeatureDownloaderImpl()
{
  if ( mProgressTask )
  {
    mProgressTask->finalize();
    mProgressTask = nullptr;
  }
}

void QgsFeatureDownloaderImpl::emitFeatureReceived( QVector<QgsFeatureUniqueIdPair> features )
{
  emit mDownloader->featureReceived( features );
}

void QgsFeatureDownloaderImpl::emitFeatureReceived( long long featureCount )
{
  emit mDownloader->featureReceived( featureCount );
}

void QgsFeatureDownloaderImpl::emitEndOfDownload( bool success )
{
  emit mDownloader->endOfDownload( success );
}

void QgsFeatureDownloaderImpl::emitResumeMainThread()
{
  emit mDownloader->resumeMainThread();
}

void QgsFeatureDownloaderImpl::stop()
{
  QgsDebugMsgLevel( u"QgsFeatureDownloaderImpl::stop()"_s, 4 );
  mStop = true;
  emitDoStop();
}

void QgsFeatureDownloaderImpl::setStopFlag()
{
  QgsDebugMsgLevel( u"QgsFeatureDownloaderImpl::setStopFlag()"_s, 4 );
  mStop = true;
}


// Called from GUI thread
void QgsFeatureDownloaderImpl::createProgressTask( long long numberMatched )
{
  Q_ASSERT( qApp->thread() == QThread::currentThread() );

  // Make sure that the creation is done in an atomic way, so that the
  // starting thread (running QgsFeatureDownloaderImpl::run()) can be sure that
  // this function has either run completely, or not at all (mStop == true),
  // when it wants to destroy mProgressTask
  QMutexLocker locker( &mMutexCreateProgressTask );

  if ( mStop )
    return;
  Q_ASSERT( !mProgressTask );

  mProgressTask = new QgsFeatureDownloaderProgressTask( QObject::tr( "Loading features for layer %1" ).arg( mSharedBase->layerName() ), numberMatched );
  QgsApplication::taskManager()->addTask( mProgressTask );
}

void QgsFeatureDownloaderImpl::endOfRun( bool serializeFeatures, bool success, long long totalDownloadedFeatureCount, bool truncatedResponse, bool interrupted, const QString &errorMessage )
{
  {
    QMutexLocker locker( &mMutexCreateProgressTask );
    mStop = true;
  }

  if ( serializeFeatures )
    mSharedBase->endOfDownload( success, totalDownloadedFeatureCount, truncatedResponse, interrupted, errorMessage );
  else if ( !errorMessage.isEmpty() )
    mSharedBase->pushError( errorMessage );

  // We must emit the signal *AFTER* the previous call to mShared->endOfDownload()
  // to avoid issues with iterators that would start just now, wouldn't detect
  // that the downloader has finished, would register to itself, but would never
  // receive the endOfDownload signal. This is not just a theoretical problem.
  // If you switch both calls, it happens quite easily in Release mode with the
  // test suite.
  emitEndOfDownload( success );

  if ( mProgressTask )
  {
    mProgressTask->finalize();
    mProgressTask = nullptr;
  }

  if ( mTimer )
  {
    mTimer->deleteLater();
    mTimer = nullptr;
  }
}
