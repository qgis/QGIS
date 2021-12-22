/***************************************************************************
  qgswebdavexternalstorage.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswebdavexternalstorage_p.h"

#include "qgsnetworkcontentfetcherregistry.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"

#include <QFile>
#include <QPointer>
#include <QFileInfo>

///@cond PRIVATE

QgsWebDAVExternalStorageStoreTask::QgsWebDAVExternalStorageStoreTask( const QUrl &url, const QString &filePath, const QString &authCfg )
  : QgsTask( tr( "Storing %1" ).arg( QFileInfo( filePath ).baseName() ) )
  , mUrl( url )
  , mFilePath( filePath )
  , mAuthCfg( authCfg )
  , mFeedback( new QgsFeedback( this ) )
{
}

bool QgsWebDAVExternalStorageStoreTask::run()
{
  QgsBlockingNetworkRequest request;
  request.setAuthCfg( mAuthCfg );

  QNetworkRequest req( mUrl );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsWebDAVExternalStorageStoreTask" ) );

  QFile *f = new QFile( mFilePath );
  f->open( QIODevice::ReadOnly );

  connect( &request, &QgsBlockingNetworkRequest::uploadProgress, this, [ = ]( qint64 bytesReceived, qint64 bytesTotal )
  {
    if ( !isCanceled() && bytesTotal > 0 )
    {
      const int progress = ( bytesReceived * 100 ) / bytesTotal;
      setProgress( progress );
    }
  } );

  QgsBlockingNetworkRequest::ErrorCode err = request.put( req, f, mFeedback.get() );

  if ( err != QgsBlockingNetworkRequest::NoError )
  {
    mErrorString = request.errorMessage();
  }

  return !isCanceled() && err == QgsBlockingNetworkRequest::NoError;
}

void QgsWebDAVExternalStorageStoreTask::cancel()
{
  mFeedback->cancel();
  QgsTask::cancel();
}

QString QgsWebDAVExternalStorageStoreTask::errorString() const
{
  return mErrorString;
}

QgsWebDAVExternalStorageStoredContent::QgsWebDAVExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg )
{
  QString storageUrl = url;
  if ( storageUrl.endsWith( "/" ) )
    storageUrl.append( QFileInfo( filePath ).fileName() );

  mUploadTask = new QgsWebDAVExternalStorageStoreTask( storageUrl, filePath, authcfg );

  connect( mUploadTask, &QgsTask::taskCompleted, this, [ = ]
  {
    mUrl = storageUrl;
    mStatus = Qgis::ContentStatus::Finished;
    emit stored();
  } );

  connect( mUploadTask, &QgsTask::taskTerminated, this, [ = ]
  {
    reportError( mUploadTask->errorString() );
  } );

  connect( mUploadTask, &QgsTask::progressChanged, this, [ = ]( double progress )
  {
    emit progressChanged( progress );
  } );
}

void QgsWebDAVExternalStorageStoredContent::store()
{
  mStatus = Qgis::ContentStatus::Running;
  QgsApplication::taskManager()->addTask( mUploadTask );
}


void QgsWebDAVExternalStorageStoredContent::cancel()
{
  if ( !mUploadTask )
    return;

  disconnect( mUploadTask, &QgsTask::taskTerminated, this, nullptr );
  connect( mUploadTask, &QgsTask::taskTerminated, this, [ = ]
  {
    mStatus = Qgis::ContentStatus::Canceled;
    emit canceled();
  } );

  mUploadTask->cancel();
}

QString QgsWebDAVExternalStorageStoredContent::url() const
{
  return mUrl;
}


QgsWebDAVExternalStorageFetchedContent::QgsWebDAVExternalStorageFetchedContent( QgsFetchedContent *fetchedContent )
  : mFetchedContent( fetchedContent )
{
  connect( mFetchedContent, &QgsFetchedContent::fetched, this, &QgsWebDAVExternalStorageFetchedContent::onFetched );
  connect( mFetchedContent, &QgsFetchedContent::errorOccurred, this, [ = ]( QNetworkReply::NetworkError code, const QString & errorMsg )
  {
    Q_UNUSED( code );
    reportError( errorMsg );
  } );
}

void QgsWebDAVExternalStorageFetchedContent::fetch()
{
  if ( !mFetchedContent )
    return;

  mStatus = Qgis::ContentStatus::Running;
  mFetchedContent->download();

  // could be already fetched/cached
  if ( mFetchedContent->status() == QgsFetchedContent::Finished )
  {
    mStatus = Qgis::ContentStatus::Finished;
    emit fetched();
  }
}

QString QgsWebDAVExternalStorageFetchedContent::filePath() const
{
  return mFetchedContent ? mFetchedContent->filePath() : QString();
}

void QgsWebDAVExternalStorageFetchedContent::onFetched()
{
  if ( !mFetchedContent )
    return;

  if ( mFetchedContent->status() == QgsFetchedContent::Finished )
  {
    mStatus = Qgis::ContentStatus::Finished;
    emit fetched();
  }
}

void QgsWebDAVExternalStorageFetchedContent::cancel()
{
  mFetchedContent->cancel();
}

QString QgsWebDAVExternalStorage::type() const
{
  return QStringLiteral( "WebDAV" );
};

QString QgsWebDAVExternalStorage::displayName() const
{
  return QObject::tr( "WebDAV Storage" );
};

QgsExternalStorageStoredContent *QgsWebDAVExternalStorage::doStore( const QString &filePath, const QString &url, const QString &authcfg ) const
{
  return new QgsWebDAVExternalStorageStoredContent( filePath, url, authcfg );
};

QgsExternalStorageFetchedContent *QgsWebDAVExternalStorage::doFetch( const QString &url, const QString &authConfig ) const
{
  QgsFetchedContent *fetchedContent = QgsApplication::networkContentFetcherRegistry()->fetch( url, Qgis::ActionStart::Deferred, authConfig );

  return new QgsWebDAVExternalStorageFetchedContent( fetchedContent );
}

///@endcond
