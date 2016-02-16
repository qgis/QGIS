/***************************************************************************
                              qgstransformeffect.cpp
                              ----------------------
    begin                : March 2015
    copyright            : (C) 2015 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstransformeffect.h"
#include "qgssymbollayerv2utils.h"
#include "qgsunittypes.h"
#include <QPicture>
#include <QTransform>

QgsPaintEffect* QgsTransformEffect::create( const QgsStringMap &map )
{
  QgsTransformEffect* newEffect = new QgsTransformEffect();
  newEffect->readProperties( map );
  return newEffect;
}

QgsTransformEffect::QgsTransformEffect()
    : QgsPaintEffect()
    , mTranslateX( 0.0 )
    , mTranslateY( 0.0 )
    , mTranslateUnit( QgsSymbolV2::MM )
    , mScaleX( 1.0 )
    , mScaleY( 1.0 )
    , mRotation( 0.0 )
    , mShearX( 0.0 )
    , mShearY( 0.0 )
    , mReflectX( false )
    , mReflectY( false )
{

}

QgsTransformEffect::~QgsTransformEffect()
{

}

void QgsTransformEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QPainter* painter = context.painter();

  //apply transformations
  painter->save();

  QTransform t = createTransform( context );
  painter->setTransform( t, true );
  drawSource( *painter );

  painter->restore();
}

QgsStringMap QgsTransformEffect::properties() const
{
  QgsStringMap props;
  props.insert( "reflect_x", mReflectX ? "1" : "0" );
  props.insert( "reflect_y", mReflectY ? "1" : "0" );
  props.insert( "scale_x", QString::number( mScaleX ) );
  props.insert( "scale_y", QString::number( mScaleY ) );
  props.insert( "rotation", QString::number( mRotation ) );
  props.insert( "shear_x", QString::number( mShearX ) );
  props.insert( "shear_y", QString::number( mShearY ) );
  props.insert( "translate_x", QString::number( mTranslateX ) );
  props.insert( "translate_y", QString::number( mTranslateY ) );
  props.insert( "translate_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mTranslateUnit ) );
  props.insert( "translate_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mTranslateMapUnitScale ) );
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  return props;
}

void QgsTransformEffect::readProperties( const QgsStringMap &props )
{
  mEnabled = props.value( "enabled", "1" ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( "draw_mode", "2" ).toInt() );
  mReflectX = props.value( "reflect_x", "0" ).toInt();
  mReflectY = props.value( "reflect_y", "0" ).toInt();
  mScaleX = props.value( "scale_x", "1.0" ).toDouble();
  mScaleY = props.value( "scale_y", "1.0" ).toDouble();
  mRotation = props.value( "rotation", "0.0" ).toDouble();
  mTranslateX = props.value( "translate_x", "0.0" ).toDouble();
  mTranslateY = props.value( "translate_y", "0.0" ).toDouble();
  mTranslateUnit = QgsSymbolLayerV2Utils::decodeOutputUnit( props.value( "translate_unit" ) );
  mTranslateMapUnitScale = QgsSymbolLayerV2Utils::decodeMapUnitScale( props.value( "translate_unit_scale" ) );
}

QgsTransformEffect* QgsTransformEffect::clone() const
{
  QgsTransformEffect* newEffect = new QgsTransformEffect( *this );
  return newEffect;
}

QRectF QgsTransformEffect::boundingRect( const QRectF &rect, const QgsRenderContext& context ) const
{
  QTransform t = createTransform( context );
  return t.mapRect( rect );
}

QTransform QgsTransformEffect::createTransform( const QgsRenderContext& context ) const
{
  QTransform t;

  if ( !source() )
    return t;

  int width = source()->boundingRect().width();
  int height = source()->boundingRect().height();
  int top = source()->boundingRect().top();
  int left = source()->boundingRect().left();

  //remember that the below operations are effectively performed in the opposite order
  //so, first the reflection applies, then scale, shear, rotate and lastly translation

  double translateX = mTranslateX *
                      QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mTranslateUnit, mTranslateMapUnitScale );
  double translateY = mTranslateY *
                      QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mTranslateUnit, mTranslateMapUnitScale );

  t.translate( translateX + left + width / 2.0,
               translateY + top + height / 2.0 );

  t.rotate( mRotation );
  t.shear( mShearX, mShearY );
  t.scale( mScaleX, mScaleY );

  if ( mReflectX || mReflectY )
  {
    t.scale( mReflectX ? -1 : 1, mReflectY ? -1 : 1 );
  }

  t.translate( -left - width / 2.0,
               -top - height / 2.0 );

  return t;
}
