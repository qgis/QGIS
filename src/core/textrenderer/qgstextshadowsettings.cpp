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

#include "qgscolorutils.h"
#include "qgspainting.h"
#include "qgspallabeling.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer_p.h"
#include "qgstextrendererutils.h"
#include "qgsunittypes.h"
#include "qgsvectorlayer.h"

QgsTextShadowSettings::QgsTextShadowSettings()
{
  d = new QgsTextShadowSettingsPrivate();
}

QgsTextShadowSettings::QgsTextShadowSettings( const QgsTextShadowSettings &other ) //NOLINT
  : d( other.d )
{

}

QgsTextShadowSettings::QgsTextShadowSettings( QgsTextShadowSettings &&other ) //NOLINT
  : d( std::move( other.d ) )
{

}

QgsTextShadowSettings &QgsTextShadowSettings::operator=( const QgsTextShadowSettings &other )  //NOLINT
{
  if ( &other == this )
    return *this;

  d = other.d;
  return *this;
}

QgsTextShadowSettings &QgsTextShadowSettings::operator=( QgsTextShadowSettings &&other )  //NOLINT
{
  if ( &other == this )
    return *this;

  d = std::move( other.d );
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

Qgis::RenderUnit QgsTextShadowSettings::offsetUnit() const
{
  return d->offsetUnits;
}

void QgsTextShadowSettings::setOffsetUnit( Qgis::RenderUnit units )
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

Qgis::RenderUnit QgsTextShadowSettings::blurRadiusUnit() const
{
  return d->radiusUnits;
}

void QgsTextShadowSettings::setBlurRadiusUnit( Qgis::RenderUnit units )
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
  d->enabled = layer->customProperty( u"labeling/shadowDraw"_s, QVariant( false ) ).toBool();
  d->shadowUnder = static_cast< ShadowPlacement >( layer->customProperty( u"labeling/shadowUnder"_s, QVariant( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = layer->customProperty( u"labeling/shadowOffsetAngle"_s, QVariant( 135 ) ).toInt();
  d->offsetDist = layer->customProperty( u"labeling/shadowOffsetDist"_s, QVariant( 1.0 ) ).toDouble();

  if ( layer->customProperty( u"labeling/shadowOffsetUnit"_s ).toString().isEmpty() )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shadowOffsetUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shadowOffsetUnit"_s ).toString() );
  }
  if ( layer->customProperty( u"labeling/shadowOffsetMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shadowOffsetMapUnitMinScale"_s, 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shadowOffsetMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shadowOffsetMapUnitScale"_s ).toString() );
  }
  d->offsetGlobal = layer->customProperty( u"labeling/shadowOffsetGlobal"_s, QVariant( true ) ).toBool();
  d->radius = layer->customProperty( u"labeling/shadowRadius"_s, QVariant( 1.5 ) ).toDouble();

  if ( layer->customProperty( u"labeling/shadowRadiusUnit"_s ).toString().isEmpty() )
  {
    d->radiusUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shadowRadiusUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shadowRadiusUnit"_s ).toString() );
  }
  if ( layer->customProperty( u"labeling/shadowRadiusMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shadowRadiusMapUnitMinScale"_s, 0.0 ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shadowRadiusMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->radiusMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shadowRadiusMapUnitScale"_s ).toString() );
  }
  d->radiusAlphaOnly = layer->customProperty( u"labeling/shadowRadiusAlphaOnly"_s, QVariant( false ) ).toBool();

  if ( layer->customProperty( u"labeling/shadowOpacity"_s ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( u"labeling/shadowTransparency"_s ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( u"labeling/shadowOpacity"_s ).toDouble() );
  }
  d->scale = layer->customProperty( u"labeling/shadowScale"_s, QVariant( 100 ) ).toInt();
  d->color = QgsTextRendererUtils::readColor( layer, u"labeling/shadowColor"_s, Qt::black, false );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< Qgis::BlendMode >( layer->customProperty( u"labeling/shadowBlendMode"_s, QVariant( static_cast< int >( Qgis::BlendMode::Multiply ) ) ).toUInt() ) );
}

void QgsTextShadowSettings::readXml( const QDomElement &elem )
{
  const QDomElement shadowElem = elem.firstChildElement( u"shadow"_s );
  d->enabled = shadowElem.attribute( u"shadowDraw"_s, u"0"_s ).toInt();
  d->shadowUnder = static_cast< ShadowPlacement >( shadowElem.attribute( u"shadowUnder"_s, QString::number( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = shadowElem.attribute( u"shadowOffsetAngle"_s, u"135"_s ).toInt();
  d->offsetDist = shadowElem.attribute( u"shadowOffsetDist"_s, u"1"_s ).toDouble();

  if ( !shadowElem.hasAttribute( u"shadowOffsetUnit"_s ) )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( shadowElem.attribute( u"shadowOffsetUnits"_s ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( u"shadowOffsetUnit"_s ) );
  }

  if ( !shadowElem.hasAttribute( u"shadowOffsetMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = shadowElem.attribute( u"shadowOffsetMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = shadowElem.attribute( u"shadowOffsetMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( u"shadowOffsetMapUnitScale"_s ) );
  }
  d->offsetGlobal = shadowElem.attribute( u"shadowOffsetGlobal"_s, u"1"_s ).toInt();
  d->radius = shadowElem.attribute( u"shadowRadius"_s, u"1.5"_s ).toDouble();

  if ( !shadowElem.hasAttribute( u"shadowRadiusUnit"_s ) )
  {
    d->radiusUnits = QgsTextRendererUtils::convertFromOldLabelUnit( shadowElem.attribute( u"shadowRadiusUnits"_s ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( u"shadowRadiusUnit"_s ) );
  }
  if ( !shadowElem.hasAttribute( u"shadowRadiusMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = shadowElem.attribute( u"shadowRadiusMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = shadowElem.attribute( u"shadowRadiusMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->radiusMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( u"shadowRadiusMapUnitScale"_s ) );
  }
  d->radiusAlphaOnly = shadowElem.attribute( u"shadowRadiusAlphaOnly"_s, u"0"_s ).toInt();

  if ( !shadowElem.hasAttribute( u"shadowOpacity"_s ) )
  {
    d->opacity = ( 1 - shadowElem.attribute( u"shadowTransparency"_s ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( shadowElem.attribute( u"shadowOpacity"_s ).toDouble() );
  }
  d->scale = shadowElem.attribute( u"shadowScale"_s, u"100"_s ).toInt();
  d->color = QgsColorUtils::colorFromString( shadowElem.attribute( u"shadowColor"_s, QgsColorUtils::colorToString( Qt::black ) ) );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< Qgis::BlendMode >( shadowElem.attribute( u"shadowBlendMode"_s, QString::number( static_cast<int>( Qgis::BlendMode::Multiply ) ) ).toUInt() ) );
}

QDomElement QgsTextShadowSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement shadowElem = doc.createElement( u"shadow"_s );
  shadowElem.setAttribute( u"shadowDraw"_s, d->enabled );
  shadowElem.setAttribute( u"shadowUnder"_s, static_cast< unsigned int >( d->shadowUnder ) );
  shadowElem.setAttribute( u"shadowOffsetAngle"_s, d->offsetAngle );
  shadowElem.setAttribute( u"shadowOffsetDist"_s, d->offsetDist );
  shadowElem.setAttribute( u"shadowOffsetUnit"_s, QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  shadowElem.setAttribute( u"shadowOffsetMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  shadowElem.setAttribute( u"shadowOffsetGlobal"_s, d->offsetGlobal );
  shadowElem.setAttribute( u"shadowRadius"_s, d->radius );
  shadowElem.setAttribute( u"shadowRadiusUnit"_s, QgsUnitTypes::encodeUnit( d->radiusUnits ) );
  shadowElem.setAttribute( u"shadowRadiusMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->radiusMapUnitScale ) );
  shadowElem.setAttribute( u"shadowRadiusAlphaOnly"_s, d->radiusAlphaOnly );
  shadowElem.setAttribute( u"shadowOpacity"_s, d->opacity );
  shadowElem.setAttribute( u"shadowScale"_s, d->scale );
  shadowElem.setAttribute( u"shadowColor"_s, QgsColorUtils::colorToString( d->color ) );
  shadowElem.setAttribute( u"shadowBlendMode"_s, static_cast< int >( QgsPainting::getBlendModeEnum( d->blendMode ) ) );
  return shadowElem;
}

void QgsTextShadowSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::Property::ShadowDraw, context.expressionContext(), d->enabled );
  }

  // data defined shadow under type?
  QVariant exprVal = properties.value( QgsPalLayerSettings::Property::ShadowUnder, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString str = exprVal.toString().trimmed();
    if ( !str.isEmpty() )
    {
      d->shadowUnder = QgsTextRendererUtils::decodeShadowPlacementType( str );
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowOffsetAngle ) )
  {
    context.expressionContext().setOriginalValueVariable( d->offsetAngle );
    d->offsetAngle = properties.valueAsInt( QgsPalLayerSettings::Property::ShadowOffsetAngle, context.expressionContext(), d->offsetAngle );
  }
  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowOffsetDist ) )
  {
    context.expressionContext().setOriginalValueVariable( d->offsetDist );
    d->offsetDist = properties.valueAsDouble( QgsPalLayerSettings::Property::ShadowOffsetDist, context.expressionContext(), d->offsetDist );
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShadowOffsetUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->offsetUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowRadius ) )
  {
    context.expressionContext().setOriginalValueVariable( d->radius );
    d->radius = properties.valueAsDouble( QgsPalLayerSettings::Property::ShadowRadius, context.expressionContext(), d->radius );
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShadowRadiusUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiusUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::Property::ShadowOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !QgsVariantUtils::isNull( val ) )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowScale ) )
  {
    context.expressionContext().setOriginalValueVariable( d->scale );
    d->scale = properties.valueAsInt( QgsPalLayerSettings::Property::ShadowScale, context.expressionContext(), d->scale );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->color ) );
    d->color = properties.valueAsColor( QgsPalLayerSettings::Property::ShadowColor, context.expressionContext(), d->color );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShadowBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::Property::ShadowBlendMode, context.expressionContext() );
    const QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }
}

QSet<QString> QgsTextShadowSettings::referencedFields( const QgsRenderContext & ) const
{
  return QSet< QString >(); // nothing for now
}
