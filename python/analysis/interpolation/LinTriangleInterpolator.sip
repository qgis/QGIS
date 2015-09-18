class LinTriangleInterpolator : TriangleInterpolator
{
%TypeHeaderCode
#include <LinTriangleInterpolator.h>
%End
  public:
    /** Default constructor*/
    LinTriangleInterpolator();
    /** Constructor with reference to a DualEdgeTriangulation object*/
    LinTriangleInterpolator( DualEdgeTriangulation* tin );
    /** Destructor*/
    virtual ~LinTriangleInterpolator();
    /** Calculates the normal vector and assigns it to vec*/
    virtual bool calcNormVec( double x, double y, Vector3D* result );
    /** Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point*/
    virtual bool calcPoint( double x, double y, Point3D* result );
    /** Returns a pointer to the current Triangulation object*/
    virtual DualEdgeTriangulation* getTriangulation() const;
    /** Sets a Triangulation*/
    virtual void setTriangulation( DualEdgeTriangulation* tin );


  protected:
    /** Calculates the first derivative with respect to x for a linear surface and assigns it to vec*/
    virtual bool calcFirstDerX( double x, double y, Vector3D* result );
    /** Calculates the first derivative with respect to y for a linear surface and assigns it to vec*/
    virtual bool calcFirstDerY( double x, double y, Vector3D* result );
};
