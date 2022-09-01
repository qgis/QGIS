/***************************************************************************
  qgstextshadowsettings.cpp
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextshadowsettings.h"
#include "qgstextrenderer_p.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgspainting.h"
#include "qgspallabeling.h"
#include "qgstextrendererutils.h"

QgsTextShadowSettings::QgsTextShadowSettings()
{
  d = new QgsTextShadowSettingsPrivate();
}

QgsTextShadowSettings::QgsTextShadowSettings( const QgsTextShadowSettings &other ) //NOLINT
  : d( other.d )
{

}

QgsTextShadowSettings &QgsTextShadowSettings::operator=( const QgsTextShadowSettings &other )  //NOLINT
{
  d = other.d;
  return *this;
}

QgsTextShadowSettings::~QgsTextShadowSettings() //NOLINT
{

}

bool QgsTextShadowSettings::operator==( const QgsTextShadowSettings &other ) const
{
  if ( d->enabled != other.enabled()
       || d->shadowUnder != other.shadowPlacement()
       || d->offsetAngle != other.offsetAngle()
       || d->offsetDist != other.offsetDistance()
       || d->offsetUnits != other.offsetUnit()
       || d->offsetMapUnitScale != other.offsetMapUnitScale()
       || d->offsetGlobal != other.offsetGlobal()
       || d->radius != other.blurRadius()
       || d->radiusUnits != other.blurRadiusUnit()
       || d->radiusMapUnitScale != other.blurRadiusMapUnitScale()
       || d->radiusAlphaOnly != other.blurAlphaOnly()
       || d->scale != other.scale()
       || d->color != other.color()
       || d->opacity != other.opacity()
       || d->blendMode != other.blendMode() )
    return false;

  return true;
}

bool QgsTextShadowSettings::operator!=( const QgsTextShadowSettings &other ) const
{
  return !( *this == other );
}

bool QgsTextShadowSettings::enabled() const
{
  return d->enabled;
}

void QgsTextShadowSettings::setEnabled( bool enabled )
{
  d->enabled = enabled;
}

QgsTextShadowSettings::ShadowPlacement QgsTextShadowSettings::shadowPlacement() const
{
  return d->shadowUnder;
}

void QgsTextShadowSettings::setShadowPlacement( QgsTextShadowSettings::ShadowPlacement placement )
{
  d->shadowUnder = placement;
}

int QgsTextShadowSettings::offsetAngle() const
{
  return d->offsetAngle;
}

void QgsTextShadowSettings::setOffsetAngle( int angle )
{
  d->offsetAngle = angle;
}

double QgsTextShadowSettings::offsetDistance() const
{
  return d->offsetDist;
}

void QgsTextShadowSettings::setOffsetDistance( double distance )
{
  d->offsetDist = distance;
}

QgsUnitTypes::RenderUnit QgsTextShadowSettings::offsetUnit() const
{
  return d->offsetUnits;
}

void QgsTextShadowSettings::setOffsetUnit( QgsUnitTypes::RenderUnit units )
{
  d->offsetUnits = units;
}

QgsMapUnitScale QgsTextShadowSettings::offsetMapUnitScale() const
{
  return d->offsetMapUnitScale;
}

void QgsTextShadowSettings::setOffsetMapUnitScale( const QgsMapUnitScale &scale )
{
  d->offsetMapUnitScale = scale;
}

bool QgsTextShadowSettings::offsetGlobal() const
{
  return d->offsetGlobal;
}

void QgsTextShadowSettings::setOffsetGlobal( bool global )
{
  d->offsetGlobal = global;
}

double QgsTextShadowSettings::blurRadius() const
{
  return d->radius;
}

void QgsTextShadowSettings::setBlurRadius( double radius )
{
  d->radius = radius;
}

QgsUnitTypes::RenderUnit QgsTextShadowSettings::blurRadiusUnit() const
{
  return d->radiusUnits;
}

void QgsTextShadowSettings::setBlurRadiusUnit( QgsUnitTypes::RenderUnit units )
{
  d->radiusUnits = units;
}

QgsMapUnitScale QgsTextShadowSettings::blurRadiusMapUnitScale() const
{
  return d->radiusMapUnitScale;
}

void QgsTextShadowSettings::setBlurRadiusMapUnitScale( const QgsMapUnitScale &scale )
{
  d->radiusMapUnitScale = scale;
}

bool QgsTextShadowSettings::blurAlphaOnly() const
{
  return d->radiusAlphaOnly;
}

void QgsTextShadowSettings::setBlurAlphaOnly( bool alphaOnly )
{
  d->radiusAlphaOnly = alphaOnly;
}

double QgsTextShadowSettings::opacity() const
{
  return d->opacity;
}

void QgsTextShadowSettings::setOpacity( double opacity )
{
  d->opacity = opacity;
}

int QgsTextShadowSettings::scale() const
{
  return d->scale;
}

void QgsTextShadowSettings::setScale( int scale )
{
  d->scale = scale;
}

QColor QgsTextShadowSettings::color() const
{
  return d->color;
}

void QgsTextShadowSettings::setColor( const QColor &color )
{
  d->color = color;
}

QPainter::CompositionMode QgsTextShadowSettings::blendMode() const
{
  return d->blendMode;
}

void QgsTextShadowSettings::setBlendMode( QPainter::CompositionMode mode )
{
  d->blendMode = mode;
}

void QgsTextShadowSettings::readFromLayer( QgsVectorLayer *layer )
{
  d->enabled = layer->customProperty( QStringLiteral( "labeling/shadowDraw" ), QVariant( false ) ).toBool();
  d->shadowUnder = static_cast< ShadowPlacement >( layer->customProperty( QStringLiteral( "labeling/shadowUnder" ), QVariant( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = layer->customProperty( QStringLiteral( "labeling/shadowOffsetAngle" ), QVariant( 135 ) ).toInt();
  d->offsetDist = layer->customProperty( QStringLiteral( "labeling/shadowOffsetDist" ), QVariant( 1.0 ) ).toDouble();

  if ( layer->customProperty( QStringLiteral( "labeling/shadowOffsetUnit" ) ).toString().isEmpty() )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shadowOffsetUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shadowOffsetUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitMinScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitScale" ) ).toString() );
  }
  d->offsetGlobal = layer->customProperty( QStringLiteral( "labeling/shadowOffsetGlobal" ), QVariant( true ) ).toBool();
  d->radius = layer->customProperty( QStringLiteral( "labeling/shadowRadius" ), QVariant( 1.5 ) ).toDouble();

  if ( layer->customProperty( QStringLiteral( "labeling/shadowRadiusUnit" ) ).toString().isEmpty() )
  {
    d->radiusUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shadowRadiusUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shadowRadiusUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitMinScale" ), 0.0 ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitMaxScale" ), 0.0 ).toDouble();
    d->radiusMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitScale" ) ).toString() );
  }
  d->radiusAlphaOnly = layer->customProperty( QStringLiteral( "labeling/shadowRadiusAlphaOnly" ), QVariant( false ) ).toBool();

  if ( layer->customProperty( QStringLiteral( "labeling/shadowOpacity" ) ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( QStringLiteral( "labeling/shadowTransparency" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( QStringLiteral( "labeling/shadowOpacity" ) ).toDouble() );
  }
  d->scale = layer->customProperty( QStringLiteral( "labeling/shadowScale" ), QVariant( 100 ) ).toInt();
  d->color = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/shadowColor" ), Qt::black, false );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/shadowBlendMode" ), QVariant( QgsPainting::BlendMultiply ) ).toUInt() ) );
}

void QgsTextShadowSettings::readXml( const QDomElement &elem )
{
  const QDomElement shadowElem = elem.firstChildElement( QStringLiteral( "shadow" ) );
  d->enabled = shadowElem.attribute( QStringLiteral( "shadowDraw" ), QStringLiteral( "0" ) ).toInt();
  d->shadowUnder = static_cast< ShadowPlacement >( shadowElem.attribute( QStringLiteral( "shadowUnder" ), QString::number( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = shadowElem.attribute( QStringLiteral( "shadowOffsetAngle" ), QStringLiteral( "135" ) ).toInt();
  d->offsetDist = shadowElem.attribute( QStringLiteral( "shadowOffsetDist" ), QStringLiteral( "1" ) ).toDouble();

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowOffsetUnit" ) ) )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( shadowElem.attribute( QStringLiteral( "shadowOffsetUnits" ) ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( QStringLiteral( "shadowOffsetUnit" ) ) );
  }

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowOffsetMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = shadowElem.attribute( QStringLiteral( "shadowOffsetMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = shadowElem.attribute( QStringLiteral( "shadowOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( QStringLiteral( "shadowOffsetMapUnitScale" ) ) );
  }
  d->offsetGlobal = shadowElem.attribute( QStringLiteral( "shadowOffsetGlobal" ), QStringLiteral( "1" ) ).toInt();
  d->radius = shadowElem.attribute( QStringLiteral( "shadowRadius" ), QStringLiteral( "1.5" ) ).toDouble();

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowRadiusUnit" ) ) )
  {
    d->radiusUnits = QgsTextRendererUtils::convertFromOldLabelUnit( shadowElem.attribute( QStringLiteral( "shadowRadiusUnits" ) ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( QStringLiteral( "shadowRadiusUnit" ) ) );
  }
  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowRadiusMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = shadowElem.attribute( QStringLiteral( "shadowRadiusMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = shadowElem.attribute( QStringLiteral( "shadowRadiusMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiusMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( QStringLiteral( "shadowRadiusMapUnitScale" ) ) );
  }
  d->radiusAlphaOnly = shadowElem.attribute( QStringLiteral( "shadowRadiusAlphaOnly" ), QStringLiteral( "0" ) ).toInt();

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowOpacity" ) ) )
  {
    d->opacity = ( 1 - shadowElem.attribute( QStringLiteral( "shadowTransparency" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( shadowElem.attribute( QStringLiteral( "shadowOpacity" ) ).toDouble() );
  }
  d->scale = shadowElem.attribute( QStringLiteral( "shadowScale" ), QStringLiteral( "100" ) ).toInt();
  d->color = QgsSymbolLayerUtils::decodeColor( shadowElem.attribute( QStringLiteral( "shadowColor" ), QgsSymbolLayerUtils::encodeColor( Qt::black ) ) );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( shadowElem.attribute( QStringLiteral( "shadowBlendMode" ), QString::number( QgsPainting::BlendMultiply ) ).toUInt() ) );
}

QDomElement QgsTextShadowSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement shadowElem = doc.createElement( QStringLiteral( "shadow" ) );
  shadowElem.setAttribute( QStringLiteral( "shadowDraw" ), d->enabled );
  shadowElem.setAttribute( QStringLiteral( "shadowUnder" ), static_cast< unsigned int >( d->shadowUnder ) );
  shadowElem.setAttribute( QStringLiteral( "shadowOffsetAngle" ), d->offsetAngle );
  shadowElem.setAttribute( QStringLiteral( "shadowOffsetDist" ), d->offsetDist );
  shadowElem.setAttribute( QStringLiteral( "shadowOffsetUnit" ), QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  shadowElem.setAttribute( QStringLiteral( "shadowOffsetMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  shadowElem.setAttribute( QStringLiteral( "shadowOffsetGlobal" ), d->offsetGlobal );
  shadowElem.setAttribute( QStringLiteral( "shadowRadius" ), d->radius );
  shadowElem.setAttribute( QStringLiteral( "shadowRadiusUnit" ), QgsUnitTypes::encodeUnit( d->radiusUnits ) );
  shadowElem.setAttribute( QStringLiteral( "shadowRadiusMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->radiusMapUnitScale ) );
  shadowElem.setAttribute( QStringLiteral( "shadowRadiusAlphaOnly" ), d->radiusAlphaOnly );
  shadowElem.setAttribute( QStringLiteral( "shadowOpacity" ), d->opacity );
  shadowElem.setAttribute( QStringLiteral( "shadowScale" ), d->scale );
  shadowElem.setAttribute( QStringLiteral( "shadowColor" ), QgsSymbolLayerUtils::encodeColor( d->color ) );
  shadowElem.setAttribute( QStringLiteral( "shadowBlendMode" ), QgsPainting::getBlendModeEnum( d->blendMode ) );
  return shadowElem;
}

void QgsTextShadowSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::ShadowDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::ShadowDraw, context.expressionContext(), d->enabled );
  }

  // data defined shadow under type?
  QVariant exprVal = properties.value( QgsPalLayerSettings::ShadowUnder, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString str = exprVal.toString().trimmed();
    if ( !str.isEmpty() )
    {
      d->shadowUnder = QgsTextRendererUtils::decodeShadowPlacementType( str );
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowOffsetAngle ) )
  {
    context.expressionContext().setOriginalValueVariable( d->offsetAngle );
    d->offsetAngle = properties.valueAsInt( QgsPalLayerSettings::ShadowOffsetAngle, context.expressionContext(), d->offsetAngle );
  }
  if ( properties.isActive( QgsPalLayerSettings::ShadowOffsetDist ) )
  {
    context.expressionContext().setOriginalValueVariable( d->offsetDist );
    d->offsetDist = properties.valueAsDouble( QgsPalLayerSettings::ShadowOffsetDist, context.expressionContext(), d->offsetDist );
  }

  exprVal = properties.value( QgsPalLayerSettings::ShadowOffsetUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->offsetUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowRadius ) )
  {
    context.expressionContext().setOriginalValueVariable( d->radius );
    d->radius = properties.valueAsDouble( QgsPalLayerSettings::ShadowRadius, context.expressionContext(), d->radius );
  }

  exprVal = properties.value( QgsPalLayerSettings::ShadowRadiusUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiusUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::ShadowOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !QgsVariantUtils::isNull( val ) )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowScale ) )
  {
    context.expressionContext().setOriginalValueVariable( d->scale );
    d->scale = properties.valueAsInt( QgsPalLayerSettings::ShadowScale, context.expressionContext(), d->scale );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->color ) );
    d->color = properties.valueAsColor( QgsPalLayerSettings::ShadowColor, context.expressionContext(), d->color );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::ShadowBlendMode, context.expressionContext() );
    const QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }
}

QSet<QString> QgsTextShadowSettings::referencedFields( const QgsRenderContext & ) const
{
  return QSet< QString >(); // nothing for now
}
