/***************************************************************************
                              qgsdropshadoweffect.cpp
                             ------------------------
    begin                : December 2014
    copyright            : (C) 2014 Nyall Dawson
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

#include "qgsdropshadoweffect.h"
#include "qgsimageoperation.h"
#include "qgssymbollayerv2utils.h"

QgsPaintEffect *QgsDropShadowEffect::create( const QgsStringMap &map )
{
  QgsDropShadowEffect* effect = new QgsDropShadowEffect();
  effect->readProperties( map );
  return effect;
}

QgsDropShadowEffect::QgsDropShadowEffect()
    : QgsPaintEffect()
    , mBlurLevel( 10 )
    , mOffsetAngle( 135 )
    , mOffsetDist( 2.0 )
    , mOffsetUnit( QgsSymbolV2::MM )
    , mTransparency( 0.0 )
    , mScale( 1.0 )
    , mColor( Qt::black )
    , mBlendMode( QPainter::CompositionMode_Multiply )
{

}

QgsDropShadowEffect::~QgsDropShadowEffect()
{

}

void QgsDropShadowEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QImage colorisedIm = sourceAsImage( context )->copy();
  QgsImageOperation::overlayColor( colorisedIm, mColor );
  QgsImageOperation::stackBlur( colorisedIm, mBlurLevel );

  double offsetDist = mOffsetDist *
                      QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mOffsetUnit, mOffsetMapUnitScale );

  double angleRad = mOffsetAngle * M_PI / 180; // to radians
  QPointF transPt( -offsetDist * cos( angleRad + M_PI / 2 ),
                   -offsetDist * sin( angleRad + M_PI / 2 ) );

  //transparency, scale
  QgsImageOperation::multiplyOpacity( colorisedIm, 1.0 - mTransparency );

  QPainter* painter = context.painter();
  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ) + transPt, colorisedIm );
  painter->restore();
}

QgsPaintEffect *QgsDropShadowEffect::clone() const
{
  return new QgsDropShadowEffect( *this );
}

QgsStringMap QgsDropShadowEffect::properties() const
{
  QgsStringMap props;
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  props.insert( "blend_mode", QString::number( int( mBlendMode ) ) );
  props.insert( "transparency", QString::number( mTransparency ) );
  props.insert( "blur_level", QString::number( mBlurLevel ) );
  props.insert( "offset_angle", QString::number( mOffsetAngle ) );
  props.insert( "offset_distance", QString::number( mOffsetDist ) );
  props.insert( "offset_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit ) );
  props.insert( "offset_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale ) );
  props.insert( "scale", QString::number( mScale ) );
  props.insert( "color", QgsSymbolLayerV2Utils::encodeColor( mColor ) );
  return props;
}

void QgsDropShadowEffect::readProperties( const QgsStringMap &props )
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
  int level = props.value( "blur_level" ).toInt( &ok );
  if ( ok )
  {
    mBlurLevel = level;
  }
  int angle = props.value( "offset_angle" ).toInt( &ok );
  if ( ok )
  {
    mOffsetAngle = angle;
  }
  double distance = props.value( "offset_distance" ).toDouble( &ok );
  if ( ok )
  {
    mOffsetDist = distance;
  }
  mOffsetUnit = QgsSymbolLayerV2Utils::decodeOutputUnit( props.value( "offset_unit" ) );
  mOffsetMapUnitScale = QgsSymbolLayerV2Utils::decodeMapUnitScale( props.value( "offset_unit_scale" ) );
  double scale = props.value( "scale" ).toDouble( &ok );
  if ( ok )
  {
    mScale = scale;
  }
  if ( props.contains( "color" ) )
  {
    mColor = QgsSymbolLayerV2Utils::decodeColor( props.value( "color" ) );
  }
}

QRectF QgsDropShadowEffect::boundingRect( const QRectF &rect, const QgsRenderContext& context ) const
{
  //offset distance
  double spread = mOffsetDist * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mOffsetUnit, mOffsetMapUnitScale );
  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  spread += mBlurLevel * 2 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}
