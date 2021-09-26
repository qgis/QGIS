/***************************************************************************
                              qgsshadoweffect.cpp
                              -------------------
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

#include "qgsshadoweffect.h"
#include "qgsimageoperation.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

QgsShadowEffect::QgsShadowEffect()
  : mColor( Qt::black )
{

}

void QgsShadowEffect::draw( QgsRenderContext &context )
{
  if ( !source() || !enabled() || !context.painter() )
    return;

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QImage colorisedIm = sourceAsImage( context )->copy();

  if ( context.feedback() && context.feedback()->isCanceled() )
    return;

  QPainter *painter = context.painter();
  const QgsScopedQPainterState painterState( painter );
  painter->setCompositionMode( mBlendMode );

  if ( !exteriorShadow() )
  {
    //inner shadow, first invert the opacity. The color does not matter since we will
    //be replacing it anyway
    colorisedIm.invertPixels( QImage::InvertRgba );
  }

  QgsImageOperation::overlayColor( colorisedIm, mColor );

  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );
  if ( blurLevel <= 16 )
  {
    QgsImageOperation::stackBlur( colorisedIm, blurLevel, false, context.feedback() );
  }
  else
  {
    QImage *imb = QgsImageOperation::gaussianBlur( colorisedIm, blurLevel, context.feedback() );
    if ( !imb->isNull() )
      colorisedIm = QImage( *imb );
    delete imb;
  }

  const double offsetDist = context.convertToPainterUnits( mOffsetDist, mOffsetUnit, mOffsetMapUnitScale );

  const double   angleRad = mOffsetAngle * M_PI / 180; // to radians
  const QPointF transPt( -offsetDist * std::cos( angleRad + M_PI_2 ),
                         -offsetDist * std::sin( angleRad + M_PI_2 ) );

  //transparency, scale
  QgsImageOperation::multiplyOpacity( colorisedIm, mOpacity, context.feedback() );

  if ( !exteriorShadow() )
  {
    //inner shadow, do a bit of painter juggling
    QImage innerShadowIm( colorisedIm.width(), colorisedIm.height(), QImage::Format_ARGB32 );
    innerShadowIm.fill( Qt::transparent );
    QPainter imPainter( &innerShadowIm );

    //draw shadow at offset
    imPainter.drawImage( transPt.x(), transPt.y(), colorisedIm );

    //restrict shadow so it's only drawn on top of original image
    imPainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    imPainter.drawImage( 0, 0, *sourceAsImage( context ) );
    imPainter.end();

    painter->drawImage( imageOffset( context ), innerShadowIm );
  }
  else
  {
    painter->drawImage( imageOffset( context ) + transPt, colorisedIm );
  }
}

QVariantMap QgsShadowEffect::properties() const
{
  QVariantMap props;
  props.insert( QStringLiteral( "enabled" ), mEnabled ? "1" : "0" );
  props.insert( QStringLiteral( "draw_mode" ), QString::number( int( mDrawMode ) ) );
  props.insert( QStringLiteral( "blend_mode" ), QString::number( int( mBlendMode ) ) );
  props.insert( QStringLiteral( "opacity" ), QString::number( mOpacity ) );
  props.insert( QStringLiteral( "blur_level" ), QString::number( mBlurLevel ) );
  props.insert( QStringLiteral( "blur_unit" ), QgsUnitTypes::encodeUnit( mBlurUnit ) );
  props.insert( QStringLiteral( "blur_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mBlurMapUnitScale ) );
  props.insert( QStringLiteral( "offset_angle" ), QString::number( mOffsetAngle ) );
  props.insert( QStringLiteral( "offset_distance" ), QString::number( mOffsetDist ) );
  props.insert( QStringLiteral( "offset_unit" ), QgsUnitTypes::encodeUnit( mOffsetUnit ) );
  props.insert( QStringLiteral( "offset_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale ) );
  props.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  return props;
}

void QgsShadowEffect::readProperties( const QVariantMap &props )
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
  const int angle = props.value( QStringLiteral( "offset_angle" ) ).toInt( &ok );
  if ( ok )
  {
    mOffsetAngle = angle;
  }
  const double distance = props.value( QStringLiteral( "offset_distance" ) ).toDouble( &ok );
  if ( ok )
  {
    mOffsetDist = distance;
  }
  mOffsetUnit = QgsUnitTypes::decodeRenderUnit( props.value( QStringLiteral( "offset_unit" ) ).toString() );
  mOffsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( props.value( QStringLiteral( "offset_unit_scale" ) ).toString() );
  if ( props.contains( QStringLiteral( "color" ) ) )
  {
    mColor = QgsSymbolLayerUtils::decodeColor( props.value( QStringLiteral( "color" ) ).toString() );
  }
}

QRectF QgsShadowEffect::boundingRect( const QRectF &rect, const QgsRenderContext &context ) const
{
  //blur radius and offset distance
  const int blurLevel = std::round( context.convertToPainterUnits( mBlurLevel, mBlurUnit, mBlurMapUnitScale, Qgis::RenderSubcomponentProperty::BlurSize ) );

  // spread is initially the shadow offset size
  double spread = context.convertToPainterUnits( mOffsetDist, mOffsetUnit, mOffsetMapUnitScale, Qgis::RenderSubcomponentProperty::ShadowOffset );

  //plus possible extension due to blur, with a couple of extra pixels thrown in for safety
  spread += blurLevel * 2 + 10;
  return rect.adjusted( -spread, -spread, spread, spread );
}


//
// QgsDropShadowEffect
//

QgsPaintEffect *QgsDropShadowEffect::create( const QVariantMap &map )
{
  QgsDropShadowEffect *effect = new QgsDropShadowEffect();
  effect->readProperties( map );
  return effect;
}

QgsDropShadowEffect::QgsDropShadowEffect()
  : QgsShadowEffect()
{

}

QString QgsDropShadowEffect::type() const
{
  return QStringLiteral( "dropShadow" );
}

QgsDropShadowEffect *QgsDropShadowEffect::clone() const
{
  return new QgsDropShadowEffect( *this );
}

bool QgsDropShadowEffect::exteriorShadow() const
{
  return true;
}


//
// QgsInnerShadowEffect
//

QgsPaintEffect *QgsInnerShadowEffect::create( const QVariantMap &map )
{
  QgsInnerShadowEffect *effect = new QgsInnerShadowEffect();
  effect->readProperties( map );
  return effect;
}

QgsInnerShadowEffect::QgsInnerShadowEffect()
  : QgsShadowEffect()
{

}

QString QgsInnerShadowEffect::type() const
{
  return QStringLiteral( "innerShadow" );
}

QgsInnerShadowEffect *QgsInnerShadowEffect::clone() const
{
  return new QgsInnerShadowEffect( *this );
}

bool QgsInnerShadowEffect::exteriorShadow() const
{
  return false;
}
