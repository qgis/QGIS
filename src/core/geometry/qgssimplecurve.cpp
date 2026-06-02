/***************************************************************************
                       qgssimplecurve.h
                         ------------
    begin                : May 2026
    copyright            : (C) 2026 by Germán Carrillo
    email                : german at opengis dot ch
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
