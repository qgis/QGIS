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
#include "qgsrasterrenderer.h"
#include "qgsexception.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsmapclippingutils.h"
#include "qgsrasterpipe.h"
#include "qgselevationmap.h"
#include "qgsgdalutils.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgsrastertransparency.h"
#include "qgsrasterlayerutils.h"

#include <QElapsedTimer>
#include <QPointer>
#include <QThread>

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
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( *( mR->renderContext() ), mR->mRasterViewPort, &feedback );
  mR->mReadyToCompose = true;
  QgsDebugMsgLevel( QStringLiteral( "total raster preview time: %1 ms" ).arg( t.elapsed() ), 3 );
  mLastPreview = QTime::currentTime();
}

///@endcond
///
QgsRasterLayerRenderer::QgsRasterLayerRenderer( QgsRasterLayer *layer, QgsRenderContext &rendererContext )
  : QgsMapLayerRenderer( layer->id(), &rendererContext )
  , mLayerName( layer->name() )
  , mLayerOpacity( layer->opacity() )
  , mProviderCapabilities( layer->dataProvider()->providerCapabilities() )
  , mFeedback( new QgsRasterLayerRendererFeedback( this ) )
  , mEnableProfile( rendererContext.flags() & Qgis::RenderContextFlag::RecordProfile )
{
  mReadyToCompose = false;

  QElapsedTimer timer;
  timer.start();

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
        myProjectedViewExtent.setNull();
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
      myProjectedLayerExtent.setNull();
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

  const double dpi = 25.4 * rendererContext.scaleFactor();
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
  layer->dataProvider()->setDpi( std::floor( dpi * rendererContext.devicePixelRatio() ) );

  // copy the whole raster pipe!
  mPipe.reset( new QgsRasterPipe( *layer->pipe() ) );

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
  const QgsRasterLayerElevationProperties *elevationProperties = qobject_cast<QgsRasterLayerElevationProperties *>( layer->elevationProperties() );

  if ( ( temporalProperties->isActive() && renderContext()->isTemporal() )
       || ( elevationProperties->hasElevation() && !renderContext()->zRange().isInfinite() ) )
  {
    // temporal and/or elevation band filtering may be applicable
    bool matched = false;
    const int matchedBand = QgsRasterLayerUtils::renderedBandForElevationAndTemporalRange(
                              layer,
                              rendererContext.temporalRange(),
                              rendererContext.zRange(),
                              matched
                            );
    if ( matched && matchedBand > 0 )
    {
      mPipe->renderer()->setInputBand( matchedBand );
    }
  }

  if ( temporalProperties->isActive() && renderContext()->isTemporal() )
  {
    switch ( temporalProperties->mode() )
    {
      case Qgis::RasterTemporalMode::FixedTemporalRange:
      case Qgis::RasterTemporalMode::RedrawLayerOnly:
      case Qgis::RasterTemporalMode::FixedRangePerBand:
        break;

      case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
        // in this mode we need to pass on the desired render temporal range to the data provider
        if ( QgsRasterDataProviderTemporalCapabilities *temporalCapabilities = mPipe->provider()->temporalCapabilities() )
        {
          temporalCapabilities->setRequestedTemporalRange( rendererContext.temporalRange() );
          temporalCapabilities->setIntervalHandlingMethod( temporalProperties->intervalHandlingMethod() );
        }
        break;
    }
  }
  else if ( QgsRasterDataProviderTemporalCapabilities *temporalCapabilities = mPipe->provider()->temporalCapabilities() )
  {
    temporalCapabilities->setRequestedTemporalRange( QgsDateTimeRange() );
    temporalCapabilities->setIntervalHandlingMethod( temporalProperties->intervalHandlingMethod() );
  }

  mClippingRegions = QgsMapClippingUtils::collectClippingRegionsForLayer( *renderContext(), layer );

  if ( elevationProperties && elevationProperties->hasElevation() )
  {
    mDrawElevationMap = true;
    mElevationScale = elevationProperties->zScale();
    mElevationOffset = elevationProperties->zOffset();
    mElevationBand = elevationProperties->bandNumber();

    if ( !rendererContext.zRange().isInfinite() )
    {
      switch ( elevationProperties->mode() )
      {
        case Qgis::RasterElevationMode::FixedElevationRange:
          // don't need to handle anything here -- the layer renderer will never be created if the
          // render context range doesn't match the layer's fixed elevation range
          break;

        case Qgis::RasterElevationMode::FixedRangePerBand:
        case Qgis::RasterElevationMode::DynamicRangePerBand:
          // temporal/elevation band based filtering was already handled earlier in this method
          break;

        case Qgis::RasterElevationMode::RepresentsElevationSurface:
        {
          if ( mPipe->renderer()->usesBands().contains( mElevationBand ) )
          {
            // if layer has elevation settings and we are only rendering a slice of z values => we need to filter pixels by elevation

            std::unique_ptr< QgsRasterTransparency > transparency;
            if ( const QgsRasterTransparency *rendererTransparency = mPipe->renderer()->rasterTransparency() )
              transparency = std::make_unique< QgsRasterTransparency >( *rendererTransparency );
            else
              transparency = std::make_unique< QgsRasterTransparency >();

            QVector<QgsRasterTransparency::TransparentSingleValuePixel> transparentPixels = transparency->transparentSingleValuePixelList();

            // account for z offset/zscale by reversing these calculations, so that we get the z range in
            // raw pixel values
            const double adjustedLower = ( rendererContext.zRange().lower() - mElevationOffset ) / mElevationScale;
            const double adjustedUpper = ( rendererContext.zRange().upper() - mElevationOffset ) / mElevationScale;
            transparentPixels.append( QgsRasterTransparency::TransparentSingleValuePixel( std::numeric_limits<double>::lowest(), adjustedLower, 0, true, !rendererContext.zRange().includeLower() ) );
            transparentPixels.append( QgsRasterTransparency::TransparentSingleValuePixel( adjustedUpper, std::numeric_limits<double>::max(), 0, !rendererContext.zRange().includeUpper(), true ) );

            transparency->setTransparentSingleValuePixelList( transparentPixels );
            mPipe->renderer()->setRasterTransparency( transparency.release() );
          }
          break;
        }
      }
    }
  }

  mFeedback->setRenderContext( rendererContext );

  mPipe->moveToThread( nullptr );

  mPreparationTime = timer.elapsed();
}

QgsRasterLayerRenderer::~QgsRasterLayerRenderer()
{
  delete mFeedback;

  delete mRasterViewPort;
}

bool QgsRasterLayerRenderer::render()
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( mEnableProfile )
  {
    profile = std::make_unique< QgsScopedRuntimeProfile >( mLayerName, QStringLiteral( "rendering" ), layerId() );
    if ( mPreparationTime > 0 )
      QgsApplication::profiler()->record( QObject::tr( "Create renderer" ), mPreparationTime / 1000.0, QStringLiteral( "rendering" ) );
  }

  // Skip rendering of out of view tiles (xyz)
  if ( !mRasterViewPort || ( renderContext()->testFlag( Qgis::RenderContextFlag::RenderPreviewJob ) &&
                             !( mProviderCapabilities &
                                QgsRasterInterface::Capability::Prefetch ) ) )
    return true;

  mPipe->moveToThread( QThread::currentThread() );

  QElapsedTimer time;
  time.start();

  std::unique_ptr< QgsScopedRuntimeProfile > preparingProfile;
  if ( mEnableProfile )
  {
    preparingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Preparing render" ), QStringLiteral( "rendering" ) );
  }

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
    const QPainterPath path = QgsMapClippingUtils::calculatePainterClipRegion( mClippingRegions, *renderContext(), Qgis::LayerType::Raster, needsPainterClipPath );
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

  preparingProfile.reset();
  std::unique_ptr< QgsScopedRuntimeProfile > renderingProfile;
  if ( mEnableProfile )
  {
    renderingProfile = std::make_unique< QgsScopedRuntimeProfile >( QObject::tr( "Rendering" ), QStringLiteral( "rendering" ) );
  }

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe->last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( *( renderContext() ), mRasterViewPort, mFeedback );

  if ( mDrawElevationMap )
    drawElevationMap();

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

  mPipe->moveToThread( nullptr );

  return !mFeedback->isCanceled();
}

QgsFeedback *QgsRasterLayerRenderer::feedback() const
{
  return mFeedback;
}

bool QgsRasterLayerRenderer::forceRasterRender() const
{
  if ( !mRasterViewPort || !mPipe )
    return false;  // this layer is not going to get rendered

  // preview of intermediate raster rendering results requires a temporary output image
  if ( renderContext()->testFlag( Qgis::RenderContextFlag::RenderPartialOutput ) )
    return true;

  if ( QgsRasterRenderer *renderer = mPipe->renderer() )
  {
    if ( !( renderer->flags() & Qgis::RasterRendererFlag::InternalLayerOpacityHandling )
         && renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) && ( !qgsDoubleNear( mLayerOpacity, 1.0 ) ) )
      return true;
  }

  return false;
}

void QgsRasterLayerRenderer::drawElevationMap()
{
  QgsRasterDataProvider *dataProvider = mPipe->provider();
  if ( renderContext()->elevationMap() && dataProvider )
  {
    double dpiScalefactor;

    if ( renderContext()->dpiTarget() >= 0.0 )
      dpiScalefactor = renderContext()->dpiTarget() / ( renderContext()->scaleFactor() * 25.4 );
    else
      dpiScalefactor = 1.0;

    int outputWidth = static_cast<int>( static_cast<double>( mRasterViewPort->mWidth )  / dpiScalefactor * renderContext()->devicePixelRatio() );
    int outputHeight =  static_cast<int>( static_cast<double>( mRasterViewPort->mHeight ) / dpiScalefactor * renderContext()->devicePixelRatio() );

    QSize viewSize = renderContext()->deviceOutputSize();
    int viewWidth =  static_cast<int>( viewSize.width() / dpiScalefactor );
    int viewHeight =  static_cast<int>( viewSize.height() / dpiScalefactor );

    bool canRenderElevation = false;
    std::unique_ptr<QgsRasterBlock> elevationBlock;
    if ( mRasterViewPort->mSrcCRS == mRasterViewPort->mDestCRS )
    {
      elevationBlock.reset(
        dataProvider->block(
          mElevationBand,
          mRasterViewPort->mDrawnExtent,
          outputWidth,
          outputHeight,
          mFeedback ) );
      canRenderElevation = true;
    }
    else
    {
      // Destinaton CRS is different from the source CRS.
      // Using the raster projector lead to have big artifacts when rendering the elevation map.
      // To get a smoother elevation map, we use GDAL resampling with coordinates transform
      QgsRectangle viewExtentInLayerCoordinate = renderContext()->extent();

      // If view extent is infinite, we use the data provider extent
      if ( viewExtentInLayerCoordinate.xMinimum() == std::numeric_limits<double>::lowest() &&
           viewExtentInLayerCoordinate.yMinimum() == std::numeric_limits<double>::lowest() &&
           viewExtentInLayerCoordinate.xMaximum() == std::numeric_limits<double>::max() &&
           viewExtentInLayerCoordinate.yMaximum() == std::numeric_limits<double>::max() )
      {
        viewExtentInLayerCoordinate = dataProvider->extent();
      }

      double xLayerResol = viewExtentInLayerCoordinate.width() / static_cast<double>( viewWidth );
      double yLayerResol = viewExtentInLayerCoordinate.height() / static_cast<double>( viewHeight );

      double overSampling = 1;
      if ( mPipe->resampleFilter() )
        overSampling = mPipe->resampleFilter()->maxOversampling();

      if ( dataProvider->capabilities() & QgsRasterDataProvider::Size )
      {
        // If the dataprovider has size capability, we calculate the requested resolution to provider
        double providerXResol = dataProvider->extent().width() / dataProvider->xSize();
        double providerYResol = dataProvider->extent().height() / dataProvider->ySize();
        overSampling = ( xLayerResol / providerXResol + yLayerResol / providerYResol ) / 2;
      }

      GDALResampleAlg alg;
      if ( overSampling > 1 )
        alg = QgsGdalUtils::gdalResamplingAlgorithm( dataProvider->zoomedOutResamplingMethod() );
      else
        alg = QgsGdalUtils::gdalResamplingAlgorithm( dataProvider->zoomedInResamplingMethod() );

      Qgis::DataType dataType = dataProvider->dataType( mElevationBand );

      if ( dataType != Qgis::DataType::UnknownDataType ) // resampling data by GDAL is not compatible with unknown data type
      {
        // we need extra pixels on border to avoid effect border with resampling (at least 2 pixels band for cubic alg)
        int sourceWidth = viewWidth + 4;
        int sourceHeight = viewHeight + 4;
        viewExtentInLayerCoordinate = QgsRectangle(
                                        viewExtentInLayerCoordinate.xMinimum() - xLayerResol * 2,
                                        viewExtentInLayerCoordinate.yMinimum() - yLayerResol * 2,
                                        viewExtentInLayerCoordinate.xMaximum() + xLayerResol * 2,
                                        viewExtentInLayerCoordinate.yMaximum() + yLayerResol * 2 );

        // Now we can do the resampling
        std::unique_ptr<QgsRasterBlock> sourcedata( dataProvider->block( mElevationBand, viewExtentInLayerCoordinate, sourceWidth, sourceHeight, mFeedback ) );
        gdal::dataset_unique_ptr gdalDsInput =
          QgsGdalUtils::blockToSingleBandMemoryDataset( viewExtentInLayerCoordinate, sourcedata.get() );


        elevationBlock.reset( new QgsRasterBlock( dataType,
                              outputWidth,
                              outputHeight ) );

        elevationBlock->setNoDataValue( dataProvider->sourceNoDataValue( mElevationBand ) );

        gdal::dataset_unique_ptr gdalDsOutput =
          QgsGdalUtils::blockToSingleBandMemoryDataset( mRasterViewPort->mDrawnExtent, elevationBlock.get() );

        // For coordinate transformation, we try to obtain a coordinate operation string from the transform context.
        // Depending of the CRS, if we can't we use GDAL transformation directly from the source and destination CRS
        QString coordinateOperation;
        const QgsCoordinateTransformContext &transformContext = renderContext()->transformContext();
        if ( transformContext.mustReverseCoordinateOperation( mRasterViewPort->mSrcCRS, mRasterViewPort->mDestCRS ) )
          coordinateOperation = transformContext.calculateCoordinateOperation( mRasterViewPort->mDestCRS, mRasterViewPort->mSrcCRS );
        else
          coordinateOperation = transformContext.calculateCoordinateOperation( mRasterViewPort->mSrcCRS, mRasterViewPort->mDestCRS );

        if ( coordinateOperation.isEmpty() )
          canRenderElevation = QgsGdalUtils::resampleSingleBandRaster( gdalDsInput.get(), gdalDsOutput.get(), alg,
                               mRasterViewPort->mSrcCRS, mRasterViewPort->mDestCRS );
        else
          canRenderElevation = QgsGdalUtils::resampleSingleBandRaster( gdalDsInput.get(), gdalDsOutput.get(), alg,
                               coordinateOperation.toUtf8().constData() );
      }
    }

    if ( canRenderElevation )
    {
      QPoint topLeft;
      if ( renderContext()->mapToPixel().mapRotation() != 0 )
      {
        // Now rendering elevation on the elevation map, we need to take care of rotation:
        // again a resampling but this time with a geotransform.
        const QgsMapToPixel &mtp = renderContext()->mapToPixel();
        QgsElevationMap *elevMap = renderContext()->elevationMap();

        int elevMapWidth = elevMap->rawElevationImage().width();
        int elevMapHeight = elevMap->rawElevationImage().height();

        int bottom = 0;
        int top = elevMapHeight;
        int left = elevMapWidth;
        int right = 0;

        QList<QgsPointXY> corners;
        corners << QgsPointXY( mRasterViewPort->mDrawnExtent.xMinimum(), mRasterViewPort->mDrawnExtent.yMinimum() )
                << QgsPointXY( mRasterViewPort->mDrawnExtent.xMaximum(), mRasterViewPort->mDrawnExtent.yMaximum() )
                << QgsPointXY( mRasterViewPort->mDrawnExtent.xMinimum(), mRasterViewPort->mDrawnExtent.yMaximum() )
                << QgsPointXY( mRasterViewPort->mDrawnExtent.xMaximum(), mRasterViewPort->mDrawnExtent.yMinimum() );

        for ( const QgsPointXY &corner : std::as_const( corners ) )
        {
          const QgsPointXY dpt = mtp.transform( corner );
          int x = static_cast<int>( std::round( dpt.x() ) );
          int y = static_cast<int>( std::round( dpt.y() ) );

          if ( x < left )
            left = x;
          if ( x > right )
            right = x;
          if ( y < top )
            top = y;
          if ( y > bottom )
            bottom = y;
        }

        const QgsPointXY origin = mtp.toMapCoordinates( left, top );
        double gridXSize = mtp.toMapCoordinates( right, top ).distance( origin );
        double gridYSize = mtp.toMapCoordinates( left, bottom ).distance( origin );
        double angleRad = renderContext()->mapToPixel().mapRotation() / 180 * M_PI;

        gdal::dataset_unique_ptr gdalDsInput =
          QgsGdalUtils::blockToSingleBandMemoryDataset( mRasterViewPort->mDrawnExtent, elevationBlock.get() );

        std::unique_ptr<QgsRasterBlock> rotatedElevationBlock =
          std::make_unique<QgsRasterBlock>( elevationBlock->dataType(),
                                            ( right - left ) * renderContext()->devicePixelRatio() + 1,
                                            ( bottom - top ) * renderContext()->devicePixelRatio() + 1 );

        rotatedElevationBlock->setNoDataValue( elevationBlock->noDataValue() );

        gdal::dataset_unique_ptr gdalDsOutput =
          QgsGdalUtils::blockToSingleBandMemoryDataset( angleRad, origin, gridXSize, gridYSize, rotatedElevationBlock.get() );

        if ( QgsGdalUtils::resampleSingleBandRaster(
               gdalDsInput.get(),
               gdalDsOutput.get(),
               QgsGdalUtils::gdalResamplingAlgorithm( dataProvider->zoomedInResamplingMethod() ), nullptr ) )
        {
          elevationBlock.reset( rotatedElevationBlock.release() );
        }

        topLeft = QPoint( left, top );
      }
      else
      {
        topLeft = mRasterViewPort->mTopLeftPoint.toQPointF().toPoint();
      }

      renderContext()->elevationMap()->fillWithRasterBlock(
        elevationBlock.get(),
        topLeft.y() * renderContext()->devicePixelRatio(),
        topLeft.x() * renderContext()->devicePixelRatio(),
        mElevationScale,
        mElevationOffset );
    }
  }
}
