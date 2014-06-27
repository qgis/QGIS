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

#include "qgslogger.h"
#include "qgsmaplayerrenderer.h"
#include "qgspallabeling.h"


QgsMapRendererParallelJob::QgsMapRendererParallelJob( const QgsMapSettings& settings )
    : QgsMapRendererQImageJob( settings )
    , mStatus( Idle )
    , mLabelingEngine( 0 )
{
}

QgsMapRendererParallelJob::~QgsMapRendererParallelJob()
{
  if ( isActive() )
  {
    cancel();
  }

  delete mLabelingEngine;
  mLabelingEngine = 0;
}

void QgsMapRendererParallelJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mStatus = RenderingLayers;

  delete mLabelingEngine;
  mLabelingEngine = 0;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngine = new QgsPalLabeling;
    mLabelingEngine->loadEngineSettings();
    mLabelingEngine->init( mSettings );
  }


  mLayerJobs = prepareJobs( 0, mLabelingEngine );

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
  return mLabelingEngine ? mLabelingEngine->takeResults() : 0;
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
  QgsDebugMsg( QString( "job %1 start" ).arg(( ulong ) &job, 0, 16 ) );

  try
  {
    job.renderer->render();
  }
  catch ( QgsException & e )
  {
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception & e )
  {
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromAscii( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( "Caught unhandled unknown exception" );
  }

  int tt = t.elapsed();
  QgsDebugMsg( QString( "job %1 end [%2 ms]" ).arg(( ulong ) &job, 0, 16 ).arg( tt ) );
  Q_UNUSED( tt );
}


void QgsMapRendererParallelJob::renderLabelsStatic( QgsMapRendererParallelJob* self )
{
  QPainter painter( &self->mFinalImage );

  try
  {
    drawLabeling( self->mSettings, self->mLabelingRenderContext, self->mLabelingEngine, &painter );
  }
  catch ( QgsException & e )
  {
    QgsDebugMsg( "Caught unhandled QgsException: " + e.what() );
  }
  catch ( std::exception & e )
  {
    QgsDebugMsg( "Caught unhandled std::exception: " + QString::fromAscii( e.what() ) );
  }
  catch ( ... )
  {
    QgsDebugMsg( "Caught unhandled unknown exception" );
  }

  painter.end();
}

