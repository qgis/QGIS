/***************************************************************************
 qgsfillsymbollayerv2.cpp
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

#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsmarkersymbollayerv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsdxfexport.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgslogger.h"
#include "qgsvectorcolorrampv2.h"

#include <QPainter>
#include <QFile>
#include <QSvgRenderer>
#include <QDomDocument>
#include <QDomElement>

QgsSimpleFillSymbolLayerV2::QgsSimpleFillSymbolLayerV2( QColor color, Qt::BrushStyle style, QColor borderColor, Qt::PenStyle borderStyle, double borderWidth,
    Qt::PenJoinStyle penJoinStyle )
    : mBrushStyle( style )
    , mBorderColor( borderColor )
    , mBorderStyle( borderStyle )
    , mBorderWidth( borderWidth )
    , mBorderWidthUnit( QgsSymbolV2::MM )
    , mPenJoinStyle( penJoinStyle )
    , mOffsetUnit( QgsSymbolV2::MM )
{
  mColor = color;
}

void QgsSimpleFillSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mBorderWidthUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsSimpleFillSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mBorderWidthUnit;
  if ( mOffsetUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsSimpleFillSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mBorderWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleFillSymbolLayerV2::mapUnitScale() const
{
  if ( mBorderWidthMapUnitScale == mOffsetMapUnitScale )
  {
    return mBorderWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSimpleFillSymbolLayerV2::applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QBrush& brush, QPen& pen, QPen& selPen )
{
  if ( mDataDefinedProperties.isEmpty() )
    return; // shortcut

  QgsExpression* colorExpression = expression( "color" );
  if ( colorExpression )
  {
    brush.setColor( QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
  }
  QgsExpression* colorBorderExpression = expression( "color_border" );
  if ( colorBorderExpression )
  {
    pen.setColor( QgsSymbolLayerV2Utils::decodeColor( colorBorderExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
  }
  QgsExpression* widthBorderExpression = expression( "width_border" );
  if ( widthBorderExpression )
  {
    double width = widthBorderExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
    width *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mBorderWidthUnit, mBorderWidthMapUnitScale );
    pen.setWidthF( width );
    selPen.setWidthF( width );
  }
}


QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLEFILL_COLOR;
  Qt::BrushStyle style = DEFAULT_SIMPLEFILL_STYLE;
  QColor borderColor = DEFAULT_SIMPLEFILL_BORDERCOLOR;
  Qt::PenStyle borderStyle = DEFAULT_SIMPLEFILL_BORDERSTYLE;
  double borderWidth = DEFAULT_SIMPLEFILL_BORDERWIDTH;
  Qt::PenJoinStyle penJoinStyle = DEFAULT_SIMPLEFILL_JOINSTYLE;
  QPointF offset;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "style" ) )
    style = QgsSymbolLayerV2Utils::decodeBrushStyle( props["style"] );
  if ( props.contains( "color_border" ) )
  {
    //pre 2.5 projects used "color_border"
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["color_border"] );
  }
  else if ( props.contains( "outline_color" ) )
  {
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["outline_color"] );
  }
  else if ( props.contains( "line_color" ) )
  {
    borderColor = QgsSymbolLayerV2Utils::decodeColor( props["line_color"] );
  }

  if ( props.contains( "style_border" ) )
  {
    //pre 2.5 projects used "style_border"
    borderStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["style_border"] );
  }
  else if ( props.contains( "outline_style" ) )
  {
    borderStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["outline_style"] );
  }
  else if ( props.contains( "line_style" ) )
  {
    borderStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["line_style"] );
  }
  if ( props.contains( "width_border" ) )
  {
    //pre 2.5 projects used "width_border"
    borderWidth = props["width_border"].toDouble();
  }
  else if ( props.contains( "outline_width" ) )
  {
    borderWidth = props["outline_width"].toDouble();
  }
  else if ( props.contains( "line_width" ) )
  {
    borderWidth = props["line_width"].toDouble();
  }
  if ( props.contains( "offset" ) )
    offset = QgsSymbolLayerV2Utils::decodePoint( props["offset"] );
  if ( props.contains( "joinstyle" ) )
    penJoinStyle = QgsSymbolLayerV2Utils::decodePenJoinStyle( props["joinstyle"] );

  QgsSimpleFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( color, style, borderColor, borderStyle, borderWidth, penJoinStyle );
  sl->setOffset( offset );
  if ( props.contains( "border_width_unit" ) )
  {
    sl->setBorderWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["border_width_unit"] ) );
  }
  else if ( props.contains( "outline_width_unit" ) )
  {
    sl->setBorderWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  }
  else if ( props.contains( "line_width_unit" ) )
  {
    sl->setBorderWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["line_width_unit"] ) );
  }
  if ( props.contains( "offset_unit" ) )
    sl->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );

  if ( props.contains( "border_width_map_unit_scale" ) )
    sl->setBorderWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["border_width_map_unit_scale"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );

  if ( props.contains( "color_expression" ) )
  {
    sl->setDataDefinedProperty( "color", props["color_expression"] );
  }
  if ( props.contains( "color_border_expression" ) )
  {
    sl->setDataDefinedProperty( "color_border", props["color_border_expression"] );
  }
  if ( props.contains( "width_border_expression" ) )
  {
    sl->setDataDefinedProperty( "width_border", props["width_border_expression"] );
  }
  return sl;
}


QString QgsSimpleFillSymbolLayerV2::layerType() const
{
  return "SimpleFill";
}

void QgsSimpleFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor fillColor = mColor;
  fillColor.setAlphaF( context.alpha() * mColor.alphaF() );
  mBrush = QBrush( fillColor, mBrushStyle );

  // scale brush content for printout
  double rasterScaleFactor = context.renderContext().rasterScaleFactor();
  if ( rasterScaleFactor != 1.0 )
  {
    mBrush.setMatrix( QMatrix().scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor ) );
  }

  QColor selColor = context.renderContext().selectionColor();
  QColor selPenColor = selColor == mColor ? selColor : mBorderColor;
  if ( ! selectionIsOpaque ) selColor.setAlphaF( context.alpha() );
  mSelBrush = QBrush( selColor );
  // N.B. unless a "selection line color" is implemented in addition to the "selection color" option
  // this would mean symbols with "no fill" look the same whether or not they are selected
  if ( selectFillStyle )
    mSelBrush.setStyle( mBrushStyle );

  QColor borderColor = mBorderColor;
  borderColor.setAlphaF( context.alpha() * mBorderColor.alphaF() );
  mPen = QPen( borderColor );
  mSelPen = QPen( selPenColor );
  mPen.setStyle( mBorderStyle );
  mPen.setWidthF( mBorderWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mBorderWidthUnit, mBorderWidthMapUnitScale ) );
  mPen.setJoinStyle( mPenJoinStyle );
  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsSimpleFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSimpleFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
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
    offset.setX( mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  _renderPolygon( p, points, rings, context );

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
}

QgsStringMap QgsSimpleFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["style"] = QgsSymbolLayerV2Utils::encodeBrushStyle( mBrushStyle );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mBorderColor );
  map["outline_style"] = QgsSymbolLayerV2Utils::encodePenStyle( mBorderStyle );
  map["outline_width"] = QString::number( mBorderWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mBorderWidthUnit );
  map["border_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mBorderWidthMapUnitScale );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::clone() const
{
  QgsSimpleFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( mColor, mBrushStyle, mBorderColor, mBorderStyle, mBorderWidth, mPenJoinStyle );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  sl->setBorderWidthUnit( mBorderWidthUnit );
  sl->setBorderWidthMapUnitScale( mBorderWidthMapUnitScale );
  copyDataDefinedProperties( sl );
  return sl;
}

void QgsSimpleFillSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  if ( mBrushStyle == Qt::NoBrush && mBorderStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( "se:PolygonSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  if ( mBrushStyle != Qt::NoBrush )
  {
    // <Fill>
    QDomElement fillElem = doc.createElement( "se:Fill" );
    symbolizerElem.appendChild( fillElem );
    QgsSymbolLayerV2Utils::fillToSld( doc, fillElem, mBrushStyle, mColor );
  }

  if ( mBorderStyle != Qt::NoPen )
  {
    // <Stroke>
    QDomElement strokeElem = doc.createElement( "se:Stroke" );
    symbolizerElem.appendChild( strokeElem );
    QgsSymbolLayerV2Utils::lineToSld( doc, strokeElem, mBorderStyle, mBorderColor, mBorderWidth, &mPenJoinStyle );
  }

  // <se:Displacement>
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, symbolizerElem, mOffset );
}

QString QgsSimpleFillSymbolLayerV2::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  //brush
  QString symbolStyle;
  symbolStyle.append( QgsSymbolLayerV2Utils::ogrFeatureStyleBrush( mColor ) );
  symbolStyle.append( ";" );
  //pen
  symbolStyle.append( QgsSymbolLayerV2Utils::ogrFeatureStylePen( mBorderWidth, mmScaleFactor, mapUnitScaleFactor, mBorderColor, mPenJoinStyle ) );
  return symbolStyle;
}

QgsSymbolLayerV2* QgsSimpleFillSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QColor color, borderColor;
  Qt::BrushStyle fillStyle;
  Qt::PenStyle borderStyle;
  double borderWidth;

  QDomElement fillElem = element.firstChildElement( "Fill" );
  QgsSymbolLayerV2Utils::fillFromSld( fillElem, fillStyle, color );

  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  QgsSymbolLayerV2Utils::lineFromSld( strokeElem, borderStyle, borderColor, borderWidth );

  QPointF offset;
  QgsSymbolLayerV2Utils::displacementFromSldElement( element, offset );

  QgsSimpleFillSymbolLayerV2* sl = new QgsSimpleFillSymbolLayerV2( color, fillStyle, borderColor, borderStyle, borderWidth );
  sl->setOffset( offset );
  return sl;
}

double QgsSimpleFillSymbolLayerV2::estimateMaxBleed() const
{
  double penBleed = mBorderStyle == Qt::NoPen ? 0 : ( mBorderWidth / 2.0 );
  double offsetBleed = mOffset.x() > mOffset.y() ? mOffset.x() : mOffset.y();
  return penBleed + offsetBleed;
}

double QgsSimpleFillSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  double width = mBorderWidth;
  QgsExpression* widthBorderExpression = expression( "width_border" );
  if ( widthBorderExpression )
  {
    width = widthBorderExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  return width * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), mBorderWidthUnit, e.mapUnits() );
}

QColor QgsSimpleFillSymbolLayerV2::dxfColor( const QgsSymbolV2RenderContext& context ) const
{
  QgsExpression* colorExpression = expression( "border_color" );
  if ( colorExpression )
  {
    return QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  return mBorderColor;
}

Qt::PenStyle QgsSimpleFillSymbolLayerV2::dxfPenStyle() const
{
  return mBorderStyle;
}

QColor QgsSimpleFillSymbolLayerV2::dxfBrushColor( const QgsSymbolV2RenderContext& context ) const
{
  QgsExpression* colorExpression = expression( "color" );
  if ( colorExpression )
  {
    return QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  return mColor;
}

Qt::BrushStyle QgsSimpleFillSymbolLayerV2::dxfBrushStyle() const
{
  return mBrushStyle;
}

//QgsGradientFillSymbolLayer

QgsGradientFillSymbolLayerV2::QgsGradientFillSymbolLayerV2( QColor color, QColor color2,
    GradientColorType colorType, GradientType gradientType,
    GradientCoordinateMode coordinateMode, GradientSpread spread )
    : mGradientColorType( colorType )
    , mGradientRamp( NULL )
    , mGradientType( gradientType )
    , mCoordinateMode( coordinateMode )
    , mGradientSpread( spread )
    , mReferencePoint1( QPointF( 0.5, 0 ) )
    , mReferencePoint1IsCentroid( false )
    , mReferencePoint2( QPointF( 0.5, 1 ) )
    , mReferencePoint2IsCentroid( false )
    , mOffsetUnit( QgsSymbolV2::MM )
{
  mColor = color;
  mColor2 = color2;
}

QgsGradientFillSymbolLayerV2::~QgsGradientFillSymbolLayerV2()
{
  delete mGradientRamp;
}

QgsSymbolLayerV2* QgsGradientFillSymbolLayerV2::create( const QgsStringMap& props )
{
  //default to a two-color, linear gradient with feature mode and pad spreading
  GradientType type = QgsGradientFillSymbolLayerV2::Linear;
  GradientColorType colorType = QgsGradientFillSymbolLayerV2::SimpleTwoColor;
  GradientCoordinateMode coordinateMode = QgsGradientFillSymbolLayerV2::Feature;
  GradientSpread gradientSpread = QgsGradientFillSymbolLayerV2::Pad;
  //default to gradient from the default fill color to white
  QColor color = DEFAULT_SIMPLEFILL_COLOR, color2 = Qt::white;
  QPointF referencePoint1 = QPointF( 0.5, 0 );
  bool refPoint1IsCentroid = false;
  QPointF referencePoint2 = QPointF( 0.5, 1 );
  bool refPoint2IsCentroid = false;
  double angle = 0;
  QPointF offset;

  //update gradient properties from props
  if ( props.contains( "type" ) )
    type = ( GradientType )props["type"].toInt();
  if ( props.contains( "coordinate_mode" ) )
    coordinateMode = ( GradientCoordinateMode )props["coordinate_mode"].toInt();
  if ( props.contains( "spread" ) )
    gradientSpread = ( GradientSpread )props["spread"].toInt();
  if ( props.contains( "color_type" ) )
    colorType = ( GradientColorType )props["color_type"].toInt();
  if ( props.contains( "gradient_color" ) )
  {
    //pre 2.5 projects used "gradient_color"
    color = QgsSymbolLayerV2Utils::decodeColor( props["gradient_color"] );
  }
  else if ( props.contains( "color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  }
  if ( props.contains( "gradient_color2" ) )
  {
    color2 = QgsSymbolLayerV2Utils::decodeColor( props["gradient_color2"] );
  }

  if ( props.contains( "reference_point1" ) )
    referencePoint1 = QgsSymbolLayerV2Utils::decodePoint( props["reference_point1"] );
  if ( props.contains( "reference_point1_iscentroid" ) )
    refPoint1IsCentroid = props["reference_point1_iscentroid"].toInt();
  if ( props.contains( "reference_point2" ) )
    referencePoint2 = QgsSymbolLayerV2Utils::decodePoint( props["reference_point2"] );
  if ( props.contains( "reference_point2_iscentroid" ) )
    refPoint2IsCentroid = props["reference_point2_iscentroid"].toInt();
  if ( props.contains( "angle" ) )
    angle = props["angle"].toDouble();

  if ( props.contains( "offset" ) )
    offset = QgsSymbolLayerV2Utils::decodePoint( props["offset"] );

  //attempt to create color ramp from props
  QgsVectorColorRampV2* gradientRamp = QgsVectorGradientColorRampV2::create( props );

  //create a new gradient fill layer with desired properties
  QgsGradientFillSymbolLayerV2* sl = new QgsGradientFillSymbolLayerV2( color, color2, colorType, type, coordinateMode, gradientSpread );
  sl->setOffset( offset );
  if ( props.contains( "offset_unit" ) )
    sl->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    sl->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  sl->setReferencePoint1( referencePoint1 );
  sl->setReferencePoint1IsCentroid( refPoint1IsCentroid );
  sl->setReferencePoint2( referencePoint2 );
  sl->setReferencePoint2IsCentroid( refPoint2IsCentroid );
  sl->setAngle( angle );
  if ( gradientRamp )
    sl->setColorRamp( gradientRamp );

  //data defined symbology expressions
  if ( props.contains( "color_expression" ) )
    sl->setDataDefinedProperty( "color", props["color_expression"] );
  if ( props.contains( "color2_expression" ) )
    sl->setDataDefinedProperty( "color2", props["color2_expression"] );
  if ( props.contains( "angle_expression" ) )
    sl->setDataDefinedProperty( "angle", props["angle_expression"] );
  if ( props.contains( "gradient_type_expression" ) )
    sl->setDataDefinedProperty( "gradient_type", props["gradient_type_expression"] );
  if ( props.contains( "coordinate_mode_expression" ) )
    sl->setDataDefinedProperty( "coordinate_mode", props["coordinate_mode_expression"] );
  if ( props.contains( "spread_expression" ) )
    sl->setDataDefinedProperty( "spread", props["spread_expression"] );
  if ( props.contains( "reference1_x_expression" ) )
    sl->setDataDefinedProperty( "reference1_x", props["reference1_x_expression"] );
  if ( props.contains( "reference1_y_expression" ) )
    sl->setDataDefinedProperty( "reference1_y", props["reference1_y_expression"] );
  if ( props.contains( "reference1_iscentroid_expression" ) )
    sl->setDataDefinedProperty( "reference1_iscentroid", props["reference1_iscentroid_expression"] );
  if ( props.contains( "reference2_x_expression" ) )
    sl->setDataDefinedProperty( "reference2_x", props["reference2_x_expression"] );
  if ( props.contains( "reference2_y_expression" ) )
    sl->setDataDefinedProperty( "reference2_y", props["reference2_y_expression"] );
  if ( props.contains( "reference2_iscentroid_expression" ) )
    sl->setDataDefinedProperty( "reference2_iscentroid", props["reference2_iscentroid_expression"] );

  return sl;
}

void QgsGradientFillSymbolLayerV2::setColorRamp( QgsVectorColorRampV2* ramp )
{
  delete mGradientRamp;
  mGradientRamp = ramp;
}

QString QgsGradientFillSymbolLayerV2::layerType() const
{
  return "GradientFill";
}

void QgsGradientFillSymbolLayerV2::applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, const QPolygonF& points )
{

  if ( mDataDefinedProperties.isEmpty() && !mReferencePoint1IsCentroid && !mReferencePoint2IsCentroid )
  {
    //shortcut
    applyGradient( context, mBrush, mColor, mColor2,  mGradientColorType, mGradientRamp, mGradientType, mCoordinateMode,
                   mGradientSpread, mReferencePoint1, mReferencePoint2, mAngle );
    return;
  }

  //first gradient color
  QgsExpression* colorExpression = expression( "color" );
  QColor color = mColor;
  if ( colorExpression )
    color = QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );

  //second gradient color
  QgsExpression* colorExpression2 = expression( "color2" );
  QColor color2 = mColor2;
  if ( colorExpression2 )
    color2 = QgsSymbolLayerV2Utils::decodeColor( colorExpression2->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );

  //gradient rotation angle
  QgsExpression* angleExpression = expression( "angle" );
  double angle = mAngle;
  if ( angleExpression )
    angle = angleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();

  //gradient type
  QgsExpression* typeExpression = expression( "gradient_type" );
  QgsGradientFillSymbolLayerV2::GradientType gradientType = mGradientType;
  if ( typeExpression )
  {
    QString currentType = typeExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    if ( currentType == QObject::tr( "linear" ) )
    {
      gradientType = QgsGradientFillSymbolLayerV2::Linear;
    }
    else if ( currentType == QObject::tr( "radial" ) )
    {
      gradientType = QgsGradientFillSymbolLayerV2::Radial;
    }
    else if ( currentType == QObject::tr( "conical" ) )
    {
      gradientType = QgsGradientFillSymbolLayerV2::Conical;
    }
    else
    {
      //default to linear
      gradientType = QgsGradientFillSymbolLayerV2::Linear;
    }
  }

  //coordinate mode
  QgsExpression* coordModeExpression = expression( "coordinate_mode" );
  GradientCoordinateMode coordinateMode = mCoordinateMode;
  if ( coordModeExpression )
  {
    QString currentCoordMode = coordModeExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    if ( currentCoordMode == QObject::tr( "feature" ) )
    {
      coordinateMode = QgsGradientFillSymbolLayerV2::Feature;
    }
    else if ( currentCoordMode == QObject::tr( "viewport" ) )
    {
      coordinateMode = QgsGradientFillSymbolLayerV2::Viewport;
    }
    else
    {
      //default to feature mode
      coordinateMode = QgsGradientFillSymbolLayerV2::Feature;
    }
  }

  //gradient spread
  QgsExpression* spreadExpression = expression( "spread" );
  GradientSpread spread = mGradientSpread;
  if ( spreadExpression )
  {
    QString currentSpread = spreadExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    if ( currentSpread == QObject::tr( "pad" ) )
    {
      spread = QgsGradientFillSymbolLayerV2::Pad;
    }
    else if ( currentSpread == QObject::tr( "repeat" ) )
    {
      spread = QgsGradientFillSymbolLayerV2::Repeat;
    }
    else if ( currentSpread == QObject::tr( "reflect" ) )
    {
      spread = QgsGradientFillSymbolLayerV2::Reflect;
    }
    else
    {
      //default to pad spread
      spread = QgsGradientFillSymbolLayerV2::Pad;
    }
  }

  //reference point 1 x & y
  QgsExpression* ref1XExpression = expression( "reference1_x" );
  double refPoint1X = mReferencePoint1.x();
  if ( ref1XExpression )
    refPoint1X = ref1XExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  QgsExpression* ref1YExpression = expression( "reference1_y" );
  double refPoint1Y = mReferencePoint1.y();
  if ( ref1YExpression )
    refPoint1Y = ref1YExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  QgsExpression* ref1IsCentroidExpression = expression( "reference1_iscentroid" );
  bool refPoint1IsCentroid = mReferencePoint1IsCentroid;
  if ( ref1IsCentroidExpression )
    refPoint1IsCentroid = ref1IsCentroidExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toBool();

  //reference point 2 x & y
  QgsExpression* ref2XExpression = expression( "reference2_x" );
  double refPoint2X = mReferencePoint2.x();
  if ( ref2XExpression )
    refPoint2X = ref2XExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  QgsExpression* ref2YExpression = expression( "reference2_y" );
  double refPoint2Y = mReferencePoint2.y();
  if ( ref2YExpression )
    refPoint2Y = ref2YExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  QgsExpression* ref2IsCentroidExpression = expression( "reference2_iscentroid" );
  bool refPoint2IsCentroid = mReferencePoint2IsCentroid;
  if ( ref2IsCentroidExpression )
    refPoint2IsCentroid = ref2IsCentroidExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toBool();

  if ( refPoint1IsCentroid || refPoint2IsCentroid )
  {
    //either the gradient is starting or ending at a centroid, so calculate it
    QPointF centroid = QgsSymbolLayerV2Utils::polygonCentroid( points );
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

QPointF QgsGradientFillSymbolLayerV2::rotateReferencePoint( const QPointF & refPoint, double angle )
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

void QgsGradientFillSymbolLayerV2::applyGradient( const QgsSymbolV2RenderContext &context, QBrush &brush,
    const QColor &color, const QColor &color2, const GradientColorType &gradientColorType,
    QgsVectorColorRampV2 *gradientRamp, const GradientType &gradientType,
    const GradientCoordinateMode &coordinateMode, const GradientSpread &gradientSpread,
    const QPointF &referencePoint1, const QPointF &referencePoint2, const double angle )
{
  //update alpha of gradient colors
  QColor fillColor = color;
  fillColor.setAlphaF( context.alpha() * fillColor.alphaF() );
  QColor fillColor2 = color2;
  fillColor2.setAlphaF( context.alpha() * fillColor2.alphaF() );

  //rotate reference points
  QPointF rotatedReferencePoint1 = angle != 0 ? rotateReferencePoint( referencePoint1, angle ) : referencePoint1;
  QPointF rotatedReferencePoint2 = angle != 0 ? rotateReferencePoint( referencePoint2, angle ) : referencePoint2;

  //create a QGradient with the desired properties
  QGradient gradient;
  switch ( gradientType )
  {
    case QgsGradientFillSymbolLayerV2::Linear:
      gradient = QLinearGradient( rotatedReferencePoint1, rotatedReferencePoint2 );
      break;
    case QgsGradientFillSymbolLayerV2::Radial:
      gradient = QRadialGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).length() );
      break;
    case QgsGradientFillSymbolLayerV2::Conical:
      gradient = QConicalGradient( rotatedReferencePoint1, QLineF( rotatedReferencePoint1, rotatedReferencePoint2 ).angle() );
      break;
  }
  switch ( coordinateMode )
  {
    case QgsGradientFillSymbolLayerV2::Feature:
      gradient.setCoordinateMode( QGradient::ObjectBoundingMode );
      break;
    case QgsGradientFillSymbolLayerV2::Viewport:
      gradient.setCoordinateMode( QGradient::StretchToDeviceMode );
      break;
  }
  switch ( gradientSpread )
  {
    case QgsGradientFillSymbolLayerV2::Pad:
      gradient.setSpread( QGradient::PadSpread );
      break;
    case QgsGradientFillSymbolLayerV2::Reflect:
      gradient.setSpread( QGradient::ReflectSpread );
      break;
    case QgsGradientFillSymbolLayerV2::Repeat:
      gradient.setSpread( QGradient::RepeatSpread );
      break;
  }

  //add stops to gradient
  if ( gradientColorType == QgsGradientFillSymbolLayerV2::ColorRamp && gradientRamp && gradientRamp->type() == "gradient" )
  {
    //color ramp gradient
    QgsVectorGradientColorRampV2* gradRamp = static_cast<QgsVectorGradientColorRampV2*>( gradientRamp );
    gradRamp->addStopsToGradient( &gradient, context.alpha() );
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

void QgsGradientFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor selColor = context.renderContext().selectionColor();
  if ( ! selectionIsOpaque ) selColor.setAlphaF( context.alpha() );
  mSelBrush = QBrush( selColor );

  //update mBrush to use a gradient fill with specified properties
  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsGradientFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsGradientFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
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
    offset.setX( mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  _renderPolygon( p, points, rings, context );

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
}

QgsStringMap QgsGradientFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["gradient_color2"] = QgsSymbolLayerV2Utils::encodeColor( mColor2 );
  map["color_type"] = QString::number( mGradientColorType );
  map["type"] = QString::number( mGradientType );
  map["coordinate_mode"] = QString::number( mCoordinateMode );
  map["spread"] = QString::number( mGradientSpread );
  map["reference_point1"] = QgsSymbolLayerV2Utils::encodePoint( mReferencePoint1 );
  map["reference_point1_iscentroid"] = QString::number( mReferencePoint1IsCentroid );
  map["reference_point2"] = QgsSymbolLayerV2Utils::encodePoint( mReferencePoint2 );
  map["reference_point2_iscentroid"] = QString::number( mReferencePoint2IsCentroid );
  map["angle"] = QString::number( mAngle );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  saveDataDefinedProperties( map );
  if ( mGradientRamp )
  {
    map.unite( mGradientRamp->properties() );
  }
  return map;
}

QgsSymbolLayerV2* QgsGradientFillSymbolLayerV2::clone() const
{
  QgsGradientFillSymbolLayerV2* sl = new QgsGradientFillSymbolLayerV2( mColor, mColor2, mGradientColorType, mGradientType, mCoordinateMode, mGradientSpread );
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
  copyDataDefinedProperties( sl );
  return sl;
}

double QgsGradientFillSymbolLayerV2::estimateMaxBleed() const
{
  double offsetBleed = mOffset.x() > mOffset.y() ? mOffset.x() : mOffset.y();
  return offsetBleed;
}

void QgsGradientFillSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsGradientFillSymbolLayerV2::outputUnit() const
{
  return mOffsetUnit;
}

void QgsGradientFillSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsGradientFillSymbolLayerV2::mapUnitScale() const
{
  return mOffsetMapUnitScale;
}

//QgsShapeburstFillSymbolLayer

QgsShapeburstFillSymbolLayerV2::QgsShapeburstFillSymbolLayerV2( QColor color, QColor color2, ShapeburstColorType colorType,
    int blurRadius, bool useWholeShape, double maxDistance ) :

    mBlurRadius( blurRadius ),
    mUseWholeShape( useWholeShape ),
    mMaxDistance( maxDistance ),
    mDistanceUnit( QgsSymbolV2::MM ),
    mColorType( colorType ),
    mColor2( color2 ),
    mGradientRamp( NULL ),
    mTwoColorGradientRamp( 0 ),
    mIgnoreRings( false ),
    mOffsetUnit( QgsSymbolV2::MM )
{
  mColor = color;
}

QgsShapeburstFillSymbolLayerV2::~QgsShapeburstFillSymbolLayerV2()
{
  delete mGradientRamp;
}

QgsSymbolLayerV2* QgsShapeburstFillSymbolLayerV2::create( const QgsStringMap& props )
{
  //default to a two-color gradient
  ShapeburstColorType colorType = QgsShapeburstFillSymbolLayerV2::SimpleTwoColor;
  QColor color = DEFAULT_SIMPLEFILL_COLOR, color2 = Qt::white;
  int blurRadius = 0;
  bool useWholeShape = true;
  double maxDistance = 5;
  QPointF offset;

  //update fill properties from props
  if ( props.contains( "color_type" ) )
  {
    colorType = ( ShapeburstColorType )props["color_type"].toInt();
  }
  if ( props.contains( "shapeburst_color" ) )
  {
    //pre 2.5 projects used "shapeburst_color"
    color = QgsSymbolLayerV2Utils::decodeColor( props["shapeburst_color"] );
  }
  else if ( props.contains( "color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  }

  if ( props.contains( "shapeburst_color2" ) )
  {
    //pre 2.5 projects used "shapeburst_color2"
    color2 = QgsSymbolLayerV2Utils::decodeColor( props["shapeburst_color2"] );
  }
  else if ( props.contains( "gradient_color2" ) )
  {
    color2 = QgsSymbolLayerV2Utils::decodeColor( props["gradient_color2"] );
  }
  if ( props.contains( "blur_radius" ) )
  {
    blurRadius = props["blur_radius"].toInt();
  }
  if ( props.contains( "use_whole_shape" ) )
  {
    useWholeShape = props["use_whole_shape"].toInt();
  }
  if ( props.contains( "max_distance" ) )
  {
    maxDistance = props["max_distance"].toDouble();
  }
  if ( props.contains( "offset" ) )
  {
    offset = QgsSymbolLayerV2Utils::decodePoint( props["offset"] );
  }

  //attempt to create color ramp from props
  QgsVectorColorRampV2* gradientRamp = QgsVectorGradientColorRampV2::create( props );

  //create a new shapeburst fill layer with desired properties
  QgsShapeburstFillSymbolLayerV2* sl = new QgsShapeburstFillSymbolLayerV2( color, color2, colorType, blurRadius, useWholeShape, maxDistance );
  sl->setOffset( offset );
  if ( props.contains( "offset_unit" ) )
  {
    sl->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  }
  if ( props.contains( "distance_unit" ) )
  {
    sl->setDistanceUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["distance_unit"] ) );
  }
  if ( props.contains( "offset_map_unit_scale" ) )
  {
    sl->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  }
  if ( props.contains( "distance_map_unit_scale" ) )
  {
    sl->setDistanceMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["distance_map_unit_scale"] ) );
  }
  if ( props.contains( "ignore_rings" ) )
  {
    sl->setIgnoreRings( props["ignore_rings"].toInt() );
  }
  if ( gradientRamp )
  {
    sl->setColorRamp( gradientRamp );
  }

  if ( props.contains( "color_expression" ) )
    sl->setDataDefinedProperty( "color", props["color_expression"] );
  if ( props.contains( "color2_expression" ) )
    sl->setDataDefinedProperty( "color2", props["color2_expression"] );
  if ( props.contains( "blur_radius_expression" ) )
    sl->setDataDefinedProperty( "blur_radius", props["blur_radius_expression"] );
  if ( props.contains( "use_whole_shape_expression" ) )
    sl->setDataDefinedProperty( "use_whole_shape", props["use_whole_shape_expression"] );
  if ( props.contains( "max_distance_expression" ) )
    sl->setDataDefinedProperty( "max_distance", props["max_distance_expression"] );
  if ( props.contains( "ignore_rings_expression" ) )
    sl->setDataDefinedProperty( "ignore_rings", props["ignore_rings_expression"] );

  return sl;
}

QString QgsShapeburstFillSymbolLayerV2::layerType() const
{
  return "ShapeburstFill";
}

void QgsShapeburstFillSymbolLayerV2::setColorRamp( QgsVectorColorRampV2* ramp )
{
  delete mGradientRamp;
  mGradientRamp = ramp;
}

void QgsShapeburstFillSymbolLayerV2::applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QColor& color, QColor& color2, int& blurRadius, bool& useWholeShape,
    double& maxDistance, bool& ignoreRings )
{
  //first gradient color
  QgsExpression* colorExpression = expression( "color" );
  color = mColor;
  if ( colorExpression )
    color = QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );

  //second gradient color
  QgsExpression* colorExpression2 = expression( "color2" );
  color2 = mColor2;
  if ( colorExpression2 )
    color2 = QgsSymbolLayerV2Utils::decodeColor( colorExpression2->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );

  //blur radius
  QgsExpression* blurRadiusExpression = expression( "blur_radius" );
  blurRadius = mBlurRadius;
  if ( blurRadiusExpression )
    blurRadius = blurRadiusExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toInt();

  //use whole shape
  QgsExpression* useWholeShapeExpression = expression( "use_whole_shape" );
  useWholeShape = mUseWholeShape;
  if ( useWholeShapeExpression )
    useWholeShape = useWholeShapeExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toBool();

  //max distance
  QgsExpression* maxDistanceExpression = expression( "max_distance" );
  maxDistance = mMaxDistance;
  if ( maxDistanceExpression )
    maxDistance = maxDistanceExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();

  //ignore rings
  QgsExpression* ignoreRingsExpression = expression( "ignore_rings" );
  ignoreRings = mIgnoreRings;
  if ( ignoreRingsExpression )
    ignoreRings = ignoreRingsExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toBool();

}

void QgsShapeburstFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  //TODO - check this
  QColor selColor = context.renderContext().selectionColor();
  if ( ! selectionIsOpaque ) selColor.setAlphaF( context.alpha() );
  mSelBrush = QBrush( selColor );

  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsShapeburstFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsShapeburstFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
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
      offset.setX( mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
      offset.setY( mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
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
  if ( !useWholeShape && maxDistance != 0 )
  {
    //convert max distance to pixels
    const QgsRenderContext& ctx = context.renderContext();
    outputPixelMaxDist = maxDistance * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
  }

  //if we are using the two color mode, create a gradient ramp
  if ( mColorType == QgsShapeburstFillSymbolLayerV2::SimpleTwoColor )
  {
    mTwoColorGradientRamp = new QgsVectorGradientColorRampV2( color1, color2 );
  }

  //no border for shapeburst fills
  p->setPen( QPen( Qt::NoPen ) );

  //calculate margin size in pixels so that QImage of polygon has sufficient space to draw the full blur effect
  int sideBuffer = 4 + ( blurRadius + 2 ) * 4;
  //create a QImage to draw shapeburst in
  double imWidth = points.boundingRect().width() + ( sideBuffer * 2 );
  double imHeight = points.boundingRect().height() + ( sideBuffer * 2 );
  QImage * fillImage = new QImage( imWidth * context.renderContext().rasterScaleFactor(),
                                   imHeight * context.renderContext().rasterScaleFactor(), QImage::Format_ARGB32_Premultiplied );
  //Fill this image with black. Initially the distance transform is drawn in greyscale, where black pixels have zero distance from the
  //polygon boundary. Since we don't care about pixels which fall outside the polygon, we start with a black image and then draw over it the
  //polygon in white. The distance transform function then fills in the correct distance values for the white pixels.
  fillImage->fill( Qt::black );

  //also create an image to store the alpha channel
  QImage * alphaImage = new QImage( fillImage->width(), fillImage->height(), QImage::Format_ARGB32_Premultiplied );
  //initially fill the alpha channel image with a transparent color
  alphaImage->fill( Qt::transparent );

  //now, draw the polygon in the alpha channel image
  QPainter imgPainter;
  imgPainter.begin( alphaImage );
  imgPainter.setRenderHint( QPainter::Antialiasing, true );
  imgPainter.setBrush( QBrush( Qt::white ) );
  imgPainter.setPen( QPen( Qt::black ) );
  imgPainter.translate( -points.boundingRect().left() + sideBuffer, - points.boundingRect().top() + sideBuffer );
  imgPainter.scale( context.renderContext().rasterScaleFactor(), context.renderContext().rasterScaleFactor() );
  _renderPolygon( &imgPainter, points, rings, context );
  imgPainter.end();

  //now that we have a render of the polygon in white, draw this onto the shapeburst fill image too
  //(this avoids calling _renderPolygon twice, since that can be slow)
  imgPainter.begin( fillImage );
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
    imgPainter.scale( context.renderContext().rasterScaleFactor(), context.renderContext().rasterScaleFactor() );
    _renderPolygon( &imgPainter, points, NULL, context );
  }
  imgPainter.end();

  //apply distance transform to image, uses the current color ramp to calculate final pixel colors
  double * dtArray = distanceTransform( fillImage );

  //copy distance transform values back to QImage, shading by appropriate color ramp
  dtArrayToQImage( dtArray, fillImage, mColorType == QgsShapeburstFillSymbolLayerV2::SimpleTwoColor ? mTwoColorGradientRamp : mGradientRamp,
                   context.alpha(), useWholeShape, outputPixelMaxDist );

  //clean up some variables
  delete [] dtArray;
  if ( mColorType == QgsShapeburstFillSymbolLayerV2::SimpleTwoColor )
  {
    delete mTwoColorGradientRamp;
  }

  //apply blur if desired
  if ( blurRadius > 0 )
  {
    QgsSymbolLayerV2Utils::blurImageInPlace( *fillImage, QRect( 0, 0, fillImage->width(), fillImage->height() ), blurRadius, false );
  }

  //apply alpha channel to distance transform image, so that areas outside the polygon are transparent
  imgPainter.begin( fillImage );
  imgPainter.setCompositionMode( QPainter::CompositionMode_DestinationIn );
  imgPainter.drawImage( 0, 0, *alphaImage );
  imgPainter.end();
  //we're finished with the alpha channel image now
  delete alphaImage;

  //draw shapeburst image in correct place in the destination painter

  p->save();
  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    p->translate( offset );
  }

  p->scale( 1 / context.renderContext().rasterScaleFactor(), 1 / context.renderContext().rasterScaleFactor() );
  p->drawImage( points.boundingRect().left() - sideBuffer, points.boundingRect().top() - sideBuffer, *fillImage );

  delete fillImage;

  if ( !mOffset.isNull() )
  {
    p->translate( -offset );
  }
  p->restore();

}

//fast distance transform code, adapted from http://cs.brown.edu/~pff/dt/

/* distance transform of a 1d function using squared distance */
void QgsShapeburstFillSymbolLayerV2::distanceTransform1d( double *f, int n, int *v, double *z, double *d )
{
  int k = 0;
  v[0] = 0;
  z[0] = -INF;
  z[1] = + INF;
  for ( int q = 1; q <= n - 1; q++ )
  {
    double s  = (( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    while ( s <= z[k] )
    {
      k--;
      s  = (( f[q] + q * q ) - ( f[v[k]] + ( v[k] * v[k] ) ) ) / ( 2 * q - 2 * v[k] );
    }
    k++;
    v[k] = q;
    z[k] = s;
    z[k+1] = + INF;
  }

  k = 0;
  for ( int q = 0; q <= n - 1; q++ )
  {
    while ( z[k+1] < q )
      k++;
    d[q] = ( q - v[k] ) * ( q - v[k] ) + f[v[k]];
  }
}

/* distance transform of 2d function using squared distance */
void QgsShapeburstFillSymbolLayerV2::distanceTransform2d( double * im, int width, int height )
{
  int maxDimension = qMax( width, height );
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
      f[x] = im[  x + y*width ];
    }
    distanceTransform1d( f, width, v, z, d );
    for ( int x = 0; x < width; x++ )
    {
      im[  x + y*width ] = d[x];
    }
  }

  delete [] d;
  delete [] f;
  delete [] v;
  delete [] z;
}

/* distance transform of a binary QImage */
double * QgsShapeburstFillSymbolLayerV2::distanceTransform( QImage *im )
{
  int width = im->width();
  int height = im->height();

  double * dtArray = new double[width * height];

  //load qImage to array
  QRgb tmpRgb;
  int idx = 0;
  for ( int heightIndex = 0; heightIndex < height; ++heightIndex )
  {
    QRgb* scanLine = ( QRgb* )im->constScanLine( heightIndex );
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

void QgsShapeburstFillSymbolLayerV2::dtArrayToQImage( double * array, QImage *im, QgsVectorColorRampV2* ramp, double layerAlpha, bool useWholeShape, int maxPixelDistance )
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
    maxDistanceValue = sqrt( dtMaxValue );
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
    QRgb* scanLine = ( QRgb* )im->scanLine( heightIndex );
    for ( int widthIndex = 0; widthIndex < width; ++widthIndex )
    {
      //result of distance transform
      squaredVal = array[idx];

      //scale result to fit in the range [0, 1]
      pixVal = squaredVal > 0 ? qMin(( sqrt( squaredVal ) / maxDistanceValue ), 1.0 ) : 0;

      //convert value to color from ramp
      pixColor = ramp->color( pixVal );

      int pixAlpha = pixColor.alpha();
      if (( layerHasAlpha ) || ( pixAlpha != 255 ) )
      {
        //apply layer's transparency to alpha value
        double alpha = pixAlpha * layerAlpha;
        //premultiply ramp color since we are storing this in a ARGB32_Premultiplied QImage
        QgsSymbolLayerV2Utils::premultiplyColor( pixColor, alpha );
      }

      scanLine[widthIndex] = pixColor.rgba();
      idx++;
    }
  }
}

QgsStringMap QgsShapeburstFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["gradient_color2"] = QgsSymbolLayerV2Utils::encodeColor( mColor2 );
  map["color_type"] = QString::number( mColorType );
  map["blur_radius"] = QString::number( mBlurRadius );
  map["use_whole_shape"] = QString::number( mUseWholeShape );
  map["max_distance"] = QString::number( mMaxDistance );
  map["distance_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mDistanceUnit );
  map["distance_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mDistanceMapUnitScale );
  map["ignore_rings"] = QString::number( mIgnoreRings );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );

  saveDataDefinedProperties( map );

  if ( mGradientRamp )
  {
    map.unite( mGradientRamp->properties() );
  }

  return map;
}

QgsSymbolLayerV2* QgsShapeburstFillSymbolLayerV2::clone() const
{
  QgsShapeburstFillSymbolLayerV2* sl = new QgsShapeburstFillSymbolLayerV2( mColor, mColor2, mColorType, mBlurRadius, mUseWholeShape, mMaxDistance );
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
  copyDataDefinedProperties( sl );
  return sl;
}

double QgsShapeburstFillSymbolLayerV2::estimateMaxBleed() const
{
  double offsetBleed = qMax( mOffset.x(), mOffset.y() );
  return offsetBleed;
}

void QgsShapeburstFillSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mDistanceUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsShapeburstFillSymbolLayerV2::outputUnit() const
{
  if ( mDistanceUnit == mOffsetUnit )
  {
    return mDistanceUnit;
  }
  return QgsSymbolV2::Mixed;
}

void QgsShapeburstFillSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mDistanceMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
}

QgsMapUnitScale QgsShapeburstFillSymbolLayerV2::mapUnitScale() const
{
  if ( mDistanceMapUnitScale == mOffsetMapUnitScale )
  {
    return mDistanceMapUnitScale;
  }
  return QgsMapUnitScale();
}


//QgsImageFillSymbolLayer

QgsImageFillSymbolLayer::QgsImageFillSymbolLayer(): mOutlineWidth( 0.0 ), mOutlineWidthUnit( QgsSymbolV2::MM ), mOutline( 0 )
{
  setSubSymbol( new QgsLineSymbolV2() );
}

QgsImageFillSymbolLayer::~QgsImageFillSymbolLayer()
{
}

void QgsImageFillSymbolLayer::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  mNextAngle = mAngle;
  applyDataDefinedSettings( context );

  p->setPen( QPen( Qt::NoPen ) );
  if ( context.selected() )
  {
    QColor selColor = context.renderContext().selectionColor();
    // Alister - this doesn't seem to work here
    //if ( ! selectionIsOpaque )
    //  selColor.setAlphaF( context.alpha() );
    p->setBrush( QBrush( selColor ) );
    _renderPolygon( p, points, rings, context );
  }

  if ( qgsDoubleNear( mNextAngle, 0.0 ) )
  {
    p->setBrush( mBrush );
  }
  else
  {
    QTransform t = mBrush.transform();
    t.rotate( mNextAngle );
    QBrush rotatedBrush = mBrush;
    rotatedBrush.setTransform( t );
    p->setBrush( rotatedBrush );
  }
  _renderPolygon( p, points, rings, context );
  if ( mOutline )
  {
    mOutline->renderPolyline( points, context.feature(), context.renderContext(), -1, selectFillBorder && context.selected() );
    if ( rings )
    {
      QList<QPolygonF>::const_iterator ringIt = rings->constBegin();
      for ( ; ringIt != rings->constEnd(); ++ringIt )
      {
        mOutline->renderPolyline( *ringIt, context.feature(), context.renderContext(), -1, selectFillBorder && context.selected() );
      }
    }
  }
}

bool QgsImageFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( !symbol ) //unset current outline
  {
    delete mOutline;
    mOutline = 0;
    return true;
  }

  if ( symbol->type() != QgsSymbolV2::Line )
  {
    delete symbol;
    return false;
  }

  QgsLineSymbolV2* lineSymbol = dynamic_cast<QgsLineSymbolV2*>( symbol );
  if ( lineSymbol )
  {
    delete mOutline;
    mOutline = lineSymbol;
    return true;
  }

  delete symbol;
  return false;
}

void QgsImageFillSymbolLayer::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsImageFillSymbolLayer::outputUnit() const
{
  return mOutlineWidthUnit;
}

void QgsImageFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale& scale )
{
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsImageFillSymbolLayer::mapUnitScale() const
{
  return mOutlineWidthMapUnitScale;
}

double QgsImageFillSymbolLayer::estimateMaxBleed() const
{
  if ( mOutline && mOutline->symbolLayer( 0 ) )
  {
    double subLayerBleed = mOutline->symbolLayer( 0 )->estimateMaxBleed();
    return subLayerBleed;
  }
  return 0;
}

double QgsImageFillSymbolLayer::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  double width = mOutlineWidth;
  QgsExpression* widthExpression = expression( "width" );
  if ( widthExpression )
  {
    width = widthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  return width * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), mOutlineWidthUnit, e.mapUnits() );
}

QColor QgsImageFillSymbolLayer::dxfColor( const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( context );
  if ( !mOutline )
  {
    return QColor( Qt::black );
  }
  return mOutline->color();
}

Qt::PenStyle QgsImageFillSymbolLayer::dxfPenStyle() const
{
  return Qt::SolidLine;
#if 0
  if ( !mOutline )
  {
    return Qt::SolidLine;
  }
  else
  {
    return mOutline->dxfPenStyle();
  }
#endif //0
}


//QgsSVGFillSymbolLayer

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QString& svgFilePath, double width, double angle ): QgsImageFillSymbolLayer(),
    mPatternWidth( width ),
    mPatternWidthUnit( QgsSymbolV2::MM ),
    mSvgOutlineWidthUnit( QgsSymbolV2::MM )
{
  setSvgFilePath( svgFilePath );
  mOutlineWidth = 0.3;
  mAngle = angle;
  setDefaultSvgParams();
  mSvgPattern = 0;
}

QgsSVGFillSymbolLayer::QgsSVGFillSymbolLayer( const QByteArray& svgData, double width, double angle ): QgsImageFillSymbolLayer(),
    mPatternWidth( width ),
    mPatternWidthUnit( QgsSymbolV2::MM ),
    mSvgData( svgData ),
    mSvgOutlineWidthUnit( QgsSymbolV2::MM )
{
  storeViewBox();
  mOutlineWidth = 0.3;
  mAngle = angle;
  setSubSymbol( new QgsLineSymbolV2() );
  setDefaultSvgParams();
  mSvgPattern = 0;
}

QgsSVGFillSymbolLayer::~QgsSVGFillSymbolLayer()
{
  delete mSvgPattern;
}

void QgsSVGFillSymbolLayer::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mPatternWidthUnit = unit;
  mSvgOutlineWidthUnit = unit;
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsSVGFillSymbolLayer::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mPatternWidthUnit != unit || mSvgOutlineWidthUnit != unit || mOutlineWidthUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsSVGFillSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsImageFillSymbolLayer::setMapUnitScale( scale );
  mPatternWidthMapUnitScale = scale;
  mSvgOutlineWidthMapUnitScale = scale;
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsSVGFillSymbolLayer::mapUnitScale() const
{
  if ( QgsImageFillSymbolLayer::mapUnitScale() == mPatternWidthMapUnitScale &&
       mPatternWidthMapUnitScale == mSvgOutlineWidthMapUnitScale &&
       mSvgOutlineWidthMapUnitScale == mOutlineWidthMapUnitScale )
  {
    return mPatternWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

void QgsSVGFillSymbolLayer::setSvgFilePath( const QString& svgPath )
{
  mSvgData = QgsSvgCache::instance()->getImageData( svgPath );
  storeViewBox();

  mSvgFilePath = svgPath;
  setDefaultSvgParams();
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::create( const QgsStringMap& properties )
{
  QByteArray data;
  double width = 20;
  QString svgFilePath;
  double angle = 0.0;

  if ( properties.contains( "width" ) )
  {
    width = properties["width"].toDouble();
  }
  if ( properties.contains( "svgFile" ) )
  {
    QString svgName = properties["svgFile"];
    QString savePath = QgsSymbolLayerV2Utils::symbolNameToPath( svgName );
    svgFilePath = ( savePath.isEmpty() ? svgName : savePath );
  }
  if ( properties.contains( "angle" ) )
  {
    angle = properties["angle"].toDouble();
  }

  QgsSVGFillSymbolLayer* symbolLayer = 0;
  if ( !svgFilePath.isEmpty() )
  {
    symbolLayer = new QgsSVGFillSymbolLayer( svgFilePath, width, angle );
  }
  else
  {
    if ( properties.contains( "data" ) )
    {
      data = QByteArray::fromHex( properties["data"].toLocal8Bit() );
    }
    symbolLayer = new QgsSVGFillSymbolLayer( data, width, angle );
  }

  //svg parameters
  if ( properties.contains( "svgFillColor" ) )
  {
    //pre 2.5 projects used "svgFillColor"
    symbolLayer->setSvgFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["svgFillColor"] ) );
  }
  else if ( properties.contains( "color" ) )
  {
    symbolLayer->setSvgFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["color"] ) );
  }
  if ( properties.contains( "svgOutlineColor" ) )
  {
    //pre 2.5 projects used "svgOutlineColor"
    symbolLayer->setSvgOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["svgOutlineColor"] ) );
  }
  else if ( properties.contains( "outline_color" ) )
  {
    symbolLayer->setSvgOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] ) );
  }
  else if ( properties.contains( "line_color" ) )
  {
    symbolLayer->setSvgOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["line_color"] ) );
  }
  if ( properties.contains( "svgOutlineWidth" ) )
  {
    //pre 2.5 projects used "svgOutlineWidth"
    symbolLayer->setSvgOutlineWidth( properties["svgOutlineWidth"].toDouble() );
  }
  else if ( properties.contains( "outline_width" ) )
  {
    symbolLayer->setSvgOutlineWidth( properties["outline_width"].toDouble() );
  }
  else if ( properties.contains( "line_width" ) )
  {
    symbolLayer->setSvgOutlineWidth( properties["line_width"].toDouble() );
  }

  //units
  if ( properties.contains( "pattern_width_unit" ) )
  {
    symbolLayer->setPatternWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["pattern_width_unit"] ) );
  }
  if ( properties.contains( "pattern_width_map_unit_scale" ) )
  {
    symbolLayer->setPatternWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["pattern_width_map_unit_scale"] ) );
  }
  if ( properties.contains( "svg_outline_width_unit" ) )
  {
    symbolLayer->setSvgOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["svg_outline_width_unit"] ) );
  }
  if ( properties.contains( "svg_outline_width_map_unit_scale" ) )
  {
    symbolLayer->setSvgOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["svg_outline_width_map_unit_scale"] ) );
  }
  if ( properties.contains( "outline_width_unit" ) )
  {
    symbolLayer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  if ( properties.contains( "outline_width_map_unit_scale" ) )
  {
    symbolLayer->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["outline_width_map_unit_scale"] ) );
  }

  if ( properties.contains( "width_expression" ) )
    symbolLayer->setDataDefinedProperty( "width", properties["width_expression"] );
  if ( properties.contains( "svgFile_expression" ) )
    symbolLayer->setDataDefinedProperty( "svgFile", properties["svgFile_expression"] );
  if ( properties.contains( "angle_expression" ) )
    symbolLayer->setDataDefinedProperty( "angle", properties["angle_expression"] );
  if ( properties.contains( "svgFillColor_expression" ) )
    symbolLayer->setDataDefinedProperty( "svgFillColor", properties["svgFillColor_expression"] );
  if ( properties.contains( "svgOutlineColor_expression" ) )
    symbolLayer->setDataDefinedProperty( "svgOutlineColor", properties["svgOutlineColor_expression"] );
  if ( properties.contains( "svgOutlineWidth_expression" ) )
    symbolLayer->setDataDefinedProperty( "svgOutlineWidth", properties["svgOutlineWidth_expression"] );

  return symbolLayer;
}

QString QgsSVGFillSymbolLayer::layerType() const
{
  return "SVGFill";
}

void QgsSVGFillSymbolLayer::applyPattern( QBrush& brush, const QString& svgFilePath, double patternWidth, QgsSymbolV2::OutputUnit patternWidthUnit,
    const QColor& svgFillColor, const QColor& svgOutlineColor, double svgOutlineWidth,
    QgsSymbolV2::OutputUnit svgOutlineWidthUnit, const QgsSymbolV2RenderContext& context,
    const QgsMapUnitScale& patternWidthMapUnitScale, const QgsMapUnitScale& svgOutlineWidthMapUnitScale )
{
  if ( mSvgViewBox.isNull() )
  {
    return;
  }

  delete mSvgPattern;
  mSvgPattern = 0;
  double size = patternWidth * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context.renderContext(), patternWidthUnit, patternWidthMapUnitScale );

  if (( int )size < 1.0 || 10000.0 < size )
  {
    mSvgPattern = new QImage();
    brush.setTextureImage( *mSvgPattern );
  }
  else
  {
    bool fitsInCache = true;
    double outlineWidth = svgOutlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), svgOutlineWidthUnit, svgOutlineWidthMapUnitScale );
    const QImage& patternImage = QgsSvgCache::instance()->svgAsImage( svgFilePath, size, svgFillColor, svgOutlineColor, outlineWidth,
                                 context.renderContext().scaleFactor(), context.renderContext().rasterScaleFactor(), fitsInCache );
    if ( !fitsInCache )
    {
      const QPicture& patternPict = QgsSvgCache::instance()->svgAsPicture( svgFilePath, size, svgFillColor, svgOutlineColor, outlineWidth,
                                    context.renderContext().scaleFactor(), 1.0 );
      double hwRatio = 1.0;
      if ( patternPict.width() > 0 )
      {
        hwRatio = ( double )patternPict.height() / ( double )patternPict.width();
      }
      mSvgPattern = new QImage(( int )size, ( int )( size * hwRatio ), QImage::Format_ARGB32_Premultiplied );
      mSvgPattern->fill( 0 ); // transparent background

      QPainter p( mSvgPattern );
      p.drawPicture( QPointF( size / 2, size * hwRatio / 2 ), patternPict );
    }

    QTransform brushTransform;
    brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
    if ( !qgsDoubleNear( context.alpha(), 1.0 ) )
    {
      QImage transparentImage = fitsInCache ? patternImage.copy() : mSvgPattern->copy();
      QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
      brush.setTextureImage( transparentImage );
    }
    else
    {
      brush.setTextureImage( fitsInCache ? patternImage : *mSvgPattern );
    }
    brush.setTransform( brushTransform );
  }
}

void QgsSVGFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{

  applyPattern( mBrush, mSvgFilePath, mPatternWidth, mPatternWidthUnit, mSvgFillColor, mSvgOutlineColor, mSvgOutlineWidth, mSvgOutlineWidthUnit, context, mPatternWidthMapUnitScale, mSvgOutlineWidthMapUnitScale );

  if ( mOutline )
  {
    mOutline->startRender( context.renderContext(), context.fields() );
  }

  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsSVGFillSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mOutline )
  {
    mOutline->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsSVGFillSymbolLayer::properties() const
{
  QgsStringMap map;
  if ( !mSvgFilePath.isEmpty() )
  {
    map.insert( "svgFile", QgsSymbolLayerV2Utils::symbolPathToName( mSvgFilePath ) );
  }
  else
  {
    map.insert( "data", QString( mSvgData.toHex() ) );
  }

  map.insert( "width", QString::number( mPatternWidth ) );
  map.insert( "angle", QString::number( mAngle ) );

  //svg parameters
  map.insert( "color", QgsSymbolLayerV2Utils::encodeColor( mSvgFillColor ) );
  map.insert( "outline_color", QgsSymbolLayerV2Utils::encodeColor( mSvgOutlineColor ) );
  map.insert( "outline_width", QString::number( mSvgOutlineWidth ) );

  //units
  map.insert( "pattern_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mPatternWidthUnit ) );
  map.insert( "pattern_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mPatternWidthMapUnitScale ) );
  map.insert( "svg_outline_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mSvgOutlineWidthUnit ) );
  map.insert( "svg_outline_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mSvgOutlineWidthMapUnitScale ) );
  map.insert( "outline_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit ) );
  map.insert( "outline_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale ) );

  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::clone() const
{
  QgsSVGFillSymbolLayer* clonedLayer = 0;
  if ( !mSvgFilePath.isEmpty() )
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgFilePath, mPatternWidth, mAngle );
    clonedLayer->setSvgFillColor( mSvgFillColor );
    clonedLayer->setSvgOutlineColor( mSvgOutlineColor );
    clonedLayer->setSvgOutlineWidth( mSvgOutlineWidth );
  }
  else
  {
    clonedLayer = new QgsSVGFillSymbolLayer( mSvgData, mPatternWidth, mAngle );
  }

  clonedLayer->setPatternWidthUnit( mPatternWidthUnit );
  clonedLayer->setPatternWidthMapUnitScale( mPatternWidthMapUnitScale );
  clonedLayer->setSvgOutlineWidthUnit( mSvgOutlineWidthUnit );
  clonedLayer->setSvgOutlineWidthMapUnitScale( mSvgOutlineWidthMapUnitScale );
  clonedLayer->setOutlineWidthUnit( mOutlineWidthUnit );
  clonedLayer->setOutlineWidthMapUnitScale( mOutlineWidthMapUnitScale );

  if ( mOutline )
  {
    clonedLayer->setSubSymbol( mOutline->clone() );
  }
  copyDataDefinedProperties( clonedLayer );
  return clonedLayer;
}

void QgsSVGFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PolygonSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  QDomElement fillElem = doc.createElement( "se:Fill" );
  symbolizerElem.appendChild( fillElem );

  QDomElement graphicFillElem = doc.createElement( "se:GraphicFill" );
  fillElem.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  graphicFillElem.appendChild( graphicElem );

  if ( !mSvgFilePath.isEmpty() )
  {
    QgsSymbolLayerV2Utils::externalGraphicToSld( doc, graphicElem, mSvgFilePath, "image/svg+xml", mSvgFillColor, mPatternWidth );
  }
  else
  {
    // TODO: create svg from data
    // <se:InlineContent>
    symbolizerElem.appendChild( doc.createComment( "SVG from data not implemented yet" ) );
  }

  if ( mSvgOutlineColor.isValid() || mSvgOutlineWidth >= 0 )
  {
    QgsSymbolLayerV2Utils::lineToSld( doc, graphicElem, Qt::SolidLine, mSvgOutlineColor, mSvgOutlineWidth );
  }

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mAngle );
  }
  else if ( angle + mAngle != 0 )
  {
    angleFunc = QString::number( angle + mAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  if ( mOutline )
  {
    // the outline sub symbol should be stored within the Stroke element,
    // but it will be stored in a separated LineSymbolizer because it could
    // have more than one layer
    mOutline->toSld( doc, element, props );
  }
}

QgsSymbolLayerV2* QgsSVGFillSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QString path, mimeType;
  QColor fillColor, borderColor;
  Qt::PenStyle penStyle;
  double size, borderWidth;

  QDomElement fillElem = element.firstChildElement( "Fill" );
  if ( fillElem.isNull() )
    return NULL;

  QDomElement graphicFillElem = fillElem.firstChildElement( "GraphicFill" );
  if ( graphicFillElem.isNull() )
    return NULL;

  QDomElement graphicElem = graphicFillElem.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  if ( !QgsSymbolLayerV2Utils::externalGraphicFromSld( graphicElem, path, mimeType, fillColor, size ) )
    return NULL;

  if ( mimeType != "image/svg+xml" )
    return NULL;

  QgsSymbolLayerV2Utils::lineFromSld( graphicElem, penStyle, borderColor, borderWidth );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QgsSVGFillSymbolLayer* sl = new QgsSVGFillSymbolLayer( path, size, angle );
  sl->setSvgFillColor( fillColor );
  sl->setSvgOutlineColor( borderColor );
  sl->setSvgOutlineWidth( borderWidth );

  // try to get the outline
  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( !strokeElem.isNull() )
  {
    QgsSymbolLayerV2 *l = QgsSymbolLayerV2Utils::createLineLayerFromSld( strokeElem );
    if ( l )
    {
      QgsSymbolLayerV2List layers;
      layers.append( l );
      sl->setSubSymbol( new QgsLineSymbolV2( layers ) );
    }
  }

  return sl;
}

void QgsSVGFillSymbolLayer::applyDataDefinedSettings( const QgsSymbolV2RenderContext& context )
{
  QgsExpression* widthExpression = expression( "width" );
  QgsExpression* svgFileExpression = expression( "svgFile" );
  QgsExpression* fillColorExpression = expression( "svgFillColor" );
  QgsExpression* outlineColorExpression = expression( "svgOutlineColor" );
  QgsExpression* outlineWidthExpression = expression( "svgOutlineWidth" );
  QgsExpression* angleExpression = expression( "angle" );
  if ( !widthExpression && !svgFileExpression && !fillColorExpression && !outlineColorExpression && !outlineWidthExpression && !angleExpression )
  {
    return; //no data defined settings
  }

  if ( angleExpression )
  {
    mNextAngle = angleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }

  double width = mPatternWidth;
  if ( widthExpression )
  {
    width = widthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  QString svgFile = mSvgFilePath;
  if ( svgFileExpression )
  {
    svgFile = svgFileExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
  }
  QColor svgFillColor = mSvgFillColor;
  if ( fillColorExpression )
  {
    svgFillColor = QgsSymbolLayerV2Utils::decodeColor( fillColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  QColor svgOutlineColor = mSvgOutlineColor;
  if ( outlineColorExpression )
  {
    svgOutlineColor = QgsSymbolLayerV2Utils::decodeColor( outlineColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  double outlineWidth = mSvgOutlineWidth;
  if ( outlineWidthExpression )
  {
    outlineWidth = outlineWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  applyPattern( mBrush, svgFile, width, mPatternWidthUnit, svgFillColor, svgOutlineColor, outlineWidth,
                mSvgOutlineWidthUnit, context, mPatternWidthMapUnitScale, mSvgOutlineWidthMapUnitScale );

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
  return;
}

void QgsSVGFillSymbolLayer::setDefaultSvgParams()
{
  //default values
  mSvgFillColor = QColor( 0, 0, 0 );
  mSvgOutlineColor = QColor( 0, 0, 0 );
  mSvgOutlineWidth = 0.3;

  if ( mSvgFilePath.isEmpty() )
  {
    return;
  }

  bool hasFillParam, hasOutlineParam, hasOutlineWidthParam;
  QColor defaultFillColor, defaultOutlineColor;
  double defaultOutlineWidth;
  QgsSvgCache::instance()->containsParams( mSvgFilePath, hasFillParam, defaultFillColor, hasOutlineParam, defaultOutlineColor, hasOutlineWidthParam,
      defaultOutlineWidth );

  if ( hasFillParam )
  {
    mSvgFillColor = defaultFillColor;
  }
  if ( hasOutlineParam )
  {
    mSvgOutlineColor = defaultOutlineColor;
  }
  if ( hasOutlineWidthParam )
  {
    mSvgOutlineWidth = defaultOutlineWidth;
  }
}


QgsLinePatternFillSymbolLayer::QgsLinePatternFillSymbolLayer(): QgsImageFillSymbolLayer(), mDistanceUnit( QgsSymbolV2::MM ), mLineWidthUnit( QgsSymbolV2::MM ),
    mOffsetUnit( QgsSymbolV2::MM ), mFillLineSymbol( 0 )
{
  setSubSymbol( new QgsLineSymbolV2() );
  QgsImageFillSymbolLayer::setSubSymbol( 0 ); //no outline
}

void QgsLinePatternFillSymbolLayer::setLineWidth( double w )
{
  mFillLineSymbol->setWidth( w );
  mLineWidth = w;
}

void QgsLinePatternFillSymbolLayer::setColor( const QColor& c )
{
  mFillLineSymbol->setColor( c );
  mColor = c;
}

QgsLinePatternFillSymbolLayer::~QgsLinePatternFillSymbolLayer()
{
  delete mFillLineSymbol;
}

bool QgsLinePatternFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2* lineSymbol = dynamic_cast<QgsLineSymbolV2*>( symbol );
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

QgsSymbolV2* QgsLinePatternFillSymbolLayer::subSymbol()
{
  return mFillLineSymbol;
}

double QgsLinePatternFillSymbolLayer::estimateMaxBleed() const
{
  return 0;
}

void QgsLinePatternFillSymbolLayer::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mDistanceUnit = unit;
  mLineWidthUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsLinePatternFillSymbolLayer::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceUnit != unit || mLineWidthUnit != unit || mOffsetUnit != unit )
  {
    return QgsSymbolV2::Mixed;
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

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayer::create( const QgsStringMap& properties )
{
  QgsLinePatternFillSymbolLayer* patternLayer = new QgsLinePatternFillSymbolLayer();

  //default values
  double lineAngle = 45;
  double distance = 5;
  double lineWidth = 0.5;
  QColor color( Qt::black );
  double offset = 0.0;

  if ( properties.contains( "lineangle" ) )
  {
    //pre 2.5 projects used "lineangle"
    lineAngle = properties["lineangle"].toDouble();
  }
  else if ( properties.contains( "angle" ) )
  {
    lineAngle = properties["angle"].toDouble();
  }
  patternLayer->setLineAngle( lineAngle );

  if ( properties.contains( "distance" ) )
  {
    distance = properties["distance"].toDouble();
  }
  patternLayer->setDistance( distance );

  if ( properties.contains( "linewidth" ) )
  {
    //pre 2.5 projects used "linewidth"
    lineWidth = properties["linewidth"].toDouble();
  }
  else if ( properties.contains( "outline_width" ) )
  {
    lineWidth = properties["outline_width"].toDouble();
  }
  else if ( properties.contains( "line_width" ) )
  {
    lineWidth = properties["line_width"].toDouble();
  }
  patternLayer->setLineWidth( lineWidth );

  if ( properties.contains( "color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( properties["color"] );
  }
  else if ( properties.contains( "outline_color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] );
  }
  else if ( properties.contains( "line_color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( properties["line_color"] );
  }
  patternLayer->setColor( color );

  if ( properties.contains( "offset" ) )
  {
    offset = properties["offset"].toDouble();
  }
  patternLayer->setOffset( offset );


  if ( properties.contains( "distance_unit" ) )
  {
    patternLayer->setDistanceUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["distance_unit"] ) );
  }
  if ( properties.contains( "distance_map_unit_scale" ) )
  {
    patternLayer->setDistanceMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["distance_map_unit_scale"] ) );
  }
  if ( properties.contains( "line_width_unit" ) )
  {
    patternLayer->setLineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["line_width_unit"] ) );
  }
  else if ( properties.contains( "outline_width_unit" ) )
  {
    patternLayer->setLineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  if ( properties.contains( "line_width_map_unit_scale" ) )
  {
    patternLayer->setLineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["line_width_map_unit_scale"] ) );
  }
  if ( properties.contains( "offset_unit" ) )
  {
    patternLayer->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["offset_unit"] ) );
  }
  if ( properties.contains( "offset_map_unit_scale" ) )
  {
    patternLayer->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["offset_map_unit_scale"] ) );
  }
  if ( properties.contains( "outline_width_unit" ) )
  {
    patternLayer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  if ( properties.contains( "outline_width_map_unit_scale" ) )
  {
    patternLayer->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["outline_width_map_unit_scale"] ) );
  }


  //data defined properties
  if ( properties.contains( "lineangle_expression" ) )
  {
    patternLayer->setDataDefinedProperty( "lineangle", properties["lineangle_expression"] );
  }
  if ( properties.contains( "distance_expression" ) )
  {
    patternLayer->setDataDefinedProperty( "distance", properties["distance_expression"] );
  }
  if ( properties.contains( "linewidth_expression" ) )
  {
    patternLayer->setDataDefinedProperty( "linewidth", properties["linewidth_expression"] );
  }
  if ( properties.contains( "color_expression" ) )
  {
    patternLayer->setDataDefinedProperty( "color", properties["color_expression"] );
  }
  return patternLayer;
}

QString QgsLinePatternFillSymbolLayer::layerType() const
{
  return "LinePatternFill";
}

void QgsLinePatternFillSymbolLayer::applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double lineAngle, double distance,
    double lineWidth, const QColor& color )
{
  Q_UNUSED( lineWidth );
  Q_UNUSED( color );

  mBrush.setTextureImage( QImage() ); // set empty in case we have to return

  if ( !mFillLineSymbol )
  {
    return;
  }
  // We have to make a copy because marker intervals will have to be adjusted
  QgsLineSymbolV2* fillLineSymbol = dynamic_cast<QgsLineSymbolV2*>( mFillLineSymbol->clone() );
  if ( !fillLineSymbol )
  {
    return;
  }

  const QgsRenderContext& ctx = context.renderContext();
  //double outlinePixelWidth = lineWidth * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx,  mLineWidthUnit, mLineWidthMapUnitScale );
  double outputPixelDist = distance * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDistanceUnit, mDistanceMapUnitScale );
  double outputPixelOffset = mOffset * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx,  mOffsetUnit, mOffsetMapUnitScale );

  // To get all patterns into image, we have to consider symbols size (estimateMaxBleed()).
  // For marker lines we have to get markers interval.
  double outputPixelBleed = 0;
  double outputPixelInterval = 0; // maximum interval
  for ( int i = 0; i < fillLineSymbol->symbolLayerCount(); i++ )
  {
    QgsSymbolLayerV2 *layer = fillLineSymbol->symbolLayer( i );
    double layerBleed = layer->estimateMaxBleed();
    // TODO: to get real bleed we have to scale it using context and units,
    // unfortunately estimateMaxBleed() ignore units completely, e.g.
    // QgsMarkerLineSymbolLayerV2::estimateMaxBleed() is mixing marker size and
    // offset regardless units. This has to be fixed especially
    // in estimateMaxBleed(), context probably has to be used.
    // For now, we only support millimeters
    double outputPixelLayerBleed = layerBleed * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, QgsSymbolV2::MM );
    outputPixelBleed = qMax( outputPixelBleed, outputPixelLayerBleed );

    QgsMarkerLineSymbolLayerV2 *markerLineLayer = dynamic_cast<QgsMarkerLineSymbolLayerV2 *>( layer );
    if ( markerLineLayer )
    {
      double outputPixelLayerInterval = markerLineLayer->interval() * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, markerLineLayer->intervalUnit(), markerLineLayer->intervalMapUnitScale() );

      // There may be multiple marker lines with different intervals.
      // In theory we should find the least common multiple, but that could be too
      // big (multiplication of intervals in the worst case).
      // Because patterns without small common interval would look strange, we
      // believe that the longest interval should usually be sufficient.
      outputPixelInterval = qMax( outputPixelInterval, outputPixelLayerInterval );
    }
  }

  if ( outputPixelInterval > 0 )
  {
    // We have to adjust marker intervals to integer pixel size to get
    // repeatable pattern.
    double intervalScale = qRound( outputPixelInterval ) / outputPixelInterval;
    outputPixelInterval = qRound( outputPixelInterval );

    for ( int i = 0; i < fillLineSymbol->symbolLayerCount(); i++ )
    {
      QgsSymbolLayerV2 *layer = fillLineSymbol->symbolLayer( i );

      QgsMarkerLineSymbolLayerV2 *markerLineLayer = dynamic_cast<QgsMarkerLineSymbolLayerV2 *>( layer );
      if ( markerLineLayer )
      {
        markerLineLayer->setInterval( intervalScale * markerLineLayer->interval() );
      }
    }
  }

  //create image
  int height, width;
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
    height = outputPixelDist / cos( lineAngle * M_PI / 180 ); //keep perpendicular distance between lines constant
    width = outputPixelDist / sin( lineAngle * M_PI / 180 );

    // recalculate real angle and distance after rounding to pixels
    lineAngle = 180 * atan2(( double ) height, ( double ) width ) / M_PI;
    if ( lineAngle < 0 )
    {
      lineAngle += 360.;
    }

    height = qAbs( height );
    width = qAbs( width );

    outputPixelDist = height * cos( lineAngle * M_PI / 180 );

    // Round offset to correspond to one pixel height, otherwise lines may
    // be shifted on tile border if offset falls close to pixel center
    int offsetHeight = qRound( qAbs( outputPixelOffset / cos( lineAngle * M_PI / 180 ) ) );
    outputPixelOffset = offsetHeight * cos( lineAngle * M_PI / 180 );
  }

  //depending on the angle, we might need to render into a larger image and use a subset of it
  double dx = 0;
  double dy = 0;

  // Add buffer based on bleed but keep precisely the height/width ratio (angle)
  // thus we add integer multiplications of width and height covering the bleed
  int bufferMulti = qMax( qCeil( outputPixelBleed / width ), qCeil( outputPixelBleed / width ) );

  // Always buffer at least once so that center of line marker in upper right corner
  // does not fall outside due to representation error
  bufferMulti = qMax( bufferMulti, 1 );

  int xBuffer = width * bufferMulti;
  int yBuffer = height * bufferMulti;
  int innerWidth = width;
  int innerHeight = height;
  width += 2 * xBuffer;
  height += 2 * yBuffer;

  if ( width > 10000 || height > 10000 ) //protect symbol layer from eating too much memory
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
    dx = outputPixelDist * cos(( 90 - lineAngle ) * M_PI / 180.0 );
    dy = outputPixelDist * sin(( 90 - lineAngle ) * M_PI / 180.0 );
    p1 = QPointF( 0, height );
    p2 = QPointF( width, 0 );
    p3 = QPointF( -dx, height - dy );
    p4 = QPointF( width - dx, -dy );
    p5 = QPointF( dx, height + dy );
    p6 = QPointF( width + dx, dy );
  }
  else if ( lineAngle > 180 && lineAngle < 270 )
  {
    dx = outputPixelDist * cos(( 90 - lineAngle ) * M_PI / 180.0 );
    dy = outputPixelDist * sin(( 90 - lineAngle ) * M_PI / 180.0 );
    p1 = QPointF( width, 0 );
    p2 = QPointF( 0, height );
    p3 = QPointF( width - dx, -dy );
    p4 = QPointF( -dx, height - dy );
    p5 = QPointF( width + dx, dy );
    p6 = QPointF( dx, height + dy );
  }
  else if ( lineAngle > 90 && lineAngle < 180 )
  {
    dy = outputPixelDist * cos(( 180 - lineAngle ) * M_PI / 180 );
    dx = outputPixelDist * sin(( 180 - lineAngle ) * M_PI / 180 );
    p1 = QPointF( 0, 0 );
    p2 = QPointF( width, height );
    p5 = QPointF( dx, -dy );
    p6 = QPointF( width + dx, height - dy );
    p3 = QPointF( -dx, dy );
    p4 = QPointF( width - dx, height + dy );
  }
  else if ( lineAngle > 270 && lineAngle < 360 )
  {
    dy = outputPixelDist * cos(( 180 - lineAngle ) * M_PI / 180 );
    dx = outputPixelDist * sin(( 180 - lineAngle ) * M_PI / 180 );
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
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p1, p3, outputPixelDist + outputPixelOffset );
    p3 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p2, p4, outputPixelDist + outputPixelOffset );
    p4 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p1, p5, outputPixelDist - outputPixelOffset );
    p5 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p2, p6, outputPixelDist - outputPixelOffset );
    p6 = QPointF( tempPt.x(), tempPt.y() );

    //update p1, p2 last
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p1, p3, outputPixelOffset );
    p1 = QPointF( tempPt.x(), tempPt.y() );
    tempPt = QgsSymbolLayerV2Utils::pointOnLineWithDistance( p2, p4, outputPixelOffset );
    p2 = QPointF( tempPt.x(), tempPt.y() );;
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
  // right and below the mathematically defined points (not symetrical)
  // and such tiles become useless for are filling
  p.setRenderHint( QPainter::Antialiasing, true );

  // line rendering needs context for drawing on patternImage
  QgsRenderContext lineRenderContext;
  lineRenderContext.setPainter( &p );
  lineRenderContext.setRasterScaleFactor( 1.0 );
  lineRenderContext.setScaleFactor( context.renderContext().scaleFactor() * context.renderContext().rasterScaleFactor() );
  QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() / context.renderContext().rasterScaleFactor() );
  lineRenderContext.setMapToPixel( mtp );
  lineRenderContext.setForceVectorOutput( false );

  fillLineSymbol->startRender( lineRenderContext, context.fields() );

  QVector<QPolygonF> polygons;
  polygons.append( QPolygonF() << p1 << p2 );
  polygons.append( QPolygonF() << p3 << p4 );
  if ( !qgsDoubleNear( lineAngle, 0 ) && !qgsDoubleNear( lineAngle, 360 ) && !qgsDoubleNear( lineAngle, 90 ) && !qgsDoubleNear( lineAngle, 180 ) && !qgsDoubleNear( lineAngle, 270 ) )
  {
    polygons.append( QPolygonF() << p5 << p6 );
  }

  foreach ( QPolygonF polygon, polygons )
  {
    fillLineSymbol->renderPolyline( polygon, context.feature(), lineRenderContext, -1, context.selected() );
  }

  fillLineSymbol->stopRender( lineRenderContext );
  p.end();

  // Cut off the buffer
  patternImage = patternImage.copy( xBuffer, yBuffer, patternImage.width() - 2 * xBuffer, patternImage.height() - 2 * yBuffer );

  //set image to mBrush
  if ( !qgsDoubleNear( context.alpha(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
    brush.setTextureImage( transparentImage );
  }
  else
  {
    brush.setTextureImage( patternImage );
  }

  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  brush.setTransform( brushTransform );

  delete fillLineSymbol;
}

void QgsLinePatternFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  applyPattern( context, mBrush, mLineAngle, mDistance, mLineWidth, mColor );

  if ( mFillLineSymbol )
  {
    mFillLineSymbol->startRender( context.renderContext(), context.fields() );
  }

  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsLinePatternFillSymbolLayer::stopRender( QgsSymbolV2RenderContext & )
{
}

QgsStringMap QgsLinePatternFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map.insert( "angle", QString::number( mLineAngle ) );
  map.insert( "distance", QString::number( mDistance ) );
  map.insert( "line_width", QString::number( mLineWidth ) );
  map.insert( "color", QgsSymbolLayerV2Utils::encodeColor( mColor ) );
  map.insert( "offset", QString::number( mOffset ) );
  map.insert( "distance_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mDistanceUnit ) );
  map.insert( "line_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mLineWidthUnit ) );
  map.insert( "offset_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit ) );
  map.insert( "distance_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mDistanceMapUnitScale ) );
  map.insert( "line_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mLineWidthMapUnitScale ) );
  map.insert( "offset_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale ) );
  map.insert( "outline_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit ) );
  map.insert( "outline_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale ) );
  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayer::clone() const
{
  QgsLinePatternFillSymbolLayer* clonedLayer = static_cast<QgsLinePatternFillSymbolLayer*>( QgsLinePatternFillSymbolLayer::create( properties() ) );
  if ( mFillLineSymbol )
  {
    clonedLayer->setSubSymbol( mFillLineSymbol->clone() );
  }
  return clonedLayer;
}

void QgsLinePatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PolygonSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  QDomElement fillElem = doc.createElement( "se:Fill" );
  symbolizerElem.appendChild( fillElem );

  QDomElement graphicFillElem = doc.createElement( "se:GraphicFill" );
  fillElem.appendChild( graphicFillElem );

  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  graphicFillElem.appendChild( graphicElem );

  //line properties must be inside the graphic definition
  QColor lineColor = mFillLineSymbol ? mFillLineSymbol->color() : QColor();
  double lineWidth = mFillLineSymbol ? mFillLineSymbol->width() : 0.0;
  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, "horline", QColor(), lineColor, Qt::SolidLine, lineWidth, mDistance );

  // <Rotation>
  QString angleFunc;
  bool ok;
  double angle = props.value( "angle", "0" ).toDouble( &ok );
  if ( !ok )
  {
    angleFunc = QString( "%1 + %2" ).arg( props.value( "angle", "0" ) ).arg( mLineAngle );
  }
  else if ( angle + mLineAngle != 0 )
  {
    angleFunc = QString::number( angle + mLineAngle );
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );

  // <se:Displacement>
  QPointF lineOffset( sin( mLineAngle ) * mOffset, cos( mLineAngle ) * mOffset );
  QgsSymbolLayerV2Utils::createDisplacementElement( doc, graphicElem, lineOffset );
}

QString QgsLinePatternFillSymbolLayer::ogrFeatureStyleWidth( double widthScaleFactor ) const
{
  QString featureStyle;
  featureStyle.append( "Brush(" );
  featureStyle.append( QString( "fc:%1" ).arg( mColor.name() ) );
  featureStyle.append( QString( ",bc:%1" ).arg( "#00000000" ) ); //transparent background
  featureStyle.append( ",id:\"ogr-brush-2\"" );
  featureStyle.append( QString( ",a:%1" ).arg( mLineAngle ) );
  featureStyle.append( QString( ",s:%1" ).arg( mLineWidth * widthScaleFactor ) );
  featureStyle.append( ",dx:0mm" );
  featureStyle.append( QString( ",dy:%1mm" ).arg( mDistance * widthScaleFactor ) );
  featureStyle.append( ")" );
  return featureStyle;
}

void QgsLinePatternFillSymbolLayer::applyDataDefinedSettings( const QgsSymbolV2RenderContext& context )
{
  QgsExpression* lineAngleExpression = expression( "lineangle" );
  QgsExpression* distanceExpression = expression( "distance" );
  QgsExpression* lineWidthExpression = expression( "linewidth" );
  QgsExpression* colorExpression = expression( "color" );
  if ( !lineAngleExpression && !distanceExpression && !lineWidthExpression && !colorExpression )
  {
    return; //no data defined settings
  }

  double lineAngle = mLineAngle;
  if ( lineAngleExpression )
  {
    lineAngle = lineAngleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double distance = mDistance;
  if ( distanceExpression )
  {
    distance = distanceExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double lineWidth = mLineWidth;
  if ( lineWidthExpression )
  {
    lineWidth = lineWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  QColor color = mColor;
  if ( colorExpression )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( colorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() );
  }
  applyPattern( context, mBrush, lineAngle, distance, lineWidth, color );
}

QgsSymbolLayerV2* QgsLinePatternFillSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QString name;
  QColor fillColor, lineColor;
  double size, lineWidth;
  Qt::PenStyle lineStyle;

  QDomElement fillElem = element.firstChildElement( "Fill" );
  if ( fillElem.isNull() )
    return NULL;

  QDomElement graphicFillElem = fillElem.firstChildElement( "GraphicFill" );
  if ( graphicFillElem.isNull() )
    return NULL;

  QDomElement graphicElem = graphicFillElem.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  if ( !QgsSymbolLayerV2Utils::wellKnownMarkerFromSld( graphicElem, name, fillColor, lineColor, lineStyle, lineWidth, size ) )
    return NULL;

  if ( name != "horline" )
    return NULL;

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerV2Utils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  double offset = 0.0;
  QPointF vectOffset;
  if ( QgsSymbolLayerV2Utils::displacementFromSldElement( graphicElem, vectOffset ) )
  {
    offset = sqrt( pow( vectOffset.x(), 2 ) + pow( vectOffset.y(), 2 ) );
  }

  QgsLinePatternFillSymbolLayer* sl = new QgsLinePatternFillSymbolLayer();
  sl->setColor( lineColor );
  sl->setLineWidth( lineWidth );
  sl->setLineAngle( angle );
  sl->setOffset( offset );
  sl->setDistance( size );

  // try to get the outline
  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( !strokeElem.isNull() )
  {
    QgsSymbolLayerV2 *l = QgsSymbolLayerV2Utils::createLineLayerFromSld( strokeElem );
    if ( l )
    {
      QgsSymbolLayerV2List layers;
      layers.append( l );
      sl->setSubSymbol( new QgsLineSymbolV2( layers ) );
    }
  }

  return sl;
}


////////////////////////

QgsPointPatternFillSymbolLayer::QgsPointPatternFillSymbolLayer(): QgsImageFillSymbolLayer(), mMarkerSymbol( 0 ), mDistanceX( 15 ),
    mDistanceXUnit( QgsSymbolV2::MM ), mDistanceY( 15 ), mDistanceYUnit( QgsSymbolV2::MM ), mDisplacementX( 0 ), mDisplacementXUnit( QgsSymbolV2::MM ),
    mDisplacementY( 0 ), mDisplacementYUnit( QgsSymbolV2::MM )
{
  mDistanceX = 15;
  mDistanceY = 15;
  mDisplacementX = 0;
  mDisplacementY = 0;
  setSubSymbol( new QgsMarkerSymbolV2() );
  QgsImageFillSymbolLayer::setSubSymbol( 0 ); //no outline
}

QgsPointPatternFillSymbolLayer::~QgsPointPatternFillSymbolLayer()
{
}

void QgsPointPatternFillSymbolLayer::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsImageFillSymbolLayer::setOutputUnit( unit );
  mDistanceXUnit = unit;
  mDistanceYUnit = unit;
  mDisplacementXUnit = unit;
  mDisplacementYUnit = unit;
}

QgsSymbolV2::OutputUnit QgsPointPatternFillSymbolLayer::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsImageFillSymbolLayer::outputUnit();
  if ( mDistanceXUnit != unit || mDistanceYUnit != unit || mDisplacementXUnit != unit || mDisplacementYUnit != unit )
  {
    return QgsSymbolV2::Mixed;
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

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayer::create( const QgsStringMap& properties )
{
  QgsPointPatternFillSymbolLayer* layer = new QgsPointPatternFillSymbolLayer();
  if ( properties.contains( "distance_x" ) )
  {
    layer->setDistanceX( properties["distance_x"].toDouble() );
  }
  if ( properties.contains( "distance_y" ) )
  {
    layer->setDistanceY( properties["distance_y"].toDouble() );
  }
  if ( properties.contains( "displacement_x" ) )
  {
    layer->setDisplacementX( properties["displacement_x"].toDouble() );
  }
  if ( properties.contains( "displacement_y" ) )
  {
    layer->setDisplacementY( properties["displacement_y"].toDouble() );
  }

  if ( properties.contains( "distance_x_unit" ) )
  {
    layer->setDistanceXUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["distance_x_unit"] ) );
  }
  if ( properties.contains( "distance_x_map_unit_scale" ) )
  {
    layer->setDistanceXMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["distance_x_map_unit_scale"] ) );
  }
  if ( properties.contains( "distance_y_unit" ) )
  {
    layer->setDistanceYUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["distance_y_unit"] ) );
  }
  if ( properties.contains( "distance_y_map_unit_scale" ) )
  {
    layer->setDistanceYMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["distance_y_map_unit_scale"] ) );
  }
  if ( properties.contains( "displacement_x_unit" ) )
  {
    layer->setDisplacementXUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["displacement_x_unit"] ) );
  }
  if ( properties.contains( "displacement_x_map_unit_scale" ) )
  {
    layer->setDisplacementXMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["displacement_x_map_unit_scale"] ) );
  }
  if ( properties.contains( "displacement_y_unit" ) )
  {
    layer->setDisplacementYUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["displacement_y_unit"] ) );
  }
  if ( properties.contains( "displacement_y_map_unit_scale" ) )
  {
    layer->setDisplacementYMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["displacement_y_map_unit_scale"] ) );
  }
  if ( properties.contains( "outline_width_unit" ) )
  {
    layer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  if ( properties.contains( "outline_width_map_unit_scale" ) )
  {
    layer->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["outline_width_map_unit_scale"] ) );
  }

  //data defined properties
  if ( properties.contains( "distance_x_expression" ) )
  {
    layer->setDataDefinedProperty( "distance_x", properties["distance_x_expression"] );
  }
  if ( properties.contains( "distance_y_expression" ) )
  {
    layer->setDataDefinedProperty( "distance_y", properties["distance_y_expression"] );
  }
  if ( properties.contains( "displacement_x_expression" ) )
  {
    layer->setDataDefinedProperty( "displacement_x", properties["displacement_x_expression"] );
  }
  if ( properties.contains( "displacement_y_expression" ) )
  {
    layer->setDataDefinedProperty( "displacement_y", properties["displacement_y_expression"] );
  }
  return layer;
}

QString QgsPointPatternFillSymbolLayer::layerType() const
{
  return "PointPatternFill";
}

void QgsPointPatternFillSymbolLayer::applyPattern( const QgsSymbolV2RenderContext& context, QBrush& brush, double distanceX, double distanceY,
    double displacementX, double displacementY )
{
  //render 3 rows and columns in one go to easily incorporate displacement
  const QgsRenderContext& ctx = context.renderContext();
  double width = distanceX * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDistanceXUnit, mDistanceXMapUnitScale ) * 2.0;
  double height = distanceY * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDistanceYUnit, mDisplacementYMapUnitScale ) * 2.0;

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
    pointRenderContext.setRasterScaleFactor( 1.0 );
    pointRenderContext.setScaleFactor( context.renderContext().scaleFactor() * context.renderContext().rasterScaleFactor() );
    QgsMapToPixel mtp( context.renderContext().mapToPixel().mapUnitsPerPixel() / context.renderContext().rasterScaleFactor() );
    pointRenderContext.setMapToPixel( mtp );
    pointRenderContext.setForceVectorOutput( false );

    mMarkerSymbol->startRender( pointRenderContext, context.fields() );

    //render corner points
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, 0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( 0, height ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width, height ), context.feature(), pointRenderContext );

    //render displaced points
    double displacementPixelX = displacementX * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDisplacementXUnit, mDisplacementXMapUnitScale );
    double displacementPixelY = displacementY * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( ctx, mDisplacementYUnit, mDisplacementYMapUnitScale );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, -displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0 + displacementPixelX, height / 2.0 - displacementPixelY ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width + displacementPixelX, height / 2.0 ), context.feature(), pointRenderContext );
    mMarkerSymbol->renderPoint( QPointF( width / 2.0, height - displacementPixelY ), context.feature(), pointRenderContext );

    mMarkerSymbol->stopRender( pointRenderContext );
  }

  if ( !qgsDoubleNear( context.alpha(), 1.0 ) )
  {
    QImage transparentImage = patternImage.copy();
    QgsSymbolLayerV2Utils::multiplyImageOpacity( &transparentImage, context.alpha() );
    brush.setTextureImage( transparentImage );
  }
  else
  {
    brush.setTextureImage( patternImage );
  }
  QTransform brushTransform;
  brushTransform.scale( 1.0 / context.renderContext().rasterScaleFactor(), 1.0 / context.renderContext().rasterScaleFactor() );
  brush.setTransform( brushTransform );
}

void QgsPointPatternFillSymbolLayer::startRender( QgsSymbolV2RenderContext& context )
{
  applyPattern( context, mBrush, mDistanceX, mDistanceY, mDisplacementX, mDisplacementY );

  if ( mOutline )
  {
    mOutline->startRender( context.renderContext(), context.fields() );
  }
  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
}

void QgsPointPatternFillSymbolLayer::stopRender( QgsSymbolV2RenderContext& context )
{
  if ( mOutline )
  {
    mOutline->stopRender( context.renderContext() );
  }
}

QgsStringMap QgsPointPatternFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map.insert( "distance_x", QString::number( mDistanceX ) );
  map.insert( "distance_y", QString::number( mDistanceY ) );
  map.insert( "displacement_x", QString::number( mDisplacementX ) );
  map.insert( "displacement_y", QString::number( mDisplacementY ) );
  map.insert( "distance_x_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mDistanceXUnit ) );
  map.insert( "distance_y_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mDistanceYUnit ) );
  map.insert( "displacement_x_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mDisplacementXUnit ) );
  map.insert( "displacement_y_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mDisplacementYUnit ) );
  map.insert( "distance_x_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mDistanceXMapUnitScale ) );
  map.insert( "distance_y_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mDistanceYMapUnitScale ) );
  map.insert( "displacement_x_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mDisplacementXMapUnitScale ) );
  map.insert( "displacement_y_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mDisplacementYMapUnitScale ) );
  map.insert( "outline_width_unit", QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit ) );
  map.insert( "outline_width_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale ) );
  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayer::clone() const
{
  QgsPointPatternFillSymbolLayer* clonedLayer = static_cast<QgsPointPatternFillSymbolLayer*>( QgsPointPatternFillSymbolLayer::create( properties() ) );
  if ( mMarkerSymbol )
  {
    clonedLayer->setSubSymbol( mMarkerSymbol->clone() );
  }
  return clonedLayer;
}

void QgsPointPatternFillSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  for ( int i = 0; i < mMarkerSymbol->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( "se:PolygonSymbolizer" );
    if ( !props.value( "uom", "" ).isEmpty() )
      symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

    QDomElement fillElem = doc.createElement( "se:Fill" );
    symbolizerElem.appendChild( fillElem );

    QDomElement graphicFillElem = doc.createElement( "se:GraphicFill" );
    fillElem.appendChild( graphicFillElem );

    // store distanceX, distanceY, displacementX, displacementY in a <VendorOption>
    QString dist =  QgsSymbolLayerV2Utils::encodePoint( QPointF( mDistanceX, mDistanceY ) );
    QDomElement distanceElem = QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "distance", dist );
    symbolizerElem.appendChild( distanceElem );

    QgsSymbolLayerV2 *layer = mMarkerSymbol->symbolLayer( i );
    QgsMarkerSymbolLayerV2 *markerLayer = static_cast<QgsMarkerSymbolLayerV2 *>( layer );
    if ( !markerLayer )
    {
      QString errorMsg = QString( "MarkerSymbolLayerV2 expected, %1 found. Skip it." ).arg( layer->layerType() );
      graphicFillElem.appendChild( doc.createComment( errorMsg ) );
    }
    else
    {
      markerLayer->writeSldMarker( doc, graphicFillElem, props );
    }
  }
}

QgsSymbolLayerV2* QgsPointPatternFillSymbolLayer::createFromSld( QDomElement &element )
{
  Q_UNUSED( element );
  return NULL;
}

bool QgsPointPatternFillSymbolLayer::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( !symbol )
  {
    return false;
  }

  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2* markerSymbol = static_cast<QgsMarkerSymbolV2*>( symbol );
    delete mMarkerSymbol;
    mMarkerSymbol = markerSymbol;
  }
  return true;
}

void QgsPointPatternFillSymbolLayer::applyDataDefinedSettings( const QgsSymbolV2RenderContext& context )
{
  QgsExpression* distanceXExpression = expression( "distance_x" );
  QgsExpression* distanceYExpression = expression( "distance_y" );
  QgsExpression* displacementXExpression = expression( "displacement_x" );
  QgsExpression* displacementYExpression = expression( "displacement_y" );

#if 0
  // TODO: enable but check also if mMarkerSymbol has data defined properties
  if ( !distanceXExpression && !distanceYExpression && !displacementXExpression && !displacementYExpression )
  {
    return;
  }
#endif

  double distanceX = mDistanceX;
  if ( distanceXExpression )
  {
    distanceX = distanceXExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double distanceY = mDistanceY;
  if ( distanceYExpression )
  {
    distanceY = distanceYExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double displacementX = mDisplacementX;
  if ( displacementXExpression )
  {
    displacementX = displacementXExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double displacementY = mDisplacementY;
  if ( displacementYExpression )
  {
    displacementY = displacementYExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  applyPattern( context, mBrush, distanceX, distanceY, displacementX, displacementY );
}

double QgsPointPatternFillSymbolLayer::estimateMaxBleed() const
{
  return 0;
}

QSet<QString> QgsPointPatternFillSymbolLayer::usedAttributes() const
{
  QSet<QString> attributes = QgsSymbolLayerV2::usedAttributes();

  if ( mMarkerSymbol )
    attributes.unite( mMarkerSymbol->usedAttributes() );

  return attributes;
}

//////////////


QgsCentroidFillSymbolLayerV2::QgsCentroidFillSymbolLayerV2(): mMarker( NULL ), mPointOnSurface( false )
{
  setSubSymbol( new QgsMarkerSymbolV2() );
}

QgsCentroidFillSymbolLayerV2::~QgsCentroidFillSymbolLayerV2()
{
  delete mMarker;
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2::create( const QgsStringMap& properties )
{
  QgsCentroidFillSymbolLayerV2* sl = new QgsCentroidFillSymbolLayerV2();

  if ( properties.contains( "point_on_surface" ) )
    sl->setPointOnSurface( properties["point_on_surface"].toInt() != 0 );

  return sl;
}

QString QgsCentroidFillSymbolLayerV2::layerType() const
{
  return "CentroidFill";
}

void QgsCentroidFillSymbolLayerV2::setColor( const QColor& color )
{
  mMarker->setColor( color );
  mColor = color;
}

void QgsCentroidFillSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mMarker->setAlpha( context.alpha() );
  mMarker->startRender( context.renderContext(), context.fields() );
}

void QgsCentroidFillSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsCentroidFillSymbolLayerV2::renderPolygon( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( rings );

  QPointF centroid = mPointOnSurface ? QgsSymbolLayerV2Utils::polygonPointOnSurface( points ) : QgsSymbolLayerV2Utils::polygonCentroid( points );
  mMarker->renderPoint( centroid, context.feature(), context.renderContext(), -1, context.selected() );
}

QgsStringMap QgsCentroidFillSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["point_on_surface"] = QString::number( mPointOnSurface );
  return map;
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2::clone() const
{
  QgsCentroidFillSymbolLayerV2* x = new QgsCentroidFillSymbolLayerV2();
  x->mAngle = mAngle;
  x->mColor = mColor;
  x->setSubSymbol( mMarker->clone() );
  x->setPointOnSurface( mPointOnSurface );
  return x;
}

void QgsCentroidFillSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  // SLD 1.0 specs says: "if a line, polygon, or raster geometry is
  // used with PointSymbolizer, then the semantic is to use the centroid
  // of the geometry, or any similar representative point.
  mMarker->toSld( doc, element, props );
}

QgsSymbolLayerV2* QgsCentroidFillSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QgsSymbolLayerV2 *l = QgsSymbolLayerV2Utils::createMarkerLayerFromSld( element );
  if ( !l )
    return NULL;

  QgsSymbolLayerV2List layers;
  layers.append( l );
  QgsMarkerSymbolV2 *marker = new QgsMarkerSymbolV2( layers );

  QgsCentroidFillSymbolLayerV2* sl = new QgsCentroidFillSymbolLayerV2();
  sl->setSubSymbol( marker );
  return sl;
}


QgsSymbolV2* QgsCentroidFillSymbolLayerV2::subSymbol()
{
  return mMarker;
}

bool QgsCentroidFillSymbolLayerV2::setSubSymbol( QgsSymbolV2* symbol )
{
  if ( symbol == NULL || symbol->type() != QgsSymbolV2::Marker )
  {
    delete symbol;
    return false;
  }

  delete mMarker;
  mMarker = static_cast<QgsMarkerSymbolV2*>( symbol );
  mColor = mMarker->color();
  return true;
}

QSet<QString> QgsCentroidFillSymbolLayerV2::usedAttributes() const
{
  QSet<QString> attributes;

  attributes.unite( QgsSymbolLayerV2::usedAttributes() );

  if ( mMarker )
    attributes.unite( mMarker->usedAttributes() );

  return attributes;
}

void QgsCentroidFillSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  if ( mMarker )
  {
    mMarker->setOutputUnit( unit );
  }
}

QgsSymbolV2::OutputUnit QgsCentroidFillSymbolLayerV2::outputUnit() const
{
  if ( mMarker )
  {
    return mMarker->outputUnit();
  }
  return QgsSymbolV2::Mixed; //mOutputUnit;
}

void QgsCentroidFillSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  if ( mMarker )
  {
    mMarker->setMapUnitScale( scale );
  }
}

QgsMapUnitScale QgsCentroidFillSymbolLayerV2::mapUnitScale() const
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
    , mCoordinateMode( QgsRasterFillSymbolLayer::Feature )
    , mAlpha( 1.0 )
    , mOffsetUnit( QgsSymbolV2::MM )
    , mWidth( 0.0 )
    , mWidthUnit( QgsSymbolV2::Pixel )
{
  QgsImageFillSymbolLayer::setSubSymbol( 0 ); //disable sub symbol
}

QgsRasterFillSymbolLayer::~QgsRasterFillSymbolLayer()
{

}

QgsSymbolLayerV2 *QgsRasterFillSymbolLayer::create( const QgsStringMap &properties )
{
  FillCoordinateMode mode = QgsRasterFillSymbolLayer::Feature;
  double alpha = 1.0;
  QPointF offset;
  double angle = 0.0;
  double width = 0.0;

  QString imagePath;
  if ( properties.contains( "imageFile" ) )
  {
    imagePath = properties["imageFile"];
  }
  if ( properties.contains( "coordinate_mode" ) )
  {
    mode = ( FillCoordinateMode )properties["coordinate_mode"].toInt();
  }
  if ( properties.contains( "alpha" ) )
  {
    alpha = properties["alpha"].toDouble();
  }
  if ( properties.contains( "offset" ) )
  {
    offset = QgsSymbolLayerV2Utils::decodePoint( properties["offset"] );
  }
  if ( properties.contains( "angle" ) )
  {
    angle = properties["angle"].toDouble();
  }
  if ( properties.contains( "width" ) )
  {
    width = properties["width"].toDouble();
  }
  QgsRasterFillSymbolLayer* symbolLayer = new QgsRasterFillSymbolLayer( imagePath );
  symbolLayer->setCoordinateMode( mode );
  symbolLayer->setAlpha( alpha );
  symbolLayer->setOffset( offset );
  symbolLayer->setAngle( angle );
  symbolLayer->setWidth( width );
  if ( properties.contains( "offset_unit" ) )
  {
    symbolLayer->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["offset_unit"] ) );
  }
  if ( properties.contains( "offset_map_unit_scale" ) )
  {
    symbolLayer->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["offset_map_unit_scale"] ) );
  }
  if ( properties.contains( "width_unit" ) )
  {
    symbolLayer->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["width_unit"] ) );
  }
  if ( properties.contains( "width_map_unit_scale" ) )
  {
    symbolLayer->setWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["width_map_unit_scale"] ) );
  }

  //data defined
  if ( properties.contains( "file_expression" ) )
  {
    symbolLayer->setDataDefinedProperty( "file", properties["file_expression"] );
  }
  if ( properties.contains( "alpha_expression" ) )
  {
    symbolLayer->setDataDefinedProperty( "alpha", properties["alpha_expression"] );
  }
  if ( properties.contains( "angle_expression" ) )
  {
    symbolLayer->setDataDefinedProperty( "angle", properties["angle_expression"] );
  }
  if ( properties.contains( "width_expression" ) )
  {
    symbolLayer->setDataDefinedProperty( "width", properties["width_expression"] );
  }
  return symbolLayer;
}

bool QgsRasterFillSymbolLayer::setSubSymbol( QgsSymbolV2 *symbol )
{
  Q_UNUSED( symbol );
  return true;
}

QString QgsRasterFillSymbolLayer::layerType() const
{
  return "RasterFill";
}

void QgsRasterFillSymbolLayer::renderPolygon( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolV2RenderContext &context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QPointF offset;
  if ( !mOffset.isNull() )
  {
    offset.setX( mOffset.x() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
    offset.setY( mOffset.y() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ) );
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

void QgsRasterFillSymbolLayer::startRender( QgsSymbolV2RenderContext &context )
{
  prepareExpressions( context.fields(), context.renderContext().rendererScale() );
  applyPattern( mBrush, mImageFilePath, mWidth, mAlpha, context );
}

void QgsRasterFillSymbolLayer::stopRender( QgsSymbolV2RenderContext &context )
{
  Q_UNUSED( context );
}

QgsStringMap QgsRasterFillSymbolLayer::properties() const
{
  QgsStringMap map;
  map["imageFile"] = mImageFilePath;
  map["coordinate_mode"] = QString::number( mCoordinateMode );
  map["alpha"] = QString::number( mAlpha );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["angle"] = QString::number( mAngle );
  map["width"] = QString::number( mWidth );
  map["width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mWidthUnit );
  map["width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mWidthMapUnitScale );

  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2 *QgsRasterFillSymbolLayer::clone() const
{
  QgsRasterFillSymbolLayer* sl = new QgsRasterFillSymbolLayer( mImageFilePath );
  sl->setCoordinateMode( mCoordinateMode );
  sl->setAlpha( mAlpha );
  sl->setOffset( mOffset );
  sl->setOffsetUnit( mOffsetUnit );
  sl->setOffsetMapUnitScale( mOffsetMapUnitScale );
  sl->setAngle( mAngle );
  sl->setWidth( mWidth );
  sl->setWidthUnit( mWidthUnit );
  sl->setWidthMapUnitScale( mWidthMapUnitScale );
  copyDataDefinedProperties( sl );
  return sl;
}

double QgsRasterFillSymbolLayer::estimateMaxBleed() const
{
  return mOffset.x() > mOffset.y() ? mOffset.x() : mOffset.y();
}

void QgsRasterFillSymbolLayer::setImageFilePath( const QString &imagePath )
{
  mImageFilePath = imagePath;
}

void QgsRasterFillSymbolLayer::setCoordinateMode( const QgsRasterFillSymbolLayer::FillCoordinateMode mode )
{
  mCoordinateMode = mode;
}

void QgsRasterFillSymbolLayer::setAlpha( const double alpha )
{
  mAlpha = alpha;
}

void QgsRasterFillSymbolLayer::applyDataDefinedSettings( const QgsSymbolV2RenderContext &context )
{
  if ( mDataDefinedProperties.isEmpty() )
    return; // shortcut

  QgsExpression* widthExpression = expression( "width" );
  QgsExpression* fileExpression = expression( "file" );
  QgsExpression* alphaExpression = expression( "alpha" );
  QgsExpression* angleExpression = expression( "angle" );

  if ( !widthExpression && !angleExpression && !alphaExpression && !fileExpression )
  {
    return; //no data defined settings
  }

  if ( angleExpression )
  {
    mNextAngle = angleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }

  if ( !widthExpression && !alphaExpression && !fileExpression )
  {
    return; //nothing further to do
  }

  double width = mWidth;
  if ( widthExpression )
  {
    width = widthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  double alpha = mAlpha;
  if ( alphaExpression )
  {
    alpha = alphaExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  QString file = mImageFilePath;
  if ( fileExpression )
  {
    file = fileExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
  }
  applyPattern( mBrush, file, width, alpha, context );
}

void QgsRasterFillSymbolLayer::applyPattern( QBrush &brush, const QString &imageFilePath, const double width, const double alpha, const QgsSymbolV2RenderContext &context )
{
  QImage image( imageFilePath );
  if ( image.isNull() )
  {
    return;
  }
  if ( !image.hasAlphaChannel() )
  {
    image = image.convertToFormat( QImage::Format_ARGB32 );
  }

  double pixelWidth;
  if ( width > 0 )
  {
    pixelWidth = width * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
  }
  else
  {
    pixelWidth = image.width();
  }

  //reduce alpha of image
  if ( alpha < 1.0 )
  {
    QPainter p;
    p.begin( &image );
    p.setCompositionMode( QPainter::CompositionMode_DestinationIn );
    QColor alphaColor( 0, 0, 0 );
    alphaColor.setAlphaF( alpha );
    p.fillRect( image.rect(), alphaColor );
    p.end();
  }

  //resize image if required
  if ( !qgsDoubleNear( pixelWidth, image.width() ) )
  {
    image = image.scaledToWidth( pixelWidth, Qt::SmoothTransformation );
  }

  brush.setTextureImage( image );
}
