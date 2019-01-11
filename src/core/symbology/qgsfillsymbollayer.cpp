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
#include "qgsunittypes.h"
#include "qgsmessagelog.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QDomElement>

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
    brush.setColor( mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor ) );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeBrushStyle( mBrushStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyFillStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
      brush.setStyle( QgsSymbolLayerUtils::decodeBrushStyle( exprVal.toString() ) );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    pen.setColor( mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor ) );
  }
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext() );
    double width = exprVal.toDouble( &ok );
    if ( ok )
    {
      width = context.renderContext().convertToPainterUnits( width, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
      pen.setWidthF( width );
      selPen.setWidthF( width );
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


QgsSymbolLayer *QgsSimpleFillSymbolLayer::create( const QgsStringMap &props )
{
  QColor color = DEFAULT_SIMPLEFILL_COLOR;
  Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE;
  QColor strokeColor = DEFAULT_SIMPLEFILL_BORDERCOLOR;
  Qt::PenStyle strokeStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE;
  double strokeWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE;
  QPointF offset;

  if ( props.contains( QStringLiteral( "color" ) ) )
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  if ( props.contains( QStringLiteral( "style" ) ) )
    style = QgsSymbolLayerUtils::decodeBrushStyle( props[QStringLiteral( "style" )] );
  if ( props.contains( QStringLiteral( "color_border" ) ) )
  {
    //pre 2.5 projects used "color_border"
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color_border" )] );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )] );
  }
  else if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    strokeColor = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )] );
  }

  if ( props.contains( QStringLiteral( "style_border" ) ) )
  {
    //pre 2.5 projects used "style_border"
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "style_border" )] );
  }
  else if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )] );
  }
  else if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    strokeStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )] );
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
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    penJoinStyle = QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )] );

  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = qgis::make_unique< QgsSimpleFillSymbolLayer >( color, style, strokeColor, strokeStyle, strokeWidth, penJoinStyle );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "border_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "border_width_unit" )] ) );
  }
  else if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )] ) );
  }
  else if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    sl->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );

  if ( props.contains( QStringLiteral( "border_width_map_unit_scale" ) ) )
    sl->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "border_width_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );

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
  if ( ! SELECTION_IS_OPAQUE ) selColor.setAlphaF( context.opacity() );
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
  Q_UNUSED( context );
}

void QgsSimpleFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  applyDataDefinedSymbology( context, mBrush, mPen, mSelPen );

  p->setBrush( context.selected() ? mSelBrush : mBrush );
  p->setPen( context.selected() ? mSelPen : mPen );

  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( mOffset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( mOffset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  _renderPolygon( p, points, rings, context );

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
}

QgsStringMap QgsSimpleFillSymbolLayer::properties() const
{
  QgsStringMap map;
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
  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = qgis::make_unique< QgsSimpleFillSymbolLayer >( mColor, mBrushStyle, mStrokeColor, mStrokeStyle, mStrokeWidth, mPenJoinStyle );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  sl->setStrokeWidthUnit( mStrokeWidthUnit );
  sl->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  copyDataDefinedProperties( sl.get() );
  copyPaintEffect( sl.get() );
  return sl.release();
}

void QgsSimpleFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  if ( mBrushStyle == Qt::NoBrush && mStrokeStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

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

  QString uom = element.attribute( QStringLiteral( "uom" ), QString() );
  offset.setX( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.x() ) );
  offset.setY( QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset.y() ) );
  strokeWidth = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, strokeWidth );

  std::unique_ptr< QgsSimpleFillSymbolLayer > sl = qgis::make_unique< QgsSimpleFillSymbolLayer >( color, fillStyle, strokeColor, strokeStyle, strokeWidth );
  sl->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
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
  return width * e.mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
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
    GradientColorType colorType, GradientType gradientType,
    GradientCoordinateMode coordinateMode, GradientSpread spread )
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

QgsSymbolLayer *QgsGradientFillSymbolLayer::create( const QgsStringMap &props )
{
  //default to a two-color, linear gradient with feature mode and pad spreading
  GradientType type = QgsGradientFillSymbolLayer::Linear;
  GradientColorType colorType = QgsGradientFillSymbolLayer::SimpleTwoColor;
  GradientCoordinateMode coordinateMode = QgsGradientFillSymbolLayer::Feature;
  GradientSpread gradientSpread = QgsGradientFillSymbolLayer::Pad;
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
    type = static_cast< GradientType >( props[QStringLiteral( "type" )].toInt() );
  if ( props.contains( QStringLiteral( "coordinate_mode" ) ) )
    coordinateMode = static_cast< GradientCoordinateMode >( props[QStringLiteral( "coordinate_mode" )].toInt() );
  if ( props.contains( QStringLiteral( "spread" ) ) )
    gradientSpread = static_cast< GradientSpread >( props[QStringLiteral( "spread" )].toInt() );
  if ( props.contains( QStringLiteral( "color_type" ) ) )
    colorType = static_cast< GradientColorType >( props[QStringLiteral( "color_type" )].toInt() );
  if ( props.contains( QStringLiteral( "gradient_color" ) ) )
  {
    //pre 2.5 projects used "gradient_color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color" )] );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  }
  if ( props.contains( QStringLiteral( "gradient_color2" ) ) )
  {
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color2" )] );
  }

  if ( props.contains( QStringLiteral( "reference_point1" ) ) )
    referencePoint1 = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "reference_point1" )] );
  if ( props.contains( QStringLiteral( "reference_point1_iscentroid" ) ) )
    refPoint1IsCentroid = props[QStringLiteral( "reference_point1_iscentroid" )].toInt();
  if ( props.contains( QStringLiteral( "reference_point2" ) ) )
    referencePoint2 = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "reference_point2" )] );
  if ( props.contains( QStringLiteral( "reference_point2_iscentroid" ) ) )
    refPoint2IsCentroid = props[QStringLiteral( "reference_point2_iscentroid" )].toInt();
  if ( props.contains( QStringLiteral( "angle" ) ) )
    angle = props[QStringLiteral( "angle" )].toDouble();

  if ( props.contains( QStringLiteral( "offset" ) ) )
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] );

  //attempt to create color ramp from props
  QgsColorRamp *gradientRamp = nullptr;
  if ( props.contains( QStringLiteral( "rampType" ) ) && props[QStringLiteral( "rampType" )] == QStringLiteral( "cpt-city" ) )
  {
    gradientRamp = QgsCptCityColorRamp::create( props );
  }
  else
  {
    gradientRamp = QgsGradientColorRamp::create( props );
  }

  //create a new gradient fill layer with desired properties
  std::unique_ptr< QgsGradientFillSymbolLayer > sl = qgis::make_unique< QgsGradientFillSymbolLayer >( color, color2, colorType, type, coordinateMode, gradientSpread );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
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
  }

  //second gradient color
  QColor color2 = mColor2;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertySecondaryColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor2 ) );
    color2 = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertySecondaryColor, context.renderContext().expressionContext(), mColor2 );
  }

  //gradient rotation angle
  double angle = mAngle;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle );
  }

  //gradient type
  QgsGradientFillSymbolLayer::GradientType gradientType = mGradientType;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientType ) )
  {
    QString currentType = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyGradientType, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentType == QObject::tr( "linear" ) )
      {
        gradientType = QgsGradientFillSymbolLayer::Linear;
      }
      else if ( currentType == QObject::tr( "radial" ) )
      {
        gradientType = QgsGradientFillSymbolLayer::Radial;
      }
      else if ( currentType == QObject::tr( "conical" ) )
      {
        gradientType = QgsGradientFillSymbolLayer::Conical;
      }
    }
  }

  //coordinate mode
  GradientCoordinateMode coordinateMode = mCoordinateMode;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCoordinateMode ) )
  {
    QString currentCoordMode = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCoordinateMode, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentCoordMode == QObject::tr( "feature" ) )
      {
        coordinateMode = QgsGradientFillSymbolLayer::Feature;
      }
      else if ( currentCoordMode == QObject::tr( "viewport" ) )
      {
        coordinateMode = QgsGradientFillSymbolLayer::Viewport;
      }
    }
  }

  //gradient spread
  GradientSpread spread = mGradientSpread;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyGradientSpread ) )
  {
    QString currentSpread = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyGradientSpread, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      if ( currentSpread == QObject::tr( "pad" ) )
      {
        spread = QgsGradientFillSymbolLayer::Pad;
      }
      else if ( currentSpread == QObject::tr( "repeat" ) )
      {
        spread = QgsGradientFillSymbolLayer::Repeat;
      }
      else if ( currentSpread == QObject::tr( "reflect" ) )
      {
        spread = QgsGradientFillSymbolLayer::Reflect;
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
    const QColor &color, const QColor &color2, GradientColorType gradientColorType,
    QgsColorRamp *gradientRamp, GradientType gradientType,
    GradientCoordinateMode coordinateMode, GradientSpread gradientSpread,
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
    case QgsGradientFillSymbolLayer::Linear:
      gradient = QLinearGradient( rotatedReferencePoint1, rotatedReferencePoint2 );
      break;
    case QgsGradientFillSymbolLayer::Radial:
      gradient = QRadialGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).length() );
      break;
    case QgsGradientFillSymbolLayer::Conical:
      gradient = QConicalGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).angle() );
      break;
  }
  switch ( coordinateMode )
  {
    case QgsGradientFillSymbolLayer::Feature:
      gradient.setCoordinateMode( QGradient::ObjectBoundingMode );
      break;
    case QgsGradientFillSymbolLayer::Viewport:
      gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
      break;
  }
  switch ( gradientSpread )
  {
    case QgsGradientFillSymbolLayer::Pad:
      gradient.setSpread( QGradient::PadSpread );
      break;
    case QgsGradientFillSymbolLayer::Reflect:
      gradient.setSpread( QGradient::ReflectSpread );
      break;
    case QgsGradientFillSymbolLayer::Repeat:
      gradient.setSpread( QGradient::RepeatSpread );
      break;
  }

  //add stops to gradient
  if ( gradientColorType == QgsGradientFillSymbolLayer::ColorRamp && gradientRamp &&
       ( gradientRamp->type() == QLatin1String( "gradient" ) || gradientRamp->type() == QLatin1String( "cpt-city" ) ) )
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
  Q_UNUSED( context );
}

void QgsGradientFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  applyDataDefinedSymbology( context, points );

  p->setBrush( context.selected() ? mSelBrush : mBrush );
  p->setPen( Qt::NoPen );

  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( mOffset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( mOffset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  _renderPolygon( p, points, rings, context );

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
}

QgsStringMap QgsGradientFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "gradient_color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  map[QStringLiteral( "color_type" )] = QString::number( mGradientColorType );
  map[QStringLiteral( "type" )] = QString::number( mGradientType );
  map[QStringLiteral( "coordinate_mode" )] = QString::number( mCoordinateMode );
  map[QStringLiteral( "spread" )] = QString::number( mGradientSpread );
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
    map.unite( mGradientRamp->properties() );
  }
  return map;
}

QgsGradientFillSymbolLayer *QgsGradientFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsGradientFillSymbolLayer > sl = qgis::make_unique< QgsGradientFillSymbolLayer >( mColor, mColor2, mGradientColorType, mGradientType, mCoordinateMode, mGradientSpread );
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

void QgsGradientFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  mOffsetUnit = unit;
}

QgsUnitTypes::RenderUnit QgsGradientFillSymbolLayer::outputUnit() const
{
  return mOffsetUnit;
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

QgsShapeburstFillSymbolLayer::QgsShapeburstFillSymbolLayer( const QColor &color, const QColor &color2, ShapeburstColorType colorType,
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

QgsSymbolLayer *QgsShapeburstFillSymbolLayer::create( const QgsStringMap &props )
{
  //default to a two-color gradient
  ShapeburstColorType colorType = QgsShapeburstFillSymbolLayer::SimpleTwoColor;
  QColor color = DEFAULT_SIMPLEFILL_COLOR, color2 = Qt::white;
  int blurRadius = 0;
  bool useWholeShape = true;
  double maxDistance = 5;
  QPointF offset;

  //update fill properties from props
  if ( props.contains( QStringLiteral( "color_type" ) ) )
  {
    colorType = static_cast< ShapeburstColorType >( props[QStringLiteral( "color_type" )].toInt() );
  }
  if ( props.contains( QStringLiteral( "shapeburst_color" ) ) )
  {
    //pre 2.5 projects used "shapeburst_color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "shapeburst_color" )] );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  }

  if ( props.contains( QStringLiteral( "shapeburst_color2" ) ) )
  {
    //pre 2.5 projects used "shapeburst_color2"
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "shapeburst_color2" )] );
  }
  else if ( props.contains( QStringLiteral( "gradient_color2" ) ) )
  {
    color2 = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "gradient_color2" )] );
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
    offset = QgsSymbolLayerUtils::decodePoint( props[QStringLiteral( "offset" )] );
  }

  //attempt to create color ramp from props
  QgsColorRamp *gradientRamp = nullptr;
  if ( props.contains( QStringLiteral( "rampType" ) ) && props[QStringLiteral( "rampType" )] == QStringLiteral( "cpt-city" ) )
  {
    gradientRamp = QgsCptCityColorRamp::create( props );
  }
  else
  {
    gradientRamp = QgsGradientColorRamp::create( props );
  }

  //create a new shapeburst fill layer with desired properties
  std::unique_ptr< QgsShapeburstFillSymbolLayer > sl = qgis::make_unique< QgsShapeburstFillSymbolLayer >( color, color2, colorType, blurRadius, useWholeShape, maxDistance );
  sl->setOffset( offset );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
  {
    sl->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "distance_unit" ) ) )
  {
    sl->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "distance_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    sl->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  if ( props.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    sl->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "distance_map_unit_scale" )] ) );
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
  Q_UNUSED( context );
}

void QgsShapeburstFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
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
    QPointF offset;
    if ( !mOffset.isNull() )
    {
      offset.setX( context.renderContext().convertToPainterUnits( mOffset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
      offset.setY( context.renderContext().convertToPainterUnits( mOffset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
      p->translate( offset );
    }
    _renderPolygon( p, points, rings, context );
    if ( !mOffset.isNull() )
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
  if ( mColorType == QgsShapeburstFillSymbolLayer::SimpleTwoColor )
  {
    twoColorGradientRamp = qgis::make_unique< QgsGradientColorRamp >( color1, color2 );
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
  std::unique_ptr< QImage > fillImage = qgis::make_unique< QImage >( imWidth,
                                        imHeight, QImage::Format_ARGB32_Premultiplied );
  if ( fillImage->isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not allocate sufficient memory for shapeburst fill" ) );
    return;
  }

  //also create an image to store the alpha channel
  std::unique_ptr< QImage > alphaImage = qgis::make_unique< QImage >( fillImage->width(), fillImage->height(), QImage::Format_ARGB32_Premultiplied );
  if ( alphaImage->isNull() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not allocate sufficient memory for shapeburst fill" ) );
    return;
  }

  //Fill this image with black. Initially the distance transform is drawn in greyscale, where black pixels have zero distance from the
  //polygon boundary. Since we don't care about pixels which fall outside the polygon, we start with a black image and then draw over it the
  //polygon in white. The distance transform function then fills in the correct distance values for the white pixels.
  fillImage->fill( Qt::black );

  //initially fill the alpha channel image with a transparent color
  alphaImage->fill( Qt::transparent );

  //now, draw the polygon in the alpha channel image
  QPainter imgPainter;
  imgPainter.begin( alphaImage.get() );
  imgPainter.setRenderHint( QPainter::Antialiasing, true );
  imgPainter.setBrush( QBrush( Qt::white ) );
  imgPainter.setPen( QPen( Qt::black ) );
  imgPainter.translate( -points.boundingRect().left() + sideBuffer, - points.boundingRect().top() + sideBuffer );
  _renderPolygon( &imgPainter, points, rings, context );
  imgPainter.end();

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

  //apply distance transform to image, uses the current color ramp to calculate final pixel colors
  double *dtArray = distanceTransform( fillImage.get() );

  //copy distance transform values back to QImage, shading by appropriate color ramp
  dtArrayToQImage( dtArray, fillImage.get(), mColorType == QgsShapeburstFillSymbolLayer::SimpleTwoColor ? twoColorGradientRamp.get() : mGradientRamp.get(),
                   context.opacity(), useWholeShape, outputPixelMaxDist );

  //clean up some variables
  delete [] dtArray;

  //apply blur if desired
  if ( blurRadius > 0 )
  {
    QgsSymbolLayerUtils::blurImageInPlace( *fillImage, QRect( 0, 0, fillImage->width(), fillImage->height() ), blurRadius, false );
  }

  //apply alpha channel to distance transform image, so that areas outside the polygon are transparent
  imgPainter.begin( fillImage.get() );
  imgPainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
  imgPainter.drawImage( 0, 0, *alphaImage );
  imgPainter.end();
  //we're finished with the alpha channel image now
  alphaImage.reset();

  //draw shapeburst image in correct place in the destination painter

  p->save();
  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( mOffset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( mOffset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  p->drawImage( points.boundingRect().left() - sideBuffer, points.boundingRect().top() - sideBuffer, *fillImage );

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
  p->restore();

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
void QgsShapeburstFillSymbolLayer::distanceTransform2d( double *im, int width, int height )
{
  int maxDimension = std::max( width, height );
  double *f = new double[ maxDimension ];
  int *v = new int[ maxDimension ];
  double *z = new double[ maxDimension + 1 ];
  double *d = new double[ maxDimension ];

  // transform along columns
  for ( int x = 0; x < width; x++ )
  {
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
double *QgsShapeburstFillSymbolLayer::distanceTransform( QImage *im )
{
  int width = im->width();
  int height = im->height();

  double *dtArray = new double[width * height];

  //load qImage to array
  QRgb tmpRgb;
  int idx = 0;
  for ( int heightIndex = 0; heightIndex < height; ++heightIndex )
  {
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
  distanceTransform2d( dtArray, width, height );

  return dtArray;
}

void QgsShapeburstFillSymbolLayer::dtArrayToQImage( double *array, QImage *im, QgsColorRamp *ramp, double layerAlpha, bool useWholeShape, int maxPixelDistance )
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
  QColor pixColor;
  bool layerHasAlpha = layerAlpha < 1.0;

  for ( int heightIndex = 0; heightIndex < height; ++heightIndex )
  {
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
      pixColor = ramp->color( pixVal );

      int pixAlpha = pixColor.alpha();
      if ( ( layerHasAlpha ) || ( pixAlpha != 255 ) )
      {
        //apply layer's transparency to alpha value
        double alpha = pixAlpha * layerAlpha;
        //premultiply ramp color since we are storing this in a ARGB32_Premultiplied QImage
        QgsSymbolLayerUtils::premultiplyColor( pixColor, alpha );
      }

      scanLine[widthIndex] = pixColor.rgba();
      idx++;
    }
  }
}

QgsStringMap QgsShapeburstFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "gradient_color2" )] = QgsSymbolLayerUtils::encodeColor( mColor2 );
  map[QStringLiteral( "color_type" )] = QString::number( mColorType );
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
    map.unite( mGradientRamp->properties() );
  }

  return map;
}

QgsShapeburstFillSymbolLayer *QgsShapeburstFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsShapeburstFillSymbolLayer > sl = qgis::make_unique< QgsShapeburstFillSymbolLayer >( mColor, mColor2, mColorType, mBlurRadius, mUseWholeShape, mMaxDistance );
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
  setSubSymbol( new QgsLineSymbol() );
}

void QgsImageFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
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
  if ( context.renderContext().testFlag( QgsRenderContext::RenderMapTile ) )
  {
    //transform brush to upper left corner of geometry bbox
    QPointF leftCorner = points.boundingRect().topLeft();
    QTransform t = mBrush.transform();
    t.translate( leftCorner.x(), leftCorner.y() );
    mBrush.setTransform( t );
  }

  if ( context.selected() )
  {
    QColor selColor = context.renderContext().selectionColor();
    // Alister - this doesn't seem to work here
    //if ( ! selectionIsOpaque )
    //  selColor.setAlphaF( context.alpha() );
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
  if ( mStroke )
  {
    mStroke->renderPolyline( points, context.feature(), context.renderContext(), -1, SELECT_FILL_BORDER && context.selected() );
    if ( rings )
    {
      QList<QPolygonF>::const_iterator ringIt = rings->constBegin();
      for ( ; ringIt != rings->constEnd(); ++ringIt )
      {
        mStroke->renderPolyline( *ringIt, context.feature(), context.renderContext(), -1, SELECT_FILL_BORDER && context.selected() );
      }
    }
  }

  mBrush.setTransform( bkTransform );
}

bool QgsImageFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol ) //unset current stroke
  {
    mStroke.reset( nullptr );
    return true;
  }

  if ( symbol->type() != QgsSymbol::Line )
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

double QgsImageFillSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mStroke && mStroke->symbolLayer( 0 ) )
  {
    double subLayerBleed = mStroke->symbolLayer( 0 )->estimateMaxBleed( context );
    return subLayerBleed;
  }
  return 0;
}

double QgsImageFillSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double width = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  return width * e.mapUnitScaleFactor( e.symbologyScale(), mStrokeWidthUnit, e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
}

QColor QgsImageFillSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( context );
  if ( !mStroke )
  {
    return QColor( Qt::black );
  }
  return mStroke->color();
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

QSet<QString> QgsImageFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsFillSymbolLayer::usedAttributes( context );
  if ( mStroke )
    attr.unite( mStroke->usedAttributes( context ) );
  return attr;
}


//QgsSVGFillSymbolLayer

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QString &svgFilePath, double width, double angle )
  : QgsImageFillSymbolLayer()
  , mPatternWidth( width )
{
  setSvgFilePath( svgFilePath );
  mStrokeWidth = 0.3;
  mAngle = angle;
  mColor = QColor( 255, 255, 255 );
  setDefaultSvgParams();
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
  setSubSymbol( new QgsLineSymbol() );
  setDefaultSvgParams();
}

void QgsSVGFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mPatternWidthUnit = unit;
  mSvgStrokeWidthUnit = unit;
  mStrokeWidthUnit = unit;
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
  mStrokeWidthMapUnitScale = scale;
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

QgsSymbolLayer *QgsSVGFillSymbolLayer::create( const QgsStringMap &properties )
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
    svgFilePath = properties[QStringLiteral( "svgFile" )];
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    angle = properties[QStringLiteral( "angle" )].toDouble();
  }

  std::unique_ptr< QgsSVGFillSymbolLayer > symbolLayer;
  if ( !svgFilePath.isEmpty() )
  {
    symbolLayer = qgis::make_unique< QgsSVGFillSymbolLayer >( svgFilePath, width, angle );
  }
  else
  {
    if ( properties.contains( QStringLiteral( "data" ) ) )
    {
      data = QByteArray::fromHex( properties[QStringLiteral( "data" )].toLocal8Bit() );
    }
    symbolLayer = qgis::make_unique< QgsSVGFillSymbolLayer >( data, width, angle );
  }

  //svg parameters
  if ( properties.contains( QStringLiteral( "svgFillColor" ) ) )
  {
    //pre 2.5 projects used "svgFillColor"
    symbolLayer->setSvgFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "svgFillColor" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    symbolLayer->setSvgFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )] ) );
  }
  if ( properties.contains( QStringLiteral( "svgOutlineColor" ) ) )
  {
    //pre 2.5 projects used "svgOutlineColor"
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "svgOutlineColor" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    symbolLayer->setSvgStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )] ) );
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
    symbolLayer->setPatternWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "pattern_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "pattern_width_map_unit_scale" ) ) )
  {
    symbolLayer->setPatternWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "pattern_width_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "svg_outline_width_unit" ) ) )
  {
    symbolLayer->setSvgStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "svg_outline_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "svg_outline_width_map_unit_scale" ) ) )
  {
    symbolLayer->setSvgStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "svg_outline_width_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    symbolLayer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    symbolLayer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )] ) );
  }

  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer.release();
}

void QgsSVGFillSymbolLayer::resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QgsStringMap::iterator it = properties.find( QStringLiteral( "svgFile" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = QgsSymbolLayerUtils::svgSymbolPathToName( it.value(), pathResolver );
    else
      it.value() = QgsSymbolLayerUtils::svgSymbolNameToPath( it.value(), pathResolver );
  }
}

QString QgsSVGFillSymbolLayer::layerType() const
{
  return QStringLiteral( "SVGFill" );
}

void QgsSVGFillSymbolLayer::applyPattern( QBrush &brush, const QString &svgFilePath, double patternWidth, QgsUnitTypes::RenderUnit patternWidthUnit,
    const QColor &svgFillColor, const QColor &svgStrokeColor, double svgStrokeWidth,
    QgsUnitTypes::RenderUnit svgStrokeWidthUnit, const QgsSymbolRenderContext &context,
    const QgsMapUnitScale &patternWidthMapUnitScale, const QgsMapUnitScale &svgStrokeWidthMapUnitScale )
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
                          context.renderContext().scaleFactor(), fitsInCache );
    if ( !fitsInCache )
    {
      QPicture patternPict = QgsApplication::svgCache()->svgAsPicture( svgFilePath, size, svgFillColor, svgStrokeColor, strokeWidth,
                             context.renderContext().scaleFactor() );
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

  applyPattern( mBrush, mSvgFilePath, mPatternWidth, mPatternWidthUnit, mColor, mSvgStrokeColor, mSvgStrokeWidth, mSvgStrokeWidthUnit, context, mPatternWidthMapUnitScale, mSvgStrokeWidthMapUnitScale );

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

QgsStringMap QgsSVGFillSymbolLayer::properties() const
{
  QgsStringMap map;
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
  return map;
}

QgsSVGFillSymbolLayer *QgsSVGFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsSVGFillSymbolLayer > clonedLayer;
  if ( !mSvgFilePath.isEmpty() )
  {
    clonedLayer = qgis::make_unique< QgsSVGFillSymbolLayer >( mSvgFilePath, mPatternWidth, mAngle );
    clonedLayer->setSvgFillColor( mColor );
    clonedLayer->setSvgStrokeColor( mSvgStrokeColor );
    clonedLayer->setSvgStrokeWidth( mSvgStrokeWidth );
  }
  else
  {
    clonedLayer = qgis::make_unique< QgsSVGFillSymbolLayer >( mSvgData, mPatternWidth, mAngle );
  }

  clonedLayer->setPatternWidthUnit( mPatternWidthUnit );
  clonedLayer->setPatternWidthMapUnitScale( mPatternWidthMapUnitScale );
  clonedLayer->setSvgStrokeWidthUnit( mSvgStrokeWidthUnit );
  clonedLayer->setSvgStrokeWidthMapUnitScale( mSvgStrokeWidthMapUnitScale );
  clonedLayer->setStrokeWidthUnit( mStrokeWidthUnit );
  clonedLayer->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );

  if ( mStroke )
  {
    clonedLayer->setSubSymbol( mStroke->clone() );
  }
  copyDataDefinedProperties( clonedLayer.get() );
  copyPaintEffect( clonedLayer.get() );
  return clonedLayer.release();
}

void QgsSVGFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
  element.appendChild( symbolizerElem );

  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

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
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ) ).arg( mAngle );
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

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );
  strokeWidth = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, strokeWidth );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  std::unique_ptr< QgsSVGFillSymbolLayer > sl = qgis::make_unique< QgsSVGFillSymbolLayer >( path, size, angle );
  sl->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
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
  applyPattern( mBrush, svgFile, width, mPatternWidthUnit, svgFillColor, svgStrokeColor, strokeWidth,
                mSvgStrokeWidthUnit, context, mPatternWidthMapUnitScale, mSvgStrokeWidthMapUnitScale );

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


QgsLinePatternFillSymbolLayer::QgsLinePatternFillSymbolLayer()
  : QgsImageFillSymbolLayer()
{
  setSubSymbol( new QgsLineSymbol() );
  QgsImageFillSymbolLayer::setSubSymbol( nullptr ); //no stroke
}

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

QgsLinePatternFillSymbolLayer::~QgsLinePatternFillSymbolLayer()
{
  delete mFillLineSymbol;
}

bool QgsLinePatternFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == QgsSymbol::Line )
  {
    QgsLineSymbol *lineSymbol = dynamic_cast<QgsLineSymbol *>( symbol );
    if ( lineSymbol )
    {
      delete mFillLineSymbol;
      mFillLineSymbol = lineSymbol;

      return true;
    }
  }
  delete symbol;
  return false;
}

QgsSymbol *QgsLinePatternFillSymbolLayer::subSymbol()
{
  return mFillLineSymbol;
}

QSet<QString> QgsLinePatternFillSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsImageFillSymbolLayer::usedAttributes( context );
  if ( mFillLineSymbol )
    attr.unite( mFillLineSymbol->usedAttributes( context ) );
  return attr;
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
}

QgsUnitTypes::RenderUnit QgsLinePatternFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceUnit != unit || mLineWidthUnit != unit || mOffsetUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
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

QgsSymbolLayer *QgsLinePatternFillSymbolLayer::create( const QgsStringMap &properties )
{
  std::unique_ptr< QgsLinePatternFillSymbolLayer > patternLayer = qgis::make_unique< QgsLinePatternFillSymbolLayer >();

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
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )] );
  }
  else if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )] );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )] );
  }
  patternLayer->setColor( color );

  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    offset = properties[QStringLiteral( "offset" )].toDouble();
  }
  patternLayer->setOffset( offset );


  if ( properties.contains( QStringLiteral( "distance_unit" ) ) )
  {
    patternLayer->setDistanceUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "distance_map_unit_scale" ) ) )
  {
    patternLayer->setDistanceMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    patternLayer->setLineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    patternLayer->setLineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "line_width_map_unit_scale" ) ) )
  {
    patternLayer->setLineWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "line_width_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    patternLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    patternLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    patternLayer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    patternLayer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )] ) );
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
  double outputPixelOffset = ctx.convertToPainterUnits( mOffset, mOffsetUnit, mOffsetMapUnitScale );

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

  fillLineSymbol->startRender( lineRenderContext, context.fields() );

  QVector<QPolygonF> polygons;
  polygons.append( QPolygonF() << p1 << p2 );
  polygons.append( QPolygonF() << p3 << p4 );
  if ( !qgsDoubleNear( lineAngle, 0 ) && !qgsDoubleNear( lineAngle, 360 ) && !qgsDoubleNear( lineAngle, 90 ) && !qgsDoubleNear( lineAngle, 180 ) && !qgsDoubleNear( lineAngle, 270 ) )
  {
    polygons.append( QPolygonF() << p5 << p6 );
  }

  for ( const QPolygonF &polygon : qgis::as_const( polygons ) )
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
  applyPattern( context, mBrush, mLineAngle, mDistance );

  if ( mFillLineSymbol )
  {
    mFillLineSymbol->startRender( context.renderContext(), context.fields() );
  }
}

void QgsLinePatternFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mFillLineSymbol )
  {
    mFillLineSymbol->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsLinePatternFillSymbolLayer::properties() const
{
  QgsStringMap map;
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

void QgsLinePatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

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
    angleFunc = QStringLiteral( "%1 + %2" ).arg( props.value( QStringLiteral( "angle" ), QStringLiteral( "0" ) ) ).arg( mLineAngle );
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
  featureStyle.append( QStringLiteral( ",bc:%1" ).arg( QStringLiteral( "#00000000" ) ) ); //transparent background
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

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );
  lineWidth = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, lineWidth );

  std::unique_ptr< QgsLinePatternFillSymbolLayer > sl = qgis::make_unique< QgsLinePatternFillSymbolLayer >();
  sl->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
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
  mDistanceX = 15;
  mDistanceY = 15;
  mDisplacementX = 0;
  mDisplacementY = 0;
  setSubSymbol( new QgsMarkerSymbol() );
  QgsImageFillSymbolLayer::setSubSymbol( nullptr ); //no stroke
}

QgsPointPatternFillSymbolLayer::~QgsPointPatternFillSymbolLayer()
{
  delete mMarkerSymbol;
}

void QgsPointPatternFillSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mDistanceXUnit = unit;
  mDistanceYUnit = unit;
  mDisplacementXUnit = unit;
  mDisplacementYUnit = unit;
  if ( mMarkerSymbol )
  {
    mMarkerSymbol->setOutputUnit( unit );
  }

}

QgsUnitTypes::RenderUnit QgsPointPatternFillSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceXUnit != unit || mDistanceYUnit != unit || mDisplacementXUnit != unit || mDisplacementYUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsPointPatternFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsImageFillSymbolLayer::setMapUnitScale( scale );
  mDistanceXMapUnitScale = scale;
  mDistanceYMapUnitScale = scale;
  mDisplacementXMapUnitScale = scale;
  mDisplacementYMapUnitScale = scale;
}

QgsMapUnitScale QgsPointPatternFillSymbolLayer::mapUnitScale() const
{
  if ( QgsImageFillSymbolLayer::mapUnitScale() == mDistanceXMapUnitScale &&
       mDistanceXMapUnitScale == mDistanceYMapUnitScale &&
       mDistanceYMapUnitScale == mDisplacementXMapUnitScale &&
       mDisplacementXMapUnitScale == mDisplacementYMapUnitScale )
  {
    return mDistanceXMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsPointPatternFillSymbolLayer::create( const QgsStringMap &properties )
{
  std::unique_ptr< QgsPointPatternFillSymbolLayer > layer = qgis::make_unique< QgsPointPatternFillSymbolLayer >();
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

  if ( properties.contains( QStringLiteral( "distance_x_unit" ) ) )
  {
    layer->setDistanceXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_x_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "distance_x_map_unit_scale" ) ) )
  {
    layer->setDistanceXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_x_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "distance_y_unit" ) ) )
  {
    layer->setDistanceYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "distance_y_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "distance_y_map_unit_scale" ) ) )
  {
    layer->setDistanceYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "distance_y_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_x_unit" ) ) )
  {
    layer->setDisplacementXUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "displacement_x_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_x_map_unit_scale" ) ) )
  {
    layer->setDisplacementXMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "displacement_x_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_y_unit" ) ) )
  {
    layer->setDisplacementYUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "displacement_y_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "displacement_y_map_unit_scale" ) ) )
  {
    layer->setDisplacementYMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "displacement_y_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    layer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )] ) );
  }

  layer->restoreOldDataDefinedProperties( properties );

  return layer.release();
}

QString QgsPointPatternFillSymbolLayer::layerType() const
{
  return QStringLiteral( "PointPatternFill" );
}

void QgsPointPatternFillSymbolLayer::applyPattern( const QgsSymbolRenderContext &context, QBrush &brush, double distanceX, double distanceY,
    double displacementX, double displacementY )
{
  //render 3 rows and columns in one go to easily incorporate displacement
  const QgsRenderContext &ctx = context.renderContext();
  double width = ctx.convertToPainterUnits( distanceX, mDistanceXUnit, mDistanceXMapUnitScale ) * 2.0;
  double height = ctx.convertToPainterUnits( distanceY, mDistanceYUnit, mDisplacementYMapUnitScale ) * 2.0;

  if ( width > 10000 || height > 10000 ) //protect symbol layer from eating too much memory
  {
    QImage img;
    brush.setTextureImage( img );
    return;
  }

  QImage patternImage( width, height, QImage::Format_ARGB32 );
  patternImage.fill( 0 );

  if ( mMarkerSymbol )
  {
    QPainter p( &patternImage );

    //marker rendering needs context for drawing on patternImage
    QgsRenderContext pointRenderContext;
    pointRenderContext.setRendererScale( context.renderContext().rendererScale() );
    pointRenderContext.setPainter( &p );
    pointRenderContext.setScaleFactor( context.renderContext().scaleFactor() );
    if ( context.renderContext().flags() & QgsRenderContext::Antialiasing )
    {
      pointRenderContext.setFlag( QgsRenderContext::Antialiasing, true );
      p.setRenderHint( QPainter::Antialiasing, true );
    }
    QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() );
    pointRenderContext.setMapToPixel( mtp );
    pointRenderContext.setForceVectorOutput( false );
    pointRenderContext.setExpressionContext( context.renderContext().expressionContext() );

    mMarkerSymbol->startRender( pointRenderContext, context.fields() );

    //render corner points
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( 0, height ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, height ), context.feature(), pointRenderContext );

    //render displaced points
    double displacementPixelX = ctx.convertToPainterUnits( displacementX, mDisplacementXUnit, mDisplacementXMapUnitScale );
    double displacementPixelY = ctx.convertToPainterUnits( displacementY, mDisplacementYUnit, mDisplacementYMapUnitScale );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, -displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0 + displacementPixelX, height / 2.0 - displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width + displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, height - displacementPixelY ), context.feature(), pointRenderContext );

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
  applyPattern( context, mBrush, mDistanceX, mDistanceY, mDisplacementX, mDisplacementY );

  if ( mStroke )
  {
    mStroke->startRender( context.renderContext(), context.fields() );
  }
}

void QgsPointPatternFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  if ( mStroke )
  {
    mStroke->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsPointPatternFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map.insert( QStringLiteral( "distance_x" ), QString::number( mDistanceX ) );
  map.insert( QStringLiteral( "distance_y" ), QString::number( mDistanceY ) );
  map.insert( QStringLiteral( "displacement_x" ), QString::number( mDisplacementX ) );
  map.insert( QStringLiteral( "displacement_y" ), QString::number( mDisplacementY ) );
  map.insert( QStringLiteral( "distance_x_unit" ), QgsUnitTypes::encodeUnit( mDistanceXUnit ) );
  map.insert( QStringLiteral( "distance_y_unit" ), QgsUnitTypes::encodeUnit( mDistanceYUnit ) );
  map.insert( QStringLiteral( "displacement_x_unit" ), QgsUnitTypes::encodeUnit( mDisplacementXUnit ) );
  map.insert( QStringLiteral( "displacement_y_unit" ), QgsUnitTypes::encodeUnit( mDisplacementYUnit ) );
  map.insert( QStringLiteral( "distance_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceXMapUnitScale ) );
  map.insert( QStringLiteral( "distance_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDistanceYMapUnitScale ) );
  map.insert( QStringLiteral( "displacement_x_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDisplacementXMapUnitScale ) );
  map.insert( QStringLiteral( "displacement_y_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mDisplacementYMapUnitScale ) );
  map.insert( QStringLiteral( "outline_width_unit" ), QgsUnitTypes::encodeUnit( mStrokeWidthUnit ) );
  map.insert( QStringLiteral( "outline_width_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale ) );
  return map;
}

QgsPointPatternFillSymbolLayer *QgsPointPatternFillSymbolLayer::clone() const
{
  QgsPointPatternFillSymbolLayer *clonedLayer = static_cast<QgsPointPatternFillSymbolLayer *>( QgsPointPatternFillSymbolLayer::create( properties() ) );
  if ( mMarkerSymbol )
  {
    clonedLayer->setSubSymbol( mMarkerSymbol->clone() );
  }
  copyDataDefinedProperties( clonedLayer );
  copyPaintEffect( clonedLayer );
  return clonedLayer;
}

void QgsPointPatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  for ( int i = 0; i < mMarkerSymbol->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PolygonSymbolizer" ) );
    if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
      symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

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
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    if ( !markerLayer )
    {
      QString errorMsg = QStringLiteral( "MarkerSymbolLayerV2 expected, %1 found. Skip it." ).arg( layer->layerType() );
      graphicFillElem.appendChild( doc.createComment( errorMsg ) );
    }
    else
    {
      markerLayer->writeSldMarker( doc, graphicFillElem, props );
    }
  }
}

QgsSymbolLayer *QgsPointPatternFillSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element );
  return nullptr;
}

bool QgsPointPatternFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == QgsSymbol::Marker )
  {
    QgsMarkerSymbol *markerSymbol = static_cast<QgsMarkerSymbol *>( symbol );
    delete mMarkerSymbol;
    mMarkerSymbol = markerSymbol;
  }
  return true;
}

void QgsPointPatternFillSymbolLayer::applyDataDefinedSettings( QgsSymbolRenderContext &context )
{
  if ( !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceX ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDistanceY )
       && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementX ) && !mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyDisplacementY )
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
  applyPattern( context, mBrush, distanceX, distanceY, displacementX, displacementY );
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

QgsSymbolLayer *QgsCentroidFillSymbolLayer::create( const QgsStringMap &properties )
{
  std::unique_ptr< QgsCentroidFillSymbolLayer > sl = qgis::make_unique< QgsCentroidFillSymbolLayer >();

  if ( properties.contains( QStringLiteral( "point_on_surface" ) ) )
    sl->setPointOnSurface( properties[QStringLiteral( "point_on_surface" )].toInt() != 0 );
  if ( properties.contains( QStringLiteral( "point_on_all_parts" ) ) )
    sl->setPointOnAllParts( properties[QStringLiteral( "point_on_all_parts" )].toInt() != 0 );

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
  mMarker->setOpacity( context.opacity() );
  mMarker->startRender( context.renderContext(), context.fields() );

  mCurrentFeatureId = -1;
  mBiggestPartIndex = 0;
}

void QgsCentroidFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsCentroidFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  Q_UNUSED( rings );

  if ( !mPointOnAllParts )
  {
    const QgsFeature *feature = context.feature();
    if ( feature )
    {
      if ( feature->id() != mCurrentFeatureId )
      {
        mCurrentFeatureId = feature->id();
        mBiggestPartIndex = 1;

        if ( context.geometryPartCount() > 1 )
        {
          const QgsGeometry geom = feature->geometry();
          const QgsGeometryCollection *geomCollection = static_cast<const QgsGeometryCollection *>( geom.constGet() );

          double area = 0;
          double areaBiggest = 0;
          for ( int i = 0; i < context.geometryPartCount(); ++i )
          {
            area = geomCollection->geometryN( i )->area();
            if ( area > areaBiggest )
            {
              areaBiggest = area;
              mBiggestPartIndex = i + 1;
            }
          }
        }
      }
    }
  }

  if ( mPointOnAllParts || ( context.geometryPartNum() == mBiggestPartIndex ) )
  {
    QPointF centroid = mPointOnSurface ? QgsSymbolLayerUtils::polygonPointOnSurface( points ) : QgsSymbolLayerUtils::polygonCentroid( points );
    mMarker->renderPoint( centroid, context.feature(), context.renderContext(), -1, context.selected() );
  }
}

QgsStringMap QgsCentroidFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "point_on_surface" )] = QString::number( mPointOnSurface );
  map[QStringLiteral( "point_on_all_parts" )] = QString::number( mPointOnAllParts );
  return map;
}

QgsCentroidFillSymbolLayer *QgsCentroidFillSymbolLayer::clone() const
{
  std::unique_ptr< QgsCentroidFillSymbolLayer > x = qgis::make_unique< QgsCentroidFillSymbolLayer >();
  x->mAngle = mAngle;
  x->mColor = mColor;
  x->setSubSymbol( mMarker->clone() );
  x->setPointOnSurface( mPointOnSurface );
  x->setPointOnAllParts( mPointOnAllParts );
  copyDataDefinedProperties( x.get() );
  copyPaintEffect( x.get() );
  return x.release();
}

void QgsCentroidFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
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

  std::unique_ptr< QgsCentroidFillSymbolLayer > sl = qgis::make_unique< QgsCentroidFillSymbolLayer >();
  sl->setSubSymbol( marker.release() );
  return sl.release();
}


QgsSymbol *QgsCentroidFillSymbolLayer::subSymbol()
{
  return mMarker.get();
}

bool QgsCentroidFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  if ( !symbol || symbol->type() != QgsSymbol::Marker )
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
}

QgsSymbolLayer *QgsRasterFillSymbolLayer::create( const QgsStringMap &properties )
{
  FillCoordinateMode mode = QgsRasterFillSymbolLayer::Feature;
  double alpha = 1.0;
  QPointF offset;
  double angle = 0.0;
  double width = 0.0;

  QString imagePath;
  if ( properties.contains( QStringLiteral( "imageFile" ) ) )
  {
    imagePath = properties[QStringLiteral( "imageFile" )];
  }
  if ( properties.contains( QStringLiteral( "coordinate_mode" ) ) )
  {
    mode = static_cast< FillCoordinateMode >( properties[QStringLiteral( "coordinate_mode" )].toInt() );
  }
  if ( properties.contains( QStringLiteral( "alpha" ) ) )
  {
    alpha = properties[QStringLiteral( "alpha" )].toDouble();
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    offset = QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )] );
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    angle = properties[QStringLiteral( "angle" )].toDouble();
  }
  if ( properties.contains( QStringLiteral( "width" ) ) )
  {
    width = properties[QStringLiteral( "width" )].toDouble();
  }
  std::unique_ptr< QgsRasterFillSymbolLayer > symbolLayer = qgis::make_unique< QgsRasterFillSymbolLayer >( imagePath );
  symbolLayer->setCoordinateMode( mode );
  symbolLayer->setOpacity( alpha );
  symbolLayer->setOffset( offset );
  symbolLayer->setAngle( angle );
  symbolLayer->setWidth( width );
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    symbolLayer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "width_unit" ) ) )
  {
    symbolLayer->setWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "width_map_unit_scale" ) ) )
  {
    symbolLayer->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "width_map_unit_scale" )] ) );
  }

  symbolLayer->restoreOldDataDefinedProperties( properties );

  return symbolLayer.release();
}

void QgsRasterFillSymbolLayer::resolvePaths( QgsStringMap &properties, const QgsPathResolver &pathResolver, bool saving )
{
  QgsStringMap::iterator it = properties.find( QStringLiteral( "imageFile" ) );
  if ( it != properties.end() )
  {
    if ( saving )
      it.value() = pathResolver.writePath( it.value() );
    else
      it.value() = pathResolver.readPath( it.value() );
  }
}

bool QgsRasterFillSymbolLayer::setSubSymbol( QgsSymbol *symbol )
{
  Q_UNUSED( symbol );
  return true;
}

QString QgsRasterFillSymbolLayer::layerType() const
{
  return QStringLiteral( "RasterFill" );
}

void QgsRasterFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( context.renderContext().convertToPainterUnits( mOffset.x(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( context.renderContext().convertToPainterUnits( mOffset.y(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }
  if ( mCoordinateMode == Feature )
  {
    QRectF boundingRect = points.boundingRect();
    mBrush.setTransform( mBrush.transform().translate( boundingRect.left() - mBrush.transform().dx(),
                         boundingRect.top() - mBrush.transform().dy() ) );
  }

  QgsImageFillSymbolLayer::renderPolygon( points, rings, context );
  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
}

void QgsRasterFillSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  applyPattern( mBrush, mImageFilePath, mWidth, mOpacity, context );
}

void QgsRasterFillSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

QgsStringMap QgsRasterFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "imageFile" )] = mImageFilePath;
  map[QStringLiteral( "coordinate_mode" )] = QString::number( mCoordinateMode );
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
  std::unique_ptr< QgsRasterFillSymbolLayer > sl = qgis::make_unique< QgsRasterFillSymbolLayer >( mImageFilePath );
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

void QgsRasterFillSymbolLayer::setImageFilePath( const QString &imagePath )
{
  mImageFilePath = imagePath;
}

void QgsRasterFillSymbolLayer::setCoordinateMode( const QgsRasterFillSymbolLayer::FillCoordinateMode mode )
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

void QgsRasterFillSymbolLayer::applyPattern( QBrush &brush, const QString &imageFilePath, const double width, const double alpha, const QgsSymbolRenderContext &context )
{
  QSize size;
  if ( width > 0 )
  {
    size.setWidth( context.renderContext().convertToPainterUnits( width, mWidthUnit, mWidthMapUnitScale ) );
    size.setHeight( 0 );
  }

  bool cached;
  QImage img = QgsApplication::imageCache()->pathAsImage( imageFilePath, size, true, alpha, cached );
  if ( img.isNull() )
    return;

  brush.setTextureImage( img );
}
