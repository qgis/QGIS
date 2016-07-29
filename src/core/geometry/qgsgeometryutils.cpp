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

#include "qgscurvev2.h"
#include "qgscurvepolygonv2.h"
#include "qgsgeometrycollectionv2.h"
#include "qgslinestringv2.h"
#include "qgswkbptr.h"

#include <QStringList>
#include <QVector>

QList<QgsLineStringV2*> QgsGeometryUtils::extractLineStrings( const QgsAbstractGeometryV2* geom )
{
  QList< QgsLineStringV2* > linestrings;
  if ( !geom )
    return linestrings;

  QList< const QgsAbstractGeometryV2 * > geometries;
  geometries << geom;
  while ( ! geometries.isEmpty() )
  {
    const QgsAbstractGeometryV2* g = geometries.takeFirst();
    if ( const QgsCurveV2* curve = dynamic_cast< const QgsCurveV2* >( g ) )
    {
      linestrings << static_cast< QgsLineStringV2* >( curve->segmentize() );
    }
    else if ( const QgsGeometryCollectionV2* collection = dynamic_cast< const QgsGeometryCollectionV2* >( g ) )
    {
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        geometries.append( collection->geometryN( i ) );
      }
    }
    else if ( const QgsCurvePolygonV2* curvePolygon = dynamic_cast< const QgsCurvePolygonV2* >( g ) )
    {
      if ( curvePolygon->exteriorRing() )
        linestrings << static_cast< QgsLineStringV2* >( curvePolygon->exteriorRing()->segmentize() );

      for ( int i = 0; i < curvePolygon->numInteriorRings(); ++i )
      {
        linestrings << static_cast< QgsLineStringV2* >( curvePolygon->interiorRing( i )->segmentize() );
      }
    }
  }
  return linestrings;
}

QgsPointV2 QgsGeometryUtils::closestVertex( const QgsAbstractGeometryV2& geom, const QgsPointV2& pt, QgsVertexId& id )
{
  double minDist = std::numeric_limits<double>::max();
  double currentDist = 0;
  QgsPointV2 minDistPoint;

  QgsVertexId vertexId;
  QgsPointV2 vertex;
  while ( geom.nextVertex( vertexId, vertex ) )
  {
    currentDist = QgsGeometryUtils::sqrDistance2D( pt, vertex );
    // The <= is on purpose: for geometries with closing vertices, this ensures
    // that the closing vertex is retuned. For the node tool, the rubberband
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

double QgsGeometryUtils::distanceToVertex( const QgsAbstractGeometryV2 &geom, const QgsVertexId &id )
{
  double currentDist = 0;
  QgsVertexId vertexId;
  QgsPointV2 vertex;
  QgsPointV2 previousVertex;

  bool first = true;
  while ( geom.nextVertex( vertexId, vertex ) )
  {
    if ( !first )
    {
      currentDist += sqrt( QgsGeometryUtils::sqrDistance2D( previousVertex, vertex ) );
    }

    previousVertex = vertex;
    first = false;

    if ( vertexId == id )
    {
      //found target vertex
      return currentDist;
    }
  }

  //could not find target vertex
  return -1;
}

void QgsGeometryUtils::adjacentVertices( const QgsAbstractGeometryV2& geom, QgsVertexId atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex )
{
  bool polygonType = ( geom.dimension()  == 2 );

  QgsCoordinateSequenceV2 coords = geom.coordinateSequence();

  //get feature
  if ( coords.size() <= atVertex.part )
  {
    return; //error, no such feature
  }

  const QgsRingSequenceV2 &part = coords.at( atVertex.part );

  //get ring
  if ( part.size() <= atVertex.ring )
  {
    return; //error, no such ring
  }
  const QgsPointSequenceV2 &ring = part.at( atVertex.ring );
  if ( ring.size() <= atVertex.vertex )
  {
    return;
  }

  //vertex in the middle
  if ( atVertex.vertex > 0 && atVertex.vertex < ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part;
    beforeVertex.ring = atVertex.ring;
    beforeVertex.vertex = atVertex.vertex - 1;
    afterVertex.part = atVertex.part;
    afterVertex.ring = atVertex.ring;
    afterVertex.vertex = atVertex.vertex + 1;
  }
  else if ( atVertex.vertex == 0 )
  {
    afterVertex.part = atVertex.part;
    afterVertex.ring = atVertex.ring;
    afterVertex.vertex = atVertex.vertex + 1;
    if ( polygonType && ring.size() > 3 )
    {
      beforeVertex.part = atVertex.part;
      beforeVertex.ring = atVertex.ring;
      beforeVertex.vertex = ring.size() - 2;
    }
    else
    {
      beforeVertex = QgsVertexId(); //before vertex invalid
    }
  }
  else if ( atVertex.vertex == ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part;
    beforeVertex.ring = atVertex.ring;
    beforeVertex.vertex = atVertex.vertex - 1;
    if ( polygonType )
    {
      afterVertex.part = atVertex.part;
      afterVertex.ring = atVertex.ring;
      afterVertex.vertex = 1;
    }
    else
    {
      afterVertex = QgsVertexId(); //after vertex invalid
    }
  }
}

double QgsGeometryUtils::sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 )
{
  return ( pt1.x() - pt2.x() ) * ( pt1.x() - pt2.x() ) + ( pt1.y() - pt2.y() ) * ( pt1.y() - pt2.y() );
}

double QgsGeometryUtils::sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX, double& minDistY, double epsilon )
{
  //normal vector
  double nx = y2 - y1;
  double ny = -( x2 - x1 );

  double t;
  t = ( ptX * ny - ptY * nx - x1 * ny + y1 * nx ) / (( x2 - x1 ) * ny - ( y2 - y1 ) * nx );

  if ( t < 0.0 )
  {
    minDistX = x1;
    minDistY = y1;
  }
  else if ( t > 1.0 )
  {
    minDistX = x2;
    minDistY = y2;
  }
  else
  {
    minDistX = x1 + t * ( x2 - x1 );
    minDistY = y1 + t * ( y2 - y1 );
  }

  double dist = ( minDistX - ptX ) * ( minDistX - ptX ) + ( minDistY - ptY ) * ( minDistY - ptY );

  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( dist, 0.0, epsilon ) )
  {
    minDistX = ptX;
    minDistY = ptY;
    return 0.0;
  }

  return dist;
}

bool QgsGeometryUtils::lineIntersection( const QgsPointV2& p1, QgsVector v, const QgsPointV2& q1, QgsVector w, QgsPointV2& inter )
{
  double d = v.y() * w.x() - v.x() * w.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  double dx = q1.x() - p1.x();
  double dy = q1.y() - p1.y();
  double k = ( dy * w.x() - dx * w.y() ) / d;

  inter = QgsPointV2( p1.x() + v.x() * k, p1.y() + v.y() * k );

  return true;
}

bool QgsGeometryUtils::segmentIntersection( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &q1, const QgsPointV2 &q2, QgsPointV2 &inter, double tolerance )
{
  QgsVector v( p2.x() - p1.x(), p2.y() - p1.y() );
  QgsVector w( q2.x() - q1.x(), q2.y() - q1.y() );
  double vl = v.length();
  double wl = w.length();

  if ( qFuzzyIsNull( vl ) || qFuzzyIsNull( wl ) )
  {
    return false;
  }
  v = v / vl;
  w = w / wl;

  if ( !QgsGeometryUtils::lineIntersection( p1, v, q1, w, inter ) )
    return false;

  double lambdav = QgsVector( inter.x() - p1.x(), inter.y() - p1.y() ) *  v;
  if ( lambdav < 0. + tolerance || lambdav > vl - tolerance )
    return false;

  double lambdaw = QgsVector( inter.x() - q1.x(), inter.y() - q1.y() ) * w;
  if ( lambdaw < 0. + tolerance || lambdaw >= wl - tolerance )
    return false;

  return true;
}

QList<QgsGeometryUtils::SelfIntersection> QgsGeometryUtils::getSelfIntersections( const QgsAbstractGeometryV2 *geom, int part, int ring, double tolerance )
{
  QList<SelfIntersection> intersections;

  int n = geom->vertexCount( part, ring );
  bool isClosed = geom->vertexAt( QgsVertexId( part, ring, 0 ) ) == geom->vertexAt( QgsVertexId( part, ring, n - 1 ) );

  // Check every pair of segments for intersections
  for ( int i = 0, j = 1; j < n; i = j++ )
  {
    QgsPointV2 pi = geom->vertexAt( QgsVertexId( part, ring, i ) );
    QgsPointV2 pj = geom->vertexAt( QgsVertexId( part, ring, j ) );
    if ( QgsGeometryUtils::sqrDistance2D( pi, pj ) < tolerance * tolerance ) continue;

    // Don't test neighboring edges
    int start = j + 1;
    int end = i == 0 && isClosed ? n - 1 : n;
    for ( int k = start, l = start + 1; l < end; k = l++ )
    {
      QgsPointV2 pk = geom->vertexAt( QgsVertexId( part, ring, k ) );
      QgsPointV2 pl = geom->vertexAt( QgsVertexId( part, ring, l ) );

      QgsPointV2 inter;
      if ( !QgsGeometryUtils::segmentIntersection( pi, pj, pk, pl, inter, tolerance ) ) continue;

      SelfIntersection s;
      s.segment1 = i;
      s.segment2 = k;
      if ( s.segment1 > s.segment2 )
      {
        qSwap( s.segment1, s.segment2 );
      }
      s.point = inter;
      intersections.append( s );
    }
  }
  return intersections;
}

double QgsGeometryUtils::leftOfLine( double x, double y, double x1, double y1, double x2, double y2 )
{
  double f1 = x - x1;
  double f2 = y2 - y1;
  double f3 = y - y1;
  double f4 = x2 - x1;
  return f1*f2 - f3*f4;
}

QgsPointV2 QgsGeometryUtils::pointOnLineWithDistance( const QgsPointV2& startPoint, const QgsPointV2& directionPoint, double distance )
{
  double dx = directionPoint.x() - startPoint.x();
  double dy = directionPoint.y() - startPoint.y();
  double length = sqrt( dx * dx + dy * dy );

  if ( qgsDoubleNear( length, 0.0 ) )
  {
    return startPoint;
  }

  double scaleFactor = distance / length;
  return QgsPointV2( startPoint.x() + dx * scaleFactor, startPoint.y() + dy * scaleFactor );
}

double QgsGeometryUtils::ccwAngle( double dy, double dx )
{
  double angle = atan2( dy, dx ) * 180 / M_PI;
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

void QgsGeometryUtils::circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius, double& centerX, double& centerY )
{
  double dx21, dy21, dx31, dy31, h21, h31, d;

  //closed circle
  if ( qgsDoubleNear( pt1.x(), pt3.x() ) && qgsDoubleNear( pt1.y(), pt3.y() ) )
  {
    centerX = ( pt1.x() + pt2.x() ) / 2.0;
    centerY = ( pt1.y() + pt2.y() ) / 2.0;
    radius = sqrt( pow( centerX - pt1.x(), 2.0 ) + pow( centerY - pt1.y(), 2.0 ) );
    return;
  }

  // Using cartesian circumcenter eguations from page https://en.wikipedia.org/wiki/Circumscribed_circle
  dx21 = pt2.x() - pt1.x();
  dy21 = pt2.y() - pt1.y();
  dx31 = pt3.x() - pt1.x();
  dy31 = pt3.y() - pt1.y();

  h21 = pow( dx21, 2.0 ) + pow( dy21, 2.0 );
  h31 = pow( dx31, 2.0 ) + pow( dy31, 2.0 );

  // 2*Cross product, d<0 means clockwise and d>0 counterclockwise sweeping angle
  d = 2 * ( dx21 * dy31 - dx31 * dy21 );

  // Check colinearity, Cross product = 0
  if ( qgsDoubleNear( fabs( d ), 0.0, 0.00000000001 ) )
  {
    radius = -1.0;
    return;
  }

  // Calculate centroid coordinates and radius
  centerX = pt1.x() + ( h21 * dy31 - h31 * dy21 ) / d;
  centerY = pt1.y() - ( h21 * dx31 - h31 * dx21 ) / d;
  radius = sqrt( pow( centerX - pt1.x(), 2.0 ) + pow( centerY - pt1.y(), 2.0 ) );
}

bool QgsGeometryUtils::circleClockwise( double angle1, double angle2, double angle3 )
{
  if ( angle3 >= angle1 )
  {
    if ( angle2 > angle1 && angle2 < angle3 )
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    if ( angle2 > angle1 || angle2 < angle3 )
    {
      return false;
    }
    else
    {
      return true;
    }
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
  circleCenterRadius( QgsPointV2( x1, y1 ), QgsPointV2( x2, y2 ), QgsPointV2( x3, y3 ), radius, centerX, centerY );
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
      return( p3Angle - p1Angle );
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
      return( -( p1Angle - p3Angle ) );
    }
    else
    {
      return( p3Angle + ( 360 - p1Angle ) );
    }
  }
}

bool QgsGeometryUtils::segmentMidPoint( const QgsPointV2& p1, const QgsPointV2& p2, QgsPointV2& result, double radius, const QgsPointV2& mousePos )
{
  QgsPointV2 midPoint(( p1.x() + p2.x() ) / 2.0, ( p1.y() + p2.y() ) / 2.0 );
  double midDist = sqrt( sqrDistance2D( p1, midPoint ) );
  if ( radius < midDist )
  {
    return false;
  }
  double centerMidDist = sqrt( radius * radius - midDist * midDist );
  double dist = radius - centerMidDist;

  double midDx = midPoint.x() - p1.x();
  double midDy = midPoint.y() - p1.y();

  //get the four possible midpoints
  QVector<QgsPointV2> possibleMidPoints;
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPointV2( midPoint.x() - midDy, midPoint.y() + midDx ), dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPointV2( midPoint.x() - midDy, midPoint.y() + midDx ), 2 * radius - dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPointV2( midPoint.x() + midDy, midPoint.y() - midDx ), dist ) );
  possibleMidPoints.append( pointOnLineWithDistance( midPoint, QgsPointV2( midPoint.x() + midDy, midPoint.y() - midDx ), 2 * radius - dist ) );

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
  return true;
}

double QgsGeometryUtils::circleTangentDirection( const QgsPointV2& tangentPoint, const QgsPointV2& cp1,
    const QgsPointV2& cp2, const QgsPointV2& cp3 )
{
  //calculate circle midpoint
  double mX, mY, radius;
  circleCenterRadius( cp1, cp2, cp3, radius, mX, mY );

  double p1Angle = QgsGeometryUtils::ccwAngle( cp1.y() - mY, cp1.x() - mX );
  double p2Angle = QgsGeometryUtils::ccwAngle( cp2.y() - mY, cp2.x() - mX );
  double p3Angle = QgsGeometryUtils::ccwAngle( cp3.y() - mY, cp3.x() - mX );
  if ( circleClockwise( p1Angle, p2Angle, p3Angle ) )
  {
    return lineAngle( tangentPoint.x(), tangentPoint.y(), mX, mY );
  }
  else
  {
    return lineAngle( mX, mY, tangentPoint.x(), tangentPoint.y() );
  }
}

QgsPointSequenceV2 QgsGeometryUtils::pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure )
{
  int dim = 2 + is3D + isMeasure;
  QgsPointSequenceV2 points;
  QStringList coordList = wktCoordinateList.split( ',', QString::SkipEmptyParts );

  //first scan through for extra unexpected dimensions
  bool foundZ = false;
  bool foundM = false;
  Q_FOREACH ( const QString& pointCoordinates, coordList )
  {
    QStringList coordinates = pointCoordinates.split( ' ', QString::SkipEmptyParts );
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

  Q_FOREACH ( const QString& pointCoordinates, coordList )
  {
    QStringList coordinates = pointCoordinates.split( ' ', QString::SkipEmptyParts );
    if ( coordinates.size() < dim )
      continue;

    int idx = 0;
    double x = coordinates[idx++].toDouble();
    double y = coordinates[idx++].toDouble();

    double z = 0;
    if (( is3D || foundZ ) && coordinates.length() > idx )
      z = coordinates[idx++].toDouble();

    double m = 0;
    if (( isMeasure || foundM ) && coordinates.length() > idx )
      m = coordinates[idx++].toDouble();

    QgsWKBTypes::Type t = QgsWKBTypes::Point;
    if ( is3D || foundZ )
    {
      if ( isMeasure || foundM )
        t = QgsWKBTypes::PointZM;
      else
        t = QgsWKBTypes::PointZ;
    }
    else
    {
      if ( isMeasure || foundM )
        t = QgsWKBTypes::PointM;
      else
        t = QgsWKBTypes::Point;
    }

    points.append( QgsPointV2( t, x, y, z, m ) );
  }

  return points;
}

void QgsGeometryUtils::pointsToWKB( QgsWkbPtr& wkb, const QgsPointSequenceV2 &points, bool is3D, bool isMeasure )
{
  wkb << static_cast<quint32>( points.size() );
  Q_FOREACH ( const QgsPointV2& point, points )
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

QString QgsGeometryUtils::pointsToWKT( const QgsPointSequenceV2 &points, int precision, bool is3D, bool isMeasure )
{
  QString wkt = "(";
  Q_FOREACH ( const QgsPointV2& p, points )
  {
    wkt += qgsDoubleToString( p.x(), precision );
    wkt += ' ' + qgsDoubleToString( p.y(), precision );
    if ( is3D )
      wkt += ' ' + qgsDoubleToString( p.z(), precision );
    if ( isMeasure )
      wkt += ' ' + qgsDoubleToString( p.m(), precision );
    wkt += ", ";
  }
  if ( wkt.endsWith( ", " ) )
    wkt.chop( 2 ); // Remove last ", "
  wkt += ')';
  return wkt;
}

QDomElement QgsGeometryUtils::pointsToGML2( const QgsPointSequenceV2 &points, QDomDocument& doc, int precision, const QString &ns )
{
  QDomElement elemCoordinates = doc.createElementNS( ns, "coordinates" );

  QString strCoordinates;

  Q_FOREACH ( const QgsPointV2& p, points )
    strCoordinates += qgsDoubleToString( p.x(), precision ) + ',' + qgsDoubleToString( p.y(), precision ) + ' ';

  if ( strCoordinates.endsWith( ' ' ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  return elemCoordinates;
}

QDomElement QgsGeometryUtils::pointsToGML3( const QgsPointSequenceV2 &points, QDomDocument& doc, int precision, const QString &ns, bool is3D )
{
  QDomElement elemPosList = doc.createElementNS( ns, "posList" );
  elemPosList.setAttribute( "srsDimension", is3D ? 3 : 2 );

  QString strCoordinates;
  Q_FOREACH ( const QgsPointV2& p, points )
  {
    strCoordinates += qgsDoubleToString( p.x(), precision ) + ' ' + qgsDoubleToString( p.y(), precision ) + ' ';
    if ( is3D )
      strCoordinates += qgsDoubleToString( p.z(), precision ) + ' ';
  }
  if ( strCoordinates.endsWith( ' ' ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  return elemPosList;
}

QString QgsGeometryUtils::pointsToJSON( const QgsPointSequenceV2 &points, int precision )
{
  QString json = "[ ";
  Q_FOREACH ( const QgsPointV2& p, points )
  {
    json += '[' + qgsDoubleToString( p.x(), precision ) + ", " + qgsDoubleToString( p.y(), precision ) + "], ";
  }
  if ( json.endsWith( ", " ) )
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
    clippedAngle = fmod( clippedAngle, 2 * M_PI );
  }
  if ( clippedAngle < 0.0 )
  {
    clippedAngle += 2 * M_PI;
  }
  return clippedAngle;
}

QPair<QgsWKBTypes::Type, QString> QgsGeometryUtils::wktReadBlock( const QString &wkt )
{
  QgsWKBTypes::Type wkbType = QgsWKBTypes::parseType( wkt );

  QRegExp cooRegEx( "^[^\\(]*\\((.*)\\)[^\\)]*$" );
  QString contents = cooRegEx.indexIn( wkt ) >= 0 ? cooRegEx.cap( 1 ) : QString();
  return qMakePair( wkbType, contents );
}

QStringList QgsGeometryUtils::wktGetChildBlocks( const QString &wkt, const QString& defaultType )
{
  int level = 0;
  QString block;
  QStringList blocks;
  for ( int i = 0, n = wkt.length(); i < n; ++i )
  {
    if ( wkt[i].isSpace() && level == 0 )
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

double QgsGeometryUtils::lineAngle( double x1, double y1, double x2, double y2 )
{
  double at = atan2( y2 - y1, x2 - x1 );
  double a = -at + M_PI / 2.0;
  return normalizedAngle( a );
}

double QgsGeometryUtils::linePerpendicularAngle( double x1, double y1, double x2, double y2 )
{
  double a = lineAngle( x1, y1, x2, y2 );
  a += ( M_PI / 2.0 );
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
