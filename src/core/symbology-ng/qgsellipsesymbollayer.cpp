/***************************************************************************
 qgsellipsesymbollayer.cpp
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
#include "qgsellipsesymbollayer.h"
#include "qgsdxfexport.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsunittypes.h"
#include "qgsproperty.h"

#include <QPainter>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

QgsEllipseSymbolLayer::QgsEllipseSymbolLayer()
    : QgsMarkerSymbolLayer()
    , mSymbolName( QStringLiteral( "circle" ) )
    , mSymbolWidth( 4 )
    , mSymbolWidthUnit( QgsUnitTypes::RenderMillimeters )
    , mSymbolHeight( 3 )
    , mSymbolHeightUnit( QgsUnitTypes::RenderMillimeters )
    , mOutlineColor( Qt::black )
    , mOutlineStyle( Qt::SolidLine )
    , mPenJoinStyle( DEFAULT_ELLIPSE_JOINSTYLE )
    , mOutlineWidth( 0 )
    , mOutlineWidthUnit( QgsUnitTypes::RenderMillimeters )
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

QgsSymbolLayer* QgsEllipseSymbolLayer::create( const QgsStringMap& properties )
{
  QgsEllipseSymbolLayer* layer = new QgsEllipseSymbolLayer();
  if ( properties.contains( QStringLiteral( "symbol_name" ) ) )
  {
    layer->setSymbolName( properties[ QStringLiteral( "symbol_name" )] );
  }
  if ( properties.contains( QStringLiteral( "symbol_width" ) ) )
  {
    layer->setSymbolWidth( properties[QStringLiteral( "symbol_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "symbol_width_unit" ) ) )
  {
    layer->setSymbolWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "symbol_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_width_map_unit_scale" ) ) )
  {
    layer->setSymbolWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "symbol_width_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_height" ) ) )
  {
    layer->setSymbolHeight( properties[QStringLiteral( "symbol_height" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "symbol_height_unit" ) ) )
  {
    layer->setSymbolHeightUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "symbol_height_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_height_map_unit_scale" ) ) )
  {
    layer->setSymbolHeightMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "symbol_height_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    layer->setAngle( properties[QStringLiteral( "angle" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "outline_style" ) ) )
  {
    layer->setOutlineStyle( QgsSymbolLayerUtils::decodePenStyle( properties[QStringLiteral( "outline_style" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "line_style" ) ) )
  {
    layer->setOutlineStyle( QgsSymbolLayerUtils::decodePenStyle( properties[QStringLiteral( "line_style" )] ) );
  }
  if ( properties.contains( QStringLiteral( "joinstyle" ) ) )
  {
    layer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[QStringLiteral( "joinstyle" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width" ) ) )
  {
    layer->setOutlineWidth( properties[QStringLiteral( "outline_width" )].toDouble() );
  }
  else if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    layer->setOutlineWidth( properties[QStringLiteral( "line_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    layer->setOutlineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    layer->setOutlineWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    layer->setOutlineWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "fill_color" ) ) )
  {
    //pre 2.5 projects used "fill_color"
    layer->setFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "fill_color" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    layer->setFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )] ) );
  }
  if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    layer->setOutlineColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )] ) );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    layer->setOutlineColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )] ) );
  }
  if ( properties.contains( QStringLiteral( "size" ) ) )
  {
    layer->setSize( properties[QStringLiteral( "size" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "size_unit" ) ) )
  {
    layer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "size_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "size_map_unit_scale" ) ) )
  {
    layer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "size_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    layer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    layer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )] ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    layer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  if ( properties.contains( QStringLiteral( "horizontal_anchor_point" ) ) )
  {
    layer->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HorizontalAnchorPoint( properties[ QStringLiteral( "horizontal_anchor_point" )].toInt() ) );
  }
  if ( properties.contains( QStringLiteral( "vertical_anchor_point" ) ) )
  {
    layer->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VerticalAnchorPoint( properties[ QStringLiteral( "vertical_anchor_point" )].toInt() ) );
  }

  //data defined properties
  layer->restoreOldDataDefinedProperties( properties );

  return layer;
}

void QgsEllipseSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext& context )
{
  double scaledWidth = mSymbolWidth;
  double scaledHeight = mSymbolHeight;

  if ( mProperties.hasActiveProperties() )
  {
    bool ok;
    context.setOriginalValueVariable( mOutlineWidth );
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyOutlineWidth, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      double width = exprVal.toDouble( &ok );
      if ( ok )
      {
        width = context.renderContext().convertToPainterUnits( width, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
        mPen.setWidthF( width );
      }
    }

    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mOutlineStyle ) );
    exprVal = mProperties.value( QgsSymbolLayer::PropertyOutlineStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      mPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
    }

    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    exprVal = mProperties.value( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
    }

    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    mBrush.setColor( mProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor ) );

    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mOutlineColor ) );
    mPen.setColor( mProperties.valueAsColor( QgsSymbolLayer::PropertyOutlineColor, context.renderContext().expressionContext(), mOutlineColor ) );

    if ( mProperties.isActive( QgsSymbolLayer::PropertyWidth ) || mProperties.isActive( QgsSymbolLayer::PropertyHeight ) || mProperties.isActive( QgsSymbolLayer::PropertyName ) )
    {
      QString symbolName =  mSymbolName;
      context.setOriginalValueVariable( mSymbolName );
      exprVal = mProperties.value( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext() );
      if ( exprVal.isValid() )
      {
        symbolName = exprVal.toString();
      }
      preparePath( symbolName, context, &scaledWidth, &scaledHeight, context.feature() );
    }
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


void QgsEllipseSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext& context,
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
  if ( mProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), 0 ) + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & QgsSymbol::DynamicRotation || usingDataDefinedRotation;
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
      const QgsGeometry g = f->geometry();
      if ( !g.isEmpty() && g.type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel& m2p = context.renderContext().mapToPixel();
        angle += m2p.mapRotation();
      }
    }
  }

  if ( angle )
    offset = _rotatedOffset( offset, angle );
}

QString QgsEllipseSymbolLayer::layerType() const
{
  return QStringLiteral( "EllipseMarker" );
}

void QgsEllipseSymbolLayer::startRender( QgsSymbolRenderContext& context )
{
  QgsMarkerSymbolLayer::startRender( context ); // get anchor point expressions
  if ( !context.feature() || !dataDefinedProperties().hasActiveProperties() )
  {
    preparePath( mSymbolName, context );
  }
  mPen.setColor( mOutlineColor );
  mPen.setStyle( mOutlineStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mOutlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale ) );
  mBrush.setColor( mColor );
}

void QgsEllipseSymbolLayer::stopRender( QgsSymbolRenderContext & )
{
}

QgsEllipseSymbolLayer* QgsEllipseSymbolLayer::clone() const
{
  QgsEllipseSymbolLayer* m = new QgsEllipseSymbolLayer();
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

void QgsEllipseSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QLatin1String( "" ) ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QLatin1String( "" ) ) );

  writeSldMarker( doc, symbolizerElem, props );
}

void QgsEllipseSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  double outlineWidth = QgsSymbolLayerUtils::rescaleUom( mOutlineWidth, mOutlineWidthUnit, props );
  double symbolWidth = QgsSymbolLayerUtils::rescaleUom( mSymbolWidth, mSymbolWidthUnit, props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, mSymbolName, mColor, mOutlineColor, mOutlineStyle, outlineWidth, symbolWidth );

  // <Rotation>
  QgsProperty ddRotation = mProperties.property( QgsSymbolLayer::PropertyAngle );

  QString angleFunc = props.value( QStringLiteral( "angle" ), QLatin1String( "" ) );
  if ( angleFunc.isEmpty() )  // symbol has no angle set
  {
    if ( ddRotation && ddRotation.isActive() )
    {
      angleFunc = ddRotation.asExpression();
    }
    else if ( !qgsDoubleNear( mAngle, 0.0 ) )
      angleFunc = QString::number( mAngle );
  }
  else if ( ddRotation && ddRotation.isActive() )
  {
    // the symbol has an angle and the symbol layer have a rotation
    // property set
    angleFunc = QStringLiteral( "%1 + %2" ).arg( angleFunc, ddRotation.asExpression() );
  }
  else if ( !qgsDoubleNear( mAngle, 0.0 ) )
  {
    // both the symbol and the symbol layer have angle value set
    bool ok;
    double angle = angleFunc.toDouble( &ok );
    if ( !ok )
    {
      // its a string (probably a property name or a function)
      angleFunc = QStringLiteral( "%1 + %2" ).arg( angleFunc ).arg( mAngle );
    }
    else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
    {
      // it's a double value
      angleFunc = QString::number( angle + mAngle );
    }
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc );

  // <Displacement>
  QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );

  // store w/h factor in a <VendorOption>
  double widthHeightFactor = mSymbolWidth / mSymbolHeight;
  QDomElement factorElem = QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "widthHeightFactor" ), QString::number( widthHeightFactor ) );
  graphicElem.appendChild( factorElem );
}

QgsSymbolLayer* QgsEllipseSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name = QStringLiteral( "circle" );
  QColor fillColor, borderColor;
  double borderWidth, size;
  double widthHeightFactor = 1.0;
  Qt::PenStyle borderStyle;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == QLatin1String( "widthHeightFactor" ) )
    {
      bool ok;
      double v = it.value().toDouble( &ok );
      if ( ok && !qgsDoubleNear( v, 0.0 ) && v > 0 )
        widthHeightFactor = v;
    }
  }

  if ( !QgsSymbolLayerUtils::wellKnownMarkerFromSld( graphicElem, name, fillColor, borderColor, borderStyle, borderWidth, size ) )
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

  QgsEllipseSymbolLayer *m = new QgsEllipseSymbolLayer();
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

QgsStringMap QgsEllipseSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "symbol_name" )] = mSymbolName;
  map[QStringLiteral( "symbol_width" )] = QString::number( mSymbolWidth );
  map[QStringLiteral( "symbol_width_unit" )] = QgsUnitTypes::encodeUnit( mSymbolWidthUnit );
  map[QStringLiteral( "symbol_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolWidthMapUnitScale );
  map[QStringLiteral( "symbol_height" )] = QString::number( mSymbolHeight );
  map[QStringLiteral( "symbol_height_unit" )] = QgsUnitTypes::encodeUnit( mSymbolHeightUnit );
  map[QStringLiteral( "symbol_height_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolHeightMapUnitScale );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "outline_style" )] = QgsSymbolLayerUtils::encodePenStyle( mOutlineStyle );
  map[QStringLiteral( "outline_width" )] = QString::number( mOutlineWidth );
  map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mOutlineWidthUnit );
  map[QStringLiteral( "outline_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOutlineWidthMapUnitScale );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mOutlineColor );
  map[QStringLiteral( "offset" )] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "size" )] = QString::number( mSize );
  map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[QStringLiteral( "size_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[QStringLiteral( "horizontal_anchor_point" )] = QString::number( mHorizontalAnchorPoint );
  map[QStringLiteral( "vertical_anchor_point" )] = QString::number( mVerticalAnchorPoint );
  return map;
}

QSizeF QgsEllipseSymbolLayer::calculateSize( QgsSymbolRenderContext& context, double* scaledWidth, double* scaledHeight )
{
  double width = 0;

  if ( mProperties.isActive( QgsSymbolLayer::PropertyWidth ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    width = mProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSymbolWidth );
  }
  else //2. priority: global width setting
  {
    width = mSymbolWidth;
  }
  if ( scaledWidth )
  {
    *scaledWidth = width;
  }
  width = context.renderContext().convertToPainterUnits( width, mSymbolWidthUnit, mSymbolHeightMapUnitScale );

  double height = 0;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyHeight ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    height = mProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), mSymbolHeight );
  }
  else //2. priority: global height setting
  {
    height = mSymbolHeight;
  }
  if ( scaledHeight )
  {
    *scaledHeight = height;
  }
  height = context.renderContext().convertToPainterUnits( height, mSymbolHeightUnit, mSymbolHeightMapUnitScale );
  return QSizeF( width, height );
}

void QgsEllipseSymbolLayer::preparePath( const QString& symbolName, QgsSymbolRenderContext& context, double* scaledWidth, double* scaledHeight, const QgsFeature* )
{
  mPainterPath = QPainterPath();

  QSizeF size = calculateSize( context, scaledWidth, scaledHeight );

  if ( symbolName == QLatin1String( "circle" ) )
  {
    mPainterPath.addEllipse( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
  }
  else if ( symbolName == QLatin1String( "semi_circle" ) )
  {
    mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 0, 180 );
    mPainterPath.lineTo( 0, 0 );
  }
  else if ( symbolName == QLatin1String( "rectangle" ) )
  {
    mPainterPath.addRect( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
  }
  else if ( symbolName == QLatin1String( "diamond" ) )
  {
    mPainterPath.moveTo( -size.width() / 2.0, 0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, 0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, 0 );
  }
  else if ( symbolName == QLatin1String( "cross" ) )
  {
    mPainterPath.moveTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.moveTo( -size.width() / 2.0, 0 );
    mPainterPath.lineTo( size.width() / 2.0, 0 );
  }
  else if ( symbolName == QLatin1String( "triangle" ) )
  {
    mPainterPath.moveTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
  }
  else if ( symbolName == QLatin1String( "left_half_triangle" ) )
  {
    mPainterPath.moveTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
  }
  else if ( symbolName == QLatin1String( "right_half_triangle" ) )
  {
    mPainterPath.moveTo( -size.width() / 2.0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, size.height() / 2.0 );
    mPainterPath.lineTo( 0, -size.height() / 2.0 );
    mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
  }
}

void QgsEllipseSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mSymbolWidthUnit = unit;
  mSymbolHeightUnit = unit;
  mOutlineWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsEllipseSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsMarkerSymbolLayer::outputUnit();
  if ( mSymbolWidthUnit != unit || mSymbolHeightUnit != unit || mOutlineWidthUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsEllipseSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
  mSymbolWidthMapUnitScale = scale;
  mSymbolHeightMapUnitScale = scale;
  mOutlineWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsEllipseSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayer::mapUnitScale() == mSymbolWidthMapUnitScale &&
       mSymbolWidthMapUnitScale == mSymbolHeightMapUnitScale &&
       mSymbolHeightMapUnitScale == mOutlineWidthMapUnitScale )
  {
    return mSymbolWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QRectF QgsEllipseSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext& context )
{
  QSizeF size = calculateSize( context );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, size.width(), size.height(), hasDataDefinedRotation, offset, angle );

  QMatrix transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  double penWidth = 0.0;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyOutlineWidth ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyOutlineWidth, context.renderContext().expressionContext() );

    if ( exprVal.isValid() )
    {
      bool ok;
      double outlineWidth = exprVal.toDouble( &ok );
      if ( ok )
      {
        penWidth = context.renderContext().convertToPainterUnits( outlineWidth, mOutlineWidthUnit, mOutlineWidthMapUnitScale );
      }
    }
  }
  if ( mProperties.isActive( QgsSymbolLayer::PropertyOutlineStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mOutlineStyle ) );
    QVariant exprVal = mProperties.value( QgsSymbolLayer::PropertyOutlineStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() && exprVal.toString() == QLatin1String( "no" ) )
    {
      penWidth = 0.0;
    }
  }

  //antialiasing, add 1 pixel
  penWidth += 1;

  QRectF symbolBounds = transform.mapRect( QRectF( -size.width() / 2.0,
                        -size.height() / 2.0,
                        size.width(),
                        size.height() ) );

  //extend bounds by pen width / 2.0
  symbolBounds.adjust( -penWidth / 2.0, -penWidth / 2.0,
                       penWidth / 2.0, penWidth / 2.0 );

  return symbolBounds;
}

bool QgsEllipseSymbolLayer::writeDxf( QgsDxfExport& e, double mmMapUnitScaleFactor, const QString& layerName, QgsSymbolRenderContext &context, QPointF shift ) const
{
  //width
  double symbolWidth = mSymbolWidth;

  if ( mProperties.isActive( QgsSymbolLayer::PropertyWidth ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    symbolWidth = mProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSymbolWidth );
  }
  if ( mSymbolWidthUnit == QgsUnitTypes::RenderMillimeters )
  {
    symbolWidth *= mmMapUnitScaleFactor;
  }

  //height
  double symbolHeight = mSymbolHeight;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyHeight ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    symbolWidth = mProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), mSymbolHeight );
  }
  if ( mSymbolHeightUnit == QgsUnitTypes::RenderMillimeters )
  {
    symbolHeight *= mmMapUnitScaleFactor;
  }

  //outline width
  double outlineWidth = mOutlineWidth;

  if ( mProperties.isActive( QgsSymbolLayer::PropertyOutlineWidth ) )
  {
    context.setOriginalValueVariable( mOutlineWidth );
    outlineWidth = mProperties.valueAsDouble( QgsSymbolLayer::PropertyOutlineWidth, context.renderContext().expressionContext(), mOutlineWidth );
  }
  if ( mOutlineWidthUnit == QgsUnitTypes::RenderMillimeters )
  {
    outlineWidth *= outlineWidth;
  }

  //fill color
  QColor fc = mColor;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fc = mProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  //outline color
  QColor oc = mOutlineColor;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyOutlineColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mOutlineColor ) );
    oc = mProperties.valueAsColor( QgsSymbolLayer::PropertyOutlineColor, context.renderContext().expressionContext(), mOutlineColor );
  }

  //symbol name
  QString symbolName = mSymbolName;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( mSymbolName );
    symbolName = mProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), mSymbolName );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( mProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    rotation = mProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
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

  if ( symbolName == QLatin1String( "circle" ) )
  {
    if ( qgsDoubleNear( halfWidth, halfHeight ) )
    {
      QgsPointV2 pt( t.map( QPointF( 0, 0 ) ) );
      e.writeFilledCircle( layerName, oc, pt, halfWidth );
    }
    else
    {
      QgsPointSequence line;

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
        e.writePolygon( QgsRingSequence() << line, layerName, QStringLiteral( "SOLID" ), fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( line, layerName, QStringLiteral( "CONTINUOUS" ), oc, outlineWidth );
    }
  }
  else if ( symbolName == QLatin1String( "rectangle" ) )
  {
    QgsPointSequence p;
    p << QgsPointV2( t.map( QPointF( -halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( -halfWidth, halfHeight ) ) );
    p << p[0];

    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequence() << p, layerName, QStringLiteral( "SOLID" ), fc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, QStringLiteral( "CONTINUOUS" ), oc, outlineWidth );
    return true;
  }
  else if ( symbolName == QLatin1String( "cross" ) && mPen.style() != Qt::NoPen )
  {
    e.writePolyline( QgsPointSequence()
                     << QgsPointV2( t.map( QPointF( -halfWidth, 0 ) ) )
                     << QgsPointV2( t.map( QPointF( halfWidth, 0 ) ) ),
                     layerName, QStringLiteral( "CONTINUOUS" ), oc, outlineWidth );
    e.writePolyline( QgsPointSequence()
                     << QgsPointV2( t.map( QPointF( 0, halfHeight ) ) )
                     << QgsPointV2( t.map( QPointF( 0, -halfHeight ) ) ),
                     layerName, QStringLiteral( "CONTINUOUS" ), oc, outlineWidth );
    return true;
  }
  else if ( symbolName == QLatin1String( "triangle" ) )
  {
    QgsPointSequence p;
    p << QgsPointV2( t.map( QPointF( -halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( halfWidth, -halfHeight ) ) )
    << QgsPointV2( t.map( QPointF( 0, halfHeight ) ) );
    p << p[0];
    if ( mBrush.style() != Qt::NoBrush )
      e.writePolygon( QgsRingSequence() << p, layerName, QStringLiteral( "SOLID" ), fc );
    if ( mPen.style() != Qt::NoPen )
      e.writePolyline( p, layerName, QStringLiteral( "CONTINUOUS" ), oc, outlineWidth );
    return true;
  }

  return false; //soon...
}


