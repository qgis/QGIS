/***************************************************************************
                             qgsbox3d.cpp
                             ------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsbox3d.h"


QgsBox3d::QgsBox3d( double xmin, double ymin, double zmin, double xmax, double ymax, double zmax )
  : mBounds2d( xmin, ymin, xmax, ymax )
  , mZmin( zmin )
  , mZmax( zmax )
{}

QgsBox3d::QgsBox3d( const QgsPoint &p1, const QgsPoint &p2 )
  : mBounds2d( p1.x(), p1.y(), p2.x(), p2.y() )
  , mZmin( std::min( p1.z(), p2.z() ) )
  , mZmax( std::max( p1.z(), p2.z() ) )
{
  mBounds2d.normalize();
}

void QgsBox3d::setXMinimum( double x )
{
  mBounds2d.setXMinimum( x );
}

void QgsBox3d::setXMaximum( double x )
{
  mBounds2d.setXMaximum( x );
}

void QgsBox3d::setYMinimum( double y )
{
  mBounds2d.setYMinimum( y );
}

void QgsBox3d::setYMaximum( double y )
{
  mBounds2d.setYMaximum( y );
}

void QgsBox3d::setZMinimum( double z )
{
  mZmin = z;
}

void QgsBox3d::setZMaximum( double z )
{
  mZmax = z;
}

void QgsBox3d::normalize()
{
  mBounds2d.normalize();
  double z1 = std::min( mZmin, mZmax );
  double z2 = std::max( mZmin, mZmax );
  mZmin = z1;
  mZmax = z2;
}

QgsBox3d QgsBox3d::intersect( const QgsBox3d &other ) const
{
  QgsRectangle intersect2d = mBounds2d.intersect( &( other.mBounds2d ) );
  double zMin = std::max( mZmin, other.mZmin );
  double zMax = std::min( mZmax, other.mZmax );
  return QgsBox3d( intersect2d.xMinimum(), intersect2d.yMinimum(), zMin,
                   intersect2d.xMaximum(), intersect2d.yMaximum(), zMax );
}

bool QgsBox3d::is2d() const
{
  return qgsDoubleNear( mZmin, mZmax ) || ( mZmin > mZmax );
}

bool QgsBox3d::intersects( const QgsBox3d &other ) const
{
  if ( !mBounds2d.intersects( other.mBounds2d ) )
    return false;

  double z1 = ( mZmin > other.mZmin ? mZmin : other.mZmin );
  double z2 = ( mZmax < other.mZmax ? mZmax : other.mZmax );
  return z1 <= z2;
}

bool QgsBox3d::contains( const QgsBox3d &other ) const
{
  if ( !mBounds2d.contains( other.mBounds2d ) )
    return false;

  return ( other.mZmin >= mZmin && other.mZmax <= mZmax );
}

bool QgsBox3d::contains( const QgsPoint &p ) const
{
  if ( !mBounds2d.contains( QgsPointXY( p.x(), p.y() ) ) )
    return false;

  if ( p.is3D() )
    return mZmin <= p.z() && p.z() <= mZmax;
  else
    return true;
}
