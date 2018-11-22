/***************************************************************************
         qgsvectorfieldsymbollayer.cpp
         -----------------------------
  begin                : Octorer 25, 2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorfieldsymbollayer.h"
#include "qgsvectorlayer.h"
#include "qgsunittypes.h"
#include "qgssymbollayerutils.h"

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer()
{
  setSubSymbol( new QgsLineSymbol() );
}

void QgsVectorFieldSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mDistanceUnit = unit;
}

QgsUnitTypes::RenderUnit QgsVectorFieldSymbolLayer::outputUnit() const
{
  if ( QgsMarkerSymbolLayer::outputUnit() == mDistanceUnit )
  {
    return mDistanceUnit;
  }
  return QgsUnitTypes::RenderUnknownUnit;
}

void QgsVectorFieldSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
  mDistanceMapUnitScale = scale;
}

QgsMapUnitScale QgsVectorFieldSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayer::mapUnitScale() == mDistanceMapUnitScale )
  {
    return mDistanceMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsVectorFieldSymbolLayer::create( const QgsStringMap &properties )
{
  QgsVectorFieldSymbolLayer *symbolLayer = new QgsVectorFieldSymbolLayer();
  if ( properties.contains( QStringLiteral( "x_attribute" ) ) )
  {
    symbolLayer->setXAttribute( properties[QStringLiteral( "x_attribute" )] );
  }
  if ( properties.contains( QStringLiteral( "y_attribute" ) ) )
  {
    symbolLayer->setYAttribute( properties[QStringLiteral( "y_attribute" )] );
  }
  if ( properties.contains( QStringLiteral( "distance_unit" ) ) )
  {
    symbolLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    symbolLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "scale" ) ) )
  {
    symbolLayer->setScale( properties[QStringLiteral( "scale" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "vector_field_type" ) ) )
  {
    symbolLayer->setVectorFieldType( static_cast< VectorFieldType >( properties[QStringLiteral( "vector_field_type" )].toInt() ) );
  }
  if ( properties.contains( QStringLiteral( "angle_orientation" ) ) )
  {
    symbolLayer->setAngleOrientation( static_cast< AngleOrientation >( properties[QStringLiteral( "angle_orientation" )].toInt() ) );
  }
  if ( properties.contains( QStringLiteral( "angle_units" ) ) )
  {
    symbolLayer->setAngleUnits( static_cast< AngleUnits >( properties[QStringLiteral( "angle_units" )].toInt() ) );
  }
  if ( properties.contains( QStringLiteral( "size" ) ) )
  {
    symbolLayer->setSize( properties[QStringLiteral( "size" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "size_unit" ) ) )
  {
    symbolLayer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "size_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "size_map_unit_scale" ) ) )
  {
    symbolLayer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "size_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    symbolLayer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  return symbolLayer;
}

bool QgsVectorFieldSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol->type() == QgsSymbol::Line )
  {
    mLineSymbol.reset( static_cast<QgsLineSymbol *>( symbol ) );
    return true;
  }
  return false;
}

void QgsVectorFieldSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  if ( !mLineSymbol )
  {
    return;
  }

  const QgsRenderContext &ctx = context.renderContext();

  const QgsFeature *f = context.feature();
  if ( !f )
  {
    //preview
    QPolygonF line;
    line << QPointF( 0, 50 );
    line << QPointF( 100, 50 );
    mLineSymbol->renderPolyline( line, nullptr, context.renderContext() );
  }

  double xComponent = 0;
  double yComponent = 0;

  double xVal = 0;
  if ( f && mXIndex != -1 )
  {
    xVal = f->attribute( mXIndex ).toDouble();
  }
  double yVal = 0;
  if ( f && mYIndex != -1 )
  {
    yVal = f->attribute( mYIndex ).toDouble();
  }

  switch ( mVectorFieldType )
  {
    case Cartesian:
      xComponent = ctx.convertToPainterUnits( xVal, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = ctx.convertToPainterUnits( yVal, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Polar:
      convertPolarToCartesian( xVal, yVal, xComponent, yComponent );
      xComponent = ctx.convertToPainterUnits( xComponent, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = ctx.convertToPainterUnits( yComponent, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Height:
      xComponent = 0;
      yComponent = ctx.convertToPainterUnits( yVal, mDistanceUnit, mDistanceMapUnitScale );
      break;
    default:
      break;
  }

  xComponent *= mScale;
  yComponent *= mScale;

  QPolygonF line;
  line << point;
  line << QPointF( point.x() + xComponent, point.y() - yComponent );
  mLineSymbol->renderPolyline( line, f, context.renderContext() );
}

void QgsVectorFieldSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->startRender( context.renderContext(), context.fields() );
  }

  QgsFields fields = context.fields();
  if ( !fields.isEmpty() )
  {
    mXIndex = fields.lookupField( mXAttribute );
    mYIndex = fields.lookupField( mYAttribute );
  }
  else
  {
    mXIndex = -1;
    mYIndex = -1;
  }
}

void QgsVectorFieldSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->stopRender( context.renderContext() );
  }
}

QgsVectorFieldSymbolLayer *QgsVectorFieldSymbolLayer::clone() const
{
  QgsSymbolLayer *clonedLayer = QgsVectorFieldSymbolLayer::create( properties() );
  if ( mLineSymbol )
  {
    clonedLayer->setSubSymbol( mLineSymbol->clone() );
  }
  return static_cast< QgsVectorFieldSymbolLayer * >( clonedLayer );
}

QgsStringMap QgsVectorFieldSymbolLayer::properties() const
{
  QgsStringMap properties;
  properties[QStringLiteral( "x_attribute" )] = mXAttribute;
  properties[QStringLiteral( "y_attribute" )] = mYAttribute;
  properties[QStringLiteral( "distance_unit" )] = QgsUnitTypes::encodeUnit( mDistanceUnit );
  properties[QStringLiteral( "distance_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceMapUnitScale );
  properties[QStringLiteral( "scale" )] = QString::number( mScale );
  properties[QStringLiteral( "vector_field_type" )] = QString::number( mVectorFieldType );
  properties[QStringLiteral( "angle_orientation" )] = QString::number( mAngleOrientation );
  properties[QStringLiteral( "angle_units" )] = QString::number( mAngleUnits );
  properties[QStringLiteral( "size" )] = QString::number( mSize );
  properties[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  properties[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  properties[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  properties[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  properties[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  return properties;
}

void QgsVectorFieldSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  element.appendChild( doc.createComment( QStringLiteral( "VectorField not implemented yet..." ) ) );
  mLineSymbol->toSld( doc, element, props );
}

QgsSymbolLayer *QgsVectorFieldSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element );
  return nullptr;
}

void QgsVectorFieldSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext &context, QSize size )
{
  if ( mLineSymbol )
  {
    mLineSymbol->drawPreviewIcon( context.renderContext().painter(), size );
  }
}

QSet<QString> QgsVectorFieldSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsMarkerSymbolLayer::usedAttributes( context );
  if ( !mXAttribute.isEmpty() )
  {
    attributes.insert( mXAttribute );
  }
  if ( !mYAttribute.isEmpty() )
  {
    attributes.insert( mYAttribute );
  }
  if ( mLineSymbol )
  {
    attributes.unite( mLineSymbol->usedAttributes( context ) );
  }
  return attributes;
}

void QgsVectorFieldSymbolLayer::convertPolarToCartesian( double length, double angle, double &x, double &y ) const
{
  //convert angle to degree and to north orientation
  if ( mAngleOrientation == CounterclockwiseFromEast )
  {
    if ( angle <= 90 )
    {
      angle = 90 - angle;
    }
    else
    {
      angle = 360 - angle + 90;
    }
  }

  if ( mAngleUnits == Degrees )
  {
    angle = angle * M_PI / 180.0;
  }

  x = length * std::sin( angle );
  y = length * std::cos( angle );
}

void QgsVectorFieldSymbolLayer::setColor( const QColor &color )
{
  if ( mLineSymbol )
    mLineSymbol->setColor( color );

  mColor = color;
}

QColor QgsVectorFieldSymbolLayer::color() const
{
  return mLineSymbol ? mLineSymbol->color() : mColor;
}


