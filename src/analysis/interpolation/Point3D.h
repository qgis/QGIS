/***************************************************************************
                          Point3D.h  -  description
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

#ifndef POINT3D_H
#define POINT3D_H

#include <cmath>

/**Point3D is a class to represent a three dimensional point*/
class ANALYSIS_EXPORT Point3D
{
  protected:
    /**X-coordinate*/
    double mX;
    /**Y-coordinate*/
    double mY;
    /**Z-coordinate*/
    double mZ;
  public:
    Point3D();
    /**Constructor with the x-, y- and z-coordinate as arguments*/
    Point3D( double x, double y, double z );
    Point3D( const Point3D& p );
    ~Point3D();
    Point3D& operator=( const Point3D& p );
    bool operator==( const Point3D& p );
    bool operator!=( const Point3D& p );
    /**calculates the three-dimensional distance to another point*/
    double dist3D( Point3D* p ) const;
    /**Returns the x-coordinate of the point*/
    double getX() const;
    /**Returns the y-coordinate of the point*/
    double getY() const;
    /**Returns the z-coordinate of the point*/
    double getZ() const;
    /**Sets the x-coordinate of the point*/
    void setX( double x );
    /**Sets the y-coordinate of the point*/
    void setY( double y );
    /**Sets the z-coordinate of the point*/
    void setZ( double z );
};

inline Point3D::Point3D() : mX( 0 ), mY( 0 ), mZ( 0 )
{

}

inline Point3D::Point3D( double x, double y, double z ) : mX( x ), mY( y ), mZ( z )
{

}

inline Point3D::Point3D( const Point3D& p ): mX( p.mX ), mY( p.mY ), mZ( p.mZ )
{

}

inline Point3D::~Point3D()
{

}

inline double Point3D::getX() const
{
  return mX;
}

inline double Point3D::getY() const
{
  return mY;
}

inline double Point3D::getZ() const
{
  return mZ;
}

inline void Point3D::setX( double x )
{
  mX = x;
}

inline void Point3D::setY( double y )
{
  mY = y;
}

inline void Point3D::setZ( double z )
{
  mZ = z;
}

#endif
