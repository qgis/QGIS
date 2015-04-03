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

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer()
    : mXAttribute( "" )
    , mYAttribute( "" )
    , mDistanceUnit( QgsSymbolV2::MM )
    , mScale( 1.0 )
    , mVectorFieldType( Cartesian )
    , mAngleOrientation( ClockwiseFromNorth )
    , mAngleUnits( Degrees )
    , mLineSymbol( 0 )
    , mXIndex( -1 )
    , mYIndex( -1 )
{
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsVectorFieldSymbolLayer::~QgsVectorFieldSymbolLayer()
{
  delete mLineSymbol;
}

void QgsVectorFieldSymbolLayer::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsMarkerSymbolLayerV2::setOutputUnit( unit );
  mDistanceUnit = unit;
}

QgsSymbolV2::OutputUnit QgsVectorFieldSymbolLayer::outputUnit() const
{
  if ( QgsMarkerSymbolLayerV2::outputUnit() == mDistanceUnit )
  {
    return mDistanceUnit;
  }
  return QgsSymbolV2::Mixed;
}

void QgsVectorFieldSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayerV2::setMapUnitScale( scale );
  mDistanceMapUnitScale = scale;
}

QgsMapUnitScale QgsVectorFieldSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayerV2::mapUnitScale() == mDistanceMapUnitScale )
  {
    return mDistanceMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayerV2* QgsVectorFieldSymbolLayer::create( const QgsStringMap& properties )
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
    symbolLayer->setDistanceUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["distance_unit"] ) );
  }
  if ( properties.contains( "distance_map_unit_scale" ) )
  {
    symbolLayer->setDistanceMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["distance_map_unit_scale"] ) );
  }
  if ( properties.contains( "scale" ) )
  {
    symbolLayer->setScale( properties["scale"].toDouble() );
  }
  if ( properties.contains( "vector_field_type" ) )
  {
    symbolLayer->setVectorFieldType(( VectorFieldType )( properties["vector_field_type"].toInt() ) );
  }
  if ( properties.contains( "angle_orientation" ) )
  {
    symbolLayer->setAngleOrientation(( AngleOrientation )( properties["angle_orientation"].toInt() ) );
  }
  if ( properties.contains( "angle_units" ) )
  {
    symbolLayer->setAngleUnits(( AngleUnits )( properties["angle_units"].toInt() ) );
  }
  if ( properties.contains( "size" ) )
  {
    symbolLayer->setSize( properties["size"].toDouble() );
  }
  if ( properties.contains( "size_unit" ) )
  {
    symbolLayer->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["size_unit"] ) );
  }
  if ( properties.contains( "size_map_unit_scale" ) )
  {
    symbolLayer->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["size_map_unit_scale"] ) );
  }
  if ( properties.contains( "offset" ) )
  {
    symbolLayer->setOffset( QgsSymbolLayerV2Utils::decodePoint( properties["offset"] ) );
  }
  if ( properties.contains( "offset_unit" ) )
  {
    symbolLayer->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["offset_unit"] ) );
  }
  if ( properties.contains( "offset_map_unit_scale" ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["offset_map_unit_scale"] ) );
  }
  return symbolLayer;
}

bool QgsVectorFieldSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( symbol->type() == QgsSymbolV2::Line )
  {
    delete mLineSymbol;
    mLineSymbol = static_cast<QgsLineSymbolV2*>( symbol );
    return true;
  }
  return false;
}

void QgsVectorFieldSymbolLayer::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
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
    mLineSymbol->renderPolyline( line, 0, context.renderContext() );
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
      xComponent = xVal * QgsSymbolLayerV2Utils::lineWidthScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = yVal * QgsSymbolLayerV2Utils::lineWidthScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Polar:
      convertPolarToCartesian( xVal, yVal, xComponent, yComponent );
      xComponent = xComponent * QgsSymbolLayerV2Utils::lineWidthScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
      yComponent = yComponent * QgsSymbolLayerV2Utils::lineWidthScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
      break;
    case Height:
      xComponent = 0;
      yComponent = yVal * QgsSymbolLayerV2Utils::lineWidthScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
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

void QgsVectorFieldSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->startRender( context.renderContext(), context.fields() );
  }

  const QgsFields* fields = context.fields();
  if ( fields )
  {
    mXIndex = fields->fieldNameIndex( mXAttribute );
    mYIndex = fields->fieldNameIndex( mYAttribute );
  }
  else
  {
    mXIndex = -1;
    mYIndex = -1;
  }
}

void QgsVectorFieldSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->stopRender( context.renderContext() );
  }
}

QgsSymbolLayerV2* QgsVectorFieldSymbolLayer::clone() const
{
  QgsSymbolLayerV2* clonedLayer = QgsVectorFieldSymbolLayer::create( properties() );
  if ( mLineSymbol )
  {
    clonedLayer->setSubSymbol( mLineSymbol->clone() );
  }
  return clonedLayer;
}

QgsStringMap QgsVectorFieldSymbolLayer::properties() const
{
  QgsStringMap properties;
  properties["x_attribute"] = mXAttribute;
  properties["y_attribute"] = mYAttribute;
  properties["distance_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mDistanceUnit );
  properties["distance_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mDistanceMapUnitScale );
  properties["scale"] = QString::number( mScale );
  properties["vector_field_type"] = QString::number( mVectorFieldType );
  properties["angle_orientation"] = QString::number( mAngleOrientation );
  properties["angle_units"] = QString::number( mAngleUnits );
  properties["size"] = QString::number( mSize );
  properties["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  properties["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  properties["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  properties["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  properties["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  return properties;
}

void QgsVectorFieldSymbolLayer::toSld( QDomDocument& doc, QDomElement &element, QgsStringMap props ) const
{
  element.appendChild( doc.createComment( "VectorField not implemented yet..." ) );
  mLineSymbol->toSld( doc, element, props );
}

QgsSymbolLayerV2* QgsVectorFieldSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element );
  return NULL;
}

void QgsVectorFieldSymbolLayer::drawPreviewIcon( QgsSymbolV2RenderContext& context, QSize size )
{
  if ( mLineSymbol )
  {
    mLineSymbol->drawPreviewIcon( context.renderContext().painter(), size );
  }
}

QSet<QString> QgsVectorFieldSymbolLayer::usedAttributes() const
{
  QSet<QString> attributes;
  if ( !mXAttribute.isEmpty() )
  {
    attributes.insert( mXAttribute );
  }
  if ( !mYAttribute.isEmpty() )
  {
    attributes.insert( mYAttribute );
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


