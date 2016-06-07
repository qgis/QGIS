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

QgsRenderContext::QgsRenderContext()
    : mFlags( DrawEditingInfo | UseAdvancedEffects | DrawSelection | UseRenderingOptimization )
    , mPainter( nullptr )
    , mCoordTransform( nullptr )
    , mRenderingStopped( false )
    , mScaleFactor( 1.0 )
    , mRasterScaleFactor( 1.0 )
    , mRendererScale( 1.0 )
    , mLabelingEngine( nullptr )
    , mLabelingEngine2( nullptr )
    , mGeometry( nullptr )
    , mFeatureFilterProvider( nullptr )
    , mSegmentationTolerance( M_PI_2 / 90 )
    , mSegmentationToleranceType( QgsAbstractGeometryV2::MaximumAngle )
{
  mVectorSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
}

QgsRenderContext::QgsRenderContext( const QgsRenderContext& rh )
    : mFlags( rh.mFlags )
    , mPainter( rh.mPainter )
    , mCoordTransform( rh.mCoordTransform )
    , mExtent( rh.mExtent )
    , mMapToPixel( rh.mMapToPixel )
    , mRenderingStopped( rh.mRenderingStopped )
    , mScaleFactor( rh.mScaleFactor )
    , mRasterScaleFactor( rh.mRasterScaleFactor )
    , mRendererScale( rh.mRendererScale )
    , mLabelingEngine( rh.mLabelingEngine )
    , mLabelingEngine2( rh.mLabelingEngine2 )
    , mSelectionColor( rh.mSelectionColor )
    , mVectorSimplifyMethod( rh.mVectorSimplifyMethod )
    , mExpressionContext( rh.mExpressionContext )
    , mGeometry( rh.mGeometry )
    , mFeatureFilterProvider( rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr )
    , mSegmentationTolerance( rh.mSegmentationTolerance )
    , mSegmentationToleranceType( rh.mSegmentationToleranceType )
{
}

QgsRenderContext&QgsRenderContext::operator=( const QgsRenderContext & rh )
{
  mFlags = rh.mFlags;
  mPainter = rh.mPainter;
  mCoordTransform = rh.mCoordTransform;
  mExtent = rh.mExtent;
  mMapToPixel = rh.mMapToPixel;
  mRenderingStopped = rh.mRenderingStopped;
  mScaleFactor = rh.mScaleFactor;
  mRasterScaleFactor = rh.mRasterScaleFactor;
  mRendererScale = rh.mRendererScale;
  mLabelingEngine = rh.mLabelingEngine;
  mLabelingEngine2 = rh.mLabelingEngine2;
  mSelectionColor = rh.mSelectionColor;
  mVectorSimplifyMethod = rh.mVectorSimplifyMethod;
  mExpressionContext = rh.mExpressionContext;
  mGeometry = rh.mGeometry;
  mFeatureFilterProvider = rh.mFeatureFilterProvider ? rh.mFeatureFilterProvider->clone() : nullptr;
  mSegmentationTolerance = rh.mSegmentationTolerance;
  mSegmentationToleranceType = rh.mSegmentationToleranceType;
  return *this;
}

QgsRenderContext::~QgsRenderContext()
{
  delete mFeatureFilterProvider;
  mFeatureFilterProvider = nullptr;
}

void QgsRenderContext::setFlags( const QgsRenderContext::Flags& flags )
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

QgsRenderContext QgsRenderContext::fromMapSettings( const QgsMapSettings& mapSettings )
{
  QgsRenderContext ctx;
  ctx.setMapToPixel( mapSettings.mapToPixel() );
  ctx.setExtent( mapSettings.visibleExtent() );
  ctx.setFlag( DrawEditingInfo, mapSettings.testFlag( QgsMapSettings::DrawEditingInfo ) );
  ctx.setFlag( ForceVectorOutput, mapSettings.testFlag( QgsMapSettings::ForceVectorOutput ) );
  ctx.setFlag( UseAdvancedEffects, mapSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) );
  ctx.setFlag( UseRenderingOptimization, mapSettings.testFlag( QgsMapSettings::UseRenderingOptimization ) );
  ctx.setCoordinateTransform( nullptr );
  ctx.setSelectionColor( mapSettings.selectionColor() );
  ctx.setFlag( DrawSelection, mapSettings.testFlag( QgsMapSettings::DrawSelection ) );
  ctx.setFlag( DrawSymbolBounds, mapSettings.testFlag( QgsMapSettings::DrawSymbolBounds ) );
  ctx.setFlag( RenderMapTile, mapSettings.testFlag( QgsMapSettings::RenderMapTile ) );
  ctx.setFlag( Antialiasing, mapSettings.testFlag( QgsMapSettings::Antialiasing ) );
  ctx.setRasterScaleFactor( 1.0 );
  ctx.setScaleFactor( mapSettings.outputDpi() / 25.4 ); // = pixels per mm
  ctx.setRendererScale( mapSettings.scale() );
  ctx.setExpressionContext( mapSettings.expressionContext() );
  ctx.setSegmentationTolerance( mapSettings.segmentationTolerance() );
  ctx.setSegmentationToleranceType( mapSettings.segmentationToleranceType() );

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

void QgsRenderContext::setCoordinateTransform( const QgsCoordinateTransform* t )
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

void QgsRenderContext::setFeatureFilterProvider( const QgsFeatureFilterProvider* ffp )
{
  delete mFeatureFilterProvider;
  mFeatureFilterProvider = nullptr;

  if ( ffp )
  {
    mFeatureFilterProvider = ffp->clone();
  }
}
