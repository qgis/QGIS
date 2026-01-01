/***************************************************************************
  qgsmaprenderersequentialjob.cpp
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaprenderersequentialjob.h"

#include "qgslabelingresults.h"
#include "qgslogger.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgspallabeling.h"
#include "qgsrendereditemresults.h"

#include "moc_qgsmaprenderersequentialjob.cpp"

QgsMapRendererSequentialJob::QgsMapRendererSequentialJob( const QgsMapSettings &settings )
  : QgsMapRendererQImageJob( settings )

{
  QgsDebugMsgLevel( u"SEQUENTIAL construct"_s, 5 );

  mImage = QImage( mSettings.deviceOutputSize(), mSettings.outputImageFormat() );
  mImage.setDevicePixelRatio( mSettings.devicePixelRatio() );
  mImage.setDotsPerMeterX( 1000 * settings.outputDpi() / 25.4 );
  mImage.setDotsPerMeterY( 1000 * settings.outputDpi() / 25.4 );
  mImage.fill( Qt::transparent );
}

QgsMapRendererSequentialJob::~QgsMapRendererSequentialJob()
{
  QgsDebugMsgLevel( u"SEQUENTIAL destruct"_s, 5 );
  if ( isActive() )
  {
    // still running!
    QgsDebugMsgLevel( u"SEQUENTIAL destruct -- still running! (canceling)"_s, 5 );
    cancel();
  }

  Q_ASSERT( !mInternalJob && !mPainter );
}


void QgsMapRendererSequentialJob::startPrivate()
{
  if ( isActive() )
    return; // do nothing if we are already running

  mLabelingResults.reset();

  mRenderingStart.start();

  mErrors.clear();

  QgsDebugMsgLevel( u"SEQUENTIAL START"_s, 5 );

  Q_ASSERT( !mInternalJob && !mPainter );

  mPainter = new QPainter( &mImage );

  mInternalJob = new QgsMapRendererCustomPainterJob( mSettings, mPainter );
  mInternalJob->setLabelSink( labelSink() );
  mInternalJob->setCache( mCache );

  connect( mInternalJob, &QgsMapRendererJob::finished, this, &QgsMapRendererSequentialJob::internalFinished );
  connect( mInternalJob, &QgsMapRendererJob::layerRendered, this, &QgsMapRendererSequentialJob::layerRendered );
  connect( mInternalJob, &QgsMapRendererJob::layerRenderingStarted, this, &QgsMapRendererSequentialJob::layerRenderingStarted );

  mInternalJob->start();
}


void QgsMapRendererSequentialJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsgLevel( u"sequential - cancel internal"_s, 5 );
  // cppcheck-suppress nullPointerRedundantCheck
  mInternalJob->cancel();

  Q_ASSERT( !mInternalJob && !mPainter );
}

void QgsMapRendererSequentialJob::cancelWithoutBlocking()
{
  if ( !isActive() )
    return;

  QgsDebugMsgLevel( u"sequential - cancel internal"_s, 5 );
  mInternalJob->cancelWithoutBlocking();
}

void QgsMapRendererSequentialJob::waitForFinished()
{
  if ( !isActive() )
    return;

  mInternalJob->waitForFinished();
}

bool QgsMapRendererSequentialJob::isActive() const
{
  return nullptr != mInternalJob;
}

bool QgsMapRendererSequentialJob::usedCachedLabels() const
{
  return mUsedCachedLabels;
}

QgsLabelingResults *QgsMapRendererSequentialJob::takeLabelingResults()
{
  return mLabelingResults.release();
}


QImage QgsMapRendererSequentialJob::renderedImage()
{
  if ( isActive() && mCache )
    // this will allow immediate display of cached layers and at the same time updates of the layer being rendered
    return composeImage( mSettings, mInternalJob->jobs(), LabelRenderJob() );
  else
    return mImage;
}


void QgsMapRendererSequentialJob::internalFinished()
{
  QgsDebugMsgLevel( u"SEQUENTIAL finished"_s, 5 );

  mPainter->end();
  delete mPainter;
  mPainter = nullptr;

  mLabelingResults.reset( mInternalJob->takeLabelingResults() );
  mUsedCachedLabels = mInternalJob->usedCachedLabels();
  mLayersRedrawnFromCache = mInternalJob->layersRedrawnFromCache();

  mRenderedItemResults.reset( mInternalJob->takeRenderedItemResults() );

  mErrors = mInternalJob->errors();

  // now we are in a slot called from mInternalJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mInternalJob->deleteLater();
  mInternalJob = nullptr;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}
