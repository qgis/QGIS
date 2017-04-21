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
#include "qgsexpression.h"
#include "qgsvectorlayer.h"
#include "qgsfeaturefilterprovider.h"
#include "qgslogger.h"

#define POINTS_TO_MM 2.83464567
#define INCH_TO_MM 25.4

QgsRenderContext::QgsRenderContext()
  : mFlags( DrawEditingInfo | UseAdvancedEffects | DrawSelection | UseRenderingOptimization )
{
  mVectorSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  QgsDebugMsgLevel( QString( "QgsRenderContext::QgsRenderContex -0- " ), 4 );
}

QgsRenderContext::QgsRenderContext( const QgsRenderContext &rh )
  : mFlags( rh.mFlags )
  , mPainter( rh.mPainter )
  , mCoordTransform( rh.mCoordTransform )
  , mExtent( rh.mExtent )
  , mMapToPixel( rh.mMapToPixel )
  , mRenderingStopped( rh.mRenderingStopped )
  , mScaleFactor( rh.mScaleFactor )
  , mRendererScale( rh.mRendererScale )
  , mLabelingEngine( rh.mLabelingEngine )
  , mSelectionColor( rh.mSelectionColor )
  , mVectorSimplifyMethod( rh.mVectorSimplifyMethod )
  , mExpressionContext( rh.mExpressionContext )
  , mGeometry( rh.mGeometry )
  , mFeatureFilterProvider( rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr )
  , mSegmentationTolerance( rh.mSegmentationTolerance )
  , mSegmentationToleranceType( rh.mSegmentationToleranceType )
{
  mDistanceArea.setSourceCrs( rh.mDistanceArea.sourceCrs() );
  mDistanceArea.setEllipsoid( rh.mDistanceArea.ellipsoid() );
  QgsDebugMsgLevel( QString( "QgsRenderContext::QgsRenderContext -1-:  mapUnits[%1] from center[%2] sourceCrs[%3] ellipsoidAcronym[%4]" )
                    .arg( QgsUnitTypes::toString( mDistanceArea.sourceCrs().mapUnits() ) )
                    .arg( mExtent.center().wellKnownText() )
                    .arg( mDistanceArea.sourceCrs().description() )
                    .arg( mDistanceArea.ellipsoid() ), 4 );
}

QgsRenderContext &QgsRenderContext::operator=( const QgsRenderContext &rh )
{
  mFlags = rh.mFlags;
  mPainter = rh.mPainter;
  mCoordTransform = rh.mCoordTransform;
  mExtent = rh.mExtent;
  mMapToPixel = rh.mMapToPixel;
  mRenderingStopped = rh.mRenderingStopped;
  mScaleFactor = rh.mScaleFactor;
  mRendererScale = rh.mRendererScale;
  mLabelingEngine = rh.mLabelingEngine;
  mSelectionColor = rh.mSelectionColor;
  mVectorSimplifyMethod = rh.mVectorSimplifyMethod;
  mExpressionContext = rh.mExpressionContext;
  mGeometry = rh.mGeometry;
  mFeatureFilterProvider.reset( rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr );
  mSegmentationTolerance = rh.mSegmentationTolerance;
  mSegmentationToleranceType = rh.mSegmentationToleranceType;
  mDistanceArea.setSourceCrs( rh.mDistanceArea.sourceCrs() );
  mDistanceArea.setEllipsoid( rh.mDistanceArea.ellipsoid() );
  QgsDebugMsgLevel( QString( "QgsRenderContext::operator:  mapUnits[%1] from center[%2] sourceCrs[%3] ellipsoidAcronym[%4]" )
                    .arg( QgsUnitTypes::toString( mDistanceArea.sourceCrs().mapUnits() ) )
                    .arg( mExtent.center().wellKnownText() )
                    .arg( mDistanceArea.sourceCrs().description() )
                    .arg( mDistanceArea.ellipsoid() ), 4 );
  return *this;
}

QgsRenderContext QgsRenderContext::fromQPainter( QPainter *painter )
{
  QgsRenderContext context;
  context.setPainter( painter );
  if ( painter && painter->device() )
  {
    context.setScaleFactor( painter->device()->logicalDpiX() / 25.4 );
  }
  else
  {
    context.setScaleFactor( 3.465 ); //assume 88 dpi as standard value
  }
  QgsDebugMsgLevel( QString( "QgsRenderContext::fromQPainter -0- " ), 4 );
  return context;
}

void QgsRenderContext::setFlags( QgsRenderContext::Flags flags )
{
  mFlags = flags;
}

void QgsRenderContext::setFlag( QgsRenderContext::Flag flag, bool on )
{
  if ( on )
    mFlags |= flag;
  else
    mFlags &= ~flag;
}

QgsRenderContext::Flags QgsRenderContext::flags() const
{
  return mFlags;
}

bool QgsRenderContext::testFlag( QgsRenderContext::Flag flag ) const
{
  return mFlags.testFlag( flag );
}

QgsRenderContext QgsRenderContext::fromMapSettings( const QgsMapSettings &mapSettings )
{
  QgsRenderContext ctx;
  ctx.setMapToPixel( mapSettings.mapToPixel() );
  ctx.setExtent( mapSettings.visibleExtent() );
  ctx.setFlag( DrawEditingInfo, mapSettings.testFlag( QgsMapSettings::DrawEditingInfo ) );
  ctx.setFlag( ForceVectorOutput, mapSettings.testFlag( QgsMapSettings::ForceVectorOutput ) );
  ctx.setFlag( UseAdvancedEffects, mapSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) );
  ctx.setFlag( UseRenderingOptimization, mapSettings.testFlag( QgsMapSettings::UseRenderingOptimization ) );
  ctx.setCoordinateTransform( QgsCoordinateTransform() );
  ctx.setSelectionColor( mapSettings.selectionColor() );
  ctx.setFlag( DrawSelection, mapSettings.testFlag( QgsMapSettings::DrawSelection ) );
  ctx.setFlag( DrawSymbolBounds, mapSettings.testFlag( QgsMapSettings::DrawSymbolBounds ) );
  ctx.setFlag( RenderMapTile, mapSettings.testFlag( QgsMapSettings::RenderMapTile ) );
  ctx.setFlag( Antialiasing, mapSettings.testFlag( QgsMapSettings::Antialiasing ) );
  ctx.setFlag( RenderPartialOutput, mapSettings.testFlag( QgsMapSettings::RenderPartialOutput ) );
  ctx.setScaleFactor( mapSettings.outputDpi() / 25.4 ); // = pixels per mm
  ctx.setRendererScale( mapSettings.scale() );
  ctx.setExpressionContext( mapSettings.expressionContext() );
  ctx.setSegmentationTolerance( mapSettings.segmentationTolerance() );
  ctx.setSegmentationToleranceType( mapSettings.segmentationToleranceType() );
  ctx.mDistanceArea.setSourceCrs( mapSettings.destinationCrs() );
  ctx.mDistanceArea.setEllipsoid( mapSettings.ellipsoid() );
  QgsDebugMsgLevel( QString( "QgsRenderContext::fromMapSettings:mapUnits[%1] from center[%2] sourceCrs[%3] ellipsoidAcronym[%4,%5]" )
                    .arg( QgsUnitTypes::toString( ctx.mDistanceArea.sourceCrs().mapUnits() ) )
                    .arg( mapSettings.visibleExtent().center().wellKnownText() )
                    .arg( ctx.mDistanceArea.sourceCrs().description() )
                    .arg( ctx.mDistanceArea.ellipsoid() )
                    .arg( mapSettings.ellipsoid() ), 4 );

  //this flag is only for stopping during the current rendering progress,
  //so must be false at every new render operation
  ctx.setRenderingStopped( false );

  return ctx;
}

bool QgsRenderContext::forceVectorOutput() const
{
  return mFlags.testFlag( ForceVectorOutput );
}

bool QgsRenderContext::useAdvancedEffects() const
{
  return mFlags.testFlag( UseAdvancedEffects );
}

void QgsRenderContext::setUseAdvancedEffects( bool enabled )
{
  setFlag( UseAdvancedEffects, enabled );
}

bool QgsRenderContext::drawEditingInformation() const
{
  return mFlags.testFlag( DrawEditingInfo );
}

bool QgsRenderContext::showSelection() const
{
  return mFlags.testFlag( DrawSelection );
}

void QgsRenderContext::setCoordinateTransform( const QgsCoordinateTransform &t )
{
  mCoordTransform = t;
}

void QgsRenderContext::setDrawEditingInformation( bool b )
{
  setFlag( DrawEditingInfo, b );
}

void QgsRenderContext::setForceVectorOutput( bool force )
{
  setFlag( ForceVectorOutput, force );
}

void QgsRenderContext::setShowSelection( const bool showSelection )
{
  setFlag( DrawSelection, showSelection );
}

bool QgsRenderContext::useRenderingOptimization() const
{
  return mFlags.testFlag( UseRenderingOptimization );
}

void QgsRenderContext::setUseRenderingOptimization( bool enabled )
{
  setFlag( UseRenderingOptimization, enabled );
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

double QgsRenderContext::convertToPainterUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale ) const
{
  double conversionFactor = 1.0;
  switch ( unit )
  {
    case QgsUnitTypes::RenderMillimeters:
      conversionFactor = mScaleFactor;
      break;

    case QgsUnitTypes::RenderPoints:
      conversionFactor = mScaleFactor / POINTS_TO_MM;
      break;

    case QgsUnitTypes::RenderInches:
      conversionFactor = mScaleFactor * INCH_TO_MM;
      break;

    case QgsUnitTypes::RenderMetersInMapUnits:
    {
      double mup = mapToPixel().mapUnitsPerPixel() / convertMetersToMapUnits( 1.0 );
      if ( mup > 0 )
      {
        conversionFactor = 1.0 / mup;
      }
      else
      {
        conversionFactor = 1.0;
      }
      QgsDebugMsgLevel( QString( "convertToPainterUnits -1- : size[%1] mup[%2] mapUnitsPerPixel[%3] conversionFactor[%4] mapUnits[%5] from center[%6] sourceCrs[%7] isGeographic[%8]" ).arg( size )
                        .arg( mup )
                        .arg( mapToPixel().mapUnitsPerPixel() )
                        .arg( conversionFactor )
                        .arg( QgsUnitTypes::toString( mDistanceArea.sourceCrs().mapUnits() ) )
                        .arg( mExtent.center().wellKnownText() )
                        .arg( mDistanceArea.sourceCrs().description() )
                        .arg( mDistanceArea.sourceCrs().isGeographic() ), 4 );
      unit = QgsUnitTypes::RenderMapUnits;
      // Act as if RenderMapUnits with values of meters converted to MapUnits
      break;
    }
    case QgsUnitTypes::RenderMapUnits:
    {
      double mup = scale.computeMapUnitsPerPixel( *this );
      if ( mup > 0 )
      {
        conversionFactor = 1.0 / mup;
      }
      else
      {
        conversionFactor = 1.0;
      }
      QgsDebugMsgLevel( QString( "convertToPainterUnits -2- : size[%1]  mup[%2] conversionFactor[%3] sourceCrs[%4] isGeographic[%5]" ).arg( size )
                        .arg( mup )
                        .arg( conversionFactor )
                        .arg( mDistanceArea.sourceCrs().description() )
                        .arg( mDistanceArea.sourceCrs().isGeographic() ), 4 );
      break;
    }
    case QgsUnitTypes::RenderPixels:
      conversionFactor = 1.0;
      break;

    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      conversionFactor = 1.0;
      break;
  }

  double convertedSize = size * conversionFactor;

  if ( unit == QgsUnitTypes::RenderMapUnits )
  {
    //check max/min size
    if ( scale.minSizeMMEnabled )
      convertedSize = qMax( convertedSize, scale.minSizeMM * mScaleFactor );
    if ( scale.maxSizeMMEnabled )
      convertedSize = qMin( convertedSize, scale.maxSizeMM * mScaleFactor );
    QgsDebugMsgLevel( QString( "convertToPainterUnits -3- : convertedSize[%1]" ).arg( convertedSize ), 4 );
  }

  return convertedSize;
}

double QgsRenderContext::convertToMapUnits( double size, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &scale ) const
{
  double mup = mMapToPixel.mapUnitsPerPixel();

  switch ( unit )
  {
    case QgsUnitTypes::RenderMetersInMapUnits:
    {
      QgsDebugMsgLevel( QString( "convertToMapUnits: mup[%1] size[%2] mapUnits[%3]from center[%4] sourceCrs[%5]" ).arg( mup )
                        .arg( size )
                        .arg( QgsUnitTypes::toString( mDistanceArea.sourceCrs().mapUnits() ) )
                        .arg( mExtent.center().wellKnownText() )
                        .arg( mDistanceArea.sourceCrs().description() ), 4 );
      size = convertMetersToMapUnits( size );
      // Fall through to RenderMapUnits with values of meters converted to MapUnits
    }
    case QgsUnitTypes::RenderMapUnits:
    {
      // check scale
      double minSizeMU = -DBL_MAX;
      if ( scale.minSizeMMEnabled )
      {
        minSizeMU = scale.minSizeMM * mScaleFactor * mup;
      }
      if ( !qgsDoubleNear( scale.minScale, 0.0 ) )
      {
        minSizeMU = qMax( minSizeMU, size * ( scale.minScale * mRendererScale ) );
      }
      size = qMax( size, minSizeMU );

      double maxSizeMU = DBL_MAX;
      if ( scale.maxSizeMMEnabled )
      {
        maxSizeMU = scale.maxSizeMM * mScaleFactor * mup;
      }
      if ( !qgsDoubleNear( scale.maxScale, 0.0 ) )
      {
        maxSizeMU = qMin( maxSizeMU, size * ( scale.maxScale * mRendererScale ) );
      }
      size = qMin( size, maxSizeMU );

      return size;
    }
    case QgsUnitTypes::RenderMillimeters:
    {
      return size * mScaleFactor * mup;
    }
    case QgsUnitTypes::RenderPoints:
    {
      return size * mScaleFactor * mup / POINTS_TO_MM;
    }
    case QgsUnitTypes::RenderInches:
    {
      return size * mScaleFactor * mup * INCH_TO_MM;
    }
    case QgsUnitTypes::RenderPixels:
    {
      return size * mup;
    }

    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 0.0;
  }
  return 0.0;
}

double QgsRenderContext::convertFromMapUnits( double sizeInMapUnits, QgsUnitTypes::RenderUnit outputUnit ) const
{
  double mup = mMapToPixel.mapUnitsPerPixel();

  switch ( outputUnit )
  {
    case QgsUnitTypes::RenderMetersInMapUnits:
    {
      QgsDebugMsgLevel( QString( "convertFromMapUnits: meter[%1] sizeInMapUnits[%2] mapUnits[%3]from center[%4] sourceCrs[%5]" ).arg( convertMetersToMapUnits( 1.0 ) )
                        .arg( sizeInMapUnits )
                        .arg( QgsUnitTypes::toString( mDistanceArea.sourceCrs().mapUnits() ) )
                        .arg( mExtent.center().wellKnownText() )
                        .arg( mDistanceArea.sourceCrs().description() ), 4 );
      return sizeInMapUnits / convertMetersToMapUnits( 1.0 );
    }
    case QgsUnitTypes::RenderMapUnits:
    {
      return sizeInMapUnits;
    }
    case QgsUnitTypes::RenderMillimeters:
    {
      return sizeInMapUnits / ( mScaleFactor * mup );
    }
    case QgsUnitTypes::RenderPoints:
    {
      return sizeInMapUnits / ( mScaleFactor * mup / POINTS_TO_MM );
    }
    case QgsUnitTypes::RenderInches:
    {
      return sizeInMapUnits / ( mScaleFactor * mup * INCH_TO_MM );
    }
    case QgsUnitTypes::RenderPixels:
    {
      return sizeInMapUnits / mup;
    }

    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      //no sensible value
      return 0.0;
  }
  return 0.0;
}

double QgsRenderContext::convertMetersToMapUnits( double meters ) const
{
  if ( mDistanceArea.sourceCrs().mapUnits() != QgsUnitTypes::DistanceMeters )
  {
    return mDistanceArea.measureLineProjected( mExtent.center(), meters );
  }
  return meters;
}
