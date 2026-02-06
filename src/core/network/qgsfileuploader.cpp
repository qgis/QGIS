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

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsblockingnetworkrequest.h"
#include "qgslogger.h"
#include "qgssetrequestinitiator_p.h"

#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <qmimedatabase.h>

#include "moc_qgsfileuploader.cpp"

+using namespace Qt::StringLiterals;
+
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
}

void QgsFileUploader::startUpload()
{
  QNetworkRequest request( mUrl );
  QgsSetRequestInitiatorClass( request, u"QgsFileUploader"_s );
  if ( !mAuthCfg.isEmpty() )
  {
    QgsApplication::authManager()->updateNetworkRequest( request, mAuthCfg );
  }



  auto multiPart = std::make_unique<QHttpMultiPart>( QHttpMultiPart::FormDataType );

  QHttpPart filePart;
  auto file = std::make_unique<QFile>( mFile.fileName() );
  QFileInfo fi = QFileInfo( file->fileName() );

  QMimeDatabase db;
  QMimeType mimeType = db.mimeTypeForFile( file->fileName() );

  filePart.setHeader( QNetworkRequest::ContentTypeHeader, mimeType.name() );
  filePart.setHeader( QNetworkRequest::ContentDispositionHeader, u"form-data; %1filename=\"%2\""_s.arg(
                        ( mFormName.isEmpty() ) ? QString( "" ) : u"name=\"%1\"; "_s.arg( mFormName ),
                        fi.fileName()
                      ) );
  if ( !file->open( QIODevice::ReadOnly ) )
  {
    error( tr( "Error reading file %1" ).arg( mFile.fileName() ) );
    onFinished();
    return;
  }
  filePart.setBodyDevice( file.get() );
  file.release()->setParent( multiPart.get() );

  multiPart->append( filePart );


  QgsBlockingNetworkRequest *networkRequest = new QgsBlockingNetworkRequest();

  if ( !mAuthCfg.isEmpty() )
  {
    networkRequest->setAuthCfg( mAuthCfg );
  }


  connect( networkRequest, &QgsBlockingNetworkRequest::uploadProgress, this, &QgsFileUploader::onUploadProgress );
  connect( this, &QgsFileUploader::uploadCanceled, networkRequest, &QgsBlockingNetworkRequest::abort );


  const QgsBlockingNetworkRequest::ErrorCode errorCode = networkRequest->post( request, multiPart.get(), false );

  if ( errorCode == QgsBlockingNetworkRequest::NoError )
  {
    // upload successful, nothing to do it's all good
  }
  else if ( errorCode == QgsBlockingNetworkRequest:: TimeoutError )
  {
    error( tr( "Network request %1 timed out" ).arg( networkRequest->errorMessage() ) );
  }
  else if ( errorCode == QgsBlockingNetworkRequest::NetworkError )
  {
    error( tr( "Upload failed, Server returned: %1" ).arg( networkRequest->errorMessage() ) );
  }
  else  // All other errors
  {
    error( tr( "Upload failed: %1" ).arg( networkRequest->errorMessage() ) );
  }
  onFinished();

  networkRequest->deleteLater();
}

void QgsFileUploader::cancelUpload()
{
  mUploadCanceled = true;
  emit uploadCanceled();
  onFinished();
}


void QgsFileUploader::error( const QStringList &errorMessages )
{
  for ( const QString &error : errorMessages )
    mErrors << error;

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

    else
    {
      emit uploadCompleted( mUrl );
    }
  }
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

