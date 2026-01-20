/***************************************************************************
                         qgsalgorithmfileuploader.cpp
                         ---------------------
  Date                 : August 2025
  Copyright            : (C) 2025 by Valentin Buira
  Email                : valentin dot buira at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmfileuploader.h"
#include "moc_qgsalgorithmfileuploader.cpp"
#include "qgsprocessingparameters.h"
#include "qgis.h"
#include "qgsfileuploader.h"
#include "qgsfileutils.h"
#include "qgsnetworkaccessmanager.h"

#include <QEventLoop>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

///@cond PRIVATE

QString QgsFileUploaderAlgorithm::name() const
{
  return QStringLiteral( "fileuploader" );
}

QString QgsFileUploaderAlgorithm::displayName() const
{
  return tr( "Upload a file via HTTP(S)" );
}

QString QgsFileUploaderAlgorithm::shortDescription() const
{
  return tr( "Upload a file to the URL with an HTTP(S) request." );
}

QStringList QgsFileUploaderAlgorithm::tags() const
{
  return tr( "file,uploader,internet,url,upload,post,request,https" ).split( ',' );
}

QString QgsFileUploaderAlgorithm::group() const
{
  return tr( "File tools" );
}

QString QgsFileUploaderAlgorithm::groupId() const
{
  return QStringLiteral( "filetools" );
}

QString QgsFileUploaderAlgorithm::shortHelpString() const
{
  return tr( "This algorithm upload a file to the URL with an HTTP(S) request\n\n"
             "The optional form name field parameter emulate a filled-in form in which a user has pressed the submit button. This enables uploading of binary files when url end point require a form name key" );
}

QgsFileUploaderAlgorithm *QgsFileUploaderAlgorithm::createInstance() const
{
  return new QgsFileUploaderAlgorithm();
}

void QgsFileUploaderAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "FILE" ), QObject::tr( "File to upload" ), Qgis::ProcessingFileParameterBehavior::File, QString(), QVariant(), false, QObject::tr( "All file (%1)" ).arg( QLatin1String( "*.*" ) ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "URL" ), tr( "To URL" ), QVariant(), false, false ) );

  auto formNameParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "FORMNAME" ), tr( "Form name field" ), QString(), false, true );
  formNameParam->setHelp( QObject::tr( "The optional form name field parameter emulate a filled-in form in which a user has pressed the submit button. This enables uploading of binary files when url end point require a form name key" ) );
  formNameParam->setFlags( formNameParam->flags() | Qgis::ProcessingParameterFlag::Optional );
  addParameter( formNameParam.release() );
}

QVariantMap QgsFileUploaderAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFeedback = feedback;
  QString url = parameterAsString( parameters, QStringLiteral( "URL" ), context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL specified" ) );

  const QString filePath = parameterAsFile( parameters, QStringLiteral( "FILE" ), context );
  const bool exists = QFileInfo::exists( filePath );
  if ( !feedback->isCanceled() && !exists )
    throw QgsProcessingException( tr( "The file %1 doesn't exist." ).arg( filePath ) );

  const QString formNameKey = parameterAsString( parameters, QStringLiteral( "FORMNAME" ), context );
  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );

  QEventLoop loop;
  QTimer progressTimer;
  QUrl uploadUrl;
  QStringList errors;


  QgsFileUploader *uploader = new QgsFileUploader( filePath, url, formNameKey );

  connect( mFeedback, &QgsFeedback::canceled, uploader, &QgsFileUploader::cancelUpload );
  connect( uploader, &QgsFileUploader::uploadError, this, [&errors]( const QStringList &e ) { errors = e; } );
  connect( uploader, &QgsFileUploader::uploadProgress, this, &QgsFileUploaderAlgorithm::receiveProgressFromUploader );
  connect( uploader, &QgsFileUploader::uploadCompleted, this, [&uploadUrl]( const QUrl url ) { uploadUrl = url; } );
  connect( &progressTimer, &QTimer::timeout, this, &QgsFileUploaderAlgorithm::sendProgressFeedback );
  progressTimer.start( 1000 );

  uploader->startUpload();

  progressTimer.stop();
  if ( errors.size() > 0 )
    throw QgsProcessingException( errors.join( '\n' ) );


  url = uploadUrl.toDisplayString();
  feedback->pushInfo( QObject::tr( "Successfully upload file to %1" ).arg( url ) );

  QVariantMap outputs;
  return outputs;
}

void QgsFileUploaderAlgorithm::sendProgressFeedback()
{
  if ( !mSent.isEmpty() && mLastReport != mSent )
  {
    mLastReport = mSent;
    if ( mTotal.isEmpty() )
      mFeedback->pushInfo( tr( "%1 uploaded" ).arg( mSent ) );
    else
      mFeedback->pushInfo( tr( "%1 of %2 uploaded" ).arg( mSent, mTotal ) );
  }
}

void QgsFileUploaderAlgorithm::receiveProgressFromUploader( qint64 bytesSent, qint64 bytesTotal )
{
  mSent = QgsFileUtils::representFileSize( bytesSent );
  if ( bytesTotal > 0 )
  {
    if ( mTotal.isEmpty() )
      mTotal = QgsFileUtils::representFileSize( bytesTotal );

    mFeedback->setProgress( ( bytesSent * 100 ) / bytesTotal );
  }
}

///@endcond
