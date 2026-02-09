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

#include "qgis.h"
#include "qgsfileuploader.h"
#include "qgsfileutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsprocessingparameters.h"

#include <QFileInfo>
#include <QString>
#include <QTimer>
#include <QUrl>

#include "moc_qgsalgorithmfileuploader.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsFileUploaderAlgorithm::name() const
{
  return u"fileuploader"_s;
}

QString QgsFileUploaderAlgorithm::displayName() const
{
  return tr( "Upload a file via HTTP(S)" );
}

QString QgsFileUploaderAlgorithm::shortDescription() const
{
  return tr( "Uploads a file to the URL with an HTTP(S) request." );
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
  return u"filetools"_s;
}

QString QgsFileUploaderAlgorithm::shortHelpString() const
{
  return tr( "This algorithm uploads a file to the URL with an HTTP(S) request\n\n"
             "The optional form name field parameter emulates a filled-in form in which a user has pressed the submit button. This enables uploading of binary files when the URL endpoint requires a form name key" );
}

QgsFileUploaderAlgorithm *QgsFileUploaderAlgorithm::createInstance() const
{
  return new QgsFileUploaderAlgorithm();
}

void QgsFileUploaderAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( u"FILE"_s, QObject::tr( "File to upload" ), Qgis::ProcessingFileParameterBehavior::File, QString(), QVariant(), false, QObject::tr( "All files (%1)" ).arg( "*.*"_L1 ) ) );
  addParameter( new QgsProcessingParameterString( u"URL"_s, tr( "To URL" ), QVariant(), false, false ) );

  auto formNameParam = std::make_unique<QgsProcessingParameterString>( u"FORMNAME"_s, tr( "Form name field" ), QString(), false, true );
  formNameParam->setHelp( QObject::tr( "The optional form name field parameter emulates a filled-in form in which a user has pressed the submit button. This enables uploading of binary files when url end point requires a form name key" ) );
  formNameParam->setFlags( formNameParam->flags() | Qgis::ProcessingParameterFlag::Optional );
  addParameter( formNameParam.release() );
}

QVariantMap QgsFileUploaderAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFeedback = feedback;
  QString url = parameterAsString( parameters, u"URL"_s, context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL specified" ) );

  const QString filePath = parameterAsFile( parameters, u"FILE"_s, context );
  const bool exists = QFileInfo::exists( filePath );
  if ( !feedback->isCanceled() && !exists )
    throw QgsProcessingException( tr( "The file %1 doesn't exist." ).arg( filePath ) );

  const QString formNameKey = parameterAsString( parameters, u"FORMNAME"_s, context );

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
  feedback->pushInfo( QObject::tr( "Successfully uploaded file to %1" ).arg( url ) );

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

    mFeedback->setProgress( ( static_cast<double>( bytesSent ) / static_cast<double>( bytesTotal ) ) * 100.0 );
  }
}

///@endcond
