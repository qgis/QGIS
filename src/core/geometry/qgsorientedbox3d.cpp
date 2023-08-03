/***************************************************************************
                         qgsorientedbox3d.cpp
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsorientedbox3d.h"
#include "qgsbox3d.h"
#include "qgscoordinatetransform.h"
#include "qgsmatrix4x4.h"
#include "qgsvector3d.h"

QgsOrientedBox3D::QgsOrientedBox3D() = default;

QgsOrientedBox3D::QgsOrientedBox3D( const QList<double> &center, const QList<double> &halfAxes )
{
  if ( center.size() == 3 )
  {
    mCenter[0] = center.at( 0 );
    mCenter[1] = center.at( 1 );
    mCenter[2] = center.at( 2 );
  }
  if ( halfAxes.size() == 9 )
  {
    for ( int i = 0; i < 9; ++i )
    {
      mHalfAxes[i] = halfAxes.at( i );
    }
  }
}

QgsOrientedBox3D::QgsOrientedBox3D( const QgsVector3D &center, const QList<QgsVector3D> &halfAxes )
{
  mCenter[0] = center.x();
  mCenter[1] = center.y();
  mCenter[2] = center.z();
  if ( halfAxes.size() == 3 )
  {
    for ( int i = 0; i < 3; ++i )
    {
      mHalfAxes[static_cast< int >( i * 3 )] = halfAxes.at( i ).x();
      mHalfAxes[static_cast< int >( i * 3 + 1 )] = halfAxes.at( i ).y();
      mHalfAxes[static_cast< int >( i * 3 + 2 )] = halfAxes.at( i ).z();
    }
  }
}

QgsOrientedBox3D QgsOrientedBox3D::fromBox3D( const QgsBox3D &box )
{
  return QgsOrientedBox3D( box.center(), QList< QgsVector3D >
  {
    QgsVector3D( box.width() * 0.5, 0, 0 ),
    QgsVector3D( 0, box.height() * 0.5, 0 ),
    QgsVector3D( 0, 0, box.depth() * 0.5 )
  } );
}

bool QgsOrientedBox3D::isNull() const
{
  return std::isnan( mCenter[0] ) || std::isnan( mCenter[1] ) || std::isnan( mCenter[2] );
}

QList< double > QgsOrientedBox3D::halfAxesList() const
{
  QList< double > res;
  res.reserve( 9 );
  for ( int i = 0; i < 9; ++i )
  {
    res.append( mHalfAxes[i] );
  }
  return res;
}

QgsBox3D QgsOrientedBox3D::extent() const
{
  const double extent[3]
  {
    std::fabs( mHalfAxes[0] ) + std::fabs( mHalfAxes[3] ) + std::fabs( mHalfAxes[6] ),
    std::fabs( mHalfAxes[1] ) + std::fabs( mHalfAxes[4] ) + std::fabs( mHalfAxes[7] ),
    std::fabs( mHalfAxes[2] ) + std::fabs( mHalfAxes[5] ) + std::fabs( mHalfAxes[8] ),
  };

  const double minX = mCenter[0] - extent[0];
  const double maxX = mCenter[0] + extent[0];
  const double minY = mCenter[1] - extent[1];
  const double maxY = mCenter[1] + extent[1];
  const double minZ = mCenter[2] - extent[2];
  const double maxZ = mCenter[2] + extent[2];

  return QgsBox3D( minX, minY, minZ, maxX, maxY, maxZ );
}

QVector<QgsVector3D> QgsOrientedBox3D::corners() const
{
  const QgsVector3D center( mCenter[0], mCenter[1], mCenter[2] );
  const QgsVector3D a1( mHalfAxes[0], mHalfAxes[1], mHalfAxes[2] ), a0( -mHalfAxes[0], -mHalfAxes[1], -mHalfAxes[2] );
  const QgsVector3D b1( mHalfAxes[3], mHalfAxes[4], mHalfAxes[5] ), b0( -mHalfAxes[3], -mHalfAxes[4], -mHalfAxes[5] );
  const QgsVector3D c1( mHalfAxes[6], mHalfAxes[7], mHalfAxes[8] ), c0( -mHalfAxes[6], -mHalfAxes[7], -mHalfAxes[8] );

  QVector<QgsVector3D> cor( 8 );
  QgsVector3D *corData = cor.data();
  for ( int i = 0; i < 8; ++i, ++corData )
  {
    const QgsVector3D aa = ( i % 2 == 0 ? a1 : a0 );
    const QgsVector3D bb = ( ( i / 2 ) % 2 == 0 ? b1 : b0 );
    const QgsVector3D cc = ( i / 4 == 0 ? c1 : c0 );
    const QgsVector3D q = aa + bb + cc;
    *corData = center + q;
  }
  return cor;
}

QgsVector3D QgsOrientedBox3D::size() const
{
  QgsVector3D axis1( mHalfAxes[0], mHalfAxes[1], mHalfAxes[2] );
  QgsVector3D axis2( mHalfAxes[3], mHalfAxes[4], mHalfAxes[5] );
  QgsVector3D axis3( mHalfAxes[6], mHalfAxes[7], mHalfAxes[8] );
  return QgsVector3D( 2 * axis1.length(), 2 * axis2.length(), 2 * axis3.length() );
}

QgsBox3D QgsOrientedBox3D::reprojectedExtent( const QgsCoordinateTransform &ct ) const
{
  // reproject corners to destination CRS
  QVector<QgsVector3D> c = corners();
  Q_ASSERT( c.count() == 8 );
  for ( int i = 0; i < 8; ++i )
  {
    c[i] = ct.transform( c[i] );
  }

  // find AABB for the 8 transformed points
  QgsVector3D v0 = c[0], v1 = c[0];
  for ( const QgsVector3D &v : std::as_const( c ) )
  {
    if ( v.x() < v0.x() ) v0.setX( v.x() );
    if ( v.y() < v0.y() ) v0.setY( v.y() );
    if ( v.z() < v0.z() ) v0.setZ( v.z() );
    if ( v.x() > v1.x() ) v1.setX( v.x() );
    if ( v.y() > v1.y() ) v1.setY( v.y() );
    if ( v.z() > v1.z() ) v1.setZ( v.z() );
  }
  return QgsBox3D( v0.x(), v0.y(), v0.z(), v1.x(), v1.y(), v1.z() );
}

QgsOrientedBox3D QgsOrientedBox3D::transformed( const QgsMatrix4x4 &transform ) const
{
  const double *ptr = transform.constData();
  const QgsMatrix4x4 mm( ptr[0], ptr[4], ptr[8], 0,
                         ptr[1], ptr[5], ptr[9], 0,
                         ptr[2], ptr[6], ptr[10], 0,
                         0, 0, 0, 1 );

  const QgsVector3D trCenter = transform.map( QgsVector3D( mCenter[0], mCenter[1], mCenter[2] ) );

  const QgsVector3D col1 = mm.map( QgsVector3D( mHalfAxes[0], mHalfAxes[1], mHalfAxes[2] ) );
  const QgsVector3D col2 = mm.map( QgsVector3D( mHalfAxes[3], mHalfAxes[4], mHalfAxes[5] ) );
  const QgsVector3D col3 = mm.map( QgsVector3D( mHalfAxes[6], mHalfAxes[7], mHalfAxes[8] ) );

  return QgsOrientedBox3D( QList<double>() << trCenter.x() << trCenter.y() << trCenter.z(),
                           QList<double>() << col1.x() << col1.y() << col1.z()
                           << col2.x() << col2.y() << col2.z()
                           << col3.x() << col3.y() << col3.z() );
}

bool QgsOrientedBox3D::intersects( const QgsOrientedBox3D &other ) const
{
  // use the Separating Axis Theorem (SAT) for OBB (Oriented Bounding Box) collision detection.
  // based off section 5 in OBBTree: A Hierarchical Structure for Rapid Interference Detection (1996)

  const QgsVector3D thisCenter = center();
  const QgsVector3D thisHalfAxis[3]
  {
    { mHalfAxes[0], mHalfAxes[1], mHalfAxes[2] },
    { mHalfAxes[3], mHalfAxes[4], mHalfAxes[5] },
    { mHalfAxes[6], mHalfAxes[7], mHalfAxes[8] }
  };
  const QgsVector3D otherCenter = other.center();
  const QgsVector3D otherHalfAxis[3]
  {
    { other.mHalfAxes[0], other.mHalfAxes[1], other.mHalfAxes[2] },
    { other.mHalfAxes[3], other.mHalfAxes[4], other.mHalfAxes[5] },
    { other.mHalfAxes[6], other.mHalfAxes[7], other.mHalfAxes[8] }
  };

  for ( int a = 0; a < 3; ++a )
  {
    const QgsVector3D *aAxis = thisHalfAxis + a;
    for ( int b = 0; b < 3; ++b )
    {
      const QgsVector3D *bAxis = otherHalfAxis + b;
      QgsVector3D lAxis = QgsVector3D::crossProduct( *aAxis, *bAxis );
      if ( lAxis.isNull() )
        continue;

      lAxis.normalize();

      const double tl = std::abs( QgsVector3D::dotProduct( lAxis, otherCenter ) - QgsVector3D::dotProduct( lAxis, thisCenter ) );
      const double ra = std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[2] ) );
      const double rb = std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[2] ) );
      const double penetration = ( ra + rb ) - tl;
      if ( penetration <= 0 )
        return false;
    }
  }

  for ( int a = 0; a < 3; ++a )
  {
    QgsVector3D lAxis = *( thisHalfAxis + a );
    lAxis.normalize();

    const double tl = std::abs( QgsVector3D::dotProduct( lAxis, otherCenter ) - QgsVector3D::dotProduct( lAxis, thisCenter ) );
    const double ra = std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[2] ) );
    const double rb = std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[2] ) );
    const double penetration = ( ra + rb ) - tl;
    if ( penetration <= 0 )
      return false;
  }

  for ( int b = 0; b < 3; ++b )
  {
    QgsVector3D lAxis = *( otherHalfAxis + b );
    lAxis.normalize();

    const double tl = std::abs( QgsVector3D::dotProduct( lAxis, otherCenter ) - QgsVector3D::dotProduct( lAxis, thisCenter ) );
    const double ra = std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, thisHalfAxis[2] ) );
    const double rb = std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[0] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[1] ) ) + std::abs( QgsVector3D::dotProduct( lAxis, otherHalfAxis[2] ) );
    const double penetration = ( ra + rb ) - tl;
    if ( penetration <= 0 )
      return false;
  }

  return true;
}
