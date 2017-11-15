/***************************************************************************
                         qgslinestring.cpp
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

#include "qgslinestring.h"
#include "qgsapplication.h"
#include "qgscompoundcurve.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"

#include <cmath>
#include <memory>
#include <QPainter>
#include <limits>
#include <QDomDocument>


/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsLineString::QgsLineString()
{
  mWkbType = QgsWkbTypes::LineString;
}

QgsLineString::QgsLineString( const QVector<QgsPoint> &points )
{
  if ( points.isEmpty() )
  {
    mWkbType = QgsWkbTypes::LineString;
    return;
  }
  QgsWkbTypes::Type ptType = points.at( 0 ).wkbType();
  mWkbType = QgsWkbTypes::zmType( QgsWkbTypes::LineString, QgsWkbTypes::hasZ( ptType ), QgsWkbTypes::hasM( ptType ) );
  mX.resize( points.count() );
  mY.resize( points.count() );
  double *x = mX.data();
  double *y = mY.data();
  double *z = nullptr;
  double *m = nullptr;
  if ( QgsWkbTypes::hasZ( mWkbType ) )
  {
    mZ.resize( points.count() );
    z = mZ.data();
  }
  if ( QgsWkbTypes::hasM( mWkbType ) )
  {
    mM.resize( points.count() );
    m = mM.data();
  }

  for ( const QgsPoint &pt : points )
  {
    *x++ = pt.x();
    *y++ = pt.y();
    if ( z )
      *z++ = pt.z();
    if ( m )
      *m++ = pt.m();
  }
}

QgsLineString::QgsLineString( const QVector<double> &x, const QVector<double> &y, const QVector<double> &z, const QVector<double> &m )
{
  mWkbType = QgsWkbTypes::LineString;
  int pointCount = std::min( x.size(), y.size() );
  if ( x.size() == pointCount )
  {
    mX = x;
  }
  else
  {
    mX = x.mid( 0, pointCount );
  }
  if ( y.size() == pointCount )
  {
    mY = y;
  }
  else
  {
    mY = y.mid( 0, pointCount );
  }
  if ( !z.isEmpty() && z.count() >= pointCount )
  {
    mWkbType = QgsWkbTypes::addZ( mWkbType );
    if ( z.size() == pointCount )
    {
      mZ = z;
    }
    else
    {
      mZ = z.mid( 0, pointCount );
    }
  }
  if ( !m.isEmpty() && m.count() >= pointCount )
  {
    mWkbType = QgsWkbTypes::addM( mWkbType );
    if ( m.size() == pointCount )
    {
      mM = m;
    }
    else
    {
      mM = m.mid( 0, pointCount );
    }
  }
}

QgsLineString::QgsLineString( const QVector<QgsPointXY> &points )
{
  mWkbType = QgsWkbTypes::LineString;
  mX.reserve( points.size() );
  mY.reserve( points.size() );
  for ( const QgsPointXY &p : points )
  {
    mX << p.x();
    mY << p.y();
  }
}

bool QgsLineString::operator==( const QgsCurve &other ) const
{
  const QgsLineString *otherLine = qgsgeometry_cast< const QgsLineString * >( &other );
  if ( !otherLine )
    return false;

  if ( mWkbType != otherLine->mWkbType )
    return false;

  if ( mX.count() != otherLine->mX.count() )
    return false;

  for ( int i = 0; i < mX.count(); ++i )
  {
    if ( !qgsDoubleNear( mX.at( i ), otherLine->mX.at( i ) )
         || !qgsDoubleNear( mY.at( i ), otherLine->mY.at( i ) ) )
      return false;

    if ( is3D() && !qgsDoubleNear( mZ.at( i ), otherLine->mZ.at( i ) ) )
      return false;

    if ( isMeasure() && !qgsDoubleNear( mM.at( i ), otherLine->mM.at( i ) ) )
      return false;
  }

  return true;
}

bool QgsLineString::operator!=( const QgsCurve &other ) const
{
  return !operator==( other );
}

QgsLineString *QgsLineString::clone() const
{
  return new QgsLineString( *this );
}

void QgsLineString::clear()
{
  mX.clear();
  mY.clear();
  mZ.clear();
  mM.clear();
  mWkbType = QgsWkbTypes::LineString;
  clearCache();
}

bool QgsLineString::isEmpty() const
{
  return mX.isEmpty();
}

QgsLineString *QgsLineString::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  // prepare result
  std::unique_ptr<QgsLineString> result { createEmptyWithSameType() };

  bool res = snapToGridPrivate( hSpacing, vSpacing, dSpacing, mSpacing, mX, mY, mZ, mM,
                                result->mX, result->mY, result->mZ, result->mM );
  if ( res )
    return result.release();
  else
    return nullptr;
}

bool QgsLineString::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::LineString )
  {
    return false;
  }
  mWkbType = type;
  importVerticesFromWkb( wkbPtr );
  return true;
}

void QgsLineString::fromWkbPoints( QgsWkbTypes::Type type, const QgsConstWkbPtr &wkb )
{
  mWkbType = type;
  importVerticesFromWkb( wkb );
}

QgsRectangle QgsLineString::calculateBoundingBox() const
{
  double xmin = std::numeric_limits<double>::max();
  double ymin = std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  for ( double x : mX )
  {
    if ( x < xmin )
      xmin = x;
    if ( x > xmax )
      xmax = x;
  }
  for ( double y : mY )
  {
    if ( y < ymin )
      ymin = y;
    if ( y > ymax )
      ymax = y;
  }
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::LineString )
    return false;
  mWkbType = parts.first;

  setPoints( QgsGeometryUtils::pointsFromWKT( parts.second, is3D(), isMeasure() ) );
  return true;
}

QByteArray QgsLineString::asWkb() const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  binarySize += numPoints() * ( 2 + is3D() + isMeasure() ) * sizeof( double );

  QByteArray wkbArray;
  wkbArray.resize( binarySize );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  QgsPointSequence pts;
  points( pts );
  QgsGeometryUtils::pointsToWKB( wkb, pts, is3D(), isMeasure() );
  return wkbArray;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QString QgsLineString::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + ' ';
  QgsPointSequence pts;
  points( pts );
  wkt += QgsGeometryUtils::pointsToWKT( pts, precision, is3D(), isMeasure() );
  return wkt;
}

QDomElement QgsLineString::asGml2( QDomDocument &doc, int precision, const QString &ns ) const
{
  QgsPointSequence pts;
  points( pts );

  QDomElement elemLineString = doc.createElementNS( ns, QStringLiteral( "LineString" ) );

  if ( isEmpty() )
    return elemLineString;

  elemLineString.appendChild( QgsGeometryUtils::pointsToGML2( pts, doc, precision, ns ) );

  return elemLineString;
}

QDomElement QgsLineString::asGml3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QgsPointSequence pts;
  points( pts );

  QDomElement elemLineString = doc.createElementNS( ns, QStringLiteral( "LineString" ) );

  if ( isEmpty() )
    return elemLineString;

  elemLineString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
  return elemLineString;
}

QString QgsLineString::asJson( int precision ) const
{
  QgsPointSequence pts;
  points( pts );

  return "{\"type\": \"LineString\", \"coordinates\": " + QgsGeometryUtils::pointsToJSON( pts, precision ) + '}';
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

double QgsLineString::length() const
{
  double length = 0;
  int size = mX.size();
  double dx, dy;
  for ( int i = 1; i < size; ++i )
  {
    dx = mX.at( i ) - mX.at( i - 1 );
    dy = mY.at( i ) - mY.at( i - 1 );
    length += std::sqrt( dx * dx + dy * dy );
  }
  return length;
}

QgsPoint QgsLineString::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( 0 );
}

QgsPoint QgsLineString::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( numPoints() - 1 );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsLineString *QgsLineString::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( tolerance );
  Q_UNUSED( toleranceType );
  return static_cast<QgsLineString *>( clone() );
}

int QgsLineString::numPoints() const
{
  return mX.size();
}

int QgsLineString::nCoordinates() const
{
  return mX.size();
}

QgsPoint QgsLineString::pointN( int i ) const
{
  if ( i < 0 || i >= mX.size() )
  {
    return QgsPoint();
  }

  double x = mX.at( i );
  double y = mY.at( i );
  double z = std::numeric_limits<double>::quiet_NaN();
  double m = std::numeric_limits<double>::quiet_NaN();

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

  QgsWkbTypes::Type t = QgsWkbTypes::Point;
  if ( mWkbType == QgsWkbTypes::LineString25D )
  {
    t = QgsWkbTypes::Point25D;
  }
  else if ( hasZ && hasM )
  {
    t = QgsWkbTypes::PointZM;
  }
  else if ( hasZ )
  {
    t = QgsWkbTypes::PointZ;
  }
  else if ( hasM )
  {
    t = QgsWkbTypes::PointM;
  }
  return QgsPoint( t, x, y, z, m );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

double QgsLineString::xAt( int index ) const
{
  if ( index >= 0 && index < mX.size() )
    return mX.at( index );
  else
    return 0.0;
}

double QgsLineString::yAt( int index ) const
{
  if ( index >= 0 && index < mY.size() )
    return mY.at( index );
  else
    return 0.0;
}

double QgsLineString::zAt( int index ) const
{
  if ( index >= 0 && index < mZ.size() )
    return mZ.at( index );
  else
    return std::numeric_limits<double>::quiet_NaN();
}

double QgsLineString::mAt( int index ) const
{
  if ( index >= 0 && index < mM.size() )
    return mM.at( index );
  else
    return std::numeric_limits<double>::quiet_NaN();
}

void QgsLineString::setXAt( int index, double x )
{
  if ( index >= 0 && index < mX.size() )
    mX[ index ] = x;
  clearCache();
}

void QgsLineString::setYAt( int index, double y )
{
  if ( index >= 0 && index < mY.size() )
    mY[ index ] = y;
  clearCache();
}

void QgsLineString::setZAt( int index, double z )
{
  if ( index >= 0 && index < mZ.size() )
    mZ[ index ] = z;
}

void QgsLineString::setMAt( int index, double m )
{
  if ( index >= 0 && index < mM.size() )
    mM[ index ] = m;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::points( QgsPointSequence &pts ) const
{
  pts.clear();
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsLineString::setPoints( const QgsPointSequence &points )
{
  clearCache(); //set bounding box invalid

  if ( points.isEmpty() )
  {
    clear();
    return;
  }

  //get wkb type from first point
  const QgsPoint &firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWkbTypes::LineString );

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
      double z = points.at( i ).z();
      mZ[i] = std::isnan( z ) ? 0 : z;
    }
    if ( hasM )
    {
      double m = points.at( i ).m();
      mM[i] = std::isnan( m ) ? 0 : m;
    }
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::append( const QgsLineString *line )
{
  if ( !line )
  {
    return;
  }

  if ( numPoints() < 1 )
  {
    setZMTypeFromSubGeometry( line, QgsWkbTypes::LineString );
  }

  // do not store duplicit points
  if ( numPoints() > 0 &&
       line->numPoints() > 0 &&
       endPoint() == line->startPoint() )
  {
    mX.pop_back();
    mY.pop_back();

    if ( is3D() )
    {
      mZ.pop_back();
    }
    if ( isMeasure() )
    {
      mM.pop_back();
    }
  }

  mX += line->mX;
  mY += line->mY;

  if ( is3D() )
  {
    if ( line->is3D() )
    {
      mZ += line->mZ;
    }
    else
    {
      // if append line does not have z coordinates, fill with NaN to match number of points in final line
      mZ.insert( mZ.count(), mX.size() - mZ.size(), std::numeric_limits<double>::quiet_NaN() );
    }
  }

  if ( isMeasure() )
  {
    if ( line->isMeasure() )
    {
      mM += line->mM;
    }
    else
    {
      // if append line does not have m values, fill with NaN to match number of points in final line
      mM.insert( mM.count(), mX.size() - mM.size(), std::numeric_limits<double>::quiet_NaN() );
    }
  }

  clearCache(); //set bounding box invalid
}

QgsLineString *QgsLineString::reversed() const
{
  QgsLineString *copy = clone();
  std::reverse( copy->mX.begin(), copy->mX.end() );
  std::reverse( copy->mY.begin(), copy->mY.end() );
  if ( copy->is3D() )
  {
    std::reverse( copy->mZ.begin(), copy->mZ.end() );
  }
  if ( copy->isMeasure() )
  {
    std::reverse( copy->mM.begin(), copy->mM.end() );
  }
  return copy;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::draw( QPainter &p ) const
{
  p.drawPolyline( asQPolygonF() );
}

void QgsLineString::addToPainterPath( QPainterPath &path ) const
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

void QgsLineString::drawAsPolygon( QPainter &p ) const
{
  p.drawPolygon( asQPolygonF() );
}

QgsCompoundCurve *QgsLineString::toCurveType() const
{
  QgsCompoundCurve *compoundCurve = new QgsCompoundCurve();
  compoundCurve->addCurve( clone() );
  return compoundCurve;
}

void QgsLineString::extend( double startDistance, double endDistance )
{
  if ( mX.size() < 2 || mY.size() < 2 )
    return;

  // start of line
  if ( startDistance > 0 )
  {
    double currentLen = std::sqrt( std::pow( mX.at( 0 ) - mX.at( 1 ), 2 ) +
                                   std::pow( mY.at( 0 ) - mY.at( 1 ), 2 ) );
    double newLen = currentLen + startDistance;
    mX[ 0 ] = mX.at( 1 ) + ( mX.at( 0 ) - mX.at( 1 ) ) / currentLen * newLen;
    mY[ 0 ] = mY.at( 1 ) + ( mY.at( 0 ) - mY.at( 1 ) ) / currentLen * newLen;
  }
  // end of line
  if ( endDistance > 0 )
  {
    int last = mX.size() - 1;
    double currentLen = std::sqrt( std::pow( mX.at( last ) - mX.at( last - 1 ), 2 ) +
                                   std::pow( mY.at( last ) - mY.at( last - 1 ), 2 ) );
    double newLen = currentLen + endDistance;
    mX[ last ] = mX.at( last - 1 ) + ( mX.at( last ) - mX.at( last - 1 ) ) / currentLen * newLen;
    mY[ last ] = mY.at( last - 1 ) + ( mY.at( last ) - mY.at( last - 1 ) ) / currentLen * newLen;
  }
}

QgsLineString *QgsLineString::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsLineString >();
  result->mWkbType = mWkbType;
  return result.release();
}

QString QgsLineString::geometryType() const
{
  return QStringLiteral( "LineString" );
}

int QgsLineString::dimension() const
{
  return 1;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  double *zArray = mZ.data();

  bool hasZ = is3D();
  int nPoints = numPoints();
  bool useDummyZ = !hasZ || !transformZ;
  if ( useDummyZ )
  {
    zArray = new double[nPoints];
    for ( int i = 0; i < nPoints; ++i )
    {
      zArray[i] = 0;
    }
  }
  ct.transformCoords( nPoints, mX.data(), mY.data(), zArray, d );
  if ( useDummyZ )
  {
    delete[] zArray;
  }
  clearCache();
}

void QgsLineString::transform( const QTransform &t )
{
  int nPoints = numPoints();
  for ( int i = 0; i < nPoints; ++i )
  {
    qreal x, y;
    t.map( mX.at( i ), mY.at( i ), &x, &y );
    mX[i] = x;
    mY[i] = y;
  }
  clearCache();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  if ( position.vertex < 0 || position.vertex > mX.size() )
  {
    return false;
  }

  if ( mWkbType == QgsWkbTypes::Unknown || mX.isEmpty() )
  {
    setZMTypeFromSubGeometry( &vertex, QgsWkbTypes::LineString );
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
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsLineString::moveVertex( QgsVertexId position, const QgsPoint &newPos )
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
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsLineString::deleteVertex( QgsVertexId position )
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

  if ( numPoints() == 1 )
  {
    clear();
  }

  clearCache(); //set bounding box invalid
  return true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::addVertex( const QgsPoint &pt )
{
  if ( mWkbType == QgsWkbTypes::Unknown || mX.isEmpty() )
  {
    setZMTypeFromSubGeometry( &pt, QgsWkbTypes::LineString );
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
  clearCache(); //set bounding box invalid
}

double QgsLineString::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, bool *leftOf, double epsilon ) const
{
  double sqrDist = std::numeric_limits<double>::max();
  double testDist = 0;
  double segmentPtX, segmentPtY;

  int size = mX.size();
  if ( size == 0 || size == 1 )
  {
    vertexAfter = QgsVertexId( 0, 0, 0 );
    return -1;
  }
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
        *leftOf = ( QgsGeometryUtils::leftOfLine( pt.x(), pt.y(), prevX, prevY, currentX, currentY ) < 0 );
      }
      vertexAfter.part = 0;
      vertexAfter.ring = 0;
      vertexAfter.vertex = i;
    }
  }
  return sqrDist;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::pointAt( int node, QgsPoint &point, QgsVertexId::VertexType &type ) const
{
  if ( node < 0 || node >= numPoints() )
  {
    return false;
  }
  point = pointN( node );
  type = QgsVertexId::SegmentVertex;
  return true;
}

QgsPoint QgsLineString::centroid() const
{
  if ( mX.isEmpty() )
    return QgsPoint();

  int numPoints = mX.count();
  if ( numPoints == 1 )
    return QgsPoint( mX.at( 0 ), mY.at( 0 ) );

  double totalLineLength = 0.0;
  double prevX = mX.at( 0 );
  double prevY = mY.at( 0 );
  double sumX = 0.0;
  double sumY = 0.0;

  for ( int i = 1; i < numPoints ; ++i )
  {
    double currentX = mX.at( i );
    double currentY = mY.at( i );
    double segmentLength = std::sqrt( std::pow( currentX - prevX, 2.0 ) +
                                      std::pow( currentY - prevY, 2.0 ) );
    if ( qgsDoubleNear( segmentLength, 0.0 ) )
      continue;

    totalLineLength += segmentLength;
    sumX += segmentLength * 0.5 * ( currentX + prevX );
    sumY += segmentLength * 0.5 * ( currentY + prevY );
    prevX = currentX;
    prevY = currentY;
  }

  if ( qgsDoubleNear( totalLineLength, 0.0 ) )
    return QgsPoint( mX.at( 0 ), mY.at( 0 ) );
  else
    return QgsPoint( sumX / totalLineLength, sumY / totalLineLength );

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::sumUpArea( double &sum ) const
{
  int maxIndex = numPoints() - 1;

  for ( int i = 0; i < maxIndex; ++i )
  {
    sum += 0.5 * ( mX.at( i ) * mY.at( i + 1 ) - mY.at( i ) * mX.at( i + 1 ) );
  }
}

void QgsLineString::importVerticesFromWkb( const QgsConstWkbPtr &wkb )
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
  clearCache(); //set bounding box invalid
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::close()
{
  if ( numPoints() < 1 || isClosed() )
  {
    return;
  }
  addVertex( startPoint() );
}

double QgsLineString::vertexAngle( QgsVertexId vertex ) const
{
  if ( mX.count() < 2 )
  {
    //undefined
    return 0.0;
  }

  if ( vertex.vertex == 0 || vertex.vertex >= ( numPoints() - 1 ) )
  {
    if ( isClosed() )
    {
      double previousX = mX.at( numPoints() - 2 );
      double previousY = mY.at( numPoints() - 2 );
      double currentX = mX.at( 0 );
      double currentY = mY.at( 0 );
      double afterX = mX.at( 1 );
      double afterY = mY.at( 1 );
      return QgsGeometryUtils::averageAngle( previousX, previousY, currentX, currentY, afterX, afterY );
    }
    else if ( vertex.vertex == 0 )
    {
      return QgsGeometryUtils::lineAngle( mX.at( 0 ), mY.at( 0 ), mX.at( 1 ), mY.at( 1 ) );
    }
    else
    {
      int a = numPoints() - 2;
      int b = numPoints() - 1;
      return QgsGeometryUtils::lineAngle( mX.at( a ), mY.at( a ), mX.at( b ), mY.at( b ) );
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

double QgsLineString::segmentLength( QgsVertexId startVertex ) const
{
  if ( startVertex.vertex < 0 || startVertex.vertex >= mX.count() - 1 )
    return 0.0;

  double dx = mX.at( startVertex.vertex + 1 ) - mX.at( startVertex.vertex );
  double dy = mY.at( startVertex.vertex + 1 ) - mY.at( startVertex.vertex );
  return std::sqrt( dx * dx + dy * dy );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  clearCache();
  if ( mWkbType == QgsWkbTypes::Unknown )
  {
    mWkbType = QgsWkbTypes::LineStringZ;
    return true;
  }

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  mZ.clear();
  int nPoints = numPoints();
  mZ.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mZ << zValue;
  }
  return true;
}

bool QgsLineString::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  clearCache();
  if ( mWkbType == QgsWkbTypes::Unknown )
  {
    mWkbType = QgsWkbTypes::LineStringM;
    return true;
  }

  if ( mWkbType == QgsWkbTypes::LineString25D )
  {
    mWkbType = QgsWkbTypes::LineStringZM;
  }
  else
  {
    mWkbType = QgsWkbTypes::addM( mWkbType );
  }

  mM.clear();
  int nPoints = numPoints();
  mM.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mM << mValue;
  }
  return true;
}

bool QgsLineString::dropZValue()
{
  if ( !is3D() )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  mZ.clear();
  return true;
}

bool QgsLineString::dropMValue()
{
  if ( !isMeasure() )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::dropM( mWkbType );
  mM.clear();
  return true;
}

bool QgsLineString::convertTo( QgsWkbTypes::Type type )
{
  if ( type == mWkbType )
    return true;

  clearCache();
  if ( type == QgsWkbTypes::LineString25D )
  {
    //special handling required for conversion to LineString25D
    dropMValue();
    addZValue( std::numeric_limits<double>::quiet_NaN() );
    mWkbType = QgsWkbTypes::LineString25D;
    return true;
  }
  else
  {
    return QgsCurve::convertTo( type );
  }
}
