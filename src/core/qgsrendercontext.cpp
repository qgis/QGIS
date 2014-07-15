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
    : mPainter( 0 ),
    mCoordTransform( 0 ),
    mDrawEditingInformation( true ),
    mForceVectorOutput( false ),
    mUseAdvancedEffects( true ),
    mRenderingStopped( false ),
    mScaleFactor( 1.0 ),
    mRasterScaleFactor( 1.0 ),
    mRendererScale( 1.0 ),
    mLabelingEngine( NULL ),
    mShowSelection( true ),
    mUseRenderingOptimization( true )
{
  mVectorSimplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
}

QgsRenderContext::~QgsRenderContext()
{
}

QgsRenderContext QgsRenderContext::fromMapSettings( const QgsMapSettings& mapSettings )
{
  QgsRenderContext ctx;
  ctx.setMapToPixel( mapSettings.mapToPixel() );
  ctx.setExtent( mapSettings.visibleExtent() );
  ctx.setDrawEditingInformation( mapSettings.testFlag( QgsMapSettings::DrawEditingInfo ) );
  ctx.setForceVectorOutput( mapSettings.testFlag( QgsMapSettings::ForceVectorOutput ) );
  ctx.setUseAdvancedEffects( mapSettings.testFlag( QgsMapSettings::UseAdvancedEffects ) );
  ctx.setUseRenderingOptimization( mapSettings.testFlag( QgsMapSettings::UseRenderingOptimization ) );
  ctx.setCoordinateTransform( 0 );
  ctx.setSelectionColor( mapSettings.selectionColor() );
  ctx.setShowSelection( mapSettings.testFlag( QgsMapSettings::DrawSelection ) );
  ctx.setRasterScaleFactor( 1.0 );
  ctx.setScaleFactor( mapSettings.outputDpi() / 25.4 ); // = pixels per mm
  ctx.setRendererScale( mapSettings.scale() );

  //this flag is only for stopping during the current rendering progress,
  //so must be false at every new render operation
  ctx.setRenderingStopped( false );

  return ctx;
}

void QgsRenderContext::setCoordinateTransform( const QgsCoordinateTransform* t )
{
  mCoordTransform = t;
}
