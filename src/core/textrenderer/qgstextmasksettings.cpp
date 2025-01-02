/***************************************************************************
  qgstextmasksettings.cpp
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

#include "qgstextmasksettings.h"
#include "qgstextrenderer_p.h"
#include "qgspallabeling.h"
#include "qgssymbollayerutils.h"
#include "qgspainteffectregistry.h"
#include "qgsapplication.h"
#include "qgsunittypes.h"

QgsTextMaskSettings::QgsTextMaskSettings()
{
  d = new QgsTextMaskSettingsPrivate();
}

QgsTextMaskSettings::~QgsTextMaskSettings() = default;

QgsTextMaskSettings::QgsTextMaskSettings( const QgsTextMaskSettings &other ) //NOLINT
  : d( other.d )
{
}

QgsTextMaskSettings &QgsTextMaskSettings::operator=( const QgsTextMaskSettings &other )  //NOLINT
{
  d = other.d;
  return *this;
}

bool QgsTextMaskSettings::operator==( const QgsTextMaskSettings &other ) const
{
  if ( d->enabled != other.enabled()
       || d->type != other.type()
       || d->size != other.size()
       || d->sizeUnit != other.sizeUnit()
       || d->sizeMapUnitScale != other.sizeMapUnitScale()
       || d->joinStyle != other.joinStyle()
       || d->opacity != other.opacity()
       || d->maskedSymbolLayers != other.maskedSymbolLayers() )
    return false;

  if ( static_cast< bool >( d->paintEffect ) != static_cast< bool >( other.paintEffect() )
       || ( d->paintEffect && d->paintEffect->properties() != other.paintEffect()->properties() ) )
    return false;

  return true;
}

bool QgsTextMaskSettings::operator!=( const QgsTextMaskSettings &other ) const
{
  return !( *this == other );
}

bool QgsTextMaskSettings::enabled() const
{
  return d->enabled;
}

void QgsTextMaskSettings::setEnabled( bool enabled )
{
  d->enabled = enabled;
}

QgsTextMaskSettings::MaskType QgsTextMaskSettings::type() const
{
  return d->type;
}

void QgsTextMaskSettings::setType( QgsTextMaskSettings::MaskType type )
{
  d->type = type;
}


double QgsTextMaskSettings::size() const
{
  return d->size;
}

void QgsTextMaskSettings::setSize( double size )
{
  d->size = size;
}

Qgis::RenderUnit QgsTextMaskSettings::sizeUnit() const
{
  return d->sizeUnit;
}

void QgsTextMaskSettings::setSizeUnit( Qgis::RenderUnit unit )
{
  d->sizeUnit = unit;
}

QgsMapUnitScale QgsTextMaskSettings::sizeMapUnitScale() const
{
  return d->sizeMapUnitScale;
}

void QgsTextMaskSettings::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  d->sizeMapUnitScale = scale;
}

Qt::PenJoinStyle QgsTextMaskSettings::joinStyle() const
{
  return d->joinStyle;
}

void QgsTextMaskSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
}

double QgsTextMaskSettings::opacity() const
{
  return d->opacity;
}

void QgsTextMaskSettings::setOpacity( double opacity )
{
  d->opacity = opacity;
}

QgsPaintEffect *QgsTextMaskSettings::paintEffect() const
{
  return d->paintEffect.get();
}

void QgsTextMaskSettings::setPaintEffect( QgsPaintEffect *effect )
{
  d->paintEffect.reset( effect );
}

void QgsTextMaskSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::MaskEnabled ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::Property::MaskEnabled, context.expressionContext(), d->enabled );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::MaskBufferSize ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size );
    d->size = properties.valueAsDouble( QgsPalLayerSettings::Property::MaskBufferSize, context.expressionContext(), d->size );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::MaskBufferUnit ) )
  {
    const QVariant exprVal = properties.value( QgsPalLayerSettings::Property::MaskBufferUnit, context.expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      const QString units = exprVal.toString();
      if ( !units.isEmpty() )
      {
        bool ok;
        const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
        if ( ok )
          d->sizeUnit = res;
      }
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::MaskOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::Property::MaskOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !QgsVariantUtils::isNull( val ) )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::MaskJoinStyle ) )
  {
    const QVariant exprVal = properties.value( QgsPalLayerSettings::Property::MaskJoinStyle, context.expressionContext() );
    const QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
    }
  }
}

QSet<QString> QgsTextMaskSettings::referencedFields( const QgsRenderContext & ) const
{
  return QSet< QString >(); // nothing for now
}

void QgsTextMaskSettings::readXml( const QDomElement &elem )
{
  const QDomElement textMaskElem = elem.firstChildElement( QStringLiteral( "text-mask" ) );
  d->enabled = textMaskElem.attribute( QStringLiteral( "maskEnabled" ), QStringLiteral( "0" ) ).toInt();
  d->type = static_cast<QgsTextMaskSettings::MaskType>( textMaskElem.attribute( QStringLiteral( "maskType" ), QStringLiteral( "0" ) ).toInt() );
  if ( textMaskElem.hasAttribute( QStringLiteral( "maskSize2" ) ) )
  {
    d->size = textMaskElem.attribute( QStringLiteral( "maskSize2" ), QStringLiteral( "1.5" ) ).toDouble();
  }
  else
  {
    // the older maskSize attribute used to be treated as 0 instead of the default 1.5 mm size when no mask
    // settings were available, and then when the project was saved this incorrect 0 value would become an
    // actual valid attribute in the XML. Since we can't now differentiate a valid 0 value from an accidental
    // 0 value in older projects, we instead assume "0" as a mistake and reset it to 1.5.
    // when the project is saved the newer maskSize2 attribute will be used and we know that a "0" value
    // WAS an explicit user choice.
    d->size = textMaskElem.attribute( QStringLiteral( "maskSize" ) ).toDouble();
    if ( d->size == 0 )
      d->size = 1.5;
  }
  d->sizeUnit = QgsUnitTypes::decodeRenderUnit( textMaskElem.attribute( QStringLiteral( "maskSizeUnits" ) ) );
  d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textMaskElem.attribute( QStringLiteral( "maskSizeMapUnitScale" ) ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( textMaskElem.attribute( QStringLiteral( "maskJoinStyle" ), QString::number( Qt::RoundJoin ) ).toUInt() );
  d->opacity = textMaskElem.attribute( QStringLiteral( "maskOpacity" ), QStringLiteral( "1.0" ) ).toDouble();
  const QDomElement effectElem = textMaskElem.firstChildElement( QStringLiteral( "effect" ) );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( nullptr );
  d->maskedSymbolLayers = stringToSymbolLayerReferenceList( textMaskElem.attribute( QStringLiteral( "maskedSymbolLayers" ) ) );
}

QDomElement QgsTextMaskSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement textMaskElem = doc.createElement( QStringLiteral( "text-mask" ) );
  textMaskElem.setAttribute( QStringLiteral( "maskEnabled" ), d->enabled );
  textMaskElem.setAttribute( QStringLiteral( "maskType" ), d->type );
  textMaskElem.setAttribute( QStringLiteral( "maskSize" ), d->size );
  // deliberate -- see comment in readXml
  textMaskElem.setAttribute( QStringLiteral( "maskSize2" ), d->size );
  textMaskElem.setAttribute( QStringLiteral( "maskSizeUnits" ), QgsUnitTypes::encodeUnit( d->sizeUnit ) );
  textMaskElem.setAttribute( QStringLiteral( "maskSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  textMaskElem.setAttribute( QStringLiteral( "maskJoinStyle" ), static_cast< unsigned int >( d->joinStyle ) );
  textMaskElem.setAttribute( QStringLiteral( "maskOpacity" ), d->opacity );
  if ( d->paintEffect && !QgsPaintEffectRegistry::isDefaultStack( d->paintEffect.get() ) )
    d->paintEffect->saveProperties( doc, textMaskElem );
  textMaskElem.setAttribute( QStringLiteral( "maskedSymbolLayers" ), symbolLayerReferenceListToString( d->maskedSymbolLayers ) );
  return textMaskElem;
}

QList<QgsSymbolLayerReference> QgsTextMaskSettings::maskedSymbolLayers() const
{
  return d->maskedSymbolLayers;
}

void QgsTextMaskSettings::setMaskedSymbolLayers( const QList<QgsSymbolLayerReference> &maskedSymbols )
{
  d->maskedSymbolLayers = maskedSymbols;
}
