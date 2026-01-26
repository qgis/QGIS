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
  if ( !enabled() || !context.painter() || source().isNull() )
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
  props.insert( u"reflect_x"_s, mReflectX ? "1" : "0" );
  props.insert( u"reflect_y"_s, mReflectY ? "1" : "0" );
  props.insert( u"scale_x"_s, QString::number( mScaleX ) );
  props.insert( u"scale_y"_s, QString::number( mScaleY ) );
  props.insert( u"rotation"_s, QString::number( mRotation ) );
  props.insert( u"shear_x"_s, QString::number( mShearX ) );
  props.insert( u"shear_y"_s, QString::number( mShearY ) );
  props.insert( u"translate_x"_s, QString::number( mTranslateX ) );
  props.insert( u"translate_y"_s, QString::number( mTranslateY ) );
  props.insert( u"translate_unit"_s, QgsUnitTypes::encodeUnit( mTranslateUnit ) );
  props.insert( u"translate_unit_scale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mTranslateMapUnitScale ) );
  props.insert( u"enabled"_s, mEnabled ? "1" : "0" );
  props.insert( u"draw_mode"_s, QString::number( int( mDrawMode ) ) );
  return props;
}

void QgsTransformEffect::readProperties( const QVariantMap &props )
{
  mEnabled = props.value( u"enabled"_s, u"1"_s ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( u"draw_mode"_s, u"2"_s ).toInt() );
  mReflectX = props.value( u"reflect_x"_s, u"0"_s ).toInt();
  mReflectY = props.value( u"reflect_y"_s, u"0"_s ).toInt();
  mScaleX = props.value( u"scale_x"_s, u"1.0"_s ).toDouble();
  mScaleY = props.value( u"scale_y"_s, u"1.0"_s ).toDouble();
  mRotation = props.value( u"rotation"_s, u"0.0"_s ).toDouble();
  mShearX = props.value( u"shear_x"_s, u"0.0"_s ).toDouble();
  mShearY = props.value( u"shear_y"_s, u"0.0"_s ).toDouble();
  mTranslateX = props.value( u"translate_x"_s, u"0.0"_s ).toDouble();
  mTranslateY = props.value( u"translate_y"_s, u"0.0"_s ).toDouble();
  mTranslateUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"translate_unit"_s ).toString() );
  mTranslateMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"translate_unit_scale"_s ).toString() );
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

  const QPicture &pic = source();
  if ( pic.isNull() )
    return t;

  const int width = pic.boundingRect().width();
  const int height = pic.boundingRect().height();
  const int top = pic.boundingRect().top();
  const int left = pic.boundingRect().left();

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
