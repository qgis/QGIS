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
#include "qgsgeometrypaintdevice.h"
#include "qgspainting.h"
#include "qgsfillsymbol.h"

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

  QgsSymbolRenderContext symbolContext( context, Qgis::RenderUnit::Unknown, opacity, selected, renderHints(), f );
  symbolContext.setGeometryPartCount( symbolRenderContext()->geometryPartCount() );
  symbolContext.setGeometryPartNum( symbolRenderContext()->geometryPartNum() );

  // If we're drawing using symbol levels, we only draw buffers for the bottom most level
  const bool usingBuffer = ( layerIdx == -1 || layerIdx == 0 ) && mBufferSettings && mBufferSettings->enabled() && mBufferSettings->fillSymbol()
                           && !symbolRenderContext()->renderHints().testFlag( Qgis::SymbolRenderHint::ExcludeSymbolBuffers );

  if ( layerIdx != -1 && !usingBuffer )
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

  // handle symbol buffers -- we do this by deferring the rendering of the symbol and redirecting
  // to QPictures, and then using the actual rendered shape from the QPictures to determine the buffer shape.
  QPainter *originalTargetPainter = nullptr;
  // this is an array, we need to separate out the symbol layers if we're drawing only one symbol level
  std::vector< QPicture > picturesForDeferredRendering;
  std::unique_ptr< QPainter > deferredRenderingPainter;
  if ( usingBuffer )
  {
    originalTargetPainter = context.painter();
    picturesForDeferredRendering.emplace_back( QPicture() );
    deferredRenderingPainter = std::make_unique< QPainter >( &picturesForDeferredRendering.front() );
    context.setPainter( deferredRenderingPainter.get() );
  }

  int symbolLayerIndex = -1;
  for ( QgsSymbolLayer *symbolLayer : std::as_const( mLayers ) )
  {
    symbolLayerIndex++;
    if ( context.renderingStopped() )
      break;

    if ( deferredRenderingPainter && layerIdx != -1 && symbolLayerIndex != 0 )
    {
      // if we're using deferred rendering along with symbol level drawing, we
      // start a new picture for each symbol layer drawn
      deferredRenderingPainter->end();
      picturesForDeferredRendering.emplace_back( QPicture() );
      deferredRenderingPainter->begin( &picturesForDeferredRendering.back() );
    }

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

  // if required, render the calculated buffer below the symbol
  if ( usingBuffer )
  {
    deferredRenderingPainter->end();
    deferredRenderingPainter.reset();

    QgsGeometryPaintDevice geometryPaintDevice;
    QPainter geometryPainter( &geometryPaintDevice );
    // render all the symbol layers onto the geometry painter, so we can calculate a single
    // buffer for ALL of them
    for ( const auto &deferredPicture : picturesForDeferredRendering )
    {
      QgsPainting::drawPicture( &geometryPainter, QPointF( 0, 0 ), deferredPicture );
    }
    geometryPainter.end();

    // retrieve the shape of the rendered symbol
    const QgsGeometry renderedShape( geometryPaintDevice.geometry().clone() );

    context.setPainter( originalTargetPainter );

    // next, buffer out the rendered shape, and draw!
    const double bufferSize = context.convertToPainterUnits( mBufferSettings->size(), mBufferSettings->sizeUnit(), mBufferSettings->sizeMapUnitScale() );
    Qgis::JoinStyle joinStyle = Qgis::JoinStyle::Round;
    switch ( mBufferSettings->joinStyle() )
    {
      case Qt::MiterJoin:
      case Qt::SvgMiterJoin:
        joinStyle = Qgis::JoinStyle::Miter;
        break;
      case Qt::BevelJoin:
        joinStyle = Qgis::JoinStyle::Bevel;
        break;
      case Qt::RoundJoin:
        joinStyle = Qgis::JoinStyle::Round;
        break;

      case Qt::MPenJoinStyle:
        break;
    }

    const QgsGeometry bufferedGeometry = renderedShape.buffer( bufferSize, 8, Qgis::EndCapStyle::Round, joinStyle, 2 );
    const QList<QList<QPolygonF> > polygons = QgsSymbolLayerUtils::toQPolygonF( bufferedGeometry, Qgis::SymbolType::Fill );
    for ( const QList< QPolygonF > &polygon : polygons )
    {
      QVector< QPolygonF > rings;
      for ( int i = 1; i < polygon.size(); ++i )
        rings << polygon.at( i );
      mBufferSettings->fillSymbol()->renderPolygon( polygon.value( 0 ), &rings, nullptr, context );
    }

    // finally, draw the actual rendered symbol on top. If symbol levels are at play then this will ONLY
    // be the target symbol level, not all of them.
    QgsPainting::drawPicture( context.painter(), QPointF( 0, 0 ), picturesForDeferredRendering.front() );
  }
}

QRectF QgsMarkerSymbol::bounds( QPointF point, QgsRenderContext &context, const QgsFeature &feature ) const
{
  QgsSymbolRenderContext symbolContext( context, Qgis::RenderUnit::Unknown, mOpacity, false, renderHints(), &feature, feature.fields() );

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
