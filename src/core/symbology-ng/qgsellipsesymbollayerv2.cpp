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
#include "qgsdxfexport.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgsdatadefined.h"
#include "qgslogger.h"
#include "qgsunittypes.h"

#include <QPainter>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

QgsEllipseSymbolLayerV2::QgsEllipseSymbolLayerV2()
    : QgsMarkerSymbolLayerV2()
    , mSymbolName( "circle" )
    , mSymbolWidth( 4 )
    , mSymbolWidthUnit( QgsSymbolV2::MM )
    , mSymbolHeight( 3 )
    , mSymbolHeightUnit( QgsSymbolV2::MM )
    , mOutlineColor( Qt::black )
    , mOutlineStyle( Qt::SolidLine )
    , mPenJoinStyle( DEFAULT_ELLIPSE_JOINSTYLE )
    , mOutlineWidth( 0 )
    , mOutlineWidthUnit( QgsSymbolV2::MM )
{
  mColor = Qt::white;
  mPen.setColor( mOutlineColor );
  mPen.setStyle( mOutlineStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidth( 1.0 );
  mBrush.setColor( mColor );
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
  if ( properties.contains( "symbol_width_map_unit_scale" ) )
  {
    layer->setSymbolWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["symbol_width_map_unit_scale"] ) );
  }
  if ( properties.contains( "symbol_height" ) )
  {
    layer->setSymbolHeight( properties["symbol_height"].toDouble() );
  }
  if ( properties.contains( "symbol_height_unit" ) )
  {
    layer->setSymbolHeightUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["symbol_height_unit"] ) );
  }
  if ( properties.contains( "symbol_height_map_unit_scale" ) )
  {
    layer->setSymbolHeightMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["symbol_height_map_unit_scale"] ) );
  }
  if ( properties.contains( "angle" ) )
  {
    layer->setAngle( properties["angle"].toDouble() );
  }
  if ( properties.contains( "outline_style" ) )
  {
    layer->setOutlineStyle( QgsSymbolLayerV2Utils::decodePenStyle( properties["outline_style"] ) );
  }
  else if ( properties.contains( "line_style" ) )
  {
    layer->setOutlineStyle( QgsSymbolLayerV2Utils::decodePenStyle( properties["line_style"] ) );
  }
  if ( properties.contains( "joinstyle" ) )
  {
    layer->setPenJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( properties["joinstyle"] ) );
  }
  if ( properties.contains( "outline_width" ) )
  {
    layer->setOutlineWidth( properties["outline_width"].toDouble() );
  }
  else if ( properties.contains( "line_width" ) )
  {
    layer->setOutlineWidth( properties["line_width"].toDouble() );
  }
  if ( properties.contains( "outline_width_unit" ) )
  {
    layer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["outline_width_unit"] ) );
  }
  else if ( properties.contains( "line_width_unit" ) )
  {
    layer->setOutlineWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["line_width_unit"] ) );
  }
  if ( properties.contains( "outline_width_map_unit_scale" ) )
  {
    layer->setOutlineWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["outline_width_map_unit_scale"] ) );
  }
  if ( properties.contains( "fill_color" ) )
  {
    //pre 2.5 projects used "fill_color"
    layer->setFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["fill_color"] ) );
  }
  else if ( properties.contains( "color" ) )
  {
    layer->setFillColor( QgsSymbolLayerV2Utils::decodeColor( properties["color"] ) );
  }
  if ( properties.contains( "outline_color" ) )
  {
    layer->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["outline_color"] ) );
  }
  else if ( properties.contains( "line_color" ) )
  {
    layer->setOutlineColor( QgsSymbolLayerV2Utils::decodeColor( properties["line_color"] ) );
  }
  if ( properties.contains( "size" ) )
  {
    layer->setSize( properties["size"].toDouble() );
  }
  if ( properties.contains( "size_unit" ) )
  {
    layer->setSizeUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["size_unit"] ) );
  }
  if ( properties.contains( "size_map_unit_scale" ) )
  {
    layer->setSizeMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["size_map_unit_scale"] ) );
  }
  if ( properties.contains( "offset" ) )
  {
    layer->setOffset( QgsSymbolLayerV2Utils::decodePoint( properties["offset"] ) );
  }
  if ( properties.contains( "offset_unit" ) )
  {
    layer->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( properties["offset_unit"] ) );
  }
  if ( properties.contains( "offset_map_unit_scale" ) )
  {
    layer->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( properties["offset_map_unit_scale"] ) );
  }
  if ( properties.contains( "horizontal_anchor_point" ) )
  {
    layer->setHorizontalAnchorPoint( QgsMarkerSymbolLayerV2::HorizontalAnchorPoint( properties[ "horizontal_anchor_point" ].toInt() ) );
  }
  if ( properties.contains( "vertical_anchor_point" ) )
  {
    layer->setVerticalAnchorPoint( QgsMarkerSymbolLayerV2::VerticalAnchorPoint( properties[ "vertical_anchor_point" ].toInt() ) );
  }

  //data defined properties
  layer->restoreDataDefinedProperties( properties );

  //compatibility with old project file format
  if ( !properties["width_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH, new QgsDataDefined( properties["width_field"] ) );
  }
  if ( !properties["height_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT, new QgsDataDefined( properties["height_field"] ) );
  }
  if ( !properties["rotation_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION, new QgsDataDefined( properties["rotation_field"] ) );
  }
  if ( !properties["outline_width_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, new QgsDataDefined( properties[ "outline_width_field" ] ) );
  }
  if ( !properties["fill_color_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL_COLOR, new QgsDataDefined( properties["fill_color_field"] ) );
  }
  if ( !properties["outline_color_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_COLOR, new QgsDataDefined( properties["outline_color_field"] ) );
  }
  if ( !properties["symbol_name_field"].isEmpty() )
  {
    layer->setDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME, new QgsDataDefined( properties["symbol_name_field"] ) );
  }

  return layer;
}

void QgsEllipseSymbolLayerV2::renderPoint( QPointF point, QgsSymbolV2RenderContext& context )
{
  bool ok;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    double width = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
    width = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), width, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
    mPen.setWidthF( width );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle ) );
    QString styleString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      Qt::PenStyle style = QgsSymbolLayerV2Utils::decodePenStyle( styleString );
      mPen.setStyle( style );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle ) );
    QString style = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOIN_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      mPen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( style ) );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      mBrush.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      mPen.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
  }
  double scaledWidth = mSymbolWidth;
  double scaledHeight = mSymbolHeight;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH ) || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT ) || hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME ) )
  {
    QString symbolName =  mSymbolName;
    if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME ) )
    {
      context.setOriginalValueVariable( mSymbolName );
      symbolName = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME, context, mSymbolName ).toString();
    }
    preparePath( symbolName, context, &scaledWidth, &scaledHeight, context.feature() );
  }

  //offset and rotation
  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledWidth, scaledHeight, hasDataDefinedRotation, offset, angle );

  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QMatrix transform;
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );
  if ( !qgsDoubleNear( angle, 0.0 ) )
  {
    transform.rotate( angle );
  }

  p->setPen( mPen );
  p->setBrush( mBrush );
  p->drawPath( transform.map( mPainterPath ) );
}


void QgsEllipseSymbolLayerV2::calculateOffsetAndRotation( QgsSymbolV2RenderContext& context,
    double scaledWidth,
    double scaledHeight,
    bool& hasDataDefinedRotation,
    QPointF& offset,
    double& angle ) const
{
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledWidth, scaledHeight, mSymbolWidthUnit, mSymbolHeightUnit, offsetX, offsetY, mSymbolWidthMapUnitScale, mSymbolHeightMapUnitScale );
  offset = QPointF( offsetX, offsetY );

//priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  bool ok = true;
  angle = mAngle + mLineAngle;
  bool usingDataDefinedRotation = false;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION ) )
  {
    context.setOriginalValueVariable( angle );
    angle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION, context, mAngle, &ok ).toDouble() + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbolV2::DataDefinedRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature* f = context.feature();
    if ( f )
    {
      const QgsGeometry *g = f->constGeometry();
      if ( g && g->type() == QGis::Point )
      {
        const QgsMapToPixel& m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}

QString QgsEllipseSymbolLayerV2::layerType() const
{
  return "EllipseMarker";
}

void QgsEllipseSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QgsMarkerSymbolLayerV2::startRender( context ); // get anchor point expressions
  if ( !context.feature() || !hasDataDefinedProperties() )
  {
    preparePath( mSymbolName, context );
  }
  mPen.setColor( mOutlineColor );
  mPen.setStyle( mOutlineStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );
  mBrush.setColor( mColor );
  prepareExpressions( context );
}

void QgsEllipseSymbolLayerV2::stopRender( QgsSymbolV2RenderContext & )
{
}

QgsEllipseSymbolLayerV2* QgsEllipseSymbolLayerV2::clone() const
{
  QgsEllipseSymbolLayerV2* m = new QgsEllipseSymbolLayerV2();
  m->setSymbolName( mSymbolName );
  m->setSymbolWidth( mSymbolWidth );
  m->setSymbolHeight( mSymbolHeight );
  m->setOutlineStyle( mOutlineStyle );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setOutlineStyle( mOutlineStyle );
  m->setPenJoinStyle( mPenJoinStyle );
  m->setOutlineWidth( mOutlineWidth );
  m->setColor( color() );
  m->setOutlineColor( mOutlineColor );
  m->setSymbolWidthUnit( mSymbolWidthUnit );
  m->setSymbolWidthMapUnitScale( mSymbolWidthMapUnitScale );
  m->setSymbolHeightUnit( mSymbolHeightUnit );
  m->setSymbolHeightMapUnitScale( mSymbolHeightMapUnitScale );
  m->setOutlineWidthUnit( mOutlineWidthUnit );
  m->setOutlineWidthMapUnitScale( mOutlineWidthMapUnitScale );
  m->setAngle( mAngle );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );

  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsEllipseSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:PointSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  writeSldMarker( doc, symbolizerElem, props );
}

void QgsEllipseSymbolLayerV2::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  element.appendChild( graphicElem );

  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, mSymbolName, mColor, mOutlineColor, mOutlineStyle, mOutlineWidth, mSymbolWidth );

  // store w/h factor in a <VendorOption>
  double widthHeightFactor = mSymbolWidth / mSymbolHeight;
  QDomElement factorElem = QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "widthHeightFactor", QString::number( widthHeightFactor ) );
  graphicElem.appendChild( factorElem );

  // <Rotation>
  QgsDataDefined* ddRotation = getDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION );

  QString angleFunc = props.value( "angle", "" );
  if ( angleFunc.isEmpty() )  // symbol has no angle set
  {
    if ( ddRotation && ddRotation->isActive() )
    {
      angleFunc = ddRotation->useExpression() ? ddRotation->expressionString() : ddRotation->field();
    }
    else if ( !qgsDoubleNear( mAngle, 0.0 ) )
      angleFunc = QString::number( mAngle );
  }
  else if ( ddRotation && ddRotation->isActive() )
  {
    // the symbol has an angle and the symbol layer have a rotation
    // property set
    angleFunc = QString( "%1 + %2" ).arg( angleFunc, ddRotation->useExpression() ? ddRotation->expressionString() : ddRotation->field() );
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
    return nullptr;

  QString name = "circle";
  QColor fillColor, borderColor;
  double borderWidth, size;
  double widthHeightFactor = 1.0;
  Qt::PenStyle borderStyle;

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

  if ( !QgsSymbolLayerV2Utils::wellKnownMarkerFromSld( graphicElem, name, fillColor, borderColor, borderStyle, borderWidth, size ) )
    return nullptr;

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
  m->setOutlineStyle( borderStyle );
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
  map["symbol_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSymbolWidthMapUnitScale );
  map["symbol_height"] = QString::number( mSymbolHeight );
  map["symbol_height_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSymbolHeightUnit );
  map["symbol_height_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSymbolHeightMapUnitScale );
  map["angle"] = QString::number( mAngle );
  map["outline_style"] = QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle );
  map["outline_width"] = QString::number( mOutlineWidth );
  map["outline_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOutlineWidthUnit );
  map["outline_width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOutlineWidthMapUnitScale );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["outline_color"] = QgsSymbolLayerV2Utils::encodeColor( mOutlineColor );
  map["offset"] = QgsSymbolLayerV2Utils::encodePoint( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["size"] = QString::number( mSize );
  map["size_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mSizeUnit );
  map["size_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mSizeMapUnitScale );
  map["horizontal_anchor_point"] = QString::number( mHorizontalAnchorPoint );
  map["vertical_anchor_point"] = QString::number( mVerticalAnchorPoint );
  saveDataDefinedProperties( map );
  return map;
}

QSizeF QgsEllipseSymbolLayerV2::calculateSize( QgsSymbolV2RenderContext& context, double* scaledWidth, double* scaledHeight )
{
  double width = 0;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    width = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH, context, mSymbolWidth ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    width = mSize;
  }
  else //3. priority: global width setting
  {
    width = mSymbolWidth;
  }
  if ( scaledWidth )
  {
    *scaledWidth = width;
  }
  width = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), width, mSymbolWidthUnit, mSymbolHeightMapUnitScale );

  double height = 0;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    height = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT, context, mSymbolHeight ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    height = mSize;
  }
  else //3. priority: global height setting
  {
    height = mSymbolHeight;
  }
  if ( scaledHeight )
  {
    *scaledHeight = height;
  }
  height = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), height, mSymbolHeightUnit, mSymbolHeightMapUnitScale );
  return QSizeF( width, height );
}

void QgsEllipseSymbolLayerV2::preparePath( const QString& symbolName, QgsSymbolV2RenderContext& context, double* scaledWidth, double* scaledHeight, const QgsFeature* )
{
  mPainterPath = QPainterPath();

  QSizeF size = calculateSize( context, scaledWidth, scaledHeight );

  if ( symbolName == "circle" )
  {
    mPainterPath.addEllipse( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
  }
  else if ( symbolName == "semi_circle" )
  {
    mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 0, 180 );
    mPainterPath.lineTo( 0, 0 );
  }
  else if ( symbolName == "rectangle" )
  {
    mPainterPath.addRect( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
  }
  else if ( symbolName == "diamond" )
  {
    mPainterPath.moveTo( -size.width() / 2.0, 0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, 0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, 0 );
  }
  else if ( symbolName == "cross" )
  {
    mPainterPath.moveTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.moveTo( -size.width() / 2.0, 0 );
    mPainterPath.lineTo( size.width() / 2.0, 0 );
  }
  else if ( symbolName == "triangle" )
  {
    mPainterPath.moveTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
  }
  else if ( symbolName == "left_half_triangle" )
  {
    mPainterPath.moveTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
  }
  else if ( symbolName == "right_half_triangle" )
  {
    mPainterPath.moveTo( -size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
  }
}

void QgsEllipseSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsMarkerSymbolLayerV2::setOutputUnit( unit );
  mSymbolWidthUnit = unit;
  mSymbolHeightUnit = unit;
  mOutlineWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsEllipseSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsMarkerSymbolLayerV2::outputUnit();
  if ( mSymbolWidthUnit != unit || mSymbolHeightUnit != unit || mOutlineWidthUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsEllipseSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayerV2::setMapUnitScale( scale );
  mSymbolWidthMapUnitScale = scale;
  mSymbolHeightMapUnitScale = scale;
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsEllipseSymbolLayerV2::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayerV2::mapUnitScale() == mSymbolWidthMapUnitScale &&
       mSymbolWidthMapUnitScale == mSymbolHeightMapUnitScale &&
       mSymbolHeightMapUnitScale == mOutlineWidthMapUnitScale )
  {
    return mSymbolWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QRectF QgsEllipseSymbolLayerV2::bounds( QPointF point, QgsSymbolV2RenderContext& context )
{
  QSizeF size = calculateSize( context );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, size.width(), size.height(), hasDataDefinedRotation, offset, angle );

  double pixelSize = 1.0 / context.renderContext().rasterScaleFactor();

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  double penWidth = 0.0;
  bool ok = true;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    double outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, QVariant(), &ok ).toDouble();
    if ( ok )
    {
      penWidth = QgsSymbolLayerV2Utils::convertToPainterUnits( context.renderContext(), outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
    }
  }
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodePenStyle( mOutlineStyle ) );
    QString outlineStyle = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok && outlineStyle == "no" )
    {
      penWidth = 0.0;
    }
  }

  //antialiasing
  penWidth += pixelSize;

  QRectF symbolBounds = transform.mapRect( QRectF( -size.width() / 2.0,
                        -size.height() / 2.0,
                        size.width(),
                        size.height() ) );

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -penWidth / 2.0, -penWidth / 2.0,
                       penWidth / 2.0, penWidth / 2.0 );

  return symbolBounds;
}

bool QgsEllipseSymbolLayerV2::writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolV2RenderContext &context, QPointF shift ) const
{
  //width
  double symbolWidth = mSymbolWidth;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    symbolWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH, context, mSymbolWidth ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    symbolWidth = mSize;
  }
  if ( mSymbolWidthUnit == QgsSymbolV2::MM )
  {
    symbolWidth *= mmMapUnitScaleFactor;
  }

  //height
  double symbolHeight = mSymbolHeight;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    symbolHeight = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_HEIGHT, context, mSymbolHeight ).toDouble();
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale ) //2. priority: is data defined size on symbol level
  {
    symbolHeight = mSize;
  }
  if ( mSymbolHeightUnit == QgsSymbolV2::MM )
  {
    symbolHeight *= mmMapUnitScaleFactor;
  }

  //outline width
  double outlineWidth = mOutlineWidth;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_WIDTH, context, mOutlineWidth ).toDouble();
  }
  if ( mOutlineWidthUnit == QgsSymbolV2::MM )
  {
    outlineWidth *= outlineWidth;
  }

  //fill color
  bool ok;
  QColor fc = mColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_FILL_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      fc = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  //outline color
  QColor oc = mOutlineColor;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerV2Utils::encodeColor( mOutlineColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OUTLINE_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      oc = QgsSymbolLayerV2Utils::decodeColor( colorString );
  }

  //symbol name
  QString symbolName = mSymbolName;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME ) )
  {
    context.setOriginalValueVariable( mSymbolName );
    symbolName = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_SYMBOL_NAME, context, mSymbolName ).toString();
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION ) )
  {
    context.setOriginalValueVariable( mAngle );
    rotation = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_ROTATION, context, mAngle ).toDouble() + mLineAngle;
  }
  else if ( !qgsDoubleNear( mAngle + mLineAngle, 0.0 ) )
  {
    rotation = mAngle + mLineAngle;
  }
  rotation = -rotation; //rotation in Qt is counterclockwise
  if ( rotation )
    off = _rotatedOffset( off, rotation );

  QTransform t;
  t.translate( shift.x() + offsetX, shift.y() + offsetY );

  if ( !qgsDoubleNear( rotation, 0.0 ) )
    t.rotate( rotation );

  double halfWidth = symbolWidth / 2.0;
  double halfHeight = symbolHeight / 2.0;

  if ( symbolName == "circle" )
  {
    if ( qgsDoubleNear( halfWidth, halfHeight ) )
    {
      QgsPointV2 pt( t.map( QPointF( 0, 0 ) ) );
      e.writeFilledCircle( layerName, oc, pt, halfWidth );
    }
    else
    {
      QgsPointSequenceV2 line;

      double stepsize = 2 * M_PI / 40;
      for ( int i = 0; i < 39; ++i )
      {
        double angle = stepsize * i;
        double x = halfWidth * cos( angle );
        double y = halfHeight * sin( angle );
        line << QgsPointV2( t.map( QPointF( x, y ) ) );
      }
      //close ellipse with first point
      line << line.at( 0 );

      if ( mBrush.style() != Qt::NoBrush )
        e.writePolygon( QgsRingSequenceV2() << line, layerName, "SOLID", fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( line, layerName, "CONTINUOUS", oc, outlineWidth );
    }
  }
  else if ( symbolName == "rectangle" )
  {
    QgsPointSequenceV2 p;
    p << QgsPointV2( t.map( QPointF( -halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( -halfWidth, halfHeight ) ) );
    p << p[0];

    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequenceV2() << p, layerName, "SOLID", fc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, "CONTINUOUS", oc, outlineWidth );
    return true;
  }
  else if ( symbolName == "cross" && mPen.style() != Qt::NoPen )
  {
    e.writePolyline( QgsPointSequenceV2()
                     << QgsPointV2( t.map( QPointF( -halfWidth, 0 ) ) )
                     << QgsPointV2( t.map( QPointF( halfWidth, 0 ) ) ),
                     layerName, "CONTINUOUS", oc, outlineWidth );
    e.writePolyline( QgsPointSequenceV2()
                     << QgsPointV2( t.map( QPointF( 0, halfHeight ) ) )
                     << QgsPointV2( t.map( QPointF( 0, -halfHeight ) ) ),
                     layerName, "CONTINUOUS", oc, outlineWidth );
    return true;
  }
  else if ( symbolName == "triangle" )
  {
    QgsPointSequenceV2 p;
    p << QgsPointV2( t.map( QPointF( -halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( 0, halfHeight ) ) );
    p << p[0];
    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequenceV2() << p, layerName, "SOLID", fc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, "CONTINUOUS", oc, outlineWidth );
    return true;
  }

  return false; //soon...
}


