/***************************************************************************
  qgsfileuploader.cpp
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

#include "qgsfileuploader.h"
#include "moc_qgsfileuploader.cpp"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsvariantutils.h"
#include "qgslogger.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#ifndef QT_NO_SSL
#include <QSslError>
#endif

QgsFileUploader::QgsFileUploader( const QString &uploadFileName, const QUrl &url, const QString &authcfg, bool delayStart, Qgis::HttpMethod httpMethod, const QByteArray &data )
  : mUrl( url )
  , mUploadCanceled( false )
  , mHttpMethod( httpMethod )
  , mData( data )
{
  if ( !uploadFileName.isEmpty() )
    mFile.setFileName( uploadFileName );
  mAuthCfg = authcfg; // Do I need auth ?
  if ( !delayStart ) // Do I need this
    startUpload();
}


QgsFileUploader::~QgsFileUploader()
{
  if ( mReply )
  {
    mReply->abort();
    mReply->deleteLater();
  }
}

void QgsFileUploader::startUpload()
{
  QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

  QNetworkRequest request( mUrl );
  // request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsFileUploader" ) );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
  }

  if ( mReply )
  {
    disconnect( mReply, &QNetworkReply::readyRead, this, &QgsFileUploader::onReadyRead );
    disconnect( mReply, &QNetworkReply::finished, this, &QgsFileUploader::onFinished );
    disconnect( mReply, &QNetworkReply::uploadProgress, this, &QgsFileUploader::onUploadProgress );
    mReply->abort();
    mReply->deleteLater();
  }


  // QFile *file = new QFile(filePath);

  mReply = nam->post( request, mFile.readAll() );
  // mReply = nam->get(request);

  // switch ( mHttpMethod )
  // {

  //   case Qgis::HttpMethod::Post:
  //   {
  //     mReply = nam->post( request, mData );
  //     break;
  //   }
  //   case Qgis::HttpMethod::Get:
  //   {
  //     mReply = nam->get( request );
  //     break;
  //   }

  //   case Qgis::HttpMethod::Head:
  //   case Qgis::HttpMethod::Put:
  //   case Qgis::HttpMethod::Delete:
  //     QgsDebugError( QStringLiteral( "Unsupported HTTP method: %1" ).arg( qgsEnumValueToKey( mHttpMethod ) ) );
  //     // not supported
  //     break;
  // }

  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg );
  }

  // connect( mReply, &QNetworkReply::readyRead, this, &QgsFileUploader::onReadyRead );
  connect( mReply, &QNetworkReply::finished, this, &QgsFileUploader::onFinished );
  connect( mReply, &QNetworkReply::downloadProgress, this, &QgsFileUploader::onUploadProgress );
  connect( nam, qOverload< QNetworkReply *>( &QgsNetworkAccessManager::requestTimedOut ), this, &QgsFileUploader::onRequestTimedOut, Qt::UniqueConnection );
#ifndef QT_NO_SSL
  connect( nam, &QgsNetworkAccessManager::sslErrors, this, &QgsFileUploader::onSslErrors, Qt::UniqueConnection );
#endif
}

void QgsFileUploader::cancelUpload()
{
  mUploadCanceled = true;
  emit uploadCanceled();
  onFinished();
}

void QgsFileUploader::onRequestTimedOut( QNetworkReply *reply )
{
  if ( reply == mReply )
    error( tr( "Network request %1 timed out" ).arg( mUrl.toString() ) );
}

#ifndef QT_NO_SSL
void QgsFileUploader::onSslErrors( QNetworkReply *reply, const QList<QSslError> &errors )
{
  if ( reply == mReply )
  {
    QStringList errorMessages;
    errorMessages.reserve( errors.size() + 1 );
    errorMessages <<  QStringLiteral( "SSL Errors: " );

    for ( const QSslError &error : errors )
      errorMessages << error.errorString();

    error( errorMessages );
  }
}
#endif


void QgsFileUploader::error( const QStringList &errorMessages )
{
  for ( const QString &error : errorMessages )
    mErrors << error;

  if ( mReply )
    mReply->abort();
  emit uploadError( mErrors );
}

void QgsFileUploader::error( const QString &errorMessage )
{
  error( QStringList() << errorMessage );
}

void QgsFileUploader::onReadyRead()
{
  // Q_ASSERT( mReply );
  // if ( mFile.fileName().isEmpty() )
  // {
  //   error( tr( "No output filename specified" ) );
  //   onFinished();
  // }
  // else if ( ! mFile.isOpen() && ! mFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  // {
  //   error( tr( "Cannot open output file: %1" ).arg( mFile.fileName() ) );
  //   onFinished();
  // }
  // else
  // {
  //   const QByteArray data = mReply->readAll();
  //   mFile.write( data );
  // }
}

void QgsFileUploader::onFinished()
{
  qDebug() << "finished ! ";
  // when canceled
  if ( ! mErrors.isEmpty() || mUploadCanceled )
  {
    if ( mFile.isOpen() )
      mFile.close();
  }
  else
  {
    // download finished normally
    if ( mFile.isOpen() )
    {
      mFile.flush();
      mFile.close();
    }

    if ( mReply->error() )
    {
      // mFile.remove();
      qDebug() << "ERROR:" << mReply->readAll();
      error( tr( "Upload failed: %1" ).arg( mReply->errorString() ) );
    }
    else
    {
      emit uploadCompleted( mReply->url() );
    }
  }
  emit uploadExited();
  this->deleteLater();
}


void QgsFileUploader::onUploadProgress( qint64 bytesSent, qint64 bytesTotal )
{
  if ( mUploadCanceled )
  {
    return;
  }
  emit uploadProgress( bytesSent, bytesTotal );
}

