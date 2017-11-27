/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Triangulation.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class Triangulation
{
%Docstring
 Interface for Triangulation classes*
%End

%TypeHeaderCode
#include "Triangulation.h"
%End
  public:
    enum ForcedCrossBehavior
    {
      SnappingTypeVertex,
      DeleteFirst,
      InsertVertex
    };
    virtual ~Triangulation();

    virtual void addLine( Line3D *line /Transfer/, bool breakline ) = 0;
%Docstring
 Adds a line (e.g. a break-, structure- or an isoline) to the triangulation.
 The class takes ownership of the line object and its points
%End

    virtual int addPoint( QgsPoint *p ) = 0;
%Docstring
 Adds a point to the triangulation
 Ownership is transferred to this class
 :rtype: int
%End

    virtual bool calcNormal( double x, double y, Vector3D *result /Out/ ) = 0;
%Docstring
 Calculates the normal at a point on the surface and assigns it to 'result'.
 :return: true in case of success and false in case of failure
 :rtype: bool
%End

    virtual void performConsistencyTest() = 0;
%Docstring
Performs a consistency check, remove this later
%End

    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ ) = 0;
%Docstring
 Calculates x-, y and z-value of the point on the surface and assigns it to 'result'.
 Returns true in case of success and flase in case of failure
 :rtype: bool
%End

    virtual QgsPoint *getPoint( unsigned int i ) const = 0;
%Docstring
Returns a pointer to the point with number i. Any virtual points must have the number -1
 :rtype: QgsPoint
%End

    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, int *n1 /Out/, QgsPoint *p2 /Out/, int *n2 /Out/, QgsPoint *p3 /Out/, int *n3 /Out/ ) = 0 /PyName=getTriangleVertices/;
%Docstring
 Finds out in which triangle the point with coordinates x and y is and
 assigns the numbers of the vertices to 'n1', 'n2' and 'n3' and the vertices to 'p1', 'p2' and 'p3'
 :rtype: bool
%End

    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, QgsPoint *p2 /Out/, QgsPoint *p3 /Out/ ) = 0;
%Docstring
Finds out, in which triangle the point with coordinates x and y is and assigns the  points at the vertices to 'p1', 'p2' and 'p3
 :rtype: bool
%End

    virtual int getOppositePoint( int p1, int p2 ) = 0;
%Docstring
Returns the number of the point opposite to the triangle points p1, p2 (which have to be on a halfedge)
 :rtype: int
%End

    virtual double getXMax() const = 0;
%Docstring
Returns the largest x-coordinate value of the bounding box
 :rtype: float
%End

    virtual double getXMin() const = 0;
%Docstring
Returns the smallest x-coordinate value of the bounding box
 :rtype: float
%End

    virtual double getYMax() const = 0;
%Docstring
Returns the largest y-coordinate value of the bounding box
 :rtype: float
%End

    virtual double getYMin() const = 0;
%Docstring
Returns the smallest x-coordinate value of the bounding box
 :rtype: float
%End

    virtual int getNumberOfPoints() const = 0;
%Docstring
Returns the number of points
 :rtype: int
%End

    virtual QList<int> *getSurroundingTriangles( int pointno ) = 0;
%Docstring
 Returns a pointer to a value list with the information of the triangles surrounding (counterclockwise) a point.
 Four integer values describe a triangle, the first three are the number of the half edges of the triangle
 and the fourth is -10, if the third (and most counterclockwise) edge is a breakline, and -20 otherwise.
 The value list has to be deleted by the code which called the method.
 Any virtual point needs to have the number -1
 :rtype: list of int
%End

    virtual QList<int> *getPointsAroundEdge( double x, double y ) = 0;
%Docstring
 Returns a value list with the numbers of the four points, which would be affected by an edge swap.
 This function is e.g. needed by NormVecDecorator to know the points,
 for which the normals have to be recalculated.
 The list has to be deleted by the code which calls this method
 :rtype: list of int
%End


    virtual void setForcedCrossBehavior( Triangulation::ForcedCrossBehavior b ) = 0;
%Docstring
Sets the behavior of the triangulation in case of crossing forced lines
%End

    virtual void setEdgeColor( int r, int g, int b ) = 0;
%Docstring
Sets the color of the normal edges
%End

    virtual void setForcedEdgeColor( int r, int g, int b ) = 0;
%Docstring
Sets the color of the forced edges
%End

    virtual void setBreakEdgeColor( int r, int g, int b ) = 0;
%Docstring
Sets the color of the breaklines
%End

    virtual void setTriangleInterpolator( TriangleInterpolator *interpolator ) = 0;
%Docstring
Sets an interpolator object
%End

    virtual void eliminateHorizontalTriangles() = 0;
%Docstring
Eliminates the horizontal triangles by swapping
%End

    virtual void ruppertRefinement() = 0;
%Docstring
Adds points to make the triangles better shaped (algorithm of ruppert)
%End

    virtual bool pointInside( double x, double y ) = 0;
%Docstring
Returns true, if the point with coordinates x and y is inside the convex hull and false otherwise
 :rtype: bool
%End



    virtual bool swapEdge( double x, double y ) = 0;
%Docstring
Swaps the edge which is closest to the point with x and y coordinates (if this is possible)
 :rtype: bool
%End

    virtual bool saveAsShapefile( const QString &fileName ) const = 0;
%Docstring
 Saves the triangulation as a (line) shapefile
 :return: true in case of success
 :rtype: bool
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/Triangulation.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
