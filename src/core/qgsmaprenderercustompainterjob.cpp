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

#include "qgslabelingenginev2.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerrenderer.h"
#include "qgspallabeling.h"
#include "qgsvectorlayer.h"
#include "qgsrendererv2.h"

#define LABELING_V2

QgsMapRendererCustomPainterJob::QgsMapRendererCustomPainterJob( const QgsMapSettings& settings, QPainter* painter )
    : QgsMapRendererJob( settings )
    , mPainter( painter )
    , mLabelingEngine( nullptr )
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

  delete mLabelingEngine;
  mLabelingEngine = nullptr;

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
  QString errMsg = QString( "pre-set DPI not equal to painter's DPI (%1 vs %2)" ).arg( thePaintDevice->logicalDpiX() ).arg( mSettings.outputDpi() );
  Q_ASSERT_X( qgsDoubleNear( thePaintDevice->logicalDpiX(), mSettings.outputDpi() ), "Job::startRender()", errMsg.toAscii().data() );
#endif

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

  mLayerJobs = prepareJobs( mPainter, mLabelingEngine, mLabelingEngineV2 );
  // prepareJobs calls mapLayer->createMapRenderer may involve cloning a RasterDataProvider,
  // whose constructor may need to download some data (i.e. WMS, AMS) and doing so runs a
  // QEventLoop waiting for the network request to complete. If unluckily someone calls
  // mapCanvas->refresh() while this is happening, QgsMapRendererCustomPainterJob::cancel is
  // called, deleting the QgsMapRendererCustomPainterJob while this function is running.
  // Hence we need to check whether the job is still active before proceeding
  if ( !isActive() )
    return;

  QgsDebugMsg( "Rendering prepared in (seconds): " + QString( "%1" ).arg( prepareTime.elapsed() / 1000.0 ) );

  if ( mRenderSynchronously )
  {
    // do the rendering right now!
    doRender();
    return;
  }

  // now we are ready to start rendering!
  connect( &mFutureWatcher, SIGNAL( finished() ), SLOT( futureFinished() ) );

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

  QgsDebugMsg( "QPAINTER cancelling" );
  disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( futureFinished() ) );

  mLabelingRenderContext.setRenderingStopped( true );
  for ( LayerRenderJobs::iterator it = mLayerJobs.begin(); it != mLayerJobs.end(); ++it )
  {
    it->context.setRenderingStopped( true );
  }

  QTime t;
  t.start();

  mFutureWatcher.waitForFinished();

  QgsDebugMsg( QString( "QPAINER cancel waited %1 ms" ).arg( t.elapsed() / 1000.0 ) );

  futureFinished();

  QgsDebugMsg( "QPAINTER cancelled" );
}

void QgsMapRendererCustomPainterJob::waitForFinished()
{
  if ( !isActive() )
    return;

  disconnect( &mFutureWatcher, SIGNAL( finished() ), this, SLOT( futureFinished() ) );

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


QgsLabelingResults* QgsMapRendererCustomPainterJob::takeLabelingResults()
{
  if ( mLabelingEngine )
    return mLabelingEngine->takeResults();
  else if ( mLabelingEngineV2 )
    return mLabelingEngineV2->takeResults();
  else
    return nullptr;
}


void QgsMapRendererCustomPainterJob::waitForFinishedWithEventLoop( const QEventLoop::ProcessEventsFlags& flags )
{
  QEventLoop loop;
  connect( &mFutureWatcher, SIGNAL( finished() ), &loop, SLOT( quit() ) );
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

  logRenderingTime( mLayerJobs );

  // final cleanup
  cleanupJobs( mLayerJobs );

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

      job.renderer->render();

      job.renderingTime = layerTime.elapsed();
    }

    if ( job.img )
    {
      // If we flattened this layer for alternate blend modes, composite it now
      mPainter->drawImage( 0, 0, *job.img );
    }

  }

  QgsDebugMsg( "Done rendering map layers" );

  if ( mSettings.testFlag( QgsMapSettings::DrawLabeling ) && !mLabelingRenderContext.renderingStopped() )
    drawLabeling( mSettings, mLabelingRenderContext, mLabelingEngine, mLabelingEngineV2, mPainter );

  QgsDebugMsg( "Rendering completed in (seconds): " + QString( "%1" ).arg( renderTime.elapsed() / 1000.0 ) );
}


void QgsMapRendererJob::drawLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine, QgsLabelingEngineV2* labelingEngine2, QPainter* painter )
{
  QgsDebugMsg( "Draw labeling start" );

  QTime t;
  t.start();

  // Reset the composition mode before rendering the labels
  painter->setCompositionMode( QPainter::CompositionMode_SourceOver );

  // TODO: this is not ideal - we could override rendering stopped flag that has been set in meanwhile
  renderContext = QgsRenderContext::fromMapSettings( settings );
  renderContext.setPainter( painter );
  renderContext.setLabelingEngine( labelingEngine );

#if !defined(QGIS_DISABLE_DEPRECATED)
  // old labeling - to be removed at some point...
  drawOldLabeling( settings, renderContext );
#endif
  drawNewLabeling( settings, renderContext, labelingEngine );

  if ( labelingEngine2 )
  {
    // set correct extent
    renderContext.setExtent( settings.visibleExtent() );
    renderContext.setCoordinateTransform( nullptr );

    labelingEngine2->run( renderContext );
  }

  QgsDebugMsg( QString( "Draw labeling took (seconds): %1" ).arg( t.elapsed() / 1000. ) );
}


void QgsMapRendererJob::drawOldLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext )
{
  // render all labels for vector layers in the stack, starting at the base
  QListIterator<QString> li( settings.layers() );
  li.toBack();
  while ( li.hasPrevious() )
  {
    if ( renderContext.renderingStopped() )
    {
      break;
    }

    QString layerId = li.previous();

    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );

    if ( !ml || ( ml->type() != QgsMapLayer::VectorLayer ) )
      continue;

    // only make labels if the layer is visible
    // after scale dep viewing settings are checked
    if ( !ml->isInScaleRange( settings.scale() ) )
      continue;

    const QgsCoordinateTransform* ct = nullptr;
    QgsRectangle r1 = settings.visibleExtent(), r2;

    if ( settings.hasCrsTransformEnabled() )
    {
      ct = settings.layerTransform( ml );
      if ( ct )
        reprojectToLayerExtent( ml, ct, r1, r2 );
    }

    renderContext.setCoordinateTransform( ct );
    renderContext.setExtent( r1 );

    ml->drawLabels( renderContext );
  }
}


void QgsMapRendererJob::drawNewLabeling( const QgsMapSettings& settings, QgsRenderContext& renderContext, QgsPalLabeling* labelingEngine )
{
  if ( labelingEngine && !renderContext.renderingStopped() )
  {
    // set correct extent
    renderContext.setExtent( settings.visibleExtent() );
    renderContext.setCoordinateTransform( nullptr );

    labelingEngine->drawLabeling( renderContext );
    labelingEngine->exit();
  }
}

void QgsMapRendererJob::updateLayerGeometryCaches()
{
  QMap<QString, QgsGeometryCache>::const_iterator it = mGeometryCaches.constBegin();
  for ( ; it != mGeometryCaches.constEnd(); ++it )
  {
    const QgsGeometryCache& cache = it.value();
    if ( QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( it.key() ) ) )
      * vl->cache() = cache;
  }
  mGeometryCaches.clear();
}


bool QgsMapRendererJob::needTemporaryImage( QgsMapLayer* ml )
{
  if ( ml->type() == QgsMapLayer::VectorLayer )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( vl->rendererV2() && vl->rendererV2()->forceRasterRender() )
    {
      //raster rendering is forced for this layer
      return true;
    }
    if ( mSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) &&
         (( vl->blendMode() != QPainter::CompositionMode_SourceOver )
          || ( vl->featureBlendMode() != QPainter::CompositionMode_SourceOver )
          || ( vl->layerTransparency() != 0 ) ) )
    {
      //layer properties require rasterisation
      return true;
    }
  }

  return false;
}

