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

#include "qgsfeedback.h"
#include "qgslabelingengine.h"
#include "qgslogger.h"
#include "qgsmaplayerrenderer.h"
#include "qgsproject.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlistutils_p.h"

#include <QtConcurrentMap>
#include <QtConcurrentRun>

QgsMapRendererParallelJob::QgsMapRendererParallelJob( const QgsMapSettings &settings )
  : QgsMapRendererQImageJob( settings )
  , mStatus( Idle )
{
}

QgsMapRendererParallelJob::~QgsMapRendererParallelJob()
{
  if ( isActive() )
  {
    cancel();
  }
}

void QgsMapRendererParallelJob::startPrivate()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mStatus = RenderingLayers;

  mLabelingEngineV2.reset();

  if ( mSettings.testFlag( Qgis::MapSettingsFlag::DrawLabeling ) )
  {
    mLabelingEngineV2.reset( new QgsDefaultLabelingEngine() );
    mLabelingEngineV2->setMapSettings( mSettings );
  }

  const bool canUseLabelCache = prepareLabelCache();
  mLayerJobs = prepareJobs( nullptr, mLabelingEngineV2.get() );
  mLabelJob = prepareLabelingJob( nullptr, mLabelingEngineV2.get(), canUseLabelCache );
  mSecondPassLayerJobs = prepareSecondPassJobs( mLayerJobs, mLabelJob );

  QgsDebugMsgLevel( QStringLiteral( "QThreadPool max thread count is %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ), 2 );

  // start async job

  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersFinished );

  mFuture = QtConcurrent::map( mLayerJobs, renderLayerStatic );
  mFutureWatcher.setFuture( mFuture );
}

void QgsMapRendererParallelJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsgLevel( QStringLiteral( "PARALLEL cancel at status %1" ).arg( mStatus ), 2 );

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJob &job : mLayerJobs )
  {
    job.context()->setRenderingStopped( true );
    if ( job.renderer && job.renderer->feedback() )
      job.renderer->feedback()->cancel();
  }

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersFinished );

    mFutureWatcher.waitForFinished();

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderingFinished );

    mLabelingFutureWatcher.waitForFinished();

    renderingFinished();
  }

  if ( mStatus == RenderingSecondPass )
  {
    disconnect( &mSecondPassFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersSecondPassFinished );

    mSecondPassFutureWatcher.waitForFinished();

    renderLayersSecondPassFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

void QgsMapRendererParallelJob::cancelWithoutBlocking()
{
  if ( !isActive() )
    return;

  QgsDebugMsgLevel( QStringLiteral( "PARALLEL cancel at status %1" ).arg( mStatus ), 2 );

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJob &job : mLayerJobs )
  {
    job.context()->setRenderingStopped( true );
    if ( job.renderer && job.renderer->feedback() )
      job.renderer->feedback()->cancel();
  }

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersFinished );
    connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderingFinished );
  }
}

void QgsMapRendererParallelJob::waitForFinished()
{
  if ( !isActive() )
    return;

  if ( mStatus == RenderingLayers )
  {
    disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersFinished );

    QElapsedTimer t;
    t.start();

    mFutureWatcher.waitForFinished();

    QgsDebugMsgLevel( QStringLiteral( "waitForFinished (1): %1 ms" ).arg( t.elapsed() / 1000.0 ), 2 );

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderingFinished );

    QElapsedTimer t;
    t.start();

    mLabelingFutureWatcher.waitForFinished();

    QgsDebugMsgLevel( QStringLiteral( "waitForFinished (2): %1 ms" ).arg( t.elapsed() / 1000.0 ), 2 );

    renderingFinished();
  }

  if ( mStatus == RenderingSecondPass )
  {
    disconnect( &mSecondPassFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersSecondPassFinished );

    QElapsedTimer t;
    t.start();

    mSecondPassFutureWatcher.waitForFinished();

    QgsDebugMsg( QStringLiteral( "waitForFinished (1): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderLayersSecondPassFinished();
  }

  Q_ASSERT( mStatus == Idle );
}

bool QgsMapRendererParallelJob::isActive() const
{
  return mStatus != Idle;
}

bool QgsMapRendererParallelJob::usedCachedLabels() const
{
  return mLabelJob.cached;
}

QgsLabelingResults *QgsMapRendererParallelJob::takeLabelingResults()
{
  if ( mLabelingEngineV2 )
    return mLabelingEngineV2->takeResults();
  else
    return nullptr;
}

QImage QgsMapRendererParallelJob::renderedImage()
{
  // if status == Idle we are either waiting for the render to start, OR have finished the render completely.
  // We can differentiate between those states by checking whether mFinalImage is null -- at the "waiting for
  // render to start" state mFinalImage has not yet been created.
  const bool jobIsComplete = mStatus == Idle && !mFinalImage.isNull();

  if ( !jobIsComplete )
    return composeImage( mSettings, mLayerJobs, mLabelJob, mCache );
  else
    return mFinalImage; // when rendering labels or idle
}

void QgsMapRendererParallelJob::renderLayersFinished()
{
  Q_ASSERT( mStatus == RenderingLayers );

  for ( const LayerRenderJob &job : mLayerJobs )
  {
    if ( !job.errors.isEmpty() )
    {
      mErrors.append( Error( job.layerId, job.errors.join( ',' ) ) );
    }
  }

  // compose final image for labeling
  if ( mSecondPassLayerJobs.empty() )
  {
    mFinalImage = composeImage( mSettings, mLayerJobs, mLabelJob, mCache );
  }

  QgsDebugMsgLevel( QStringLiteral( "PARALLEL layers finished" ), 2 );

  if ( mSettings.testFlag( Qgis::MapSettingsFlag::DrawLabeling ) && !mLabelJob.context.renderingStopped() )
  {
    mStatus = RenderingLabels;

    connect( &mLabelingFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderingFinished );

    // now start rendering of labeling!
    mLabelingFuture = QtConcurrent::run( renderLabelsStatic, this );
    mLabelingFutureWatcher.setFuture( mLabelingFuture );
    emit renderingLayersFinished();
  }
  else
  {
    renderingFinished();
  }
}

#define DEBUG_RENDERING 0

void QgsMapRendererParallelJob::renderingFinished()
{
#if DEBUG_RENDERING
  int i = 0;
  for ( LayerRenderJob &job : mLayerJobs )
  {
    if ( job.img )
    {
      job.img->save( QString( "/tmp/first_pass_%1.png" ).arg( i ) );
    }
    if ( job.maskPass.image )
    {
      job.maskPass.image->save( QString( "/tmp/first_pass_%1_mask.png" ).arg( i ) );
    }
    i++;
  }
  if ( mLabelJob.img )
  {
    mLabelJob.img->save( QString( "/tmp/labels.png" ) );
  }
  if ( mLabelJob.maskImage )
  {
    mLabelJob.maskImage->save( QString( "/tmp/labels_mask.png" ) );
  }
#endif
  if ( ! mSecondPassLayerJobs.empty() )
  {
    mStatus = RenderingSecondPass;
    // We have a second pass to do.
    connect( &mSecondPassFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersSecondPassFinished );
    mSecondPassFuture = QtConcurrent::map( mSecondPassLayerJobs, renderLayerStatic );
    mSecondPassFutureWatcher.setFuture( mSecondPassFuture );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "PARALLEL finished" ), 2 );

    logRenderingTime( mLayerJobs, mSecondPassLayerJobs, mLabelJob );

    cleanupJobs( mLayerJobs );

    cleanupLabelJob( mLabelJob );

    mStatus = Idle;

    mRenderingTime = mRenderingStart.elapsed();

    emit finished();
  }
}

void QgsMapRendererParallelJob::renderLayersSecondPassFinished()
{
  QgsDebugMsgLevel( QStringLiteral( "PARALLEL finished" ), 2 );

  // compose second pass images into first pass images
  composeSecondPass( mSecondPassLayerJobs, mLabelJob );

  // compose final image
  mFinalImage = composeImage( mSettings, mLayerJobs, mLabelJob );

  logRenderingTime( mLayerJobs, mSecondPassLayerJobs, mLabelJob );

  cleanupJobs( mLayerJobs );

  cleanupSecondPassJobs( mSecondPassLayerJobs );

  cleanupLabelJob( mLabelJob );

  mStatus = Idle;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}

/*
 * See section "Smarter Map Redraws"
 * in https://github.com/qgis/QGIS-Enhancement-Proposals/issues/181
 */
// #define SIMULATE_SLOW_RENDERER

void QgsMapRendererParallelJob::renderLayerStatic( LayerRenderJob &job )
{
  if ( job.context()->renderingStopped() )
    return;

  if ( job.cached )
    return;

  if ( job.img )
  {
    job.img->fill( 0 );
    job.imageInitialized = true;
  }

  QElapsedTimer t;
  t.start();
  QgsDebugMsgLevel( QStringLiteral( "job %1 start (layer %2)" ).arg( reinterpret_cast< quint64 >( &job ), 0, 16 ).arg( job.layerId ), 2 );
  try
  {
#ifdef SIMULATE_SLOW_RENDERER
    QThread::sleep( 1 );
#endif
    job.completed = job.renderer->render();
  }
  catch ( QgsException &e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e )
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromLatin1( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( QStringLiteral( "Caught unhandled unknown exception" ) );
  }

  job.errors = job.renderer->errors();
  job.renderingTime += t.elapsed();
  QgsDebugMsgLevel( QStringLiteral( "job %1 end [%2 ms] (layer %3)" ).arg( reinterpret_cast< quint64 >( &job ), 0, 16 ).arg( job.renderingTime ).arg( job.layerId ), 2 );
}


void QgsMapRendererParallelJob::renderLabelsStatic( QgsMapRendererParallelJob *self )
{
  LabelRenderJob &job = self->mLabelJob;

  if ( !job.cached )
  {
    QElapsedTimer labelTime;
    labelTime.start();

    QPainter painter;
    if ( job.img )
    {
      job.img->fill( 0 );
      painter.begin( job.img );
    }
    else
    {
      painter.begin( &self->mFinalImage );
    }

    // draw the labels!
    try
    {
      drawLabeling( job.context, self->mLabelingEngineV2.get(), &painter );
    }
    catch ( QgsException &e )
    {
      Q_UNUSED( e )
      QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
    }
    catch ( std::exception &e )
    {
      Q_UNUSED( e )
      QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromLatin1( e.what() ) );
    }
    catch ( ... )
    {
      QgsDebugMsg( QStringLiteral( "Caught unhandled unknown exception" ) );
    }

    painter.end();

    job.renderingTime = labelTime.elapsed();
    job.complete = true;
    job.participatingLayers = _qgis_listRawToQPointer( self->mLabelingEngineV2->participatingLayers() );
    if ( job.img )
    {
      self->mFinalImage = composeImage( self->mSettings, self->mLayerJobs, self->mLabelJob );
    }
  }
}

