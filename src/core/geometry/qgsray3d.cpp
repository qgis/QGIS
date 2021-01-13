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

#include "qgsrange.h"

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

bool QgsRay3D::operator==( const QgsRay3D &r )
{
  return this->mOrigin == r.origin() && this->mDirection == r.direction();
}

QVector3D QgsRay3D::projectedPoint( const QVector3D &point ) const
{
  return mOrigin + QVector3D::dotProduct( point - mOrigin, mDirection ) * mDirection;
}

bool QgsRay3D::intersectsWith( const QgsBox3d &box ) const
{
  double tminX = box.xMinimum() - mOrigin.x(), tmaxX = box.xMaximum() - mOrigin.x();
  double tminY = box.yMinimum() - mOrigin.y(), tmaxY = box.yMaximum() - mOrigin.y();
  double tminZ = box.zMinimum() - mOrigin.z(), tmaxZ = box.zMaximum() - mOrigin.z();
  if ( mDirection.x() < 0 ) std::swap( tminX, tmaxX );
  if ( mDirection.y() < 0 ) std::swap( tminY, tmaxY );
  if ( mDirection.z() < 0 ) std::swap( tminZ, tmaxZ );
  if ( mDirection.x() != 0 )
  {
    tminX /= mDirection.x();
    tmaxX /= mDirection.x();
  }
  else
  {
    tminX = std::numeric_limits<double>::lowest();
    tmaxX = std::numeric_limits<double>::max();
  }
  if ( mDirection.y() != 0 )
  {
    tminY /= mDirection.y();
    tmaxY /= mDirection.y();
  }
  else
  {
    tminY = std::numeric_limits<double>::lowest();
    tmaxY = std::numeric_limits<double>::max();
  }
  if ( mDirection.z() != 0 )
  {
    tminZ /= mDirection.z();
    tmaxZ /= mDirection.z();
  }
  else
  {
    tminZ = std::numeric_limits<double>::lowest();
    tmaxZ = std::numeric_limits<double>::max();
  }
  QgsDoubleRange tRange( std::max( std::max( tminX, tminY ), tminZ ), std::min( std::min( tmaxX, tmaxY ), tmaxZ ) );
  return !tRange.isEmpty();
}

bool QgsRay3D::isInFront( const QVector3D &point ) const
{
  return QVector3D::dotProduct( ( point - mOrigin ).normalized(), mDirection ) >= 0.0;
}

double QgsRay3D::angleToPoint( const QVector3D &point ) const
{
  // project point onto the ray
  QVector3D projPoint = projectedPoint( point );

  // calculate the angle between the point and the projected point
  QVector3D v1 = projPoint - mOrigin ;
  QVector3D v2 = point - projPoint;
  return qRadiansToDegrees( std::atan2( v2.length(), v1.length() ) );
}
