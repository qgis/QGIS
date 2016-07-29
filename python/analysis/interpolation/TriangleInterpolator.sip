class TriangleInterpolator
{
%TypeHeaderCode
#include <TriangleInterpolator.h>
%End
  public:
    virtual ~TriangleInterpolator();
    /** Calculates the normal vector and assigns it to vec*/
    virtual bool calcNormVec( double x, double y, Vector3D* result ) = 0;
    /** Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point*/
    virtual bool calcPoint( double x, double y, Point3D* result ) = 0;
};
