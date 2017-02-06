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
#include "qgsproject.h"
#include "qgsmaplayerrenderer.h"
#include "qgspallabeling.h"
#include "qgsvectorlayer.h"
#include "qgsrenderer.h"
#include "qgsmaplayerlistutils.h"

QgsMapRendererCustomPainterJob::QgsMapRendererCustomPainterJob( const QgsMapSettings& settings, QPainter* painter )
    : QgsMapRendererJob( settings )
    , mPainter( painter )
    , mLabelingEngineV2( nullptr )
    , mActive( false )
    , mRenderSynchronously( false )
{
  QgsDebugMsg( "QPAINTER construct" );
}

QgsMapRendererCustomPainterJob::~QgsMapRendererCustomPainterJob()
{
  QgsDebugMsg( "QPAINTER destruct" );
  Q_ASSERT( !mFutureWatcher.isRunning() );
  //cancel();

  delete mLabelingEngineV2;
  mLabelingEngineV2 = nullptr;
}

void QgsMapRendererCustomPainterJob::start()
{
  if ( isActive() )
    return;

  mRenderingStart.start();

  mActive = true;

  mErrors.clear();

  QgsDebugMsg( "QPAINTER run!" );

  QgsDebugMsg( "Preparing list of layer jobs for rendering" );
  QTime prepareTime;
  prepareTime.start();

  // clear the background
  mPainter->fillRect( 0, 0, mSettings.outputSize().width(), mSettings.outputSize().height(), mSettings.backgroundColor() );

  mPainter->setRenderHint( QPainter::Antialiasing, mSettings.testFlag( QgsMapSettings::Antialiasing ) );

#ifndef QT_NO_DEBUG
  QPaintDevice* thePaintDevice = mPainter->device();
  QString errMsg = QStringLiteral( "pre-set DPI not equal to painter's DPI (%1 vs %2)" ).arg( thePaintDevice->logicalDpiX() ).arg( mSettings.outputDpi() );
  Q_ASSERT_X( qgsDoubleNear( thePaintDevice->logicalDpiX(), mSettings.outputDpi() ), "Job::startRender()", errMsg.toLatin1().data() );
#endif

  delete mLabelingEngineV2;
  mLabelingEngineV2 = nullptr;

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) )
  {
    mLabelingEngineV2 = new QgsLabelingEngine();
    mLabelingEngineV2->readSettingsFromProject( QgsProject::instance() );
    mLabelingEngineV2->setMapSettings( mSettings );
  }

  bool canUseLabelCache = prepareLabelCache();
  mLayerJobs = prepareJobs( mPainter, mLabelingEngineV2 );
  mLabelJob = prepareLabelingJob( mPainter, mLabelingEngineV2, canUseLabelCache );

  QgsDebugMsg( "Rendering prepared in (seconds): " + QString( "%1" ).arg( prepareTime.elapsed() / 1000.0 ) );

  if ( mRenderSynchronously )
  {
    // do the rendering right now!
    doRender();
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
    QgsDebugMsg( "QPAINTER not running!" );
    return;
  }

  QgsDebugMsg( "QPAINTER canceling" );
  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererCustomPainterJob::futureFinished );

  mLabelJob.context.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
    if ( it->renderer && it->renderer->feedback() )
      it->renderer->feedback()->cancel();
  }

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsg( QString( "QPAINER cancel waited %1 ms" ).arg( t.elapsed() / 1000.0 ) );

  futureFinished();

  QgsDebugMsg( "QPAINTER canceled" );
}

void QgsMapRendererCustomPainterJob::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, &QFutureWatcher<void>::finished, this, &QgsMapRendererCustomPainterJob::futureFinished );

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsg( QString( "waitForFinished: %1 ms" ).arg( t.elapsed() / 1000.0 ) );

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

QgsLabelingResults* QgsMapRendererCustomPainterJob::takeLabelingResults()
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


void QgsMapRendererCustomPainterJob::futureFinished()
{
  mActive = false;
  mRenderingTime = mRenderingStart.elapsed();
  QgsDebugMsg( "QPAINTER futureFinished" );

  logRenderingTime( mLayerJobs, mLabelJob );

  // final cleanup
  cleanupJobs( mLayerJobs );
  cleanupLabelJob( mLabelJob );

  emit finished();
}


void QgsMapRendererCustomPainterJob::staticRender( QgsMapRendererCustomPainterJob* self )
{
  try
  {
    self->doRender();
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
}

void QgsMapRendererCustomPainterJob::doRender()
{
  QgsDebugMsg( "Starting to render layer stack." );
  QTime renderTime;
  renderTime.start();

  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    LayerRenderJob& job = *it;

    if ( job.context.renderingStopped() )
      break;

    if ( job.context.useAdvancedEffects() )
    {
      // Set the QPainter composition mode so that this layer is rendered using
      // the desired blending mode
      mPainter->setCompositionMode( job.blendMode );
    }

    if ( !job.cached )
    {
      QTime layerTime;
      layerTime.start();

      if ( job.img )
        job.img->fill( 0 );

      job.renderer->render();

      job.renderingTime = layerTime.elapsed();
    }

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      mPainter->setOpacity( job.opacity );
      mPainter->drawImage( 0, 0, *job.img );
      mPainter->setOpacity( 1.0 );
    }

  }

  QgsDebugMsg( "Done rendering map layers" );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelJob.context.renderingStopped() )
  {
    if ( !mLabelJob.cached )
    {
      QTime labelTime;
      labelTime.start();

      if ( mLabelJob.img )
      {
        QPainter painter;
        mLabelJob.img->fill( 0 );
        painter.begin( mLabelJob.img );
        mLabelJob.context.setPainter( &painter );
        drawLabeling( mSettings, mLabelJob.context, mLabelingEngineV2, &painter );
        painter.end();
      }
      else
      {
        drawLabeling( mSettings, mLabelJob.context, mLabelingEngineV2, mPainter );
      }

      mLabelJob.complete = true;
      mLabelJob.renderingTime = labelTime.elapsed();
      mLabelJob.participatingLayers = _qgis_listRawToQPointer( mLabelingEngineV2->participatingLayers() );
    }
  }
  if ( mLabelJob.img && mLabelJob.complete )
  {
    mPainter->setCompositionMode( QPainter::CompositionMode_SourceOver );
    mPainter->setOpacity( 1.0 );
    mPainter->drawImage( 0, 0, *mLabelJob.img );
  }

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );
}


void QgsMapRendererJob::drawLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsLabelingEngine* labelingEngine2, QPainter* painter )
{
  QgsDebugMsg( "Draw labeling start" );

  QTime t;
  t.start();

  // Reset the composition mode before rendering the labels
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  // TODO: this is not ideal - we could override rendering stopped flag that has been set in meanwhile
  renderContext = QgsRenderContext::fromMapSettings( settings );
  renderContext.setPainter( painter );

  if ( labelingEngine2 )
  {
    // set correct extent
    renderContext.setExtent( settings.visibleExtent() );
    renderContext.setCoordinateTransform( QgsCoordinateTransform() );

    labelingEngine2->run( renderContext );
  }

  QgsDebugMsg( QString( "Draw labeling took (seconds): %1" ).arg( t.elapsed() / 1000. ) );
}


void QgsMapRendererJob::updateLayerGeometryCaches()
{
  QMap<QString, QgsGeometryCache>::const_iterator it = mGeometryCaches.constBegin();
  for ( ; it != mGeometryCaches.constEnd(); ++it )
  {
    const QgsGeometryCache& cache = it.value();
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsProject::instance()->mapLayer( it.key() ) ) )
      * vl->cache() = cache;
  }
  mGeometryCaches.clear();
}


bool QgsMapRendererJob::needTemporaryImage( QgsMapLayer* ml )
{
  if ( ml->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( vl->renderer() && vl->renderer()->forceRasterRender() )
    {
      //raster rendering is forced for this layer
      return true;
    }
    if ( mSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) &&
         (( vl->blendMode() != QPainter::CompositionMode_SourceOver )
          || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
          || ( vl->layerTransparency() != 0 ) ) )
    {
      //layer properties require rasterization
      return true;
    }
  }
  else if ( ml->type() == QgsMapLayer::RasterLayer )
  {
    // preview of intermediate raster rendering results requires a temporary output image
    if ( mSettings.testFlag( QgsMapSettings::RenderPartialOutput ) )
      return true;
  }

  return false;
}

