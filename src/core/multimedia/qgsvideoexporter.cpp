/***************************************************************************
                             qgsvideoexporter.h
                             --------------------------
    begin                : November 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvideoexporter.h"

#include "qgsfeedback.h"

#include <QDirIterator>
#include <QUrl>
#include <QtMultimedia/QMediaCaptureSession>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimedia/QVideoFrameInput>

#include "moc_qgsvideoexporter.cpp"

bool QgsVideoExporter::isAvailable()
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 8, 0 )
  return false;
#else
  return true;
#endif
}

QgsVideoExporter::QgsVideoExporter( const QString &filename, QSize size, double framesPerSecond )
  : mFileName( filename )
  , mSize( size )
  , mFramesPerSecond( framesPerSecond )
  , mFrameDurationUs( static_cast< qint64>( 1000000 / framesPerSecond ) )
{

}

QgsVideoExporter::~QgsVideoExporter()
{
}

void QgsVideoExporter::setFeedback( QgsFeedback *feedback )
{
  mFeedback = feedback;
}

QgsFeedback *QgsVideoExporter::feedback()
{
  return mFeedback;
}

void QgsVideoExporter::setInputFiles( const QStringList &files )
{
  mInputFiles = files;
}

void QgsVideoExporter::setInputFilesByPattern( const QString &directory, const QString &pattern )
{
  QDirIterator it( directory, pattern.isEmpty() ? QStringList() : QStringList{ pattern }, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags );
  mInputFiles.clear();
  while ( it.hasNext() )
  {
    const QString fullPath = it.next();
    mInputFiles << fullPath;
  }

  std::sort( mInputFiles.begin(), mInputFiles.end() );
}

QStringList QgsVideoExporter::inputFiles() const
{
  return mInputFiles;
}

void QgsVideoExporter::setFileFormat( QMediaFormat::FileFormat format )
{
  mFormat = format;
}

QMediaFormat::FileFormat QgsVideoExporter::fileFormat() const
{
  return mFormat;
}

void QgsVideoExporter::setVideoCodec( QMediaFormat::VideoCodec codec )
{
  mCodec = codec;
}

QMediaFormat::VideoCodec QgsVideoExporter::videoCodec() const
{
  return mCodec;
}

QMediaRecorder::Error QgsVideoExporter::error() const
{
  return mError;
}

QString QgsVideoExporter::errorString() const
{
  return mErrorString;
}

void QgsVideoExporter::writeVideo()
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 8, 0 )
  throw QgsNotSupportedException( u"Writing video is not supported on this system"_s );
#else
  mSession = std::make_unique< QMediaCaptureSession >();
  mRecorder = std::make_unique< QMediaRecorder >();
  mVideoInput = std::make_unique< QVideoFrameInput >();
  mSession->setVideoFrameInput( mVideoInput.get() );
  mSession->setRecorder( mRecorder.get() );
  mRecorder->setOutputLocation( QUrl::fromLocalFile( mFileName ) );

  QMediaFormat mediaFormat;
  mediaFormat.setFileFormat( mFormat );
  mediaFormat.setVideoCodec( mCodec );
  mRecorder->setMediaFormat( mediaFormat );

  // TODO: expose
  mRecorder->setQuality( QMediaRecorder::Quality::VeryHighQuality );
  mRecorder->setVideoBitRate( 2000 );
  mRecorder->setEncodingMode( QMediaRecorder::EncodingMode::TwoPassEncoding );

  mRecorder->setVideoResolution( mSize );
  mRecorder->setVideoFrameRate( mFramesPerSecond );

  QObject::connect( mVideoInput.get(), &QVideoFrameInput::readyToSendVideoFrame, this, &QgsVideoExporter::feedFrames );
  QObject::connect( mRecorder.get(), &QMediaRecorder::recorderStateChanged, this, &QgsVideoExporter::checkStatus );
  QObject::connect( mRecorder.get(), &QMediaRecorder::errorOccurred, this, &QgsVideoExporter::handleError );

  mRecorder->record();

  if ( mFeedback )
  {
    mFeedback->setProgress( 0 );
  }
  feedFrames();
#endif
}

void QgsVideoExporter::feedFrames()
{
#if QT_VERSION >= QT_VERSION_CHECK( 6, 8, 0 )
  if ( !mRecorder
       || !mVideoInput
       || mRecorder->recorderState() != QMediaRecorder::RecorderState::RecordingState )
    return;

  while ( mCurrentFrameIndex < mInputFiles.count() )
  {
    const QImage frame( mInputFiles.at( mCurrentFrameIndex ) );
    QVideoFrame videoFrame( frame );
    const qint64 startUs = mCurrentFrameIndex * mFrameDurationUs;
    videoFrame.setStartTime( startUs );
    videoFrame.setEndTime( startUs + mFrameDurationUs );

    const bool sent = mVideoInput->sendVideoFrame( videoFrame );
    if ( !sent )
      return;

    mCurrentFrameIndex++;

    if ( mFeedback )
    {
      mFeedback->setProgress( 100.0 * static_cast< double >( mCurrentFrameIndex ) / static_cast< double >( mInputFiles.count() ) );
      if ( mFeedback->isCanceled() )
      {
        mRecorder->stop();
        return;
      }
    }
  }

  if ( mCurrentFrameIndex >= mInputFiles.count() )
  {
    mRecorder->stop();
  }
#endif
}

void QgsVideoExporter::checkStatus( QMediaRecorder::RecorderState state )
{
  switch ( state )
  {
    case QMediaRecorder::StoppedState:
    {
      if ( mCurrentFrameIndex >= mInputFiles.count() )
      {
        emit finished();
      }
      break;
    }

    case QMediaRecorder::RecordingState:
    case QMediaRecorder::PausedState:
      break;
  }
}

void QgsVideoExporter::handleError( QMediaRecorder::Error error, const QString &errorString )
{
  mError = error;
  mErrorString = errorString;
}
