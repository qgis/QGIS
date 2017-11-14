/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/DualEdgeTriangulation.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class DualEdgeTriangulation: Triangulation
{
%Docstring
 DualEdgeTriangulation is an implementation of a triangulation class based on the dual edge data structure*
%End

%TypeHeaderCode
#include "DualEdgeTriangulation.h"
%End
  public:
    DualEdgeTriangulation();
    DualEdgeTriangulation( int nop, Triangulation *decorator );
    virtual ~DualEdgeTriangulation();
    void setDecorator( Triangulation *d );
    virtual void addLine( Line3D *line /Transfer/, bool breakline );

%Docstring
Adds a line (e.g. a break-, structure- or an isoline) to the triangulation. The class takes ownership of the line object and its points
%End
    virtual int addPoint( QgsPoint *p /Transfer/ );

%Docstring
Adds a point to the triangulation and returns the number of this point in case of success or -100 in case of failure
 :rtype: int
%End
    virtual void performConsistencyTest();
%Docstring
Performs a consistency check, remove this later
%End
    virtual bool calcNormal( double x, double y, Vector3D *result /Out/ );
%Docstring
Calculates the normal at a point on the surface
 :rtype: bool
%End
    virtual bool calcPoint( double x, double y, QgsPoint *result /Out/ );
%Docstring
Calculates x-, y and z-value of the point on the surface
 :rtype: bool
%End
    virtual QgsPoint *getPoint( unsigned int i ) const;
%Docstring
Returns a pointer to the point with number i
 :rtype: QgsPoint
%End
    virtual int getOppositePoint( int p1, int p2 );

%Docstring
Returns the number of the point opposite to the triangle points p1, p2 (which have to be on a halfedge)
 :rtype: int
%End
    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, int *n1 /Out/, QgsPoint *p2 /Out/, int *n2 /Out/, QgsPoint *p3 /Out/, int *n3 /Out/ ) /PyName=getTriangleVertices/;
%Docstring
Finds out, in which triangle the point with coordinates x and y is and assigns the numbers of the vertices to 'n1', 'n2' and 'n3' and the vertices to 'p1', 'p2' and 'p3'
 :rtype: bool
%End
    virtual bool getTriangle( double x, double y, QgsPoint *p1 /Out/, QgsPoint *p2 /Out/, QgsPoint *p3 /Out/ );
%Docstring
Finds out, in which triangle the point with coordinates x and y is and assigns addresses to the points at the vertices to 'p1', 'p2' and 'p3
 :rtype: bool
%End
    virtual QList<int> *getSurroundingTriangles( int pointno );

%Docstring
Returns a pointer to a value list with the information of the triangles surrounding (counterclockwise) a point. Four integer values describe a triangle, the first three are the number of the half edges of the triangle and the fourth is -10, if the third (and most counterclockwise) edge is a breakline, and -20 otherwise. The value list has to be deleted by the code which called the method
 :rtype: list of int
%End
    virtual double getXMax() const;
%Docstring
Returns the largest x-coordinate value of the bounding box
 :rtype: float
%End
    virtual double getXMin() const;
%Docstring
Returns the smallest x-coordinate value of the bounding box
 :rtype: float
%End
    virtual double getYMax() const;
%Docstring
Returns the largest y-coordinate value of the bounding box
 :rtype: float
%End
    virtual double getYMin() const;
%Docstring
Returns the smallest x-coordinate value of the bounding box
 :rtype: float
%End
    virtual int getNumberOfPoints() const;
%Docstring
Returns the number of points
 :rtype: int
%End
    virtual void setForcedCrossBehavior( Triangulation::ForcedCrossBehavior b );
%Docstring
Sets the behavior of the triangulation in case of crossing forced lines
%End
    virtual void setEdgeColor( int r, int g, int b );
%Docstring
Sets the color of the normal edges
%End
    virtual void setForcedEdgeColor( int r, int g, int b );
%Docstring
Sets the color of the forced edges
%End
    virtual void setBreakEdgeColor( int r, int g, int b );
%Docstring
Sets the color of the breaklines
%End
    virtual void setTriangleInterpolator( TriangleInterpolator *interpolator );

%Docstring
Sets an interpolator object
%End
    virtual void eliminateHorizontalTriangles();

%Docstring
Eliminates the horizontal triangles by swapping or by insertion of new points
%End
    virtual void ruppertRefinement();
%Docstring
Adds points to make the triangles better shaped (algorithm of ruppert)
%End
    virtual bool pointInside( double x, double y );

%Docstring
Returns true, if the point with coordinates x and y is inside the convex hull and false otherwise
 :rtype: bool
%End
    virtual bool swapEdge( double x, double y );
%Docstring
Swaps the edge which is closest to the point with x and y coordinates (if this is possible)
 :rtype: bool
%End
    virtual QList<int> *getPointsAroundEdge( double x, double y );
%Docstring
Returns a value list with the numbers of the four points, which would be affected by an edge swap. This function is e.g. needed by NormVecDecorator to know the points, for which the normals have to be recalculated. The returned ValueList has to be deleted by the code which calls the method
 :rtype: list of int
%End

    virtual bool saveAsShapefile( const QString &fileName ) const;
%Docstring
 Saves the triangulation as a (line) shapefile
:return: true in case of success*
 :rtype: bool
%End

  protected:
    unsigned int insertEdge( int dual, int next, int point, bool mbreak, bool forced );
%Docstring
Inserts an edge and makes sure, everything is OK with the storage of the edge. The number of the HalfEdge is returned
%End
    int insertForcedSegment( int p1, int p2, bool breakline );
%Docstring
Inserts a forced segment between the points with the numbers p1 and p2 into the triangulation and returns the number of a HalfEdge belonging to this forced edge or -100 in case of failure
 :rtype: int
%End
    int baseEdgeOfPoint( int point );
%Docstring
Returns the number of an edge which points to the point with number 'point' or -1 if there is an error
 :rtype: int
%End
    int baseEdgeOfTriangle( QgsPoint *point );
%Docstring
Returns the number of a HalfEdge from a triangle in which 'point' is in. If the number -10 is returned, this means, that 'point' is outside the convex hull. If -5 is returned, then numerical problems with the leftOfTest occurred (and the value of the possible edge is stored in the variable 'mUnstableEdge'. -20 means, that the inserted point is exactly on an edge (the number is stored in the variable 'mEdgeWithPoint'). -25 means, that the point is already in the triangulation (the number of the point is stored in the member 'mTwiceInsPoint'. If -100 is returned, this means that something else went wrong
 :rtype: int
%End
    bool checkSwap( unsigned int edge, unsigned int recursiveDeep );
%Docstring
Checks, if 'edge' has to be swapped because of the empty circle criterion. If so, doSwap(...) is called.
 :rtype: bool
%End
    void doSwap( unsigned int edge, unsigned int recursiveDeep );
%Docstring
Swaps 'edge' and test recursively for other swaps (delaunay criterion)
%End
    void doOnlySwap( unsigned int edge );
%Docstring
Swaps 'edge' and does no recursiv testing
%End
    bool swapPossible( unsigned int edge );
%Docstring
Returns true, if it is possible to swap an edge, otherwise false(concave quad or edge on (or outside) the convex hull)
 :rtype: bool
%End
    void triangulatePolygon( QList<int> *poly, QList<int> *free, int mainedge );
%Docstring
Divides a polygon in a triangle and two polygons and calls itself recursively for these two polygons. 'poly' is a pointer to a list with the numbers of the edges of the polygon, 'free' is a pointer to a list of free halfedges, and 'mainedge' is the number of the edge, towards which the new triangle is inserted. Mainedge has to be the same as poly->begin(), otherwise the recursion does not work
%End
    bool halfEdgeBBoxTest( int edge, double xlowleft, double ylowleft, double xupright, double yupright ) const;
%Docstring
Tests, if the bounding box of the halfedge with index i intersects the specified bounding box. The main purpose for this method is the drawing of the triangulation
 :rtype: bool
%End
    double swapMinAngle( int edge ) const;
%Docstring
Calculates the minimum angle, which would be present, if the specified halfedge would be swapped
 :rtype: float
%End
    int splitHalfEdge( int edge, float position );
%Docstring
Inserts a new point on the halfedge with number 'edge'. The position can have a value from 0 to 1 (e.g. 0.5 would be in the middle). The return value is the number of the new inserted point. tin is the triangulation, which should be used to calculate the elevation of the inserted point
 :rtype: int
%End
    bool edgeOnConvexHull( int edge );
%Docstring
Returns true, if a half edge is on the convex hull and false otherwise
 :rtype: bool
%End
    void evaluateInfluenceRegion( QgsPoint *point, int edge, QSet<int> &set );
%Docstring
Function needed for the ruppert algorithm. Tests, if point is in the circle through both endpoints of edge and the endpoint of edge->dual->next->point. If so, the function calls itself recursively for edge->next and edge->next->next. Stops, if it finds a forced edge or a convex hull edge
%End
};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/interpolation/DualEdgeTriangulation.h                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
