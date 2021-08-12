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
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"
#include <QPicture>
#include <QTransform>

QgsPaintEffect *QgsTransformEffect::create( const QVariantMap &map )
{
  QgsTransformEffect *newEffect = new QgsTransformEffect();
  newEffect->readProperties( map );
  return newEffect;
}

void QgsTransformEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QPainter *painter = context.painter();

  //apply transformations
  const QgsScopedQPainterState painterState( painter );

  const QTransform t = createTransform( context );
  painter->setTransform( t, true );
  drawSource( *painter );
}

QVariantMap QgsTransformEffect::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "reflect_x" ), mReflectX ? "1" : "0" );
  props.insert( QStringLiteral( "reflect_y" ), mReflectY ? "1" : "0" );
  props.insert( QStringLiteral( "scale_x" ), QString::number( mScaleX ) );
  props.insert( QStringLiteral( "scale_y" ), QString::number( mScaleY ) );
  props.insert( QStringLiteral( "rotation" ), QString::number( mRotation ) );
  props.insert( QStringLiteral( "shear_x" ), QString::number( mShearX ) );
  props.insert( QStringLiteral( "shear_y" ), QString::number( mShearY ) );
  props.insert( QStringLiteral( "translate_x" ), QString::number( mTranslateX ) );
  props.insert( QStringLiteral( "translate_y" ), QString::number( mTranslateY ) );
  props.insert( QStringLiteral( "translate_unit" ), QgsUnitTypes::encodeUnit( mTranslateUnit ) );
  props.insert( QStringLiteral( "translate_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mTranslateMapUnitScale ) );
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( int( mDrawMode ) ) );
  return props;
}

void QgsTransformEffect::readProperties( const QVariantMap &props )
{
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( QStringLiteral( "draw_mode" ), QStringLiteral( "2" ) ).toInt() );
  mReflectX = props.value( QStringLiteral( "reflect_x" ), QStringLiteral( "0" ) ).toInt();
  mReflectY = props.value( QStringLiteral( "reflect_y" ), QStringLiteral( "0" ) ).toInt();
  mScaleX = props.value( QStringLiteral( "scale_x" ), QStringLiteral( "1.0" ) ).toDouble();
  mScaleY = props.value( QStringLiteral( "scale_y" ), QStringLiteral( "1.0" ) ).toDouble();
  mRotation = props.value( QStringLiteral( "rotation" ), QStringLiteral( "0.0" ) ).toDouble();
  mShearX = props.value( QStringLiteral( "shear_x" ), QStringLiteral( "0.0" ) ).toDouble();
  mShearY = props.value( QStringLiteral( "shear_y" ), QStringLiteral( "0.0" ) ).toDouble();
  mTranslateX = props.value( QStringLiteral( "translate_x" ), QStringLiteral( "0.0" ) ).toDouble();
  mTranslateY = props.value( QStringLiteral( "translate_y" ), QStringLiteral( "0.0" ) ).toDouble();
  mTranslateUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "translate_unit" ) ).toString() );
  mTranslateMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "translate_unit_scale" ) ).toString() );
}

QgsTransformEffect *QgsTransformEffect::clone() const
{
  QgsTransformEffect *newEffect = new QgsTransformEffect( *this );
  return newEffect;
}

QRectF QgsTransformEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  const QTransform t = createTransform( context );
  return t.mapRect( rect );
}

QTransform QgsTransformEffect::createTransform( const QgsRenderContext &context ) const
{
  QTransform t;

  if ( !source() )
    return t;

  const int width = source()->boundingRect().width();
  const int height = source()->boundingRect().height();
  const int top = source()->boundingRect().top();
  const int left = source()->boundingRect().left();

  //remember that the below operations are effectively performed in the opposite order
  //so, first the reflection applies, then scale, shear, rotate and lastly translation

  const double translateX = context.convertToPainterUnits( mTranslateX, mTranslateUnit, mTranslateMapUnitScale );
  const double translateY = context.convertToPainterUnits( mTranslateY, mTranslateUnit, mTranslateMapUnitScale );

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
