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
#include "qgssymbollayerv2utils.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"

#include <QPainter>
#include <QDomDocument>
#include <QDomElement>

#include <cmath>

QgsSimpleLineSymbolLayerV2::QgsSimpleLineSymbolLayerV2( QColor color, double width, Qt::PenStyle penStyle )
    : mPenStyle( penStyle ), mPenJoinStyle( DEFAULT_SIMPLELINE_JOINSTYLE ), mPenCapStyle( DEFAULT_SIMPLELINE_CAPSTYLE ), mOffset( 0 ), mOffsetUnit( QgsSymbolV2::MM ),
    mUseCustomDashPattern( false ), mCustomDashPatternUnit( QgsSymbolV2::MM )
{
  mColor = color;
  mWidth = width;
  mCustomDashVector << 5 << 2;
}

void QgsSimpleLineSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mWidthUnit = unit;
  mOffsetUnit = unit;
  mCustomDashPatternUnit = unit;
}

QgsSymbolV2::OutputUnit  QgsSimpleLineSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mWidthUnit;
  if ( mOffsetUnit != unit || mCustomDashPatternUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}


QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_SIMPLELINE_COLOR;
  double width = DEFAULT_SIMPLELINE_WIDTH;
  Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "width" ) )
    width = props["width"].toDouble();
  if ( props.contains( "penstyle" ) )
    penStyle = QgsSymbolLayerV2Utils::decodePenStyle( props["penstyle"] );


  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( color, width, penStyle );
  if ( props.contains( "width_unit" ) )
    l->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["width_unit"] ) );
  if ( props.contains( "offset" ) )
    l->setOffset( props["offset"].toDouble() );
  if ( props.contains( "offset_unit" ) )
    l->setOffsetUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["offset_unit"] ) );
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

  //data defined properties
  if ( props.contains( "color_expression" ) )
    l->setDataDefinedProperty( "color", props["color_expression"] );
  if ( props.contains( "width_expression" ) )
    l->setDataDefinedProperty( "width", props["width_expression"] );
  if ( props.contains( "offset_expression" ) )
    l->setDataDefinedProperty( "offset", props["offset_expression"] );
  if ( props.contains( "customdash_expression" ) )
    l->setDataDefinedProperty( "customdash", props["customdash_expression"] );
  if ( props.contains( "joinstyle_expression" ) )
    l->setDataDefinedProperty( "joinstyle", props["joinstyle_expression"] );
  if ( props.contains( "capstyle_expression" ) )
    l->setDataDefinedProperty( "capstyle", props["capstyle_expression"] );

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
  double scaledWidth = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit );
  mPen.setWidthF( scaledWidth );
  if ( mUseCustomDashPattern && scaledWidth != 0 )
  {
    mPen.setStyle( Qt::CustomDashLine );

    //scale pattern vector
    QVector<qreal> scaledVector;
    QVector<qreal>::const_iterator it = mCustomDashVector.constBegin();
    for ( ; it != mCustomDashVector.constEnd(); ++it )
    {
      //the dash is specified in terms of pen widths, therefore the division
      scaledVector << ( *it ) *  QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mCustomDashPatternUnit ) / scaledWidth;
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
  prepareExpressions( context.layer() );
}

void QgsSimpleLineSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsSimpleLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  double offset = 0.0;
  applyDataDefinedSymbology( context, mPen, mSelPen, offset );

  p->setPen( context.selected() ? mSelPen : mPen );

  if ( offset == 0 )
  {
    p->drawPolyline( points );
  }
  else
  {
    double scaledOffset = offset * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit );
    p->drawPolyline( ::offsetLine( points, scaledOffset ) );
  }
}

QgsStringMap QgsSimpleLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["width"] = QString::number( mWidth );
  map["width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mWidthUnit );
  map["penstyle"] = QgsSymbolLayerV2Utils::encodePenStyle( mPenStyle );
  map["joinstyle"] = QgsSymbolLayerV2Utils::encodePenJoinStyle( mPenJoinStyle );
  map["capstyle"] = QgsSymbolLayerV2Utils::encodePenCapStyle( mPenCapStyle );
  map["offset"] = QString::number( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["use_custom_dash"] = ( mUseCustomDashPattern ? "1" : "0" );
  map["customdash"] = QgsSymbolLayerV2Utils::encodeRealVector( mCustomDashVector );
  map["customdash_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mCustomDashPatternUnit );
  saveDataDefinedProperties( map );
  return map;
}

QgsSymbolLayerV2* QgsSimpleLineSymbolLayerV2::clone() const
{
  QgsSimpleLineSymbolLayerV2* l = new QgsSimpleLineSymbolLayerV2( mColor, mWidth, mPenStyle );
  l->setWidthUnit( mWidthUnit );
  l->setOffsetUnit( mOffsetUnit );
  l->setCustomDashPatternUnit( mCustomDashPatternUnit );
  l->setOffset( mOffset );
  l->setPenJoinStyle( mPenJoinStyle );
  l->setPenCapStyle( mPenCapStyle );
  l->setUseCustomDashPattern( mUseCustomDashPattern );
  l->setCustomDashVector( mCustomDashVector );
  copyDataDefinedProperties( l );
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

void QgsSimpleLineSymbolLayerV2::applyDataDefinedSymbology( QgsSymbolV2RenderContext& context, QPen& pen, QPen& selPen, double& offset )
{
  //data defined properties
  double scaledWidth = 0;
  QgsExpression* strokeWidthExpression = expression( "width" );
  if ( strokeWidthExpression )
  {
    scaledWidth = strokeWidthExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble()
                  * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
  }
  else if ( context.renderHints() & QgsSymbolV2::DataDefinedSizeScale )
  {
    scaledWidth = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit );
    pen.setWidthF( scaledWidth );
    selPen.setWidthF( scaledWidth );
  }

  //color
  QgsExpression* strokeColorExpression = expression( "color" );
  if ( strokeColorExpression )
  {
    pen.setColor( QgsSymbolLayerV2Utils::decodeColor( strokeColorExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString() ) );
  }

  //offset
  offset = mOffset;
  QgsExpression* lineOffsetExpression = expression( "offset" );
  if ( lineOffsetExpression )
  {
    offset = lineOffsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }

  //dash dot vector
  QgsExpression* dashPatternExpression = expression( "customdash" );
  if ( dashPatternExpression )
  {
    QVector<qreal> dashVector;
    QStringList dashList = dashPatternExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString().split( ";" );
    QStringList::const_iterator dashIt = dashList.constBegin();
    for ( ; dashIt != dashList.constEnd(); ++dashIt )
    {
      dashVector.push_back( dashIt->toDouble() * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mCustomDashPatternUnit ) / mPen.widthF() );
    }
    pen.setDashPattern( dashVector );
  }

  //join style
  QgsExpression* joinStyleExpression = expression( "joinstyle" );
  if ( joinStyleExpression )
  {
    QString joinStyleString = joinStyleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    pen.setJoinStyle( QgsSymbolLayerV2Utils::decodePenJoinStyle( joinStyleString ) );
  }

  //cap style
  QgsExpression* capStyleExpression = expression( "capstyle" );
  if ( capStyleExpression )
  {
    QString capStyleString = capStyleExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
    pen.setCapStyle( QgsSymbolLayerV2Utils::decodePenCapStyle( capStyleString ) );
  }
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
  mOffset = 0;
  mOffsetUnit = QgsSymbolV2::MM;
  mPlacement = Interval;

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

  //data defined properties
  if ( props.contains( "interval_expression" ) )
  {
    x->setDataDefinedProperty( "interval", props["interval_expression"] );
  }
  if ( props.contains( "offset_expression" ) )
  {
    x->setDataDefinedProperty( "offset", props["offset_expression"] );
  }
  if ( props.contains( "placement_expression" ) )
  {
    x->setDataDefinedProperty( "placement", props["placement_expression"] );
  }

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

  mMarker->startRender( context.renderContext(), context.layer() );

  //prepare expressions for data defined properties
  prepareExpressions( context.layer() );
}

void QgsMarkerLineSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  mMarker->stopRender( context.renderContext() );
}

void QgsMarkerLineSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  double offset = mOffset;
  QgsExpression* offsetExpression = expression( "offset" );
  if ( offsetExpression )
  {
    offset = offsetExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }

  Placement placement = mPlacement;
  QgsExpression* placementExpression = expression( "placement" );
  if ( placementExpression )
  {
    QString placementString = placementExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toString();
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
    QPolygonF points2 = ::offsetLine( points, offset * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mOffsetUnit ) );
    if ( placement == Interval )
      renderPolylineInterval( points2, context );
    else if ( placement == CentralPoint )
      renderPolylineCentral( points2, context );
    else
      renderPolylineVertex( points2, context, placement );
  }
}

void QgsMarkerLineSymbolLayerV2::renderPolylineInterval( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  if ( points.isEmpty() )
    return;

  QPointF lastPt = points[0];
  double lengthLeft = 0; // how much is left until next marker
  bool first = true;
  double origAngle = mMarker->angle();

  QgsRenderContext& rc = context.renderContext();
  double interval = mInterval;

  QgsExpression* intervalExpression = expression( "interval" );
  if ( intervalExpression )
  {
    interval = intervalExpression->evaluate( const_cast<QgsFeature*>( context.feature() ) ).toDouble();
  }
  if ( interval <= 0 )
  {
    interval = 0.1;
  }

  double painterUnitInterval = interval * QgsSymbolLayerV2Utils::lineWidthScaleFactor( rc, mIntervalUnit );

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
      mMarker->setAngle( origAngle + ( l.angle() * 180 / M_PI ) );
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

  // restore original rotation
  mMarker->setAngle( origAngle );

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
  double angle;
  int i, maxCount;
  bool isRing = false;

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

  for ( ; i < maxCount; ++i )
  {
    const QPointF& pt = points[i];

    // rotate marker (if desired)
    if ( mRotateMarker )
    {
      if ( i == 0 )
      {
        if ( !isRing )
        {
          // use first segment's angle
          const QPointF& nextPt = points[i+1];
          if ( pt == nextPt )
            continue;
          angle = MyLine( pt, nextPt ).angle();
        }
        else
        {
          // closed ring: use average angle between first and last segment
          const QPointF& prevPt = points[points.count() - 2];
          const QPointF& nextPt = points[1];
          if ( prevPt == pt || nextPt == pt )
            continue;

          angle = _averageAngle( prevPt, pt, nextPt );
        }
      }
      else if ( i == points.count() - 1 )
      {
        if ( !isRing )
        {
          // use last segment's angle
          const QPointF& prevPt = points[i-1];
          if ( pt == prevPt )
            continue;
          angle = MyLine( prevPt, pt ).angle();
        }
        else
        {
          // don't draw the last marker - it has been drawn already
          continue;
        }
      }
      else
      {
        // use average angle
        const QPointF& prevPt = points[i-1];
        const QPointF& nextPt = points[i+1];
        if ( prevPt == pt || nextPt == pt )
          continue;

        angle = _averageAngle( prevPt, pt, nextPt );
      }
      mMarker->setAngle( origAngle + angle * 180 / M_PI );
    }

    mMarker->renderPoint( points.at( i ), context.feature(), rc, -1, context.selected() );
  }

  // restore original rotation
  mMarker->setAngle( origAngle );
}

void QgsMarkerLineSymbolLayerV2::renderPolylineCentral( const QPolygonF& points, QgsSymbolV2RenderContext& context )
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


QgsStringMap QgsMarkerLineSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["rotate"] = ( mRotateMarker ? "1" : "0" );
  map["interval"] = QString::number( mInterval );
  map["offset"] = QString::number( mOffset );
  map["offset_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mOffsetUnit );
  map["interval_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mIntervalUnit );
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
  x->setIntervalUnit( mIntervalUnit );
  copyDataDefinedProperties( x );
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
      graphicStrokeElem.appendChild( doc.createComment( QString( "MarkerSymbolLayerV2 expected, %1 found. Skip it." ).arg( markerLayer->layerType() ) ) );
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
  mIntervalUnit = unit;
  mOffsetUnit = unit;
}

QgsSymbolV2::OutputUnit QgsMarkerLineSymbolLayerV2::outputUnit() const
{
  QgsSymbolV2::OutputUnit unit = mIntervalUnit;
  if ( mOffsetUnit != unit )
  {
    return QgsSymbolV2::Mixed;
  }
  return unit;
}

/////////////

QgsLineDecorationSymbolLayerV2::QgsLineDecorationSymbolLayerV2( QColor color, double width )
{
  mColor = color;
  mWidth = width;
}

QgsLineDecorationSymbolLayerV2::~QgsLineDecorationSymbolLayerV2()
{
}

QgsSymbolLayerV2* QgsLineDecorationSymbolLayerV2::create( const QgsStringMap& props )
{
  QColor color = DEFAULT_LINEDECORATION_COLOR;
  double width = DEFAULT_LINEDECORATION_WIDTH;

  if ( props.contains( "color" ) )
    color = QgsSymbolLayerV2Utils::decodeColor( props["color"] );
  if ( props.contains( "width" ) )
    width = props["width"].toDouble();


  QgsLineDecorationSymbolLayerV2* layer = new QgsLineDecorationSymbolLayerV2( color, width );
  if ( props.contains( "width_unit" ) )
  {
    layer->setWidthUnit( QgsSymbolLayerV2Utils::decodeOutputUnit( props["width_unit"] ) );
  }
  return layer;
}

QString QgsLineDecorationSymbolLayerV2::layerType() const
{
  return "LineDecoration";
}

void QgsLineDecorationSymbolLayerV2::startRender( QgsSymbolV2RenderContext& context )
{
  QColor penColor = mColor;
  penColor.setAlphaF( mColor.alphaF() * context.alpha() );

  double width = mWidth * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit );
  mPen.setWidth( width );
  mPen.setColor( penColor );
  QColor selColor = context.renderContext().selectionColor();
  if ( ! selectionIsOpaque )
    selColor.setAlphaF( context.alpha() );
  mSelPen.setWidth( context.outputLineWidth( width ) );
  mSelPen.setColor( selColor );
}

void QgsLineDecorationSymbolLayerV2::stopRender( QgsSymbolV2RenderContext& context )
{
  Q_UNUSED( context );
}

void QgsLineDecorationSymbolLayerV2::renderPolyline( const QPolygonF& points, QgsSymbolV2RenderContext& context )
{
  // draw arrow at the end of line

  QPainter* p = context.renderContext().painter();
  if ( !p )
  {
    return;
  }

  int cnt = points.count();
  if ( cnt < 2 )
  {
    return;
  }
  QPointF p2 = points.at( --cnt );
  QPointF p1 = points.at( --cnt );
  while ( p2 == p1 && cnt )
    p1 = points.at( --cnt );
  if ( p1 == p2 )
  {
    // this is a collapsed line... don't bother drawing an arrow
    // with arbitrary orientation
    return;
  }

  double angle = atan2( p2.y() - p1.y(), p2.x() - p1.x() );
  double size = ( mWidth * 8 ) * QgsSymbolLayerV2Utils::lineWidthScaleFactor( context.renderContext(), mWidthUnit );
  double angle1 = angle + M_PI / 6;
  double angle2 = angle - M_PI / 6;

  QPointF p2_1 = p2 - QPointF( size * cos( angle1 ), size * sin( angle1 ) );
  QPointF p2_2 = p2 - QPointF( size * cos( angle2 ), size * sin( angle2 ) );

  p->setPen( context.selected() ? mSelPen : mPen );
  p->drawLine( p2, p2_1 );
  p->drawLine( p2, p2_2 );
}

QgsStringMap QgsLineDecorationSymbolLayerV2::properties() const
{
  QgsStringMap map;
  map["color"] = QgsSymbolLayerV2Utils::encodeColor( mColor );
  map["width"] = QString::number( mWidth );
  map["width_unit"] = QgsSymbolLayerV2Utils::encodeOutputUnit( mWidthUnit );
  return map;
}

QgsSymbolLayerV2* QgsLineDecorationSymbolLayerV2::clone() const
{
  QgsLineDecorationSymbolLayerV2* layer = new QgsLineDecorationSymbolLayerV2( mColor, mWidth );
  layer->setWidthUnit( mWidthUnit );
  return layer;
}

void QgsLineDecorationSymbolLayerV2::toSld( QDomDocument &doc, QDomElement &element, QgsStringMap props ) const
{
  QDomElement symbolizerElem = doc.createElement( "se:LineSymbolizer" );
  if ( !props.value( "uom", "" ).isEmpty() )
    symbolizerElem.setAttribute( "uom", props.value( "uom", "" ) );
  element.appendChild( symbolizerElem );

  QgsSymbolLayerV2Utils::createGeometryElement( doc, symbolizerElem, props.value( "geom" , "" ) );

  // <Stroke>
  QDomElement strokeElem = doc.createElement( "se:Stroke" );
  symbolizerElem.appendChild( strokeElem );

  // <GraphicStroke>
  QDomElement graphicStrokeElem = doc.createElement( "se:GraphicStroke" );
  strokeElem.appendChild( graphicStrokeElem );

  // <Graphic>
  QDomElement graphicElem = doc.createElement( "se:Graphic" );
  graphicStrokeElem.appendChild( graphicElem );

  // <Mark>
  QgsSymbolLayerV2Utils::wellKnownMarkerToSld( doc, graphicElem, "arrowhead", QColor(), mColor, mWidth, mWidth*8 );

  // <Rotation>
  QgsSymbolLayerV2Utils::createRotationElement( doc, graphicElem, props.value( "angle", "" ) );

  // use <VendorOption> to draw the decoration at end of the line
  symbolizerElem.appendChild( QgsSymbolLayerV2Utils::createVendorOptionElement( doc, "placement", "lastPoint" ) );
}

void QgsLineDecorationSymbolLayerV2::setOutputUnit( QgsSymbolV2::OutputUnit unit )
{
  mWidthUnit = unit;
}

QgsSymbolV2::OutputUnit QgsLineDecorationSymbolLayerV2::outputUnit() const
{
  return mWidthUnit;
}
