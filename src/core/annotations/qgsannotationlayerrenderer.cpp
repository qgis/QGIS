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

QgsAnnotationLayerRenderer::QgsAnnotationLayerRenderer( QgsAnnotationLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mFeedback( std::make_unique< QgsFeedback >() )
  , mLayerOpacity( layer->opacity() )
{
  // clone items from layer
  const QMap< QString, QgsAnnotationItem * > items = layer->items();
  mItems.reserve( items.size() );
  for ( auto it = items.constBegin(); it != items.constEnd(); ++it )
  {
    if ( it.value() )
      mItems << ( *it )->clone();
  }

  std::sort( mItems.begin(), mItems.end(), []( QgsAnnotationItem * a, QgsAnnotationItem * b ) { return a->zIndex() < b->zIndex(); } );  //clazy:exclude=detaching-member
}

QgsAnnotationLayerRenderer::~QgsAnnotationLayerRenderer()
{
  qDeleteAll( mItems );
}

QgsFeedback *QgsAnnotationLayerRenderer::feedback() const
{
  return mFeedback.get();
}

bool QgsAnnotationLayerRenderer::render()
{
  QgsRenderContext &context = *renderContext();

  bool canceled = false;
  for ( QgsAnnotationItem *item : std::as_const( mItems ) )
  {
    if ( mFeedback->isCanceled() )
    {
      canceled = true;
      break;
    }

    item->render( context, mFeedback.get() );
  }
  return !canceled;
}

bool QgsAnnotationLayerRenderer::forceRasterRender() const
{
  return renderContext()->testFlag( QgsRenderContext::UseAdvancedEffects ) && ( !qgsDoubleNear( mLayerOpacity, 1.0 ) );
}
