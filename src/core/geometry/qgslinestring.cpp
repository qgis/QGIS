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

#include <QPainter>
#include <limits>
#include <QDomDocument>
#include <QtCore/qmath.h>


/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsLineString::QgsLineString(): QgsCurve()
{
  mWkbType = QgsWkbTypes::LineString;
}

bool QgsLineString::operator==( const QgsCurve& other ) const
{
  const QgsLineString* otherLine = dynamic_cast< const QgsLineString* >( &other );
  if ( !otherLine )
    return false;

  if ( mWkbType != otherLine->mWkbType )
    return false;

  if ( mPoints.count() != otherLine->mPoints.count() )
    return false;

  QgsPointSequence::const_iterator thisIt = mPoints.constBegin();
  QgsPointSequence::const_iterator otherIt = otherLine->mPoints.constBegin();

  for ( ; thisIt != mPoints.constEnd(); ++thisIt, ++otherIt )
  {
    if ( !qgsDoubleNear( thisIt->x(), otherIt->x() )
         || !qgsDoubleNear( thisIt->y(), otherIt->y() ) )
      return false;

    if ( is3D() && !qgsDoubleNear( thisIt->z(), otherIt->z() ) )
      return false;

    if ( isMeasure() && !qgsDoubleNear( thisIt->m(), otherIt->m() ) )
      return false;
  }

  return true;
}

bool QgsLineString::operator!=( const QgsCurve& other ) const
{
  return !operator==( other );
}

QgsLineString *QgsLineString::clone() const
{
  return new QgsLineString( *this );
}

void QgsLineString::clear()
{
  mPoints.clear();
  mWkbType = QgsWkbTypes::LineString;
  clearCache();
}

bool QgsLineString::isEmpty() const
{
  return mX.isEmpty();
}

bool QgsLineString::fromWkb( QgsConstWkbPtr& wkbPtr )
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

void QgsLineString::fromWkbPoints( QgsWkbTypes::Type type, const QgsConstWkbPtr& wkb )
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

  Q_FOREACH ( const QgsPointV2& point, mPoints )
  {
    xmin = qMin( xmin, point.x() );
    xmax = qMax( xmax, point.x() );
    ymin = qMin( ymin, point.y() );
    ymax = qMax( ymax, point.y() );
  }
  return QgsRectangle( xmin, ymin, xmax, ymax );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::fromWkt( const QString& wkt )
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

QDomElement QgsLineString::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QgsPointSequence pts;
  points( pts );

  QDomElement elemLineString = doc.createElementNS( ns, QStringLiteral( "LineString" ) );
  elemLineString.appendChild( QgsGeometryUtils::pointsToGML2( pts, doc, precision, ns ) );

  return elemLineString;
}

QDomElement QgsLineString::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QgsPointSequence pts;
  points( pts );

  QDomElement elemLineString = doc.createElementNS( ns, QStringLiteral( "LineString" ) );
  elemLineString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D() ) );
  return elemLineString;
}

QString QgsLineString::asJSON( int precision ) const
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
  int size = mPoints.size();
  double dx, dy;
  for ( int i = 1; i < size; ++i )
  {
    dx = mPoints.at( i ).x() - mPoints.at( i - 1 ).x();
    dy = mPoints.at( i ).y() - mPoints.at( i - 1 ).y();
    length += sqrt( dx * dx + dy * dy );
  }
  return length;
}

QgsPointV2 QgsLineString::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( 0 );
}

QgsPointV2 QgsLineString::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPointV2();
  }
  return pointN( numPoints() - 1 );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsLineString* QgsLineString::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( tolerance );
  Q_UNUSED( toleranceType );
  return static_cast<QgsLineString*>( clone() );
}

int QgsLineString::numPoints() const
{
  return mPoints.size();
}

QgsPointV2 QgsLineString::pointN( int i ) const
{
  if ( i < 0 || i >= mPoints.size() )
  {
    return QgsPointV2();
  }

  return mPoints.at( i );
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

double QgsLineString::xAt( int index ) const
{
  if ( index >= 0 && index < mPoints.size() )
    return mPoints.at( index ).x();
  else
    return 0.0;
}

double QgsLineString::yAt( int index ) const
{
  if ( index >= 0 && index < mPoints.size() )
    return mPoints.at( index ).y();
  else
    return 0.0;
}

double QgsLineString::zAt( int index ) const
{
  if ( index >= 0 && index < mPoints.size() )
    return mPoints.at( index ).z();
  else
    return 0.0;
}

double QgsLineString::mAt( int index ) const
{
  if ( index >= 0 && index < mPoints.size() )
    return mPoints.at( index ).m();
  else
    return 0.0;
}

void QgsLineString::setXAt( int index, double x )
{
  if ( index >= 0 && index < mPoints.size() )
    mPoints[ index ].setX( x );
  clearCache();
}

void QgsLineString::setYAt( int index, double y )
{
  if ( index >= 0 && index < mPoints.size() )
    mPoints[ index ].setY( y );
  clearCache();
}

void QgsLineString::setZAt( int index, double z )
{
  if ( index >= 0 && index < mPoints.size() )
    mPoints[ index ].setZ( z );
  clearCache();
}

void QgsLineString::setMAt( int index, double m )
{
  if ( index >= 0 && index < mPoints.size() )
    mPoints[ index ].setM( m );
  clearCache();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::points( QgsPointSequence &pts ) const
{
  pts = mPoints;
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
  const QgsPointV2& firstPt = points.at( 0 );
  setZMTypeFromSubGeometry( &firstPt, QgsWkbTypes::LineString );
  QgsWkbTypes::Type pointType = firstPt.wkbType();

  mPoints.clear();
  mPoints.reserve( points.size() );

  QgsPointSequence::const_iterator pointIt = points.constBegin();

  for ( int i = 0; pointIt != points.constEnd(); ++i, ++pointIt )
  {
    // we don't trust that the wkb type of passed points is correct!
    mPoints << QgsPointV2( pointType, pointIt->x(), pointIt->y(),
                           pointIt->z(), pointIt->m() );
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::append( const QgsLineString* line )
{
  if ( !line )
  {
    return;
  }

  QgsWkbTypes::Type pointType = QgsWkbTypes::Point;
  if ( mPoints.isEmpty() )
  {
    setZMTypeFromSubGeometry( line, QgsWkbTypes::LineString );
    pointType = line->pointN( 0 ).wkbType();
  }
  else
  {
    pointType = mPoints.at( 0 ).wkbType();
  }

  // do not store duplicate points
  if ( numPoints() > 0 &&
       line->numPoints() > 0 &&
       endPoint() == line->startPoint() )
  {
    mPoints.pop_back();
  }

  mPoints.reserve( mPoints.size() + line->mPoints.size() );

  QgsPointSequence::const_iterator pointIt = line->mPoints.constBegin();
  for ( ; pointIt != line->mPoints.constEnd(); ++pointIt )
  {
    // we don't trust that the wkb type of passed points is correct!
    mPoints << QgsPointV2( pointType, pointIt->x(), pointIt->y(),
                           pointIt->z(), pointIt->m() );
  }

  clearCache(); //set bounding box invalid
}

QgsLineString* QgsLineString::reversed() const
{
  QgsLineString* copy = clone();
  std::reverse( copy->mPoints.begin(), copy->mPoints.end() );
  return copy;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::draw( QPainter& p ) const
{
  p.drawPolyline( asQPolygonF() );
}

void QgsLineString::addToPainterPath( QPainterPath& path ) const
{
  int nPoints = numPoints();
  if ( nPoints < 1 )
  {
    return;
  }

  if ( path.isEmpty() || path.currentPosition() != QPointF( mPoints.at( 0 ).x(), mPoints.at( 0 ).y() ) )
  {
    path.moveTo( mPoints.at( 0 ).x(), mPoints.at( 0 ).y() );
  }

  for ( int i = 1; i < nPoints; ++i )
  {
    path.lineTo( mPoints.at( i ).x(), mPoints.at( i ).y() );
  }
}

void QgsLineString::drawAsPolygon( QPainter& p ) const
{
  p.drawPolygon( asQPolygonF() );
}

QgsAbstractGeometry* QgsLineString::toCurveType() const
{
  QgsCompoundCurve* compoundCurve = new QgsCompoundCurve();
  compoundCurve->addCurve( clone() );
  return compoundCurve;
}

void QgsLineString::extend( double startDistance, double endDistance )
{
  if ( mPoints.size() < 2 )
    return;

  // start of line
  if ( startDistance > 0 )
  {
    double currentLen = sqrt( qPow( mPoints.at( 0 ).x() - mPoints.at( 1 ).x(), 2 ) +
                              qPow( mPoints.at( 0 ).y() - mPoints.at( 1 ).y(), 2 ) );
    double newLen = currentLen + startDistance;
    mPoints[ 0 ].setX( mPoints.at( 1 ).x() + ( mPoints.at( 0 ).x() - mPoints.at( 1 ).x() ) / currentLen * newLen );
    mPoints[ 0 ].setY( mPoints.at( 1 ).y() + ( mPoints.at( 0 ).y() - mPoints.at( 1 ).y() ) / currentLen * newLen );
  }
  // end of line
  if ( endDistance > 0 )
  {
    int last = mPoints.size() - 1;
    double currentLen = sqrt( qPow( mPoints.at( last ).x() - mPoints.at( last - 1 ).x(), 2 ) +
                              qPow( mPoints.at( last ).y() - mPoints.at( last - 1 ).y(), 2 ) );
    double newLen = currentLen + endDistance;
    mPoints[ last ].setX( mPoints.at( last - 1 ).x() + ( mPoints.at( last ).x() - mPoints.at( last - 1 ).x() ) / currentLen * newLen );
    mPoints[ last ].setY( mPoints.at( last - 1 ).y() + ( mPoints.at( last ).y() - mPoints.at( last - 1 ).y() ) / currentLen * newLen );
  }
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    if ( transformZ )
      ct.transformInPlace( pointIt->rx(), pointIt->ry(), pointIt->rz(), d );
    else
    {
      double z = 0;
      ct.transformInPlace( pointIt->rx(), pointIt->ry(), z, d );
    }
  }
  clearCache();
}

void QgsLineString::transform( const QTransform& t )
{
  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    qreal x, y;
    t.map( pointIt->x(), pointIt->y(), &x, &y );
    pointIt->setX( x );
    pointIt->setY( y );
  }
  clearCache();
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsLineString::insertVertex( QgsVertexId position, const QgsPointV2& vertex )
{
  if ( position.vertex < 0 || position.vertex > mPoints.size() )
  {
    return false;
  }

  if ( mWkbType == QgsWkbTypes::Unknown || mPoints.isEmpty() )
  {
    setZMTypeFromSubGeometry( &vertex, QgsWkbTypes::LineString );
    mPoints.insert( position.vertex, vertex );
  }
  else
  {
    // don't trust passed point wkb type
    mPoints.insert( position.vertex,
                    QgsPointV2( mPoints.at( 0 ).wkbType(),
                                vertex.x(), vertex.y(),
                                vertex.z(), vertex.m() ) );
  }
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsLineString::moveVertex( QgsVertexId position, const QgsPointV2& newPos )
{
  if ( position.vertex < 0 || position.vertex >= mPoints.size() )
  {
    return false;
  }
  QgsPointV2& p = mPoints[ position.vertex ];
  p.setX( newPos.x() );
  p.setY( newPos.y() );

  if ( is3D() && newPos.is3D() )
  {
    p.setZ( newPos.z() );
  }
  if ( isMeasure() && newPos.isMeasure() )
  {
    p.setM( newPos.m() );
  }
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsLineString::deleteVertex( QgsVertexId position )
{
  if ( position.vertex >= mPoints.size() || position.vertex < 0 )
  {
    return false;
  }

  mPoints.removeAt( position.vertex );

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

void QgsLineString::addVertex( const QgsPointV2& pt )
{
  if ( mWkbType == QgsWkbTypes::Unknown || mPoints.isEmpty() )
  {
    setZMTypeFromSubGeometry( &pt, QgsWkbTypes::LineString );
    mPoints.append( pt );
  }
  else
  {
    // don't trust passed point wkb type
    mPoints.append( QgsPointV2( mPoints.at( 0 ).wkbType(),
                                pt.x(), pt.y(),
                                pt.z(), pt.m() ) );
  }

  clearCache(); //set bounding box invalid
}

double QgsLineString::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  double sqrDist = std::numeric_limits<double>::max();
  double testDist = 0;
  double segmentPtX, segmentPtY;

  int size = mPoints.size();
  if ( size == 0 || size == 1 )
  {
    vertexAfter = QgsVertexId( 0, 0, 0 );
    return -1;
  }
  for ( int i = 1; i < size; ++i )
  {
    double prevX = mPoints.at( i - 1 ).x();
    double prevY = mPoints.at( i - 1 ).y();
    double currentX = mPoints.at( i ).x();
    double currentY = mPoints.at( i ).y();
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

bool QgsLineString::pointAt( int node, QgsPointV2& point, QgsVertexId::VertexType& type ) const
{
  if ( node < 0 || node >= numPoints() )
  {
    return false;
  }
  point = pointN( node );
  type = QgsVertexId::SegmentVertex;
  return true;
}

QgsPointV2 QgsLineString::centroid() const
{
  if ( mPoints.isEmpty() )
    return QgsPointV2();

  int numPoints = mPoints.count();
  if ( numPoints == 1 )
    return QgsPointV2( mPoints.at( 0 ).x(), mPoints.at( 0 ).y() );

  QgsPointSequence::const_iterator pointIt = mPoints.constBegin();

  double totalLineLength = 0.0;
  double prevX = pointIt->x();
  double prevY = pointIt->y();
  ++pointIt;
  double sumX = 0.0;
  double sumY = 0.0;

  for ( ; pointIt != mPoints.constEnd(); ++pointIt )
  {
    double currentX = pointIt->x();
    double currentY = pointIt->y();
    double segmentLength = sqrt( qPow( currentX - prevX, 2.0 ) +
                                 qPow( currentY - prevY, 2.0 ) );
    if ( qgsDoubleNear( segmentLength, 0.0 ) )
      continue;

    totalLineLength += segmentLength;
    sumX += segmentLength * 0.5 * ( currentX + prevX );
    sumY += segmentLength * 0.5 * ( currentY + prevY );
    prevX = currentX;
    prevY = currentY;
  }

  if ( qgsDoubleNear( totalLineLength, 0.0 ) )
    return QgsPointV2( mPoints.at( 0 ).x(), mPoints.at( 0 ).y() );
  else
    return QgsPointV2( sumX / totalLineLength, sumY / totalLineLength );

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsLineString::sumUpArea( double& sum ) const
{
  int maxIndex = numPoints() - 1;

  for ( int i = 0; i < maxIndex; ++i )
  {
    sum += 0.5 * ( mPoints.at( i ).x() * mPoints.at( i + 1 ).y() - mPoints.at( i ).y() * mPoints.at( i + 1 ).x() );
  }
}

void QgsLineString::importVerticesFromWkb( const QgsConstWkbPtr& wkb )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  QgsWkbTypes::Type pointType = QgsWkbTypes::Point;
  if ( hasZ )
    pointType = QgsWkbTypes::addZ( pointType );
  if ( hasM )
    pointType = QgsWkbTypes::addM( pointType );
  int nVertices = 0;
  wkb >> nVertices;
  mPoints.clear();
  mPoints.reserve( nVertices );
  for ( int i = 0; i < nVertices; ++i )
  {
    QgsPointV2 point( pointType, 0.0, 0.0, 0.0, 0.0 );
    wkb >> point.rx();
    wkb >> point.ry();
    if ( hasZ )
    {
      wkb >> point.rz();
    }
    if ( hasM )
    {
      wkb >> point.rm();
    }
    mPoints << point;
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
  if ( mPoints.count() < 2 )
  {
    //undefined
    return 0.0;
  }

  if ( vertex.vertex == 0 || vertex.vertex >= ( numPoints() - 1 ) )
  {
    if ( isClosed() )
    {
      double previousX = mPoints.at( numPoints() - 2 ).x();
      double previousY = mPoints.at( numPoints() - 2 ).y();
      double currentX = mPoints.at( 0 ).x();
      double currentY = mPoints.at( 0 ).y();
      double afterX = mPoints.at( 1 ).x();
      double afterY = mPoints.at( 1 ).y();
      return QgsGeometryUtils::averageAngle( previousX, previousY, currentX, currentY, afterX, afterY );
    }
    else if ( vertex.vertex == 0 )
    {
      return QgsGeometryUtils::lineAngle( mPoints.at( 0 ).x(), mPoints.at( 0 ).y(),
                                          mPoints.at( 1 ).x(), mPoints.at( 1 ).y() );
    }
    else
    {
      int a = numPoints() - 2;
      int b = numPoints() - 1;
      return QgsGeometryUtils::lineAngle( mPoints.at( a ).x(), mPoints.at( a ).y(),
                                          mPoints.at( b ).x(), mPoints.at( b ).y() );
    }
  }
  else
  {
    double previousX = mPoints.at( vertex.vertex - 1 ).x();
    double previousY = mPoints.at( vertex.vertex - 1 ).y();
    double currentX = mPoints.at( vertex.vertex ).x();
    double currentY = mPoints.at( vertex.vertex ).y();
    double afterX = mPoints.at( vertex.vertex + 1 ).x();
    double afterY = mPoints.at( vertex.vertex + 1 ).y();
    return QgsGeometryUtils::averageAngle( previousX, previousY, currentX, currentY, afterX, afterY );
  }
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

  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    pointIt->addZValue( zValue );
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

  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    pointIt->addMValue( mValue );
  }
  return true;
}

bool QgsLineString::dropZValue()
{
  if ( !is3D() )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    pointIt->dropZValue();
  }
  return true;
}

bool QgsLineString::dropMValue()
{
  if ( !isMeasure() )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::dropM( mWkbType );
  QgsPointSequence::iterator pointIt = mPoints.begin();
  for ( ; pointIt != mPoints.end(); ++pointIt )
  {
    pointIt->dropMValue();
  }
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
    mWkbType = QgsWkbTypes::LineString25D;
    QgsPointSequence::iterator pointIt = mPoints.begin();
    for ( ; pointIt != mPoints.end(); ++pointIt )
    {
      pointIt->dropMValue();
      pointIt->convertTo( QgsWkbTypes::Point25D );
    }
    return true;
  }
  else
  {
    return QgsCurve::convertTo( type );
  }
}
