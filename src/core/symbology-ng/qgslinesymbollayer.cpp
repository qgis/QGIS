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

#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

QgsSimpleLineSymbolLayer::QgsSimpleLineSymbolLayer( const QColor& color, double width, Qt::PenStyle penStyle )
    : mPenStyle( penStyle )
    , mPenJoinStyle( DEFAULT_SIMPLELINE_JOINSTYLE )
    , mPenCapStyle( DEFAULT_SIMPLELINE_CAPSTYLE )
    , mUseCustomDashPattern( false )
    , mCustomDashPatternUnit( QgsUnitTypes::RenderMillimeters )
    , mDrawInsidePolygon( false )
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

QgsSymbolLayer* QgsSimpleLineSymbolLayer::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( "line_color" ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props["line_color"] );
  }
  else if ( props.contains( "outline_color" ) )
  {
    color = QgsSymbolLayerUtils::decodeColor( props["outline_color"] );
  }
  else if ( props.contains( "color" ) )
  {
    //pre 2.5 projects used "color"
    color = QgsSymbolLayerUtils::decodeColor( props["color"] );
  }
  if ( props.contains( "line_width" ) )
  {
    width = props["line_width"].toDouble();
  }
  else if ( props.contains( "outline_width" ) )
  {
    width = props["outline_width"].toDouble();
  }
  else if ( props.contains( "width" ) )
  {
    //pre 2.5 projects used "width"
    width = props["width"].toDouble();
  }
  if ( props.contains( "line_style" ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props["line_style"] );
  }
  else if ( props.contains( "outline_style" ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props["outline_style"] );
  }
  else if ( props.contains( "penstyle" ) )
  {
    penStyle = QgsSymbolLayerUtils::decodePenStyle( props["penstyle"] );
  }

  QgsSimpleLineSymbolLayer* l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  if ( props.contains( "line_width_unit" ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props["line_width_unit"] ) );
  }
  else if ( props.contains( "outline_width_unit" ) )
  {
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props["outline_width_unit"] ) );
  }
  else if ( props.contains( "width_unit" ) )
  {
    //pre 2.5 projects used "width_unit"
    l->setWidthUnit( QgsUnitTypes::decodeRenderUnit( props["width_unit"] ) );
  }
  if ( props.contains( "width_map_unit_scale" ) )
    l->setWidthMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["width_map_unit_scale"] ) );
  if ( props.contains( "offset" ) )
    l->setOffset( props["offset"].toDouble() );
  if ( props.contains( "offset_unit" ) )
    l->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  if ( props.contains( "joinstyle" ) )
    l->setPenJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( props["joinstyle"] ) );
  if ( props.contains( "capstyle" ) )
    l->setPenCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( props["capstyle"] ) );

  if ( props.contains( "use_custom_dash" ) )
  {
    l->setUseCustomDashPattern( props["use_custom_dash"].toInt() );
  }
  if ( props.contains( "customdash" ) )
  {
    l->setCustomDashVector( QgsSymbolLayerUtils::decodeRealVector( props["customdash"] ) );
  }
  if ( props.contains( "customdash_unit" ) )
  {
    l->setCustomDashPatternUnit( QgsUnitTypes::decodeRenderUnit( props["customdash_unit"] ) );
  }
  if ( props.contains( "customdash_map_unit_scale" ) )
  {
    l->setCustomDashPatternMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["customdash_map_unit_scale"] ) );
  }

  if ( props.contains( "draw_inside_polygon" ) )
  {
    l->setDrawInsidePolygon( props["draw_inside_polygon"].toInt() );
  }

  l->restoreDataDefinedProperties( props );

  return l;
}


QString QgsSimpleLineSymbolLayer::layerType() const
{
  return "SimpleLine";
}

void QgsSimpleLineSymbolLayer::startRender( QgsSymbolRenderContext& context )
{
  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.alpha() );
  mPen.setColor( penColor );
  double scaledWidth = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), mWidth, mWidthUnit, mWidthMapUnitScale );
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
         && ( scaledWidth * context.renderContext().rasterScaleFactor() ) < 1.0 )
    {
      dashWidthDiv = 1.0;
    }
    QVector<qreal> scaledVector;
    QVector<qreal>::const_iterator it = mCustomDashVector.constBegin();
    for ( ; it != mCustomDashVector.constEnd(); ++it )
    {
      //the dash is specified in terms of pen widths, therefore the division
      scaledVector << QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), ( *it ), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv;
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
  if ( ! selectionIsOpaque )
    selColor.setAlphaF( context.alpha() );
  mSelPen.setColor( selColor );

  //prepare expressions for data defined properties
  prepareExpressions( context );
}

void QgsSimpleLineSymbolLayer::stopRender( QgsSymbolRenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSimpleLineSymbolLayer::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  if ( mDrawInsidePolygon )
  {
    //only drawing the line on the interior of the polygon, so set clip path for painter
    p->save();
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

    //use intersect mode, as a clip path may already exist (eg, for composer maps)
    p->setClipPath( clipPath, Qt::IntersectClip );
  }

  renderPolyline( points, context );
  if ( rings )
  {
    mOffset = -mOffset; // invert the offset for rings!
    Q_FOREACH ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
    mOffset = -mOffset;
  }

  if ( mDrawInsidePolygon )
  {
    //restore painter to reset clip path
    p->restore();
  }

}

void QgsSimpleLineSymbolLayer::renderPolyline( const QPolygonF& points, QgsSymbolRenderContext& context )
{
  QPainter* p = context.renderContext().painter();
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
    double scaledOffset = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), offset, mOffsetUnit, mOffsetMapUnitScale );
    QList<QPolygonF> mline = ::offsetLine( points, scaledOffset, context.feature() ? context.feature()->geometry().type() : QgsWkbTypes::LineGeometry );
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
  map["line_color"] = QgsSymbolLayerUtils::encodeColor( mColor );
  map["line_width"] = QString::number( mWidth );
  map["line_width_unit"] = QgsUnitTypes::encodeUnit( mWidthUnit );
  map["width_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mWidthMapUnitScale );
  map["line_style"] = QgsSymbolLayerUtils::encodePenStyle( mPenStyle );
  map["joinstyle"] = QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle );
  map["capstyle"] = QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle );
  map["offset"] = QString::number( mOffset );
  map["offset_unit"] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["use_custom_dash"] = ( mUseCustomDashPattern ? "1" : "0" );
  map["customdash"] = QgsSymbolLayerUtils::encodeRealVector( mCustomDashVector );
  map["customdash_unit"] = QgsUnitTypes::encodeUnit( mCustomDashPatternUnit );
  map["customdash_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mCustomDashPatternMapUnitScale );
  map["draw_inside_polygon"] = ( mDrawInsidePolygon ? "1" : "0" );
  saveDataDefinedProperties( map );
  return map;
}

QgsSimpleLineSymbolLayer* QgsSimpleLineSymbolLayer::clone() const
{
  QgsSimpleLineSymbolLayer* l = new QgsSimpleLineSymbolLayer( mColor, mWidth, mPenStyle );
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
  copyDataDefinedProperties( l );
  copyPaintEffect( l );
  return l;
}

void QgsSimpleLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  if ( mPenStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( "se:LineSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( "se:Stroke" );
  symbolizerElem.appendChild( strokeElem );

  Qt::PenStyle penStyle = mUseCustomDashPattern ? Qt::CustomDashLine : mPenStyle;
  double width = QgsSymbolLayerUtils::rescaleUom( mWidth, mWidthUnit, props );
  QVector<qreal> customDashVector = QgsSymbolLayerUtils::rescaleUom( mCustomDashVector, mCustomDashPatternUnit, props );
  QgsSymbolLayerUtils::lineToSld( doc, strokeElem, penStyle, mColor, width,
                                  &mPenJoinStyle, &mPenCapStyle, &customDashVector );

  // <se:PerpendicularOffset>
  if ( !qgsDoubleNear( mOffset, 0.0 ) )
  {
    QDomElement perpOffsetElem = doc.createElement( "se:PerpendicularOffset" );
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

QgsSymbolLayer* QgsSimpleLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement strokeElem = element.firstChildElement( "Stroke" );
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
  QDomElement perpOffsetElem = element.firstChildElement( "PerpendicularOffset" );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QgsSimpleLineSymbolLayer* l = new QgsSimpleLineSymbolLayer( color, width, penStyle );
  l->setOffset( offset );
  l->setPenJoinStyle( penJoinStyle );
  l->setPenCapStyle( penCapStyle );
  l->setUseCustomDashPattern( penStyle == Qt::CustomDashLine );
  l->setCustomDashVector( customDashVector );
  return l;
}

void QgsSimpleLineSymbolLayer::applyDataDefinedSymbology( QgsSymbolRenderContext& context, QPen& pen, QPen& selPen, double& offset )
{
  if ( !hasDataDefinedProperties() )
    return; // shortcut

  //data defined properties
  bool hasStrokeWidthExpression = false;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_WIDTH ) )
  {
    context.setOriginalValueVariable( mWidth );
    double scaledWidth = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(),
                         evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_WIDTH, context, mWidth ).toDouble(),
                         mWidthUnit, mWidthMapUnitScale );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
    hasStrokeWidthExpression = true;
  }

  //color
  bool ok;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_COLOR ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setColor( QgsSymbolLayerUtils::decodeColor( colorString ) );
  }

  //offset
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET, context, offset ).toDouble();
  }

  //dash dot vector
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_CUSTOMDASH ) )
  {
    double scaledWidth = QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), mWidth, mWidthUnit, mWidthMapUnitScale );
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
         && ( scaledWidth * context.renderContext().rasterScaleFactor() ) < 1.0 )
    {
      dashWidthDiv = 1.0;
    }

    QVector<qreal> dashVector;
    QStringList dashList = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_CUSTOMDASH, context, QVariant(), &ok ).toString().split( ';' );
    if ( ok )
    {
      QStringList::const_iterator dashIt = dashList.constBegin();
      for ( ; dashIt != dashList.constEnd(); ++dashIt )
      {
        dashVector.push_back( QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), dashIt->toDouble(), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv );
      }
      pen.setDashPattern( dashVector );
    }
  }

  //line style
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_LINE_STYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenStyle( mPenStyle ) );
    QString lineStyleString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_LINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setStyle( QgsSymbolLayerUtils::decodePenStyle( lineStyleString ) );
  }

  //join style
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_JOINSTYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenJoinStyle( mPenJoinStyle ) );
    QString joinStyleString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_JOINSTYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setJoinStyle( QgsSymbolLayerUtils::decodePenJoinStyle( joinStyleString ) );
  }

  //cap style
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_CAPSTYLE ) )
  {
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodePenCapStyle( mPenCapStyle ) );
    QString capStyleString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_CAPSTYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setCapStyle( QgsSymbolLayerUtils::decodePenCapStyle( capStyleString ) );
  }
}

double QgsSimpleLineSymbolLayer::estimateMaxBleed() const
{
  if ( mDrawInsidePolygon )
  {
    //set to clip line to the interior of polygon, so we expect no bleed
    return 0;
  }
  else
  {
    return ( mWidth / 2.0 ) + mOffset;
  }
}

QVector<qreal> QgsSimpleLineSymbolLayer::dxfCustomDashPattern( QgsUnitTypes::RenderUnit& unit ) const
{
  unit = mCustomDashPatternUnit;
  return mUseCustomDashPattern ? mCustomDashVector : QVector<qreal>();
}

Qt::PenStyle QgsSimpleLineSymbolLayer::dxfPenStyle() const
{
  return mPenStyle;
}

double QgsSimpleLineSymbolLayer::dxfWidth( const QgsDxfExport& e, QgsSymbolRenderContext& context ) const
{
  double width = mWidth;

  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_WIDTH ) )
  {
    context.setOriginalValueVariable( mWidth );
    width = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_WIDTH, context, mWidth ).toDouble() * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
  }

  return width * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
}

QColor QgsSimpleLineSymbolLayer::dxfColor( QgsSymbolRenderContext& context ) const
{
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_COLOR ) )
  {
    bool ok;
    context.setOriginalValueVariable( QgsSymbolLayerUtils::encodeColor( mColor ) );
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      return ( QgsSymbolLayerUtils::decodeColor( colorString ) );
  }
  return mColor;
}

double QgsSimpleLineSymbolLayer::dxfOffset( const QgsDxfExport& e, QgsSymbolRenderContext &context ) const
{
  Q_UNUSED( e );
  double offset = mOffset;

  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET, context, mOffset ).toDouble();
  }
  return offset;
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
      mLength = sqrt( x * x + y * y );
    }

    // return angle in radians
    double angle()
    {
      double a = ( mVertical ? M_PI / 2 : atan( mT ) );

      if ( !mIncreasing )
        a += M_PI;
      return a;
    }

    // return difference for x,y when going along the line with specified interval
    QPointF diffForInterval( double interval )
    {
      if ( mVertical )
        return ( mIncreasing ? QPointF( 0, interval ) : QPointF( 0, -interval ) );

      double alpha = atan( mT );
      double dx = cos( alpha ) * interval;
      double dy = sin( alpha ) * interval;
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

QgsMarkerLineSymbolLayer::~QgsMarkerLineSymbolLayer()
{
  delete mMarker;
}

QgsSymbolLayer* QgsMarkerLineSymbolLayer::create( const QgsStringMap& props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;


  if ( props.contains( "interval" ) )
    interval = props["interval"].toDouble();
  if ( props.contains( "rotate" ) )
    rotate = ( props["rotate"] == "1" );

  QgsMarkerLineSymbolLayer* x = new QgsMarkerLineSymbolLayer( rotate, interval );
  if ( props.contains( "offset" ) )
  {
    x->setOffset( props["offset"].toDouble() );
  }
  if ( props.contains( "offset_unit" ) )
  {
    x->setOffsetUnit( QgsUnitTypes::decodeRenderUnit( props["offset_unit"] ) );
  }
  if ( props.contains( "interval_unit" ) )
  {
    x->setIntervalUnit( QgsUnitTypes::decodeRenderUnit( props["interval_unit"] ) );
  }
  if ( props.contains( "offset_along_line" ) )
  {
    x->setOffsetAlongLine( props["offset_along_line"].toDouble() );
  }
  if ( props.contains( "offset_along_line_unit" ) )
  {
    x->setOffsetAlongLineUnit( QgsUnitTypes::decodeRenderUnit( props["offset_along_line_unit"] ) );
  }
  if ( props.contains(( "offset_along_line_map_unit_scale" ) ) )
  {
    x->setOffsetAlongLineMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["offset_along_line_map_unit_scale"] ) );
  }

  if ( props.contains( "offset_map_unit_scale" ) )
  {
    x->setOffsetMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  }
  if ( props.contains( "interval_map_unit_scale" ) )
  {
    x->setIntervalMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( props["interval_map_unit_scale"] ) );
  }

  if ( props.contains( "placement" ) )
  {
    if ( props["placement"] == "vertex" )
      x->setPlacement( Vertex );
    else if ( props["placement"] == "lastvertex" )
      x->setPlacement( LastVertex );
    else if ( props["placement"] == "firstvertex" )
      x->setPlacement( FirstVertex );
    else if ( props["placement"] == "centralpoint" )
      x->setPlacement( CentralPoint );
    else if ( props["placement"] == "curvepoint" )
      x->setPlacement( CurvePoint );
    else
      x->setPlacement( Interval );
  }

  x->restoreDataDefinedProperties( props );

  return x;
}

QString QgsMarkerLineSymbolLayer::layerType() const
{
  return "MarkerLine";
}

void QgsMarkerLineSymbolLayer::setColor( const QColor& color )
{
  mMarker->setColor( color );
  mColor = color;
}

QColor QgsMarkerLineSymbolLayer::color() const
{
  return mMarker ? mMarker->color() : mColor;
}

void QgsMarkerLineSymbolLayer::startRender( QgsSymbolRenderContext& context )
{
  mMarker->setAlpha( context.alpha() );

  // if being rotated, it gets initialized with every line segment
  QgsSymbol::RenderHints hints = 0;
  if ( mRotateMarker )
    hints |= QgsSymbol::DynamicRotation;
  mMarker->setRenderHints( hints );

  mMarker->startRender( context.renderContext(), context.fields() );

  //prepare expressions for data defined properties
  prepareExpressions( context );
}

void QgsMarkerLineSymbolLayer::stopRender( QgsSymbolRenderContext& context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsMarkerLineSymbolLayer::renderPolyline( const QPolygonF& points, QgsSymbolRenderContext& context )
{
  double offset = mOffset;

  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET ) )
  {
    context.setOriginalValueVariable( mOffset );
    offset = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET, context, mOffset ).toDouble();
  }

  Placement placement = mPlacement;

  bool ok;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_PLACEMENT ) )
  {
    QString placementString = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_PLACEMENT, context, QVariant(), &ok ).toString();
    if ( ok )
    {
      if ( placementString.compare( "vertex", Qt::CaseInsensitive ) == 0 )
      {
        placement = Vertex;
      }
      else if ( placementString.compare( "lastvertex", Qt::CaseInsensitive ) == 0 )
      {
        placement = LastVertex;
      }
      else if ( placementString.compare( "firstvertex", Qt::CaseInsensitive ) == 0 )
      {
        placement = FirstVertex;
      }
      else if ( placementString.compare( "centerpoint", Qt::CaseInsensitive ) == 0 )
      {
        placement = CentralPoint;
      }
      else if ( placementString.compare( "curvepoint", Qt::CaseInsensitive ) == 0 )
      {
        placement = CurvePoint;
      }
      else
      {
        placement = Interval;
      }
    }
  }

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
    QList<QPolygonF> mline = ::offsetLine( points, QgsSymbolLayerUtils::convertToPainterUnits( context.renderContext(), offset, mOffsetUnit, mOffsetMapUnitScale ), context.feature() ? context.feature()->geometry().type() : QgsWkbTypes::LineGeometry );

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
}

void QgsMarkerLineSymbolLayer::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolRenderContext& context )
{
  const QgsCurvePolygon* curvePolygon = dynamic_cast<const QgsCurvePolygon*>( context.renderContext().geometry() );

  if ( curvePolygon )
  {
    context.renderContext().setGeometry( curvePolygon->exteriorRing() );
  }
  renderPolyline( points, context );
  if ( rings )
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
}

void QgsMarkerLineSymbolLayer::renderPolylineInterval( const QPolygonF& points, QgsSymbolRenderContext& context )
{
  if ( points.isEmpty() )
    return;

  QPointF lastPt = points[0];
  double lengthLeft = 0; // how much is left until next marker
  bool first = mOffsetAlongLine ? false : true; //only draw marker at first vertex when no offset along line is set

  QgsRenderContext& rc = context.renderContext();
  double interval = mInterval;

  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_INTERVAL ) )
  {
    context.setOriginalValueVariable( mInterval );
    interval = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_INTERVAL, context, mInterval ).toDouble();
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }
  double offsetAlongLine = mOffsetAlongLine;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET_ALONG_LINE ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET_ALONG_LINE, context, mOffsetAlongLine ).toDouble();
  }

  double painterUnitInterval = QgsSymbolLayerUtils::convertToPainterUnits( rc, interval, mIntervalUnit, mIntervalMapUnitScale );
  lengthLeft = painterUnitInterval - QgsSymbolLayerUtils::convertToPainterUnits( rc, offsetAlongLine, mIntervalUnit, mIntervalMapUnitScale );

  for ( int i = 1; i < points.count(); ++i )
  {
    const QPointF& pt = points[i];

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

    // draw first marker
    if ( first )
    {
      mMarker->renderPoint( lastPt, context.feature(), rc, -1, context.selected() );
      first = false;
    }

    // while we're not at the end of line segment, draw!
    while ( lengthLeft > painterUnitInterval )
    {
      // "c" is 1 for regular point or in interval (0,1] for begin of line segment
      lastPt += c * diff;
      lengthLeft -= painterUnitInterval;
      mMarker->renderPoint( lastPt, context.feature(), rc, -1, context.selected() );
      c = 1; // reset c (if wasn't 1 already)
    }

    lastPt = pt;
  }
}

static double _averageAngle( QPointF prevPt, QPointF pt, QPointF nextPt )
{
  // calc average angle between the previous and next point
  double a1 = MyLine( prevPt, pt ).angle();
  double a2 = MyLine( pt, nextPt ).angle();
  double unitX = cos( a1 ) + cos( a2 ), unitY = sin( a1 ) + sin( a2 );

  return atan2( unitY, unitX );
}

void QgsMarkerLineSymbolLayer::renderPolylineVertex( const QPolygonF& points, QgsSymbolRenderContext& context, Placement placement )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext& rc = context.renderContext();

  double origAngle = mMarker->angle();
  int i, maxCount;
  bool isRing = false;

  double offsetAlongLine = mOffsetAlongLine;
  if ( hasDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET_ALONG_LINE ) )
  {
    context.setOriginalValueVariable( mOffsetAlongLine );
    offsetAlongLine = evaluateDataDefinedProperty( QgsSymbolLayer::EXPR_OFFSET_ALONG_LINE, context, mOffsetAlongLine ).toDouble();
  }
  if ( !qgsDoubleNear( offsetAlongLine, 0.0 ) )
  {
    //scale offset along line
    offsetAlongLine = QgsSymbolLayerUtils::convertToPainterUnits( rc, offsetAlongLine, mOffsetAlongLineUnit, mOffsetAlongLineMapUnitScale );
  }

  if ( qgsDoubleNear( offsetAlongLine, 0.0 ) && context.renderContext().geometry()
       && context.renderContext().geometry()->hasCurvedSegments() && ( placement == Vertex || placement == CurvePoint ) )
  {
    QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
    const QgsMapToPixel& mtp = context.renderContext().mapToPixel();

    QgsVertexId vId;
    QgsPointV2 vPoint;
    double x, y, z;
    QPointF mapPoint;
    while ( context.renderContext().geometry()->nextVertex( vId, vPoint ) )
    {
      if (( placement == Vertex && vId.type == QgsVertexId::SegmentVertex )
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
    return;
  }

  if ( offsetAlongLine > 0 && ( placement == FirstVertex || placement == LastVertex ) )
  {
    double distance;
    distance = placement == FirstVertex ? offsetAlongLine : -offsetAlongLine;
    renderOffsetVertexAlongLine( points, i, distance, context );
    // restore original rotation
    mMarker->setAngle( origAngle );
    return;
  }

  for ( ; i < maxCount; ++i )
  {
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
}

double QgsMarkerLineSymbolLayer::markerAngle( const QPolygonF& points, bool isRing, int vertex )
{
  double angle = 0;
  const QPointF& pt = points[vertex];

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
        const QPointF& nextPt = points[vertex+1];
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
        const QPointF& prevPt = points[vertex-1];
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

void QgsMarkerLineSymbolLayer::renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolRenderContext& context )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext& rc = context.renderContext();
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
  int startPoint = distance > 0 ? qMin( vertex + 1, points.count() - 1 ) : qMax( vertex - 1, 0 );
  int endPoint = distance > 0 ? points.count() - 1 : 0;
  double distanceLeft = qAbs( distance );

  for ( int i = startPoint; pointIncrement > 0 ? i <= endPoint : i >= endPoint; i += pointIncrement )
  {
    const QPointF& pt = points[i];

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
  return;
}

void QgsMarkerLineSymbolLayer::renderPolylineCentral( const QPolygonF& points, QgsSymbolRenderContext& context )
{
  if ( !points.isEmpty() )
  {
    // calc length
    qreal length = 0;
    QPolygonF::const_iterator it = points.constBegin();
    QPointF last = *it;
    for ( ++it; it != points.constEnd(); ++it )
    {
      length += sqrt(( last.x() - it->x() ) * ( last.x() - it->x() ) +
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
      next_at += sqrt(( last.x() - it->x() ) * ( last.x() - it->x() ) +
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
  map["rotate"] = ( mRotateMarker ? "1" : "0" );
  map["interval"] = QString::number( mInterval );
  map["offset"] = QString::number( mOffset );
  map["offset_along_line"] = QString::number( mOffsetAlongLine );
  map["offset_along_line_unit"] = QgsUnitTypes::encodeUnit( mOffsetAlongLineUnit );
  map["offset_along_line_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetAlongLineMapUnitScale );
  map["offset_unit"] = QgsUnitTypes::encodeUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["interval_unit"] = QgsUnitTypes::encodeUnit( mIntervalUnit );
  map["interval_map_unit_scale"] = QgsSymbolLayerUtils::encodeMapUnitScale( mIntervalMapUnitScale );
  if ( mPlacement == Vertex )
    map["placement"] = "vertex";
  else if ( mPlacement == LastVertex )
    map["placement"] = "lastvertex";
  else if ( mPlacement == FirstVertex )
    map["placement"] = "firstvertex";
  else if ( mPlacement == CentralPoint )
    map["placement"] = "centralpoint";
  else if ( mPlacement == CurvePoint )
    map["placement"] = "curvepoint";
  else
    map["placement"] = "interval";

  saveDataDefinedProperties( map );
  return map;
}

QgsSymbol* QgsMarkerLineSymbolLayer::subSymbol()
{
  return mMarker;
}

bool QgsMarkerLineSymbolLayer::setSubSymbol( QgsSymbol* symbol )
{
  if ( !symbol || symbol->type() != QgsSymbol::Marker )
  {
    delete symbol;
    return false;
  }

  delete mMarker;
  mMarker = static_cast<QgsMarkerSymbol*>( symbol );
  mColor = mMarker->color();
  return true;
}

QgsMarkerLineSymbolLayer* QgsMarkerLineSymbolLayer::clone() const
{
  QgsMarkerLineSymbolLayer* x = new QgsMarkerLineSymbolLayer( mRotateMarker, mInterval );
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
  copyDataDefinedProperties( x );
  copyPaintEffect( x );
  return x;
}

void QgsMarkerLineSymbolLayer::toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap& props ) const
{
  for ( int i = 0; i < mMarker->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( "se:LineSymbolizer" );
    if ( !props.value( "uom", "" ).isEmpty() )
      symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerUtils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

    QString gap;
    switch ( mPlacement )
    {
      case FirstVertex:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, "placement", "firstPoint" ) );
        break;
      case LastVertex:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, "placement", "lastPoint" ) );
        break;
      case CentralPoint:
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, "placement", "centralPoint" ) );
        break;
      case Vertex:
        // no way to get line/polygon's vertices, use a VendorOption
        symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, "placement", "points" ) );
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
      symbolizerElem.appendChild( QgsSymbolLayerUtils::createVendorOptionElement( doc, "rotateMarker", "0" ) );
    }

    // <Stroke>
    QDomElement strokeElem = doc.createElement( "se:Stroke" );
    symbolizerElem.appendChild( strokeElem );

    // <GraphicStroke>
    QDomElement graphicStrokeElem = doc.createElement( "se:GraphicStroke" );
    strokeElem.appendChild( graphicStrokeElem );

    QgsSymbolLayer *layer = mMarker->symbolLayer( i );
    QgsMarkerSymbolLayer *markerLayer = static_cast<QgsMarkerSymbolLayer *>( layer );
    if ( !markerLayer )
    {
      graphicStrokeElem.appendChild( doc.createComment( QString( "MarkerSymbolLayerV2 expected, %1 found. Skip it." ).arg( layer->layerType() ) ) );
    }
    else
    {
      markerLayer->writeSldMarker( doc, graphicStrokeElem, props );
    }

    if ( !gap.isEmpty() )
    {
      QDomElement gapElem = doc.createElement( "se:Gap" );
      QgsSymbolLayerUtils::createExpressionElement( doc, gapElem, gap );
      graphicStrokeElem.appendChild( gapElem );
    }

    if ( !qgsDoubleNear( mOffset, 0.0 ) )
    {
      QDomElement perpOffsetElem = doc.createElement( "se:PerpendicularOffset" );
      double offset = QgsSymbolLayerUtils::rescaleUom( mOffset, mOffsetUnit, props );
      perpOffsetElem.appendChild( doc.createTextNode( qgsDoubleToString( offset ) ) );
      symbolizerElem.appendChild( perpOffsetElem );
    }
  }
}

QgsSymbolLayer* QgsMarkerLineSymbolLayer::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( strokeElem.isNull() )
    return nullptr;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( "GraphicStroke" );
  if ( graphicStrokeElem.isNull() )
    return nullptr;

  // retrieve vendor options
  bool rotateMarker = true;
  Placement placement = Interval;

  QgsStringMap vendorOptions = QgsSymbolLayerUtils::getVendorOptionList( element );
  for ( QgsStringMap::iterator it = vendorOptions.begin(); it != vendorOptions.end(); ++it )
  {
    if ( it.key() == "placement" )
    {
      if ( it.value() == "points" ) placement = Vertex;
      else if ( it.value() == "firstPoint" ) placement = FirstVertex;
      else if ( it.value() == "lastPoint" ) placement = LastVertex;
      else if ( it.value() == "centralPoint" ) placement = CentralPoint;
    }
    else if ( it.value() == "rotateMarker" )
    {
      rotateMarker = it.value() == "0";
    }
  }

  QgsMarkerSymbol *marker = nullptr;

  QgsSymbolLayer *l = QgsSymbolLayerUtils::createMarkerLayerFromSld( graphicStrokeElem );
  if ( l )
  {
    QgsSymbolLayerList layers;
    layers.append( l );
    marker = new QgsMarkerSymbol( layers );
  }

  if ( !marker )
    return nullptr;

  double interval = 0.0;
  QDomElement gapElem = graphicStrokeElem.firstChildElement( "Gap" );
  if ( !gapElem.isNull() )
  {
    bool ok;
    double d = gapElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      interval = d;
  }

  double offset = 0.0;
  QDomElement perpOffsetElem = graphicStrokeElem.firstChildElement( "PerpendicularOffset" );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QgsMarkerLineSymbolLayer* x = new QgsMarkerLineSymbolLayer( rotateMarker );
  x->setPlacement( placement );
  x->setInterval( interval );
  x->setSubSymbol( marker );
  x->setOffset( offset );
  return x;
}

void QgsMarkerLineSymbolLayer::setWidth( double width )
{
  mMarker->setSize( width );
}

void QgsMarkerLineSymbolLayer::setDataDefinedProperty( const QString& property, QgsDataDefined* dataDefined )
{
  if ( property == QgsSymbolLayer::EXPR_WIDTH && mMarker && dataDefined )
  {
    mMarker->setDataDefinedSize( *dataDefined );
  }
  QgsLineSymbolLayer::setDataDefinedProperty( property, dataDefined );
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

QSet<QString> QgsMarkerLineSymbolLayer::usedAttributes() const
{
  QSet<QString> attr = QgsLineSymbolLayer::usedAttributes();
  if ( mMarker )
    attr.unite( mMarker->usedAttributes() );
  return attr;
}

double QgsMarkerLineSymbolLayer::estimateMaxBleed() const
{
  return ( mMarker->size() / 2.0 ) + mOffset;
}



