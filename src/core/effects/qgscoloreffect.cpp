/***************************************************************************
                              qgscoloreffect.cpp
                              ------------------
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

#include "qgscoloreffect.h"

#include <algorithm>

#include "qgscolorutils.h"
#include "qgsimageoperation.h"
#include "qgsrendercontext.h"

QgsPaintEffect *QgsColorEffect::create( const QVariantMap &map )
{
  QgsColorEffect *newEffect = new QgsColorEffect();
  newEffect->readProperties( map );
  return newEffect;
}

QgsColorEffect::QgsColorEffect()
  : mColorizeColor( QColor::fromRgb( 255, 128, 128 ) )
{

}

Qgis::PaintEffectFlags QgsColorEffect::flags() const
{
  return Qgis::PaintEffectFlag::RequiresRasterization;
}

void QgsColorEffect::draw( QgsRenderContext &context )
{
  if ( !enabled() || !context.painter() || source().isNull() )
    return;

  if ( context.rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::ForceVector )
  {
    //just draw unmodified source, we can't render this effect when forcing vectors
    drawSource( *context.painter() );
    return;
  }

  QPainter *painter = context.painter();

  //rasterize source and apply modifications
  QImage image = sourceAsImage( context ).copy();

  QgsImageOperation::adjustBrightnessContrast( image, mBrightness, mContrast / 100.0 + 1, context.feedback() );

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  if ( mGrayscaleMode != QgsImageOperation::GrayscaleOff )
  {
    QgsImageOperation::convertToGrayscale( image, static_cast< QgsImageOperation::GrayscaleMode >( mGrayscaleMode ), context.feedback() );
  }

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QgsImageOperation::adjustHueSaturation( image, mSaturation, mColorizeOn ? mColorizeColor : QColor(), mColorizeStrength / 100.0, context.feedback() );

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QgsImageOperation::multiplyOpacity( image, mOpacity, context.feedback() );

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QgsScopedQPainterState painterState( painter );
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), image );
}


QVariantMap QgsColorEffect::properties() const
{
  QVariantMap props;
  props.insert( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  props.insert( u"draw_mode"_s, QString::number( int( mDrawMode ) ) );
  props.insert( u"blend_mode"_s, QString::number( int( mBlendMode ) ) );
  props.insert( u"opacity"_s, QString::number( mOpacity ) );
  props.insert( u"brightness"_s, QString::number( mBrightness ) );
  props.insert( u"contrast"_s, QString::number( mContrast ) );
  props.insert( u"saturation"_s, QString::number( mSaturation ) );
  props.insert( u"grayscale_mode"_s, QString::number( int( mGrayscaleMode ) ) );
  props.insert( u"colorize"_s, mColorizeOn ? u"1"_s : u"0"_s );
  props.insert( u"colorize_color"_s, QgsColorUtils::colorToString( mColorizeColor ) );
  props.insert( u"colorize_strength"_s, QString::number( mColorizeStrength ) );

  return props;
}

void QgsColorEffect::readProperties( const QVariantMap &props )
{
  bool ok;
  QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( u"blend_mode"_s ).toInt( &ok ) );
  if ( ok )
  {
    mBlendMode = mode;
  }
  if ( props.contains( u"transparency"_s ) )
  {
    double transparency = props.value( u"transparency"_s ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = 1.0 - transparency;
    }
  }
  else
  {
    double opacity = props.value( u"opacity"_s ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = opacity;
    }
  }
  mEnabled = props.value( u"enabled"_s, u"1"_s ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( u"draw_mode"_s, u"2"_s ).toInt() );

  mBrightness = props.value( u"brightness"_s, u"0"_s ).toInt();
  mContrast = props.value( u"contrast"_s, u"0"_s ).toInt();
  mSaturation = props.value( u"saturation"_s, u"1.0"_s ).toDouble();
  mGrayscaleMode = static_cast< QgsImageOperation::GrayscaleMode >( props.value( u"grayscale_mode"_s, u"0"_s ).toInt() );
  mColorizeOn = props.value( u"colorize"_s, u"0"_s ).toInt();
  if ( props.contains( u"colorize_color"_s ) )
  {
    setColorizeColor( QgsColorUtils::colorFromString( props.value( u"colorize_color"_s ).toString() ) );
  }
  mColorizeStrength = props.value( u"colorize_strength"_s, u"100"_s ).toInt();
}

QgsColorEffect *QgsColorEffect::clone() const
{
  QgsColorEffect *newEffect = new QgsColorEffect( *this );
  return newEffect;
}

void QgsColorEffect::setBrightness( int brightness )
{
  mBrightness = std::clamp( brightness, -255, 255 );
}

void QgsColorEffect::setContrast( int contrast )
{
  mContrast = std::clamp( contrast, -100, 100 );
}

void QgsColorEffect::setColorizeColor( const QColor &colorizeColor )
{
  mColorizeColor = colorizeColor;
}
