/***************************************************************************
                             qgsannotation.cpp
                             -----------------
    begin                : January 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotation.h"
#include "qgssymbollayerutils.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include <QPen>
#include <QPainter>

QgsAnnotation::QgsAnnotation( QObject* parent )
    : QObject( parent )
    , mMarkerSymbol( new QgsMarkerSymbol() )
{

}

void QgsAnnotation::setVisible( bool visible )
{
  if ( mVisible == visible )
    return;

  mVisible = visible;
  emit appearanceChanged();
}

void QgsAnnotation::setHasFixedMapPosition( bool fixed )
{
  if ( mHasFixedMapPosition == fixed )
    return;

  mHasFixedMapPosition = fixed;
  updateBalloon();
  emit moved();
}

void QgsAnnotation::setMapPosition( const QgsPoint& position )
{
  mMapPosition = position;
  emit moved();
}

void QgsAnnotation::setMapPositionCrs( const QgsCoordinateReferenceSystem& crs )
{
  mMapPositionCrs = crs;
  emit moved();
}

void QgsAnnotation::setRelativePosition( QPointF position )
{
  mRelativePosition = position;
  emit moved();
}

void QgsAnnotation::setFrameOffsetFromReferencePoint( QPointF offset )
{
  mOffsetFromReferencePoint = offset;
  updateBalloon();
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setFrameSize( QSizeF size )
{
  QSizeF frameSize = minimumFrameSize().expandedTo( size ); //don't allow frame sizes below minimum
  mFrameSize = frameSize;
  updateBalloon();
  emit moved();
  emit appearanceChanged();
}

void QgsAnnotation::setContentsMargin( const QgsMargins& margins )
{
  mContentsMargins = margins;
  emit appearanceChanged();
}

void QgsAnnotation::setFrameBorderWidth( double width )
{
  mFrameBorderWidth = width;
  emit appearanceChanged();
}

void QgsAnnotation::setFrameColor( const QColor& c )
{
  mFrameColor = c;
  emit appearanceChanged();
}

void QgsAnnotation::setFrameBackgroundColor( const QColor& c )
{
  mFrameBackgroundColor = c;
  emit appearanceChanged();
}

void QgsAnnotation::render( QgsRenderContext& context ) const
{
  QPainter* painter = context.painter();
  if ( !painter )
  {
    return;
  }

  painter->save();
  drawFrame( context );
  if ( mHasFixedMapPosition )
  {
    drawMarkerSymbol( context );
  }
  if ( mHasFixedMapPosition )
  {
    painter->translate( mOffsetFromReferencePoint.x() + context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        mOffsetFromReferencePoint.y() + context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  else
  {
    painter->translate( context.convertToPainterUnits( mContentsMargins.left(), QgsUnitTypes::RenderMillimeters ),
                        context.convertToPainterUnits( mContentsMargins.top(), QgsUnitTypes::RenderMillimeters ) );
  }
  QSizeF size( mFrameSize.width() - context.convertToPainterUnits( mContentsMargins.left() + mContentsMargins.right(), QgsUnitTypes::RenderMillimeters ),
               mFrameSize.height() - context.convertToPainterUnits( mContentsMargins.top() + mContentsMargins.bottom(), QgsUnitTypes::RenderMillimeters ) );

  renderAnnotation( context, size );
  painter->restore();
}

void QgsAnnotation::setMarkerSymbol( QgsMarkerSymbol* symbol )
{
  mMarkerSymbol.reset( symbol );
  emit appearanceChanged();
}

void QgsAnnotation::setMapLayer( QgsMapLayer* layer )
{
  mMapLayer = layer;
  emit mapLayerChanged();
}

void QgsAnnotation::updateBalloon()
{
  //first test if the point is in the frame. In that case we don't need a balloon.
  if ( !mHasFixedMapPosition ||
       ( mOffsetFromReferencePoint.x() < 0 && ( mOffsetFromReferencePoint.x() + mFrameSize.width() ) > 0
         && mOffsetFromReferencePoint.y() < 0 && ( mOffsetFromReferencePoint.y() + mFrameSize.height() ) > 0 ) )
  {
    mBalloonSegment = -1;
    return;
  }

  //edge list
  QList<QLineF> segmentList;
  segmentList << segment( 0 );
  segmentList << segment( 1 );
  segmentList << segment( 2 );
  segmentList << segment( 3 );

  //find  closest edge / closest edge point
  double minEdgeDist = DBL_MAX;
  int minEdgeIndex = -1;
  QLineF minEdge;
  QgsPoint minEdgePoint;
  QgsPoint origin( 0, 0 );

  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segmentList.at( i );
    QgsPoint currentMinDistPoint;
    double currentMinDist = origin.sqrDistToSegment( currentSegment.x1(), currentSegment.y1(), currentSegment.x2(), currentSegment.y2(), currentMinDistPoint );
    if ( currentMinDist < minEdgeDist )
    {
      minEdgeIndex = i;
      minEdgePoint = currentMinDistPoint;
      minEdgeDist = currentMinDist;
      minEdge = currentSegment;
    }
  }

  if ( minEdgeIndex < 0 )
  {
    return;
  }

  //make that configurable for the item
  double segmentPointWidth = 10;

  mBalloonSegment = minEdgeIndex;
  QPointF minEdgeEnd = minEdge.p2();
  mBalloonSegmentPoint1 = QPointF( minEdgePoint.x(), minEdgePoint.y() );
  if ( sqrt( minEdgePoint.sqrDist( minEdgeEnd.x(), minEdgeEnd.y() ) ) < segmentPointWidth )
  {
    mBalloonSegmentPoint1 = pointOnLineWithDistance( minEdge.p2(), minEdge.p1(), segmentPointWidth );
  }

  mBalloonSegmentPoint2 = pointOnLineWithDistance( mBalloonSegmentPoint1, minEdge.p2(), 10 );
}

QLineF QgsAnnotation::segment( int index ) const
{
  if ( mHasFixedMapPosition )
  {
    switch ( index )
    {
      case 0:
        return QLineF( mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y(), mOffsetFromReferencePoint.x()
                       + mFrameSize.width(), mOffsetFromReferencePoint.y() );
      case 1:
        return QLineF( mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y(),
                       mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y() + mFrameSize.height() );
      case 2:
        return QLineF( mOffsetFromReferencePoint.x() + mFrameSize.width(), mOffsetFromReferencePoint.y() + mFrameSize.height(),
                       mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() + mFrameSize.height() );
      case 3:
        return QLineF( mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() + mFrameSize.height(),
                       mOffsetFromReferencePoint.x(), mOffsetFromReferencePoint.y() );
      default:
        return QLineF();
    }
  }
  else
  {
    switch ( index )
    {
      case 0:
        return QLineF( 0, 0, mFrameSize.width(), 0 );
      case 1:
        return QLineF( mFrameSize.width(), 0,
                       mFrameSize.width(), mFrameSize.height() );
      case 2:
        return QLineF( mFrameSize.width(), mFrameSize.height(),
                       0, mFrameSize.height() );
      case 3:
        return QLineF( 0, mFrameSize.height(),
                       0, 0 );
      default:
        return QLineF();
    }
  }
}


QPointF QgsAnnotation::pointOnLineWithDistance( QPointF startPoint, QPointF directionPoint, double distance ) const
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );
  double scaleFactor = distance / length;
  return QPointF( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}

void QgsAnnotation::drawFrame( QgsRenderContext& context ) const
{
  QPen framePen( mFrameColor );
  framePen.setWidthF( context.convertToPainterUnits( mFrameBorderWidth, QgsUnitTypes::RenderPixels ) );

  QPainter* p = context.painter();

  p->setPen( framePen );
  QBrush frameBrush( mFrameBackgroundColor );
  p->setBrush( frameBrush );
  p->setRenderHint( QPainter::Antialiasing, context.flags() & QgsRenderContext::Antialiasing );

  QPolygonF poly;
  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segment( i );
    poly << currentSegment.p1();
    if ( i == mBalloonSegment && mHasFixedMapPosition )
    {
      poly << mBalloonSegmentPoint1;
      poly << QPointF( 0, 0 );
      poly << mBalloonSegmentPoint2;
    }
    poly << currentSegment.p2();
  }
  p->drawPolygon( poly );
}

void QgsAnnotation::drawMarkerSymbol( QgsRenderContext& context ) const
{
  if ( !context.painter() )
  {
    return;
  }

  if ( mMarkerSymbol )
  {
    mMarkerSymbol->startRender( context );
    mMarkerSymbol->renderPoint( QPointF( 0, 0 ), nullptr, context );
    mMarkerSymbol->stopRender( context );
  }
}

void QgsAnnotation::_writeXml( QDomElement& itemElem, QDomDocument& doc ) const
{
  if ( itemElem.isNull() )
  {
    return;
  }
  QDomElement annotationElem = doc.createElement( QStringLiteral( "AnnotationItem" ) );
  annotationElem.setAttribute( QStringLiteral( "mapPositionFixed" ), mHasFixedMapPosition );
  annotationElem.setAttribute( QStringLiteral( "mapPosX" ), qgsDoubleToString( mMapPosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "mapPosY" ), qgsDoubleToString( mMapPosition.y() ) );
  if ( mMapPositionCrs.isValid() )
    mMapPositionCrs.writeXml( annotationElem, doc );
  annotationElem.setAttribute( QStringLiteral( "offsetX" ), qgsDoubleToString( mOffsetFromReferencePoint.x() ) );
  annotationElem.setAttribute( QStringLiteral( "offsetY" ), qgsDoubleToString( mOffsetFromReferencePoint.y() ) );
  annotationElem.setAttribute( QStringLiteral( "frameWidth" ), qgsDoubleToString( mFrameSize.width() ) );
  annotationElem.setAttribute( QStringLiteral( "frameHeight" ), qgsDoubleToString( mFrameSize.height() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosX" ), qgsDoubleToString( mRelativePosition.x() ) );
  annotationElem.setAttribute( QStringLiteral( "canvasPosY" ), qgsDoubleToString( mRelativePosition.y() ) );
  annotationElem.setAttribute( QStringLiteral( "frameBorderWidth" ), qgsDoubleToString( mFrameBorderWidth ) );
  annotationElem.setAttribute( QStringLiteral( "contentsMargin" ), mContentsMargins.toString() );
  annotationElem.setAttribute( QStringLiteral( "frameColor" ), mFrameColor.name() );
  annotationElem.setAttribute( QStringLiteral( "frameColorAlpha" ), mFrameColor.alpha() );
  annotationElem.setAttribute( QStringLiteral( "frameBackgroundColor" ), mFrameBackgroundColor.name() );
  annotationElem.setAttribute( QStringLiteral( "frameBackgroundColorAlpha" ), mFrameBackgroundColor.alpha() );
  annotationElem.setAttribute( QStringLiteral( "visible" ), isVisible() );
  if ( mMapLayer )
  {
    annotationElem.setAttribute( QStringLiteral( "mapLayer" ), mMapLayer->id() );
  }
  if ( mMarkerSymbol )
  {
    QDomElement symbolElem = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "marker symbol" ), mMarkerSymbol.data(), doc );
    if ( !symbolElem.isNull() )
    {
      annotationElem.appendChild( symbolElem );
    }
  }
  itemElem.appendChild( annotationElem );
}

void QgsAnnotation::_readXml( const QDomElement& annotationElem, const QDomDocument& doc )
{
  Q_UNUSED( doc );
  if ( annotationElem.isNull() )
  {
    return;
  }
  QPointF pos;
  pos.setX( annotationElem.attribute( QStringLiteral( "canvasPosX" ), QStringLiteral( "0" ) ).toDouble() );
  pos.setY( annotationElem.attribute( QStringLiteral( "canvasPosY" ), QStringLiteral( "0" ) ).toDouble() );
  mRelativePosition = pos;
  QgsPoint mapPos;
  mapPos.setX( annotationElem.attribute( QStringLiteral( "mapPosX" ), QStringLiteral( "0" ) ).toDouble() );
  mapPos.setY( annotationElem.attribute( QStringLiteral( "mapPosY" ), QStringLiteral( "0" ) ).toDouble() );
  mMapPosition = mapPos;

  if ( !mMapPositionCrs.readXml( annotationElem ) )
  {
    mMapPositionCrs = QgsCoordinateReferenceSystem();
  }

  mFrameBorderWidth = annotationElem.attribute( QStringLiteral( "frameBorderWidth" ), QStringLiteral( "0.5" ) ).toDouble();
  mContentsMargins = QgsMargins::fromString( annotationElem.attribute( QStringLiteral( "contentsMargin" ) ) );
  mFrameColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameColor" ), QStringLiteral( "#000000" ) ) );
  mFrameColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
  mFrameBackgroundColor.setNamedColor( annotationElem.attribute( QStringLiteral( "frameBackgroundColor" ) ) );
  mFrameBackgroundColor.setAlpha( annotationElem.attribute( QStringLiteral( "frameBackgroundColorAlpha" ), QStringLiteral( "255" ) ).toInt() );
  mFrameSize.setWidth( annotationElem.attribute( QStringLiteral( "frameWidth" ), QStringLiteral( "50" ) ).toDouble() );
  mFrameSize.setHeight( annotationElem.attribute( QStringLiteral( "frameHeight" ), QStringLiteral( "50" ) ).toDouble() );
  mOffsetFromReferencePoint.setX( annotationElem.attribute( QStringLiteral( "offsetX" ), QStringLiteral( "0" ) ).toDouble() );
  mOffsetFromReferencePoint.setY( annotationElem.attribute( QStringLiteral( "offsetY" ), QStringLiteral( "0" ) ).toDouble() );
  mHasFixedMapPosition = annotationElem.attribute( QStringLiteral( "mapPositionFixed" ), QStringLiteral( "1" ) ).toInt();
  mVisible = annotationElem.attribute( QStringLiteral( "visible" ), QStringLiteral( "1" ) ).toInt();
  if ( annotationElem.hasAttribute( QStringLiteral( "mapLayer" ) ) )
  {
    mMapLayer = QgsProject::instance()->mapLayer( annotationElem.attribute( QStringLiteral( "mapLayer" ) ) );
  }

  //marker symbol
  QDomElement symbolElem = annotationElem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !symbolElem.isNull() )
  {
    QgsMarkerSymbol* symbol = QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( symbolElem );
    if ( symbol )
    {
      mMarkerSymbol.reset( symbol );
    }
  }

  updateBalloon();
  emit mapLayerChanged();
}

