/***************************************************************************
 qgslinesymbollayer.cpp
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

#include "qgslinesymbollayer.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsdxfexport.h"
#include "qgssymbollayerutils.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrysimplifier.h"
#include "qgsunittypes.h"
#include "qgsproperty.h"

#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

QgsSimpleLineSymbolLayer::QgsSimpleLineSymbolLayer( const QColor &color, double width, Qt::PenStyle penStyle )
  : mPenStyle( penStyle )
{
  mColor = color;
  mWidth = width;
  mCustomDashVector << 5 << 2;
}

void QgsSimpleLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
  mCustomDashPatternUnit = unit;
}

QgsUnitTypes::RenderUnit  QgsSimpleLineSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit || mCustomDashPatternUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsSimpleLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  mWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
  mCustomDashPatternMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleLineSymbolLayer::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == mWidthMapUnitScale &&
       mWidthMapUnitScale == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == mCustomDashPatternMapUnitScale )
  {
    return mWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayer *QgsSimpleLineSymbolLayer::create( const QgsStringMap &props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( QStringLiteral( "line_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "line_color" )] );
  }
  else if ( props.contains( QStringLiteral( "outline_color" ) ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "outline_color" )] );
  }
  else if ( props.contains( QStringLiteral( "color" ) ) )
  {
    //pre 2.5 projects used "color"
    color = QgsSymbolLayerUtils::decodeColor( props[QStringLiteral( "color" )] );
  }
  if ( props.contains( QStringLiteral( "line_width" ) ) )
  {
    width = props[QStringLiteral( "line_width" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "outline_width" ) ) )
  {
    width = props[QStringLiteral( "outline_width" )].toDouble();
  }
  else if ( props.contains( QStringLiteral( "width" ) ) )
  {
    //pre 2.5 projects used "width"
    width = props[QStringLiteral( "width" )].toDouble();
  }
  if ( props.contains( QStringLiteral( "line_style" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "line_style" )] );
  }
  else if ( props.contains( QStringLiteral( "outline_style" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "outline_style" )] );
  }
  else if ( props.contains( QStringLiteral( "penstyle" ) ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props[QStringLiteral( "penstyle" )] );
  }

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  if ( props.contains( QStringLiteral( "line_width_unit" ) ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "line_width_unit" )] ) );
  }
  else if ( props.contains( QStringLiteral( "outline_width_unit" ) ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "outline_width_unit" )] ) );
  }
  else if ( props.contains( QStringLiteral( "width_unit" ) ) )
  {
    //pre 2.5 projects used "width_unit"
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "width_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "width_map_unit_scale" ) ) )
    l->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "width_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "offset" ) ) )
    l->setOffset( props[QStringLiteral( "offset" )].toDouble() );
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  if ( props.contains( QStringLiteral( "joinstyle" ) ) )
    l->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props[QStringLiteral( "joinstyle" )] ) );
  if ( props.contains( QStringLiteral( "capstyle" ) ) )
    l->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props[QStringLiteral( "capstyle" )] ) );

  if ( props.contains( QStringLiteral( "use_custom_dash" ) ) )
  {
    l->setUseCustomDashPattern( props[QStringLiteral( "use_custom_dash" )].toInt() );
  }
  if ( props.contains( QStringLiteral( "customdash" ) ) )
  {
    l->setCustomDashVector( QgsSymbolLayerUtils::decodeRealVector( props[QStringLiteral( "customdash" )] ) );
  }
  if ( props.contains( QStringLiteral( "customdash_unit" ) ) )
  {
    l->setCustomDashPatternUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "customdash_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "customdash_map_unit_scale" ) ) )
  {
    l->setCustomDashPatternMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "customdash_map_unit_scale" )] ) );
  }

  if ( props.contains( QStringLiteral( "draw_inside_polygon" ) ) )
  {
    l->setDrawInsidePolygon( props[QStringLiteral( "draw_inside_polygon" )].toInt() );
  }

  if ( props.contains( QStringLiteral( "ring_filter" ) ) )
  {
    l->setRingFilter( static_cast< RenderRingFilter>( props[QStringLiteral( "ring_filter" )].toInt() ) );
  }

  l->restoreOldDataDefinedProperties( props );

  return l;
}


QString QgsSimpleLineSymbolLayer::layerType() const
{
  return QStringLiteral( "SimpleLine" );
}

void QgsSimpleLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.opacity() );
  mPen.setColor( penColor );
  double scaledWidth = context.renderContext().convertToPainterUnits( mWidth, mWidthUnit, mWidthMapUnitScale );
  mPen.setWidthF( scaledWidth );
  if ( mUseCustomDashPattern && !qgsDoubleNear( scaledWidth, 0 ) )
  {
    mPen.setStyle( Qt::CustomDashLine );

    //scale pattern vector
    double dashWidthDiv = scaledWidth;
    //fix dash pattern width in Qt 4.8
    QStringList versionSplit = QString( qVersion() ).split( '.' );
    if ( versionSplit.size() > 1
         && versionSplit.at( 1 ).toInt() >= 8
         && scaledWidth < 1.0 )
    {
      dashWidthDiv = 1.0;
    }
    QVector<qreal> scaledVector;
    QVector<qreal>::const_iterator it = mCustomDashVector.constBegin();
    for ( ; it != mCustomDashVector.constEnd(); ++it )
    {
      //the dash is specified in terms of pen widths, therefore the division
      scaledVector << context.renderContext().convertToPainterUnits( ( *it ), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv;
    }
    mPen.setDashPattern( scaledVector );
  }
  else
  {
    mPen.setStyle( mPenStyle );
  }
  mPen.setJoinStyle( mPenJoinStyle );
  mPen.setCapStyle( mPenCapStyle );

  mSelPen = mPen;
  QColor selColor = context.renderContext().selectionColor();
  if ( ! SELECTION_IS_OPAQUE )
    selColor.setAlphaF( context.opacity() );
  mSelPen.setColor( selColor );
}

void QgsSimpleLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  Q_UNUSED( context );
}

void QgsSimpleLineSymbolLayer::renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  if ( mDrawInsidePolygon )
    p->save();

  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
    {
      if ( mDrawInsidePolygon )
      {
        //only drawing the line on the interior of the polygon, so set clip path for painter
        QPainterPath clipPath;
        clipPath.addPolygon( points );

        if ( rings )
        {
          //add polygon rings
          QList<QPolygonF>::const_iterator it = rings->constBegin();
          for ( ; it != rings->constEnd(); ++it )
          {
            QPolygonF ring = *it;
            clipPath.addPolygon( ring );
          }
        }

        //use intersect mode, as a clip path may already exist (e.g., for composer maps)
        p->setClipPath( clipPath, Qt::IntersectClip );
      }

      renderPolyline( points, context );
    }
    break;

    case InteriorRingsOnly:
      break;
  }

  if ( rings )
  {
    switch ( mRingFilter )
    {
      case AllRings:
      case InteriorRingsOnly:
      {
        mOffset = -mOffset; // invert the offset for rings!
        for ( const QPolygonF &ring : qgis::as_const( *rings ) )
          renderPolyline( ring, context );
        mOffset = -mOffset;
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }

  if ( mDrawInsidePolygon )
  {
    //restore painter to reset clip path
    p->restore();
  }

}

void QgsSimpleLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  QPainter *p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  double offset = mOffset;
  applyDataDefinedSymbology( context, mPen, mSelPen, offset );

  p->setPen( context.selected() ? mSelPen : mPen );
  p->setBrush( Qt::NoBrush );

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #2 points).
  if ( points.size() <= 2 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::AntialiasingSimplification ) &&
       QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( points, context.renderContext().vectorSimplifyMethod().threshold() ) &&
       ( p->renderHints() & QPainter::Antialiasing ) )
  {
    p->setRenderHint( QPainter::Antialiasing, false );
#if 0
    p->drawPolyline( points );
#else
    QPainterPath path;
    path.addPolygon( points );
    p->drawPath( path );
#endif
    p->setRenderHint( QPainter::Antialiasing, true );
    return;
  }

  if ( qgsDoubleNear( offset, 0 ) )
  {
#if 0
    p->drawPolyline( points );
#else
    QPainterPath path;
    path.addPolygon( points );
    p->drawPath( path );
#endif
  }
  else
  {
    double scaledOffset = context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale );
    QList<QPolygonF> mline = ::offsetLine( points, scaledOffset, context.originalGeometryType() != QgsWkbTypes::UnknownGeometry ? context.originalGeometryType() : QgsWkbTypes::LineGeometry );
    for ( int part = 0; part < mline.count(); ++part )
    {
#if 0
      p->drawPolyline( mline );
#else
      QPainterPath path;
      path.addPolygon( mline[ part ] );
      p->drawPath( path );
#endif
    }
  }
}

QgsStringMap QgsSimpleLineSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "line_color" )] = QgsSymbolLayerUtils::encodeColor( mColor );
  map[QStringLiteral( "line_width" )] = QString::number( mWidth );
  map[QStringLiteral( "line_width_unit" )] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map[QStringLiteral( "width_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  map[QStringLiteral( "line_style" )] = QgsSymbolLayerUtils::encodePenStyle( mPenStyle );
  map[QStringLiteral( "joinstyle" )] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map[QStringLiteral( "capstyle" )] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "use_custom_dash" )] = ( mUseCustomDashPattern ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "customdash" )] = QgsSymbolLayerUtils::encodeRealVector( mCustomDashVector );
  map[QStringLiteral( "customdash_unit" )] = QgsUnitTypes::encodeUnit( mCustomDashPatternUnit );
  map[QStringLiteral( "customdash_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mCustomDashPatternMapUnitScale );
  map[QStringLiteral( "draw_inside_polygon" )] = ( mDrawInsidePolygon ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "ring_filter" )] = QString::number( static_cast< int >( mRingFilter ) );
  return map;
}

QgsSimpleLineSymbolLayer *QgsSimpleLineSymbolLayer::clone() const
{
  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( mColor, mWidth, mPenStyle );
  l->setWidthUnit( mWidthUnit );
  l->setWidthMapUnitScale( mWidthMapUnitScale );
  l->setOffsetUnit( mOffsetUnit );
  l->setOffsetMapUnitScale( mOffsetMapUnitScale );
  l->setCustomDashPatternUnit( mCustomDashPatternUnit );
  l->setCustomDashPatternMapUnitScale( mCustomDashPatternMapUnitScale );
  l->setOffset( mOffset );
  l->setPenJoinStyle( mPenJoinStyle );
  l->setPenCapStyle( mPenCapStyle );
  l->setUseCustomDashPattern( mUseCustomDashPattern );
  l->setCustomDashVector( mCustomDashVector );
  l->setDrawInsidePolygon( mDrawInsidePolygon );
  l->setRingFilter( mRingFilter );
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

void QgsSimpleLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  if ( mPenStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:LineSymbolizer" ) );
  if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
    symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
  symbolizerElem.appendChild( strokeElem );

  Qt::PenStyle penStyle = mUseCustomDashPattern ? Qt::CustomDashLine : mPenStyle;
  double width = QgsSymbolLayerUtils::rescaleUom( mWidth, mWidthUnit, props );
  QVector<qreal> customDashVector = QgsSymbolLayerUtils::rescaleUom( mCustomDashVector, mCustomDashPatternUnit, props );
  QgsSymbolLayerUtils::lineToSld( doc, strokeElem, penStyle, mColor, width,
                                  &mPenJoinStyle, &mPenCapStyle, &customDashVector );

  // <se:PerpendicularOffset>
  if ( !qgsDoubleNear( mOffset, 0.0 ) )
  {
    QDomElement perpOffsetElem = doc.createElement( QStringLiteral( "se:PerpendicularOffset" ) );
    double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
    perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
    symbolizerElem.appendChild( perpOffsetElem );
  }
}

QString QgsSimpleLineSymbolLayer::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  if ( mUseCustomDashPattern )
  {
    return QgsSymbolLayerUtils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor,
           mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset, &mCustomDashVector );
  }
  else
  {
    return QgsSymbolLayerUtils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor, mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset );
  }
}

QgsSymbolLayer *QgsSimpleLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
    return nullptr;

  Qt::PenStyle penStyle;
  QColor color;
  double width;
  Qt::PenJoinStyle penJoinStyle;
  Qt::PenCapStyle penCapStyle;
  QVector<qreal> customDashVector;

  if ( !QgsSymbolLayerUtils::lineFromSld( strokeElem, penStyle,
                                          color, width,
                                          &penJoinStyle, &penCapStyle,
                                          &customDashVector ) )
    return nullptr;

  double offset = 0.0;
  QDomElement perpOffsetElem = element.firstChildElement( QStringLiteral( "PerpendicularOffset" ) );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  width = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, width );
  offset = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset );

  QgsSimpleLineSymbolLayer *l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  l->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  l->setOffset( offset );
  l->setPenJoinStyle( penJoinStyle );
  l->setPenCapStyle( penCapStyle );
  l->setUseCustomDashPattern( penStyle == Qt::CustomDashLine );
  l->setCustomDashVector( customDashVector );
  return l;
}

void QgsSimpleLineSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext &context, QPen &pen, QPen &selPen, double &offset )
{
  if ( !dataDefinedProperties().hasActiveProperties() )
    return; // shortcut

  //data defined properties
  bool hasStrokeWidthExpression = false;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    double scaledWidth = context.renderContext().convertToPainterUnits(
                           mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mWidth ),
                           mWidthUnit, mWidthMapUnitScale );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
    hasStrokeWidthExpression = true;
  }

  //color
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    pen.setColor( mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor ) );
  }

  //offset
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), offset );
  }

  //dash dot vector
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCustomDash ) )
  {
    double scaledWidth = context.renderContext().convertToPainterUnits( mWidth, mWidthUnit, mWidthMapUnitScale );
    double dashWidthDiv = mPen.widthF();

    if ( hasStrokeWidthExpression )
    {
      dashWidthDiv = pen.widthF();
      scaledWidth = pen.widthF();
    }

    //fix dash pattern width in Qt 4.8
    QStringList versionSplit = QString( qVersion() ).split( '.' );
    if ( versionSplit.size() > 1
         && versionSplit.at( 1 ).toInt() >= 8
         && scaledWidth < 1.0 )
    {
      dashWidthDiv = 1.0;
    }

    QVector<qreal> dashVector;
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyCustomDash, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      QStringList dashList = exprVal.toString().split( ';' );
      QStringList::const_iterator dashIt = dashList.constBegin();
      for ( ; dashIt != dashList.constEnd(); ++dashIt )
      {
        dashVector.push_back( context.renderContext().convertToPainterUnits( dashIt->toDouble(), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv );
      }
      pen.setDashPattern( dashVector );
    }
  }

  //line style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mPenStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyStrokeStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
      pen.setStyle( QgsSymbolLayerUtils::decodePenStyle( exprVal.toString() ) );
  }

  //join style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyJoinStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyJoinStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
      pen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( exprVal.toString() ) );
  }

  //cap style
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyCapStyle ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyCapStyle, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
      pen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( exprVal.toString() ) );
  }
}

double QgsSimpleLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  if ( mDrawInsidePolygon )
  {
    //set to clip line to the interior of polygon, so we expect no bleed
    return 0;
  }
  else
  {
    return context.convertToPainterUnits( ( mWidth / 2.0 ), mWidthUnit, mWidthMapUnitScale ) +
           context.convertToPainterUnits( std::fabs( mOffset ), mOffsetUnit, mOffsetMapUnitScale );
  }
}

QVector<qreal> QgsSimpleLineSymbolLayer::dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const
{
  unit = mCustomDashPatternUnit;
  return mUseCustomDashPattern ? mCustomDashVector : QVector<qreal>();
}

Qt::PenStyle QgsSimpleLineSymbolLayer::dxfPenStyle() const
{
  return mPenStyle;
}

double QgsSimpleLineSymbolLayer::dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  double width = mWidth;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeWidth ) )
  {
    context.setOriginalValueVariable( mWidth );
    width = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyStrokeWidth, context.renderContext().expressionContext(), mWidth );
  }

  width *= e.mapUnitScaleFactor( e.symbologyScale(), widthUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mWidthUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( width, mWidthMapUnitScale, context.renderContext().scaleFactor() );
  }
  return width;
}

QColor QgsSimpleLineSymbolLayer::dxfColor( QgsSymbolRenderContext &context ) const
{
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyStrokeColor ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    return mDataDefinedProperties.valueAsColor( QgsSymbolLayer::PropertyStrokeColor, context.renderContext().expressionContext(), mColor );
  }
  return mColor;
}

double QgsSimpleLineSymbolLayer::dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e );
  double offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), mOffset );
  }

  offset *= e.mapUnitScaleFactor( e.symbologyScale(), offsetUnit(), e.mapUnits(), context.renderContext().mapToPixel().mapUnitsPerPixel() );
  if ( mOffsetUnit == QgsUnitTypes::RenderMapUnits )
  {
    e.clipValueToMapUnitScale( offset, mOffsetMapUnitScale, context.renderContext().scaleFactor() );
  }
  return -offset; //direction seems to be inverse to symbology offset
}

/////////

///@cond PRIVATE

class MyLine
{
  public:
    MyLine( QPointF p1, QPointF p2 )
      : mVertical( false )
      , mIncreasing( false )
      , mT( 0.0 )
      , mLength( 0.0 )
    {
      if ( p1 == p2 )
        return; // invalid

      // tangent and direction
      if ( qgsDoubleNear( p1.x(), p2.x() ) )
      {
        // vertical line - tangent undefined
        mVertical = true;
        mIncreasing = ( p2.y() > p1.y() );
      }
      else
      {
        mVertical = false;
        mT = float( p2.y() - p1.y() ) / ( p2.x() - p1.x() );
        mIncreasing = ( p2.x() > p1.x() );
      }

      // length
      double x = ( p2.x() - p1.x() );
      double y = ( p2.y() - p1.y() );
      mLength = std::sqrt( x * x + y * y );
    }

    // return angle in radians
    double angle()
    {
      double a = ( mVertical ? M_PI_2 : std::atan( mT ) );

      if ( !mIncreasing )
        a += M_PI;
      return a;
    }

    // return difference for x,y when going along the line with specified interval
    QPointF diffForInterval( double interval )
    {
      if ( mVertical )
        return ( mIncreasing ? QPointF( 0, interval ) : QPointF( 0, -interval ) );

      double alpha = std::atan( mT );
      double dx = std::cos( alpha ) * interval;
      double dy = std::sin( alpha ) * interval;
      return ( mIncreasing ? QPointF( dx, dy ) : QPointF( -dx, -dy ) );
    }

    double length() { return mLength; }

  protected:
    bool mVertical;
    bool mIncreasing;
    double mT;
    double mLength;
};

///@endcond

QgsMarkerLineSymbolLayer::QgsMarkerLineSymbolLayer( bool rotateMarker, double interval )
{
  mRotateMarker = rotateMarker;
  mInterval = interval;
  mIntervalUnit = QgsUnitTypes::RenderMillimeters;
  mMarker = nullptr;
  mPlacement = Interval;
  mOffsetAlongLine = 0;
  mOffsetAlongLineUnit = QgsUnitTypes::RenderMillimeters;

  setSubSymbol( new QgsMarkerSymbol() );
}

QgsSymbolLayer *QgsMarkerLineSymbolLayer::create( const QgsStringMap &props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;


  if ( props.contains( QStringLiteral( "interval" ) ) )
    interval = props[QStringLiteral( "interval" )].toDouble();
  if ( props.contains( QStringLiteral( "rotate" ) ) )
    rotate = ( props[QStringLiteral( "rotate" )] == QLatin1String( "1" ) );

  QgsMarkerLineSymbolLayer *x = new QgsMarkerLineSymbolLayer( rotate, interval );
  if ( props.contains( QStringLiteral( "offset" ) ) )
  {
    x->setOffset( props[QStringLiteral( "offset" )].toDouble() );
  }
  if ( props.contains( QStringLiteral( "offset_unit" ) ) )
  {
    x->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "interval_unit" ) ) )
  {
    x->setIntervalUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "interval_unit" )] ) );
  }
  if ( props.contains( QStringLiteral( "offset_along_line" ) ) )
  {
    x->setOffsetAlongLine( props[QStringLiteral( "offset_along_line" )].toDouble() );
  }
  if ( props.contains( QStringLiteral( "offset_along_line_unit" ) ) )
  {
    x->setOffsetAlongLineUnit( QgsUnitTypes::decodeRenderUnit( props[QStringLiteral( "offset_along_line_unit" )] ) );
  }
  if ( props.contains( ( QStringLiteral( "offset_along_line_map_unit_scale" ) ) ) )
  {
    x->setOffsetAlongLineMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_along_line_map_unit_scale" )] ) );
  }

  if ( props.contains( QStringLiteral( "offset_map_unit_scale" ) ) )
  {
    x->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "offset_map_unit_scale" )] ) );
  }
  if ( props.contains( QStringLiteral( "interval_map_unit_scale" ) ) )
  {
    x->setIntervalMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props[QStringLiteral( "interval_map_unit_scale" )] ) );
  }

  if ( props.contains( QStringLiteral( "placement" ) ) )
  {
    if ( props[QStringLiteral( "placement" )] == QLatin1String( "vertex" ) )
      x->setPlacement( Vertex );
    else if ( props[QStringLiteral( "placement" )] == QLatin1String( "lastvertex" ) )
      x->setPlacement( LastVertex );
    else if ( props[QStringLiteral( "placement" )] == QLatin1String( "firstvertex" ) )
      x->setPlacement( FirstVertex );
    else if ( props[QStringLiteral( "placement" )] == QLatin1String( "centralpoint" ) )
      x->setPlacement( CentralPoint );
    else if ( props[QStringLiteral( "placement" )] == QLatin1String( "curvepoint" ) )
      x->setPlacement( CurvePoint );
    else
      x->setPlacement( Interval );
  }

  if ( props.contains( QStringLiteral( "ring_filter" ) ) )
  {
    x->setRingFilter( static_cast< RenderRingFilter>( props[QStringLiteral( "ring_filter" )].toInt() ) );
  }

  x->restoreOldDataDefinedProperties( props );

  return x;
}

QString QgsMarkerLineSymbolLayer::layerType() const
{
  return QStringLiteral( "MarkerLine" );
}

void QgsMarkerLineSymbolLayer::setColor( const QColor &color )
{
  mMarker->setColor( color );
  mColor = color;
}

QColor QgsMarkerLineSymbolLayer::color() const
{
  return mMarker ? mMarker->color() : mColor;
}

void QgsMarkerLineSymbolLayer::startRender( QgsSymbolRenderContext &context )
{
  mMarker->setOpacity( context.opacity() );

  // if being rotated, it gets initialized with every line segment
  QgsSymbol::RenderHints hints = nullptr;
  if ( mRotateMarker )
    hints |= QgsSymbol::DynamicRotation;
  mMarker->setRenderHints( hints );

  mMarker->startRender( context.renderContext(), context.fields() );
}

void QgsMarkerLineSymbolLayer::stopRender( QgsSymbolRenderContext &context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsMarkerLineSymbolLayer::renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  double offset = mOffset;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffset ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffset, context.renderContext().expressionContext(), mOffset );
  }

  Placement placement = mPlacement;

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyPlacement ) )
  {
    QVariant exprVal = mDataDefinedProperties.value( QgsSymbolLayer::PropertyPlacement, context.renderContext().expressionContext() );
    if ( exprVal.isValid() )
    {
      QString placementString = exprVal.toString();
      if ( placementString.compare( QLatin1String( "interval" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = Interval;
      }
      else if ( placementString.compare( QLatin1String( "vertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = Vertex;
      }
      else if ( placementString.compare( QLatin1String( "lastvertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = LastVertex;
      }
      else if ( placementString.compare( QLatin1String( "firstvertex" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = FirstVertex;
      }
      else if ( placementString.compare( QLatin1String( "centerpoint" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = CentralPoint;
      }
      else if ( placementString.compare( QLatin1String( "curvepoint" ), Qt::CaseInsensitive ) == 0 )
      {
        placement = CurvePoint;
      }
      else
      {
        placement = Interval;
      }
    }
  }


  context.renderContext().painter()->save();

  if ( qgsDoubleNear( offset, 0.0 ) )
  {
    if ( placement == Interval )
      renderPolylineInterval( points, context );
    else if ( placement == CentralPoint )
      renderPolylineCentral( points, context );
    else
      renderPolylineVertex( points, context, placement );
  }
  else
  {
    context.renderContext().setGeometry( nullptr ); //always use segmented geometry with offset
    QList<QPolygonF> mline = ::offsetLine( points, context.renderContext().convertToPainterUnits( offset, mOffsetUnit, mOffsetMapUnitScale ), context.originalGeometryType() != QgsWkbTypes::UnknownGeometry ? context.originalGeometryType() : QgsWkbTypes::LineGeometry );

    for ( int part = 0; part < mline.count(); ++part )
    {
      const QPolygonF &points2 = mline[ part ];

      if ( placement == Interval )
        renderPolylineInterval( points2, context );
      else if ( placement == CentralPoint )
        renderPolylineCentral( points2, context );
      else
        renderPolylineVertex( points2, context, placement );
    }
  }

  context.renderContext().painter()->restore();
}

void QgsMarkerLineSymbolLayer::renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context )
{
  const QgsCurvePolygon *curvePolygon = dynamic_cast<const QgsCurvePolygon *>( context.renderContext().geometry() );

  if ( curvePolygon )
  {
    context.renderContext().setGeometry( curvePolygon->exteriorRing() );
  }

  switch ( mRingFilter )
  {
    case AllRings:
    case ExteriorRingOnly:
      renderPolyline( points, context );
      break;
    case InteriorRingsOnly:
      break;
  }

  if ( rings )
  {
    switch ( mRingFilter )
    {
      case AllRings:
      case InteriorRingsOnly:
      {
        mOffset = -mOffset; // invert the offset for rings!
        for ( int i = 0; i < rings->size(); ++i )
        {
          if ( curvePolygon )
          {
            context.renderContext().setGeometry( curvePolygon->interiorRing( i ) );
          }
          renderPolyline( rings->at( i ), context );
        }
        mOffset = -mOffset;
      }
      break;
      case ExteriorRingOnly:
        break;
    }
  }
}

void QgsMarkerLineSymbolLayer::renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( points.isEmpty() )
    return;

  QPointF lastPt = points[0];
  double lengthLeft = 0; // how much is left until next marker

  QgsRenderContext &rc = context.renderContext();
  double interval = mInterval;

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  context.renderContext().expressionContext().appendScope( scope );

  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyInterval ) )
  {
    context.setOriginalValueVariable( mInterval );
    interval = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyInterval, context.renderContext().expressionContext(), mInterval );
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }
  double offsetAlongLine = mOffsetAlongLine;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }

  double painterUnitInterval = rc.convertToPainterUnits( interval, mIntervalUnit, mIntervalMapUnitScale );
  lengthLeft = painterUnitInterval - rc.convertToPainterUnits( offsetAlongLine, mIntervalUnit, mIntervalMapUnitScale );

  int pointNum = 0;
  for ( int i = 1; i < points.count(); ++i )
  {
    const QPointF &pt = points[i];

    if ( lastPt == pt ) // must not be equal!
      continue;

    // for each line, find out dx and dy, and length
    MyLine l( lastPt, pt );
    QPointF diff = l.diffForInterval( painterUnitInterval );

    // if there's some length left from previous line
    // use only the rest for the first point in new line segment
    double c = 1 - lengthLeft / painterUnitInterval;

    lengthLeft += l.length();

    // rotate marker (if desired)
    if ( mRotateMarker )
    {
      mMarker->setLineAngle( l.angle() * 180 / M_PI );
    }


    // while we're not at the end of line segment, draw!
    while ( lengthLeft > painterUnitInterval )
    {
      // "c" is 1 for regular point or in interval (0,1] for begin of line segment
      lastPt += c * diff;
      lengthLeft -= painterUnitInterval;
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );
      mMarker->renderPoint( lastPt, context.feature(), rc, -1, context.selected() );
      c = 1; // reset c (if wasn't 1 already)
    }

    lastPt = pt;
  }

  delete context.renderContext().expressionContext().popScope();
}

static double _averageAngle( QPointF prevPt, QPointF pt, QPointF nextPt )
{
  // calc average angle between the previous and next point
  double a1 = MyLine( prevPt, pt ).angle();
  double a2 = MyLine( pt, nextPt ).angle();
  double unitX = std::cos( a1 ) + std::cos( a2 ), unitY = std::sin( a1 ) + std::sin( a2 );

  return std::atan2( unitY, unitX );
}

void QgsMarkerLineSymbolLayer::renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Placement placement )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext &rc = context.renderContext();

  double origAngle = mMarker->angle();
  int i, maxCount;
  bool isRing = false;

  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  context.renderContext().expressionContext().appendScope( scope );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_COUNT, points.size(), true ) );

  double offsetAlongLine = mOffsetAlongLine;
  if ( mDataDefinedProperties.isActive( QgsSymbolLayer::PropertyOffsetAlongLine ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = mDataDefinedProperties.valueAsDouble( QgsSymbolLayer::PropertyOffsetAlongLine, context.renderContext().expressionContext(), mOffsetAlongLine );
  }
  if ( !qgsDoubleNear( offsetAlongLine, 0.0 ) )
  {
    //scale offset along line
    offsetAlongLine = rc.convertToPainterUnits( offsetAlongLine, mOffsetAlongLineUnit, mOffsetAlongLineMapUnitScale );
  }

  if ( qgsDoubleNear( offsetAlongLine, 0.0 ) && context.renderContext().geometry()
       && context.renderContext().geometry()->hasCurvedSegments() && ( placement == Vertex || placement == CurvePoint ) )
  {
    QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
    const QgsMapToPixel &mtp = context.renderContext().mapToPixel();

    QgsVertexId vId;
    QgsPoint vPoint;
    double x, y, z;
    QPointF mapPoint;
    int pointNum = 0;
    while ( context.renderContext().geometry()->nextVertex( vId, vPoint ) )
    {
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );

      if ( ( placement == Vertex && vId.type == QgsVertexId::SegmentVertex )
           || ( placement == CurvePoint && vId.type == QgsVertexId::CurveVertex ) )
      {
        //transform
        x = vPoint.x(), y = vPoint.y();
        z = 0.0;
        if ( ct.isValid() )
        {
          ct.transformInPlace( x, y, z );
        }
        mapPoint.setX( x );
        mapPoint.setY( y );
        mtp.transformInPlace( mapPoint.rx(), mapPoint.ry() );
        if ( mRotateMarker )
        {
          double angle = context.renderContext().geometry()->vertexAngle( vId );
          mMarker->setAngle( angle * 180 / M_PI );
        }
        mMarker->renderPoint( mapPoint, context.feature(), rc, -1, context.selected() );
      }
    }

    delete context.renderContext().expressionContext().popScope();
    return;
  }

  if ( placement == FirstVertex )
  {
    i = 0;
    maxCount = 1;
  }
  else if ( placement == LastVertex )
  {
    i = points.count() - 1;
    maxCount = points.count();
  }
  else if ( placement == Vertex )
  {
    i = 0;
    maxCount = points.count();
    if ( points.first() == points.last() )
      isRing = true;
  }
  else
  {
    delete context.renderContext().expressionContext().popScope();
    return;
  }

  if ( offsetAlongLine > 0 && ( placement == FirstVertex || placement == LastVertex ) )
  {
    double distance;
    distance = placement == FirstVertex ? offsetAlongLine : -offsetAlongLine;
    renderOffsetVertexAlongLine( points, i, distance, context );
    // restore original rotation
    mMarker->setAngle( origAngle );

    delete context.renderContext().expressionContext().popScope();
    return;
  }

  int pointNum = 0;
  for ( ; i < maxCount; ++i )
  {
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_GEOMETRY_POINT_NUM, ++pointNum, true ) );

    if ( isRing && placement == Vertex && i == points.count() - 1 )
    {
      continue; // don't draw the last marker - it has been drawn already
    }
    // rotate marker (if desired)
    if ( mRotateMarker )
    {
      double angle = markerAngle( points, isRing, i );
      mMarker->setAngle( origAngle + angle * 180 / M_PI );
    }

    mMarker->renderPoint( points.at( i ), context.feature(), rc, -1, context.selected() );
  }

  // restore original rotation
  mMarker->setAngle( origAngle );

  delete context.renderContext().expressionContext().popScope();
}

double QgsMarkerLineSymbolLayer::markerAngle( const QPolygonF &points, bool isRing, int vertex )
{
  double angle = 0;
  const QPointF &pt = points[vertex];

  if ( isRing || ( vertex > 0 && vertex < points.count() - 1 ) )
  {
    int prevIndex = vertex - 1;
    int nextIndex = vertex + 1;

    if ( isRing && ( vertex == 0 || vertex == points.count() - 1 ) )
    {
      prevIndex = points.count() - 2;
      nextIndex = 1;
    }

    QPointF prevPoint, nextPoint;
    while ( prevIndex >= 0 )
    {
      prevPoint = points[ prevIndex ];
      if ( prevPoint != pt )
      {
        break;
      }
      --prevIndex;
    }

    while ( nextIndex < points.count() )
    {
      nextPoint = points[ nextIndex ];
      if ( nextPoint != pt )
      {
        break;
      }
      ++nextIndex;
    }

    if ( prevIndex >= 0 && nextIndex < points.count() )
    {
      angle = _averageAngle( prevPoint, pt, nextPoint );
    }
  }
  else //no ring and vertex is at start / at end
  {
    if ( vertex == 0 )
    {
      while ( vertex < points.size() - 1 )
      {
        const QPointF &nextPt = points[vertex + 1];
        if ( pt != nextPt )
        {
          angle = MyLine( pt, nextPt ).angle();
          return angle;
        }
        ++vertex;
      }
    }
    else
    {
      // use last segment's angle
      while ( vertex >= 1 ) //in case of duplicated vertices, take the next suitable one
      {
        const QPointF &prevPt = points[vertex - 1];
        if ( pt != prevPt )
        {
          angle = MyLine( prevPt, pt ).angle();
          return angle;
        }
        --vertex;
      }
    }
  }
  return angle;
}

void QgsMarkerLineSymbolLayer::renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext &context )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext &rc = context.renderContext();
  double origAngle = mMarker->angle();
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    // rotate marker (if desired)
    if ( mRotateMarker )
    {
      bool isRing = false;
      if ( points.first() == points.last() )
        isRing = true;
      double angle = markerAngle( points, isRing, vertex );
      mMarker->setAngle( origAngle + angle * 180 / M_PI );
    }
    mMarker->renderPoint( points[vertex], context.feature(), rc, -1, context.selected() );
    return;
  }

  int pointIncrement = distance > 0 ? 1 : -1;
  QPointF previousPoint = points[vertex];
  int startPoint = distance > 0 ? std::min( vertex + 1, points.count() - 1 ) : std::max( vertex - 1, 0 );
  int endPoint = distance > 0 ? points.count() - 1 : 0;
  double distanceLeft = std::fabs( distance );

  for ( int i = startPoint; pointIncrement > 0 ? i <= endPoint : i >= endPoint; i += pointIncrement )
  {
    const QPointF &pt = points[i];

    if ( previousPoint == pt ) // must not be equal!
      continue;

    // create line segment
    MyLine l( previousPoint, pt );

    if ( distanceLeft < l.length() )
    {
      //destination point is in current segment
      QPointF markerPoint = previousPoint + l.diffForInterval( distanceLeft );
      // rotate marker (if desired)
      if ( mRotateMarker )
      {
        mMarker->setAngle( origAngle + ( l.angle() * 180 / M_PI ) );
      }
      mMarker->renderPoint( markerPoint, context.feature(), rc, -1, context.selected() );
      return;
    }

    distanceLeft -= l.length();
    previousPoint = pt;
  }

  //didn't find point
}

void QgsMarkerLineSymbolLayer::renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context )
{
  if ( !points.isEmpty() )
  {
    // calc length
    qreal length = 0;
    QPolygonF::const_iterator it = points.constBegin();
    QPointF last = *it;
    for ( ++it; it != points.constEnd(); ++it )
    {
      length += std::sqrt( ( last.x() - it->x() ) * ( last.x() - it->x() ) +
                           ( last.y() - it->y() ) * ( last.y() - it->y() ) );
      last = *it;
    }

    // find the segment where the central point lies
    it = points.constBegin();
    last = *it;
    qreal last_at = 0, next_at = 0;
    QPointF next;
    int segment = 0;
    for ( ++it; it != points.constEnd(); ++it )
    {
      next = *it;
      next_at += std::sqrt( ( last.x() - it->x() ) * ( last.x() - it->x() ) +
                            ( last.y() - it->y() ) * ( last.y() - it->y() ) );
      if ( next_at >= length / 2 )
        break; // we have reached the center
      last = *it;
      last_at = next_at;
      segment++;
    }

    // find out the central point on segment
    MyLine l( last, next ); // for line angle
    qreal k = ( length * 0.5 - last_at ) / ( next_at - last_at );
    QPointF pt = last + ( next - last ) * k;

    // draw the marker
    double origAngle = mMarker->angle();
    if ( mRotateMarker )
      mMarker->setAngle( origAngle + l.angle() * 180 / M_PI );
    mMarker->renderPoint( pt, context.feature(), context.renderContext(), -1, context.selected() );
    if ( mRotateMarker )
      mMarker->setAngle( origAngle );
  }
}


QgsStringMap QgsMarkerLineSymbolLayer::properties() const
{
  QgsStringMap map;
  map[QStringLiteral( "rotate" )] = ( mRotateMarker ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map[QStringLiteral( "interval" )] = QString::number( mInterval );
  map[QStringLiteral( "offset" )] = QString::number( mOffset );
  map[QStringLiteral( "offset_along_line" )] = QString::number( mOffsetAlongLine );
  map[QStringLiteral( "offset_along_line_unit" )] = QgsUnitTypes::encodeUnit( mOffsetAlongLineUnit );
  map[QStringLiteral( "offset_along_line_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetAlongLineMapUnitScale );
  map[QStringLiteral( "offset_unit" )] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map[QStringLiteral( "offset_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map[QStringLiteral( "interval_unit" )] = QgsUnitTypes::encodeUnit( mIntervalUnit );
  map[QStringLiteral( "interval_map_unit_scale" )] = QgsSymbolLayerUtils::encodeMapUnitScale( mIntervalMapUnitScale );
  if ( mPlacement == Vertex )
    map[QStringLiteral( "placement" )] = QStringLiteral( "vertex" );
  else if ( mPlacement == LastVertex )
    map[QStringLiteral( "placement" )] = QStringLiteral( "lastvertex" );
  else if ( mPlacement == FirstVertex )
    map[QStringLiteral( "placement" )] = QStringLiteral( "firstvertex" );
  else if ( mPlacement == CentralPoint )
    map[QStringLiteral( "placement" )] = QStringLiteral( "centralpoint" );
  else if ( mPlacement == CurvePoint )
    map[QStringLiteral( "placement" )] = QStringLiteral( "curvepoint" );
  else
    map[QStringLiteral( "placement" )] = QStringLiteral( "interval" );

  map[QStringLiteral( "ring_filter" )] = QString::number( static_cast< int >( mRingFilter ) );
  return map;
}

QgsSymbol *QgsMarkerLineSymbolLayer::subSymbol()
{
  return mMarker.get();
}

bool QgsMarkerLineSymbolLayer::setSubSymbol( QgsSymbol *symbol )
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

QgsMarkerLineSymbolLayer *QgsMarkerLineSymbolLayer::clone() const
{
  QgsMarkerLineSymbolLayer *x = new QgsMarkerLineSymbolLayer( mRotateMarker, mInterval );
  x->setSubSymbol( mMarker->clone() );
  x->setOffset( mOffset );
  x->setPlacement( mPlacement );
  x->setOffsetUnit( mOffsetUnit );
  x->setOffsetMapUnitScale( mOffsetMapUnitScale );
  x->setIntervalUnit( mIntervalUnit );
  x->setIntervalMapUnitScale( mIntervalMapUnitScale );
  x->setOffsetAlongLine( mOffsetAlongLine );
  x->setOffsetAlongLineMapUnitScale( mOffsetAlongLineMapUnitScale );
  x->setOffsetAlongLineUnit( mOffsetAlongLineUnit );
  x->setRingFilter( mRingFilter );
  copyDataDefinedProperties( x );
  copyPaintEffect( x );
  return x;
}

void QgsMarkerLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const
{
  for ( int i = 0; i < mMarker->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( QStringLiteral( "se:LineSymbolizer" ) );
    if ( !props.value( QStringLiteral( "uom" ), QString() ).isEmpty() )
      symbolizerElem.setAttribute( QStringLiteral( "uom" ), props.value( QStringLiteral( "uom" ), QString() ) );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( QStringLiteral( "geom" ), QString() ) );

    QString gap;
    switch ( mPlacement )
    {
      case FirstVertex:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "firstPoint" ) ) );
        break;
      case LastVertex:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "lastPoint" ) ) );
        break;
      case CentralPoint:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "centralPoint" ) ) );
        break;
      case Vertex:
        // no way to get line/polygon's vertices, use a VendorOption
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "placement" ), QStringLiteral( "points" ) ) );
        break;
      default:
        double interval = QgsSymbolLayerUtils::rescaleUom( mInterval, mIntervalUnit, props );
        gap = qgsDoubleToString( interval );
        break;
    }

    if ( !mRotateMarker )
    {
      // markers in LineSymbolizer must be drawn following the line orientation,
      // use a VendorOption when no marker rotation
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, QStringLiteral( "rotateMarker" ), QStringLiteral( "0" ) ) );
    }

    // <Stroke>
    QDomElement strokeElem = doc.createElement( QStringLiteral( "se:Stroke" ) );
    symbolizerElem.appendChild( strokeElem );

    // <GraphicStroke>
    QDomElement graphicStrokeElem = doc.createElement( QStringLiteral( "se:GraphicStroke" ) );
    strokeElem.appendChild( graphicStrokeElem );

    QgsSymbolLayer *layer = mMarker->symbolLayer( i );
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    if ( !markerLayer )
    {
      graphicStrokeElem.appendChild( doc.createComment( QStringLiteral( "MarkerSymbolLayerV2 expected, %1 found. Skip it." ).arg( layer->layerType() ) ) );
    }
    else
    {
      markerLayer->writeSldMarker( doc, graphicStrokeElem, props );
    }

    if ( !gap.isEmpty() )
    {
      QDomElement gapElem = doc.createElement( QStringLiteral( "se:Gap" ) );
      QgsSymbolLayerUtils::createExpressionElement( doc, gapElem, gap );
      graphicStrokeElem.appendChild( gapElem );
    }

    if ( !qgsDoubleNear( mOffset, 0.0 ) )
    {
      QDomElement perpOffsetElem = doc.createElement( QStringLiteral( "se:PerpendicularOffset" ) );
      double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
      perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
      symbolizerElem.appendChild( perpOffsetElem );
    }
  }
}

QgsSymbolLayer *QgsMarkerLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( QStringLiteral( "Entered." ) );

  QDomElement strokeElem = element.firstChildElement( QStringLiteral( "Stroke" ) );
  if ( strokeElem.isNull() )
    return nullptr;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( QStringLiteral( "GraphicStroke" ) );
  if ( graphicStrokeElem.isNull() )
    return nullptr;

  // retrieve vendor options
  bool rotateMarker = true;
  Placement placement = Interval;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( element );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == QLatin1String( "placement" ) )
    {
      if ( it.value() == QLatin1String( "points" ) ) placement = Vertex;
      else if ( it.value() == QLatin1String( "firstPoint" ) ) placement = FirstVertex;
      else if ( it.value() == QLatin1String( "lastPoint" ) ) placement = LastVertex;
      else if ( it.value() == QLatin1String( "centralPoint" ) ) placement = CentralPoint;
    }
    else if ( it.value() == QLatin1String( "rotateMarker" ) )
    {
      rotateMarker = it.value() == QLatin1String( "0" );
    }
  }

  std::unique_ptr< QgsMarkerSymbol > marker;

  QgsSymbolLayer *l = QgsSymbolLayerUtils::createMarkerLayerFromSld( graphicStrokeElem );
  if ( l )
  {
    QgsSymbolLayerList layers;
    layers.append( l );
    marker.reset( new QgsMarkerSymbol( layers ) );
  }

  if ( !marker )
    return nullptr;

  double interval = 0.0;
  QDomElement gapElem = graphicStrokeElem.firstChildElement( QStringLiteral( "Gap" ) );
  if ( !gapElem.isNull() )
  {
    bool ok;
    double d = gapElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      interval = d;
  }

  double offset = 0.0;
  QDomElement perpOffsetElem = graphicStrokeElem.firstChildElement( QStringLiteral( "PerpendicularOffset" ) );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QString uom = element.attribute( QStringLiteral( "uom" ) );
  interval = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, interval );
  offset = QgsSymbolLayerUtils::sizeInPixelsFromSldUom( uom, offset );

  QgsMarkerLineSymbolLayer *x = new QgsMarkerLineSymbolLayer( rotateMarker );
  x->setOutputUnit( QgsUnitTypes::RenderUnit::RenderPixels );
  x->setPlacement( placement );
  x->setInterval( interval );
  x->setSubSymbol( marker.release() );
  x->setOffset( offset );
  return x;
}

void QgsMarkerLineSymbolLayer::setWidth( double width )
{
  mMarker->setSize( width );
}

void QgsMarkerLineSymbolLayer::setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property )
{
  if ( key == QgsSymbolLayer::PropertyWidth && mMarker && property )
  {
    mMarker->setDataDefinedSize( property );
  }
  QgsLineSymbolLayer::setDataDefinedProperty( key, property );
}

double QgsMarkerLineSymbolLayer::width() const
{
  return mMarker->size();
}

void QgsMarkerLineSymbolLayer::setOutputUnit( QgsUnitTypes::RenderUnit unit )
{
  QgsLineSymbolLayer::setOutputUnit( unit );
  mMarker->setOutputUnit( unit );
  mIntervalUnit = unit;
  mOffsetUnit = unit;
  mOffsetAlongLineUnit = unit;
}

QgsUnitTypes::RenderUnit QgsMarkerLineSymbolLayer::outputUnit() const
{
  QgsUnitTypes::RenderUnit unit = QgsLineSymbolLayer::outputUnit();
  if ( mIntervalUnit != unit || mOffsetUnit != unit || mOffsetAlongLineUnit != unit )
  {
    return QgsUnitTypes::RenderUnknownUnit;
  }
  return unit;
}

void QgsMarkerLineSymbolLayer::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayer::setMapUnitScale( scale );
  mIntervalMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
  mOffsetAlongLineMapUnitScale = scale;
}

QgsMapUnitScale QgsMarkerLineSymbolLayer::mapUnitScale() const
{
  if ( QgsLineSymbolLayer::mapUnitScale() == mIntervalMapUnitScale &&
       mIntervalMapUnitScale == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == mOffsetAlongLineMapUnitScale )
  {
    return mOffsetMapUnitScale;
  }
  return QgsMapUnitScale();
}

QSet<QString> QgsMarkerLineSymbolLayer::usedAttributes( const QgsRenderContext &context ) const
{
  QSet<QString> attr = QgsLineSymbolLayer::usedAttributes( context );
  if ( mMarker )
    attr.unite( mMarker->usedAttributes( context ) );
  return attr;
}

bool QgsMarkerLineSymbolLayer::hasDataDefinedProperties() const
{
  if ( QgsSymbolLayer::hasDataDefinedProperties() )
    return true;
  if ( mMarker && mMarker->hasDataDefinedProperties() )
    return true;
  return false;
}

double QgsMarkerLineSymbolLayer::estimateMaxBleed( const QgsRenderContext &context ) const
{
  return context.convertToPainterUnits( ( mMarker->size() / 2.0 ), mMarker->sizeUnit(), mMarker->sizeMapUnitScale() ) +
         context.convertToPainterUnits( std::fabs( mOffset ), mOffsetUnit, mOffsetMapUnitScale );
}



