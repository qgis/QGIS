/***************************************************************************
                         qgsalgorithmfiledownloader.cpp
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

#include "qgsalgorithmfiledownloader.h"
#include "qgsfiledownloader.h"
#include "qgsfileutils.h"
#include <QEventLoop>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>

///@cond PRIVATE

QString QgsFileDownloaderAlgorithm::name() const
{
  return QStringLiteral( "filedownloader" );
}

QString QgsFileDownloaderAlgorithm::displayName() const
{
  return tr( "Download file" );
}

QStringList QgsFileDownloaderAlgorithm::tags() const
{
  return tr( "file,downloader,internet,url,fetch,get,https" ).split( ',' );
}

QString QgsFileDownloaderAlgorithm::group() const
{
  return tr( "File tools" );
}

QString QgsFileDownloaderAlgorithm::groupId() const
{
  return QStringLiteral( "filetools" );
}

QString QgsFileDownloaderAlgorithm::shortHelpString() const
{
  return tr( "This algorithm downloads a URL on the file system." );
}

QgsFileDownloaderAlgorithm *QgsFileDownloaderAlgorithm::createInstance() const
{
  return new QgsFileDownloaderAlgorithm();
}

QgsProcessingAlgorithm::Flags QgsFileDownloaderAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsFileDownloaderAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "URL" ), tr( "URL" ), QVariant(), false, false ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ),
                tr( "File destination" ), QObject::tr( "All files (*.*)" ), QVariant(), true ) );
  addOutput( new QgsProcessingOutputFile( QStringLiteral( "OUTPUT" ), tr( "File destination" ) ) );
}

QVariantMap QgsFileDownloaderAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFeedback = feedback;
  QString url = parameterAsString( parameters, QStringLiteral( "URL" ), context );
  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );

  QEventLoop loop;
  QTimer timer;
  QgsFileDownloader *downloader = new QgsFileDownloader( QUrl( url ), outputFile, QString(), true );
  connect( mFeedback, &QgsFeedback::canceled, downloader, &QgsFileDownloader::cancelDownload );
  connect( downloader, &QgsFileDownloader::downloadError, this, &QgsFileDownloaderAlgorithm::reportErrors );
  connect( downloader, &QgsFileDownloader::downloadProgress, this, &QgsFileDownloaderAlgorithm::receiveProgressFromDownloader );
  connect( downloader, &QgsFileDownloader::downloadExited, &loop, &QEventLoop::quit );
  connect( &timer, &QTimer::timeout, this, &QgsFileDownloaderAlgorithm::sendProgressFeedback );
  downloader->startDownload();
  timer.start( 1000 );

  loop.exec();

  timer.stop();
  bool exists = QFileInfo( outputFile ).exists();
  if ( !feedback->isCanceled() && !exists )
    throw QgsProcessingException( tr( "Output file doesn't exist." ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), exists ? outputFile : QString() );
  return outputs;
}

void QgsFileDownloaderAlgorithm::reportErrors( QStringList errors )
{
  throw QgsProcessingException( errors.join( '\n' ) );
}

void QgsFileDownloaderAlgorithm::sendProgressFeedback()
{
  if ( !mReceived.isEmpty() && mLastReport != mReceived )
  {
    mLastReport = mReceived;
    if ( mTotal.isEmpty() )
      mFeedback->pushInfo( tr( "%1 downloaded." ).arg( mReceived ) );
    else
      mFeedback->pushInfo( tr( "%1 of %2 downloaded." ).arg( mReceived ).arg( mTotal ) );
  }
}

void QgsFileDownloaderAlgorithm::receiveProgressFromDownloader( qint64 bytesReceived, qint64 bytesTotal )
{
  mReceived = QgsFileUtils::representFileSize( bytesReceived );
  if ( bytesTotal > 0 )
  {
    if ( mTotal.isEmpty() )
      mTotal = QgsFileUtils::representFileSize( bytesTotal );

    mFeedback->setProgress( ( bytesReceived * 100 ) / bytesTotal );
  }
}

///@endcond
