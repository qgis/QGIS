/***************************************************************************
                         qgscircularstring.cpp
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

#include "qgscircularstring.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgswkbptr.h"
#include "qgslogger.h"
#include "qgsgeometrytransformer.h"
#include "qgsfeedback.h"

#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <memory>
#include <nlohmann/json.hpp>

QgsCircularString::QgsCircularString()
{
  mWkbType = QgsWkbTypes::CircularString;
}

QgsCircularString::QgsCircularString( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3 )
{
  //get wkb type from first point
  bool hasZ = p1.is3D();
  bool hasM = p1.isMeasure();
  mWkbType = QgsWkbTypes::CircularString;

  mX.resize( 3 );
  mX[ 0 ] = p1.x();
  mX[ 1 ] = p2.x();
  mX[ 2 ] = p3.x();
  mY.resize( 3 );
  mY[ 0 ] = p1.y();
  mY[ 1 ] = p2.y();
  mY[ 2 ] = p3.y();
  if ( hasZ )
  {
    mWkbType = QgsWkbTypes::addZ( mWkbType );
    mZ.resize( 3 );
    mZ[ 0 ] = p1.z();
    mZ[ 1 ] = p2.z();
    mZ[ 2 ] = p3.z();
  }
  if ( hasM )
  {
    mWkbType = QgsWkbTypes::addM( mWkbType );
    mM.resize( 3 );
    mM[ 0 ] = p1.m();
    mM[ 1 ] = p2.m();
    mM[ 2 ] = p3.m();
  }
}

QgsCircularString::QgsCircularString( const QVector<double> &x, const QVector<double> &y, const QVector<double> &z, const QVector<double> &m )
{
  mWkbType = QgsWkbTypes::CircularString;
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
    mWkbType = QgsWkbTypes::CircularStringZ;
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

QgsCircularString QgsCircularString::fromTwoPointsAndCenter( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &center, const bool useShortestArc )
{
  const QgsPoint midPoint = QgsGeometryUtils::segmentMidPointFromCenter( p1, p2, center, useShortestArc );
  return QgsCircularString( p1, midPoint, p2 );
}

bool QgsCircularString::equals( const QgsCurve &other ) const
{
  const QgsCircularString *otherLine = dynamic_cast< const QgsCircularString * >( &other );
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

QgsCircularString *QgsCircularString::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsCircularString >();
  result->mWkbType = mWkbType;
  return result.release();
}

int QgsCircularString::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsCircularString *otherLine = qgsgeometry_cast<const QgsCircularString *>( other );
  if ( !otherLine )
    return -1;

  const int size = mX.size();
  const int otherSize = otherLine->mX.size();
  if ( size > otherSize )
  {
    return 1;
  }
  else if ( size < otherSize )
  {
    return -1;
  }

  if ( is3D() && !otherLine->is3D() )
    return 1;
  else if ( !is3D() && otherLine->is3D() )
    return -1;
  const bool considerZ = is3D();

  if ( isMeasure() && !otherLine->isMeasure() )
    return 1;
  else if ( !isMeasure() && otherLine->isMeasure() )
    return -1;
  const bool considerM = isMeasure();

  for ( int i = 0; i < size; i++ )
  {
    const double x = mX[i];
    const double otherX = otherLine->mX[i];
    if ( x < otherX )
    {
      return -1;
    }
    else if ( x > otherX )
    {
      return 1;
    }

    const double y = mY[i];
    const double otherY = otherLine->mY[i];
    if ( y < otherY )
    {
      return -1;
    }
    else if ( y > otherY )
    {
      return 1;
    }

    if ( considerZ )
    {
      const double z = mZ[i];
      const double otherZ = otherLine->mZ[i];

      if ( z < otherZ )
      {
        return -1;
      }
      else if ( z > otherZ )
      {
        return 1;
      }
    }

    if ( considerM )
    {
      const double m = mM[i];
      const double otherM = otherLine->mM[i];

      if ( m < otherM )
      {
        return -1;
      }
      else if ( m > otherM )
      {
        return 1;
      }
    }
  }
  return 0;
}

QString QgsCircularString::geometryType() const
{
  return QStringLiteral( "CircularString" );
}

int QgsCircularString::dimension() const
{
  return 1;
}

QgsCircularString *QgsCircularString::clone() const
{
  return new QgsCircularString( *this );
}

void QgsCircularString::clear()
{
  mWkbType = QgsWkbTypes::CircularString;
  mX.clear();
  mY.clear();
  mZ.clear();
  mM.clear();
  clearCache();
}

QgsRectangle QgsCircularString::calculateBoundingBox() const
{
  QgsRectangle bbox;
  int nPoints = numPoints();
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    if ( i == 0 )
    {
      bbox = segmentBoundingBox( QgsPoint( mX[i], mY[i] ), QgsPoint( mX[i + 1], mY[i + 1] ), QgsPoint( mX[i + 2], mY[i + 2] ) );
    }
    else
    {
      QgsRectangle segmentBox = segmentBoundingBox( QgsPoint( mX[i], mY[i] ), QgsPoint( mX[i + 1], mY[i + 1] ), QgsPoint( mX[i + 2], mY[i + 2] ) );
      bbox.combineExtentWith( segmentBox );
    }
  }

  if ( nPoints > 0 && nPoints % 2 == 0 )
  {
    if ( nPoints == 2 )
    {
      bbox.combineExtentWith( mX[ 0 ], mY[ 0 ] );
    }
    bbox.combineExtentWith( mX[ nPoints - 1 ], mY[ nPoints - 1 ] );
  }
  return bbox;
}

void QgsCircularString::scroll( int index )
{
  const int size = mX.size();
  if ( index < 1 || index >= size - 1 )
    return;

  const bool useZ = is3D();
  const bool useM = isMeasure();

  QVector<double> newX( size );
  QVector<double> newY( size );
  QVector<double> newZ( useZ ? size : 0 );
  QVector<double> newM( useM ? size : 0 );
  auto it = std::copy( mX.constBegin() + index, mX.constEnd() - 1, newX.begin() );
  it = std::copy( mX.constBegin(), mX.constBegin() + index, it );
  *it = *newX.constBegin();
  mX = std::move( newX );

  it = std::copy( mY.constBegin() + index, mY.constEnd() - 1, newY.begin() );
  it = std::copy( mY.constBegin(), mY.constBegin() + index, it );
  *it = *newY.constBegin();
  mY = std::move( newY );
  if ( useZ )
  {
    it = std::copy( mZ.constBegin() + index, mZ.constEnd() - 1, newZ.begin() );
    it = std::copy( mZ.constBegin(), mZ.constBegin() + index, it );
    *it = *newZ.constBegin();
    mZ = std::move( newZ );
  }
  if ( useM )
  {
    it = std::copy( mM.constBegin() + index, mM.constEnd() - 1, newM.begin() );
    it = std::copy( mM.constBegin(), mM.constBegin() + index, it );
    *it = *newM.constBegin();
    mM = std::move( newM );
  }
}

QgsRectangle QgsCircularString::segmentBoundingBox( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 )
{
  double centerX, centerY, radius;
  QgsGeometryUtils::circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );

  double p1Angle = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double p2Angle = QgsGeometryUtils::ccwAngle( pt2.y() - centerY, pt2.x() - centerX );
  double p3Angle = QgsGeometryUtils::ccwAngle( pt3.y() - centerY, pt3.x() - centerX );

  //start point, end point and compass points in between can be on bounding box
  QgsRectangle bbox( pt1.x(), pt1.y(), pt1.x(), pt1.y() );
  bbox.combineExtentWith( pt3.x(), pt3.y() );

  QgsPointSequence compassPoints = compassPointsOnSegment( p1Angle, p2Angle, p3Angle, centerX, centerY, radius );
  QgsPointSequence::const_iterator cpIt = compassPoints.constBegin();
  for ( ; cpIt != compassPoints.constEnd(); ++cpIt )
  {
    bbox.combineExtentWith( cpIt->x(), cpIt->y() );
  }
  return bbox;
}

QgsPointSequence QgsCircularString::compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius )
{
  QgsPointSequence pointList;

  QgsPoint nPoint( centerX, centerY + radius );
  QgsPoint ePoint( centerX + radius, centerY );
  QgsPoint sPoint( centerX, centerY - radius );
  QgsPoint wPoint( centerX - radius, centerY );

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

bool QgsCircularString::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
    return false;

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::CircularString )
  {
    return false;
  }
  clearCache();
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

bool QgsCircularString::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::CircularString )
    return false;
  mWkbType = parts.first;

  parts.second = parts.second.remove( '(' ).remove( ')' );
  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.simplified().remove( ' ' );
  if ( ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 ) ||
       secondWithoutParentheses.isEmpty() )
    return true;

  QgsPointSequence points = QgsGeometryUtils::pointsFromWKT( parts.second, is3D(), isMeasure() );
  if ( points.isEmpty() )
    return false;

  setPoints( points );
  return true;
}

int QgsCircularString::wkbSize( QgsAbstractGeometry::WkbFlags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  binarySize += numPoints() * ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return binarySize;
}

QByteArray QgsCircularString::asWkb( WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsCircularString::wkbSize( flags ) );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  QgsPointSequence pts;
  points( pts );
  QgsGeometryUtils::pointsToWKB( wkb, pts, is3D(), isMeasure(), flags );
  return wkbArray;
}

QString QgsCircularString::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + ' ';

  if ( isEmpty() )
    wkt += QLatin1String( "EMPTY" );
  else
  {
    QgsPointSequence pts;
    points( pts );
    wkt += QgsGeometryUtils::pointsToWKT( pts, precision, is3D(), isMeasure() );
  }
  return wkt;
}

QDomElement QgsCircularString::asGml2( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{
  // GML2 does not support curves
  std::unique_ptr< QgsLineString > line( curveToLine() );
  QDomElement gml = line->asGml2( doc, precision, ns, axisOrder );
  return gml;
}

QDomElement QgsCircularString::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QgsPointSequence pts;
  points( pts );

  QDomElement elemCurve = doc.createElementNS( ns, QStringLiteral( "Curve" ) );

  if ( isEmpty() )
    return elemCurve;

  QDomElement elemSegments = doc.createElementNS( ns, QStringLiteral( "segments" ) );
  QDomElement elemArcString = doc.createElementNS( ns, QStringLiteral( "ArcString" ) );
  elemArcString.appendChild( QgsGeometryUtils::pointsToGML3( pts, doc, precision, ns, is3D(), axisOrder ) );
  elemSegments.appendChild( elemArcString );
  elemCurve.appendChild( elemSegments );
  return elemCurve;
}


json QgsCircularString::asJsonObject( int precision ) const
{
  // GeoJSON does not support curves
  std::unique_ptr< QgsLineString > line( curveToLine() );
  return line->asJsonObject( precision );
}

bool QgsCircularString::isEmpty() const
{
  return mX.isEmpty();
}

bool QgsCircularString::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  if ( !isEmpty() && ( numPoints() < 3 ) )
  {
    error = QObject::tr( "CircularString has less than 3 points and is not empty." );
    return false;
  }
  return QgsCurve::isValid( error, flags );
}

//curve interface
double QgsCircularString::length() const
{
  int nPoints = numPoints();
  double length = 0;
  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    length += QgsGeometryUtils::circleLength( mX[i], mY[i], mX[i + 1], mY[i + 1], mX[i + 2], mY[i + 2] );
  }
  return length;
}

QgsPoint QgsCircularString::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( 0 );
}

QgsPoint QgsCircularString::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( numPoints() - 1 );
}

QgsLineString *QgsCircularString::curveToLine( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QgsLineString *line = new QgsLineString();
  QgsPointSequence points;
  int nPoints = numPoints();

  for ( int i = 0; i < ( nPoints - 2 ) ; i += 2 )
  {
    QgsGeometryUtils::segmentizeArc( pointN( i ), pointN( i + 1 ), pointN( i + 2 ), points, tolerance, toleranceType, is3D(), isMeasure() );
  }

  line->setPoints( points );
  return line;
}

QgsCircularString *QgsCircularString::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  // prepare result
  std::unique_ptr<QgsCircularString> result { createEmptyWithSameType() };

  bool res = snapToGridPrivate( hSpacing, vSpacing, dSpacing, mSpacing, mX, mY, mZ, mM,
                                result->mX, result->mY, result->mZ, result->mM );
  if ( res )
    return result.release();
  else
    return nullptr;
}

bool QgsCircularString::removeDuplicateNodes( double epsilon, bool useZValues )
{
  if ( mX.count() <= 3 )
    return false; // don't create degenerate lines
  bool result = false;
  double prevX = mX.at( 0 );
  double prevY = mY.at( 0 );
  bool hasZ = is3D();
  bool useZ = hasZ && useZValues;
  double prevZ = useZ ? mZ.at( 0 ) : 0;
  int i = 1;
  int remaining = mX.count();
  // we have to consider points in pairs, since a segment can validly have the same start and
  // end if it has a different curve point
  while ( i + 1 < remaining )
  {
    double currentCurveX = mX.at( i );
    double currentCurveY = mY.at( i );
    double currentX = mX.at( i + 1 );
    double currentY = mY.at( i + 1 );
    double currentZ = useZ ? mZ.at( i + 1 ) : 0;
    if ( qgsDoubleNear( currentCurveX, prevX, epsilon ) &&
         qgsDoubleNear( currentCurveY, prevY, epsilon ) &&
         qgsDoubleNear( currentX, prevX, epsilon ) &&
         qgsDoubleNear( currentY, prevY, epsilon ) &&
         ( !useZ || qgsDoubleNear( currentZ, prevZ, epsilon ) ) )
    {
      result = true;
      // remove point
      mX.removeAt( i );
      mX.removeAt( i );
      mY.removeAt( i );
      mY.removeAt( i );
      if ( hasZ )
      {
        mZ.removeAt( i );
        mZ.removeAt( i );
      }
      remaining -= 2;
    }
    else
    {
      prevX = currentX;
      prevY = currentY;
      prevZ = currentZ;
      i += 2;
    }
  }
  return result;
}

int QgsCircularString::numPoints() const
{
  return std::min( mX.size(), mY.size() );
}

int QgsCircularString::indexOf( const QgsPoint &point ) const
{
  const int size = mX.size();
  if ( size == 0 )
    return -1;

  const double *x = mX.constData();
  const double *y = mY.constData();
  const bool useZ = is3D();
  const bool useM = isMeasure();
  const double *z = useZ ? mZ.constData() : nullptr;
  const double *m = useM ? mM.constData() : nullptr;

  for ( int i = 0; i < size; i += 2 )
  {
    if ( qgsDoubleNear( *x, point.x() )
         && qgsDoubleNear( *y, point.y() )
         && ( !useZ || qgsDoubleNear( *z, point.z() ) )
         && ( !useM || qgsDoubleNear( *m, point.m() ) ) )
      return i;

    // we skip over curve points!
    x++;
    x++;
    y++;
    y++;
    if ( useZ )
    {
      z++;
      z++;
    }
    if ( useM )
    {
      m++;
      m++;
    }
  }
  return -1;
}

QgsPoint QgsCircularString::pointN( int i ) const
{
  if ( i < 0 || std::min( mX.size(), mY.size() ) <= i )
  {
    return QgsPoint();
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

  QgsWkbTypes::Type t = QgsWkbTypes::Point;
  if ( is3D() && isMeasure() )
  {
    t = QgsWkbTypes::PointZM;
  }
  else if ( is3D() )
  {
    t = QgsWkbTypes::PointZ;
  }
  else if ( isMeasure() )
  {
    t = QgsWkbTypes::PointM;
  }
  return QgsPoint( t, x, y, z, m );
}

double QgsCircularString::xAt( int index ) const
{
  if ( index >= 0 && index < mX.size() )
    return mX.at( index );
  else
    return 0.0;
}

double QgsCircularString::yAt( int index ) const
{
  if ( index >= 0 && index < mY.size() )
    return mY.at( index );
  else
    return 0.0;
}

bool QgsCircularString::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
{
  if ( !transformer )
    return false;

  bool hasZ = is3D();
  bool hasM = isMeasure();
  int size = mX.size();

  double *srcX = mX.data();
  double *srcY = mY.data();
  double *srcM = hasM ? mM.data() : nullptr;
  double *srcZ = hasZ ? mZ.data() : nullptr;

  bool res = true;
  for ( int i = 0; i < size; ++i )
  {
    double x = *srcX;
    double y = *srcY;
    double z = hasZ ? *srcZ : std::numeric_limits<double>::quiet_NaN();
    double m = hasM ? *srcM : std::numeric_limits<double>::quiet_NaN();
    if ( !transformer->transformPoint( x, y, z, m ) )
    {
      res = false;
      break;
    }

    *srcX++ = x;
    *srcY++ = y;
    if ( hasM )
      *srcM++ = m;
    if ( hasZ )
      *srcZ++ = z;

    if ( feedback && feedback->isCanceled() )
    {
      res = false;
      break;
    }
  }
  clearCache();
  return res;
}

void QgsCircularString::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int size = mX.size();

  double *srcX = mX.data(); // clazy:exclude=detaching-member
  double *srcY = mY.data(); // clazy:exclude=detaching-member
  double *srcM = hasM ? mM.data() : nullptr; // clazy:exclude=detaching-member
  double *srcZ = hasZ ? mZ.data() : nullptr; // clazy:exclude=detaching-member

  double *destX = srcX;
  double *destY = srcY;
  double *destM = srcM;
  double *destZ = srcZ;

  int filteredPoints = 0;
  for ( int i = 0; i < size; ++i )
  {
    double x = *srcX++;
    double y = *srcY++;
    double z = hasZ ? *srcZ++ : std::numeric_limits<double>::quiet_NaN();
    double m = hasM ? *srcM++ : std::numeric_limits<double>::quiet_NaN();

    if ( filter( QgsPoint( x, y, z, m ) ) )
    {
      filteredPoints++;
      *destX++ = x;
      *destY++ = y;
      if ( hasM )
        *destM++ = m;
      if ( hasZ )
        *destZ++ = z;
    }
  }

  mX.resize( filteredPoints );
  mY.resize( filteredPoints );
  if ( hasZ )
    mZ.resize( filteredPoints );
  if ( hasM )
    mM.resize( filteredPoints );

  clearCache();
}

void QgsCircularString::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int size = mX.size();

  double *srcX = mX.data();
  double *srcY = mY.data();
  double *srcM = hasM ? mM.data() : nullptr;
  double *srcZ = hasZ ? mZ.data() : nullptr;

  for ( int i = 0; i < size; ++i )
  {
    double x = *srcX;
    double y = *srcY;
    double z = hasZ ? *srcZ : std::numeric_limits<double>::quiet_NaN();
    double m = hasM ? *srcM : std::numeric_limits<double>::quiet_NaN();
    QgsPoint res = transform( QgsPoint( x, y, z, m ) );
    *srcX++ = res.x();
    *srcY++ = res.y();
    if ( hasM )
      *srcM++ = res.m();
    if ( hasZ )
      *srcZ++ = res.z();
  }
  clearCache();
}

std::tuple<std::unique_ptr<QgsCurve>, std::unique_ptr<QgsCurve> > QgsCircularString::splitCurveAtVertex( int index ) const
{
  const bool useZ = is3D();
  const bool useM = isMeasure();

  const int size = mX.size();
  if ( size == 0 )
    return std::make_tuple( std::make_unique< QgsCircularString >(), std::make_unique< QgsCircularString >() );

  index = std::clamp( index, 0, size - 1 );

  const int part1Size = index + 1;
  QVector< double > x1( part1Size );
  QVector< double > y1( part1Size );
  QVector< double > z1( useZ ? part1Size : 0 );
  QVector< double > m1( useM ? part1Size : 0 );

  const double *sourceX = mX.constData();
  const double *sourceY = mY.constData();
  const double *sourceZ = useZ ? mZ.constData() : nullptr;
  const double *sourceM = useM ? mM.constData() : nullptr;

  double *destX = x1.data();
  double *destY = y1.data();
  double *destZ = useZ ? z1.data() : nullptr;
  double *destM = useM ? m1.data() : nullptr;

  std::copy( sourceX, sourceX + part1Size, destX );
  std::copy( sourceY, sourceY + part1Size, destY );
  if ( useZ )
    std::copy( sourceZ, sourceZ + part1Size, destZ );
  if ( useM )
    std::copy( sourceM, sourceM + part1Size, destM );

  const int part2Size = size - index;
  if ( part2Size < 2 )
    return std::make_tuple( std::make_unique< QgsCircularString >( x1, y1, z1, m1 ), std::make_unique< QgsCircularString >() );

  QVector< double > x2( part2Size );
  QVector< double > y2( part2Size );
  QVector< double > z2( useZ ? part2Size : 0 );
  QVector< double > m2( useM ? part2Size : 0 );
  destX = x2.data();
  destY = y2.data();
  destZ = useZ ? z2.data() : nullptr;
  destM = useM ? m2.data() : nullptr;
  std::copy( sourceX + index, sourceX + size, destX );
  std::copy( sourceY + index, sourceY + size, destY );
  if ( useZ )
    std::copy( sourceZ + index, sourceZ + size, destZ );
  if ( useM )
    std::copy( sourceM + index, sourceM + size, destM );

  if ( part1Size < 2 )
    return std::make_tuple( std::make_unique< QgsCircularString >(), std::make_unique< QgsCircularString >( x2, y2, z2, m2 ) );
  else
    return std::make_tuple( std::make_unique< QgsCircularString >( x1, y1, z1, m1 ), std::make_unique< QgsCircularString >( x2, y2, z2, m2 ) );
}

void QgsCircularString::points( QgsPointSequence &pts ) const
{
  pts.clear();
  int nPts = numPoints();
  for ( int i = 0; i < nPts; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsCircularString::setPoints( const QgsPointSequence &points )
{
  clearCache();

  if ( points.empty() )
  {
    mWkbType = QgsWkbTypes::CircularString;
    mX.clear();
    mY.clear();
    mZ.clear();
    mM.clear();
    return;
  }

  //get wkb type from first point
  const QgsPoint &firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWkbTypes::CircularString );

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

void QgsCircularString::append( const QgsCircularString *line )
{
  if ( !line || line->isEmpty() )
  {
    return;
  }

  if ( numPoints() < 1 )
  {
    setZMTypeFromSubGeometry( line, QgsWkbTypes::CircularString );
  }

  // do not store duplicate points
  if ( numPoints() > 0 &&
       line->numPoints() > 0 &&
       qgsDoubleNear( endPoint().x(), line->startPoint().x() ) &&
       qgsDoubleNear( endPoint().y(), line->startPoint().y() ) &&
       ( !is3D() || !line->is3D() || qgsDoubleNear( endPoint().z(), line->startPoint().z() ) ) &&
       ( !isMeasure() || !line->isMeasure() || qgsDoubleNear( endPoint().m(), line->startPoint().m() ) ) )
  {
    mX.pop_back();
    mY.pop_back();

    if ( is3D() && line->is3D() )
    {
      mZ.pop_back();
    }
    if ( isMeasure() && line->isMeasure() )
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

void QgsCircularString::draw( QPainter &p ) const
{
  QPainterPath path;
  addToPainterPath( path );
  p.drawPath( path );
}

void QgsCircularString::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  clearCache();

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
}

void QgsCircularString::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  clearCache();

  int nPoints = numPoints();
  bool hasZ = is3D();
  bool hasM = isMeasure();
  for ( int i = 0; i < nPoints; ++i )
  {
    qreal x, y;
    t.map( mX.at( i ), mY.at( i ), &x, &y );
    mX[i] = x;
    mY[i] = y;
    if ( hasZ )
    {
      mZ[i] = mZ.at( i ) * zScale + zTranslate;
    }
    if ( hasM )
    {
      mM[i] = mM.at( i ) * mScale + mTranslate;
    }
  }
}

void arcTo( QPainterPath &path, QPointF pt1, QPointF pt2, QPointF pt3 )
{
  double centerX, centerY, radius;
  QgsGeometryUtils::circleCenterRadius( QgsPoint( pt1.x(), pt1.y() ), QgsPoint( pt2.x(), pt2.y() ), QgsPoint( pt3.x(), pt3.y() ),
                                        radius, centerX, centerY );

  double p1Angle = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double sweepAngle = QgsGeometryUtils::sweepAngle( centerX, centerY, pt1.x(), pt1.y(), pt2.x(), pt2.y(), pt3.x(), pt3.y() );

  double diameter = 2 * radius;
  path.arcTo( centerX - radius, centerY - radius, diameter, diameter, -p1Angle, -sweepAngle );
}

void QgsCircularString::addToPainterPath( QPainterPath &path ) const
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
    arcTo( path, QPointF( mX[i], mY[i] ), QPointF( mX[i + 1], mY[i + 1] ), QPointF( mX[i + 2], mY[i + 2] ) );
  }

  //if number of points is even, connect to last point with straight line (even though the circular string is not valid)
  if ( nPoints % 2 == 0 )
  {
    path.lineTo( mX[ nPoints - 1 ], mY[ nPoints - 1 ] );
  }
}

void QgsCircularString::drawAsPolygon( QPainter &p ) const
{
  draw( p );
}

bool QgsCircularString::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  if ( position.vertex >= mX.size() || position.vertex < 1 )
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
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsCircularString::moveVertex( QgsVertexId position, const QgsPoint &newPos )
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

bool QgsCircularString::deleteVertex( QgsVertexId position )
{
  int nVertices = this->numPoints();
  if ( nVertices < 4 ) //circular string must have at least 3 vertices
  {
    clear();
    return true;
  }
  if ( position.vertex < 0 || position.vertex > ( nVertices - 1 ) )
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

  clearCache(); //set bounding box invalid
  return true;
}

void QgsCircularString::deleteVertex( int i )
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
  clearCache();
}

double QgsCircularString::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  double minDist = std::numeric_limits<double>::max();
  QgsPoint minDistSegmentPoint;
  QgsVertexId minDistVertexAfter;
  int minDistLeftOf = 0;

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

  if ( minDist == std::numeric_limits<double>::max() )
    return -1; // error: no segments

  segmentPt = minDistSegmentPoint;
  vertexAfter = minDistVertexAfter;
  vertexAfter.part = 0;
  vertexAfter.ring = 0;
  if ( leftOf )
  {
    *leftOf = qgsDoubleNear( minDist, 0.0 ) ? 0 : minDistLeftOf;
  }
  return minDist;
}

bool QgsCircularString::pointAt( int node, QgsPoint &point, Qgis::VertexType &type ) const
{
  if ( node < 0 || node >= numPoints() )
  {
    return false;
  }
  point = pointN( node );
  type = ( node % 2 == 0 ) ? Qgis::VertexType::Segment : Qgis::VertexType::Curve;
  return true;
}

void QgsCircularString::sumUpArea( double &sum ) const
{
  if ( mHasCachedSummedUpArea )
  {
    sum += mSummedUpArea;
    return;
  }

  int maxIndex = numPoints() - 2;
  mSummedUpArea = 0;
  for ( int i = 0; i < maxIndex; i += 2 )
  {
    QgsPoint p1( mX[i], mY[i] );
    QgsPoint p2( mX[i + 1], mY[i + 1] );
    QgsPoint p3( mX[i + 2], mY[i + 2] );

    //segment is a full circle, p2 is the center point
    if ( p1 == p3 )
    {
      double r2 = QgsGeometryUtils::sqrDistance2D( p1, p2 ) / 4.0;
      mSummedUpArea += M_PI * r2;
      continue;
    }

    mSummedUpArea += 0.5 * ( mX[i] * mY[i + 2] - mY[i] * mX[i + 2] );

    //calculate area between circle and chord, then sum / subtract from total area
    double midPointX = ( p1.x() + p3.x() ) / 2.0;
    double midPointY = ( p1.y() + p3.y() ) / 2.0;

    double radius, centerX, centerY;
    QgsGeometryUtils::circleCenterRadius( p1, p2, p3, radius, centerX, centerY );

    double d = std::sqrt( QgsGeometryUtils::sqrDistance2D( QgsPoint( centerX, centerY ), QgsPoint( midPointX, midPointY ) ) );
    double r2 = radius * radius;

    if ( d > radius )
    {
      //d cannot be greater than radius, something must be wrong...
      continue;
    }

    bool circlePointLeftOfLine = QgsGeometryUtils::leftOfLine( p2.x(), p2.y(), p1.x(), p1.y(), p3.x(), p3.y() ) < 0;
    bool centerPointLeftOfLine = QgsGeometryUtils::leftOfLine( centerX, centerY, p1.x(), p1.y(), p3.x(), p3.y() ) < 0;

    double cov = 0.5 - d * std::sqrt( r2 - d * d ) / ( M_PI * r2 ) - M_1_PI * std::asin( d / radius );
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
      mSummedUpArea += circleChordArea;
    }
    else
    {
      mSummedUpArea -= circleChordArea;
    }
  }

  mHasCachedSummedUpArea = true;
  sum += mSummedUpArea;
}

bool QgsCircularString::hasCurvedSegments() const
{
  return true;
}

double QgsCircularString::closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
    const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon )
{
  double radius, centerX, centerY;
  QgsPoint pt1( x1, y1 );
  QgsPoint pt2( x2, y2 );
  QgsPoint pt3( x3, y3 );

  QgsGeometryUtils::circleCenterRadius( pt1, pt2, pt3, radius, centerX, centerY );
  double angle = QgsGeometryUtils::ccwAngle( pt.y() - centerY, pt.x() - centerX );
  double angle1 = QgsGeometryUtils::ccwAngle( pt1.y() - centerY, pt1.x() - centerX );
  double angle2 = QgsGeometryUtils::ccwAngle( pt2.y() - centerY, pt2.x() - centerX );
  double angle3 = QgsGeometryUtils::ccwAngle( pt3.y() - centerY, pt3.x() - centerX );

  bool clockwise = QgsGeometryUtils::circleClockwise( angle1, angle2, angle3 );

  if ( QgsGeometryUtils::angleOnCircle( angle, angle1, angle2, angle3 ) )
  {
    //get point on line center -> pt with distance radius
    segmentPt = QgsGeometryUtils::pointOnLineWithDistance( QgsPoint( centerX, centerY ), pt, radius );

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
    double sqrDistancePointToCenter = ( pt.x() - centerX ) * ( pt.x() - centerX ) + ( pt.y() - centerY ) * ( pt.y() - centerY );
    *leftOf = clockwise ? ( sqrDistancePointToCenter > radius * radius ? -1 : 1 )
              : ( sqrDistancePointToCenter < radius * radius ? -1 : 1 );
  }

  return sqrDistance;
}

void QgsCircularString::insertVertexBetween( int after, int before, int pointOnCircle )
{
  double xAfter = mX.at( after );
  double yAfter = mY.at( after );
  double xBefore = mX.at( before );
  double yBefore = mY.at( before );
  double xOnCircle = mX.at( pointOnCircle );
  double yOnCircle = mY.at( pointOnCircle );

  double radius, centerX, centerY;
  QgsGeometryUtils::circleCenterRadius( QgsPoint( xAfter, yAfter ), QgsPoint( xBefore, yBefore ), QgsPoint( xOnCircle, yOnCircle ), radius, centerX, centerY );

  double x = ( xAfter + xBefore ) / 2.0;
  double y = ( yAfter + yBefore ) / 2.0;

  QgsPoint newVertex = QgsGeometryUtils::pointOnLineWithDistance( QgsPoint( centerX, centerY ), QgsPoint( x, y ), radius );
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
  clearCache();
}

double QgsCircularString::vertexAngle( QgsVertexId vId ) const
{
  if ( numPoints() < 3 )
  {
    //undefined
    return 0.0;
  }

  int before = vId.vertex - 1;
  int vertex = vId.vertex;
  int after = vId.vertex + 1;

  if ( vId.vertex % 2 != 0 ) // a curve vertex
  {
    if ( vId.vertex >= 1 && vId.vertex < numPoints() - 1 )
    {
      return QgsGeometryUtils::circleTangentDirection( QgsPoint( mX[vertex], mY[vertex] ), QgsPoint( mX[before], mY[before] ),
             QgsPoint( mX[vertex], mY[vertex] ), QgsPoint( mX[after], mY[after] ) );
    }
  }
  else //a point vertex
  {
    if ( vId.vertex == 0 )
    {
      return QgsGeometryUtils::circleTangentDirection( QgsPoint( mX[0], mY[0] ), QgsPoint( mX[0], mY[0] ),
             QgsPoint( mX[1], mY[1] ), QgsPoint( mX[2], mY[2] ) );
    }
    if ( vId.vertex >= numPoints() - 1 )
    {
      int a = numPoints() - 3;
      int b = numPoints() - 2;
      int c = numPoints() - 1;
      return QgsGeometryUtils::circleTangentDirection( QgsPoint( mX[c], mY[c] ), QgsPoint( mX[a], mY[a] ),
             QgsPoint( mX[b], mY[b] ), QgsPoint( mX[c], mY[c] ) );
    }
    else
    {
      if ( vId.vertex + 2 > numPoints() - 1 )
      {
        return 0.0;
      }

      int vertex1 = vId.vertex - 2;
      int vertex2 = vId.vertex - 1;
      int vertex3 = vId.vertex;
      double angle1 = QgsGeometryUtils::circleTangentDirection( QgsPoint( mX[vertex3], mY[vertex3] ),
                      QgsPoint( mX[vertex1], mY[vertex1] ), QgsPoint( mX[vertex2], mY[vertex2] ), QgsPoint( mX[vertex3], mY[vertex3] ) );
      int vertex4 = vId.vertex + 1;
      int vertex5 = vId.vertex + 2;
      double angle2 = QgsGeometryUtils::circleTangentDirection( QgsPoint( mX[vertex3], mY[vertex3] ),
                      QgsPoint( mX[vertex3], mY[vertex3] ), QgsPoint( mX[vertex4], mY[vertex4] ), QgsPoint( mX[vertex5], mY[vertex5] ) );
      return QgsGeometryUtils::averageAngle( angle1, angle2 );
    }
  }
  return 0.0;
}

double QgsCircularString::segmentLength( QgsVertexId startVertex ) const
{
  if ( startVertex.vertex < 0 || startVertex.vertex >= mX.count() - 2 )
    return 0.0;

  if ( startVertex.vertex % 2 == 1 )
    return 0.0; // curve point?

  double x1 = mX.at( startVertex.vertex );
  double y1 = mY.at( startVertex.vertex );
  double x2 = mX.at( startVertex.vertex + 1 );
  double y2 = mY.at( startVertex.vertex + 1 );
  double x3 = mX.at( startVertex.vertex + 2 );
  double y3 = mY.at( startVertex.vertex + 2 );
  return QgsGeometryUtils::circleLength( x1, y1, x2, y2, x3, y3 );
}

QgsCircularString *QgsCircularString::reversed() const
{
  QgsCircularString *copy = clone();
  std::reverse( copy->mX.begin(), copy->mX.end() );
  std::reverse( copy->mY.begin(), copy->mY.end() );
  if ( is3D() )
  {
    std::reverse( copy->mZ.begin(), copy->mZ.end() );
  }
  if ( isMeasure() )
  {
    std::reverse( copy->mM.begin(), copy->mM.end() );
  }
  return copy;
}

QgsPoint *QgsCircularString::interpolatePoint( const double distance ) const
{
  if ( distance < 0 )
    return nullptr;

  double distanceTraversed = 0;
  const int totalPoints = numPoints();
  if ( totalPoints == 0 )
    return nullptr;

  QgsWkbTypes::Type pointType = QgsWkbTypes::Point;
  if ( is3D() )
    pointType = QgsWkbTypes::PointZ;
  if ( isMeasure() )
    pointType = QgsWkbTypes::addM( pointType );

  const double *x = mX.constData();
  const double *y = mY.constData();
  const double *z = is3D() ? mZ.constData() : nullptr;
  const double *m = isMeasure() ? mM.constData() : nullptr;

  double prevX = *x++;
  double prevY = *y++;
  double prevZ = z ? *z++ : 0.0;
  double prevM = m ? *m++ : 0.0;

  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    return new QgsPoint( pointType, prevX, prevY, prevZ, prevM );
  }

  for ( int i = 0; i < ( totalPoints - 2 ) ; i += 2 )
  {
    double x1 = prevX;
    double y1 = prevY;
    double z1 = prevZ;
    double m1 = prevM;

    double x2 = *x++;
    double y2 = *y++;
    double z2 = z ? *z++ : 0.0;
    double m2 = m ? *m++ : 0.0;

    double x3 = *x++;
    double y3 = *y++;
    double z3 = z ? *z++ : 0.0;
    double m3 = m ? *m++ : 0.0;

    const double segmentLength = QgsGeometryUtils::circleLength( x1, y1, x2, y2, x3, y3 );
    if ( distance < distanceTraversed + segmentLength || qgsDoubleNear( distance, distanceTraversed + segmentLength ) )
    {
      // point falls on this segment - truncate to segment length if qgsDoubleNear test was actually > segment length
      const double distanceToPoint = std::min( distance - distanceTraversed, segmentLength );
      return new QgsPoint( QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                           QgsPoint( pointType, x2, y2, z2, m2 ),
                           QgsPoint( pointType, x3, y3, z3, m3 ), distanceToPoint ) );
    }

    distanceTraversed += segmentLength;

    prevX = x3;
    prevY = y3;
    prevZ = z3;
    prevM = m3;
  }

  return nullptr;
}

QgsCircularString *QgsCircularString::curveSubstring( double startDistance, double endDistance ) const
{
  if ( startDistance < 0 && endDistance < 0 )
    return createEmptyWithSameType();

  endDistance = std::max( startDistance, endDistance );

  const int totalPoints = numPoints();
  if ( totalPoints == 0 )
    return clone();

  QVector< QgsPoint > substringPoints;
  substringPoints.reserve( totalPoints );

  QgsWkbTypes::Type pointType = QgsWkbTypes::Point;
  if ( is3D() )
    pointType = QgsWkbTypes::PointZ;
  if ( isMeasure() )
    pointType = QgsWkbTypes::addM( pointType );

  const double *x = mX.constData();
  const double *y = mY.constData();
  const double *z = is3D() ? mZ.constData() : nullptr;
  const double *m = isMeasure() ? mM.constData() : nullptr;

  double distanceTraversed = 0;
  double prevX = *x++;
  double prevY = *y++;
  double prevZ = z ? *z++ : 0.0;
  double prevM = m ? *m++ : 0.0;
  bool foundStart = false;

  if ( startDistance < 0 )
    startDistance = 0;

  for ( int i = 0; i < ( totalPoints - 2 ) ; i += 2 )
  {
    double x1 = prevX;
    double y1 = prevY;
    double z1 = prevZ;
    double m1 = prevM;

    double x2 = *x++;
    double y2 = *y++;
    double z2 = z ? *z++ : 0.0;
    double m2 = m ? *m++ : 0.0;

    double x3 = *x++;
    double y3 = *y++;
    double z3 = z ? *z++ : 0.0;
    double m3 = m ? *m++ : 0.0;

    bool addedSegmentEnd = false;
    const double segmentLength = QgsGeometryUtils::circleLength( x1, y1, x2, y2, x3, y3 );
    if ( distanceTraversed <= startDistance && startDistance < distanceTraversed + segmentLength )
    {
      // start point falls on this segment
      const double distanceToStart = startDistance - distanceTraversed;
      const QgsPoint startPoint = QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                                  QgsPoint( pointType, x2, y2, z2, m2 ),
                                  QgsPoint( pointType, x3, y3, z3, m3 ), distanceToStart );

      // does end point also fall on this segment?
      const bool endPointOnSegment = distanceTraversed + segmentLength > endDistance;
      if ( endPointOnSegment )
      {
        const double distanceToEnd = endDistance - distanceTraversed;
        const double midPointDistance = ( distanceToEnd - distanceToStart ) * 0.5 + distanceToStart;
        substringPoints << startPoint
                        << QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                            QgsPoint( pointType, x2, y2, z2, m2 ),
                            QgsPoint( pointType, x3, y3, z3, m3 ), midPointDistance )
                        << QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                            QgsPoint( pointType, x2, y2, z2, m2 ),
                            QgsPoint( pointType, x3, y3, z3, m3 ), distanceToEnd );
        addedSegmentEnd = true;
      }
      else
      {
        const double midPointDistance = ( segmentLength - distanceToStart ) * 0.5 + distanceToStart;
        substringPoints << startPoint
                        << QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                            QgsPoint( pointType, x2, y2, z2, m2 ),
                            QgsPoint( pointType, x3, y3, z3, m3 ), midPointDistance )
                        << QgsPoint( pointType, x3, y3, z3, m3 );
        addedSegmentEnd = true;
      }
      foundStart = true;
    }
    if ( !addedSegmentEnd && foundStart && ( distanceTraversed + segmentLength > endDistance ) )
    {
      // end point falls on this segment
      const double distanceToEnd = endDistance - distanceTraversed;
      // add mid point, at half way along this arc, then add the interpolated end point
      substringPoints <<  QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                      QgsPoint( pointType, x2, y2, z2, m2 ),
                      QgsPoint( pointType, x3, y3, z3, m3 ), distanceToEnd / 2.0 )

                      << QgsGeometryUtils::interpolatePointOnArc( QgsPoint( pointType, x1, y1, z1, m1 ),
                          QgsPoint( pointType, x2, y2, z2, m2 ),
                          QgsPoint( pointType, x3, y3, z3, m3 ), distanceToEnd );
    }
    else if ( !addedSegmentEnd && foundStart )
    {
      substringPoints << QgsPoint( pointType, x2, y2, z2, m2 )
                      << QgsPoint( pointType, x3, y3, z3, m3 );
    }

    prevX = x3;
    prevY = y3;
    prevZ = z3;
    prevM = m3;
    distanceTraversed += segmentLength;
    if ( distanceTraversed >= endDistance )
      break;
  }

  // start point is the last node
  if ( !foundStart && qgsDoubleNear( distanceTraversed, startDistance ) )
  {
    substringPoints << QgsPoint( pointType, prevX, prevY, prevZ, prevM )
                    << QgsPoint( pointType, prevX, prevY, prevZ, prevM )
                    << QgsPoint( pointType, prevX, prevY, prevZ, prevM );
  }

  std::unique_ptr< QgsCircularString > result = std::make_unique< QgsCircularString >();
  result->setPoints( substringPoints );
  return result.release();
}

bool QgsCircularString::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::addZ( mWkbType );

  int nPoints = numPoints();
  mZ.clear();
  mZ.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mZ << zValue;
  }
  return true;
}

bool QgsCircularString::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::addM( mWkbType );

  int nPoints = numPoints();
  mM.clear();
  mM.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mM << mValue;
  }
  return true;
}

bool QgsCircularString::dropZValue()
{
  if ( !QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  clearCache();

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  mZ.clear();
  return true;
}

bool QgsCircularString::dropMValue()
{
  if ( !QgsWkbTypes::hasM( mWkbType ) )
    return false;

  clearCache();

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  mM.clear();
  return true;
}

void QgsCircularString::swapXy()
{
  std::swap( mX, mY );
  clearCache();
}
