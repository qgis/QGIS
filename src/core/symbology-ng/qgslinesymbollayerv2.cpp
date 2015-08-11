/***************************************************************************
 qgslinesymbollayerv2.cpp
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

#include "qgslinesymbollayerv2.h"
#include "qgsdxfexport.h"
#include "qgssymbollayerv2utils.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsgeometrysimplifier.h"

#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

QgsSimpleLineSymbolLayerV2::QgsSimpleLineSymbolLayerV2( QColor color, double width, Qt::PenStyle penStyle )
    : mPenStyle( penStyle )
    , mPenJoinStyle( DEFAULT_SIMPLELINE_JOINSTYLE )
    , mPenCapStyle( DEFAULT_SIMPLELINE_CAPSTYLE )
    , mUseCustomDashPattern( false )
    , mCustomDashPatternUnit( QgsSymbolV2::MM )
    , mDrawInsidePolygon( false )
{
  mColor = color;
  mWidth = width;
  mCustomDashVector << 5 << 2;
}

void QgsSimpleLineSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsLineSymbolLayerV2::setOutputUnit( unit );
  mWidthUnit = unit;
  mOffsetUnit = unit;
  mCustomDashPatternUnit = unit;
}

QgsSymbolV2::OutputUnit  QgsSimpleLineSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsLineSymbolLayerV2::outputUnit();
  if ( mWidthUnit != unit || mOffsetUnit != unit || mCustomDashPatternUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsSimpleLineSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayerV2::setMapUnitScale( scale );
  mWidthMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
  mCustomDashPatternMapUnitScale = scale;
}

QgsMapUnitScale QgsSimpleLineSymbolLayerV2::mapUnitScale() const
{
  if ( QgsLineSymbolLayerV2::mapUnitScale() == mWidthMapUnitScale &&
       mWidthMapUnitScale == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == mCustomDashPatternMapUnitScale )
  {
    return mWidthMapUnitScale;
  }
  return QgsMapUnitScale();
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( "line_color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( props["line_color"] );
  }
  else if ( props.contains( "outline_color" ) )
  {
    color = QgsSymbolLayerV2Utils::decodeColor( props["outline_color"] );
  }
  else if ( props.contains( "color" ) )
  {
    //pre 2.5 projects used "color"
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
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
    penStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["line_style"] );
  }
  else if ( props.contains( "outline_style" ) )
  {
    penStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["outline_style"] );
  }
  else if ( props.contains( "penstyle" ) )
  {
    penStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["penstyle"] );
  }

  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );
  if ( props.contains( "line_width_unit" ) )
  {
    l->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["line_width_unit"] ) );
  }
  else if ( props.contains( "outline_width_unit" ) )
  {
    l->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["outline_width_unit"] ) );
  }
  else if ( props.contains( "width_unit" ) )
  {
    //pre 2.5 projects used "width_unit"
    l->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["width_unit"] ) );
  }
  if ( props.contains( "width_map_unit_scale" ) )
    l->setWidthMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["width_map_unit_scale"] ) );
  if ( props.contains( "offset" ) )
    l->setOffset( props["offset"].toDouble() );
  if ( props.contains( "offset_unit" ) )
    l->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  if ( props.contains( "offset_map_unit_scale" ) )
    l->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  if ( props.contains( "joinstyle" ) )
    l->setPenJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( props["joinstyle"] ) );
  if ( props.contains( "capstyle" ) )
    l->setPenCapStyle( QgsSymbolLayerV2Utils::decodePenCapStyle( props["capstyle"] ) );

  if ( props.contains( "use_custom_dash" ) )
  {
    l->setUseCustomDashPattern( props["use_custom_dash"].toInt() );
  }
  if ( props.contains( "customdash" ) )
  {
    l->setCustomDashVector( QgsSymbolLayerV2Utils::decodeRealVector( props["customdash"] ) );
  }
  if ( props.contains( "customdash_unit" ) )
  {
    l->setCustomDashPatternUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["customdash_unit"] ) );
  }
  if ( props.contains( "customdash_map_unit_scale" ) )
  {
    l->setCustomDashPatternMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["customdash_map_unit_scale"] ) );
  }

  if ( props.contains( "draw_inside_polygon" ) )
  {
    l->setDrawInsidePolygon( props["draw_inside_polygon"].toInt() );
  }

  l->restoreDataDefinedProperties( props );

  return l;
}


QString QgsSimpleLineSymbolLayerV2::layerType() const
{
  return "SimpleLine";
}

void QgsSimpleLineSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.alpha() );
  mPen.setColor( penColor );
  double scaledWidth = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
  mPen.setWidthF( scaledWidth );
  if ( mUseCustomDashPattern && scaledWidth != 0 )
  {
    mPen.setStyle( Qt::CustomDashLine );

    //scale pattern vector
    double dashWidthDiv = scaledWidth;
    //fix dash pattern width in Qt 4.8
    QStringList versionSplit = QString( qVersion() ).split( "." );
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
      scaledVector << ( *it ) *  QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv;
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

void QgsSimpleLineSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSimpleLineSymbolLayerV2::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
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

    if ( rings != NULL )
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
    foreach ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
    mOffset = -mOffset;
  }

  if ( mDrawInsidePolygon )
  {
    //restore painter to reset clip path
    p->restore();
  }

}

void QgsSimpleLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  //size scaling by field
  if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
  {
    applySizeScale( context, mPen, mSelPen );
  }

  double offset = mOffset;
  applyDataDefinedSymbology( context, mPen, mSelPen, offset );

  p->setPen( context.selected() ? mSelPen : mPen );

  // Disable 'Antialiasing' if the geometry was generalized in the current RenderContext (We known that it must have least #2 points).
  if ( points.size() <= 2 &&
       ( context.renderContext().vectorSimplifyMethod().simplifyHints() & QgsVectorSimplifyMethod::AntialiasingSimplification ) &&
       QgsAbstractGeometrySimplifier::isGeneralizableByDeviceBoundingBox( points, context.renderContext().vectorSimplifyMethod().threshold() ) &&
       ( p->renderHints() & QPainter::Antialiasing ) )
  {
    p->setRenderHint( QPainter::Antialiasing, false );
    p->drawPolyline( points );
    p->setRenderHint( QPainter::Antialiasing, true );
    return;
  }

  if ( qgsDoubleNear( offset, 0 ) )
  {
    p->drawPolyline( points );
  }
  else
  {
    double scaledOffset = offset * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale );
    QList<QPolygonF> mline = ::offsetLine( points, scaledOffset, context.feature() ? context.feature()->constGeometry()->type() : QGis::Line );
    for ( int part = 0; part < mline.count(); ++part )
      p->drawPolyline( mline[ part ] );
  }
}

QgsStringMap QgsSimpleLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["line_color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["line_width"] = QString::number( mWidth );
  map["line_width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mWidthUnit );
  map["width_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mWidthMapUnitScale );
  map["line_style"] = QgsSymbolLayerV2Utils::encodePenStyle( mPenStyle );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["capstyle"] = QgsSymbolLayerV2Utils::encodePenCapStyle( mPenCapStyle );
  map["offset"] = QString::number( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["use_custom_dash"] = ( mUseCustomDashPattern ? "1" : "0" );
  map["customdash"] = QgsSymbolLayerV2Utils::encodeRealVector( mCustomDashVector );
  map["customdash_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mCustomDashPatternUnit );
  map["customdash_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mCustomDashPatternMapUnitScale );
  map["draw_inside_polygon"] = ( mDrawInsidePolygon ? "1" : "0" );
  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::clone() const
{
  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( mColor, mWidth, mPenStyle );
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

void QgsSimpleLineSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  if ( mPenStyle == Qt::NoPen )
    return;

  QDomElement symbolizerElem = doc.createElement( "se:LineSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  // <Geometry>
  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( "se:Stroke" );
  symbolizerElem.appendChild( strokeElem );

  Qt::PenStyle penStyle = mUseCustomDashPattern ? Qt::CustomDashLine : mPenStyle;
  QgsSymbolLayerV2Utils::lineToSld( doc, strokeElem, penStyle, mColor, mWidth,
                                    &mPenJoinStyle, &mPenCapStyle, &mCustomDashVector );

  // <se:PerpendicularOffset>
  if ( mOffset != 0 )
  {
    QDomElement perpOffsetElem = doc.createElement( "se:PerpendicularOffset" );
    perpOffsetElem.appendChild( doc.createTextNode( QString::number( mOffset ) ) );
    symbolizerElem.appendChild( perpOffsetElem );
  }
}

QString QgsSimpleLineSymbolLayerV2::ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const
{
  if ( mUseCustomDashPattern )
  {
    return QgsSymbolLayerV2Utils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor,
           mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset, &mCustomDashVector );
  }
  else
  {
    return QgsSymbolLayerV2Utils::ogrFeatureStylePen( mWidth, mmScaleFactor, mapUnitScaleFactor, mPen.color(), mPenJoinStyle,
           mPenCapStyle, mOffset );
  }
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( strokeElem.isNull() )
    return NULL;

  Qt::PenStyle penStyle;
  QColor color;
  double width;
  Qt::PenJoinStyle penJoinStyle;
  Qt::PenCapStyle penCapStyle;
  QVector<qreal> customDashVector;

  if ( !QgsSymbolLayerV2Utils::lineFromSld( strokeElem, penStyle,
       color, width,
       &penJoinStyle, &penCapStyle,
       &customDashVector ) )
    return NULL;

  double offset = 0.0;
  QDomElement perpOffsetElem = element.firstChildElement( "PerpendicularOffset" );
  if ( !perpOffsetElem.isNull() )
  {
    bool ok;
    double d = perpOffsetElem.firstChild().nodeValue().toDouble( &ok );
    if ( ok )
      offset = d;
  }

  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );
  l->setOffset( offset );
  l->setPenJoinStyle( penJoinStyle );
  l->setPenCapStyle( penCapStyle );
  l->setUseCustomDashPattern( penStyle == Qt::CustomDashLine );
  l->setCustomDashVector( customDashVector );
  return l;
}

void QgsSimpleLineSymbolLayerV2::applySizeScale( QgsSymbolV2RenderContext& context, QPen& pen, QPen& selPen )
{
  double scaledWidth = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
  pen.setWidthF( scaledWidth );
  selPen.setWidthF( scaledWidth );
}

void QgsSimpleLineSymbolLayerV2::applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QPen& pen, QPen& selPen, double& offset )
{
  if ( !hasDataDefinedProperties() )
    return; // shortcut

  //data defined properties
  bool hasStrokeWidthExpression = false;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH ) )
  {
    double scaledWidth = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH, context, mWidth ).toDouble()
                         * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
    hasStrokeWidthExpression = true;
  }

  //color
  bool ok;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) )
  {
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setColor( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
  }

  //offset
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    offset = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context, offset ).toDouble();
  }

  //dash dot vector
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_CUSTOMDASH ) )
  {
    double scaledWidth = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
    double dashWidthDiv = mPen.widthF();

    if ( hasStrokeWidthExpression )
    {
      dashWidthDiv = pen.widthF();
      scaledWidth = pen.widthF();
    }

    //fix dash pattern width in Qt 4.8
    QStringList versionSplit = QString( qVersion() ).split( "." );
    if ( versionSplit.size() > 1
         && versionSplit.at( 1 ).toInt() >= 8
         && ( scaledWidth * context.renderContext().rasterScaleFactor() ) < 1.0 )
    {
      dashWidthDiv = 1.0;
    }

    QVector<qreal> dashVector;
    QStringList dashList = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_CUSTOMDASH, context, QVariant(), &ok ).toString().split( ";" );
    if ( ok )
    {
      QStringList::const_iterator dashIt = dashList.constBegin();
      for ( ; dashIt != dashList.constEnd(); ++dashIt )
      {
        dashVector.push_back( dashIt->toDouble() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mCustomDashPatternUnit, mCustomDashPatternMapUnitScale ) / dashWidthDiv );
      }
      pen.setDashPattern( dashVector );
    }
  }

  //line style
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_LINE_STYLE ) )
  {
    QString lineStyleString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_LINE_STYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setStyle( QgsSymbolLayerV2Utils::decodePenStyle( lineStyleString ) );
  }

  //join style
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOINSTYLE ) )
  {
    QString joinStyleString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_JOINSTYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( joinStyleString ) );
  }

  //cap style
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_CAPSTYLE ) )
  {
    QString capStyleString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_CAPSTYLE, context, QVariant(), &ok ).toString();
    if ( ok )
      pen.setCapStyle( QgsSymbolLayerV2Utils::decodePenCapStyle( capStyleString ) );
  }
}

double QgsSimpleLineSymbolLayerV2::estimateMaxBleed() const
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

QVector<qreal> QgsSimpleLineSymbolLayerV2::dxfCustomDashPattern( QgsSymbolV2::OutputUnit& unit ) const
{
  unit = mCustomDashPatternUnit;
  return mUseCustomDashPattern ? mCustomDashVector : QVector<qreal>();
}

Qt::PenStyle QgsSimpleLineSymbolLayerV2::dxfPenStyle() const
{
  return mPenStyle;
}

double QgsSimpleLineSymbolLayerV2::dxfWidth( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  double width = mWidth;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH ) )
  {
    width = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_WIDTH, context, mWidth ).toDouble() * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
  {
    width = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit, mWidthMapUnitScale );
  }

  return width * e.mapUnitScaleFactor( e.symbologyScaleDenominator(), widthUnit(), e.mapUnits() );
}

QColor QgsSimpleLineSymbolLayerV2::dxfColor( const QgsSymbolV2RenderContext& context ) const
{
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR ) )
  {
    bool ok;
    QString colorString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_COLOR, context, QVariant(), &ok ).toString();
    if ( ok )
      return ( QgsSymbolLayerV2Utils::decodeColor( colorString ) );
  }
  return mColor;
}

double QgsSimpleLineSymbolLayerV2::dxfOffset( const QgsDxfExport& e, const QgsSymbolV2RenderContext& context ) const
{
  Q_UNUSED( e );
  double offset = mOffset;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    offset = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context, mOffset ).toDouble();
  }
  return offset;
}

/////////


class MyLine
{
  public:
    MyLine( QPointF p1, QPointF p2 ) : mVertical( false ), mIncreasing( false ), mT( 0.0 ), mLength( 0.0 )
    {
      if ( p1 == p2 )
        return; // invalid

      // tangent and direction
      if ( p1.x() == p2.x() )
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


QgsMarkerLineSymbolLayerV2::QgsMarkerLineSymbolLayerV2( bool rotateMarker, double interval )
{
  mRotateMarker = rotateMarker;
  mInterval = interval;
  mIntervalUnit = QgsSymbolV2::MM;
  mMarker = NULL;
  mPlacement = Interval;
  mOffsetAlongLine = 0;
  mOffsetAlongLineUnit = QgsSymbolV2::MM;

  setSubSymbol( new QgsMarkerSymbolV2() );
}

QgsMarkerLineSymbolLayerV2::~QgsMarkerLineSymbolLayerV2()
{
  delete mMarker;
}

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2::create( const QgsStringMap& props )
{
  bool rotate = DEFAULT_MARKERLINE_ROTATE;
  double interval = DEFAULT_MARKERLINE_INTERVAL;


  if ( props.contains( "interval" ) )
    interval = props["interval"].toDouble();
  if ( props.contains( "rotate" ) )
    rotate = ( props["rotate"] == "1" );

  QgsMarkerLineSymbolLayerV2* x = new QgsMarkerLineSymbolLayerV2( rotate, interval );
  if ( props.contains( "offset" ) )
  {
    x->setOffset( props["offset"].toDouble() );
  }
  if ( props.contains( "offset_unit" ) )
  {
    x->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
  }
  if ( props.contains( "interval_unit" ) )
  {
    x->setIntervalUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["interval_unit"] ) );
  }
  if ( props.contains( "offset_along_line" ) )
  {
    x->setOffsetAlongLine( props["offset_along_line"].toDouble() );
  }
  if ( props.contains( "offset_along_line_unit" ) )
  {
    x->setOffsetAlongLineUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_along_line_unit"] ) );
  }
  if ( props.contains(( "offset_along_line_map_unit_scale" ) ) )
  {
    x->setOffsetAlongLineMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_along_line_map_unit_scale"] ) );
  }

  if ( props.contains( "offset_map_unit_scale" ) )
  {
    x->setOffsetMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["offset_map_unit_scale"] ) );
  }
  if ( props.contains( "interval_map_unit_scale" ) )
  {
    x->setIntervalMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( props["interval_map_unit_scale"] ) );
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
    else
      x->setPlacement( Interval );
  }

  x->restoreDataDefinedProperties( props );

  return x;
}

QString QgsMarkerLineSymbolLayerV2::layerType() const
{
  return "MarkerLine";
}

void QgsMarkerLineSymbolLayerV2::setColor( const QColor& color )
{
  mMarker->setColor( color );
  mColor = color;
}

void QgsMarkerLineSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  mMarker->setAlpha( context.alpha() );

  // if being rotated, it gets initialized with every line segment
  int hints = 0;
  if ( mRotateMarker )
    hints |= QgsSymbolV2::DataDefinedRotation;
  if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
    hints |= QgsSymbolV2::DataDefinedSizeScale;
  mMarker->setRenderHints( hints );

  mMarker->startRender( context.renderContext(), context.fields() );

  //prepare expressions for data defined properties
  prepareExpressions( context );
}

void QgsMarkerLineSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsMarkerLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  double offset = mOffset;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET ) )
  {
    offset = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET, context, mOffset ).toDouble();
  }

  Placement placement = mPlacement;

  bool ok;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_PLACEMENT ) )
  {
    QString placementString = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_PLACEMENT, context, QVariant(), &ok ).toString();
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
      else
      {
        placement = Interval;
      }
    }
  }

  if ( offset == 0 )
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
    QList<QPolygonF> mline = ::offsetLine( points, offset * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit, mOffsetMapUnitScale ), context.feature() ? context.feature()->constGeometry()->type() : QGis::Line );

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

void QgsMarkerLineSymbolLayerV2::renderPolygonOutline( const QPolygonF& points, QList<QPolygonF>* rings, QgsSymbolV2RenderContext& context )
{
  renderPolyline( points, context );
  if ( rings )
  {
    mOffset = -mOffset; // invert the offset for rings!
    foreach ( const QPolygonF& ring, *rings )
      renderPolyline( ring, context );
    mOffset = -mOffset;
  }
}

void QgsMarkerLineSymbolLayerV2::renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  if ( points.isEmpty() )
    return;

  QPointF lastPt = points[0];
  double lengthLeft = 0; // how much is left until next marker
  bool first = mOffsetAlongLine ? false : true; //only draw marker at first vertex when no offset along line is set

  QgsRenderContext& rc = context.renderContext();
  double interval = mInterval;

  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_INTERVAL ) )
  {
    interval = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_INTERVAL, context, mInterval ).toDouble();
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }
  double offsetAlongLine = mOffsetAlongLine;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET_ALONG_LINE ) )
  {
    offsetAlongLine = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET_ALONG_LINE, context, mOffsetAlongLine ).toDouble();
  }

  double painterUnitInterval = interval * QgsSymbolLayerV2Utils::lineWidthScaleFactor( rc, mIntervalUnit, mIntervalMapUnitScale );
  lengthLeft = painterUnitInterval - offsetAlongLine * QgsSymbolLayerV2Utils::lineWidthScaleFactor( rc, mIntervalUnit, mIntervalMapUnitScale );

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

static double _averageAngle( const QPointF& prevPt, const QPointF& pt, const QPointF& nextPt )
{
  // calc average angle between the previous and next point
  double a1 = MyLine( prevPt, pt ).angle();
  double a2 = MyLine( pt, nextPt ).angle();
  double unitX = cos( a1 ) + cos( a2 ), unitY = sin( a1 ) + sin( a2 );

  return atan2( unitY, unitX );
}

void QgsMarkerLineSymbolLayerV2::renderPolylineVertex( const QPolygonF& points, QgsSymbolV2RenderContext& context, Placement placement )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext& rc = context.renderContext();

  double origAngle = mMarker->angle();
  int i, maxCount;
  bool isRing = false;

  double offsetAlongLine = mOffsetAlongLine;
  if ( hasDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET_ALONG_LINE ) )
  {
    offsetAlongLine = evaluateDataDefinedProperty( QgsSymbolLayerV2::EXPR_OFFSET_ALONG_LINE, context, mOffsetAlongLine ).toDouble();
  }
  if ( offsetAlongLine != 0 )
  {
    //scale offset along line
    offsetAlongLine *= QgsSymbolLayerV2Utils::lineWidthScaleFactor( rc, mOffsetAlongLineUnit, mOffsetAlongLineMapUnitScale );
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
  else
  {
    i = 0;
    maxCount = points.count();
    if ( points.first() == points.last() )
      isRing = true;
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

double QgsMarkerLineSymbolLayerV2::markerAngle( const QPolygonF& points, bool isRing, int vertex )
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

void QgsMarkerLineSymbolLayerV2::renderOffsetVertexAlongLine( const QPolygonF &points, int vertex, double distance, QgsSymbolV2RenderContext& context )
{
  if ( points.isEmpty() )
    return;

  QgsRenderContext& rc = context.renderContext();
  double origAngle = mMarker->angle();
  if ( distance == 0 )
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

void QgsMarkerLineSymbolLayerV2::renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  if ( points.size() > 0 )
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


QgsStringMap QgsMarkerLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["rotate"] = ( mRotateMarker ? "1" : "0" );
  map["interval"] = QString::number( mInterval );
  map["offset"] = QString::number( mOffset );
  map["offset_along_line"] = QString::number( mOffsetAlongLine );
  map["offset_along_line_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetAlongLineUnit );
  map["offset_along_line_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetAlongLineMapUnitScale );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["offset_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mOffsetMapUnitScale );
  map["interval_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mIntervalUnit );
  map["interval_map_unit_scale"] = QgsSymbolLayerV2Utils::encodeMapUnitScale( mIntervalMapUnitScale );
  if ( mPlacement == Vertex )
    map["placement"] = "vertex";
  else if ( mPlacement == LastVertex )
    map["placement"] = "lastvertex";
  else if ( mPlacement == FirstVertex )
    map["placement"] = "firstvertex";
  else if ( mPlacement == CentralPoint )
    map["placement"] = "centralpoint";
  else
    map["placement"] = "interval";

  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolV2* QgsMarkerLineSymbolLayerV2::subSymbol()
{
  return mMarker;
}

bool QgsMarkerLineSymbolLayerV2::setSubSymbol( QgsSymbolV2* symbol )
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

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2::clone() const
{
  QgsMarkerLineSymbolLayerV2* x = new QgsMarkerLineSymbolLayerV2( mRotateMarker, mInterval );
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

void QgsMarkerLineSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  for ( int i = 0; i < mMarker->symbolLayerCount(); i++ )
  {
    QDomElement symbolizerElem = doc.createElement( "se:LineSymbolizer" );
    if ( !props.value( "uom", "" ).isEmpty() )
      symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
    element.appendChild( symbolizerElem );

    // <Geometry>
    QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom", "" ) );

    QString gap;
    switch ( mPlacement )
    {
      case FirstVertex:
        symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "placement", "firstPoint" ) );
        break;
      case LastVertex:
        symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "placement", "lastPoint" ) );
        break;
      case CentralPoint:
        symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "placement", "centralPoint" ) );
        break;
      case Vertex:
        // no way to get line/polygon's vertices, use a VendorOption
        symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "placement", "points" ) );
        break;
      default:
        gap = QString::number( mInterval );
        break;
    }

    if ( !mRotateMarker )
    {
      // markers in LineSymbolizer must be drawn following the line orientation,
      // use a VendorOption when no marker rotation
      symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "rotateMarker", "0" ) );
    }

    // <Stroke>
    QDomElement strokeElem = doc.createElement( "se:Stroke" );
    symbolizerElem.appendChild( strokeElem );

    // <GraphicStroke>
    QDomElement graphicStrokeElem = doc.createElement( "se:GraphicStroke" );
    strokeElem.appendChild( graphicStrokeElem );

    QgsSymbolLayerV2 *layer = mMarker->symbolLayer( i );
    QgsMarkerSymbolLayerV2 *markerLayer = static_cast<QgsMarkerSymbolLayerV2 *>( layer );
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
      QgsSymbolLayerV2Utils::createFunctionElement( doc, gapElem, gap );
      graphicStrokeElem.appendChild( gapElem );
    }

    if ( !qgsDoubleNear( mOffset, 0.0 ) )
    {
      QDomElement perpOffsetElem = doc.createElement( "se:PerpendicularOffset" );
      perpOffsetElem.appendChild( doc.createTextNode( QString::number( mOffset ) ) );
      symbolizerElem.appendChild( perpOffsetElem );
    }
  }
}

QgsSymbolLayerV2* QgsMarkerLineSymbolLayerV2::createFromSld( QDomElement &element )
{
  QgsDebugMsg( "Entered." );

  QDomElement strokeElem = element.firstChildElement( "Stroke" );
  if ( strokeElem.isNull() )
    return NULL;

  QDomElement graphicStrokeElem = strokeElem.firstChildElement( "GraphicStroke" );
  if ( graphicStrokeElem.isNull() )
    return NULL;

  // retrieve vendor options
  bool rotateMarker = true;
  Placement placement = Interval;

  QgsStringMap vendorOptions = QgsSymbolLayerV2Utils::getVendorOptionList( element );
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

  QgsMarkerSymbolV2 *marker = 0;

  QgsSymbolLayerV2 *l = QgsSymbolLayerV2Utils::createMarkerLayerFromSld( graphicStrokeElem );
  if ( l )
  {
    QgsSymbolLayerV2List layers;
    layers.append( l );
    marker = new QgsMarkerSymbolV2( layers );
  }

  if ( !marker )
    return NULL;

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

  QgsMarkerLineSymbolLayerV2* x = new QgsMarkerLineSymbolLayerV2( rotateMarker );
  x->setPlacement( placement );
  x->setInterval( interval );
  x->setSubSymbol( marker );
  x->setOffset( offset );
  return x;
}

void QgsMarkerLineSymbolLayerV2::setWidth( double width )
{
  mMarker->setSize( width );
}

double QgsMarkerLineSymbolLayerV2::width() const
{
  return mMarker->size();
}

void QgsMarkerLineSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  QgsLineSymbolLayerV2::setOutputUnit( unit );
  mIntervalUnit = unit;
  mOffsetUnit = unit;
  mOffsetAlongLineUnit = unit;
}

QgsSymbolV2::OutputUnit QgsMarkerLineSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = QgsLineSymbolLayerV2::outputUnit();
  if ( mIntervalUnit != unit || mOffsetUnit != unit || mOffsetAlongLineUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

void QgsMarkerLineSymbolLayerV2::setMapUnitScale( const QgsMapUnitScale &scale )
{
  QgsLineSymbolLayerV2::setMapUnitScale( scale );
  mIntervalMapUnitScale = scale;
  mOffsetMapUnitScale = scale;
  mOffsetAlongLineMapUnitScale = scale;
}

QgsMapUnitScale QgsMarkerLineSymbolLayerV2::mapUnitScale() const
{
  if ( QgsLineSymbolLayerV2::mapUnitScale() == mIntervalMapUnitScale &&
       mIntervalMapUnitScale == mOffsetMapUnitScale &&
       mOffsetMapUnitScale == mOffsetAlongLineMapUnitScale )
  {
    return mOffsetMapUnitScale;
  }
  return QgsMapUnitScale();
}

QSet<QString> QgsMarkerLineSymbolLayerV2::usedAttributes() const
{
  QSet<QString> attr = QgsLineSymbolLayerV2::usedAttributes();
  if ( mMarker )
    attr.unite( mMarker->usedAttributes() );
  return attr;
}

double QgsMarkerLineSymbolLayerV2::estimateMaxBleed() const
{
  return ( mMarker->size() / 2.0 ) + mOffset;
}



