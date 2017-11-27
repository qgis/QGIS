/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/TriDecorator.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class TriDecorator : Triangulation
{
%Docstring
 Decorator class for Triangulations (s. Decorator pattern in Gamma et al.)*
%End

%TypeHeaderCode
#include "TriDecorator.h"
%End
  public:
    TriDecorator();
    explicit TriDecorator( Triangulation *t );
    virtual ~TriDecorator();
    virtual void addLine( Line3D *line, bool breakline );
    virtual int addPoint( QgsPoint *p );
    virtual void addTriangulation( Triangulation *t );
%Docstring
Adds an association to a triangulation
%End
    virtual void performConsistencyTest();
%Docstring
Performs a consistency check, remove this later
%End
    virtual bool calcNormal( double x, double y, Vector3D *result /Out/ );
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ );
    virtual QgsPoint *getPoint( unsigned int i ) const;
    virtual int getNumberOfPoints() const;
    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, int *n1 /Out/, QgsPoint *p2 /Out/, int *n2 /Out/, QgsPoint *p3 /Out/, int *n3 /Out/ )  /PyName=getTriangleVertices/;

    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, QgsPoint *p2 /Out/, QgsPoint *p3 /Out/ );

    virtual int getOppositePoint( int p1, int p2 );
    virtual QList<int> *getSurroundingTriangles( int pointno );
    virtual double getXMax() const;
    virtual double getXMin() const;
    virtual double getYMax() const;
    virtual double getYMin() const;
    virtual void setForcedCrossBehavior( Triangulation::ForcedCrossBehavior b );
    virtual void setEdgeColor( int r, int g, int b );
    virtual void setForcedEdgeColor( int r, int g, int b );
    virtual void setBreakEdgeColor( int r, int g, int b );
    virtual void setTriangleInterpolator( TriangleInterpolator *interpolator );
    virtual void eliminateHorizontalTriangles();
    virtual void ruppertRefinement();
    virtual bool pointInside( double x, double y );
    virtual bool swapEdge( double x, double y );
    virtual QList<int> *getPointsAroundEdge( double x, double y );
  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/TriDecorator.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
