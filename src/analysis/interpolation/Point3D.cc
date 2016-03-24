/***************************************************************************
                          Point3D.cc  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "Point3D.h"
#include "qgslogger.h"

Point3D& Point3D::operator=( const Point3D & p )
{
  mX = p.mX;
  mY = p.mY;
  mZ = p.mZ;
  return ( *this );
}

bool Point3D::operator==( const Point3D& p ) const
{
  return ( mX == p.getX() && mY == p.getY() && mZ == p.getZ() );
}

bool Point3D::operator!=( const Point3D& p ) const
{
  return ( !(( *this ) == p ) );
}

double Point3D::dist3D( Point3D* p ) const
{
  if ( p )
  {
    double dx, dy, dz;
    dx = p->getX() - getX();
    dy = p->getY() - getY();
    dz = p->getZ() - getZ();
    return sqrt( dx*dx + dy*dy + dz*dz );
  }
  else
  {
    QgsDebugMsg( "" );
    return 0;
  }
}
