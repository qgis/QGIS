/***************************************************************************
                         qgssphere.cpp
                         --------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssphere.h"
#include "qgspoint.h"
#include "qgscircle.h"
#include "qgsbox3d.h"
#include "qgsvector3d.h"

QgsSphere::QgsSphere( double x, double y, double z, double radius )
  : mCenterX( x )
  , mCenterY( y )
  , mCenterZ( z )
  , mRadius( radius )
{

}

bool QgsSphere::isNull() const
{
  return std::isnan( mCenterX ) || std::isnan( mCenterY ) || std::isnan( mCenterZ );
}

bool QgsSphere::isEmpty() const
{
  return qgsDoubleNear( mRadius, 0 );
}

QgsPoint QgsSphere::center() const
{
  return QgsPoint( mCenterX, mCenterY, mCenterZ );
}

QgsVector3D QgsSphere::centerVector() const
{
  return QgsVector3D( mCenterX, mCenterY, mCenterZ );
}

void QgsSphere::setCenter( const QgsPoint &center )
{
  mCenterX = center.x();
  mCenterY = center.y();
  mCenterZ = center.z();
}

double QgsSphere::volume() const
{
  return 4.0 / 3.0 * M_PI * std::pow( mRadius, 3 );
}

double QgsSphere::surfaceArea() const
{
  return 4.0 * M_PI * std::pow( mRadius, 2 );
}

QgsCircle QgsSphere::toCircle() const
{
  if ( isNull() )
    return QgsCircle();

  return QgsCircle( QgsPoint( mCenterX, mCenterY ), mRadius );
}

QgsBox3D QgsSphere::boundingBox() const
{
  if ( isNull() )
    return QgsBox3D();

  return QgsBox3D( mCenterX - mRadius, mCenterY - mRadius, mCenterZ - mRadius,
                   mCenterX + mRadius, mCenterY + mRadius, mCenterZ + mRadius );
}

