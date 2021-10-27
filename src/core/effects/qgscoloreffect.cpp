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
#include "qgsimageoperation.h"
#include "qgssymbollayerutils.h"
#include <algorithm>

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

void QgsColorEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QPainter *painter = context.painter();

  //rasterize source and apply modifications
  QImage image = sourceAsImage( context )->copy();

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
  props.insert( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( int( mDrawMode ) ) );
  props.insert( QStringLiteral( "blend_mode" ), QString::number( int( mBlendMode ) ) );
  props.insert( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  props.insert( QStringLiteral( "brightness" ), QString::number( mBrightness ) );
  props.insert( QStringLiteral( "contrast" ), QString::number( mContrast ) );
  props.insert( QStringLiteral( "saturation" ), QString::number( mSaturation ) );
  props.insert( QStringLiteral( "grayscale_mode" ), QString::number( int( mGrayscaleMode ) ) );
  props.insert( QStringLiteral( "colorize" ), mColorizeOn ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  props.insert( QStringLiteral( "colorize_color" ), QgsSymbolLayerUtils::encodeColor( mColorizeColor ) );
  props.insert( QStringLiteral( "colorize_strength" ), QString::number( mColorizeStrength ) );

  return props;
}

void QgsColorEffect::readProperties( const QVariantMap &props )
{
  bool ok;
  QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( QStringLiteral( "blend_mode" ) ).toInt( &ok ) );
  if ( ok )
  {
    mBlendMode = mode;
  }
  if ( props.contains( QStringLiteral( "transparency" ) ) )
  {
    double transparency = props.value( QStringLiteral( "transparency" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = 1.0 - transparency;
    }
  }
  else
  {
    double opacity = props.value( QStringLiteral( "opacity" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = opacity;
    }
  }
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( QStringLiteral( "draw_mode" ), QStringLiteral( "2" ) ).toInt() );

  mBrightness = props.value( QStringLiteral( "brightness" ), QStringLiteral( "0" ) ).toInt();
  mContrast = props.value( QStringLiteral( "contrast" ), QStringLiteral( "0" ) ).toInt();
  mSaturation = props.value( QStringLiteral( "saturation" ), QStringLiteral( "1.0" ) ).toDouble();
  mGrayscaleMode = static_cast< QgsImageOperation::GrayscaleMode >( props.value( QStringLiteral( "grayscale_mode" ), QStringLiteral( "0" ) ).toInt() );
  mColorizeOn = props.value( QStringLiteral( "colorize" ), QStringLiteral( "0" ) ).toInt();
  if ( props.contains( QStringLiteral( "colorize_color" ) ) )
  {
    setColorizeColor( QgsSymbolLayerUtils::decodeColor( props.value( QStringLiteral( "colorize_color" ) ).toString() ) );
  }
  mColorizeStrength = props.value( QStringLiteral( "colorize_strength" ), QStringLiteral( "100" ) ).toInt();
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
