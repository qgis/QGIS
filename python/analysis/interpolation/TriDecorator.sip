/** Decorator class for Triangulations (s. Decorator pattern in Gamma et al.)*/
class TriDecorator : Triangulation
{
%TypeHeaderCode
#include <TriDecorator.h>
%End

  public:
    TriDecorator();
    explicit TriDecorator( Triangulation* t );
    virtual ~TriDecorator();
    virtual void addLine( Line3D* line, bool breakline );
    virtual int addPoint( Point3D* p );
    /** Adds an association to a triangulation*/
    virtual void addTriangulation( Triangulation* t );
    /** Performs a consistency check, remove this later*/
    virtual void performConsistencyTest();
    virtual bool calcNormal( double x, double y, Vector3D* result );
    virtual bool calcPoint( double x, double y, Point3D* result );
    virtual Point3D* getPoint( unsigned int i ) const;
    virtual int getNumberOfPoints() const;
    bool getTriangle( double x, double y, Point3D* p1, int* n1 /Out/, Point3D* p2 /Out/, int* n2 /Out/, Point3D* p3 /Out/, int* n3 /Out/ );
    bool getTriangle( double x, double y, Point3D* p1 /Out/, Point3D* p2 /Out/, Point3D* p3 /Out/ );
    virtual int getOppositePoint( int p1, int p2 );
    virtual QList<int>* getSurroundingTriangles( int pointno );
    virtual double getXMax() const;
    virtual double getXMin() const;
    virtual double getYMax() const;
    virtual double getYMin() const;
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b );
    virtual void setEdgeColor( int r, int g, int b );
    virtual void setForcedEdgeColor( int r, int g, int b );
    virtual void setBreakEdgeColor( int r, int g, int b );
    virtual void setTriangleInterpolator( TriangleInterpolator* interpolator );
    virtual void eliminateHorizontalTriangles();
    virtual void ruppertRefinement();
    virtual bool pointInside( double x, double y );
    virtual bool swapEdge( double x, double y );
    virtual QList<int>* getPointsAroundEdge( double x, double y );
};

