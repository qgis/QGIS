/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/CloughTocherInterpolator.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class CloughTocherInterpolator : TriangleInterpolator
{
%Docstring
 This is an implementation of a Clough-Tocher interpolator based on a triangular tessellation. The derivatives orthogonal to the boundary curves are interpolated linearly along a triangle edge.*
%End

%TypeHeaderCode
#include "CloughTocherInterpolator.h"
%End
  protected:
    void init( double x, double y );
%Docstring
Finds out, in which triangle the point with the coordinates x and y is
%End
    double calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w );
%Docstring
Calculates the Bernsteinpolynomials to calculate the Beziertriangle. 'n' is three in the cubical case, 'i', 'j', 'k' are the indices of the controllpoint and 'u', 'v', 'w' are the barycentric coordinates of the point
 :rtype: float
%End

  public:
    CloughTocherInterpolator();
%Docstring
Standard constructor
%End
    CloughTocherInterpolator( NormVecDecorator *tin );
%Docstring
Constructor with a pointer to the triangulation as argument
%End
    virtual ~CloughTocherInterpolator();
    virtual bool calcNormVec( double x, double y, Vector3D *result /Out/ );
%Docstring
Calculates the normal vector and assigns it to vec (not implemented at the moment)
 :rtype: bool
%End
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ );
%Docstring
Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point
 :rtype: bool
%End
    virtual void setTriangulation( NormVecDecorator *tin );
};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/CloughTocherInterpolator.h                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
