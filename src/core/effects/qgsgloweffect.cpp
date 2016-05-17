/***************************************************************************
                              qgsgloweffect.cpp
                              -----------------
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

#include "qgsgloweffect.h"
#include "qgssymbollayerv2utils.h"
#include "qgsimageoperation.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsunittypes.h"

QgsGlowEffect::QgsGlowEffect()
    : QgsPaintEffect()
    , mSpread( 2.0 )
    , mSpreadUnit( QgsSymbolV2::MM )
    , mRamp( nullptr )
    , mBlurLevel( 3 )
    , mTransparency( 0.5 )
    , mColor( Qt::white )
    , mBlendMode( QPainter::CompositionMode_SourceOver )
    , mColorType( SingleColor )
{

}

QgsGlowEffect::QgsGlowEffect( const QgsGlowEffect &other )
    : QgsPaintEffect( other )
    , mSpread( other.spread() )
    , mSpreadUnit( other.spreadUnit() )
    , mSpreadMapUnitScale( other.spreadMapUnitScale() )
    , mRamp( nullptr )
    , mBlurLevel( other.blurLevel() )
    , mTransparency( other.transparency() )
    , mColor( other.color() )
    , mBlendMode( other.blendMode() )
    , mColorType( other.colorType() )
{
  if ( other.ramp() )
  {
    mRamp = other.ramp()->clone();
  }
}

QgsGlowEffect::~QgsGlowEffect()
{
  delete mRamp;
}

void QgsGlowEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  QImage im = sourceAsImage( context )->copy();

  QgsVectorColorRampV2* ramp = nullptr;
  if ( mColorType == ColorRamp && mRamp )
  {
    ramp = mRamp;
  }
  else
  {
    //create a temporary ramp
    QColor transparentColor = mColor;
    transparentColor.setAlpha( 0 );
    ramp = new QgsVectorGradientColorRampV2( mColor, transparentColor );
  }

  QgsImageOperation::DistanceTransformProperties dtProps;
  dtProps.spread = mSpread * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mSpreadUnit, mSpreadMapUnitScale );
  dtProps.useMaxDistance = false;
  dtProps.shadeExterior = shadeExterior();
  dtProps.ramp = ramp;
  QgsImageOperation::distanceTransform( im, dtProps );

  if ( mBlurLevel > 0 )
  {
    QgsImageOperation::stackBlur( im, mBlurLevel );
  }

  QgsImageOperation::multiplyOpacity( im, 1.0 - mTransparency );

  if ( !shadeExterior() )
  {
    //only keep interior portion
    QPainter p( &im );
    p.setRenderHint( QPainter::Antialiasing );
    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    p.drawImage( 0, 0, *sourceAsImage( context ) );
    p.end();
  }

  QPainter* painter = context.painter();
  painter->save();
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), im );
  painter->restore();

  if ( !mRamp )
  {
    //delete temporary ramp
    delete ramp;
  }
}

QgsStringMap QgsGlowEffect::properties() const
{
  QgsStringMap props;
  props.insert( "enabled", mEnabled ? "1" : "0" );
  props.insert( "draw_mode", QString::number( int( mDrawMode ) ) );
  props.insert( "blend_mode", QString::number( int( mBlendMode ) ) );
  props.insert( "transparency", QString::number( mTransparency ) );
  props.insert( "blur_level", QString::number( mBlurLevel ) );
  props.insert( "spread", QString::number( mSpread ) );
  props.insert( "spread_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mSpreadUnit ) );
  props.insert( "spread_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mSpreadMapUnitScale ) );
  props.insert( "color_type", QString::number( static_cast< int >( mColorType ) ) );
  props.insert( "single_color", QgsSymbolLayerV2Utils::encodeColor( mColor ) );

  if ( mRamp )
  {
    props.unite( mRamp->properties() );
  }

  return props;
}

void QgsGlowEffect::readProperties( const QgsStringMap &props )
{
  bool ok;
  QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( "blend_mode" ).toInt( &ok ) );
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
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( "draw_mode", "2" ).toInt() );
  int level = props.value( "blur_level" ).toInt( &ok );
  if ( ok )
  {
    mBlurLevel = level;
  }
  double spread = props.value( "spread" ).toDouble( &ok );
  if ( ok )
  {
    mSpread = spread;
  }
  mSpreadUnit = QgsSymbolLayerV2Utils::decodeOutputUnit( props.value( "spread_unit" ) );
  mSpreadMapUnitScale = QgsSymbolLayerV2Utils::decodeMapUnitScale( props.value( "spread_unit_scale" ) );
  QgsGlowEffect::GlowColorType type = static_cast< QgsGlowEffect::GlowColorType >( props.value( "color_type" ).toInt( &ok ) );
  if ( ok )
  {
    mColorType = type;
  }
  if ( props.contains( "single_color" ) )
  {
    mColor = QgsSymbolLayerV2Utils::decodeColor( props.value( "single_color" ) );
  }

  //attempt to create color ramp from props
  delete mRamp;
  mRamp = QgsVectorGradientColorRampV2::create( props );
}

void QgsGlowEffect::setRamp( QgsVectorColorRampV2 *ramp )
{
  delete mRamp;
  mRamp = ramp;
}

QgsGlowEffect &QgsGlowEffect::operator=( const QgsGlowEffect & rhs )
{
  if ( &rhs == this )
    return *this;

  delete mRamp;

  mSpread = rhs.spread();
  mRamp = rhs.ramp() ? rhs.ramp()->clone() : nullptr;
  mBlurLevel = rhs.blurLevel();
  mTransparency = rhs.transparency();
  mColor = rhs.color();
  mBlendMode = rhs.blendMode();
  mColorType = rhs.colorType();

  return *this;
}

QRectF QgsGlowEffect::boundingRect( const QRectF &rect, const QgsRenderContext& context ) const
{
  //spread size
  double spread = mSpread * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mSpreadUnit, mSpreadMapUnitScale );
  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  spread += mBlurLevel * 2 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}


//
// QgsOuterGlowEffect
//

QgsOuterGlowEffect::QgsOuterGlowEffect()
    : QgsGlowEffect()
{

}

QgsOuterGlowEffect::~QgsOuterGlowEffect()
{

}

QgsPaintEffect *QgsOuterGlowEffect::create( const QgsStringMap &map )
{
  QgsOuterGlowEffect* effect = new QgsOuterGlowEffect();
  effect->readProperties( map );
  return effect;
}

QgsOuterGlowEffect* QgsOuterGlowEffect::clone() const
{
  QgsOuterGlowEffect* newEffect = new QgsOuterGlowEffect( *this );
  return newEffect;
}


//
// QgsInnerGlowEffect
//

QgsInnerGlowEffect::QgsInnerGlowEffect()
    : QgsGlowEffect()
{

}

QgsInnerGlowEffect::~QgsInnerGlowEffect()
{

}

QgsPaintEffect *QgsInnerGlowEffect::create( const QgsStringMap &map )
{
  QgsInnerGlowEffect* effect = new QgsInnerGlowEffect();
  effect->readProperties( map );
  return effect;
}

QgsInnerGlowEffect* QgsInnerGlowEffect::clone() const
{
  QgsInnerGlowEffect* newEffect = new QgsInnerGlowEffect( *this );
  return newEffect;
}
