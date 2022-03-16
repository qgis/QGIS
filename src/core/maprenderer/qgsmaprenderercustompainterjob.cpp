/***************************************************************************
  qgsmaprenderercustompainterjob.cpp
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

#include "qgsmaprenderercustompainterjob.h"

#include "qgsfeedback.h"
#include "qgslabelingengine.h"
#include "qgslogger.h"
#include "qgsmaplayerrenderer.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsvectorlayerlabeling.h"

#include <QtConcurrentRun>

//
// QgsMapRendererAbstractCustomPainterJob
//

QgsMapRendererAbstractCustomPainterJob::QgsMapRendererAbstractCustomPainterJob( const QgsMapSettings &settings )
  : QgsMapRendererJob( settings )
{

}

void QgsMapRendererAbstractCustomPainterJob::preparePainter( QPainter *painter, const QColor &backgroundColor )
{
  // clear the background
  painter->fillRect( 0, 0, mSettings.deviceOutputSize().width(), mSettings.deviceOutputSize().height(), backgroundColor );

  painter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( Qgis::MapSettingsFlag::Antialiasing ) );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, mSettings.testFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms ) );
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
  painter->setRenderHint( QPainter::LosslessImageRendering, mSettings.testFlag( Qgis::MapSettingsFlag::LosslessImageRendering ) );
#endif

#ifndef QT_NO_DEBUG
  QPaintDevice *paintDevice = painter->device();
  const QString errMsg = QStringLiteral( "pre-set DPI not equal to painter's DPI (%1 vs %2)" )
                         .arg( paintDevice->logicalDpiX() )
                         .arg( mSettings.outputDpi() );
  Q_ASSERT_X( qgsDoubleNear( paintDevice->logicalDpiX(), mSettings.outputDpi(), 1.0 ),
              "Job::startRender()", errMsg.toLatin1().data() );
#endif
}


//
// QgsMapRendererCustomPainterJob
//

QgsMapRendererCustomPainterJob::QgsMapRendererCustomPainterJob( const QgsMapSettings &settings, QPainter *painter )
  : QgsMapRendererAbstractCustomPainterJob( settings )
  , mPainter( painter )
  , mActive( false )
  , mRenderSynchronously( false )
{
  QgsDebugMsgLevel( QStringLiteral( "QPAINTER construct" ), 5 );
}

QgsMapRendererCustomPainterJob::~QgsMapRendererCustomPainterJob()
{
  QgsDebugMsgLevel( QStringLiteral( "QPAINTER destruct" ), 5 );
  Q_ASSERT( !mFutureWatcher.isRunning() );
  //cancel();
}

void QgsMapRendererCustomPainterJob::startPrivate()
{
  if ( isActive() )
    return;

  if ( !mPrepareOnly )
    mRenderingStart.start();

  mActive = true;

  mErrors.clear();

  QgsDebugMsgLevel( QStringLiteral( "QPAINTER run!" ), 5 );

  QgsDebugMsgLevel( QStringLiteral( "Preparing list of layer jobs for rendering" ), 5 );
  QElapsedTimer prepareTime;
  prepareTime.start();

  preparePainter( mPainter, mSettings.backgroundColor() );

  mLabelingEngineV2.reset();

  if ( mSettings.testFlag( Qgis::MapSettingsFlag::DrawLabeling ) )
  {
    mLabelingEngineV2.reset( new QgsDefaultLabelingEngine() );
    mLabelingEngineV2->setMapSettings( mSettings );
  }

  const bool canUseLabelCache = prepareLabelCache();
  mLayerJobs = prepareJobs( mPainter, mLabelingEngineV2.get() );
  mLabelJob = prepareLabelingJob( mPainter, mLabelingEngineV2.get(), canUseLabelCache );
  mSecondPassLayerJobs = prepareSecondPassJobs( mLayerJobs, mLabelJob );

  QgsDebugMsgLevel( QStringLiteral( "Rendering prepared in (seconds): %1" ).arg( prepareTime.elapsed() / 1000.0 ), 4 );

  if ( mRenderSynchronously )
  {
    if ( !mPrepareOnly )
    {
      // do the rendering right now!
      doRender();
    }
    return;
  }

  // now we are ready to start rendering!
  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererCustomPainterJob::futureFinished );

  mFuture = QtConcurrent::run( staticRender, this );
  mFutureWatcher.setFuture( mFuture );
}


void QgsMapRendererCustomPainterJob::cancel()
{
  if ( !isActive() )
  {
    QgsDebugMsgLevel( QStringLiteral( "QPAINTER not running!" ), 4 );
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "QPAINTER canceling" ), 5 );
  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererCustomPainterJob::futureFinished );
  cancelWithoutBlocking();

  QElapsedTimer t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsgLevel( QStringLiteral( "QPAINER cancel waited %1 ms" ).arg( t.elapsed() / 1000.0 ), 5 );

  futureFinished();

  QgsDebugMsgLevel( QStringLiteral( "QPAINTER canceled" ), 5 );
}

void QgsMapRendererCustomPainterJob::cancelWithoutBlocking()
{
  if ( !isActive() )
  {
    QgsDebugMsg( QStringLiteral( "QPAINTER not running!" ) );
    return;
  }

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJob &job : mLayerJobs )
  {
    job.context()->setRenderingStopped( true );
    if ( job.renderer && job.renderer->feedback() )
      job.renderer->feedback()->cancel();
  }
}

void QgsMapRendererCustomPainterJob::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererCustomPainterJob::futureFinished );

  QElapsedTimer t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsgLevel( QStringLiteral( "waitForFinished: %1 ms" ).arg( t.elapsed() / 1000.0 ), 4 );

  futureFinished();
}

bool QgsMapRendererCustomPainterJob::isActive() const
{
  return mActive;
}

bool QgsMapRendererCustomPainterJob::usedCachedLabels() const
{
  return mLabelJob.cached;
}

QgsLabelingResults *QgsMapRendererCustomPainterJob::takeLabelingResults()
{
  if ( mLabelingEngineV2 )
    return mLabelingEngineV2->takeResults();
  else
    return nullptr;
}


void QgsMapRendererCustomPainterJob::waitForFinishedWithEventLoop( QEventLoop::ProcessEventsFlags flags )
{
  QEventLoop loop;
  connect( &mFutureWatcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit );
  loop.exec( flags );
}


void QgsMapRendererCustomPainterJob::renderSynchronously()
{
  mRenderSynchronously = true;
  start();
  futureFinished();
  mRenderSynchronously = false;
}

void QgsMapRendererCustomPainterJob::prepare()
{
  mRenderSynchronously = true;
  mPrepareOnly = true;
  start();
  mPrepared = true;
}

void QgsMapRendererCustomPainterJob::renderPrepared()
{
  if ( !mPrepared )
    return;

  doRender();
  futureFinished();
  mRenderSynchronously = false;
  mPrepareOnly = false;
  mPrepared = false;
}

void QgsMapRendererCustomPainterJob::futureFinished()
{
  mActive = false;
  if ( !mPrepared ) // can't access from other thread
    mRenderingTime = mRenderingStart.elapsed();
  QgsDebugMsgLevel( QStringLiteral( "QPAINTER futureFinished" ), 5 );

  if ( !mPrepared )
    logRenderingTime( mLayerJobs, {}, mLabelJob );

  // final cleanup
  cleanupJobs( mLayerJobs );
  cleanupSecondPassJobs( mSecondPassLayerJobs );
  cleanupLabelJob( mLabelJob );

  emit finished();
}


void QgsMapRendererCustomPainterJob::staticRender( QgsMapRendererCustomPainterJob *self )
{
  try
  {
    self->doRender();
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
}

void QgsMapRendererCustomPainterJob::doRender()
{
  const bool hasSecondPass = ! mSecondPassLayerJobs.empty();
  QgsDebugMsgLevel( QStringLiteral( "Starting to render layer stack." ), 5 );
  QElapsedTimer renderTime;
  renderTime.start();

  for ( LayerRenderJob &job : mLayerJobs )
  {
    if ( job.context()->renderingStopped() )
      break;

    emit layerRenderingStarted( job.layerId );

    if ( ! hasSecondPass && job.context()->useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mPainter->setCompositionMode( job.blendMode );
    }

    if ( !job.cached )
    {
      QElapsedTimer layerTime;
      layerTime.start();

      if ( job.img )
      {
        job.img->fill( 0 );
        job.imageInitialized = true;
      }

      job.completed = job.renderer->render();

      job.renderingTime += layerTime.elapsed();
    }

    if ( ! hasSecondPass && job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      mPainter->setOpacity( job.opacity );
      mPainter->drawImage( 0, 0, *job.img );
      mPainter->setOpacity( 1.0 );
    }

    emit layerRendered( job.layerId );
  }

  emit renderingLayersFinished();
  QgsDebugMsgLevel( QStringLiteral( "Done rendering map layers" ), 5 );

  if ( mSettings.testFlag( Qgis::MapSettingsFlag::DrawLabeling ) && !mLabelJob.context.renderingStopped() )
  {
    if ( !mLabelJob.cached )
    {
      QElapsedTimer labelTime;
      labelTime.start();

      if ( mLabelJob.img )
      {
        QPainter painter;
        mLabelJob.img->fill( 0 );
        painter.begin( mLabelJob.img );
        mLabelJob.context.setPainter( &painter );
        drawLabeling( mLabelJob.context, mLabelingEngineV2.get(), &painter );
        painter.end();
      }
      else
      {
        drawLabeling( mLabelJob.context, mLabelingEngineV2.get(), mPainter );
      }

      mLabelJob.complete = true;
      mLabelJob.renderingTime = labelTime.elapsed();
      mLabelJob.participatingLayers = _qgis_listRawToQPointer( mLabelingEngineV2->participatingLayers() );
    }
  }

  if ( ! hasSecondPass )
  {
    if ( mLabelJob.img && mLabelJob.complete )
    {
      mPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
      mPainter->setOpacity( 1.0 );
      mPainter->drawImage( 0, 0, *mLabelJob.img );
    }
  }
  else
  {
    for ( LayerRenderJob &job : mSecondPassLayerJobs )
    {
      if ( job.context()->renderingStopped() )
        break;

      if ( !job.cached )
      {
        QElapsedTimer layerTime;
        layerTime.start();

        if ( job.img )
        {
          job.img->fill( 0 );
          job.imageInitialized = true;
        }

        job.completed = job.renderer->render();

        job.renderingTime += layerTime.elapsed();
      }
    }

    composeSecondPass( mSecondPassLayerJobs, mLabelJob );

    const QImage finalImage = composeImage( mSettings, mLayerJobs, mLabelJob );

    mPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
    mPainter->setOpacity( 1.0 );
    mPainter->drawImage( 0, 0, finalImage );
  }

  QgsDebugMsgLevel( QStringLiteral( "Rendering completed in (seconds): %1" ).arg( renderTime.elapsed() / 1000.0 ), 2 );
}


