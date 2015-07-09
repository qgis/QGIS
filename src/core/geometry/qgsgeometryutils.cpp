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
#include "qgswkbptr.h"
#include <QStringList>

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
    }
  }

  return minDistPoint;
}

void QgsGeometryUtils::adjacentVertices( const QgsAbstractGeometryV2& geom, const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex )
{
  bool polygonType = ( geom.dimension()  == 2 );
  QList< QList< QList< QgsPointV2 > > > coords;
  geom.coordinateSequence( coords );

  //get feature
  if ( coords.size() <= atVertex.part )
  {
    return; //error, no such feature
  }
  const QList< QList< QgsPointV2 > >& part = coords.at( atVertex.part );

  //get ring
  if ( part.size() <= atVertex.ring )
  {
    return; //error, no such ring
  }
  const QList< QgsPointV2 >& ring = part.at( atVertex.ring );
  if ( ring.size() <= atVertex.vertex )
  {
    return;
  }

  //vertex in the middle
  if ( atVertex.vertex > 0 && atVertex.vertex < ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
  }
  else if ( atVertex.vertex == 0 )
  {
    afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = atVertex.vertex + 1;
    if ( polygonType && ring.size() > 3 )
    {
      beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = ring.size() - 2;
    }
    else
    {
      beforeVertex = QgsVertexId(); //before vertex invalid
    }
  }
  else if ( atVertex.vertex == ring.size() - 1 )
  {
    beforeVertex.part = atVertex.part; beforeVertex.ring = atVertex.ring; beforeVertex.vertex = atVertex.vertex - 1;
    if ( polygonType )
    {
      afterVertex.part = atVertex.part; afterVertex.ring = atVertex.ring; afterVertex.vertex = 1;
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
  double temp, bc, cd, det;

  //closed circle
  if ( qgsDoubleNear( pt1.x(), pt3.x() ) && qgsDoubleNear( pt1.y(), pt3.y() ) )
  {
    centerX = pt2.x();
    centerY = pt2.y();
    radius = sqrt( pow( pt2.x() - pt1.x(), 2.0 ) + pow( pt2.y() - pt1.y(), 2.0 ) );
    return;
  }

  temp = pt2.x() * pt2.x() + pt2.y() * pt2.y();
  bc = ( pt1.x() * pt1.x() + pt1.y() * pt1.y() - temp ) / 2.0;
  cd = ( temp - pt3.x() * pt3.x() - pt3.y() * pt3.y() ) / 2.0;
  det = ( pt1.x() - pt2.x() ) * ( pt2.y() - pt3.y() ) - ( pt2.x() - pt3.x() ) * ( pt1.y() - pt2.y() );

  /* Check colinearity */
  if ( qgsDoubleNear( fabs( det ), 0.0 ) )
  {
    radius = -1.0;
    return;
  }

  det = 1.0 / det;
  centerX = ( bc * ( pt2.y() - pt3.y() ) - cd * ( pt1.y() - pt2.y() ) ) * det;
  centerY = (( pt1.x() - pt2.x() ) * cd - ( pt2.x() - pt3.x() ) * bc ) * det;
  radius = sqrt(( centerX - pt1.x() ) * ( centerX - pt1.x() ) + ( centerY - pt1.y() ) * ( centerY - pt1.y() ) );
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
  QList<QgsPointV2> possibleMidPoints;
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

QList<QgsPointV2> QgsGeometryUtils::pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure )
{
  int dim = 2 + is3D + isMeasure;
  QList<QgsPointV2> points;
  foreach ( const QString& pointCoordinates, wktCoordinateList.split( ",", QString::SkipEmptyParts ) )
  {
    QStringList coordinates = pointCoordinates.split( " ", QString::SkipEmptyParts );
    if ( coordinates.size() < dim )
      continue;

    int idx = 0;
    double x = coordinates[idx++].toDouble();
    double y = coordinates[idx++].toDouble();

    double z = is3D ? coordinates[idx++].toDouble() : 0.;
    double m = isMeasure ? coordinates[idx++].toDouble() : 0.;

    QgsWKBTypes::Type t = QgsWKBTypes::Point;
    if ( is3D )
    {
      if ( isMeasure )
        t = QgsWKBTypes::PointZM;
      else
        t = QgsWKBTypes::PointZ;
    }
    else
    {
      if ( isMeasure )
        t = QgsWKBTypes::PointM;
      else
        t = QgsWKBTypes::Point;
    }

    points.append( QgsPointV2( t, x, y, z, m ) );
  }
  return points;
}

void QgsGeometryUtils::pointsToWKB( QgsWkbPtr& wkb, const QList<QgsPointV2> &points, bool is3D, bool isMeasure )
{
  wkb << static_cast<quint32>( points.size() );
  foreach ( const QgsPointV2& point, points )
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

QString QgsGeometryUtils::pointsToWKT( const QList<QgsPointV2>& points, int precision, bool is3D, bool isMeasure )
{
  QString wkt = "(";
  foreach ( const QgsPointV2& p, points )
  {
    wkt += qgsDoubleToString( p.x(), precision );
    wkt += " " + qgsDoubleToString( p.y(), precision );
    if ( is3D )
      wkt += " " + qgsDoubleToString( p.z(), precision );
    if ( isMeasure )
      wkt += " " + qgsDoubleToString( p.m(), precision );
    wkt += ", ";
  }
  if ( wkt.endsWith( ", " ) )
    wkt.chop( 2 ); // Remove last ", "
  wkt += ")";
  return wkt;
}

QDomElement QgsGeometryUtils::pointsToGML2( const QList<QgsPointV2>& points, QDomDocument& doc, int precision, const QString &ns )
{
  QDomElement elemCoordinates = doc.createElementNS( ns, "coordinates" );

  QString strCoordinates;

  foreach ( const QgsPointV2& p, points )
    strCoordinates += qgsDoubleToString( p.x(), precision ) + "," + qgsDoubleToString( p.y(), precision ) + " ";

  if ( strCoordinates.endsWith( " " ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  return elemCoordinates;
}

QDomElement QgsGeometryUtils::pointsToGML3( const QList<QgsPointV2>& points, QDomDocument& doc, int precision, const QString &ns, bool is3D )
{
  QDomElement elemPosList = doc.createElementNS( ns, "posList" );
  elemPosList.setAttribute( "srsDimension", is3D ? 3 : 2 );

  QString strCoordinates;
  foreach ( const QgsPointV2& p, points )
  {
    strCoordinates += qgsDoubleToString( p.x(), precision ) + " " + qgsDoubleToString( p.y(), precision ) + " ";
    if ( is3D )
      strCoordinates += qgsDoubleToString( p.z(), precision ) + " ";
  }
  if ( strCoordinates.endsWith( " " ) )
    strCoordinates.chop( 1 ); // Remove trailing space

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  return elemPosList;
}

QString QgsGeometryUtils::pointsToJSON( const QList<QgsPointV2>& points, int precision )
{
  QString json = "[ ";
  foreach ( const QgsPointV2& p, points )
  {
    json += "[" + qgsDoubleToString( p.x(), precision ) + ", " + qgsDoubleToString( p.y(), precision ) + "], ";
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "]";
  return json;
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
        if ( block.startsWith( "(" ) && !defaultType.isEmpty() )
          block.prepend( defaultType + " " );
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
    if ( block.startsWith( "(" ) && !defaultType.isEmpty() )
      block.prepend( defaultType + " " );
    blocks.append( block );
  }
  return blocks;
}
