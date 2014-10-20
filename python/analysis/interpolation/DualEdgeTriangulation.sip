class DualEdgeTriangulation: Triangulation
{
%TypeHeaderCode
#include <DualEdgeTriangulation.h>
%End
  public:
    DualEdgeTriangulation();
    DualEdgeTriangulation( int nop, Triangulation* decorator );
    virtual ~DualEdgeTriangulation();
    void setDecorator( Triangulation* d );
    /**Adds a line (e.g. a break-, structure- or an isoline) to the triangulation. The class takes ownership of the line object and its points*/
    void addLine( Line3D* line, bool breakline );
    /**Adds a point to the triangulation and returns the number of this point in case of success or -100 in case of failure*/
    int addPoint( Point3D* p );
    /**Performs a consistency check, remove this later*/
    virtual void performConsistencyTest();
    /**Calculates the normal at a point on the surface*/
    virtual bool calcNormal( double x, double y, Vector3D* result );
    /**Calculates x-, y and z-value of the point on the surface*/
    virtual bool calcPoint( double x, double y, Point3D* result );
    /**draws the points, edges and the forced lines*/
    //virtual void draw(QPainter* p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height) const;
    /**Returns a pointer to the point with number i*/
    virtual Point3D* getPoint( unsigned int i ) const;
    /**Returns the number of the point opposite to the triangle points p1, p2 (which have to be on a halfedge)*/
    int getOppositePoint( int p1, int p2 );
    /**Finds out, in which triangle the point with coordinates x and y is and assigns the numbers of the vertices to 'n1', 'n2' and 'n3' and the vertices to 'p1', 'p2' and 'p3'*/
    //! @note not available in python bindings
    // virtual bool getTriangle( double x, double y, Point3D* p1, int* n1, Point3D* p2, int* n2, Point3D* p3, int* n3 );
    /**Finds out, in which triangle the point with coordinates x and y is and assigns addresses to the points at the vertices to 'p1', 'p2' and 'p3*/
    virtual bool getTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 );
    /**Returns a pointer to a value list with the information of the triangles surrounding (counterclockwise) a point. Four integer values describe a triangle, the first three are the number of the half edges of the triangle and the fourth is -10, if the third (and most counterclockwise) edge is a breakline, and -20 otherwise. The value list has to be deleted by the code which called the method*/
    QList<int>* getSurroundingTriangles( int pointno );
    /**Returns the largest x-coordinate value of the bounding box*/
    virtual double getXMax() const;
    /**Returns the smallest x-coordinate value of the bounding box*/
    virtual double getXMin() const;
    /**Returns the largest y-coordinate value of the bounding box*/
    virtual double getYMax() const;
    /**Returns the smallest x-coordinate value of the bounding box*/
    virtual double getYMin() const;
    /**Returns the number of points*/
    virtual int getNumberOfPoints() const;
    /**Removes the line with number i from the triangulation*/
    void removeLine( int i );
    /**Removes the point with the number i from the triangulation*/
    void removePoint( int i );
    /**Sets the behaviour of the triangulation in case of crossing forced lines*/
    virtual void setForcedCrossBehaviour( Triangulation::forcedCrossBehaviour b );
    /**Sets the color of the normal edges*/
    virtual void setEdgeColor( int r, int g, int b );
    /**Sets the color of the forced edges*/
    virtual void setForcedEdgeColor( int r, int g, int b );
    /**Sets the color of the breaklines*/
    virtual void setBreakEdgeColor( int r, int g, int b );
    /**Sets an interpolator object*/
    void setTriangleInterpolator( TriangleInterpolator* interpolator );
    /**Eliminates the horizontal triangles by swapping or by insertion of new points*/
    void eliminateHorizontalTriangles();
    /**Adds points to make the triangles better shaped (algorithm of ruppert)*/
    virtual void ruppertRefinement();
    /**Returns true, if the point with coordinates x and y is inside the convex hull and false otherwise*/
    bool pointInside( double x, double y );
    /**Reads the dual edge structure of a taff file*/
    //bool readFromTAFF(QString fileName);
    /**Saves the dual edge structure to a taff file*/
    //bool saveToTAFF(QString fileName) const;
    /**Swaps the edge which is closest to the point with x and y coordinates (if this is possible)*/
    virtual bool swapEdge( double x, double y );
    /**Returns a value list with the numbers of the four points, which would be affected by an edge swap. This function is e.g. needed by NormVecDecorator to know the points, for which the normals have to be recalculated. The returned ValueList has to be deleted by the code which calls the method*/
    virtual QList<int>* getPointsAroundEdge( double x, double y );
    /**Saves the triangulation as a (line) shapefile
    @return true in case of success*/
    virtual bool saveAsShapefile( const QString& fileName ) const;

  protected:
    /**inserts an edge and makes sure, everything is ok with the storage of the edge. The number of the HalfEdge is returned*/
    unsigned int insertEdge( int dual, int next, int point, bool mbreak, bool forced );
    /**inserts a forced segment between the points with the numbers p1 and p2 into the triangulation and returns the number of a HalfEdge belonging to this forced edge or -100 in case of failure*/
    int insertForcedSegment( int p1, int p2, bool breakline );
    /**Threshold for the leftOfTest to handle numerical instabilities*/
    //const static double leftOfTresh=0.00001;
    /**Security to prevent endless loops in 'baseEdgeOfTriangle'. It there are more iteration then this number, the point will not be inserted*/
    const static int nBaseOfRuns = 300000;
    /**Returns the number of an edge which points to the point with number 'point' or -1 if there is an error*/
    int baseEdgeOfPoint( int point );
    /**returns the number of a HalfEdge from a triangle in which 'point' is in. If the number -10 is returned, this means, that 'point' is outside the convex hull. If -5 is returned, then numerical problems with the leftOfTest occured (and the value of the possible edge is stored in the variable 'mUnstableEdge'. -20 means, that the inserted point is exactly on an edge (the number is stored in the variable 'mEdgeWithPoint'). -25 means, that the point is already in the triangulation (the number of the point is stored in the member 'mTwiceInsPoint'. If -100 is returned, this means that something else went wrong*/
    int baseEdgeOfTriangle( Point3D* point );
    /**Checks, if 'edge' has to be swapped because of the empty circle criterion. If so, doSwap(...) is called.*/
    bool checkSwap( unsigned int edge, unsigned int recursiveDeep );
    /**Swaps 'edge' and test recursively for other swaps (delaunay criterion)*/
    void doSwap( unsigned int edge, unsigned int recursiveDeep );
    /**Swaps 'edge' and does no recursive testing*/
    void doOnlySwap( unsigned int edge );
    /**Returns true, if it is possible to swap an edge, otherwise false(concave quad or edge on (or outside) the convex hull)*/
    bool swapPossible( unsigned int edge );
    /**divides a polygon in a triangle and two polygons and calls itself recursively for these two polygons. 'poly' is a pointer to a list with the numbers of the edges of the polygon, 'free' is a pointer to a list of free halfedges, and 'mainedge' is the number of the edge, towards which the new triangle is inserted. Mainedge has to be the same as poly->begin(), otherwise the recursion does not work*/
    void triangulatePolygon( QList<int>* poly, QList<int>* free, int mainedge );
    /**Tests, if the bounding box of the halfedge with index i intersects the specified bounding box. The main purpose for this method is the drawing of the triangulation*/
    bool halfEdgeBBoxTest( int edge, double xlowleft, double ylowleft, double xupright, double yupright ) const;
    /**Calculates the minimum angle, which would be present, if the specified halfedge would be swapped*/
    double swapMinAngle( int edge ) const;
    /**Inserts a new point on the halfedge with number 'edge'. The position can have a value from 0 to 1 (e.g. 0.5 would be in the middle). The return value is the number of the new inserted point. tin is the triangulation, which should be used to calculate the elevation of the inserted point*/
    int splitHalfEdge( int edge, float position );
    /**Returns true, if a half edge is on the convex hull and false otherwise*/
    bool edgeOnConvexHull( int edge );
    /**Function needed for the ruppert algorithm. Tests, if point is in the circle through both endpoints of edge and the endpoint of edge->dual->next->point. If so, the function calls itself recursively for edge->next and edge->next->next. Stops, if it finds a forced edge or a convex hull edge*/
    void evaluateInfluenceRegion( Point3D* point, int edge, QSet<int> &set );
};
