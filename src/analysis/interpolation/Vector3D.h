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

#include <cmath>
#include "qgis_analysis.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * Class Vector3D represents a 3D-Vector, capable to store x-,y- and
 * z-coordinates in double values. In fact, the class is the same as QgsPoint.
 * The name 'vector' makes it easier to understand the programs.
 * \note Not available in Python bindings
 */

class ANALYSIS_EXPORT Vector3D
{
  protected:
    //! X-component of the vector
    double mX = 0;
    //! Y-component of the vector
    double mY = 0;
    //! Z-component of the vector
    double mZ = 0;

  public:
    //! Constructor taking the three components as arguments
    Vector3D( double x, double y, double z );
    //! Default constructor
    Vector3D() = default;

    bool operator==( const Vector3D &v ) const;
    bool operator!=( const Vector3D &v ) const;
    //! Returns the x-component of the vector
    double getX() const;
    //! Returns the y-component of the vector
    double getY() const;
    //! Returns the z-component of the vector
    double getZ() const;
    //! Returns the length of the vector
    double getLength() const;
    //! Sets the x-component of the vector
    void setX( double x );
    //! Sets the y-component of the vector
    void setY( double y );
    //! Sets the z-component of the vector
    void setZ( double z );
    //! Standardises the vector
    void standardise();

  private:
#ifdef SIP_RUN
    Vector3D( const Vector3D &v );
#endif
};

#ifndef SIP_RUN

//------------------------------------------constructors------------------------------------

inline Vector3D::Vector3D( double x, double y, double z )
  : mX( x )
  , mY( y )
  , mZ( z )
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
#endif
