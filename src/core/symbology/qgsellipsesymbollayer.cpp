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

#include "qgscolorutils.h"
#include "qgsdxfexport.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsproperty.h"
#include "qgsrendercontext.h"
#include "qgssldexportcontext.h"
#include "qgssymbollayerutils.h"
#include "qgsunittypes.h"

#include <QDomDocument>
#include <QDomElement>
#include <QPainter>
#include <QSet>

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
  if ( properties.contains( u"symbol_name"_s ) )
  {
    layer->setShape( decodeShape( properties[ u"symbol_name"_s].toString() ) );
  }
  if ( properties.contains( u"size"_s ) )
  {
    layer->setSize( properties[u"size"_s].toDouble() );
  }
  if ( properties.contains( u"size_unit"_s ) )
  {
    layer->setSizeUnit( QgsUnitTypes::decodeRenderUnit( properties[u"size_unit"_s].toString() ) );
  }
  if ( properties.contains( u"size_map_unit_scale"_s ) )
  {
    layer->setSizeMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"size_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"symbol_width"_s ) )
  {
    layer->setSymbolWidth( properties[u"symbol_width"_s].toDouble() );
  }
  if ( properties.contains( u"symbol_width_unit"_s ) )
  {
    layer->setSymbolWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[u"symbol_width_unit"_s].toString() ) );
  }
  if ( properties.contains( u"symbol_width_map_unit_scale"_s ) )
  {
    layer->setSymbolWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"symbol_width_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"symbol_height"_s ) )
  {
    layer->setSymbolHeight( properties[u"symbol_height"_s].toDouble() );
  }
  if ( properties.contains( u"symbol_height_unit"_s ) )
  {
    layer->setSymbolHeightUnit( QgsUnitTypes::decodeRenderUnit( properties[u"symbol_height_unit"_s].toString() ) );
  }
  if ( properties.contains( u"symbol_height_map_unit_scale"_s ) )
  {
    layer->setSymbolHeightMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"symbol_height_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"angle"_s ) )
  {
    layer->setAngle( properties[u"angle"_s].toDouble() );
  }
  if ( properties.contains( u"outline_style"_s ) )
  {
    layer->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( properties[u"outline_style"_s].toString() ) );
  }
  else if ( properties.contains( u"line_style"_s ) )
  {
    layer->setStrokeStyle( QgsSymbolLayerUtils::decodePenStyle( properties[u"line_style"_s].toString() ) );
  }
  if ( properties.contains( u"joinstyle"_s ) )
  {
    layer->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( properties[u"joinstyle"_s].toString() ) );
  }
  if ( properties.contains( u"cap_style"_s ) )
  {
    layer->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( properties[u"cap_style"_s].toString() ) );
  }
  if ( properties.contains( u"outline_width"_s ) )
  {
    layer->setStrokeWidth( properties[u"outline_width"_s].toDouble() );
  }
  else if ( properties.contains( u"line_width"_s ) )
  {
    layer->setStrokeWidth( properties[u"line_width"_s].toDouble() );
  }
  if ( properties.contains( u"outline_width_unit"_s ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[u"outline_width_unit"_s].toString() ) );
  }
  else if ( properties.contains( u"line_width_unit"_s ) )
  {
    layer->setStrokeWidthUnit( QgsUnitTypes::decodeRenderUnit( properties[u"line_width_unit"_s].toString() ) );
  }
  if ( properties.contains( u"outline_width_map_unit_scale"_s ) )
  {
    layer->setStrokeWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"outline_width_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"fill_color"_s ) )
  {
    //pre 2.5 projects used "fill_color"
    layer->setFillColor( QgsColorUtils::colorFromString( properties[u"fill_color"_s].toString() ) );
  }
  else if ( properties.contains( u"color"_s ) )
  {
    layer->setFillColor( QgsColorUtils::colorFromString( properties[u"color"_s].toString() ) );
  }
  if ( properties.contains( u"outline_color"_s ) )
  {
    layer->setStrokeColor( QgsColorUtils::colorFromString( properties[u"outline_color"_s].toString() ) );
  }
  else if ( properties.contains( u"line_color"_s ) )
  {
    layer->setStrokeColor( QgsColorUtils::colorFromString( properties[u"line_color"_s].toString() ) );
  }
  if ( properties.contains( u"offset"_s ) )
  {
    layer->setOffset( QgsSymbolLayerUtils::decodePoint( properties[u"offset"_s].toString() ) );
  }
  if ( properties.contains( u"offset_unit"_s ) )
  {
    layer->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( properties[u"offset_unit"_s].toString() ) );
  }
  if ( properties.contains( u"offset_map_unit_scale"_s ) )
  {
    layer->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( properties[u"offset_map_unit_scale"_s].toString() ) );
  }
  if ( properties.contains( u"horizontal_anchor_point"_s ) )
  {
    layer->setHorizontalAnchorPoint( static_cast< Qgis::HorizontalAnchorPoint >( properties[ u"horizontal_anchor_point"_s].toInt() ) );
  }
  if ( properties.contains( u"vertical_anchor_point"_s ) )
  {
    layer->setVerticalAnchorPoint( static_cast< Qgis::VerticalAnchorPoint >( properties[ u"vertical_anchor_point"_s].toInt() ) );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::StrokeStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      mPen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
      mSelPen.setStyle( mPen.style() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::JoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::JoinStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
    {
      mPen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
      mSelPen.setJoinStyle( mPen.joinStyle() );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::CapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    const QString style = mDataDefinedProperties.valueAsString( QgsSymbolLayer::Property::CapStyle, context.renderContext().expressionContext(), QString(), &ok );
    if ( ok )
    {
      mPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
      mSelPen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( style ) );
    }
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::FillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QColor brushColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::FillColor, context.renderContext().expressionContext(), mColor );
    brushColor.setAlphaF( brushColor.alphaF() * context.opacity() );
    mBrush.setColor( brushColor );
  }

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    QColor penColor = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::StrokeColor, context.renderContext().expressionContext(), mStrokeColor );
    penColor.setAlphaF( penColor.alphaF() * context.opacity() );
    mPen.setColor( penColor );
  }

  QgsEllipseSymbolLayer::Shape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Width ) || mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Height ) || mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Name ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::Name, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) )
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

  const bool useSelectedColor = shouldRenderUsingSelectionColor( context );
  if ( shapeIsFilled( shape ) )
  {
    p->setPen( useSelectedColor ? mSelPen : mPen );
    p->setBrush( useSelectedColor ? mSelBrush : mBrush );
  }
  else
  {
    p->setPen( useSelectedColor ? mSelPen : mPen );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Angle ) )
  {
    context.setOriginalValueVariable( angle );
    angle = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Angle, context.renderContext().expressionContext(), 0 ) + mLineAngle;
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
      if ( !g.isNull() && g.type() == Qgis::GeometryType::Point )
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
  return u"EllipseMarker"_s;
}

Qgis::SymbolLayerFlags QgsEllipseSymbolLayer::flags() const
{
  return QgsMarkerSymbolLayer::flags() | Qgis::SymbolLayerFlag::CanCalculateMaskGeometryPerFeature;
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
  m->setSizeUnit( mSizeUnit );
  m->setSizeMapUnitScale( mSizeMapUnitScale );
  m->setOffsetUnit( mOffsetUnit );
  m->setOffsetMapUnitScale( mOffsetMapUnitScale );
  m->setShape( mShape );
  m->setSymbolWidth( mSymbolWidth );
  m->setSymbolHeight( mSymbolHeight );
  m->setStrokeStyle( mStrokeStyle );
  m->setOffset( mOffset );
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
  QgsSldExportContext context;
  context.setExtraProperties( props );
  toSld( doc, element, context );
}

bool QgsEllipseSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  QDomElement symbolizerElem = doc.createElement( u"se:PointSymbolizer"_s );
  const QVariantMap props = context.extraProperties();
  if ( !props.value( u"uom"_s, QString() ).toString().isEmpty() )
    symbolizerElem.setAttribute( u"uom"_s, props.value( u"uom"_s, QString() ).toString() );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( u"geom"_s, QString() ).toString(), context );

  return writeSldMarker( doc, symbolizerElem, context );
}

void QgsEllipseSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  QgsSldExportContext context;
  context.setExtraProperties( props );
  writeSldMarker( doc, element, context );
}

bool QgsEllipseSymbolLayer::writeSldMarker( QDomDocument &doc, QDomElement &element, QgsSldExportContext &context ) const
{
  // <Graphic>
  QDomElement graphicElem = doc.createElement( u"se:Graphic"_s );
  element.appendChild( graphicElem );

  const QVariantMap props = context.extraProperties();
  const double strokeWidth = QgsSymbolLayerUtils::rescaleUom( mStrokeWidth, mStrokeWidthUnit, props );
  const double symbolWidth = QgsSymbolLayerUtils::rescaleUom( mSymbolWidth, mSymbolWidthUnit, props );
  QgsSymbolLayerUtils::wellKnownMarkerToSld( doc, graphicElem, encodeShape( mShape ), mColor, mStrokeColor, mStrokeStyle, context, strokeWidth, symbolWidth );

  // <Rotation>
  const QgsProperty ddRotation = mDataDefinedProperties.property( QgsSymbolLayer::Property::Angle );

  QString angleFunc = props.value( u"angle"_s, QString() ).toString();
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
    angleFunc = u"%1 + %2"_s.arg( angleFunc, ddRotation.asExpression() );
  }
  else if ( !qgsDoubleNear( mAngle, 0.0 ) )
  {
    // both the symbol and the symbol layer have angle value set
    bool ok;
    const double angle = angleFunc.toDouble( &ok );
    if ( !ok )
    {
      // its a string (probably a property name or a function)
      angleFunc = u"%1 + %2"_s.arg( angleFunc ).arg( mAngle );
    }
    else if ( !qgsDoubleNear( angle + mAngle, 0.0 ) )
    {
      // it's a double value
      angleFunc = QString::number( angle + mAngle );
    }
  }
  QgsSymbolLayerUtils::createRotationElement( doc, graphicElem, angleFunc, context );

  // <Displacement>
  const QPointF offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
  QgsSymbolLayerUtils::createDisplacementElement( doc, graphicElem, offset );

  // store w/h factor in a <VendorOption>
  const double widthHeightFactor = mSymbolWidth / mSymbolHeight;
  const QDomElement factorElem = QgsSymbolLayerUtils::createVendorOptionElement( doc, u"widthHeightFactor"_s, QString::number( widthHeightFactor ) );
  graphicElem.appendChild( factorElem );
  return true;
}

QgsSymbolLayer *QgsEllipseSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsgLevel( u"Entered."_s, 4 );

  QDomElement graphicElem = element.firstChildElement( u"Graphic"_s );
  if ( graphicElem.isNull() )
    return nullptr;

  QString name = u"circle"_s;
  QColor fillColor, strokeColor;
  double strokeWidth, size;
  double widthHeightFactor = 1.0;
  Qt::PenStyle strokeStyle;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( graphicElem );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "widthHeightFactor"_L1 )
    {
      bool ok;
      const double v = it.value().toDouble( &ok );
      if ( ok && !qgsDoubleNear( v, 0.0 ) && v > 0 )
        widthHeightFactor = v;
    }
  }

  if ( !QgsSymbolLayerUtils::wellKnownMarkerFromSld( graphicElem, name, fillColor, strokeColor, strokeStyle, strokeWidth, size ) )
    return nullptr;

  double scaleFactor = 1.0;
  const QString uom = element.attribute( u"uom"_s );
  Qgis::RenderUnit sldUnitSize = QgsSymbolLayerUtils::decodeSldUom( uom, &scaleFactor );
  size = size * scaleFactor;
  strokeWidth = strokeWidth * scaleFactor;

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
  m->setOutputUnit( sldUnitSize );
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
  map[u"symbol_name"_s] = encodeShape( mShape );
  map[u"symbol_width"_s] = QString::number( mSymbolWidth );
  map[u"symbol_width_unit"_s] = QgsUnitTypes::encodeUnit( mSymbolWidthUnit );
  map[u"symbol_width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolWidthMapUnitScale );
  map[u"symbol_height"_s] = QString::number( mSymbolHeight );
  map[u"symbol_height_unit"_s] = QgsUnitTypes::encodeUnit( mSymbolHeightUnit );
  map[u"symbol_height_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mSymbolHeightMapUnitScale );
  map[u"angle"_s] = QString::number( mAngle );
  map[u"outline_style"_s] = QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle );
  map[u"outline_width"_s] = QString::number( mStrokeWidth );
  map[u"outline_width_unit"_s] = QgsUnitTypes::encodeUnit( mStrokeWidthUnit );
  map[u"outline_width_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mStrokeWidthMapUnitScale );
  map[u"joinstyle"_s] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[u"cap_style"_s] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[u"color"_s] = QgsColorUtils::colorToString( mColor );
  map[u"outline_color"_s] = QgsColorUtils::colorToString( mStrokeColor );
  map[u"offset"_s] = QgsSymbolLayerUtils::encodePoint( mOffset );
  map[u"offset_unit"_s] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[u"offset_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[u"size"_s] = QString::number( mSize );
  map[u"size_unit"_s] = QgsUnitTypes::encodeUnit( mSizeUnit );
  map[u"size_map_unit_scale"_s] = QgsSymbolLayerUtils::encodeMapUnitScale( mSizeMapUnitScale );
  map[u"horizontal_anchor_point"_s] = QString::number( static_cast< int >( mHorizontalAnchorPoint ) );
  map[u"vertical_anchor_point"_s] = QString::number( static_cast< int >( mVerticalAnchorPoint ) );
  return map;
}

QSizeF QgsEllipseSymbolLayer::calculateSize( QgsSymbolRenderContext &context, double *scaledWidth, double *scaledHeight )
{
  double width = 0;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Width ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Width, context.renderContext().expressionContext(), mSymbolWidth );
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Height ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    height = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Height, context.renderContext().expressionContext(), mSymbolHeight );
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

    case ThirdCircle:
      mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 90, 120 );
      mPainterPath.lineTo( 0, 0 );
      return;

    case QuarterCircle:
      mPainterPath.arcTo( -size.width() / 2.0, -size.height() / 2.0, size.width(), size.height(), 90, 90 );
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

    case Pentagon:
      mPainterPath.moveTo( ( size.width() * -0.9511 ) / 2.0, size.height() / ( 2 / -0.309 ) );
      mPainterPath.lineTo( ( size.width() * -0.5878 ) / 2.0, size.height() / ( 2 / 0.8090 ) );
      mPainterPath.lineTo( ( size.width() * 0.5878 ) / 2.0, size.height() / ( 2 / 0.8090 ) );
      mPainterPath.lineTo( ( size.width() * 0.9511 ) / 2.0, size.height() / ( 2 / -0.309 ) );
      mPainterPath.lineTo( 0, size.height() / -2.0 );
      mPainterPath.lineTo( ( size.width() * -0.9511 ) / 2.0, size.height() / ( 2 / -0.309 ) );
      return;

    case Hexagon:
      mPainterPath.moveTo( ( size.width() * 0.8660 ) / 2.0, size.height() / 4.0 );
      mPainterPath.lineTo( ( size.width() * 0.8660 ) / 2.0, size.height() / -4.0 );
      mPainterPath.lineTo( 0, size.height() / -2.0 );
      mPainterPath.lineTo( ( size.width() * 0.8660 ) / -2.0, size.height() / -4.0 );
      mPainterPath.lineTo( ( size.width() * 0.8660 ) / -2.0, size.height() / 4.0 );
      mPainterPath.lineTo( 0, size.height() / 2.0 );
      mPainterPath.lineTo( ( size.width() * 0.8660 ) / 2.0, size.height() / 4.0 );
      return;

    case Octagon:
    {
      static constexpr double VERTEX_OFFSET_FROM_ORIGIN = 1.0 / ( 1 + M_SQRT2 );
      mPainterPath.moveTo( ( size.width() * VERTEX_OFFSET_FROM_ORIGIN ) / -2.0, size.height() / 2.0 );
      mPainterPath.lineTo( ( size.width() * VERTEX_OFFSET_FROM_ORIGIN ) / 2.0, size.height() / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, ( size.height() * VERTEX_OFFSET_FROM_ORIGIN ) / 2.0 );
      mPainterPath.lineTo( size.width() / 2.0, ( size.height() * VERTEX_OFFSET_FROM_ORIGIN ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * VERTEX_OFFSET_FROM_ORIGIN ) / 2.0, size.height() / -2.0 );
      mPainterPath.lineTo( ( size.width() * VERTEX_OFFSET_FROM_ORIGIN ) / -2.0, size.height() / -2.0 );
      mPainterPath.lineTo( size.width() / -2.0, ( size.height() * VERTEX_OFFSET_FROM_ORIGIN ) / -2.0 );
      mPainterPath.lineTo( size.width() / -2.0, ( size.height() * VERTEX_OFFSET_FROM_ORIGIN ) / 2.0 );
      mPainterPath.lineTo( ( size.width() * VERTEX_OFFSET_FROM_ORIGIN ) / -2.0, size.height() / 2.0 );
      return;
    }

    case Star:
    {
      const double inner_r = std::cos( DEG2RAD( 72.0 ) ) / std::cos( DEG2RAD( 36.0 ) );
      mPainterPath.moveTo( ( size.width() * inner_r * std::sin( DEG2RAD( 324.0 ) ) ) / 2.0, ( size.height() * inner_r * std::cos( DEG2RAD( 324.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * std::sin( DEG2RAD( 288.0 ) ) ) / 2.0, ( size.height() * std::cos( DEG2RAD( 288.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * inner_r * std::sin( DEG2RAD( 252.0 ) ) ) / 2.0, ( size.height() * inner_r * std::cos( DEG2RAD( 252.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * std::sin( DEG2RAD( 216.0 ) ) ) / 2.0, ( size.height() * std::cos( DEG2RAD( 216.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( 0, ( size.height() * inner_r ) / 2.0 );
      mPainterPath.lineTo( ( size.width() * std::sin( DEG2RAD( 144.0 ) ) ) / 2.0, ( size.height() * std::cos( DEG2RAD( 144.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * inner_r * std::sin( DEG2RAD( 108.0 ) ) ) / 2.0, ( size.height() * inner_r * std::cos( DEG2RAD( 108.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * std::sin( DEG2RAD( 72.0 ) ) ) / 2.0, ( size.height() * std::cos( DEG2RAD( 72.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( ( size.width() * inner_r * std::sin( DEG2RAD( 36.0 ) ) ) / 2.0, ( size.height() * inner_r * std::cos( DEG2RAD( 36.0 ) ) ) / -2.0 );
      mPainterPath.lineTo( 0, size.height() / -2.0 );
      mPainterPath.lineTo( ( size.width() * inner_r * std::sin( DEG2RAD( 324.0 ) ) ) / 2.0, ( size.height() * inner_r * std::cos( DEG2RAD( 324.0 ) ) ) / -2.0 );
      return;
    }
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
    case ThirdCircle:
    case QuarterCircle:
    case Pentagon:
    case Hexagon:
    case Octagon:
    case Star:
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

void QgsEllipseSymbolLayer::setOutputUnit( Qgis::RenderUnit unit )
{
  QgsMarkerSymbolLayer::setOutputUnit( unit );
  mSymbolWidthUnit = unit;
  mSymbolHeightUnit = unit;
  mStrokeWidthUnit = unit;
}

Qgis::RenderUnit QgsEllipseSymbolLayer::outputUnit() const
{
  const Qgis::RenderUnit unit = QgsMarkerSymbolLayer::outputUnit();
  if ( mSymbolWidthUnit != unit || mSymbolHeightUnit != unit || mStrokeWidthUnit != unit )
  {
    return Qgis::RenderUnit::Unknown;
  }
  return unit;
}

bool QgsEllipseSymbolLayer::usesMapUnits() const
{
  return mSymbolWidthUnit == Qgis::RenderUnit::MapUnits || mSymbolWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mSymbolHeightUnit == Qgis::RenderUnit::MapUnits || mSymbolHeightUnit == Qgis::RenderUnit::MetersInMapUnits
         || mStrokeWidthUnit == Qgis::RenderUnit::MapUnits || mStrokeWidthUnit == Qgis::RenderUnit::MetersInMapUnits
         || mOffsetUnit == Qgis::RenderUnit::MapUnits || mOffsetUnit == Qgis::RenderUnit::MetersInMapUnits;
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
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext() );

    if ( !QgsVariantUtils::isNull( exprVal ) )
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

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mStrokeStyle ) );
    const QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::Property::StrokeStyle, context.renderContext().expressionContext() );
    if ( !QgsVariantUtils::isNull( exprVal ) && exprVal.toString() == "no"_L1 )
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

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Width ) ) //1. priority: data defined setting on symbol layer le
  {
    context.setOriginalValueVariable( mSymbolWidth );
    symbolWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Width, context.renderContext().expressionContext(), mSymbolWidth );
  }
  if ( mSymbolWidthUnit == Qgis::RenderUnit::Millimeters )
  {
    symbolWidth *= mmMapUnitScaleFactor;
  }

  //height
  double symbolHeight = mSymbolHeight;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Height ) ) //1. priority: data defined setting on symbol layer level
  {
    context.setOriginalValueVariable( mSymbolHeight );
    symbolWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Height, context.renderContext().expressionContext(), mSymbolHeight );
  }
  if ( mSymbolHeightUnit == Qgis::RenderUnit::Millimeters )
  {
    symbolHeight *= mmMapUnitScaleFactor;
  }

  //stroke width
  double strokeWidth = mStrokeWidth;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeWidth ) )
  {
    context.setOriginalValueVariable( mStrokeWidth );
    strokeWidth = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::StrokeWidth, context.renderContext().expressionContext(), mStrokeWidth );
  }
  if ( mStrokeWidthUnit == Qgis::RenderUnit::Millimeters )
  {
    strokeWidth *= strokeWidth;
  }

  //fill color
  QColor fc = mColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::FillColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    fc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::FillColor, context.renderContext().expressionContext(), mColor );
  }

  //stroke color
  QColor oc = mStrokeColor;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::StrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mStrokeColor ) );
    oc = mDataDefinedProperties.valueAsColor( QgsSymbolLayer::Property::StrokeColor, context.renderContext().expressionContext(), mStrokeColor );
  }

  //symbol name
  QgsEllipseSymbolLayer::Shape shape = mShape;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Name ) )
  {
    context.setOriginalValueVariable( encodeShape( shape ) );
    shape = decodeShape( mDataDefinedProperties.valueAsString( QgsSymbolLayer::Property::Name, context.renderContext().expressionContext(), encodeShape( shape ) ) );
  }

  //offset
  double offsetX = 0;
  double offsetY = 0;
  markerOffset( context, offsetX, offsetY );
  QPointF off( offsetX, offsetY );

  //priority for rotation: 1. data defined symbol level, 2. symbol layer rotation (mAngle)
  double rotation = 0.0;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::Property::Angle ) )
  {
    context.setOriginalValueVariable( mAngle );
    rotation = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::Property::Angle, context.renderContext().expressionContext(), mAngle ) + mLineAngle;
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
          e.writePolygon( QgsRingSequence() << line, layerName, u"SOLID"_s, fc );
        if ( mPen.style() != Qt::NoPen )
          e.writePolyline( line, layerName, u"CONTINUOUS"_s, oc, strokeWidth );
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
        e.writePolygon( QgsRingSequence() << p, layerName, u"SOLID"_s, fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( p, layerName, u"CONTINUOUS"_s, oc, strokeWidth );
      return true;
    }
    case Cross:
    {
      if ( mPen.style() != Qt::NoPen )
      {
        e.writePolyline( QgsPointSequence()
                         << QgsPoint( t.map( QPointF( -halfWidth, 0 ) ) )
                         << QgsPoint( t.map( QPointF( halfWidth, 0 ) ) ),
                         layerName, u"CONTINUOUS"_s, oc, strokeWidth );
        e.writePolyline( QgsPointSequence()
                         << QgsPoint( t.map( QPointF( 0, halfHeight ) ) )
                         << QgsPoint( t.map( QPointF( 0, -halfHeight ) ) ),
                         layerName, u"CONTINUOUS"_s, oc, strokeWidth );
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
        e.writePolygon( QgsRingSequence() << p, layerName, u"SOLID"_s, fc );
      if ( mPen.style() != Qt::NoPen )
        e.writePolyline( p, layerName, u"CONTINUOUS"_s, oc, strokeWidth );
      return true;
    }

    case Diamond:
    case Arrow:
    case HalfArc:
    case RightHalfTriangle:
    case LeftHalfTriangle:
    case SemiCircle:
    case ThirdCircle:
    case QuarterCircle:
    case Pentagon:
    case Hexagon:
    case Octagon:
    case Star:
      return false;
  }

  return false;
}

QgsEllipseSymbolLayer::Shape QgsEllipseSymbolLayer::decodeShape( const QString &name, bool *ok )
{
  if ( ok )
    *ok = true;
  const QString cleaned = name.toLower().trimmed();

  if ( cleaned == "circle"_L1 )
    return Circle;
  else if ( cleaned == "square"_L1 || cleaned == "rectangle"_L1 )
    return Rectangle;
  else if ( cleaned == "diamond"_L1 )
    return Diamond;
  else if ( cleaned == "cross"_L1 )
    return Cross;
  else if ( cleaned == "arrow"_L1 )
    return Arrow;
  else if ( cleaned == "half_arc"_L1 )
    return HalfArc;
  else if ( cleaned == "triangle"_L1 )
    return Triangle;
  else if ( cleaned == "right_half_triangle"_L1 )
    return RightHalfTriangle;
  else if ( cleaned == "left_half_triangle"_L1 )
    return LeftHalfTriangle;
  else if ( cleaned == "semi_circle"_L1 )
    return SemiCircle;
  else if ( cleaned == "third_circle"_L1 )
    return ThirdCircle;
  else if ( cleaned == "quarter_circle"_L1 )
    return QuarterCircle;
  else if ( cleaned == "pentagon"_L1 )
    return Pentagon;
  else if ( cleaned == "hexagon"_L1 )
    return Hexagon;
  else if ( cleaned == "octagon"_L1 )
    return Octagon;
  else if ( cleaned == "star"_L1 )
    return Star;  if ( ok )
    *ok = false;
  return Circle;
}

QString QgsEllipseSymbolLayer::encodeShape( QgsEllipseSymbolLayer::Shape shape )
{
  switch ( shape )
  {
    case Circle:
      return u"circle"_s;
    case Rectangle:
      return u"rectangle"_s;
    case Diamond:
      return u"diamond"_s;
    case Cross:
      return u"cross"_s;
    case Arrow:
      return u"arrow"_s;
    case HalfArc:
      return u"half_arc"_s;
    case Triangle:
      return u"triangle"_s;
    case RightHalfTriangle:
      return u"right_half_triangle"_s;
    case LeftHalfTriangle:
      return u"left_half_triangle"_s;
    case SemiCircle:
      return u"semi_circle"_s;
    case ThirdCircle:
      return u"third_circle"_s;
    case QuarterCircle:
      return u"quarter_circle"_s;
    case Pentagon:
      return u"pentagon"_s;
    case Hexagon:
      return u"hexagon"_s;
    case Octagon:
      return u"octagon"_s;
    case Star:
      return u"star"_s;
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
         << SemiCircle
         << ThirdCircle
         << QuarterCircle
         << Pentagon
         << Hexagon
         << Octagon
         << Star;
  return shapes;
}
