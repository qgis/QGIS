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
#include <QScopedPointer>

#include "qgsmaphittest.h"

#include "qgsmaplayerregistry.h"
#include "qgsrendercontext.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsrendererv2.h"
#include "qgspointdisplacementrenderer.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2utils.h"
#include "qgsgeometry.h"
#include "qgscrscache.h"

QgsMapHitTest::QgsMapHitTest( const QgsMapSettings& settings, const QgsGeometry& polygon, const LayerFilterExpression& layerFilterExpression )
    : mSettings( settings )
    , mLayerFilterExpression( layerFilterExpression )
    , mOnlyExpressions( false )
{
  if ( !polygon.isEmpty() && polygon.type() == QGis::Polygon )
  {
    mPolygon = polygon;
  }
}

QgsMapHitTest::QgsMapHitTest( const QgsMapSettings& settings, const LayerFilterExpression& layerFilterExpression )
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

  Q_FOREACH ( const QString& layerID, mSettings.layers() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
    if ( !vl || !vl->rendererV2() )
      continue;

    if ( !mOnlyExpressions )
    {
      if ( !vl->isInScaleRange( mSettings.scale() ) )
      {
        mHitTest[vl] = SymbolV2Set(); // no symbols -> will not be shown
        mHitTestRuleKey[vl] = SymbolV2Set();
        continue;
      }

      if ( mSettings.hasCrsTransformEnabled() )
      {
        context.setCoordinateTransform( mSettings.layerTransform( vl ) );
        context.setExtent( mSettings.outputExtentToLayerExtent( vl, mSettings.visibleExtent() ) );
      }
    }

    context.expressionContext() << QgsExpressionContextUtils::layerScope( vl );
    SymbolV2Set& usedSymbols = mHitTest[vl];
    SymbolV2Set& usedSymbolsRuleKey = mHitTestRuleKey[vl];
    runHitTestLayer( vl, usedSymbols, usedSymbolsRuleKey, context );
  }

  painter.end();
}

bool QgsMapHitTest::symbolVisible( QgsSymbolV2* symbol, QgsVectorLayer* layer ) const
{
  if ( !symbol || !layer || !mHitTest.contains( layer ) )
    return false;

  return mHitTest.value( layer ).contains( QgsSymbolLayerV2Utils::symbolProperties( symbol ) );
}

bool QgsMapHitTest::legendKeyVisible( const QString& ruleKey, QgsVectorLayer* layer ) const
{
  if ( !layer || !mHitTestRuleKey.contains( layer ) )
    return false;

  return mHitTestRuleKey.value( layer ).contains( ruleKey );
}

void QgsMapHitTest::runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, SymbolV2Set& usedSymbolsRuleKey, QgsRenderContext& context )
{
  bool hasStyleOverride = mSettings.layerStyleOverrides().contains( vl->id() );
  if ( hasStyleOverride )
    vl->styleManager()->setOverrideStyle( mSettings.layerStyleOverrides().value( vl->id() ) );

  QgsFeatureRendererV2* r = vl->rendererV2();
  bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature;
  r->startRender( context, vl->fields() );

  QgsGeometry transformedPolygon = mPolygon;
  if ( !mOnlyExpressions && !mPolygon.isEmpty() )
  {
    if ( mSettings.destinationCrs() != vl->crs() )
    {
      const QgsCoordinateTransform* ct = QgsCoordinateTransformCache::instance()->transform( mSettings.destinationCrs().authid(), vl->crs().authid() );
      transformedPolygon.transform( *ct );
    }
  }

  QgsFeature f;
  QgsFeatureRequest request;
  if ( !mOnlyExpressions )
  {
    if ( mPolygon.isEmpty() )
    {
      request.setFilterRect( context.extent() );
      request.setFlags( QgsFeatureRequest::ExactIntersect );
    }
    else
    {
      request.setFilterRect( transformedPolygon.boundingBox() );
    }
  }
  QgsFeatureIterator fi = vl->getFeatures( request );

  SymbolV2Set lUsedSymbols;
  SymbolV2Set lUsedSymbolsRuleKey;
  bool allExpressionFalse = false;
  bool hasExpression = mLayerFilterExpression.contains( vl->id() );
  QScopedPointer<QgsExpression> expr;
  if ( hasExpression )
  {
    expr.reset( new QgsExpression( mLayerFilterExpression[vl->id()] ) );
    expr->prepare( &context.expressionContext() );
  }
  while ( fi.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    // filter out elements outside of the polygon
    if ( !mOnlyExpressions && !mPolygon.isEmpty() )
    {
      if ( !transformedPolygon.intersects( f.constGeometry() ) )
      {
        continue;
      }
    }

    // filter out elements where the expression is false
    if ( hasExpression )
    {
      if ( !expr->evaluate( &context.expressionContext() ).toBool() )
        continue;
      else
        allExpressionFalse = false;
    }

    //make sure we store string representation of symbol, not pointer
    //otherwise layer style override changes will delete original symbols and leave hanging pointers
    Q_FOREACH ( const QString& legendKey, r->legendKeysForFeature( f, context ) )
    {
      lUsedSymbolsRuleKey.insert( legendKey );
    }

    if ( moreSymbolsPerFeature )
    {
      Q_FOREACH ( QgsSymbolV2* s, r->originalSymbolsForFeature( f, context ) )
      {
        if ( s )
          lUsedSymbols.insert( QgsSymbolLayerV2Utils::symbolProperties( s ) );
      }
    }
    else
    {
      QgsSymbolV2* s = r->originalSymbolForFeature( f, context );
      if ( s )
        lUsedSymbols.insert( QgsSymbolLayerV2Utils::symbolProperties( s ) );
    }
  }
  r->stopRender( context );

  if ( !allExpressionFalse )
  {
    // QSet is implicitly shared => constant time
    usedSymbols = lUsedSymbols;
    usedSymbolsRuleKey = lUsedSymbolsRuleKey;
  }

  if ( hasStyleOverride )
    vl->styleManager()->restoreOverrideStyle();
}

