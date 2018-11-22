/***************************************************************************
  qgsinternalgeometryengine.h - QgsInternalGeometryEngine

 ---------------------
 begin                : 13.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINTERNALGEOMETRYENGINE_H
#define QGSINTERNALGEOMETRYENGINE_H

#define SIP_NO_FILE

#include <functional>

#include "qgspointxy.h"

class QgsGeometry;
class QgsAbstractGeometry;
class QgsLineString;
class QgsLineSegment2D;

/**
 * \ingroup core
 * This class offers geometry processing methods.
 *
 * The methods are available via QgsGeometry::[geometryfunction]
 * and therefore this does not need to be accessed directly.
 *
 * \note not available in Python bindings
 */

class QgsInternalGeometryEngine
{
  public:

    /**
     * The caller is responsible that the geometry is available and unchanged
     * for the whole lifetime of this object.
     * \param geometry
     */
    explicit QgsInternalGeometryEngine( const QgsGeometry &geometry );

    /**
     * Will extrude a line or (segmentized) curve by a given offset and return a polygon
     * representation of it.
     *
     * \param x offset in x direction
     * \param y offset in y direction
     * \returns an extruded polygon
     */
    QgsGeometry extrude( double x, double y ) const;

    /**
     * Calculates the approximate pole of inaccessibility for a surface, which is the
     * most distant internal point from the boundary of the surface. This function
     * uses the 'polylabel' algorithm (Vladimir Agafonkin, 2016), which is an iterative
     * approach guaranteed to find the true pole of inaccessibility within a specified
     * tolerance. More precise tolerances require more iterations and will take longer
     * to calculate.
     * Optionally, the distance to the polygon boundary from the pole can be stored.
     */
    QgsGeometry poleOfInaccessibility( double precision, double *distanceFromBoundary = nullptr ) const;

    /**
     * Attempts to orthogonalize a line or polygon geometry by shifting vertices to make the geometries
     * angles either right angles or flat lines. This is an iterative algorithm which will loop until
     * either the vertices are within a specified tolerance of right angles or a set number of maximum
     * iterations is reached. The angle threshold parameter specifies how close to a right angle or
     * straight line an angle must be before it is attempted to be straightened.
     * \since QGIS 3.0
     */
    QgsGeometry orthogonalize( double tolerance = 1.0E-8, int maxIterations = 1000, double angleThreshold = 15.0 ) const;

    /**
     * Densifies the geometry by adding the specified number of extra nodes within each
     * segment of the geometry.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \since QGIS 3.0
     */
    QgsGeometry densifyByCount( int extraNodesPerSegment ) const;

    /**
     * Densifies the geometry by adding regularly placed extra nodes inside each segment
     * so that the maximum distance between any two nodes does not exceed the
     * specified \a distance.
     * E.g. specifying a distance 3 would cause the segment [0 0] -> [10 0]
     * to be converted to [0 0] -> [2.5 0] -> [5 0] -> [7.5 0] -> [10 0], since
     * 3 extra nodes are required on the segment and spacing these at 2.5 increments
     * allows them to be evenly spaced over the segment.
     * If the geometry has z or m values present then these will be linearly interpolated
     * at the added nodes.
     * Curved geometry types are automatically segmentized by this routine.
     * \since QGIS 3.0
     */
    QgsGeometry densifyByDistance( double distance ) const;

    /**
     * Calculates a variable width buffer for a (multi)curve geometry.
     *
     * The width of the buffer at each node in the input linestrings is calculated by
     * calling the specified \a widthFunction, which must return an array of the buffer widths
     * for every node in the line.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)curve input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry variableWidthBuffer( int segments, const std::function< std::unique_ptr< double[] >( const QgsLineString *line ) > &widthFunction ) const;

    /**
     * Calculates a tapered width buffer for a (multi)curve geometry.
     *
     * The buffer begins at a width of \a startWidth at the start of each curve, and
     * ends at a width of \a endWidth. Note that unlike QgsGeometry::buffer() methods, \a startWidth
     * and \a endWidth are the diameter of the buffer at these points, not the radius.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)curve input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry taperedBuffer( double startWidth, double endWidth, int segments ) const;

    /**
     * Calculates a variable width buffer using the m-values from a (multi)line geometry.
     *
     * The \a segments argument specifies the number of segments to approximate quarter-circle
     * curves in the buffer.
     *
     * Non (multi)line input geometries will return a null output geometry.
     *
     * \since QGIS 3.2
     */
    QgsGeometry variableWidthBufferByM( int segments ) const;

  private:
    const QgsAbstractGeometry *mGeometry = nullptr;
};

/**
 * A 2D ray which extends from an origin point to an infinite distance in a given direction.
 * \ingroup core
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsRay2D
{
  public:

    /**
     * Constructor for a ray starting at the given \a origin and extending an infinite distance
     * in the specified \a direction.
     */
    QgsRay2D( const QgsPointXY &origin, QgsVector direction )
      : origin( origin )
      , direction( direction )
    {}

    /**
     * Finds the closest intersection point of the ray and a line \a segment.
     *
     * If found, the intersection point will be stored in \a intersectPoint.
     *
     * Returns true if the ray intersects the line segment.
     */
    bool intersects( const QgsLineSegment2D &segment, QgsPointXY &intersectPoint ) const;

  private:

    QgsPointXY origin;
    QgsVector direction;
};

///@cond PRIVATE

// adapted for QGIS geometry classes from original work at https://github.com/trylock/visibility by trylock

/**
 * Compares two line segments based on their distance from a given point
 * Assumes: (1) the line segments are intersected by some ray from the origin
 *          (2) the line segments do not intersect except at their endpoints
 *          (3) no line segment is collinear with the origin
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsLineSegmentDistanceComparer
{
  public:

    /**
     * Constructor for QgsLineSegmentDistanceComparer, comparing points
     * to the specified \a origin point.
     */
    explicit QgsLineSegmentDistanceComparer( const QgsPointXY &origin )
      : mOrigin( origin )
    {}

    /**
     * Checks whether the line segment \a ab is closer to the origin than the
     * line segment \a cd.
     * \param ab line segment: left hand side of the comparison operator
     * \param cd line segment: right hand side of the comparison operator
     * \returns true if ab < cd (ab is closer than cd) to origin
     */
    bool operator()( QgsLineSegment2D ab, QgsLineSegment2D cd ) const;

  private:

    QgsPointXY mOrigin;

};


// adapted for QGIS geometry classes from original work at https://github.com/trylock/visibility by trylock

/**
 * Compares angles from an origin to points clockwise, starting at the positive y-axis.
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsClockwiseAngleComparer
{
  public:
    explicit QgsClockwiseAngleComparer( const QgsPointXY &origin )
      : mVertex( origin )
    {}

    bool operator()( const QgsPointXY &a, const QgsPointXY &b ) const;

  private:

    QgsPointXY mVertex;

};

///@endcond PRIVATE

#endif // QGSINTERNALGEOMETRYENGINE_H
