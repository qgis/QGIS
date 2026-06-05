/***************************************************************************
                       qgssimplecurve.h
                         ------------
    begin                : May 2026
    copyright            : (C) 2014 by Marco Hugentobler
                           (C) 2026 by Germán Carrillo
    email                : marco at sourcepole dot ch
                           german at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssimplecurve.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsfeedback.h"
#include "qgsgeometrytransformer.h"
#include "qgsgeometryutils.h"

#include <QString>

using namespace Qt::StringLiterals;

int QgsSimpleCurve::wkbSize( QgsAbstractGeometry::WkbFlags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  binarySize += numPoints() * ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return binarySize;
}

QByteArray QgsSimpleCurve::asWkb( WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsSimpleCurve::wkbSize( flags ) );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  QgsPointSequence pts;
  points( pts );
  QgsGeometryUtils::pointsToWKB( wkb, pts, is3D(), isMeasure(), flags );
  return wkbArray;
}

QString QgsSimpleCurve::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + ' ';

  if ( isEmpty() )
    wkt += "EMPTY"_L1;
  else
  {
    QgsPointSequence pts;
    points( pts );
    wkt += QgsGeometryUtils::pointsToWKT( pts, precision, is3D(), isMeasure() );
  }
  return wkt;
}

bool QgsSimpleCurve::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
  {
    return false;
  }

  Qgis::WkbType type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::flatType( mWkbType ) )
  {
    return false;
  }
  mWkbType = type;
  importVerticesFromWkb( wkbPtr );
  return true;
}

void QgsSimpleCurve::importVerticesFromWkb( const QgsConstWkbPtr &wkb )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int nVertices = 0;
  wkb >> nVertices;
  mX.resize( nVertices );
  mY.resize( nVertices );
  hasZ ? mZ.resize( nVertices ) : mZ.clear();
  hasM ? mM.resize( nVertices ) : mM.clear();
  double *x = mX.data();
  double *y = mY.data();
  double *m = hasM ? mM.data() : nullptr;
  double *z = hasZ ? mZ.data() : nullptr;
  for ( int i = 0; i < nVertices; ++i )
  {
    wkb >> *x++;
    wkb >> *y++;
    if ( hasZ )
    {
      wkb >> *z++;
    }
    if ( hasM )
    {
      wkb >> *m++;
    }
  }
  clearCache(); //set bounding box invalid
}

bool QgsSimpleCurve::fromWkt( const QString &wkt )
{
  clear();

  QPair<Qgis::WkbType, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::flatType( mWkbType ) )
    return false;
  mWkbType = parts.first;

  parts.second = parts.second.remove( '(' ).remove( ')' );
  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.simplified().remove( ' ' );
  if ( ( parts.second.compare( "EMPTY"_L1, Qt::CaseInsensitive ) == 0 ) || secondWithoutParentheses.isEmpty() )
    return true;

  QgsPointSequence points = QgsGeometryUtils::pointsFromWKT( parts.second, is3D(), isMeasure() );
  // There is a non number in the coordinates sequence
  // LineString ( A b, 1 2)
  if ( points.isEmpty() )
    return false;

  setPoints( points );
  return true;
}

void QgsSimpleCurve::clear()
{
  mX.clear();
  mY.clear();
  mZ.clear();
  mM.clear();
  clearCache();
}

QgsPoint QgsSimpleCurve::pointN( int i ) const
{
  if ( i < 0 || i >= mX.size() )
  {
    return QgsPoint();
  }

  double x = mX.at( i );
  double y = mY.at( i );
  double z = std::numeric_limits<double>::quiet_NaN(); // TODO: QgsCircularString had 0 here!
  double m = std::numeric_limits<double>::quiet_NaN(); // TODO: QgsCircularString had 0 here!

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

  Qgis::WkbType t = Qgis::WkbType::Point;
  if ( mWkbType == Qgis::WkbType::LineString25D )
  {
    t = Qgis::WkbType::Point25D;
  }
  else if ( hasZ && hasM )
  {
    t = Qgis::WkbType::PointZM;
  }
  else if ( hasZ )
  {
    t = Qgis::WkbType::PointZ;
  }
  else if ( hasM )
  {
    t = Qgis::WkbType::PointM;
  }
  return QgsPoint( t, x, y, z, m );
}

int QgsSimpleCurve::numPoints() const
{
  return mX.size();
}

int QgsSimpleCurve::nCoordinates() const
{
  return mX.size();
}

bool QgsSimpleCurve::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::addM( mWkbType );

  mM.clear();
  int nPoints = numPoints();
  mM.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    mM << mValue;
  }
  return true;
}

bool QgsSimpleCurve::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  clearCache();
  if ( mWkbType == Qgis::WkbType::Unknown )
  {
    mWkbType = Qgis::WkbType::LineStringZ;
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

bool QgsSimpleCurve::dropMValue()
{
  if ( !isMeasure() )
    return false;

  clearCache();
  mWkbType = QgsWkbTypes::dropM( mWkbType );
  mM.clear();
  return true;
}

bool QgsSimpleCurve::dropZValue()
{
  if ( !is3D() )
    return false;

  clearCache();

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  mZ.clear();
  return true;
}

double QgsSimpleCurve::xAt( int index ) const
{
  if ( index >= 0 && index < mX.size() )
    return mX.at( index );
  else
    return 0.0;
}

double QgsSimpleCurve::yAt( int index ) const
{
  if ( index >= 0 && index < mY.size() )
    return mY.at( index );
  else
    return 0.0;
}

void QgsSimpleCurve::setXAt( int index, double x )
{
  if ( index >= 0 && index < mX.size() )
    mX[index] = x;
  clearCache();
}

void QgsSimpleCurve::setYAt( int index, double y )
{
  if ( index >= 0 && index < mY.size() )
    mY[index] = y;
  clearCache();
}

int QgsSimpleCurve::dimension() const
{
  return 1;
}

QgsPoint QgsSimpleCurve::startPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( 0 );
}

QgsPoint QgsSimpleCurve::endPoint() const
{
  if ( numPoints() < 1 )
  {
    return QgsPoint();
  }
  return pointN( numPoints() - 1 );
}

void QgsSimpleCurve::filterVertices( const std::function<bool( const QgsPoint & )> &filter )
{
  bool hasZ = is3D();
  bool hasM = isMeasure();
  int size = mX.size();

  double *srcX = mX.data();
  double *srcY = mY.data();
  double *srcM = hasM ? mM.data() : nullptr;
  double *srcZ = hasZ ? mZ.data() : nullptr;

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

void QgsSimpleCurve::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
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

bool QgsSimpleCurve::moveVertex( QgsVertexId position, const QgsPoint &newPos )
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

QgsSimpleCurve *QgsSimpleCurve::reversed() const
{
  QgsSimpleCurve *copy = qgis::down_cast<QgsSimpleCurve *>( clone() );
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

  copy->mSummedUpArea = -mSummedUpArea;
  return copy;
}

int QgsSimpleCurve::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsSimpleCurve *otherCurve = qgsgeometry_cast<const QgsSimpleCurve *>( other );
  if ( !otherCurve )
    return -1;

  const int size = mX.size();
  const int otherSize = otherCurve->mX.size();
  if ( size > otherSize )
  {
    return 1;
  }
  else if ( size < otherSize )
  {
    return -1;
  }

  if ( is3D() && !otherCurve->is3D() )
    return 1;
  else if ( !is3D() && otherCurve->is3D() )
    return -1;
  const bool considerZ = is3D();

  if ( isMeasure() && !otherCurve->isMeasure() )
    return 1;
  else if ( !isMeasure() && otherCurve->isMeasure() )
    return -1;
  const bool considerM = isMeasure();

  for ( int i = 0; i < size; i++ )
  {
    const double x = mX[i];
    const double otherX = otherCurve->mX[i];
    if ( x < otherX )
    {
      return -1;
    }
    else if ( x > otherX )
    {
      return 1;
    }

    const double y = mY[i];
    const double otherY = otherCurve->mY[i];
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
      const double otherZ = otherCurve->mZ[i];

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
      const double otherM = otherCurve->mM[i];

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

void QgsSimpleCurve::splitCurveAtVertexProtected(
  int index, QVector< double > &x1, QVector< double > &y1, QVector< double > &z1, QVector< double > &m1, QVector< double > &x2, QVector< double > &y2, QVector< double > &z2, QVector< double > &m2
) const
{
  const bool useZ = is3D();
  const bool useM = isMeasure();

  const int size = mX.size();
  if ( size == 0 )
    return;

  index = std::clamp( index, 0, size - 1 );

  const int part1Size = index + 1;
  x1.resize( part1Size );
  y1.resize( part1Size );
  z1.resize( useZ ? part1Size : 0 );
  m1.resize( useM ? part1Size : 0 );

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
  x2.resize( part2Size );
  y2.resize( part2Size );
  z2.resize( useZ ? part2Size : 0 );
  m2.resize( useM ? part2Size : 0 );

  if ( part2Size < 2 )
    return;

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

  return;
}

void QgsSimpleCurve::append( const QgsSimpleCurve *curve )
{
  if ( !curve || curve->isEmpty() || QgsWkbTypes::flatType( mWkbType ) != QgsWkbTypes::flatType( curve->wkbType() ) )
  {
    return;
  }

  if ( numPoints() < 1 )
  {
    setZMTypeFromSubGeometry( curve, QgsWkbTypes::flatType( mWkbType ) );
  }

  // do not store duplicate points
  if ( numPoints() > 0
       && curve->numPoints() > 0
       && qgsDoubleNear( endPoint().x(), curve->startPoint().x() )
       && qgsDoubleNear( endPoint().y(), curve->startPoint().y() )
       && ( !is3D() || !curve->is3D() || qgsDoubleNear( endPoint().z(), curve->startPoint().z() ) )
       && ( !isMeasure() || !curve->isMeasure() || qgsDoubleNear( endPoint().m(), curve->startPoint().m() ) ) )
  {
    mX.pop_back();
    mY.pop_back();

    if ( is3D() && curve->is3D() )
    {
      mZ.pop_back();
    }
    if ( isMeasure() && curve->isMeasure() )
    {
      mM.pop_back();
    }
  }

  mX += curve->mX;
  mY += curve->mY;

  if ( is3D() )
  {
    if ( curve->is3D() )
    {
      mZ += curve->mZ;
    }
    else
    {
      // if append line does not have z coordinates, fill with NaN to match number of points in final line
      mZ.insert( mZ.count(), mX.size() - mZ.size(), std::numeric_limits<double>::quiet_NaN() );
    }
  }

  if ( isMeasure() )
  {
    if ( curve->isMeasure() )
    {
      mM += curve->mM;
    }
    else
    {
      // if append line does not have m values, fill with NaN to match number of points in final line
      mM.insert( mM.count(), mX.size() - mM.size(), std::numeric_limits<double>::quiet_NaN() );
    }
  }

  clearCache(); //set bounding box invalid
}

void QgsSimpleCurve::points( QgsPointSequence &pts ) const
{
  pts.clear();
  int nPoints = numPoints();
  pts.reserve( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    pts.push_back( pointN( i ) );
  }
}

void QgsSimpleCurve::setPoints( const QgsPointSequence &points )
{
  if ( points.isEmpty() )
  {
    clear();
    return;
  }

  clearCache(); //set bounding box invalid

  //get wkb type from first point
  const QgsPoint &firstPt = points.at( 0 );
  bool hasZ = firstPt.is3D();
  bool hasM = firstPt.isMeasure();

  setZMTypeFromSubGeometry( &firstPt, QgsWkbTypes::flatType( mWkbType ) );

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

void QgsSimpleCurve::scroll( int index )
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

void QgsSimpleCurve::swapXy()
{
  std::swap( mX, mY );
  clearCache();
}

bool QgsSimpleCurve::isEmpty() const
{
  return mX.isEmpty();
}


bool QgsSimpleCurve::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
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

void QgsSimpleCurve::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  double *zArray = nullptr;
  bool hasZ = is3D();
  int nPoints = numPoints();

  // it's possible that transformCoords will throw an exception - so we need to use
  // a smart pointer for the dummy z values in order to ensure that they always get cleaned up
  std::unique_ptr< double[] > dummyZ;
  if ( !hasZ || !transformZ )
  {
    dummyZ = std::make_unique<double[]>( nPoints );
    zArray = dummyZ.get();
  }
  else
  {
    zArray = mZ.data();
  }
  ct.transformCoords( nPoints, mX.data(), mY.data(), zArray, d );
  clearCache();
}

void QgsSimpleCurve::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  int nPoints = numPoints();
  bool hasZ = is3D();
  bool hasM = isMeasure();
  double *x = mX.data();
  double *y = mY.data();
  double *z = hasZ ? mZ.data() : nullptr;
  double *m = hasM ? mM.data() : nullptr;
  for ( int i = 0; i < nPoints; ++i )
  {
    double xOut, yOut;
    t.map( *x, *y, &xOut, &yOut );
    *x++ = xOut;
    *y++ = yOut;
    if ( hasZ )
    {
      *z = *z * zScale + zTranslate;
      z++;
    }
    if ( hasM )
    {
      *m = *m * mScale + mTranslate;
      m++;
    }
  }
  clearCache();
}
