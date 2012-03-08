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

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer(): mXAttribute( "" ), mYAttribute( "" ), mScale( 1.0 ),
    mVectorFieldType( Cartesian ), mAngleOrientation( ClockwiseFromNorth ), mAngleUnits( Degrees ), mXIndex( -1 ), mYIndex( -1 )
{
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsVectorFieldSymbolLayer::~QgsVectorFieldSymbolLayer()
{
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
  return symbolLayer;
}

bool QgsVectorFieldSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( symbol->type() == QgsSymbolV2::Line )
  {
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
  if ( mXIndex != -1 )
  {
    xVal = f->attributeMap()[mXIndex].toDouble();
  }
  double yVal = 0;
  if ( mYIndex != -1 )
  {
    yVal = f->attributeMap()[mYIndex].toDouble();
  }

  switch ( mVectorFieldType )
  {
    case Cartesian:
      xComponent = context.outputLineWidth( xVal );
      yComponent = context.outputLineWidth( yVal );
      break;
    case Polar:
      convertPolarToCartesian( xVal, yVal, xComponent, yComponent );
      xComponent = context.outputLineWidth( xComponent );
      yComponent = context.outputLineWidth( yComponent );
      break;
    case Height:
      xComponent = 0;
      yComponent = context.outputLineWidth( yVal );
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
    mLineSymbol->startRender( context.renderContext() );
  }

  const QgsVectorLayer* layer = context.layer();
  if ( layer )
  {
    mXIndex = layer->fieldNameIndex( mXAttribute );
    mYIndex = layer->fieldNameIndex( mYAttribute );
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
  properties["scale"] = QString::number( mScale );
  properties["vector_field_type"] = QString::number( mVectorFieldType );
  properties["angle_orientation"] = QString::number( mAngleOrientation );
  properties["angle_units"] = QString::number( mAngleUnits );
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
