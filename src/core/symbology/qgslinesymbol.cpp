/***************************************************************************
 qgslinesymbol.cpp
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

#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgspainteffect.h"

QgsLineSymbol *QgsLineSymbol::createSimple( const QVariantMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleLineSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsLineSymbol( layers );
}

QgsLineSymbol::QgsLineSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Qgis::SymbolType::Line, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleLineSymbolLayer() );
}

void QgsLineSymbol::setWidth( double w ) const
{
  const double origWidth = width();

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( layer );
    if ( lineLayer )
    {
      if ( qgsDoubleNear( lineLayer->width(), origWidth ) )
      {
        lineLayer->setWidth( w );
      }
      else if ( !qgsDoubleNear( origWidth, 0.0 ) )
      {
        // proportionally scale the width
        lineLayer->setWidth( lineLayer->width() * w / origWidth );
      }
      // also scale offset to maintain relative position
      if ( !qgsDoubleNear( origWidth, 0.0 ) && !qgsDoubleNear( lineLayer->offset(), 0.0 ) )
        lineLayer->setOffset( lineLayer->offset() * w / origWidth );
    }
    else
    {
      QgsGeometryGeneratorSymbolLayer *geomGeneratorLayer = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( layer );
      if ( geomGeneratorLayer && geomGeneratorLayer->symbolType() == Qgis::SymbolType::Line )
      {
        QgsLineSymbol *lineSymbol = qgis::down_cast<QgsLineSymbol *>( geomGeneratorLayer->subSymbol() );
        if ( qgsDoubleNear( lineSymbol->width(), origWidth ) )
        {
          lineSymbol->setWidth( w );
        }
        else if ( !qgsDoubleNear( origWidth, 0.0 ) )
        {
          // proportionally scale the width
          lineSymbol->setWidth( lineSymbol->width() * w / origWidth );
        }
      }
    }
  }
}

void QgsLineSymbol::setWidthUnit( QgsUnitTypes::RenderUnit unit ) const
{
  const auto constLLayers = mLayers;
  for ( QgsSymbolLayer *layer : constLLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Line )
      continue;

    QgsLineSymbolLayer *lineLayer = static_cast<QgsLineSymbolLayer *>( layer );
    lineLayer->setWidthUnit( unit );
  }
}

double QgsLineSymbol::width() const
{
  double maxWidth = 0;
  if ( mLayers.isEmpty() )
    return maxWidth;

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *symbolLayer : constMLayers )
  {
    const QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( symbolLayer );
    if ( lineLayer )
    {
      const double width = lineLayer->width();
      if ( width > maxWidth )
        maxWidth = width;
    }
    else
    {
      QgsGeometryGeneratorSymbolLayer *geomGeneratorLayer = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( symbolLayer );
      if ( geomGeneratorLayer && geomGeneratorLayer->symbolType() == Qgis::SymbolType::Line )
      {
        QgsLineSymbol *lineSymbol = qgis::down_cast<QgsLineSymbol *>( geomGeneratorLayer->subSymbol() );
        const double width = lineSymbol->width();
        if ( width > maxWidth )
          maxWidth = width;
      }
    }
  }
  return maxWidth;
}

double QgsLineSymbol::width( const QgsRenderContext &context ) const
{
  // return width of the largest symbol
  double maxWidth = 0;
  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Line )
      continue;
    const QgsLineSymbolLayer *lineLayer = static_cast<const QgsLineSymbolLayer *>( layer );
    const double layerWidth = lineLayer->width( context );
    maxWidth = std::max( maxWidth, layerWidth );
  }
  return maxWidth;
}

void QgsLineSymbol::setDataDefinedWidth( const QgsProperty &property ) const
{
  const double symbolWidth = width();

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    QgsLineSymbolLayer *lineLayer = dynamic_cast<QgsLineSymbolLayer *>( layer );

    if ( lineLayer )
    {
      if ( !property )
      {
        lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsProperty() );
        lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsProperty() );
      }
      else
      {
        if ( qgsDoubleNear( symbolWidth, 0.0 ) || qgsDoubleNear( lineLayer->width(), symbolWidth ) )
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, property );
        }
        else
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeWidth, QgsSymbolLayerUtils::scaleWholeSymbol( lineLayer->width() / symbolWidth, property ) );
        }

        if ( !qgsDoubleNear( lineLayer->offset(), 0.0 ) )
        {
          lineLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyOffset, QgsSymbolLayerUtils::scaleWholeSymbol( lineLayer->offset() / symbolWidth, property ) );
        }
      }
    }
  }
}

QgsProperty QgsLineSymbol::dataDefinedWidth() const
{
  const double symbolWidth = width();

  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  for ( QgsSymbolLayerList::const_iterator it = mLayers.begin(); it != mLayers.end(); ++it )
  {
    const QgsLineSymbolLayer *layer = dynamic_cast<const QgsLineSymbolLayer *>( *it );
    if ( layer && qgsDoubleNear( layer->width(), symbolWidth ) && layer->dataDefinedProperties().isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
    {
      symbolDD = layer->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layers width expressions match the "en masse" pattern
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Line )
      continue;
    const QgsLineSymbolLayer *lineLayer = static_cast<const QgsLineSymbolLayer *>( layer );

    const QgsProperty layerWidthDD = lineLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyStrokeWidth );
    const QgsProperty layerOffsetDD = lineLayer->dataDefinedProperties().property( QgsSymbolLayer::PropertyOffset );

    if ( qgsDoubleNear( lineLayer->width(), symbolWidth ) )
    {
      if ( !layerWidthDD || layerWidthDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      if ( qgsDoubleNear( symbolWidth, 0.0 ) )
        return QgsProperty();

      const QgsProperty scaledDD( QgsSymbolLayerUtils::scaleWholeSymbol( lineLayer->width() / symbolWidth, symbolDD ) );
      if ( !layerWidthDD || layerWidthDD != scaledDD )
        return QgsProperty();
    }

    const QgsProperty scaledOffsetDD( QgsSymbolLayerUtils::scaleWholeSymbol( lineLayer->offset() / symbolWidth, symbolDD ) );
    if ( layerOffsetDD && layerOffsetDD != scaledOffsetDD )
      return QgsProperty();
  }

  return symbolDD;
}

void QgsLineSymbol::renderPolyline( const QPolygonF &points, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  const double opacity = dataDefinedProperties().hasActiveProperties() ? dataDefinedProperties().valueAsDouble( QgsSymbol::PropertyOpacity, context.expressionContext(), mOpacity * 100 ) * 0.01
                         : mOpacity;

  //save old painter
  QPainter *renderPainter = context.painter();
  QgsSymbolRenderContext symbolContext( context, QgsUnitTypes::RenderUnknownUnit, opacity, selected, mRenderHints, f );
  symbolContext.setOriginalGeometryType( QgsWkbTypes::LineGeometry );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
    {
      if ( symbolLayer->type() == Qgis::SymbolType::Line )
      {
        QgsLineSymbolLayer *lineLayer = static_cast<QgsLineSymbolLayer *>( symbolLayer );
        renderPolylineUsingLayer( lineLayer, points, symbolContext );
      }
      else
        renderUsingLayer( symbolLayer, symbolContext, QgsWkbTypes::LineGeometry, &points );
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

    if ( symbolLayer->type() == Qgis::SymbolType::Line )
    {
      QgsLineSymbolLayer *lineLayer = static_cast<QgsLineSymbolLayer *>( symbolLayer );
      renderPolylineUsingLayer( lineLayer, points, symbolContext );
    }
    else
    {
      renderUsingLayer( symbolLayer, symbolContext, QgsWkbTypes::LineGeometry, &points );
    }
  }

  context.setPainter( renderPainter );
}

void QgsLineSymbol::renderPolylineUsingLayer( QgsLineSymbolLayer *layer, const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::PropertyLayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext() );
    p->translate( points.boundingRect().topLeft() );
    p.setEffect( effect );
    layer->renderPolyline( points.translated( -points.boundingRect().topLeft() ), context );
  }
  else
  {
    layer->renderPolyline( points, context );
  }
}


QgsLineSymbol *QgsLineSymbol::clone() const
{
  QgsLineSymbol *cloneSymbol = new QgsLineSymbol( cloneLayers() );
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
