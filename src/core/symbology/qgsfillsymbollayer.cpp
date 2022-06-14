/***************************************************************************
 qgsfillsymbollayer.cpp
 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfillsymbollayer.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsdxfexport.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgsimagecache.h"
#include "qgsrendercontext.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgslogger.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsunittypes.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgsimageoperation.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsexpressioncontextutils.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfeedback.h"
#include "qgsgeometryengine.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QDomElement>
#include <random>

#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

QgsSimpleFillSymbolLayer::QgsSimpleFillSymbolLayer( const QColor &color, Qt::BrushStyle style, const QColor &strokeColor, Qt::PenStyle strokeStyle, double strokeWidth,
    Qt::PenJoinStyle penJoinStyle )
  : mBrushStyle( style )
  , mStrokeColor( strokeColor )
  , mStrokeStyle( strokeStyle )
  , mStrokeWidth( strokeWidth )
  , mPenJoinStyle( penJoinStyle )
{
  mColor = color;
}

QgsSimpleFillSymbolLayer::~QgsSimpleFillSymbolLayer() = default;

void QgsSimpleFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mStrokeWidthUnit = unit;
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsSimpleFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = mStrokeWidthUnit;
  if ( mOffsetUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsSimpleFillSymbolLayer::usesMapUnits() const
{
  return mStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsSimpleFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mStrokeWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleFillSymbolLayer::mapUnitScale() const
{
  if ( mStrokeWidthMapUnitScale == mOffsetMapUnitScale )
  {
    return mStrokeWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSimpleFillSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext &context, QBrush &brush, QPen &pen, QPen &selPen )
{
  if ( !dataDefinedProperties().hasActiveProperties() )
    return; // shortcut

  bool ok;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QColor fillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
    fillColor.setAlphaF( context.opacity() * fillColor.alphaF() );
    brush.setColor( fillColor );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeBrushStyle( mBrushStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyFillStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
      brush.setStyle( QgsSymbolLayerUtils::decodeBrushStyle( exprVal.toString() ) );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    QColor penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
    penColor.setAlphaF( context.opacity() * penColor.alphaF() );
    pen.setColor( penColor );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      double width = exprVal.toDouble( &ok );
      if ( ok )
      {
        width = context.renderContext().convertToPainterUnits( width, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
        pen.setWidthF( width );
        selPen.setWidthF( width );
      }
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      pen.setStyle( QgsSymbolLayerUtils::decodePenStyle( style ) );
      selPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( style ) );
    }
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      pen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
      selPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( style ) );
    }
  }
}


QgsSymbolLayer *QgsSimpleFillSymbolLayer::create( const QVariantMap &props )
{
  QColor color = DEFAULT_SIMPLEFILL_COLOR;
  Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE;
  QColor strokeColor = DEFAULT_SIMPLEFILL_BORDERCOLOR;
  Qt::PenStyle strokeStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE;
  double strokeWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE;
  QPointF offset;

  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  if ( props.contains( QStringLiteral( "style" ) ) )
    style = QgsSymbolLayerUtils::decodeBrushStyle( props[QStringLiteral( "style" )].toString() );
  if ( props.contains( QStringLiteral( "color_border" ) ) )
  {
    //pre 2.5 projects used "color_border"
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color_border" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )].toString() );
  }

  if ( props.contains( QStringLiteral( "style_border" ) ) )
  {
    //pre 2.5 projects used "style_border"
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "style_border" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )].toString() );
  }
  if ( props.contains( QStringLiteral( "width_border" ) ) )
  {
    //pre 2.5 projects used "width_border"
    strokeWidth = props[QStringLiteral( "width_border" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "outline_width" ) ) )
  {
    strokeWidth = props[QStringLiteral( "outline_width" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "line_width" ) ) )
  {
    strokeWidth = props[QStringLiteral( "line_width" )].toDouble();
  }
  if ( props.contains( QStringLiteral( "offset" ) ) )
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    penJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )].toString() );

  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = std::make_unique< QgsSimpleFillSymbolLayer >( color, style, strokeColor, strokeStyle, strokeWidth, penJoinStyle );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "border_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "border_width_unit" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  else if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );

  if ( props.contains( QStringLiteral( "border_width_map_unit_scale" ) ) )
    sl->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "border_width_map_unit_scale" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );

  sl->restoreOldDataDefinedProperties( props );

  return sl.release();
}


QString QgsSimpleFillSymbolLayer::layerType() const
{
  return QStringLiteral( "SimpleFill" );
}

void QgsSimpleFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QColor fillColor = mColor;
  fillColor.setAlphaF( context.opacity() * mColor.alphaF() );
  mBrush = QBrush( fillColor, mBrushStyle );

  QColor selColor = context.renderContext().selectionColor();
  QColor selPenColor = selColor == mColor ? selColor : mStrokeColor;
  if ( ! SELECTION_IS_OPAQUE )
    selColor.setAlphaF( context.opacity() );
  mSelBrush = QBrush( selColor );
  // N.B. unless a "selection line color" is implemented in addition to the "selection color" option
  // this would mean symbols with "no fill" look the same whether or not they are selected
  if ( SELECT_FILL_STYLE )
    mSelBrush.setStyle( mBrushStyle );

  QColor strokeColor = mStrokeColor;
  strokeColor.setAlphaF( context.opacity() * mStrokeColor.alphaF() );
  mPen = QPen( strokeColor );
  mSelPen = QPen( selPenColor );
  mPen.setStyle( mStrokeStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
  mPen.setJoinStyle( mPenJoinStyle );
}

void QgsSimpleFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsSimpleFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QColor fillColor = mColor;
  fillColor.setAlphaF( context.opacity() * mColor.alphaF() );
  mBrush.setColor( fillColor );
  QColor strokeColor = mStrokeColor;
  strokeColor.setAlphaF( context.opacity() * mStrokeColor.alphaF() );
  mPen.setColor( strokeColor );

  applyDataDefinedSymbology( context, mBrush, mPen, mSelPen );

  QPointF offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
    if ( ok )
      offset = res;
  }

  if ( !offset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( offset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( offset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

#ifndef QT_NO_PRINTER
  if ( mBrush.style() == Qt::SolidPattern || mBrush.style() == Qt::NoBrush || !dynamic_cast<QPrinter *>( p->device() ) )
#endif
  {
    p->setPen( context.selected() ? mSelPen : mPen );
    p->setBrush( context.selected() ? mSelBrush : mBrush );
    _renderPolygon( p, points, rings, context );
  }
#ifndef QT_NO_PRINTER
  else
  {
    // workaround upstream issue https://github.com/qgis/QGIS/issues/36580
    // when a non-solid brush is set with opacity, the opacity incorrectly applies to the pen
    // when exporting to PDF/print devices
    p->setBrush( context.selected() ? mSelBrush : mBrush );
    p->setPen( Qt::NoPen );
    _renderPolygon( p, points, rings, context );

    p->setPen( context.selected() ? mSelPen : mPen );
    p->setBrush( Qt::NoBrush );
    _renderPolygon( p, points, rings, context );
  }
#endif

  if ( !offset.isNull() )
  {
    p->translate( -offset );
  }
}

QVariantMap QgsSimpleFillSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "style" )] = QgsSymbolLayerUtils::encodeBrushStyle( mBrushStyle );
  map[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mStrokeColor );
  map[QStringLiteral( "outline_style" )] = QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle );
  map[QStringLiteral( "outline_width" )] = QString::number( mStrokeWidth );
  map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  map[QStringLiteral( "border_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  return map;
}

QgsSimpleFillSymbolLayer *QgsSimpleFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = std::make_unique< QgsSimpleFillSymbolLayer >( mColor, mBrushStyle, mStrokeColor, mStrokeStyle, mStrokeWidth, mPenJoinStyle );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  sl->setStrokeWidthUnit( mStrokeWidthUnit );
  sl->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  copyDataDefinedProperties( sl.get() );
  copyPaintEffect( sl.get() );
  return sl.release();
}

void QgsSimpleFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  if ( mBrushStyle == Qt::NoBrush && mStrokeStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  if ( mBrushStyle != Qt::NoBrush )
  {
    // <Fill>
    QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
    symbolizerElem.appendChild( fillElem );
    QgsSymbolLayerUtils::fillToSld( doc, fillElem, mBrushStyle, mColor );
  }

  if ( mStrokeStyle != Qt::NoPen )
  {
    // <Stroke>
    QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
    symbolizerElem.appendChild( strokeElem );
    double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
    QgsSymbolLayerUtils::lineToSld( doc, strokeElem, mStrokeStyle, mStrokeColor, strokeWidth, &mPenJoinStyle );
  }

  // <se:Displacement>
  QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, symbolizerElem, offset );
}

QString QgsSimpleFillSymbolLayer::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  //brush
  QString symbolStyle;
  symbolStyle.append( QgsSymbolLayerUtils::ogrFeatureStyleBrush( mColor ) );
  symbolStyle.append( ';' );
  //pen
  symbolStyle.append( QgsSymbolLayerUtils::ogrFeatureStylePen( mStrokeWidth, mmScaleFactor, mapUnitScaleFactor, mStrokeColor, mPenJoinStyle ) );
  return symbolStyle;
}

QgsSymbolLayer *QgsSimpleFillSymbolLayer::createFromSld( QDomElement &element )
{
  QColor color, strokeColor;
  Qt::BrushStyle fillStyle;
  Qt::PenStyle strokeStyle;
  double strokeWidth;

  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  QgsSymbolLayerUtils::fillFromSld( fillElem, fillStyle, color );

  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  QgsSymbolLayerUtils::lineFromSld( strokeElem, strokeStyle, strokeColor, strokeWidth );

  QPointF offset;
  QgsSymbolLayerUtils::displacementFromSldElement( element, offset );

  double scaleFactor = 1.0;
  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  QgsUnitTypes::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  offset.setX( offset.x() * scaleFactor );
  offset.setY( offset.y() * scaleFactor );
  strokeWidth = strokeWidth * scaleFactor;

  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = std::make_unique< QgsSimpleFillSymbolLayer >( color, fillStyle, strokeColor, strokeStyle, strokeWidth );
  sl->setOutputUnit( sldUnitSize );
  sl->setOffset( offset );
  return sl.release();
}

double QgsSimpleFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  double penBleed = context.convertToPainterUnits( mStrokeStyle == Qt::NoPen ? 0 : ( mStrokeWidth / 2.0 ), mStrokeWidthUnit, mStrokeWidthMapUnitScale );
  double offsetBleed = context.convertToPainterUnits( std::max( std::fabs( mOffset.x() ), std::fabs( mOffset.y() ) ), mOffsetUnit, mOffsetMapUnitScale );
  return penBleed + offsetBleed;
}

double QgsSimpleFillSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double width = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  return width * QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}

QColor QgsSimpleFillSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  QColor c = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), c );
  }
  return c;
}

double QgsSimpleFillSymbolLayer::dxfAngle( QgsSymbolRenderContext &context ) const
{
  double angle = mAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle );
  }
  return angle;
}

Qt::PenStyle QgsSimpleFillSymbolLayer::dxfPenStyle() const
{
  return mStrokeStyle;
}

QColor QgsSimpleFillSymbolLayer::dxfBrushColor( QgsSymbolRenderContext &context ) const
{
  QColor c = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    c = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), c );
  }
  return c;
}

Qt::BrushStyle QgsSimpleFillSymbolLayer::dxfBrushStyle() const
{
  return mBrushStyle;
}

//QgsGradientFillSymbolLayer

QgsGradientFillSymbolLayer::QgsGradientFillSymbolLayer( const QColor &color, const QColor &color2,
    Qgis::GradientColorSource colorType, Qgis::GradientType gradientType,
    Qgis::SymbolCoordinateReference coordinateMode, Qgis::GradientSpread spread )
  : mGradientColorType( colorType )
  , mGradientType( gradientType )
  , mCoordinateMode( coordinateMode )
  , mGradientSpread( spread )
  , mReferencePoint1( QPointF( 0.5, 0 ) )
  , mReferencePoint2( QPointF( 0.5, 1 ) )
{
  mColor = color;
  mColor2 = color2;
}

QgsGradientFillSymbolLayer::~QgsGradientFillSymbolLayer()
{
  delete mGradientRamp;
}

QgsSymbolLayer *QgsGradientFillSymbolLayer::create( const QVariantMap &props )
{
  //default to a two-color, linear gradient with feature mode and pad spreading
  Qgis::GradientType type = Qgis::GradientType::Linear;
  Qgis::GradientColorSource colorType = Qgis::GradientColorSource::SimpleTwoColor;
  Qgis::SymbolCoordinateReference coordinateMode = Qgis::SymbolCoordinateReference::Feature;
  Qgis::GradientSpread gradientSpread = Qgis::GradientSpread::Pad;
  //default to gradient from the default fill color to white
  QColor color = DEFAULT_SIMPLEFILL_COLOR, color2 = Qt::white;
  QPointF referencePoint1 = QPointF( 0.5, 0 );
  bool refPoint1IsCentroid = false;
  QPointF referencePoint2 = QPointF( 0.5, 1 );
  bool refPoint2IsCentroid = false;
  double angle = 0;
  QPointF offset;

  //update gradient properties from props
  if ( props.contains( QStringLiteral( "type" ) ) )
    type = static_cast< Qgis::GradientType >( props[QStringLiteral( "type" )].toInt() );
  if ( props.contains( QStringLiteral( "coordinate_mode" ) ) )
    coordinateMode = static_cast< Qgis::SymbolCoordinateReference >( props[QStringLiteral( "coordinate_mode" )].toInt() );
  if ( props.contains( QStringLiteral( "spread" ) ) )
    gradientSpread = static_cast< Qgis::GradientSpread >( props[QStringLiteral( "spread" )].toInt() );
  if ( props.contains( QStringLiteral( "color_type" ) ) )
    colorType = static_cast< Qgis::GradientColorSource >( props[QStringLiteral( "color_type" )].toInt() );
  if ( props.contains( QStringLiteral( "gradient_color" ) ) )
  {
    //pre 2.5 projects used "gradient_color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  }
  if ( props.contains( QStringLiteral( "gradient_color2" ) ) )
  {
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color2" )].toString() );
  }

  if ( props.contains( QStringLiteral( "reference_point1" ) ) )
    referencePoint1 = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "reference_point1" )].toString() );
  if ( props.contains( QStringLiteral( "reference_point1_iscentroid" ) ) )
    refPoint1IsCentroid = props[QStringLiteral( "reference_point1_iscentroid" )].toInt();
  if ( props.contains( QStringLiteral( "reference_point2" ) ) )
    referencePoint2 = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "reference_point2" )].toString() );
  if ( props.contains( QStringLiteral( "reference_point2_iscentroid" ) ) )
    refPoint2IsCentroid = props[QStringLiteral( "reference_point2_iscentroid" )].toInt();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();

  if ( props.contains( QStringLiteral( "offset" ) ) )
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() );

  //attempt to create color ramp from props
  QgsColorRamp *gradientRamp = nullptr;
  if ( props.contains( QStringLiteral( "rampType" ) ) && props[QStringLiteral( "rampType" )] == QgsCptCityColorRamp::typeString() )
  {
    gradientRamp = QgsCptCityColorRamp::create( props );
  }
  else
  {
    gradientRamp = QgsGradientColorRamp::create( props );
  }

  //create a new gradient fill layer with desired properties
  std::unique_ptr< QgsGradientFillSymbolLayer > sl = std::make_unique< QgsGradientFillSymbolLayer >( color, color2, colorType, type, coordinateMode, gradientSpread );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  sl->setReferencePoint1( referencePoint1 );
  sl->setReferencePoint1IsCentroid( refPoint1IsCentroid );
  sl->setReferencePoint2( referencePoint2 );
  sl->setReferencePoint2IsCentroid( refPoint2IsCentroid );
  sl->setAngle( angle );
  if ( gradientRamp )
    sl->setColorRamp( gradientRamp );

  sl->restoreOldDataDefinedProperties( props );

  return sl.release();
}

void QgsGradientFillSymbolLayer::setColorRamp( QgsColorRamp *ramp )
{
  delete mGradientRamp;
  mGradientRamp = ramp;
}

QString QgsGradientFillSymbolLayer::layerType() const
{
  return QStringLiteral( "GradientFill" );
}

void QgsGradientFillSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext &context, const QPolygonF &points )
{
  if ( !dataDefinedProperties().hasActiveProperties() && !mReferencePoint1IsCentroid && !mReferencePoint2IsCentroid )
  {
    //shortcut
    applyGradient( context, mBrush, mColor, mColor2,  mGradientColorType, mGradientRamp, mGradientType, mCoordinateMode,
                   mGradientSpread, mReferencePoint1, mReferencePoint2, mAngle );
    return;
  }

  bool ok;

  //first gradient color
  QColor color = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    color = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
    color.setAlphaF( context.opacity() * color.alphaF() );
  }

  //second gradient color
  QColor color2 = mColor2;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySecondaryColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor2 ) );
    color2 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertySecondaryColor, context.renderContext().expressionContext(), mColor2 );
    color2.setAlphaF( context.opacity() * color2.alphaF() );
  }

  //gradient rotation angle
  double angle = mAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle );
  }

  //gradient type
  Qgis::GradientType gradientType = mGradientType;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientType ) )
  {
    QString currentType = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyGradientType, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentType == QObject::tr( "linear" ) )
      {
        gradientType = Qgis::GradientType::Linear;
      }
      else if ( currentType == QObject::tr( "radial" ) )
      {
        gradientType = Qgis::GradientType::Radial;
      }
      else if ( currentType == QObject::tr( "conical" ) )
      {
        gradientType = Qgis::GradientType::Conical;
      }
    }
  }

  //coordinate mode
  Qgis::SymbolCoordinateReference coordinateMode = mCoordinateMode;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCoordinateMode ) )
  {
    QString currentCoordMode = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCoordinateMode, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentCoordMode == QObject::tr( "feature" ) )
      {
        coordinateMode = Qgis::SymbolCoordinateReference::Feature;
      }
      else if ( currentCoordMode == QObject::tr( "viewport" ) )
      {
        coordinateMode = Qgis::SymbolCoordinateReference::Viewport;
      }
    }
  }

  //gradient spread
  Qgis::GradientSpread spread = mGradientSpread;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientSpread ) )
  {
    QString currentSpread = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyGradientSpread, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentSpread == QObject::tr( "pad" ) )
      {
        spread = Qgis::GradientSpread::Pad;
      }
      else if ( currentSpread == QObject::tr( "repeat" ) )
      {
        spread = Qgis::GradientSpread::Repeat;
      }
      else if ( currentSpread == QObject::tr( "reflect" ) )
      {
        spread = Qgis::GradientSpread::Reflect;
      }
    }
  }

  //reference point 1 x & y
  double refPoint1X = mReferencePoint1.x();
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference1X ) )
  {
    context.setOriginalValueVariable( refPoint1X );
    refPoint1X = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyGradientReference1X, context.renderContext().expressionContext(), refPoint1X );
  }
  double refPoint1Y = mReferencePoint1.y();
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference1Y ) )
  {
    context.setOriginalValueVariable( refPoint1Y );
    refPoint1Y = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyGradientReference1Y, context.renderContext().expressionContext(), refPoint1Y );
  }
  bool refPoint1IsCentroid = mReferencePoint1IsCentroid;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference1IsCentroid ) )
  {
    context.setOriginalValueVariable( refPoint1IsCentroid );
    refPoint1IsCentroid = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::PropertyGradientReference1IsCentroid, context.renderContext().expressionContext(), refPoint1IsCentroid );
  }

  //reference point 2 x & y
  double refPoint2X = mReferencePoint2.x();
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference2X ) )
  {
    context.setOriginalValueVariable( refPoint2X );
    refPoint2X = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyGradientReference2X, context.renderContext().expressionContext(), refPoint2X );
  }
  double refPoint2Y = mReferencePoint2.y();
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference2Y ) )
  {
    context.setOriginalValueVariable( refPoint2Y );
    refPoint2Y = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyGradientReference2Y, context.renderContext().expressionContext(), refPoint2Y );
  }
  bool refPoint2IsCentroid = mReferencePoint2IsCentroid;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientReference2IsCentroid ) )
  {
    context.setOriginalValueVariable( refPoint2IsCentroid );
    refPoint2IsCentroid = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::PropertyGradientReference2IsCentroid, context.renderContext().expressionContext(), refPoint2IsCentroid );
  }

  if ( refPoint1IsCentroid || refPoint2IsCentroid )
  {
    //either the gradient is starting or ending at a centroid, so calculate it
    QPointF centroid = QgsSymbolLayerUtils::polygonCentroid( points );
    //centroid coordinates need to be scaled to a range [0, 1] relative to polygon bounds
    QRectF bbox = points.boundingRect();
    double centroidX = ( centroid.x() - bbox.left() ) / bbox.width();
    double centroidY = ( centroid.y() - bbox.top() ) / bbox.height();

    if ( refPoint1IsCentroid )
    {
      refPoint1X = centroidX;
      refPoint1Y = centroidY;
    }
    if ( refPoint2IsCentroid )
    {
      refPoint2X = centroidX;
      refPoint2Y = centroidY;
    }
  }

  //update gradient with data defined values
  applyGradient( context, mBrush, color, color2,  mGradientColorType, mGradientRamp, gradientType, coordinateMode,
                 spread, QPointF( refPoint1X, refPoint1Y ), QPointF( refPoint2X, refPoint2Y ), angle );
}

QPointF QgsGradientFillSymbolLayer::rotateReferencePoint( QPointF refPoint, double angle )
{
  //rotate a reference point by a specified angle around the point (0.5, 0.5)

  //create a line from the centrepoint of a rectangle bounded by (0, 0) and (1, 1) to the reference point
  QLineF refLine = QLineF( QPointF( 0.5, 0.5 ), refPoint );
  //rotate this line by the current rotation angle
  refLine.setAngle( refLine.angle() + angle );
  //get new end point of line
  QPointF rotatedReferencePoint = refLine.p2();
  //make sure coords of new end point is within [0, 1]
  if ( rotatedReferencePoint.x() > 1 )
    rotatedReferencePoint.setX( 1 );
  if ( rotatedReferencePoint.x() < 0 )
    rotatedReferencePoint.setX( 0 );
  if ( rotatedReferencePoint.y() > 1 )
    rotatedReferencePoint.setY( 1 );
  if ( rotatedReferencePoint.y() < 0 )
    rotatedReferencePoint.setY( 0 );

  return rotatedReferencePoint;
}

void QgsGradientFillSymbolLayer::applyGradient( const QgsSymbolRenderContext &context, QBrush &brush,
    const QColor &color, const QColor &color2, Qgis::GradientColorSource gradientColorType,
    QgsColorRamp *gradientRamp, Qgis::GradientType gradientType,
    Qgis::SymbolCoordinateReference coordinateMode, Qgis::GradientSpread gradientSpread,
    QPointF referencePoint1, QPointF referencePoint2, const double angle )
{
  //update alpha of gradient colors
  QColor fillColor = color;
  fillColor.setAlphaF( context.opacity() * fillColor.alphaF() );
  QColor fillColor2 = color2;
  fillColor2.setAlphaF( context.opacity() * fillColor2.alphaF() );

  //rotate reference points
  QPointF rotatedReferencePoint1 = !qgsDoubleNear( angle, 0.0 ) ? rotateReferencePoint( referencePoint1, angle ) : referencePoint1;
  QPointF rotatedReferencePoint2 = !qgsDoubleNear( angle, 0.0 ) ? rotateReferencePoint( referencePoint2, angle ) : referencePoint2;

  //create a QGradient with the desired properties
  QGradient gradient;
  switch ( gradientType )
  {
    case Qgis::GradientType::Linear:
      gradient = QLinearGradient( rotatedReferencePoint1, rotatedReferencePoint2 );
      break;
    case Qgis::GradientType::Radial:
      gradient = QRadialGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).length() );
      break;
    case Qgis::GradientType::Conical:
      gradient = QConicalGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).angle() );
      break;
  }
  switch ( coordinateMode )
  {
    case Qgis::SymbolCoordinateReference::Feature:
      gradient.setCoordinateMode( QGradient::ObjectBoundingMode );
      break;
    case Qgis::SymbolCoordinateReference::Viewport:
      gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
      break;
  }
  switch ( gradientSpread )
  {
    case Qgis::GradientSpread::Pad:
      gradient.setSpread( QGradient::PadSpread );
      break;
    case Qgis::GradientSpread::Reflect:
      gradient.setSpread( QGradient::ReflectSpread );
      break;
    case Qgis::GradientSpread::Repeat:
      gradient.setSpread( QGradient::RepeatSpread );
      break;
  }

  //add stops to gradient
  if ( gradientColorType == Qgis::GradientColorSource::ColorRamp && gradientRamp &&
       ( gradientRamp->type() == QgsGradientColorRamp::typeString() || gradientRamp->type() == QgsCptCityColorRamp::typeString() ) )
  {
    //color ramp gradient
    QgsGradientColorRamp *gradRamp = static_cast<QgsGradientColorRamp *>( gradientRamp );
    gradRamp->addStopsToGradient( &gradient, context.opacity() );
  }
  else
  {
    //two color gradient
    gradient.setColorAt( 0.0, fillColor );
    gradient.setColorAt( 1.0, fillColor2 );
  }

  //update QBrush use gradient
  brush = QBrush( gradient );
}

void QgsGradientFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QColor selColor = context.renderContext().selectionColor();
  if ( ! SELECTION_IS_OPAQUE )
    selColor.setAlphaF( context.opacity() );
  mSelBrush = QBrush( selColor );
}

void QgsGradientFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsGradientFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  applyDataDefinedSymbology( context, points );

  p->setBrush( context.selected() ? mSelBrush : mBrush );
  p->setPen( Qt::NoPen );

  QPointF offset = mOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
    if ( ok )
      offset = res;
  }

  if ( !offset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( offset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( offset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  _renderPolygon( p, points, rings, context );

  if ( !offset.isNull() )
  {
    p->translate( -offset );
  }
}

QVariantMap QgsGradientFillSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "gradient_color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  map[QStringLiteral( "color_type" )] = QString::number( static_cast< int >( mGradientColorType ) );
  map[QStringLiteral( "type" )] = QString::number( static_cast<int>( mGradientType ) );
  map[QStringLiteral( "coordinate_mode" )] = QString::number( static_cast< int >( mCoordinateMode ) );
  map[QStringLiteral( "spread" )] = QString::number( static_cast< int >( mGradientSpread ) );
  map[QStringLiteral( "reference_point1" )] = QgsSymbolLayerUtils::encodePoint( mReferencePoint1 );
  map[QStringLiteral( "reference_point1_iscentroid" )] = QString::number( mReferencePoint1IsCentroid );
  map[QStringLiteral( "reference_point2" )] = QgsSymbolLayerUtils::encodePoint( mReferencePoint2 );
  map[QStringLiteral( "reference_point2_iscentroid" )] = QString::number( mReferencePoint2IsCentroid );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  if ( mGradientRamp )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    map.unite( mGradientRamp->properties() );
#else
    map.insert( mGradientRamp->properties() );
#endif
  }
  return map;
}

QgsGradientFillSymbolLayer *QgsGradientFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsGradientFillSymbolLayer > sl = std::make_unique< QgsGradientFillSymbolLayer >( mColor, mColor2, mGradientColorType, mGradientType, mCoordinateMode, mGradientSpread );
  if ( mGradientRamp )
    sl->setColorRamp( mGradientRamp->clone() );
  sl->setReferencePoint1( mReferencePoint1 );
  sl->setReferencePoint1IsCentroid( mReferencePoint1IsCentroid );
  sl->setReferencePoint2( mReferencePoint2 );
  sl->setReferencePoint2IsCentroid( mReferencePoint2IsCentroid );
  sl->setAngle( mAngle );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  copyDataDefinedProperties( sl.get() );
  copyPaintEffect( sl.get() );
  return sl.release();
}

double QgsGradientFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  double offsetBleed = context.convertToPainterUnits( std::max( std::fabs( mOffset.x() ), std::fabs( mOffset.y() ) ), mOffsetUnit, mOffsetMapUnitScale );
  return offsetBleed;
}

bool QgsGradientFillSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}

void QgsGradientFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsGradientFillSymbolLayer::outputUnit() const
{
  return mOffsetUnit;
}

bool QgsGradientFillSymbolLayer::usesMapUnits() const
{
  return mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsGradientFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsGradientFillSymbolLayer::mapUnitScale() const
{
  return mOffsetMapUnitScale;
}

//QgsShapeburstFillSymbolLayer

QgsShapeburstFillSymbolLayer::QgsShapeburstFillSymbolLayer( const QColor &color, const QColor &color2, Qgis::GradientColorSource colorType,
    int blurRadius, bool useWholeShape, double maxDistance )
  : mBlurRadius( blurRadius )
  , mUseWholeShape( useWholeShape )
  , mMaxDistance( maxDistance )
  , mColorType( colorType )
  , mColor2( color2 )
{
  mColor = color;
}

QgsShapeburstFillSymbolLayer::~QgsShapeburstFillSymbolLayer() = default;

QgsSymbolLayer *QgsShapeburstFillSymbolLayer::create( const QVariantMap &props )
{
  //default to a two-color gradient
  Qgis::GradientColorSource colorType = Qgis::GradientColorSource::SimpleTwoColor;
  QColor color = DEFAULT_SIMPLEFILL_COLOR, color2 = Qt::white;
  int blurRadius = 0;
  bool useWholeShape = true;
  double maxDistance = 5;
  QPointF offset;

  //update fill properties from props
  if ( props.contains( QStringLiteral( "color_type" ) ) )
  {
    colorType = static_cast< Qgis::GradientColorSource >( props[QStringLiteral( "color_type" )].toInt() );
  }
  if ( props.contains( QStringLiteral( "shapeburst_color" ) ) )
  {
    //pre 2.5 projects used "shapeburst_color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "shapeburst_color" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )].toString() );
  }

  if ( props.contains( QStringLiteral( "shapeburst_color2" ) ) )
  {
    //pre 2.5 projects used "shapeburst_color2"
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "shapeburst_color2" )].toString() );
  }
  else if ( props.contains( QStringLiteral( "gradient_color2" ) ) )
  {
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color2" )].toString() );
  }
  if ( props.contains( QStringLiteral( "blur_radius" ) ) )
  {
    blurRadius = props[QStringLiteral( "blur_radius" )].toInt();
  }
  if ( props.contains( QStringLiteral( "use_whole_shape" ) ) )
  {
    useWholeShape = props[QStringLiteral( "use_whole_shape" )].toInt();
  }
  if ( props.contains( QStringLiteral( "max_distance" ) ) )
  {
    maxDistance = props[QStringLiteral( "max_distance" )].toDouble();
  }
  if ( props.contains( QStringLiteral( "offset" ) ) )
  {
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )].toString() );
  }

  //attempt to create color ramp from props
  QgsColorRamp *gradientRamp = nullptr;
  if ( props.contains( QStringLiteral( "rampType" ) ) && props[QStringLiteral( "rampType" )] == QgsCptCityColorRamp::typeString() )
  {
    gradientRamp = QgsCptCityColorRamp::create( props );
  }
  else
  {
    gradientRamp = QgsGradientColorRamp::create( props );
  }

  //create a new shapeburst fill layer with desired properties
  std::unique_ptr< QgsShapeburstFillSymbolLayer > sl = std::make_unique< QgsShapeburstFillSymbolLayer >( color, color2, colorType, blurRadius, useWholeShape, maxDistance );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
  {
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "distance_unit" ) ) )
  {
    sl->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "distance_unit" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    sl->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "distance_map_unit_scale" )].toString() ) );
  }
  if ( props.contains( QStringLiteral( "ignore_rings" ) ) )
  {
    sl->setIgnoreRings( props[QStringLiteral( "ignore_rings" )].toInt() );
  }
  if ( gradientRamp )
  {
    sl->setColorRamp( gradientRamp );
  }

  sl->restoreOldDataDefinedProperties( props );

  return sl.release();
}

QString QgsShapeburstFillSymbolLayer::layerType() const
{
  return QStringLiteral( "ShapeburstFill" );
}

void QgsShapeburstFillSymbolLayer::setColorRamp( QgsColorRamp *ramp )
{
  if ( mGradientRamp.get() == ramp )
    return;

  mGradientRamp.reset( ramp );
}

void QgsShapeburstFillSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext &context, QColor &color, QColor &color2, int &blurRadius, bool &useWholeShape,
    double &maxDistance, bool &ignoreRings )
{
  //first gradient color
  color = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    color = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  //second gradient color
  color2 = mColor2;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySecondaryColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor2 ) );
    color2 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertySecondaryColor, context.renderContext().expressionContext(), mColor2 );
  }

  //blur radius
  blurRadius = mBlurRadius;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyBlurRadius ) )
  {
    context.setOriginalValueVariable( mBlurRadius );
    blurRadius = mDataDefinedProperties.valueAsInt( QgsSymbolLayer::PropertyBlurRadius, context.renderContext().expressionContext(), mBlurRadius );
  }

  //use whole shape
  useWholeShape = mUseWholeShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyShapeburstUseWholeShape ) )
  {
    context.setOriginalValueVariable( mUseWholeShape );
    useWholeShape = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::PropertyShapeburstUseWholeShape, context.renderContext().expressionContext(), mUseWholeShape );
  }

  //max distance
  maxDistance = mMaxDistance;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyShapeburstMaxDistance ) )
  {
    context.setOriginalValueVariable( mMaxDistance );
    maxDistance = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyShapeburstMaxDistance, context.renderContext().expressionContext(), mMaxDistance );
  }

  //ignore rings
  ignoreRings = mIgnoreRings;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyShapeburstIgnoreRings ) )
  {
    context.setOriginalValueVariable( mIgnoreRings );
    ignoreRings = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::PropertyShapeburstIgnoreRings, context.renderContext().expressionContext(), mIgnoreRings );
  }

}

void QgsShapeburstFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  //TODO - check this
  QColor selColor = context.renderContext().selectionColor();
  if ( ! SELECTION_IS_OPAQUE )
    selColor.setAlphaF( context.opacity() );
  mSelBrush = QBrush( selColor );
}

void QgsShapeburstFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

void QgsShapeburstFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  if ( context.selected() )
  {
    //feature is selected, draw using selection style
    p->setBrush( mSelBrush );
    QPointF offset = mOffset;

    if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
    {
      context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
      const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
      bool ok = false;
      const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
      if ( ok )
        offset = res;
    }

    if ( !offset.isNull() )
    {
      offset.setX( context.renderContext().convertToPainterUnits( offset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
      offset.setY( context.renderContext().convertToPainterUnits( offset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
      p->translate( offset );
    }
    _renderPolygon( p, points, rings, context );
    if ( !offset.isNull() )
    {
      p->translate( -offset );
    }
    return;
  }

  QColor color1, color2;
  int blurRadius;
  bool useWholeShape;
  double maxDistance;
  bool ignoreRings;
  //calculate data defined symbology
  applyDataDefinedSymbology( context, color1, color2, blurRadius, useWholeShape, maxDistance, ignoreRings );

  //calculate max distance for shapeburst fill to extend from polygon boundary, in pixels
  int outputPixelMaxDist = 0;
  if ( !useWholeShape && !qgsDoubleNear( maxDistance, 0.0 ) )
  {
    //convert max distance to pixels
    outputPixelMaxDist = static_cast< int >( std::round( context.renderContext().convertToPainterUnits( maxDistance, mDistanceUnit, mDistanceMapUnitScale ) ) );
  }

  //if we are using the two color mode, create a gradient ramp
  std::unique_ptr< QgsGradientColorRamp > twoColorGradientRamp;
  if ( mColorType == Qgis::GradientColorSource::SimpleTwoColor )
  {
    twoColorGradientRamp = std::make_unique< QgsGradientColorRamp >( color1, color2 );
  }

  //no stroke for shapeburst fills
  p->setPen( QPen( Qt::NoPen ) );

  //calculate margin size in pixels so that QImage of polygon has sufficient space to draw the full blur effect
  int sideBuffer = 4 + ( blurRadius + 2 ) * 4;
  //create a QImage to draw shapeburst in
  int pointsWidth = static_cast< int >( std::round( points.boundingRect().width() ) );
  int pointsHeight = static_cast< int >( std::round( points.boundingRect().height() ) );
  int imWidth = pointsWidth + ( sideBuffer * 2 );
  int imHeight = pointsHeight + ( sideBuffer * 2 );

  // these are all potentially very expensive operations, so check regularly if the job is canceled and abort responsively
  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  std::unique_ptr< QImage > fillImage = std::make_unique< QImage >( imWidth,
                                        imHeight, QImage::Format_ARGB32_Premultiplied );
  if ( fillImage->isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not allocate sufficient memory for shapeburst fill" ) );
    return;
  }

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //also create an image to store the alpha channel
  std::unique_ptr< QImage > alphaImage = std::make_unique< QImage >( fillImage->width(), fillImage->height(), QImage::Format_ARGB32_Premultiplied );
  if ( alphaImage->isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not allocate sufficient memory for shapeburst fill" ) );
    return;
  }

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //Fill this image with black. Initially the distance transform is drawn in greyscale, where black pixels have zero distance from the
  //polygon boundary. Since we don't care about pixels which fall outside the polygon, we start with a black image and then draw over it the
  //polygon in white. The distance transform function then fills in the correct distance values for the white pixels.
  fillImage->fill( Qt::black );

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //initially fill the alpha channel image with a transparent color
  alphaImage->fill( Qt::transparent );

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //now, draw the polygon in the alpha channel image
  QPainter imgPainter;
  imgPainter.begin( alphaImage.get() );
  imgPainter.setRenderHint( QPainter::Antialiasing, true );
  imgPainter.setBrush( QBrush( Qt::white ) );
  imgPainter.setPen( QPen( Qt::black ) );
  imgPainter.translate( -points.boundingRect().left() + sideBuffer, - points.boundingRect().top() + sideBuffer );
  _renderPolygon( &imgPainter, points, rings, context );
  imgPainter.end();

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //now that we have a render of the polygon in white, draw this onto the shapeburst fill image too
  //(this avoids calling _renderPolygon twice, since that can be slow)
  imgPainter.begin( fillImage.get() );
  if ( !ignoreRings )
  {
    imgPainter.drawImage( 0, 0, *alphaImage );
  }
  else
  {
    //using ignore rings mode, so the alpha image can't be used
    //directly as the alpha channel contains polygon rings and we need
    //to draw now without any rings
    imgPainter.setBrush( QBrush( Qt::white ) );
    imgPainter.setPen( QPen( Qt::black ) );
    imgPainter.translate( -points.boundingRect().left() + sideBuffer, - points.boundingRect().top() + sideBuffer );
    _renderPolygon( &imgPainter, points, nullptr, context );
  }
  imgPainter.end();

  if ( context.renderContext().feedback() && context.renderContext().feedback()->isCanceled() )
    return;

  //apply distance transform to image, uses the current color ramp to calculate final pixel colors
  double *dtArray = distanceTransform( fillImage.get(), context.renderContext() );

  //copy distance transform values back to QImage, shading by appropriate color ramp
  dtArrayToQImage( dtArray, fillImage.get(), mColorType == Qgis::GradientColorSource::SimpleTwoColor ? twoColorGradientRamp.get() : mGradientRamp.get(),
                   context.renderContext(), useWholeShape, outputPixelMaxDist );
  if ( context.opacity() < 1 )
  {
    QgsImageOperation::multiplyOpacity( *fillImage, context.opacity(), context.renderContext().feedback() );
  }

  //clean up some variables
  delete [] dtArray;

  //apply blur if desired
  if ( blurRadius > 0 )
  {
    QgsImageOperation::stackBlur( *fillImage, blurRadius, false, context.renderContext().feedback() );
  }

  //apply alpha channel to distance transform image, so that areas outside the polygon are transparent
  imgPainter.begin( fillImage.get() );
  imgPainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
  imgPainter.drawImage( 0, 0, *alphaImage );
  imgPainter.end();
  //we're finished with the alpha channel image now
  alphaImage.reset();

  //draw shapeburst image in correct place in the destination painter

  QgsScopedQPainterState painterState( p );
  QPointF offset = mOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
    if ( ok )
      offset = res;
  }
  if ( !offset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( offset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( offset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  p->drawImage( points.boundingRect().left() - sideBuffer, points.boundingRect().top() - sideBuffer, *fillImage );

  if ( !offset.isNull() )
  {
    p->translate( -offset );
  }
}

//fast distance transform code, adapted from http://cs.brown.edu/~pff/dt/

/* distance transform of a 1d function using squared distance */
void QgsShapeburstFillSymbolLayer::distanceTransform1d( double *f, int n, int *v, double *z, double *d )
{
  int k = 0;
  v[0] = 0;
  z[0] = -INF;
  z[1] = + INF;
  for ( int q = 1; q <= n - 1; q++ )
  {
    double s  = ( ( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    while ( s <= z[k] )
    {
      k--;
      s  = ( ( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    }
    k++;
    v[k] = q;
    z[k] = s;
    z[k + 1] = + INF;
  }

  k = 0;
  for ( int q = 0; q <= n - 1; q++ )
  {
    while ( z[k + 1] < q )
      k++;
    d[q] = ( q - v[k] ) * ( q - v[k] ) + f[v[k]];
  }
}

/* distance transform of 2d function using squared distance */
void QgsShapeburstFillSymbolLayer::distanceTransform2d( double *im, int width, int height, QgsRenderContext &context )
{
  int maxDimension = std::max( width, height );
  double *f = new double[ maxDimension ];
  int *v = new int[ maxDimension ];
  double *z = new double[ maxDimension + 1 ];
  double *d = new double[ maxDimension ];

  // transform along columns
  for ( int x = 0; x < width; x++ )
  {
    if ( context.renderingStopped() )
      break;

    for ( int y = 0; y < height; y++ )
    {
      f[y] = im[ x + y * width ];
    }
    distanceTransform1d( f, height, v, z, d );
    for ( int y = 0; y < height; y++ )
    {
      im[ x + y * width ] = d[y];
    }
  }

  // transform along rows
  for ( int y = 0; y < height; y++ )
  {
    if ( context.renderingStopped() )
      break;

    for ( int x = 0; x < width; x++ )
    {
      f[x] = im[  x + y * width ];
    }
    distanceTransform1d( f, width, v, z, d );
    for ( int x = 0; x < width; x++ )
    {
      im[  x + y * width ] = d[x];
    }
  }

  delete [] d;
  delete [] f;
  delete [] v;
  delete [] z;
}

/* distance transform of a binary QImage */
double *QgsShapeburstFillSymbolLayer::distanceTransform( QImage *im, QgsRenderContext &context )
{
  int width = im->width();
  int height = im->height();

  double *dtArray = new double[width * height];

  //load qImage to array
  QRgb tmpRgb;
  int idx = 0;
  for ( int heightIndex = 0; heightIndex < height; ++heightIndex )
  {
    if ( context.renderingStopped() )
      break;

    const QRgb *scanLine = reinterpret_cast< const QRgb * >( im->constScanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < width; ++widthIndex )
    {
      tmpRgb = scanLine[widthIndex];
      if ( qRed( tmpRgb ) == 0 )
      {
        //black pixel, so zero distance
        dtArray[ idx ] = 0;
      }
      else
      {
        //white pixel, so initially set distance as infinite
        dtArray[ idx ] = INF;
      }
      idx++;
    }
  }

  //calculate squared distance transform
  distanceTransform2d( dtArray, width, height, context );

  return dtArray;
}

void QgsShapeburstFillSymbolLayer::dtArrayToQImage( double *array, QImage *im, QgsColorRamp *ramp, QgsRenderContext &context, bool useWholeShape, int maxPixelDistance )
{
  int width = im->width();
  int height = im->height();

  //find maximum distance value
  double maxDistanceValue;

  if ( useWholeShape )
  {
    //no max distance specified in symbol properties, so calculate from maximum value in distance transform results
    double dtMaxValue = array[0];
    for ( int i = 1; i < ( width * height ); ++i )
    {
      if ( array[i] > dtMaxValue )
      {
        dtMaxValue = array[i];
      }
    }

    //values in distance transform are squared
    maxDistanceValue = std::sqrt( dtMaxValue );
  }
  else
  {
    //use max distance set in symbol properties
    maxDistanceValue = maxPixelDistance;
  }

  //update the pixels in the provided QImage
  int idx = 0;
  double squaredVal = 0;
  double pixVal = 0;

  for ( int heightIndex = 0; heightIndex < height; ++heightIndex )
  {
    if ( context.renderingStopped() )
      break;

    QRgb *scanLine = reinterpret_cast< QRgb * >( im->scanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < width; ++widthIndex )
    {
      //result of distance transform
      squaredVal = array[idx];

      //scale result to fit in the range [0, 1]
      if ( maxDistanceValue > 0 )
      {
        pixVal = squaredVal > 0 ? std::min( ( std::sqrt( squaredVal ) / maxDistanceValue ), 1.0 ) : 0;
      }
      else
      {
        pixVal = 1.0;
      }

      //convert value to color from ramp
      //premultiply ramp color since we are storing this in a ARGB32_Premultiplied QImage
      scanLine[widthIndex] = qPremultiply( ramp->color( pixVal ).rgba() );
      idx++;
    }
  }
}

QVariantMap QgsShapeburstFillSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "gradient_color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  map[QStringLiteral( "color_type" )] = QString::number( static_cast< int >( mColorType ) );
  map[QStringLiteral( "blur_radius" )] = QString::number( mBlurRadius );
  map[QStringLiteral( "use_whole_shape" )] = QString::number( mUseWholeShape );
  map[QStringLiteral( "max_distance" )] = QString::number( mMaxDistance );
  map[QStringLiteral( "distance_unit" )] = QgsUnitTypes::encodeUnit( mDistanceUnit );
  map[QStringLiteral( "distance_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceMapUnitScale );
  map[QStringLiteral( "ignore_rings" )] = QString::number( mIgnoreRings );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  if ( mGradientRamp )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    map.unite( mGradientRamp->properties() );
#else
    map.insert( mGradientRamp->properties() );
#endif
  }

  return map;
}

QgsShapeburstFillSymbolLayer *QgsShapeburstFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsShapeburstFillSymbolLayer > sl = std::make_unique< QgsShapeburstFillSymbolLayer >( mColor, mColor2, mColorType, mBlurRadius, mUseWholeShape, mMaxDistance );
  if ( mGradientRamp )
  {
    sl->setColorRamp( mGradientRamp->clone() );
  }
  sl->setDistanceUnit( mDistanceUnit );
  sl->setDistanceMapUnitScale( mDistanceMapUnitScale );
  sl->setIgnoreRings( mIgnoreRings );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  copyDataDefinedProperties( sl.get() );
  copyPaintEffect( sl.get() );
  return sl.release();
}

double QgsShapeburstFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  double offsetBleed = context.convertToPainterUnits( std::max( std::fabs( mOffset.x() ), std::fabs( mOffset.y() ) ), mOffsetUnit, mOffsetMapUnitScale );
  return offsetBleed;
}

bool QgsShapeburstFillSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}

void QgsShapeburstFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mDistanceUnit = unit;
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsShapeburstFillSymbolLayer::outputUnit() const
{
  if ( mDistanceUnit == mOffsetUnit )
  {
    return mDistanceUnit;
  }
  return QgsUnitTypes::RenderUnknownUnit;
}

bool QgsShapeburstFillSymbolLayer::usesMapUnits() const
{
  return mDistanceUnit == QgsUnitTypes::RenderMapUnits || mDistanceUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsShapeburstFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mDistanceMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsShapeburstFillSymbolLayer::mapUnitScale() const
{
  if ( mDistanceMapUnitScale == mOffsetMapUnitScale )
  {
    return mDistanceMapUnitScale;
  }
  return QgsMapUnitScale();
}


//QgsImageFillSymbolLayer

QgsImageFillSymbolLayer::QgsImageFillSymbolLayer()
{
}

QgsImageFillSymbolLayer::~QgsImageFillSymbolLayer() = default;

void QgsImageFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  mNextAngle = mAngle;
  applyDataDefinedSettings( context );

  p->setPen( QPen( Qt::NoPen ) );

  QTransform bkTransform = mBrush.transform();
  if ( applyBrushTransformFromContext( &context ) && !context.renderContext().textureOrigin().isNull() )
  {
    QPointF leftCorner = context.renderContext().textureOrigin();
    QTransform t = mBrush.transform();
    t.translate( leftCorner.x(), leftCorner.y() );
    mBrush.setTransform( t );
  }
  else
  {
    QTransform t = mBrush.transform();
    t.translate( 0, 0 );
    mBrush.setTransform( t );
  }

  if ( context.selected() )
  {
    QColor selColor = context.renderContext().selectionColor();
    p->setBrush( QBrush( selColor ) );
    _renderPolygon( p, points, rings, context );
  }

  if ( !qgsDoubleNear( mNextAngle, 0.0 ) )
  {
    QTransform t = mBrush.transform();
    t.rotate( mNextAngle );
    mBrush.setTransform( t );
  }
  p->setBrush( mBrush );
  _renderPolygon( p, points, rings, context );

  mBrush.setTransform( bkTransform );
}

void QgsImageFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mStrokeWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsImageFillSymbolLayer::outputUnit() const
{
  return mStrokeWidthUnit;
}

void QgsImageFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mStrokeWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsImageFillSymbolLayer::mapUnitScale() const
{
  return mStrokeWidthMapUnitScale;
}

double QgsImageFillSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double width = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  return width * QgsDxfExport::mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}

Qt::PenStyle QgsImageFillSymbolLayer::dxfPenStyle() const
{
  return Qt::SolidLine;
#if 0
  if ( !mStroke )
  {
    return Qt::SolidLine;
  }
  else
  {
    return mStroke->dxfPenStyle();
  }
#endif //0
}

QVariantMap QgsImageFillSymbolLayer::properties() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "coordinate_reference" ), QgsSymbolLayerUtils::encodeCoordinateReference( mCoordinateReference ) );
  return map;
}

bool QgsImageFillSymbolLayer::applyBrushTransformFromContext( QgsSymbolRenderContext *context ) const
{
  //coordinate reference
  Qgis::SymbolCoordinateReference coordinateReference = mCoordinateReference;
  if ( context && mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCoordinateMode ) )
  {
    bool ok = false;
    QString string = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCoordinateMode, context->renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      coordinateReference = QgsSymbolLayerUtils::decodeCoordinateReference( string, &ok );
      if ( !ok )
        coordinateReference = mCoordinateReference;
    }
  }

  return coordinateReference == Qgis::SymbolCoordinateReference::Feature;
}


//QgsSVGFillSymbolLayer

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QString &svgFilePath, double width, double angle )
  : QgsImageFillSymbolLayer()
  , mPatternWidth( width )
{
  mStrokeWidth = 0.3;
  mAngle = angle;
  mColor = QColor( 255, 255, 255 );
  setSvgFilePath( svgFilePath );
}

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QByteArray &svgData, double width, double angle )
  : QgsImageFillSymbolLayer()
  , mPatternWidth( width )
  , mSvgData( svgData )
{
  storeViewBox();
  mStrokeWidth = 0.3;
  mAngle = angle;
  mColor = QColor( 255, 255, 255 );
  setDefaultSvgParams();
}

QgsSVGFillSymbolLayer::~QgsSVGFillSymbolLayer() = default;

void QgsSVGFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mPatternWidthUnit = unit;
  mSvgStrokeWidthUnit = unit;
  mStrokeWidthUnit = unit;
  if ( mStroke )
    mStroke->setOutputUnit( unit );
}

QgsUnitTypes::RenderUnit QgsSVGFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mPatternWidthUnit != unit || mSvgStrokeWidthUnit != unit || mStrokeWidthUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsSVGFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsImageFillSymbolLayer::setMapUnitScale( scale );
  mPatternWidthMapUnitScale = scale;
  mSvgStrokeWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSVGFillSymbolLayer::mapUnitScale() const
{
  if ( QgsImageFillSymbolLayer::mapUnitScale() == mPatternWidthMapUnitScale &&
       mPatternWidthMapUnitScale == mSvgStrokeWidthMapUnitScale &&
       mSvgStrokeWidthMapUnitScale == mStrokeWidthMapUnitScale )
  {
    return mPatternWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSVGFillSymbolLayer::setSvgFilePath( const QString &svgPath )
{
  mSvgData = QgsApplication::svgCache()->getImageData( svgPath );
  storeViewBox();

  mSvgFilePath = svgPath;
  setDefaultSvgParams();
}

QgsSymbolLayer *QgsSVGFillSymbolLayer::create( const QVariantMap &properties )
{
  QByteArray data;
  double width = 20;
  QString svgFilePath;
  double angle = 0.0;

  if ( properties.contains( QStringLiteral( "width" ) ) )
  {
    width = properties[QStringLiteral( "width" )].toDouble();
  }
  if ( properties.contains( QStringLiteral( "svgFile" ) ) )
  {
    svgFilePath = properties[QStringLiteral( "svgFile" )].toString();
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    angle = properties[QStringLiteral( "angle" )].toDouble();
  }

  std::unique_ptr< QgsSVGFillSymbolLayer > symbolLayer;
  if ( !svgFilePath.isEmpty() )
  {
    symbolLayer = std::make_unique< QgsSVGFillSymbolLayer >( svgFilePath, width, angle );
  }
  else
  {
    if ( properties.contains( QStringLiteral( "data" ) ) )
    {
      data = QByteArray::fromHex( properties[QStringLiteral( "data" )].toString().toLocal8Bit() );
    }
    symbolLayer = std::make_unique< QgsSVGFillSymbolLayer >( data, width, angle );
  }

  //svg parameters
  if ( properties.contains( QStringLiteral( "svgFillColor" ) ) )
  {
    //pre 2.5 projects used "svgFillColor"
    symbolLayer->setSvgFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "svgFillColor" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    symbolLayer->setSvgFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "svgOutlineColor" ) ) )
  {
    //pre 2.5 projects used "svgOutlineColor"
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "svgOutlineColor" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "svgOutlineWidth" ) ) )
  {
    //pre 2.5 projects used "svgOutlineWidth"
    symbolLayer->setSvgStrokeWidth( properties[QStringLiteral( "svgOutlineWidth" )].toDouble() );
  }
  else if ( properties.contains( QStringLiteral( "outline_width" ) ) )
  {
    symbolLayer->setSvgStrokeWidth( properties[QStringLiteral( "outline_width" )].toDouble() );
  }
  else if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    symbolLayer->setSvgStrokeWidth( properties[QStringLiteral( "line_width" )].toDouble() );
  }

  //units
  if ( properties.contains( QStringLiteral( "pattern_width_unit" ) ) )
  {
    symbolLayer->setPatternWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "pattern_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "pattern_width_map_unit_scale" ) ) )
  {
    symbolLayer->setPatternWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "pattern_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "svg_outline_width_unit" ) ) )
  {
    symbolLayer->setSvgStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "svg_outline_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "svg_outline_width_map_unit_scale" ) ) )
  {
    symbolLayer->setSvgStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "svg_outline_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    symbolLayer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    symbolLayer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "parameters" ) ) )
  {
    const QVariantMap parameters = properties[QStringLiteral( "parameters" )].toMap();
    symbolLayer->setParameters( QgsProperty::variantMapToPropertyMap( parameters ) );
  }

  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer.release();
}

void QgsSVGFillSymbolLayer::resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QVariantMap::iterator it = properties.find( QStringLiteral( "svgFile" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value().toString(), pathResolver );
    else
      it.value() = QgsSymbolLayerUtils::svgSymbolNameToPath( it.value().toString(), pathResolver );
  }
}

QString QgsSVGFillSymbolLayer::layerType() const
{
  return QStringLiteral( "SVGFill" );
}

void QgsSVGFillSymbolLayer::applyPattern( QBrush &brush, const QString &svgFilePath, double patternWidth, QgsUnitTypes::RenderUnit patternWidthUnit,
    const QColor &svgFillColor, const QColor &svgStrokeColor, double svgStrokeWidth,
    QgsUnitTypes::RenderUnit svgStrokeWidthUnit, const QgsSymbolRenderContext &context,
    const QgsMapUnitScale &patternWidthMapUnitScale, const QgsMapUnitScale &svgStrokeWidthMapUnitScale, const QgsStringMap svgParameters )
{
  if ( mSvgViewBox.isNull() )
  {
    return;
  }

  double size = context.renderContext().convertToPainterUnits( patternWidth, patternWidthUnit, patternWidthMapUnitScale );

  if ( static_cast< int >( size ) < 1.0 || 10000.0 < size )
  {
    brush.setTextureImage( QImage() );
  }
  else
  {
    bool fitsInCache = true;
    double strokeWidth = context.renderContext().convertToPainterUnits( svgStrokeWidth, svgStrokeWidthUnit, svgStrokeWidthMapUnitScale );
    QImage patternImage = QgsApplication::svgCache()->svgAsImage( svgFilePath, size, svgFillColor, svgStrokeColor, strokeWidth,
                          context.renderContext().scaleFactor(), fitsInCache, 0, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ), svgParameters );
    if ( !fitsInCache )
    {
      QPicture patternPict = QgsApplication::svgCache()->svgAsPicture( svgFilePath, size, svgFillColor, svgStrokeColor, strokeWidth,
                             context.renderContext().scaleFactor(), false, 0, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
      double hwRatio = 1.0;
      if ( patternPict.width() > 0 )
      {
        hwRatio = static_cast< double >( patternPict.height() ) / static_cast< double >( patternPict.width() );
      }
      patternImage = QImage( static_cast< int >( size ), static_cast< int >( size * hwRatio ), QImage::Format_ARGB32_Premultiplied );
      patternImage.fill( 0 ); // transparent background

      QPainter p( &patternImage );
      p.drawPicture( QPointF( size / 2, size * hwRatio / 2 ), patternPict );
    }

    QTransform brushTransform;
    if ( !qgsDoubleNear( context.opacity(), 1.0 ) )
    {
      QImage transparentImage = patternImage.copy();
      QgsSymbolLayerUtils::multiplyImageOpacity( &transparentImage, context.opacity() );
      brush.setTextureImage( transparentImage );
    }
    else
    {
      brush.setTextureImage( patternImage );
    }
    brush.setTransform( brushTransform );
  }
}

void QgsSVGFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( mParameters, context.renderContext().expressionContext() );

  applyPattern( mBrush, mSvgFilePath, mPatternWidth, mPatternWidthUnit, mColor, mSvgStrokeColor, mSvgStrokeWidth, mSvgStrokeWidthUnit, context, mPatternWidthMapUnitScale, mSvgStrokeWidthMapUnitScale, evaluatedParameters );

  if ( mStroke )
  {
    mStroke->startRender( context.renderContext(), context.fields() );
  }
}

void QgsSVGFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mStroke )
  {
    mStroke->stopRender( context.renderContext() );
  }
}

void QgsSVGFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QgsImageFillSymbolLayer::renderPolygon( points, rings, context );

  if ( mStroke )
  {
    mStroke->renderPolyline( points, context.feature(), context.renderContext(), -1, SELECT_FILL_BORDER && context.selected() );
    if ( rings )
    {
      for ( auto ringIt = rings->constBegin(); ringIt != rings->constEnd(); ++ringIt )
      {
        mStroke->renderPolyline( *ringIt, context.feature(), context.renderContext(), -1, SELECT_FILL_BORDER && context.selected() );
      }
    }
  }
}

QVariantMap QgsSVGFillSymbolLayer::properties() const
{
  QVariantMap map;
  if ( !mSvgFilePath.isEmpty() )
  {
    map.insert( QStringLiteral( "svgFile" ), mSvgFilePath );
  }
  else
  {
    map.insert( QStringLiteral( "data" ), QString( mSvgData.toHex() ) );
  }

  map.insert( QStringLiteral( "width" ), QString::number( mPatternWidth ) );
  map.insert( QStringLiteral( "angle" ), QString::number( mAngle ) );

  //svg parameters
  map.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  map.insert( QStringLiteral( "outline_color" ), QgsSymbolLayerUtils::encodeColor( mSvgStrokeColor ) );
  map.insert( QStringLiteral( "outline_width" ), QString::number( mSvgStrokeWidth ) );

  //units
  map.insert( QStringLiteral( "pattern_width_unit" ), QgsUnitTypes::encodeUnit( mPatternWidthUnit ) );
  map.insert( QStringLiteral( "pattern_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mPatternWidthMapUnitScale ) );
  map.insert( QStringLiteral( "svg_outline_width_unit" ), QgsUnitTypes::encodeUnit( mSvgStrokeWidthUnit ) );
  map.insert( QStringLiteral( "svg_outline_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mSvgStrokeWidthMapUnitScale ) );
  map.insert( QStringLiteral( "outline_width_unit" ), QgsUnitTypes::encodeUnit( mStrokeWidthUnit ) );
  map.insert( QStringLiteral( "outline_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale ) );

  map[QStringLiteral( "parameters" )] = QgsProperty::propertyMapToVariantMap( mParameters );

  return map;
}

QgsSVGFillSymbolLayer *QgsSVGFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsSVGFillSymbolLayer > clonedLayer;
  if ( !mSvgFilePath.isEmpty() )
  {
    clonedLayer = std::make_unique< QgsSVGFillSymbolLayer >( mSvgFilePath, mPatternWidth, mAngle );
    clonedLayer->setSvgFillColor( mColor );
    clonedLayer->setSvgStrokeColor( mSvgStrokeColor );
    clonedLayer->setSvgStrokeWidth( mSvgStrokeWidth );
  }
  else
  {
    clonedLayer = std::make_unique< QgsSVGFillSymbolLayer >( mSvgData, mPatternWidth, mAngle );
  }

  clonedLayer->setPatternWidthUnit( mPatternWidthUnit );
  clonedLayer->setPatternWidthMapUnitScale( mPatternWidthMapUnitScale );
  clonedLayer->setSvgStrokeWidthUnit( mSvgStrokeWidthUnit );
  clonedLayer->setSvgStrokeWidthMapUnitScale( mSvgStrokeWidthMapUnitScale );
  clonedLayer->setStrokeWidthUnit( mStrokeWidthUnit );
  clonedLayer->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );

  clonedLayer->setParameters( mParameters );

  if ( mStroke )
  {
    clonedLayer->setSubSymbol( mStroke->clone() );
  }
  copyDataDefinedProperties( clonedLayer.get() );
  copyPaintEffect( clonedLayer.get() );
  return clonedLayer.release();
}

void QgsSVGFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
  symbolizerElem.appendChild( fillElem );

  QDomElement graphicFillElem = doc.createElement( QStringLiteral( "se:GraphicFill" ) );
  fillElem.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  graphicFillElem.appendChild( graphicElem );

  if ( !mSvgFilePath.isEmpty() )
  {
    // encode a parametric SVG reference
    double patternWidth = QgsSymbolLayerUtils::rescaleUom( mPatternWidth, mPatternWidthUnit, props );
    double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mSvgStrokeWidth, mSvgStrokeWidthUnit, props );
    QgsSymbolLayerUtils::parametricSvgToSld( doc, graphicElem, mSvgFilePath, mColor, patternWidth, mSvgStrokeColor, strokeWidth );
  }
  else
  {
    // TODO: create svg from data
    // <se:InlineContent>
    symbolizerElem.appendChild( doc.createComment( QStringLiteral( "SVG from data not implemented yet" ) ) );
  }

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toString() ).arg( mAngle );
  }
  else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  if ( mStroke )
  {
    // the stroke sub symbol should be stored within the Stroke element,
    // but it will be stored in a separated LineSymbolizer because it could
    // have more than one layer
    mStroke->toSld( doc, element, props );
  }
}

bool QgsSVGFillSymbolLayer::usesMapUnits() const
{
  return mPatternWidthUnit == QgsUnitTypes::RenderMapUnits || mPatternWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mSvgStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mSvgStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

QgsSymbol *QgsSVGFillSymbolLayer::subSymbol()
{
  return mStroke.get();
}

bool QgsSVGFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol ) //unset current stroke
  {
    mStroke.reset( nullptr );
    return true;
  }

  if ( symbol->type() != Qgis::SymbolType::Line )
  {
    delete symbol;
    return false;
  }

  QgsLineSymbol *lineSymbol = dynamic_cast<QgsLineSymbol *>( symbol );
  if ( lineSymbol )
  {
    mStroke.reset( lineSymbol );
    return true;
  }

  delete symbol;
  return false;
}

double QgsSVGFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mStroke && mStroke->symbolLayer( 0 ) )
  {
    double subLayerBleed = mStroke->symbolLayer( 0 )->estimateMaxBleed( context );
    return subLayerBleed;
  }
  return 0;
}

QColor QgsSVGFillSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context )
  if ( !mStroke )
  {
    return QColor( Qt::black );
  }
  return mStroke->color();
}

QSet<QString> QgsSVGFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsImageFillSymbolLayer::usedAttributes( context );
  if ( mStroke )
    attr.unite( mStroke->usedAttributes( context ) );
  return attr;
}

bool QgsSVGFillSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsImageFillSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mStroke && mStroke->hasDataDefinedProperties() )
    return true;
  return false;
}

QgsSymbolLayer *QgsSVGFillSymbolLayer::createFromSld( QDomElement &element )
{
  QString path, mimeType;
  QColor fillColor, strokeColor;
  Qt::PenStyle penStyle;
  double size, strokeWidth;

  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  if ( fillElem.isNull() )
    return nullptr;

  QDomElement graphicFillElem = fillElem.firstChildElement( QStringLiteral( "GraphicFill" ) );
  if ( graphicFillElem.isNull() )
    return nullptr;

  QDomElement graphicElem = graphicFillElem.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  if ( !QgsSymbolLayerUtils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return nullptr;

  if ( mimeType != QLatin1String( "image/svg+xml" ) )
    return nullptr;

  QgsSymbolLayerUtils::lineFromSld( graphicElem, penStyle, strokeColor, strokeWidth );

  double scaleFactor = 1.0;
  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  QgsUnitTypes::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  size = size * scaleFactor;
  strokeWidth = strokeWidth * scaleFactor;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  std::unique_ptr< QgsSVGFillSymbolLayer > sl = std::make_unique< QgsSVGFillSymbolLayer >( path, size, angle );
  sl->setOutputUnit( sldUnitSize );
  sl->setSvgFillColor( fillColor );
  sl->setSvgStrokeColor( strokeColor );
  sl->setSvgStrokeWidth( strokeWidth );

  // try to get the stroke
  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( !strokeElem.isNull() )
  {
    QgsSymbolLayer *l = QgsSymbolLayerUtils::createLineLayerFromSld( strokeElem );
    if ( l )
    {
      QgsSymbolLayerList layers;
      layers.append( l );
      sl->setSubSymbol( new QgsLineSymbol( layers ) );
    }
  }

  return sl.release();
}

void QgsSVGFillSymbolLayer::applyDataDefinedSettings( QgsSymbolRenderContext &context )
{
  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFile )
       && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor )
       && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    return; //no data defined settings
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    mNextAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mNextAngle );
  }

  double width = mPatternWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) )
  {
    context.setOriginalValueVariable( mPatternWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mPatternWidth );
  }
  QString svgFile = mSvgFilePath;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFile ) )
  {
    context.setOriginalValueVariable( mSvgFilePath );
    svgFile = QgsSymbolLayerUtils::svgSymbolNameToPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyFile, context.renderContext().expressionContext(), mSvgFilePath ),
              context.renderContext().pathResolver() );
  }
  QColor svgFillColor = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    svgFillColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }
  QColor svgStrokeColor = mSvgStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mSvgStrokeColor ) );
    svgStrokeColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mSvgStrokeColor );
  }
  double strokeWidth = mSvgStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mSvgStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mSvgStrokeWidth );
  }
  QgsStringMap evaluatedParameters = QgsSymbolLayerUtils::evaluatePropertiesMap( mParameters, context.renderContext().expressionContext() );

  applyPattern( mBrush, svgFile, width, mPatternWidthUnit, svgFillColor, svgStrokeColor, strokeWidth,
                mSvgStrokeWidthUnit, context, mPatternWidthMapUnitScale, mSvgStrokeWidthMapUnitScale, evaluatedParameters );

}

void QgsSVGFillSymbolLayer::storeViewBox()
{
  if ( !mSvgData.isEmpty() )
  {
    QSvgRenderer r( mSvgData );
    if ( r.isValid() )
    {
      mSvgViewBox = r.viewBoxF();
      return;
    }
  }

  mSvgViewBox = QRectF();
}

void QgsSVGFillSymbolLayer::setDefaultSvgParams()
{
  if ( mSvgFilePath.isEmpty() )
  {
    return;
  }

  bool hasFillParam, hasFillOpacityParam, hasStrokeParam, hasStrokeWidthParam, hasStrokeOpacityParam;
  bool hasDefaultFillColor, hasDefaultFillOpacity, hasDefaultStrokeColor, hasDefaultStrokeWidth, hasDefaultStrokeOpacity;
  QColor defaultFillColor, defaultStrokeColor;
  double defaultStrokeWidth, defaultFillOpacity, defaultStrokeOpacity;
  QgsApplication::svgCache()->containsParams( mSvgFilePath, hasFillParam, hasDefaultFillColor, defaultFillColor,
      hasFillOpacityParam, hasDefaultFillOpacity, defaultFillOpacity,
      hasStrokeParam, hasDefaultStrokeColor, defaultStrokeColor,
      hasStrokeWidthParam, hasDefaultStrokeWidth, defaultStrokeWidth,
      hasStrokeOpacityParam, hasDefaultStrokeOpacity, defaultStrokeOpacity );

  double newFillOpacity = hasFillOpacityParam ? mColor.alphaF() : 1.0;
  double newStrokeOpacity = hasStrokeOpacityParam ? mSvgStrokeColor.alphaF() : 1.0;

  if ( hasDefaultFillColor )
  {
    mColor = defaultFillColor;
    mColor.setAlphaF( newFillOpacity );
  }
  if ( hasDefaultFillOpacity )
  {
    mColor.setAlphaF( defaultFillOpacity );
  }
  if ( hasDefaultStrokeColor )
  {
    mSvgStrokeColor = defaultStrokeColor;
    mSvgStrokeColor.setAlphaF( newStrokeOpacity );
  }
  if ( hasDefaultStrokeOpacity )
  {
    mSvgStrokeColor.setAlphaF( defaultStrokeOpacity );
  }
  if ( hasDefaultStrokeWidth )
  {
    mSvgStrokeWidth = defaultStrokeWidth;
  }
}

void QgsSVGFillSymbolLayer::setParameters( const QMap<QString, QgsProperty> &parameters )
{
  mParameters = parameters;
}


QgsLinePatternFillSymbolLayer::QgsLinePatternFillSymbolLayer()
  : QgsImageFillSymbolLayer()
{
  setSubSymbol( new QgsLineSymbol() );
  QgsImageFillSymbolLayer::setSubSymbol( nullptr ); //no stroke
}

QgsLinePatternFillSymbolLayer::~QgsLinePatternFillSymbolLayer() = default;

void QgsLinePatternFillSymbolLayer::setLineWidth( double w )
{
  mFillLineSymbol->setWidth( w );
  mLineWidth = w;
}

void QgsLinePatternFillSymbolLayer::setColor( const QColor &c )
{
  mFillLineSymbol->setColor( c );
  mColor = c;
}

QColor QgsLinePatternFillSymbolLayer::color() const
{
  return mFillLineSymbol ? mFillLineSymbol->color() : mColor;
}

bool QgsLinePatternFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == Qgis::SymbolType::Line )
  {
    mFillLineSymbol.reset( qgis::down_cast<QgsLineSymbol *>( symbol ) );
    return true;
  }
  delete symbol;
  return false;
}

QgsSymbol *QgsLinePatternFillSymbolLayer::subSymbol()
{
  return mFillLineSymbol.get();
}

QSet<QString> QgsLinePatternFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsImageFillSymbolLayer::usedAttributes( context );
  if ( mFillLineSymbol )
    attr.unite( mFillLineSymbol->usedAttributes( context ) );
  return attr;
}

bool QgsLinePatternFillSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mFillLineSymbol && mFillLineSymbol->hasDataDefinedProperties() )
    return true;
  return false;
}

void QgsLinePatternFillSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  // deliberately don't pass this on to subsymbol here
}

void QgsLinePatternFillSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  // deliberately don't pass this on to subsymbol here
}

double QgsLinePatternFillSymbolLayer::estimateMaxBleed( const QgsRenderContext & ) const
{
  return 0;
}

void QgsLinePatternFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mDistanceUnit = unit;
  mLineWidthUnit = unit;
  mOffsetUnit = unit;

  if ( mFillLineSymbol )
    mFillLineSymbol->setOutputUnit( unit );
}

QgsUnitTypes::RenderUnit QgsLinePatternFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceUnit != unit || mLineWidthUnit != unit || ( mOffsetUnit != unit && mOffsetUnit != QgsUnitTypes::RenderPercentage ) )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsLinePatternFillSymbolLayer::usesMapUnits() const
{
  return mDistanceUnit == QgsUnitTypes::RenderMapUnits || mDistanceUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mLineWidthUnit == QgsUnitTypes::RenderMapUnits || mLineWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsLinePatternFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsImageFillSymbolLayer::setMapUnitScale( scale );
  mDistanceMapUnitScale = scale;
  mLineWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsLinePatternFillSymbolLayer::mapUnitScale() const
{
  if ( QgsImageFillSymbolLayer::mapUnitScale() == mDistanceMapUnitScale &&
       mDistanceMapUnitScale == mLineWidthMapUnitScale &&
       mLineWidthMapUnitScale == mOffsetMapUnitScale )
  {
    return mDistanceMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsLinePatternFillSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsLinePatternFillSymbolLayer > patternLayer = std::make_unique< QgsLinePatternFillSymbolLayer >();

  //default values
  double lineAngle = 45;
  double distance = 5;
  double lineWidth = 0.5;
  QColor color( Qt::black );
  double offset = 0.0;

  if ( properties.contains( QStringLiteral( "lineangle" ) ) )
  {
    //pre 2.5 projects used "lineangle"
    lineAngle = properties[QStringLiteral( "lineangle" )].toDouble();
  }
  else if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    lineAngle = properties[QStringLiteral( "angle" )].toDouble();
  }
  patternLayer->setLineAngle( lineAngle );

  if ( properties.contains( QStringLiteral( "distance" ) ) )
  {
    distance = properties[QStringLiteral( "distance" )].toDouble();
  }
  patternLayer->setDistance( distance );

  if ( properties.contains( QStringLiteral( "linewidth" ) ) )
  {
    //pre 2.5 projects used "linewidth"
    lineWidth = properties[QStringLiteral( "linewidth" )].toDouble();
  }
  else if ( properties.contains( QStringLiteral( "outline_width" ) ) )
  {
    lineWidth = properties[QStringLiteral( "outline_width" )].toDouble();
  }
  else if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    lineWidth = properties[QStringLiteral( "line_width" )].toDouble();
  }
  patternLayer->setLineWidth( lineWidth );

  if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )].toString() );
  }
  else if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )].toString() );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )].toString() );
  }
  patternLayer->setColor( color );

  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    offset = properties[QStringLiteral( "offset" )].toDouble();
  }
  patternLayer->setOffset( offset );


  if ( properties.contains( QStringLiteral( "distance_unit" ) ) )
  {
    patternLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    patternLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    patternLayer->setLineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    patternLayer->setLineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "line_width_map_unit_scale" ) ) )
  {
    patternLayer->setLineWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "line_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    patternLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    patternLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    patternLayer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    patternLayer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "coordinate_reference" ) ) )
  {
    patternLayer->setCoordinateReference( QgsSymbolLayerUtils::decodeCoordinateReference( properties[QStringLiteral( "coordinate_reference" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "clip_mode" ) ) )
  {
    patternLayer->setClipMode( QgsSymbolLayerUtils::decodeLineClipMode( properties.value( QStringLiteral( "clip_mode" ) ).toString() ) );
  }

  patternLayer->restoreOldDataDefinedProperties( properties );

  return patternLayer.release();
}

QString QgsLinePatternFillSymbolLayer::layerType() const
{
  return QStringLiteral( "LinePatternFill" );
}

void QgsLinePatternFillSymbolLayer::applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double lineAngle, double distance )
{
  mBrush.setTextureImage( QImage() ); // set empty in case we have to return

  if ( !mFillLineSymbol )
  {
    return;
  }
  // We have to make a copy because marker intervals will have to be adjusted
  std::unique_ptr< QgsLineSymbol > fillLineSymbol( mFillLineSymbol->clone() );
  if ( !fillLineSymbol )
  {
    return;
  }

  const QgsRenderContext &ctx = context.renderContext();
  //double strokePixelWidth = lineWidth * QgsSymbolLayerUtils::pixelSizeScaleFactor( ctx,  mLineWidthUnit, mLineWidthMapUnitScale );
  double outputPixelDist = ctx.convertToPainterUnits( distance, mDistanceUnit, mDistanceMapUnitScale );
  double outputPixelOffset = mOffsetUnit == QgsUnitTypes::RenderPercentage ? outputPixelDist * mOffset / 100
                             : ctx.convertToPainterUnits( mOffset, mOffsetUnit, mOffsetMapUnitScale );

  // NOTE: this may need to be modified if we ever change from a forced rasterized/brush approach,
  // because potentially we may want to allow vector based line pattern fills where the first line
  // is offset by a large distance

  // fix truncated pattern with larger offsets
  outputPixelOffset = std::fmod( outputPixelOffset, outputPixelDist );
  if ( outputPixelOffset > outputPixelDist / 2.0 )
    outputPixelOffset -= outputPixelDist;

  // To get all patterns into image, we have to consider symbols size (estimateMaxBleed()).
  // For marker lines we have to get markers interval.
  double outputPixelBleed = 0;
  double outputPixelInterval = 0; // maximum interval
  for ( int i = 0; i < fillLineSymbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayer *layer = fillLineSymbol->symbolLayer( i );
    double outputPixelLayerBleed = layer->estimateMaxBleed( context.renderContext() );
    outputPixelBleed = std::max( outputPixelBleed, outputPixelLayerBleed );

    QgsMarkerLineSymbolLayer *markerLineLayer = dynamic_cast<QgsMarkerLineSymbolLayer *>( layer );
    if ( markerLineLayer )
    {
      double outputPixelLayerInterval = ctx.convertToPainterUnits( markerLineLayer->interval(), markerLineLayer->intervalUnit(), markerLineLayer->intervalMapUnitScale() );

      // There may be multiple marker lines with different intervals.
      // In theory we should find the least common multiple, but that could be too
      // big (multiplication of intervals in the worst case).
      // Because patterns without small common interval would look strange, we
      // believe that the longest interval should usually be sufficient.
      outputPixelInterval = std::max( outputPixelInterval, outputPixelLayerInterval );
    }
  }

  if ( outputPixelInterval > 0 )
  {
    // We have to adjust marker intervals to integer pixel size to get
    // repeatable pattern.
    double intervalScale = std::round( outputPixelInterval ) / outputPixelInterval;
    outputPixelInterval = std::round( outputPixelInterval );

    for ( int i = 0; i < fillLineSymbol->symbolLayerCount(); i++ )
    {
      QgsSymbolLayer *layer = fillLineSymbol->symbolLayer( i );

      QgsMarkerLineSymbolLayer *markerLineLayer = dynamic_cast<QgsMarkerLineSymbolLayer *>( layer );
      if ( markerLineLayer )
      {
        markerLineLayer->setInterval( intervalScale * markerLineLayer->interval() );
      }
    }
  }

  //create image
  int height, width;
  lineAngle = std::fmod( lineAngle, 360 );
  if ( lineAngle < 0 )
    lineAngle += 360;
  if ( qgsDoubleNear( lineAngle, 0 ) || qgsDoubleNear( lineAngle, 360 ) || qgsDoubleNear( lineAngle, 180 ) )
  {
    height = outputPixelDist;
    width = outputPixelInterval > 0 ? outputPixelInterval : height;
  }
  else if ( qgsDoubleNear( lineAngle, 90 ) || qgsDoubleNear( lineAngle, 270 ) )
  {
    width = outputPixelDist;
    height = outputPixelInterval > 0 ? outputPixelInterval : width;
  }
  else
  {
    height = outputPixelDist / std::cos( lineAngle * M_PI / 180 ); //keep perpendicular distance between lines constant
    width = outputPixelDist / std::sin( lineAngle * M_PI / 180 );

    // recalculate real angle and distance after rounding to pixels
    lineAngle = 180 * std::atan2( static_cast< double >( height ), static_cast< double >( width ) ) / M_PI;
    if ( lineAngle < 0 )
    {
      lineAngle += 360.;
    }

    height = std::abs( height );
    width = std::abs( width );

    outputPixelDist = std::abs( height * std::cos( lineAngle * M_PI / 180 ) );

    // Round offset to correspond to one pixel height, otherwise lines may
    // be shifted on tile border if offset falls close to pixel center
    int offsetHeight = static_cast< int >( std::round( outputPixelOffset / std::cos( lineAngle * M_PI / 180 ) ) );
    outputPixelOffset = offsetHeight * std::cos( lineAngle * M_PI / 180 );
  }

  //depending on the angle, we might need to render into a larger image and use a subset of it
  double dx = 0;
  double dy = 0;

  // Add buffer based on bleed but keep precisely the height/width ratio (angle)
  // thus we add integer multiplications of width and height covering the bleed
  int bufferMulti = static_cast< int >( std::max( std::ceil( outputPixelBleed / width ), std::ceil( outputPixelBleed / width ) ) );

  // Always buffer at least once so that center of line marker in upper right corner
  // does not fall outside due to representation error
  bufferMulti = std::max( bufferMulti, 1 );

  int xBuffer = width * bufferMulti;
  int yBuffer = height * bufferMulti;
  int innerWidth = width;
  int innerHeight = height;
  width += 2 * xBuffer;
  height += 2 * yBuffer;

  //protect from zero width/height image and symbol layer from eating too much memory
  if ( width > 10000 || height > 10000 || width == 0 || height == 0 )
  {
    return;
  }

  QImage patternImage( width, height, QImage::Format_ARGB32 );
  patternImage.fill( 0 );

  QPointF p1, p2, p3, p4, p5, p6;
  if ( qgsDoubleNear( lineAngle, 0.0 ) || qgsDoubleNear( lineAngle, 360.0 ) || qgsDoubleNear( lineAngle, 180.0 ) )
  {
    p1 = QPointF( 0, yBuffer );
    p2 = QPointF( width, yBuffer );
    p3 = QPointF( 0, yBuffer + innerHeight );
    p4 = QPointF( width, yBuffer + innerHeight );
  }
  else if ( qgsDoubleNear( lineAngle, 90.0 ) || qgsDoubleNear( lineAngle, 270.0 ) )
  {
    p1 = QPointF( xBuffer, height );
    p2 = QPointF( xBuffer, 0 );
    p3 = QPointF( xBuffer + innerWidth, height );
    p4 = QPointF( xBuffer + innerWidth, 0 );
  }
  else if ( lineAngle > 0 && lineAngle < 90 )
  {
    dx = outputPixelDist * std::cos( ( 90 - lineAngle ) * M_PI / 180.0 );
    dy = outputPixelDist * std::sin( ( 90 - lineAngle ) * M_PI / 180.0 );
    p1 = QPointF( 0, height );
    p2 = QPointF( width, 0 );
    p3 = QPointF( -dx, height - dy );
    p4 = QPointF( width - dx, -dy );
    p5 = QPointF( dx, height + dy );
    p6 = QPointF( width + dx, dy );
  }
  else if ( lineAngle > 180 && lineAngle < 270 )
  {
    dx = outputPixelDist * std::cos( ( 90 - lineAngle ) * M_PI / 180.0 );
    dy = outputPixelDist * std::sin( ( 90 - lineAngle ) * M_PI / 180.0 );
    p1 = QPointF( width, 0 );
    p2 = QPointF( 0, height );
    p3 = QPointF( width - dx, -dy );
    p4 = QPointF( -dx, height - dy );
    p5 = QPointF( width + dx, dy );
    p6 = QPointF( dx, height + dy );
  }
  else if ( lineAngle > 90 && lineAngle < 180 )
  {
    dy = outputPixelDist * std::cos( ( 180 - lineAngle ) * M_PI / 180 );
    dx = outputPixelDist * std::sin( ( 180 - lineAngle ) * M_PI / 180 );
    p1 = QPointF( 0, 0 );
    p2 = QPointF( width, height );
    p5 = QPointF( dx, -dy );
    p6 = QPointF( width + dx, height - dy );
    p3 = QPointF( -dx, dy );
    p4 = QPointF( width - dx, height + dy );
  }
  else if ( lineAngle > 270 && lineAngle < 360 )
  {
    dy = outputPixelDist * std::cos( ( 180 - lineAngle ) * M_PI / 180 );
    dx = outputPixelDist * std::sin( ( 180 - lineAngle ) * M_PI / 180 );
    p1 = QPointF( width, height );
    p2 = QPointF( 0, 0 );
    p5 = QPointF( width + dx, height - dy );
    p6 = QPointF( dx, -dy );
    p3 = QPointF( width - dx, height + dy );
    p4 = QPointF( -dx, dy );
  }

  if ( !qgsDoubleNear( mOffset, 0.0 ) ) //shift everything
  {
    QPointF tempPt;
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p1, p3, outputPixelDist + outputPixelOffset );
    p3 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p2, p4, outputPixelDist + outputPixelOffset );
    p4 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p1, p5, outputPixelDist - outputPixelOffset );
    p5 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p2, p6, outputPixelDist - outputPixelOffset );
    p6 = QPointF( tempPt.x(), tempPt.y() );

    //update p1, p2 last
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p1, p3, outputPixelOffset );
    p1 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerUtils::pointOnLineWithDistance( p2, p4, outputPixelOffset );
    p2 = QPointF( tempPt.x(), tempPt.y() );
  }

  QPainter p( &patternImage );

#if 0
  // DEBUG: Draw rectangle
  p.setRenderHint( QPainter::Antialiasing, false ); // get true rect
  QPen pen( QColor( Qt::black ) );
  pen.setWidthF( 0.1 );
  pen.setCapStyle( Qt::FlatCap );
  p.setPen( pen );

  // To see this rectangle, comment buffer cut below.
  // Subtract 1 because not antialiased are rendered to the right/down by 1 pixel
  QPolygon polygon = QPolygon() << QPoint( 0, 0 ) << QPoint( width - 1, 0 ) << QPoint( width - 1, height - 1 ) << QPoint( 0, height - 1 ) << QPoint( 0, 0 );
  p.drawPolygon( polygon );

  polygon = QPolygon() << QPoint( xBuffer, yBuffer ) << QPoint( width - xBuffer - 1, yBuffer ) << QPoint( width - xBuffer - 1, height - yBuffer - 1 ) << QPoint( xBuffer, height - yBuffer - 1 ) << QPoint( xBuffer, yBuffer );
  p.drawPolygon( polygon );
#endif

  // Use antialiasing because without antialiasing lines are rendered to the
  // right and below the mathematically defined points (not symmetrical)
  // and such tiles become useless for are filling
  p.setRenderHint( QPainter::Antialiasing, true );

  // line rendering needs context for drawing on patternImage
  QgsRenderContext lineRenderContext;
  lineRenderContext.setPainter( &p );
  lineRenderContext.setScaleFactor( context.renderContext().scaleFactor() );
  QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() );
  lineRenderContext.setMapToPixel( mtp );
  lineRenderContext.setForceVectorOutput( false );
  lineRenderContext.setExpressionContext( context.renderContext().expressionContext() );
  lineRenderContext.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  fillLineSymbol->startRender( lineRenderContext, context.fields() );

  QVector<QPolygonF> polygons;
  polygons.append( QPolygonF() << p1 << p2 );
  polygons.append( QPolygonF() << p3 << p4 );
  if ( !qgsDoubleNear( lineAngle, 0 ) && !qgsDoubleNear( lineAngle, 360 ) && !qgsDoubleNear( lineAngle, 90 ) && !qgsDoubleNear( lineAngle, 180 ) && !qgsDoubleNear( lineAngle, 270 ) )
  {
    polygons.append( QPolygonF() << p5 << p6 );
  }

  for ( const QPolygonF &polygon : std::as_const( polygons ) )
  {
    fillLineSymbol->renderPolyline( polygon, context.feature(), lineRenderContext, -1, context.selected() );
  }

  fillLineSymbol->stopRender( lineRenderContext );
  p.end();

  // Cut off the buffer
  patternImage = patternImage.copy( xBuffer, yBuffer, patternImage.width() - 2 * xBuffer, patternImage.height() - 2 * yBuffer );

  //set image to mBrush
  if ( !qgsDoubleNear( context.opacity(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerUtils::multiplyImageOpacity( &transparentImage, context.opacity() );
    brush.setTextureImage( transparentImage );
  }
  else
  {
    brush.setTextureImage( patternImage );
  }

  QTransform brushTransform;
  brush.setTransform( brushTransform );
}

void QgsLinePatternFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // if we are using a vector based output, we need to render points as vectors
  // (OR if the line has data defined symbology, in which case we need to evaluate this line-by-line)
  mRenderUsingLines = context.renderContext().forceVectorOutput()
                      || mFillLineSymbol->hasDataDefinedProperties()
                      || mClipMode != Qgis::LineClipMode::ClipPainterOnly
                      || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineClipping );

  if ( mRenderUsingLines )
  {
    if ( mFillLineSymbol )
      mFillLineSymbol->startRender( context.renderContext(), context.fields() );
  }
  else
  {
    // optimised render for screen only, use image based brush
    applyPattern( context, mBrush, mLineAngle, mDistance );
  }
}

void QgsLinePatternFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mRenderUsingLines && mFillLineSymbol )
  {
    mFillLineSymbol->stopRender( context.renderContext() );
  }
}

void QgsLinePatternFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  if ( !mRenderUsingLines )
  {
    // use image based brush for speed
    QgsImageFillSymbolLayer::renderPolygon( points, rings, context );
    return;
  }

  // vector based output - so draw line by line!
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  double lineAngle = mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineAngle ) )
  {
    context.setOriginalValueVariable( mLineAngle );
    lineAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineAngle, context.renderContext().expressionContext(), mLineAngle );
  }

  double distance = mDistance;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineDistance ) )
  {
    context.setOriginalValueVariable( mDistance );
    distance = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineDistance, context.renderContext().expressionContext(), mDistance );
  }
  const double outputPixelDistance = context.renderContext().convertToPainterUnits( distance, mDistanceUnit, mDistanceMapUnitScale );

  double offset = mOffset;
  double outputPixelOffset = mOffsetUnit == QgsUnitTypes::RenderPercentage ? outputPixelDistance * offset / 100
                             :  context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );

  // fix truncated pattern with larger offsets
  outputPixelOffset = std::fmod( outputPixelOffset, outputPixelDistance );
  if ( outputPixelOffset > outputPixelDistance / 2.0 )
    outputPixelOffset -= outputPixelDistance;

  p->setPen( QPen( Qt::NoPen ) );

  if ( context.selected() )
  {
    QColor selColor = context.renderContext().selectionColor();
    p->setBrush( QBrush( selColor ) );
    _renderPolygon( p, points, rings, context );
  }

  // if invalid parameters, skip out
  if ( qgsDoubleNear( distance, 0 ) )
    return;

  p->save();

  Qgis::LineClipMode clipMode = mClipMode;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineClipping ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeLineClipMode( clipMode ) );
    bool ok = false;
    const QString valueString = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyLineClipping, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      Qgis::LineClipMode decodedMode = QgsSymbolLayerUtils::decodeLineClipMode( valueString, &ok );
      if ( ok )
        clipMode = decodedMode;
    }
  }

  std::unique_ptr< QgsPolygon > shapePolygon;
  std::unique_ptr< QgsGeometryEngine > shapeEngine;
  switch ( clipMode )
  {
    case Qgis::LineClipMode::NoClipping:
      break;

    case Qgis::LineClipMode::ClipToIntersection:
    {
      shapePolygon = std::make_unique< QgsPolygon >();
      shapePolygon->setExteriorRing( QgsLineString::fromQPolygonF( points ) );
      if ( rings )
      {
        for ( const QPolygonF &ring : *rings )
        {
          shapePolygon->addInteriorRing( QgsLineString::fromQPolygonF( ring ) );
        }
      }
      shapeEngine.reset( QgsGeometry::createGeometryEngine( shapePolygon.get() ) );
      shapeEngine->prepareGeometry();
      break;
    }

    case Qgis::LineClipMode::ClipPainterOnly:
    {
      QPainterPath path;
      path.addPolygon( points );
      if ( rings )
      {
        for ( const QPolygonF &ring : *rings )
        {
          path.addPolygon( ring );
        }
      }
      p->setClipPath( path, Qt::IntersectClip );
      break;
    }
  }

  const bool applyBrushTransform = applyBrushTransformFromContext( &context );
  const QRectF boundingRect = points.boundingRect();

  QTransform invertedRotateTransform;
  double left;
  double top;
  double right;
  double bottom;

  QTransform transform;
  if ( applyBrushTransform )
  {
    // rotation applies around center of feature
    transform.translate( -boundingRect.center().x(),
                         -boundingRect.center().y() );
    transform.rotate( lineAngle );
    transform.translate( boundingRect.center().x(),
                         boundingRect.center().y() );
  }
  else
  {
    // rotation applies around top of viewport
    transform.rotate( lineAngle );
  }

  const QRectF transformedBounds = transform.map( points ).boundingRect();

  // bounds are expanded out a bit to account for maximum line width
  const double buffer = QgsSymbolLayerUtils::estimateMaxSymbolBleed( mFillLineSymbol.get(), context.renderContext() );
  left = transformedBounds.left() - buffer * 2;
  top = transformedBounds.top() - buffer * 2;
  right = transformedBounds.right() + buffer * 2;
  bottom = transformedBounds.bottom() + buffer * 2;
  invertedRotateTransform = transform.inverted();

  if ( !applyBrushTransform )
  {
    top -= transformedBounds.top() - ( outputPixelDistance * std::floor( transformedBounds.top() / outputPixelDistance ) );
  }

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), scope );
  const bool needsExpressionContext = mFillLineSymbol->hasDataDefinedProperties();

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  int currentLine = 0;
  for ( double currentY = top; currentY <= bottom; currentY += outputPixelDistance )
  {
    if ( context.renderContext().renderingStopped() )
      break;

    if ( needsExpressionContext )
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_line_number" ), ++currentLine, true ) );

    double x1 = left;
    double y1 = currentY;
    double x2 = left;
    double y2 = currentY;
    invertedRotateTransform.map( left, currentY - outputPixelOffset, &x1, &y1 );
    invertedRotateTransform.map( right, currentY - outputPixelOffset, &x2, &y2 );

    if ( shapeEngine )
    {
      QgsLineString ls( QgsPoint( x1, y1 ), QgsPoint( x2, y2 ) );
      std::unique_ptr< QgsAbstractGeometry > intersection( shapeEngine->intersection( &ls ) );
      for ( auto it = intersection->const_parts_begin(); it != intersection->const_parts_end(); ++it )
      {
        if ( const QgsLineString *ls = qgsgeometry_cast< const QgsLineString * >( *it ) )
        {
          mFillLineSymbol->renderPolyline( ls->asQPolygonF(), context.feature(), context.renderContext() );
        }
      }
    }
    else
    {
      mFillLineSymbol->renderPolyline( QPolygonF() << QPointF( x1, y1 ) << QPointF( x2, y2 ), context.feature(), context.renderContext() );
    }
  }

  p->restore();

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
}

QVariantMap QgsLinePatternFillSymbolLayer::properties() const
{
  QVariantMap map = QgsImageFillSymbolLayer::properties();
  map.insert( QStringLiteral( "angle" ), QString::number( mLineAngle ) );
  map.insert( QStringLiteral( "distance" ), QString::number( mDistance ) );
  map.insert( QStringLiteral( "line_width" ), QString::number( mLineWidth ) );
  map.insert( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  map.insert( QStringLiteral( "offset" ), QString::number( mOffset ) );
  map.insert( QStringLiteral( "distance_unit" ), QgsUnitTypes::encodeUnit( mDistanceUnit ) );
  map.insert( QStringLiteral( "line_width_unit" ), QgsUnitTypes::encodeUnit( mLineWidthUnit ) );
  map.insert( QStringLiteral( "offset_unit" ), QgsUnitTypes::encodeUnit( mOffsetUnit ) );
  map.insert( QStringLiteral( "distance_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceMapUnitScale ) );
  map.insert( QStringLiteral( "line_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mLineWidthMapUnitScale ) );
  map.insert( QStringLiteral( "offset_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale ) );
  map.insert( QStringLiteral( "outline_width_unit" ), QgsUnitTypes::encodeUnit( mStrokeWidthUnit ) );
  map.insert( QStringLiteral( "outline_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale ) );
  map.insert( QStringLiteral( "clip_mode" ), QgsSymbolLayerUtils::encodeLineClipMode( mClipMode ) );
  return map;
}

QgsLinePatternFillSymbolLayer *QgsLinePatternFillSymbolLayer::clone() const
{
  QgsLinePatternFillSymbolLayer *clonedLayer = static_cast<QgsLinePatternFillSymbolLayer *>( QgsLinePatternFillSymbolLayer::create( properties() ) );
  if ( mFillLineSymbol )
  {
    clonedLayer->setSubSymbol( mFillLineSymbol->clone() );
  }
  copyPaintEffect( clonedLayer );
  copyDataDefinedProperties( clonedLayer );
  return clonedLayer;
}

void QgsLinePatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
  symbolizerElem.appendChild( fillElem );

  QDomElement graphicFillElem = doc.createElement( QStringLiteral( "se:GraphicFill" ) );
  fillElem.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  graphicFillElem.appendChild( graphicElem );

  //line properties must be inside the graphic definition
  QColor lineColor = mFillLineSymbol ? mFillLineSymbol->color() : QColor();
  double lineWidth = mFillLineSymbol ? mFillLineSymbol->width() : 0.0;
  lineWidth = QgsSymbolLayerUtils::rescaleUom( lineWidth, mLineWidthUnit,  props );
  double distance = QgsSymbolLayerUtils::rescaleUom( mDistance, mDistanceUnit,  props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, QStringLiteral( "horline" ), QColor(), lineColor, Qt::SolidLine, lineWidth, distance );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ).toString() ).arg( mLineAngle );
  }
  else if ( !qgsDoubleNear( angle + mLineAngle, 0.0 ) )
  {
    angleFunc = QString::number( angle + mLineAngle );
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <se:Displacement>
  QPointF lineOffset( std::sin( mLineAngle ) * mOffset, std::cos( mLineAngle ) * mOffset );
  lineOffset = QgsSymbolLayerUtils::rescaleUom( lineOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, lineOffset );
}

QString QgsLinePatternFillSymbolLayer::ogrFeatureStyleWidth( double widthScaleFactor ) const
{
  QString featureStyle;
  featureStyle.append( "Brush(" );
  featureStyle.append( QStringLiteral( "fc:%1" ).arg( mColor.name() ) );
  featureStyle.append( QStringLiteral( ",bc:%1" ).arg( QLatin1String( "#00000000" ) ) ); //transparent background
  featureStyle.append( ",id:\"ogr-brush-2\"" );
  featureStyle.append( QStringLiteral( ",a:%1" ).arg( mLineAngle ) );
  featureStyle.append( QStringLiteral( ",s:%1" ).arg( mLineWidth * widthScaleFactor ) );
  featureStyle.append( ",dx:0mm" );
  featureStyle.append( QStringLiteral( ",dy:%1mm" ).arg( mDistance * widthScaleFactor ) );
  featureStyle.append( ')' );
  return featureStyle;
}

void QgsLinePatternFillSymbolLayer::applyDataDefinedSettings( QgsSymbolRenderContext &context )
{
  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineAngle ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineDistance )
       && ( !mFillLineSymbol || !mFillLineSymbol->hasDataDefinedProperties() ) )
  {
    return; //no data defined settings
  }

  double lineAngle = mLineAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineAngle ) )
  {
    context.setOriginalValueVariable( mLineAngle );
    lineAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineAngle, context.renderContext().expressionContext(), mLineAngle );
  }
  double distance = mDistance;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyLineDistance ) )
  {
    context.setOriginalValueVariable( mDistance );
    distance = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyLineDistance, context.renderContext().expressionContext(), mDistance );
  }
  applyPattern( context, mBrush, lineAngle, distance );
}

QgsSymbolLayer *QgsLinePatternFillSymbolLayer::createFromSld( QDomElement &element )
{
  QString name;
  QColor fillColor, lineColor;
  double size, lineWidth;
  Qt::PenStyle lineStyle;

  QDomElement fillElem = element.firstChildElement( QStringLiteral( "Fill" ) );
  if ( fillElem.isNull() )
    return nullptr;

  QDomElement graphicFillElem = fillElem.firstChildElement( QStringLiteral( "GraphicFill" ) );
  if ( graphicFillElem.isNull() )
    return nullptr;

  QDomElement graphicElem = graphicFillElem.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  if ( !QgsSymbolLayerUtils::wellKnownMarkerFromSld( graphicElem, name, fillColor, lineColor, lineStyle, lineWidth, size ) )
    return nullptr;

  if ( name != QLatin1String( "horline" ) )
    return nullptr;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  double offset = 0.0;
  QPointF vectOffset;
  if ( QgsSymbolLayerUtils::displacementFromSldElement( graphicElem, vectOffset ) )
  {
    offset = std::sqrt( std::pow( vectOffset.x(), 2 ) + std::pow( vectOffset.y(), 2 ) );
  }

  double scaleFactor = 1.0;
  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  QgsUnitTypes::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  size = size * scaleFactor;
  lineWidth = lineWidth * scaleFactor;

  std::unique_ptr< QgsLinePatternFillSymbolLayer > sl = std::make_unique< QgsLinePatternFillSymbolLayer >();
  sl->setOutputUnit( sldUnitSize );
  sl->setColor( lineColor );
  sl->setLineWidth( lineWidth );
  sl->setLineAngle( angle );
  sl->setOffset( offset );
  sl->setDistance( size );

  // try to get the stroke
  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( !strokeElem.isNull() )
  {
    QgsSymbolLayer *l = QgsSymbolLayerUtils::createLineLayerFromSld( strokeElem );
    if ( l )
    {
      QgsSymbolLayerList layers;
      layers.append( l );
      sl->setSubSymbol( new QgsLineSymbol( layers ) );
    }
  }

  return sl.release();
}


////////////////////////

QgsPointPatternFillSymbolLayer::QgsPointPatternFillSymbolLayer()
  : QgsImageFillSymbolLayer()
{
  setSubSymbol( new QgsMarkerSymbol() );
  QgsImageFillSymbolLayer::setSubSymbol( nullptr ); //no stroke
}

QgsPointPatternFillSymbolLayer::~QgsPointPatternFillSymbolLayer() = default;

void QgsPointPatternFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mDistanceXUnit = unit;
  mDistanceYUnit = unit;
  // don't change "percentage" units -- since they adapt directly to whatever other unit is set
  if ( mDisplacementXUnit != QgsUnitTypes::RenderPercentage )
    mDisplacementXUnit = unit;
  if ( mDisplacementYUnit != QgsUnitTypes::RenderPercentage )
    mDisplacementYUnit = unit;
  if ( mOffsetXUnit != QgsUnitTypes::RenderPercentage )
    mOffsetXUnit = unit;
  if ( mOffsetYUnit != QgsUnitTypes::RenderPercentage )
    mOffsetYUnit = unit;
  if ( mRandomDeviationXUnit != QgsUnitTypes::RenderPercentage )
    mRandomDeviationXUnit = unit;
  if ( mRandomDeviationYUnit != QgsUnitTypes::RenderPercentage )
    mRandomDeviationYUnit = unit;

  if ( mMarkerSymbol )
  {
    mMarkerSymbol->setOutputUnit( unit );
  }
}

QgsUnitTypes::RenderUnit QgsPointPatternFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceXUnit != unit ||
       mDistanceYUnit != unit ||
       ( mDisplacementXUnit != QgsUnitTypes::RenderPercentage && mDisplacementXUnit != unit ) ||
       ( mDisplacementYUnit != QgsUnitTypes::RenderPercentage && mDisplacementYUnit != unit ) ||
       ( mOffsetXUnit != QgsUnitTypes::RenderPercentage && mOffsetXUnit != unit ) ||
       ( mOffsetYUnit != QgsUnitTypes::RenderPercentage && mOffsetYUnit != unit ) ||
       ( mRandomDeviationXUnit != QgsUnitTypes::RenderPercentage && mRandomDeviationXUnit != unit ) ||
       ( mRandomDeviationYUnit != QgsUnitTypes::RenderPercentage && mRandomDeviationYUnit != unit ) )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsPointPatternFillSymbolLayer::usesMapUnits() const
{
  return mDistanceXUnit == QgsUnitTypes::RenderMapUnits || mDistanceXUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mDistanceYUnit == QgsUnitTypes::RenderMapUnits || mDistanceYUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mDisplacementXUnit == QgsUnitTypes::RenderMapUnits || mDisplacementXUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mDisplacementYUnit == QgsUnitTypes::RenderMapUnits || mDisplacementYUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetXUnit == QgsUnitTypes::RenderMapUnits || mOffsetXUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetYUnit == QgsUnitTypes::RenderMapUnits || mOffsetYUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mRandomDeviationXUnit == QgsUnitTypes::RenderMapUnits || mRandomDeviationXUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mRandomDeviationYUnit == QgsUnitTypes::RenderMapUnits || mRandomDeviationYUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsPointPatternFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsImageFillSymbolLayer::setMapUnitScale( scale );
  mDistanceXMapUnitScale = scale;
  mDistanceYMapUnitScale = scale;
  mDisplacementXMapUnitScale = scale;
  mDisplacementYMapUnitScale = scale;
  mOffsetXMapUnitScale = scale;
  mOffsetYMapUnitScale = scale;
  mRandomDeviationXMapUnitScale = scale;
  mRandomDeviationYMapUnitScale = scale;
}

QgsMapUnitScale QgsPointPatternFillSymbolLayer::mapUnitScale() const
{
  if ( QgsImageFillSymbolLayer::mapUnitScale() == mDistanceXMapUnitScale &&
       mDistanceXMapUnitScale == mDistanceYMapUnitScale &&
       mDistanceYMapUnitScale == mDisplacementXMapUnitScale &&
       mDisplacementXMapUnitScale == mDisplacementYMapUnitScale &&
       mDisplacementYMapUnitScale == mOffsetXMapUnitScale &&
       mOffsetXMapUnitScale == mOffsetYMapUnitScale &&
       mRandomDeviationXMapUnitScale == mOffsetYMapUnitScale &&
       mRandomDeviationYMapUnitScale == mRandomDeviationXMapUnitScale )
  {
    return mDistanceXMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsPointPatternFillSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsPointPatternFillSymbolLayer > layer = std::make_unique< QgsPointPatternFillSymbolLayer >();
  if ( properties.contains( QStringLiteral( "distance_x" ) ) )
  {
    layer->setDistanceX( properties[QStringLiteral( "distance_x" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "distance_y" ) ) )
  {
    layer->setDistanceY( properties[QStringLiteral( "distance_y" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "displacement_x" ) ) )
  {
    layer->setDisplacementX( properties[QStringLiteral( "displacement_x" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "displacement_y" ) ) )
  {
    layer->setDisplacementY( properties[QStringLiteral( "displacement_y" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_x" ) ) )
  {
    layer->setOffsetX( properties[QStringLiteral( "offset_x" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "offset_y" ) ) )
  {
    layer->setOffsetY( properties[QStringLiteral( "offset_y" )].toDouble() );
  }

  if ( properties.contains( QStringLiteral( "distance_x_unit" ) ) )
  {
    layer->setDistanceXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_x_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "distance_x_map_unit_scale" ) ) )
  {
    layer->setDistanceXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_x_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "distance_y_unit" ) ) )
  {
    layer->setDistanceYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_y_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "distance_y_map_unit_scale" ) ) )
  {
    layer->setDistanceYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_y_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_x_unit" ) ) )
  {
    layer->setDisplacementXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "displacement_x_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_x_map_unit_scale" ) ) )
  {
    layer->setDisplacementXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "displacement_x_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_y_unit" ) ) )
  {
    layer->setDisplacementYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "displacement_y_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_y_map_unit_scale" ) ) )
  {
    layer->setDisplacementYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "displacement_y_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_x_unit" ) ) )
  {
    layer->setOffsetXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_x_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_x_map_unit_scale" ) ) )
  {
    layer->setOffsetXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_x_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_y_unit" ) ) )
  {
    layer->setOffsetYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_y_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_y_map_unit_scale" ) ) )
  {
    layer->setOffsetYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_y_map_unit_scale" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "random_deviation_x" ) ) )
  {
    layer->setMaximumRandomDeviationX( properties[QStringLiteral( "random_deviation_x" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "random_deviation_y" ) ) )
  {
    layer->setMaximumRandomDeviationY( properties[QStringLiteral( "random_deviation_y" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "random_deviation_x_unit" ) ) )
  {
    layer->setRandomDeviationXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "random_deviation_x_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "random_deviation_x_map_unit_scale" ) ) )
  {
    layer->setRandomDeviationXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "random_deviation_x_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "random_deviation_y_unit" ) ) )
  {
    layer->setRandomDeviationYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "random_deviation_y_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "random_deviation_y_map_unit_scale" ) ) )
  {
    layer->setRandomDeviationYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "random_deviation_y_map_unit_scale" )].toString() ) );
  }
  unsigned long seed = 0;
  if ( properties.contains( QStringLiteral( "seed" ) ) )
    seed = properties.value( QStringLiteral( "seed" ) ).toUInt();
  else
  {
    // if we a creating a new point pattern fill from scratch, we default to a random seed
    // because seed based fills are just nicer for users vs seeing points jump around with every map refresh
    std::random_device rd;
    std::mt19937 mt( seed == 0 ? rd() : seed );
    std::uniform_int_distribution<> uniformDist( 1, 999999999 );
    seed = uniformDist( mt );
  }
  layer->setSeed( seed );

  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    layer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "clip_mode" ) ) )
  {
    layer->setClipMode( QgsSymbolLayerUtils::decodeMarkerClipMode( properties.value( QStringLiteral( "clip_mode" ) ).toString() ) );
  }
  if ( properties.contains( QStringLiteral( "coordinate_reference" ) ) )
  {
    layer->setCoordinateReference( QgsSymbolLayerUtils::decodeCoordinateReference( properties[QStringLiteral( "coordinate_reference" )].toString() ) );
  }

  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    layer->setAngle( properties[QStringLiteral( "angle" )].toDouble() );
  }

  layer->restoreOldDataDefinedProperties( properties );

  return layer.release();
}

QString QgsPointPatternFillSymbolLayer::layerType() const
{
  return QStringLiteral( "PointPatternFill" );
}

void QgsPointPatternFillSymbolLayer::applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double distanceX, double distanceY,
    double displacementX, double displacementY, double offsetX, double offsetY )
{
  //render 3 rows and columns in one go to easily incorporate displacement
  const QgsRenderContext &ctx = context.renderContext();
  double width = ctx.convertToPainterUnits( distanceX, mDistanceXUnit, mDistanceXMapUnitScale ) * 2.0;
  double height = ctx.convertToPainterUnits( distanceY, mDistanceYUnit, mDisplacementYMapUnitScale ) * 2.0;

  double widthOffset = std::fmod(
                         mOffsetXUnit == QgsUnitTypes::RenderPercentage ? ( width * offsetX / 200 ) : ctx.convertToPainterUnits( offsetX, mOffsetXUnit, mOffsetXMapUnitScale ),
                         width );
  double heightOffset = std::fmod(
                          mOffsetYUnit == QgsUnitTypes::RenderPercentage ? ( height * offsetY / 200 ) : ctx.convertToPainterUnits( offsetY, mOffsetYUnit, mOffsetYMapUnitScale ),
                          height );

  if ( width > 10000 || height > 10000 ) //protect symbol layer from eating too much memory
  {
    QImage img;
    brush.setTextureImage( img );
    return;
  }

  QImage patternImage( width, height, QImage::Format_ARGB32 );
  patternImage.fill( 0 );
  if ( patternImage.isNull() )
  {
    brush.setTextureImage( QImage() );
    return;
  }
  if ( mMarkerSymbol )
  {
    QPainter p( &patternImage );

    //marker rendering needs context for drawing on patternImage
    QgsRenderContext pointRenderContext;
    pointRenderContext.setRendererScale( context.renderContext().rendererScale() );
    pointRenderContext.setPainter( &p );
    pointRenderContext.setScaleFactor( context.renderContext().scaleFactor() );

    context.renderContext().setPainterFlagsUsingContext( &p );
    QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() );
    pointRenderContext.setMapToPixel( mtp );
    pointRenderContext.setForceVectorOutput( false );
    pointRenderContext.setExpressionContext( context.renderContext().expressionContext() );
    pointRenderContext.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

    mMarkerSymbol->startRender( pointRenderContext, context.fields() );

    //render points on distance grid
    for ( double currentX = -width; currentX <= width * 2.0; currentX += width )
    {
      for ( double currentY = -height; currentY <= height * 2.0; currentY += height )
      {
        mMarkerSymbol->renderPoint( QPointF( currentX + widthOffset, currentY + heightOffset ), context.feature(), pointRenderContext );
      }
    }

    //render displaced points
    double displacementPixelX = mDisplacementXUnit == QgsUnitTypes::RenderPercentage
                                ? ( width * displacementX / 200 )
                                : ctx.convertToPainterUnits( displacementX, mDisplacementXUnit, mDisplacementXMapUnitScale );
    double displacementPixelY =  mDisplacementYUnit == QgsUnitTypes::RenderPercentage
                                 ? ( height * displacementY / 200 )
                                 : ctx.convertToPainterUnits( displacementY, mDisplacementYUnit, mDisplacementYMapUnitScale );
    for ( double currentX = -width; currentX <= width * 2.0; currentX += width )
    {
      for ( double currentY = -height / 2.0; currentY <= height * 2.0; currentY += height )
      {
        mMarkerSymbol->renderPoint( QPointF( currentX + widthOffset + displacementPixelX, currentY + heightOffset ), context.feature(), pointRenderContext );
      }
    }

    for ( double currentX = -width / 2.0; currentX <= width * 2.0; currentX += width )
    {
      for ( double currentY = -height; currentY <= height * 2.0; currentY += height / 2.0 )
      {
        mMarkerSymbol->renderPoint( QPointF( currentX + widthOffset + ( std::fmod( currentY, height ) != 0 ? displacementPixelX : 0 ), currentY + heightOffset - displacementPixelY ), context.feature(), pointRenderContext );
      }
    }

    mMarkerSymbol->stopRender( pointRenderContext );
  }

  if ( !qgsDoubleNear( context.opacity(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerUtils::multiplyImageOpacity( &transparentImage, context.opacity() );
    brush.setTextureImage( transparentImage );
  }
  else
  {
    brush.setTextureImage( patternImage );
  }
  QTransform brushTransform;
  brush.setTransform( brushTransform );
}

void QgsPointPatternFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  // if we are using a vector based output, we need to render points as vectors
  // (OR if the marker has data defined symbology, in which case we need to evaluate this point-by-point)
  mRenderUsingMarkers = context.renderContext().forceVectorOutput()
                        || mMarkerSymbol->hasDataDefinedProperties()
                        || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyMarkerClipping )
                        || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomOffsetX )
                        || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomOffsetY )
                        || mClipMode != Qgis::MarkerClipMode::Shape
                        || !qgsDoubleNear( mRandomDeviationX, 0 )
                        || !qgsDoubleNear( mRandomDeviationY, 0 )
                        || !qgsDoubleNear( mAngle, 0 )
                        || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );

  if ( mRenderUsingMarkers )
  {
    mMarkerSymbol->startRender( context.renderContext() );
  }
  else
  {
    // optimised render for screen only, use image based brush
    applyPattern( context, mBrush, mDistanceX, mDistanceY, mDisplacementX, mDisplacementY, mOffsetX, mOffsetY );
  }
}

void QgsPointPatternFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mRenderUsingMarkers )
  {
    mMarkerSymbol->stopRender( context.renderContext() );
  }
}

void QgsPointPatternFillSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
  // Otherwise generators used in the subsymbol will only render a single point per feature (they
  // have logic to only render once per paired call to startFeatureRender/stopFeatureRender).
}

void QgsPointPatternFillSymbolLayer::stopFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  // The base class version passes this on to the subsymbol, but we deliberately don't do that here.
  // Otherwise generators used in the subsymbol will only render a single point per feature (they
  // have logic to only render once per paired call to startFeatureRender/stopFeatureRender).
}

void QgsPointPatternFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  if ( !mRenderUsingMarkers )
  {
    // use image based brush for speed
    QgsImageFillSymbolLayer::renderPolygon( points, rings, context );
    return;
  }

  // vector based output - so draw dot by dot!
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  double angle = mAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), angle );
  }

  double distanceX = mDistanceX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceX ) )
  {
    context.setOriginalValueVariable( mDistanceX );
    distanceX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDistanceX, context.renderContext().expressionContext(), mDistanceX );
  }
  const double width = context.renderContext().convertToPainterUnits( distanceX, mDistanceXUnit, mDistanceXMapUnitScale );

  double distanceY = mDistanceY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceY ) )
  {
    context.setOriginalValueVariable( mDistanceY );
    distanceY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDistanceY, context.renderContext().expressionContext(), mDistanceY );
  }
  const double height = context.renderContext().convertToPainterUnits( distanceY, mDistanceYUnit, mDistanceYMapUnitScale );

  double offsetX = mOffsetX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetX ) )
  {
    context.setOriginalValueVariable( mOffsetX );
    offsetX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetX, context.renderContext().expressionContext(), mOffsetX );
  }
  const double widthOffset = std::fmod(
                               mOffsetXUnit == QgsUnitTypes::RenderPercentage
                               ? ( offsetX * width / 100 )
                               : context.renderContext().convertToPainterUnits( offsetX, mOffsetXUnit, mOffsetXMapUnitScale ),
                               width );

  double offsetY = mOffsetY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetY ) )
  {
    context.setOriginalValueVariable( mOffsetY );
    offsetY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetY, context.renderContext().expressionContext(), mOffsetY );
  }
  const double heightOffset = std::fmod(
                                mOffsetYUnit == QgsUnitTypes::RenderPercentage
                                ? ( offsetY * height / 100 )
                                : context.renderContext().convertToPainterUnits( offsetY, mOffsetYUnit, mOffsetYMapUnitScale ),
                                height );

  double displacementX = mDisplacementX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementX ) )
  {
    context.setOriginalValueVariable( mDisplacementX );
    displacementX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDisplacementX, context.renderContext().expressionContext(), mDisplacementX );
  }
  const double displacementPixelX = mDisplacementXUnit == QgsUnitTypes::RenderPercentage
                                    ? ( displacementX * width / 100 )
                                    : context.renderContext().convertToPainterUnits( displacementX, mDisplacementXUnit, mDisplacementXMapUnitScale );

  double displacementY = mDisplacementY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementY ) )
  {
    context.setOriginalValueVariable( mDisplacementY );
    displacementY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDisplacementY, context.renderContext().expressionContext(), mDisplacementY );
  }
  const double displacementPixelY = mDisplacementYUnit == QgsUnitTypes::RenderPercentage
                                    ? ( displacementY * height / 100 )
                                    : context.renderContext().convertToPainterUnits( displacementY, mDisplacementYUnit, mDisplacementYMapUnitScale );

  p->setPen( QPen( Qt::NoPen ) );

  if ( context.selected() )
  {
    QColor selColor = context.renderContext().selectionColor();
    p->setBrush( QBrush( selColor ) );
    _renderPolygon( p, points, rings, context );
  }

  // if invalid parameters, skip out
  if ( qgsDoubleNear( width, 0 ) || qgsDoubleNear( height, 0 ) || width < 0 || height < 0 )
    return;

  p->save();

  Qgis::MarkerClipMode clipMode = mClipMode;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyMarkerClipping ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeMarkerClipMode( clipMode ) );
    bool ok = false;
    const QString valueString = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyMarkerClipping, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      Qgis::MarkerClipMode decodedMode = QgsSymbolLayerUtils::decodeMarkerClipMode( valueString, &ok );
      if ( ok )
        clipMode = decodedMode;
    }
  }

  std::unique_ptr< QgsPolygon > shapePolygon;
  std::unique_ptr< QgsGeometryEngine > shapeEngine;
  switch ( clipMode )
  {
    case Qgis::MarkerClipMode::NoClipping:
    case Qgis::MarkerClipMode::CentroidWithin:
    case Qgis::MarkerClipMode::CompletelyWithin:
    {
      shapePolygon = std::make_unique< QgsPolygon >();
      shapePolygon->setExteriorRing( QgsLineString::fromQPolygonF( points ) );
      if ( rings )
      {
        for ( const QPolygonF &ring : *rings )
        {
          shapePolygon->addInteriorRing( QgsLineString::fromQPolygonF( ring ) );
        }
      }
      shapeEngine.reset( QgsGeometry::createGeometryEngine( shapePolygon.get() ) );
      shapeEngine->prepareGeometry();
      break;
    }

    case Qgis::MarkerClipMode::Shape:
    {
      QPainterPath path;
      path.addPolygon( points );
      if ( rings )
      {
        for ( const QPolygonF &ring : *rings )
        {
          path.addPolygon( ring );
        }
      }
      p->setClipPath( path, Qt::IntersectClip );
      break;
    }
  }

  const bool applyBrushTransform = applyBrushTransformFromContext( &context );
  const QRectF boundingRect = points.boundingRect();

  QTransform invertedRotateTransform;
  double left;
  double top;
  double right;
  double bottom;

  if ( !qgsDoubleNear( angle, 0 ) )
  {
    QTransform transform;
    if ( applyBrushTransform )
    {
      // rotation applies around center of feature
      transform.translate( -boundingRect.center().x(),
                           -boundingRect.center().y() );
      transform.rotate( -angle );
      transform.translate( boundingRect.center().x(),
                           boundingRect.center().y() );
    }
    else
    {
      // rotation applies around top of viewport
      transform.rotate( -angle );
    }

    const QRectF transformedBounds = transform.map( points ).boundingRect();
    left = transformedBounds.left() - 2 * width;
    top = transformedBounds.top() - 2 * height;
    right = transformedBounds.right() + 2 * width;
    bottom = transformedBounds.bottom() + 2 * height;
    invertedRotateTransform = transform.inverted();

    if ( !applyBrushTransform )
    {
      left -= transformedBounds.left() - ( width * std::floor( transformedBounds.left() / width ) );
      top -= transformedBounds.top() - ( height * std::floor( transformedBounds.top() / height ) );
    }
  }
  else
  {
    left = boundingRect.left() - 2 * width;
    top = boundingRect.top() - 2 * height;
    right = boundingRect.right() + 2 * width;
    bottom = boundingRect.bottom() + 2 * height;

    if ( !applyBrushTransform )
    {
      left -= boundingRect.left() - ( width * std::floor( boundingRect.left() / width ) );
      top -= boundingRect.top() - ( height * std::floor( boundingRect.top() / height ) );
    }
  }

  unsigned long seed = mSeed;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomSeed ) )
  {
    context.renderContext().expressionContext().setOriginalValueVariable( static_cast< unsigned long long >( seed ) );
    seed = mDataDefinedProperties.valueAsInt( QgsSymbolLayer::PropertyRandomSeed, context.renderContext().expressionContext(), seed );
  }

  double maxRandomDeviationX = mRandomDeviationX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomOffsetX ) )
  {
    context.setOriginalValueVariable( maxRandomDeviationX );
    maxRandomDeviationX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyRandomOffsetX, context.renderContext().expressionContext(), maxRandomDeviationX );
  }
  const double maxRandomDeviationPixelX = mRandomDeviationXUnit == QgsUnitTypes::RenderPercentage ? ( maxRandomDeviationX * width / 100 )
                                          : context.renderContext().convertToPainterUnits( maxRandomDeviationX, mRandomDeviationXUnit, mRandomDeviationXMapUnitScale );

  double maxRandomDeviationY = mRandomDeviationY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomOffsetY ) )
  {
    context.setOriginalValueVariable( maxRandomDeviationY );
    maxRandomDeviationY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyRandomOffsetY, context.renderContext().expressionContext(), maxRandomDeviationY );
  }
  const double maxRandomDeviationPixelY = mRandomDeviationYUnit == QgsUnitTypes::RenderPercentage ? ( maxRandomDeviationY * height / 100 )
                                          : context.renderContext().convertToPainterUnits( maxRandomDeviationY, mRandomDeviationYUnit, mRandomDeviationYMapUnitScale );

  std::random_device rd;
  std::mt19937 mt( seed == 0 ? rd() : seed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );
  const bool useRandomShift = !qgsDoubleNear( maxRandomDeviationPixelX, 0 ) || !qgsDoubleNear( maxRandomDeviationPixelY, 0 );

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.renderContext().expressionContext(), scope );
  int pointNum = 0;
  const bool needsExpressionContext = mMarkerSymbol->hasDataDefinedProperties();

  const bool prevIsSubsymbol = context.renderContext().flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  bool alternateColumn = false;
  int currentCol = -3; // because we actually render a few rows/cols outside the bounds, try to align the col/row numbers to start at 1 for the first visible row/col
  for ( double currentX = left; currentX <= right; currentX += width, alternateColumn = !alternateColumn )
  {
    if ( context.renderContext().renderingStopped() )
      break;

    if ( needsExpressionContext )
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_marker_column" ), ++currentCol, true ) );

    bool alternateRow = false;
    const double columnX = currentX + widthOffset;
    int currentRow = -3;
    for ( double currentY = top; currentY <= bottom; currentY += height, alternateRow = !alternateRow )
    {
      if ( context.renderContext().renderingStopped() )
        break;

      double y = currentY + heightOffset;
      double x = columnX;
      if ( alternateRow )
        x += displacementPixelX;

      if ( !alternateColumn )
        y -= displacementPixelY;

      if ( !qgsDoubleNear( angle, 0 ) )
      {
        double xx = x;
        double yy = y;
        invertedRotateTransform.map( xx, yy, &x, &y );
      }

      if ( useRandomShift )
      {
        x += ( 2 * uniformDist( mt ) - 1 ) * maxRandomDeviationPixelX;
        y += ( 2 * uniformDist( mt ) - 1 ) * maxRandomDeviationPixelY;
      }

      if ( needsExpressionContext )
      {
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
        scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_marker_row" ), ++currentRow, true ) );
      }

      if ( shapeEngine )
      {
        bool renderPoint = true;
        switch ( clipMode )
        {
          case Qgis::MarkerClipMode::CentroidWithin:
          {
            // we test using the marker bounds here and NOT just the x,y point, as the marker symbol may have offsets or other data defined properties which affect its visual placement
            const QgsRectangle markerRect = QgsRectangle( mMarkerSymbol->bounds( QPointF( x, y ), context.renderContext(), context.feature() ? *context.feature() : QgsFeature() ) );
            QgsPoint p( markerRect.center() );
            renderPoint = shapeEngine->intersects( &p );
            break;
          }

          case Qgis::MarkerClipMode::NoClipping:
          case Qgis::MarkerClipMode::CompletelyWithin:
          {
            const QgsGeometry markerBounds = QgsGeometry::fromRect( QgsRectangle( mMarkerSymbol->bounds( QPointF( x, y ), context.renderContext(), context.feature() ? *context.feature() : QgsFeature() ) ) );

            if ( clipMode == Qgis::MarkerClipMode::CompletelyWithin )
              renderPoint = shapeEngine->contains( markerBounds.constGet() );
            else
              renderPoint = shapeEngine->intersects( markerBounds.constGet() );
            break;
          }

          case Qgis::MarkerClipMode::Shape:
            break;
        }

        if ( !renderPoint )
          continue;
      }

      mMarkerSymbol->renderPoint( QPointF( x, y ), context.feature(), context.renderContext() );
    }
  }

  p->restore();

  context.renderContext().setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );
}

QVariantMap QgsPointPatternFillSymbolLayer::properties() const
{
  QVariantMap map = QgsImageFillSymbolLayer::properties();
  map.insert( QStringLiteral( "distance_x" ), QString::number( mDistanceX ) );
  map.insert( QStringLiteral( "distance_y" ), QString::number( mDistanceY ) );
  map.insert( QStringLiteral( "displacement_x" ), QString::number( mDisplacementX ) );
  map.insert( QStringLiteral( "displacement_y" ), QString::number( mDisplacementY ) );
  map.insert( QStringLiteral( "offset_x" ), QString::number( mOffsetX ) );
  map.insert( QStringLiteral( "offset_y" ), QString::number( mOffsetY ) );
  map.insert( QStringLiteral( "distance_x_unit" ), QgsUnitTypes::encodeUnit( mDistanceXUnit ) );
  map.insert( QStringLiteral( "distance_y_unit" ), QgsUnitTypes::encodeUnit( mDistanceYUnit ) );
  map.insert( QStringLiteral( "displacement_x_unit" ), QgsUnitTypes::encodeUnit( mDisplacementXUnit ) );
  map.insert( QStringLiteral( "displacement_y_unit" ), QgsUnitTypes::encodeUnit( mDisplacementYUnit ) );
  map.insert( QStringLiteral( "offset_x_unit" ), QgsUnitTypes::encodeUnit( mOffsetXUnit ) );
  map.insert( QStringLiteral( "offset_y_unit" ), QgsUnitTypes::encodeUnit( mOffsetYUnit ) );
  map.insert( QStringLiteral( "distance_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceXMapUnitScale ) );
  map.insert( QStringLiteral( "distance_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceYMapUnitScale ) );
  map.insert( QStringLiteral( "displacement_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDisplacementXMapUnitScale ) );
  map.insert( QStringLiteral( "displacement_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDisplacementYMapUnitScale ) );
  map.insert( QStringLiteral( "offset_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetXMapUnitScale ) );
  map.insert( QStringLiteral( "offset_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetYMapUnitScale ) );
  map.insert( QStringLiteral( "outline_width_unit" ), QgsUnitTypes::encodeUnit( mStrokeWidthUnit ) );
  map.insert( QStringLiteral( "outline_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale ) );
  map.insert( QStringLiteral( "clip_mode" ), QgsSymbolLayerUtils::encodeMarkerClipMode( mClipMode ) );
  map.insert( QStringLiteral( "random_deviation_x" ), QString::number( mRandomDeviationX ) );
  map.insert( QStringLiteral( "random_deviation_y" ), QString::number( mRandomDeviationY ) );
  map.insert( QStringLiteral( "random_deviation_x_unit" ), QgsUnitTypes::encodeUnit( mRandomDeviationXUnit ) );
  map.insert( QStringLiteral( "random_deviation_y_unit" ), QgsUnitTypes::encodeUnit( mRandomDeviationYUnit ) );
  map.insert( QStringLiteral( "random_deviation_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mRandomDeviationXMapUnitScale ) );
  map.insert( QStringLiteral( "random_deviation_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mRandomDeviationYMapUnitScale ) );
  map.insert( QStringLiteral( "seed" ), QString::number( mSeed ) );
  map.insert( QStringLiteral( "angle" ), mAngle );
  return map;
}

QgsPointPatternFillSymbolLayer *QgsPointPatternFillSymbolLayer::clone() const
{
  QgsPointPatternFillSymbolLayer *clonedLayer = static_cast<QgsPointPatternFillSymbolLayer *>( QgsPointPatternFillSymbolLayer::create( properties() ) );
  if ( mMarkerSymbol )
  {
    clonedLayer->setSubSymbol( mMarkerSymbol->clone() );
  }
  clonedLayer->setClipMode( mClipMode );
  copyDataDefinedProperties( clonedLayer );
  copyPaintEffect( clonedLayer );
  return clonedLayer;
}

void QgsPointPatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  for ( int i = 0; i < mMarkerSymbol->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
    if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
      symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

    QDomElement fillElem = doc.createElement( QStringLiteral( "se:Fill" ) );
    symbolizerElem.appendChild( fillElem );

    QDomElement graphicFillElem = doc.createElement( QStringLiteral( "se:GraphicFill" ) );
    fillElem.appendChild( graphicFillElem );

    // store distanceX, distanceY, displacementX, displacementY in a <VendorOption>
    double dx  = QgsSymbolLayerUtils::rescaleUom( mDistanceX, mDistanceXUnit, props );
    double dy  = QgsSymbolLayerUtils::rescaleUom( mDistanceY, mDistanceYUnit, props );
    QString dist = QgsSymbolLayerUtils::encodePoint( QPointF( dx, dy ) );
    QDomElement distanceElem = QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "distance" ), dist );
    symbolizerElem.appendChild( distanceElem );

    QgsSymbolLayer *layer = mMarkerSymbol->symbolLayer( i );
    if ( QgsMarkerSymbolLayer *markerLayer = dynamic_cast<QgsMarkerSymbolLayer *>( layer ) )
    {
      markerLayer->writeSldMarker( doc, graphicFillElem, props );
    }
    else if ( layer )
    {
      QString errorMsg = QStringLiteral( "QgsMarkerSymbolLayer expected, %1 found. Skip it." ).arg( layer->layerType() );
      graphicFillElem.appendChild( doc.createComment( errorMsg ) );
    }
    else
    {
      QString errorMsg = QStringLiteral( "Missing point pattern symbol layer. Skip it." );
      graphicFillElem.appendChild( doc.createComment( errorMsg ) );
    }
  }
}

QgsSymbolLayer *QgsPointPatternFillSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element )
  return nullptr;
}

bool QgsPointPatternFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == Qgis::SymbolType::Marker )
  {
    QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( symbol );
    mMarkerSymbol.reset( markerSymbol );
  }
  return true;
}

QgsSymbol *QgsPointPatternFillSymbolLayer::subSymbol()
{
  return mMarkerSymbol.get();
}

void QgsPointPatternFillSymbolLayer::applyDataDefinedSettings( QgsSymbolRenderContext &context )
{
  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceX ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceY )
       && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementX ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementY )
       && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetX ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetY )
       && ( !mMarkerSymbol || !mMarkerSymbol->hasDataDefinedProperties() ) )
  {
    return;
  }

  double distanceX = mDistanceX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceX ) )
  {
    context.setOriginalValueVariable( mDistanceX );
    distanceX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDistanceX, context.renderContext().expressionContext(), mDistanceX );
  }
  double distanceY = mDistanceY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceY ) )
  {
    context.setOriginalValueVariable( mDistanceY );
    distanceY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDistanceY, context.renderContext().expressionContext(), mDistanceY );
  }
  double displacementX = mDisplacementX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementX ) )
  {
    context.setOriginalValueVariable( mDisplacementX );
    displacementX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDisplacementX, context.renderContext().expressionContext(), mDisplacementX );
  }
  double displacementY = mDisplacementY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementY ) )
  {
    context.setOriginalValueVariable( mDisplacementY );
    displacementY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyDisplacementY, context.renderContext().expressionContext(), mDisplacementY );
  }
  double offsetX = mOffsetX;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetX ) )
  {
    context.setOriginalValueVariable( mOffsetX );
    offsetX = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetX, context.renderContext().expressionContext(), mOffsetX );
  }
  double offsetY = mOffsetY;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetY ) )
  {
    context.setOriginalValueVariable( mOffsetY );
    offsetY = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetY, context.renderContext().expressionContext(), mOffsetY );
  }
  applyPattern( context, mBrush, distanceX, distanceY, displacementX, displacementY, offsetX, offsetY );
}

double QgsPointPatternFillSymbolLayer::estimateMaxBleed( const QgsRenderContext & ) const
{
  return 0;
}

QSet<QString> QgsPointPatternFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsImageFillSymbolLayer::usedAttributes( context );

  if ( mMarkerSymbol )
    attributes.unite( mMarkerSymbol->usedAttributes( context ) );

  return attributes;
}

bool QgsPointPatternFillSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsImageFillSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mMarkerSymbol && mMarkerSymbol->hasDataDefinedProperties() )
    return true;
  return false;
}

void QgsPointPatternFillSymbolLayer::setColor( const QColor &c )
{
  mColor = c;
  if ( mMarkerSymbol )
    mMarkerSymbol->setColor( c );
}

QColor QgsPointPatternFillSymbolLayer::color() const
{
  return mMarkerSymbol ? mMarkerSymbol->color() : mColor;
}

//////////////


QgsCentroidFillSymbolLayer::QgsCentroidFillSymbolLayer()
{
  setSubSymbol( new QgsMarkerSymbol() );
}

QgsCentroidFillSymbolLayer::~QgsCentroidFillSymbolLayer() = default;

QgsSymbolLayer *QgsCentroidFillSymbolLayer::create( const QVariantMap &properties )
{
  std::unique_ptr< QgsCentroidFillSymbolLayer > sl = std::make_unique< QgsCentroidFillSymbolLayer >();

  if ( properties.contains( QStringLiteral( "point_on_surface" ) ) )
    sl->setPointOnSurface( properties[QStringLiteral( "point_on_surface" )].toInt() != 0 );
  if ( properties.contains( QStringLiteral( "point_on_all_parts" ) ) )
    sl->setPointOnAllParts( properties[QStringLiteral( "point_on_all_parts" )].toInt() != 0 );
  if ( properties.contains( QStringLiteral( "clip_points" ) ) )
    sl->setClipPoints( properties[QStringLiteral( "clip_points" )].toInt() != 0 );
  if ( properties.contains( QStringLiteral( "clip_on_current_part_only" ) ) )
    sl->setClipOnCurrentPartOnly( properties[QStringLiteral( "clip_on_current_part_only" )].toInt() != 0 );

  sl->restoreOldDataDefinedProperties( properties );

  return sl.release();
}

QString QgsCentroidFillSymbolLayer::layerType() const
{
  return QStringLiteral( "CentroidFill" );
}

void QgsCentroidFillSymbolLayer::setColor( const QColor &color )
{
  mMarker->setColor( color );
  mColor = color;
}

QColor QgsCentroidFillSymbolLayer::color() const
{
  return mMarker ? mMarker->color() : mColor;
}

void QgsCentroidFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mMarker->startRender( context.renderContext(), context.fields() );
}

void QgsCentroidFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsCentroidFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  Part part;
  part.exterior = points;
  if ( rings )
    part.rings = *rings;

  if ( mRenderingFeature )
  {
    // in the middle of rendering a possibly multi-part feature, so we collect all the parts and defer the actual rendering
    // until after we've received the final part
    mFeatureSymbolOpacity = context.opacity();
    mCurrentParts << part;
  }
  else
  {
    // not rendering a feature, so we can just render the polygon immediately
    const double prevOpacity = mMarker->opacity();
    mMarker->setOpacity( mMarker->opacity() * context.opacity() );
    render( context.renderContext(), QVector<Part>() << part, context.feature() ? *context.feature() : QgsFeature(), context.selected() );
    mMarker->setOpacity( prevOpacity );
  }
}

void QgsCentroidFillSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mCurrentParts.clear();
}

void QgsCentroidFillSymbolLayer::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  mRenderingFeature = false;

  const double prevOpacity = mMarker->opacity();
  mMarker->setOpacity( mMarker->opacity() * mFeatureSymbolOpacity );

  render( context, mCurrentParts, feature, false );
  mFeatureSymbolOpacity = 1;
  mMarker->setOpacity( prevOpacity );
}

void QgsCentroidFillSymbolLayer::render( QgsRenderContext &context, const QVector<QgsCentroidFillSymbolLayer::Part> &parts, const QgsFeature &feature, bool selected )
{
  bool pointOnAllParts = mPointOnAllParts;
  bool pointOnSurface = mPointOnSurface;
  bool clipPoints = mClipPoints;
  bool clipOnCurrentPartOnly = mClipOnCurrentPartOnly;

  // TODO add expressions support

  QVector< QgsGeometry > geometryParts;
  geometryParts.reserve( parts.size() );
  QPainterPath globalPath;

  int maxArea = 0;
  int maxAreaPartIdx = 0;

  for ( int i = 0; i < parts.size(); i++ )
  {
    const Part part = parts[i];
    QgsGeometry geom = QgsGeometry::fromQPolygonF( part.exterior );

    if ( !geom.isNull() && !part.rings.empty() )
    {
      QgsPolygon *poly = qgsgeometry_cast< QgsPolygon * >( geom.get() );

      if ( !pointOnAllParts )
      {
        int area = poly->area();

        if ( area > maxArea )
        {
          maxArea = area;
          maxAreaPartIdx = i;
        }
      }
    }

    if ( clipPoints && !clipOnCurrentPartOnly )
    {
      globalPath.addPolygon( part.exterior );
      for ( const QPolygonF &ring : part.rings )
      {
        globalPath.addPolygon( ring );
      }
    }
  }

  for ( int i = 0; i < parts.size(); i++ )
  {
    if ( !pointOnAllParts && i != maxAreaPartIdx )
      continue;

    const Part part = parts[i];

    if ( clipPoints )
    {
      QPainterPath path;

      if ( clipOnCurrentPartOnly )
      {
        path.addPolygon( part.exterior );
        for ( const QPolygonF &ring : part.rings )
        {
          path.addPolygon( ring );
        }
      }
      else
      {
        path = globalPath;
      }

      context.painter()->save();
      context.painter()->setClipPath( path );
    }

    QPointF centroid = pointOnSurface ? QgsSymbolLayerUtils::polygonPointOnSurface( part.exterior, &part.rings ) : QgsSymbolLayerUtils::polygonCentroid( part.exterior );

    const bool prevIsSubsymbol = context.flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
    context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );
    mMarker->renderPoint( centroid, feature.isValid() ? &feature : nullptr, context, -1, selected );
    context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

    if ( clipPoints )
    {
      context.painter()->restore();
    }
  }
}

QVariantMap QgsCentroidFillSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "point_on_surface" )] = QString::number( mPointOnSurface );
  map[QStringLiteral( "point_on_all_parts" )] = QString::number( mPointOnAllParts );
  map[QStringLiteral( "clip_points" )] = QString::number( mClipPoints );
  map[QStringLiteral( "clip_on_current_part_only" )] = QString::number( mClipOnCurrentPartOnly );
  return map;
}

QgsCentroidFillSymbolLayer *QgsCentroidFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsCentroidFillSymbolLayer > x = std::make_unique< QgsCentroidFillSymbolLayer >();
  x->mAngle = mAngle;
  x->mColor = mColor;
  x->setSubSymbol( mMarker->clone() );
  x->setPointOnSurface( mPointOnSurface );
  x->setPointOnAllParts( mPointOnAllParts );
  x->setClipPoints( mClipPoints );
  x->setClipOnCurrentPartOnly( mClipOnCurrentPartOnly );
  copyDataDefinedProperties( x.get() );
  copyPaintEffect( x.get() );
  return x.release();
}

void QgsCentroidFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // SLD 1.0 specs says: "if a line, polygon, or raster geometry is
  // used with PointSymbolizer, then the semantic is to use the centroid
  // of the geometry, or any similar representative point.
  mMarker->toSld( doc, element, props );
}

QgsSymbolLayer *QgsCentroidFillSymbolLayer::createFromSld( QDomElement &element )
{
  QgsSymbolLayer *l = QgsSymbolLayerUtils::createMarkerLayerFromSld( element );
  if ( !l )
    return nullptr;

  QgsSymbolLayerList layers;
  layers.append( l );
  std::unique_ptr< QgsMarkerSymbol > marker( new QgsMarkerSymbol( layers ) );

  std::unique_ptr< QgsCentroidFillSymbolLayer > sl = std::make_unique< QgsCentroidFillSymbolLayer >();
  sl->setSubSymbol( marker.release() );
  sl->setPointOnAllParts( false );
  return sl.release();
}


QgsSymbol *QgsCentroidFillSymbolLayer::subSymbol()
{
  return mMarker.get();
}

bool QgsCentroidFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol || symbol->type() != Qgis::SymbolType::Marker )
  {
    delete symbol;
    return false;
  }

  mMarker.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
  mColor = mMarker->color();
  return true;
}

QSet<QString> QgsCentroidFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsFillSymbolLayer::usedAttributes( context );

  if ( mMarker )
    attributes.unite( mMarker->usedAttributes( context ) );

  return attributes;
}

bool QgsCentroidFillSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mMarker && mMarker->hasDataDefinedProperties() )
    return true;
  return false;
}

bool QgsCentroidFillSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}

void QgsCentroidFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  if ( mMarker )
  {
    mMarker->setOutputUnit( unit );
  }
}

QgsUnitTypes::RenderUnit QgsCentroidFillSymbolLayer::outputUnit() const
{
  if ( mMarker )
  {
    return mMarker->outputUnit();
  }
  return QgsUnitTypes::RenderUnknownUnit; //mOutputUnit;
}

bool QgsCentroidFillSymbolLayer::usesMapUnits() const
{
  if ( mMarker )
  {
    return mMarker->usesMapUnits();
  }
  return false;
}

void QgsCentroidFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  if ( mMarker )
  {
    mMarker->setMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsCentroidFillSymbolLayer::mapUnitScale() const
{
  if ( mMarker )
  {
    return mMarker->mapUnitScale();
  }
  return QgsMapUnitScale();
}




QgsRasterFillSymbolLayer::QgsRasterFillSymbolLayer( const QString &imageFilePath )
  : QgsImageFillSymbolLayer()
  , mImageFilePath( imageFilePath )
{
  QgsImageFillSymbolLayer::setSubSymbol( nullptr ); //disable sub symbol
  mCoordinateReference = Qgis::SymbolCoordinateReference::Viewport;
}

QgsRasterFillSymbolLayer::~QgsRasterFillSymbolLayer() = default;

QgsSymbolLayer *QgsRasterFillSymbolLayer::create( const QVariantMap &properties )
{
  Qgis::SymbolCoordinateReference mode = Qgis::SymbolCoordinateReference::Feature;
  double alpha = 1.0;
  QPointF offset;
  double angle = 0.0;
  double width = 0.0;

  QString imagePath;
  if ( properties.contains( QStringLiteral( "imageFile" ) ) )
  {
    imagePath = properties[QStringLiteral( "imageFile" )].toString();
  }
  if ( properties.contains( QStringLiteral( "coordinate_mode" ) ) )
  {
    mode = static_cast< Qgis::SymbolCoordinateReference >( properties[QStringLiteral( "coordinate_mode" )].toInt() );
  }
  if ( properties.contains( QStringLiteral( "alpha" ) ) )
  {
    alpha = properties[QStringLiteral( "alpha" )].toDouble();
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    offset = QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )].toString() );
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    angle = properties[QStringLiteral( "angle" )].toDouble();
  }
  if ( properties.contains( QStringLiteral( "width" ) ) )
  {
    width = properties[QStringLiteral( "width" )].toDouble();
  }
  std::unique_ptr< QgsRasterFillSymbolLayer > symbolLayer = std::make_unique< QgsRasterFillSymbolLayer >( imagePath );
  symbolLayer->setCoordinateMode( mode );
  symbolLayer->setOpacity( alpha );
  symbolLayer->setOffset( offset );
  symbolLayer->setAngle( angle );
  symbolLayer->setWidth( width );
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "width_unit" ) ) )
  {
    symbolLayer->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "width_map_unit_scale" ) ) )
  {
    symbolLayer->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "width_map_unit_scale" )].toString() ) );
  }

  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer.release();
}

void QgsRasterFillSymbolLayer::resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QVariantMap::iterator it = properties.find( QStringLiteral( "imageFile" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = pathResolver.writePath( it.value().toString() );
    else
      it.value() = pathResolver.readPath( it.value().toString() );
  }
}

bool QgsRasterFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  Q_UNUSED( symbol )
  return true;
}

QString QgsRasterFillSymbolLayer::layerType() const
{
  return QStringLiteral( "RasterFill" );
}

void QgsRasterFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QPointF offset = mOffset;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePoint( mOffset ) );
    const QVariant val = mDataDefinedProperties.value( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), QString() );
    bool ok = false;
    const QPointF res = QgsSymbolLayerUtils::toPoint( val, &ok );
    if ( ok )
      offset = res;
  }
  if ( !offset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( offset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( offset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }
  if ( mCoordinateMode == Qgis::SymbolCoordinateReference::Feature )
  {
    QRectF boundingRect = points.boundingRect();
    mBrush.setTransform( mBrush.transform().translate( boundingRect.left() - mBrush.transform().dx(),
                         boundingRect.top() - mBrush.transform().dy() ) );
  }

  QgsImageFillSymbolLayer::renderPolygon( points, rings, context );
  if ( !offset.isNull() )
  {
    p->translate( -offset );
  }
}

void QgsRasterFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  applyPattern( mBrush, mImageFilePath, mWidth, mOpacity * context.opacity(), context );
}

void QgsRasterFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context )
}

QVariantMap QgsRasterFillSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "imageFile" )] = mImageFilePath;
  map[QStringLiteral( "coordinate_mode" )] = QString::number( static_cast< int >( mCoordinateMode ) );
  map[QStringLiteral( "alpha" )] = QString::number( mOpacity );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "width" )] = QString::number( mWidth );
  map[QStringLiteral( "width_unit" )] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[QStringLiteral( "width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  return map;
}

QgsRasterFillSymbolLayer *QgsRasterFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsRasterFillSymbolLayer > sl = std::make_unique< QgsRasterFillSymbolLayer >( mImageFilePath );
  sl->setCoordinateMode( mCoordinateMode );
  sl->setOpacity( mOpacity );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  sl->setAngle( mAngle );
  sl->setWidth( mWidth );
  sl->setWidthUnit( mWidthUnit );
  sl->setWidthMapUnitScale( mWidthMapUnitScale );
  copyDataDefinedProperties( sl.get() );
  copyPaintEffect( sl.get() );
  return sl.release();
}

double QgsRasterFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  return context.convertToPainterUnits( std::max( std::fabs( mOffset.x() ), std::fabs( mOffset.y() ) ), mOffsetUnit, mOffsetMapUnitScale );
}

bool QgsRasterFillSymbolLayer::usesMapUnits() const
{
  return mWidthUnit == QgsUnitTypes::RenderMapUnits || mWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

QColor QgsRasterFillSymbolLayer::color() const
{
  return QColor();
}

void QgsRasterFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mOffsetUnit = unit;
  mWidthUnit = unit;
}

void QgsRasterFillSymbolLayer::setImageFilePath( const QString &imagePath )
{
  mImageFilePath = imagePath;
}

void QgsRasterFillSymbolLayer::setCoordinateMode( const Qgis::SymbolCoordinateReference mode )
{
  mCoordinateMode = mode;
}

void QgsRasterFillSymbolLayer::setOpacity( const double opacity )
{
  mOpacity = opacity;
}

void QgsRasterFillSymbolLayer::applyDataDefinedSettings( QgsSymbolRenderContext &context )
{
  if ( !dataDefinedProperties().hasActiveProperties() )
    return; // shortcut

  bool hasWidthExpression = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth );
  bool hasFileExpression = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFile );
  bool hasOpacityExpression = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOpacity );
  bool hasAngleExpression = mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle );

  if ( !hasWidthExpression && !hasAngleExpression && !hasOpacityExpression && !hasFileExpression )
  {
    return; //no data defined settings
  }

  bool ok;
  if ( hasAngleExpression )
  {
    context.setOriginalValueVariable( mAngle );
    double nextAngle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), 0, &ok );
    if ( ok )
      mNextAngle = nextAngle;
  }

  if ( !hasWidthExpression && !hasOpacityExpression && !hasFileExpression )
  {
    return; //nothing further to do
  }

  double width = mWidth;
  if ( hasWidthExpression )
  {
    context.setOriginalValueVariable( mWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), width );
  }
  double opacity = mOpacity;
  if ( hasOpacityExpression )
  {
    context.setOriginalValueVariable( mOpacity );
    opacity = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOpacity, context.renderContext().expressionContext(), opacity * 100 ) / 100.0;
  }
  QString file = mImageFilePath;
  if ( hasFileExpression )
  {
    context.setOriginalValueVariable( mImageFilePath );
    file = context.renderContext().pathResolver().readPath( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyFile, context.renderContext().expressionContext(), file ) );
  }
  applyPattern( mBrush, file, width, opacity, context );
}

bool QgsRasterFillSymbolLayer::applyBrushTransformFromContext( QgsSymbolRenderContext * ) const
{
  return false;
}

void QgsRasterFillSymbolLayer::applyPattern( QBrush &brush, const QString &imageFilePath, const double width, const double alpha, const QgsSymbolRenderContext &context )
{
  QSize size;
  if ( width > 0 )
  {
    if ( mWidthUnit != QgsUnitTypes::RenderPercentage )
    {
      size.setWidth( context.renderContext().convertToPainterUnits( width, mWidthUnit, mWidthMapUnitScale ) );
    }
    else
    {
      // RenderPercentage Unit Type takes original image size
      size = QgsApplication::imageCache()->originalSize( imageFilePath );
      if ( size.isEmpty() )
        return;

      size.setWidth( ( width * size.width() ) / 100.0 );

      // don't render symbols with size below one or above 10,000 pixels
      if ( static_cast< int >( size.width() ) < 1 || 10000.0 < size.width() )
        return;
    }

    size.setHeight( 0 );
  }

  bool cached;
  QImage img = QgsApplication::imageCache()->pathAsImage( imageFilePath, size, true, alpha, cached, ( context.renderContext().flags() & Qgis::RenderContextFlag::RenderBlocking ) );
  if ( img.isNull() )
    return;

  brush.setTextureImage( img );
}


//
// QgsRandomMarkerFillSymbolLayer
//

QgsRandomMarkerFillSymbolLayer::QgsRandomMarkerFillSymbolLayer( int pointCount, Qgis::PointCountMethod method, double densityArea, unsigned long seed )
  : mCountMethod( method )
  , mPointCount( pointCount )
  , mDensityArea( densityArea )
  , mSeed( seed )
{
  setSubSymbol( new QgsMarkerSymbol() );
}

QgsRandomMarkerFillSymbolLayer::~QgsRandomMarkerFillSymbolLayer() = default;

QgsSymbolLayer *QgsRandomMarkerFillSymbolLayer::create( const QVariantMap &properties )
{
  const Qgis::PointCountMethod countMethod  = static_cast< Qgis::PointCountMethod >( properties.value( QStringLiteral( "count_method" ), QStringLiteral( "0" ) ).toInt() );
  const int pointCount = properties.value( QStringLiteral( "point_count" ), QStringLiteral( "10" ) ).toInt();
  const double densityArea = properties.value( QStringLiteral( "density_area" ), QStringLiteral( "250.0" ) ).toDouble();

  unsigned long seed = 0;
  if ( properties.contains( QStringLiteral( "seed" ) ) )
    seed = properties.value( QStringLiteral( "seed" ) ).toUInt();
  else
  {
    // if we a creating a new random marker fill from scratch, we default to a random seed
    // because seed based fills are just nicer for users vs seeing points jump around with every map refresh
    std::random_device rd;
    std::mt19937 mt( seed == 0 ? rd() : seed );
    std::uniform_int_distribution<> uniformDist( 1, 999999999 );
    seed = uniformDist( mt );
  }

  std::unique_ptr< QgsRandomMarkerFillSymbolLayer > sl = std::make_unique< QgsRandomMarkerFillSymbolLayer >( pointCount, countMethod, densityArea, seed );

  if ( properties.contains( QStringLiteral( "density_area_unit" ) ) )
    sl->setDensityAreaUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "density_area_unit" )].toString() ) );
  if ( properties.contains( QStringLiteral( "density_area_unit_scale" ) ) )
    sl->setDensityAreaUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "density_area_unit_scale" )].toString() ) );

  if ( properties.contains( QStringLiteral( "clip_points" ) ) )
  {
    sl->setClipPoints( properties[QStringLiteral( "clip_points" )].toInt() );
  }

  return sl.release();
}

QString QgsRandomMarkerFillSymbolLayer::layerType() const
{
  return QStringLiteral( "RandomMarkerFill" );
}

void QgsRandomMarkerFillSymbolLayer::setColor( const QColor &color )
{
  mMarker->setColor( color );
  mColor = color;
}

QColor QgsRandomMarkerFillSymbolLayer::color() const
{
  return mMarker ? mMarker->color() : mColor;
}

void QgsRandomMarkerFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mMarker->startRender( context.renderContext(), context.fields() );
}

void QgsRandomMarkerFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsRandomMarkerFillSymbolLayer::renderPolygon( const QPolygonF &points, const QVector<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  Part part;
  part.exterior = points;
  if ( rings )
    part.rings = *rings;

  if ( mRenderingFeature )
  {
    // in the middle of rendering a possibly multi-part feature, so we collect all the parts and defer the actual rendering
    // until after we've received the final part
    mFeatureSymbolOpacity = context.opacity();
    mCurrentParts << part;
  }
  else
  {
    // not rendering a feature, so we can just render the polygon immediately
    const double prevOpacity = mMarker->opacity();
    mMarker->setOpacity( mMarker->opacity() * context.opacity() );
    render( context.renderContext(), QVector< Part>() << part, context.feature() ? *context.feature() : QgsFeature(), context.selected() );
    mMarker->setOpacity( prevOpacity );
  }
}

void QgsRandomMarkerFillSymbolLayer::render( QgsRenderContext &context, const QVector<QgsRandomMarkerFillSymbolLayer::Part> &parts, const QgsFeature &feature, bool selected )
{
  bool clipPoints = mClipPoints;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyClipPoints ) )
  {
    context.expressionContext().setOriginalValueVariable( clipPoints );
    clipPoints = mDataDefinedProperties.valueAsBool( QgsSymbolLayer::PropertyClipPoints, context.expressionContext(), clipPoints );
  }

  QVector< QgsGeometry > geometryParts;
  geometryParts.reserve( parts.size() );
  QPainterPath path;

  for ( const Part &part : parts )
  {
    QgsGeometry geom = QgsGeometry::fromQPolygonF( part.exterior );
    if ( !geom.isNull() && !part.rings.empty() )
    {
      QgsPolygon *poly = qgsgeometry_cast< QgsPolygon * >( geom.get() );
      for ( const QPolygonF &ring : part.rings )
      {
        poly->addInteriorRing( QgsLineString::fromQPolygonF( ring ) );
      }
    }
    if ( !geom.isGeosValid() )
    {
      geom = geom.buffer( 0, 0 );
    }
    geometryParts << geom;

    if ( clipPoints )
    {
      path.addPolygon( part.exterior );
      for ( const QPolygonF &ring : part.rings )
      {
        path.addPolygon( ring );
      }
    }
  }

  const QgsGeometry geom = geometryParts.count() != 1 ? QgsGeometry::unaryUnion( geometryParts ) : geometryParts.at( 0 );

  if ( clipPoints )
  {
    context.painter()->save();
    context.painter()->setClipPath( path );
  }


  int count = mPointCount;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyPointCount ) )
  {
    context.expressionContext().setOriginalValueVariable( count );
    count = mDataDefinedProperties.valueAsInt( QgsSymbolLayer::PropertyPointCount, context.expressionContext(), count );
  }

  switch ( mCountMethod )
  {
    case Qgis::PointCountMethod::DensityBased:
    {
      double densityArea = mDensityArea;
      if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDensityArea ) )
      {
        context.expressionContext().setOriginalValueVariable( densityArea );
        densityArea = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyPointCount, context.expressionContext(), densityArea );
      }
      densityArea = context.convertToPainterUnits( std::sqrt( densityArea ), mDensityAreaUnit, mDensityAreaUnitScale );
      densityArea = std::pow( densityArea, 2 );
      count = std::max( 0.0, std::ceil( count * ( geom.area() / densityArea ) ) );
      break;
    }
    case Qgis::PointCountMethod::Absolute:
      break;
  }

  unsigned long seed = mSeed;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyRandomSeed ) )
  {
    context.expressionContext().setOriginalValueVariable( static_cast< unsigned long long >( seed ) );
    seed = mDataDefinedProperties.valueAsInt( QgsSymbolLayer::PropertyRandomSeed, context.expressionContext(), seed );
  }

  QVector< QgsPointXY > randomPoints = geom.randomPointsInPolygon( count, seed );
#if 0
  // in some cases rendering from top to bottom is nice (e.g. randomised tree markers), but in other cases it's not wanted..
  // TODO consider exposing this as an option
  std::sort( randomPoints.begin(), randomPoints.end(), []( const QgsPointXY & a, const QgsPointXY & b )->bool
  {
    return a.y() < b.y();
  } );
#endif
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  QgsExpressionContextScopePopper scopePopper( context.expressionContext(), scope );
  int pointNum = 0;
  const bool needsExpressionContext = mMarker->hasDataDefinedProperties();

  const bool prevIsSubsymbol = context.flags() & Qgis::RenderContextFlag::RenderingSubSymbol;
  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol );

  for ( const QgsPointXY &p : std::as_const( randomPoints ) )
  {
    if ( needsExpressionContext )
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
    mMarker->renderPoint( QPointF( p.x(), p.y() ), feature.isValid() ? &feature : nullptr, context, -1, selected );
  }

  context.setFlag( Qgis::RenderContextFlag::RenderingSubSymbol, prevIsSubsymbol );

  if ( clipPoints )
  {
    context.painter()->restore();
  }
}

QVariantMap QgsRandomMarkerFillSymbolLayer::properties() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "count_method" ), QString::number( static_cast< int >( mCountMethod ) ) );
  map.insert( QStringLiteral( "point_count" ), QString::number( mPointCount ) );
  map.insert( QStringLiteral( "density_area" ), QString::number( mDensityArea ) );
  map.insert( QStringLiteral( "density_area_unit" ), QgsUnitTypes::encodeUnit( mDensityAreaUnit ) );
  map.insert( QStringLiteral( "density_area_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDensityAreaUnitScale ) );
  map.insert( QStringLiteral( "seed" ), QString::number( mSeed ) );
  map.insert( QStringLiteral( "clip_points" ), QString::number( mClipPoints ) );
  return map;
}

QgsRandomMarkerFillSymbolLayer *QgsRandomMarkerFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsRandomMarkerFillSymbolLayer > res = std::make_unique< QgsRandomMarkerFillSymbolLayer >( mPointCount, mCountMethod, mDensityArea, mSeed );
  res->mAngle = mAngle;
  res->mColor = mColor;
  res->setDensityAreaUnit( mDensityAreaUnit );
  res->setDensityAreaUnitScale( mDensityAreaUnitScale );
  res->mClipPoints = mClipPoints;
  res->setSubSymbol( mMarker->clone() );
  copyDataDefinedProperties( res.get() );
  copyPaintEffect( res.get() );
  return res.release();
}

bool QgsRandomMarkerFillSymbolLayer::canCauseArtifactsBetweenAdjacentTiles() const
{
  return true;
}

QgsSymbol *QgsRandomMarkerFillSymbolLayer::subSymbol()
{
  return mMarker.get();
}

bool QgsRandomMarkerFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol || symbol->type() != Qgis::SymbolType::Marker )
  {
    delete symbol;
    return false;
  }

  mMarker.reset( static_cast<QgsMarkerSymbol *>( symbol ) );
  mColor = mMarker->color();
  return true;
}

QSet<QString> QgsRandomMarkerFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attributes = QgsFillSymbolLayer::usedAttributes( context );

  if ( mMarker )
    attributes.unite( mMarker->usedAttributes( context ) );

  return attributes;
}

bool QgsRandomMarkerFillSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mMarker && mMarker->hasDataDefinedProperties() )
    return true;
  return false;
}

int QgsRandomMarkerFillSymbolLayer::pointCount() const
{
  return mPointCount;
}

void QgsRandomMarkerFillSymbolLayer::setPointCount( int pointCount )
{
  mPointCount = pointCount;
}

unsigned long QgsRandomMarkerFillSymbolLayer::seed() const
{
  return mSeed;
}

void QgsRandomMarkerFillSymbolLayer::setSeed( unsigned long seed )
{
  mSeed = seed;
}

bool QgsRandomMarkerFillSymbolLayer::clipPoints() const
{
  return mClipPoints;
}

void QgsRandomMarkerFillSymbolLayer::setClipPoints( bool clipPoints )
{
  mClipPoints = clipPoints;
}

Qgis::PointCountMethod QgsRandomMarkerFillSymbolLayer::countMethod() const
{
  return mCountMethod;
}

void QgsRandomMarkerFillSymbolLayer::setCountMethod( Qgis::PointCountMethod method )
{
  mCountMethod = method;
}

double QgsRandomMarkerFillSymbolLayer::densityArea() const
{
  return mDensityArea;
}

void QgsRandomMarkerFillSymbolLayer::setDensityArea( double area )
{
  mDensityArea = area;
}

void QgsRandomMarkerFillSymbolLayer::startFeatureRender( const QgsFeature &, QgsRenderContext & )
{
  mRenderingFeature = true;
  mCurrentParts.clear();
}

void QgsRandomMarkerFillSymbolLayer::stopFeatureRender( const QgsFeature &feature, QgsRenderContext &context )
{
  mRenderingFeature = false;

  const double prevOpacity = mMarker->opacity();
  mMarker->setOpacity( mMarker->opacity() * mFeatureSymbolOpacity );

  render( context, mCurrentParts, feature, false );

  mFeatureSymbolOpacity = 1;
  mMarker->setOpacity( prevOpacity );
}


void QgsRandomMarkerFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mDensityAreaUnit = unit;
  if ( mMarker )
  {
    mMarker->setOutputUnit( unit );
  }
}

QgsUnitTypes::RenderUnit QgsRandomMarkerFillSymbolLayer::outputUnit() const
{
  if ( mMarker )
  {
    return mMarker->outputUnit();
  }
  return QgsUnitTypes::RenderUnknownUnit; //mOutputUnit;
}

bool QgsRandomMarkerFillSymbolLayer::usesMapUnits() const
{
  if ( mMarker )
  {
    return mMarker->usesMapUnits();
  }
  return false;
}

void QgsRandomMarkerFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  if ( mMarker )
  {
    mMarker->setMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsRandomMarkerFillSymbolLayer::mapUnitScale() const
{
  if ( mMarker )
  {
    return mMarker->mapUnitScale();
  }
  return QgsMapUnitScale();
}

