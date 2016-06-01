/***************************************************************************
  qgsmaprendererparalleljob.cpp
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

#include "qgsmaprendererparalleljob.h"

#include "qgslabelingenginev2.h"
#include "qgslogger.h"
#include "qgsmaplayerrenderer.h"
#include "qgspallabeling.h"

#include <QtConcurrentMap>

#define LABELING_V2

QgsMapRendererParallelJob::QgsMapRendererParallelJob( const QgsMapSettings& settings )
    : QgsMapRendererQImageJob( settings )
    , mStatus( Idle )
    , mLabelingEngine( nullptr )
    , mLabelingEngineV2( nullptr )
{
}

QgsMapRendererParallelJob::~QgsMapRendererParallelJob()
{
  if ( isActive() )
  {
    cancel();
  }

  delete mLabelingEngine;
  mLabelingEngine = nullptr;

  delete mLabelingEngineV2;
  mLabelingEngineV2 = nullptr;
}

void QgsMapRendererParallelJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mStatus = RenderingLayers;

  delete mLabelingEngine;
  mLabelingEngine = nullptr;

  delete mLabelingEngineV2;
  mLabelingEngineV2 = nullptr;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
#ifdef LABELING_V2
    mLabelingEngineV2 = new QgsLabelingEngineV2();
    mLabelingEngineV2->readSettingsFromProject();
    mLabelingEngineV2->setMapSettings( mSettings );
#else
    mLabelingEngine = new QgsPalLabeling;
    mLabelingEngine->loadEngineSettings();
    mLabelingEngine->init( mSettings );
#endif
  }

  mLayerJobs = prepareJobs( nullptr, mLabelingEngine, mLabelingEngineV2 );
  // prepareJobs calls mapLayer->createMapRenderer may involve cloning a RasterDataProvider,
  // whose constructor may need to download some data (i.e. WMS, AMS) and doing so runs a
  // QEventLoop waiting for the network request to complete. If unluckily someone calls
  // mapCanvas->refresh() while this is happening, QgsMapRendererCustomPainterJob::cancel is
  // called, deleting the QgsMapRendererCustomPainterJob while this function is running.
  // Hence we need to check whether the job is still active before proceeding
  if ( !isActive() )
    return;

  QgsDebugMsg( QString( "QThreadPool max thread count is %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ) );

  // start async job

  connect( &mFutureWatcher, SIGNAL( finished() ), SLOT( renderLayersFinished() ) );

  mFuture = QtConcurrent::map( mLayerJobs, renderLayerStatic );
  mFutureWatcher.setFuture( mFuture );
}

void QgsMapRendererParallelJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsg( QString( "PARALLEL cancel at status %1" ).arg( mStatus ) );

  mLabelingRenderContext.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
  }

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( renderLayersFinished() ) );

    mFutureWatcher.waitForFinished();

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    mLabelingFutureWatcher.waitForFinished();

    renderingFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

void QgsMapRendererParallelJob::waitForFinished()
{
  if ( !isActive() )
    return;

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( renderLayersFinished() ) );

    QTime t;
    t.start();

    mFutureWatcher.waitForFinished();

    QgsDebugMsg( QString( "waitForFinished (1): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    QTime t;
    t.start();

    mLabelingFutureWatcher.waitForFinished();

    QgsDebugMsg( QString( "waitForFinished (2): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderingFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

bool QgsMapRendererParallelJob::isActive() const
{
  return mStatus != Idle;
}

QgsLabelingResults* QgsMapRendererParallelJob::takeLabelingResults()
{
  if ( mLabelingEngine )
    return mLabelingEngine->takeResults();
  else if ( mLabelingEngineV2 )
    return mLabelingEngineV2->takeResults();
  else
    return nullptr;
}

QImage QgsMapRendererParallelJob::renderedImage()
{
  if ( mStatus == RenderingLayers )
    return composeImage( mSettings, mLayerJobs );
  else
    return mFinalImage; // when rendering labels or idle
}

void QgsMapRendererParallelJob::renderLayersFinished()
{
  Q_ASSERT( mStatus == RenderingLayers );

  // compose final image
  mFinalImage = composeImage( mSettings, mLayerJobs );

  logRenderingTime( mLayerJobs );

  cleanupJobs( mLayerJobs );

  QgsDebugMsg( "PARALLEL layers finished" );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelingRenderContext.renderingStopped() )
  {
    mStatus = RenderingLabels;

    connect( &mLabelingFutureWatcher, SIGNAL( finished() ), this, SLOT( renderingFinished() ) );

    // now start rendering of labeling!
    mLabelingFuture = QtConcurrent::run( renderLabelsStatic, this );
    mLabelingFutureWatcher.setFuture( mLabelingFuture );
  }
  else
  {
    renderingFinished();
  }
}

void QgsMapRendererParallelJob::renderingFinished()
{
  QgsDebugMsg( "PARALLEL finished" );

  mStatus = Idle;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}

void QgsMapRendererParallelJob::renderLayerStatic( LayerRenderJob& job )
{
  if ( job.context.renderingStopped() )
    return;

  if ( job.cached )
    return;

  QTime t;
  t.start();
  QgsDebugMsg( QString( "job %1 start (layer %2)" ).arg( reinterpret_cast< ulong >( &job ), 0, 16 ).arg( job.layerId ) );

  try
  {
    job.renderer->render();
  }
  catch ( QgsException & e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception & e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromAscii( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( "Caught unhandled unknown exception" );
  }

  job.renderingTime = t.elapsed();
  QgsDebugMsg( QString( "job %1 end [%2 ms] (layer %3)" ).arg( reinterpret_cast< ulong >( &job ), 0, 16 ).arg( job.renderingTime ).arg( job.layerId ) );
}


void QgsMapRendererParallelJob::renderLabelsStatic( QgsMapRendererParallelJob* self )
{
  QPainter painter( &self->mFinalImage );

  try
  {
    drawLabeling( self->mSettings, self->mLabelingRenderContext, self->mLabelingEngine, self->mLabelingEngineV2, &painter );
  }
  catch ( QgsException & e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception & e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromAscii( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( "Caught unhandled unknown exception" );
  }

  painter.end();
}

