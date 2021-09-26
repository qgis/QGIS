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

QgsPaintEffect *QgsBlurEffect::create( const QVariantMap &map )
{
  QgsBlurEffect *newEffect = new QgsBlurEffect();
  newEffect->readProperties( map );
  return newEffect;
}

void QgsBlurEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

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

  QImage im = sourceAsImage( context )->copy();
  QgsImageOperation::stackBlur( im, blurLevel, false, context.feedback() );
  drawBlurredImage( context, im );
}

void QgsBlurEffect::drawGaussianBlur( QgsRenderContext &context )
{
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );

  QImage *im = QgsImageOperation::gaussianBlur( *sourceAsImage( context ), blurLevel, context.feedback() );
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
  props.insert( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( static_cast< int >( mDrawMode ) ) );
  props.insert( QStringLiteral( "blend_mode" ), QString::number( static_cast< int >( mBlendMode ) ) );
  props.insert( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  props.insert( QStringLiteral( "blur_level" ), QString::number( mBlurLevel ) );
  props.insert( QStringLiteral( "blur_unit" ), QgsUnitTypes::encodeUnit( mBlurUnit ) );
  props.insert( QStringLiteral( "blur_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mBlurMapUnitScale ) );
  props.insert( QStringLiteral( "blur_method" ), QString::number( static_cast< int >( mBlurMethod ) ) );
  return props;
}

void QgsBlurEffect::readProperties( const QVariantMap &props )
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
  const QgsBlurEffect::BlurMethod method = static_cast< QgsBlurEffect::BlurMethod >( props.value( QStringLiteral( "blur_method" ) ).toInt( &ok ) );
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
