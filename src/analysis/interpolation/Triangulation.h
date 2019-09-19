/***************************************************************************
                          Triangulation.h  -  description
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

#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include <QList>
#include "qgis_sip.h"
#include <QPainter>
#include "TriangleInterpolator.h"
#include "qgis_analysis.h"
#include "qgsinterpolator.h"

class QgsFeatureSink;
class QgsFields;
class QgsFeedback;

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * Interface for Triangulation classes.
 * \note Not available in Python bindings.
*/
class ANALYSIS_EXPORT Triangulation
{
  public:
    //! Enumeration describing the behavior, if two forced lines cross.
    enum ForcedCrossBehavior
    {
      SnappingTypeVertex, //!< The second inserted forced line is snapped to the closest vertice of the first inserted forced line.
      DeleteFirst,        //!< The status of the first inserted forced line is reset to that of a normal edge (so that the second inserted forced line remain and the first not)
      InsertVertex
    };
    virtual ~Triangulation() = default;

    /**
     * Adds a line (e.g. a break-, structure- or an isoline) to the triangulation, by specifying
     * a list of source \a points.
     */
    virtual void addLine( const QVector< QgsPoint > &points, QgsInterpolator::SourceType lineType ) = 0;

    /**
     * Adds a \a point to the triangulation.
     *
     * The point should have a z-value matching the value to interpolate.
     */
    virtual int addPoint( const QgsPoint &point ) = 0;

    /**
     * Calculates the normal at a point on the surface and assigns it to 'result'.
     * \returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool calcNormal( double x, double y, Vector3D *result SIP_OUT ) = 0;

    //! Performs a consistency check, remove this later
    virtual void performConsistencyTest() = 0;

    /**
     * Calculates x-, y and z-value of the point on the surface and assigns it to 'result'.
     * Returns TRUE in case of success and FALSE in case of failure
     */
    virtual bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) = 0;

    //! Returns a pointer to the point with number i. Any virtual points must have the number -1
    virtual QgsPoint *getPoint( int i ) const = 0;

    /**
     * Finds out in which triangle the point with coordinates x and y is and
     * assigns the numbers of the vertices to 'n1', 'n2' and 'n3' and the vertices to 'p1', 'p2' and 'p3'
     */
    virtual bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, int &n1 SIP_OUT, QgsPoint &p2 SIP_OUT, int &n2 SIP_OUT, QgsPoint &p3 SIP_OUT, int &n3 SIP_OUT ) = 0 SIP_PYNAME( getTriangleVertices );

    //! Finds out, in which triangle the point with coordinates x and y is and assigns the  points at the vertices to 'p1', 'p2' and 'p3
    virtual bool getTriangle( double x, double y, QgsPoint &p1 SIP_OUT, QgsPoint &p2 SIP_OUT, QgsPoint &p3 SIP_OUT ) = 0;

    /**
     * Returns the number of the point opposite to the triangle points p1, p2 (which have to be on a halfedge).
     *
     * Returns -1 if point is a virtual point.
     * Returns -10 if point crosses over edges.
     */
    virtual int getOppositePoint( int p1, int p2 ) = 0;

    //! Returns the largest x-coordinate value of the bounding box
    virtual double getXMax() const = 0;

    //! Returns the smallest x-coordinate value of the bounding box
    virtual double getXMin() const = 0;

    //! Returns the largest y-coordinate value of the bounding box
    virtual double getYMax() const = 0;

    //! Returns the smallest x-coordinate value of the bounding box
    virtual double getYMin() const = 0;

    //! Returns the number of points
    virtual int getNumberOfPoints() const = 0;

    /**
     * Returns a pointer to a value list with the information of the triangles surrounding (counterclockwise) a point.
     * Four integer values describe a triangle, the first three are the number of the half edges of the triangle
     * and the fourth is -10, if the third (and most counterclockwise) edge is a breakline, and -20 otherwise.
     * Any virtual point needs to have the number -1
     */
    virtual QList<int> getSurroundingTriangles( int pointno ) = 0;

    /**
     * Returns a value list with the numbers of the four points, which would be affected by an edge swap.
     * This function is e.g. needed by NormVecDecorator to know the points,
     * for which the normals have to be recalculated.
     * The list has to be deleted by the code which calls this method
     */
    virtual QList<int> *getPointsAroundEdge( double x, double y ) = 0;

    //! Draws the points, edges and the forced lines
    //virtual void draw(QPainter* p, double xlowleft, double ylowleft, double xupright, double yupright, double width, double height) const=0;

    //! Sets the behavior of the triangulation in case of crossing forced lines
    virtual void setForcedCrossBehavior( Triangulation::ForcedCrossBehavior b ) = 0;

    //! Sets the color of the normal edges
    virtual void setEdgeColor( int r, int g, int b ) = 0;

    //! Sets the color of the forced edges
    virtual void setForcedEdgeColor( int r, int g, int b ) = 0;

    //! Sets the color of the breaklines
    virtual void setBreakEdgeColor( int r, int g, int b ) = 0;

    //! Sets an interpolator object
    virtual void setTriangleInterpolator( TriangleInterpolator *interpolator ) = 0;

    //! Eliminates the horizontal triangles by swapping
    virtual void eliminateHorizontalTriangles() = 0;

    //! Adds points to make the triangles better shaped (algorithm of ruppert)
    virtual void ruppertRefinement() = 0;

    //! Returns TRUE, if the point with coordinates x and y is inside the convex hull and FALSE otherwise
    virtual bool pointInside( double x, double y ) = 0;

    //! Reads the content of a taff-file
    //virtual bool readFromTAFF(QString fileName)=0;

    //! Saves the content to a taff file
    //virtual bool saveToTAFF(QString fileName) const=0;

    //! Swaps the edge which is closest to the point with x and y coordinates (if this is possible)
    virtual bool swapEdge( double x, double y ) = 0;

    /**
     * Returns the fields output by features when calling
     * saveTriangulation(). These fields should be used when creating
     * a suitable feature sink for saveTriangulation()
     * \see saveTriangulation()
     * \since QGIS 3.0
     */
    static QgsFields triangulationFields();

    /**
     * Saves the triangulation features to a feature \a sink.
     *
     * The sink must be setup to accept LineString features, with fields matching
     * those returned by triangulationFields().
     *
     * \returns TRUE in case of success
     *
     * \see triangulationFields()
     *  \since QGIS 3.0
     */
    virtual bool saveTriangulation( QgsFeatureSink *sink, QgsFeedback *feedback = nullptr ) const = 0;
};

#endif
