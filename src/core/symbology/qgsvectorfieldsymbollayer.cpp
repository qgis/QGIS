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

#include "qgslinesymbol.h"
#include "qgssldexportcontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer()
{
  setSubSymbol( new QgsLineSymbol() );
}

QgsVectorFieldSymbolLayer::~QgsVectorFieldSymbolLayer() = default;

void QgsVectorFieldSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mDistanceUnit = unit;
  if ( mLineSymbol )
    mLineSymbol->setOutputUnit( unit );
}

Qgis::RenderUnit QgsVectorFieldSymbolLayer::outputUnit() const
{
  if ( QgsMarkerSymbolLayer::outputUnit() == mDistanceUnit )
  {
    return mDistanceUnit;
  }
  return Qgis::RenderUnit::Unknown;
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
  if ( properties.contains( u"x_attribute"_s ) )
  {
    symbolLayer->setXAttribute( properties[u"x_attribute"_s].toString() );
  }
  if ( properties.contains( u"y_attribute"_s ) )
  {
    symbolLayer->setYAttribute( properties[u"y_attribute"_s].toString() );
  }
  if ( properties.contains( u"distance_unit"_s ) )
  {
    symbolLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties[u"distance_unit"_s].toString() ) );
  }
  if ( properties.contains( u"distance_map_unit_scale"_s ) )
  {
    symbolLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"distance_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"scale"_s ) )
  {
    symbolLayer->setScale( properties[u"scale"_s].toDouble() );
  }
  if ( properties.contains( u"vector_field_type"_s ) )
  {
    symbolLayer->setVectorFieldType( static_cast< VectorFieldType >( properties[u"vector_field_type"_s].toInt() ) );
  }
  if ( properties.contains( u"angle_orientation"_s ) )
  {
    symbolLayer->setAngleOrientation( static_cast< AngleOrientation >( properties[u"angle_orientation"_s].toInt() ) );
  }
  if ( properties.contains( u"angle_units"_s ) )
  {
    symbolLayer->setAngleUnits( static_cast< AngleUnits >( properties[u"angle_units"_s].toInt() ) );
  }
  if ( properties.contains( u"size"_s ) )
  {
    symbolLayer->setSize( properties[u"size"_s].toDouble() );
  }
  if ( properties.contains( u"size_unit"_s ) )
  {
    symbolLayer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[u"size_unit"_s].toString() ) );
  }
  if ( properties.contains( u"size_map_unit_scale"_s ) )
  {
    symbolLayer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"size_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"offset"_s ) )
  {
    symbolLayer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[u"offset"_s].toString() ) );
  }
  if ( properties.contains( u"offset_unit"_s ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_unit"_s].toString() ) );
  }
  if ( properties.contains( u"offset_map_unit_scale"_s ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_map_unit_scale"_s].toString() ) );
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
    mLineSymbol->setRenderHints( mLineSymbol->renderHints() | Qgis::SymbolRenderHint::IsSymbolLayerSubSymbol );
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
  properties[u"x_attribute"_s] = mXAttribute;
  properties[u"y_attribute"_s] = mYAttribute;
  properties[u"distance_unit"_s] = QgsUnitTypes::encodeUnit( mDistanceUnit );
  properties[u"distance_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceMapUnitScale );
  properties[u"scale"_s] = QString::number( mScale );
  properties[u"vector_field_type"_s] = QString::number( mVectorFieldType );
  properties[u"angle_orientation"_s] = QString::number( mAngleOrientation );
  properties[u"angle_units"_s] = QString::number( mAngleUnits );
  properties[u"size"_s] = QString::number( mSize );
  properties[u"size_unit"_s] = QgsUnitTypes::encodeUnit( mSizeUnit );
  properties[u"size_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  properties[u"offset"_s] = QgsSymbolLayerUtils::encodePoint( mOffset );
  properties[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  properties[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  return properties;
}

bool QgsVectorFieldSymbolLayer::usesMapUnits() const
{
  return mDistanceUnit == Qgis::RenderUnit::MapUnits || mDistanceUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits
         || mSizeUnit == Qgis::RenderUnit::MapUnits || mSizeUnit == Qgis::RenderUnit::MetersInMapUnits;
}

void QgsVectorFieldSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsVectorFieldSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  context.pushError( QObject::tr( "Vector field symbol layers cannot be converted to SLD" ) );
  mLineSymbol->toSld( doc, element, context );
  return false;
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


