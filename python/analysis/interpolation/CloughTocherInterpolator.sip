class CloughTocherInterpolator : TriangleInterpolator
{
%TypeHeaderCode
#include <CloughTocherInterpolator.h>
%End
  protected:
    /** Finds out, in which triangle the point with the coordinates x and y is*/
    void init( double x, double y );
    /** Calculates the Bernsteinpolynomials to calculate the Beziertriangle. 'n' is three in the cubical case, 'i', 'j', 'k' are the indices of the controllpoint and 'u', 'v', 'w' are the barycentric coordinates of the point*/
    double calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w );

  public:
    /** Standard constructor*/
    CloughTocherInterpolator();
    /** Constructor with a pointer to the triangulation as argument*/
    CloughTocherInterpolator( NormVecDecorator* tin );
    /** Destructor*/
    virtual ~CloughTocherInterpolator();
    /** Calculates the normal vector and assigns it to vec (not implemented at the moment)*/
    virtual bool calcNormVec( double x, double y, Vector3D* result );
    /** Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point*/
    virtual bool calcPoint( double x, double y, Point3D* result );
    virtual void setTriangulation( NormVecDecorator* tin );
};
