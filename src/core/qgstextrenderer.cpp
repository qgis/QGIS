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
#include "qgis.h"
#include "qgstextrenderer_p.h"
#include "qgsfontutils.h"
#include "qgspathresolver.h"
#include "qgsreadwritecontext.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerutils.h"
#include "qgspainting.h"
#include "qgsmarkersymbollayer.h"
#include "qgspainteffectregistry.h"
#include "qgspallabeling.h"
#include "qgspainterswapper.h"
#include <QFontDatabase>
#include <QDesktopWidget>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

QgsUnitTypes::RenderUnit convertFromOldLabelUnit( int val )
{
  if ( val == 0 )
    return QgsUnitTypes::RenderPoints;
  else if ( val == 1 )
    return QgsUnitTypes::RenderMillimeters;
  else if ( val == 2 )
    return QgsUnitTypes::RenderMapUnits;
  else if ( val == 3 )
    return QgsUnitTypes::RenderPercentage;
  else
    return QgsUnitTypes::RenderMillimeters;
}

static void _fixQPictureDPI( QPainter *p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale( static_cast< double >( qt_defaultDpiX() ) / p->device()->logicalDpiX(),
            static_cast< double >( qt_defaultDpiY() ) / p->device()->logicalDpiY() );
}

static QColor _readColor( QgsVectorLayer *layer, const QString &property, const QColor &defaultColor = Qt::black, bool withAlpha = true )
{
  int r = layer->customProperty( property + 'R', QVariant( defaultColor.red() ) ).toInt();
  int g = layer->customProperty( property + 'G', QVariant( defaultColor.green() ) ).toInt();
  int b = layer->customProperty( property + 'B', QVariant( defaultColor.blue() ) ).toInt();
  int a = withAlpha ? layer->customProperty( property + 'A', QVariant( defaultColor.alpha() ) ).toInt() : 255;
  return QColor( r, g, b, a );
}

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

QgsPaintEffect *QgsTextBufferSettings::paintEffect() const
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
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->sizeUnit = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    d->opacity = properties.value( QgsPalLayerSettings::BufferOpacity, context.expressionContext(), d->opacity * 100 ).toDouble() / 100.0;
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferColor ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->color ) );
    d->color = properties.valueAsColor( QgsPalLayerSettings::BufferColor, context.expressionContext(), d->color );
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::BufferBlendMode, context.expressionContext() );
    QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  if ( properties.isActive( QgsPalLayerSettings::BufferJoinStyle ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::BufferJoinStyle, context.expressionContext() );
    QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
    }
  }
}

void QgsTextBufferSettings::readFromLayer( QgsVectorLayer *layer )
{
  // text buffer
  double bufSize = layer->customProperty( QStringLiteral( "labeling/bufferSize" ), QVariant( 0.0 ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  QVariant drawBuffer = layer->customProperty( QStringLiteral( "labeling/bufferDraw" ), QVariant() );
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
    bool bufferSizeInMapUnits = layer->customProperty( QStringLiteral( "labeling/bufferSizeInMapUnits" ) ).toBool();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/bufferSizeUnits" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitMaxScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/bufferSizeMapUnitScale" ) ).toString() );
  }
  d->color = _readColor( layer, QStringLiteral( "labeling/bufferColor" ), Qt::white, false );
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
    QDomElement effectElem = doc.firstChildElement( QStringLiteral( "effect" ) ).firstChildElement( QStringLiteral( "effect" ) );
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  }
  else
    setPaintEffect( nullptr );
}

void QgsTextBufferSettings::readXml( const QDomElement &elem )
{
  QDomElement textBufferElem = elem.firstChildElement( QStringLiteral( "text-buffer" ) );
  double bufSize = textBufferElem.attribute( QStringLiteral( "bufferSize" ), QStringLiteral( "0" ) ).toDouble();

  // fix for buffer being keyed off of just its size in the past (<2.0)
  QVariant drawBuffer = textBufferElem.attribute( QStringLiteral( "bufferDraw" ) );
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
    bool bufferSizeInMapUnits = textBufferElem.attribute( QStringLiteral( "bufferSizeInMapUnits" ) ).toInt();
    d->sizeUnit = bufferSizeInMapUnits ? QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderMillimeters;
  }
  else
  {
    d->sizeUnit = QgsUnitTypes::decodeRenderUnit( textBufferElem.attribute( QStringLiteral( "bufferSizeUnits" ) ) );
  }

  if ( !textBufferElem.hasAttribute( QStringLiteral( "bufferSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = textBufferElem.attribute( QStringLiteral( "bufferSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = textBufferElem.attribute( QStringLiteral( "bufferSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
  QDomElement effectElem = textBufferElem.firstChildElement( QStringLiteral( "effect" ) );
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


//
// QgsTextBackgroundSettings
//

QgsTextBackgroundSettings::QgsTextBackgroundSettings()
{
  d = new QgsTextBackgroundSettingsPrivate();
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
}

QColor QgsTextBackgroundSettings::strokeColor() const
{
  return d->strokeColor;
}

void QgsTextBackgroundSettings::setStrokeColor( const QColor &color )
{
  d->strokeColor = color;
}

double QgsTextBackgroundSettings::strokeWidth() const
{
  return d->strokeWidth;
}

void QgsTextBackgroundSettings::setStrokeWidth( double width )
{
  d->strokeWidth = width;
}

QgsUnitTypes::RenderUnit QgsTextBackgroundSettings::strokeWidthUnit() const
{
  return d->strokeWidthUnits;
}

void QgsTextBackgroundSettings::setStrokeWidthUnit( QgsUnitTypes::RenderUnit units )
{
  d->strokeWidthUnits = units;
}

QgsMapUnitScale QgsTextBackgroundSettings::strokeWidthMapUnitScale() const
{
  return d->strokeWidthMapUnitScale;
}

void QgsTextBackgroundSettings::setStrokeWidthMapUnitScale( const QgsMapUnitScale &scale )
{
  d->strokeWidthMapUnitScale = scale;
}

Qt::PenJoinStyle QgsTextBackgroundSettings::joinStyle() const
{
  return d->joinStyle;
}

void QgsTextBackgroundSettings::setJoinStyle( Qt::PenJoinStyle style )
{
  d->joinStyle = style;
}

QgsPaintEffect *QgsTextBackgroundSettings::paintEffect() const
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
    d->sizeUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeSizeUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeSizeUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeSizeMapUnitMaxScale" ), 0.0 ).toDouble();
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
    d->offsetUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeOffsetUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeOffsetUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitMinScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
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
    d->radiiUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeRadiiUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeRadiiUnit" ) ).toString() );
  }

  if ( layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitMinScale" ), 0.0 ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitMaxScale" ), 0.0 ).toDouble();
    d->radiiMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->radiiMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/shapeRadiiMapUnitScale" ) ).toString() );
  }
  d->fillColor = _readColor( layer, QStringLiteral( "labeling/shapeFillColor" ), Qt::white, true );
  d->strokeColor = _readColor( layer, QStringLiteral( "labeling/shapeBorderColor" ), Qt::darkGray, true );
  d->strokeWidth = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidth" ), QVariant( .0 ) ).toDouble();
  if ( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnit" ) ).toString().isEmpty() )
  {
    d->strokeWidthUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitMinScale" ), 0.0 ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shapeBorderWidthMapUnitMaxScale" ), 0.0 ).toDouble();
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
    QDomElement effectElem = doc.firstChildElement( QStringLiteral( "effect" ) ).firstChildElement( QStringLiteral( "effect" ) );
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  }
  else
    setPaintEffect( nullptr );
}

void QgsTextBackgroundSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement backgroundElem = elem.firstChildElement( QStringLiteral( "background" ) );
  d->enabled = backgroundElem.attribute( QStringLiteral( "shapeDraw" ), QStringLiteral( "0" ) ).toInt();
  d->type = static_cast< ShapeType >( backgroundElem.attribute( QStringLiteral( "shapeType" ), QString::number( ShapeRectangle ) ).toUInt() );
  d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( backgroundElem.attribute( QStringLiteral( "shapeSVGFile" ) ), context.pathResolver() );
  d->sizeType = static_cast< SizeType >( backgroundElem.attribute( QStringLiteral( "shapeSizeType" ), QString::number( SizeBuffer ) ).toUInt() );
  d->size = QSizeF( backgroundElem.attribute( QStringLiteral( "shapeSizeX" ), QStringLiteral( "0" ) ).toDouble(),
                    backgroundElem.attribute( QStringLiteral( "shapeSizeY" ), QStringLiteral( "0" ) ).toDouble() );

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeSizeUnit" ) ) )
  {
    d->sizeUnits = convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeSizeUnits" ) ).toUInt() );
  }
  else
  {
    d->sizeUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeSizeUnit" ) ) );
  }

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = backgroundElem.attribute( QStringLiteral( "shapeSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->sizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = backgroundElem.attribute( QStringLiteral( "shapeSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
    d->offsetUnits = convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeOffsetUnits" ) ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeOffsetUnit" ) ) );
  }

  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeOffsetMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = backgroundElem.attribute( QStringLiteral( "shapeOffsetMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = backgroundElem.attribute( QStringLiteral( "shapeOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
    d->radiiUnits = convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeRadiiUnits" ) ).toUInt() );
  }
  else
  {
    d->radiiUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeRadiiUnit" ) ) );
  }
  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeRadiiMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = backgroundElem.attribute( QStringLiteral( "shapeRadiiMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiiMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = backgroundElem.attribute( QStringLiteral( "shapeRadiiMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
    d->strokeWidthUnits = convertFromOldLabelUnit( backgroundElem.attribute( QStringLiteral( "shapeBorderWidthUnits" ) ).toUInt() );
  }
  else
  {
    d->strokeWidthUnits = QgsUnitTypes::decodeRenderUnit( backgroundElem.attribute( QStringLiteral( "shapeBorderWidthUnit" ) ) );
  }
  if ( !backgroundElem.hasAttribute( QStringLiteral( "shapeBorderWidthMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = backgroundElem.attribute( QStringLiteral( "shapeBorderWidthMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->strokeWidthMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = backgroundElem.attribute( QStringLiteral( "shapeBorderWidthMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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

  QDomElement effectElem = backgroundElem.firstChildElement( QStringLiteral( "effect" ) );
  if ( !effectElem.isNull() )
    setPaintEffect( QgsApplication::paintEffectRegistry()->createEffect( effectElem ) );
  else
    setPaintEffect( nullptr );

  const QDomElement symbolElem = backgroundElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
    setMarkerSymbol( QgsSymbolLayerUtils::loadSymbol< QgsMarkerSymbol >( symbolElem, context ) );
  else
    setMarkerSymbol( nullptr );
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

  return backgroundElem;
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
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->sizeUnits = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeKind, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    const QString skind = exprVal.toString().trimmed();
    if ( !skind.isEmpty() )
    {
      d->type = QgsTextRendererUtils::decodeShapeType( skind );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeSizeType, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString stype = exprVal.toString().trimmed();
    if ( !stype.isEmpty() )
    {
      d->sizeType = QgsTextRendererUtils::decodeBackgroundSizeType( stype );
    }
  }

  // data defined shape SVG path?
  context.expressionContext().setOriginalValueVariable( d->svgFile );
  exprVal = properties.value( QgsPalLayerSettings::ShapeSVGFile, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString svgfile = exprVal.toString().trimmed();
    d->svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( svgfile, context.pathResolver() );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeRotation ) )
  {
    context.expressionContext().setOriginalValueVariable( d->rotation );
    d->rotation = properties.valueAsDouble( QgsPalLayerSettings::ShapeRotation, context.expressionContext(), d->rotation );
  }
  exprVal = properties.value( QgsPalLayerSettings::ShapeRotationType, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString rotstr = exprVal.toString().trimmed();
    if ( !rotstr.isEmpty() )
    {
      d->rotationType = QgsTextRendererUtils::decodeBackgroundRotationType( rotstr );
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeOffset, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( exprVal, &ok );
    if ( ok )
    {
      d->offset = res;
    }
  }
  exprVal = properties.value( QgsPalLayerSettings::ShapeOffsetUnits, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->offsetUnits = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeRadii, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    bool ok = false;
    const QSizeF res = QgsSymbolLayerUtils::toSize( exprVal, &ok );
    if ( ok )
    {
      d->radii = res;
    }
  }

  exprVal = properties.value( QgsPalLayerSettings::ShapeRadiiUnits, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiiUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    d->opacity = properties.value( QgsPalLayerSettings::ShapeOpacity, context.expressionContext(), d->opacity * 100 ).toDouble() / 100.0;
  }

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
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->strokeWidthUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeBlendMode ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::ShapeBlendMode, context.expressionContext() );
    QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  if ( properties.isActive( QgsPalLayerSettings::ShapeJoinStyle ) )
  {
    exprVal = properties.value( QgsPalLayerSettings::ShapeJoinStyle, context.expressionContext() );
    QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
    }
  }
}


//
// QgsTextShadowSettings
//

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
    d->offsetUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shadowOffsetUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shadowOffsetUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitMinScale" ), 0.0 ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shadowOffsetMapUnitMaxScale" ), 0.0 ).toDouble();
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
    d->radiusUnits = convertFromOldLabelUnit( layer->customProperty( QStringLiteral( "labeling/shadowRadiusUnits" ), 0 ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/shadowRadiusUnit" ) ).toString() );
  }
  if ( layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitMinScale" ), 0.0 ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/shadowRadiusMapUnitMaxScale" ), 0.0 ).toDouble();
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
  d->color = _readColor( layer, QStringLiteral( "labeling/shadowColor" ), Qt::black, false );
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/shadowBlendMode" ), QVariant( QgsPainting::BlendMultiply ) ).toUInt() ) );
}

void QgsTextShadowSettings::readXml( const QDomElement &elem )
{
  QDomElement shadowElem = elem.firstChildElement( QStringLiteral( "shadow" ) );
  d->enabled = shadowElem.attribute( QStringLiteral( "shadowDraw" ), QStringLiteral( "0" ) ).toInt();
  d->shadowUnder = static_cast< ShadowPlacement >( shadowElem.attribute( QStringLiteral( "shadowUnder" ), QString::number( ShadowLowest ) ).toUInt() );//ShadowLowest;
  d->offsetAngle = shadowElem.attribute( QStringLiteral( "shadowOffsetAngle" ), QStringLiteral( "135" ) ).toInt();
  d->offsetDist = shadowElem.attribute( QStringLiteral( "shadowOffsetDist" ), QStringLiteral( "1" ) ).toDouble();

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowOffsetUnit" ) ) )
  {
    d->offsetUnits = convertFromOldLabelUnit( shadowElem.attribute( QStringLiteral( "shadowOffsetUnits" ) ).toUInt() );
  }
  else
  {
    d->offsetUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( QStringLiteral( "shadowOffsetUnit" ) ) );
  }

  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowOffsetMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = shadowElem.attribute( QStringLiteral( "shadowOffsetMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->offsetMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = shadowElem.attribute( QStringLiteral( "shadowOffsetMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
    d->radiusUnits = convertFromOldLabelUnit( shadowElem.attribute( QStringLiteral( "shadowRadiusUnits" ) ).toUInt() );
  }
  else
  {
    d->radiusUnits = QgsUnitTypes::decodeRenderUnit( shadowElem.attribute( QStringLiteral( "shadowRadiusUnit" ) ) );
  }
  if ( !shadowElem.hasAttribute( QStringLiteral( "shadowRadiusMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = shadowElem.attribute( QStringLiteral( "shadowRadiusMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->radiusMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = shadowElem.attribute( QStringLiteral( "shadowRadiusMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
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
  if ( exprVal.isValid() )
  {
    QString str = exprVal.toString().trimmed();
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
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
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
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->radiusUnits = res;
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::ShadowOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    d->opacity = properties.value( QgsPalLayerSettings::ShadowOpacity, context.expressionContext(), d->opacity * 100 ).toDouble() / 100.0;
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
    QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }
}

//
// QgsTextMaskSettings
//

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

QgsUnitTypes::RenderUnit QgsTextMaskSettings::sizeUnit() const
{
  return d->sizeUnit;
}

void QgsTextMaskSettings::setSizeUnit( QgsUnitTypes::RenderUnit unit )
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
  if ( properties.isActive( QgsPalLayerSettings::MaskEnabled ) )
  {
    context.expressionContext().setOriginalValueVariable( d->enabled );
    d->enabled = properties.valueAsBool( QgsPalLayerSettings::MaskEnabled, context.expressionContext(), d->enabled );
  }

  if ( properties.isActive( QgsPalLayerSettings::MaskBufferSize ) )
  {
    context.expressionContext().setOriginalValueVariable( d->size );
    d->size = properties.valueAsDouble( QgsPalLayerSettings::MaskBufferSize, context.expressionContext(), d->size );
  }

  if ( properties.isActive( QgsPalLayerSettings::MaskBufferUnit ) )
  {
    QVariant exprVal = properties.value( QgsPalLayerSettings::MaskBufferUnit, context.expressionContext() );
    if ( exprVal.isValid() )
    {
      QString units = exprVal.toString();
      if ( !units.isEmpty() )
      {
        bool ok;
        QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
        if ( ok )
          d->sizeUnit = res;
      }
    }
  }

  if ( properties.isActive( QgsPalLayerSettings::MaskOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    d->opacity = properties.value( QgsPalLayerSettings::MaskOpacity, context.expressionContext(), d->opacity * 100 ).toDouble() / 100.0;
  }

  if ( properties.isActive( QgsPalLayerSettings::MaskJoinStyle ) )
  {
    QVariant exprVal = properties.value( QgsPalLayerSettings::MaskJoinStyle, context.expressionContext() );
    QString joinstr = exprVal.toString().trimmed();
    if ( !joinstr.isEmpty() )
    {
      d->joinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( joinstr );
    }
  }
}

void QgsTextMaskSettings::readXml( const QDomElement &elem )
{
  QDomElement textMaskElem = elem.firstChildElement( QStringLiteral( "text-mask" ) );
  d->enabled = textMaskElem.attribute( QStringLiteral( "maskEnabled" ), QStringLiteral( "0" ) ).toInt();
  d->type = static_cast<QgsTextMaskSettings::MaskType>( textMaskElem.attribute( QStringLiteral( "maskType" ), QStringLiteral( "0" ) ).toInt() );
  d->size = textMaskElem.attribute( QStringLiteral( "maskSize" ), QStringLiteral( "0" ) ).toDouble();
  d->sizeUnit = QgsUnitTypes::decodeRenderUnit( textMaskElem.attribute( QStringLiteral( "maskSizeUnits" ) ) );
  d->sizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textMaskElem.attribute( QStringLiteral( "maskSizeMapUnitScale" ) ) );
  d->joinStyle = static_cast< Qt::PenJoinStyle >( textMaskElem.attribute( QStringLiteral( "maskJoinStyle" ), QString::number( Qt::RoundJoin ) ).toUInt() );
  d->opacity = textMaskElem.attribute( QStringLiteral( "maskOpacity" ), QStringLiteral( "1.0" ) ).toDouble();
  QDomElement effectElem = textMaskElem.firstChildElement( QStringLiteral( "effect" ) );
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
  textMaskElem.setAttribute( QStringLiteral( "maskSizeUnits" ), QgsUnitTypes::encodeUnit( d->sizeUnit ) );
  textMaskElem.setAttribute( QStringLiteral( "maskSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->sizeMapUnitScale ) );
  textMaskElem.setAttribute( QStringLiteral( "maskJoinStyle" ), static_cast< unsigned int >( d->joinStyle ) );
  textMaskElem.setAttribute( QStringLiteral( "maskOpacity" ), d->opacity );
  if ( d->paintEffect && !QgsPaintEffectRegistry::isDefaultStack( d->paintEffect.get() ) )
    d->paintEffect->saveProperties( doc, textMaskElem );
  textMaskElem.setAttribute( QStringLiteral( "maskedSymbolLayers" ), symbolLayerReferenceListToString( d->maskedSymbolLayers ) );
  return textMaskElem;
}

QgsSymbolLayerReferenceList QgsTextMaskSettings::maskedSymbolLayers() const
{
  return d->maskedSymbolLayers;
}

void QgsTextMaskSettings::setMaskedSymbolLayers( QgsSymbolLayerReferenceList maskedSymbols )
{
  d->maskedSymbolLayers = maskedSymbols;
}

//
// QgsTextFormat
//

QgsTextFormat::QgsTextFormat()
{
  d = new QgsTextSettingsPrivate();
}

QgsTextFormat::QgsTextFormat( const QgsTextFormat &other ) //NOLINT
  : mBufferSettings( other.mBufferSettings )
  , mBackgroundSettings( other.mBackgroundSettings )
  , mShadowSettings( other.mShadowSettings )
  , mMaskSettings( other.mMaskSettings )
  , mTextFontFamily( other.mTextFontFamily )
  , mTextFontFound( other.mTextFontFound )
  , d( other.d )
{

}

QgsTextFormat &QgsTextFormat::operator=( const QgsTextFormat &other )  //NOLINT
{
  d = other.d;
  mBufferSettings = other.mBufferSettings;
  mBackgroundSettings = other.mBackgroundSettings;
  mShadowSettings = other.mShadowSettings;
  mMaskSettings = other.mMaskSettings;
  mTextFontFamily = other.mTextFontFamily;
  mTextFontFound = other.mTextFontFound;
  return *this;
}

QgsTextFormat::~QgsTextFormat() //NOLINT
{

}

QFont QgsTextFormat::font() const
{
  return d->textFont;
}

QFont QgsTextFormat::scaledFont( const QgsRenderContext &context ) const
{
  QFont font = d->textFont;
  int fontPixelSize = QgsTextRenderer::sizeToPixel( d->fontSize, context, d->fontSizeUnits,
                      d->fontSizeMapUnitScale );
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

QgsTextFormat::TextOrientation QgsTextFormat::orientation() const
{
  return d->orientation;
}

void QgsTextFormat::setOrientation( TextOrientation orientation )
{
  d->orientation = orientation;
}

QColor QgsTextFormat::previewBackgroundColor() const
{
  return d->previewBackgroundColor;
}

void QgsTextFormat::setPreviewBackgroundColor( const QColor &color )
{
  d->previewBackgroundColor = color;
}

void QgsTextFormat::readFromLayer( QgsVectorLayer *layer )
{
  QFont appFont = QApplication::font();
  mTextFontFamily = layer->customProperty( QStringLiteral( "labeling/fontFamily" ), QVariant( appFont.family() ) ).toString();
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

  if ( !layer->customProperty( QStringLiteral( "labeling/fontSize" ) ).isValid() )
  {
    d->fontSize = appFont.pointSizeF();
  }
  else
  {
    d->fontSize = layer->customProperty( QStringLiteral( "labeling/fontSize" ) ).toDouble();
  }

  if ( layer->customProperty( QStringLiteral( "labeling/fontSizeUnit" ) ).toString().isEmpty() )
  {
    d->fontSizeUnits = layer->customProperty( QStringLiteral( "labeling/fontSizeInMapUnits" ), QVariant( false ) ).toBool() ?
                       QgsUnitTypes::RenderMapUnits : QgsUnitTypes::RenderPoints;
  }
  else
  {
    bool ok = false;
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( layer->customProperty( QStringLiteral( "labeling/fontSizeUnit" ) ).toString(), &ok );
    if ( !ok )
      d->fontSizeUnits = QgsUnitTypes::RenderPoints;
  }
  if ( layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitScale" ) ).toString().isEmpty() )
  {
    //fallback to older property
    double oldMin = layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitMinScale" ), 0.0 ).toDouble();
    d->fontSizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitMaxScale" ), 0.0 ).toDouble();
    d->fontSizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( layer->customProperty( QStringLiteral( "labeling/fontSizeMapUnitScale" ) ).toString() );
  }
  int fontWeight = layer->customProperty( QStringLiteral( "labeling/fontWeight" ) ).toInt();
  bool fontItalic = layer->customProperty( QStringLiteral( "labeling/fontItalic" ) ).toBool();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( layer->customProperty( QStringLiteral( "labeling/namedStyle" ), QVariant( "" ) ).toString() );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->textFont.setCapitalization( static_cast< QFont::Capitalization >( layer->customProperty( QStringLiteral( "labeling/fontCapitals" ), QVariant( 0 ) ).toUInt() ) );
  d->textFont.setUnderline( layer->customProperty( QStringLiteral( "labeling/fontUnderline" ) ).toBool() );
  d->textFont.setStrikeOut( layer->customProperty( QStringLiteral( "labeling/fontStrikeout" ) ).toBool() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, layer->customProperty( QStringLiteral( "labeling/fontLetterSpacing" ), QVariant( 0.0 ) ).toDouble() );
  d->textFont.setWordSpacing( layer->customProperty( QStringLiteral( "labeling/fontWordSpacing" ), QVariant( 0.0 ) ).toDouble() );
  d->textColor = _readColor( layer, QStringLiteral( "labeling/textColor" ), Qt::black, false );
  if ( layer->customProperty( QStringLiteral( "labeling/textOpacity" ) ).toString().isEmpty() )
  {
    d->opacity = ( 1 - layer->customProperty( QStringLiteral( "labeling/textTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( layer->customProperty( QStringLiteral( "labeling/textOpacity" ) ).toDouble() );
  }
  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( layer->customProperty( QStringLiteral( "labeling/blendMode" ), QVariant( QgsPainting::BlendNormal ) ).toUInt() ) );
  d->multilineHeight = layer->customProperty( QStringLiteral( "labeling/multilineHeight" ), QVariant( 1.0 ) ).toDouble();
  d->previewBackgroundColor = _readColor( layer, QStringLiteral( "labeling/previewBkgrdColor" ), "#ffffff", false );

  mBufferSettings.readFromLayer( layer );
  mShadowSettings.readFromLayer( layer );
  mBackgroundSettings.readFromLayer( layer );
}

void QgsTextFormat::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  QDomElement textStyleElem;
  if ( elem.nodeName() == QStringLiteral( "text-style" ) )
    textStyleElem = elem;
  else
    textStyleElem = elem.firstChildElement( QStringLiteral( "text-style" ) );
  QFont appFont = QApplication::font();
  mTextFontFamily = textStyleElem.attribute( QStringLiteral( "fontFamily" ), appFont.family() );
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

  if ( textStyleElem.hasAttribute( QStringLiteral( "fontSize" ) ) )
  {
    d->fontSize = textStyleElem.attribute( QStringLiteral( "fontSize" ) ).toDouble();
  }
  else
  {
    d->fontSize = appFont.pointSizeF();
  }

  if ( !textStyleElem.hasAttribute( QStringLiteral( "fontSizeUnit" ) ) )
  {
    d->fontSizeUnits = textStyleElem.attribute( QStringLiteral( "fontSizeInMapUnits" ) ).toUInt() == 0 ? QgsUnitTypes::RenderPoints
                       : QgsUnitTypes::RenderMapUnits;
  }
  else
  {
    d->fontSizeUnits = QgsUnitTypes::decodeRenderUnit( textStyleElem.attribute( QStringLiteral( "fontSizeUnit" ) ) );
  }

  if ( !textStyleElem.hasAttribute( QStringLiteral( "fontSizeMapUnitScale" ) ) )
  {
    //fallback to older property
    double oldMin = textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitMinScale" ), QStringLiteral( "0" ) ).toDouble();
    d->fontSizeMapUnitScale.minScale = oldMin != 0 ? 1.0 / oldMin : 0;
    double oldMax = textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitMaxScale" ), QStringLiteral( "0" ) ).toDouble();
    d->fontSizeMapUnitScale.maxScale = oldMax != 0 ? 1.0 / oldMax : 0;
  }
  else
  {
    d->fontSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( textStyleElem.attribute( QStringLiteral( "fontSizeMapUnitScale" ) ) );
  }
  int fontWeight = textStyleElem.attribute( QStringLiteral( "fontWeight" ) ).toInt();
  bool fontItalic = textStyleElem.attribute( QStringLiteral( "fontItalic" ) ).toInt();
  d->textFont = QFont( fontFamily, d->fontSize, fontWeight, fontItalic );
  d->textFont.setPointSizeF( d->fontSize ); //double precision needed because of map units
  d->textNamedStyle = QgsFontUtils::translateNamedStyle( textStyleElem.attribute( QStringLiteral( "namedStyle" ) ) );
  QgsFontUtils::updateFontViaStyle( d->textFont, d->textNamedStyle ); // must come after textFont.setPointSizeF()
  d->textFont.setCapitalization( static_cast< QFont::Capitalization >( textStyleElem.attribute( QStringLiteral( "fontCapitals" ), QStringLiteral( "0" ) ).toUInt() ) );
  d->textFont.setUnderline( textStyleElem.attribute( QStringLiteral( "fontUnderline" ) ).toInt() );
  d->textFont.setStrikeOut( textStyleElem.attribute( QStringLiteral( "fontStrikeout" ) ).toInt() );
  d->textFont.setKerning( textStyleElem.attribute( QStringLiteral( "fontKerning" ), QStringLiteral( "1" ) ).toInt() );
  d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, textStyleElem.attribute( QStringLiteral( "fontLetterSpacing" ), QStringLiteral( "0" ) ).toDouble() );
  d->textFont.setWordSpacing( textStyleElem.attribute( QStringLiteral( "fontWordSpacing" ), QStringLiteral( "0" ) ).toDouble() );
  d->textColor = QgsSymbolLayerUtils::decodeColor( textStyleElem.attribute( QStringLiteral( "textColor" ), QgsSymbolLayerUtils::encodeColor( Qt::black ) ) );
  if ( !textStyleElem.hasAttribute( QStringLiteral( "textOpacity" ) ) )
  {
    d->opacity = ( 1 - textStyleElem.attribute( QStringLiteral( "textTransp" ) ).toInt() / 100.0 ); //0 -100
  }
  else
  {
    d->opacity = ( textStyleElem.attribute( QStringLiteral( "textOpacity" ) ).toDouble() );
  }
  d->orientation = QgsTextRendererUtils::decodeTextOrientation( textStyleElem.attribute( QStringLiteral( "textOrientation" ) ) );
  d->previewBackgroundColor = QgsSymbolLayerUtils::decodeColor( textStyleElem.attribute( QStringLiteral( "previewBkgrdColor" ), QgsSymbolLayerUtils::encodeColor( Qt::white ) ) );

  d->blendMode = QgsPainting::getCompositionMode(
                   static_cast< QgsPainting::BlendMode >( textStyleElem.attribute( QStringLiteral( "blendMode" ), QString::number( QgsPainting::BlendNormal ) ).toUInt() ) );

  if ( !textStyleElem.hasAttribute( QStringLiteral( "multilineHeight" ) ) )
  {
    QDomElement textFormatElem = elem.firstChildElement( QStringLiteral( "text-format" ) );
    d->multilineHeight = textFormatElem.attribute( QStringLiteral( "multilineHeight" ), QStringLiteral( "1" ) ).toDouble();
  }
  else
  {
    d->multilineHeight = textStyleElem.attribute( QStringLiteral( "multilineHeight" ), QStringLiteral( "1" ) ).toDouble();
  }

  if ( textStyleElem.firstChildElement( QStringLiteral( "text-buffer" ) ).isNull() )
  {
    mBufferSettings.readXml( elem );
  }
  else
  {
    mBufferSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "text-mask" ) ).isNull() )
  {
    mMaskSettings.readXml( elem );
  }
  else
  {
    mMaskSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "shadow" ) ).isNull() )
  {
    mShadowSettings.readXml( elem );
  }
  else
  {
    mShadowSettings.readXml( textStyleElem );
  }
  if ( textStyleElem.firstChildElement( QStringLiteral( "background" ) ).isNull() )
  {
    mBackgroundSettings.readXml( elem, context );
  }
  else
  {
    mBackgroundSettings.readXml( textStyleElem, context );
  }

  QDomElement ddElem = textStyleElem.firstChildElement( QStringLiteral( "dd_properties" ) );
  if ( ddElem.isNull() )
  {
    ddElem = elem.firstChildElement( QStringLiteral( "dd_properties" ) );
  }
  if ( !ddElem.isNull() )
  {
    d->mDataDefinedProperties.readXml( ddElem, QgsPalLayerSettings::propertyDefinitions() );
  }
  else
  {
    d->mDataDefinedProperties.clear();
  }
}

QDomElement QgsTextFormat::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  // text style
  QDomElement textStyleElem = doc.createElement( QStringLiteral( "text-style" ) );
  textStyleElem.setAttribute( QStringLiteral( "fontFamily" ), d->textFont.family() );
  textStyleElem.setAttribute( QStringLiteral( "namedStyle" ), QgsFontUtils::untranslateNamedStyle( d->textNamedStyle ) );
  textStyleElem.setAttribute( QStringLiteral( "fontSize" ), d->fontSize );
  textStyleElem.setAttribute( QStringLiteral( "fontSizeUnit" ), QgsUnitTypes::encodeUnit( d->fontSizeUnits ) );
  textStyleElem.setAttribute( QStringLiteral( "fontSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( d->fontSizeMapUnitScale ) );
  textStyleElem.setAttribute( QStringLiteral( "fontWeight" ), d->textFont.weight() );
  textStyleElem.setAttribute( QStringLiteral( "fontItalic" ), d->textFont.italic() );
  textStyleElem.setAttribute( QStringLiteral( "fontStrikeout" ), d->textFont.strikeOut() );
  textStyleElem.setAttribute( QStringLiteral( "fontUnderline" ), d->textFont.underline() );
  textStyleElem.setAttribute( QStringLiteral( "textColor" ), QgsSymbolLayerUtils::encodeColor( d->textColor ) );
  textStyleElem.setAttribute( QStringLiteral( "previewBkgrdColor" ), QgsSymbolLayerUtils::encodeColor( d->previewBackgroundColor ) );
  textStyleElem.setAttribute( QStringLiteral( "fontCapitals" ), static_cast< unsigned int >( d->textFont.capitalization() ) );
  textStyleElem.setAttribute( QStringLiteral( "fontLetterSpacing" ), d->textFont.letterSpacing() );
  textStyleElem.setAttribute( QStringLiteral( "fontWordSpacing" ), d->textFont.wordSpacing() );
  textStyleElem.setAttribute( QStringLiteral( "fontKerning" ), d->textFont.kerning() );
  textStyleElem.setAttribute( QStringLiteral( "textOpacity" ), d->opacity );
  textStyleElem.setAttribute( QStringLiteral( "textOrientation" ), QgsTextRendererUtils::encodeTextOrientation( d->orientation ) );
  textStyleElem.setAttribute( QStringLiteral( "blendMode" ), QgsPainting::getBlendModeEnum( d->blendMode ) );
  textStyleElem.setAttribute( QStringLiteral( "multilineHeight" ), d->multilineHeight );

  QDomElement ddElem = doc.createElement( QStringLiteral( "dd_properties" ) );
  d->mDataDefinedProperties.writeXml( ddElem, QgsPalLayerSettings::propertyDefinitions() );

  textStyleElem.appendChild( mBufferSettings.writeXml( doc ) );
  textStyleElem.appendChild( mMaskSettings.writeXml( doc ) );
  textStyleElem.appendChild( mBackgroundSettings.writeXml( doc, context ) );
  textStyleElem.appendChild( mShadowSettings.writeXml( doc ) );
  textStyleElem.appendChild( ddElem );

  return textStyleElem;
}

QMimeData *QgsTextFormat::toMimeData() const
{
  //set both the mime color data, and the text (format settings).
  QMimeData *mimeData = new QMimeData;
  mimeData->setColorData( QVariant( color() ) );

  QgsReadWriteContext rwContext;
  QDomDocument textDoc;
  QDomElement textElem = writeXml( textDoc, rwContext );
  textDoc.appendChild( textElem );
  mimeData->setText( textDoc.toString() );

  return mimeData;
}

QgsTextFormat QgsTextFormat::fromQFont( const QFont &font )
{
  QgsTextFormat format;
  format.setFont( font );
  if ( font.pointSizeF() > 0 )
  {
    format.setSize( font.pointSizeF() );
    format.setSizeUnit( QgsUnitTypes::RenderPoints );
  }
  else if ( font.pixelSize() > 0 )
  {
    format.setSize( font.pixelSize() );
    format.setSizeUnit( QgsUnitTypes::RenderPixels );
  }

  return format;
}

QFont QgsTextFormat::toQFont() const
{
  QFont f = font();
  switch ( sizeUnit() )
  {
    case QgsUnitTypes::RenderPoints:
      f.setPointSizeF( size() );
      break;

    case QgsUnitTypes::RenderMillimeters:
      f.setPointSizeF( size() * 2.83464567 );
      break;

    case QgsUnitTypes::RenderInches:
      f.setPointSizeF( size() * 72 );
      break;

    case QgsUnitTypes::RenderPixels:
      f.setPixelSize( static_cast< int >( std::round( size() ) ) );
      break;

    case QgsUnitTypes::RenderMapUnits:
    case QgsUnitTypes::RenderMetersInMapUnits:
    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
      // no meaning here
      break;
  }
  return f;
}

QgsTextFormat QgsTextFormat::fromMimeData( const QMimeData *data, bool *ok )
{
  if ( ok )
    *ok = false;
  QgsTextFormat format;
  if ( !data )
    return format;

  QString text = data->text();
  if ( !text.isEmpty() )
  {
    QDomDocument doc;
    QDomElement elem;
    QgsReadWriteContext rwContext;

    if ( doc.setContent( text ) )
    {
      elem = doc.documentElement();

      format.readXml( elem, rwContext );
      if ( ok )
        *ok = true;
      return format;
    }
  }
  return format;
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

QgsPropertyCollection &QgsTextFormat::dataDefinedProperties()
{
  return d->mDataDefinedProperties;
}

const QgsPropertyCollection &QgsTextFormat::dataDefinedProperties() const
{
  return d->mDataDefinedProperties;
}

void QgsTextFormat::setDataDefinedProperties( const QgsPropertyCollection &collection )
{
  d->mDataDefinedProperties = collection;
}

void QgsTextFormat::updateDataDefinedProperties( QgsRenderContext &context )
{
  if ( !d->mDataDefinedProperties.hasActiveProperties() )
    return;

  QString ddFontFamily;
  context.expressionContext().setOriginalValueVariable( d->textFont.family() );
  QVariant exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::Family, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString family = exprVal.toString().trimmed();
    if ( d->textFont.family() != family )
    {
      // testing for ddFontFamily in QFontDatabase.families() may be slow to do for every feature
      // (i.e. don't use QgsFontUtils::fontFamilyMatchOnSystem( family ) here)
      if ( QgsFontUtils::fontFamilyOnSystem( family ) )
      {
        ddFontFamily = family;
      }
    }
  }

  // data defined named font style?
  QString ddFontStyle;
  context.expressionContext().setOriginalValueVariable( d->textNamedStyle );
  exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontStyle, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString fontstyle = exprVal.toString().trimmed();
    ddFontStyle = fontstyle;
  }

  bool ddBold = false;
  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Bold ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.bold() );
    ddBold = d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Bold, context.expressionContext(), false ) ;
  }

  bool ddItalic = false;
  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Italic ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.italic() );
    ddItalic = d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Italic, context.expressionContext(), false );
  }

  // TODO: update when pref for how to resolve missing family (use matching algorithm or just default font) is implemented
  //       (currently defaults to what has been read in from layer settings)
  QFont newFont;
  QFontDatabase fontDb;
  QFont appFont = QApplication::font();
  bool newFontBuilt = false;
  if ( ddBold || ddItalic )
  {
    // new font needs built, since existing style needs removed
    newFont = QFont( !ddFontFamily.isEmpty() ? ddFontFamily : d->textFont.family() );
    newFontBuilt = true;
    newFont.setBold( ddBold );
    newFont.setItalic( ddItalic );
  }
  else if ( !ddFontStyle.isEmpty()
            && ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
  {
    if ( !ddFontFamily.isEmpty() )
    {
      // both family and style are different, build font from database
      QFont styledfont = fontDb.font( ddFontFamily, ddFontStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }

    // update the font face style
    QgsFontUtils::updateFontViaStyle( newFontBuilt ? newFont : d->textFont, ddFontStyle );
  }
  else if ( !ddFontFamily.isEmpty() )
  {
    if ( ddFontStyle.compare( QLatin1String( "Ignore" ), Qt::CaseInsensitive ) != 0 )
    {
      // just family is different, build font from database
      QFont styledfont = fontDb.font( ddFontFamily, d->textNamedStyle, appFont.pointSize() );
      if ( appFont != styledfont )
      {
        newFont = styledfont;
        newFontBuilt = true;
      }
    }
    else
    {
      newFont = QFont( ddFontFamily );
      newFontBuilt = true;
    }
  }

  if ( newFontBuilt )
  {
    // copy over existing font settings
    newFont.setUnderline( d->textFont.underline() );
    newFont.setStrikeOut( d->textFont.strikeOut() );
    newFont.setWordSpacing( d->textFont.wordSpacing() );
    newFont.setLetterSpacing( QFont::AbsoluteSpacing, d->textFont.letterSpacing() );
    d->textFont = newFont;
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Underline ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.underline() );
    d->textFont.setUnderline( d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Underline, context.expressionContext(), d->textFont.underline() ) );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Strikeout ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.strikeOut() );
    d->textFont.setStrikeOut( d->mDataDefinedProperties.valueAsBool( QgsPalLayerSettings::Strikeout, context.expressionContext(), d->textFont.strikeOut() ) );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Color ) )
  {
    context.expressionContext().setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( d->textColor ) );
    d->textColor = d->mDataDefinedProperties.valueAsColor( QgsPalLayerSettings::Color, context.expressionContext(), d->textColor );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::Size ) )
  {
    context.expressionContext().setOriginalValueVariable( size() );
    d->fontSize = d->mDataDefinedProperties.valueAsDouble( QgsPalLayerSettings::Size, context.expressionContext(), d->fontSize );
  }

  exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontSizeUnit, context.expressionContext() );
  if ( exprVal.isValid() )
  {
    QString units = exprVal.toString();
    if ( !units.isEmpty() )
    {
      bool ok;
      QgsUnitTypes::RenderUnit res = QgsUnitTypes::decodeRenderUnit( units, &ok );
      if ( ok )
        d->fontSizeUnits = res;
    }
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontOpacity ) )
  {
    context.expressionContext().setOriginalValueVariable( d->opacity * 100 );
    d->opacity = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontOpacity, context.expressionContext(), d->opacity * 100 ).toDouble() / 100.0;
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::TextOrientation ) )
  {
    const QString encoded = QgsTextRendererUtils::encodeTextOrientation( d->orientation );
    context.expressionContext().setOriginalValueVariable( encoded );
    d->orientation = QgsTextRendererUtils::decodeTextOrientation( d->mDataDefinedProperties.value( QgsPalLayerSettings::TextOrientation, context.expressionContext(), encoded ).toString() );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontLetterSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.letterSpacing() );
    d->textFont.setLetterSpacing( QFont::AbsoluteSpacing, d->mDataDefinedProperties.value( QgsPalLayerSettings::FontLetterSpacing, context.expressionContext(), d->textFont.letterSpacing() ).toDouble() );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontWordSpacing ) )
  {
    context.expressionContext().setOriginalValueVariable( d->textFont.wordSpacing() );
    d->textFont.setWordSpacing( d->mDataDefinedProperties.value( QgsPalLayerSettings::FontWordSpacing, context.expressionContext(), d->textFont.wordSpacing() ).toDouble() );
  }

  if ( d->mDataDefinedProperties.isActive( QgsPalLayerSettings::FontBlendMode ) )
  {
    exprVal = d->mDataDefinedProperties.value( QgsPalLayerSettings::FontBlendMode, context.expressionContext() );
    QString blendstr = exprVal.toString().trimmed();
    if ( !blendstr.isEmpty() )
      d->blendMode = QgsSymbolLayerUtils::decodeBlendMode( blendstr );
  }

  mShadowSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mBackgroundSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mBufferSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
  mMaskSettings.updateDataDefinedProperties( context, d->mDataDefinedProperties );
}

QPixmap QgsTextFormat::textFormatPreviewPixmap( const QgsTextFormat &format, QSize size, const QString &previewText, int padding )
{
  QgsTextFormat tempFormat = format;
  QPixmap pixmap( size );
  pixmap.fill( Qt::transparent );
  QPainter painter;
  painter.begin( &pixmap );

  painter.setRenderHint( QPainter::Antialiasing );

  QRect rect( 0, 0, size.width(), size.height() );

  // shameless eye candy - use a subtle gradient when drawing background
  painter.setPen( Qt::NoPen );
  QColor background1 = tempFormat.previewBackgroundColor();
  if ( ( background1.lightnessF() < 0.7 ) )
  {
    background1 = background1.darker( 125 );
  }
  else
  {
    background1 = background1.lighter( 125 );
  }
  QColor background2 = tempFormat.previewBackgroundColor();
  QLinearGradient linearGrad( QPointF( 0, 0 ), QPointF( 0, rect.height() ) );
  linearGrad.setColorAt( 0, background1 );
  linearGrad.setColorAt( 1, background2 );
  painter.setBrush( QBrush( linearGrad ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }
  painter.setBrush( Qt::NoBrush );
  painter.setPen( Qt::NoPen );
  padding += 1; // move text away from background border

  QgsRenderContext context;
  QgsMapToPixel newCoordXForm;
  newCoordXForm.setParameters( 1, 0, 0, 0, 0, 0 );
  context.setMapToPixel( newCoordXForm );

  context.setScaleFactor( QgsApplication::desktop()->logicalDpiX() / 25.4 );
  context.setUseAdvancedEffects( true );
  context.setPainter( &painter );

  // slightly inset text to account for buffer/background
  double xtrans = 0;
  if ( tempFormat.buffer().enabled() )
    xtrans = context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() );
  if ( tempFormat.background().enabled() && tempFormat.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
    xtrans = std::max( xtrans, context.convertToPainterUnits( tempFormat.background().size().width(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  double ytrans = 0.0;
  if ( tempFormat.buffer().enabled() )
    ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.buffer().size(), tempFormat.buffer().sizeUnit(), tempFormat.buffer().sizeMapUnitScale() ) );
  if ( tempFormat.background().enabled() )
    ytrans = std::max( ytrans, context.convertToPainterUnits( tempFormat.background().size().height(), tempFormat.background().sizeUnit(), tempFormat.background().sizeMapUnitScale() ) );

  const QStringList text = QStringList() << ( previewText.isEmpty() ? QObject::tr( "Aa" ) : previewText );
  const double textHeight = QgsTextRenderer::textHeight( context, tempFormat, text, QgsTextRenderer::Rect );
  QRectF textRect = rect;
  textRect.setLeft( xtrans + padding );
  textRect.setWidth( rect.width() - xtrans - 2 * padding );

  if ( textRect.width() > 2000 )
    textRect.setWidth( 2000 - 2 * padding );

  const double bottom = textRect.height() / 2 + textHeight / 2;
  textRect.setTop( bottom - textHeight );
  textRect.setBottom( bottom );

  QgsTextRenderer::drawText( textRect, 0, QgsTextRenderer::AlignCenter, text, context, tempFormat );

  // draw border on top of text
  painter.setBrush( Qt::NoBrush );
  painter.setPen( QPen( tempFormat.previewBackgroundColor().darker( 150 ), 0 ) );
  if ( size.width() > 30 )
  {
    painter.drawRoundedRect( rect, 6, 6 );
  }
  else
  {
    // don't use rounded rect for small previews
    painter.drawRect( rect );
  }
  painter.end();
  return pixmap;
}


int QgsTextRenderer::sizeToPixel( double size, const QgsRenderContext &c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &mapUnitScale )
{
  return static_cast< int >( c.convertToPainterUnits( size, unit, mapUnitScale ) + 0.5 ); //NOLINT
}

void QgsTextRenderer::drawText( const QRectF &rect, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, bool )
{
  QgsTextFormat tmpFormat = format;
  if ( format.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    tmpFormat.updateDataDefinedProperties( context );
  tmpFormat = updateShadowPosition( tmpFormat );

  if ( tmpFormat.background().enabled() )
  {
    drawPart( rect, rotation, alignment, textLines, context, tmpFormat, Background );
  }

  if ( tmpFormat.buffer().enabled() )
  {
    drawPart( rect, rotation, alignment, textLines, context, tmpFormat, Buffer );
  }

  drawPart( rect, rotation, alignment, textLines, context, tmpFormat, Text );
}

void QgsTextRenderer::drawText( QPointF point, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, bool )
{
  QgsTextFormat tmpFormat = format;
  if ( format.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    tmpFormat.updateDataDefinedProperties( context );
  tmpFormat = updateShadowPosition( tmpFormat );

  if ( tmpFormat.background().enabled() )
  {
    drawPart( point, rotation, alignment, textLines, context, tmpFormat, Background );
  }

  if ( tmpFormat.buffer().enabled() )
  {
    drawPart( point, rotation, alignment, textLines, context, tmpFormat, Buffer );
  }

  drawPart( point, rotation, alignment, textLines, context, tmpFormat, Text );
}

QgsTextFormat QgsTextRenderer::updateShadowPosition( const QgsTextFormat &format )
{
  if ( !format.shadow().enabled() || format.shadow().shadowPlacement() != QgsTextShadowSettings::ShadowLowest )
    return format;

  QgsTextFormat tmpFormat = format;
  if ( tmpFormat.background().enabled() && tmpFormat.background().type() != QgsTextBackgroundSettings::ShapeMarkerSymbol ) // background shadow not compatible with marker symbol backgrounds
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowShape );
  }
  else if ( tmpFormat.buffer().enabled() )
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowBuffer );
  }
  else
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowText );
  }
  return tmpFormat;
}

void QgsTextRenderer::drawPart( const QRectF &rect, double rotation, HAlignment alignment,
                                const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part, bool )
{
  if ( !context.painter() )
  {
    return;
  }

  Component component;
  component.dpiRatio = 1.0;
  component.origin = rect.topLeft();
  component.rotation = rotation;
  component.size = rect.size();
  component.hAlign = alignment;

  switch ( part )
  {
    case Background:
    {
      if ( !format.background().enabled() )
        return;

      if ( !qgsDoubleNear( rotation, 0.0 ) )
      {
        // get rotated label's center point

        double xc = rect.width() / 2.0;
        double yc = rect.height() / 2.0;

        double angle = -rotation;
        double xd = xc * std::cos( angle ) - yc * std::sin( angle );
        double yd = xc * std::sin( angle ) + yc * std::cos( angle );

        component.center = QPointF( component.origin.x() + xd, component.origin.y() + yd );
      }
      else
      {
        component.center = rect.center();
      }

      QgsTextRenderer::drawBackground( context, component, format, textLines, Rect );

      break;
    }

    case Buffer:
    {
      if ( !format.buffer().enabled() )
        break;
    }
    FALLTHROUGH
    case Text:
    case Shadow:
    {
      QFontMetricsF fm( format.scaledFont( context ) );
      drawTextInternal( part, context, format, component,
                        textLines,
                        &fm,
                        alignment );
      break;
    }
  }
}

void QgsTextRenderer::drawPart( QPointF origin, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part, bool )
{
  if ( !context.painter() )
  {
    return;
  }

  Component component;
  component.dpiRatio = 1.0;
  component.origin = origin;
  component.rotation = rotation;
  component.hAlign = alignment;

  switch ( part )
  {
    case Background:
    {
      if ( !format.background().enabled() )
        return;

      QgsTextRenderer::drawBackground( context, component, format, textLines, Point );
      break;
    }

    case Buffer:
    {
      if ( !format.buffer().enabled() )
        break;
    }
    FALLTHROUGH
    case Text:
    case Shadow:
    {
      QFontMetricsF fm( format.scaledFont( context ) );
      drawTextInternal( part, context, format, component,
                        textLines,
                        &fm,
                        alignment,
                        Point );
      break;
    }
  }
}

QFontMetricsF QgsTextRenderer::fontMetrics( QgsRenderContext &context, const QgsTextFormat &format )
{
  return QFontMetricsF( format.scaledFont( context ), context.painter() ? context.painter()->device() : nullptr );
}

void QgsTextRenderer::drawBuffer( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format, const QFontMetricsF *fontMetrics )
{
  QPainter *p = context.painter();

  QgsTextFormat::TextOrientation orientation = format.orientation();
  if ( format.orientation() == QgsTextFormat::RotationBasedOrientation )
  {
    if ( component.rotation >= -315 && component.rotation < -90 )
    {
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else if ( component.rotation >= -90 && component.rotation < -45 )
    {
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else
    {
      orientation = QgsTextFormat::HorizontalOrientation;
    }
  }

  QgsTextBufferSettings buffer = format.buffer();

  double penSize = context.convertToPainterUnits( buffer.size(), buffer.sizeUnit(), buffer.sizeMapUnitScale() );

  QPainterPath path;
  path.setFillRule( Qt::WindingFill );
  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
      path.addText( 0, 0, format.scaledFont( context ), component.text );
      break;
    case QgsTextFormat::VerticalOrientation:
    case QgsTextFormat::RotationBasedOrientation:
      double letterSpacing = format.scaledFont( context ).letterSpacing();
      double labelWidth = fontMetrics->maxWidth();
      const QStringList parts = QgsPalLabeling::splitToGraphemes( component.text );
      double partYOffset = 0.0;
      for ( const auto &part : parts )
      {
        double partXOffset = ( labelWidth - ( fontMetrics->width( part ) - letterSpacing ) ) / 2;
        path.addText( partXOffset, partYOffset, format.scaledFont( context ), part );
        partYOffset += fontMetrics->ascent() + letterSpacing;
      }
  }

  QColor bufferColor = buffer.color();
  bufferColor.setAlphaF( buffer.opacity() );
  QPen pen( bufferColor );
  pen.setWidthF( penSize );
  pen.setJoinStyle( buffer.joinStyle() );
  QColor tmpColor( bufferColor );
  // honor pref for whether to fill buffer interior
  if ( !buffer.fillBufferInterior() )
  {
    tmpColor.setAlpha( 0 );
  }

  // store buffer's drawing in QPicture for drop shadow call
  QPicture buffPict;
  QPainter buffp;
  buffp.begin( &buffPict );

  if ( buffer.paintEffect() && buffer.paintEffect()->enabled() )
  {
    context.setPainter( &buffp );

    buffer.paintEffect()->begin( context );
    context.painter()->setPen( pen );
    context.painter()->setBrush( tmpColor );
    context.painter()->drawPath( path );
    buffer.paintEffect()->end( context );

    context.setPainter( p );
  }
  else
  {
    buffp.setPen( pen );
    buffp.setBrush( tmpColor );
    buffp.drawPath( path );
  }
  buffp.end();

  if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowBuffer )
  {
    QgsTextRenderer::Component bufferComponent = component;
    bufferComponent.origin = QPointF( 0.0, 0.0 );
    bufferComponent.picture = buffPict;
    bufferComponent.pictureBuffer = penSize / 2.0;
    drawShadow( context, bufferComponent, format );
  }
  p->save();
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( buffer.blendMode() );
  }
  if ( context.flags() & QgsRenderContext::Antialiasing )
  {
    p->setRenderHint( QPainter::Antialiasing );
  }

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio, component.dpiRatio );
  _fixQPictureDPI( p );
  p->drawPicture( 0, 0, buffPict );
  p->restore();
}

void QgsTextRenderer::drawMask( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format )
{
  QgsTextMaskSettings mask = format.mask();

  // the mask is drawn to a side painter
  // or to the main painter for preview
  QPainter *p = context.isGuiPreview() ? context.painter() : context.maskPainter( context.currentMaskId() );
  if ( ! p )
    return;

  double penSize = context.convertToPainterUnits( mask.size(), mask.sizeUnit(), mask.sizeMapUnitScale() );

  // buffer: draw the text with a big pen
  QPainterPath path;
  path.setFillRule( Qt::WindingFill );
  path.addText( 0, 0, format.scaledFont( context ), component.text );

  QColor bufferColor( Qt::gray );
  bufferColor.setAlphaF( mask.opacity() );

  QPen pen;
  QBrush brush;
  brush.setColor( bufferColor );
  pen.setColor( bufferColor );
  pen.setWidthF( penSize );
  pen.setJoinStyle( mask.joinStyle() );

  p->save();

  if ( context.flags() & QgsRenderContext::Antialiasing )
  {
    p->setRenderHint( QPainter::Antialiasing );
  }

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio, component.dpiRatio );
  if ( mask.paintEffect() && mask.paintEffect()->enabled() )
  {
    QgsPainterSwapper swapper( context, p );
    {
      QgsEffectPainter effectPainter( context, mask.paintEffect() );
      context.painter()->setPen( pen );
      context.painter()->setBrush( brush );
      context.painter()->drawPath( path );
    }
  }
  else
  {
    p->setPen( pen );
    p->setBrush( brush );
    p->drawPath( path );
  }
  p->restore();
}

double QgsTextRenderer::textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, QFontMetricsF *fontMetrics )
{
  //calculate max width of text lines
  std::unique_ptr< QFontMetricsF > newFm;
  if ( !fontMetrics )
  {
    newFm.reset( new QFontMetricsF( format.scaledFont( context ) ) );
    fontMetrics = newFm.get();
  }

  double width = 0;
  switch ( format.orientation() )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      double maxLineWidth = 0;
      const auto constTextLines = textLines;
      for ( const QString &line : constTextLines )
      {
        maxLineWidth = std::max( maxLineWidth, fontMetrics->width( line ) );
      }
      width = maxLineWidth;
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    {
      double labelWidth = fontMetrics->maxWidth();
      width = labelWidth + ( textLines.size() - 1 ) * labelWidth * format.lineHeight();
      break;
    }

    case QgsTextFormat::RotationBasedOrientation:
    {
      // label mode only
      break;
    }
  }

  return width;
}

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, DrawMode mode, QFontMetricsF *fontMetrics )
{
  //calculate max width of text lines
  std::unique_ptr< QFontMetricsF > newFm;
  if ( !fontMetrics )
  {
    newFm.reset( new QFontMetricsF( format.scaledFont( context ) ) );
    fontMetrics = newFm.get();
  }

  switch ( format.orientation() )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      double labelHeight = fontMetrics->ascent() + fontMetrics->descent(); // ignore +1 for baseline
      switch ( mode )
      {
        case Label:
          // rendering labels needs special handling - in this case text should be
          // drawn with the bottom left corner coinciding with origin, vs top left
          // for standard text rendering. Line height is also slightly different.
          return labelHeight + ( textLines.size() - 1 ) * labelHeight * format.lineHeight();

        case Rect:
        case Point:
          // standard rendering - designed to exactly replicate QPainter's drawText method
          return labelHeight + ( textLines.size() - 1 ) * fontMetrics->lineSpacing() * format.lineHeight();
      }
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    {
      double labelHeight = fontMetrics->ascent();
      double letterSpacing = format.scaledFont( context ).letterSpacing();
      int maxLineLength = 0;
      for ( const auto &line : textLines )
      {
        if ( line.length() > maxLineLength )
          maxLineLength = line.length();
      }
      return labelHeight * maxLineLength + ( maxLineLength - 1 ) * letterSpacing;
      break;
    }

    case QgsTextFormat::RotationBasedOrientation:
    {
      // label mode only
      break;
    }
  }

  return 0;
}

void QgsTextRenderer::drawBackground( QgsRenderContext &context, QgsTextRenderer::Component component, const QgsTextFormat &format,
                                      const QStringList &textLines, DrawMode mode )
{
  QgsTextBackgroundSettings background = format.background();

  QPainter *prevP = context.painter();
  QPainter *p = context.painter();
  if ( background.paintEffect() && background.paintEffect()->enabled() )
  {
    background.paintEffect()->begin( context );
    p = context.painter();
  }

  //QgsDebugMsgLevel( QStringLiteral( "Background label rotation: %1" ).arg( component.rotation() ), 4 );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  if ( background.rotationType() != QgsTextBackgroundSettings::RotationFixed )
  {
    component.rotation = -( component.rotation * 180 / M_PI ); // RotationSync
    component.rotationOffset =
      background.rotationType() == QgsTextBackgroundSettings::RotationOffset ? background.rotation() : 0.0;
  }
  else // RotationFixed
  {
    component.rotation = 0.0; // don't use label's rotation
    component.rotationOffset = background.rotation();
  }

  if ( mode != Label )
  {
    // need to calculate size of text
    QFontMetricsF fm( format.scaledFont( context ) );
    double width = textWidth( context, format, textLines, &fm );
    double height = textHeight( context, format, textLines, mode, &fm );

    switch ( mode )
    {
      case Rect:
        switch ( component.hAlign )
        {
          case AlignLeft:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case AlignCenter:
            component.center = QPointF( component.origin.x() + component.size.width() / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case AlignRight:
            component.center = QPointF( component.origin.x() + component.size.width() - width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;
        }
        break;

      case Point:
      {
        double originAdjust = fm.ascent() / 2.0 - fm.leading() / 2.0;
        switch ( component.hAlign )
        {
          case AlignLeft:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case AlignCenter:
            component.center = QPointF( component.origin.x(),
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case AlignRight:
            component.center = QPointF( component.origin.x() - width / 2.0,
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;
        }
        break;
      }

      case Label:
        break;
    }

    if ( format.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
      component.size = QSizeF( width, height );
  }

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  switch ( background.type() )
  {
    case QgsTextBackgroundSettings::ShapeSVG:
    case QgsTextBackgroundSettings::ShapeMarkerSymbol:
    {
      // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

      if ( background.type() == QgsTextBackgroundSettings::ShapeSVG && background.svgFile().isEmpty() )
        return;

      if ( background.type() == QgsTextBackgroundSettings::ShapeMarkerSymbol && !background.markerSymbol() )
        return;

      double sizeOut = 0.0;
      // only one size used for SVG/marker symbol sizing/scaling (no use of shapeSize.y() or Y field in gui)
      if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
      {
        sizeOut = context.convertToPainterUnits( background.size().width(), background.sizeUnit(), background.sizeMapUnitScale() );
      }
      else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
      {
        sizeOut = std::max( component.size.width(), component.size.height() );
        double bufferSize = context.convertToPainterUnits( background.size().width(), background.sizeUnit(), background.sizeMapUnitScale() );

        // add buffer
        sizeOut += bufferSize * 2;
      }

      // don't bother rendering symbols smaller than 1x1 pixels in size
      // TODO: add option to not show any svgs under/over a certain size
      if ( sizeOut < 1.0 )
        return;

      std::unique_ptr< QgsMarkerSymbol > renderedSymbol;
      if ( background.type() == QgsTextBackgroundSettings::ShapeSVG )
      {
        QgsStringMap map; // for SVG symbology marker
        map[QStringLiteral( "name" )] = background.svgFile().trimmed();
        map[QStringLiteral( "size" )] = QString::number( sizeOut );
        map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( QgsUnitTypes::RenderPixels );
        map[QStringLiteral( "angle" )] = QString::number( 0.0 ); // angle is handled by this local painter

        // offset is handled by this local painter
        // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
        //map["offset"] = QgsSymbolLayerUtils::encodePoint( tmpLyr.shapeOffset );
        //map["offset_unit"] = QgsUnitTypes::encodeUnit(
        //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsUnitTypes::MapUnit : QgsUnitTypes::MM );

        map[QStringLiteral( "fill" )] = background.fillColor().name();
        map[QStringLiteral( "outline" )] = background.strokeColor().name();
        map[QStringLiteral( "outline-width" )] = QString::number( background.strokeWidth() );
        map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( background.strokeWidthUnit() );

        if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
        {
          QgsTextShadowSettings shadow = format.shadow();
          // configure SVG shadow specs
          QgsStringMap shdwmap( map );
          shdwmap[QStringLiteral( "fill" )] = shadow.color().name();
          shdwmap[QStringLiteral( "outline" )] = shadow.color().name();
          shdwmap[QStringLiteral( "size" )] = QString::number( sizeOut );

          // store SVG's drawing in QPicture for drop shadow call
          QPicture svgPict;
          QPainter svgp;
          svgp.begin( &svgPict );

          // draw shadow symbol

          // clone current render context map unit/mm conversion factors, but not
          // other map canvas parameters, then substitute this painter for use in symbology painting
          // NOTE: this is because the shadow needs to be scaled correctly for output to map canvas,
          //       but will be created relative to the SVG's computed size, not the current map canvas
          QgsRenderContext shdwContext;
          shdwContext.setMapToPixel( context.mapToPixel() );
          shdwContext.setScaleFactor( context.scaleFactor() );
          shdwContext.setPainter( &svgp );

          QgsSymbolLayer *symShdwL = QgsSvgMarkerSymbolLayer::create( shdwmap );
          QgsSvgMarkerSymbolLayer *svgShdwM = static_cast<QgsSvgMarkerSymbolLayer *>( symShdwL );
          QgsSymbolRenderContext svgShdwContext( shdwContext, QgsUnitTypes::RenderUnknownUnit, background.opacity() );

          svgShdwM->renderPoint( QPointF( sizeOut / 2, -sizeOut / 2 ), svgShdwContext );
          svgp.end();

          component.picture = svgPict;
          // TODO: when SVG symbol's stroke width/units is fixed in QgsSvgCache, adjust for it here
          component.pictureBuffer = 0.0;

          component.size = QSizeF( sizeOut, sizeOut );
          component.offset = QPointF( 0.0, 0.0 );

          // rotate about origin center of SVG
          p->save();
          p->translate( component.center.x(), component.center.y() );
          p->rotate( component.rotation );
          double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
          double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
          p->translate( QPointF( xoff, yoff ) );
          p->rotate( component.rotationOffset );
          p->translate( -sizeOut / 2, sizeOut / 2 );
          if ( context.flags() & QgsRenderContext::Antialiasing )
          {
            p->setRenderHint( QPainter::Antialiasing );
          }

          drawShadow( context, component, format );
          p->restore();

          delete svgShdwM;
          svgShdwM = nullptr;
        }
        renderedSymbol.reset( );

        QgsSymbolLayer *symL = QgsSvgMarkerSymbolLayer::create( map );
        renderedSymbol.reset( new QgsMarkerSymbol( QgsSymbolLayerList() << symL ) );
      }
      else
      {
        renderedSymbol.reset( background.markerSymbol()->clone() );
        renderedSymbol->setSize( sizeOut );
        renderedSymbol->setSizeUnit( QgsUnitTypes::RenderPixels );
      }

      renderedSymbol->setOpacity( background.opacity() );

      // draw the actual symbol
      p->save();
      if ( context.useAdvancedEffects() )
      {
        p->setCompositionMode( background.blendMode() );
      }
      if ( context.flags() & QgsRenderContext::Antialiasing )
      {
        p->setRenderHint( QPainter::Antialiasing );
      }
      p->translate( component.center.x(), component.center.y() );
      p->rotate( component.rotation );
      double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
      double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset );

      const QgsFeature f = context.expressionContext().feature();
      renderedSymbol->startRender( context, context.expressionContext().fields() );
      renderedSymbol->renderPoint( QPointF( 0, 0 ), &f, context );
      renderedSymbol->stopRender( context );
      p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
      p->restore();

      break;
    }

    case QgsTextBackgroundSettings::ShapeRectangle:
    case QgsTextBackgroundSettings::ShapeCircle:
    case QgsTextBackgroundSettings::ShapeSquare:
    case QgsTextBackgroundSettings::ShapeEllipse:
    {
      double w = component.size.width();
      double h = component.size.height();

      if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
      {
        w = context.convertToPainterUnits( background.size().width(), background.sizeUnit(),
                                           background.sizeMapUnitScale() );
        h = context.convertToPainterUnits( background.size().height(), background.sizeUnit(),
                                           background.sizeMapUnitScale() );
      }
      else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
      {
        if ( background.type() == QgsTextBackgroundSettings::ShapeSquare )
        {
          if ( w > h )
            h = w;
          else if ( h > w )
            w = h;
        }
        else if ( background.type() == QgsTextBackgroundSettings::ShapeCircle )
        {
          // start with label bound by circle
          h = std::sqrt( std::pow( w, 2 ) + std::pow( h, 2 ) );
          w = h;
        }
        else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse )
        {
          // start with label bound by ellipse
          h = h * M_SQRT1_2 * 2;
          w = w * M_SQRT1_2 * 2;
        }

        double bufferWidth = context.convertToPainterUnits( background.size().width(), background.sizeUnit(),
                             background.sizeMapUnitScale() );
        double bufferHeight = context.convertToPainterUnits( background.size().height(), background.sizeUnit(),
                              background.sizeMapUnitScale() );

        w += bufferWidth * 2;
        h += bufferHeight * 2;
      }

      // offsets match those of symbology: -x = left, -y = up
      QRectF rect( -w / 2.0, - h / 2.0, w, h );

      if ( rect.isNull() )
        return;

      p->save();
      if ( context.flags() & QgsRenderContext::Antialiasing )
      {
        p->setRenderHint( QPainter::Antialiasing );
      }
      p->translate( QPointF( component.center.x(), component.center.y() ) );
      p->rotate( component.rotation );
      double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
      double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset );

      double penSize = context.convertToPainterUnits( background.strokeWidth(), background.strokeWidthUnit(), background.strokeWidthMapUnitScale() );

      QPen pen;
      if ( background.strokeWidth() > 0 )
      {
        pen.setColor( background.strokeColor() );
        pen.setWidthF( penSize );
        if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle )
          pen.setJoinStyle( background.joinStyle() );
      }
      else
      {
        pen = Qt::NoPen;
      }

      // store painting in QPicture for shadow drawing
      QPicture shapePict;
      QPainter shapep;
      shapep.begin( &shapePict );
      shapep.setPen( pen );
      shapep.setBrush( background.fillColor() );

      if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle
           || background.type() == QgsTextBackgroundSettings::ShapeSquare )
      {
        if ( background.radiiUnit() == QgsUnitTypes::RenderPercentage )
        {
          shapep.drawRoundedRect( rect, background.radii().width(), background.radii().height(), Qt::RelativeSize );
        }
        else
        {
          double xRadius = context.convertToPainterUnits( background.radii().width(), background.radiiUnit(), background.radiiMapUnitScale() );
          double yRadius = context.convertToPainterUnits( background.radii().height(), background.radiiUnit(), background.radiiMapUnitScale() );
          shapep.drawRoundedRect( rect, xRadius, yRadius );
        }
      }
      else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse
                || background.type() == QgsTextBackgroundSettings::ShapeCircle )
      {
        shapep.drawEllipse( rect );
      }
      shapep.end();

      if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
      {
        component.picture = shapePict;
        component.pictureBuffer = penSize / 2.0;

        component.size = rect.size();
        component.offset = QPointF( rect.width() / 2, -rect.height() / 2 );
        drawShadow( context, component, format );
      }

      p->setOpacity( background.opacity() );
      if ( context.useAdvancedEffects() )
      {
        p->setCompositionMode( background.blendMode() );
      }

      // scale for any print output or image saving @ specific dpi
      p->scale( component.dpiRatio, component.dpiRatio );
      _fixQPictureDPI( p );
      p->drawPicture( 0, 0, shapePict );
      p->restore();
      break;
    }
  }

  if ( background.paintEffect() && background.paintEffect()->enabled() )
  {
    background.paintEffect()->end( context );
    context.setPainter( prevP );
  }
}

void QgsTextRenderer::drawShadow( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format )
{
  QgsTextShadowSettings shadow = format.shadow();

  // incoming component sizes should be multiplied by rasterCompressFactor, as
  // this allows shadows to be created at paint device dpi (e.g. high resolution),
  // then scale device painter by 1.0 / rasterCompressFactor for output

  QPainter *p = context.painter();
  double componentWidth = component.size.width(), componentHeight = component.size.height();
  double xOffset = component.offset.x(), yOffset = component.offset.y();
  double pictbuffer = component.pictureBuffer;

  // generate pixmap representation of label component drawing
  bool mapUnits = shadow.blurRadiusUnit() == QgsUnitTypes::RenderMapUnits;
  double radius = context.convertToPainterUnits( shadow.blurRadius(), shadow.blurRadiusUnit(), shadow.blurRadiusMapUnitScale() );
  radius /= ( mapUnits ? context.scaleFactor() / component.dpiRatio : 1 );
  radius = static_cast< int >( radius + 0.5 ); //NOLINT

  // TODO: add labeling gui option to adjust blurBufferClippingScale to minimize pixels, or
  //       to ensure shadow isn't clipped too tight. (Or, find a better method of buffering)
  double blurBufferClippingScale = 3.75;
  int blurbuffer = ( radius > 17 ? 16 : radius ) * blurBufferClippingScale;

  QImage blurImg( componentWidth + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  componentHeight + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  QImage::Format_ARGB32_Premultiplied );

  // TODO: add labeling gui option to not show any shadows under/over a certain size
  // keep very small QImages from causing paint device issues, i.e. must be at least > 1
  int minBlurImgSize = 1;
  // max limitation on QgsSvgCache is 10,000 for screen, which will probably be reasonable for future caching here, too
  // 4 x QgsSvgCache limit for output to print/image at higher dpi
  // TODO: should it be higher, scale with dpi, or have no limit? Needs testing with very large labels rendered at high dpi output
  int maxBlurImgSize = 40000;
  if ( blurImg.isNull()
       || ( blurImg.width() < minBlurImgSize || blurImg.height() < minBlurImgSize )
       || ( blurImg.width() > maxBlurImgSize || blurImg.height() > maxBlurImgSize ) )
    return;

  blurImg.fill( QColor( Qt::transparent ).rgba() );
  QPainter pictp;
  if ( !pictp.begin( &blurImg ) )
    return;
  pictp.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPointF imgOffset( blurbuffer + pictbuffer + xOffset,
                     blurbuffer + pictbuffer + componentHeight + yOffset );

  pictp.drawPicture( imgOffset,
                     component.picture );

  // overlay shadow color
  pictp.setCompositionMode( QPainter::CompositionMode_SourceIn );
  pictp.fillRect( blurImg.rect(), shadow.color() );
  pictp.end();

  // blur the QImage in-place
  if ( shadow.blurRadius() > 0.0 && radius > 0 )
  {
    QgsSymbolLayerUtils::blurImageInPlace( blurImg, blurImg.rect(), radius, shadow.blurAlphaOnly() );
  }

#if 0
  // debug rect for QImage shadow registration and clipping visualization
  QPainter picti;
  picti.begin( &blurImg );
  picti.setBrush( Qt::Dense7Pattern );
  QPen imgPen( QColor( 0, 0, 255, 255 ) );
  imgPen.setWidth( 1 );
  picti.setPen( imgPen );
  picti.setOpacity( 0.1 );
  picti.drawRect( 0, 0, blurImg.width(), blurImg.height() );
  picti.end();
#endif

  double offsetDist = context.convertToPainterUnits( shadow.offsetDistance(), shadow.offsetUnit(), shadow.offsetMapUnitScale() );
  double angleRad = shadow.offsetAngle() * M_PI / 180; // to radians
  if ( shadow.offsetGlobal() )
  {
    // TODO: check for differences in rotation origin and cw/ccw direction,
    //       when this shadow function is used for something other than labels

    // it's 0-->cw-->360 for labels
    //QgsDebugMsgLevel( QStringLiteral( "Shadow aggregated label rotation (degrees): %1" ).arg( component.rotation() + component.rotationOffset() ), 4 );
    angleRad -= ( component.rotation * M_PI / 180 + component.rotationOffset * M_PI / 180 );
  }

  QPointF transPt( -offsetDist * std::cos( angleRad + M_PI_2 ),
                   -offsetDist * std::sin( angleRad + M_PI_2 ) );

  p->save();
  p->setRenderHint( QPainter::SmoothPixmapTransform );
  if ( context.flags() & QgsRenderContext::Antialiasing )
  {
    p->setRenderHint( QPainter::Antialiasing );
  }
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( shadow.blendMode() );
  }
  p->setOpacity( shadow.opacity() );

  double scale = shadow.scale() / 100.0;
  // TODO: scale from center/center, left/center or left/top, instead of default left/bottom?
  p->scale( scale, scale );
  if ( component.useOrigin )
  {
    p->translate( component.origin.x(), component.origin.y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawImage( 0, 0, blurImg );
  p->restore();

  // debug rects
#if 0
  // draw debug rect for QImage painting registration
  p->save();
  p->setBrush( Qt::NoBrush );
  QPen imgPen( QColor( 255, 0, 0, 10 ) );
  imgPen.setWidth( 2 );
  imgPen.setStyle( Qt::DashLine );
  p->setPen( imgPen );
  p->scale( scale, scale );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawRect( 0, 0, blurImg.width(), blurImg.height() );
  p->restore();

  // draw debug rect for passed in component dimensions
  p->save();
  p->setBrush( Qt::NoBrush );
  QPen componentRectPen( QColor( 0, 255, 0, 70 ) );
  componentRectPen.setWidth( 1 );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->setPen( componentRectPen );
  p->drawRect( QRect( -xOffset, -componentHeight - yOffset, componentWidth, componentHeight ) );
  p->restore();
#endif
}

void QgsTextRenderer::drawTextInternal( TextPart drawType,
                                        QgsRenderContext &context,
                                        const QgsTextFormat &format,
                                        const Component &component,
                                        const QStringList &textLines,
                                        const QFontMetricsF *fontMetrics,
                                        HAlignment alignment, DrawMode mode )
{
  if ( !context.painter() )
  {
    return;
  }

  QPainter *maskPainter = context.maskPainter( context.currentMaskId() );

  QgsTextFormat::TextOrientation orientation = format.orientation();
  double rotation = -component.rotation * 180 / M_PI;
  if ( format.orientation() == QgsTextFormat::RotationBasedOrientation )
  {
    // Between 45 to 135 and 235 to 315 degrees, rely on vertical orientation
    if ( rotation >= -315 && rotation < -90 )
    {
      rotation -= 90;
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else if ( rotation >= -90 && rotation < -45 )
    {
      rotation += 90;
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else
    {
      orientation = QgsTextFormat::HorizontalOrientation;
    }
  }

  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      double labelWidest = 0.0;
      switch ( mode )
      {
        case Label:
        case Point:
          for ( const QString &line : textLines )
          {
            double labelWidth = fontMetrics->width( line );
            if ( labelWidth > labelWidest )
            {
              labelWidest = labelWidth;
            }
          }
          break;

        case Rect:
          labelWidest = component.size.width();
          break;
      }

      double labelHeight = fontMetrics->ascent() + fontMetrics->descent(); // ignore +1 for baseline
      //  double labelHighest = labelfm->height() + ( double )(( lines - 1 ) * labelHeight * tmpLyr.multilineHeight );

      // needed to move bottom of text's descender to within bottom edge of label
      double ascentOffset = 0.25 * fontMetrics->ascent(); // labelfm->descent() is not enough

      int i = 0;

      bool adjustForAlignment = alignment != AlignLeft && ( mode != Label || textLines.size() > 1 );

      const auto constTextLines = textLines;
      for ( const QString &line : constTextLines )
      {
        context.painter()->save();
        if ( context.flags() & QgsRenderContext::Antialiasing )
        {
          context.painter()->setRenderHint( QPainter::Antialiasing );
        }
        context.painter()->translate( component.origin );
        if ( !qgsDoubleNear( rotation, 0.0 ) )
          context.painter()->rotate( rotation );

        // apply to the mask painter the same transformations
        if ( maskPainter )
        {
          maskPainter->save();
          maskPainter->translate( component.origin );
          if ( !qgsDoubleNear( rotation, 0.0 ) )
            maskPainter->rotate( rotation );
        }

        // figure x offset for horizontal alignment of multiple lines
        double xMultiLineOffset = 0.0;
        double labelWidth = fontMetrics->width( line );
        if ( adjustForAlignment )
        {
          double labelWidthDiff = labelWidest - labelWidth;
          if ( alignment == AlignCenter )
          {
            labelWidthDiff /= 2;
          }
          switch ( mode )
          {
            case Label:
            case Rect:
              xMultiLineOffset = labelWidthDiff;
              break;

            case Point:
              if ( alignment == AlignRight )
                xMultiLineOffset = labelWidthDiff - labelWidest;
              else if ( alignment == AlignCenter )
                xMultiLineOffset = labelWidthDiff - labelWidest / 2.0;

              break;
          }
          //QgsDebugMsgLevel( QStringLiteral( "xMultiLineOffset: %1" ).arg( xMultiLineOffset ), 4 );
        }

        double yMultiLineOffset = ascentOffset;
        switch ( mode )
        {
          case Label:
            // rendering labels needs special handling - in this case text should be
            // drawn with the bottom left corner coinciding with origin, vs top left
            // for standard text rendering. Line height is also slightly different.
            yMultiLineOffset = - ascentOffset - ( textLines.size() - 1 - i ) * labelHeight * format.lineHeight();
            break;

          case Rect:
            // standard rendering - designed to exactly replicate QPainter's drawText method
            yMultiLineOffset = - ascentOffset + labelHeight - 1 /*baseline*/ + format.lineHeight() * fontMetrics->lineSpacing() * i;
            break;

          case Point:
            // standard rendering - designed to exactly replicate QPainter's drawText rect method
            yMultiLineOffset = 0 - ( textLines.size() - 1 - i ) * fontMetrics->lineSpacing() * format.lineHeight();
            break;

        }

        context.painter()->translate( QPointF( xMultiLineOffset, yMultiLineOffset ) );
        if ( maskPainter )
          maskPainter->translate( QPointF( xMultiLineOffset, yMultiLineOffset ) );

        Component subComponent;
        subComponent.text = line;
        subComponent.size = QSizeF( labelWidth, labelHeight );
        subComponent.offset = QPointF( 0.0, -ascentOffset );
        subComponent.rotation = -component.rotation * 180 / M_PI;
        subComponent.rotationOffset = 0.0;

        // draw the mask below the text (for preview)
        if ( format.mask().enabled() )
        {
          QgsTextRenderer::drawMask( context, subComponent, format );
        }

        if ( drawType == QgsTextRenderer::Buffer )
        {
          QgsTextRenderer::drawBuffer( context, subComponent, format, fontMetrics );
        }
        else
        {
          // draw text, QPainterPath method
          QPainterPath path;
          path.setFillRule( Qt::WindingFill );
          path.addText( 0, 0, format.scaledFont( context ), subComponent.text );

          // store text's drawing in QPicture for drop shadow call
          QPicture textPict;
          QPainter textp;
          textp.begin( &textPict );
          textp.setPen( Qt::NoPen );
          QColor textColor = format.color();
          textColor.setAlphaF( format.opacity() );
          textp.setBrush( textColor );
          textp.drawPath( path );
          // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
          //       e.g. some capitalization options, but not others
          //textp.setFont( tmpLyr.textFont );
          //textp.setPen( tmpLyr.textColor );
          //textp.drawText( 0, 0, component.text() );
          textp.end();

          if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText )
          {
            subComponent.picture = textPict;
            subComponent.pictureBuffer = 0.0; // no pen width to deal with
            subComponent.origin = QPointF( 0.0, 0.0 );

            QgsTextRenderer::drawShadow( context, subComponent, format );
          }

          // paint the text
          if ( context.useAdvancedEffects() )
          {
            context.painter()->setCompositionMode( format.blendMode() );
          }

          // scale for any print output or image saving @ specific dpi
          context.painter()->scale( subComponent.dpiRatio, subComponent.dpiRatio );

          switch ( context.textRenderFormat() )
          {
            case QgsRenderContext::TextFormatAlwaysOutlines:
            {
              // draw outlined text
              _fixQPictureDPI( context.painter() );
              context.painter()->drawPicture( 0, 0, textPict );
              break;
            }

            case QgsRenderContext::TextFormatAlwaysText:
            {
              context.painter()->setFont( format.scaledFont( context ) );
              QColor textColor = format.color();
              textColor.setAlphaF( format.opacity() );
              context.painter()->setPen( textColor );
              context.painter()->setRenderHint( QPainter::TextAntialiasing );
              context.painter()->drawText( 0, 0, subComponent.text );
            }
          }
        }
        context.painter()->restore();
        if ( maskPainter )
          maskPainter->restore();
        i++;
      }
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    case QgsTextFormat::RotationBasedOrientation:
    {
      double letterSpacing = format.scaledFont( context ).letterSpacing();

      double labelWidth = fontMetrics->maxWidth(); // label width represents the width of one line of a multi-line label
      double actualLabelWidest = labelWidth + ( textLines.size() - 1 ) * labelWidth * format.lineHeight();
      double labelWidest = 0.0;
      switch ( mode )
      {
        case Label:
        case Point:
          labelWidest = actualLabelWidest;
          break;

        case Rect:
          labelWidest = component.size.width();
          break;
      }

      int maxLineLength = 0;
      for ( auto const &line : textLines )
      {
        maxLineLength = std::max( maxLineLength, line.length() );
      }
      double actualLabelHeight = fontMetrics->ascent() + ( fontMetrics->ascent() + letterSpacing ) * ( maxLineLength - 1 );
      double ascentOffset = fontMetrics->ascent();

      int i = 0;

      bool adjustForAlignment = alignment != AlignLeft && ( mode != Label || textLines.size() > 1 );

      // apply some character replacement to draw symbols in vertical presentation
      QString lines = textLines.join( '\n' );
      const auto constTextLines = QgsStringUtils::substituteVerticalCharacters( lines ).split( '\n' );
      for ( QString line : constTextLines )
      {
        context.painter()->save();
        if ( context.flags() & QgsRenderContext::Antialiasing )
        {
          context.painter()->setRenderHint( QPainter::Antialiasing );
        }
        context.painter()->translate( component.origin );
        if ( !qgsDoubleNear( rotation, 0.0 ) )
          context.painter()->rotate( rotation );

        // apply to the mask painter the same transformations
        if ( maskPainter )
        {
          maskPainter->save();
          maskPainter->translate( component.origin );
          if ( !qgsDoubleNear( rotation, 0.0 ) )
            maskPainter->rotate( rotation );
        }

        // figure x offset of multiple lines
        double xOffset = actualLabelWidest - labelWidth - ( i * labelWidth * format.lineHeight() );
        if ( adjustForAlignment )
        {
          double labelWidthDiff = labelWidest - actualLabelWidest;
          if ( alignment == AlignCenter )
          {
            labelWidthDiff /= 2;
          }
          switch ( mode )
          {
            case Label:
            case Rect:
              xOffset += labelWidthDiff;
              break;

            case Point:
              break;
          }
        }

        double yOffset = 0.0;
        switch ( mode )
        {
          case Label:
            if ( format.orientation() == QgsTextFormat::RotationBasedOrientation )
            {
              if ( rotation >= -405 && rotation < -180 )
              {
                yOffset = ascentOffset;
              }
              else if ( rotation >= 0 && rotation < 45 )
              {
                xOffset -= actualLabelWidest;
                yOffset = -actualLabelHeight + ascentOffset + fontMetrics->descent();
              }
            }
            else
            {
              yOffset = -actualLabelHeight + ascentOffset;
            }
            break;

          case Point:
            yOffset = -actualLabelHeight + ascentOffset;
            break;

          case Rect:
            yOffset = ascentOffset;
            break;
        }

        context.painter()->translate( QPointF( xOffset, yOffset ) );

        double labelHeight = fontMetrics->ascent() + ( fontMetrics->ascent() + letterSpacing ) * ( line.length() - 1 );

        Component subComponent;
        subComponent.text = line;
        subComponent.size = QSizeF( labelWidth, labelHeight );
        subComponent.offset = QPointF( 0.0, 0.0 );
        subComponent.rotation = -component.rotation * 180 / M_PI;
        subComponent.rotationOffset = 0.0;

        // draw the mask below the text (for preview)
        if ( format.mask().enabled() )
        {
          QgsTextRenderer::drawMask( context, subComponent, format );
        }

        if ( drawType == QgsTextRenderer::Buffer )
        {
          QgsTextRenderer::drawBuffer( context, subComponent, format, fontMetrics );
        }
        else
        {
          // draw text, QPainterPath method
          QPainterPath path;
          path.setFillRule( Qt::WindingFill );
          const QStringList parts = QgsPalLabeling::splitToGraphemes( subComponent.text );
          double partYOffset = 0.0;
          for ( const auto &part : parts )
          {
            double partXOffset = ( labelWidth - ( fontMetrics->width( part ) - letterSpacing ) ) / 2;
            path.addText( partXOffset, partYOffset, format.scaledFont( context ), part );
            partYOffset += fontMetrics->ascent() + letterSpacing;
          }

          // store text's drawing in QPicture for drop shadow call
          QPicture textPict;
          QPainter textp;
          textp.begin( &textPict );
          textp.setPen( Qt::NoPen );
          QColor textColor = format.color();
          textColor.setAlphaF( format.opacity() );
          textp.setBrush( textColor );
          textp.drawPath( path );
          // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
          //       e.g. some capitalization options, but not others
          //textp.setFont( tmpLyr.textFont );
          //textp.setPen( tmpLyr.textColor );
          //textp.drawText( 0, 0, component.text() );
          textp.end();

          if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText )
          {
            subComponent.picture = textPict;
            subComponent.pictureBuffer = 0.0; // no pen width to deal with
            subComponent.origin = QPointF( 0.0, 0.0 );

            QgsTextRenderer::drawShadow( context, subComponent, format );
          }

          // paint the text
          if ( context.useAdvancedEffects() )
          {
            context.painter()->setCompositionMode( format.blendMode() );
          }

          // scale for any print output or image saving @ specific dpi
          context.painter()->scale( subComponent.dpiRatio, subComponent.dpiRatio );

          switch ( context.textRenderFormat() )
          {
            case QgsRenderContext::TextFormatAlwaysOutlines:
            {
              // draw outlined text
              _fixQPictureDPI( context.painter() );
              context.painter()->drawPicture( 0, 0, textPict );
              break;
            }

            case QgsRenderContext::TextFormatAlwaysText:
            {
              context.painter()->setFont( format.scaledFont( context ) );
              QColor textColor = format.color();
              textColor.setAlphaF( format.opacity() );
              context.painter()->setPen( textColor );
              context.painter()->setRenderHint( QPainter::TextAntialiasing );
              context.painter()->drawText( 0, 0, subComponent.text );
            }
          }
        }
        context.painter()->restore();
        if ( maskPainter )
          maskPainter->restore();
        i++;
      }
      break;
    }
  }
}


//
// QgsTextRendererUtils
//

QgsTextBackgroundSettings::ShapeType QgsTextRendererUtils::decodeShapeType( const QString &string )
{
  QgsTextBackgroundSettings::ShapeType shpkind = QgsTextBackgroundSettings::ShapeRectangle;
  const QString skind = string.trimmed();

  if ( skind.compare( QLatin1String( "Square" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeSquare;
  }
  else if ( skind.compare( QLatin1String( "Ellipse" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeEllipse;
  }
  else if ( skind.compare( QLatin1String( "Circle" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeCircle;
  }
  else if ( skind.compare( QLatin1String( "SVG" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeSVG;
  }
  else if ( skind.compare( QLatin1String( "marker" ), Qt::CaseInsensitive ) == 0 )
  {
    shpkind = QgsTextBackgroundSettings::ShapeMarkerSymbol;
  }
  return shpkind;
}

QgsTextBackgroundSettings::SizeType QgsTextRendererUtils::decodeBackgroundSizeType( const QString &string )
{
  const QString stype = string.trimmed();
  // "Buffer"
  QgsTextBackgroundSettings::SizeType sizType = QgsTextBackgroundSettings::SizeBuffer;

  if ( stype.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
  {
    sizType = QgsTextBackgroundSettings::SizeFixed;
  }
  return sizType;
}

QgsTextBackgroundSettings::RotationType QgsTextRendererUtils::decodeBackgroundRotationType( const QString &string )
{
  const QString rotstr = string.trimmed();
  // "Sync"
  QgsTextBackgroundSettings::RotationType rottype = QgsTextBackgroundSettings::RotationSync;

  if ( rotstr.compare( QLatin1String( "Offset" ), Qt::CaseInsensitive ) == 0 )
  {
    rottype = QgsTextBackgroundSettings::RotationOffset;
  }
  else if ( rotstr.compare( QLatin1String( "Fixed" ), Qt::CaseInsensitive ) == 0 )
  {
    rottype = QgsTextBackgroundSettings::RotationFixed;
  }
  return rottype;
}

QgsTextShadowSettings::ShadowPlacement QgsTextRendererUtils::decodeShadowPlacementType( const QString &string )
{
  const QString str = string.trimmed();
  // "Lowest"
  QgsTextShadowSettings::ShadowPlacement shdwtype = QgsTextShadowSettings::ShadowLowest;

  if ( str.compare( QLatin1String( "Text" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowText;
  }
  else if ( str.compare( QLatin1String( "Buffer" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowBuffer;
  }
  else if ( str.compare( QLatin1String( "Background" ), Qt::CaseInsensitive ) == 0 )
  {
    shdwtype = QgsTextShadowSettings::ShadowShape;
  }
  return shdwtype;
}

QString QgsTextRendererUtils::encodeTextOrientation( QgsTextFormat::TextOrientation orientation )
{
  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
      return QStringLiteral( "horizontal" );
    case QgsTextFormat::VerticalOrientation:
      return QStringLiteral( "vertical" );
    case QgsTextFormat::RotationBasedOrientation:
      return QStringLiteral( "rotation-based" );
  }
  return QString();
}

QgsTextFormat::TextOrientation QgsTextRendererUtils::decodeTextOrientation( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;

  QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "horizontal" ) )
    return QgsTextFormat::HorizontalOrientation;
  else if ( cleaned == QLatin1String( "vertical" ) )
    return QgsTextFormat::VerticalOrientation;
  else if ( cleaned == QLatin1String( "rotation-based" ) )
    return QgsTextFormat::RotationBasedOrientation;

  if ( ok )
    *ok = false;
  return QgsTextFormat::HorizontalOrientation;
}
