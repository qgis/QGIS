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
#include "qgstextrenderer_p.h"
#include "qgsvectorlayer.h"
#include "qgspallabeling.h"
#include "qgssymbollayerutils.h"
#include "qgsfillsymbollayer.h"
#include "qgspainting.h"
#include "qgstextrendererutils.h"
#include "qgspainteffectregistry.h"

QgsTextBackgroundSettings::QgsTextBackgroundSettings()
{
  d = new QgsTextBackgroundSettingsPrivate();

  // Create a default fill symbol to preserve API promise until QGIS 4.0
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

QgsTextBackgroundSettings &QgsTextBackgroundSettings::operator=( const QgsTextBackgroundSettings &other )  //NOLINT
{
  d = other.d;
  return *this;
}

QgsTextBackgroundSettings::~QgsTextBackgroundSettings() //NOLINT
{

}

bool QgsTextBackgroundSettings::operator==( const QgsTextBackgroundSettings &other ) const
{
  if ( !d->enabled == other.enabled()
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
  d->markerSymbol.reset( symbol );
}

QgsFillSymbol *QgsTextBackgroundSettings::fillSymbol() const
{
  return d->fillSymbol.get();
}

void QgsTextBackgroundSettings::setFillSymbol( QgsFillSymbol *symbol )
{
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

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::sizeUnit() const
{
  return d->sizeUnits;
}

void QgsTextBackgroundSettings::setSizeUnit( QgsUnitTypes::RenderUnit unit )
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

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::offsetUnit() const
{
  return d->offsetUnits;
}

void QgsTextBackgroundSettings::setOffsetUnit( QgsUnitTypes::RenderUnit units )
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

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::radiiUnit() const
{
  return d->radiiUnits;
}

void QgsTextBackgroundSettings::setRadiiUnit( QgsUnitTypes::RenderUnit units )
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
  return d->fillColor;
}

void QgsTextBackgroundSettings::setFillColor( const QColor &color )
{
  d->fillColor = color;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setColor( color );
  }
}

QColor QgsTextBackgroundSettings::strokeColor() const
{
  return d->strokeColor;
}

void QgsTextBackgroundSettings::setStrokeColor( const QColor &color )
{
  d->strokeColor = color;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeColor( color );
  }
}

double QgsTextBackgroundSettings::strokeWidth() const
{
  return d->strokeWidth;
}

void QgsTextBackgroundSettings::setStrokeWidth( double width )
{
  d->strokeWidth = width;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
  {
    QgsSimpleFillSymbolLayer *fill = qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) );
    fill->setStrokeWidth( width );
    fill->setStrokeStyle( !qgsDoubleNear( width, 0.0 ) ? Qt::SolidLine : Qt::NoPen );
  }
}

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::strokeWidthUnit() const
{
  return d->strokeWidthUnits;
}

void QgsTextBackgroundSettings::setStrokeWidthUnit( QgsUnitTypes::RenderUnit units )
{
  d->strokeWidthUnits = units;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeWidthUnit( units );
  }
}

QgsMapUnitScale QgsTextBackgroundSettings::strokeWidthMapUnitScale() const
{
  return d->strokeWidthMapUnitScale;
}

void QgsTextBackgroundSettings::setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale )
{
  d->strokeWidthMapUnitScale = scale;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
  {
    qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) )->setStrokeWidthMapUnitScale( scale );
  }
}

Qt::PenJoinStyle QgsTextBackgroundSettings::joinStyle() const
{
  return d->joinStyle;
}

void QgsTextBackgroundSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
  if ( d->fillSymbol && d->fillSymbol->symbolLayers().at( 0 )->layerType() == QLatin1String( "SimpleFill" ) )
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
  d->enabled = layer->customProperty( QStringLiteral( "labeling/shapeDraw" ), QVariant( false ) ).toBool();
  d->type = static_cast< ShapeType >( layer->customProperty( QStringLiteral( "labeling/shapeType" ), QVariant( ShapeRectangle ) ).toUInt() );
  d->svgFile = layer->customProperty( QStringLiteral( "labeling/shapeSVGFile" ), QVariant( "" ) ).toString();
  d->sizeType = static_cast< SizeType >( layer->customProperty( QStringLiteral( "labeling/shapeSizeType" ), QVariant( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( layer->customProperty( QStringLiteral( "labeling/shapeSizeX" ), QVariant( 0.0 ) ).toDouble(),
                    layer->customProperty( QStringLiteral( "labeling/shapeSizeY" ), QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( QStringLiteral( "labeling/shapeSizeUnit" ) ).toString().isEmpty() )
  {
    d->sizeUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeSizeUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeSizeUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitMaxScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitScale" ) ).toString() );
  }
  d->rotationType = static_cast< RotationType >( layer->customProperty( QStringLiteral( "labeling/shapeRotationType" ), QVariant( RotationSync ) ).toUInt() );
  d->rotation = layer->customProperty( QStringLiteral( "labeling/shapeRotation" ), QVariant( 0.0 ) ).toDouble();
  d->offset = QPointF( layer->customProperty( QStringLiteral( "labeling/shapeOffsetX" ), QVariant( 0.0 ) ).toDouble(),
                       layer->customProperty( QStringLiteral( "labeling/shapeOffsetY" ), QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( QStringLiteral( "labeling/shapeOffsetUnit" ) ).toString().isEmpty() )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeOffsetUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeOffsetUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitMinScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitScale" ) ).toString() );
  }
  d->radii = QSizeF( layer->customProperty( QStringLiteral( "labeling/shapeRadiiX" ), QVariant( 0.0 ) ).toDouble(),
                     layer->customProperty( QStringLiteral( "labeling/shapeRadiiY" ), QVariant( 0.0 ) ).toDouble() );


  if ( layer->customProperty( QStringLiteral( "labeling/shapeRadiiUnit" ) ).toString().isEmpty() )
  {
    d->radiiUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeRadiiUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeRadiiUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitMinScale" ), 0.0 ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitMaxScale" ), 0.0 ).toDouble();
    d->radiiMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitScale" ) ).toString() );
  }
  d->fillColor = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/shapeFillColor" ), Qt::white, true );
  d->strokeColor = QgsTextRendererUtils::readColor( layer, QStringLiteral( "labeling/shapeBorderColor" ), Qt::darkGray, true );
  d->strokeWidth = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidth" ), QVariant( .0 ) ).toDouble();
  if ( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnit" ) ).toString().isEmpty() )
  {
    d->strokeWidthUnits = QgsTextRendererUtils::convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    const double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitMinScale" ), 0.0 ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitMaxScale" ), 0.0 ).toDouble();
    d->strokeWidthMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->strokeWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitScale" ) ).toString() );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( layer->customProperty( QStringLiteral( "labeling/shapeJoinStyle" ), QVariant( Qt::BevelJoin ) ).toUInt() );

  if ( layer->customProperty( QStringLiteral( "labeling/shapeOpacity" ) ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( QStringLiteral( "labeling/shapeTransparency" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( QStringLiteral( "labeling/shapeOpacity" ) ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/shapeBlendMode" ), QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );

  if ( layer->customProperty( QStringLiteral( "labeling/shapeEffect" ) ).isValid() )
  {
    QDomDocument doc( QStringLiteral( "effect" ) );
    doc.setContent( layer->customProperty( QStringLiteral( "labeling/shapeEffect" ) ).toString() );
    const QDomElement effectElem = doc.firstChildElement( QStringLiteral( "effect" ) ).firstChildElement( QStringLiteral( "effect" ) );
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  }
  else
    setPaintEffect( nullptr );
}

void QgsTextBackgroundSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  const QDomElement backgroundElem = elem.firstChildElement( QStringLiteral( "background" ) );
  d->enabled = backgroundElem.attribute( QStringLiteral( "shapeDraw" ), QStringLiteral( "0" ) ).toInt();
  d->type = static_cast< ShapeType >( backgroundElem.attribute( QStringLiteral( "shapeType" ), QString::number( ShapeRectangle ) ).toUInt() );
  d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( backgroundElem.attribute( QStringLiteral( "shapeSVGFile" ) ), context.pathResolver() );
  d->sizeType = static_cast< SizeType >( backgroundElem.attribute( QStringLiteral( "shapeSizeType" ), QString::number( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( backgroundElem.attribute( QStringLiteral( "shapeSizeX" ), QStringLiteral( "0" ) ).toDouble(),
                    backgroundElem.attribute( QStringLiteral( "shapeSizeY" ), QStringLiteral( "0" ) ).toDouble() );

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeSizeUnit" ) ) )
  {
    d->sizeUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeSizeUnits" ) ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeSizeUnit" ) ) );
  }

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( QStringLiteral( "shapeSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( QStringLiteral( "shapeSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( QStringLiteral( "shapeSizeMapUnitScale" ) ) );
  }
  d->rotationType = static_cast< RotationType >( backgroundElem.attribute( QStringLiteral( "shapeRotationType" ), QString::number( RotationSync ) ).toUInt() );
  d->rotation = backgroundElem.attribute( QStringLiteral( "shapeRotation" ), QStringLiteral( "0" ) ).toDouble();
  d->offset = QPointF( backgroundElem.attribute( QStringLiteral( "shapeOffsetX" ), QStringLiteral( "0" ) ).toDouble(),
                       backgroundElem.attribute( QStringLiteral( "shapeOffsetY" ), QStringLiteral( "0" ) ).toDouble() );

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeOffsetUnit" ) ) )
  {
    d->offsetUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeOffsetUnits" ) ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeOffsetUnit" ) ) );
  }

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeOffsetMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( QStringLiteral( "shapeOffsetMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( QStringLiteral( "shapeOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( QStringLiteral( "shapeOffsetMapUnitScale" ) ) );
  }
  d->radii = QSizeF( backgroundElem.attribute( QStringLiteral( "shapeRadiiX" ), QStringLiteral( "0" ) ).toDouble(),
                     backgroundElem.attribute( QStringLiteral( "shapeRadiiY" ), QStringLiteral( "0" ) ).toDouble() );

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeRadiiUnit" ) ) )
  {
    d->radiiUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeRadiiUnits" ) ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeRadiiUnit" ) ) );
  }
  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeRadiiMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( QStringLiteral( "shapeRadiiMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( QStringLiteral( "shapeRadiiMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiiMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( QStringLiteral( "shapeRadiiMapUnitScale" ) ) );
  }
  d->fillColor = QgsSymbolLayerUtils::decodeColor( backgroundElem.attribute( QStringLiteral( "shapeFillColor" ), QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );
  d->strokeColor = QgsSymbolLayerUtils::decodeColor( backgroundElem.attribute( QStringLiteral( "shapeBorderColor" ), QgsSymbolLayerUtils::encodeColor( Qt::darkGray ) ) );
  d->strokeWidth = backgroundElem.attribute( QStringLiteral( "shapeBorderWidth" ), QStringLiteral( "0" ) ).toDouble();

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeBorderWidthUnit" ) ) )
  {
    d->strokeWidthUnits = QgsTextRendererUtils::convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeBorderWidthUnits" ) ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeBorderWidthUnit" ) ) );
  }
  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeBorderWidthMapUnitScale" ) ) )
  {
    //fallback to older property
    const double oldMin = backgroundElem.attribute( QStringLiteral( "shapeBorderWidthMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    const double oldMax = backgroundElem.attribute( QStringLiteral( "shapeBorderWidthMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->strokeWidthMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->strokeWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( QStringLiteral( "shapeBorderWidthMapUnitScale" ) ) );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( backgroundElem.attribute( QStringLiteral( "shapeJoinStyle" ), QString::number( Qt::BevelJoin ) ).toUInt() );

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeOpacity" ) ) )
  {
    d->opacity = ( 1 - backgroundElem.attribute( QStringLiteral( "shapeTransparency" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( backgroundElem.attribute( QStringLiteral( "shapeOpacity" ) ).toDouble() );
  }

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( backgroundElem.attribute( QStringLiteral( "shapeBlendMode" ), QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );

  const QDomElement effectElem = backgroundElem.firstChildElement( QStringLiteral( "effect" ) );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( nullptr );

  setMarkerSymbol( nullptr );
  setFillSymbol( nullptr );
  const QDomNodeList symbols = backgroundElem.elementsByTagName( QStringLiteral( "symbol" ) );
  for ( int i = 0; i < symbols.size(); ++i )
  {
    if ( symbols.at( i ).isElement() )
    {
      const QDomElement symbolElement = symbols.at( i ).toElement();
      const QString symbolElementName = symbolElement.attribute( QStringLiteral( "name" ) );
      if ( symbolElementName == QLatin1String( "markerSymbol" ) )
      {
        setMarkerSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElement, context ) );
      }
      else if ( symbolElementName == QLatin1String( "fillSymbol" ) )
      {
        setFillSymbol( QgsSymbolLayerUtils::loadSymbol< QgsFillSymbol >( symbolElement, context ) );
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
  QDomElement backgroundElem = doc.createElement( QStringLiteral( "background" ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeDraw" ), d->enabled );
  backgroundElem.setAttribute( QStringLiteral( "shapeType" ), static_cast< unsigned int >( d->type ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeSVGFile" ), QgsSymbolLayerUtils::svgSymbolPathToName( d->svgFile, context.pathResolver() ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeSizeType" ), static_cast< unsigned int >( d->sizeType ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeSizeX" ), d->size.width() );
  backgroundElem.setAttribute( QStringLiteral( "shapeSizeY" ), d->size.height() );
  backgroundElem.setAttribute( QStringLiteral( "shapeSizeUnit" ), QgsUnitTypes::encodeUnit( d->sizeUnits ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeRotationType" ), static_cast< unsigned int >( d->rotationType ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeRotation" ), d->rotation );
  backgroundElem.setAttribute( QStringLiteral( "shapeOffsetX" ), d->offset.x() );
  backgroundElem.setAttribute( QStringLiteral( "shapeOffsetY" ), d->offset.y() );
  backgroundElem.setAttribute( QStringLiteral( "shapeOffsetUnit" ), QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeOffsetMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeRadiiX" ), d->radii.width() );
  backgroundElem.setAttribute( QStringLiteral( "shapeRadiiY" ), d->radii.height() );
  backgroundElem.setAttribute( QStringLiteral( "shapeRadiiUnit" ), QgsUnitTypes::encodeUnit( d->radiiUnits ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeRadiiMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->radiiMapUnitScale ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeFillColor" ), QgsSymbolLayerUtils::encodeColor( d->fillColor ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeBorderColor" ), QgsSymbolLayerUtils::encodeColor( d->strokeColor ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeBorderWidth" ), d->strokeWidth );
  backgroundElem.setAttribute( QStringLiteral( "shapeBorderWidthUnit" ), QgsUnitTypes::encodeUnit( d->strokeWidthUnits ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeBorderWidthMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->strokeWidthMapUnitScale ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeJoinStyle" ), static_cast< unsigned int >( d->joinStyle ) );
  backgroundElem.setAttribute( QStringLiteral( "shapeOpacity" ), d->opacity );
  backgroundElem.setAttribute( QStringLiteral( "shapeBlendMode" ), QgsPainting::getBlendModeEnum( d->blendMode ) );
  if ( d->paintEffect && !QgsPaintEffectRegistry::isDefaultStack( d->paintEffect.get() ) )
    d->paintEffect->saveProperties( doc, backgroundElem );

  if ( d->markerSymbol )
    backgroundElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "markerSymbol" ), d->markerSymbol.get(), doc, context ) );

  if ( d->fillSymbol )
    backgroundElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "fillSymbol" ), d->fillSymbol.get(), doc, context ) );

  return backgroundElem;
}

void QgsTextBackgroundSettings::upgradeDataDefinedProperties( QgsPropertyCollection &properties )
{
  if ( !d->fillSymbol || d->fillSymbol->symbolLayers().at( 0 )->layerType() != QLatin1String( "SimpleFill" ) )
    return;
  QgsSimpleFillSymbolLayer *fill = qgis::down_cast< QgsSimpleFillSymbolLayer * >( d->fillSymbol->symbolLayers().at( 0 ) );

  if ( d->type != QgsTextBackgroundSettings::ShapeSVG )
  {
    if ( properties.hasProperty( QgsPalLayerSettings::ShapeFillColor ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::PropertyFillColor ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyFillColor, properties.property( QgsPalLayerSettings::ShapeFillColor ) );
      properties.setProperty( QgsPalLayerSettings::ShapeFillColor, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::ShapeStrokeColor ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::PropertyStrokeColor ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyStrokeColor, properties.property( QgsPalLayerSettings::ShapeStrokeColor ) );
      properties.setProperty( QgsPalLayerSettings::ShapeStrokeColor, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::ShapeStrokeWidth ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::PropertyStrokeWidth ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyStrokeWidth, properties.property( QgsPalLayerSettings::ShapeStrokeWidth ) );
      properties.setProperty( QgsPalLayerSettings::ShapeStrokeWidth, QgsProperty() );
    }

    if ( properties.hasProperty( QgsPalLayerSettings::ShapeJoinStyle ) &&
         !fill->dataDefinedProperties().hasProperty( QgsSymbolLayer::PropertyJoinStyle ) )
    {
      fill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyJoinStyle, properties.property( QgsPalLayerSettings::ShapeJoinStyle ) );
      properties.setProperty( QgsPalLayerSettings::ShapeJoinStyle, QgsProperty() );
    }
  }
}

void QgsTextBackgroundSettings::updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties )
{
  if ( properties.isActive( QgsPalLayerSettings::ShapeDraw ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::ShapeDraw, context.expressionContext(), d->enabled );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeSizeX ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size.width() );
    d->size.setWidth( properties.valueAsDouble( QgsPalLayerSettings::ShapeSizeX, context.expressionContext(), d->size.width() ) );
  }
  if ( properties.isActive( QgsPalLayerSettings::ShapeSizeY ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size.height() );
    d->size.setHeight( properties.valueAsDouble( QgsPalLayerSettings::ShapeSizeY, context.expressionContext(), d->size.height() ) );
  }

  QVariant exprVal = properties.value( QgsPalLayerSettings::ShapeSizeUnits, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->sizeUnits = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeKind, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString skind = exprVal.toString().trimmed();
    if ( !skind.isEmpty() )
    {
      d->type = QgsTextRendererUtils::decodeShapeType( skind );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeSizeType, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString stype = exprVal.toString().trimmed();
    if ( !stype.isEmpty() )
    {
      d->sizeType = QgsTextRendererUtils::decodeBackgroundSizeType( stype );
    }
  }

  // data defined shape SVG path?
  context.expressionContext().setOriginalValueVariable( d->svgFile );
  exprVal = properties.value( QgsPalLayerSettings::ShapeSVGFile, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString svgfile = exprVal.toString().trimmed();
    d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeRotation ) )
  {
    context.expressionContext().setOriginalValueVariable( d->rotation );
    d->rotation = properties.valueAsDouble( QgsPalLayerSettings::ShapeRotation, context.expressionContext(), d->rotation );
  }
  exprVal = properties.value( QgsPalLayerSettings::ShapeRotationType, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString rotstr = exprVal.toString().trimmed();
    if ( !rotstr.isEmpty() )
    {
      d->rotationType = QgsTextRendererUtils::decodeBackgroundRotationType( rotstr );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeOffset, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      d->offset = res;
    }
  }
  exprVal = properties.value( QgsPalLayerSettings::ShapeOffsetUnits, context.expressionContext() );
  if ( !exprVal.isNull() )
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

  exprVal = properties.value( QgsPalLayerSettings::ShapeRadii, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    bool ok = false;
    const QSizeF res = QgsSymbolLayerUtils::toSize( exprVal, &ok );
    if ( ok )
    {
      d->radii = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeRadiiUnits, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiiUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    const QVariant val = properties.value( QgsPalLayerSettings::ShapeOpacity, context.expressionContext(), d->opacity * 100 );
    if ( !val.isNull() )
    {
      d->opacity = val.toDouble() / 100.0;
    }
  }

  // for non-SVG background types, those data defined properties will not having an impact,
  // instead use data defined properties within symbols
  if ( properties.isActive( QgsPalLayerSettings::ShapeFillColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->fillColor ) );
    d->fillColor = properties.valueAsColor( QgsPalLayerSettings::ShapeFillColor, context.expressionContext(), d->fillColor );
  }
  if ( properties.isActive( QgsPalLayerSettings::ShapeStrokeColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->strokeColor ) );
    d->strokeColor = properties.valueAsColor( QgsPalLayerSettings::ShapeStrokeColor, context.expressionContext(), d->strokeColor );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeStrokeWidth ) )
  {
    context.expressionContext().setOriginalValueVariable( d->strokeWidth );
    d->strokeWidth = properties.valueAsDouble( QgsPalLayerSettings::ShapeStrokeWidth, context.expressionContext(), d->strokeWidth );
  }
  exprVal = properties.value( QgsPalLayerSettings::ShapeStrokeWidthUnits, context.expressionContext() );
  if ( !exprVal.isNull() )
  {
    const QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      const QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->strokeWidthUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::ShapeBlendMode, context.expressionContext() );
    const QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeJoinStyle ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::ShapeJoinStyle, context.expressionContext() );
    const QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
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
