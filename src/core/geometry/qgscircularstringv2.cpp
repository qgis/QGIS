/***************************************************************************
                         qgscircularstringv2.cpp
                         -----------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscircularstringv2.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgsmaptopixel.h"
#include "qgspointv2.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>

QgsCircularStringV2::QgsCircularStringV2(): QgsCurveV2()
{

}

QgsCircularStringV2::~QgsCircularStringV2()
{

}

QgsAbstractGeometryV2 *QgsCircularStringV2::clone() const
{
  return new QgsCircularStringV2( *this );
}

void QgsCircularStringV2::clear()
{
  mX.clear();
  mY.clear();
  mZ.clear();
  mM.clear();
  mWkbType = QgsWKBTypes::Unknown;
}

QgsRectangle QgsCircularStringV2::calculateBoundingBox() const
{
  QgsRectangle bbox;
  int nPoints = numPoints();
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    if ( i == 0 )
    {
      bbox = segmentBoundingBox( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ) );
    }
    else
    {
      QgsRectangle segmentBox = segmentBoundingBox( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ) );
      bbox.combineExtentWith( &segmentBox );
    }
  }
  return bbox;
}

QgsRectangle QgsCircularStringV2::segmentBoundingBox( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3 )
{
  double centerX, centerY, radius;
  QgsGeometryUtils::circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );

  double p1Angle = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  if ( p1Angle > 360 )
  {
    p1Angle -= 360;
  }
  double p2Angle = QgsGeometryUtils::ccwAngle( pt2.y() - centerY, pt2.x() - centerX );
  if ( p2Angle > 360 )
  {
    p2Angle -= 360;
  }
  double p3Angle = QgsGeometryUtils::ccwAngle( pt3.y() - centerY, pt3.x() - centerX );
  if ( p3Angle > 360 )
  {
    p3Angle -= 360;
  }

  //start point, end point and compass points in between can be on bounding box
  QgsRectangle bbox( pt1.x(), pt1.y(), pt1.x(), pt1.y() );
  bbox.combineExtentWith( pt3.x(), pt3.y() );

  QList<QgsPointV2> compassPoints = compassPointsOnSegment( p1Angle, p2Angle, p3Angle, centerX, centerY, radius );
  QList<QgsPointV2>::const_iterator cpIt = compassPoints.constBegin();
  for ( ; cpIt != compassPoints.constEnd(); ++cpIt )
  {
    bbox.combineExtentWith( cpIt->x(), cpIt->y() );
  }
  return bbox;
}

QList<QgsPointV2> QgsCircularStringV2::compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius )
{
  QList<QgsPointV2> pointList;

  QgsPointV2 nPoint( centerX, centerY + radius );
  QgsPointV2 ePoint( centerX + radius, centerY );
  QgsPointV2 sPoint( centerX, centerY - radius );
  QgsPointV2 wPoint( centerX - radius, centerY );

  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      if ( p1Angle <= 90 && p3Angle >= 90 )
      {
        pointList.append( nPoint );
      }
      if ( p1Angle <= 180 && p3Angle >= 180 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle <= 270 && p3Angle >= 270 )
      {
        pointList.append( sPoint );
      }
    }
    else
    {
      pointList.append( ePoint );
      if ( p1Angle >= 90 || p3Angle <= 90 )
      {
        pointList.append( nPoint );
      }
      if ( p1Angle >= 180 || p3Angle <= 180 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle >= 270 || p3Angle <= 270 )
      {
        pointList.append( sPoint );
      }
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      if ( p1Angle >= 270 && p3Angle <= 270 )
      {
        pointList.append( sPoint );
      }
      if ( p1Angle >= 180 && p3Angle <= 180 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle >= 90 && p3Angle <= 90 )
      {
        pointList.append( nPoint );
      }
    }
    else
    {
      pointList.append( ePoint );
      if ( p1Angle <= 270 || p3Angle >= 270 )
      {
        pointList.append( sPoint );
      }
      if ( p1Angle <= 180 || p3Angle >= 180 )
      {
        pointList.append( wPoint );
      }
      if ( p1Angle <= 90 || p3Angle >= 90 )
      {
        pointList.append( nPoint );
      }
    }
  }
  return pointList;
}

bool QgsCircularStringV2::fromWkb( const unsigned char* wkb )
{
  if ( !wkb )
  {
    return false;
  }

  QgsConstWkbPtr wkbPtr( wkb );
  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::CircularString )
  {
    return false;
  }
  mWkbType = type;

  //type
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int nVertices = 0;
  wkbPtr >> nVertices;
  mX.resize( nVertices );
  mY.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();
  for ( int i = 0; i < nVertices; ++i )
  {
    wkbPtr >> mX[i];
    wkbPtr >> mY[i];
    if ( hasZ )
    {
      wkbPtr >> mZ[i];
    }
    if ( hasM )
    {
      wkbPtr >> mM[i];
    }
  }

  return true;
}

bool QgsCircularStringV2::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::parseType( geometryType() ) )
    return false;
  mWkbType = parts.first;

  setPoints( QgsGeometryUtils::pointsFromWKT( parts.second, is3D(), isMeasure() ) );
  return true;
}

int QgsCircularStringV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  size += numPoints() * ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return size;
}

unsigned char* QgsCircularStringV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  QList<QgsPointV2> pts;
  points( pts );
  QgsGeometryUtils::pointsToWKB( wkb, pts, is3D(), isMeasure() );
  return geomPtr;
}

QString QgsCircularStringV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " ";
  QList<QgsPointV2> pts;
  points( pts );
  wkt += QgsGeometryUtils::pointsToWKT( pts, precision, is3D(), isMeasure() );
  return wkt;
}

QDomElement QgsCircularStringV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QgsLineStringV2* line = curveToLine();
  QDomElement gml = line->asGML2( doc, precision, ns );
  delete line;
  return gml;
}

QDomElement QgsCircularStringV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QList<QgsPointV2> pts;
  points( pts );

  QDomElement elemCurve = doc.createElementNS( ns, "Curve" );
  QDomElement elemSegments = doc.createElementNS( ns, "segments" );
  QDomElement elemArcString = doc.createElementNS( ns, "ArcString" );
  elemArcString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
  elemSegments.appendChild( elemArcString );
  elemCurve.appendChild( elemSegments );
  return elemCurve;
}

QString QgsCircularStringV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QgsLineStringV2* line = curveToLine();
  QString json = line->asJSON( precision );
  delete line;
  return json;
}

//curve interface
double QgsCircularStringV2::length() const
{
  int nPoints = numPoints();
  double length = 0;
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    length += QgsGeometryUtils::circleLength( mX[i], mY[i], mX[i + 1], mY[i + 1], mX[i + 2], mY[i + 2] );
  }
  return length;
}

QgsPointV2 QgsCircularStringV2::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( 0 );
}

QgsPointV2 QgsCircularStringV2::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( numPoints() - 1 );
}

QgsLineStringV2* QgsCircularStringV2::curveToLine() const
{
  QgsLineStringV2* line = new QgsLineStringV2();
  QList<QgsPointV2> points;
  int nPoints = numPoints();

  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    segmentize( pointN( i ), pointN( i + 1 ), pointN( i + 2 ), points );
  }

  line->setPoints( points );
  return line;
}

int QgsCircularStringV2::numPoints() const
{
  return qMin( mX.size(), mY.size() );
}

QgsPointV2 QgsCircularStringV2::pointN( int i ) const
{
  if ( qMin( mX.size(), mY.size() ) <= i )
  {
    return QgsPointV2();
  }

  double x = mX.at( i );
  double y = mY.at( i );
  double z = 0;
  double m = 0;

  if ( is3D() )
  {
    z = mZ.at( i );
  }
  if ( isMeasure() )
  {
    m = mM.at( i );
  }

  QgsWKBTypes::Type t = QgsWKBTypes::Point;
  if ( is3D() && isMeasure() )
  {
    t = QgsWKBTypes::PointZM;
  }
  else if ( is3D() )
  {
    t = QgsWKBTypes::PointZ;
  }
  else if ( isMeasure() )
  {
    t = QgsWKBTypes::PointM;
  }
  return QgsPointV2( t, x, y, z, m );
}

void QgsCircularStringV2::points( QList<QgsPointV2>& pts ) const
{
  pts.clear();
  int nPts = numPoints();
  for ( int i = 0; i < nPts; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsCircularStringV2::setPoints( const QList<QgsPointV2>& points )
{
  if ( points.size() < 1 )
  {
    mWkbType = QgsWKBTypes::Unknown; mX.clear(); mY.clear(); mZ.clear(); mM.clear();
    return;
  }

  //get wkb type from first point
  const QgsPointV2& firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWKBTypes::CircularString );

  mX.resize( points.size() );
  mY.resize( points.size() );
  if ( hasZ )
  {
    mZ.resize( points.size() );
  }
  else
  {
    mZ.clear();
  }
  if ( hasM )
  {
    mM.resize( points.size() );
  }
  else
  {
    mM.clear();
  }

  for ( int i = 0; i < points.size(); ++i )
  {
    mX[i] = points[i].x();
    mY[i] = points[i].y();
    if ( hasZ )
    {
      mZ[i] = points[i].z();
    }
    if ( hasM )
    {
      mM[i] = points[i].m();
    }
  }
}

void QgsCircularStringV2::segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const
{
  //adapted code from postgis
  double radius = 0;
  double centerX = 0;
  double centerY = 0;
  QgsGeometryUtils::circleCenterRadius( p1, p2, p3, radius, centerX, centerY );
  int segSide = segmentSide( p1, p3, p2 );

  if ( radius < 0 || qgsDoubleNear( segSide, 0.0 ) ) //points are colinear
  {
    points.append( p1 );
    points.append( p2 );
    points.append( p3 );
    return;
  }

  bool clockwise = false;
  if ( segSide == -1 )
  {
    clockwise = true;
  }

  double increment = fabs( M_PI_2 / 90 ); //one segment per degree

  //angles of pt1, pt2, pt3
  double a1 = atan2( p1.y() - centerY, p1.x() - centerX );
  double a2 = atan2( p2.y() - centerY, p2.x() - centerX );
  double a3 = atan2( p3.y() - centerY, p3.x() - centerX );

  if ( clockwise )
  {
    increment *= -1;
    /* Adjust a3 down so we can decrement from a1 to a3 cleanly */
    if ( a3 > a1 )
      a3 -= 2.0 * M_PI;
    if ( a2 > a1 )
      a2 -= 2.0 * M_PI;
  }
  else
  {
    /* Adjust a3 up so we can increment from a1 to a3 cleanly */
    if ( a3 < a1 )
      a3 += 2.0 * M_PI;
    if ( a2 < a1 )
      a2 += 2.0 * M_PI;
  }

  bool hasZ = is3D();
  bool hasM = isMeasure();

  double x, y;
  double z = 0;
  double m = 0;

  points.append( p1 );
  if ( p2 != p3 && p1 != p2 ) //draw straight line segment if two points have the same position
  {
    for ( double angle = a1 + increment; clockwise ? angle > a3 : angle < a3; angle += increment )
    {
      x = centerX + radius * cos( angle );
      y = centerY + radius * sin( angle );

      if ( !hasZ && !hasM )
      {
        points.append( QgsPointV2( x, y ) );
        continue;
      }

      if ( hasZ )
      {
        z = interpolateArc( angle, a1, a2, a3, p1.z(), p2.z(), p3.z() );
      }
      if ( hasM )
      {
        m = interpolateArc( angle, a1, a2, a3, p1.m(), p2.m(), p3.m() );
      }

      points.append( QgsPointV2( mWkbType, x, y, z, m ) );
    }
  }
  points.append( p3 );
}

int QgsCircularStringV2::segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const
{
  double side = (( pt2.x() - pt1.x() ) * ( pt3.y() - pt1.y() ) - ( pt3.x() - pt1.x() ) * ( pt2.y() - pt1.y() ) );
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

double QgsCircularStringV2::interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const
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

void QgsCircularStringV2::draw( QPainter& p ) const
{
  QPainterPath path;
  addToPainterPath( path );
  p.drawPath( path );
}

void QgsCircularStringV2::transform( const QgsCoordinateTransform& ct )
{
  double* zArray = mZ.data();

  bool hasZ = is3D();
  int nPoints = numPoints();
  if ( !hasZ )
  {
    zArray = new double[nPoints];
    for ( int i = 0; i < nPoints; ++i )
    {
      zArray[i] = 0;
    }
  }
  ct.transformCoords( nPoints, mX.data(), mY.data(), zArray );
  if ( !hasZ )
  {
    delete[] zArray;
  }
}

void QgsCircularStringV2::transform( const QTransform& t )
{
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    t.map( mX[i], mY[i], &mX[i], &mY[i] );
  }
}

void QgsCircularStringV2::clip( const QgsRectangle& rect )
{
  //todo...
}

void QgsCircularStringV2::addToPainterPath( QPainterPath& path ) const
{
  int nPoints = numPoints();
  if ( nPoints < 1 )
  {
    return;
  }

  if ( path.isEmpty() || path.currentPosition() != QPointF( mX[0], mY[0] ) )
  {
    path.moveTo( QPointF( mX[0], mY[0] ) );
  }

  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    QList<QgsPointV2> pt;
    segmentize( QgsPointV2( mX[i], mY[i] ), QgsPointV2( mX[i + 1], mY[i + 1] ), QgsPointV2( mX[i + 2], mY[i + 2] ), pt );
    for ( int j = 1; j < pt.size(); ++j )
    {
      path.lineTo( pt.at( j ).x(), pt.at( j ).y() );
    }
    //arcTo( path, QPointF( mX[i], mY[i] ), QPointF( mX[i + 1], mY[i + 1] ), QPointF( mX[i + 2], mY[i + 2] ) );
  }
}

void QgsCircularStringV2::arcTo( QPainterPath& path, const QPointF& pt1, const QPointF& pt2, const QPointF& pt3 )
{
  double centerX, centerY, radius;
  QgsGeometryUtils::circleCenterRadius( QgsPointV2( pt1.x(), pt1.y() ), QgsPointV2( pt2.x(), pt2.y() ), QgsPointV2( pt3.x(), pt3.y() ),
                                        radius, centerX, centerY );

  double p1Angle = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double sweepAngle = QgsGeometryUtils::sweepAngle( centerX, centerY, pt1.x(), pt1.y(), pt2.x(), pt2.y(), pt3.x(), pt3.y() );

  double diameter = 2 * radius;
  path.arcTo( centerX - radius, centerY - radius, diameter, diameter, p1Angle, sweepAngle );
}

void QgsCircularStringV2::drawAsPolygon( QPainter& p ) const
{
  draw( p );
}

bool QgsCircularStringV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  if ( position.vertex > mX.size() || position.vertex < 1 )
  {
    return false;
  }

  mX.insert( position.vertex, vertex.x() );
  mY.insert( position.vertex, vertex.y() );
  if ( is3D() )
  {
    mZ.insert( position.vertex, vertex.z() );
  }
  if ( isMeasure() )
  {
    mM.insert( position.vertex, vertex.m() );
  }

  bool vertexNrEven = ( position.vertex % 2 == 0 );
  if ( vertexNrEven )
  {
    insertVertexBetween( position.vertex - 2, position.vertex - 1, position.vertex );
  }
  else
  {
    insertVertexBetween( position.vertex, position.vertex + 1, position.vertex - 1 );
  }
  return true;
}

bool QgsCircularStringV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  if ( position.vertex < 0 || position.vertex > mX.size() )
  {
    return false;
  }

  mX[position.vertex] = newPos.x();
  mY[position.vertex] = newPos.y();
  if ( is3D() && newPos.is3D() )
  {
    mZ[position.vertex] = newPos.z();
  }
  if ( isMeasure() && newPos.isMeasure() )
  {
    mM[position.vertex] = newPos.m();
  }
  mBoundingBox = QgsRectangle(); //set bounding box invalid
  return true;
}

bool QgsCircularStringV2::deleteVertex( const QgsVertexId& position )
{
  int nVertices = this->numPoints();
  if ( nVertices < 4 ) //circular string must have at least 3 vertices
  {
    return false;
  }
  if ( position.vertex < 1 || position.vertex > ( nVertices - 2 ) )
  {
    return false;
  }

  if ( position.vertex < ( nVertices - 2 ) )
  {
    //remove this and the following vertex
    deleteVertex( position.vertex + 1 );
    deleteVertex( position.vertex );
  }
  else //remove this and the preceding vertex
  {
    deleteVertex( position.vertex );
    deleteVertex( position.vertex - 1 );
  }

  return true;
}

void QgsCircularStringV2::deleteVertex( int i )
{
  mX.remove( i );
  mY.remove( i );
  if ( is3D() )
  {
    mZ.remove( i );
  }
  if ( isMeasure() )
  {
    mM.remove( i );
  }
}

double QgsCircularStringV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  Q_UNUSED( epsilon );
  double minDist = std::numeric_limits<double>::max();
  QgsPointV2 minDistSegmentPoint;
  QgsVertexId minDistVertexAfter;
  bool minDistLeftOf = false;

  double currentDist = 0.0;

  int nPoints = numPoints();
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    currentDist = closestPointOnArc( mX[i], mY[i], mX[i + 1], mY[i + 1], mX[i + 2], mY[i + 2], pt, segmentPt, vertexAfter, leftOf, epsilon );
    if ( currentDist < minDist )
    {
      minDist = currentDist;
      minDistSegmentPoint = segmentPt;
      minDistVertexAfter.vertex = vertexAfter.vertex + i;
      if ( leftOf )
      {
        minDistLeftOf = *leftOf;
      }
    }
  }

  segmentPt = minDistSegmentPoint;
  vertexAfter = minDistVertexAfter;
  vertexAfter.part = 0; vertexAfter.ring = 0;
  if ( leftOf )
  {
    *leftOf = minDistLeftOf;
  }
  return minDist;
}

bool QgsCircularStringV2::pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const
{
  if ( i >= numPoints() )
  {
    return false;
  }
  vertex = pointN( i );
  type = ( i % 2 == 0 ) ? QgsVertexId::SegmentVertex : QgsVertexId::CurveVertex;
  return true;
}

void QgsCircularStringV2::sumUpArea( double& sum ) const
{
  int maxIndex = numPoints() - 1;

  for ( int i = 0; i < maxIndex; i += 2 )
  {
    QgsPointV2 p1( mX[i], mY[i] );
    QgsPointV2 p2( mX[i + 1], mY[i + 1] );
    QgsPointV2 p3( mX[i + 2], mY[i + 2] );

    //segment is a full circle, p2 is the center point
    if ( p1 == p3 )
    {
      double r2 = QgsGeometryUtils::sqrDistance2D( p1, p2 );
      sum += M_PI * r2;
      continue;
    }

    sum += 0.5 * ( mX[i] * mY[i+2] - mY[i] * mX[i+2] );

    //calculate area between circle and chord, then sum / subtract from total area
    double midPointX = ( p1.x() + p3.x() ) / 2.0;
    double midPointY = ( p1.y() + p3.y() ) / 2.0;

    double radius, centerX, centerY;
    QgsGeometryUtils::circleCenterRadius( p1, p2, p3, radius, centerX, centerY );

    double d = sqrt( QgsGeometryUtils::sqrDistance2D( QgsPointV2( centerX, centerY ), QgsPointV2( midPointX, midPointY ) ) );
    double r2 = radius * radius;

    if ( d > radius )
    {
      //d cannot be greater than radius, something must be wrong...
      continue;
    }

    bool circlePointLeftOfLine = QgsGeometryUtils::leftOfLine( p2.x(), p2.y(), p1.x(), p1.y(), p3.x(), p3.y() ) < 0;
    bool centerPointLeftOfLine = QgsGeometryUtils::leftOfLine( centerX, centerY, p1.x(), p1.y(), p3.x(), p3.y() ) < 0;

    double cov = 0.5 - d * sqrt( r2 - d * d ) / ( M_PI * r2 ) - 1 / M_PI * asin( d / radius );
    double circleChordArea = 0;
    if ( circlePointLeftOfLine == centerPointLeftOfLine )
    {
      circleChordArea = M_PI * r2 * ( 1 - cov );
    }
    else
    {
      circleChordArea = M_PI * r2 * cov;
    }

    if ( !circlePointLeftOfLine )
    {
      sum += circleChordArea;
    }
    else
    {
      sum -= circleChordArea;
    }
  }
}

double QgsCircularStringV2::closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
    const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon )
{
  double radius, centerX, centerY;
  QgsPointV2 pt1( x1, y1 );
  QgsPointV2 pt2( x2, y2 );
  QgsPointV2 pt3( x3, y3 );

  QgsGeometryUtils::circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );
  double angle = QgsGeometryUtils::ccwAngle( pt.y() - centerY, pt.x() - centerX );
  double angle1 = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double angle2 = QgsGeometryUtils::ccwAngle( pt2.y() - centerY, pt2.x() - centerX );
  double angle3 = QgsGeometryUtils::ccwAngle( pt3.y() - centerY, pt3.x() - centerX );

  bool clockwise = QgsGeometryUtils::circleClockwise( angle1, angle2, angle3 );

  if ( QgsGeometryUtils::angleOnCircle( angle, angle1, angle2, angle3 ) )
  {
    //get point on line center -> pt with distance radius
    segmentPt = QgsGeometryUtils::pointOnLineWithDistance( QgsPointV2( centerX, centerY ), pt, radius );

    //vertexAfter
    vertexAfter.vertex = QgsGeometryUtils::circleAngleBetween( angle, angle1, angle2, clockwise ) ? 1 : 2;
  }
  else
  {
    double distPtPt1 = QgsGeometryUtils::sqrDistance2D( pt, pt1 );
    double distPtPt3 = QgsGeometryUtils::sqrDistance2D( pt, pt3 );
    segmentPt = ( distPtPt1 <= distPtPt3 ) ? pt1 : pt3;
    vertexAfter.vertex = ( distPtPt1 <= distPtPt3 ) ? 1 : 2;
  }

  double sqrDistance = QgsGeometryUtils::sqrDistance2D( segmentPt, pt );
  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( sqrDistance, 0.0, epsilon ) )
  {
    segmentPt.setX( pt.x() );
    segmentPt.setY( pt.y() );
    sqrDistance = 0.0;
  }

  if ( leftOf )
  {
    *leftOf = clockwise ? sqrDistance > radius : sqrDistance < radius;
  }

  return sqrDistance;
}

void QgsCircularStringV2::insertVertexBetween( int after, int before, int pointOnCircle )
{
  double xAfter = mX[after];
  double yAfter = mY[after];
  double xBefore = mX[before];
  double yBefore = mY[before];
  double xOnCircle = mX[ pointOnCircle ];
  double yOnCircle = mY[ pointOnCircle ];

  double radius, centerX, centerY;
  QgsGeometryUtils::circleCenterRadius( QgsPointV2( xAfter, yAfter ), QgsPointV2( xBefore, yBefore ), QgsPointV2( xOnCircle, yOnCircle ), radius, centerX, centerY );

  double x = ( xAfter + xBefore ) / 2.0;
  double y = ( yAfter + yBefore ) / 2.0;

  QgsPointV2 newVertex = QgsGeometryUtils::pointOnLineWithDistance( QgsPointV2( centerX, centerY ), QgsPointV2( x, y ), radius );
  mX.insert( before, newVertex.x() );
  mY.insert( before, newVertex.y() );

  if ( is3D() )
  {
    mZ.insert( before, ( mZ[after] + mZ[before] ) / 2.0 );
  }
  if ( isMeasure() )
  {
    mM.insert( before, ( mM[after] + mM[before] ) / 2.0 );
  }
}
