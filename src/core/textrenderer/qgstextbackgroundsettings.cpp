/***************************************************************************
  qgstextbackgroundsettings.cpp
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

#include "qgstextbackgroundsettings.h"

#include "qgsapplication.h"
#include "qgscolorutils.h"
#include "qgsfillsymbollayer.h"
#include "qgspainteffectregistry.h"
#include "qgspainting.h"
#include "qgspallabeling.h"
#include "qgssymbollayerutils.h"
#include "qgstextrenderer_p.h"
#include "qgstextrendererutils.h"
#include "qgsunittypes.h"
#include "qgsvectorlayer.h"

QgsTextBackgroundSettings::QgsTextBackgroundSettings()
{
  d = new QgsTextBackgroundSettingsPrivate();

  // Create a default fill symbol to preserve API promise until QGIS 5.0
  QgsSimpleFillSymbolLayer *fill = new QgsSimpleFillSymbolLayer( d->fillColor, Qt::SolidPattern, d->strokeColor );
  fill->setStrokeWidth( d->strokeWidth );
  fill->setStrokeWidthUnit( d->strokeWidthUnits );
  fill->setStrokeWidthMapUnitScale( d->strokeWidthMapUnitScale );
  fill->setStrokeStyle( !qgsDoubleNear( d->strokeWidth, 0.0 ) ? Qt::SolidLine : Qt::NoPen );
  fill->setPenJoinStyle( d->joinStyle );

  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, fill );
  setFillSymbol( fillSymbol );
}

QgsTextBackgroundSettings::QgsTextBackgroundSettings( const QgsTextBackgroundSettings &other ) //NOLINT
  : d( other.d )
{

}

QgsTextBackgroundSettings::QgsTextBackgroundSettings( QgsTextBackgroundSettings &&other ) //NOLINT
  : d( std::move( other.d ) )
{

}

QgsTextBackgroundSettings &QgsTextBackgroundSettings::operator=( const QgsTextBackgroundSettings &other )  //NOLINT
{
  if ( &other == this )
    return *this;

  d = other.d;
  return *this;
}

QgsTextBackgroundSettings &QgsTextBackgroundSettings::operator=( QgsTextBackgroundSettings &&other )  //NOLINT
{
  if ( &other == this )
    return *this;

  d = std::move( other.d );
  return *this;
}

QgsTextBackgroundSettings::~QgsTextBackgroundSettings() //NOLINT
{

}

bool QgsTextBackgroundSettings::operator==( const QgsTextBackgroundSettings &other ) const
{
  if ( d->enabled != other.enabled()
       || d->type != other.type()
       || d->svgFile != other.svgFile()
       || d->sizeType != other.sizeType()
       || d->size != other.size()
       || d->sizeUnits != other.sizeUnit()
       || d->sizeMapUnitScale != other.sizeMapUnitScale()
       || d->rotationType != other.rotationType()
       || d->rotation != other.rotation()
       || d->offset != other.offset()
       || d->offsetUnits != other.offsetUnit()
       || d->offsetMapUnitScale != other.offsetMapUnitScale()
       || d->radii != other.radii()
       || d->radiiUnits != other.radiiUnit()
       || d->radiiMapUnitScale != other.radiiMapUnitScale()
       || d->blendMode != other.blendMode()
       || d->fillColor != other.fillColor()
       || d->strokeColor != other.strokeColor()
       || d->opacity != other.opacity()
       || d->strokeWidth != other.strokeWidth()
       || d->strokeWidthUnits != other.strokeWidthUnit()
       || d->strokeWidthMapUnitScale != other.strokeWidthMapUnitScale()
       || d->joinStyle != other.joinStyle() )
    return false;

  if ( static_cast< bool >( d->paintEffect ) != static_cast< bool >( other.paintEffect() )
       || ( d->paintEffect && d->paintEffect->properties() != other.paintEffect()->properties() ) )
    return false;

  if ( static_cast< bool >( d->markerSymbol ) != static_cast< bool >( other.markerSymbol() )
       || ( d->markerSymbol && QgsSymbolLayerUtils::symbolProperties( d->markerSymbol.get() ) != QgsSymbolLayerUtils::symbolProperties( other.markerSymbol() ) ) )
    return false;

  if ( static_cast< bool >( d->fillSymbol ) != static_cast< bool >( other.fillSymbol() )
       || ( d->fillSymbol && QgsSymbolLayerUtils::symbolProperties( d->fillSymbol.get() ) != QgsSymbolLayerUtils::symbolProperties( other.fillSymbol() ) ) )
    return false;

  return true;
}

bool QgsTextBackgroundSettings::operator!=( const QgsTextBackgroundSettings &other ) const
{
  return !( *this == other );
}

bool QgsTextBackgroundSettings::enabled() const
{
  return d->enabled;
}

void QgsTextBackgroundSettings::setEnabled( bool enabled )
{
  d->enabled = enabled;
}

QgsTextBackgroundSettings::ShapeType QgsTextBackgroundSettings::type() const
{
  return d->type;
}

void QgsTextBackgroundSettings::setType( QgsTextBackgroundSettings::ShapeType type )
{
  d->type = type;
}

QString QgsTextBackgroundSettings::svgFile() const
{
  return d->svgFile;
}

void QgsTextBackgroundSettings::setSvgFile( const QString &file )
{
  d->svgFile = file;
}

QgsMarkerSymbol *QgsTextBackgroundSettings::markerSymbol() const
{
  return d->markerSymbol.get();
}

void QgsTextBackgroundSettings::setMarkerSymbol( QgsMarkerSymbol *symbol )
{
  if ( symbol )
    // Remove symbol layer unique id to have correct settings equality
    QgsSymbolLayerUtils::clearSymbolLayerIds( symbol );

  d->markerSymbol.reset( symbol );
}

QgsFillSymbol *QgsTextBackgroundSettings::fillSymbol() const
{
  return d->fillSymbol.get();
}

void QgsTextBackgroundSettings::setFillSymbol( QgsFillSymbol *symbol )
{
  if ( symbol )
    // Remove symbol layer unique id to have correct settings equality
    QgsSymbolLayerUtils::clearSymbolLayerIds( symbol );

  d->fillSymbol.reset( symbol );
}

QgsTextBackgroundSettings::SizeType QgsTextBackgroundSettings::sizeType() const
{
  return d->sizeType;
}

void QgsTextBackgroundSettings::setSizeType( QgsTextBackgroundSettings::SizeType type )
{
  d->sizeType = type;
}

QSizeF QgsTextBackgroundSettings::size() const
{
  return d->size;
}

void QgsTextBackgroundSettings::setSize( QSizeF size )
{
  d->size = size;
}

Qgis::RenderUnit QgsTextBackgroundSettings::sizeUnit() const
{
  return d->sizeUnits;
}

void QgsTextBackgroundSettings::setSizeUnit( Qgis::RenderUnit unit )
{
  d->sizeUnits = unit;
}

QgsMapUnitScale QgsTextBackgroundSettings::sizeMapUnitScale() const
{
  return d->sizeMapUnitScale;
}

void QgsTextBackgroundSettings::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  d->sizeMapUnitScale = scale;
}

QgsTextBackgroundSettings::RotationType QgsTextBackgroundSettings::rotationType() const
{
  return d->rotationType;
}

void QgsTextBackgroundSettings::setRotationType( QgsTextBackgroundSettings::RotationType type )
{
  d->rotationType = type;
}

double QgsTextBackgroundSettings::rotation() const
{
  return d->rotation;
}

void QgsTextBackgroundSettings::setRotation( double rotation )
{
  d->rotation = rotation;
}

QPointF QgsTextBackgroundSettings::offset() const
{
  return d->offset;
}

void QgsTextBackgroundSettings::setOffset( QPointF offset )
{
  d->offset = offset;
}

Qgis::RenderUnit QgsTextBackgroundSettings::offsetUnit() const
{
  return d->offsetUnits;
}

void QgsTextBackgroundSettings::setOffsetUnit( Qgis::RenderUnit units )
{
  d->offsetUnits = units;
}

QgsMapUnitScale QgsTextBackgroundSettings::offsetMapUnitScale() const
{
  return d->offsetMapUnitScale;
}

void QgsTextBackgroundSettings::setOffsetMapUnitScale( const QgsMapUnitScale &scale )
{
  d->offsetMapUnitScale = scale;
}

QSizeF QgsTextBackgroundSettings::radii() const
{
  return d->radii;
}

void QgsTextBackgroundSettings::setRadii( QSizeF radii )
{
  d->radii = radii;
}

Qgis::RenderUnit QgsTextBackgroundSettings::radiiUnit() const
{
  return d->radiiUnits;
}

void QgsTextBackgroundSettings::setRadiiUnit( Qgis::RenderUnit units )
{
  d->radiiUnits = units;
}

QgsMapUnitScale QgsTextBackgroundSettings::radiiMapUnitScale() const
{
  return d->radiiMapUnitScale;
}

void QgsTextBackgroundSettings::setRadiiMapUnitScale( const QgsMapUnitScale &scale )
{
  d->radiiMapUnitScale = scale;
}

double QgsTextBackgroundSettings::opacity() const
{
  return d->opacity;
}

void QgsTextBackgroundSettings::setOpacity( double opacity )
{
  d->opacity = opacity;
}

QPainter::CompositionMode QgsTextBackgroundSettings::blendMode() const
{
  return d->blendMode;
}

void QgsTextBackgroundSettings::setBlendMode( QPainter::CompositionMode mode )
{
  d->blendMode = mode;
}

QColor QgsTextBackgroundSettings::fillColor() const
{
  return d->fillSymbol ? d->fillSymbol->color() : d->fillColor;
}

void QgsTextBackgroundSettings::setFillColor( const QColor &color )
{
  d->fillColor = color;
  if ( d->fillSymbol )
  {
    d->fillSymbol->setColor( color );
  }
}

QColor QgsTextBackgroundSettings::strokeColor() const
{
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    return qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->strokeColor();
  }
  return d->strokeColor;
}

void QgsTextBackgroundSettings::setStrokeColor( const QColor &color )
{
  d->strokeColor = color;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeColor( color );
  }
}

double QgsTextBackgroundSettings::strokeWidth() const
{
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    return qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->strokeWidth();
  }
  return d->strokeWidth;
}

void QgsTextBackgroundSettings::setStrokeWidth( double width )
{
  d->strokeWidth = width;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    QgsSimpleFillSymbolLayer *fill = qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) );
    fill->setStrokeWidth( width );
    fill->setStrokeStyle( !qgsDoubleNear( width, 0.0 ) ? Qt::SolidLine : Qt::NoPen );
  }
}

Qgis::RenderUnit QgsTextBackgroundSettings::strokeWidthUnit() const
{
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    return qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->strokeWidthUnit();
  }
  return d->strokeWidthUnits;
}

void QgsTextBackgroundSettings::setStrokeWidthUnit( Qgis::RenderUnit units )
{
  d->strokeWidthUnits = units;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeWidthUnit( units );
  }
}

QgsMapUnitScale QgsTextBackgroundSettings::strokeWidthMapUnitScale() const
{
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    return qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->strokeWidthMapUnitScale();
  }
  return d->strokeWidthMapUnitScale;
}

void QgsTextBackgroundSettings::setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale )
{
  d->strokeWidthMapUnitScale = scale;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeWidthMapUnitScale( scale );
  }
}

Qt::PenJoinStyle QgsTextBackgroundSettings::joinStyle() const
{
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    return qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->penJoinStyle();
  }
  return d->joinStyle;
}

void QgsTextBackgroundSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == "SimpleFill"_L1 )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setPenJoinStyle( style );
  }
}

const QgsPaintEffect *QgsTextBackgroundSettings::paintEffect() const
{
  return d->paintEffect.get();
}

void QgsTextBackgroundSettings::setPaintEffect( QgsPaintEffect *effect )
{
  d->paintEffect.reset( effect );
}

void QgsTextBackgroundSettings::readFromLayer( QgsVectorLayer *layer )
{
  d->enabled = layer->customProperty( u"labeling/shapeDraw"_s, QVariant( false ) ).toBool();
  d->type = static_cast< ShapeType >( layer->customProperty( u"labeling/shapeType"_s, QVariant( ShapeRectangle ) ).toUInt() );
  d->svgFile = layer->customProperty( u"labeling/shapeSVGFile"_s, QVariant( "" ) ).toString();
  d->sizeType = static_cast< SizeType >( layer->customProperty( u"labeling/shapeSizeType"_s, QVariant( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( layer->customProperty( u"labeling/shapeSizeX"_s, QVariant( 0.0 ) ).toDouble(),
                    layer->customProperty( u"labeling/shapeSizeY"_s, QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( u"labeling/shapeSizeUnit"_s ).toString().isEmpty() )
  {
    d->sizeUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shapeSizeUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shapeSizeUnit"_s ).toString() );
  }

  if ( layer->customProperty( u"labeling/shapeSizeMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shapeSizeMapUnitMinScale"_s, 0.0 ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shapeSizeMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shapeSizeMapUnitScale"_s ).toString() );
  }
  d->rotationType = static_cast< RotationType >( layer->customProperty( u"labeling/shapeRotationType"_s, QVariant( RotationSync ) ).toUInt() );
  d->rotation = layer->customProperty( u"labeling/shapeRotation"_s, QVariant( 0.0 ) ).toDouble();
  d->offset = QPointF( layer->customProperty( u"labeling/shapeOffsetX"_s, QVariant( 0.0 ) ).toDouble(),
                       layer->customProperty( u"labeling/shapeOffsetY"_s, QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( u"labeling/shapeOffsetUnit"_s ).toString().isEmpty() )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shapeOffsetUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shapeOffsetUnit"_s ).toString() );
  }

  if ( layer->customProperty( u"labeling/shapeOffsetMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shapeOffsetMapUnitMinScale"_s, 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shapeOffsetMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shapeOffsetMapUnitScale"_s ).toString() );
  }
  d->radii = QSizeF( layer->customProperty( u"labeling/shapeRadiiX"_s, QVariant( 0.0 ) ).toDouble(),
                     layer->customProperty( u"labeling/shapeRadiiY"_s, QVariant( 0.0 ) ).toDouble() );


  if ( layer->customProperty( u"labeling/shapeRadiiUnit"_s ).toString().isEmpty() )
  {
    d->radiiUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shapeRadiiUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shapeRadiiUnit"_s ).toString() );
  }

  if ( layer->customProperty( u"labeling/shapeRadiiMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shapeRadiiMapUnitMinScale"_s, 0.0 ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shapeRadiiMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->radiiMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shapeRadiiMapUnitScale"_s ).toString() );
  }
  d->fillColor = QgsTextRendererUtils::readColor( layer, u"labeling/shapeFillColor"_s, Qt::white, true );
  d->strokeColor = QgsTextRendererUtils::readColor( layer, u"labeling/shapeBorderColor"_s, Qt::darkGray, true );
  d->strokeWidth = layer->customProperty( u"labeling/shapeBorderWidth"_s, QVariant( .0 ) ).toDouble();
  if ( layer->customProperty( u"labeling/shapeBorderWidthUnit"_s ).toString().isEmpty() )
  {
    d->strokeWidthUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( u"labeling/shapeBorderWidthUnits"_s, 0 ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( u"labeling/shapeBorderWidthUnit"_s ).toString() );
  }
  if ( layer->customProperty( u"labeling/shapeBorderWidthMapUnitScale"_s ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( u"labeling/shapeBorderWidthMapUnitMinScale"_s, 0.0 ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( u"labeling/shapeBorderWidthMapUnitMaxScale"_s, 0.0 ).toDouble();
    d->strokeWidthMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->strokeWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( u"labeling/shapeBorderWidthMapUnitScale"_s ).toString() );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( layer->customProperty( u"labeling/shapeJoinStyle"_s, QVariant( Qt::BevelJoin ) ).toUInt() );

  if ( layer->customProperty( u"labeling/shapeOpacity"_s ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( u"labeling/shapeTransparency"_s ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( u"labeling/shapeOpacity"_s ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< Qgis::BlendMode >( layer->customProperty( u"labeling/shapeBlendMode"_s, QVariant( static_cast< int>( Qgis::BlendMode::Normal ) ) ).toUInt() ) );

  if ( layer->customProperty( u"labeling/shapeEffect"_s ).isValid() )
  {
    QDomDocument doc( u"effect"_s );
    doc.setContent( layer->customProperty( u"labeling/shapeEffect"_s ).toString() );
    const QDomElement effectElem = doc.firstChildElement( u"effect"_s ).firstChildElement( u"effect"_s );
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  }
  else
    setPaintEffect( nullptr );
}

void QgsTextBackgroundSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement backgroundElem = elem.firstChildElement( u"background"_s );
  d->enabled = backgroundElem.attribute( u"shapeDraw"_s, u"0"_s ).toInt();
  d->type = static_cast< ShapeType >( backgroundElem.attribute( u"shapeType"_s, QString::number( ShapeRectangle ) ).toUInt() );
  d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( backgroundElem.attribute( u"shapeSVGFile"_s ), context.pathResolver() );
  d->sizeType = static_cast< SizeType >( backgroundElem.attribute( u"shapeSizeType"_s, QString::number( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( backgroundElem.attribute( u"shapeSizeX"_s, u"0"_s ).toDouble(),
                    backgroundElem.attribute( u"shapeSizeY"_s, u"0"_s ).toDouble() );

  if ( !backgroundElem.hasAttribute( u"shapeSizeUnit"_s ) )
  {
    d->sizeUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( u"shapeSizeUnits"_s ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( u"shapeSizeUnit"_s ) );
  }

  if ( !backgroundElem.hasAttribute( u"shapeSizeMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( u"shapeSizeMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( u"shapeSizeMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( u"shapeSizeMapUnitScale"_s ) );
  }
  d->rotationType = static_cast< RotationType >( backgroundElem.attribute( u"shapeRotationType"_s, QString::number( RotationSync ) ).toUInt() );
  d->rotation = backgroundElem.attribute( u"shapeRotation"_s, u"0"_s ).toDouble();
  d->offset = QPointF( backgroundElem.attribute( u"shapeOffsetX"_s, u"0"_s ).toDouble(),
                       backgroundElem.attribute( u"shapeOffsetY"_s, u"0"_s ).toDouble() );

  if ( !backgroundElem.hasAttribute( u"shapeOffsetUnit"_s ) )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( u"shapeOffsetUnits"_s ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( u"shapeOffsetUnit"_s ) );
  }

  if ( !backgroundElem.hasAttribute( u"shapeOffsetMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( u"shapeOffsetMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( u"shapeOffsetMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( u"shapeOffsetMapUnitScale"_s ) );
  }
  d->radii = QSizeF( backgroundElem.attribute( u"shapeRadiiX"_s, u"0"_s ).toDouble(),
                     backgroundElem.attribute( u"shapeRadiiY"_s, u"0"_s ).toDouble() );

  if ( !backgroundElem.hasAttribute( u"shapeRadiiUnit"_s ) )
  {
    d->radiiUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( u"shapeRadiiUnits"_s ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( u"shapeRadiiUnit"_s ) );
  }
  if ( !backgroundElem.hasAttribute( u"shapeRadiiMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( u"shapeRadiiMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( u"shapeRadiiMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->radiiMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( u"shapeRadiiMapUnitScale"_s ) );
  }
  d->fillColor = QgsColorUtils::colorFromString( backgroundElem.attribute( u"shapeFillColor"_s, QgsColorUtils::colorToString( Qt::white ) ) );
  d->strokeColor = QgsColorUtils::colorFromString( backgroundElem.attribute( u"shapeBorderColor"_s, QgsColorUtils::colorToString( Qt::darkGray ) ) );
  d->strokeWidth = backgroundElem.attribute( u"shapeBorderWidth"_s, u"0"_s ).toDouble();

  if ( !backgroundElem.hasAttribute( u"shapeBorderWidthUnit"_s ) )
  {
    d->strokeWidthUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( u"shapeBorderWidthUnits"_s ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( u"shapeBorderWidthUnit"_s ) );
  }
  if ( !backgroundElem.hasAttribute( u"shapeBorderWidthMapUnitScale"_s ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( u"shapeBorderWidthMapUnitMinScale"_s, u"0"_s ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( u"shapeBorderWidthMapUnitMaxScale"_s, u"0"_s ).toDouble();
    d->strokeWidthMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->strokeWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( u"shapeBorderWidthMapUnitScale"_s ) );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( backgroundElem.attribute( u"shapeJoinStyle"_s, QString::number( Qt::BevelJoin ) ).toUInt() );

  if ( !backgroundElem.hasAttribute( u"shapeOpacity"_s ) )
  {
    d->opacity = ( 1 - backgroundElem.attribute( u"shapeTransparency"_s ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( backgroundElem.attribute( u"shapeOpacity"_s ).toDouble() );
  }

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< Qgis::BlendMode >( backgroundElem.attribute( u"shapeBlendMode"_s, QString::number( static_cast< int >( Qgis::BlendMode::Normal ) ) ).toUInt() ) );

  const QDomElement effectElem = backgroundElem.firstChildElement( u"effect"_s );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( nullptr );

  setMarkerSymbol( nullptr );
  setFillSymbol( nullptr );
  const QDomNodeList symbols = backgroundElem.elementsByTagName( u"symbol"_s );
  for ( int i = 0; i < symbols.size(); ++i )
  {
    if ( symbols.at( i ).isElement() )
    {
      const QDomElement symbolElement = symbols.at( i ).toElement();
      const QString symbolElementName = symbolElement.attribute( u"name"_s );
      if ( symbolElementName == "markerSymbol"_L1 )
      {
        setMarkerSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElement, context ).release() );
      }
      else if ( symbolElementName == "fillSymbol"_L1 )
      {
        setFillSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElement, context ).release() );
      }
    }
  }

  if ( !d->fillSymbol )
  {
    QgsSimpleFillSymbolLayer *fill = new QgsSimpleFillSymbolLayer( d->fillColor, Qt::SolidPattern, d->strokeColor );
    fill->setStrokeWidth( d->strokeWidth );
    fill->setStrokeWidthUnit( d->strokeWidthUnits );
    fill->setStrokeWidthMapUnitScale( d->strokeWidthMapUnitScale );
    fill->setStrokeStyle( !qgsDoubleNear( d->strokeWidth, 0.0 ) ? Qt::SolidLine : Qt::NoPen );
    fill->setPenJoinStyle( d->joinStyle );

    QgsFillSymbol *fillSymbol = new QgsFillSymbol();
    fillSymbol->changeSymbolLayer( 0, fill );
    setFillSymbol( fillSymbol );
  }
}

QDomElement QgsTextBackgroundSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement backgroundElem = doc.createElement( u"background"_s );
  backgroundElem.setAttribute( u"shapeDraw"_s, d->enabled );
  backgroundElem.setAttribute( u"shapeType"_s, static_cast< unsigned int >( d->type ) );
  backgroundElem.setAttribute( u"shapeSVGFile"_s, QgsSymbolLayerUtils::svgSymbolPathToName( d->svgFile, context.pathResolver() ) );
  backgroundElem.setAttribute( u"shapeSizeType"_s, static_cast< unsigned int >( d->sizeType ) );
  backgroundElem.setAttribute( u"shapeSizeX"_s, d->size.width() );
  backgroundElem.setAttribute( u"shapeSizeY"_s, d->size.height() );
  backgroundElem.setAttribute( u"shapeSizeUnit"_s, QgsUnitTypes::encodeUnit( d->sizeUnits ) );
  backgroundElem.setAttribute( u"shapeSizeMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  backgroundElem.setAttribute( u"shapeRotationType"_s, static_cast< unsigned int >( d->rotationType ) );
  backgroundElem.setAttribute( u"shapeRotation"_s, d->rotation );
  backgroundElem.setAttribute( u"shapeOffsetX"_s, d->offset.x() );
  backgroundElem.setAttribute( u"shapeOffsetY"_s, d->offset.y() );
  backgroundElem.setAttribute( u"shapeOffsetUnit"_s, QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  backgroundElem.setAttribute( u"shapeOffsetMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  backgroundElem.setAttribute( u"shapeRadiiX"_s, d->radii.width() );
  backgroundElem.setAttribute( u"shapeRadiiY"_s, d->radii.height() );
  backgroundElem.setAttribute( u"shapeRadiiUnit"_s, QgsUnitTypes::encodeUnit( d->radiiUnits ) );
  backgroundElem.setAttribute( u"shapeRadiiMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->radiiMapUnitScale ) );
  backgroundElem.setAttribute( u"shapeFillColor"_s, QgsColorUtils::colorToString( d->fillColor ) );
  backgroundElem.setAttribute( u"shapeBorderColor"_s, QgsColorUtils::colorToString( d->strokeColor ) );
  backgroundElem.setAttribute( u"shapeBorderWidth"_s, d->strokeWidth );
  backgroundElem.setAttribute( u"shapeBorderWidthUnit"_s, QgsUnitTypes::encodeUnit( d->strokeWidthUnits ) );
  backgroundElem.setAttribute( u"shapeBorderWidthMapUnitScale"_s, QgsSymbolLayerUtils::encodeMapUnitScale( d->strokeWidthMapUnitScale ) );
  backgroundElem.setAttribute( u"shapeJoinStyle"_s, static_cast< unsigned int >( d->joinStyle ) );
  backgroundElem.setAttribute( u"shapeOpacity"_s, d->opacity );
  backgroundElem.setAttribute( u"shapeBlendMode"_s, static_cast< int >( QgsPainting::getBlendModeEnum( d->blendMode ) ) );
  if ( d->paintEffect && !QgsPaintEffectRegistry::isDefaultStack( d->paintEffect.get() ) )
    d->paintEffect->saveProperties( doc, backgroundElem );

  if ( d->markerSymbol )
    backgroundElem.appendChild( QgsSymbolLayerUtils::saveSymbol( u"markerSymbol"_s, d->markerSymbol.get(), doc, context ) );

  if ( d->fillSymbol )
    backgroundElem.appendChild( QgsSymbolLayerUtils::saveSymbol( u"fillSymbol"_s, d->fillSymbol.get(), doc, context ) );

  return backgroundElem;
}

void QgsTextBackgroundSettings::upgradeDataDefinedProperties( QgsPropertyCollection &properties )
{
  if ( !d->fillSymbol || d->fillSymbol->symbolLayers().at( 0 )->layerType() != "SimpleFill"_L1 )
    return;
  QgsSimpleFillSymbolLayer *fill = qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) );

  if ( d->type != QgsTextBackgroundSettings::ShapeSVG )
  {
    if ( properties.hasProperty( QgsPalLayerSettings::Property::ShapeFillColor ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::Property::FillColor ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::Property::FillColor, properties.property( QgsPalLayerSettings::Property::ShapeFillColor ) );
      properties.setProperty( QgsPalLayerSettings::Property::ShapeFillColor, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::Property::ShapeStrokeColor ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::Property::StrokeColor ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::Property::StrokeColor, properties.property( QgsPalLayerSettings::Property::ShapeStrokeColor ) );
      properties.setProperty( QgsPalLayerSettings::Property::ShapeStrokeColor, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::Property::ShapeStrokeWidth ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::Property::StrokeWidth ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::Property::StrokeWidth, properties.property( QgsPalLayerSettings::Property::ShapeStrokeWidth ) );
      properties.setProperty( QgsPalLayerSettings::Property::ShapeStrokeWidth, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::Property::ShapeJoinStyle ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::Property::JoinStyle ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::Property::JoinStyle, properties.property( QgsPalLayerSettings::Property::ShapeJoinStyle ) );
      properties.setProperty( QgsPalLayerSettings::Property::ShapeJoinStyle, QgsProperty() );
    }
  }
}

void QgsTextBackgroundSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::Property::ShapeDraw, context.expressionContext(), d->enabled );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeSizeX ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size.width() );
    d->size.setWidth( properties.valueAsDouble( QgsPalLayerSettings::Property::ShapeSizeX, context.expressionContext(), d->size.width() ) );
  }
  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeSizeY ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size.height() );
    d->size.setHeight( properties.valueAsDouble( QgsPalLayerSettings::Property::ShapeSizeY, context.expressionContext(), d->size.height() ) );
  }

  QVariant exprVal = properties.value( QgsPalLayerSettings::Property::ShapeSizeUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->sizeUnits = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeKind, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString skind = exprVal.toString().trimmed();
    if ( !skind.isEmpty() )
    {
      d->type = QgsTextRendererUtils::decodeShapeType( skind );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeSizeType, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString stype = exprVal.toString().trimmed();
    if ( !stype.isEmpty() )
    {
      d->sizeType = QgsTextRendererUtils::decodeBackgroundSizeType( stype );
    }
  }

  // data defined shape SVG path?
  context.expressionContext().setOriginalValueVariable( d->svgFile );
  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeSVGFile, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString svgfile = exprVal.toString().trimmed();
    d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeRotation ) )
  {
    context.expressionContext().setOriginalValueVariable( d->rotation );
    d->rotation = properties.valueAsDouble( QgsPalLayerSettings::Property::ShapeRotation, context.expressionContext(), d->rotation );
  }
  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeRotationType, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString rotstr = exprVal.toString().trimmed();
    if ( !rotstr.isEmpty() )
    {
      d->rotationType = QgsTextRendererUtils::decodeBackgroundRotationType( rotstr );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeOffset, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      d->offset = res;
    }
  }
  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeOffsetUnits, context.expressionContext() );
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

  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeRadii, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    bool ok = false;
    const QSizeF res = QgsSymbolLayerUtils::toSize( exprVal, &ok );
    if ( ok )
    {
      d->radii = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeRadiiUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiiUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::Property::ShapeOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !QgsVariantUtils::isNull( val ) )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  // for non-SVG background types, using data defined properties within the fill symbol is preferable
  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeFillColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->fillColor ) );
    setFillColor( properties.valueAsColor( QgsPalLayerSettings::Property::ShapeFillColor, context.expressionContext(), d->fillColor ) );
  }
  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeStrokeColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->strokeColor ) );
    setStrokeColor( properties.valueAsColor( QgsPalLayerSettings::Property::ShapeStrokeColor, context.expressionContext(), d->strokeColor ) );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeStrokeWidth ) )
  {
    context.expressionContext().setOriginalValueVariable( d->strokeWidth );
    setStrokeWidth( properties.valueAsDouble( QgsPalLayerSettings::Property::ShapeStrokeWidth, context.expressionContext(), d->strokeWidth ) );
  }
  exprVal = properties.value( QgsPalLayerSettings::Property::ShapeStrokeWidthUnits, context.expressionContext() );
  if ( !QgsVariantUtils::isNull( exprVal ) )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const Qgis::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        setStrokeWidthUnit( res );
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::Property::ShapeBlendMode, context.expressionContext() );
    const QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  if ( properties.isActive( QgsPalLayerSettings::Property::ShapeJoinStyle ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::Property::ShapeJoinStyle, context.expressionContext() );
    const QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( joinstr ) );
    }
  }
}

QSet<QString> QgsTextBackgroundSettings::referencedFields( const QgsRenderContext &context ) const
{
  QSet< QString > fields;
  if ( d->markerSymbol )
  {
    fields.unite( d->markerSymbol->usedAttributes( context ) );
  }
  if ( d->fillSymbol )
  {
    fields.unite( d->fillSymbol->usedAttributes( context ) );
  }
  return fields;
}
