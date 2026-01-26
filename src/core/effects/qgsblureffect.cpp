/***************************************************************************
                              qgsblureffect.cpp
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

#include "qgsblureffect.h"

#include "qgsimageoperation.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsPaintEffect *QgsBlurEffect::create( const QVariantMap &map )
{
  QgsBlurEffect *newEffect = new QgsBlurEffect();
  newEffect->readProperties( map );
  return newEffect;
}

Qgis::PaintEffectFlags QgsBlurEffect::flags() const
{
  return Qgis::PaintEffectFlag::RequiresRasterization;
}

void QgsBlurEffect::draw( QgsRenderContext &context )
{
  if ( !enabled() || !context.painter() || source().isNull() )
    return;

  if ( context.rasterizedRenderingPolicy() == Qgis::RasterizedRenderingPolicy::ForceVector )
  {
    //just draw unmodified source, we can't render this effect when forcing vectors
    drawSource( *context.painter() );
    return;
  }

  switch ( mBlurMethod )
  {
    case StackBlur:
      drawStackBlur( context );
      break;
    case GaussianBlur:
      drawGaussianBlur( context );
      break;
  }
}

void QgsBlurEffect::drawStackBlur( QgsRenderContext &context )
{
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );

  QImage im = sourceAsImage( context ).copy();
  QgsImageOperation::stackBlur( im, blurLevel, false, context.feedback() );
  drawBlurredImage( context, im );
}

void QgsBlurEffect::drawGaussianBlur( QgsRenderContext &context )
{
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );

  QImage source = sourceAsImage( context ).copy();
  QImage *im = QgsImageOperation::gaussianBlur( source, blurLevel, context.feedback() );
  if ( !im->isNull() )
    drawBlurredImage( context, *im );
  delete im;
}

void QgsBlurEffect::drawBlurredImage( QgsRenderContext &context, QImage &image )
{
  //opacity
  QgsImageOperation::multiplyOpacity( image, mOpacity, context.feedback() );

  QPainter *painter = context.painter();
  const QgsScopedQPainterState painterState( painter );
  painter->setCompositionMode( mBlendMode );
  painter->drawImage( imageOffset( context ), image );
}

QVariantMap QgsBlurEffect::properties() const
{
  QVariantMap props;
  props.insert( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  props.insert( u"draw_mode"_s, QString::number( static_cast< int >( mDrawMode ) ) );
  props.insert( u"blend_mode"_s, QString::number( static_cast< int >( mBlendMode ) ) );
  props.insert( u"opacity"_s, QString::number( mOpacity ) );
  props.insert( u"blur_level"_s, QString::number( mBlurLevel ) );
  props.insert( u"blur_unit"_s, QgsUnitTypes::encodeUnit( mBlurUnit ) );
  props.insert( u"blur_unit_scale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( mBlurMapUnitScale ) );
  props.insert( u"blur_method"_s, QString::number( static_cast< int >( mBlurMethod ) ) );
  return props;
}

void QgsBlurEffect::readProperties( const QVariantMap &props )
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
  const QgsBlurEffect::BlurMethod method = static_cast< QgsBlurEffect::BlurMethod >( props.value( u"blur_method"_s ).toInt( &ok ) );
  if ( ok )
  {
    mBlurMethod = method;
  }
}

QgsBlurEffect *QgsBlurEffect::clone() const
{
  QgsBlurEffect *newEffect = new QgsBlurEffect( *this );
  return newEffect;
}

QRectF QgsBlurEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );

  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  const double spread = blurLevel * 2.0 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}
