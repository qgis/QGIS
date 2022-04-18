/***************************************************************************
  qgsquickmapcanvasmap.cpp
  --------------------------------------
  Date                 : 10.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QScreen>

#include "qgis.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaprenderercache.h"
#include "qgsmaprendererparalleljob.h"
#include "qgsmessagelog.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsannotationlayer.h"
#include "qgsvectorlayer.h"
#include "qgslabelingresults.h"

#include "qgsquickmapcanvasmap.h"
#include "qgsquickmapsettings.h"


QgsQuickMapCanvasMap::QgsQuickMapCanvasMap( QQuickItem *parent )
  : QQuickItem( parent )
  , mMapSettings( std::make_unique<QgsQuickMapSettings>() )
  , mCache( std::make_unique<QgsMapRendererCache>() )
{
  connect( this, &QQuickItem::windowChanged, this, &QgsQuickMapCanvasMap::onWindowChanged );
  connect( &mRefreshTimer, &QTimer::timeout, this, [ = ] { refreshMap(); } );
  connect( &mMapUpdateTimer, &QTimer::timeout, this, &QgsQuickMapCanvasMap::renderJobUpdated );

  connect( mMapSettings.get(), &QgsQuickMapSettings::extentChanged, this, &QgsQuickMapCanvasMap::onExtentChanged );
  connect( mMapSettings.get(), &QgsQuickMapSettings::layersChanged, this, &QgsQuickMapCanvasMap::onLayersChanged );

  connect( this, &QgsQuickMapCanvasMap::renderStarting, this, &QgsQuickMapCanvasMap::isRenderingChanged );
  connect( this, &QgsQuickMapCanvasMap::mapCanvasRefreshed, this, &QgsQuickMapCanvasMap::isRenderingChanged );

  mMapUpdateTimer.setSingleShot( false );
  mMapUpdateTimer.setInterval( 250 );
  mRefreshTimer.setSingleShot( true );
  setTransformOrigin( QQuickItem::TopLeft );
  setFlags( QQuickItem::ItemHasContents );
}

QgsQuickMapCanvasMap::~QgsQuickMapCanvasMap() = default;

QgsQuickMapSettings *QgsQuickMapCanvasMap::mapSettings() const
{
  return mMapSettings.get();
}

void QgsQuickMapCanvasMap::zoom( QPointF center, qreal scale )
{
  QgsRectangle extent = mMapSettings->extent();
  QgsPoint oldCenter( extent.center() );
  QgsPoint mousePos( mMapSettings->screenToCoordinate( center ) );

  QgsPointXY newCenter( mousePos.x() + ( ( oldCenter.x() - mousePos.x() ) * scale ),
                        mousePos.y() + ( ( oldCenter.y() - mousePos.y() ) * scale ) );

  // same as zoomWithCenter (no coordinate transformations are needed)
  extent.scale( scale, &newCenter );
  mMapSettings->setExtent( extent );
}

void QgsQuickMapCanvasMap::pan( QPointF oldPos, QPointF newPos )
{
  QgsPoint start = mMapSettings->screenToCoordinate( oldPos.toPoint() );
  QgsPoint end = mMapSettings->screenToCoordinate( newPos.toPoint() );

  double dx = end.x() - start.x();
  double dy = end.y() - start.y();

  // modify the extent
  QgsRectangle extent = mMapSettings->extent();

  extent.setXMinimum( extent.xMinimum() + dx );
  extent.setXMaximum( extent.xMaximum() + dx );
  extent.setYMaximum( extent.yMaximum() + dy );
  extent.setYMinimum( extent.yMinimum() + dy );

  mMapSettings->setExtent( extent );
}

void QgsQuickMapCanvasMap::refreshMap()
{
  stopRendering(); // if any...

  QgsMapSettings mapSettings = mMapSettings->mapSettings();
  if ( !mapSettings.hasValidSettings() )
    return;

  //build the expression context
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::mapSettingsScope( mapSettings );

  QgsProject *project = mMapSettings->project();
  if ( project )
  {
    expressionContext << QgsExpressionContextUtils::projectScope( project );

    mapSettings.setLabelingEngineSettings( project->labelingEngineSettings() );

    // render main annotation layer above all other layers
    QList<QgsMapLayer *> allLayers = mapSettings.layers();
    allLayers.insert( 0, project->mainAnnotationLayer() );
    mapSettings.setLayers( allLayers );
  }

  mapSettings.setExpressionContext( expressionContext );

  // enables on-the-fly simplification of geometries to spend less time rendering
  mapSettings.setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization );
  // with incremental rendering - enables updates of partially rendered layers (good for WMTS, XYZ layers)
  mapSettings.setFlag( Qgis::MapSettingsFlag::RenderPartialOutput, mIncrementalRendering );

  // create the renderer job
  Q_ASSERT( !mJob );
  mJob = new QgsMapRendererParallelJob( mapSettings );

  if ( mIncrementalRendering )
    mMapUpdateTimer.start();

  connect( mJob, &QgsMapRendererJob::renderingLayersFinished, this, &QgsQuickMapCanvasMap::renderJobUpdated );
  connect( mJob, &QgsMapRendererJob::finished, this, &QgsQuickMapCanvasMap::renderJobFinished );
  mJob->setCache( mCache.get() );

  mJob->start();

  if ( !mSilentRefresh )
  {
    emit renderStarting();
  }
  else
  {
    mSilentRefresh = false;
  }
}

void QgsQuickMapCanvasMap::renderJobUpdated()
{
  if ( !mJob )
    return;

  mImage = mJob->renderedImage();
  mImageMapSettings = mJob->mapSettings();
  mDirty = true;
  // Temporarily freeze the canvas, we only need to reset the geometry but not trigger a repaint
  bool freeze = mFreeze;
  mFreeze = true;
  updateTransform();
  mFreeze = freeze;

  update();
  emit mapCanvasRefreshed();
}

void QgsQuickMapCanvasMap::renderJobFinished()
{
  if ( !mJob )
    return;

  const QgsMapRendererJob::Errors errors = mJob->errors();
  for ( const QgsMapRendererJob::Error &error : errors )
  {
    QgsMessageLog::logMessage( QStringLiteral( "%1 :: %2" ).arg( error.layerID, error.message ), tr( "Rendering" ) );
  }

  // take labeling results before emitting renderComplete, so labeling map tools
  // connected to signal work with correct results
  delete mLabelingResults;
  mLabelingResults = mJob->takeLabelingResults();

  mImage = mJob->renderedImage();
  mImageMapSettings = mJob->mapSettings();

  // now we are in a slot called from mJob - do not delete it immediately
  // so the class is still valid when the execution returns to the class
  mJob->deleteLater();
  mJob = nullptr;
  mDirty = true;
  mMapUpdateTimer.stop();

  // Temporarily freeze the canvas, we only need to reset the geometry but not trigger a repaint
  bool freeze = mFreeze;
  mFreeze = true;
  updateTransform();
  mFreeze = freeze;

  update();
  emit mapCanvasRefreshed();

  if ( mDeferredRefreshPending )
  {
    mDeferredRefreshPending = false;
    mSilentRefresh = true;
    refresh();
  }
}

void QgsQuickMapCanvasMap::layerRepaintRequested( bool deferred )
{
  if ( mMapSettings->outputSize().isNull() )
    return; // the map image size has not been set yet

  if ( !mFreeze )
  {
    if ( deferred )
    {
      if ( !mJob )
      {
        mSilentRefresh = true;
        refresh();
      }
      else
      {
        mDeferredRefreshPending = true;
      }
    }
    else
    {
      refresh();
    }
  }
}

void QgsQuickMapCanvasMap::onWindowChanged( QQuickWindow *window )
{
  if ( mWindow == window )
    return;

  if ( mWindow )
    disconnect( mWindow, &QQuickWindow::screenChanged, this, &QgsQuickMapCanvasMap::onScreenChanged );

  if ( window )
  {
    connect( window, &QQuickWindow::screenChanged, this, &QgsQuickMapCanvasMap::onScreenChanged );
    onScreenChanged( window->screen() );
  }

  mWindow = window;
}

void QgsQuickMapCanvasMap::onScreenChanged( QScreen *screen )
{
  if ( screen )
  {
    if ( screen->devicePixelRatio() > 0 )
    {
      mMapSettings->setDevicePixelRatio( screen->devicePixelRatio() );
    }
    mMapSettings->setOutputDpi( screen->physicalDotsPerInch() );
  }
}

void QgsQuickMapCanvasMap::onExtentChanged()
{
  updateTransform();

  // And trigger a new rendering job
  refresh();
}

void QgsQuickMapCanvasMap::updateTransform()
{
  QgsRectangle imageExtent = mImageMapSettings.visibleExtent();
  QgsRectangle newExtent = mMapSettings->mapSettings().visibleExtent();
  setScale( imageExtent.width() / newExtent.width() );

  QgsPointXY pixelPt = mMapSettings->coordinateToScreen( QgsPoint( imageExtent.xMinimum(), imageExtent.yMaximum() ) );
  setX( pixelPt.x() );
  setY( pixelPt.y() );
}

int QgsQuickMapCanvasMap::mapUpdateInterval() const
{
  return mMapUpdateTimer.interval();
}

void QgsQuickMapCanvasMap::setMapUpdateInterval( int mapUpdateInterval )
{
  if ( mMapUpdateTimer.interval() == mapUpdateInterval )
    return;

  mMapUpdateTimer.setInterval( mapUpdateInterval );

  emit mapUpdateIntervalChanged();
}

bool QgsQuickMapCanvasMap::incrementalRendering() const
{
  return mIncrementalRendering;
}

void QgsQuickMapCanvasMap::setIncrementalRendering( bool incrementalRendering )
{
  if ( incrementalRendering == mIncrementalRendering )
    return;

  mIncrementalRendering = incrementalRendering;
  emit incrementalRenderingChanged();
}

bool QgsQuickMapCanvasMap::freeze() const
{
  return mFreeze;
}

void QgsQuickMapCanvasMap::setFreeze( bool freeze )
{
  if ( freeze == mFreeze )
    return;

  mFreeze = freeze;

  if ( mFreeze )
    stopRendering();
  else
    refresh();

  emit freezeChanged();
}

bool QgsQuickMapCanvasMap::isRendering() const
{
  return mJob;
}

QSGNode *QgsQuickMapCanvasMap::updatePaintNode( QSGNode *oldNode, QQuickItem::UpdatePaintNodeData * )
{
  if ( mDirty )
  {
    delete oldNode;
    oldNode = nullptr;
    mDirty = false;
  }

  QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>( oldNode );
  if ( !node )
  {
    node = new QSGSimpleTextureNode();
    QSGTexture *texture = window()->createTextureFromImage( mImage );
    node->setTexture( texture );
    node->setOwnsTexture( true );
  }

  QRectF rect( boundingRect() );
  QSizeF size = mImage.size();
  if ( !size.isEmpty() )
    size /= mMapSettings->devicePixelRatio();

  // Check for resizes that change the w/h ratio
  if ( !rect.isEmpty() && !size.isEmpty() && !qgsDoubleNear( rect.width() / rect.height(), ( size.width() ) / static_cast<double>( size.height() ), 3 ) )
  {
    if ( qgsDoubleNear( rect.height(), mImage.height() ) )
    {
      rect.setHeight( rect.width() / size.width() * size.height() );
    }
    else
    {
      rect.setWidth( rect.height() / size.height() * size.width() );
    }
  }

  node->setRect( rect );

  return node;
}

void QgsQuickMapCanvasMap::geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry )
{
  QQuickItem::geometryChanged( newGeometry, oldGeometry );
  if ( newGeometry.size() != oldGeometry.size() )
  {
    mMapSettings->setOutputSize( newGeometry.size().toSize() );
    refresh();
  }
}

void QgsQuickMapCanvasMap::onLayersChanged()
{
  if ( mMapSettings->extent().isEmpty() )
    zoomToFullExtent();

  for ( const QMetaObject::Connection &conn : std::as_const( mLayerConnections ) )
  {
    disconnect( conn );
  }
  mLayerConnections.clear();

  const QList<QgsMapLayer *> layers = mMapSettings->layers();
  for ( QgsMapLayer *layer : layers )
  {
    mLayerConnections << connect( layer, &QgsMapLayer::repaintRequested, this, &QgsQuickMapCanvasMap::layerRepaintRequested );
  }

  refresh();
}

void QgsQuickMapCanvasMap::destroyJob( QgsMapRendererJob *job )
{
  job->cancel();
  job->deleteLater();
}

void QgsQuickMapCanvasMap::stopRendering()
{
  if ( mJob )
  {
    disconnect( mJob, &QgsMapRendererJob::renderingLayersFinished, this, &QgsQuickMapCanvasMap::renderJobUpdated );
    disconnect( mJob, &QgsMapRendererJob::finished, this, &QgsQuickMapCanvasMap::renderJobFinished );

    mJob->cancelWithoutBlocking();
    mJob = nullptr;
  }
}

void QgsQuickMapCanvasMap::zoomToFullExtent()
{
  QgsRectangle extent;
  const QList<QgsMapLayer *> layers = mMapSettings->layers();
  for ( QgsMapLayer *layer : layers )
  {
    if ( mMapSettings->destinationCrs() != layer->crs() )
    {
      QgsCoordinateTransform transform( layer->crs(), mMapSettings->destinationCrs(), mMapSettings->transformContext() );
      try
      {
        extent.combineExtentWith( transform.transformBoundingBox( layer->extent() ) );
      }
      catch ( const QgsCsException &exp )
      {
        // Ignore extent if it can't be transformed
      }
    }
    else
    {
      extent.combineExtentWith( layer->extent() );
    }
  }
  mMapSettings->setExtent( extent );

  refresh();
}

void QgsQuickMapCanvasMap::refresh()
{
  if ( mMapSettings->outputSize().isNull() )
    return; // the map image size has not been set yet

  if ( !mFreeze )
    mRefreshTimer.start( 1 );
}

void QgsQuickMapCanvasMap::clearCache()
{
  if ( mCache )
    mCache->clear();
}
