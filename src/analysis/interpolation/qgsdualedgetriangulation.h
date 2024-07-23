/**************************************************************************
                          QgsDualEdgeTriangulation.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DUALEDGETRIANGULATION_H
#define DUALEDGETRIANGULATION_H

#include <QVector>
#include <QList>
#include <QSet>
#include <QColor>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QBuffer>
#include <QStringList>
#include <QCursor>

#include <cfloat>

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgspoint.h"

#include "qgstriangulation.h"
#include "HalfEdge.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * \brief DualEdgeTriangulation is an implementation of a triangulation class based on the dual edge data structure.
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.16
*/
class ANALYSIS_EXPORT QgsDualEdgeTriangulation: public QgsTriangulation
{
  public:
    QgsDualEdgeTriangulation();

    QgsDualEdgeTriangulation( const QgsDualEdgeTriangulation & ) = delete;
    QgsDualEdgeTriangulation &operator=( const QgsDualEdgeTriangulation &other ) = delete;

    //! Constructor with a number of points to reserve
    QgsDualEdgeTriangulation( int nop );
    ~QgsDualEdgeTriangulation() override;
    void addLine( const QVector< QgsPoint > &points, QgsInterpolator::SourceType lineType ) override;
    int addPoint( const QgsPoint &p ) override;
    //! Performs a consistency check, remove this later
    void performConsistencyTest() override;
    //! Calculates the normal at a point on the surface
    bool calcNormal( double x, double y, QgsPoint &result SIP_OUT ) override;
    bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) override;
    //! Draws the points, edges and the forced lines
    //virtual void draw(QPainter* p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height) const;
    //! Returns a pointer to the point with number i
    QgsPoint *point( int i ) const override;
    int oppositePoint( int p1, int p2 ) override;
    bool triangleVertices( double x, double y, QgsPoint &p1 SIP_OUT, int &n1 SIP_OUT, QgsPoint &p2 SIP_OUT, int &n2 SIP_OUT, QgsPoint &p3 SIP_OUT, int &n3 SIP_OUT ) override;
    bool triangleVertices( double x, double y, QgsPoint &p1 SIP_OUT, QgsPoint &p2 SIP_OUT, QgsPoint &p3 SIP_OUT ) override;
    QList<int> surroundingTriangles( int pointno ) override;
    //! Returns the largest x-coordinate value of the bounding box
    double xMax() const override { return mXMax; }
    //! Returns the smallest x-coordinate value of the bounding box
    double xMin() const override { return mXMin; }
    //! Returns the largest y-coordinate value of the bounding box
    double yMax() const override { return mYMax; }
    //! Returns the smallest x-coordinate value of the bounding box
    double yMin() const override { return mYMin; }
    //! Returns the number of points
    int pointsCount() const override;
    //! Sets the behavior of the triangulation in case of crossing forced lines
    void setForcedCrossBehavior( QgsTriangulation::ForcedCrossBehavior b ) override;
    //! Sets an interpolator object
    void setTriangleInterpolator( TriangleInterpolator *interpolator ) override;
    //! Eliminates the horizontal triangles by swapping or by insertion of new points
    void eliminateHorizontalTriangles() override;
    //! Adds points to make the triangles better shaped (algorithm of ruppert)
    void ruppertRefinement() override;
    //! Returns TRUE, if the point with coordinates x and y is inside the convex hull and FALSE otherwise
    bool pointInside( double x, double y ) override;
    //! Swaps the edge which is closest to the point with x and y coordinates (if this is possible)
    bool swapEdge( double x, double y ) override;
    //! Returns a value list with the numbers of the four points, which would be affected by an edge swap. This function is e.g. needed by NormVecDecorator to know the points, for which the normals have to be recalculated. The returned ValueList has to be deleted by the code which calls the method
    QList<int> pointsAroundEdge( double x, double y ) override;

    bool saveTriangulation( QgsFeatureSink *sink, QgsFeedback *feedback = nullptr ) const override;

    virtual QgsMesh triangulationToMesh( QgsFeedback *feedback = nullptr ) const override;

  private:
    //! X-coordinate of the upper right corner of the bounding box
    double mXMax = 0;
    //! X-coordinate of the lower left corner of the bounding box
    double mXMin = 0;
    //! Y-coordinate of the upper right corner of the bounding box
    double mYMax = 0;
    //! Y-coordinate of the lower left corner of the bounding box
    double mYMin = 0;
    //! Default value for the number of storable points at the beginning
    static const unsigned int DEFAULT_STORAGE_FOR_POINTS = 100000;
    //! Stores pointers to all points in the triangulations (including the points contained in the lines)
    QVector<QgsPoint *> mPointVector;
    //! Default value for the number of storable HalfEdges at the beginning
    static const unsigned int DEFAULT_STORAGE_FOR_HALF_EDGES = 300006;
    //! Stores pointers to the HalfEdges
    QVector<HalfEdge *> mHalfEdge;
    //! Association to an interpolator object
    TriangleInterpolator *mTriangleInterpolator = nullptr;
    //! Member to store the behavior in case of crossing forced segments
    QgsTriangulation::ForcedCrossBehavior mForcedCrossBehavior = QgsTriangulation::DeleteFirst;
    //! Inserts an edge and makes sure, everything is OK with the storage of the edge. The number of the HalfEdge is returned
    unsigned int insertEdge( int dual, int next, int point, bool mbreak, bool forced );
    //! Inserts a forced segment between the points with the numbers p1 and p2 into the triangulation and returns the number of a HalfEdge belonging to this forced edge or -100 in case of failure
    int insertForcedSegment( int p1, int p2, QgsInterpolator::SourceType segmentType );
    //! Security to prevent endless loops in 'baseEdgeOfTriangle'. It there are more iteration then this number, the point will not be inserted
    static const int MAX_BASE_ITERATIONS = 300000;
    //! Returns the number of an edge which points to the point with number 'point' or -1 if there is an error
    int baseEdgeOfPoint( int point );

    /**
     * Returns the number of a HalfEdge from a triangle in which 'point' is in. If the number -10 is returned, this means, that 'point' is outside the convex hull.
     * If -5 is returned, then numerical problems with the leftOfTest occurred (and the value of the possible edge is stored in the variable 'mUnstableEdge'.
     * -20 means, that the inserted point is exactly on an edge (the number is stored in the variable 'mEdgeWithPoint').
     * -25 means, that the point is already in the triangulation (the number of the point is stored in the member 'mTwiceInsPoint'.
     * If -100 is returned, this means that something else went wrong
     */
    int baseEdgeOfTriangle( const QgsPoint &point );
    //! Checks, if 'edge' has to be swapped because of the empty circle criterion. If so, doSwap(...) is called.
    bool checkSwapRecursively( unsigned int edge, unsigned int recursiveDeep );
    //! Return true if edge need to be swapped after Delaunay criteria control
    bool isEdgeNeedSwap( unsigned int edge ) const;
    //! Swaps 'edge' and test recursively for other swaps (delaunay criterion)
    void doSwapRecursively( unsigned int edge, unsigned int recursiveDeep );
    //! Swaps 'edge' and does no recursiv testing
    void doOnlySwap( unsigned int edge );
    //! Number of an edge which does not point to the virtual point. It continuously updated for a fast search
    unsigned int mEdgeInside = 0;
    //! Number of an edge on the outside of the convex hull. It is updated in method 'baseEdgeOfTriangle' to enable insertion of points outside the convex hull
    int mEdgeOutside = -1;
    //! If an inserted point is exactly on an existing edge, 'baseEdgeOfTriangle' returns -20 and sets the variable 'mEdgeWithPoint'
    unsigned int mEdgeWithPoint = 0;
    //! If an instability occurs in 'baseEdgeOfTriangle', mUnstableEdge is set to the value of the current edge
    unsigned int mUnstableEdge = 0;
    //! If a point has been inserted twice, its number is stored in this member
    int mTwiceInsPoint = 0;
    //! Returns TRUE, if it is possible to swap an edge, otherwise FALSE(concave quad or edge on (or outside) the convex hull)
    bool swapPossible( unsigned int edge ) const;
    //! Divides a polygon in a triangle and two polygons and calls itself recursively for these two polygons. 'poly' is a pointer to a list with the numbers of the edges of the polygon, 'free' is a pointer to a list of free halfedges, and 'mainedge' is the number of the edge, towards which the new triangle is inserted. Mainedge has to be the same as poly->begin(), otherwise the recursion does not work
    void triangulatePolygon( QList<int> *poly, QList<int> *free, int mainedge );
    //! Tests, if the bounding box of the halfedge with index i intersects the specified bounding box. The main purpose for this method is the drawing of the triangulation
    bool halfEdgeBBoxTest( int edge, double xlowleft, double ylowleft, double xupright, double yupright ) const;
    //! Calculates the minimum angle, which would be present, if the specified halfedge would be swapped
    double swapMinAngle( int edge ) const;
    //! Inserts a new point on the halfedge with number 'edge'. The position can have a value from 0 to 1 (e.g. 0.5 would be in the middle). The return value is the number of the new inserted point. tin is the triangulation, which should be used to calculate the elevation of the inserted point
    int splitHalfEdge( int edge, float position );
    //! Returns TRUE, if a half edge is on the convex hull and FALSE otherwise
    bool edgeOnConvexHull( int edge );
    //! Function needed for the ruppert algorithm. Tests, if point is in the circle through both endpoints of edge and the endpoint of edge->dual->next->point. If so, the function calls itself recursively for edge->next and edge->next->next. Stops, if it finds a forced edge or a convex hull edge
    void evaluateInfluenceRegion( QgsPoint *point, int edge, QSet<int> &set );
    //! Dimension of the triangulation, -1 : no point, 0 : 1 point, 1 : 2 point or only colinear edges, 2 : 3 or more points and at least 2 non colinear edges
    int mDimension = -1;

    int firstEdgeOutSide();

    void removeLastPoint();


    friend class TestQgsInterpolator;
};

#ifndef SIP_RUN

inline QgsDualEdgeTriangulation::QgsDualEdgeTriangulation()
{
  mPointVector.reserve( DEFAULT_STORAGE_FOR_POINTS );
  mHalfEdge.reserve( DEFAULT_STORAGE_FOR_HALF_EDGES );
}

inline QgsDualEdgeTriangulation::QgsDualEdgeTriangulation( int nop )
{
  mPointVector.reserve( nop );
  mHalfEdge.reserve( nop );
}

inline int QgsDualEdgeTriangulation::pointsCount() const
{
  return mPointVector.count();
}

inline QgsPoint *QgsDualEdgeTriangulation::point( int i ) const
{
  if ( i < 0 || i >= mPointVector.count() )
    return nullptr;

  return mPointVector.at( i );
}

inline bool QgsDualEdgeTriangulation::halfEdgeBBoxTest( int edge, double xlowleft, double ylowleft, double xupright, double yupright ) const
{
  return (
           ( point( mHalfEdge[edge]->getPoint() )->x() >= xlowleft &&
             point( mHalfEdge[edge]->getPoint() )->x() <= xupright &&
             point( mHalfEdge[edge]->getPoint() )->y() >= ylowleft &&
             point( mHalfEdge[edge]->getPoint() )->y() <= yupright ) ||
           ( point( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() )->x() >= xlowleft &&
             point( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() )->x() <= xupright &&
             point( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() )->y() >= ylowleft &&
             point( mHalfEdge[mHalfEdge[edge]->getDual()]->getPoint() )->y() <= yupright )
         );
}

#endif
#endif



