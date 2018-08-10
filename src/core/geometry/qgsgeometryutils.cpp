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

#include "qgscurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometrycollection.h"
#include "qgslinestring.h"
#include "qgswkbptr.h"
#include "qgslogger.h"

#include <memory>
#include <QStringList>
#include <QVector>
#include <QRegularExpression>

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
  geometry.closestSegment( point, closestPoint, vertexAfter, nullptr, DEFAULT_SEGMENT_EPSILON );
  if ( vertexAfter.isValid() )
  {
    QgsPoint pointAfter = geometry.vertexAt( vertexAfter );
    if ( vertexAfter.vertex > 0 )
    {
      QgsVertexId vertexBefore = vertexAfter;
      vertexBefore.vertex--;
      QgsPoint pointBefore = geometry.vertexAt( vertexBefore );
      double length = pointBefore.distance( pointAfter );
      double distance = pointBefore.distance( closestPoint );

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
    if ( !first )
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

double QgsGeometryUtils::sqrDistance2D( const QgsPoint &pt1, const QgsPoint &pt2 )
{
  return ( pt1.x() - pt2.x() ) * ( pt1.x() - pt2.x() ) + ( pt1.y() - pt2.y() ) * ( pt1.y() - pt2.y() );
}

double QgsGeometryUtils::sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double &minDistX, double &minDistY, double epsilon )
{
  minDistX = x1;
  minDistY = y1;

  double dx = x2 - x1;
  double dy = y2 - y1;

  if ( !qgsDoubleNear( dx, 0.0 ) || !qgsDoubleNear( dy, 0.0 ) )
  {
    double t = ( ( ptX - x1 ) * dx + ( ptY - y1 ) * dy ) / ( dx * dx + dy * dy );
    if ( t > 1 )
    {
      minDistX = x2;
      minDistY = y2;
    }
    else if ( t > 0 )
    {
      minDistX += dx * t;
      minDistY += dy * t;
    }
  }

  dx = ptX - minDistX;
  dy = ptY - minDistY;

  double dist = dx * dx + dy * dy;

  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( dist, 0.0, epsilon ) )
  {
    minDistX = ptX;
    minDistY = ptY;
    return 0.0;
  }

  return dist;
}

bool QgsGeometryUtils::lineIntersection( const QgsPoint &p1, QgsVector v1, const QgsPoint &p2, QgsVector v2, QgsPoint &intersection )
{
  double d = v1.y() * v2.x() - v1.x() * v2.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double k = ( dy * v2.x() - dx * v2.y() ) / d;

  intersection = QgsPoint( p1.x() + v1.x() * k, p1.y() + v1.y() * k );

  // z support for intersection point
  QgsGeometryUtils::setZValueFromPoints( QgsPointSequence() << p1 << p2, intersection );

  return true;
}

bool QgsGeometryUtils::segmentIntersection( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &q1, const QgsPoint &q2, QgsPoint &intersectionPoint, bool &isIntersection, const double tolerance, bool acceptImproperIntersection )
{
  isIntersection = false;

  QgsVector v( p2.x() - p1.x(), p2.y() - p1.y() );
  QgsVector w( q2.x() - q1.x(), q2.y() - q1.y() );
  double vl = v.length();
  double wl = w.length();

  if ( qgsDoubleNear( vl, 0.0, tolerance ) || qgsDoubleNear( wl, 0.0, tolerance ) )
  {
    return false;
  }
  v = v / vl;
  w = w / wl;

  if ( !QgsGeometryUtils::lineIntersection( p1, v, q1, w, intersectionPoint ) )
  {
    return false;
  }

  isIntersection = true;
  if ( acceptImproperIntersection )
  {
    if ( ( p1 == q1 ) || ( p1 == q2 ) )
    {
      intersectionPoint = p1;
      return true;
    }
    else if ( ( p2 == q1 ) || ( p2 == q2 ) )
    {
      intersectionPoint = p2;
      return true;
    }

    double x, y;
    if (
      // intersectionPoint = p1
      qgsDoubleNear( QgsGeometryUtils::sqrDistToLine( p1.x(), p1.y(), q1.x(), q1.y(), q2.x(), q2.y(), x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = p2
      qgsDoubleNear( QgsGeometryUtils::sqrDistToLine( p2.x(), p2.y(), q1.x(), q1.y(), q2.x(), q2.y(), x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = q1
      qgsDoubleNear( QgsGeometryUtils::sqrDistToLine( q1.x(), q1.y(), p1.x(), p1.y(), p2.x(), p2.y(), x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = q2
      qgsDoubleNear( QgsGeometryUtils::sqrDistToLine( q2.x(), q2.y(), p1.x(), p1.y(), p2.x(), p2.y(), x, y, tolerance ), 0.0, tolerance )
    )
    {
      return true;
    }
  }

  double lambdav = QgsVector( intersectionPoint.x() - p1.x(), intersectionPoint.y() - p1.y() ) *  v;
  if ( lambdav < 0. + tolerance || lambdav > vl - tolerance )
    return false;

  double lambdaw = QgsVector( intersectionPoint.x() - q1.x(), intersectionPoint.y() - q1.y() ) * w;
  return !( lambdaw < 0. + tolerance || lambdaw >= wl - tolerance );

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

  const double dr = std::sqrt( std::pow( dx, 2 ) + std::pow( dy, 2 ) );
  const double d = x1 * y2 - x2 * y1;

  const double disc = std::pow( radius, 2 ) * std::pow( dr, 2 ) - std::pow( d, 2 );

  if ( disc < 0 )
  {
    //no intersection or tangent
    return false;
  }
  else
  {
    // two solutions
    const int sgnDy = dy < 0 ? -1 : 1;

    const double ax = center.x() + ( d * dy + sgnDy * dx * std::sqrt( std::pow( radius, 2 ) * std::pow( dr, 2 ) - std::pow( d, 2 ) ) ) / ( std::pow( dr, 2 ) );
    const double ay = center.y() + ( -d * dx + std::fabs( dy ) * std::sqrt( std::pow( radius, 2 ) * std::pow( dr, 2 ) - std::pow( d, 2 ) ) ) / ( std::pow( dr, 2 ) );
    const QgsPointXY p1( ax, ay );

    const double bx = center.x() + ( d * dy - sgnDy * dx * std::sqrt( std::pow( radius, 2 ) * std::pow( dr, 2 ) - std::pow( d, 2 ) ) ) / ( std::pow( dr, 2 ) );
    const double by = center.y() + ( -d * dx - std::fabs( dy ) * std::sqrt( std::pow( radius, 2 ) * std::pow( dr, 2 ) - std::pow( d, 2 ) ) ) / ( std::pow( dr, 2 ) );
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
int QgsGeometryUtils::circleCircleIntersections( QgsPointXY center0, const double r0, QgsPointXY center1, const double r1, QgsPointXY &intersection1, QgsPointXY &intersection2 )
{
  // determine the straight-line distance between the centers
  const double d = center0.distance( center1 );

  // check for solvability
  if ( d > ( r0 + r1 ) )
  {
    // no solution. circles do not intersect.
    return 0;
  }
  else if ( d < std::fabs( r0 - r1 ) )
  {
    // no solution. one circle is contained in the other
    return 0;
  }
  else if ( qgsDoubleNear( d, 0 ) && ( qgsDoubleNear( r0, r1 ) ) )
  {
    // no solutions, the circles coincide
    return 0;
  }

  /* 'point 2' is the point where the line through the circle
   * intersection points crosses the line between the circle
   * centers.
  */

  // Determine the distance from point 0 to point 2.
  const double a = ( ( r0 * r0 ) - ( r1 * r1 ) + ( d * d ) ) / ( 2.0 * d ) ;

  /* dx and dy are the vertical and horizontal distances between
   * the circle centers.
   */
  const double dx = center1.x() - center0.x();
  const double dy = center1.y() - center0.y();

  // Determine the coordinates of point 2.
  const double x2 = center0.x() + ( dx * a / d );
  const double y2 = center0.y() + ( dy * a / d );

  /* Determine the distance from point 2 to either of the
   * intersection points.
   */
  const double h = std::sqrt( ( r0 * r0 ) - ( a * a ) );

  /* Now determine the offsets of the intersection points from
   * point 2.
   */
  const double rx = dy * ( h / d );
  const double ry = dx * ( h / d );

  // determine the absolute intersection points
  intersection1 = QgsPointXY( x2 + rx, y2 - ry );
  intersection2 = QgsPointXY( x2 - rx, y2 +  ry );

  // see if we have 1 or 2 solutions
  if ( qgsDoubleNear( d, r0 + r1 ) )
    return 1;

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

QVector<QgsGeometryUtils::SelfIntersection> QgsGeometryUtils::selfIntersections( const QgsAbstractGeometry *geom, int part, int ring, double tolerance )
{
  QVector<SelfIntersection> intersections;

  int n = geom->vertexCount( part, ring );
  bool isClosed = geom->vertexAt( QgsVertexId( part, ring, 0 ) ) == geom->vertexAt( QgsVertexId( part, ring, n - 1 ) );

  // Check every pair of segments for intersections
  for ( int i = 0, j = 1; j < n; i = j++ )
  {
    QgsPoint pi = geom->vertexAt( QgsVertexId( part, ring, i ) );
    QgsPoint pj = geom->vertexAt( QgsVertexId( part, ring, j ) );
    if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < tolerance * tolerance ) continue;

    // Don't test neighboring edges
    int start = j + 1;
    int end = i == 0 && isClosed ? n - 1 : n;
    for ( int k = start, l = start + 1; l < end; k = l++ )
    {
      QgsPoint pk = geom->vertexAt( QgsVertexId( part, ring, k ) );
      QgsPoint pl = geom->vertexAt( QgsVertexId( part, ring, l ) );

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

int QgsGeometryUtils::leftOfLine( double x, double y, double x1, double y1, double x2, double y2 )
{
  double f1 = x - x1;
  double f2 = y2 - y1;
  double f3 = y - y1;
  double f4 = x2 - x1;
  double test = ( f1 * f2 - f3 * f4 );
  // return -1, 0, or 1
  return qgsDoubleNear( test, 0.0 ) ? 0 : ( test < 0 ? -1 : 1 );
}

QgsPoint QgsGeometryUtils::pointOnLineWithDistance( const QgsPoint &startPoint, const QgsPoint &directionPoint, double distance )
{
  double x, y;
  pointOnLineWithDistance( startPoint.x(), startPoint.y(), directionPoint.x(), directionPoint.y(), distance, x, y );
  return QgsPoint( x, y );
}

void QgsGeometryUtils::pointOnLineWithDistance( double x1, double y1, double x2, double y2, double distance, double &x, double &y, double *z1, double *z2, double *z, double *m1, double *m2, double *m )
{
  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double length = std::sqrt( dx * dx + dy * dy );

  if ( qgsDoubleNear( length, 0.0 ) )
  {
    x = x1;
    y = y1;
    if ( z && z1 )
      *z = *z1;
    if ( m && m1 )
      *m = *m1;
  }
  else
  {
    const double scaleFactor = distance / length;
    x = x1 + dx * scaleFactor;
    y = y1 + dy * scaleFactor;
    if ( z && z1 && z2 )
      *z = *z1 + ( *z2 - *z1 ) * scaleFactor;
    if ( m && m1 && m2 )
      *m = *m1 + ( *m2 - *m1 ) * scaleFactor;
  }
}

double QgsGeometryUtils::ccwAngle( double dy, double dx )
{
  double angle = std::atan2( dy, dx ) * 180 / M_PI;
  if ( angle < 0 )
  {
    return 360 + angle;
  }
  else if ( angle > 360 )
  {
    return 360 - angle;
  }
  return angle;
}

void QgsGeometryUtils::circleCenterRadius( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double &radius, double &centerX, double &centerY )
{
  double dx21, dy21, dx31, dy31, h21, h31, d;

  //closed circle
  if ( qgsDoubleNear( pt1.x(), pt3.x() ) && qgsDoubleNear( pt1.y(), pt3.y() ) )
  {
    centerX = ( pt1.x() + pt2.x() ) / 2.0;
    centerY = ( pt1.y() + pt2.y() ) / 2.0;
    radius = std::sqrt( std::pow( centerX - pt1.x(), 2.0 ) + std::pow( centerY - pt1.y(), 2.0 ) );
    return;
  }

  // Using Cartesian circumcenter eguations from page https://en.wikipedia.org/wiki/Circumscribed_circle
  dx21 = pt2.x() - pt1.x();
  dy21 = pt2.y() - pt1.y();
  dx31 = pt3.x() - pt1.x();
  dy31 = pt3.y() - pt1.y();

  h21 = std::pow( dx21, 2.0 ) + std::pow( dy21, 2.0 );
  h31 = std::pow( dx31, 2.0 ) + std::pow( dy31, 2.0 );

  // 2*Cross product, d<0 means clockwise and d>0 counterclockwise sweeping angle
  d = 2 * ( dx21 * dy31 - dx31 * dy21 );

  // Check colinearity, Cross product = 0
  if ( qgsDoubleNear( std::fabs( d ), 0.0, 0.00000000001 ) )
  {
    radius = -1.0;
    return;
  }

  // Calculate centroid coordinates and radius
  centerX = pt1.x() + ( h21 * dy31 - h31 * dy21 ) / d;
  centerY = pt1.y() - ( h21 * dx31 - h31 * dx21 ) / d;
  radius = std::sqrt( std::pow( centerX - pt1.x(), 2.0 ) + std::pow( centerY - pt1.y(), 2.0 ) );
}

bool QgsGeometryUtils::circleClockwise( double angle1, double angle2, double angle3 )
{
  if ( angle3 >= angle1 )
  {
    return !( angle2 > angle1 && angle2 < angle3 );
  }
  else
  {
    return !( angle2 > angle1 || angle2 < angle3 );
  }
}

bool QgsGeometryUtils::circleAngleBetween( double angle, double angle1, double angle2, bool clockwise )
{
  if ( clockwise )
  {
    if ( angle2 < angle1 )
    {
      return ( angle <= angle1 && angle >= angle2 );
    }
    else
    {
      return ( angle <= angle1 || angle >= angle2 );
    }
  }
  else
  {
    if ( angle2 > angle1 )
    {
      return ( angle >= angle1 && angle <= angle2 );
    }
    else
    {
      return ( angle >= angle1 || angle <= angle2 );
    }
  }
}

bool QgsGeometryUtils::angleOnCircle( double angle, double angle1, double angle2, double angle3 )
{
  bool clockwise = circleClockwise( angle1, angle2, angle3 );
  return circleAngleBetween( angle, angle1, angle3, clockwise );
}

double QgsGeometryUtils::circleLength( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double centerX, centerY, radius;
  circleCenterRadius( QgsPoint( x1, y1 ), QgsPoint( x2, y2 ), QgsPoint( x3, y3 ), radius, centerX, centerY );
  double length = M_PI / 180.0 * radius * sweepAngle( centerX, centerY, x1, y1, x2, y2, x3, y3 );
  if ( length < 0 )
  {
    length = -length;
  }
  return length;
}

double QgsGeometryUtils::sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double p1Angle = QgsGeometryUtils::ccwAngle( y1 - centerY, x1 - centerX );
  double p2Angle = QgsGeometryUtils::ccwAngle( y2 - centerY, x2 - centerX );
  double p3Angle = QgsGeometryUtils::ccwAngle( y3 - centerY, x3 - centerX );

  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      return ( p3Angle - p1Angle );
    }
    else
    {
      return ( - ( p1Angle + ( 360 - p3Angle ) ) );
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      return ( -( p1Angle - p3Angle ) );
    }
    else
    {
      return ( p3Angle + ( 360 - p1Angle ) );
    }
  }
}

bool QgsGeometryUtils::segmentMidPoint( const QgsPoint &p1, const QgsPoint &p2, QgsPoint &result, double radius, const QgsPoint &mousePos )
{
  QgsPoint midPoint( ( p1.x() + p2.x() ) / 2.0, ( p1.y() + p2.y() ) / 2.0 );
  double midDist = std::sqrt( sqrDistance2D( p1, midPoint ) );
  if ( radius < midDist )
  {
    return false;
  }
  double centerMidDist = std::sqrt( radius * radius - midDist * midDist );
  double dist = radius - centerMidDist;

  double midDx = midPoint.x() - p1.x();
  double midDy = midPoint.y() - p1.y();

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
    double currentDist = sqrDistance2D( mousePos, possibleMidPoints.at( i ) );
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

  // add z support if necessary
  QgsGeometryUtils::setZValueFromPoints( QgsPointSequence() << p1 << p2, result );

  return true;
}

QgsPoint QgsGeometryUtils::segmentMidPointFromCenter( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &center, const bool useShortestArc )
{
  double midPointAngle = averageAngle( lineAngle( center.x(), center.y(), p1.x(), p1.y() ),
                                       lineAngle( center.x(), center.y(), p2.x(), p2.y() ) );
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

  double p1Angle = QgsGeometryUtils::ccwAngle( cp1.y() - mY, cp1.x() - mX );
  double p2Angle = QgsGeometryUtils::ccwAngle( cp2.y() - mY, cp2.x() - mX );
  double p3Angle = QgsGeometryUtils::ccwAngle( cp3.y() - mY, cp3.x() - mX );
  double angle = 0;
  if ( circleClockwise( p1Angle, p2Angle, p3Angle ) )
  {
    angle = lineAngle( tangentPoint.x(), tangentPoint.y(), mX, mY ) - M_PI_2;
  }
  else
  {
    angle = lineAngle( mX, mY, tangentPoint.x(), tangentPoint.y() ) - M_PI_2;
  }
  if ( angle < 0 )
    angle += 2 * M_PI;
  return angle;
}

void QgsGeometryUtils::segmentizeArc( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3, QgsPointSequence &points, double tolerance, QgsAbstractGeometry::SegmentationToleranceType toleranceType, bool hasZ, bool hasM )
{
  bool reversed = false;
  int segSide = segmentSide( p1, p3, p2 );

  QgsPoint circlePoint1;
  const QgsPoint circlePoint2 = p2;
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
    double halfAngle = std::acos( -tolerance / radius + 1 );
    increment = 2 * halfAngle;
  }

  //angles of pt1, pt2, pt3
  double a1 = std::atan2( circlePoint1.y() - centerY, circlePoint1.x() - centerX );
  double a2 = std::atan2( circlePoint2.y() - centerY, circlePoint2.x() - centerX );
  double a3 = std::atan2( circlePoint3.y() - centerY, circlePoint3.x() - centerX );

  // Make segmentation symmetric
  const bool symmetric = true;
  if ( symmetric )
  {
    double angle = a3 - a1;
    if ( angle < 0 ) angle += M_PI * 2;

    /* Number of segments in output */
    int segs = ceil( angle / increment );
    /* Tweak increment to be regular for all the arc */
    increment = angle / segs;
  }

  /* Adjust a3 up so we can increment from a1 to a3 cleanly */
  if ( a3 < a1 )
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
    QgsWkbTypes::Type pointWkbType = QgsWkbTypes::Point;
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
    double tolError = increment / 100;
    double stopAngle = a3 - tolError;
    for ( double angle = a1 + increment; angle < stopAngle; angle += increment )
    {
      x = centerX + radius * std::cos( angle );
      y = centerY + radius * std::sin( angle );

      if ( hasZ )
      {
        z = interpolateArcValue( angle, a1, a2, a3, circlePoint1.z(), circlePoint2.z(), circlePoint3.z() );
      }
      if ( hasM )
      {
        m = interpolateArcValue( angle, a1, a2, a3, circlePoint1.m(), circlePoint2.m(), circlePoint3.m() );
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
  double side = ( ( pt2.x() - pt1.x() ) * ( pt3.y() - pt1.y() ) - ( pt3.x() - pt1.x() ) * ( pt2.y() - pt1.y() ) );
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

double QgsGeometryUtils::interpolateArcValue( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 )
{
  /* Counter-clockwise sweep */
  if ( a1 < a2 )
  {
    if ( angle <= a2 )
      return zm1 + ( zm2 - zm1 ) * ( angle - a1 ) / ( a2 - a1 );
    else
      return zm2 + ( zm3 - zm2 ) * ( angle - a2 ) / ( a3 - a2 );
  }
  /* Clockwise sweep */
  else
  {
    if ( angle >= a2 )
      return zm1 + ( zm2 - zm1 ) * ( a1 - angle ) / ( a1 - a2 );
    else
      return zm2 + ( zm3 - zm2 ) * ( a2 - angle ) / ( a2 - a3 );
  }
}

QgsPointSequence QgsGeometryUtils::pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure )
{
  int dim = 2 + is3D + isMeasure;
  QgsPointSequence points;
  const QStringList coordList = wktCoordinateList.split( ',', QString::SkipEmptyParts );

  //first scan through for extra unexpected dimensions
  bool foundZ = false;
  bool foundM = false;
  QRegularExpression rx( QStringLiteral( "\\s" ) );
  for ( const QString &pointCoordinates : coordList )
  {
    QStringList coordinates = pointCoordinates.split( rx, QString::SkipEmptyParts );
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
    QStringList coordinates = pointCoordinates.split( rx, QString::SkipEmptyParts );
    if ( coordinates.size() < dim )
      continue;

    int idx = 0;
    double x = coordinates[idx++].toDouble();
    double y = coordinates[idx++].toDouble();

    double z = 0;
    if ( ( is3D || foundZ ) && coordinates.length() > idx )
      z = coordinates[idx++].toDouble();

    double m = 0;
    if ( ( isMeasure || foundM ) && coordinates.length() > idx )
      m = coordinates[idx++].toDouble();

    QgsWkbTypes::Type t = QgsWkbTypes::Point;
    if ( is3D || foundZ )
    {
      if ( isMeasure || foundM )
        t = QgsWkbTypes::PointZM;
      else
        t = QgsWkbTypes::PointZ;
    }
    else
    {
      if ( isMeasure || foundM )
        t = QgsWkbTypes::PointM;
      else
        t = QgsWkbTypes::Point;
    }

    points.append( QgsPoint( t, x, y, z, m ) );
  }

  return points;
}

void QgsGeometryUtils::pointsToWKB( QgsWkbPtr &wkb, const QgsPointSequence &points, bool is3D, bool isMeasure )
{
  wkb << static_cast<quint32>( points.size() );
  for ( const QgsPoint &point : points )
  {
    wkb << point.x() << point.y();
    if ( is3D )
    {
      wkb << point.z();
    }
    if ( isMeasure )
    {
      wkb << point.m();
    }
  }
}

QString QgsGeometryUtils::pointsToWKT( const QgsPointSequence &points, int precision, bool is3D, bool isMeasure )
{
  QString wkt = QStringLiteral( "(" );
  for ( const QgsPoint &p : points )
  {
    wkt += qgsDoubleToString( p.x(), precision );
    wkt += ' ' + qgsDoubleToString( p.y(), precision );
    if ( is3D )
      wkt += ' ' + qgsDoubleToString( p.z(), precision );
    if ( isMeasure )
      wkt += ' ' + qgsDoubleToString( p.m(), precision );
    wkt += QLatin1String( ", " );
  }
  if ( wkt.endsWith( QLatin1String( ", " ) ) )
    wkt.chop( 2 ); // Remove last ", "
  wkt += ')';
  return wkt;
}

QDomElement QgsGeometryUtils::pointsToGML2( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder &axisOrder )
{
  QDomElement elemCoordinates = doc.createElementNS( ns, QStringLiteral( "coordinates" ) );

  // coordinate separator
  QString cs = QStringLiteral( "," );
  // tupel separator
  QString ts = QStringLiteral( " " );

  elemCoordinates.setAttribute( QStringLiteral( "cs" ), cs );
  elemCoordinates.setAttribute( QStringLiteral( "ts" ), ts );

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

QDomElement QgsGeometryUtils::pointsToGML3( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, bool is3D, const QgsAbstractGeometry::AxisOrder &axisOrder )
{
  QDomElement elemPosList = doc.createElementNS( ns, QStringLiteral( "posList" ) );
  elemPosList.setAttribute( QStringLiteral( "srsDimension" ), is3D ? 3 : 2 );

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
  QString json = QStringLiteral( "[ " );
  for ( const QgsPoint &p : points )
  {
    json += '[' + qgsDoubleToString( p.x(), precision ) + ", " + qgsDoubleToString( p.y(), precision ) + "], ";
  }
  if ( json.endsWith( QLatin1String( ", " ) ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += ']';
  return json;
}

double QgsGeometryUtils::normalizedAngle( double angle )
{
  double clippedAngle = angle;
  if ( clippedAngle >= M_PI * 2 || clippedAngle <= -2 * M_PI )
  {
    clippedAngle = std::fmod( clippedAngle, 2 * M_PI );
  }
  if ( clippedAngle < 0.0 )
  {
    clippedAngle += 2 * M_PI;
  }
  return clippedAngle;
}

QPair<QgsWkbTypes::Type, QString> QgsGeometryUtils::wktReadBlock( const QString &wkt )
{
  QgsWkbTypes::Type wkbType = QgsWkbTypes::parseType( wkt );

  QRegularExpression cooRegEx( QStringLiteral( "^[^\\(]*\\((.*)\\)[^\\)]*$" ) );
  cooRegEx.setPatternOptions( QRegularExpression::DotMatchesEverythingOption );
  QRegularExpressionMatch match = cooRegEx.match( wkt );
  QString contents = match.hasMatch() ? match.captured( 1 ) : QString();
  return qMakePair( wkbType, contents );
}

QStringList QgsGeometryUtils::wktGetChildBlocks( const QString &wkt, const QString &defaultType )
{
  int level = 0;
  QString block;
  QStringList blocks;
  for ( int i = 0, n = wkt.length(); i < n; ++i )
  {
    if ( ( wkt[i].isSpace() || wkt[i] == '\n' || wkt[i] == '\t' ) && level == 0 )
      continue;

    if ( wkt[i] == ',' && level == 0 )
    {
      if ( !block.isEmpty() )
      {
        if ( block.startsWith( '(' ) && !defaultType.isEmpty() )
          block.prepend( defaultType + ' ' );
        blocks.append( block );
      }
      block.clear();
      continue;
    }
    if ( wkt[i] == '(' )
      ++level;
    else if ( wkt[i] == ')' )
      --level;
    block += wkt[i];
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
  QgsWkbTypes::Type pType( QgsWkbTypes::Point );


  double x = ( pt1.x() + pt2.x() ) / 2.0;
  double y = ( pt1.y() + pt2.y() ) / 2.0;
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
  double delta_x = pt2.x() - pt1.x();
  double delta_y = pt2.y() - pt1.y();
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
    double y = ( -c - a * p.x() ) / b;
    double m = gradient( s1, s2 );
    double d2 = 1 + m * m;
    double H = p.y() - y;
    double dx = m * H / d2;
    double dy = m * dx;
    p2 = QgsPoint( p.x() + dx, y + dy );
  }

  line.addVertex( p );
  line.addVertex( p2 );

  return line;
}

double QgsGeometryUtils::lineAngle( double x1, double y1, double x2, double y2 )
{
  double at = std::atan2( y2 - y1, x2 - x1 );
  double a = -at + M_PI_2;
  return normalizedAngle( a );
}

double QgsGeometryUtils::angleBetweenThreePoints( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double angle1 = std::atan2( y1 - y2, x1 - x2 );
  double angle2 = std::atan2( y3 - y2, x3 - x2 );
  return normalizedAngle( angle1 - angle2 );
}

double QgsGeometryUtils::linePerpendicularAngle( double x1, double y1, double x2, double y2 )
{
  double a = lineAngle( x1, y1, x2, y2 );
  a += M_PI_2;
  return normalizedAngle( a );
}

double QgsGeometryUtils::averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  // calc average angle between the previous and next point
  double a1 = lineAngle( x1, y1, x2, y2 );
  double a2 = lineAngle( x2, y2, x3, y3 );
  return averageAngle( a1, a2 );
}

double QgsGeometryUtils::averageAngle( double a1, double a2 )
{
  a1 = normalizedAngle( a1 );
  a2 = normalizedAngle( a2 );
  double clockwiseDiff = 0.0;
  if ( a2 >= a1 )
  {
    clockwiseDiff = a2 - a1;
  }
  else
  {
    clockwiseDiff = a2 + ( 2 * M_PI - a1 );
  }
  double counterClockwiseDiff = 2 * M_PI - clockwiseDiff;

  double resultAngle = 0;
  if ( clockwiseDiff <= counterClockwiseDiff )
  {
    resultAngle = a1 + clockwiseDiff / 2.0;
  }
  else
  {
    resultAngle = a1 - counterClockwiseDiff / 2.0;
  }
  return normalizedAngle( resultAngle );
}

double QgsGeometryUtils::skewLinesDistance( const QgsVector3D &P1, const QgsVector3D &P12,
    const QgsVector3D &P2, const QgsVector3D &P22 )
{
  QgsVector3D u1 = P12 - P1;
  QgsVector3D u2 = P22 - P2;
  QgsVector3D u3 = QgsVector3D::crossProduct( u1, u2 );
  if ( u3.length() == 0 ) return 1;
  u3.normalize();
  QgsVector3D dir = P1 - P2;
  return std::fabs( ( QgsVector3D::dotProduct( dir, u3 ) ) ); // u3 is already normalized
}

bool QgsGeometryUtils::skewLinesProjection( const QgsVector3D &P1, const QgsVector3D &P12,
    const QgsVector3D &P2, const QgsVector3D &P22,
    QgsVector3D &X1, double epsilon )
{
  QgsVector3D d = P2 - P1;
  QgsVector3D u1 = P12 - P1;
  u1.normalize();
  QgsVector3D u2 = P22 - P2;
  u2.normalize();
  QgsVector3D u3 = QgsVector3D::crossProduct( u1, u2 );

  if ( std::fabs( u3.x() ) <= epsilon &&
       std::fabs( u3.y() ) <= epsilon &&
       std::fabs( u3.z() ) <= epsilon )
  {
    // The rays are almost parallel.
    return false;
  }

  // X1 and X2 are the closest points on lines
  // we want to find X1 (lies on u1)
  // solving the linear equation in r1 and r2: Xi = Pi + ri*ui
  // we are only interested in X1 so we only solve for r1.
  float a1 = QgsVector3D::dotProduct( u1, u1 ), b1 = QgsVector3D::dotProduct( u1, u2 ), c1 = QgsVector3D::dotProduct( u1, d );
  float a2 = QgsVector3D::dotProduct( u1, u2 ), b2 = QgsVector3D::dotProduct( u2, u2 ), c2 = QgsVector3D::dotProduct( u2, d );
  if ( !( std::fabs( b1 ) > epsilon ) )
  {
    // Denominator is close to zero.
    return false;
  }
  if ( !( a2 != -1 && a2 != 1 ) )
  {
    // Lines are parallel
    return false;
  }

  double r1 = ( c2 - b2 * c1 / b1 ) / ( a2 - b2 * a1 / b1 );
  X1 = P1 + u1 * r1;

  return true;
}

bool QgsGeometryUtils::linesIntersection3D( const QgsVector3D &La1, const QgsVector3D &La2,
    const QgsVector3D &Lb1, const QgsVector3D &Lb2,
    QgsVector3D &intersection )
{

  // if all Vector are on the same plane (have the same Z), use the 2D intersection
  // else return a false result
  if ( qgsDoubleNear( La1.z(), La2.z() ) && qgsDoubleNear( La1.z(), Lb1.z() ) && qgsDoubleNear( La1.z(), Lb2.z() ) )
  {
    QgsPoint ptInter;
    bool isIntersection;
    segmentIntersection( QgsPoint( La1.x(), La1.y() ),
                         QgsPoint( La2.x(), La2.y() ),
                         QgsPoint( Lb1.x(), Lb1.y() ),
                         QgsPoint( Lb2.x(), Lb2.y() ),
                         ptInter,
                         isIntersection,
                         1e-8,
                         true );
    intersection.set( ptInter.x(), ptInter.y(), La1.z() );
    return true;
  }

  // first check if lines have an exact intersection point
  // do it by checking if the shortest distance is exactly 0
  float distance = skewLinesDistance( La1, La2, Lb1, Lb2 );
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    // 3d lines have exact intersection point.
    QgsVector3D C = La2;
    QgsVector3D D = Lb2;
    QgsVector3D e = La1 - La2;
    QgsVector3D f = Lb1 - Lb2;
    QgsVector3D g = D - C;
    if ( qgsDoubleNear( ( QgsVector3D::crossProduct( f, g ) ).length(), 0.0 ) || qgsDoubleNear( ( QgsVector3D::crossProduct( f, e ) ).length(), 0.0 ) )
    {
      // Lines have no intersection, are they parallel?
      return false;
    }

    QgsVector3D fgn = QgsVector3D::crossProduct( f, g );
    fgn.normalize();

    QgsVector3D fen = QgsVector3D::crossProduct( f, e );
    fen.normalize();

    int di = -1;
    if ( fgn == fen ) // same direction?
      di *= -1;

    intersection = C + e * di * ( QgsVector3D::crossProduct( f, g ).length() / QgsVector3D::crossProduct( f, e ).length() );
    return true;
  }

  // try to calculate the approximate intersection point
  QgsVector3D X1, X2;
  bool firstIsDone = skewLinesProjection( La1, La2, Lb1, Lb2, X1 );
  bool secondIsDone = skewLinesProjection( Lb1, Lb2, La1, La2, X2 );

  if ( !firstIsDone || !secondIsDone )
  {
    // Could not obtain projection point.
    return false;
  }

  intersection = ( X1 + X2 ) / 2.0;
  return true;
}

bool QgsGeometryUtils::setZValueFromPoints( const QgsPointSequence &points, QgsPoint &point )
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
