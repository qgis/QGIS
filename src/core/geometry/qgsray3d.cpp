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
