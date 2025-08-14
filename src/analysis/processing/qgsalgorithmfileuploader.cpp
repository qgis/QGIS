/***************************************************************************
                         qgsalgorithmfileuploader.cpp
                         ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne at kartoza dot com
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
#include "qgsfiledownloader.h"
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
  return tr( "file,uploader,internet,url,fetch,get,post,request,https" ).split( ',' );
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
  return tr( "This algorithm upload a file to the URL with an HTTP(S) request" );
}

QgsFileUploaderAlgorithm *QgsFileUploaderAlgorithm::createInstance() const
{
  return new QgsFileUploaderAlgorithm();
}

void QgsFileUploaderAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "FILE" ), QObject::tr( "File to upload" ), Qgis::ProcessingFileParameterBehavior::File, QString(), QVariant(), false, QObject::tr( "All file (%1)" ).arg( QLatin1String( "*.*" ) ) ) );


  addParameter( new QgsProcessingParameterString( QStringLiteral( "URL" ), tr( "To URL" ), QVariant(), false, false ) );

  auto methodParam = std::make_unique<QgsProcessingParameterEnum>(
    QStringLiteral( "METHOD" ),
    QObject::tr( "Method" ),
    QStringList()
      << QObject::tr( "GET" )
      << QObject::tr( "POST" ),
    false,
    0
  );
  methodParam->setHelp( QObject::tr( "The HTTP method to use for the request" ) );
  methodParam->setFlags( methodParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( methodParam.release() );

  auto dataParam = std::make_unique<QgsProcessingParameterString>(
    QStringLiteral( "DATA" ), tr( "Data" ), QVariant(), false, true
  );
  dataParam->setHelp( QObject::tr( "The data to add in the body if the request is a POST" ) );
  dataParam->setFlags( dataParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dataParam.release() );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), tr( "File destination" ), QObject::tr( "All files (*.*)" ), QVariant(), false ) );
}

QVariantMap QgsFileUploaderAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFeedback = feedback;
  QString url = parameterAsString( parameters, QStringLiteral( "URL" ), context );
  if ( url.isEmpty() )
    throw QgsProcessingException( tr( "No URL specified" ) );

  const QString filePath = parameterAsFile( parameters, QStringLiteral( "FILE" ), context );
  QString data = parameterAsString( parameters, QStringLiteral( "DATA" ), context );
  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );

  QEventLoop loop;
  QTimer timer;
  QUrl uploadUrl;
  QStringList errors;

  Qgis::HttpMethod httpMethod = static_cast<Qgis::HttpMethod>( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );

  if ( httpMethod == Qgis::HttpMethod::Get && !data.isEmpty() )
  {
    feedback->pushWarning( tr( "DATA parameter is not used when it's a GET request." ) );
    data = QString();
  }

  // QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();


  // QNetworkRequest request( url );

  // const QByteArray file_data;

  // QFile *file = new QFile(filePath);
  // // file->read()
  // QNetworkReply *reply = nam->post(request, file->readAll());

  // connect( reply, &QNetworkReply::uploadProgress, this, &QgsFileUploaderAlgorithm::receiveProgressFromUploader);
  // connect( reply, &QNetworkReply::finished, this, [&loop]() { loop.exit(); }  );
  // // connect( reply, &QNetworkReply::errorOccurred,  this, [&errors, &loop]( const QStringList &e ) { errors = e; loop.exit(); } );


  // // const QByteArray data = mReply->readAll();
  // // mFile.write( data );


  QgsFileUploader *uploader = new QgsFileUploader( filePath, url, QString(), true, httpMethod, data.toUtf8() );

  connect( mFeedback, &QgsFeedback::canceled, uploader, &QgsFileUploader::cancelUpload );
  connect( uploader, &QgsFileUploader::uploadError, this, [&errors, &loop]( const QStringList &e ) { errors = e; loop.exit(); } );
  connect( uploader, &QgsFileUploader::uploadProgress, this, &QgsFileUploaderAlgorithm::receiveProgressFromUploader );
  connect( uploader, &QgsFileUploader::uploadCompleted, this, [&uploadUrl]( const QUrl url ) { uploadUrl = url; } );
  connect( uploader, &QgsFileUploader::uploadExited, this, [&loop]() { loop.exit(); } );
  connect( &timer, &QTimer::timeout, this, &QgsFileUploaderAlgorithm::sendProgressFeedback );

  uploader->startUpload();

  // QgsFileDownloader *downloader = new QgsFileDownloader( QUrl( url ), outputFile, QString(), true, httpMethod, data.toUtf8() );
  // connect( mFeedback, &QgsFeedback::canceled, downloader, &QgsFileDownloader::cancelDownload );
  // connect( downloader, &QgsFileDownloader::downloadError, this, [&errors, &loop]( const QStringList &e ) { errors = e; loop.exit(); } );
  // connect( downloader, &QgsFileDownloader::downloadProgress, this, &QgsFileUploaderAlgorithm::receiveProgressFromUploader );
  // connect( downloader, &QgsFileDownloader::downloadCompleted, this, [&downloadedUrl]( const QUrl url ) { downloadedUrl = url; } );
  // connect( downloader, &QgsFileDownloader::downloadExited, this, [&loop]() { loop.exit(); } );
  // connect( &timer, &QTimer::timeout, this, &QgsFileUploaderAlgorithm::sendProgressFeedback );
  // downloader->startDownload();
  timer.start( 1000 );

  loop.exec();

  timer.stop();
  if ( errors.size() > 0 )
    throw QgsProcessingException( errors.join( '\n' ) );

  // const bool exists = QFileInfo::exists( outputFile );
  // if ( !feedback->isCanceled() && !exists )
  //   throw QgsProcessingException( tr( "Output file doesn't exist." ) );

  url = uploadUrl.toDisplayString();
  feedback->pushInfo( QObject::tr( "Successfully downloaded %1" ).arg( url ) );

  if ( parameters.value( QStringLiteral( "OUTPUT" ) ) == QgsProcessing::TEMPORARY_OUTPUT )
  {
    // the output is temporary and its file name automatically generated, try to add a file extension
    const int length = url.size();
    const int lastDotIndex = url.lastIndexOf( "." );
    const int lastSlashIndex = url.lastIndexOf( "/" );
    if ( lastDotIndex > -1 && lastDotIndex > lastSlashIndex && length - lastDotIndex <= 6 )
    {
      QFile tmpFile( outputFile );
      tmpFile.rename( tmpFile.fileName() + url.mid( lastDotIndex ) );
      outputFile += url.mid( lastDotIndex );
    }
  }

  QVariantMap outputs;
  // outputs.insert( QStringLiteral( "OUTPUT" ), exists ? outputFile : QString() );
  return outputs;
}

void QgsFileUploaderAlgorithm::sendProgressFeedback()
{
  if ( !mSent.isEmpty() && mLastReport != mSent )
  {
    mLastReport = mSent;
    if ( mTotal.isEmpty() )
      mFeedback->pushInfo( tr( "%1 downloaded" ).arg( mSent ) );
    else
      mFeedback->pushInfo( tr( "%1 of %2 downloaded" ).arg( mSent, mTotal ) );
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
