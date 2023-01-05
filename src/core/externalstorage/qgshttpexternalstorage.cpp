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

#include "qgshttpexternalstorage_p.h"

#include "qgsnetworkcontentfetcherregistry.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgsfeedback.h"

#include <QFile>
#include <QPointer>
#include <QFileInfo>
#include <QCryptographicHash>

///@cond PRIVATE

QgsHttpExternalStorageStoreTask::QgsHttpExternalStorageStoreTask( const QUrl &url, const QString &filePath, const QString &authCfg )
  : QgsTask( tr( "Storing %1" ).arg( QFileInfo( filePath ).baseName() ) )
  , mUrl( url )
  , mFilePath( filePath )
  , mAuthCfg( authCfg )
  , mFeedback( new QgsFeedback( this ) )
{
}

bool QgsHttpExternalStorageStoreTask::run()
{
  QgsBlockingNetworkRequest request;
  request.setAuthCfg( mAuthCfg );

  QNetworkRequest req( mUrl );
  QgsSetRequestInitiatorClass( req, QStringLiteral( "QgsHttpExternalStorageStoreTask" ) );

  QFile *f = new QFile( mFilePath );
  f->open( QIODevice::ReadOnly );

  if ( mPrepareRequestHandler )
    mPrepareRequestHandler( req, f );

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

void QgsHttpExternalStorageStoreTask::cancel()
{
  mFeedback->cancel();
  QgsTask::cancel();
}

QString QgsHttpExternalStorageStoreTask::errorString() const
{
  return mErrorString;
}

void QgsHttpExternalStorageStoreTask::setPrepareRequestHandler( std::function< void( QNetworkRequest &request, QFile *f ) > handler )
{
  mPrepareRequestHandler = handler;
}

QgsHttpExternalStorageStoredContent::QgsHttpExternalStorageStoredContent( const QString &filePath, const QString &url, const QString &authcfg )
{
  QString storageUrl = url;
  if ( storageUrl.endsWith( "/" ) )
    storageUrl.append( QFileInfo( filePath ).fileName() );

  mUploadTask = new QgsHttpExternalStorageStoreTask( storageUrl, filePath, authcfg );

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

void QgsHttpExternalStorageStoredContent::store()
{
  mStatus = Qgis::ContentStatus::Running;
  QgsApplication::taskManager()->addTask( mUploadTask );
}


void QgsHttpExternalStorageStoredContent::cancel()
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

QString QgsHttpExternalStorageStoredContent::url() const
{
  return mUrl;
}

void QgsHttpExternalStorageStoredContent::setPrepareRequestHandler( std::function< void( QNetworkRequest &request, QFile *f ) > handler )
{
  mUploadTask->setPrepareRequestHandler( handler );
}


QgsHttpExternalStorageFetchedContent::QgsHttpExternalStorageFetchedContent( QgsFetchedContent *fetchedContent )
  : mFetchedContent( fetchedContent )
{
  connect( mFetchedContent, &QgsFetchedContent::fetched, this, &QgsHttpExternalStorageFetchedContent::onFetched );
  connect( mFetchedContent, &QgsFetchedContent::errorOccurred, this, [ = ]( QNetworkReply::NetworkError code, const QString & errorMsg )
  {
    Q_UNUSED( code );
    reportError( errorMsg );
  } );
}

void QgsHttpExternalStorageFetchedContent::fetch()
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

QString QgsHttpExternalStorageFetchedContent::filePath() const
{
  return mFetchedContent ? mFetchedContent->filePath() : QString();
}

void QgsHttpExternalStorageFetchedContent::onFetched()
{
  if ( !mFetchedContent )
    return;

  if ( mFetchedContent->status() == QgsFetchedContent::Finished )
  {
    mStatus = Qgis::ContentStatus::Finished;
    emit fetched();
  }
}

void QgsHttpExternalStorageFetchedContent::cancel()
{
  mFetchedContent->cancel();
}


// WEB DAV PROTOCOL

QString QgsWebDavExternalStorage::type() const
{
  return QStringLiteral( "WebDAV" );
};

QString QgsWebDavExternalStorage::displayName() const
{
  return QObject::tr( "WebDAV Storage" );
};

QgsExternalStorageStoredContent *QgsWebDavExternalStorage::doStore( const QString &filePath, const QString &url, const QString &authcfg ) const
{
  return new QgsHttpExternalStorageStoredContent( filePath, url, authcfg );
};

QgsExternalStorageFetchedContent *QgsWebDavExternalStorage::doFetch( const QString &url, const QString &authConfig ) const
{
  QgsFetchedContent *fetchedContent = QgsApplication::networkContentFetcherRegistry()->fetch( url, Qgis::ActionStart::Deferred, authConfig );

  return new QgsHttpExternalStorageFetchedContent( fetchedContent );
}


// AWS S3 PROTOCOL

QString QgsAwsS3ExternalStorage::type() const
{
  return QStringLiteral( "AWSS3" );
};

QString QgsAwsS3ExternalStorage::displayName() const
{
  return QObject::tr( "AWS S3" );
};

QgsExternalStorageStoredContent *QgsAwsS3ExternalStorage::doStore( const QString &filePath, const QString &url, const QString &authcfg ) const
{
  std::unique_ptr<QgsHttpExternalStorageStoredContent> storedContent = std::make_unique<QgsHttpExternalStorageStoredContent>( filePath, url, authcfg );
  storedContent->setPrepareRequestHandler( []( QNetworkRequest & request, QFile * f )
  {
    QCryptographicHash payloadCrypto( QCryptographicHash::Sha256 );
    payloadCrypto.addData( f );
    QByteArray payloadHash = payloadCrypto.result().toHex();
    f->seek( 0 );
    request.setRawHeader( QByteArray( "X-Amz-Content-SHA256" ), payloadHash );
  } );

  return storedContent.release();
};

QgsExternalStorageFetchedContent *QgsAwsS3ExternalStorage::doFetch( const QString &url, const QString &authConfig ) const
{
  QgsFetchedContent *fetchedContent = QgsApplication::networkContentFetcherRegistry()->fetch( url, Qgis::ActionStart::Deferred, authConfig );

  return new QgsHttpExternalStorageFetchedContent( fetchedContent );
}
///@endcond
