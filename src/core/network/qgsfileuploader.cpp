/***************************************************************************
  qgsfileuploader.cpp
  --------------------------------------
  Date                 : August 2025
  Copyright            : (C) 2025 by Valentin Buira
  Email                : valentin dot buira at gmail dot com
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

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <qmimedatabase.h>
#ifndef QT_NO_SSL
#include <QSslError>
#endif

QgsFileUploader::QgsFileUploader( const QString &uploadFileName, const QUrl &url, const QString &formName, const QString &authcfg, bool delayStart )
  : mUrl( url )
  , mFormName( formName )
  , mUploadCanceled( false )
{
  if ( !uploadFileName.isEmpty() )
    mFile.setFileName( uploadFileName );
  mAuthCfg = authcfg;
  if ( !delayStart )
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
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsFileUploader" ) );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
  }

  if ( mReply )
  {
    disconnect( mReply, &QNetworkReply::finished, this, &QgsFileUploader::onFinished );
    disconnect( mReply, &QNetworkReply::uploadProgress, this, &QgsFileUploader::onUploadProgress );
    mReply->abort();
    mReply->deleteLater();
  }


  QHttpMultiPart *multiPart = new QHttpMultiPart( QHttpMultiPart::FormDataType );

  QHttpPart filePart;
  QFile *file = new QFile( mFile.fileName() );
  QFileInfo fi = QFileInfo( file->fileName() );

  QMimeDatabase db;
  QMimeType mimeType = db.mimeTypeForFile( file->fileName() );

  filePart.setHeader( QNetworkRequest::ContentTypeHeader, mimeType.name() );
  filePart.setHeader( QNetworkRequest::ContentDispositionHeader, QStringLiteral( "form-data; %1filename=\"%2\"" ).arg(
                        ( mFormName.isEmpty() ) ? QString( "" ) : QStringLiteral( "name=\"%1\"; " ).arg( mFormName ),
                        fi.fileName()
                      ) );
  if ( !file->open( QIODevice::ReadOnly ) )
  {
    error( tr( "Error reading file %1" ).arg( mFile.fileName() ) );
    onFinished();
    return;
  }
  filePart.setBodyDevice( file );
  file->setParent( multiPart );

  multiPart->append( filePart );

  mReply = nam->post( request, multiPart );
  multiPart->setParent( mReply );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkReply( mReply, mAuthCfg );
  }

  connect( mReply, &QNetworkReply::finished, this, &QgsFileUploader::onFinished );
  connect( mReply, &QNetworkReply::uploadProgress, this, &QgsFileUploader::onUploadProgress );
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


void QgsFileUploader::onFinished()
{
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
      error( tr( "Upload failed: %1" ).arg( mReply->errorString() ) );
      error( tr( "Server returned: %1" ).arg( QString::fromUtf8( mReply->readAll() ) ) );
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

