/***************************************************************************
                          Vector3D.cc  -  description
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

#include "Vector3D.h"

double Vector3D::getLength() const
{
  return sqrt( getX()*getX() + getY()*getY() + getZ()*getZ() );
}

void Vector3D::standardise()
{
  double length = getLength();
  setX( getX() / length );
  setY( getY() / length );
  setZ( getZ() / length );
}

Vector3D::Vector3D( const Vector3D& v )
    : mX( v.mX )
    , mY( v.mY )
    , mZ( v.mZ )
{
}

Vector3D& Vector3D::operator=( const Vector3D & v )
{
  mX = v.mX;
  mY = v.mY;
  mZ = v.mZ;
  return ( *this );
}

bool Vector3D::operator==( const Vector3D& v ) const
{
  return ( mX == v.getX() && mY == v.getY() && mZ == v.getZ() );
}

bool Vector3D::operator!=( const Vector3D& v ) const
{
  return ( !(( *this ) == v ) );
}
