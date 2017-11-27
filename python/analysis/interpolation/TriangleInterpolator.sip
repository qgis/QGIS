/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/TriangleInterpolator.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class TriangleInterpolator
{
%Docstring
 This is an interface for interpolator classes for triangulations*
%End

%TypeHeaderCode
#include "TriangleInterpolator.h"
%End
  public:
    virtual ~TriangleInterpolator();
    virtual bool calcNormVec( double x, double y, Vector3D *result /Out/ ) = 0;
%Docstring
Calculates the normal vector and assigns it to vec
 :rtype: bool
%End
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ ) = 0;
%Docstring
Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point
 :rtype: bool
%End
};









/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/TriangleInterpolator.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
