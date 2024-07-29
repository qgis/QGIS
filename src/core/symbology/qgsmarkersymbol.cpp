/***************************************************************************
 qgsmarkersymbol.cpp
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

#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgspainteffect.h"

QgsMarkerSymbol *QgsMarkerSymbol::createSimple( const QVariantMap &properties )
{
  QgsSymbolLayer *sl = QgsSimpleMarkerSymbolLayer::create( properties );
  if ( !sl )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( sl );
  return new QgsMarkerSymbol( layers );
}

QgsMarkerSymbol::QgsMarkerSymbol( const QgsSymbolLayerList &layers )
  : QgsSymbol( Qgis::SymbolType::Marker, layers )
{
  if ( mLayers.isEmpty() )
    mLayers.append( new QgsSimpleMarkerSymbolLayer() );
}

void QgsMarkerSymbol::setAngle( double symbolAngle ) const
{
  double origAngle = angle();
  double angleDiff = symbolAngle - origAngle;
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer );
    if ( markerLayer )
      markerLayer->setAngle( markerLayer->angle() + angleDiff );
  }
}

double QgsMarkerSymbol::angle() const
{
  for ( QgsSymbolLayer *layer : std::as_const( mLayers ) )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    return markerLayer->angle();
  }
  return 0;
}

void QgsMarkerSymbol::setLineAngle( double lineAng ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setLineAngle( lineAng );
  }
}

void QgsMarkerSymbol::setDataDefinedAngle( const QgsProperty &property )
{
  const double symbolRotation = angle();


  for ( QgsSymbolLayer *layer : std::as_const( mLayers ) )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( !property )
    {
      layer->setDataDefinedProperty( QgsSymbolLayer::Property::Angle, QgsProperty() );
    }
    else
    {
      if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
      {
        layer->setDataDefinedProperty( QgsSymbolLayer::Property::Angle, property );
      }
      else
      {
        QgsProperty rotatedDD = QgsSymbolLayerUtils::rotateWholeSymbol( markerLayer->angle() - symbolRotation, property );
        layer->setDataDefinedProperty( QgsSymbolLayer::Property::Angle, rotatedDD );
      }
    }
  }
}

QgsProperty QgsMarkerSymbol::dataDefinedAngle() const
{
  const double symbolRotation = angle();
  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  const auto layers = mLayers;
  for ( QgsSymbolLayer *layer : layers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) && markerLayer->dataDefinedProperties().isActive( QgsSymbolLayer::Property::Angle ) )
    {
      symbolDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::Property::Angle );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layer's angle expressions match the "en masse" pattern
  for ( QgsSymbolLayer *layer : layers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

    QgsProperty layerAngleDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::Property::Angle );

    if ( qgsDoubleNear( markerLayer->angle(), symbolRotation ) )
    {
      if ( !layerAngleDD || layerAngleDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      QgsProperty rotatedDD( QgsSymbolLayerUtils::rotateWholeSymbol( markerLayer->angle() - symbolRotation, symbolDD ) );
      if ( !layerAngleDD || layerAngleDD != rotatedDD )
        return QgsProperty();
    }
  }
  return symbolDD;
}


void QgsMarkerSymbol::setSize( double s ) const
{
  double origSize = size();

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer );
    if ( markerLayer )
    {
      if ( qgsDoubleNear( markerLayer->size(), origSize ) )
      {
        markerLayer->setSize( s );
      }
      else if ( !qgsDoubleNear( origSize, 0.0 ) )
      {
        // proportionally scale size
        markerLayer->setSize( markerLayer->size() * s / origSize );
      }
      // also scale offset to maintain relative position
      if ( !qgsDoubleNear( origSize, 0.0 ) && ( !qgsDoubleNear( markerLayer->offset().x(), 0.0 ) || !qgsDoubleNear( markerLayer->offset().y(), 0.0 ) ) )
        markerLayer->setOffset( QPointF( markerLayer->offset().x() * s / origSize,
                                         markerLayer->offset().y() * s / origSize ) );
    }
    else
    {
      QgsGeometryGeneratorSymbolLayer *geomGeneratorLayer = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( layer );
      if ( geomGeneratorLayer && geomGeneratorLayer->symbolType() == Qgis::SymbolType::Marker )
      {
        QgsMarkerSymbol *markerSymbol = qgis::down_cast<QgsMarkerSymbol *>( geomGeneratorLayer->subSymbol() );
        if ( qgsDoubleNear( markerSymbol->size(), origSize ) )
        {
          markerSymbol->setSize( s );
        }
        else if ( !qgsDoubleNear( origSize, 0.0 ) )
        {
          // proportionally scale the width
          markerSymbol->setSize( markerSymbol->size() * s / origSize );
        }
      }
    }
  }
}

double QgsMarkerSymbol::size() const
{
  // return size of the largest symbol
  double maxSize = 0;
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    const QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer );
    if ( markerLayer )
    {
      const double lsize = markerLayer->size();
      if ( lsize > maxSize )
        maxSize = lsize;
    }
    else
    {
      QgsGeometryGeneratorSymbolLayer *geomGeneratorLayer = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( layer );
      if ( geomGeneratorLayer && geomGeneratorLayer->symbolType() == Qgis::SymbolType::Marker )
      {
        QgsMarkerSymbol *markerSymbol = qgis::down_cast<QgsMarkerSymbol *>( geomGeneratorLayer->subSymbol() );
        const double lsize = markerSymbol->size();
        if ( lsize > maxSize )
          maxSize = lsize;
      }
    }
  }
  return maxSize;
}

double QgsMarkerSymbol::size( const QgsRenderContext &context ) const
{
  // return size of the largest symbol
  double maxSize = 0;
  for ( QgsSymbolLayer *layer : mLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    const double layerSize = context.convertToPainterUnits( markerLayer->size(), markerLayer->sizeUnit(), markerLayer->sizeMapUnitScale() );
    maxSize = std::max( maxSize, layerSize );
  }
  return maxSize;
}

void QgsMarkerSymbol::setSizeUnit( Qgis::RenderUnit unit ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setSizeUnit( unit );
  }
}

Qgis::RenderUnit QgsMarkerSymbol::sizeUnit() const
{
  bool first = true;
  Qgis::RenderUnit unit = Qgis::RenderUnit::Unknown;

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

    if ( first )
      unit = markerLayer->sizeUnit();
    else
    {
      if ( unit != markerLayer->sizeUnit() )
        return Qgis::RenderUnit::Unknown;
    }

    first = false;
  }
  return unit;
}

void QgsMarkerSymbol::setSizeMapUnitScale( const QgsMapUnitScale &scale ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setSizeMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsMarkerSymbol::sizeMapUnitScale() const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;

    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    return markerLayer->sizeMapUnitScale();
  }
  return QgsMapUnitScale();
}

void QgsMarkerSymbol::setDataDefinedSize( const QgsProperty &property ) const
{
  const double symbolSize = size();

  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );

    if ( !property )
    {
      markerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty() );
      markerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Offset, QgsProperty() );
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) || qgsDoubleNear( markerLayer->size(), symbolSize ) )
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, property );
      }
      else
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsSymbolLayerUtils::scaleWholeSymbol( markerLayer->size() / symbolSize, property ) );
      }

      if ( !qgsDoubleNear( markerLayer->offset().x(), 0.0 ) || !qgsDoubleNear( markerLayer->offset().y(), 0.0 ) )
      {
        markerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Offset, QgsSymbolLayerUtils::scaleWholeSymbol(
                                               markerLayer->offset().x() / symbolSize,
                                               markerLayer->offset().y() / symbolSize, property ) );
      }
    }
  }
}

QgsProperty QgsMarkerSymbol::dataDefinedSize() const
{
  const double symbolSize = size();

  QgsProperty symbolDD;

  // find the base of the "en masse" pattern
  const auto layers = mLayers;
  for ( QgsSymbolLayer *layer : layers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) && markerLayer->dataDefinedProperties().isActive( QgsSymbolLayer::Property::Size ) )
    {
      symbolDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::Property::Size );
      break;
    }
  }

  if ( !symbolDD )
    return QgsProperty();

  // check that all layers size expressions match the "en masse" pattern
  for ( QgsSymbolLayer *layer : layers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );

    QgsProperty layerSizeDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::Property::Size );
    QgsProperty layerOffsetDD = markerLayer->dataDefinedProperties().property( QgsSymbolLayer::Property::Offset );

    if ( qgsDoubleNear( markerLayer->size(), symbolSize ) )
    {
      if ( !layerSizeDD || layerSizeDD != symbolDD )
        return QgsProperty();
    }
    else
    {
      if ( qgsDoubleNear( symbolSize, 0.0 ) )
        return QgsProperty();

      QgsProperty scaledDD( QgsSymbolLayerUtils::scaleWholeSymbol( markerLayer->size() / symbolSize, symbolDD ) );
      if ( !layerSizeDD || layerSizeDD != scaledDD )
        return QgsProperty();
    }

    QgsProperty scaledOffsetDD( QgsSymbolLayerUtils::scaleWholeSymbol( markerLayer->offset().x() / symbolSize, markerLayer->offset().y() / symbolSize, symbolDD ) );
    if ( layerOffsetDD && layerOffsetDD != scaledOffsetDD )
      return QgsProperty();
  }

  return symbolDD;
}

void QgsMarkerSymbol::setScaleMethod( Qgis::ScaleMethod scaleMethod ) const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    markerLayer->setScaleMethod( scaleMethod );
  }
}

Qgis::ScaleMethod QgsMarkerSymbol::scaleMethod() const
{
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() != Qgis::SymbolType::Marker )
      continue;
    const QgsMarkerSymbolLayer *markerLayer = static_cast<const QgsMarkerSymbolLayer *>( layer );
    // return scale method of the first symbol layer
    return markerLayer->scaleMethod();
  }

  return DEFAULT_SCALE_METHOD;
}

void QgsMarkerSymbol::renderPointUsingLayer( QgsMarkerSymbolLayer *layer, QPointF point, QgsSymbolRenderContext &context )
{
  static QPointF nullPoint( 0, 0 );

  if ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::Property::LayerEnabled, context.renderContext().expressionContext(), true ) )
    return;

  QgsPaintEffect *effect = layer->paintEffect();
  if ( effect && effect->enabled() )
  {
    QgsEffectPainter p( context.renderContext() );
    p->translate( point );
    p.setEffect( effect );
    layer->renderPoint( nullPoint, context );
  }
  else
  {
    layer->renderPoint( point, context );
  }
}

void QgsMarkerSymbol::renderPoint( QPointF point, const QgsFeature *f, QgsRenderContext &context, int layerIdx, bool selected )
{
  const double opacity = dataDefinedProperties().hasActiveProperties() ? dataDefinedProperties().valueAsDouble( QgsSymbol::Property::Opacity, context.expressionContext(), mOpacity * 100 ) * 0.01
                         : mOpacity;

  QgsSymbolRenderContext symbolContext( context, Qgis::RenderUnit::Unknown, opacity, selected, mRenderHints, f );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  if ( layerIdx != -1 )
  {
    QgsSymbolLayer *symbolLayer = mLayers.value( layerIdx );
    if ( symbolLayer && symbolLayer->enabled() && context.isSymbolLayerEnabled( symbolLayer ) )
    {
      if ( symbolLayer->type() == Qgis::SymbolType::Marker )
      {
        QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( symbolLayer );
        renderPointUsingLayer( markerLayer, point, symbolContext );
      }
      else
      {
        QPolygonF points;
        points.append( point );
        renderUsingLayer( symbolLayer, symbolContext, Qgis::GeometryType::Point, &points );
      }
    }
    return;
  }


  for ( QgsSymbolLayer *symbolLayer : std::as_const( mLayers ) )
  {
    if ( context.renderingStopped() )
      break;

    if ( !symbolLayer->enabled() || !context.isSymbolLayerEnabled( symbolLayer ) )
      continue;

    if ( symbolLayer->type() == Qgis::SymbolType::Marker )
    {
      QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( symbolLayer );
      renderPointUsingLayer( markerLayer, point, symbolContext );
    }
    else
    {
      QPolygonF points;
      points.append( point );
      renderUsingLayer( symbolLayer, symbolContext, Qgis::GeometryType::Point, &points );
    }
  }
}

QRectF QgsMarkerSymbol::bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature ) const
{
  QgsSymbolRenderContext symbolContext( context, Qgis::RenderUnit::Unknown, mOpacity, false, mRenderHints, &feature, feature.fields() );

  QRectF bound;
  const auto constMLayers = mLayers;
  for ( QgsSymbolLayer *layer : constMLayers )
  {
    if ( layer->type() == Qgis::SymbolType::Marker )
    {
      if ( !layer->enabled()
           || ( layer->dataDefinedProperties().hasActiveProperties() && !layer->dataDefinedProperties().valueAsBool( QgsSymbolLayer::Property::LayerEnabled, context.expressionContext(), true ) ) )
        continue;

      QgsMarkerSymbolLayer *symbolLayer = static_cast< QgsMarkerSymbolLayer * >( layer );
      if ( bound.isNull() )
        bound = symbolLayer->bounds( point, symbolContext );
      else
        bound = bound.united( symbolLayer->bounds( point, symbolContext ) );
    }
  }
  return bound;
}

QgsMarkerSymbol *QgsMarkerSymbol::clone() const
{
  QgsMarkerSymbol *cloneSymbol = new QgsMarkerSymbol( cloneLayers() );
  cloneSymbol->copyCommonProperties( this );
  return cloneSymbol;
}
