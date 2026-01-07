/***************************************************************************
                      qgsgeometryutils.cpp
-------------------------------------------------------------------
Date                 : 21 Nov 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsgeometryutils.h"

#include <limits>
#include <memory>
#include <nlohmann/json.hpp>

#include "qgsabstractgeometry.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometrycollection.h"
#include "qgslinestring.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsvertexid.h"
#include "qgswkbptr.h"

#include <QRegularExpression>
#include <QStringList>
#include <QVector>

QVector<QgsLineString *> QgsGeometryUtils::extractLineStrings( const QgsAbstractGeometry *geom )
{
  QVector< QgsLineString * > linestrings;
  if ( !geom )
    return linestrings;

  QVector< const QgsAbstractGeometry * > geometries;
  geometries << geom;
  while ( ! geometries.isEmpty() )
  {
    const QgsAbstractGeometry *g = geometries.takeFirst();
    if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( g ) )
    {
      linestrings << static_cast< QgsLineString * >( curve->segmentize() );
    }
    else if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( g ) )
    {
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        geometries.append( collection->geometryN( i ) );
      }
    }
    else if ( const QgsCurvePolygon *curvePolygon = qgsgeometry_cast< const QgsCurvePolygon * >( g ) )
    {
      if ( curvePolygon->exteriorRing() )
        linestrings << static_cast< QgsLineString * >( curvePolygon->exteriorRing()->segmentize() );

      for ( int i = 0; i < curvePolygon->numInteriorRings(); ++i )
      {
        linestrings << static_cast< QgsLineString * >( curvePolygon->interiorRing( i )->segmentize() );
      }
    }
  }
  return linestrings;
}

QgsPoint QgsGeometryUtils::closestVertex( const QgsAbstractGeometry &geom, const QgsPoint &pt, QgsVertexId &id )
{
  double minDist = std::numeric_limits<double>::max();
  double currentDist = 0;
  QgsPoint minDistPoint;
  id = QgsVertexId(); // set as invalid

  if ( geom.isEmpty() || pt.isEmpty() )
    return minDistPoint;

  QgsVertexId vertexId;
  QgsPoint vertex;
  while ( geom.nextVertex( vertexId, vertex ) )
  {
    currentDist = QgsGeometryUtils::sqrDistance2D( pt, vertex );
    // The <= is on purpose: for geometries with closing vertices, this ensures
    // that the closing vertex is returned. For the vertex tool, the rubberband
    // of the closing vertex is above the opening vertex, hence with the <=
    // situations where the covered opening vertex rubberband is selected are
    // avoided.
    if ( currentDist <= minDist )
    {
      minDist = currentDist;
      minDistPoint = vertex;
      id.part = vertexId.part;
      id.ring = vertexId.ring;
      id.vertex = vertexId.vertex;
      id.type = vertexId.type;
    }
  }

  return minDistPoint;
}

QgsPoint QgsGeometryUtils::closestPoint( const QgsAbstractGeometry &geometry, const QgsPoint &point )
{
  QgsPoint closestPoint;
  QgsVertexId vertexAfter;
  geometry.closestSegment( point, closestPoint, vertexAfter, nullptr, Qgis::DEFAULT_SEGMENT_EPSILON );
  if ( vertexAfter.isValid() )
  {
    const QgsPoint pointAfter = geometry.vertexAt( vertexAfter );
    if ( vertexAfter.vertex > 0 )
    {
      QgsVertexId vertexBefore = vertexAfter;
      vertexBefore.vertex--;
      const QgsPoint pointBefore = geometry.vertexAt( vertexBefore );
      const double length = pointBefore.distance( pointAfter );
      const double distance = pointBefore.distance( closestPoint );

      if ( qgsDoubleNear( distance, 0.0 ) )
        closestPoint = pointBefore;
      else if ( qgsDoubleNear( distance, length ) )
        closestPoint = pointAfter;
      else
      {
        if ( QgsWkbTypes::hasZ( geometry.wkbType() ) && length )
          closestPoint.addZValue( pointBefore.z() + ( pointAfter.z() - pointBefore.z() ) * distance / length );
        if ( QgsWkbTypes::hasM( geometry.wkbType() ) )
          closestPoint.addMValue( pointBefore.m() + ( pointAfter.m() - pointBefore.m() ) * distance / length );
      }
    }
  }

  return closestPoint;
}

double QgsGeometryUtils::distanceToVertex( const QgsAbstractGeometry &geom, QgsVertexId id )
{
  double currentDist = 0;
  QgsVertexId vertexId;
  QgsPoint vertex;
  while ( geom.nextVertex( vertexId, vertex ) )
  {
    if ( vertexId == id )
    {
      //found target vertex
      return currentDist;
    }
    currentDist += geom.segmentLength( vertexId );
  }

  //could not find target vertex
  return -1;
}

bool QgsGeometryUtils::verticesAtDistance( const QgsAbstractGeometry &geometry, double distance, QgsVertexId &previousVertex, QgsVertexId &nextVertex )
{
  double currentDist = 0;
  previousVertex = QgsVertexId();
  nextVertex = QgsVertexId();

  if ( geometry.isEmpty() )
  {
    return false;
  }

  QgsPoint point;
  QgsPoint previousPoint;

  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    geometry.nextVertex( previousVertex, point );
    nextVertex = previousVertex;
    return true;
  }

  bool first = true;
  while ( currentDist < distance && geometry.nextVertex( nextVertex, point ) )
  {
    if ( !first && nextVertex.part == previousVertex.part && nextVertex.ring == previousVertex.ring )
    {
      currentDist += std::sqrt( QgsGeometryUtils::sqrDistance2D( previousPoint, point ) );
    }

    if ( qgsDoubleNear( currentDist, distance ) )
    {
      // exact hit!
      previousVertex = nextVertex;
      return true;
    }

    if ( currentDist > distance )
    {
      return true;
    }

    previousVertex = nextVertex;
    previousPoint = point;
    first = false;
  }

  //could not find target distance
  return false;
}

double QgsGeometryUtils::distToInfiniteLine( const QgsPoint &point, const QgsPoint &linePoint1, const QgsPoint &linePoint2, double epsilon )
{
  const double area = std::abs(
                        ( linePoint1.x() - linePoint2.x() ) * ( point.y() - linePoint2.y() ) -
                        ( linePoint1.y() - linePoint2.y() ) * ( point.x() - linePoint2.x() )
                      );

  const double length = std::sqrt(
                          std::pow( linePoint1.x() - linePoint2.x(), 2 ) +
                          std::pow( linePoint1.y() - linePoint2.y(), 2 )
                        );

  const double distance = area / length;
  return qgsDoubleNear( distance, 0.0, epsilon ) ? 0.0 : distance;
}

bool QgsGeometryUtils::lineCircleIntersection( const QgsPointXY &center, const double radius,
    const QgsPointXY &linePoint1, const QgsPointXY &linePoint2,
    QgsPointXY &intersection )
{
  // formula taken from http://mathworld.wolfram.com/Circle-LineIntersection.html

  const double x1 = linePoint1.x() - center.x();
  const double y1 = linePoint1.y() - center.y();
  const double x2 = linePoint2.x() - center.x();
  const double y2 = linePoint2.y() - center.y();
  const double dx = x2 - x1;
  const double dy = y2 - y1;

  const double dr2 = std::pow( dx, 2 ) + std::pow( dy, 2 );
  const double d = x1 * y2 - x2 * y1;

  const double disc = std::pow( radius, 2 ) * dr2 - std::pow( d, 2 );

  if ( disc < 0 )
  {
    //no intersection or tangent
    return false;
  }
  else
  {
    // two solutions
    const int sgnDy = dy < 0 ? -1 : 1;

    const double sqrDisc = std::sqrt( disc );

    const double ax = center.x() + ( d * dy + sgnDy * dx * sqrDisc ) / dr2;
    const double ay = center.y() + ( -d * dx + std::fabs( dy ) * sqrDisc ) / dr2;
    const QgsPointXY p1( ax, ay );

    const double bx = center.x() + ( d * dy - sgnDy * dx * sqrDisc ) / dr2;
    const double by = center.y() + ( -d * dx - std::fabs( dy ) * sqrDisc ) / dr2;
    const QgsPointXY p2( bx, by );

    // snap to nearest intersection

    if ( intersection.sqrDist( p1 ) < intersection.sqrDist( p2 ) )
    {
      intersection.set( p1.x(), p1.y() );
    }
    else
    {
      intersection.set( p2.x(), p2.y() );
    }
    return true;
  }
}

// based on public domain work by 3/26/2005 Tim Voght
// see http://paulbourke.net/geometry/circlesphere/tvoght.c
int QgsGeometryUtils::circleCircleIntersections( const QgsPointXY &center1, const double r1, const QgsPointXY &center2, const double r2, QgsPointXY &intersection1, QgsPointXY &intersection2 )
{
  // determine the straight-line distance between the centers
  const double d = center1.distance( center2 );

  // check if the circles intersect at only 1 point, either "externally" or "internally"
  const bool singleSolutionExt = qgsDoubleNear( d, r1 + r2 );
  const bool singleSolutionInt = qgsDoubleNear( d, std::fabs( r1 - r2 ) );

  // check for solvability
  if ( !singleSolutionExt && d > ( r1 + r2 ) )
  {
    // no solution. circles do not intersect.
    return 0;
  }
  else if ( !singleSolutionInt && d < std::fabs( r1 - r2 ) )
  {
    // no solution. one circle is contained in the other
    return 0;
  }
  else if ( qgsDoubleNear( d, 0 ) && ( qgsDoubleNear( r1, r2 ) ) )
  {
    // no solutions, the circles coincide
    return 0;
  }

  /* 'point 2' is the point where the line through the circle
   * intersection points crosses the line between the circle
   * centers.
  */

  /* Determine the distance 'a' from point 0 to point 2.
   * In the general case, a = ( ( r1 * r1 ) - ( r2 * r2 ) + ( d * d ) ) / ( 2.0 * d ).
   * If d = r1 + r2 or d = r1 - r2 (i.e. r1 > r2), then a = r1; if d = r2 - r1 (i.e. r2 > r1), then a = -r1.
  */
  const double a = singleSolutionExt ? r1 : ( singleSolutionInt ? ( r1 > r2 ? r1 : -r1 ) : ( ( r1 * r1 ) - ( r2 * r2 ) + ( d * d ) ) / ( 2.0 * d ) );

  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  const double dx = center2.x() - center1.x();
  const double dy = center2.y() - center1.y();

  // Determine the coordinates of point 2.
  const double x2 = center1.x() + ( dx * a / d );
  const double y2 = center1.y() + ( dy * a / d );

  // only 1 solution
  if ( singleSolutionExt || singleSolutionInt )
  {
    intersection1 = QgsPointXY( x2, y2 );
    intersection2 = QgsPointXY( x2, y2 );

    return 1;
  }

  // 2 solutions

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  const double h = std::sqrt( ( r1 * r1 ) - ( a * a ) );

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  const double rx = dy * ( h / d );
  const double ry = dx * ( h / d );

  // determine the absolute intersection points
  intersection1 = QgsPointXY( x2 + rx, y2 - ry );
  intersection2 = QgsPointXY( x2 - rx, y2 +  ry );

  return 2;
}

// Using https://stackoverflow.com/a/1351794/1861260
// and inspired by http://csharphelper.com/blog/2014/11/find-the-tangent-lines-between-a-point-and-a-circle-in-c/
bool QgsGeometryUtils::tangentPointAndCircle( const QgsPointXY &center, double radius, const QgsPointXY &p, QgsPointXY &pt1, QgsPointXY &pt2 )
{
  // distance from point to center of circle
  const double dx = center.x() - p.x();
  const double dy = center.y() - p.y();
  const double distanceSquared = dx * dx + dy * dy;
  const double radiusSquared = radius * radius;
  if ( distanceSquared < radiusSquared )
  {
    // point is inside circle!
    return false;
  }

  // distance from point to tangent point, using pythagoras
  const double distanceToTangent = std::sqrt( distanceSquared - radiusSquared );

  // tangent points are those where the original circle intersects a circle centered
  // on p with radius distanceToTangent
  circleCircleIntersections( center, radius, p, distanceToTangent, pt1, pt2 );

  return true;
}

// inspired by http://csharphelper.com/blog/2014/12/find-the-tangent-lines-between-two-circles-in-c/
int QgsGeometryUtils::circleCircleOuterTangents( const QgsPointXY &center1, double radius1, const QgsPointXY &center2, double radius2, QgsPointXY &line1P1, QgsPointXY &line1P2, QgsPointXY &line2P1, QgsPointXY &line2P2 )
{
  if ( radius1 > radius2 )
    return circleCircleOuterTangents( center2, radius2, center1, radius1, line1P1, line1P2, line2P1, line2P2 );

  const double radius2a = radius2 - radius1;
  if ( !tangentPointAndCircle( center2, radius2a, center1, line1P2, line2P2 ) )
  {
    // there are no tangents
    return 0;
  }

  // get the vector perpendicular to the
  // first tangent with length radius1
  QgsVector v1( -( line1P2.y() - center1.y() ), line1P2.x() - center1.x() );
  const double v1Length = v1.length();
  v1 = v1 * ( radius1 / v1Length );

  // offset the tangent vector's points
  line1P1 = center1 + v1;
  line1P2 = line1P2 + v1;

  // get the vector perpendicular to the
  // second tangent with length radius1
  QgsVector v2( line2P2.y() - center1.y(), -( line2P2.x() - center1.x() ) );
  const double v2Length = v2.length();
  v2 = v2 * ( radius1 / v2Length );

  // offset the tangent vector's points
  line2P1 = center1 + v2;
  line2P2 = line2P2 + v2;

  return 2;
}

// inspired by http://csharphelper.com/blog/2014/12/find-the-tangent-lines-between-two-circles-in-c/
int QgsGeometryUtils::circleCircleInnerTangents( const QgsPointXY &center1, double radius1, const QgsPointXY &center2, double radius2, QgsPointXY &line1P1, QgsPointXY &line1P2, QgsPointXY &line2P1, QgsPointXY &line2P2 )
{
  if ( radius1 > radius2 )
    return circleCircleInnerTangents( center2, radius2, center1, radius1, line1P1, line1P2, line2P1, line2P2 );

  // determine the straight-line distance between the centers
  const double d = center1.distance( center2 );
  const double radius1a = radius1 + radius2;

  // check for solvability
  if ( d <= radius1a || qgsDoubleNear( d, radius1a ) )
  {
    // no solution. circles intersect or touch.
    return 0;
  }

  if ( !tangentPointAndCircle( center1, radius1a, center2, line1P2, line2P2 ) )
  {
    // there are no tangents
    return 0;
  }

  // get the vector perpendicular to the
  // first tangent with length radius2
  QgsVector v1( ( line1P2.y() - center2.y() ), -( line1P2.x() - center2.x() ) );
  const double v1Length = v1.length();
  v1 = v1 * ( radius2 / v1Length );

  // offset the tangent vector's points
  line1P1 = center2 + v1;
  line1P2 = line1P2 + v1;

  // get the vector perpendicular to the
  // second tangent with length radius2
  QgsVector v2( -( line2P2.y() - center2.y() ), line2P2.x() - center2.x() );
  const double v2Length = v2.length();
  v2 = v2 * ( radius2 / v2Length );

  // offset the tangent vector's points in opposite direction
  line2P1 = center2 + v2;
  line2P2 = line2P2 + v2;

  return 2;
}

QVector<QgsGeometryUtils::SelfIntersection> QgsGeometryUtils::selfIntersections( const QgsAbstractGeometry *geom, int part, int ring, double tolerance )
{
  QVector<SelfIntersection> intersections;

  const int n = geom->vertexCount( part, ring );
  const bool isClosed = geom->vertexAt( QgsVertexId( part, ring, 0 ) ) == geom->vertexAt( QgsVertexId( part, ring, n - 1 ) );

  // Check every pair of segments for intersections
  for ( int i = 0, j = 1; j < n; i = j++ )
  {
    const QgsPoint pi = geom->vertexAt( QgsVertexId( part, ring, i ) );
    const QgsPoint pj = geom->vertexAt( QgsVertexId( part, ring, j ) );
    if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < tolerance * tolerance ) continue;

    // Don't test neighboring edges
    const int start = j + 1;
    const int end = i == 0 && isClosed ? n - 1 : n;
    for ( int k = start, l = start + 1; l < end; k = l++ )
    {
      const QgsPoint pk = geom->vertexAt( QgsVertexId( part, ring, k ) );
      const QgsPoint pl = geom->vertexAt( QgsVertexId( part, ring, l ) );

      QgsPoint inter;
      bool intersection = false;
      if ( !QgsGeometryUtils::segmentIntersection( pi, pj, pk, pl, inter, intersection, tolerance ) ) continue;

      SelfIntersection s;
      s.segment1 = i;
      s.segment2 = k;
      if ( s.segment1 > s.segment2 )
      {
        std::swap( s.segment1, s.segment2 );
      }
      s.point = inter;
      intersections.append( s );
    }
  }
  return intersections;
}

int QgsGeometryUtils::leftOfLine( const QgsPoint &point, const QgsPoint &p1, const QgsPoint &p2 )
{
  return QgsGeometryUtilsBase::leftOfLine( point.x(), point.y(), p1.x(), p1.y(), p2.x(), p2.y() );
}

QgsPoint QgsGeometryUtils::pointOnLineWithDistance( const QgsPoint &startPoint, const QgsPoint &directionPoint, double distance )
{
  double x, y;
  QgsGeometryUtilsBase::pointOnLineWithDistance( startPoint.x(), startPoint.y(), directionPoint.x(), directionPoint.y(), distance, x, y );
  return QgsPoint( x, y );
}


QgsPoint QgsGeometryUtils::interpolatePointOnArc( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double distance )
{
  double centerX, centerY, radius;
  circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );

  const double theta = distance / radius; // angle subtended
  const double anglePt1 = std::atan2( pt1.y() - centerY, pt1.x() - centerX );
  const double anglePt2 = std::atan2( pt2.y() - centerY, pt2.x() - centerX );
  const double anglePt3 = std::atan2( pt3.y() - centerY, pt3.x() - centerX );
  const bool isClockwise = QgsGeometryUtilsBase::circleClockwise( anglePt1, anglePt2, anglePt3 );
  const double angleDest = anglePt1 + ( isClockwise ? -theta : theta );

  const double x = centerX + radius * ( std::cos( angleDest ) );
  const double y = centerY + radius * ( std::sin( angleDest ) );

  const double z = pt1.is3D() ?
                   QgsGeometryUtilsBase::interpolateArcValue( angleDest, anglePt1, anglePt2, anglePt3, pt1.z(), pt2.z(), pt3.z() )
                   : 0;
  const double m = pt1.isMeasure() ?
                   QgsGeometryUtilsBase::interpolateArcValue( angleDest, anglePt1, anglePt2, anglePt3, pt1.m(), pt2.m(), pt3.m() )
                   : 0;

  return QgsPoint( pt1.wkbType(), x, y, z, m );
}

bool QgsGeometryUtils::segmentMidPoint( const QgsPoint &p1, const QgsPoint &p2, QgsPoint &result, double radius, const QgsPoint &mousePos )
{
  const QgsPoint midPoint( ( p1.x() + p2.x() ) / 2.0, ( p1.y() + p2.y() ) / 2.0 );
  const double midDist = std::sqrt( sqrDistance2D( p1, midPoint ) );
  if ( radius < midDist )
  {
    return false;
  }
  const double centerMidDist = std::sqrt( radius * radius - midDist * midDist );
  const double dist = radius - centerMidDist;

  const double midDx = midPoint.x() - p1.x();
  const double midDy = midPoint.y() - p1.y();

  //get the four possible midpoints
  QVector<QgsPoint> possibleMidPoints;
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPoint( midPoint.x() - midDy, midPoint.y() + midDx ), dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPoint( midPoint.x() - midDy, midPoint.y() + midDx ), 2 * radius - dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPoint( midPoint.x() + midDy, midPoint.y() - midDx ), dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPoint( midPoint.x() + midDy, midPoint.y() - midDx ), 2 * radius - dist ) );

  //take the closest one
  double minDist = std::numeric_limits<double>::max();
  int minDistIndex = -1;
  for ( int i = 0; i < possibleMidPoints.size(); ++i )
  {
    const double currentDist = sqrDistance2D( mousePos, possibleMidPoints.at( i ) );
    if ( currentDist < minDist )
    {
      minDistIndex = i;
      minDist = currentDist;
    }
  }

  if ( minDistIndex == -1 )
  {
    return false;
  }

  result = possibleMidPoints.at( minDistIndex );

  // add z and m support if necessary
  QgsGeometryUtils::transferFirstZOrMValueToPoint( QgsPointSequence() << p1 << p2, result );

  return true;
}

QgsPoint QgsGeometryUtils::segmentMidPointFromCenter( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &center, const bool useShortestArc )
{
  double midPointAngle = QgsGeometryUtilsBase::averageAngle( QgsGeometryUtilsBase::lineAngle( center.x(), center.y(), p1.x(), p1.y() ),
                         QgsGeometryUtilsBase::lineAngle( center.x(), center.y(), p2.x(), p2.y() ) );
  if ( !useShortestArc )
    midPointAngle += M_PI;
  return center.project( center.distance( p1 ), midPointAngle * 180 / M_PI );
}

double QgsGeometryUtils::circleTangentDirection( const QgsPoint &tangentPoint, const QgsPoint &cp1,
    const QgsPoint &cp2, const QgsPoint &cp3 )
{
  //calculate circle midpoint
  double mX, mY, radius;
  circleCenterRadius( cp1, cp2, cp3, radius, mX, mY );

  const double p1Angle = QgsGeometryUtilsBase::ccwAngle( cp1.y() - mY, cp1.x() - mX );
  const double p2Angle = QgsGeometryUtilsBase::ccwAngle( cp2.y() - mY, cp2.x() - mX );
  const double p3Angle = QgsGeometryUtilsBase::ccwAngle( cp3.y() - mY, cp3.x() - mX );
  double angle = 0;
  if ( QgsGeometryUtilsBase::circleClockwise( p1Angle, p2Angle, p3Angle ) )
  {
    angle = QgsGeometryUtilsBase::lineAngle( tangentPoint.x(), tangentPoint.y(), mX, mY ) - M_PI_2;
  }
  else
  {
    angle = QgsGeometryUtilsBase::lineAngle( mX, mY, tangentPoint.x(), tangentPoint.y() ) - M_PI_2;
  }
  if ( angle < 0 )
    angle += 2 * M_PI;
  return angle;
}

// Ported from PostGIS' pt_continues_arc
bool QgsGeometryUtils::pointContinuesArc( const QgsPoint &a1, const QgsPoint &a2, const QgsPoint &a3, const QgsPoint &b, double distanceTolerance, double pointSpacingAngleTolerance )
{
  double centerX = 0;
  double centerY = 0;
  double radius = 0;
  circleCenterRadius( a1, a2, a3, radius, centerX, centerY );

  // Co-linear a1/a2/a3
  if ( radius < 0.0 )
    return false;

  // distance of candidate point to center of arc a1-a2-a3
  const double bDistance = std::sqrt( ( b.x() - centerX ) * ( b.x() - centerX ) +
                                      ( b.y() - centerY ) * ( b.y() - centerY ) );

  double diff = std::fabs( radius - bDistance );

  auto arcAngle = []( const QgsPoint & a, const QgsPoint & b, const QgsPoint & c )->double
  {
    const double abX = b.x() - a.x();
    const double abY = b.y() - a.y();

    const double cbX = b.x() - c.x();
    const double cbY = b.y() - c.y();

    const double dot = ( abX * cbX + abY * cbY ); /* dot product */
    const double cross = ( abX * cbY - abY * cbX ); /* cross product */

    const double alpha = std::atan2( cross, dot );

    return alpha;
  };

  // Is the point b on the circle?
  if ( diff < distanceTolerance )
  {
    const double angle1 = arcAngle( a1, a2, a3 );
    const double angle2 = arcAngle( a2, a3, b );

    // Is the sweep angle similar to the previous one?
    // We only consider a segment replaceable by an arc if the points within
    // it are regularly spaced
    diff = std::fabs( angle1 - angle2 );
    if ( diff > pointSpacingAngleTolerance )
    {
      return false;
    }

    const int a2Side = QgsGeometryUtilsBase::leftOfLine( a2.x(), a2.y(), a1.x(), a1.y(), a3.x(), a3.y() );
    const int bSide  = QgsGeometryUtilsBase::leftOfLine( b.x(), b.y(), a1.x(), a1.y(), a3.x(), a3.y() );

    // Is the point b on the same side of a1/a3 as the mid-point a2 is?
    // If not, it's in the unbounded part of the circle, so it continues the arc, return true.
    if ( bSide != a2Side )
      return true;
  }
  return false;
}

void QgsGeometryUtils::segmentizeArc( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, QgsPointSequence &points, double tolerance, QgsAbstractGeometry::SegmentationToleranceType toleranceType, bool hasZ, bool hasM )
{
  bool reversed = false;
  const int segSide = segmentSide( p1, p3, p2 );

  QgsPoint circlePoint1;
  const QgsPoint &circlePoint2 = p2;
  QgsPoint circlePoint3;

  if ( segSide == -1 )
  {
    // Reverse !
    circlePoint1 = p3;
    circlePoint3 = p1;
    reversed = true;
  }
  else
  {
    circlePoint1 = p1;
    circlePoint3 = p3;
  }

  //adapted code from PostGIS
  double radius = 0;
  double centerX = 0;
  double centerY = 0;
  circleCenterRadius( circlePoint1, circlePoint2, circlePoint3, radius, centerX, centerY );

  if ( circlePoint1 != circlePoint3 && ( radius < 0 || qgsDoubleNear( segSide, 0.0 ) ) ) //points are colinear
  {
    points.append( p1 );
    points.append( p2 );
    points.append( p3 );
    return;
  }

  double increment = tolerance; //one segment per degree
  if ( toleranceType == QgsAbstractGeometry::MaximumDifference )
  {
    // Ensure tolerance is not higher than twice the radius
    tolerance = std::min( tolerance, radius * 2 );
    const double halfAngle = std::acos( -tolerance / radius + 1 );
    increment = 2 * halfAngle;
  }

  //angles of pt1, pt2, pt3
  const double a1 = std::atan2( circlePoint1.y() - centerY, circlePoint1.x() - centerX );
  double a2 = std::atan2( circlePoint2.y() - centerY, circlePoint2.x() - centerX );
  double a3 = std::atan2( circlePoint3.y() - centerY, circlePoint3.x() - centerX );

  // Make segmentation symmetric
  const bool symmetric = true;
  if ( symmetric )
  {
    double angle = a3 - a1;
    // angle == 0 when full circle
    if ( angle <= 0 ) angle += M_PI * 2;

    /* Number of segments in output */
    const int segs = ceil( angle / increment );
    /* Tweak increment to be regular for all the arc */
    increment = angle / segs;
  }

  /* Adjust a3 up so we can increment from a1 to a3 cleanly */
  // a3 == a1 when full circle
  if ( a3 <= a1 )
    a3 += 2.0 * M_PI;
  if ( a2 < a1 )
    a2 += 2.0 * M_PI;

  double x, y;
  double z = 0;
  double m = 0;

  QVector<QgsPoint> stringPoints;
  stringPoints.insert( 0, circlePoint1 );
  if ( circlePoint2 != circlePoint3 && circlePoint1 != circlePoint2 ) //draw straight line segment if two points have the same position
  {
    Qgis::WkbType pointWkbType = Qgis::WkbType::Point;
    if ( hasZ )
      pointWkbType = QgsWkbTypes::addZ( pointWkbType );
    if ( hasM )
      pointWkbType = QgsWkbTypes::addM( pointWkbType );

    // As we're adding the last point in any case, we'll avoid
    // including a point which is at less than 1% increment distance
    // from it (may happen to find them due to numbers approximation).
    // NOTE that this effectively allows in output some segments which
    //      are more distant than requested. This is at most 1% off
    //      from requested MaxAngle and less for MaxError.
    const double tolError = increment / 100;
    const double stopAngle = a3 - tolError;
    for ( double angle = a1 + increment; angle < stopAngle; angle += increment )
    {
      x = centerX + radius * std::cos( angle );
      y = centerY + radius * std::sin( angle );

      if ( hasZ )
      {
        z = QgsGeometryUtilsBase::interpolateArcValue( angle, a1, a2, a3, circlePoint1.z(), circlePoint2.z(), circlePoint3.z() );
      }
      if ( hasM )
      {
        m = QgsGeometryUtilsBase::interpolateArcValue( angle, a1, a2, a3, circlePoint1.m(), circlePoint2.m(), circlePoint3.m() );
      }

      stringPoints.insert( stringPoints.size(), QgsPoint( pointWkbType, x, y, z, m ) );
    }
  }
  stringPoints.insert( stringPoints.size(), circlePoint3 );

  // TODO: check if or implement QgsPointSequence directly taking an iterator to append
  if ( reversed )
  {
    std::reverse( stringPoints.begin(), stringPoints.end() );
  }
  if ( ! points.empty() && stringPoints.front() == points.back() ) stringPoints.pop_front();
  points.append( stringPoints );
}

int QgsGeometryUtils::segmentSide( const QgsPoint &pt1, const QgsPoint &pt3, const QgsPoint &pt2 )
{
  const double side = ( ( pt2.x() - pt1.x() ) * ( pt3.y() - pt1.y() ) - ( pt3.x() - pt1.x() ) * ( pt2.y() - pt1.y() ) );
  if ( side == 0.0 )
  {
    return 0;
  }
  else
  {
    if ( side < 0 )
    {
      return -1;
    }
    if ( side > 0 )
    {
      return 1;
    }
    return 0;
  }
}

QgsPointSequence QgsGeometryUtils::pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure )
{
  const int dim = 2 + is3D + isMeasure;
  QgsPointSequence points;

  const QStringList coordList = wktCoordinateList.split( ',', Qt::SkipEmptyParts );

  //first scan through for extra unexpected dimensions
  bool foundZ = false;
  bool foundM = false;
  const thread_local QRegularExpression rx( u"\\s"_s );
  const thread_local QRegularExpression rxIsNumber( u"^[+-]?(\\d\\.?\\d*[Ee][+\\-]?\\d+|(\\d+\\.\\d*|\\d*\\.\\d+)|\\d+)$"_s );
  for ( const QString &pointCoordinates : coordList )
  {
    const QStringList coordinates = pointCoordinates.split( rx, Qt::SkipEmptyParts );

    // exit with an empty set if one list contains invalid value.
    if ( coordinates.filter( rxIsNumber ).size() != coordinates.size() )
      return points;

    if ( coordinates.size() == 3 && !foundZ && !foundM && !is3D && !isMeasure )
    {
      // 3 dimensional coordinates, but not specifically marked as such. We allow this
      // anyway and upgrade geometry to have Z dimension
      foundZ = true;
    }
    else if ( coordinates.size() >= 4 && ( !( is3D || foundZ ) || !( isMeasure || foundM ) ) )
    {
      // 4 (or more) dimensional coordinates, but not specifically marked as such. We allow this
      // anyway and upgrade geometry to have Z&M dimensions
      foundZ = true;
      foundM = true;
    }
  }

  for ( const QString &pointCoordinates : coordList )
  {
    QStringList coordinates = pointCoordinates.split( rx, Qt::SkipEmptyParts );
    if ( coordinates.size() < dim )
      continue;

    int idx = 0;
    const double x = coordinates[idx++].toDouble();
    const double y = coordinates[idx++].toDouble();

    double z = 0;
    if ( ( is3D || foundZ ) && coordinates.length() > idx )
      z = coordinates[idx++].toDouble();

    double m = 0;
    if ( ( isMeasure || foundM ) && coordinates.length() > idx )
      m = coordinates[idx++].toDouble();

    Qgis::WkbType t = Qgis::WkbType::Point;
    if ( is3D || foundZ )
    {
      if ( isMeasure || foundM )
        t = Qgis::WkbType::PointZM;
      else
        t = Qgis::WkbType::PointZ;
    }
    else
    {
      if ( isMeasure || foundM )
        t = Qgis::WkbType::PointM;
      else
        t = Qgis::WkbType::Point;
    }

    points.append( QgsPoint( t, x, y, z, m ) );
  }

  return points;
}

void QgsGeometryUtils::pointsToWKB( QgsWkbPtr &wkb, const QgsPointSequence &points, bool is3D, bool isMeasure, QgsAbstractGeometry::WkbFlags flags )
{
  wkb << static_cast<quint32>( points.size() );
  for ( const QgsPoint &point : points )
  {
    wkb << point.x() << point.y();
    if ( is3D )
    {
      double z = point.z();
      if ( flags & QgsAbstractGeometry::FlagExportNanAsDoubleMin
           && std::isnan( z ) )
        z = -std::numeric_limits<double>::max();

      wkb << z;
    }
    if ( isMeasure )
    {
      double m = point.m();
      if ( flags & QgsAbstractGeometry::FlagExportNanAsDoubleMin
           && std::isnan( m ) )
        m = -std::numeric_limits<double>::max();

      wkb << m;
    }
  }
}

QString QgsGeometryUtils::pointsToWKT( const QgsPointSequence &points, int precision, bool is3D, bool isMeasure )
{
  QString wkt = u"("_s;
  for ( const QgsPoint &p : points )
  {
    wkt += qgsDoubleToString( p.x(), precision );
    wkt += ' ' + qgsDoubleToString( p.y(), precision );
    if ( is3D )
      wkt += ' ' + qgsDoubleToString( p.z(), precision );
    if ( isMeasure )
      wkt += ' ' + qgsDoubleToString( p.m(), precision );
    wkt += ", "_L1;
  }
  if ( wkt.endsWith( ", "_L1 ) )
    wkt.chop( 2 ); // Remove last ", "
  wkt += ')';
  return wkt;
}

QDomElement QgsGeometryUtils::pointsToGML2( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, QgsAbstractGeometry::AxisOrder axisOrder )
{
  QDomElement elemCoordinates = doc.createElementNS( ns, u"coordinates"_s );

  // coordinate separator
  const QString cs = u","_s;
  // tuple separator
  const QString ts = u" "_s;

  elemCoordinates.setAttribute( u"cs"_s, cs );
  elemCoordinates.setAttribute( u"ts"_s, ts );

  QString strCoordinates;

  for ( const QgsPoint &p : points )
    if ( axisOrder == QgsAbstractGeometry::AxisOrder::XY )
      strCoordinates += qgsDoubleToString( p.x(), precision ) + cs + qgsDoubleToString( p.y(), precision ) + ts;
    else
      strCoordinates += qgsDoubleToString( p.y(), precision ) + cs + qgsDoubleToString( p.x(), precision ) + ts;

  if ( strCoordinates.endsWith( ts ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  return elemCoordinates;
}

QDomElement QgsGeometryUtils::pointsToGML3( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, bool is3D, QgsAbstractGeometry::AxisOrder axisOrder )
{
  QDomElement elemPosList = doc.createElementNS( ns, u"posList"_s );
  elemPosList.setAttribute( u"srsDimension"_s, is3D ? 3 : 2 );

  QString strCoordinates;
  for ( const QgsPoint &p : points )
  {
    if ( axisOrder == QgsAbstractGeometry::AxisOrder::XY )
      strCoordinates += qgsDoubleToString( p.x(), precision ) + ' ' + qgsDoubleToString( p.y(), precision ) + ' ';
    else
      strCoordinates += qgsDoubleToString( p.y(), precision ) + ' ' + qgsDoubleToString( p.x(), precision ) + ' ';
    if ( is3D )
      strCoordinates += qgsDoubleToString( p.z(), precision ) + ' ';
  }
  if ( strCoordinates.endsWith( ' ' ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  return elemPosList;
}

QString QgsGeometryUtils::pointsToJSON( const QgsPointSequence &points, int precision )
{
  QString json = u"[ "_s;
  for ( const QgsPoint &p : points )
  {
    json += '[' + qgsDoubleToString( p.x(), precision ) + ", "_L1 + qgsDoubleToString( p.y(), precision ) + "], "_L1;
  }
  if ( json.endsWith( ", "_L1 ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += ']';
  return json;
}


json QgsGeometryUtils::pointsToJson( const QgsPointSequence &points, int precision )
{
  json coordinates( json::array() );
  for ( const QgsPoint &p : points )
  {
    if ( p.is3D() )
    {
      coordinates.push_back( { qgsRound( p.x(), precision ), qgsRound( p.y(), precision ), qgsRound( p.z(), precision ) } );
    }
    else
    {
      coordinates.push_back( { qgsRound( p.x(), precision ), qgsRound( p.y(), precision ) } );
    }
  }
  return coordinates;
}

QPair<Qgis::WkbType, QString> QgsGeometryUtils::wktReadBlock( const QString &wkt )
{
  QString wktParsed = wkt;
  QString contents;
  bool isEmpty = false;
  const QLatin1String empty { "EMPTY" };
  if ( wkt.contains( empty, Qt::CaseInsensitive ) )
  {
    const thread_local QRegularExpression whiteSpaces( "\\s" );
    wktParsed.remove( whiteSpaces );
    const int index = wktParsed.indexOf( empty, 0, Qt::CaseInsensitive );

    if ( index == wktParsed.length() - empty.size() )
    {
      // "EMPTY" found at the end of the QString
      // Extract the part of the QString to the left of "EMPTY"
      wktParsed = wktParsed.left( index );
      contents = empty;
      isEmpty = true;
    }
    else
    {
      wktParsed = wkt; // reset to original content
    }
  }
  if ( !isEmpty )
  {
    const int openedParenthesisCount = wktParsed.count( '(' );
    const int closedParenthesisCount = wktParsed.count( ')' );
    // closes missing parentheses
    for ( int i = 0 ;  i < openedParenthesisCount - closedParenthesisCount; ++i )
      wktParsed.push_back( ')' );
    // removes extra parentheses
    wktParsed.truncate( wktParsed.size() - ( closedParenthesisCount - openedParenthesisCount ) );

    const thread_local QRegularExpression cooRegEx( u"^[^\\(]*\\((.*)\\)[^\\)]*$"_s, QRegularExpression::DotMatchesEverythingOption );
    const QRegularExpressionMatch match = cooRegEx.match( wktParsed );
    contents = match.hasMatch() ? match.captured( 1 ) : QString();
  }
  const Qgis::WkbType wkbType = QgsWkbTypes::parseType( wktParsed );
  return qMakePair( wkbType, contents );
}

QStringList QgsGeometryUtils::wktGetChildBlocks( const QString &wkt, const QString &defaultType )
{
  int level = 0;
  QString block;
  block.reserve( wkt.size() );
  QStringList blocks;

  const QChar *wktData = wkt.data();
  const int wktLength = wkt.length();
  for ( int i = 0, n = wktLength; i < n; ++i, ++wktData )
  {
    if ( ( wktData->isSpace() || *wktData == '\n' || *wktData == '\t' ) && level == 0 )
      continue;

    if ( *wktData == ',' && level == 0 )
    {
      if ( !block.isEmpty() )
      {
        if ( block.startsWith( '(' ) && !defaultType.isEmpty() )
          block.prepend( defaultType + ' ' );
        blocks.append( block );
      }
      block.resize( 0 );
      continue;
    }
    if ( *wktData == '(' )
      ++level;
    else if ( *wktData == ')' )
      --level;
    block += *wktData;
  }
  if ( !block.isEmpty() )
  {
    if ( block.startsWith( '(' ) && !defaultType.isEmpty() )
      block.prepend( defaultType + ' ' );
    blocks.append( block );
  }
  return blocks;
}

QgsPoint QgsGeometryUtils::midpoint( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  Qgis::WkbType pType( Qgis::WkbType::Point );


  const double x = ( pt1.x() + pt2.x() ) / 2.0;
  const double y = ( pt1.y() + pt2.y() ) / 2.0;
  double z = std::numeric_limits<double>::quiet_NaN();
  double m = std::numeric_limits<double>::quiet_NaN();

  if ( pt1.is3D() || pt2.is3D() )
  {
    pType = QgsWkbTypes::addZ( pType );
    z = ( pt1.z() + pt2.z() ) / 2.0;
  }

  if ( pt1.isMeasure() || pt2.isMeasure() )
  {
    pType = QgsWkbTypes::addM( pType );
    m = ( pt1.m() + pt2.m() ) / 2.0;
  }

  return QgsPoint( pType, x, y, z, m );
}

QgsPoint QgsGeometryUtils::interpolatePointOnLine( const QgsPoint &p1, const QgsPoint &p2, const double fraction )
{
  const double _fraction = 1 - fraction;
  return QgsPoint( p1.wkbType(),
                   p1.x() * _fraction + p2.x() * fraction,
                   p1.y() * _fraction + p2.y() * fraction,
                   p1.is3D() ? p1.z() * _fraction + p2.z() * fraction : std::numeric_limits<double>::quiet_NaN(),
                   p1.isMeasure() ? p1.m() * _fraction + p2.m() * fraction : std::numeric_limits<double>::quiet_NaN() );
}

QgsPointXY QgsGeometryUtils::interpolatePointOnLine( const double x1, const double y1, const double x2, const double y2, const double fraction )
{
  const double deltaX = ( x2 - x1 ) * fraction;
  const double deltaY = ( y2 - y1 ) * fraction;
  return QgsPointXY( x1 + deltaX, y1 + deltaY );
}

QgsPointXY QgsGeometryUtils::interpolatePointOnLineByValue( const double x1, const double y1, const double v1, const double x2, const double y2, const double v2, const double value )
{
  if ( qgsDoubleNear( v1, v2 ) )
    return QgsPointXY( x1, y1 );

  const double fraction = ( value - v1 ) / ( v2 - v1 );
  return interpolatePointOnLine( x1, y1, x2, y2, fraction );
}

double QgsGeometryUtils::gradient( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  const double delta_x = pt2.x() - pt1.x();
  const double delta_y = pt2.y() - pt1.y();
  if ( qgsDoubleNear( delta_x, 0.0 ) )
  {
    return INFINITY;
  }

  return delta_y / delta_x;
}

void QgsGeometryUtils::coefficients( const QgsPoint &pt1, const QgsPoint &pt2, double &a, double &b, double &c )
{
  if ( qgsDoubleNear( pt1.x(), pt2.x() ) )
  {
    a = 1;
    b = 0;
    c = -pt1.x();
  }
  else if ( qgsDoubleNear( pt1.y(), pt2.y() ) )
  {
    a = 0;
    b = 1;
    c = -pt1.y();
  }
  else
  {
    a = pt1.y() - pt2.y();
    b = pt2.x() - pt1.x();
    c = pt1.x() * pt2.y() - pt1.y() * pt2.x();
  }

}

QgsLineString QgsGeometryUtils::perpendicularSegment( const QgsPoint &p, const QgsPoint &s1, const QgsPoint &s2 )
{
  QgsLineString line;
  QgsPoint p2;

  if ( ( p == s1 ) || ( p == s2 ) )
  {
    return line;
  }

  double a, b, c;
  coefficients( s1, s2, a, b, c );

  if ( qgsDoubleNear( a, 0 ) )
  {
    p2 = QgsPoint( p.x(), s1.y() );
  }
  else if ( qgsDoubleNear( b, 0 ) )
  {
    p2 = QgsPoint( s1.x(), p.y() );
  }
  else
  {
    const double y = ( -c - a * p.x() ) / b;
    const double m = gradient( s1, s2 );
    const double d2 = 1 + m * m;
    const double H = p.y() - y;
    const double dx = m * H / d2;
    const double dy = m * dx;
    p2 = QgsPoint( p.x() + dx, y + dy );
  }

  line.addVertex( p );
  line.addVertex( p2 );

  return line;
}

bool QgsGeometryUtils::transferFirstMValueToPoint( const QgsPointSequence &points, QgsPoint &point )
{
  bool rc = false;

  for ( const QgsPoint &pt : points )
  {
    if ( pt.isMeasure() )
    {
      point.convertTo( QgsWkbTypes::addM( point.wkbType() ) );
      point.setM( pt.m() );
      rc = true;
      break;
    }
  }

  return rc;
}

bool QgsGeometryUtils::transferFirstZValueToPoint( const QgsPointSequence &points, QgsPoint &point )
{
  bool rc = false;

  for ( const QgsPoint &pt : points )
  {
    if ( pt.is3D() )
    {
      point.convertTo( QgsWkbTypes::addZ( point.wkbType() ) );
      point.setZ( pt.z() );
      rc = true;
      break;
    }
  }

  return rc;
}

QgsPoint QgsGeometryUtils::createPointWithMatchingDimensions( double x, double y, const QgsPoint &reference )
{
  if ( reference.is3D() && reference.isMeasure() )
    return QgsPoint( Qgis::WkbType::PointZM, x, y, 0.0, 0.0 );
  else if ( reference.is3D() )
    return QgsPoint( Qgis::WkbType::PointZ, x, y, 0.0 );
  else if ( reference.isMeasure() )
    return QgsPoint( Qgis::WkbType::PointM, x, y, 0.0, 0.0 );
  else
    return QgsPoint( x, y );
}

QgsPoint QgsGeometryUtils::interpolatePointOnSegment( double x, double y,
    const QgsPoint &segmentStart, const QgsPoint &segmentEnd )
{
  QgsPoint result = createPointWithMatchingDimensions( x, y, segmentStart );
  const double distanceFromStart = QgsGeometryUtilsBase::distance2D( segmentStart.x(), segmentStart.y(), x, y );

  if ( segmentStart.is3D() && segmentEnd.is3D() )
  {
    double z1 = segmentStart.z();
    double z2 = segmentEnd.z();
    double interpolatedZ;
    double tempX, tempY;
    QgsGeometryUtilsBase::pointOnLineWithDistance(
      segmentStart.x(), segmentStart.y(), segmentEnd.x(), segmentEnd.y(),
      distanceFromStart, tempX, tempY, &z1, &z2, &interpolatedZ );
    result.setZ( interpolatedZ );
  }

  if ( segmentStart.isMeasure() && segmentEnd.isMeasure() )
  {
    double m1 = segmentStart.m();
    double m2 = segmentEnd.m();
    double interpolatedM;
    double tempX, tempY;
    QgsGeometryUtilsBase::pointOnLineWithDistance(
      segmentStart.x(), segmentStart.y(), segmentEnd.x(), segmentEnd.y(),
      distanceFromStart, tempX, tempY, nullptr, nullptr, nullptr, &m1, &m2, &interpolatedM );
    result.setM( interpolatedM );
  }

  return result;
}

bool QgsGeometryUtils::createChamfer( const QgsPoint &segment1Start, const QgsPoint &segment1End,
                                      const QgsPoint &segment2Start, const QgsPoint &segment2End,
                                      double distance1, double distance2,
                                      QgsPoint &chamferStart, QgsPoint &chamferEnd,
                                      double epsilon )
{
  // Create chamfer points using the utility function
  double chamferStartX, chamferStartY, chamferEndX, chamferEndY;

  QgsGeometryUtilsBase::createChamfer(
    segment1Start.x(), segment1Start.y(), segment1End.x(), segment1End.y(),
    segment2Start.x(), segment2Start.y(), segment2End.x(), segment2End.y(),
    distance1, distance2,
    chamferStartX, chamferStartY,
    chamferEndX, chamferEndY,
    nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr,
    epsilon
  );

  chamferStart = interpolatePointOnSegment( chamferStartX, chamferStartY, segment1Start, segment1End );
  chamferEnd = interpolatePointOnSegment( chamferEndX, chamferEndY, segment2Start, segment2End );

  return true;
}

bool QgsGeometryUtils::createFillet( const QgsPoint &segment1Start, const QgsPoint &segment1End,
                                     const QgsPoint &segment2Start, const QgsPoint &segment2End,
                                     double radius,
                                     QgsPoint &filletPoint1,
                                     QgsPoint &filletMidPoint,
                                     QgsPoint &filletPoint2,
                                     double epsilon )
{
  // Create fillet arc using the utility function
  double filletPointsX[3], filletPointsY[3];

  QgsGeometryUtilsBase::createFillet(
    segment1Start.x(), segment1Start.y(), segment1End.x(), segment1End.y(),
    segment2Start.x(), segment2Start.y(), segment2End.x(), segment2End.y(),
    radius,
    filletPointsX, filletPointsY,
    nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr,
    epsilon
  );

  filletPoint1 = interpolatePointOnSegment( filletPointsX[0], filletPointsY[0], segment1Start, segment1End );
  filletMidPoint = createPointWithMatchingDimensions( filletPointsX[1], filletPointsY[1], segment1Start );
  filletPoint2 = interpolatePointOnSegment( filletPointsX[2], filletPointsY[2], segment2Start, segment2End );

  // Interpolate Z and M for midpoint
  if ( segment1Start.is3D() && segment1End.is3D() && segment2Start.is3D() && segment2End.is3D() )
  {
    filletMidPoint.setZ( ( filletPoint1.z() + filletPoint2.z() ) / 2.0 );
  }
  if ( segment1Start.isMeasure() && segment1End.isMeasure() && segment2Start.isMeasure() && segment2End.isMeasure() )
  {
    filletMidPoint.setM( ( filletPoint1.m() + filletPoint2.m() ) / 2.0 );
  }

  return true;
}

bool QgsGeometryUtils::createFilletArray( const QgsPoint &segment1Start, const QgsPoint &segment1End,
    const QgsPoint &segment2Start, const QgsPoint &segment2End,
    double radius,
    QgsPoint filletPoints[3],
    double epsilon )
{
  QgsPoint p1, p2, p3;
  createFillet( segment1Start, segment1End, segment2Start, segment2End, radius, p1, p2, p3, epsilon );
  filletPoints[0] = p1;
  filletPoints[1] = p2;
  filletPoints[2] = p3;
  return true;
}

std::unique_ptr<QgsLineString> QgsGeometryUtils::createChamferGeometry(
  const QgsPoint &segment1Start, const QgsPoint &segment1End,
  const QgsPoint &segment2Start, const QgsPoint &segment2End,
  double distance1, double distance2 )
{
  QgsPoint chamferStart, chamferEnd;
  createChamfer( segment1Start, segment1End, segment2Start, segment2End, distance1, distance2, chamferStart, chamferEnd );

  return std::make_unique<QgsLineString>(
           QVector<QgsPoint> { segment1Start, chamferStart, chamferEnd, segment2Start } );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryUtils::createFilletGeometry(
  const QgsPoint &segment1Start, const QgsPoint &segment1End,
  const QgsPoint &segment2Start, const QgsPoint &segment2End,
  double radius, int segments )
{
  QgsPoint filletPoints[3];
  createFilletArray( segment1Start, segment1End, segment2Start, segment2End, radius, filletPoints );

  // Calculate far endpoints for complete geometry
  double intersectionX, intersectionY;
  bool isIntersection;
  QgsGeometryUtilsBase::segmentIntersection(
    segment1Start.x(), segment1Start.y(), segment1End.x(), segment1End.y(),
    segment2Start.x(), segment2Start.y(), segment2End.x(), segment2End.y(),
    intersectionX, intersectionY, isIntersection, 1e-8, true );

  if ( !isIntersection )
  {
    throw QgsInvalidArgumentException( "Segments do not intersect." );
  }

  const double dist1ToStart = QgsGeometryUtilsBase::distance2D( intersectionX, intersectionY, segment1Start.x(), segment1Start.y() );
  const double dist1ToEnd = QgsGeometryUtilsBase::distance2D( intersectionX, intersectionY, segment1End.x(), segment1End.y() );
  const double dist2ToStart = QgsGeometryUtilsBase::distance2D( intersectionX, intersectionY, segment2Start.x(), segment2Start.y() );
  const double dist2ToEnd = QgsGeometryUtilsBase::distance2D( intersectionX, intersectionY, segment2End.x(), segment2End.y() );

  const QgsPoint segment1FarEnd = ( dist1ToStart < dist1ToEnd ) ? segment1End : segment1Start;
  const QgsPoint segment2FarEnd = ( dist2ToStart < dist2ToEnd ) ? segment2End : segment2Start;

  if ( segments <= 0 )
  {
    // Return CompoundCurve with circular arc
    auto completeCurve = std::make_unique<QgsCompoundCurve>();

    // First linear segment
    auto firstSegment = std::make_unique<QgsLineString>(
                          QVector<QgsPoint> { segment1FarEnd, filletPoints[0] } );
    completeCurve->addCurve( firstSegment.release() );

    // Circular arc segment
    auto circularString = std::make_unique<QgsCircularString>();
    circularString->setPoints( {filletPoints[0], filletPoints[1], filletPoints[2]} );
    completeCurve->addCurve( circularString.release() );

    // Last linear segment
    auto lastSegment = std::make_unique<QgsLineString>(
                         QVector<QgsPoint> { filletPoints[2], segment2FarEnd } );
    completeCurve->addCurve( lastSegment.release() );

    return completeCurve;
  }
  else
  {
    // Return segmented LineString
    QVector<QgsPoint> points;
    points.append( segment1FarEnd );

    // Convert circular arc to line segments with specified number of segments
    QgsCircularString tempArc;
    tempArc.setPoints( {filletPoints[0], filletPoints[1], filletPoints[2]} );

    // Calculate appropriate tolerance based on desired number of segments
    // Calculate the actual arc angle and divide by desired number of segments
    // Note: segmentizeArc uses ceil(angle/tolerance), so we need to ensure we get exactly the desired number of segments
    double angleTolerance = M_PI / 180.0; // Default to 1 degree
    if ( segments > 0 )
    {
      double radius, centerX, centerY;
      QgsGeometryUtils::circleCenterRadius( filletPoints[0], filletPoints[1], filletPoints[2], radius, centerX, centerY );
      const double arcAngle = std::abs( QgsGeometryUtilsBase::sweepAngle( centerX, centerY,
                                        filletPoints[0].x(), filletPoints[0].y(),
                                        filletPoints[1].x(), filletPoints[1].y(),
                                        filletPoints[2].x(), filletPoints[2].y() ) ) * M_PI / 180.0; // Convert to radians
      // Add small epsilon to avoid ceil() rounding up due to numerical precision
      angleTolerance = arcAngle / segments * ( 1.0 + 1e-10 );
    }

    std::unique_ptr<QgsLineString> segmentizedArc( tempArc.curveToLine( angleTolerance, QgsAbstractGeometry::MaximumAngle ) );

    for ( int i = 0; i < segmentizedArc->numPoints(); ++i )
    {
      points.append( segmentizedArc->vertexAt( QgsVertexId( 0, 0, i ) ) );
    }

    points.append( segment2FarEnd );

    return std::make_unique<QgsLineString>( points );
  }
}

double QgsGeometryUtils::maxFilletRadius( const QgsPoint &segment1Start, const QgsPoint &segment1End,
    const QgsPoint &segment2Start, const QgsPoint &segment2End,
    double epsilon )
{
  return QgsGeometryUtilsBase::maximumFilletRadius( segment1Start.x(), segment1Start.y(), segment1End.x(), segment1End.y(), segment2Start.x(), segment2Start.y(), segment2End.x(), segment2End.y(), epsilon );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryUtils::chamferVertex(
  const QgsCurve *curve, int vertexIndex,
  double distance1, double distance2 )
{
  return doChamferFilletOnVertex( QgsGeometry::ChamferFilletOperationType::Chamfer, curve, vertexIndex, distance1, distance2, 0 );
}

std::unique_ptr<QgsAbstractGeometry> QgsGeometryUtils::filletVertex(
  const QgsCurve *curve, int vertexIndex,
  double radius, int segments )
{
  return doChamferFilletOnVertex( QgsGeometry::ChamferFilletOperationType::Fillet, curve, vertexIndex, radius, 0.0, segments );
}

std::unique_ptr< QgsAbstractGeometry > QgsGeometryUtils::doChamferFilletOnVertex(
  QgsGeometry::ChamferFilletOperationType operation, const QgsCurve *curve, int vertexIndex,
  double value1, double value2, int segments
)
{
  if ( !curve )
    throw QgsInvalidArgumentException( "Curve is null." );
  if ( curve->isClosed() )
  {
    if ( curve->numPoints() < 4 )
      throw QgsInvalidArgumentException( "Closed curve must have at least 4 vertex." );
    if ( vertexIndex < 0 || vertexIndex > curve->numPoints() - 1 )
      throw QgsInvalidArgumentException( u"Vertex index out of range. %1 must be in [0, %2]."_s.arg( vertexIndex ).arg( curve->numPoints() - 1 ) );
  }
  else
  {
    if ( curve->numPoints() < 3 )
      throw QgsInvalidArgumentException( "Opened curve must have at least 3 points." );
    if ( vertexIndex <= 0 || vertexIndex >= curve->numPoints() - 1 )
      throw QgsInvalidArgumentException( u"Vertex index out of range. %1 must be in (0, %2)."_s.arg( vertexIndex ).arg( curve->numPoints() - 1 ) );
  }

  // Extract the three consecutive vertices
  QgsPoint pPrev = curve->vertexAt( QgsVertexId( 0, 0, vertexIndex - 1 ) );
  const QgsPoint p = curve->vertexAt( QgsVertexId( 0, 0, vertexIndex ) );
  QgsPoint pNext = curve->vertexAt( QgsVertexId( 0, 0, vertexIndex + 1 ) );
  if ( curve->isClosed() )
  {
    if ( vertexIndex - 1 < 0 )
      pPrev = curve->vertexAt( QgsVertexId( 0, 0, curve->numPoints() - 2 ) );
    if ( vertexIndex + 1 >= curve->numPoints() )
      pNext = curve->vertexAt( QgsVertexId( 0, 0, 1 ) );
  }

  QgsPoint firstNewPoint, middlePoint, lastNewPoint;

  if ( operation == QgsGeometry::ChamferFilletOperationType::Fillet )
  {
    double rad = std::min( value1, pPrev.distance( p ) * 0.95 );
    rad = std::min( rad, pNext.distance( p ) * 0.95 );

    // Create fillet
    QgsPoint filletPoints[3];
    createFilletArray( pPrev, p, p, pNext, rad, filletPoints );

    firstNewPoint = filletPoints[0];
    middlePoint = filletPoints[1];
    lastNewPoint = filletPoints[2];
  }
  else if ( operation == QgsGeometry::ChamferFilletOperationType::Chamfer )
  {
    // Create chamfer
    createChamfer( pPrev, p, p, pNext, value1, value2, firstNewPoint, lastNewPoint );
  }
  else
    throw QgsInvalidArgumentException( u"Operation '%1' is unknown."_s.arg( qgsEnumValueToKey( operation ) ) );

  // Handle LineString geometries
  if ( qgsgeometry_cast<const QgsLineString *>( curve ) )
  {
    QVector<QgsPoint> points;

    int min = 0;
    if ( curve->isClosed() && vertexIndex == curve->numPoints() - 1 )
      min = 1;

    // Add points before the operated vertex
    for ( int i = min; i < vertexIndex; ++i )
    {
      points.append( curve->vertexAt( QgsVertexId( 0, 0, i ) ) );
    }

    if ( operation == QgsGeometry::ChamferFilletOperationType::Fillet )
    {
      // Add fillet arc as line segments with proper segmentation
      if ( firstNewPoint != pPrev )
        points.append( firstNewPoint );

      if ( segments > 0 )
      {
        QgsCircularString tempArc;
        tempArc.setPoints( { firstNewPoint, middlePoint, lastNewPoint } );

        // Calculate the actual arc angle and divide by desired number of segments
        // Note: segmentizeArc uses ceil(angle/tolerance), so we need to ensure we get exactly the desired number of segments
        double radius, centerX, centerY;
        QgsGeometryUtils::circleCenterRadius( firstNewPoint, middlePoint, lastNewPoint, radius, centerX, centerY );
        const double arcAngle = std::abs( QgsGeometryUtilsBase::sweepAngle( centerX, centerY,
                                          firstNewPoint.x(), firstNewPoint.y(),
                                          middlePoint.x(), middlePoint.y(),
                                          lastNewPoint.x(), lastNewPoint.y() ) ) * M_PI / 180.0; // Convert to radians
        // Add small epsilon to avoid ceil() rounding up due to numerical precision
        const double angleTolerance = arcAngle / segments * ( 1.0 + 1e-10 );
        std::unique_ptr<QgsLineString> segmentizedArc( tempArc.curveToLine( angleTolerance, QgsAbstractGeometry::MaximumAngle ) );

        for ( int i = 1; i < segmentizedArc->numPoints() - 1; ++i )
        {
          points.append( segmentizedArc->vertexAt( QgsVertexId( 0, 0, i ) ) );
        }
      }
      else
      {
        points.append( middlePoint );
      }

      if ( lastNewPoint != pNext )
        points.append( lastNewPoint );
    }
    else
    {
      if ( firstNewPoint != pPrev )
        points.append( firstNewPoint );
      if ( lastNewPoint != pNext )
        points.append( lastNewPoint );
    }

    int max = curve->numPoints();
    if ( curve->isClosed() && vertexIndex == 0 )
      max = curve->numPoints() - 1;

    for ( int i = vertexIndex + 1; i < max; ++i )
    {
      points.append( curve->vertexAt( QgsVertexId( 0, 0, i ) ) );
    }

    return std::make_unique<QgsLineString>( points );
  }

  if ( const QgsCompoundCurve *compound = qgsgeometry_cast<const QgsCompoundCurve *>( curve ) )
  {
    auto newCompound = std::make_unique<QgsCompoundCurve>();

    int globalVertexIndex = 0;
    int targetCurveIndex = -1;
    int vertexInCurve = -1;

    for ( int curveIdx = 0; curveIdx < compound->nCurves(); ++curveIdx )
    {
      const QgsCurve *subcurve = compound->curveAt( curveIdx );
      const int subcurvePoints = subcurve->numPoints();

      if ( globalVertexIndex + subcurvePoints > vertexIndex )
      {
        targetCurveIndex = curveIdx;
        vertexInCurve = vertexIndex - globalVertexIndex;
        break;
      }
      globalVertexIndex += subcurvePoints - 1;
    }

    if ( targetCurveIndex == -1 )
    {
      throw QgsInvalidArgumentException( "While generating output: unable to find curve within compound." );
    }

    const QgsCurve *targetCurve = compound->curveAt( targetCurveIndex );

    // Add curves before the target curve
    for ( int i = 0; i < targetCurveIndex; ++i )
    {
      std::unique_ptr<QgsCurve> tmpCurv( compound->curveAt( i )->clone() );
      if ( curve->isClosed() && vertexIndex == curve->numPoints() - 1 )
      {
        tmpCurv->insertVertex( QgsVertexId( 0, 0, 1 ), lastNewPoint );
        tmpCurv->deleteVertex( QgsVertexId( 0, 0, 0 ) );
      }
      newCompound->addCurve( tmpCurv.release() );
    }

    // Handle the curve containing the vertex
    if ( vertexInCurve > 0 )
    {
      QVector<QgsPoint> beforePoints;
      for ( int j = 0; j < vertexInCurve; ++j )
      {
        beforePoints.append( targetCurve->vertexAt( QgsVertexId( 0, 0, j ) ) );
      }
      beforePoints.append( firstNewPoint );

      if ( beforePoints.size() > 1 )
      {
        auto beforeVertex = std::make_unique<QgsLineString>( beforePoints );
        newCompound->addCurve( beforeVertex.release() );
      }
    }

    if ( operation == QgsGeometry::ChamferFilletOperationType::Fillet )
    {
      // Add the fillet arc - for CompoundCurve, preserve circular nature unless segments > 0
      if ( segments <= 0 )
      {
        // Preserve circular arc
        auto filletArc = std::make_unique<QgsCircularString>();
        filletArc->setPoints( { firstNewPoint, middlePoint, lastNewPoint } );
        newCompound->addCurve( filletArc.release() );
      }
      else
      {
        // Segmentize the arc
        QgsCircularString tempArc;
        tempArc.setPoints( { firstNewPoint, middlePoint, lastNewPoint } );

        const double angleTolerance = ( 2.0 * M_PI ) / ( 4.0 * segments );
        std::unique_ptr<QgsLineString> segmentizedArc( tempArc.curveToLine( angleTolerance, QgsAbstractGeometry::MaximumAngle ) );

        newCompound->addCurve( segmentizedArc.release() );
      }
    }
    else
    {
      auto chamferLine = std::make_unique<QgsLineString>(
                           QVector<QgsPoint> { firstNewPoint, lastNewPoint }
                         );
      newCompound->addCurve( chamferLine.release() );
    }

    if ( vertexInCurve < targetCurve->numPoints() - 1 )
    {
      QVector<QgsPoint> afterPoints;
      afterPoints.append( lastNewPoint );
      for ( int j = vertexInCurve + 1; j < targetCurve->numPoints(); ++j )
      {
        afterPoints.append( targetCurve->vertexAt( QgsVertexId( 0, 0, j ) ) );
      }

      if ( afterPoints.size() > 1 )
      {
        auto afterVertex = std::make_unique<QgsLineString>( afterPoints );
        newCompound->addCurve( afterVertex.release() );
      }
    }

    // Add curves after the target curve
    for ( int i = targetCurveIndex + 1; i < compound->nCurves(); ++i )
    {
      std::unique_ptr<QgsCurve> tmpCurv( compound->curveAt( i )->clone() );
      if ( curve->isClosed() && vertexIndex == 0 )
      {
        tmpCurv->insertVertex( QgsVertexId( 0, 0, tmpCurv->numPoints() - 1 ), firstNewPoint );
        tmpCurv->deleteVertex( QgsVertexId( 0, 0, tmpCurv->numPoints() - 1 ) );
      }
      newCompound->addCurve( tmpCurv.release() );
    }

    return newCompound;
  }

  throw QgsInvalidArgumentException( "While generating output: curse is not a QgsLineString nor a QgsCompoundCurve." );
}

bool QgsGeometryUtils::pointsAreCollinear( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double epsilon )
{
  if ( pt1.is3D() )
  {
    return QgsGeometryUtilsBase::points3DAreCollinear( pt1.x(), pt1.y(), pt1.z(), pt2.x(), pt2.y(), pt2.z(), pt3.x(), pt3.y(), pt3.z(), epsilon );
  }
  else
  {
    return QgsGeometryUtilsBase::pointsAreCollinear( pt1.x(), pt1.y(), pt2.x(), pt2.y(), pt3.x(), pt3.y(), epsilon );
  }
}
