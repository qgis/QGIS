/***************************************************************************
    qgsgrouplayerrenderer.cpp
    ----------------
  Date                 : September 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrouplayerrenderer.h"
#include "qgsgrouplayer.h"
#include "qgsfeedback.h"
#include "qgspainteffect.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include <optional>

QgsGroupLayerRenderer::QgsGroupLayerRenderer( QgsGroupLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mLayerOpacity( layer->opacity() )
{
  const QList< QgsMapLayer * > layers = layer->childLayers();
  const QgsCoordinateReferenceSystem destinationCrs = context.coordinateTransform().destinationCrs();
  for ( QgsMapLayer *childLayer : layers )
  {
    // we have to temporarily set the context's crs and extent to the correct one for the child layer, BEFORE creating the
    // child layer's renderer
    QgsCoordinateTransform layerToDestTransform( childLayer->crs(), destinationCrs, context.transformContext() );
    layerToDestTransform.setBallparkTransformsAreAppropriate( true );
    context.setCoordinateTransform( layerToDestTransform );
    try
    {
      const QgsRectangle extentInChildLayerCrs = layerToDestTransform.transformBoundingBox( context.mapExtent(), Qgis::TransformDirection::Reverse );
      context.setExtent( extentInChildLayerCrs );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Error transforming extent of %1 to destination CRS" ).arg( childLayer->id() ) );
      continue;
    }

    mChildRenderers.emplace_back( childLayer->createMapRenderer( context ) );
    mRendererCompositionModes.emplace_back( childLayer->blendMode() );
    mRendererOpacity.emplace_back( childLayer->type() != QgsMapLayerType::RasterLayer ? childLayer->opacity() : 1.0 );
    mTransforms.emplace_back( layerToDestTransform );
  }

  mPaintEffect.reset( layer->paintEffect() && layer->paintEffect()->enabled() ? layer->paintEffect()->clone() : nullptr );

  mForceRasterRender = layer->blendMode() != QPainter::CompositionMode_SourceOver;
}

QgsGroupLayerRenderer::~QgsGroupLayerRenderer() = default;

QgsFeedback *QgsGroupLayerRenderer::feedback() const
{
  return mFeedback.get();
}

bool QgsGroupLayerRenderer::render()
{
  QgsRenderContext &context = *renderContext();

  context.painter()->save();
  if ( mPaintEffect )
  {
    mPaintEffect->begin( context );
  }

  const QgsCoordinateReferenceSystem destinationCrs = context.coordinateTransform().destinationCrs();
  bool canceled = false;
  int i = 0;
  for ( const std::unique_ptr< QgsMapLayerRenderer > &renderer : std::as_const( mChildRenderers ) )
  {
    if ( mFeedback->isCanceled() )
    {
      canceled = true;
      break;
    }

    context.setCoordinateTransform( mTransforms[i] );

    // don't need to catch exceptions here -- it would have already been caught in the QgsGroupLayerRenderer constructor!
    const QgsRectangle extentInChildLayerCrs = mTransforms[i].transformBoundingBox( context.mapExtent(), Qgis::TransformDirection::Reverse );
    context.setExtent( extentInChildLayerCrs );

    QImage image;
    if ( context.useAdvancedEffects() )
      context.painter()->setCompositionMode( mRendererCompositionModes[i] );

    QPainter *prevPainter = context.painter();
    std::unique_ptr< QPainter > imagePainter;
    if ( renderer->forceRasterRender() )
    {
      image = QImage( context.deviceOutputSize(), context.imageFormat() );
      image.setDevicePixelRatio( static_cast<qreal>( context.devicePixelRatio() ) );
      image.fill( 0 );
      imagePainter = std::make_unique< QPainter >( &image );

      context.setPainterFlagsUsingContext( imagePainter.get() );
      context.setPainter( imagePainter.get() );
    }
    renderer->render();

    if ( imagePainter )
    {
      imagePainter->end();
      context.setPainter( prevPainter );

      context.painter()->setOpacity( mRendererOpacity[i] );
      context.painter()->drawImage( 0, 0, image );
      context.painter()->setOpacity( 1.0 );
    }
    context.painter()->setCompositionMode( QPainter::CompositionMode_SourceOver );
    i++;
  }

  if ( mPaintEffect )
  {
    mPaintEffect->end( context );
  }

  context.painter()->restore();

  return !canceled;
}

bool QgsGroupLayerRenderer::forceRasterRender() const
{
  if ( !renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) )
    return false;

  if ( mForceRasterRender || !qgsDoubleNear( mLayerOpacity, 1.0 ) )
    return true;

  for ( QPainter::CompositionMode mode : mRendererCompositionModes )
  {
    if ( mode != QPainter::CompositionMode_SourceOver )
      return true;
  }

  return false;
}
