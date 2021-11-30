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
#include "qgssymbollayerutils.h"
#include "qgsimageoperation.h"
#include "qgscolorrampimpl.h"
#include "qgsunittypes.h"

QgsGlowEffect::QgsGlowEffect()
  : mColor( Qt::white )
{

}

QgsGlowEffect::QgsGlowEffect( const QgsGlowEffect &other )
  : QgsPaintEffect( other )
{
  operator=( other );
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

  QgsColorRamp *ramp = nullptr;
  std::unique_ptr< QgsGradientColorRamp > tempRamp;
  if ( mColorType == ColorRamp && mRamp )
  {
    ramp = mRamp;
  }
  else
  {
    //create a temporary ramp
    QColor transparentColor = mColor;
    transparentColor.setAlpha( 0 );
    tempRamp.reset( new QgsGradientColorRamp( mColor, transparentColor ) );
    ramp = tempRamp.get();
  }

  QgsImageOperation::DistanceTransformProperties dtProps;
  dtProps.spread = context.convertToPainterUnits( mSpread, mSpreadUnit, mSpreadMapUnitScale, Qgis::RenderSubcomponentProperty::GlowSpread );
  dtProps.useMaxDistance = false;
  dtProps.shadeExterior = shadeExterior();
  dtProps.ramp = ramp;
  QgsImageOperation::distanceTransform( im, dtProps, context.feedback() );

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );
  if ( blurLevel <= 16 )
  {
    QgsImageOperation::stackBlur( im, blurLevel, false, context.feedback() );
  }
  else
  {
    QImage *imb = QgsImageOperation::gaussianBlur( im, blurLevel, context.feedback() );
    if ( !imb->isNull() )
      im = QImage( *imb );
    delete imb;
  }

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QgsImageOperation::multiplyOpacity( im, mOpacity, context.feedback() );

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  if ( !shadeExterior() )
  {
    //only keep interior portion
    QPainter p( &im );
    p.setRenderHint( QPainter::Antialiasing );
    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    p.drawImage( 0, 0, *sourceAsImage( context ) );
    p.end();
  }

  QPainter *painter = context.painter();
  const QgsScopedQPainterState painterState( painter );
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), im );
}

QVariantMap QgsGlowEffect::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( int( mDrawMode ) ) );
  props.insert( QStringLiteral( "blend_mode" ), QString::number( int( mBlendMode ) ) );
  props.insert( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  props.insert( QStringLiteral( "blur_level" ), QString::number( mBlurLevel ) );
  props.insert( QStringLiteral( "blur_unit" ), QgsUnitTypes::encodeUnit( mBlurUnit ) );
  props.insert( QStringLiteral( "blur_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mBlurMapUnitScale ) );
  props.insert( QStringLiteral( "spread" ), QString::number( mSpread ) );
  props.insert( QStringLiteral( "spread_unit" ), QgsUnitTypes::encodeUnit( mSpreadUnit ) );
  props.insert( QStringLiteral( "spread_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mSpreadMapUnitScale ) );
  props.insert( QStringLiteral( "color_type" ), QString::number( static_cast< int >( mColorType ) ) );
  props.insert( QStringLiteral( "single_color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );

  if ( mRamp )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    props.unite( mRamp->properties() );
#else
    props.insert( mRamp->properties() );
#endif
  }

  return props;
}

void QgsGlowEffect::readProperties( const QVariantMap &props )
{
  bool ok;
  const QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( QStringLiteral( "blend_mode" ) ).toInt( &ok ) );
  if ( ok )
  {
    mBlendMode = mode;
  }
  if ( props.contains( QStringLiteral( "transparency" ) ) )
  {
    const double transparency = props.value( QStringLiteral( "transparency" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = 1.0 - transparency;
    }
  }
  else
  {
    const double opacity = props.value( QStringLiteral( "opacity" ) ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = opacity;
    }
  }
  mEnabled = props.value( QStringLiteral( "enabled" ), QStringLiteral( "1" ) ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( QStringLiteral( "draw_mode" ), QStringLiteral( "2" ) ).toInt() );
  const double level = props.value( QStringLiteral( "blur_level" ) ).toDouble( &ok );
  if ( ok )
  {
    mBlurLevel = level;
    if ( !props.contains( QStringLiteral( "blur_unit" ) ) )
    {
      // deal with pre blur unit era by assuming 96 dpi and converting pixel values as millimeters
      mBlurLevel *= 0.2645;
    }
  }
  mBlurUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "blur_unit" ) ).toString() );
  mBlurMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "blur_unit_scale" ) ).toString() );
  const double spread = props.value( QStringLiteral( "spread" ) ).toDouble( &ok );
  if ( ok )
  {
    mSpread = spread;
  }
  mSpreadUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "spread_unit" ) ).toString() );
  mSpreadMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "spread_unit_scale" ) ).toString() );
  const QgsGlowEffect::GlowColorType type = static_cast< QgsGlowEffect::GlowColorType >( props.value( QStringLiteral( "color_type" ) ).toInt( &ok ) );
  if ( ok )
  {
    mColorType = type;
  }
  if ( props.contains( QStringLiteral( "single_color" ) ) )
  {
    mColor = QgsSymbolLayerUtils::decodeColor( props.value( QStringLiteral( "single_color" ) ).toString() );
  }

  //attempt to create color ramp from props
  delete mRamp;
  if ( props.contains( QStringLiteral( "rampType" ) ) && props[QStringLiteral( "rampType" )] == QgsCptCityColorRamp::typeString() )
  {
    mRamp = QgsCptCityColorRamp::create( props );
  }
  else
  {
    mRamp = QgsGradientColorRamp::create( props );
  }
}

void QgsGlowEffect::setRamp( QgsColorRamp *ramp )
{
  delete mRamp;
  mRamp = ramp;
}

QgsGlowEffect &QgsGlowEffect::operator=( const QgsGlowEffect &rhs )
{
  if ( &rhs == this )
    return *this;

  delete mRamp;

  mSpread = rhs.spread();
  mSpreadUnit = rhs.spreadUnit();
  mSpreadMapUnitScale = rhs.spreadMapUnitScale();
  mRamp = rhs.ramp() ? rhs.ramp()->clone() : nullptr;
  mBlurLevel = rhs.blurLevel();
  mBlurUnit = rhs.mBlurUnit;
  mBlurMapUnitScale = rhs.mBlurMapUnitScale;
  mOpacity = rhs.opacity();
  mColor = rhs.color();
  mBlendMode = rhs.blendMode();
  mColorType = rhs.colorType();

  return *this;
}

QRectF QgsGlowEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  //blur radius and spread size
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );
  double spread = context.convertToPainterUnits( mSpread, mSpreadUnit, mSpreadMapUnitScale, Qgis::RenderSubcomponentProperty::GlowSpread );

  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  spread += blurLevel * 2 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}


//
// QgsOuterGlowEffect
//

QgsOuterGlowEffect::QgsOuterGlowEffect()
  : QgsGlowEffect()
{

}

QgsPaintEffect *QgsOuterGlowEffect::create( const QVariantMap &map )
{
  QgsOuterGlowEffect *effect = new QgsOuterGlowEffect();
  effect->readProperties( map );
  return effect;
}

QgsOuterGlowEffect *QgsOuterGlowEffect::clone() const
{
  QgsOuterGlowEffect *newEffect = new QgsOuterGlowEffect( *this );
  return newEffect;
}


//
// QgsInnerGlowEffect
//

QgsInnerGlowEffect::QgsInnerGlowEffect()
  : QgsGlowEffect()
{

}

QgsPaintEffect *QgsInnerGlowEffect::create( const QVariantMap &map )
{
  QgsInnerGlowEffect *effect = new QgsInnerGlowEffect();
  effect->readProperties( map );
  return effect;
}

QgsInnerGlowEffect *QgsInnerGlowEffect::clone() const
{
  QgsInnerGlowEffect *newEffect = new QgsInnerGlowEffect( *this );
  return newEffect;
}
