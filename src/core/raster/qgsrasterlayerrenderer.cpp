/***************************************************************************
  qgsrasterlayerrenderer.cpp
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

#include "qgsrasterlayerrenderer.h"

#include "qgsmessagelog.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterprojector.h"
#include "qgsrendercontext.h"
#include "qgsproject.h"
#include "qgsexception.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsmapclippingutils.h"
#include "qgsrasterpipe.h"

#include <QElapsedTimer>
#include <QPointer>

///@cond PRIVATE

QgsRasterLayerRendererFeedback::QgsRasterLayerRendererFeedback( QgsRasterLayerRenderer *r )
  : mR( r )
  , mMinimalPreviewInterval( 250 )
{
  setRenderPartialOutput( r->renderContext()->testFlag( Qgis::RenderContextFlag::RenderPartialOutput ) );
}

void QgsRasterLayerRendererFeedback::onNewData()
{
  if ( !renderPartialOutput() )
    return;  // we were not asked for partial renders and we may not have a temporary image for overwriting...

  // update only once upon a time
  // (preview itself takes some time)
  if ( mLastPreview.isValid() && mLastPreview.msecsTo( QTime::currentTime() ) < mMinimalPreviewInterval )
    return;

  // TODO: update only the area that got new data

  QgsDebugMsgLevel( QStringLiteral( "new raster preview! %1" ).arg( mLastPreview.msecsTo( QTime::currentTime() ) ), 3 );
  QElapsedTimer t;
  t.start();
  QgsRasterBlockFeedback feedback;
  feedback.setPreviewOnly( true );
  feedback.setRenderPartialOutput( true );
  QgsRasterIterator iterator( mR->mPipe->last() );
  QgsRasterDrawer drawer( &iterator, mR->renderContext()->dpiTarget() );
  drawer.draw( mR->renderContext()->painter(), mR->mRasterViewPort, &mR->renderContext()->mapToPixel(), &feedback );
  mR->mReadyToCompose = true;
  QgsDebugMsgLevel( QStringLiteral( "total raster preview time: %1 ms" ).arg( t.elapsed() ), 3 );
  mLastPreview = QTime::currentTime();
}

///@endcond
///
QgsRasterLayerRenderer::QgsRasterLayerRenderer( QgsRasterLayer *layer, QgsRenderContext &rendererContext )
  : QgsMapLayerRenderer( layer->id(), &rendererContext )
  , mProviderCapabilities( static_cast<QgsRasterDataProvider::Capability>( layer->dataProvider()->capabilities() ) )
  , mFeedback( new QgsRasterLayerRendererFeedback( this ) )
{
  mReadyToCompose = false;
  QgsMapToPixel mapToPixel = rendererContext.mapToPixel();
  if ( rendererContext.mapToPixel().mapRotation() )
  {
    // unset rotation for the sake of local computations.
    // Rotation will be handled by QPainter later
    // TODO: provide a method of QgsMapToPixel to fetch map center
    //       in geographical units
    const QgsPointXY center = mapToPixel.toMapCoordinates(
                                static_cast<int>( mapToPixel.mapWidth() / 2.0 ),
                                static_cast<int>( mapToPixel.mapHeight() / 2.0 )
                              );
    mapToPixel.setMapRotation( 0, center.x(), center.y() );
  }

  QgsRectangle myProjectedViewExtent;
  QgsRectangle myProjectedLayerExtent;

  if ( rendererContext.coordinateTransform().isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "coordinateTransform set -> project extents." ), 4 );
    if ( rendererContext.extent().xMinimum() == std::numeric_limits<double>::lowest() &&
         rendererContext.extent().yMinimum() == std::numeric_limits<double>::lowest() &&
         rendererContext.extent().xMaximum() == std::numeric_limits<double>::max() &&
         rendererContext.extent().yMaximum() == std::numeric_limits<double>::max() )
    {
      // We get in this situation if the view CRS is geographical and the
      // extent goes beyond -180,-90,180,90. To avoid reprojection issues to the
      // layer CRS, then this dummy extent is returned by QgsMapRendererJob::reprojectToLayerExtent()
      // Don't try to reproject it now to view extent as this would return
      // a null rectangle.
      myProjectedViewExtent = rendererContext.extent();
    }
    else
    {
      try
      {
        QgsCoordinateTransform ct = rendererContext.coordinateTransform();
        ct.setBallparkTransformsAreAppropriate( true );
        myProjectedViewExtent = ct.transformBoundingBox( rendererContext.extent() );
      }
      catch ( QgsCsException &cs )
      {
        QgsMessageLog::logMessage( QObject::tr( "Could not reproject view extent: %1" ).arg( cs.what() ), QObject::tr( "Raster" ) );
        myProjectedViewExtent.setMinimal();
      }
    }

    try
    {
      QgsCoordinateTransform ct = rendererContext.coordinateTransform();
      ct.setBallparkTransformsAreAppropriate( true );
      myProjectedLayerExtent = ct.transformBoundingBox( layer->extent() );
    }
    catch ( QgsCsException &cs )
    {
      QgsMessageLog::logMessage( QObject::tr( "Could not reproject layer extent: %1" ).arg( cs.what() ), QObject::tr( "Raster" ) );
      myProjectedLayerExtent.setMinimal();
    }
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "coordinateTransform not set" ), 4 );
    myProjectedViewExtent = rendererContext.extent();
    myProjectedLayerExtent = layer->extent();
  }

  // clip raster extent to view extent
  QgsRectangle myRasterExtent = layer->ignoreExtents() ? myProjectedViewExtent : myProjectedViewExtent.intersect( myProjectedLayerExtent );
  if ( myRasterExtent.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "draw request outside view extent." ), 2 );
    // nothing to do
    return;
  }

  QgsDebugMsgLevel( "theViewExtent is " + rendererContext.extent().toString(), 4 );
  QgsDebugMsgLevel( "myProjectedViewExtent is " + myProjectedViewExtent.toString(), 4 );
  QgsDebugMsgLevel( "myProjectedLayerExtent is " + myProjectedLayerExtent.toString(), 4 );
  QgsDebugMsgLevel( "myRasterExtent is " + myRasterExtent.toString(), 4 );

  //
  // The first thing we do is set up the QgsRasterViewPort. This struct stores all the settings
  // relating to the size (in pixels and coordinate system units) of the raster part that is
  // in view in the map window. It also stores the origin.
  //
  //this is not a class level member because every time the user pans or zooms
  //the contents of the rasterViewPort will change
  mRasterViewPort = new QgsRasterViewPort();

  mRasterViewPort->mDrawnExtent = myRasterExtent;
  if ( rendererContext.coordinateTransform().isValid() )
  {
    mRasterViewPort->mSrcCRS = layer->crs();
    mRasterViewPort->mDestCRS = rendererContext.coordinateTransform().destinationCrs();
    mRasterViewPort->mTransformContext = rendererContext.transformContext();
  }
  else
  {
    mRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
    mRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid
  }

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  mRasterViewPort->mTopLeftPoint = mapToPixel.transform( myRasterExtent.xMinimum(), myRasterExtent.yMaximum() );
  mRasterViewPort->mBottomRightPoint = mapToPixel.transform( myRasterExtent.xMaximum(), myRasterExtent.yMinimum() );

  // align to output device grid, i.e. std::floor/ceil to integers
  // TODO: this should only be done if paint device is raster - screen, image
  // for other devices (pdf) it can have floating point origin
  // we could use floating point for raster devices as well, but respecting the
  // output device grid should make it more effective as the resampling is done in
  // the provider anyway
  mRasterViewPort->mTopLeftPoint.setX( std::floor( mRasterViewPort->mTopLeftPoint.x() ) );
  mRasterViewPort->mTopLeftPoint.setY( std::floor( mRasterViewPort->mTopLeftPoint.y() ) );
  mRasterViewPort->mBottomRightPoint.setX( std::ceil( mRasterViewPort->mBottomRightPoint.x() ) );
  mRasterViewPort->mBottomRightPoint.setY( std::ceil( mRasterViewPort->mBottomRightPoint.y() ) );
  // recalc myRasterExtent to aligned values
  myRasterExtent.set(
    mapToPixel.toMapCoordinates( mRasterViewPort->mTopLeftPoint.x(),
                                 mRasterViewPort->mBottomRightPoint.y() ),
    mapToPixel.toMapCoordinates( mRasterViewPort->mBottomRightPoint.x(),
                                 mRasterViewPort->mTopLeftPoint.y() )
  );

  //raster viewport top left / bottom right are already rounded to int
  mRasterViewPort->mWidth = static_cast<qgssize>( std::abs( mRasterViewPort->mBottomRightPoint.x() - mRasterViewPort->mTopLeftPoint.x() ) );
  mRasterViewPort->mHeight = static_cast<qgssize>( std::abs( mRasterViewPort->mBottomRightPoint.y() - mRasterViewPort->mTopLeftPoint.y() ) );

  // painter could be null (in parallel rendering for instance) so we fallback on scaleFactor which
  // should be equal to outputDpi and so logicalDpiX (except for QPicture)
  const double dpi = rendererContext.painter() ? rendererContext.painter()->device()->logicalDpiX() :
                     25.4 * rendererContext.scaleFactor();
  if ( mProviderCapabilities & QgsRasterDataProvider::DpiDependentData
       && rendererContext.dpiTarget() >= 0.0 )
  {
    const double dpiScaleFactor = rendererContext.dpiTarget() / dpi;
    mRasterViewPort->mWidth *= dpiScaleFactor;
    mRasterViewPort->mHeight *= dpiScaleFactor;
  }
  else
  {
    rendererContext.setDpiTarget( -1.0 );
  }

  //the drawable area can start to get very very large when you get down displaying 2x2 or smaller, this is because
  //mapToPixel.mapUnitsPerPixel() is less then 1,
  //so we will just get the pixel data and then render these special cases differently in paintImageToCanvas()

  QgsDebugMsgLevel( QStringLiteral( "mapUnitsPerPixel = %1" ).arg( mapToPixel.mapUnitsPerPixel() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mWidth = %1" ).arg( layer->width() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mHeight = %1" ).arg( layer->height() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "myRasterExtent.xMinimum() = %1" ).arg( myRasterExtent.xMinimum() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "myRasterExtent.xMaximum() = %1" ).arg( myRasterExtent.xMaximum() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "myRasterExtent.yMinimum() = %1" ).arg( myRasterExtent.yMinimum() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "myRasterExtent.yMaximum() = %1" ).arg( myRasterExtent.yMaximum() ), 3 );

  QgsDebugMsgLevel( QStringLiteral( "mTopLeftPoint.x() = %1" ).arg( mRasterViewPort->mTopLeftPoint.x() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mBottomRightPoint.x() = %1" ).arg( mRasterViewPort->mBottomRightPoint.x() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mTopLeftPoint.y() = %1" ).arg( mRasterViewPort->mTopLeftPoint.y() ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mBottomRightPoint.y() = %1" ).arg( mRasterViewPort->mBottomRightPoint.y() ), 3 );

  QgsDebugMsgLevel( QStringLiteral( "mWidth = %1" ).arg( mRasterViewPort->mWidth ), 3 );
  QgsDebugMsgLevel( QStringLiteral( "mHeight = %1" ).arg( mRasterViewPort->mHeight ), 3 );

  // /\/\/\ - added to handle zoomed-in rasters

  // TODO R->mLastViewPort = *mRasterViewPort;

  // TODO: is it necessary? Probably WMS only?
  layer->dataProvider()->setDpi( dpi );

  // copy the whole raster pipe!
  mPipe = new QgsRasterPipe( *layer->pipe() );
  QObject::connect( mPipe->provider(), &QgsRasterDataProvider::statusChanged, layer, &QgsRasterLayer::statusChanged );
  QgsRasterRenderer *rasterRenderer = mPipe->renderer();
  if ( rasterRenderer
       && !( rendererContext.flags() & Qgis::RenderContextFlag::RenderPreviewJob )
       && !( rendererContext.flags() & Qgis::RenderContextFlag::Render3DMap ) )
  {
    layer->refreshRendererIfNeeded( rasterRenderer, rendererContext.extent() );
  }

  mPipe->evaluateDataDefinedProperties( rendererContext.expressionContext() );

  const QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< const QgsRasterLayerTemporalProperties * >( layer->temporalProperties() );
  if ( temporalProperties->isActive() && renderContext()->isTemporal() )
  {
    switch ( temporalProperties->mode() )
    {
      case Qgis::RasterTemporalMode::FixedTemporalRange:
      case Qgis::RasterTemporalMode::RedrawLayerOnly:
        break;

      case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
        // in this mode we need to pass on the desired render temporal range to the data provider
        if ( mPipe->provider()->temporalCapabilities() )
        {
          mPipe->provider()->temporalCapabilities()->setRequestedTemporalRange( rendererContext.temporalRange() );
          mPipe->provider()->temporalCapabilities()->setIntervalHandlingMethod( temporalProperties->intervalHandlingMethod() );
        }
        break;
    }
  }
  else if ( mPipe->provider()->temporalCapabilities() )
  {
    mPipe->provider()->temporalCapabilities()->setRequestedTemporalRange( QgsDateTimeRange() );
    mPipe->provider()->temporalCapabilities()->setIntervalHandlingMethod( temporalProperties->intervalHandlingMethod() );
  }

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  mFeedback->setRenderContext( rendererContext );
}

QgsRasterLayerRenderer::~QgsRasterLayerRenderer()
{
  delete mFeedback;

  delete mRasterViewPort;
  delete mPipe;
}

bool QgsRasterLayerRenderer::render()
{
  // Skip rendering of out of view tiles (xyz)
  if ( !mRasterViewPort || ( renderContext()->testFlag( Qgis::RenderContextFlag::RenderPreviewJob ) &&
                             !( mProviderCapabilities &
                                QgsRasterInterface::Capability::Prefetch ) ) )
    return true;

  QElapsedTimer time;
  time.start();
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

  const QgsScopedQPainterState painterSate( renderContext()->painter() );
  if ( !mClippingRegions.empty() )
  {
    bool needsPainterClipPath = false;
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), QgsMapLayerType::RasterLayer, needsPainterClipPath );
    if ( needsPainterClipPath )
      renderContext()->painter()->setClipPath( path, Qt::IntersectClip );
  }

  QgsRasterProjector *projector = mPipe->projector();
  bool restoreOldResamplingStage = false;
  const Qgis::RasterResamplingStage oldResamplingState = mPipe->resamplingStage();

  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector
  if ( projector )
  {
    // Force provider resampling if reprojection is needed
    if ( ( mPipe->provider()->providerCapabilities() & QgsRasterDataProvider::ProviderHintCanPerformProviderResampling ) &&
         mRasterViewPort->mSrcCRS != mRasterViewPort->mDestCRS &&
         oldResamplingState != Qgis::RasterResamplingStage::Provider )
    {
      restoreOldResamplingStage = true;
      mPipe->setResamplingStage( Qgis::RasterResamplingStage::Provider );
    }
    projector->setCrs( mRasterViewPort->mSrcCRS, mRasterViewPort->mDestCRS, mRasterViewPort->mTransformContext );
  }

  // important -- disable SmoothPixmapTransform for raster layer renders. We want individual pixels to be clearly defined!
  renderContext()->painter()->setRenderHint( QPainter::SmoothPixmapTransform, false );

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe->last() );
  QgsRasterDrawer drawer( &iterator, renderContext()->dpiTarget() );
  drawer.draw( renderContext()->painter(), mRasterViewPort, &renderContext()->mapToPixel(), mFeedback );

  if ( restoreOldResamplingStage )
  {
    mPipe->setResamplingStage( oldResamplingState );
  }

  const QStringList errors = mFeedback->errors();
  for ( const QString &error : errors )
  {
    mErrors.append( error );
  }

  QgsDebugMsgLevel( QStringLiteral( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ), 4 );
  mReadyToCompose = true;

  return !mFeedback->isCanceled();
}

QgsFeedback *QgsRasterLayerRenderer::feedback() const
{
  return mFeedback;
}

bool QgsRasterLayerRenderer::forceRasterRender() const
{
  // preview of intermediate raster rendering results requires a temporary output image
  return renderContext()->testFlag( Qgis::RenderContextFlag::RenderPartialOutput );
}
