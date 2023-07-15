/***************************************************************************
                         qgsorientedboundingbox.cpp
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

#include "qgsorientedboundingbox.h"
#include "qgsbox3d.h"

QgsOrientedBoundingBox::QgsOrientedBoundingBox() = default;

QgsOrientedBoundingBox::QgsOrientedBoundingBox( const QList<double> &center, QList<double> &halfAxes )
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

bool QgsOrientedBoundingBox::isNull() const
{
  return std::isnan( mCenter[0] ) || std::isnan( mCenter[1] ) || std::isnan( mCenter[2] );
}

QList< double > QgsOrientedBoundingBox::halfAxesList() const
{
  QList< double > res;
  res.reserve( 9 );
  for ( int i = 0; i < 9; ++i )
  {
    res.append( mHalfAxes[i] );
  }
  return res;
}

QgsBox3d QgsOrientedBoundingBox::extent() const
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

  return QgsBox3d( minX, minY, minZ, maxX, maxY, maxZ );
}
