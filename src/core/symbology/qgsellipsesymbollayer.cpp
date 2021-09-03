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
#include "qgssymbollayerutils.h"

#include <QPainter>
#include <QSet>
#include <QDomDocument>
#include <QDomElement>

QgsEllipseSymbolLayer::QgsEllipseSymbolLayer()
  : mStrokeColor( QColor( 35, 35, 35 ) )
{
  mColor = Qt::white;
  mPen.setColor( mStrokeColor );
  mPen.setStyle( mStrokeStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setWidth( 1.0 );
  mBrush.setColor( mColor );
  mBrush.setStyle( Qt::SolidPattern );
  mOffset = QPointF( 0, 0 );
  mAngle = 0;
}

QgsEllipseSymbolLayer::~QgsEllipseSymbolLayer() = default;

QgsSymbolLayer *QgsEllipseSymbolLayer::create( const QVariantMap &properties )
{
  QgsEllipseSymbolLayer *layer = new QgsEllipseSymbolLayer();
  if ( properties.contains( QStringLiteral( "symbol_name" ) ) )
  {
    layer->setShape( decodeShape( properties[ QStringLiteral( "symbol_name" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "size" ) ) )
  {
    layer->setSize( properties[QStringLiteral( "size" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "size_unit" ) ) )
  {
    layer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "size_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "size_map_unit_scale" ) ) )
  {
    layer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "size_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_width" ) ) )
  {
    layer->setSymbolWidth( properties[QStringLiteral( "symbol_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "symbol_width_unit" ) ) )
  {
    layer->setSymbolWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "symbol_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_width_map_unit_scale" ) ) )
  {
    layer->setSymbolWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "symbol_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_height" ) ) )
  {
    layer->setSymbolHeight( properties[QStringLiteral( "symbol_height" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "symbol_height_unit" ) ) )
  {
    layer->setSymbolHeightUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "symbol_height_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "symbol_height_map_unit_scale" ) ) )
  {
    layer->setSymbolHeightMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "symbol_height_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "angle" ) ) )
  {
    layer->setAngle( properties[QStringLiteral( "angle" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "outline_style" ) ) )
  {
    layer->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( properties[QStringLiteral( "outline_style" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "line_style" ) ) )
  {
    layer->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( properties[QStringLiteral( "line_style" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "joinstyle" ) ) )
  {
    layer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[QStringLiteral( "joinstyle" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "cap_style" ) ) )
  {
    layer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[QStringLiteral( "cap_style" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width" ) ) )
  {
    layer->setStrokeWidth( properties[QStringLiteral( "outline_width" )].toDouble() );
  }
  else if ( properties.contains( QStringLiteral( "line_width" ) ) )
  {
    layer->setStrokeWidth( properties[QStringLiteral( "line_width" )].toDouble() );
  }
  if ( properties.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "outline_width_unit" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "line_width_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_width_map_unit_scale" ) ) )
  {
    layer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "outline_width_map_unit_scale" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "fill_color" ) ) )
  {
    //pre 2.5 projects used "fill_color"
    layer->setFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "fill_color" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "color" ) ) )
  {
    layer->setFillColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "color" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "outline_color" ) ) )
  {
    layer->setStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "outline_color" )].toString() ) );
  }
  else if ( properties.contains( QStringLiteral( "line_color" ) ) )
  {
    layer->setStrokeColor( QgsSymbolLayerUtils::decodeColor( properties[QStringLiteral( "line_color" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset" ) ) )
  {
    layer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[QStringLiteral( "offset" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_unit" ) ) )
  {
    layer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[QStringLiteral( "offset_unit" )].toString() ) );
  }
  if ( properties.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    layer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[QStringLiteral( "offset_map_unit_scale" )].toString() ) );
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

void QgsEllipseSymbolLayer::renderPoint( QPointF point, QgsSymbolRenderContext &context )
{
  double scaledWidth = mSymbolWidth;
  double scaledHeight = mSymbolHeight;

  QColor brushColor = mColor;
  brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
  mBrush.setColor( brushColor );

  QColor penColor = mStrokeColor;
  penColor.setAlphaF( penColor.alphaF() * context.opacity() );
  mPen.setColor( penColor );

  bool ok;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      double width = exprVal.toDouble( &ok );
      if ( ok )
      {
        width = context.renderContext().convertToPainterUnits( width, mStrokeWidthUnit, mStrokeWidthMapUnitScale );
        mPen.setWidthF( width );
        mSelPen.setWidthF( width );
      }
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      mPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
      mSelPen.setStyle( mPen.style() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
      mSelPen.setJoinStyle( mPen.joinStyle() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    const QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyCapStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
      mSelPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QColor brushColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
    brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
    mBrush.setColor( brushColor );
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    QColor penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
    penColor.setAlphaF( penColor.alphaF() * context.opacity() );
    mPen.setColor( penColor );
  }

  QgsEllipseSymbolLayer::Shape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) || mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() )
    {
      shape = decodeShape( exprVal.toString() );
    }
    preparePath( shape, context, &scaledWidth, &scaledHeight, context.feature() );
  }

  //offset and rotation
  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, scaledWidth, scaledHeight, hasDataDefinedRotation, offset, angle );

  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  QTransform transform;
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );
  if ( !qgsDoubleNear( angle, 0.0 ) )
  {
    transform.rotate( angle );
  }

  if ( shapeIsFilled( shape ) )
  {
    p->setPen( context.selected() ? mSelPen : mPen );
    p->setBrush( context.selected() ? mSelBrush : mBrush );
  }
  else
  {
    p->setPen( context.selected() ? mSelPen : mPen );
    p->setBrush( QBrush() );
  }
  p->drawPath( transform.map( mPainterPath ) );
}


void QgsEllipseSymbolLayer::calculateOffsetAndRotation( QgsSymbolRenderContext &context,
    double scaledWidth,
    double scaledHeight,
    bool &hasDataDefinedRotation,
    QPointF &offset,
    double &angle ) const
{
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, scaledWidth, scaledHeight, mSymbolWidthUnit, mSymbolHeightUnit, offsetX, offsetY, mSymbolWidthMapUnitScale, mSymbolHeightMapUnitScale );
  offset = QPointF( offsetX, offsetY );

//priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  const bool ok = true;
  angle = mAngle + mLineAngle;
  bool usingDataDefinedRotation = false;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), 0 ) + mLineAngle;
    usingDataDefinedRotation = ok;
  }

  hasDataDefinedRotation = context.renderHints() & Qgis::SymbolRenderHint::DynamicRotation || usingDataDefinedRotation;
  if ( hasDataDefinedRotation )
  {
    // For non-point markers, "dataDefinedRotation" means following the
    // shape (shape-data defined). For them, "field-data defined" does
    // not work at all. TODO: if "field-data defined" ever gets implemented
    // we'll need a way to distinguish here between the two, possibly
    // using another flag in renderHints()
    const QgsFeature *f = context.feature();
    if ( f )
    {
      const QgsGeometry g = f->geometry();
      if ( !g.isNull() && g.type() == QgsWkbTypes::PointGeometry )
      {
        const QgsMapToPixel &m2p = context.renderContext().mapToPixel();
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

void QgsEllipseSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QgsMarkerSymbolLayer::startRender( context ); // get anchor point expressions
  if ( !context.feature() || !dataDefinedProperties().hasActiveProperties() )
  {
    preparePath( mShape, context );
  }
  mPen.setColor( mStrokeColor );
  mPen.setStyle( mStrokeStyle );
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setCapStyle( mPenCapStyle );
  mPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
  mBrush.setColor( mColor );

  QColor selBrushColor = context.renderContext().selectionColor();
  QColor selPenColor = selBrushColor == mColor ? selBrushColor : mStrokeColor;
  if ( context.opacity() < 1  && !SELECTION_IS_OPAQUE )
  {
    selBrushColor.setAlphaF( context.opacity() );
    selPenColor.setAlphaF( context.opacity() );
  }
  mSelBrush = QBrush( selBrushColor );
  mSelPen = QPen( !shapeIsFilled( mShape ) ? selBrushColor : selPenColor );
  mSelPen.setStyle( mStrokeStyle );
  mSelPen.setWidthF( context.renderContext().convertToPainterUnits( mStrokeWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale ) );
}

void QgsEllipseSymbolLayer::stopRender( QgsSymbolRenderContext & )
{
}

QgsEllipseSymbolLayer *QgsEllipseSymbolLayer::clone() const
{
  QgsEllipseSymbolLayer *m = new QgsEllipseSymbolLayer();
  m->setShape( mShape );
  m->setSymbolWidth( mSymbolWidth );
  m->setSymbolHeight( mSymbolHeight );
  m->setStrokeStyle( mStrokeStyle );
  m->setOffset( mOffset );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setStrokeStyle( mStrokeStyle );
  m->setPenJoinStyle( mPenJoinStyle );
  m->setPenCapStyle( mPenCapStyle );
  m->setStrokeWidth( mStrokeWidth );
  m->setColor( color() );
  m->setStrokeColor( mStrokeColor );
  m->setSymbolWidthUnit( mSymbolWidthUnit );
  m->setSymbolWidthMapUnitScale( mSymbolWidthMapUnitScale );
  m->setSymbolHeightUnit( mSymbolHeightUnit );
  m->setSymbolHeightMapUnitScale( mSymbolHeightMapUnitScale );
  m->setStrokeWidthUnit( mStrokeWidthUnit );
  m->setStrokeWidthMapUnitScale( mStrokeWidthMapUnitScale );
  m->setAngle( mAngle );
  m->setHorizontalAnchorPoint( mHorizontalAnchorPoint );
  m->setVerticalAnchorPoint( mVerticalAnchorPoint );

  copyDataDefinedProperties( m );
  copyPaintEffect( m );
  return m;
}

void QgsEllipseSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:PointSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ).toString() );

  writeSldMarker( doc, symbolizerElem, props );
}

void QgsEllipseSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( QStringLiteral( "se:Graphic" ) );
  element.appendChild( graphicElem );

  const double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  const double symbolWidth = QgsSymbolLayerUtils::rescaleUom( mSymbolWidth, mSymbolWidthUnit, props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, encodeShape( mShape ), mColor, mStrokeColor, mStrokeStyle, strokeWidth, symbolWidth );

  // <Rotation>
  const QgsProperty ddRotation = mDataDefinedProperties.property( QgsSymbolLayer::PropertyAngle );

  QString angleFunc = props.value( QStringLiteral( "angle" ), QString() ).toString();
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
    const double angle = angleFunc.toDouble( &ok );
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
  const QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );

  // store w/h factor in a <VendorOption>
  const double widthHeightFactor = mSymbolWidth / mSymbolHeight;
  const QDomElement factorElem = QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "widthHeightFactor" ), QString::number( widthHeightFactor ) );
  graphicElem.appendChild( factorElem );
}

QgsSymbolLayer *QgsEllipseSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered." ), 4 );

  QDomElement graphicElem = element.firstChildElement( QStringLiteral( "Graphic" ) );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name = QStringLiteral( "circle" );
  QColor fillColor, strokeColor;
  double strokeWidth, size;
  double widthHeightFactor = 1.0;
  Qt::PenStyle strokeStyle;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == QLatin1String( "widthHeightFactor" ) )
    {
      bool ok;
      const double v = it.value().toDouble( &ok );
      if ( ok && !qgsDoubleNear( v, 0.0 ) && v > 0 )
        widthHeightFactor = v;
    }
  }

  if ( !QgsSymbolLayerUtils::wellKnownMarkerFromSld( graphicElem, name, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
    return nullptr;

  const QString uom = element.attribute( QStringLiteral( "uom" ) );
  size = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, size );
  strokeWidth = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, strokeWidth );

  double angle = 0.0;
  QString angleFunc;
  if ( QgsSymbolLayerUtils::rotationFromSldElement( graphicElem, angleFunc ) )
  {
    bool ok;
    const double d = angleFunc.toDouble( &ok );
    if ( ok )
      angle = d;
  }

  QgsEllipseSymbolLayer *m = new QgsEllipseSymbolLayer();
  m->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  m->setShape( decodeShape( name ) );
  m->setFillColor( fillColor );
  m->setStrokeColor( strokeColor );
  m->setStrokeStyle( strokeStyle );
  m->setStrokeWidth( strokeWidth );
  m->setSymbolWidth( size );
  m->setSymbolHeight( size / widthHeightFactor );
  m->setAngle( angle );
  return m;
}

QVariantMap QgsEllipseSymbolLayer::properties() const
{
  QVariantMap map;
  map[QStringLiteral( "symbol_name" )] = encodeShape( mShape );
  map[QStringLiteral( "symbol_width" )] = QString::number( mSymbolWidth );
  map[QStringLiteral( "symbol_width_unit" )] = QgsUnitTypes::encodeUnit( mSymbolWidthUnit );
  map[QStringLiteral( "symbol_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolWidthMapUnitScale );
  map[QStringLiteral( "symbol_height" )] = QString::number( mSymbolHeight );
  map[QStringLiteral( "symbol_height_unit" )] = QgsUnitTypes::encodeUnit( mSymbolHeightUnit );
  map[QStringLiteral( "symbol_height_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolHeightMapUnitScale );
  map[QStringLiteral( "angle" )] = QString::number( mAngle );
  map[QStringLiteral( "outline_style" )] = QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle );
  map[QStringLiteral( "outline_width" )] = QString::number( mStrokeWidth );
  map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  map[QStringLiteral( "outline_width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "cap_style" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[QStringLiteral( "color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "outline_color" )] = QgsSymbolLayerUtils::encodeColor( mStrokeColor );
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

QSizeF QgsEllipseSymbolLayer::calculateSize( QgsSymbolRenderContext &context, double *scaledWidth, double *scaledHeight )
{
  double width = 0;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSymbolWidth );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    height = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), mSymbolHeight );
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

void QgsEllipseSymbolLayer::preparePath( const QgsEllipseSymbolLayer::Shape &shape, QgsSymbolRenderContext &context, double *scaledWidth, double *scaledHeight, const QgsFeature * )
{
  mPainterPath = QPainterPath();

  const QSizeF size = calculateSize( context, scaledWidth, scaledHeight );

  switch ( shape )
  {
    case Circle:
      mPainterPath.addEllipse( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
      return;

    case SemiCircle:
      mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 0, 180 );
      mPainterPath.lineTo( 0, 0 );
      return;

    case Rectangle:
      mPainterPath.addRect( QRectF( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height() ) );
      return;

    case Diamond:
      mPainterPath.moveTo( -size.width() / 2.0, 0 );
      mPainterPath.lineTo( 0, size.height() / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, 0 );
      mPainterPath.lineTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( -size.width() / 2.0, 0 );
      return;

    case Cross:
      mPainterPath.moveTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( 0, size.height() / 2.0 );
      mPainterPath.moveTo( -size.width() / 2.0, 0 );
      mPainterPath.lineTo( size.width() / 2.0, 0 );
      return;

    case Arrow:
      mPainterPath.moveTo( -size.width() / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
      return;

    case HalfArc:
      mPainterPath.moveTo( size.width() / 2.0, 0 );
      mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 0, 180 );
      return;

    case Triangle:
      mPainterPath.moveTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( 0, -size.height() / 2.0 );
      return;

    case LeftHalfTriangle:
      mPainterPath.moveTo( 0, size.height() / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( 0, size.height() / 2.0 );
      return;

    case RightHalfTriangle:
      mPainterPath.moveTo( -size.width() / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( 0, size.height() / 2.0 );
      mPainterPath.lineTo( 0, -size.height() / 2.0 );
      mPainterPath.lineTo( -size.width() / 2.0, size.height() / 2.0 );
      return;
  }
}

bool QgsEllipseSymbolLayer::shapeIsFilled( const QgsEllipseSymbolLayer::Shape &shape )
{
  switch ( shape )
  {
    case Circle:
    case Rectangle:
    case Diamond:
    case Triangle:
    case RightHalfTriangle:
    case LeftHalfTriangle:
    case SemiCircle:
      return true;

    case Cross:
    case Arrow:
    case HalfArc:
      return false;
  }

  return true;
}

void QgsEllipseSymbolLayer::setSize( double size )
{
  if ( mSymbolWidth >= mSymbolHeight )
  {
    mSymbolHeight = mSymbolHeight * size / mSymbolWidth;
    mSymbolWidth = size;
  }
  else
  {
    mSymbolWidth = mSymbolWidth * size / mSymbolHeight;
    mSymbolHeight = size;
  }
  QgsMarkerSymbolLayer::setSize( size );
}

void QgsEllipseSymbolLayer::setSymbolWidth( double w )
{
  mSymbolWidth = w;
  QgsMarkerSymbolLayer::setSize( mSymbolWidth >= mSymbolHeight ? mSymbolWidth : mSymbolHeight );
}

void QgsEllipseSymbolLayer::setSymbolHeight( double h )
{
  mSymbolHeight = h;
  QgsMarkerSymbolLayer::setSize( mSymbolWidth >= mSymbolHeight ? mSymbolWidth : mSymbolHeight );
}

void QgsEllipseSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mSymbolWidthUnit = unit;
  mSymbolHeightUnit = unit;
  mStrokeWidthUnit = unit;
}

QgsUnitTypes::RenderUnit QgsEllipseSymbolLayer::outputUnit() const
{
  const QgsUnitTypes::RenderUnit unit = QgsMarkerSymbolLayer::outputUnit();
  if ( mSymbolWidthUnit != unit || mSymbolHeightUnit != unit || mStrokeWidthUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

bool QgsEllipseSymbolLayer::usesMapUnits() const
{
  return mSymbolWidthUnit == QgsUnitTypes::RenderMapUnits || mSymbolWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mSymbolHeightUnit == QgsUnitTypes::RenderMapUnits || mSymbolHeightUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mStrokeWidthUnit == QgsUnitTypes::RenderMapUnits || mStrokeWidthUnit == QgsUnitTypes::RenderMetersInMapUnits
         || mOffsetUnit == QgsUnitTypes::RenderMapUnits || mOffsetUnit == QgsUnitTypes::RenderMetersInMapUnits;
}

void QgsEllipseSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsMarkerSymbolLayer::setMapUnitScale( scale );
  mSymbolWidthMapUnitScale = scale;
  mSymbolHeightMapUnitScale = scale;
  mStrokeWidthMapUnitScale = scale;
}

QgsMapUnitScale QgsEllipseSymbolLayer::mapUnitScale() const
{
  if ( QgsMarkerSymbolLayer::mapUnitScale() == mSymbolWidthMapUnitScale &&
       mSymbolWidthMapUnitScale == mSymbolHeightMapUnitScale &&
       mSymbolHeightMapUnitScale == mStrokeWidthMapUnitScale )
  {
    return mSymbolWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QRectF QgsEllipseSymbolLayer::bounds( QPointF point, QgsSymbolRenderContext &context )
{
  const QSizeF size = calculateSize( context );

  bool hasDataDefinedRotation = false;
  QPointF offset;
  double angle = 0;
  calculateOffsetAndRotation( context, size.width(), size.height(), hasDataDefinedRotation, offset, angle );

  QTransform transform;

  // move to the desired position
  transform.translate( point.x() + offset.x(), point.y() + offset.y() );

  if ( !qgsDoubleNear( angle, 0.0 ) )
    transform.rotate( angle );

  double penWidth = mStrokeWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext() );

    if ( !exprVal.isNull() )
    {
      bool ok;
      const double strokeWidth = exprVal.toDouble( &ok );
      if ( ok )
      {
        penWidth = strokeWidth;
      }
    }
  }
  penWidth = context.renderContext().convertToPainterUnits( penWidth, mStrokeWidthUnit, mStrokeWidthMapUnitScale );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext() );
    if ( !exprVal.isNull() && exprVal.toString() == QLatin1String( "no" ) )
    {
      penWidth = 0.0;
    }
  }
  else if ( mStrokeStyle == Qt::NoPen )
    penWidth = 0;

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

bool QgsEllipseSymbolLayer::writeDxf( QgsDxfExport &e, double mmMapUnitScaleFactor, const QString &layerName, QgsSymbolRenderContext &context, QPointF shift ) const
{
  //width
  double symbolWidth = mSymbolWidth;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyWidth ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    symbolWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyWidth, context.renderContext().expressionContext(), mSymbolWidth );
  }
  if ( mSymbolWidthUnit == QgsUnitTypes::RenderMillimeters )
  {
    symbolWidth *= mmMapUnitScaleFactor;
  }

  //height
  double symbolHeight = mSymbolHeight;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyHeight ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    symbolWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyHeight, context.renderContext().expressionContext(), mSymbolHeight );
  }
  if ( mSymbolHeightUnit == QgsUnitTypes::RenderMillimeters )
  {
    symbolHeight *= mmMapUnitScaleFactor;
  }

  //stroke width
  double strokeWidth = mStrokeWidth;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  if ( mStrokeWidthUnit == QgsUnitTypes::RenderMillimeters )
  {
    strokeWidth *= strokeWidth;
  }

  //fill color
  QColor fc = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyFillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyFillColor, context.renderContext().expressionContext(), mColor );
  }

  //stroke color
  QColor oc = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    oc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mStrokeColor );
  }

  //symbol name
  QgsEllipseSymbolLayer::Shape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyName ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    shape = decodeShape( mDataDefinedProperties.valueAsString( QgsSymbolLayer::PropertyName, context.renderContext().expressionContext(), encodeShape( shape ) ) );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyAngle ) )
  {
    context.setOriginalValueVariable( mAngle );
    rotation = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyAngle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
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

  const double halfWidth = symbolWidth / 2.0;
  const double halfHeight = symbolHeight / 2.0;

  switch ( shape )
  {
    case Circle:
    {
      if ( qgsDoubleNear( halfWidth, halfHeight ) )
      {
        const QgsPoint pt( t.map( QPointF( 0, 0 ) ) );
        e.writeFilledCircle( layerName, oc, pt, halfWidth );
      }
      else
      {
        QgsPointSequence line;

        const double stepsize = 2 * M_PI / 40;
        for ( int i = 0; i < 39; ++i )
        {
          const double angle = stepsize * i;
          const double x = halfWidth * std::cos( angle );
          const double y = halfHeight * std::sin( angle );
          line << QgsPoint( t.map( QPointF( x, y ) ) );
        }
        //close ellipse with first point
        line << line.at( 0 );

        if ( mBrush.style() != Qt::NoBrush )
          e.writePolygon( QgsRingSequence() << line, layerName, QStringLiteral( "SOLID" ), fc );
        if ( mPen.style() != Qt::NoPen )
          e.writePolyline( line, layerName, QStringLiteral( "CONTINUOUS" ), oc, strokeWidth );
      }
      return true;
    }

    case Rectangle:
    {
      QgsPointSequence p;
      p << QgsPoint( t.map( QPointF( -halfWidth, -halfHeight ) ) )
        << QgsPoint( t.map( QPointF( halfWidth, -halfHeight ) ) )
        << QgsPoint( t.map( QPointF( halfWidth, halfHeight ) ) )
        << QgsPoint( t.map( QPointF( -halfWidth, halfHeight ) ) );
      p << p[0];

      if ( mBrush.style() != Qt::NoBrush )
        e.writePolygon( QgsRingSequence() << p, layerName, QStringLiteral( "SOLID" ), fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( p, layerName, QStringLiteral( "CONTINUOUS" ), oc, strokeWidth );
      return true;
    }
    case Cross:
    {
      if ( mPen.style() != Qt::NoPen )
      {
        e.writePolyline( QgsPointSequence()
                         << QgsPoint( t.map( QPointF( -halfWidth, 0 ) ) )
                         << QgsPoint( t.map( QPointF( halfWidth, 0 ) ) ),
                         layerName, QStringLiteral( "CONTINUOUS" ), oc, strokeWidth );
        e.writePolyline( QgsPointSequence()
                         << QgsPoint( t.map( QPointF( 0, halfHeight ) ) )
                         << QgsPoint( t.map( QPointF( 0, -halfHeight ) ) ),
                         layerName, QStringLiteral( "CONTINUOUS" ), oc, strokeWidth );
        return true;
      }
      break;
    }

    case Triangle:
    {
      QgsPointSequence p;
      p << QgsPoint( t.map( QPointF( -halfWidth, -halfHeight ) ) )
        << QgsPoint( t.map( QPointF( halfWidth, -halfHeight ) ) )
        << QgsPoint( t.map( QPointF( 0, halfHeight ) ) );
      p << p[0];
      if ( mBrush.style() != Qt::NoBrush )
        e.writePolygon( QgsRingSequence() << p, layerName, QStringLiteral( "SOLID" ), fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( p, layerName, QStringLiteral( "CONTINUOUS" ), oc, strokeWidth );
      return true;
    }

    case Diamond:
    case Arrow:
    case HalfArc:
    case RightHalfTriangle:
    case LeftHalfTriangle:
    case SemiCircle:
      return false;
  }

  return false;
}

QgsEllipseSymbolLayer::Shape QgsEllipseSymbolLayer::decodeShape( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == QLatin1String( "circle" ) )
    return Circle;
  else if ( cleaned == QLatin1String( "square" ) || cleaned == QLatin1String( "rectangle" ) )
    return Rectangle;
  else if ( cleaned == QLatin1String( "diamond" ) )
    return Diamond;
  else if ( cleaned == QLatin1String( "cross" ) )
    return Cross;
  else if ( cleaned == QLatin1String( "arrow" ) )
    return Arrow;
  else if ( cleaned == QLatin1String( "half_arc" ) )
    return HalfArc;
  else if ( cleaned == QLatin1String( "triangle" ) )
    return Triangle;
  else if ( cleaned == QLatin1String( "right_half_triangle" ) )
    return RightHalfTriangle;
  else if ( cleaned == QLatin1String( "left_half_triangle" ) )
    return LeftHalfTriangle;
  else if ( cleaned == QLatin1String( "semi_circle" ) )
    return SemiCircle;

  if ( ok )
    *ok = false;
  return Circle;
}

QString QgsEllipseSymbolLayer::encodeShape( QgsEllipseSymbolLayer::Shape shape )
{
  switch ( shape )
  {
    case Circle:
      return QStringLiteral( "circle" );
    case Rectangle:
      return QStringLiteral( "rectangle" );
    case Diamond:
      return QStringLiteral( "diamond" );
    case Cross:
      return QStringLiteral( "cross" );
    case Arrow:
      return QStringLiteral( "arrow" );
    case HalfArc:
      return QStringLiteral( "half_arc" );
    case Triangle:
      return QStringLiteral( "triangle" );
    case RightHalfTriangle:
      return QStringLiteral( "right_half_triangle" );
    case LeftHalfTriangle:
      return QStringLiteral( "left_half_triangle" );
    case SemiCircle:
      return QStringLiteral( "semi_circle" );
  }
  return QString();
}

QList<QgsEllipseSymbolLayer::Shape> QgsEllipseSymbolLayer::availableShapes()
{
  QList< Shape > shapes;
  shapes << Circle
         << Rectangle
         << Diamond
         << Cross
         << Arrow
         << HalfArc
         << Triangle
         << LeftHalfTriangle
         << RightHalfTriangle
         << SemiCircle;
  return shapes;
}
