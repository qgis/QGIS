/***************************************************************************
                          Vector3D.h  -  description
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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif

class Vector3D
    /**
      Class Vector3D represents a 3D-Vector, capable to store x-,y- and z-coordinates in double values. In fact, the class is the same as Point3D. The name 'vector' makes it easier to understand the programms.
      */

{
  protected:
    /**X-component of the vector*/
    double mX;
    /**Y-component of the vector*/
    double mY;
    /**Z-component of the vector*/
    double mZ;

  public:
    /**Constructor taking the three components as arguments*/
    Vector3D( double x, double y, double z );
    /**Default constructor*/
    Vector3D();
    /**Copy constructor*/
    Vector3D( const Vector3D& v );
    /**Destructor*/
    ~Vector3D();
    Vector3D& operator=( const Vector3D& v );
    bool operator==( const Vector3D& v );
    bool operator!=( const Vector3D& v );
    /**Returns the x-component of the vector*/
    double getX() const;
    /**Returns the y-component of the vector*/
    double getY() const;
    /**Returns the z-component of the vector*/
    double getZ() const;
    /**Returns the length of the vector*/
    double getLength() const;
    /**Sets the x-component of the vector*/
    void setX( double x );
    /**Sets the y-component of the vector*/
    void setY( double y );
    /**Sets the z-component of the vector*/
    void setZ( double z );
    /**Standardises the vector*/
    void standardise();
};

//------------------------------------------constructors------------------------------------

inline Vector3D::Vector3D( double x, double y, double z ) : mX( x ), mY( y ), mZ( z )
{

}

inline Vector3D::Vector3D() : mX( 0 ), mY( 0 ), mZ( 0 )//using a list
{

}

inline Vector3D::~Vector3D()
{

}

//-------------------------------------------setter and getters-------------------------------

inline double Vector3D::getX() const
{
  return mX;
}

inline double Vector3D::getY() const
{
  return mY;
}

inline double Vector3D::getZ() const
{
  return mZ;
}

inline void Vector3D::setX( double x )
{
  mX = x;
}

inline void Vector3D::setY( double y )
{
  mY = y;
}

inline void Vector3D::setZ( double z )
{
  mZ = z;
}

#endif


































