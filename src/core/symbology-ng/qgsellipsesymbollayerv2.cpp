/***************************************************************************
    qgsellipsesymbollayerv2.cpp
    ---------------------
    begin                : June 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsellipsesymbollayerv2.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QPainter>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

QgsEllipseSymbolLayerV2::QgsEllipseSymbolLayerV2(): mSymbolName( "circle" ), mSymbolWidth( 4 ), mSymbolWidthUnit( QgsSymbolV2::MM ), mSymbolHeight( 3 ),
    mSymbolHeightUnit( QgsSymbolV2::MM ), mFillColor( Qt::white ), mOutlineColor( Qt::black ), mOutlineWidth( 0 ), mOutlineWidthUnit( QgsSymbolV2::MM )
{
  mPen.setColor( mOutlineColor );
  mPen.setWidth( 1.0 );
  mPen.setJoinStyle( Qt::MiterJoin );
  mBrush.setColor( mFillColor );
  mBrush.setStyle( Qt::SolidPattern );
  mOffset = QPointF( 0, 0 );

  mAngle = 0;
}

QgsEllipseSymbolLayerV2::~QgsEllipseSymbolLayerV2()
{
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::create( const QgsStringMap& properties )
{
  QgsEllipseSymbolLayerV2* layer = new QgsEllipseSymbolLayerV2();
  if ( properties.contains( "symbol_name" ) )
  {
    layer->setSymbolName( properties[ "symbol_name" ] );
  }
  if ( properties.contains( "symbol_width" ) )
  {
    layer->setSymbolWidth( properties["symbol_width"].toDouble() );
  }
  if ( properties.contains( "symbol_width_unit" ) )
  {
    layer->setSymbolWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["symbol_width_unit"] ) );
  }
  if ( properties.contains( "symbol_height" ) )
  {
    layer->setSymbolHeight( properties["symbol_height"].toDouble() );
  }
  if ( properties.contains( "symbol_height_unit" ) )
  {
    layer->setSymbolHeightUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["symbol_height_unit"] ) );
  }
  if ( properties.contains( "angle" ) )
  {
    layer->setAngle( properties["angle"].toDouble() );
  }
  if ( properties.contains( "outline_width" ) )
  {
    layer->setOutlineWidth( properties["outline_width"].toDouble() );
  }
  if ( properties.contains( "outline_width_unit" ) )
  {
    layer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  if ( properties.contains( "fill_color" ) )
  {
    layer->setFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["fill_color"] ) );
  }
  if ( properties.contains( "outline_color" ) )
  {
    layer->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] ) );
  }
  if ( properties.contains( "offset" ) )
  {
    layer->setOffset( QgsSymbolLayerV2Utils::decodePoint( properties["offset"] ) );
  }
  if ( properties.contains( "offset_unit" ) )
  {
    layer->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["offset_unit"] ) );
  }

  //data defined properties
  if ( properties.contains( "width_expression" ) )
  {
    layer->setDataDefinedProperty( "width", properties["width_expression"] );
  }
  if ( properties.contains( "height_expression" ) )
  {
    layer->setDataDefinedProperty( "height", properties["height_expression"] );
  }
  if ( properties.contains( "rotation_expression" ) )
  {
    layer->setDataDefinedProperty( "rotation", properties["rotation_expression"] );
  }
  if ( properties.contains( "outline_width_expression" ) )
  {
    layer->setDataDefinedProperty( "outline_width", properties[ "outline_width_expression" ] );
  }
  if ( properties.contains( "fill_color_expression" ) )
  {
    layer->setDataDefinedProperty( "fill_color", properties["fill_color_expression"] );
  }
  if ( properties.contains( "outline_color_expression" ) )
  {
    layer->setDataDefinedProperty( "outline_color", properties["outline_color_expression"] );
  }
  if ( properties.contains( "symbol_name_expression" ) )
  {
    layer->setDataDefinedProperty( "symbol_name", properties["symbol_name_expression"] );
  }
  if ( properties.contains( "offset_expression" ) )
  {
    layer->setDataDefinedProperty( "offset", properties["offset_expression"] );
  }

  //compatibility with old project file format
  if ( !properties["width_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "width", properties["width_field"] );
  }
  if ( !properties["height_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "height", properties["height_field"] );
  }
  if ( !properties["rotation_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "rotation", properties["rotation_field"] );
  }
  if ( !properties["outline_width_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "outline_width", properties[ "outline_width_field" ] );
  }
  if ( !properties["fill_color_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "fill_color", properties["fill_color_field"] );
  }
  if ( !properties["outline_color_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "outline_color", properties["outline_color_field"] );
  }
  if ( !properties["symbol_name_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( "symbol_name", properties["symbol_name_field"] );
  }

  return layer;
}

void QgsEllipseSymbolLayerV2::renderPoint( const QPointF& point, QgsSymbolV2RenderContext& context )
{
  QgsExpression* outlineWidthExpression = expression( "outline_width" );
  QgsExpression* fillColorExpression = expression( "fill_color" );
  QgsExpression* outlineColorExpression = expression( "outline_color" );
  QgsExpression* widthExpression = expression( "width" );
  QgsExpression* heightExpression = expression( "height" );
  QgsExpression* symbolNameExpression = expression( "symbol_name" );
  QgsExpression* rotationExpression = expression( "rotation" );

  if ( outlineWidthExpression )
  {
    double width = outlineWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
    width *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit );
    mPen.setWidthF( width );
  }
  if ( fillColorExpression )
  {
    QString colorString = fillColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    mBrush.setColor( QColor( colorString ) );
  }
  if ( outlineColorExpression )
  {
    QString colorString = outlineColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    mPen.setColor( QColor( colorString ) );
  }
  if ( widthExpression || heightExpression || symbolNameExpression )
  {
    QString symbolName =  mSymbolName;
    if ( symbolNameExpression )
    {
      symbolName = symbolNameExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    }
    preparePath( symbolName, context, context.feature() );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( rotationExpression )
  {
    rotation = rotationExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  else if ( !qgsDoubleNear( mAngle, 0.0 ) )
  {
    rotation = mAngle;
  }
  if ( rotation )
    off = _rotatedOffset( off, rotation );

  QMatrix transform;
  transform.translate( point.x() + off.x(), point.y() + off.y() );
  if ( !qgsDoubleNear( rotation, 0.0 ) )
  {
    transform.rotate( rotation );
  }

  p->setPen( mPen );
  p->setBrush( mBrush );
  p->drawPath( transform.map( mPainterPath ) );
}

QString QgsEllipseSymbolLayerV2::layerType() const
{
  return "EllipseMarker";
}

void QgsEllipseSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  if ( !context.feature() || !hasDataDefinedProperty() )
  {
    preparePath( mSymbolName, context );
  }
  mPen.setColor( mOutlineColor );
  mPen.setWidthF( mOutlineWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOutlineWidthUnit ) );
  mBrush.setColor( mFillColor );
  prepareExpressions( context.layer() );
}

void QgsEllipseSymbolLayerV2::stopRender( QgsSymbolV2RenderContext & )
{
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::clone() const
{
  return QgsEllipseSymbolLayerV2::create( properties() );
}

void QgsEllipseSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PointSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  writeSldMarker( doc, symbolizerElem, props );
}

void QgsEllipseSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, mSymbolName, mFillColor, mOutlineColor, mOutlineWidth, mSymbolWidth );

  // store w/h factor in a <VendorOption>
  double widthHeightFactor = mSymbolWidth / mSymbolHeight;
  QDomElement factorElem = QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "widthHeightFactor", QString::number( widthHeightFactor ) );
  graphicElem.appendChild( factorElem );

  // <Rotation>
  const QgsExpression* rotationExpression = dataDefinedProperty( "rotation" );
  QString angleFunc = props.value( "angle", "" );
  if ( angleFunc.isEmpty() )  // symbol has no angle set
  {

    if ( rotationExpression )
      angleFunc = rotationExpression->dump();
    else if ( !qgsDoubleNear( mAngle, 0.0 ) )
      angleFunc = QString::number( mAngle );
  }
  else if ( rotationExpression )
  {
    // the symbol has an angle and the symbol layer have a rotation
    // property set
    angleFunc = QString( "%1 + %2" ).arg( angleFunc ).arg( rotationExpression->dump() );
  }
  else if ( !qgsDoubleNear( mAngle, 0.0 ) )
  {
    // both the symbol and the symbol layer have angle value set
    bool ok;
    double angle = angleFunc.toDouble( &ok );
    if ( !ok )
    {
      // its a string (probably a property name or a function)
      angleFunc = QString( "%1 + %2" ).arg( angleFunc ).arg( mAngle );
    }
    else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
    {
      // it's a double value
      angleFunc = QString::number( angle + mAngle );
    }
  }
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, angleFunc );
}

QgsSymbolLayerV2* QgsEllipseSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( "Graphic" );
  if ( graphicElem.isNull() )
    return NULL;

  QString name = "circle";
  QColor fillColor, borderColor;
  double borderWidth, size;
  double widthHeightFactor = 1.0;

  QgsStringMap vendorOptions = QgsSymbolLayerV2Utils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "widthHeightFactor" )
    {
      bool ok;
      double v = it.value().toDouble( &ok );
      if ( ok && !qgsDoubleNear( v, 0.0 ) && v > 0 )
        widthHeightFactor = v;
    }
  }

  if ( !QgsSymbolLayerV2Utils::wellKnownMarkerFromSld( graphicElem, name, fillColor, borderColor, borderWidth, size ) )
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

  QgsEllipseSymbolLayerV2 *m = new QgsEllipseSymbolLayerV2();
  m->setSymbolName( name );
  m->setFillColor( fillColor );
  m->setOutlineColor( borderColor );
  m->setOutlineWidth( borderWidth );
  m->setSymbolWidth( size );
  m->setSymbolHeight( size / widthHeightFactor );
  m->setAngle( angle );
  return m;
}

QgsStringMap QgsEllipseSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["symbol_name"] = mSymbolName;
  map["symbol_width"] = QString::number( mSymbolWidth );
  map["symbol_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSymbolWidthUnit );
  map["symbol_height"] = QString::number( mSymbolHeight );
  map["symbol_height_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSymbolHeightUnit );
  map["angle"] = QString::number( mAngle );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );
  map["fill_color"] = QgsSymbolLayerV2Utils::encodeColor( mFillColor );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  saveDataDefinedProperties( map );
  return map;
}

bool QgsEllipseSymbolLayerV2::hasDataDefinedProperty() const
{
  return ( dataDefinedProperty( "width" ) || dataDefinedProperty( "height" ) || dataDefinedProperty( "rotation" )
           || dataDefinedProperty( "outline_width" ) || dataDefinedProperty( "fill_color" ) || dataDefinedProperty( "outline_color" )
           || dataDefinedProperty( "symbol_name" ) || dataDefinedProperty( "offset" ) );
}

void QgsEllipseSymbolLayerV2::preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, const QgsFeature* f )
{
  mPainterPath = QPainterPath();
  const QgsRenderContext& ct = context.renderContext();

  double width = 0;

  QgsExpression* widthExpression = expression( "width" );
  if ( widthExpression ) //1. priority: data defined setting on symbol layer level
  {
    width = widthExpression->evaluate( const_cast<QgsFeature*>( f ) ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    width = mSize;
  }
  else //3. priority: global width setting
  {
    width = mSymbolWidth;
  }
  width *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( ct, mSymbolWidthUnit );

  double height = 0;
  QgsExpression* heightExpression = expression( "height" );
  if ( heightExpression ) //1. priority: data defined setting on symbol layer level
  {
    height =  heightExpression->evaluate( const_cast<QgsFeature*>( f ) ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    height = mSize;
  }
  else //3. priority: global height setting
  {
    height = mSymbolHeight;
  }
  height *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( ct, mSymbolHeightUnit );

  if ( symbolName == "circle" )
  {
    mPainterPath.addEllipse( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if ( symbolName == "rectangle" )
  {
    mPainterPath.addRect( QRectF( -width / 2.0, -height / 2.0, width, height ) );
  }
  else if ( symbolName == "cross" )
  {
    mPainterPath.moveTo( 0, -height / 2.0 );
    mPainterPath.lineTo( 0, height / 2.0 );
    mPainterPath.moveTo( -width / 2.0, 0 );
    mPainterPath.lineTo( width / 2.0, 0 );
  }
  else if ( symbolName == "triangle" )
  {
    mPainterPath.moveTo( 0, -height / 2.0 );
    mPainterPath.lineTo( -width / 2.0, height / 2.0 );
    mPainterPath.lineTo( width / 2.0, height / 2.0 );
    mPainterPath.lineTo( 0, -height / 2.0 );
  }
}

void QgsEllipseSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mSymbolWidthUnit = unit;
  mSymbolHeightUnit = unit;
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsEllipseSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mSymbolWidthUnit;
  if ( mSymbolHeightUnit != unit || mOutlineWidthUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}
