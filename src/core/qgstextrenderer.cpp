/***************************************************************************
  qgstextrenderer.cpp
  -------------------s
   begin                : September 2015
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

#include "qgstextrenderer.h"
#include "qgstextrenderer_p.h"
#include "qgsfontutils.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgspainting.h"
#include <QFontDatabase>

static QColor _readColor( QgsVectorLayer* layer, const QString& property, const QColor& defaultColor = Qt::black, bool withAlpha = true )
{
  int r = layer->customProperty( property + 'R', QVariant( defaultColor.red() ) ).toInt();
  int g = layer->customProperty( property + 'G', QVariant( defaultColor.green() ) ).toInt();
  int b = layer->customProperty( property + 'B', QVariant( defaultColor.blue() ) ).toInt();
  int a = withAlpha ? layer->customProperty( property + 'A', QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

static void _writeColor( QgsVectorLayer* layer, const QString& property, const QColor& color, bool withAlpha = true )
{
  layer->setCustomProperty( property + 'R', color.red() );
  layer->setCustomProperty( property + 'G', color.green() );
  layer->setCustomProperty( property + 'B', color.blue() );
  if ( withAlpha )
    layer->setCustomProperty( property + 'A', color.alpha() );
}

QgsTextBufferSettings::QgsTextBufferSettings()
{
  d = new QgsTextBufferSettingsPrivate();
}

QgsTextBufferSettings::QgsTextBufferSettings( const QgsTextBufferSettings &other )
    : d( other.d )
{
}

QgsTextBufferSettings &QgsTextBufferSettings::operator=( const QgsTextBufferSettings & other )
{
  d = other.d;
  return *this;
}

QgsTextBufferSettings::~QgsTextBufferSettings()
{

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

void QgsTextBufferSettings::readFromLayer( QgsVectorLayer* layer )
{
  // text buffer
  double bufSize = layer->customProperty( "labeling/bufferSize", QVariant( 0.0 ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  QVariant drawBuffer = layer->customProperty( "labeling/bufferDraw", QVariant() );
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

  if ( layer->customProperty( "labeling/bufferSizeUnits" ).toString().isEmpty() )
  {
    bool bufferSizeInMapUnits = layer->customProperty( "labeling/bufferSizeInMapUnits" ).toBool();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/bufferSizeUnits" ).toString() );
  }

  if ( layer->customProperty( "labeling/bufferSizeMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->sizeMapUnitScale.minScale = layer->customProperty( "labeling/bufferSizeMapUnitMinScale", 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = layer->customProperty( "labeling/bufferSizeMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/bufferSizeMapUnitScale" ).toString() );
  }
  d->color = _readColor( layer, "labeling/bufferColor", Qt::white, false );
  if ( layer->customProperty( "labeling/bufferOpacity" ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( "labeling/bufferTransp" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( "labeling/bufferOpacity" ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( "labeling/bufferBlendMode", QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( layer->customProperty( "labeling/bufferJoinStyle", QVariant( Qt::RoundJoin ) ).toUInt() );

  d->fillBufferInterior = !layer->customProperty( "labeling/bufferNoFill", QVariant( false ) ).toBool();
}

void QgsTextBufferSettings::writeToLayer( QgsVectorLayer* layer ) const
{
  layer->setCustomProperty( "labeling/bufferDraw", d->enabled );
  layer->setCustomProperty( "labeling/bufferSize", d->size );
  layer->setCustomProperty( "labeling/bufferSizeUnits", QgsUnitTypes::encodeUnit( d->sizeUnit ) );
  layer->setCustomProperty( "labeling/bufferSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  _writeColor( layer, "labeling/bufferColor", d->color );
  layer->setCustomProperty( "labeling/bufferNoFill", !d->fillBufferInterior );
  layer->setCustomProperty( "labeling/bufferOpacity", d->opacity );
  layer->setCustomProperty( "labeling/bufferJoinStyle", static_cast< unsigned int >( d->joinStyle ) );
  layer->setCustomProperty( "labeling/bufferBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
}

void QgsTextBufferSettings::readXml( const QDomElement& elem )
{
  QDomElement textBufferElem = elem.firstChildElement( "text-buffer" );
  double bufSize = textBufferElem.attribute( "bufferSize", "0" ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  QVariant drawBuffer = textBufferElem.attribute( "bufferDraw" );
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

  if ( !textBufferElem.hasAttribute( "bufferSizeUnits" ) )
  {
    bool bufferSizeInMapUnits = textBufferElem.attribute( "bufferSizeInMapUnits" ).toInt();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( textBufferElem.attribute( "bufferSizeUnits" ) );
  }

  if ( !textBufferElem.hasAttribute( "bufferSizeMapUnitScale" ) )
  {
    //fallback to older property
    d->sizeMapUnitScale.minScale = textBufferElem.attribute( "bufferSizeMapUnitMinScale", "0" ).toDouble();
    d->sizeMapUnitScale.maxScale = textBufferElem.attribute( "bufferSizeMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textBufferElem.attribute( "bufferSizeMapUnitScale" ) );
  }
  d->color = QgsSymbolLayerUtils::decodeColor( textBufferElem.attribute( "bufferColor", QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );

  if ( !textBufferElem.hasAttribute( "bufferOpacity" ) )
  {
    d->opacity = ( 1 - textBufferElem.attribute( "bufferTransp" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( textBufferElem.attribute( "bufferOpacity" ).toDouble() );
  }

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( textBufferElem.attribute( "bufferBlendMode", QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( textBufferElem.attribute( "bufferJoinStyle", QString::number( Qt::RoundJoin ) ).toUInt() );
  d->fillBufferInterior = !textBufferElem.attribute( "bufferNoFill", "0" ).toInt();
}

QDomElement QgsTextBufferSettings::writeXml( QDomDocument& doc ) const
{
  // text buffer
  QDomElement textBufferElem = doc.createElement( "text-buffer" );
  textBufferElem.setAttribute( "bufferDraw", d->enabled );
  textBufferElem.setAttribute( "bufferSize", d->size );
  textBufferElem.setAttribute( "bufferSizeUnits", QgsUnitTypes::encodeUnit( d->sizeUnit ) );
  textBufferElem.setAttribute( "bufferSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  textBufferElem.setAttribute( "bufferColor", QgsSymbolLayerUtils::encodeColor( d->color ) );
  textBufferElem.setAttribute( "bufferNoFill", !d->fillBufferInterior );
  textBufferElem.setAttribute( "bufferOpacity", d->opacity );
  textBufferElem.setAttribute( "bufferJoinStyle", static_cast< unsigned int >( d->joinStyle ) );
  textBufferElem.setAttribute( "bufferBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
  return textBufferElem;
}


//
// QgsTextBackgroundSettings
//

QgsTextBackgroundSettings::QgsTextBackgroundSettings()
{
  d = new QgsTextBackgroundSettingsPrivate();
}

QgsTextBackgroundSettings::QgsTextBackgroundSettings( const QgsTextBackgroundSettings &other )
    : d( other.d )
{

}

QgsTextBackgroundSettings &QgsTextBackgroundSettings::operator=( const QgsTextBackgroundSettings & other )
{
  d = other.d;
  return *this;
}

QgsTextBackgroundSettings::~QgsTextBackgroundSettings()
{

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

void QgsTextBackgroundSettings::setSize( const QSizeF &size )
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

void QgsTextBackgroundSettings::setOffset( const QPointF &offset )
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

void QgsTextBackgroundSettings::setRadii( const QSizeF &radii )
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
}

QColor QgsTextBackgroundSettings::borderColor() const
{
  return d->borderColor;
}

void QgsTextBackgroundSettings::setBorderColor( const QColor &color )
{
  d->borderColor = color;
}

double QgsTextBackgroundSettings::borderWidth() const
{
  return d->borderWidth;
}

void QgsTextBackgroundSettings::setBorderWidth( double width )
{
  d->borderWidth = width;
}

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::borderWidthUnit() const
{
  return d->borderWidthUnits;
}

void QgsTextBackgroundSettings::setBorderWidthUnit( QgsUnitTypes::RenderUnit units )
{
  d->borderWidthUnits = units;
}

QgsMapUnitScale QgsTextBackgroundSettings::borderWidthMapUnitScale() const
{
  return d->borderWidthMapUnitScale;
}

void QgsTextBackgroundSettings::setBorderWidthMapUnitScale( const QgsMapUnitScale &scale )
{
  d->borderWidthMapUnitScale = scale;
}

Qt::PenJoinStyle QgsTextBackgroundSettings::joinStyle() const
{
  return d->joinStyle;
}

void QgsTextBackgroundSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
}

void QgsTextBackgroundSettings::readFromLayer( QgsVectorLayer* layer )
{
  d->enabled = layer->customProperty( "labeling/shapeDraw", QVariant( false ) ).toBool();
  d->type = static_cast< ShapeType >( layer->customProperty( "labeling/shapeType", QVariant( ShapeRectangle ) ).toUInt() );
  d->svgFile = layer->customProperty( "labeling/shapeSVGFile", QVariant( "" ) ).toString();
  d->sizeType = static_cast< SizeType >( layer->customProperty( "labeling/shapeSizeType", QVariant( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( layer->customProperty( "labeling/shapeSizeX", QVariant( 0.0 ) ).toDouble(),
                    layer->customProperty( "labeling/shapeSizeY", QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( "labeling/shapeSizeUnit" ).toString().isEmpty() )
  {
    d->sizeUnits = layer->customProperty( "labeling/shapeSizeUnits", 0 ).toUInt() == 0 ?
                   QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shapeSizeUnit" ).toString() );
  }

  if ( layer->customProperty( "labeling/shapeSizeMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->sizeMapUnitScale.minScale = layer->customProperty( "labeling/shapeSizeMapUnitMinScale", 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = layer->customProperty( "labeling/shapeSizeMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shapeSizeMapUnitScale" ).toString() );
  }
  d->rotationType = static_cast< RotationType >( layer->customProperty( "labeling/shapeRotationType", QVariant( RotationSync ) ).toUInt() );
  d->rotation = layer->customProperty( "labeling/shapeRotation", QVariant( 0.0 ) ).toDouble();
  d->offset = QPointF( layer->customProperty( "labeling/shapeOffsetX", QVariant( 0.0 ) ).toDouble(),
                       layer->customProperty( "labeling/shapeOffsetY", QVariant( 0.0 ) ).toDouble() );

  if ( layer->customProperty( "labeling/shapeOffsetUnit" ).toString().isEmpty() )
  {
    d->offsetUnits = layer->customProperty( "labeling/shapeOffsetUnits", 0 ).toUInt() == 0 ?
                     QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shapeOffsetUnit" ).toString() );
  }

  if ( layer->customProperty( "labeling/shapeOffsetMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->offsetMapUnitScale.minScale = layer->customProperty( "labeling/shapeOffsetMapUnitMinScale", 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = layer->customProperty( "labeling/shapeOffsetMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shapeOffsetMapUnitScale" ).toString() );
  }
  d->radii = QSizeF( layer->customProperty( "labeling/shapeRadiiX", QVariant( 0.0 ) ).toDouble(),
                     layer->customProperty( "labeling/shapeRadiiY", QVariant( 0.0 ) ).toDouble() );


  if ( layer->customProperty( "labeling/shapeRadiiUnit" ).toString().isEmpty() )
  {
    d->radiiUnits = layer->customProperty( "labeling/shapeRadiiUnits", 0 ).toUInt() == 0 ?
                    QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shapeRadiiUnit" ).toString() );
  }

  if ( layer->customProperty( "labeling/shapeRadiiMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->radiiMapUnitScale.minScale = layer->customProperty( "labeling/shapeRadiiMapUnitMinScale", 0.0 ).toDouble();
    d->radiiMapUnitScale.maxScale = layer->customProperty( "labeling/shapeRadiiMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shapeRadiiMapUnitScale" ).toString() );
  }
  d->fillColor = _readColor( layer, "labeling/shapeFillColor", Qt::white, true );
  d->borderColor = _readColor( layer, "labeling/shapeBorderColor", Qt::darkGray, true );
  d->borderWidth = layer->customProperty( "labeling/shapeBorderWidth", QVariant( .0 ) ).toDouble();
  if ( layer->customProperty( "labeling/shapeBorderWidthUnit" ).toString().isEmpty() )
  {
    d->borderWidthUnits = layer->customProperty( "labeling/shapeBorderWidthUnits", 0 ).toUInt() == 0 ?
                          QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->borderWidthUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shapeBorderWidthUnit" ).toString() );
  }
  if ( layer->customProperty( "labeling/shapeBorderWidthMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->borderWidthMapUnitScale.minScale = layer->customProperty( "labeling/shapeBorderWidthMapUnitMinScale", 0.0 ).toDouble();
    d->borderWidthMapUnitScale.maxScale = layer->customProperty( "labeling/shapeBorderWidthMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->borderWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shapeBorderWidthMapUnitScale" ).toString() );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( layer->customProperty( "labeling/shapeJoinStyle", QVariant( Qt::BevelJoin ) ).toUInt() );

  if ( layer->customProperty( "labeling/shapeOpacity" ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( "labeling/shapeTransparency" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( "labeling/shapeOpacity" ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( "labeling/shapeBlendMode", QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
}

void QgsTextBackgroundSettings::writeToLayer( QgsVectorLayer* layer ) const
{
  layer->setCustomProperty( "labeling/shapeDraw", d->enabled );
  layer->setCustomProperty( "labeling/shapeType", static_cast< unsigned int >( d->type ) );
  layer->setCustomProperty( "labeling/shapeSVGFile", d->svgFile );
  layer->setCustomProperty( "labeling/shapeSizeType", static_cast< unsigned int >( d->sizeType ) );
  layer->setCustomProperty( "labeling/shapeSizeX", d->size.width() );
  layer->setCustomProperty( "labeling/shapeSizeY", d->size.height() );
  layer->setCustomProperty( "labeling/shapeSizeUnit", QgsUnitTypes::encodeUnit( d->sizeUnits ) );
  layer->setCustomProperty( "labeling/shapeSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  layer->setCustomProperty( "labeling/shapeRotationType", static_cast< unsigned int >( d->rotationType ) );
  layer->setCustomProperty( "labeling/shapeRotation", d->rotation );
  layer->setCustomProperty( "labeling/shapeOffsetX", d->offset.x() );
  layer->setCustomProperty( "labeling/shapeOffsetY", d->offset.y() );
  layer->setCustomProperty( "labeling/shapeOffsetUnit", QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  layer->setCustomProperty( "labeling/shapeOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  layer->setCustomProperty( "labeling/shapeRadiiX", d->radii.width() );
  layer->setCustomProperty( "labeling/shapeRadiiY", d->radii.height() );
  layer->setCustomProperty( "labeling/shapeRadiiUnit", QgsUnitTypes::encodeUnit( d->radiiUnits ) );
  layer->setCustomProperty( "labeling/shapeRadiiMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->radiiMapUnitScale ) );
  _writeColor( layer, "labeling/shapeFillColor", d->fillColor, true );
  _writeColor( layer, "labeling/shapeBorderColor", d->borderColor, true );
  layer->setCustomProperty( "labeling/shapeBorderWidth", d->borderWidth );
  layer->setCustomProperty( "labeling/shapeBorderWidthUnit", QgsUnitTypes::encodeUnit( d->borderWidthUnits ) );
  layer->setCustomProperty( "labeling/shapeBorderWidthMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->borderWidthMapUnitScale ) );
  layer->setCustomProperty( "labeling/shapeJoinStyle", static_cast< unsigned int >( d->joinStyle ) );
  layer->setCustomProperty( "labeling/shapeOpacity", d->opacity );
  layer->setCustomProperty( "labeling/shapeBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
}

void QgsTextBackgroundSettings::readXml( const QDomElement& elem )
{
  QDomElement backgroundElem = elem.firstChildElement( "background" );
  d->enabled = backgroundElem.attribute( "shapeDraw", "0" ).toInt();
  d->type = static_cast< ShapeType >( backgroundElem.attribute( "shapeType", QString::number( ShapeRectangle ) ).toUInt() );
  d->svgFile = backgroundElem.attribute( "shapeSVGFile" );
  d->sizeType = static_cast< SizeType >( backgroundElem.attribute( "shapeSizeType", QString::number( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( backgroundElem.attribute( "shapeSizeX", "0" ).toDouble(),
                    backgroundElem.attribute( "shapeSizeY", "0" ).toDouble() );

  if ( !backgroundElem.hasAttribute( "shapeSizeUnit" ) )
  {
    d->sizeUnits = backgroundElem.attribute( "shapeSizeUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                   : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( "shapeSizeUnit" ) );
  }

  if ( !backgroundElem.hasAttribute( "shapeSizeMapUnitScale" ) )
  {
    //fallback to older property
    d->sizeMapUnitScale.minScale = backgroundElem.attribute( "shapeSizeMapUnitMinScale", "0" ).toDouble();
    d->sizeMapUnitScale.maxScale = backgroundElem.attribute( "shapeSizeMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( "shapeSizeMapUnitScale" ) );
  }
  d->rotationType = static_cast< RotationType >( backgroundElem.attribute( "shapeRotationType", QString::number( RotationSync ) ).toUInt() );
  d->rotation = backgroundElem.attribute( "shapeRotation", "0" ).toDouble();
  d->offset = QPointF( backgroundElem.attribute( "shapeOffsetX", "0" ).toDouble(),
                       backgroundElem.attribute( "shapeOffsetY", "0" ).toDouble() );

  if ( !backgroundElem.hasAttribute( "shapeOffsetUnit" ) )
  {
    d->offsetUnits = backgroundElem.attribute( "shapeOffsetUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                     : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( "shapeOffsetUnit" ) );
  }

  if ( !backgroundElem.hasAttribute( "shapeOffsetMapUnitScale" ) )
  {
    //fallback to older property
    d->offsetMapUnitScale.minScale = backgroundElem.attribute( "shapeOffsetMapUnitMinScale", "0" ).toDouble();
    d->offsetMapUnitScale.maxScale = backgroundElem.attribute( "shapeOffsetMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( "shapeOffsetMapUnitScale" ) );
  }
  d->radii = QSizeF( backgroundElem.attribute( "shapeRadiiX", "0" ).toDouble(),
                     backgroundElem.attribute( "shapeRadiiY", "0" ).toDouble() );

  if ( !backgroundElem.hasAttribute( "shapeRadiiUnit" ) )
  {
    d->radiiUnits = backgroundElem.attribute( "shapeRadiiUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                    : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( "shapeRadiiUnit" ) );
  }
  if ( !backgroundElem.hasAttribute( "shapeRadiiMapUnitScale" ) )
  {
    //fallback to older property
    d->radiiMapUnitScale.minScale = backgroundElem.attribute( "shapeRadiiMapUnitMinScale", "0" ).toDouble();
    d->radiiMapUnitScale.maxScale = backgroundElem.attribute( "shapeRadiiMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( "shapeRadiiMapUnitScale" ) );
  }
  d->fillColor = QgsSymbolLayerUtils::decodeColor( backgroundElem.attribute( "shapeFillColor", QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );
  d->borderColor = QgsSymbolLayerUtils::decodeColor( backgroundElem.attribute( "shapeBorderColor", QgsSymbolLayerUtils::encodeColor( Qt::darkGray ) ) );
  d->borderWidth = backgroundElem.attribute( "shapeBorderWidth", "0" ).toDouble();

  if ( !backgroundElem.hasAttribute( "shapeBorderWidthUnit" ) )
  {
    d->borderWidthUnits = backgroundElem.attribute( "shapeBorderWidthUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                          : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->borderWidthUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( "shapeBorderWidthUnit" ) );
  }
  if ( !backgroundElem.hasAttribute( "shapeBorderWidthMapUnitScale" ) )
  {
    //fallback to older property
    d->borderWidthMapUnitScale.minScale = backgroundElem.attribute( "shapeBorderWidthMapUnitMinScale", "0" ).toDouble();
    d->borderWidthMapUnitScale.maxScale = backgroundElem.attribute( "shapeBorderWidthMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->borderWidthMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( backgroundElem.attribute( "shapeBorderWidthMapUnitScale" ) );
  }
  d->joinStyle = static_cast< Qt::PenJoinStyle >( backgroundElem.attribute( "shapeJoinStyle", QString::number( Qt::BevelJoin ) ).toUInt() );

  if ( !backgroundElem.hasAttribute( "shapeOpacity" ) )
  {
    d->opacity = ( 1 - backgroundElem.attribute( "shapeTransparency" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( backgroundElem.attribute( "shapeOpacity" ).toDouble() );
  }

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( backgroundElem.attribute( "shapeBlendMode", QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );

}

QDomElement QgsTextBackgroundSettings::writeXml( QDomDocument& doc ) const
{
  QDomElement backgroundElem = doc.createElement( "background" );
  backgroundElem.setAttribute( "shapeDraw", d->enabled );
  backgroundElem.setAttribute( "shapeType", static_cast< unsigned int >( d->type ) );
  backgroundElem.setAttribute( "shapeSVGFile", d->svgFile );
  backgroundElem.setAttribute( "shapeSizeType", static_cast< unsigned int >( d->sizeType ) );
  backgroundElem.setAttribute( "shapeSizeX", d->size.width() );
  backgroundElem.setAttribute( "shapeSizeY", d->size.height() );
  backgroundElem.setAttribute( "shapeSizeUnit", QgsUnitTypes::encodeUnit( d->sizeUnits ) );
  backgroundElem.setAttribute( "shapeSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  backgroundElem.setAttribute( "shapeRotationType", static_cast< unsigned int >( d->rotationType ) );
  backgroundElem.setAttribute( "shapeRotation", d->rotation );
  backgroundElem.setAttribute( "shapeOffsetX", d->offset.x() );
  backgroundElem.setAttribute( "shapeOffsetY", d->offset.y() );
  backgroundElem.setAttribute( "shapeOffsetUnit", QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  backgroundElem.setAttribute( "shapeOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  backgroundElem.setAttribute( "shapeRadiiX", d->radii.width() );
  backgroundElem.setAttribute( "shapeRadiiY", d->radii.height() );
  backgroundElem.setAttribute( "shapeRadiiUnit", QgsUnitTypes::encodeUnit( d->radiiUnits ) );
  backgroundElem.setAttribute( "shapeRadiiMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->radiiMapUnitScale ) );
  backgroundElem.setAttribute( "shapeFillColor", QgsSymbolLayerUtils::encodeColor( d->fillColor ) );
  backgroundElem.setAttribute( "shapeBorderColor", QgsSymbolLayerUtils::encodeColor( d->borderColor ) );
  backgroundElem.setAttribute( "shapeBorderWidth", d->borderWidth );
  backgroundElem.setAttribute( "shapeBorderWidthUnit", QgsUnitTypes::encodeUnit( d->borderWidthUnits ) );
  backgroundElem.setAttribute( "shapeBorderWidthMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->borderWidthMapUnitScale ) );
  backgroundElem.setAttribute( "shapeJoinStyle", static_cast< unsigned int >( d->joinStyle ) );
  backgroundElem.setAttribute( "shapeOpacity", d->opacity );
  backgroundElem.setAttribute( "shapeBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
  return backgroundElem;
}


//
// QgsTextShadowSettings
//

QgsTextShadowSettings::QgsTextShadowSettings()
{
  d = new QgsTextShadowSettingsPrivate();
}

QgsTextShadowSettings::QgsTextShadowSettings( const QgsTextShadowSettings &other )
    : d( other.d )
{

}

QgsTextShadowSettings &QgsTextShadowSettings::operator=( const QgsTextShadowSettings & other )
{
  d = other.d;
  return *this;
}

QgsTextShadowSettings::~QgsTextShadowSettings()
{

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

void QgsTextShadowSettings::readFromLayer( QgsVectorLayer* layer )
{
  d->enabled = layer->customProperty( "labeling/shadowDraw", QVariant( false ) ).toBool();
  d->shadowUnder = static_cast< ShadowPlacement >( layer->customProperty( "labeling/shadowUnder", QVariant( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = layer->customProperty( "labeling/shadowOffsetAngle", QVariant( 135 ) ).toInt();
  d->offsetDist = layer->customProperty( "labeling/shadowOffsetDist", QVariant( 1.0 ) ).toDouble();

  if ( layer->customProperty( "labeling/shadowOffsetUnit" ).toString().isEmpty() )
  {
    d->offsetUnits = layer->customProperty( "labeling/shadowOffsetUnits", 0 ).toUInt() == 0 ?
                     QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shadowOffsetUnit" ).toString() );
  }
  if ( layer->customProperty( "labeling/shadowOffsetMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->offsetMapUnitScale.minScale = layer->customProperty( "labeling/shadowOffsetMapUnitMinScale", 0.0 ).toDouble();
    d->offsetMapUnitScale.maxScale = layer->customProperty( "labeling/shadowOffsetMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shadowOffsetMapUnitScale" ).toString() );
  }
  d->offsetGlobal = layer->customProperty( "labeling/shadowOffsetGlobal", QVariant( true ) ).toBool();
  d->radius = layer->customProperty( "labeling/shadowRadius", QVariant( 1.5 ) ).toDouble();

  if ( layer->customProperty( "labeling/shadowRadiusUnit" ).toString().isEmpty() )
  {
    d->radiusUnits = layer->customProperty( "labeling/shadowRadiusUnits", 0 ).toUInt() == 0 ?
                     QgsUnitTypes::RenderMillimeters : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/shadowRadiusUnit" ).toString() );
  }
  if ( layer->customProperty( "labeling/shadowRadiusMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->radiusMapUnitScale.minScale = layer->customProperty( "labeling/shadowRadiusMapUnitMinScale", 0.0 ).toDouble();
    d->radiusMapUnitScale.maxScale = layer->customProperty( "labeling/shadowRadiusMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/shadowRadiusMapUnitScale" ).toString() );
  }
  d->radiusAlphaOnly = layer->customProperty( "labeling/shadowRadiusAlphaOnly", QVariant( false ) ).toBool();

  if ( layer->customProperty( "labeling/shadowOpacity" ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( "labeling/shadowTransparency" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( "labeling/shadowOpacity" ).toDouble() );
  }
  d->scale = layer->customProperty( "labeling/shadowScale", QVariant( 100 ) ).toInt();
  d->color = _readColor( layer, "labeling/shadowColor", Qt::black, false );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( "labeling/shadowBlendMode", QVariant( QgsPainting::BlendMultiply ) ).toUInt() ) );
}

void QgsTextShadowSettings::writeToLayer( QgsVectorLayer* layer ) const
{
  layer->setCustomProperty( "labeling/shadowDraw", d->enabled );
  layer->setCustomProperty( "labeling/shadowUnder", static_cast< unsigned int >( d->shadowUnder ) );
  layer->setCustomProperty( "labeling/shadowOffsetAngle", d->offsetAngle );
  layer->setCustomProperty( "labeling/shadowOffsetDist", d->offsetDist );
  layer->setCustomProperty( "labeling/shadowOffsetUnit", QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  layer->setCustomProperty( "labeling/shadowOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  layer->setCustomProperty( "labeling/shadowOffsetGlobal", d->offsetGlobal );
  layer->setCustomProperty( "labeling/shadowRadius", d->radius );
  layer->setCustomProperty( "labeling/shadowRadiusUnit", QgsUnitTypes::encodeUnit( d->radiusUnits ) );
  layer->setCustomProperty( "labeling/shadowRadiusMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->radiusMapUnitScale ) );
  layer->setCustomProperty( "labeling/shadowRadiusAlphaOnly", d->radiusAlphaOnly );
  layer->setCustomProperty( "labeling/shadowOpacity", d->opacity );
  layer->setCustomProperty( "labeling/shadowScale", d->scale );
  _writeColor( layer, "labeling/shadowColor", d->color, false );
  layer->setCustomProperty( "labeling/shadowBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
}

void QgsTextShadowSettings::readXml( const QDomElement& elem )
{
  QDomElement shadowElem = elem.firstChildElement( "shadow" );
  d->enabled = shadowElem.attribute( "shadowDraw", "0" ).toInt();
  d->shadowUnder = static_cast< ShadowPlacement >( shadowElem.attribute( "shadowUnder", QString::number( ShadowLowest ) ).toUInt() );//ShadowLowest ;
  d->offsetAngle = shadowElem.attribute( "shadowOffsetAngle", "135" ).toInt();
  d->offsetDist = shadowElem.attribute( "shadowOffsetDist", "1" ).toDouble();

  if ( !shadowElem.hasAttribute( "shadowOffsetUnit" ) )
  {
    d->offsetUnits = shadowElem.attribute( "shadowOffsetUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                     : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( "shadowOffsetUnit" ) );
  }

  if ( !shadowElem.hasAttribute( "shadowOffsetMapUnitScale" ) )
  {
    //fallback to older property
    d->offsetMapUnitScale.minScale = shadowElem.attribute( "shadowOffsetMapUnitMinScale", "0" ).toDouble();
    d->offsetMapUnitScale.maxScale = shadowElem.attribute( "shadowOffsetMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->offsetMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( "shadowOffsetMapUnitScale" ) );
  }
  d->offsetGlobal = shadowElem.attribute( "shadowOffsetGlobal", "1" ).toInt();
  d->radius = shadowElem.attribute( "shadowRadius", "1.5" ).toDouble();

  if ( !shadowElem.hasAttribute( "shadowRadiusUnit" ) )
  {
    d->radiusUnits = shadowElem.attribute( "shadowRadiusUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderMillimeters
                     : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( "shadowRadiusUnit" ) );
  }
  if ( !shadowElem.hasAttribute( "shadowRadiusMapUnitScale" ) )
  {
    //fallback to older property
    d->radiusMapUnitScale.minScale = shadowElem.attribute( "shadowRadiusMapUnitMinScale", "0" ).toDouble();
    d->radiusMapUnitScale.maxScale = shadowElem.attribute( "shadowRadiusMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->radiusMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( shadowElem.attribute( "shadowRadiusMapUnitScale" ) );
  }
  d->radiusAlphaOnly = shadowElem.attribute( "shadowRadiusAlphaOnly", "0" ).toInt();

  if ( !shadowElem.hasAttribute( "shadowOpacity" ) )
  {
    d->opacity = ( 1 - shadowElem.attribute( "shadowTransparency" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( shadowElem.attribute( "shadowOpacity" ).toDouble() );
  }
  d->scale = shadowElem.attribute( "shadowScale", "100" ).toInt();
  d->color = QgsSymbolLayerUtils::decodeColor( shadowElem.attribute( "shadowColor", QgsSymbolLayerUtils::encodeColor( Qt::black ) ) );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( shadowElem.attribute( "shadowBlendMode", QString::number( QgsPainting::BlendMultiply ) ).toUInt() ) );
}

QDomElement QgsTextShadowSettings::writeXml( QDomDocument& doc ) const
{
  QDomElement shadowElem = doc.createElement( "shadow" );
  shadowElem.setAttribute( "shadowDraw", d->enabled );
  shadowElem.setAttribute( "shadowUnder", static_cast< unsigned int >( d->shadowUnder ) );
  shadowElem.setAttribute( "shadowOffsetAngle", d->offsetAngle );
  shadowElem.setAttribute( "shadowOffsetDist", d->offsetDist );
  shadowElem.setAttribute( "shadowOffsetUnit", QgsUnitTypes::encodeUnit( d->offsetUnits ) );
  shadowElem.setAttribute( "shadowOffsetMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->offsetMapUnitScale ) );
  shadowElem.setAttribute( "shadowOffsetGlobal", d->offsetGlobal );
  shadowElem.setAttribute( "shadowRadius", d->radius );
  shadowElem.setAttribute( "shadowRadiusUnit", QgsUnitTypes::encodeUnit( d->radiusUnits ) );
  shadowElem.setAttribute( "shadowRadiusMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->radiusMapUnitScale ) );
  shadowElem.setAttribute( "shadowRadiusAlphaOnly", d->radiusAlphaOnly );
  shadowElem.setAttribute( "shadowOpacity", d->opacity );
  shadowElem.setAttribute( "shadowScale", d->scale );
  shadowElem.setAttribute( "shadowColor", QgsSymbolLayerUtils::encodeColor( d->color ) );
  shadowElem.setAttribute( "shadowBlendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
  return shadowElem;
}

//
// QgsTextFormat
//

QgsTextFormat::QgsTextFormat()
    : mTextFontFound( true )
{
  d = new QgsTextSettingsPrivate();
}

QgsTextFormat::QgsTextFormat( const QgsTextFormat &other )
    : mBufferSettings( other.mBufferSettings )
    , mBackgroundSettings( other.mBackgroundSettings )
    , mShadowSettings( other.mShadowSettings )
    , mTextFontFamily( other.mTextFontFamily )
    , mTextFontFound( other.mTextFontFound )
    , d( other.d )
{

}

QgsTextFormat &QgsTextFormat::operator=( const QgsTextFormat & other )
{
  d = other.d;
  mBufferSettings = other.mBufferSettings;
  mBackgroundSettings = other.mBackgroundSettings;
  mShadowSettings = other.mShadowSettings;
  mTextFontFamily = other.mTextFontFamily;
  mTextFontFound = other.mTextFontFound;
  return *this;
}

QgsTextFormat::~QgsTextFormat()
{

}

QFont QgsTextFormat::font() const
{
  return d->textFont;
}

QFont QgsTextFormat::scaledFont( const QgsRenderContext& context ) const
{
  QFont font = d->textFont;
  int fontPixelSize = QgsTextRenderer::sizeToPixel( d->fontSize, context, d->fontSizeUnits,
                      true, d->fontSizeMapUnitScale );
  font.setPixelSize( fontPixelSize );
  return font;
}

void QgsTextFormat::setFont( const QFont &font )
{
  d->textFont = font;
}

QString QgsTextFormat::namedStyle() const
{
  if ( !d->textNamedStyle.isEmpty() )
    return d->textNamedStyle;

  QFontDatabase db;
  return db.styleString( d->textFont );
}

void QgsTextFormat::setNamedStyle( const QString &style )
{
  QgsFontUtils::updateFontViaStyle( d->textFont, style );
  d->textNamedStyle = style;
}

QgsUnitTypes::RenderUnit QgsTextFormat::sizeUnit() const
{
  return d->fontSizeUnits;
}

void QgsTextFormat::setSizeUnit( QgsUnitTypes::RenderUnit unit )
{
  d->fontSizeUnits = unit;
}

QgsMapUnitScale QgsTextFormat::sizeMapUnitScale() const
{
  return d->fontSizeMapUnitScale;
}

void QgsTextFormat::setSizeMapUnitScale( const QgsMapUnitScale &scale )
{
  d->fontSizeMapUnitScale = scale;
}

double QgsTextFormat::size() const
{
  return d->fontSize;
}

void QgsTextFormat::setSize( double size )
{
  d->fontSize = size;
}

QColor QgsTextFormat::color() const
{
  return d->textColor;
}

void QgsTextFormat::setColor( const QColor &color )
{
  d->textColor = color;
}

double QgsTextFormat::opacity() const
{
  return d->opacity;
}

void QgsTextFormat::setOpacity( double opacity )
{
  d->opacity = opacity;
}

QPainter::CompositionMode QgsTextFormat::blendMode() const
{
  return d->blendMode;
}

void QgsTextFormat::setBlendMode( QPainter::CompositionMode mode )
{
  d->blendMode = mode;
}

double QgsTextFormat::lineHeight() const
{
  return d->multilineHeight;
}

void QgsTextFormat::setLineHeight( double height )
{
  d->multilineHeight = height;
}

void QgsTextFormat::readFromLayer( QgsVectorLayer* layer )
{
  QFont appFont = QApplication::font();
  mTextFontFamily = layer->customProperty( "labeling/fontFamily", QVariant( appFont.family() ) ).toString();
  QString fontFamily = mTextFontFamily;
  if ( mTextFontFamily != appFont.family() && !QgsFontUtils::fontFamilyMatchOnSystem( mTextFontFamily ) )
  {
    // trigger to notify about font family substitution
    mTextFontFound = false;

    // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
    // currently only defaults to matching algorithm for resolving [foundry], if a font of similar family is found (default for QFont)

    // for now, do not use matching algorithm for substitution if family not found, substitute default instead
    fontFamily = appFont.family();
  }
  else
  {
    mTextFontFound = true;
  }

  if ( !layer->customProperty( "labeling/fontSize" ).isValid() )
  {
    d->fontSize = appFont.pointSizeF();
  }
  else
  {
    d->fontSize = layer->customProperty( "labeling/fontSize" ).toDouble();
  }

  if ( layer->customProperty( "labeling/fontSizeUnit" ).toString().isEmpty() )
  {
    d->fontSizeUnits = layer->customProperty( "labeling/fontSizeInMapUnits", 0 ).toUInt() == 0 ?
                       QgsUnitTypes::RenderPoints : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    bool ok = false;
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( "labeling/fontSizeUnit" ).toString(), &ok );
    if ( !ok )
      d->fontSizeUnits = QgsUnitTypes::RenderPoints;
  }
  if ( layer->customProperty( "labeling/fontSizeMapUnitScale" ).toString().isEmpty() )
  {
    //fallback to older property
    d->fontSizeMapUnitScale.minScale = layer->customProperty( "labeling/fontSizeMapUnitMinScale", 0.0 ).toDouble();
    d->fontSizeMapUnitScale.maxScale = layer->customProperty( "labeling/fontSizeMapUnitMaxScale", 0.0 ).toDouble();
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( "labeling/fontSizeMapUnitScale" ).toString() );
  }
  int fontWeight = layer->customProperty( "labeling/fontWeight" ).toInt();
  bool fontItalic = layer->customProperty( "labeling/fontItalic" ).toBool();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( layer->customProperty( "labeling/namedStyle", QVariant( "" ) ).toString() );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->textFont.setCapitalization( static_cast< QFont::Capitalization >( layer->customProperty( "labeling/fontCapitals", QVariant( 0 ) ).toUInt() ) );
  d->textFont.setUnderline( layer->customProperty( "labeling/fontUnderline" ).toBool() );
  d->textFont.setStrikeOut( layer->customProperty( "labeling/fontStrikeout" ).toBool() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( "labeling/fontLetterSpacing", QVariant( 0.0 ) ).toDouble() );
  d->textFont.setWordSpacing( layer->customProperty( "labeling/fontWordSpacing", QVariant( 0.0 ) ).toDouble() );
  d->textColor = _readColor( layer, "labeling/textColor", Qt::black, false );
  if ( layer->customProperty( "labeling/textOpacity" ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( "labeling/textTransp" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( "labeling/textOpacity" ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( "labeling/blendMode", QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->multilineHeight = layer->customProperty( "labeling/multilineHeight", QVariant( 1.0 ) ).toDouble();

  mBufferSettings.readFromLayer( layer );
  mShadowSettings.readFromLayer( layer );
  mBackgroundSettings.readFromLayer( layer );
}

void QgsTextFormat::writeToLayer( QgsVectorLayer* layer ) const
{
  layer->setCustomProperty( "labeling/fontFamily", d->textFont.family() );
  layer->setCustomProperty( "labeling/namedStyle", QgsFontUtils::untranslateNamedStyle( d->textNamedStyle ) );
  layer->setCustomProperty( "labeling/fontSize", d->fontSize );
  layer->setCustomProperty( "labeling/fontSizeUnit", QgsUnitTypes::encodeUnit( d->fontSizeUnits ) );
  layer->setCustomProperty( "labeling/fontSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->fontSizeMapUnitScale ) );
  layer->setCustomProperty( "labeling/fontWeight", d->textFont.weight() );
  layer->setCustomProperty( "labeling/fontItalic", d->textFont.italic() );
  layer->setCustomProperty( "labeling/fontStrikeout", d->textFont.strikeOut() );
  layer->setCustomProperty( "labeling/fontUnderline", d->textFont.underline() );
  _writeColor( layer, "labeling/textColor", d->textColor );
  layer->setCustomProperty( "labeling/textOpacity", d->opacity );
  layer->setCustomProperty( "labeling/fontCapitals", static_cast< unsigned int >( d->textFont.capitalization() ) );
  layer->setCustomProperty( "labeling/fontLetterSpacing", d->textFont.letterSpacing() );
  layer->setCustomProperty( "labeling/fontWordSpacing", d->textFont.wordSpacing() );
  layer->setCustomProperty( "labeling/textOpacity", d->opacity );
  layer->setCustomProperty( "labeling/blendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
  layer->setCustomProperty( "labeling/multilineHeight", d->multilineHeight );

  mBufferSettings.writeToLayer( layer );
  mShadowSettings.writeToLayer( layer );
  mBackgroundSettings.writeToLayer( layer );
}

void QgsTextFormat::readXml( const QDomElement& elem )
{
  QDomElement textStyleElem = elem.firstChildElement( "text-style" );
  QFont appFont = QApplication::font();
  mTextFontFamily = textStyleElem.attribute( "fontFamily", appFont.family() );
  QString fontFamily = mTextFontFamily;
  if ( mTextFontFamily != appFont.family() && !QgsFontUtils::fontFamilyMatchOnSystem( mTextFontFamily ) )
  {
    // trigger to notify user about font family substitution (signal emitted in QgsVectorLayer::prepareLabelingAndDiagrams)
    mTextFontFound = false;

    // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
    // currently only defaults to matching algorithm for resolving [foundry], if a font of similar family is found (default for QFont)

    // for now, do not use matching algorithm for substitution if family not found, substitute default instead
    fontFamily = appFont.family();
  }
  else
  {
    mTextFontFound = true;
  }

  if ( textStyleElem.hasAttribute( "fontSize" ) )
  {
    d->fontSize = textStyleElem.attribute( "fontSize" ).toDouble();
  }
  else
  {
    d->fontSize = appFont.pointSizeF();
  }

  if ( !textStyleElem.hasAttribute( "fontSizeUnit" ) )
  {
    d->fontSizeUnits = textStyleElem.attribute( "fontSizeInMapUnits" ).toUInt() == 0 ? QgsUnitTypes::RenderPoints
                       : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( textStyleElem.attribute( "fontSizeUnit" ) );
  }

  if ( !textStyleElem.hasAttribute( "fontSizeMapUnitScale" ) )
  {
    //fallback to older property
    d->fontSizeMapUnitScale.minScale = textStyleElem.attribute( "fontSizeMapUnitMinScale", "0" ).toDouble();
    d->fontSizeMapUnitScale.maxScale = textStyleElem.attribute( "fontSizeMapUnitMaxScale", "0" ).toDouble();
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textStyleElem.attribute( "fontSizeMapUnitScale" ) );
  }
  int fontWeight = textStyleElem.attribute( "fontWeight" ).toInt();
  bool fontItalic = textStyleElem.attribute( "fontItalic" ).toInt();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textFont.setPointSizeF( d->fontSize ); //double precision needed because of map units
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( textStyleElem.attribute( "namedStyle" ) );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->textFont.setCapitalization( static_cast< QFont::Capitalization >( textStyleElem.attribute( "fontCapitals", "0" ).toUInt() ) );
  d->textFont.setUnderline( textStyleElem.attribute( "fontUnderline" ).toInt() );
  d->textFont.setStrikeOut( textStyleElem.attribute( "fontStrikeout" ).toInt() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, textStyleElem.attribute( "fontLetterSpacing", "0" ).toDouble() );
  d->textFont.setWordSpacing( textStyleElem.attribute( "fontWordSpacing", "0" ).toDouble() );
  d->textColor = QgsSymbolLayerUtils::decodeColor( textStyleElem.attribute( "textColor", QgsSymbolLayerUtils::encodeColor( Qt::black ) ) );
  if ( !textStyleElem.hasAttribute( "textOpacity" ) )
  {
    d->opacity = ( 1 - textStyleElem.attribute( "textTransp" ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( textStyleElem.attribute( "textOpacity" ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( textStyleElem.attribute( "blendMode", QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );

  if ( !textStyleElem.hasAttribute( "multilineHeight" ) )
  {
    QDomElement textFormatElem = elem.firstChildElement( "text-format" );
    d->multilineHeight = textFormatElem.attribute( "multilineHeight", "1" ).toDouble();
  }
  else
  {
    d->multilineHeight = textStyleElem.attribute( "multilineHeight", "1" ).toDouble();
  }

  if ( textStyleElem.firstChildElement( "text-buffer" ).isNull() )
  {
    mBufferSettings.readXml( elem );
  }
  else
  {
    mBufferSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( "shadow" ).isNull() )
  {
    mShadowSettings.readXml( elem );
  }
  else
  {
    mShadowSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( "background" ).isNull() )
  {
    mBackgroundSettings.readXml( elem );
  }
  else
  {
    mBackgroundSettings.readXml( textStyleElem );
  }
}

QDomElement QgsTextFormat::writeXml( QDomDocument& doc ) const
{
  // text style
  QDomElement textStyleElem = doc.createElement( "text-style" );
  textStyleElem.setAttribute( "fontFamily", d->textFont.family() );
  textStyleElem.setAttribute( "namedStyle", QgsFontUtils::untranslateNamedStyle( d->textNamedStyle ) );
  textStyleElem.setAttribute( "fontSize", d->fontSize );
  textStyleElem.setAttribute( "fontSizeUnit", QgsUnitTypes::encodeUnit( d->fontSizeUnits ) );
  textStyleElem.setAttribute( "fontSizeMapUnitScale", QgsSymbolLayerUtils::encodeMapUnitScale( d->fontSizeMapUnitScale ) );
  textStyleElem.setAttribute( "fontWeight", d->textFont.weight() );
  textStyleElem.setAttribute( "fontItalic", d->textFont.italic() );
  textStyleElem.setAttribute( "fontStrikeout", d->textFont.strikeOut() );
  textStyleElem.setAttribute( "fontUnderline", d->textFont.underline() );
  textStyleElem.setAttribute( "textColor", QgsSymbolLayerUtils::encodeColor( d->textColor ) );
  textStyleElem.setAttribute( "fontCapitals", static_cast< unsigned int >( d->textFont.capitalization() ) );
  textStyleElem.setAttribute( "fontLetterSpacing", d->textFont.letterSpacing() );
  textStyleElem.setAttribute( "fontWordSpacing", d->textFont.wordSpacing() );
  textStyleElem.setAttribute( "textOpacity", d->opacity );
  textStyleElem.setAttribute( "blendMode", QgsPainting::getBlendModeEnum( d->blendMode ) );
  textStyleElem.setAttribute( "multilineHeight", d->multilineHeight );

  textStyleElem.appendChild( mBufferSettings.writeXml( doc ) );
  textStyleElem.appendChild( mBackgroundSettings.writeXml( doc ) );
  textStyleElem.appendChild( mShadowSettings.writeXml( doc ) );
  return textStyleElem;
}

bool QgsTextFormat::containsAdvancedEffects() const
{
  if ( d->blendMode != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mBufferSettings.enabled() && mBufferSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mBackgroundSettings.enabled() && mBackgroundSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  if ( mShadowSettings.enabled() && mShadowSettings.blendMode() != QPainter::CompositionMode_SourceOver )
    return true;

  return false;
}


int QgsTextRenderer::sizeToPixel( double size, const QgsRenderContext& c, QgsUnitTypes::RenderUnit unit, bool rasterfactor, const QgsMapUnitScale& mapUnitScale )
{
  return static_cast< int >( scaleToPixelContext( size, c, unit, rasterfactor, mapUnitScale ) + 0.5 );
}

double QgsTextRenderer::scaleToPixelContext( double size, const QgsRenderContext& c, QgsUnitTypes::RenderUnit unit, bool rasterfactor, const QgsMapUnitScale& mapUnitScale )
{
  // if render context is that of device (i.e. not a scaled map), just return size
  double mapUnitsPerPixel = mapUnitScale.computeMapUnitsPerPixel( c );

  switch ( unit )
  {
    case QgsUnitTypes::RenderMapUnits:
      if ( mapUnitsPerPixel > 0.0 )
      {
        size = size / mapUnitsPerPixel * ( rasterfactor ? c.rasterScaleFactor() : 1 );
      }
      break;

    case QgsUnitTypes::RenderPixels:
      //already in pixels
      break;

    case QgsUnitTypes::RenderMillimeters:
      size *= c.scaleFactor() * ( rasterfactor ? c.rasterScaleFactor() : 1 );
      break;

    case QgsUnitTypes::RenderPoints:
      size *= 0.352778 * c.scaleFactor() * ( rasterfactor ? c.rasterScaleFactor() : 1 );
      break;

    case QgsUnitTypes::RenderPercentage:
    case QgsUnitTypes::RenderUnknownUnit:
      // no sensible choice
      break;
  }
  return size;
}
