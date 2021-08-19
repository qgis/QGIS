/***************************************************************************
  qgstextbuffersettings.cpp
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

#include "qgstextbuffersettings.h"
#include "qgstextrenderer_p.h"
#include "qgsvectorlayer.h"
#include "qgspallabeling.h"
#include "qgssymbollayerutils.h"
#include "qgspainting.h"
#include "qgspainteffectregistry.h"
#include "qgstextrendererutils.h"

QgsTextBufferSettings::QgsTextBufferSettings()
{
  d = new QgsTextBufferSettingsPrivate();
}

QgsTextBufferSettings::QgsTextBufferSettings( const QgsTextBufferSettings &other ) //NOLINT
  : d( other.d )
{
}

QgsTextBufferSettings &QgsTextBufferSettings::operator=( const QgsTextBufferSettings &other )  //NOLINT
{
  d = other.d;
  return *this;
}

QgsTextBufferSettings::~QgsTextBufferSettings() //NOLINT
{

}

bool QgsTextBufferSettings::operator==( const QgsTextBufferSettings &other ) const
{
  if ( d->enabled != other.enabled()
       || d->size != other.size()
       || d->sizeUnit != other.sizeUnit()
       || d->sizeMapUnitScale != other.sizeMapUnitScale()
       || d->color != other.color()
       || d->opacity != other.opacity()
       || d->fillBufferInterior != other.fillBufferInterior()
       || d->joinStyle != other.joinStyle()
       || d->blendMode != other.blendMode() )
    return false;

  if ( static_cast< bool >( d->paintEffect ) != static_cast< bool >( other.paintEffect() )
       || ( d->paintEffect && d->paintEffect->properties() != other.paintEffect()->properties() ) )
    return false;

  return true;
}

bool QgsTextBufferSettings::operator!=( const QgsTextBufferSettings &other ) const
{
  return !( *this == other );
}

bool QgsTextBufferSettings::enabled() const
{
  return d->enabled;
}

void QgsTextBufferSettings::setEnabled( bool enabled )
{
  d->enabled = enabled;
}

double QgsTextBufferSettings::size() const
{
  return d->size;
}

void QgsTextBufferSettings::setSize( double size )
{
  d->size = size;
}

QgsUnitTypes::RenderUnit QgsTextBufferSettings::sizeUnit() const
{
  return d->sizeUnit;
}

void QgsTextBufferSettings::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  d->sizeUnit = unit;
}

QgsMapUnitScale QgsTextBufferSettings::sizeMapUnitScale() const
{
  return d->sizeMapUnitScale;
}

void QgsTextBufferSettings::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  d->sizeMapUnitScale = scale;
}

QColor QgsTextBufferSettings::color() const
{
  return d->color;
}

void QgsTextBufferSettings::setColor( const QColor &color )
{
  d->color = color;
}

bool QgsTextBufferSettings::fillBufferInterior() const
{
  return d->fillBufferInterior;
}

void QgsTextBufferSettings::setFillBufferInterior( bool fill )
{
  d->fillBufferInterior = fill;
}

double QgsTextBufferSettings::opacity() const
{
  return d->opacity;
}

void QgsTextBufferSettings::setOpacity( double opacity )
{
  d->opacity = opacity;
}

Qt::PenJoinStyle QgsTextBufferSettings::joinStyle() const
{
  return d->joinStyle;
}

void QgsTextBufferSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
}

QPainter::CompositionMode QgsTextBufferSettings::blendMode() const
{
  return d->blendMode;
}

void QgsTextBufferSettings::setBlendMode( QPainter::CompositionMode mode )
{
  d->blendMode = mode;
}

const QgsPaintEffect *QgsTextBufferSettings::paintEffect() const
{
  return d->paintEffect.get();
}

void QgsTextBufferSettings::setPaintEffect( QgsPaintEffect *effect )
{
  d->paintEffect.reset( effect );
}

void QgsTextBufferSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::BufferDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::BufferDraw, context.expressionContext(), d->enabled );
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferSize ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size );
    d->size = properties.valueAsDouble( QgsPalLayerSettings::BufferSize, context.expressionContext(), d->size );
  }

  QVariant exprVal = properties.value( QgsPalLayerSettings::BufferUnit, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->sizeUnit = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::BufferOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !val.isNull() )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->color ) );
    d->color = properties.valueAsColor( QgsPalLayerSettings::BufferColor, context.expressionContext(), d->color );
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::BufferBlendMode, context.expressionContext() );
    const QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferJoinStyle ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::BufferJoinStyle, context.expressionContext() );
    const QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
    }
  }
}

QSet<QString> QgsTextBufferSettings::referencedFields( const QgsRenderContext & ) const
{
  return QSet< QString >(); // nothing for now
}

void QgsTextBufferSettings::readFromLayer( QgsVectorLayer *layer )
{
  // text buffer
  const double bufSize = layer->customProperty( QStringLiteral( "labeling/bufferSize" ), QVariant( 0.0 ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  const QVariant drawBuffer = layer->customProperty( QStringLiteral( "labeling/bufferDraw" ), QVariant() );
  if ( drawBuffer.isValid() )
  {
    d->enabled = drawBuffer.toBool();
    d->size = bufSize;
  }
  else if ( bufSize != 0.0 )
  {
    d->enabled = true;
    d->size = bufSize;
  }
  else
  {
    // keep bufferSize at new 1.0 default
    d->enabled = false;
  }

  if ( layer->customProperty( QStringLiteral( "labeling/bufferSizeUnits" ) ).toString().isEmpty() )
  {
    const bool bufferSizeInMapUnits = layer->customProperty( QStringLiteral( "labeling/bufferSizeInMapUnits" ) ).toBool();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/bufferSizeUnits" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitMaxScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitScale" ) ).toString() );
  }
  d->color = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/bufferColor" ), Qt::white, false );
  if ( layer->customProperty( QStringLiteral( "labeling/bufferOpacity" ) ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( QStringLiteral( "labeling/bufferTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( QStringLiteral( "labeling/bufferOpacity" ) ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/bufferBlendMode" ), QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( layer->customProperty( QStringLiteral( "labeling/bufferJoinStyle" ), QVariant( Qt::RoundJoin ) ).toUInt() );

  d->fillBufferInterior = !layer->customProperty( QStringLiteral( "labeling/bufferNoFill" ), QVariant( false ) ).toBool();

  if ( layer->customProperty( QStringLiteral( "labeling/bufferEffect" ) ).isValid() )
  {
    QDomDocument doc( QStringLiteral( "effect" ) );
    doc.setContent( layer->customProperty( QStringLiteral( "labeling/bufferEffect" ) ).toString() );
    const QDomElement effectElem = doc.firstChildElement( QStringLiteral( "effect" ) ).firstChildElement( QStringLiteral( "effect" ) );
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  }
  else
    setPaintEffect( nullptr );
}

void QgsTextBufferSettings::readXml( const QDomElement &elem )
{
  const QDomElement textBufferElem = elem.firstChildElement( QStringLiteral( "text-buffer" ) );
  const double bufSize = textBufferElem.attribute( QStringLiteral( "bufferSize" ), QStringLiteral( "0" ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  const QVariant drawBuffer = textBufferElem.attribute( QStringLiteral( "bufferDraw" ) );
  if ( drawBuffer.isValid() )
  {
    d->enabled = drawBuffer.toBool();
    d->size = bufSize;
  }
  else if ( bufSize != 0.0 )
  {
    d->enabled = true;
    d->size = bufSize;
  }
  else
  {
    // keep bufferSize at new 1.0 default
    d->enabled = false;
  }

  if ( !textBufferElem.hasAttribute( QStringLiteral( "bufferSizeUnits" ) ) )
  {
    const bool bufferSizeInMapUnits = textBufferElem.attribute( QStringLiteral( "bufferSizeInMapUnits" ) ).toInt();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( textBufferElem.attribute( QStringLiteral( "bufferSizeUnits" ) ) );
  }

  if ( !textBufferElem.hasAttribute( QStringLiteral( "bufferSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = textBufferElem.attribute( QStringLiteral( "bufferSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = textBufferElem.attribute( QStringLiteral( "bufferSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textBufferElem.attribute( QStringLiteral( "bufferSizeMapUnitScale" ) ) );
  }
  d->color = QgsSymbolLayerUtils::decodeColor( textBufferElem.attribute( QStringLiteral( "bufferColor" ), QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );

  if ( !textBufferElem.hasAttribute( QStringLiteral( "bufferOpacity" ) ) )
  {
    d->opacity = ( 1 - textBufferElem.attribute( QStringLiteral( "bufferTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( textBufferElem.attribute( QStringLiteral( "bufferOpacity" ) ).toDouble() );
  }

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( textBufferElem.attribute( QStringLiteral( "bufferBlendMode" ), QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( textBufferElem.attribute( QStringLiteral( "bufferJoinStyle" ), QString::number( Qt::RoundJoin ) ).toUInt() );
  d->fillBufferInterior = !textBufferElem.attribute( QStringLiteral( "bufferNoFill" ), QStringLiteral( "0" ) ).toInt();
  const QDomElement effectElem = textBufferElem.firstChildElement( QStringLiteral( "effect" ) );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( nullptr );
}

QDomElement QgsTextBufferSettings::writeXml( QDomDocument &doc ) const
{
  // text buffer
  QDomElement textBufferElem = doc.createElement( QStringLiteral( "text-buffer" ) );
  textBufferElem.setAttribute( QStringLiteral( "bufferDraw" ), d->enabled );
  textBufferElem.setAttribute( QStringLiteral( "bufferSize" ), d->size );
  textBufferElem.setAttribute( QStringLiteral( "bufferSizeUnits" ), QgsUnitTypes::encodeUnit( d->sizeUnit ) );
  textBufferElem.setAttribute( QStringLiteral( "bufferSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  textBufferElem.setAttribute( QStringLiteral( "bufferColor" ), QgsSymbolLayerUtils::encodeColor( d->color ) );
  textBufferElem.setAttribute( QStringLiteral( "bufferNoFill" ), !d->fillBufferInterior );
  textBufferElem.setAttribute( QStringLiteral( "bufferOpacity" ), d->opacity );
  textBufferElem.setAttribute( QStringLiteral( "bufferJoinStyle" ), static_cast< unsigned int >( d->joinStyle ) );
  textBufferElem.setAttribute( QStringLiteral( "bufferBlendMode" ), QgsPainting::getBlendModeEnum( d->blendMode ) );
  if ( d->paintEffect && !QgsPaintEffectRegistry::isDefaultStack( d->paintEffect.get() ) )
    d->paintEffect->saveProperties( doc, textBufferElem );
  return textBufferElem;
}
