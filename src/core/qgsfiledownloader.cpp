/***************************************************************************
  qgsfiledownloader.cpp
  --------------------------------------
  Date                 : November 2016
  Copyright            : (C) 2016 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfiledownloader.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#ifndef QT_NO_SSL
#include <QSslError>
#endif

QgsFileDownloader::QgsFileDownloader( const QUrl &url, const QString &outputFileName, const QString &authcfg, bool delayStart )
  : mUrl( url )
  , mDownloadCanceled( false )
{
  mFile.setFileName( outputFileName );
  mAuthCfg = authcfg;
  if ( !delayStart )
    startDownload();
}


QgsFileDownloader::~QgsFileDownloader()
{
  if ( mReply )
  {
    mReply->abort();
    mReply->deleteLater();
  }
}


void QgsFileDownloader::startDownload()
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  QNetworkRequest request( mUrl );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
  }

  if ( mReply )
  {
    disconnect( mReply, &QNetworkReply::readyRead, this, &QgsFileDownloader::onReadyRead );
    disconnect( mReply, &QNetworkReply::finished, this, &QgsFileDownloader::onFinished );
    disconnect( mReply, &QNetworkReply::downloadProgress, this, &QgsFileDownloader::onDownloadProgress );
    mReply->abort();
    mReply->deleteLater();
  }

  mReply = nam->get( request );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg );
  }

  connect( mReply, &QNetworkReply::readyRead, this, &QgsFileDownloader::onReadyRead );
  connect( mReply, &QNetworkReply::finished, this, &QgsFileDownloader::onFinished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsFileDownloader::onDownloadProgress );
  connect( nam, &QgsNetworkAccessManager::requestTimedOut, this, &QgsFileDownloader::onRequestTimedOut, Qt::UniqueConnection );
#ifndef QT_NO_SSL
  connect( nam, &QgsNetworkAccessManager::sslErrors, this, &QgsFileDownloader::onSslErrors, Qt::UniqueConnection );
#endif
}

void QgsFileDownloader::cancelDownload()
{
  mDownloadCanceled = true;
  emit downloadCanceled();
  onFinished();
}

void QgsFileDownloader::onRequestTimedOut()
{
  error( tr( "Network request %1 timed out" ).arg( mUrl.toString() ) );
}

#ifndef QT_NO_SSL
void QgsFileDownloader::onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  Q_UNUSED( reply );
  QStringList errorMessages;
  errorMessages <<  QStringLiteral( "SSL Errors: " );
  for ( auto end = errors.size(), i = 0; i != end; ++i )
  {
    errorMessages << errors[i].errorString();
  }
  error( errorMessages );
}
#endif


void QgsFileDownloader::error( const QStringList &errorMessages )
{
  for ( auto end = errorMessages.size(), i = 0; i != end; ++i )
  {
    mErrors << errorMessages[i];
  }
  if ( mReply )
    mReply->abort();
  emit downloadError( mErrors );
}

void QgsFileDownloader::error( const QString &errorMessage )
{
  error( QStringList() << errorMessage );
}

void QgsFileDownloader::onReadyRead()
{
  Q_ASSERT( mReply );
  if ( mFile.fileName().isEmpty() )
  {
    error( tr( "No output filename specified" ) );
    onFinished();
  }
  else if ( ! mFile.isOpen() && ! mFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    error( tr( "Cannot open output file: %1" ).arg( mFile.fileName() ) );
    onFinished();
  }
  else
  {
    QByteArray data = mReply->readAll();
    mFile.write( data );
  }
}

void QgsFileDownloader::onFinished()
{
  // when canceled
  if ( ! mErrors.isEmpty() || mDownloadCanceled )
  {
    if ( mFile.isOpen() )
      mFile.close();
    if ( mFile.exists() )
      mFile.remove();
  }
  else
  {
    // download finished normally
    if ( mFile.isOpen() )
    {
      mFile.flush();
      mFile.close();
    }

    // get redirection url
    QVariant redirectionTarget = mReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( mReply->error() )
    {
      mFile.remove();
      error( tr( "Download failed: %1" ).arg( mReply->errorString() ) );
    }
    else if ( !redirectionTarget.isNull() )
    {
      QUrl newUrl = mUrl.resolved( redirectionTarget.toUrl() );
      mUrl = newUrl;
      mReply->deleteLater();
      if ( !mFile.open( QIODevice::WriteOnly ) )
      {
        mFile.remove();
        error( tr( "Cannot open output file: %1" ).arg( mFile.fileName() ) );
      }
      else
      {
        mFile.resize( 0 );
        mFile.close();
        startDownload();
      }
      return;
    }
    else
    {
      emit downloadCompleted();
    }
  }
  emit downloadExited();
  this->deleteLater();
}


void QgsFileDownloader::onDownloadProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  if ( mDownloadCanceled )
  {
    return;
  }
  emit downloadProgress( bytesReceived, bytesTotal );
}

