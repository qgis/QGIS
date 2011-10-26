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

QgsVectorFieldSymbolLayer::QgsVectorFieldSymbolLayer(): mXAttribute( -1 ), mYAttribute( -1 ), mScale( 1.0 ),
    mVectorFieldType( Cartesian ), mAngleOrientation( ClockwiseFromNorth ), mAngleUnits( Degrees )
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
    symbolLayer->setXAttribute( properties["x_attribute"].toInt() );
  }
  if ( properties.contains( "y_attribute" ) )
  {
    symbolLayer->setYAttribute( properties["y_attribute"].toInt() );
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
  //soon...
}

void QgsVectorFieldSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  if ( mLineSymbol )
  {
    mLineSymbol->startRender( context.renderContext() );
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
  return QgsVectorFieldSymbolLayer::create( properties() );
}

QgsStringMap QgsVectorFieldSymbolLayer::properties() const
{
  QgsStringMap properties;
  properties["x_attribute"] = QString::number( mXAttribute );
  properties["y_attribute"] = QString::number( mYAttribute );
  properties["scale"] = QString::number( mScale );
  properties["vector_field_type"] = QString::number( mVectorFieldType );
  properties["angle_orientation"] = QString::number( mAngleOrientation );
  properties["angle_units"] = QString::number( mAngleUnits );
  return properties;
}
