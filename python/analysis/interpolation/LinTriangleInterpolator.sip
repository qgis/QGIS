/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/LinTriangleInterpolator.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class LinTriangleInterpolator : TriangleInterpolator
{
%Docstring
 LinTriangleInterpolator is a class which interpolates linearly on a triangulation*
%End

%TypeHeaderCode
#include "LinTriangleInterpolator.h"
%End
  public:
    LinTriangleInterpolator();
%Docstring
Default constructor
%End
    LinTriangleInterpolator( DualEdgeTriangulation *tin );
%Docstring
Constructor with reference to a DualEdgeTriangulation object
%End
    virtual ~LinTriangleInterpolator();
    virtual bool calcNormVec( double x, double y, Vector3D *result /Out/ );
%Docstring
Calculates the normal vector and assigns it to vec
 :rtype: bool
%End
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ );
%Docstring
Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point
 :rtype: bool
%End
    virtual DualEdgeTriangulation *getTriangulation() const;
%Docstring
Returns a pointer to the current Triangulation object
 :rtype: DualEdgeTriangulation
%End
    virtual void setTriangulation( DualEdgeTriangulation *tin );
%Docstring
Sets a Triangulation
%End


  protected:
    virtual bool calcFirstDerX( double x, double y, Vector3D *result /Out/ );
%Docstring
Calculates the first derivative with respect to x for a linear surface and assigns it to vec
 :rtype: bool
%End
    virtual bool calcFirstDerY( double x, double y, Vector3D *result /Out/ );
%Docstring
Calculates the first derivative with respect to y for a linear surface and assigns it to vec
 :rtype: bool
%End
};








/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/LinTriangleInterpolator.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
