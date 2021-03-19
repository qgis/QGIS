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

QLineF segment( int index, QRectF rect )
{
  switch ( index )
  {
    case 0:
      return QLineF( rect.left(),
                     rect.top(),
                     rect.right(),
                     rect.top() );
    case 1:
      return QLineF( rect.right(),
                     rect.top(),
                     rect.right(),
                     rect.bottom() );
    case 2:
      return QLineF( rect.right(),
                     rect.bottom(),
                     rect.left(),
                     rect.bottom() );
    case 3:
      return QLineF( rect.left(),
                     rect.bottom(),
                     rect.left(),
                     rect.top() );
    default:
      return QLineF();
  }
}

QPolygonF QgsShapeGenerator::createBalloon( const QgsPointXY &origin, const QRectF &rect, double wedgeWidth )
{
  int balloonSegment = -1;
  QPointF balloonSegmentPoint1;
  QPointF balloonSegmentPoint2;

  //first test if the point is in the frame. In that case we don't need a balloon and can just use a rect
  if ( rect.contains( origin.toQPointF() ) )
  {
    balloonSegment = -1;
  }
  else
  {
    //edge list
    QList<QLineF> segmentList;
    segmentList << segment( 0, rect );
    segmentList << segment( 1, rect );
    segmentList << segment( 2, rect );
    segmentList << segment( 3, rect );

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
      if ( std::sqrt( minEdgePoint.sqrDist( minEdgeEnd.x(), minEdgeEnd.y() ) ) < wedgeWidth )
      {
        double x = 0;
        double y = 0;
        QgsGeometryUtils::pointOnLineWithDistance( minEdge.p2().x(), minEdge.p2().y(), minEdge.p1().x(), minEdge.p1().y(), wedgeWidth, x, y );
        balloonSegmentPoint1 = QPointF( x, y );
      }

      {
        double x = 0;
        double y = 0;
        QgsGeometryUtils::pointOnLineWithDistance( balloonSegmentPoint1.x(), balloonSegmentPoint1.y(), minEdge.p2().x(), minEdge.p2().y(), wedgeWidth, x, y );
        balloonSegmentPoint2 = QPointF( x, y );
      }
    }
  }

  QPolygonF poly;
  poly.reserve( 12 );
  for ( int i = 0; i < 4; ++i )
  {
    QLineF currentSegment = segment( i, rect );
    poly << currentSegment.p1();

    if ( i == balloonSegment )
    {
      poly << balloonSegmentPoint1;
      poly << origin.toQPointF();
      poly << balloonSegmentPoint2;
    }

    poly << currentSegment.p2();
  }
  if ( poly.at( 0 ) != poly.at( poly.count() - 1 ) )
    poly << poly.at( 0 );
  return poly;
}
