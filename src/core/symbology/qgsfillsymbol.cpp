/***************************************************************************
 qgsfillsymbol.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgspainteffect.h"

QgsFillSymbol *QgsFillSymbol::createSimple( const QVariantMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleFillSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsFillSymbol( layers );
}


QgsFillSymbol::QgsFillSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Qgis::SymbolType::Fill, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleFillSymbolLayer() );
}

void QgsFillSymbol::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  const double opacity = dataDefinedProperties().hasActiveProperties() ? dataDefinedProperties().valueAsDouble( QgsSymbol::PropertyOpacity, context.expressionContext(), mOpacity * 100 ) * 0.01
                         : mOpacity;

  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, opacity, selected, mRenderHints, f );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::PolygonGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
    {
      if ( symbolLayer->type() == Qgis::SymbolType::Fill || symbolLayer->type() == Qgis::SymbolType::Line )
        renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
      else
        renderUsingLayer( symbolLayer, symbolContext, QgsWkbTypes::PolygonGeometry, &points, rings );
    }
    return;
  }

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *symbolLayer : constMLayers )
  {
    if ( context.renderingStopped() )
      break;

    if ( !symbolLayer->enabled() || !context.isSymbolLayerEnabled( symbolLayer ) )
      continue;

    if ( symbolLayer->type() == Qgis::SymbolType::Fill || symbolLayer->type() == Qgis::SymbolType::Line )
      renderPolygonUsingLayer( symbolLayer, points, rings, symbolContext );
    else
      renderUsingLayer( symbolLayer, symbolContext, QgsWkbTypes::PolygonGeometry, &points, rings );
  }
}

void QgsFillSymbol::renderPolygonUsingLayer( QgsSymbolLayer *layer, const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context ) const
{
  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  const Qgis::SymbolType layertype = layer->type();

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    const QRectF bounds = polygonBounds( points, rings );
    QVector<QPolygonF> *translatedRings = translateRings( rings, -bounds.left(), -bounds.top() );

    QgsEffectPainter p( context.renderContext() );
    p->translate( bounds.topLeft() );
    p.setEffect( effect );
    if ( layertype == Qgis::SymbolType::Fill )
    {
      ( static_cast<QgsFillSymbolLayer *>( layer ) )->renderPolygon( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    else if ( layertype == Qgis::SymbolType::Line )
    {
      ( static_cast<QgsLineSymbolLayer *>( layer ) )->renderPolygonStroke( points.translated( -bounds.topLeft() ), translatedRings, context );
    }
    delete translatedRings;
  }
  else
  {
    if ( layertype == Qgis::SymbolType::Fill )
    {
      ( static_cast<QgsFillSymbolLayer *>( layer ) )->renderPolygon( points, rings, context );
    }
    else if ( layertype == Qgis::SymbolType::Line )
    {
      ( static_cast<QgsLineSymbolLayer *>( layer ) )->renderPolygonStroke( points, rings, context );
    }
  }
}

QRectF QgsFillSymbol::polygonBounds( const QPolygonF &points, const QVector<QPolygonF> *rings ) const
{
  QRectF bounds = points.boundingRect();
  if ( rings )
  {
    for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
    {
      bounds = bounds.united( ( *it ).boundingRect() );
    }
  }
  return bounds;
}

QVector<QPolygonF> *QgsFillSymbol::translateRings( const QVector<QPolygonF> *rings, double dx, double dy ) const
{
  if ( !rings )
    return nullptr;

  QVector<QPolygonF> *translatedRings = new QVector<QPolygonF>;
  translatedRings->reserve( rings->size() );
  for ( auto it = rings->constBegin(); it != rings->constEnd(); ++it )
  {
    translatedRings->append( ( *it ).translated( dx, dy ) );
  }
  return translatedRings;
}

QgsFillSymbol *QgsFillSymbol::clone() const
{
  QgsFillSymbol *cloneSymbol = new QgsFillSymbol( cloneLayers() );
  cloneSymbol->setOpacity( mOpacity );
  Q_NOWARN_DEPRECATED_PUSH
  cloneSymbol->setLayer( mLayer );
  Q_NOWARN_DEPRECATED_POP
  cloneSymbol->setClipFeaturesToExtent( mClipFeaturesToExtent );
  cloneSymbol->setForceRHR( mForceRHR );
  cloneSymbol->setDataDefinedProperties( dataDefinedProperties() );
  cloneSymbol->setFlags( mSymbolFlags );
  cloneSymbol->setAnimationSettings( mAnimationSettings );
  return cloneSymbol;
}

void QgsFillSymbol::setAngle( double angle ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Fill )
      continue;

    QgsFillSymbolLayer *fillLayer = static_cast<QgsFillSymbolLayer *>( layer );

    if ( fillLayer )
      fillLayer->setAngle( angle );
  }
}


