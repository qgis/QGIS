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

QgsRenderContext::QgsRenderContext()
    : mFlags( DrawEditingInfo | UseAdvancedEffects | DrawSelection | UseRenderingOptimization )
    , mPainter( 0 )
    , mCoordTransform( 0 )
    , mRenderingStopped( false )
    , mScaleFactor( 1.0 )
    , mRasterScaleFactor( 1.0 )
    , mRendererScale( 1.0 )
    , mLabelingEngine( NULL )
    , mLabelingEngine2( 0 )
    , mGeometry( 0 )
{
  mVectorSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
}

QgsRenderContext::~QgsRenderContext()
{
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
  ctx.setCoordinateTransform( 0 );
  ctx.setSelectionColor( mapSettings.selectionColor() );
  ctx.setFlag( DrawSelection, mapSettings.testFlag( QgsMapSettings::DrawSelection ) );
  ctx.setRasterScaleFactor( 1.0 );
  ctx.setScaleFactor( mapSettings.outputDpi() / 25.4 ); // = pixels per mm
  ctx.setRendererScale( mapSettings.scale() );
  ctx.setExpressionContext( mapSettings.expressionContext() );

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
