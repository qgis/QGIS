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
#include "qgslinesymbol.h"

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer()
{
  setSubSymbol( new QgsLineSymbol() );
}

QgsVectorFieldSymbolLayer::~QgsVectorFieldSymbolLayer() = default;

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

QgsSymbolLayer *QgsVectorFieldSymbolLayer::create( const QVariantMap &properties )
{
  QgsVectorFieldSymbolLayer *symbolLayer = new QgsVectorFieldSymbolLayer();
  if ( properties.contains( QStringLiteral( "x_attribute" ) ) )
  {
    symbolLayer->setXAttribute( properties[QStringLiteral( "x_attribute" )].toString() );
  }
  if ( properties.contains( QStringLiteral( "y_attribute" ) ) )
  {
    symbolLayer->setYAttribute( properties[QStringLiteral( "y_attribute" )].toString() );
  }
  if ( properties.contains( QStringLiteral( "distance_unit" ) ) )
  {
    symbolLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    symbolLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_map_unit_scale" )].toString() ) );
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
    symbolLayer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "size_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "size_map_unit_scale" ) ) )
  {
    symbolLayer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "size_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    symbolLayer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }
  return symbolLayer;
}

bool QgsVectorFieldSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( symbol->type() == Qgis::SymbolType::Line )
  {
    mLineSymbol.reset( static_cast<QgsLineSymbol *>( symbol ) );
    return true;
  }
  return false;
}

QgsSymbol *QgsVectorFieldSymbolLayer::subSymbol()
{
  return mLineSymbol.get();
}

void QgsVectorFieldSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  if ( !mLineSymbol )
  {
    return;
  }

  const QgsRenderContext &ctx = context.renderContext();

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  if ( !context.feature() )
  {
    //preview
    QPolygonF line;
    line << QPointF( 0, 50 );
    line << QPointF( 100, 50 );
    mLineSymbol->renderPolyline( line, nullptr, context.renderContext() );
    context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
    return;
  }

  const QgsFeature f( *context.feature() );

  double xComponent = 0;
  double yComponent = 0;

  double xVal = 0;
  if ( mXIndex != -1 )
  {
    xVal = f.attribute( mXIndex ).toDouble();
  }
  double yVal = 0;
  if ( mYIndex != -1 )
  {
    yVal = f.attribute( mYIndex ).toDouble();
  }

  const QgsMapToPixel &m2p = ctx.mapToPixel();
  const double mapRotation = m2p.mapRotation();

  QPolygonF line;
  line << point;

  QPointF destPoint;
  switch ( mVectorFieldType )
  {
    case Cartesian:
    {
      destPoint = QPointF( point.x() + mScale * ctx.convertToPainterUnits( xVal, mDistanceUnit, mDistanceMapUnitScale ),
                           point.y() - mScale * ctx.convertToPainterUnits( yVal, mDistanceUnit, mDistanceMapUnitScale ) );
      break;
    }

    case Polar:
    {
      convertPolarToCartesian( xVal, yVal, xComponent, yComponent );
      destPoint = QPointF( point.x() + mScale * ctx.convertToPainterUnits( xComponent, mDistanceUnit, mDistanceMapUnitScale ),
                           point.y() - mScale * ctx.convertToPainterUnits( yComponent, mDistanceUnit, mDistanceMapUnitScale ) );
      break;
    }

    case Height:
    {
      destPoint = QPointF( point.x(), point.y() - ( mScale * ctx.convertToPainterUnits( yVal, mDistanceUnit, mDistanceMapUnitScale ) ) );
      break;
    }
  }

  if ( !qgsDoubleNear( mapRotation, 0.0 ) && mVectorFieldType != Height )
  {
    const double radians = mapRotation * M_PI / 180.0;
    destPoint = QPointF( cos( radians ) * ( destPoint.x() - point.x() ) - sin( radians ) * ( destPoint.y() - point.y() ) + point.x(),
                         sin( radians ) * ( destPoint.x() - point.x() ) + cos( radians ) * ( destPoint.y() - point.y() ) + point.y() );
  }

  line << destPoint;

  mLineSymbol->renderPolyline( line, &f, context.renderContext() );
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
}

void QgsVectorFieldSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->startRender( context.renderContext(), context.fields() );
  }

  const QgsFields fields = context.fields();
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

QVariantMap QgsVectorFieldSymbolLayer::properties() const
{
  QVariantMap properties;
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

bool QgsVectorFieldSymbolLayer::usesMapUnits() const
{
  return mDistanceUnit == QgsUnitTypes::RenderMapUnits || mDistanceUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mSizeUnit == QgsUnitTypes::RenderMapUnits || mSizeUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsVectorFieldSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  element.appendChild( doc.createComment( QStringLiteral( "VectorField not implemented yet..." ) ) );
  mLineSymbol->toSld( doc, element, props );
}

QgsSymbolLayer *QgsVectorFieldSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element )
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

bool QgsVectorFieldSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mLineSymbol && mLineSymbol->hasDataDefinedProperties() )
    return true;
  return false;
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


