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
  tmpImage.setDotsPerMeterX( static_cast< int >( std::round( mSettings.outputDpi() * 25.4 ) ) );
  tmpImage.setDotsPerMeterY( static_cast< int >( std::round( mSettings.outputDpi() * 25.4 ) ) );
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

///@cond PRIVATE
QMap<QString, QList<QString> > QgsMapHitTest::resultsPy() const
{
  QMap<QString, QList<QString> > res;
  for ( auto it = mHitTestRuleKey.begin(); it != mHitTestRuleKey.end(); ++it )
  {
    res.insert( it.key(), qgis::setToList( it.value() ) );
  }
  return res;
}
///@endcond PRIVATE

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


//
// QgsMapHitTestTask
//

QgsMapHitTestTask::QgsMapHitTestTask( const QgsMapSettings &settings, const QgsGeometry &polygon, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression )
  : QgsTask( tr( "Updating Legend" ), QgsTask::Flag::CanCancel | QgsTask::Flag::CancelWithoutPrompt | QgsTask::Flag::Silent )
  , mSettings( settings )
  , mLayerFilterExpression( layerFilterExpression )
  , mPolygon( polygon )
  , mOnlyExpressions( false )
{
  prepare();
}

QgsMapHitTestTask::QgsMapHitTestTask( const QgsMapSettings &settings, const QgsMapHitTest::LayerFilterExpression &layerFilterExpression )
  : QgsTask( tr( "Updating Legend" ), QgsTask::Flag::CanCancel | QgsTask::Flag::CancelWithoutPrompt | QgsTask::Flag::Silent )
  , mSettings( settings )
  , mLayerFilterExpression( layerFilterExpression )
  , mOnlyExpressions( true )
{
  prepare();
}

QMap<QString, QSet<QString> > QgsMapHitTestTask::results() const
{
  return mResults;
}

///@cond PRIVATE
QMap<QString, QList<QString> > QgsMapHitTestTask::resultsPy() const
{
  QMap<QString, QList<QString> > res;
  for ( auto it = mResults.begin(); it != mResults.end(); ++it )
  {
    res.insert( it.key(), qgis::setToList( it.value() ) );
  }
  return res;
}
///@endcond PRIVATE

void QgsMapHitTestTask::prepare()
{
  const QList< QgsMapLayer * > layers = mSettings.layers( true );
  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
    if ( !vl || !vl->renderer() )
      continue;

    QgsMapLayerStyleOverride styleOverride( vl );
    if ( mSettings.layerStyleOverrides().contains( vl->id() ) )
      styleOverride.setOverrideStyle( mSettings.layerStyleOverrides().value( vl->id() ) );

    if ( !mOnlyExpressions )
    {
      if ( !vl->isInScaleRange( mSettings.scale() ) )
      {
        continue;
      }
    }

    PreparedLayerData layerData;
    layerData.source = std::make_unique< QgsVectorLayerFeatureSource >( vl );
    layerData.layerId = vl->id();
    layerData.crs = vl->crs();
    layerData.fields = vl->fields();
    layerData.renderer.reset( vl->renderer()->clone() );
    layerData.transform = mSettings.layerTransform( vl );
    layerData.extent = mSettings.outputExtentToLayerExtent( vl, mSettings.visibleExtent() );
    layerData.layerScope.reset( QgsExpressionContextUtils::layerScope( vl ) );

    mPreparedData.emplace_back( std::move( layerData ) );
  }
}

void QgsMapHitTestTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();

  QgsTask::cancel();
}

bool QgsMapHitTestTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();
  connect( mFeedback.get(), &QgsFeedback::progressChanged, this, &QgsTask::progressChanged );

  std::unique_ptr< QgsMapHitTest > hitTest;
  if ( !mOnlyExpressions )
    hitTest = std::make_unique< QgsMapHitTest >( mSettings, mPolygon, mLayerFilterExpression );
  else
    hitTest = std::make_unique< QgsMapHitTest >( mSettings, mLayerFilterExpression );

  // TODO: do we need this temp image?
  QImage tmpImage( mSettings.outputSize(), mSettings.outputImageFormat() );
  tmpImage.setDotsPerMeterX( static_cast< int >( std::round( mSettings.outputDpi() * 25.4 ) ) );
  tmpImage.setDotsPerMeterY( static_cast< int >( std::round( mSettings.outputDpi() * 25.4 ) ) );
  QPainter painter( &tmpImage );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mSettings );
  context.setPainter( &painter ); // we are not going to draw anything, but we still need a working painter

  std::size_t layerIdx = 0;
  const std::size_t totalCount = mPreparedData.size();
  for ( auto &layerData : mPreparedData )
  {
    mFeedback->setProgress( static_cast< double >( layerIdx ) / static_cast< double >( totalCount ) * 100.0 );
    if ( mFeedback->isCanceled() )
      break;

    QgsMapHitTest::SymbolSet &usedSymbols = hitTest->mHitTest[layerData.layerId];
    QgsMapHitTest::SymbolSet &usedSymbolsRuleKey = hitTest->mHitTestRuleKey[layerData.layerId];

    context.setCoordinateTransform( layerData.transform );
    context.setExtent( layerData.extent );

    QgsExpressionContextScope *layerScope = layerData.layerScope.release();
    QgsExpressionContextScopePopper scopePopper( context.expressionContext(), layerScope );

    hitTest->runHitTestFeatureSource( layerData.source.get(),
                                      layerData.layerId,
                                      layerData.crs,
                                      layerData.fields,
                                      layerData.renderer.get(),
                                      usedSymbols,
                                      usedSymbolsRuleKey,
                                      context,
                                      mFeedback.get() );
    layerIdx++;
  }

  mResults = hitTest->mHitTestRuleKey;

  mFeedback.reset();

  return true;
}

