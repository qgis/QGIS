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
#include "qgsvector3d.h"

QgsOrientedBox3D::QgsOrientedBox3D() = default;

QgsOrientedBox3D::QgsOrientedBox3D( const QList<double> &center, QList<double> &halfAxes )
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
