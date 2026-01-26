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

#include <memory>

#include "qgscolorrampimpl.h"
#include "qgscolorutils.h"
#include "qgsimageoperation.h"
#include "qgssymbollayerutils.h"
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

Qgis::PaintEffectFlags QgsGlowEffect::flags() const
{
  return Qgis::PaintEffectFlag::RequiresRasterization;
}

void QgsGlowEffect::draw( QgsRenderContext &context )
{
  if ( !enabled() || !context.painter() || source().isNull() )
    return;

  if ( context.rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::ForceVector )
  {
    //just draw unmodified source, we can't render this effect when forcing vectors
    drawSource( *context.painter() );
    return;
  }

  QImage im = sourceAsImage( context ).copy();

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
    tempRamp = std::make_unique<QgsGradientColorRamp>( mColor, transparentColor );
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
    p.drawImage( 0, 0, sourceAsImage( context ) );
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
  props.insert( u"enabled"_s, mEnabled ? "1" : "0" );
  props.insert( u"draw_mode"_s, QString::number( int( mDrawMode ) ) );
  props.insert( u"blend_mode"_s, QString::number( int( mBlendMode ) ) );
  props.insert( u"opacity"_s, QString::number( mOpacity ) );
  props.insert( u"blur_level"_s, QString::number( mBlurLevel ) );
  props.insert( u"blur_unit"_s, QgsUnitTypes::encodeUnit( mBlurUnit ) );
  props.insert( u"blur_unit_scale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mBlurMapUnitScale ) );
  props.insert( u"spread"_s, QString::number( mSpread ) );
  props.insert( u"spread_unit"_s, QgsUnitTypes::encodeUnit( mSpreadUnit ) );
  props.insert( u"spread_unit_scale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mSpreadMapUnitScale ) );
  props.insert( u"color_type"_s, QString::number( static_cast< int >( mColorType ) ) );
  props.insert( u"single_color"_s, QgsColorUtils::colorToString( mColor ) );

  if ( mRamp )
  {
    props.insert( mRamp->properties() );
  }

  return props;
}

void QgsGlowEffect::readProperties( const QVariantMap &props )
{
  bool ok;
  const QPainter::CompositionMode mode = static_cast< QPainter::CompositionMode >( props.value( u"blend_mode"_s ).toInt( &ok ) );
  if ( ok )
  {
    mBlendMode = mode;
  }
  if ( props.contains( u"transparency"_s ) )
  {
    const double transparency = props.value( u"transparency"_s ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = 1.0 - transparency;
    }
  }
  else
  {
    const double opacity = props.value( u"opacity"_s ).toDouble( &ok );
    if ( ok )
    {
      mOpacity = opacity;
    }
  }
  mEnabled = props.value( u"enabled"_s, u"1"_s ).toInt();
  mDrawMode = static_cast< QgsPaintEffect::DrawMode >( props.value( u"draw_mode"_s, u"2"_s ).toInt() );
  const double level = props.value( u"blur_level"_s ).toDouble( &ok );
  if ( ok )
  {
    mBlurLevel = level;
    if ( !props.contains( u"blur_unit"_s ) )
    {
      // deal with pre blur unit era by assuming 96 dpi and converting pixel values as millimeters
      mBlurLevel *= 0.2645;
    }
  }
  mBlurUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"blur_unit"_s ).toString() );
  mBlurMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"blur_unit_scale"_s ).toString() );
  const double spread = props.value( u"spread"_s ).toDouble( &ok );
  if ( ok )
  {
    mSpread = spread;
  }
  mSpreadUnit = QgsUnitTypes::decodeRenderUnit( props.value( u"spread_unit"_s ).toString() );
  mSpreadMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( u"spread_unit_scale"_s ).toString() );
  const QgsGlowEffect::GlowColorType type = static_cast< QgsGlowEffect::GlowColorType >( props.value( u"color_type"_s ).toInt( &ok ) );
  if ( ok )
  {
    mColorType = type;
  }
  if ( props.contains( u"single_color"_s ) )
  {
    mColor = QgsColorUtils::colorFromString( props.value( u"single_color"_s ).toString() );
  }

  //attempt to create color ramp from props
  delete mRamp;
  if ( props.contains( u"rampType"_s ) && props[u"rampType"_s] == QgsCptCityColorRamp::typeString() )
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
