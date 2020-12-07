/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Vector3D.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class Vector3D
{
%Docstring
 Class Vector3D represents a 3D-Vector, capable to store x-,y- and
 z-coordinates in double values. In fact, the class is the same as QgsPoint.
 The name 'vector' makes it easier to understand the programs.
%End

%TypeHeaderCode
#include "Vector3D.h"
%End
  protected:

  public:
    Vector3D( double x, double y, double z );
%Docstring
Constructor taking the three components as arguments
%End
    Vector3D();
%Docstring
Default constructor
%End

    bool operator==( const Vector3D &v ) const;
    bool operator!=( const Vector3D &v ) const;
%Docstring
 :rtype: bool
%End
    double getX() const;
%Docstring
Returns the x-component of the vector
 :rtype: float
%End
    double getY() const;
%Docstring
Returns the y-component of the vector
 :rtype: float
%End
    double getZ() const;
%Docstring
Returns the z-component of the vector
 :rtype: float
%End
    double getLength() const;
%Docstring
Returns the length of the vector
 :rtype: float
%End
    void setX( double x );
%Docstring
Sets the x-component of the vector
%End
    void setY( double y );
%Docstring
Sets the y-component of the vector
%End
    void setZ( double z );
%Docstring
Sets the z-component of the vector
%End
    void standardise();
%Docstring
Standardises the vector
%End

  private:
    Vector3D( const Vector3D &v );
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Vector3D.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
