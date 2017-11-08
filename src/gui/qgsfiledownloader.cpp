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

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#ifndef QT_NO_OPENSSL
#include <QSslError>
#endif

QgsFileDownloader::QgsFileDownloader( QUrl url, QString outputFileName, bool enableGuiNotifications )
    : mUrl( url )
    , mReply( nullptr )
    , mProgressDialog( nullptr )
    , mDownloadCanceled( false )
    , mErrors()
    , mGuiNotificationsEnabled( enableGuiNotifications )
{
  mFile.setFileName( outputFileName );
  startDownload();
}


QgsFileDownloader::~QgsFileDownloader()
{
  if ( mReply )
  {
    mReply->abort();
    mReply->deleteLater();
  }
  if ( mProgressDialog )
  {
    mProgressDialog->deleteLater();
  }
}


void QgsFileDownloader::startDownload()
{
  QgsNetworkAccessManager* nam = QgsNetworkAccessManager::instance();

  QNetworkRequest request( mUrl );

  mReply = nam->get( request );

  connect( mReply, SIGNAL( readyRead() ), this, SLOT( onReadyRead() ) );
  connect( mReply, SIGNAL( finished() ), this, SLOT( onFinished() ) );
  connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( onDownloadProgress( qint64, qint64 ) ) );
  connect( nam, SIGNAL( requestTimedOut( QNetworkReply* ) ), this, SLOT( onRequestTimedOut() ) );
#ifndef QT_NO_OPENSSL
  connect( nam, SIGNAL( sslErrors( QNetworkReply*, QList<QSslError> ) ), this, SLOT( onSslErrors( QNetworkReply*, QList<QSslError> ) ) );
#endif
  if ( mGuiNotificationsEnabled )
  {
    mProgressDialog = new QProgressDialog();
    mProgressDialog->setWindowTitle( tr( "Download" ) );
    mProgressDialog->setLabelText( tr( "Downloading %1." ).arg( mFile.fileName() ) );
    mProgressDialog->show();
    connect( mProgressDialog, SIGNAL( canceled() ), this, SLOT( onDownloadCanceled() ) );
  }
}

void QgsFileDownloader::onDownloadCanceled()
{
  mDownloadCanceled = true;
  emit downloadCanceled();
  onFinished();
}

void QgsFileDownloader::onRequestTimedOut()
{
  error( tr( "Network request %1 timed out" ).arg( mUrl.toString() ) );
}

#ifndef QT_NO_OPENSSL
void QgsFileDownloader::onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  Q_UNUSED( reply );
  QStringList errorMessages;
  errorMessages <<  "SSL Errors: ";
  for ( int end = errors.size(), i = 0; i != end; ++i )
  {
    errorMessages << errors[i].errorString();
  }
  error( errorMessages );
}
#endif


void QgsFileDownloader::error( QStringList errorMessages )
{
  for ( int end = errorMessages.size(), i = 0; i != end; ++i )
  {
    mErrors.append( errorMessages[i] );
  }
  // Show error
  if ( mGuiNotificationsEnabled )
  {
    QMessageBox::warning( nullptr, tr( "Download failed" ), mErrors.join( "<br>" ) );
  }
  if ( mReply )
    mReply->abort();
  emit downloadError( mErrors );
}

void QgsFileDownloader::error( QString errorMessage )
{
  error( QStringList() << errorMessage );
}

void QgsFileDownloader::onReadyRead()
{
  Q_ASSERT( mReply );
  if ( ! mFile.isOpen() && ! mFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
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
    mFile.close();
    mFile.remove();
    if ( mGuiNotificationsEnabled )
      mProgressDialog->hide();
  }
  else
  {
    // download finished normally
    if ( mGuiNotificationsEnabled )
      mProgressDialog->hide();
    mFile.flush();
    mFile.close();

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
      mFile.open( QIODevice::WriteOnly );
      mFile.resize( 0 );
      mFile.close();
      startDownload();
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
  if ( mGuiNotificationsEnabled )
  {
    mProgressDialog->setMaximum( bytesTotal );
    mProgressDialog->setValue( bytesReceived );
  }
  emit downloadProgress( bytesReceived, bytesTotal );
}

