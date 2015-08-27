/***************************************************************************
                         qgslinestringv2.cpp
                         -------------------
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

#include "qgslinestringv2.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <limits>
#include <QDomDocument>

QgsLineStringV2::QgsLineStringV2(): QgsCurveV2()
{
  mWkbType = QgsWKBTypes::LineString;
}

QgsLineStringV2::~QgsLineStringV2()
{}

QgsAbstractGeometryV2 *QgsLineStringV2::clone() const
{
  return new QgsLineStringV2( *this );
}

void QgsLineStringV2::clear()
{
  mCoords.clear();
  mZ.clear();
  mM.clear();
  mWkbType = QgsWKBTypes::Unknown;
}

bool QgsLineStringV2::fromWkb( const unsigned char* wkb )
{
  if ( !wkb )
  {
    return false;
  }
  QgsConstWkbPtr wkbPtr( wkb );
  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::LineString )
  {
    return false;
  }
  mWkbType = type;
  importVerticesFromWkb( wkbPtr );
  return true;
}

void QgsLineStringV2::fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb )
{
  mWkbType = type;
  importVerticesFromWkb( wkb );
}

bool QgsLineStringV2::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::parseType( geometryType() ) )
    return false;
  mWkbType = parts.first;

  setPoints( QgsGeometryUtils::pointsFromWKT( parts.second, is3D(), isMeasure() ) );
  return true;
}

int QgsLineStringV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  size += numPoints() * ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return size;
}

unsigned char* QgsLineStringV2::asWkb( int& binarySize ) const
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

QString QgsLineStringV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " ";
  QList<QgsPointV2> pts;
  points( pts );
  wkt += QgsGeometryUtils::pointsToWKT( pts, precision, is3D(), isMeasure() );
  return wkt;
}

QDomElement QgsLineStringV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QList<QgsPointV2> pts;
  points( pts );

  QDomElement elemLineString = doc.createElementNS( ns, "LineString" );
  elemLineString.appendChild( QgsGeometryUtils::pointsToGML2( pts, doc, precision, ns ) );

  return elemLineString;
}

QDomElement QgsLineStringV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QList<QgsPointV2> pts;
  points( pts );

  QDomElement elemCurve = doc.createElementNS( ns, "Curve" );
  QDomElement elemSegments = doc.createElementNS( ns, "segments" );
  QDomElement elemArcString = doc.createElementNS( ns, "LineString" );
  elemArcString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
  elemSegments.appendChild( elemArcString );
  elemCurve.appendChild( elemSegments );

  return elemCurve;
}

QString QgsLineStringV2::asJSON( int precision ) const
{
  QList<QgsPointV2> pts;
  points( pts );

  return "{\"type\": \"LineString\", \"coordinates\": " + QgsGeometryUtils::pointsToJSON( pts, precision ) + "}";
}

double QgsLineStringV2::length() const
{
  double length = 0;
  int size = mCoords.size();
  double dx, dy;
  for ( int i = 1; i < size; ++i )
  {
    dx = mCoords[i].x() - mCoords[ i - 1 ].x();
    dy = mCoords[i].y() - mCoords[ i - 1 ].y();
    length += sqrt( dx * dx + dy * dy );
  }
  return length;
}

QgsPointV2 QgsLineStringV2::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( 0 );
}

QgsPointV2 QgsLineStringV2::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( numPoints() - 1 );
}

QgsLineStringV2* QgsLineStringV2::curveToLine() const
{
  return static_cast<QgsLineStringV2*>( clone() );
}

int QgsLineStringV2::numPoints() const
{
  return mCoords.size();
}

QgsPointV2 QgsLineStringV2::pointN( int i ) const
{
  if ( mCoords.size() <= i )
  {
    return QgsPointV2();
  }

  const QPointF& pt = mCoords.at( i );
  double z = 0;
  double m = 0;

  bool hasZ = is3D();
  if ( hasZ )
  {
    z = mZ.at( i );
  }
  bool hasM = isMeasure();
  if ( hasM )
  {
    m = mM.at( i );
  }

  QgsWKBTypes::Type t = QgsWKBTypes::Point;
  if ( hasZ && hasM )
  {
    t = QgsWKBTypes::PointZM;
  }
  else if ( hasZ )
  {
    t = QgsWKBTypes::PointZ;
  }
  else if ( hasM )
  {
    t = QgsWKBTypes::PointM;
  }
  return QgsPointV2( t, pt.x(), pt.y(), z, m );
}

void QgsLineStringV2::points( QList<QgsPointV2>& pts ) const
{
  pts.clear();
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsLineStringV2::setPoints( const QList<QgsPointV2>& points )
{
  if ( points.size() < 1 )
  {
    mWkbType = QgsWKBTypes::Unknown;
    mCoords.clear();
    mZ.clear();
    mM.clear();
    return;
  }

  //get wkb type from first point
  const QgsPointV2& firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWKBTypes::LineString );

  mCoords.resize( points.size() );
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
    mCoords[i].rx() = points[i].x();
    mCoords[i].ry() = points[i].y();
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

void QgsLineStringV2::append( const QgsLineStringV2* line )
{
  if ( !line )
  {
    return;
  }

  if ( numPoints() < 1 )
  {
    setZMTypeFromSubGeometry( line, QgsWKBTypes::LineString );
  }

  mCoords += line->mCoords;
  mZ += line->mZ;
  mM += line->mM;
}

void QgsLineStringV2::draw( QPainter& p ) const
{
  p.drawPolyline( mCoords );
}

void QgsLineStringV2::addToPainterPath( QPainterPath& path ) const
{
  int nPoints = numPoints();
  if ( nPoints < 1 )
  {
    return;
  }

  if ( path.isEmpty() || path.currentPosition() != mCoords[0] )
  {
    path.moveTo( mCoords[0] );
  }

  for ( int i = 1; i < nPoints; ++i )
  {
    path.lineTo( mCoords[i] );
  }
}

void QgsLineStringV2::drawAsPolygon( QPainter& p ) const
{
  p.drawPolygon( mCoords );
}

void QgsLineStringV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d )
{
  ct.transformPolygon( mCoords, d );
}

void QgsLineStringV2::transform( const QTransform& t )
{
  mCoords = t.map( mCoords );
}

bool QgsLineStringV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  if ( position.vertex < 0 || position.vertex > mCoords.size() )
  {
    return false;
  }
  mCoords.insert( position.vertex, QPointF( vertex.x(), vertex.y() ) );
  if ( is3D() )
  {
    mZ.insert( position.vertex, vertex.z() );
  }
  if ( isMeasure() )
  {
    mM.insert( position.vertex, vertex.m() );
  }
  return true;
}

bool QgsLineStringV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  if ( position.vertex < 0 || position.vertex >= mCoords.size() )
  {
    return false;
  }
  mCoords[position.vertex].rx() = newPos.x();
  mCoords[position.vertex].ry() = newPos.y();
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

bool QgsLineStringV2::deleteVertex( const QgsVertexId& position )
{
  if ( position.vertex >= mCoords.size() || position.vertex < 0 )
  {
    return false;
  }

  mCoords.remove( position.vertex );
  if ( is3D() )
  {
    mZ.remove( position.vertex );
  }
  if ( isMeasure() )
  {
    mM.remove( position.vertex );
  }
  return true;
}

void QgsLineStringV2::addVertex( const QgsPointV2& pt )
{
  if ( mWkbType == QgsWKBTypes::Unknown )
  {
    setZMTypeFromSubGeometry( &pt, QgsWKBTypes::LineString );
  }

  mCoords.append( QPointF( pt.x(), pt.y() ) );
  if ( is3D() )
  {
    mZ.append( pt.z() );
  }
  if ( isMeasure() )
  {
    mM.append( pt.m() );
  }
}

double QgsLineStringV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  double sqrDist = std::numeric_limits<double>::max();
  double testDist = 0;
  double segmentPtX, segmentPtY;

  int size = mCoords.size();
  for ( int i = 1; i < size; ++i )
  {
    const QPointF& prev = mCoords.at( i - 1 );
    const QPointF& currentPt = mCoords.at( i );
    testDist = QgsGeometryUtils::sqrDistToLine( pt.x(), pt.y(), prev.x(), prev.y(), currentPt.x(), currentPt.y(), segmentPtX, segmentPtY, epsilon );
    if ( testDist < sqrDist )
    {
      sqrDist = testDist;
      segmentPt.setX( segmentPtX );
      segmentPt.setY( segmentPtY );
      if ( leftOf )
      {
        *leftOf = ( QgsGeometryUtils::leftOfLine( segmentPtX, segmentPtY, prev.x(), prev.y(), pt.x(), pt.y() ) < 0 );
      }
      vertexAfter.part = 0; vertexAfter.ring = 0; vertexAfter.vertex = i;
    }
  }
  return sqrDist;
}

bool QgsLineStringV2::pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const
{
  if ( i >= numPoints() )
  {
    return false;
  }
  vertex = pointN( i );
  type = QgsVertexId::SegmentVertex;
  return true;
}

void QgsLineStringV2::sumUpArea( double& sum ) const
{
  int maxIndex = numPoints() - 1;
  for ( int i = 0; i < maxIndex; ++i )
  {
    sum += 0.5 * ( mCoords[i].x() * mCoords[i+1].y() - mCoords[i].y() * mCoords[i+1].x() );
  }
}

void QgsLineStringV2::importVerticesFromWkb( const QgsConstWkbPtr& wkb )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int nVertices = 0;
  wkb >> nVertices;
  mCoords.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();
  for ( int i = 0; i < nVertices; ++i )
  {
    wkb >> mCoords[i].rx();
    wkb >> mCoords[i].ry();
    if ( hasZ )
    {
      wkb >> mZ[i];
    }
    if ( hasM )
    {
      wkb >> mM[i];
    }
  }
}

void QgsLineStringV2::close()
{
  if ( numPoints() < 1 || isClosed() )
  {
    return;
  }
  addVertex( startPoint() );
}
