/***************************************************************************
                             qgspicturerasterizer.cpp
                             -------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspicturerasterizer.h"

#include "qgis.h"
#include "qgsfeedback.h"
#include "qgspainting.h"

#include <QFutureWatcher>
#include <QPainter>
#include <QTimer>
#include <QtConcurrent>

#include "moc_qgspicturerasterizer.cpp"

//
// QgsPictureRasterizerJob
//

QgsPictureRasterizerJob::QgsPictureRasterizerJob( const QPicture &picture, const QSize &targetSize )
  : mTargetSize( targetSize )
  , mFeedback( std::make_unique< QgsFeedback >() )
{
  connect( &mWatcher, &QFutureWatcher< QImage >::finished, this, &QgsPictureRasterizerJob::finished );

  QgsFeedback *feedback = mFeedback.get();
  mWatcher.setFuture( QtConcurrent::run( [picture, targetSize, feedback]() -> QImage {
    if ( picture.isNull() || ( feedback && feedback->isCanceled() ) )
      return QImage();

    const QRectF bounds = QRectF( picture.boundingRect() );
    if ( bounds.isEmpty() )
      return QImage();

    const QSize outputSize = !targetSize.isEmpty() ? targetSize : bounds.size().toSize();
    if ( outputSize.isEmpty() )
      return QImage();

    QImage image( outputSize, QImage::Format_ARGB32_Premultiplied );
    image.fill( Qt::transparent );
    if ( feedback && feedback->isCanceled() )
      return QImage();

    QPainter painter( &image );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setRenderHint( QPainter::TextAntialiasing, true );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

    painter.scale( outputSize.width() / bounds.width(), outputSize.height() / bounds.height() );
    painter.translate( -bounds.topLeft() );
    QgsPainting::drawPicture( &painter, QPointF( 0, 0 ), picture );
    painter.end();

    if ( feedback && feedback->isCanceled() )
      return QImage();

    return image;
  } ) );
}

void QgsPictureRasterizerJob::cancelWithoutBlocking()
{
  mFeedback->cancel();
}

void QgsPictureRasterizerJob::cancelAndWait()
{
  mFeedback->cancel();
  mWatcher.waitForFinished();
}

bool QgsPictureRasterizerJob::isCanceled() const
{
  return mFeedback->isCanceled();
}


//
// QgsPictureRasterizer
//

QgsPictureRasterizer::QgsPictureRasterizer( QObject *parent )
  : QObject( parent )
{
  mUpdateTimer = new QTimer( this );
  mUpdateTimer->setSingleShot( true );
  connect( mUpdateTimer, &QTimer::timeout, this, &QgsPictureRasterizer::startRenderJob );
}

QgsPictureRasterizer::~QgsPictureRasterizer()
{
  mUpdateTimer->stop();
  cancelRunningJob( true );
}

void QgsPictureRasterizer::setPicture( const QPicture &picture )
{
  mPicture = QgsPainting::deepCopyQPicture( picture );

  mInvalidated = true;

  cancelRunningJob( false );

  if ( mPicture.isNull() || mPicture.boundingRect().isEmpty() )
  {
    mUpdateTimer->stop();
    mCachedImage = QImage();
    mInvalidated = false;
  }
}

void QgsPictureRasterizer::clear()
{
  mUpdateTimer->stop();
  cancelRunningJob( false );

  mPicture = QPicture();
  mCachedImage = QImage();
  mTargetSize = QSize();
  mInvalidated = false;
}

void QgsPictureRasterizer::invalidate()
{
  mInvalidated = true;
  if ( !mPicture.isNull() && !mPicture.boundingRect().isEmpty() && !mTargetSize.isEmpty() )
  {
    mUpdateTimer->start( 1 );
  }
}

QImage QgsPictureRasterizer::image( const QSize &targetSize )
{
  if ( targetSize.isEmpty() || mPicture.isNull() || mPicture.boundingRect().isEmpty() )
    return QImage();

  // force size to match picture aspect ratio
  QSize size = targetSize;
  if ( mMaxCacheSize > 0 && ( targetSize.width() > mMaxCacheSize || targetSize.height() > mMaxCacheSize ) )
  {
    size = size.scaled( mMaxCacheSize, mMaxCacheSize, Qt::KeepAspectRatio );
  }

  if ( !mCachedImage.isNull() && !mInvalidated && mCachedImage.size() == size )
  {
    return mCachedImage;
  }

  const bool jobMatchesTarget = mJob && !mInvalidated && mJob->targetSize() == size;
  const bool timerMatchesTarget = mUpdateTimer->isActive() && !mInvalidated && mTargetSize == size;

  if ( !jobMatchesTarget && !timerMatchesTarget )
  {
    mTargetSize = size;
    mUpdateTimer->start( 1 );
  }

  return mCachedImage;
}

bool QgsPictureRasterizer::isRendering() const
{
  return static_cast< bool >( mJob ) || mUpdateTimer->isActive();
}

void QgsPictureRasterizer::setMaxCacheSize( int maxSize )
{
  if ( mMaxCacheSize == maxSize )
    return;

  mMaxCacheSize = maxSize;
  invalidate();
}

void QgsPictureRasterizer::startRenderJob()
{
  cancelRunningJob( false );

  if ( mPicture.isNull() || mPicture.boundingRect().isEmpty() || mTargetSize.isEmpty() )
    return;

  mInvalidated = false;
  const QPicture pictureCopy = QgsPainting::deepCopyQPicture( mPicture );
  mJob = std::make_unique< QgsPictureRasterizerJob >( pictureCopy, mTargetSize );
  connect( mJob.get(), &QgsPictureRasterizerJob::finished, this, &QgsPictureRasterizer::onRenderJobFinished );
}

void QgsPictureRasterizer::onRenderJobFinished()
{
  if ( !mJob )
    return;

  if ( !mJob->isCanceled() )
  {
    const QImage result = mJob->result();
    if ( !result.isNull() )
    {
      mCachedImage = result;
    }
  }

  mJob.reset();
  emit rendered();
}

void QgsPictureRasterizer::cancelRunningJob( bool block )
{
  if ( !mJob )
    return;

  disconnect( mJob.get(), &QgsPictureRasterizerJob::finished, this, &QgsPictureRasterizer::onRenderJobFinished );
  if ( block )
  {
    mJob->cancelAndWait();
    mJob.reset();
  }
  else
  {
    QgsPictureRasterizerJob *oldJob = mJob.release();
    connect( oldJob, &QgsPictureRasterizerJob::finished, oldJob, &QObject::deleteLater );
    oldJob->cancelWithoutBlocking();
  }
}
