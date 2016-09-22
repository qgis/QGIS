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

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer()
    : mXAttribute( "" )
    , mYAttribute( "" )
    , mDistanceUnit( QgsUnitTypes::RenderMillimeters )
    , mScale( 1.0 )
    , mVectorFieldType( Cartesian )
    , mAngleOrientation( ClockwiseFromNorth )
    , mAngleUnits( Degrees )
    , mLineSymbol( nullptr )
    , mXIndex( -1 )
    , mYIndex( -1 )
{
  setSubSymbol( new QgsLineSymbol() );
}

QgsVectorFieldSymbolLayer::~QgsVectorFieldSymbolLayer()
{
  delete mLineSymbol;
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

QgsSymbolLayer* QgsVectorFieldSymbolLayer::create( const QgsStringMap& properties )
{
  QgsVectorFieldSymbolLayer* symbolLayer = new QgsVectorFieldSymbolLayer();
  if ( properties.contains( "x_attribute" ) )
  {
    symbolLayer->setXAttribute( properties["x_attribute"] );
  }
  if ( properties.contains( "y_attribute" ) )
  {
    symbolLayer->setYAttribute( properties["y_attribute"] );
  }
  if ( properties.contains( "distance_unit" ) )
  {
    symbolLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties["distance_unit"] ) );
  }
  if ( properties.contains( "distance_map_unit_scale" ) )
  {
    symbolLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties["distance_map_unit_scale"] ) );
  }
  if ( properties.contains( "scale" ) )
  {
    symbolLayer->setScale( properties["scale"].toDouble() );
  }
  if ( properties.contains( "vector_field_type" ) )
  {
    symbolLayer->setVectorFieldType( static_cast< VectorFieldType >( properties["vector_field_type"].toInt() ) );
  }
  if ( properties.contains( "angle_orientation" ) )
  {
    symbolLayer->setAngleOrientation( static_cast< AngleOrientation >( properties["angle_orientation"].toInt() ) );
  }
  if ( properties.contains( "angle_units" ) )
  {
    symbolLayer->setAngleUnits( static_cast< AngleUnits >( properties["angle_units"].toInt() ) );
  }
  if ( properties.contains( "size" ) )
  {
    symbolLayer->setSize( properties["size"].toDouble() );
  }
  if ( properties.contains( "size_unit" ) )
  {
    symbolLayer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties["size_unit"] ) );
  }
  if ( properties.contains( "size_map_unit_scale" ) )
  {
    symbolLayer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties["size_map_unit_scale"] ) );
  }
  if ( properties.contains( "offset" ) )
  {
    symbolLayer->setOffset( QgsSymbolLayerUtils::decodePoint( properties["offset"] ) );
  }
  if ( properties.contains( "offset_unit" ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties["offset_unit"] ) );
  }
  if ( properties.contains( "offset_map_unit_scale" ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties["offset_map_unit_scale"] ) );
  }
  return symbolLayer;
}

bool QgsVectorFieldSymbolLayer::setSubSymbol( QgsSymbol* symbol )
{
  if ( symbol->type() == QgsSymbol::Line )
  {
    delete mLineSymbol;
    mLineSymbol = static_cast<QgsLineSymbol*>( symbol );
    return true;
  }
  return false;
}

void QgsVectorFieldSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext& context )
{
  if ( !mLineSymbol )
  {
    return;
  }

  const QgsRenderContext& ctx = context.renderContext();

  const QgsFeature* f = context.feature();
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
      xComponent = QgsSymbolLayerUtils::convertToPainterUnits( ctx, xVal, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = QgsSymbolLayerUtils::convertToPainterUnits( ctx, yVal, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Polar:
      convertPolarToCartesian( xVal, yVal, xComponent, yComponent );
      xComponent = QgsSymbolLayerUtils::convertToPainterUnits( ctx, xComponent, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = QgsSymbolLayerUtils::convertToPainterUnits( ctx, yComponent, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Height:
      xComponent = 0;
      yComponent = QgsSymbolLayerUtils::convertToPainterUnits( ctx, yVal, mDistanceUnit, mDistanceMapUnitScale );
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

void QgsVectorFieldSymbolLayer::startRender( QgsSymbolRenderContext& context )
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

void QgsVectorFieldSymbolLayer::stopRender( QgsSymbolRenderContext& context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->stopRender( context.renderContext() );
  }
}

QgsVectorFieldSymbolLayer* QgsVectorFieldSymbolLayer::clone() const
{
  QgsSymbolLayer* clonedLayer = QgsVectorFieldSymbolLayer::create( properties() );
  if ( mLineSymbol )
  {
    clonedLayer->setSubSymbol( mLineSymbol->clone() );
  }
  return static_cast< QgsVectorFieldSymbolLayer* >( clonedLayer );
}

QgsStringMap QgsVectorFieldSymbolLayer::properties() const
{
  QgsStringMap properties;
  properties["x_attribute"] = mXAttribute;
  properties["y_attribute"] = mYAttribute;
  properties["distance_unit"] = QgsUnitTypes::encodeUnit( mDistanceUnit );
  properties["distance_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceMapUnitScale );
  properties["scale"] = QString::number( mScale );
  properties["vector_field_type"] = QString::number( mVectorFieldType );
  properties["angle_orientation"] = QString::number( mAngleOrientation );
  properties["angle_units"] = QString::number( mAngleUnits );
  properties["size"] = QString::number( mSize );
  properties["size_unit"] = QgsUnitTypes::encodeUnit( mSizeUnit );
  properties["size_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  properties["offset"] = QgsSymbolLayerUtils::encodePoint( mOffset );
  properties["offset_unit"] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  properties["offset_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  return properties;
}

void QgsVectorFieldSymbolLayer::toSld( QDomDocument& doc, QDomElement &element, const QgsStringMap& props ) const
{
  element.appendChild( doc.createComment( "VectorField not implemented yet..." ) );
  mLineSymbol->toSld( doc, element, props );
}

QgsSymbolLayer* QgsVectorFieldSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element );
  return nullptr;
}

void QgsVectorFieldSymbolLayer::drawPreviewIcon( QgsSymbolRenderContext& context, QSize size )
{
  if ( mLineSymbol )
  {
    mLineSymbol->drawPreviewIcon( context.renderContext().painter(), size );
  }
}

QSet<QString> QgsVectorFieldSymbolLayer::usedAttributes() const
{
  QSet<QString> attributes = QgsMarkerSymbolLayer::usedAttributes();
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
    attributes.unite( mLineSymbol->usedAttributes() );
  }
  return attributes;
}

void QgsVectorFieldSymbolLayer::convertPolarToCartesian( double length, double angle, double& x, double& y ) const
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

  x = length * sin( angle );
  y = length * cos( angle );
}

void QgsVectorFieldSymbolLayer::setColor( const QColor& color )
{
  if ( mLineSymbol )
    mLineSymbol->setColor( color );

  mColor = color;
}

QColor QgsVectorFieldSymbolLayer::color() const
{
  return mLineSymbol ? mLineSymbol->color() : mColor;
}


