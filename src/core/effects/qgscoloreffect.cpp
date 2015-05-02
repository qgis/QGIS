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
#include "qgssymbollayerv2utils.h"

QgsPaintEffect *QgsColorEffect::create( const QgsStringMap &map )
{
  QgsColorEffect* newEffect = new QgsColorEffect();
  newEffect->readProperties( map );
  return newEffect;
}

QgsColorEffect::QgsColorEffect()
    : QgsPaintEffect()
    , mTransparency( 0.0 )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mBrightness( 0 )
    , mContrast( 0 )
    , mSaturation( 1.0 )
    , mGrayscaleMode( QgsImageOperation::GrayscaleOff )
    , mColorizeOn( false )
    , mColorizeColor( QColor::fromRgb( 255, 128, 128 ) )
    , mColorizeStrength( 100 )
{

}

QgsColorEffect::~QgsColorEffect()
{

}

void QgsColorEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QPainter* painter = context.painter();

  //rasterise source and apply modifications
  QImage image = sourceAsImage( context )->copy();

  QgsImageOperation::adjustBrightnessContrast( image, mBrightness, mContrast / 100.0 + 1 );
  if ( mGrayscaleMode != QgsImageOperation::GrayscaleOff )
  {
    QgsImageOperation::convertToGrayscale( image, ( QgsImageOperation::GrayscaleMode ) mGrayscaleMode );
  }
  QgsImageOperation::adjustHueSaturation( image, mSaturation, mColorizeOn ? mColorizeColor : QColor(), mColorizeStrength / 100.0 );

  QgsImageOperation::multiplyOpacity( image, 1.0 - mTransparency );
  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), image );
  painter->restore();
}


QgsStringMap QgsColorEffect::properties() const
{
  QgsStringMap props;
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  props.insert( "blend_mode", QString::number( int( mBlendMode ) ) );
  props.insert( "transparency", QString::number( mTransparency ) );
  props.insert( "brightness", QString::number( mBrightness ) );
  props.insert( "contrast", QString::number( mContrast ) );
  props.insert( "saturation", QString::number( mSaturation ) );
  props.insert( "grayscale_mode", QString::number( int( mGrayscaleMode ) ) );
  props.insert( "colorize", mColorizeOn ? "1" : "0" );
  props.insert( "colorize_color", QgsSymbolLayerV2Utils::encodeColor( mColorizeColor ) );
  props.insert( "colorize_strength", QString::number( mColorizeStrength ) );

  return props;
}

void QgsColorEffect::readProperties( const QgsStringMap &props )
{
  bool ok;
  QPainter::CompositionMode mode = ( QPainter::CompositionMode )props.value( "blend_mode" ).toInt( &ok );
  if ( ok )
  {
    mBlendMode = mode;
  }
  double transparency = props.value( "transparency" ).toDouble( &ok );
  if ( ok )
  {
    mTransparency = transparency;
  }
  mEnabled = props.value( "enabled", "1" ).toInt();
  mDrawMode = ( QgsPaintEffect::DrawMode )props.value( "draw_mode", "2" ).toInt();

  mBrightness = props.value( "brightness", "0" ).toInt();
  mContrast = props.value( "contrast", "0" ).toInt();
  mSaturation = props.value( "saturation", "1.0" ).toDouble();
  mGrayscaleMode = ( QgsImageOperation::GrayscaleMode )props.value( "grayscale_mode", "0" ).toInt();
  mColorizeOn = props.value( "colorize", "0" ).toInt();
  if ( props.contains( "colorize_color" ) )
  {
    setColorizeColor( QgsSymbolLayerV2Utils::decodeColor( props.value( "colorize_color" ) ) );
  }
  mColorizeStrength = props.value( "colorize_strength", "100" ).toInt();
}

QgsPaintEffect *QgsColorEffect::clone() const
{
  QgsColorEffect* newEffect = new QgsColorEffect( *this );
  return newEffect;
}

void QgsColorEffect::setColorizeColor( const QColor& colorizeColor )
{
  mColorizeColor = colorizeColor;
}
