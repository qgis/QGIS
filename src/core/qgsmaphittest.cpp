/***************************************************************************
    qgsmaphittest.cpp
    ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaphittest.h"

#include "qgsfeatureiterator.h"
#include "qgsrendercontext.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaplayerstyle.h"
#include "qgsvectorlayerfeatureiterator.h"

QgsMapHitTest::QgsMapHitTest( const QgsMapSettings &settings, const QgsGeometry &polygon, const LayerFilterExpression &layerFilterExpression )
  : mSettings( settings )
  , mLayerFilterExpression( layerFilterExpression )
  , mOnlyExpressions( false )
{
  if ( !polygon.isNull() && polygon.type() == Qgis::GeometryType::Polygon )
  {
    mPolygon = polygon;
  }
}

QgsMapHitTest::QgsMapHitTest( const QgsMapSettings &settings, const LayerFilterExpression &layerFilterExpression )
  : mSettings( settings )
  , mLayerFilterExpression( layerFilterExpression )
  , mOnlyExpressions( true )
{
}

void QgsMapHitTest::run()
{
  // TODO: do we need this temp image?
  QImage tmpImage( mSettings.outputSize(), mSettings.outputImageFormat() );
  tmpImage.setDotsPerMeterX( mSettings.outputDpi() * 25.4 );
  tmpImage.setDotsPerMeterY( mSettings.outputDpi() * 25.4 );
  QPainter painter( &tmpImage );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mSettings );
  context.setPainter( &painter ); // we are not going to draw anything, but we still need a working painter

  const QList< QgsMapLayer * > layers = mSettings.layers( true );
  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vl || !vl->renderer() )
      continue;

    if ( !mOnlyExpressions )
    {
      if ( !vl->isInScaleRange( mSettings.scale() ) )
      {
        continue;
      }

      context.setCoordinateTransform( mSettings.layerTransform( vl ) );
      context.setExtent( mSettings.outputExtentToLayerExtent( vl, mSettings.visibleExtent() ) );
    }

    context.expressionContext() << QgsExpressionContextUtils::layerScope( vl );
    SymbolSet &usedSymbols = mHitTest[vl->id()];
    SymbolSet &usedSymbolsRuleKey = mHitTestRuleKey[vl->id()];

    QgsMapLayerStyleOverride styleOverride( vl );
    if ( mSettings.layerStyleOverrides().contains( vl->id() ) )
      styleOverride.setOverrideStyle( mSettings.layerStyleOverrides().value( vl->id() ) );

    std::unique_ptr< QgsVectorLayerFeatureSource > source = std::make_unique< QgsVectorLayerFeatureSource >( vl );
    runHitTestFeatureSource( source.get(),
                             vl->id(), vl->crs(), vl->fields(), vl->renderer(),
                             usedSymbols, usedSymbolsRuleKey, context,
                             nullptr );
  }

  painter.end();
}

QMap<QString, QSet<QString> > QgsMapHitTest::results() const
{
  return mHitTestRuleKey;
}

bool QgsMapHitTest::symbolVisible( QgsSymbol *symbol, QgsVectorLayer *layer ) const
{
  if ( !symbol || !layer )
    return false;

  auto it = mHitTest.constFind( layer->id() );
  if ( it == mHitTest.constEnd() )
    return false;

  return it->contains( QgsSymbolLayerUtils::symbolProperties( symbol ) );
}

bool QgsMapHitTest::legendKeyVisible( const QString &ruleKey, QgsVectorLayer *layer ) const
{
  if ( !layer )
    return false;

  auto it = mHitTestRuleKey.constFind( layer->id() );
  if ( it == mHitTestRuleKey.constEnd() )
    return false;

  return it->contains( ruleKey );
}

void QgsMapHitTest::runHitTestFeatureSource( QgsAbstractFeatureSource *source,
    const QString &layerId,
    const QgsCoordinateReferenceSystem &crs,
    const QgsFields &fields,
    const QgsFeatureRenderer *renderer,
    SymbolSet &usedSymbols,
    SymbolSet &usedSymbolsRuleKey,
    QgsRenderContext &context,
    QgsFeedback *feedback )
{
  std::unique_ptr< QgsFeatureRenderer > r( renderer->clone() );
  const bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature;
  r->startRender( context, fields );

  // shortcut early if we know that there's nothing visible
  if ( r->canSkipRender() )
  {
    r->stopRender( context );
    return;
  }

  // if there's no legend items for this layer, shortcut out early
  QSet< QString > remainingKeysToFind = r->legendKeys();
  if ( remainingKeysToFind.empty() )
  {
    r->stopRender( context );
    return;
  }

  QgsFeatureRequest request;
  if ( feedback )
    request.setFeedback( feedback );

  const QString rendererFilterExpression = r->filter( fields );
  if ( !rendererFilterExpression.isEmpty() )
  {
    request.setFilterExpression( rendererFilterExpression );
  }

  QSet<QString> requiredAttributes = r->usedAttributes( context );

  QgsGeometry transformedPolygon = mPolygon;
  if ( !mOnlyExpressions && !mPolygon.isNull() )
  {
    if ( mSettings.destinationCrs() != crs )
    {
      const QgsCoordinateTransform ct( mSettings.destinationCrs(), crs, mSettings.transformContext() );
      transformedPolygon.transform( ct );
    }
  }

  if ( feedback && feedback->isCanceled() )
  {
    r->stopRender( context );
    return;
  }

  if ( auto it = mLayerFilterExpression.constFind( layerId ); it != mLayerFilterExpression.constEnd() )
  {
    const QString expression = *it;
    QgsExpression expr( expression );
    expr.prepare( &context.expressionContext() );

    requiredAttributes.unite( expr.referencedColumns() );
    request.combineFilterExpression( expression );
  }

  request.setSubsetOfAttributes( requiredAttributes, fields );

  std::unique_ptr< QgsGeometryEngine > polygonEngine;
  if ( !mOnlyExpressions )
  {
    if ( mPolygon.isNull() )
    {
      request.setFilterRect( context.extent() );
      request.setFlags( QgsFeatureRequest::ExactIntersect );
    }
    else
    {
      request.setFilterRect( transformedPolygon.boundingBox() );
      polygonEngine.reset( QgsGeometry::createGeometryEngine( transformedPolygon.constGet() ) );
      polygonEngine->prepareGeometry();
    }
  }

  if ( feedback && feedback->isCanceled() )
  {
    r->stopRender( context );
    return;
  }

  QgsFeatureIterator fi = source->getFeatures( request );

  usedSymbols.clear();
  usedSymbolsRuleKey.clear();

  QgsFeature f;
  while ( fi.nextFeature( f ) )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    // filter out elements outside of the polygon
    if ( f.hasGeometry() && polygonEngine )
    {
      if ( !polygonEngine->intersects( f.geometry().constGet() ) )
      {
        continue;
      }
    }

    context.expressionContext().setFeature( f );

    //make sure we store string representation of symbol, not pointer
    //otherwise layer style override changes will delete original symbols and leave hanging pointers
    const QSet< QString > legendKeysForFeature = r->legendKeysForFeature( f, context );
    for ( const QString &legendKey : legendKeysForFeature )
    {
      usedSymbolsRuleKey.insert( legendKey );
      remainingKeysToFind.remove( legendKey );
    }

    if ( moreSymbolsPerFeature )
    {
      const QgsSymbolList originalSymbolsForFeature = r->originalSymbolsForFeature( f, context );
      for ( QgsSymbol *s : originalSymbolsForFeature )
      {
        if ( s )
          usedSymbols.insert( QgsSymbolLayerUtils::symbolProperties( s ) );
      }
    }
    else
    {
      QgsSymbol *s = r->originalSymbolForFeature( f, context );
      if ( s )
        usedSymbols.insert( QgsSymbolLayerUtils::symbolProperties( s ) );
    }

    if ( remainingKeysToFind.empty() )
    {
      // already found features for all legend items, no need to keep searching
      break;
    }
  }
  r->stopRender( context );
}

