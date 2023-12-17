/***************************************************************************
                             qgsshapegenerator.cpp
                             ----------------
    begin                : March 2021
    copyright            : (C) 2021 Nyall Dawson
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

#include "qgsshapegenerator.h"
#include "qgsgeometryutils.h"
#include <QLineF>
#include <QList>
#include <QPainterPath>
#include <algorithm>

QLineF segment( int index, QRectF rect, double radius )
{
  const int yMultiplier = rect.height() < 0 ? -1 : 1;
  switch ( index )
  {
    case 0:
      return QLineF( rect.left() + radius,
                     rect.top(),
                     rect.right() - radius,
                     rect.top() );
    case 1:
      return QLineF( rect.right(),
                     rect.top() + yMultiplier * radius,
                     rect.right(),
                     rect.bottom() - yMultiplier * radius );
    case 2:
      return QLineF( rect.right() - radius,
                     rect.bottom(),
                     rect.left() + radius,
                     rect.bottom() );
    case 3:
      return QLineF( rect.left(),
                     rect.bottom() - yMultiplier * radius,
                     rect.left(),
                     rect.top() + yMultiplier * radius );
    default:
      return QLineF();
  }
}

QPolygonF QgsShapeGenerator::createBalloon( const QgsPointXY &origin, const QRectF &rect, double wedgeWidth )
{
  return createBalloon( origin, rect, wedgeWidth, 0 ).toFillPolygon();
}

QPainterPath QgsShapeGenerator::createBalloon( const QgsPointXY &origin, const QRectF &rect, double wedgeWidth, double cornerRadius )
{
  int balloonSegment = -1;
  QPointF balloonSegmentPoint1;
  QPointF balloonSegmentPoint2;

  const bool invertedY = rect.height() < 0;

  cornerRadius = std::min( cornerRadius, std::min( std::fabs( rect.height() ), rect.width() ) / 2.0 );

  //first test if the point is in the frame. In that case we don't need a balloon and can just use a rect
  if ( rect.contains( origin.toQPointF() ) )
  {
    balloonSegment = -1;
  }
  else
  {
    //edge list
    QList<QLineF> segmentList;
    segmentList << segment( 0, rect, cornerRadius );
    segmentList << segment( 1, rect, cornerRadius );
    segmentList << segment( 2, rect, cornerRadius );
    segmentList << segment( 3, rect, cornerRadius );

    // find closest edge / closest edge point
    double minEdgeDist = std::numeric_limits<double>::max();
    int minEdgeIndex = -1;
    QLineF minEdge;
    QgsPointXY minEdgePoint( 0, 0 );

    for ( int i = 0; i < 4; ++i )
    {
      QLineF currentSegment = segmentList.at( i );
      QgsPointXY currentMinDistPoint;
      double currentMinDist = origin.sqrDistToSegment( currentSegment.x1(), currentSegment.y1(), currentSegment.x2(), currentSegment.y2(), currentMinDistPoint );
      bool isPreferredSegment = false;
      if ( qgsDoubleNear( currentMinDist, minEdgeDist ) )
      {
        // two segments are close - work out which looks nicer
        const double angle = fmod( origin.azimuth( currentMinDistPoint ) + 360.0, 360.0 );
        if ( angle < 45 || angle > 315 )
          isPreferredSegment = i == 0;
        else if ( angle < 135 )
          isPreferredSegment = i == 3;
        else if ( angle < 225 )
          isPreferredSegment = i == 2;
        else
          isPreferredSegment = i == 1;
      }
      else if ( currentMinDist < minEdgeDist )
        isPreferredSegment = true;

      if ( isPreferredSegment )
      {
        minEdgeIndex = i;
        minEdgePoint = currentMinDistPoint;
        minEdgeDist = currentMinDist;
        minEdge = currentSegment;
      }
    }

    if ( minEdgeIndex >= 0 )
    {
      balloonSegment = minEdgeIndex;
      QPointF minEdgeEnd = minEdge.p2();
      balloonSegmentPoint1 = QPointF( minEdgePoint.x(), minEdgePoint.y() );

      const double segmentLength = minEdge.length();
      const double clampedWedgeWidth = std::clamp( wedgeWidth, 0.0, segmentLength );
      if ( std::sqrt( minEdgePoint.sqrDist( minEdgeEnd.x(), minEdgeEnd.y() ) ) < clampedWedgeWidth )
      {
        double x = 0;
        double y = 0;
        QgsGeometryUtilsBase::pointOnLineWithDistance( minEdge.p2().x(), minEdge.p2().y(), minEdge.p1().x(), minEdge.p1().y(), clampedWedgeWidth, x, y );
        balloonSegmentPoint1 = QPointF( x, y );
      }

      {
        double x = 0;
        double y = 0;
        QgsGeometryUtilsBase::pointOnLineWithDistance( balloonSegmentPoint1.x(), balloonSegmentPoint1.y(), minEdge.p2().x(), minEdge.p2().y(), clampedWedgeWidth, x, y );
        balloonSegmentPoint2 = QPointF( x, y );
      }
    }
  }

  QPainterPath path;
  QPointF p0;
  QPointF p1;
  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segment( i, rect, cornerRadius );
    if ( i == 0 )
    {
      p0 = currentSegment.p1();
      path.moveTo( currentSegment.p1() );
    }
    else
    {
      if ( invertedY )
        path.arcTo( std::min( p1.x(), currentSegment.p1().x() ),
                    std::min( p1.y(), currentSegment.p1().y() ),
                    cornerRadius, cornerRadius,
                    i == 0 ? -180 : ( i == 1 ? -90 : ( i == 2 ? 0 : 90 ) ),
                    90 );
      else
        path.arcTo( std::min( p1.x(), currentSegment.p1().x() ),
                    std::min( p1.y(), currentSegment.p1().y() ),
                    cornerRadius, cornerRadius,
                    i == 0 ? 180 : ( i == 1 ? 90 : ( i == 2 ? 0 : -90 ) ),
                    -90 );
    }

    if ( i == balloonSegment )
    {
      path.lineTo( balloonSegmentPoint1 );
      path.lineTo( origin.toQPointF() );
      path.lineTo( balloonSegmentPoint2 );
    }

    p1 = currentSegment.p2();
    path.lineTo( p1 );
  }

  if ( invertedY )
    path.arcTo( std::min( p1.x(), p0.x() ),
                std::min( p1.y(), p0.y() ),
                cornerRadius, cornerRadius,
                180, 90 );
  else
    path.arcTo( std::min( p1.x(), p0.x() ),
                std::min( p1.y(), p0.y() ),
                cornerRadius, cornerRadius,
                -180, -90 );

  return path;
}
