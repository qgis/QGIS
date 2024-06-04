/***************************************************************************
                              qgsrendercontext.cpp
                              --------------------
  begin                : March 16, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsrendercontext.h"

#include "qgsmapsettings.h"
#include "qgsfeaturefilterprovider.h"
#include "qgslogger.h"
#include "qgselevationmap.h"
#include "qgsunittypes.h"
#include "qgssymbollayer.h"

#define POINTS_TO_MM 2.83464567
#define INCH_TO_MM 25.4

QgsRenderContext::QgsRenderContext()
  : mFlags( Qgis::RenderContextFlag::DrawEditingInfo | Qgis::RenderContextFlag::UseAdvancedEffects | Qgis::RenderContextFlag::DrawSelection | Qgis::RenderContextFlag::UseRenderingOptimization )
{
  mVectorSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  // For RenderMetersInMapUnits support, when rendering in Degrees, the Ellipsoid must be set
  // - for Previews/Icons the default Extent can be used
  mDistanceArea.setEllipsoid( mDistanceArea.sourceCrs().ellipsoidAcronym() );
}

QgsRenderContext::~QgsRenderContext() = default;

QgsRenderContext::QgsRenderContext( const QgsRenderContext &rh )
  : QgsTemporalRangeObject( rh )
  , mFlags( rh.mFlags )
  , mPainter( rh.mPainter )
  , mPreviewRenderPainter( rh.mPreviewRenderPainter )
  , mMaskPainter( rh.mMaskPainter )
  , mCoordTransform( rh.mCoordTransform )
  , mDistanceArea( rh.mDistanceArea )
  , mExtent( rh.mExtent )
  , mOriginalMapExtent( rh.mOriginalMapExtent )
  , mMapToPixel( rh.mMapToPixel )
  , mRenderingStopped( rh.mRenderingStopped )
  , mFeedback( rh.mFeedback )
  , mScaleFactor( rh.mScaleFactor )
  , mDpiTarget( rh.mDpiTarget )
  , mRendererScale( rh.mRendererScale )
  , mSymbologyReferenceScale( rh.mSymbologyReferenceScale )
  , mLabelingEngine( rh.mLabelingEngine )
  , mLabelSink( rh.mLabelSink )
  , mSelectionColor( rh.mSelectionColor )
  , mVectorSimplifyMethod( rh.mVectorSimplifyMethod )
  , mExpressionContext( rh.mExpressionContext )
  , mGeometry( rh.mGeometry )
  , mFeatureFilterProvider( rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr )
  , mSegmentationTolerance( rh.mSegmentationTolerance )
  , mSegmentationToleranceType( rh.mSegmentationToleranceType )
  , mTransformContext( rh.mTransformContext )
  , mPathResolver( rh.mPathResolver )
  , mTextRenderFormat( rh.mTextRenderFormat )
  , mRenderedFeatureHandlers( rh.mRenderedFeatureHandlers )
  , mHasRenderedFeatureHandlers( rh.mHasRenderedFeatureHandlers )
  , mCustomRenderingFlags( rh.mCustomRenderingFlags )
  , mDisabledSymbolLayers()
  , mClippingRegions( rh.mClippingRegions )
  , mFeatureClipGeometry( rh.mFeatureClipGeometry )
  , mTextureOrigin( rh.mTextureOrigin )
  , mZRange( rh.mZRange )
  , mSize( rh.mSize )
  , mDevicePixelRatio( rh.mDevicePixelRatio )
  , mImageFormat( rh.mImageFormat )
  , mRendererUsage( rh.mRendererUsage )
  , mFrameRate( rh.mFrameRate )
  , mCurrentFrame( rh.mCurrentFrame )
  , mSymbolLayerClipPaths( rh.mSymbolLayerClipPaths )
  , mSymbolLayerClippingGeometries( rh.mSymbolLayerClippingGeometries )
#ifdef QGISDEBUG
  , mHasTransformContext( rh.mHasTransformContext )
#endif
  , mElevationMap( rh.mElevationMap )
{
}

QgsRenderContext &QgsRenderContext::operator=( const QgsRenderContext &rh )
{
  mFlags = rh.mFlags;
  mPainter = rh.mPainter;
  mPreviewRenderPainter = rh.mPreviewRenderPainter;
  mMaskPainter = rh.mMaskPainter;
  mCoordTransform = rh.mCoordTransform;
  mExtent = rh.mExtent;
  mOriginalMapExtent = rh.mOriginalMapExtent;
  mMapToPixel = rh.mMapToPixel;
  mRenderingStopped = rh.mRenderingStopped;
  mFeedback = rh.mFeedback;
  mScaleFactor = rh.mScaleFactor;
  mDpiTarget = rh.mDpiTarget;
  mRendererScale = rh.mRendererScale;
  mSymbologyReferenceScale = rh.mSymbologyReferenceScale;
  mLabelingEngine = rh.mLabelingEngine;
  mLabelSink = rh.mLabelSink;
  mSelectionColor = rh.mSelectionColor;
  mVectorSimplifyMethod = rh.mVectorSimplifyMethod;
  mExpressionContext = rh.mExpressionContext;
  mGeometry = rh.mGeometry;
  mFeatureFilterProvider.reset( rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr );
  mSegmentationTolerance = rh.mSegmentationTolerance;
  mSegmentationToleranceType = rh.mSegmentationToleranceType;
  mDistanceArea = rh.mDistanceArea;
  mTransformContext = rh.mTransformContext;
  mPathResolver = rh.mPathResolver;
  mTextRenderFormat = rh.mTextRenderFormat;
  mRenderedFeatureHandlers = rh.mRenderedFeatureHandlers;
  mHasRenderedFeatureHandlers = rh.mHasRenderedFeatureHandlers;
  mCustomRenderingFlags = rh.mCustomRenderingFlags;
  mClippingRegions = rh.mClippingRegions;
  mFeatureClipGeometry = rh.mFeatureClipGeometry;
  mTextureOrigin = rh.mTextureOrigin;
  mZRange = rh.mZRange;
  mSize = rh.mSize;
  mDevicePixelRatio = rh.mDevicePixelRatio;
  mImageFormat = rh.mImageFormat;
  setIsTemporal( rh.isTemporal() );
  mRendererUsage = rh.mRendererUsage;
  mFrameRate = rh.mFrameRate;
  mCurrentFrame = rh.mCurrentFrame;
  mSymbolLayerClipPaths = rh.mSymbolLayerClipPaths;
  mSymbolLayerClippingGeometries = rh.mSymbolLayerClippingGeometries;
  if ( isTemporal() )
    setTemporalRange( rh.temporalRange() );
#ifdef QGISDEBUG
  mHasTransformContext = rh.mHasTransformContext;
#endif
  mElevationMap = rh.elevationMap();

  return *this;
}

QgsRenderContext QgsRenderContext::fromQPainter( QPainter *painter )
{
  QgsRenderContext context;
  context.setPainter( painter );
  if ( painter && painter->device() )
  {
    context.setScaleFactor( painter->device()->physicalDpiX() / 25.4 );
  }
  else
  {
    context.setScaleFactor( 3.465 ); //assume 88 dpi as standard value
  }

  if ( painter && painter->renderHints() & QPainter::Antialiasing )
    context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  if ( painter && painter->renderHints() & QPainter::SmoothPixmapTransform )
    context.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  if ( painter && painter->renderHints() & QPainter::LosslessImageRendering )
    context.setFlag( Qgis::RenderContextFlag::LosslessImageRendering, true );

  return context;
}

void QgsRenderContext::setPainterFlagsUsingContext( QPainter *painter ) const
{
  if ( !painter )
    painter = mPainter;

  if ( !painter )
    return;

  painter->setRenderHint( QPainter::Antialiasing, mFlags & Qgis::RenderContextFlag::Antialiasing );
  painter->setRenderHint( QPainter::LosslessImageRendering, mFlags & Qgis::RenderContextFlag::LosslessImageRendering );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, mFlags & Qgis::RenderContextFlag::HighQualityImageTransforms );
}

QgsCoordinateTransformContext QgsRenderContext::transformContext() const
{
#ifdef QGISDEBUG
  if ( !mHasTransformContext )
    QgsDebugMsgLevel( QStringLiteral( "No QgsCoordinateTransformContext context set for transform" ), 4 );
#endif
  return mTransformContext;
}

void QgsRenderContext::setTransformContext( const QgsCoordinateTransformContext &context )
{
  mTransformContext = context;
#ifdef QGISDEBUG
  mHasTransformContext = true;
#endif
}

void QgsRenderContext::setFeedback( QgsFeedback *feedback )
{
  mFeedback = feedback;
}

QgsFeedback *QgsRenderContext::feedback() const
{
  return mFeedback;
}

void QgsRenderContext::setFlags( Qgis::RenderContextFlags flags )
{
  mFlags = flags;
}

void QgsRenderContext::setFlag( Qgis::RenderContextFlag flag, bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~( static_cast< int >( flag ) );
}

Qgis::RenderContextFlags QgsRenderContext::flags() const
{
  return mFlags;
}

bool QgsRenderContext::testFlag( Qgis::RenderContextFlag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsRenderContext QgsRenderContext::fromMapSettings( const QgsMapSettings &mapSettings )
{
  QgsRenderContext ctx;
  QgsRectangle extent = mapSettings.visibleExtent();
  extent.grow( mapSettings.extentBuffer() );
  ctx.setMapToPixel( mapSettings.mapToPixel() );
  ctx.setExtent( extent );
  ctx.setMapExtent( mapSettings.visibleExtent() );
  ctx.setFlag( Qgis::RenderContextFlag::DrawEditingInfo, mapSettings.testFlag( Qgis::MapSettingsFlag::DrawEditingInfo ) );
  ctx.setFlag( Qgis::RenderContextFlag::ForceVectorOutput, mapSettings.testFlag( Qgis::MapSettingsFlag::ForceVectorOutput ) );
  ctx.setFlag( Qgis::RenderContextFlag::UseAdvancedEffects, mapSettings.testFlag( Qgis::MapSettingsFlag::UseAdvancedEffects ) );
  ctx.setFlag( Qgis::RenderContextFlag::UseRenderingOptimization, mapSettings.testFlag( Qgis::MapSettingsFlag::UseRenderingOptimization ) );
  ctx.setCoordinateTransform( QgsCoordinateTransform() );
  ctx.setSelectionColor( mapSettings.selectionColor() );
  ctx.setFlag( Qgis::RenderContextFlag::DrawSelection, mapSettings.testFlag( Qgis::MapSettingsFlag::DrawSelection ) );
  ctx.setFlag( Qgis::RenderContextFlag::DrawSymbolBounds, mapSettings.testFlag( Qgis::MapSettingsFlag::DrawSymbolBounds ) );
  ctx.setFlag( Qgis::RenderContextFlag::RenderMapTile, mapSettings.testFlag( Qgis::MapSettingsFlag::RenderMapTile ) );
  ctx.setFlag( Qgis::RenderContextFlag::Antialiasing, mapSettings.testFlag( Qgis::MapSettingsFlag::Antialiasing ) );
  ctx.setFlag( Qgis::RenderContextFlag::RenderPartialOutput, mapSettings.testFlag( Qgis::MapSettingsFlag::RenderPartialOutput ) );
  ctx.setFlag( Qgis::RenderContextFlag::RenderPreviewJob, mapSettings.testFlag( Qgis::MapSettingsFlag::RenderPreviewJob ) );
  ctx.setFlag( Qgis::RenderContextFlag::RenderBlocking, mapSettings.testFlag( Qgis::MapSettingsFlag::RenderBlocking ) );
  ctx.setFlag( Qgis::RenderContextFlag::LosslessImageRendering, mapSettings.testFlag( Qgis::MapSettingsFlag::LosslessImageRendering ) );
  ctx.setFlag( Qgis::RenderContextFlag::Render3DMap, mapSettings.testFlag( Qgis::MapSettingsFlag::Render3DMap ) );
  ctx.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, mapSettings.testFlag( Qgis::MapSettingsFlag::HighQualityImageTransforms ) );
  ctx.setFlag( Qgis::RenderContextFlag::SkipSymbolRendering, mapSettings.testFlag( Qgis::MapSettingsFlag::SkipSymbolRendering ) );
  ctx.setFlag( Qgis::RenderContextFlag::RecordProfile, mapSettings.testFlag( Qgis::MapSettingsFlag::RecordProfile ) );
  ctx.setScaleFactor( mapSettings.outputDpi() / 25.4 ); // = pixels per mm
  ctx.setDpiTarget( mapSettings.dpiTarget() >= 0.0 ? mapSettings.dpiTarget() : -1.0 );
  ctx.setRendererScale( mapSettings.scale() );
  ctx.setExpressionContext( mapSettings.expressionContext() );
  ctx.setSegmentationTolerance( mapSettings.segmentationTolerance() );
  ctx.setSegmentationToleranceType( mapSettings.segmentationToleranceType() );
  ctx.mDistanceArea.setSourceCrs( mapSettings.destinationCrs(), mapSettings.transformContext() );
  ctx.mDistanceArea.setEllipsoid( mapSettings.ellipsoid() );
  ctx.setTransformContext( mapSettings.transformContext() );
  ctx.setPathResolver( mapSettings.pathResolver() );
  ctx.setTextRenderFormat( mapSettings.textRenderFormat() );
  ctx.setVectorSimplifyMethod( mapSettings.simplifyMethod() );
  ctx.mRenderedFeatureHandlers = mapSettings.renderedFeatureHandlers();
  ctx.mHasRenderedFeatureHandlers = !mapSettings.renderedFeatureHandlers().isEmpty();
  //this flag is only for stopping during the current rendering progress,
  //so must be false at every new render operation
  ctx.setRenderingStopped( false );
  ctx.mCustomRenderingFlags = mapSettings.customRenderingFlags();
  ctx.setIsTemporal( mapSettings.isTemporal() );
  if ( ctx.isTemporal() )
    ctx.setTemporalRange( mapSettings.temporalRange() );

  ctx.setZRange( mapSettings.zRange() );
  ctx.setOutputSize( mapSettings.outputSize() );
  ctx.setDevicePixelRatio( mapSettings.devicePixelRatio() );
  ctx.setImageFormat( mapSettings.outputImageFormat() );

  ctx.mClippingRegions = mapSettings.clippingRegions();

  ctx.mRendererUsage = mapSettings.rendererUsage();
  ctx.mFrameRate = mapSettings.frameRate();
  ctx.mCurrentFrame = mapSettings.currentFrame();

  return ctx;
}

bool QgsRenderContext::forceVectorOutput() const
{
  return mFlags.testFlag( Qgis::RenderContextFlag::ForceVectorOutput );
}

bool QgsRenderContext::useAdvancedEffects() const
{
  return mFlags.testFlag( Qgis::RenderContextFlag::UseAdvancedEffects );
}

void QgsRenderContext::setUseAdvancedEffects( bool enabled )
{
  setFlag( Qgis::RenderContextFlag::UseAdvancedEffects, enabled );
}

bool QgsRenderContext::drawEditingInformation() const
{
  return mFlags.testFlag( Qgis::RenderContextFlag::DrawEditingInfo );
}

bool QgsRenderContext::showSelection() const
{
  return mFlags.testFlag( Qgis::RenderContextFlag::DrawSelection );
}

void QgsRenderContext::setCoordinateTransform( const QgsCoordinateTransform &t )
{
  mCoordTransform = t;
}

void QgsRenderContext::setDrawEditingInformation( bool b )
{
  setFlag( Qgis::RenderContextFlag::DrawEditingInfo, b );
}

void QgsRenderContext::setForceVectorOutput( bool force )
{
  setFlag( Qgis::RenderContextFlag::ForceVectorOutput, force );
}

void QgsRenderContext::setShowSelection( const bool showSelection )
{
  setFlag( Qgis::RenderContextFlag::DrawSelection, showSelection );
}

bool QgsRenderContext::useRenderingOptimization() const
{
  return mFlags.testFlag( Qgis::RenderContextFlag::UseRenderingOptimization );
}

void QgsRenderContext::setUseRenderingOptimization( bool enabled )
{
  setFlag( Qgis::RenderContextFlag::UseRenderingOptimization, enabled );
}

void QgsRenderContext::setFeatureFilterProvider( const QgsFeatureFilterProvider *ffp )
{
  if ( ffp )
  {
    mFeatureFilterProvider.reset( ffp->clone() );
  }
  else
  {
    mFeatureFilterProvider.reset( nullptr );
  }
}

const QgsFeatureFilterProvider *QgsRenderContext::featureFilterProvider() const
{
  return mFeatureFilterProvider.get();
}

double QgsRenderContext::convertToPainterUnits( double size, Qgis::RenderUnit unit, const QgsMapUnitScale &scale, Qgis::RenderSubcomponentProperty property ) const
{
  double conversionFactor = 1.0;
  bool isMapUnitHack = false;
  switch ( unit )
  {
    case Qgis::RenderUnit::Millimeters:
      conversionFactor = mScaleFactor;
      break;

    case Qgis::RenderUnit::Points:
      conversionFactor = mScaleFactor / POINTS_TO_MM;
      break;

    case Qgis::RenderUnit::Inches:
      conversionFactor = mScaleFactor * INCH_TO_MM;
      break;

    case Qgis::RenderUnit::MetersInMapUnits:
    {
      if ( mMapToPixel.isValid() )
        size = convertMetersToMapUnits( size );
      unit = Qgis::RenderUnit::MapUnits;
      // Fall through to RenderMapUnits with size in meters converted to size in MapUnits
      [[fallthrough]];
    }
    case Qgis::RenderUnit::MapUnits:
    {
      if ( mMapToPixel.isValid() )
      {
        const double mup = scale.computeMapUnitsPerPixel( *this );
        if ( mup > 0 )
        {
          conversionFactor = 1.0 / mup;
        }
        else
        {
          conversionFactor = 1.0;
        }
      }
      else
      {
        // invalid map to pixel. A size in map units can't be calculated, so treat the size as points
        // and clamp it to a reasonable range. It's the best we can do in this situation!
        isMapUnitHack = true;
        conversionFactor = mScaleFactor / POINTS_TO_MM;
      }
      break;
    }
    case Qgis::RenderUnit::Pixels:
      conversionFactor = 1.0;
      break;

    case Qgis::RenderUnit::Unknown:
    case Qgis::RenderUnit::Percentage:
      //no sensible value
      conversionFactor = 1.0;
      break;
  }

  double convertedSize = size * conversionFactor;

  if ( unit == Qgis::RenderUnit::MapUnits )
  {
    //check max/min size
    if ( scale.minSizeMMEnabled )
      convertedSize = std::max( convertedSize, scale.minSizeMM * mScaleFactor );
    if ( scale.maxSizeMMEnabled )
      convertedSize = std::min( convertedSize, scale.maxSizeMM * mScaleFactor );
  }

  if ( isMapUnitHack )
  {
    // since we are arbitrarily treating map units as mm, we need to clamp to an (arbitrary!) reasonable range.
    convertedSize = std::clamp( convertedSize, 10.0, 100.0 );
  }
  else
  {
    const double symbologyReferenceScaleFactor = mSymbologyReferenceScale > 0 ? mSymbologyReferenceScale / mRendererScale : 1;
    convertedSize *= symbologyReferenceScaleFactor;
  }

  if ( mFlags & Qgis::RenderContextFlag::RenderSymbolPreview )
  {
    // apply property based constraints in order to optimise symbol preview rendering
    switch ( property )
    {
      case Qgis::RenderSubcomponentProperty::Generic:
        break;

      case Qgis::RenderSubcomponentProperty::ShadowOffset:
        // excessively large shadow offset in symbol preview icons is undesirable -- it pushes the shadow outside of view
        convertedSize = std::min( convertedSize, 100.0 );
        break;
      case Qgis::RenderSubcomponentProperty::BlurSize:
        // excessively large blur in symbol preview icons is too slow to calculate
        convertedSize = std::min<double>( convertedSize, 30 );
        break;
      case Qgis::RenderSubcomponentProperty::GlowSpread:
        // excessively large glow spread in symbol preview icons is too slow to calculate
        convertedSize = std::min<double>( convertedSize, 50 );
        break;
    }
  }

  return convertedSize;
}

double QgsRenderContext::convertToMapUnits( double size, Qgis::RenderUnit unit, const QgsMapUnitScale &scale ) const
{
  const double mup = mMapToPixel.mapUnitsPerPixel();

  const double symbologyReferenceScaleFactor = mSymbologyReferenceScale > 0 ? mSymbologyReferenceScale / mRendererScale : 1;

  switch ( unit )
  {
    case Qgis::RenderUnit::MetersInMapUnits:
    {
      size = convertMetersToMapUnits( size );
      // Fall through to RenderMapUnits with values of meters converted to MapUnits
      [[fallthrough]];
    }
    case Qgis::RenderUnit::MapUnits:
    {
      // check scale
      double minSizeMU = std::numeric_limits<double>::lowest();
      if ( scale.minSizeMMEnabled )
      {
        minSizeMU = scale.minSizeMM * mScaleFactor * mup;
      }
      if ( !qgsDoubleNear( scale.minScale, 0.0 ) )
      {
        minSizeMU = std::max( minSizeMU, size * ( mRendererScale / scale.minScale ) );
      }
      size = std::max( size, minSizeMU );

      double maxSizeMU = std::numeric_limits<double>::max();
      if ( scale.maxSizeMMEnabled )
      {
        maxSizeMU = scale.maxSizeMM * mScaleFactor * mup;
      }
      if ( !qgsDoubleNear( scale.maxScale, 0.0 ) )
      {
        maxSizeMU = std::min( maxSizeMU, size * ( mRendererScale / scale.maxScale ) );
      }
      size = std::min( size, maxSizeMU );

      return size;
    }
    case Qgis::RenderUnit::Millimeters:
    {
      return size * mScaleFactor * mup / symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Points:
    {
      return size * mScaleFactor * mup / POINTS_TO_MM / symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Inches:
    {
      return size * mScaleFactor * mup * INCH_TO_MM / symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Pixels:
    {
      return size * mup / symbologyReferenceScaleFactor;
    }

    case Qgis::RenderUnit::Unknown:
    case Qgis::RenderUnit::Percentage:
      //no sensible value
      return 0.0;
  }
  return 0.0;
}

double QgsRenderContext::convertFromMapUnits( double sizeInMapUnits, Qgis::RenderUnit outputUnit ) const
{
  const double mup = mMapToPixel.mapUnitsPerPixel();
  const double symbologyReferenceScaleFactor = mSymbologyReferenceScale > 0 ? mSymbologyReferenceScale / mRendererScale : 1;

  switch ( outputUnit )
  {
    case Qgis::RenderUnit::MetersInMapUnits:
    {
      return sizeInMapUnits / convertMetersToMapUnits( 1.0 );
    }
    case Qgis::RenderUnit::MapUnits:
    {
      return sizeInMapUnits;
    }
    case Qgis::RenderUnit::Millimeters:
    {
      return sizeInMapUnits / ( mScaleFactor * mup ) * symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Points:
    {
      return sizeInMapUnits / ( mScaleFactor * mup / POINTS_TO_MM ) * symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Inches:
    {
      return sizeInMapUnits / ( mScaleFactor * mup * INCH_TO_MM ) * symbologyReferenceScaleFactor;
    }
    case Qgis::RenderUnit::Pixels:
    {
      return sizeInMapUnits / mup * symbologyReferenceScaleFactor;
    }

    case Qgis::RenderUnit::Unknown:
    case Qgis::RenderUnit::Percentage:
      //no sensible value
      return 0.0;
  }
  return 0.0;
}

double QgsRenderContext::convertMetersToMapUnits( double meters ) const
{
  switch ( mDistanceArea.sourceCrs().mapUnits() )
  {
    case Qgis::DistanceUnit::Meters:
      return meters;
    case Qgis::DistanceUnit::Degrees:
    {
      if ( mExtent.isNull() )
      {
        // we don't have an extent to calculate exactly -- so just use a very rough approximation
        return meters * QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Meters, Qgis::DistanceUnit::Degrees );
      }

      QgsPointXY pointCenter = mExtent.center();
      // The Extent is in the sourceCrs(), when different from destinationCrs()
      // - the point must be transformed, since DistanceArea uses the destinationCrs()
      // Note: the default QgsCoordinateTransform() : authid() will return an empty String
      if ( !mCoordTransform.isShortCircuited() )
      {
        try
        {
          pointCenter = mCoordTransform.transform( pointCenter );
        }
        catch ( const QgsCsException & )
        {
          QgsDebugError( QStringLiteral( "QgsRenderContext::convertMetersToMapUnits(): failed to reproject pointCenter" ) );
          // what should we return;.. ?
          return meters;
        }
      }

      const int multiplier = meters < 0 ? -1 : 1;
      return multiplier * mDistanceArea.measureLineProjected( pointCenter, std::fabs( meters ) );
    }
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Inches:
    case Qgis::DistanceUnit::Unknown:
      return ( meters * QgsUnitTypes::fromUnitToUnitFactor( Qgis::DistanceUnit::Meters, mDistanceArea.sourceCrs().mapUnits() ) );
  }
  return meters;
}

QList<QgsRenderedFeatureHandlerInterface *> QgsRenderContext::renderedFeatureHandlers() const
{
  return mRenderedFeatureHandlers;
}

QList<QgsMapClippingRegion> QgsRenderContext::clippingRegions() const
{
  return mClippingRegions;
}

QgsGeometry QgsRenderContext::featureClipGeometry() const
{
  return mFeatureClipGeometry;
}

void QgsRenderContext::setFeatureClipGeometry( const QgsGeometry &geometry )
{
  mFeatureClipGeometry = geometry;
}

QPointF QgsRenderContext::textureOrigin() const
{
  return mTextureOrigin;
}

void QgsRenderContext::setTextureOrigin( const QPointF &origin )
{
  mTextureOrigin = origin;
}

QgsDoubleRange QgsRenderContext::zRange() const
{
  return mZRange;
}

void QgsRenderContext::setZRange( const QgsDoubleRange &range )
{
  mZRange = range;
}

QSize QgsRenderContext::outputSize() const
{
  return mSize;
}

void QgsRenderContext::setOutputSize( QSize size )
{
  mSize = size;
}

float QgsRenderContext::devicePixelRatio() const
{
  return mDevicePixelRatio;
}

void QgsRenderContext::setDevicePixelRatio( float ratio )
{
  mDevicePixelRatio = ratio;
}

QSize QgsRenderContext::deviceOutputSize() const
{
  return outputSize() * mDevicePixelRatio;
}

double QgsRenderContext::frameRate() const
{
  return mFrameRate;
}

void QgsRenderContext::setFrameRate( double rate )
{
  mFrameRate = rate;
}

long long QgsRenderContext::currentFrame() const
{
  return mCurrentFrame;
}

void QgsRenderContext::setCurrentFrame( long long frame )
{
  mCurrentFrame = frame;
}

QgsElevationMap *QgsRenderContext::elevationMap() const
{
  return mElevationMap;
}

void QgsRenderContext::setElevationMap( QgsElevationMap *map )
{
  mElevationMap = map;
}

void QgsRenderContext::addSymbolLayerClipPath( const QString &symbolLayerId, QPainterPath path )
{
  mSymbolLayerClipPaths[ symbolLayerId ].append( path );
}

QList<QPainterPath> QgsRenderContext::symbolLayerClipPaths( const QString &symbolLayerId ) const
{
  return mSymbolLayerClipPaths[ symbolLayerId ];
}

void QgsRenderContext::addSymbolLayerClipGeometry( const QString &symbolLayerId, const QgsGeometry &geometry )
{
  mSymbolLayerClippingGeometries[ symbolLayerId ].append( geometry );
}

QVector<QgsGeometry> QgsRenderContext::symbolLayerClipGeometries( const QString &symbolLayerId ) const
{
  return mSymbolLayerClippingGeometries[ symbolLayerId ];
}

void QgsRenderContext::setDisabledSymbolLayers( const QSet<const QgsSymbolLayer *> &symbolLayers )
{
  mDisabledSymbolLayers.clear();
  for ( const QgsSymbolLayer *symbolLayer : symbolLayers )
    mDisabledSymbolLayers << symbolLayer->id();
}

void QgsRenderContext::setDisabledSymbolLayersV2( const QSet<QString> &symbolLayers )
{
  mDisabledSymbolLayers = symbolLayers;
}

QSet<const QgsSymbolLayer *> QgsRenderContext::disabledSymbolLayers() const
{
  return QSet<const QgsSymbolLayer *>();
}

QSet<QString> QgsRenderContext::disabledSymbolLayersV2() const
{
  return mDisabledSymbolLayers;
}

bool QgsRenderContext::isSymbolLayerEnabled( const QgsSymbolLayer *layer ) const
{
  return !mDisabledSymbolLayers.contains( layer->id() );
}
