/***************************************************************************
                        qgsgeometryutils.h
  -------------------------------------------------------------------
Date                 : 21 Nov 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYUTILS_H
#define QGSGEOMETRYUTILS_H

#include <limits>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspoint.h"
#include "qgsabstractgeometry.h"
#include "qgsvector3d.h"


class QgsLineString;

/**
 * \ingroup core
 * \class QgsGeometryUtils
 * \brief Contains various geometry utility functions.
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsGeometryUtils
{
  public:

    /**
     * Returns list of linestrings extracted from the passed geometry. The returned objects
     *  have to be deleted by the caller.
     */
    static QVector<QgsLineString *> extractLineStrings( const QgsAbstractGeometry *geom ) SIP_FACTORY;

    /**
     * Returns the closest vertex to a geometry for a specified point.
     * On error null point will be returned and "id" argument will be invalid.
     */
    static QgsPoint closestVertex( const QgsAbstractGeometry &geom, const QgsPoint &pt, QgsVertexId &id SIP_OUT );

    /**
     * Returns the nearest point on a segment of a \a geometry
     * for the specified \a point. The z and m values will be linearly interpolated between
     * the two neighbouring vertices.
     */
    static QgsPoint closestPoint( const QgsAbstractGeometry &geometry, const QgsPoint &point );

    /**
     * Returns the distance along a geometry from its first vertex to the specified vertex.
     * \param geom geometry
     * \param id vertex id to find distance to
     * \returns distance to vertex (following geometry)
     * \since QGIS 2.16
     */
    static double distanceToVertex( const QgsAbstractGeometry &geom, QgsVertexId id );

    /**
     * Retrieves the vertices which are before and after the interpolated point at a specified distance along a linestring
     * (or polygon boundary).
     * \param geometry line or polygon geometry
     * \param distance distance to traverse along geometry
     * \param previousVertex will be set to previous vertex ID
     * \param nextVertex will be set to next vertex ID
     * \returns TRUE if vertices were successfully retrieved
     * \note if the distance coincides exactly with a vertex, then both previousVertex and nextVertex will be set to this vertex
     * \since QGIS 3.0
     */
    static bool verticesAtDistance( const QgsAbstractGeometry &geometry,
                                    double distance,
                                    QgsVertexId &previousVertex SIP_OUT,
                                    QgsVertexId &nextVertex SIP_OUT );

    /**
     * Returns the squared 2D distance between two points.
     */
    static double sqrDistance2D( const QgsPoint &pt1, const QgsPoint &pt2 );

    /**
     * Returns the squared distance between a point and a line.
     */
    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double &minDistX SIP_OUT, double &minDistY SIP_OUT, double epsilon );

    /**
     * Computes the intersection between two lines. Z dimension is
     * supported and is retrieved from the first 3D point amongst \a p1 and
     * \a p2.
     * \param p1 Point on the first line
     * \param v1 Direction vector of the first line
     * \param p2 Point on the second line
     * \param v2 Direction vector of the second line
     * \param intersection Output parameter, the intersection point
     * \returns Whether the lines intersect
     */
    static bool lineIntersection( const QgsPoint &p1, QgsVector v1, const QgsPoint &p2, QgsVector v2, QgsPoint &intersection SIP_OUT );

    /**
     * \brief Compute the intersection between two segments
     * \param p1 First segment start point
     * \param p2 First segment end point
     * \param q1 Second segment start point
     * \param q2 Second segment end point
     * \param intersectionPoint Output parameter, the intersection point
     * \param isIntersection Output parameter, return TRUE if an intersection is found
     * \param tolerance The tolerance to use
     * \param acceptImproperIntersection By default, this method returns true only if segments have proper intersection. If set true, returns also TRUE if segments have improper intersection (end of one segment on other segment ; continuous segments).
     * \returns  Whether the segments intersect
     * * Example:
     * \code{.py}
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 1, 1 ), QgsPoint( 1, 0 ) )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # Whether the segments intersect, the intersection point, is intersect
     *   # (False, 'Point (0 0)', False)
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsPoint( 1, 5 ) )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # (False, 'Point (0 5)', True)
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 5 ), QgsPoint( 1, 5 ), acceptImproperIntersection=True )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # (True, 'Point (0 5)', True)
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 2 ), QgsPoint( 1, 5 ) )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # (False, 'Point (0 2)', True)
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, 0 ), QgsPoint( 0, 5 ), QgsPoint( 0, 2 ), QgsPoint( 1, 5 ), acceptImproperIntersection=True )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # (True, 'Point (0 2)', True)
     *   ret = QgsGeometryUtils.segmentIntersection( QgsPoint( 0, -5 ), QgsPoint( 0, 5 ), QgsPoint( 2, 0 ), QgsPoint( -1, 0 ) )
     *   ret[0], ret[1].asWkt(), ret[2]
     *   # (True, 'Point (0 0)', True)
     * \endcode
     */
    static bool segmentIntersection( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &q1, const QgsPoint &q2, QgsPoint &intersectionPoint SIP_OUT, bool &isIntersection SIP_OUT, double tolerance = 1e-8, bool acceptImproperIntersection = false );

    /**
     * \brief Compute the intersection of a line and a circle.
     * If the intersection has two solutions (points),
     * the closest point to the initial \a intersection point is returned.
     * \param center the center of the circle
     * \param radius the radius of the circle
     * \param linePoint1 a first point on the line
     * \param linePoint2 a second point on the line
     * \param intersection the initial point and the returned intersection point
     * \return TRUE if an intersection has been found
     */
    static bool lineCircleIntersection( const QgsPointXY &center, double radius,
                                        const QgsPointXY &linePoint1, const QgsPointXY &linePoint2,
                                        QgsPointXY &intersection SIP_INOUT );

    /**
     * Calculates the intersections points between the circle with center \a center1 and
     * radius \a radius1 and the circle with center \a center2 and radius \a radius2.
     *
     * If found, the intersection points will be stored in \a intersection1 and \a intersection2.
     *
     * \returns number of intersection points found.
     *
     * \since QGIS 3.2
     */
    static int circleCircleIntersections( QgsPointXY center1, double radius1,
                                          QgsPointXY center2, double radius2,
                                          QgsPointXY &intersection1 SIP_OUT, QgsPointXY &intersection2 SIP_OUT );

    /**
     * Calculates the tangent points between the circle with the specified \a center and \a radius
     * and the point \a p.
     *
     * If found, the tangent points will be stored in \a pt1 and \a pt2.
     *
     * \since QGIS 3.2
     */
    static bool tangentPointAndCircle( const QgsPointXY &center, double radius,
                                       const QgsPointXY &p, QgsPointXY &pt1 SIP_OUT, QgsPointXY &pt2 SIP_OUT );

    /**
     * Calculates the outer tangent points for two circles, centered at \a center1 and
     * \a center2 and with radii of \a radius1 and \a radius2 respectively.
     *
     * The outer tangent points correspond to the points at which the two lines
     * which are drawn so that they are tangential to both circles touch
     * the circles.
     *
     * The first tangent line is described by the points
     * stored in \a line1P1 and \a line1P2,
     * and the second line is described by the points stored in \a line2P1
     * and \a line2P2.
     *
     * Returns the number of tangents (either 0 or 2).
     *
     * \since QGIS 3.2
     */
    static int circleCircleOuterTangents(
      const QgsPointXY &center1, double radius1, const QgsPointXY &center2, double radius2,
      QgsPointXY &line1P1 SIP_OUT, QgsPointXY &line1P2 SIP_OUT,
      QgsPointXY &line2P1 SIP_OUT, QgsPointXY &line2P2 SIP_OUT );

    /**
     * Calculates the inner tangent points for two circles, centered at \a
     * center1 and \a center2 and with radii of \a radius1 and \a radius2
     * respectively.
     *
     * The inner tangent points correspond to the points at which the two lines
     * which are drawn so that they are tangential to both circles and are
     * crossing each other.
     *
     * The first tangent line is described by the points
     * stored in \a line1P1 and \a line1P2,
     * and the second line is described by the points stored in \a line2P1
     * and \a line2P2.
     *
     * Returns the number of tangents (either 0 or 2).
     *
     * \since QGIS 3.6
     */
    static int circleCircleInnerTangents(
      const QgsPointXY &center1, double radius1, const QgsPointXY &center2, double radius2,
      QgsPointXY &line1P1 SIP_OUT, QgsPointXY &line1P2 SIP_OUT,
      QgsPointXY &line2P1 SIP_OUT, QgsPointXY &line2P2 SIP_OUT );

    /**
     * \brief Project the point on a segment
     * \param p The point
     * \param s1 The segment start point
     * \param s2 The segment end point
     * \returns The projection of the point on the segment
     */
    static QgsPoint projectPointOnSegment( const QgsPoint &p, const QgsPoint &s1, const QgsPoint &s2 )
    {
      double nx = s2.y() - s1.y();
      double ny = -( s2.x() - s1.x() );
      double t = ( p.x() * ny - p.y() * nx - s1.x() * ny + s1.y() * nx ) / ( ( s2.x() - s1.x() ) * ny - ( s2.y() - s1.y() ) * nx );
      return t < 0. ? s1 : t > 1. ? s2 : QgsPoint( s1.x() + ( s2.x() - s1.x() ) * t, s1.y() + ( s2.y() - s1.y() ) * t );
    }

    //! \note not available in Python bindings
    struct SelfIntersection SIP_SKIP
    {
      int segment1;
      int segment2;
      QgsPoint point;
    };

    /**
     * \brief Find self intersections in a polyline
     * \param geom The geometry to check
     * \param part The part of the geometry to check
     * \param ring The ring of the geometry part to check
     * \param tolerance The tolerance to use
     * \returns The list of self intersections
     * \note not available in Python bindings
     * \since QGIS 2.12
     */
    static QVector<SelfIntersection> selfIntersections( const QgsAbstractGeometry *geom, int part, int ring, double tolerance ) SIP_SKIP;

    /**
     * Returns a value < 0 if the point (\a x, \a y) is left of the line from (\a x1, \a y1) -> ( \a x2, \a y2).
     * A positive return value indicates the point is to the right of the line.
     *
     * If the return value is 0, then the test was unsuccessful (e.g. due to testing a point exactly
     * on the line, or exactly in line with the segment) and the result is undefined.
     */
    static int leftOfLine( const double x, const double y, const double x1, const double y1, const double x2, const double y2 );

    /**
     * Returns a value < 0 if the point \a point is left of the line from \a p1 -> \a p2.
     * A positive return value indicates the point is to the right of the line.
     *
     * If the return value is 0, then the test was unsuccessful (e.g. due to testing a point exactly
     * on the line, or exactly in line with the segment) and the result is undefined.
     *
     * \since QGIS 3.6
     */
    static int leftOfLine( const QgsPoint &point, const QgsPoint &p1, const QgsPoint &p2 );

    /**
     * Returns a point a specified \a distance toward a second point.
     */
    static QgsPoint pointOnLineWithDistance( const QgsPoint &startPoint, const QgsPoint &directionPoint, double distance );

    /**
     * Calculates the point a specified \a distance from (\a x1, \a y1) toward a second point (\a x2, \a y2).
     *
     * Optionally, interpolated z and m values can be obtained by specifying the \a z1, \a z2 and \a z arguments
     * and/or the \a m1, \a m2, \a m arguments.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.4
     */
    static void pointOnLineWithDistance( double x1, double y1, double x2, double y2, double distance, double &x, double &y,
                                         double *z1 = nullptr, double *z2 = nullptr, double *z = nullptr,
                                         double *m1 = nullptr, double *m2 = nullptr, double *m = nullptr ) SIP_SKIP;

    /**
     * Interpolates a point on an arc defined by three points, \a pt1, \a pt2 and \a pt3. The arc will be
     * interpolated by the specified \a distance from \a pt1.
     *
     * Any z or m values present in the points will also be linearly interpolated in the output.
     *
     * \since QGIS 3.4
     */
    static QgsPoint interpolatePointOnArc( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double distance );

    //! Returns the counter clockwise angle between a line with components dx, dy and the line with dx > 0 and dy = 0
    static double ccwAngle( double dy, double dx );

    //! Returns radius and center of the circle through pt1, pt2, pt3
    static void circleCenterRadius( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, double &radius SIP_OUT,
                                    double &centerX SIP_OUT, double &centerY SIP_OUT );

    /**
     * Returns TRUE if the circle defined by three angles is ordered clockwise.
     *
     * The angles are defined counter-clockwise from the origin, i.e. using
     * Euclidean angles as opposed to geographic "North up" angles.
     */
    static bool circleClockwise( double angle1, double angle2, double angle3 );

    //! Returns TRUE if, in a circle, angle is between angle1 and angle2
    static bool circleAngleBetween( double angle, double angle1, double angle2, bool clockwise );

    /**
     * Returns TRUE if an angle is between angle1 and angle3 on a circle described by
     * angle1, angle2 and angle3.
     */
    static bool angleOnCircle( double angle, double angle1, double angle2, double angle3 );

    //! Length of a circular string segment defined by pt1, pt2, pt3
    static double circleLength( double x1, double y1, double x2, double y2, double x3, double y3 );

    //! Calculates angle of a circular string part defined by pt1, pt2, pt3
    static double sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 );

    /**
     * Calculates midpoint on circle passing through \a p1 and \a p2, closest to
     * the given coordinate \a mousePos. Z dimension is supported and is retrieved from the
     * first 3D point amongst \a p1 and \a p2.
     * \see segmentMidPointFromCenter()
     */
    static bool segmentMidPoint( const QgsPoint &p1, const QgsPoint &p2, QgsPoint &result SIP_OUT, double radius, const QgsPoint &mousePos );

    /**
     * Calculates the midpoint on the circle passing through \a p1 and \a p2,
     * with the specified \a center coordinate.
     *
     * If \a useShortestArc is TRUE, then the midpoint returned will be that corresponding
     * to the shorter arc from \a p1 to \a p2. If it is FALSE, the longer arc from \a p1
     * to \a p2 will be used (i.e. winding the other way around the circle).
     *
     * \see segmentMidPoint()
     * \since QGIS 3.2
     */
    static QgsPoint segmentMidPointFromCenter( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &center, bool useShortestArc = true );

    //! Calculates the direction angle of a circle tangent (clockwise from north in radians)
    static double circleTangentDirection( const QgsPoint &tangentPoint, const QgsPoint &cp1, const QgsPoint &cp2, const QgsPoint &cp3 );

    /**
     * Convert circular arc defined by p1, p2, p3 (p1/p3 being start resp. end point, p2 lies on the arc) into a sequence of points.
     * \since 3.0
     */
    static void segmentizeArc( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3,
                               QgsPointSequence SIP_PYALTERNATIVETYPE( QVector<QgsPoint> ) &points SIP_OUT, double tolerance = M_PI_2 / 90,
                               QgsAbstractGeometry::SegmentationToleranceType toleranceType = QgsAbstractGeometry::MaximumAngle,
                               bool hasZ = false, bool hasM = false );

    /**
     * For line defined by points pt1 and pt3, find out on which side of the line is point pt3.
     * Returns -1 if pt3 on the left side, 1 if pt3 is on the right side or 0 if pt3 lies on the line.
     * \since 3.0
     */
    static int segmentSide( const QgsPoint &pt1, const QgsPoint &pt3, const QgsPoint &pt2 );

    /**
     * Interpolate a value at given angle on circular arc given values (zm1, zm2, zm3) at three different angles (a1, a2, a3).
     * \since 3.0
     */
    static double interpolateArcValue( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 );

    /**
     * Returns a list of points contained in a WKT string.
     * \note not available in Python bindings
     */
    static QgsPointSequence pointsFromWKT( const QString &wktCoordinateList, bool is3D, bool isMeasure ) SIP_SKIP;

    /**
     * Returns a LinearRing { uint32 numPoints; Point points[numPoints]; }
     * \note not available in Python bindings
     */
    static void pointsToWKB( QgsWkbPtr &wkb, const QgsPointSequence &points, bool is3D, bool isMeasure ) SIP_SKIP;

    /**
     * Returns a WKT coordinate list
     * \note not available in Python bindings
     */
    static QString pointsToWKT( const QgsPointSequence &points, int precision, bool is3D, bool isMeasure ) SIP_SKIP;

    /**
     * Returns a gml::coordinates DOM element.
     * \note not available in Python bindings
     */
    static QDomElement pointsToGML2( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) SIP_SKIP;

    /**
     * Returns a gml::posList DOM element.
     * \note not available in Python bindings
     */
    static QDomElement pointsToGML3( const QgsPointSequence &points, QDomDocument &doc, int precision, const QString &ns, bool is3D, QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) SIP_SKIP;

    /**
     * Returns a geoJSON coordinates string.
     * \note not available in Python bindings
     */
    static QString pointsToJSON( const QgsPointSequence &points, int precision ) SIP_SKIP;

    /**
     * Ensures that an angle is in the range 0 <= angle < 2 pi.
     * \param angle angle in radians
     * \returns equivalent angle within the range [0, 2 pi)
     */
    static double normalizedAngle( double angle );

    /**
     * Calculates the direction of line joining two points in radians, clockwise from the north direction.
     * \param x1 x-coordinate of line start
     * \param y1 y-coordinate of line start
     * \param x2 x-coordinate of line end
     * \param y2 y-coordinate of line end
     * \returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double lineAngle( double x1, double y1, double x2, double y2 );

    /**
     * Calculates the angle between the lines AB and BC, where AB and BC described
     * by points a, b and b, c.
     * \param x1 x-coordinate of point a
     * \param y1 y-coordinate of point a
     * \param x2 x-coordinate of point b
     * \param y2 y-coordinate of point b
     * \param x3 x-coordinate of point c
     * \param y3 y-coordinate of point c
     * \returns angle between lines in radians. Returned value is undefined if two or more points are equal.
     */
    static double angleBetweenThreePoints( double x1, double y1, double x2, double y2,
                                           double x3, double y3 );

    /**
     * Calculates the perpendicular angle to a line joining two points. Returned angle is in radians,
     * clockwise from the north direction.
     * \param x1 x-coordinate of line start
     * \param y1 y-coordinate of line start
     * \param x2 x-coordinate of line end
     * \param y2 y-coordinate of line end
     * \returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double linePerpendicularAngle( double x1, double y1, double x2, double y2 );

    /**
     * Calculates the average angle (in radians) between the two linear segments from
     * (\a x1, \a y1) to (\a x2, \a y2) and (\a x2, \a y2) to (\a x3, \a y3).
     */
    static double averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 );

    /**
     * Averages two angles, correctly handling negative angles and ensuring the result is between 0 and 2 pi.
     * \param a1 first angle (in radians)
     * \param a2 second angle (in radians)
     * \returns average angle (in radians)
     */
    static double averageAngle( double a1, double a2 );

    /**
     * Parses a WKT block of the format "TYPE( contents )" and returns a pair of geometry type to contents ("Pair(wkbType, "contents")")
     * \note not available in Python bindings
     */
    static QPair<QgsWkbTypes::Type, QString> wktReadBlock( const QString &wkt ) SIP_SKIP;

    /**
     * Parses a WKT string and returns of list of blocks contained in the WKT.
     * \param wkt WKT string in the format "TYPE1 (contents1), TYPE2 (TYPE3 (contents3), TYPE4 (contents4))"
     * \param defaultType default geometry type for children
     * \returns list of WKT child block strings, e.g., List("TYPE1 (contents1)", "TYPE2 (TYPE3 (contents3), TYPE4 (contents4))")
     * \note not available in Python bindings
     */
    static QStringList wktGetChildBlocks( const QString &wkt, const QString &defaultType = QString() ) SIP_SKIP;

    /**
     * Returns a middle point between points pt1 and pt2.
     * Z value is computed if one of this point have Z.
     * M value is computed if one of this point have M.
     * \param pt1 first point.
     * \param pt2 second point.
     * \returns New point at middle between points pt1 and pt2.
     * * Example:
     * \code{.py}
     *   p = QgsPoint( 4, 6 ) # 2D point
     *   pr = midpoint ( p, QgsPoint( 2, 2 ) )
     *   # pr is a 2D point: 'Point (3 4)'
     *   pr = midpoint ( p, QgsPoint( QgsWkbTypes.PointZ, 2, 2, 2 ) )
     *   # pr is a 3D point: 'PointZ (3 4 1)'
     *   pr = midpoint ( p, QgsPoint( QgsWkbTypes.PointM, 2, 2, 0, 2 ) )
     *   # pr is a 3D point: 'PointM (3 4 1)'
     *   pr = midpoint ( p, QgsPoint( QgsWkbTypes.PointZM, 2, 2, 2, 2 ) )
     *   # pr is a 3D point: 'PointZM (3 4 1 1)'
     * \endcode
     * \since QGIS 3.0
     */
    static QgsPoint midpoint( const QgsPoint &pt1, const QgsPoint &pt2 );

    /**
     * Interpolates the position of a point a \a fraction of the way along
     * the line from (\a x1, \a y1) to (\a x2, \a y2).
     *
     * Usually the \a fraction should be between 0 and 1, where 0 represents the
     * point at the start of the line (\a x1, \a y1) and 1 represents
     * the end of the line (\a x2, \a y2). However, it is possible to
     * use a \a fraction < 0 or > 1, in which case the returned point
     * is extrapolated from the supplied line.
     *
     * \see interpolatePointOnLineByValue()
     * \since QGIS 3.0.2
     */
    static QgsPointXY interpolatePointOnLine( double x1, double y1, double x2, double y2, double fraction );

    /**
     * Interpolates the position of a point a \a fraction of the way along
     * the line from \a p1 to \a p2.
     *
     * Usually the \a fraction should be between 0 and 1, where 0 represents the
     * point at the start of the line (\a p1) and 1 represents
     * the end of the line (\a p2). However, it is possible to
     * use a \a fraction < 0 or > 1, in which case the returned point
     * is extrapolated from the supplied line.
     *
     * Any Z or M values present in the input points will also be interpolated
     * and present in the returned point.
     *
     * \see interpolatePointOnLineByValue()
     * \since QGIS 3.0.2
     */
    static QgsPoint interpolatePointOnLine( const QgsPoint &p1, const QgsPoint &p2, double fraction );

    /**
     * Interpolates the position of a point along the line from (\a x1, \a y1)
     * to (\a x2, \a y2).
     *
     * The position is interpolated using a supplied target \a value and the value
     * at the start of the line (\a v1) and end of the line (\a v2). The returned
     * point will be linearly interpolated to match position corresponding to
     * the target \a value.
     *
     * \see interpolatePointOnLine()
     * \since QGIS 3.0.2
     */
    static QgsPointXY interpolatePointOnLineByValue( double x1, double y1, double v1, double x2, double y2, double v2, double value );

    /**
     * Returns the gradient of a line defined by points \a pt1 and \a pt2.
     * \param pt1 first point.
     * \param pt2 second point.
     * \returns The gradient of this linear entity, or infinity if vertical
     * \since QGIS 3.0
     */
    static double gradient( const QgsPoint &pt1, const QgsPoint &pt2 );

    /**
     * Returns the coefficients (a, b, c for equation "ax + by + c = 0") of a line defined by points \a pt1 and \a pt2.
     * \param pt1 first point.
     * \param pt2 second point.
     * \param a Output parameter, a coefficient of the equation.
     * \param b Output parameter, b coefficient of the equation.
     * \param c Output parameter, c coefficient of the equation.
     * \since QGIS 3.0
     */
    static void coefficients( const QgsPoint &pt1, const QgsPoint &pt2,
                              double &a SIP_OUT, double &b SIP_OUT, double &c SIP_OUT );

    /**
     * \brief Create a perpendicular line segment from p to segment [s1, s2]
     * \param p The point
     * \param s1 The segment start point
     * \param s2 The segment end point
     * \returns A line (segment) from p to perpendicular point on segment [s1, s2]
     */
    static QgsLineString perpendicularSegment( const QgsPoint &p, const QgsPoint &s1, const QgsPoint &s2 );


    /**
     * An algorithm to calculate the shortest distance between two skew lines.
     * \param P1 is the first point of the first line,
     * \param P12 is the second point on the first line,
     * \param P2 is the first point on the second line,
     * \param P22 is the second point on the second line.
     * \return the shortest distance
     */
    static double skewLinesDistance( const QgsVector3D &P1, const QgsVector3D &P12,
                                     const QgsVector3D &P2, const QgsVector3D &P22 );

    /**
     * A method to project one skew line onto another.
     * \param P1 is a first point that belonds to first skew line,
     * \param P12 is the second point that belongs to first skew line,
     * \param P2 is the first point that belongs to second skew line,
     * \param P22 is the second point that belongs to second skew line,
     * \param X1 is the result projection point of line P2P22 onto line P1P12,
     * \param epsilon the tolerance to use.
     * \return TRUE if such point exists, FALSE - otherwise.
     */
    static bool skewLinesProjection( const QgsVector3D &P1, const QgsVector3D &P12,
                                     const QgsVector3D &P2, const QgsVector3D &P22,
                                     QgsVector3D &X1  SIP_OUT,
                                     double epsilon = 0.0001 );

    /**
     * An algorithm to calculate an (approximate) intersection of two lines in 3D.
     * \param La1 is the first point on the first line,
     * \param La2 is the second point on the first line,
     * \param Lb1 is the first point on the second line,
     * \param Lb2 is the second point on the second line,
     * \param intersection is the result intersection, of it can be found.
     * \return TRUE if the intersection can be found, FALSE - otherwise.
     * example:
     * \code{.py}
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(2,1,0), QgsVector3D(2,3,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(2.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(2,1,0), QgsVector3D(2,0,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(2.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(0,1,0), QgsVector3D(0,3,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(0.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(0,1,0), QgsVector3D(0,0,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(0.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(5,1,0), QgsVector3D(5,3,0))
     *   # (False, PyQt5.QtGui.QgsVector3D(0.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(0,0,0), QgsVector3D(5,0,0), QgsVector3D(5,1,0), QgsVector3D(5,0,0))
     *   # (False, PyQt5.QtGui.QgsVector3D(0.0, 0.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(1,1,0), QgsVector3D(2,2,0), QgsVector3D(3,1,0), QgsVector3D(3,2,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(3.0, 3.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(1,1,0), QgsVector3D(2,2,0), QgsVector3D(3,2,0), QgsVector3D(3,1,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(3.0, 3.0, 0.0))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(5,5,5), QgsVector3D(0,0,0), QgsVector3D(0,5,5), QgsVector3D(5,0,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(2.5, 2.5, 2.5))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(2.5,2.5,2.5), QgsVector3D(0,5,0), QgsVector3D(2.5,2.5,2.5), QgsVector3D(5,0,0))
     *   # (True, PyQt5.QtGui.QgsVector3D(2.5, 2.5, 2.5))
     *   QgsGeometryUtils.linesIntersection3D(QgsVector3D(2.5,2.5,2.5), QgsVector3D(5,0,0), QgsVector3D(0,5,5), QgsVector3D(5,5,5))
     *   # (True, PyQt5.QtGui.QgsVector3D(0.0, 5.0, 5.0))
     *   \endcode
     */
    static bool linesIntersection3D( const QgsVector3D &La1, const QgsVector3D &La2,
                                     const QgsVector3D &Lb1, const QgsVector3D &Lb2,
                                     QgsVector3D &intersection  SIP_OUT );

    /**
     * A Z dimension is added to \a point if one of the point in the list
     * \a points is in 3D. Moreover, the Z value of \a point is updated with.
     *
     * \param points List of points in which a 3D point is searched.
     * \param point The point to update with Z dimension and value.
     * \returns TRUE if the point is updated, FALSE otherwise
     *
     * \since QGIS 3.0
     */
    static bool setZValueFromPoints( const QgsPointSequence &points, QgsPoint &point );

    //! \note not available in Python bindings
    enum ComponentType SIP_SKIP
    {
      Vertex,
      Ring,
      Part
    };

    //! \note not available in Python bindings
    template<class T> static double closestSegmentFromComponents( T &container, ComponentType ctype, const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon ) SIP_SKIP
    {
      double minDist = std::numeric_limits<double>::max();
      double minDistSegmentX = 0.0, minDistSegmentY = 0.0;
      QgsVertexId minDistVertexAfter;
      int minDistLeftOf = 0;
      double sqrDist = 0.0;
      int vertexOffset = 0;
      int ringOffset = 0;
      int partOffset = 0;

      for ( int i = 0; i < container.size(); ++i )
      {
        sqrDist = container.at( i )->closestSegment( pt, segmentPt, vertexAfter, leftOf, epsilon );
        if ( sqrDist >= 0 && sqrDist < minDist )
        {
          minDist = sqrDist;
          minDistSegmentX = segmentPt.x();
          minDistSegmentY = segmentPt.y();
          minDistVertexAfter = vertexAfter;
          minDistVertexAfter.vertex = vertexAfter.vertex + vertexOffset;
          minDistVertexAfter.part = vertexAfter.part + partOffset;
          minDistVertexAfter.ring = vertexAfter.ring + ringOffset;
          if ( leftOf )
          {
            minDistLeftOf = *leftOf;
          }
        }

        if ( ctype == Vertex )
        {
          //-1 because compoundcurve counts duplicated vertices of neighbour curves as one node
          vertexOffset += container.at( i )->nCoordinates() - 1;
        }
        else if ( ctype == Ring )
        {
          ringOffset += 1;
        }
        else if ( ctype == Part )
        {
          partOffset += 1;
        }
      }

      if ( minDist == std::numeric_limits<double>::max() )
        return -1;  // error: no segments

      segmentPt.setX( minDistSegmentX );
      segmentPt.setY( minDistSegmentY );
      vertexAfter = minDistVertexAfter;
      if ( leftOf )
      {
        *leftOf = minDistLeftOf;
      }
      return minDist;
    }
};

#endif // QGSGEOMETRYUTILS_H
