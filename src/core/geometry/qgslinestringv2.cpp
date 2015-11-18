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

QgsLineStringV2 *QgsLineStringV2::clone() const
{
  return new QgsLineStringV2( *this );
}

void QgsLineStringV2::clear()
{
  mX.clear();
  mY.clear();
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
  QString wkt = wktTypeStr() + ' ';
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

  return "{\"type\": \"LineString\", \"coordinates\": " + QgsGeometryUtils::pointsToJSON( pts, precision ) + '}';
}

double QgsLineStringV2::length() const
{
  double length = 0;
  int size = mX.size();
  double dx, dy;
  for ( int i = 1; i < size; ++i )
  {
    dx = mX.at( i ) - mX.at( i - 1 );
    dy = mY.at( i ) - mY.at( i - 1 );
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
  return mX.size();
}

QgsPointV2 QgsLineStringV2::pointN( int i ) const
{
  if ( mX.size() <= i )
  {
    return QgsPointV2();
  }

  double x = mX.at( i );
  double y = mY.at( i );
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
  return QgsPointV2( t, x, y, z, m );
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
    mX.clear();
    mY.clear();
    mZ.clear();
    mM.clear();
    return;
  }

  //get wkb type from first point
  const QgsPointV2& firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWKBTypes::LineString );

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
    mX[i] = points.at( i ).x();
    mY[i] = points.at( i ).y();
    if ( hasZ )
    {
      mZ[i] = points.at( i ).z();
    }
    if ( hasM )
    {
      mM[i] = points.at( i ).m();
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

  mX += line->mX;
  mY += line->mY;
  mZ += line->mZ;
  mM += line->mM;
}

void QgsLineStringV2::draw( QPainter& p ) const
{
  p.drawPolyline( qPolygonF() );
}

void QgsLineStringV2::addToPainterPath( QPainterPath& path ) const
{
  int nPoints = numPoints();
  if ( nPoints < 1 )
  {
    return;
  }

  if ( path.isEmpty() || path.currentPosition() != QPointF( mX.at( 0 ), mY.at( 0 ) ) )
  {
    path.moveTo( mX.at( 0 ), mY.at( 0 ) );
  }

  for ( int i = 1; i < nPoints; ++i )
  {
    path.lineTo( mX.at( i ), mY.at( i ) );
  }
}

void QgsLineStringV2::drawAsPolygon( QPainter& p ) const
{
  p.drawPolygon( qPolygonF() );
}

QPolygonF QgsLineStringV2::qPolygonF() const
{
  QPolygonF points;
  for ( int i = 0; i < mX.count(); ++i )
  {
    points << QPointF( mX.at( i ), mY.at( i ) );
  }
  return points;
}

void QgsLineStringV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d )
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
  ct.transformCoords( nPoints, mX.data(), mY.data(), zArray, d );
  if ( !hasZ )
  {
    delete[] zArray;
  }

}

void QgsLineStringV2::transform( const QTransform& t )
{
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    qreal x, y;
    t.map( mX.at( i ), mY.at( i ), &x, &y );
    mX[i] = x; mY[i] = y;
  }
}

bool QgsLineStringV2::insertVertex( const QgsVertexId& position, const QgsPointV2& vertex )
{
  if ( position.vertex < 0 || position.vertex > mX.size() )
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
  mBoundingBox = QgsRectangle(); //set bounding box invalid
  return true;
}

bool QgsLineStringV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  if ( position.vertex < 0 || position.vertex >= mX.size() )
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

bool QgsLineStringV2::deleteVertex( const QgsVertexId& position )
{
  if ( position.vertex >= mX.size() || position.vertex < 0 )
  {
    return false;
  }

  mX.remove( position.vertex );
  mY.remove( position.vertex );
  if ( is3D() )
  {
    mZ.remove( position.vertex );
  }
  if ( isMeasure() )
  {
    mM.remove( position.vertex );
  }
  mBoundingBox = QgsRectangle(); //set bounding box invalid
  return true;
}

void QgsLineStringV2::addVertex( const QgsPointV2& pt )
{
  if ( mWkbType == QgsWKBTypes::Unknown )
  {
    setZMTypeFromSubGeometry( &pt, QgsWKBTypes::LineString );
  }

  mX.append( pt.x() );
  mY.append( pt.y() );
  if ( is3D() )
  {
    mZ.append( pt.z() );
  }
  if ( isMeasure() )
  {
    mM.append( pt.m() );
  }
  mBoundingBox = QgsRectangle(); //set bounding box invalid
}

double QgsLineStringV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  double sqrDist = std::numeric_limits<double>::max();
  double testDist = 0;
  double segmentPtX, segmentPtY;

  int size = mX.size();
  for ( int i = 1; i < size; ++i )
  {
    double prevX = mX.at( i - 1 );
    double prevY = mY.at( i - 1 );
    double currentX = mX.at( i );
    double currentY = mY.at( i );
    testDist = QgsGeometryUtils::sqrDistToLine( pt.x(), pt.y(), prevX, prevY, currentX, currentY, segmentPtX, segmentPtY, epsilon );
    if ( testDist < sqrDist )
    {
      sqrDist = testDist;
      segmentPt.setX( segmentPtX );
      segmentPt.setY( segmentPtY );
      if ( leftOf )
      {
        *leftOf = ( QgsGeometryUtils::leftOfLine( segmentPtX, segmentPtY, prevX, prevY, pt.x(), pt.y() ) < 0 );
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
    sum += 0.5 * ( mX.at( i ) * mY.at( i + 1 ) - mY.at( i ) * mX.at( i + 1 ) );
  }
}

void QgsLineStringV2::importVerticesFromWkb( const QgsConstWkbPtr& wkb )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int nVertices = 0;
  wkb >> nVertices;
  mX.resize( nVertices );
  mY.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();
  for ( int i = 0; i < nVertices; ++i )
  {
    wkb >> mX[i];
    wkb >> mY[i];
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

double QgsLineStringV2::vertexAngle( const QgsVertexId& vertex ) const
{
  if ( vertex.vertex == 0 || vertex.vertex >= ( numPoints() - 1 ) )
  {
    if ( isClosed() )
    {
      double previousX = mX.at( numPoints() - 1 );
      double previousY = mY.at( numPoints() - 1 );
      double currentX = mX.at( 0 );
      double currentY = mY.at( 0 );
      double afterX = mX.at( 1 );
      double afterY = mY.at( 1 );
      return QgsGeometryUtils::averageAngle( previousX, previousY, currentX, currentY, afterX, afterY );
    }
    else if ( vertex.vertex == 0 )
    {
      return QgsGeometryUtils::linePerpendicularAngle( mX.at( 0 ), mY.at( 0 ), mX.at( 1 ), mY.at( 1 ) );
    }
    else
    {
      int a = numPoints() - 2;
      int b = numPoints() - 1;
      return QgsGeometryUtils::linePerpendicularAngle( mX.at( a ), mY.at( a ), mX.at( b ), mY.at( b ) );
    }
  }
  else
  {
    double previousX = mX.at( vertex.vertex - 1 );
    double previousY = mY.at( vertex.vertex - 1 );
    double currentX = mX.at( vertex.vertex );
    double currentY = mY.at( vertex.vertex );
    double afterX = mX.at( vertex.vertex + 1 );
    double afterY = mY.at( vertex.vertex + 1 );
    return QgsGeometryUtils::averageAngle( previousX, previousY, currentX, currentY, afterX, afterY );
  }
}

bool QgsLineStringV2::addZValue( double zValue )
{
  if ( QgsWKBTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addZ( mWkbType );

  mZ.clear();
  int nPoints = numPoints();
  mZ.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mZ << zValue;
  }
  return true;
}

bool QgsLineStringV2::addMValue( double mValue )
{
  if ( QgsWKBTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addM( mWkbType );

  mM.clear();
  int nPoints = numPoints();
  mM.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mM << mValue;
  }
  return true;
}
