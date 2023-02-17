/***************************************************************************
    qgsannotationlayerrenderer.cpp
    ------------------
    copyright            : (C) 2019 by Sandro Mani
    email                : smani at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotationlayerrenderer.h"
#include "qgsannotationlayer.h"
#include "qgsfeedback.h"
#include "qgsrenderedannotationitemdetails.h"
#include "qgspainteffect.h"
#include "qgsrendercontext.h"
#include <optional>

QgsAnnotationLayerRenderer::QgsAnnotationLayerRenderer( QgsAnnotationLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mLayerOpacity( layer->opacity() )
{
  // Clone items from layer which fall inside the rendered extent
  // Because some items have scale dependent bounds, we have to accept some limitations here.
  // first, we can use the layer's spatial index to very quickly retrieve items we know will fall within the visible
  // extent. This will ONLY apply to items which have a non-scale-dependent bounding box though.

  const QStringList itemsList = layer->queryIndex( context.extent() );
  QSet< QString > items( itemsList.begin(), itemsList.end() );

  // we also have NO choice but to clone ALL non-indexed items (i.e. those with a scale-dependent bounding box)
  // since these won't be in the layer's spatial index, and it's too expensive to determine their actual bounding box
  // upfront (we are blocking the main thread right now!)

  // TODO -- come up with some brilliant way to avoid this and also index scale-dependent items ;)
  items.unite( layer->mNonIndexedItems );

  mItems.reserve( items.size() );
  std::transform( items.begin(), items.end(), std::back_inserter( mItems ),
                  [layer]( const QString & id ) ->std::pair< QString, std::unique_ptr< QgsAnnotationItem > >
  {
    return std::make_pair( id, std::unique_ptr< QgsAnnotationItem >( layer->item( id )->clone() ) );
  } );

  std::sort( mItems.begin(), mItems.end(), [](
               const std::pair< QString, std::unique_ptr< QgsAnnotationItem > > &a,
               const std::pair< QString, std::unique_ptr< QgsAnnotationItem > > &b )
  { return a.second->zIndex() < b.second->zIndex(); } );

  if ( layer->paintEffect() && layer->paintEffect()->enabled() )
  {
    mPaintEffect.reset( layer->paintEffect()->clone() );
  }
}

QgsAnnotationLayerRenderer::~QgsAnnotationLayerRenderer() = default;

QgsFeedback *QgsAnnotationLayerRenderer::feedback() const
{
  return mFeedback.get();
}

bool QgsAnnotationLayerRenderer::render()
{
  QgsRenderContext &context = *renderContext();

  if ( mPaintEffect )
  {
    mPaintEffect->begin( context );
  }

  bool canceled = false;
  for ( const std::pair< QString, std::unique_ptr< QgsAnnotationItem > > &item : std::as_const( mItems ) )
  {
    if ( mFeedback->isCanceled() )
    {
      canceled = true;
      break;
    }

    std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
    if ( item.second->useSymbologyReferenceScale() )
    {
      referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, item.second->symbologyReferenceScale() ) );
    }

    const QgsRectangle bounds = item.second->boundingBox( context );
    if ( bounds.intersects( context.extent() ) )
    {
      item.second->render( context, mFeedback.get() );
      std::unique_ptr< QgsRenderedAnnotationItemDetails > details = std::make_unique< QgsRenderedAnnotationItemDetails >( mLayerID, item.first );
      details->setBoundingBox( bounds );
      appendRenderedItemDetails( details.release() );
    }
  }

  if ( mPaintEffect )
  {
    mPaintEffect->end( context );
  }

  return !canceled;
}

bool QgsAnnotationLayerRenderer::forceRasterRender() const
{
  return renderContext()->testFlag( Qgis::RenderContextFlag::UseAdvancedEffects ) && ( !qgsDoubleNear( mLayerOpacity, 1.0 ) );
}
