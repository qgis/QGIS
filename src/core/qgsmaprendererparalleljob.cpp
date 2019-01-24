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
#include "qgsmaplayerlistutils.h"

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

void QgsMapRendererParallelJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mStatus = RenderingLayers;

  mLabelingEngineV2.reset();

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngineV2.reset( new QgsLabelingEngine() );
    mLabelingEngineV2->setMapSettings( mSettings );
  }

  bool canUseLabelCache = prepareLabelCache();
  mLayerJobs = prepareJobs( nullptr, mLabelingEngineV2.get() );
  mLabelJob = prepareLabelingJob( nullptr, mLabelingEngineV2.get(), canUseLabelCache );

  QgsDebugMsg( QStringLiteral( "QThreadPool max thread count is %1" ).arg( QThreadPool::globalInstance()->maxThreadCount() ) );

  // start async job

  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderLayersFinished );

  mFuture = QtConcurrent::map( mLayerJobs, renderLayerStatic );
  mFutureWatcher.setFuture( mFuture );
}

void QgsMapRendererParallelJob::cancel()
{
  if ( !isActive() )
    return;

  QgsDebugMsg( QStringLiteral( "PARALLEL cancel at status %1" ).arg( mStatus ) );

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
    if ( it->renderer && it->renderer->feedback() )
      it->renderer->feedback()->cancel();
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

  Q_ASSERT( mStatus == Idle );
}

void QgsMapRendererParallelJob::cancelWithoutBlocking()
{
  if ( !isActive() )
    return;

  QgsDebugMsg( QStringLiteral( "PARALLEL cancel at status %1" ).arg( mStatus ) );

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
    if ( it->renderer && it->renderer->feedback() )
      it->renderer->feedback()->cancel();
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

    QTime t;
    t.start();

    mFutureWatcher.waitForFinished();

    QgsDebugMsg( QStringLiteral( "waitForFinished (1): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderLayersFinished();
  }

  if ( mStatus == RenderingLabels )
  {
    disconnect( &mLabelingFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererParallelJob::renderingFinished );

    QTime t;
    t.start();

    mLabelingFutureWatcher.waitForFinished();

    QgsDebugMsg( QStringLiteral( "waitForFinished (2): %1 ms" ).arg( t.elapsed() / 1000.0 ) );

    renderingFinished();
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
  if ( mStatus == RenderingLayers )
    return composeImage( mSettings, mLayerJobs, mLabelJob );
  else
    return mFinalImage; // when rendering labels or idle
}

void QgsMapRendererParallelJob::renderLayersFinished()
{
  Q_ASSERT( mStatus == RenderingLayers );

  LayerRenderJobs::const_iterator it = mLayerJobs.constBegin();
  for ( ; it != mLayerJobs.constEnd(); ++it )
  {
    if ( !it->errors.isEmpty() )
    {
      mErrors.append( Error( it->layer->id(), it->errors.join( ',' ) ) );
    }
  }

  // compose final image
  mFinalImage = composeImage( mSettings, mLayerJobs, mLabelJob );

  QgsDebugMsg( QStringLiteral( "PARALLEL layers finished" ) );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelJob.context.renderingStopped() )
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

void QgsMapRendererParallelJob::renderingFinished()
{
  QgsDebugMsg( QStringLiteral( "PARALLEL finished" ) );

  logRenderingTime( mLayerJobs, mLabelJob );

  cleanupJobs( mLayerJobs );

  cleanupLabelJob( mLabelJob );

  mStatus = Idle;

  mRenderingTime = mRenderingStart.elapsed();

  emit finished();
}

void QgsMapRendererParallelJob::renderLayerStatic( LayerRenderJob &job )
{
  if ( job.context.renderingStopped() )
    return;

  if ( job.cached )
    return;

  if ( job.img )
  {
    job.img->fill( 0 );
    job.imageInitialized = true;
  }

  QTime t;
  t.start();
  QgsDebugMsgLevel( QStringLiteral( "job %1 start (layer %2)" ).arg( reinterpret_cast< quint64 >( &job ), 0, 16 ).arg( job.layer ? job.layer->id() : QString() ), 2 );
  try
  {
    job.renderer->render();
  }
  catch ( QgsException &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception &e )
  {
    Q_UNUSED( e );
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromLatin1( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( QStringLiteral( "Caught unhandled unknown exception" ) );
  }

  job.errors = job.renderer->errors();
  job.renderingTime += t.elapsed();
  QgsDebugMsgLevel( QStringLiteral( "job %1 end [%2 ms] (layer %3)" ).arg( reinterpret_cast< quint64 >( &job ), 0, 16 ).arg( job.renderingTime ).arg( job.layer ? job.layer->id() : QString() ), 2 );
}


void QgsMapRendererParallelJob::renderLabelsStatic( QgsMapRendererParallelJob *self )
{
  LabelRenderJob &job = self->mLabelJob;

  if ( !job.cached )
  {
    QTime labelTime;
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
      Q_UNUSED( e );
      QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
    }
    catch ( std::exception &e )
    {
      Q_UNUSED( e );
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

