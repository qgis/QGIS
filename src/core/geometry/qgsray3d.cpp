/***************************************************************************
  qgsray3d.cpp
  --------------------------------------
  Date                 : January 2021
  Copyright            : (C) 2021 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsray3d.h"

#include <QtMath>

QgsRay3D::QgsRay3D( const QVector3D &origin, const QVector3D &direction )
  : mOrigin( origin )
  , mDirection( direction.normalized() )
{

}

void QgsRay3D::setOrigin( const QVector3D &origin )
{
  mOrigin = origin;
}

void QgsRay3D::setDirection( const QVector3D direction )
{
  mDirection = direction.normalized();
}

QVector3D QgsRay3D::projectedPoint( const QVector3D &point ) const
{
  return mOrigin + QVector3D::dotProduct( point - mOrigin, mDirection ) * mDirection;
}

bool QgsRay3D::isInFront( const QVector3D &point ) const
{
  return QVector3D::dotProduct( ( point - mOrigin ).normalized(), mDirection ) >= 0.0;
}

double QgsRay3D::angleToPoint( const QVector3D &point ) const
{
  // project point onto the ray
  const QVector3D projPoint = projectedPoint( point );

  // calculate the angle between the point and the projected point
  const QVector3D v1 = projPoint - mOrigin ;
  const QVector3D v2 = point - projPoint;
  return qRadiansToDegrees( std::atan2( v2.length(), v1.length() ) );
}

bool QgsRay3D::intersects( const QgsBox3d &box ) const
{
  const float invX = 1 / mDirection.x();
  const float invY = 1 / mDirection.y();
  const float invZ = 1 / mDirection.z();
  const float xOffset = box.width() > 0 ? 0 : 0.1;
  const float yOffset = box.height() > 0 ? 0 : 0.1;
  const float zOffset = box.depth() > 0 ? 0 : 0.1;

  double t1 = ( box.xMinimum() - mOrigin.x() ) * invX;
  double t2 = ( box.xMaximum() - mOrigin.x() + xOffset ) * invX;

  double tmin = std::min( t1, t2 );
  double tmax = std::max( t1, t2 );

  t1 = ( box.yMinimum() - mOrigin.y() ) * invY;
  t2 = ( box.yMaximum() - mOrigin.y() + yOffset ) * invY;

  tmin = std::max( tmin, std::min( std::min( t1, t2 ), tmax ) );
  tmax = std::min( tmax, std::max( std::max( t1, t2 ), tmin ) );

  t1 = ( box.zMinimum() - mOrigin.z() ) * invZ;
  t2 = ( box.zMaximum() - mOrigin.z() + zOffset ) * invZ;

  tmin = std::max( tmin, std::min( std::min( t1, t2 ), tmax ) );
  tmax = std::min( tmax, std::max( std::max( t1, t2 ), tmin ) );

  return tmax > std::max( tmin, 0.0 );
}
