/***************************************************************************
                       qgsgeometryutils_base.h
 -------------------------------------------------------------------
Date                 : 14 september 2023
Copyright            : (C) 2023 by Loïc Bartoletti
email                : loic dot bartoletti at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvector3d.h"
#include "qgsvector.h"
#include <iterator>

/**
 * \ingroup core
 * \class QgsGeometryUtilsBase
 * \brief Convenience functions for geometry utils.
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsGeometryUtilsBase
{
  public:

    /**
     * Returns the squared 3D distance between (\a x1, \a y1, \a z1) and (\a x2, \a y2, \a z2).
     *
     * \warning No check is done if z contains NaN value. This is the caller's responsibility.
     * \since QGIS 3.36
     */
    static double sqrDistance3D( double x1, double y1, double z1, double x2, double y2, double z2 ) SIP_HOLDGIL {return ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) + ( z1 - z2 ) * ( z1 - z2 ); }

    /**
     * Returns the 3D distance between (\a x1, \a y1, \a z1) and (\a x2, \a y2, \a z2).
     *
     * \warning No check is done if z contains NaN value. This is the caller's responsibility.
     * \since QGIS 3.36
     */
    static double distance3D( double x1, double y1, double z1, double x2, double y2, double z2 ) SIP_HOLDGIL {return std::sqrt( sqrDistance3D( x1, y1, z1, x2, y2, z2 ) ); }

    /**
     * Returns the squared 2D distance between (\a x1, \a y1) and (\a x2, \a y2).
     */
    static double sqrDistance2D( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL {return ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ); }

    /**
     * Returns the 2D distance between (\a x1, \a y1) and (\a x2, \a y2).
     */
    static double distance2D( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL {return std::sqrt( sqrDistance2D( x1, y1, x2, y2 ) ); }

    /**
     * Returns the squared distance between a point and a line.
     */
    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double &minDistX SIP_OUT, double &minDistY SIP_OUT, double epsilon ) SIP_HOLDGIL;

    /**
     * Returns a value < 0 if the point (\a x, \a y) is left of the line from (\a x1, \a y1) -> (\a x2, \a y2).
     * A positive return value indicates the point is to the right of the line.
     *
     * If the return value is 0, then the test was unsuccessful (e.g. due to testing a point exactly
     * on the line, or exactly in line with the segment) and the result is undefined.
     */
    static int leftOfLine( const double x, const double y, const double x1, const double y1, const double x2, const double y2 ) SIP_HOLDGIL;

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
     * Calculates a point a certain \a proportion of the way along the segment from (\a x1, \a y1) to (\a x2, \a y2),
     * offset from the segment by the specified \a offset amount.
     *
     * \param x1 x-coordinate of start of segment
     * \param y1 y-coordinate of start of segment
     * \param x2 x-coordinate of end of segment
     * \param y2 y-coordinate of end of segment
     * \param proportion proportion of the segment's length at which to place the point (between 0.0 and 1.0)
     * \param offset perpendicular offset from segment to apply to point. A negative \a offset shifts the point to the left of the segment, while a positive \a offset will shift it to the right of the segment.
     * \param x calculated point x-coordinate
     * \param y calculated point y-coordinate
     *
     * ### Example
     *
     * \code{.py}
     *   # Offset point at center of segment by 2 units to the right
     *   x, y = QgsGeometryUtils.perpendicularOffsetPointAlongSegment( 1, 5, 11, 5, 0.5, 2 )
     *   # (6.0, 3.0)
     *
     *   # Offset point at center of segment by 2 units to the left
     *   x, y = QgsGeometryUtils.perpendicularOffsetPointAlongSegment( 1, 5, 11, 5, 0.5, -2 )
     *   # (6.0, 7.0)
     * \endcode
     *
     * \since QGIS 3.20
     */
    static void perpendicularOffsetPointAlongSegment( double x1, double y1, double x2, double y2, double proportion, double offset, double *x SIP_OUT, double *y SIP_OUT );

    //! Returns the counter clockwise angle between a line with components dx, dy and the line with dx > 0 and dy = 0
    static double ccwAngle( double dy, double dx ) SIP_HOLDGIL;

    //! Returns radius and center of the circle through (\a x1 \a y1), (\a x2 \a y2), (\a x3 \a y3)
    static void circleCenterRadius( double x1, double y1, double x2, double y2, double x3, double y3, double &radius SIP_OUT,
                                    double &centerX SIP_OUT, double &centerY SIP_OUT ) SIP_HOLDGIL;

    /**
     * Returns TRUE if the circle defined by three angles is ordered clockwise.
     *
     * The angles are defined counter-clockwise from the origin, i.e. using
     * Euclidean angles as opposed to geographic "North up" angles.
     */
    static bool circleClockwise( double angle1, double angle2, double angle3 ) SIP_HOLDGIL;

    //! Returns TRUE if, in a circle, angle is between angle1 and angle2
    static bool circleAngleBetween( double angle, double angle1, double angle2, bool clockwise ) SIP_HOLDGIL;

    /**
     * Returns TRUE if an angle is between angle1 and angle3 on a circle described by
     * angle1, angle2 and angle3.
     */
    static bool angleOnCircle( double angle, double angle1, double angle2, double angle3 ) SIP_HOLDGIL;

    //! Length of a circular string segment defined by pt1, pt2, pt3
    static double circleLength( double x1, double y1, double x2, double y2, double x3, double y3 ) SIP_HOLDGIL;

    //! Calculates angle of a circular string part defined by pt1, pt2, pt3
    static double sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 ) SIP_HOLDGIL;

    /**
     * Interpolate a value at given angle on circular arc given values (zm1, zm2, zm3) at three different angles (a1, a2, a3).
     */
    static double interpolateArcValue( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) SIP_HOLDGIL;

    /**
     * Ensures that an angle is in the range 0 <= angle < 2 pi.
     * \param angle angle in radians
     * \returns equivalent angle within the range [0, 2 pi)
     */
    static double normalizedAngle( double angle ) SIP_HOLDGIL;

    /**
     * Calculates the direction of line joining two points in radians, clockwise from the north direction.
     * \param x1 x-coordinate of line start
     * \param y1 y-coordinate of line start
     * \param x2 x-coordinate of line end
     * \param y2 y-coordinate of line end
     * \returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double lineAngle( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL;

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
                                           double x3, double y3 ) SIP_HOLDGIL;

    /**
     * Calculates the perpendicular angle to a line joining two points. Returned angle is in radians,
     * clockwise from the north direction.
     * \param x1 x-coordinate of line start
     * \param y1 y-coordinate of line start
     * \param x2 x-coordinate of line end
     * \param y2 y-coordinate of line end
     * \returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double linePerpendicularAngle( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL;

    /**
     * Calculates the average angle (in radians) between the two linear segments from
     * (\a x1, \a y1) to (\a x2, \a y2) and (\a x2, \a y2) to (\a x3, \a y3).
     */
    static double averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 ) SIP_HOLDGIL;

    /**
     * Averages two angles, correctly handling negative angles and ensuring the result is between 0 and 2 pi.
     * \param a1 first angle (in radians)
     * \param a2 second angle (in radians)
     * \returns average angle (in radians)
     */
    static double averageAngle( double a1, double a2 ) SIP_HOLDGIL;

    /**
     * Returns a number representing the closest side of a rectangle defined by /a right,
     * \a bottom, \a left, \a top to the point at (\a x, \a y), where
     * the point may be in the interior of the rectangle or outside it.
     *
     * The returned value may be:
     *
     * 1. Point is closest to top side of rectangle
     * 2. Point is located on the top-right diagonal of rectangle, equally close to the top and right sides
     * 3. Point is closest to right side of rectangle
     * 4. Point is located on the bottom-right diagonal of rectangle, equally close to the bottom and right sides
     * 5. Point is closest to bottom side of rectangle
     * 6. Point is located on the bottom-left diagonal of rectangle, equally close to the bottom and left sides
     * 7. Point is closest to left side of rectangle
     * 8. Point is located on the top-left diagonal of rectangle, equally close to the top and left sides
     *
     * \note This method effectively partitions the space outside of the rectangle into Voronoi cells, so a point
     * to the top left of the rectangle may be assigned to the left or top sides based on its position relative
     * to the diagonal line extended from the rectangle's top-left corner.
     *
     * \since QGIS 3.20
     */
    static int closestSideOfRectangle( double right, double bottom, double left, double top, double x, double y );

    /**
     * \brief Create a perpendicular line segment to a given segment [\a segmentPoint1,\a segmentPoint2] with its center at \a centerPoint.
     *
     * May be used to split geometries. Unless \a segmentLength is specified the new centered perpendicular line segment will have double the length of the input segment.
     *
     * The result is a line (segment) centered in point p and perpendicular to segment [segmentPoint1, segmentPoint2].
     *
     * \param centerPointX x-coordinate of the point where the center of the perpendicular should be located
     * \param centerPointY y-coordinate of the point where the center of the perpendicular should be located
     * \param segmentPoint1x: x-coordinate of segmentPoint1, the segment's start point
     * \param segmentPoint1y: y-coordinate of segmentPoint1, the segment's start point
     * \param segmentPoint2x: x-coordinate of segmentPoint2, the segment's end point
     * \param segmentPoint2y: y-coordinate of segmentPoint2, the segment's end point
     * \param perpendicularSegmentPoint1x: x-coordinate of the perpendicularCenterSegment's start point
     * \param perpendicularSegmentPoint1y: y-coordinate of the perpendicularCenterSegment's start point
     * \param perpendicularSegmentPoint2x: x-coordinate of the perpendicularCenterSegment's end point
     * \param perpendicularSegmentPoint2y: y-coordinate of the perpendicularCenterSegment's end point
     * \param segmentLength (optional) Trims to given length. A segmentLength value of 0 refers to the default length which is double the length of the input segment. Set to 1 for a normalized length.
     *
     *
     * \since QGIS 3.24
     */

    static void perpendicularCenterSegment( double centerPointX, double centerPointY,
                                            double segmentPoint1x, double segmentPoint1y,
                                            double segmentPoint2x, double segmentPoint2y,
                                            double &perpendicularSegmentPoint1x SIP_OUT, double &perpendicularSegmentPoint1y SIP_OUT,
                                            double &perpendicularSegmentPoint2x SIP_OUT, double &perpendicularSegmentPoint2y SIP_OUT,
                                            double segmentLength = 0
                                          ) SIP_HOLDGIL;

    /**
     * An algorithm to calculate the shortest distance between two skew lines.
     * \param P1 is the first point of the first line,
     * \param P12 is the second point on the first line,
     * \param P2 is the first point on the second line,
     * \param P22 is the second point on the second line.
     * \return the shortest distance
     */
    static double skewLinesDistance( const QgsVector3D &P1, const QgsVector3D &P12,
                                     const QgsVector3D &P2, const QgsVector3D &P22 ) SIP_HOLDGIL;

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
                                     QgsVector3D &X1 SIP_OUT,
                                     double epsilon = 0.0001 ) SIP_HOLDGIL;

    /**
     * An algorithm to calculate an (approximate) intersection of two lines in 3D.
     * \param La1 is the first point on the first line,
     * \param La2 is the second point on the first line,
     * \param Lb1 is the first point on the second line,
     * \param Lb2 is the second point on the second line,
     * \param intersection is the result intersection, of it can be found.
     * \return TRUE if the intersection can be found, FALSE - otherwise.
     *
     * ### Example
     *
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
                                     QgsVector3D &intersection SIP_OUT ) SIP_HOLDGIL;

    /**
     * Returns the area of the triangle denoted by the points (\a aX, \a aY), (\a bX, \a bY) and
     * (\a cX, \a cY).
     *
     * \since QGIS 3.10
     */
    static double triangleArea( double aX, double aY, double bX, double bY, double cX, double cY ) SIP_HOLDGIL;

    /**
     * Given the line (\a x1, \a y1) to (\a x2, \a y2) and a point (\a px, \a py) returns the fraction
     * of the line length at which the point lies.
     *
     * \warning this method requires that the point definitely lies on the line!
     *
     * \since QGIS 3.32
     */
    static double pointFractionAlongLine( double x1, double y1, double x2, double y2, double px, double py );

    /**
     * Returns a weighted point inside the triangle denoted by the points (\a aX, \a aY), (\a bX, \a bY) and
     * (\a cX, \a cY).
     *
     * \param aX x-coordinate of first vertex in triangle
     * \param aY y-coordinate of first vertex in triangle
     * \param bX x-coordinate of second vertex in triangle
     * \param bY y-coordinate of second vertex in triangle
     * \param cX x-coordinate of third vertex in triangle
     * \param cY y-coordinate of third vertex in triangle
     * \param weightB weighting factor along axis A-B (between 0 and 1)
     * \param weightC weighting factor along axis A-C (between 0 and 1)
     * \param pointX x-coordinate of generated point
     * \param pointY y-coordinate of generated point
     *
     * \since QGIS 3.10
     */
    static void weightedPointInTriangle( double aX, double aY, double bX, double bY, double cX, double cY,
                                         double weightB, double weightC, double &pointX SIP_OUT, double &pointY SIP_OUT ) SIP_HOLDGIL;

    /**
     * Given the points (\a x1, \a y1), (\a x2, \a y2) and (\a x3, \a y3) returns TRUE if these
     * points can be considered collinear with a specified tolerance \a epsilon.
     *
     * \since QGIS 3.32
     */
    static bool pointsAreCollinear( double x1, double y1, double x2, double y2, double x3, double y3, double epsilon );

    /**
     * Returns the point (\a pointX, \a pointY) forming the bisector from segment (\a aX \a aY) (\a bX \a bY)
     * and segment (\a bX, \a bY) (\a dX, \a dY).
     * The bisector segment of AB-CD is (point, projection of point by \a angle)
     *
     * \param aX x-coordinate of first vertex of the segment ab
     * \param aY y-coordinate of first vertex of the segment ab
     * \param bX x-coordinate of second vertex of the segment ab
     * \param bY y-coordinate of second vertex of the segment ab
     * \param cX x-coordinate of first vertex of the segment cd
     * \param cY y-coordinate of first vertex of the segment cd
     * \param dX x-coordinate of second vertex of the segment cd
     * \param dY y-coordinate of second vertex of the segment cd
     * \param pointX x-coordinate of generated point
     * \param pointY y-coordinate of generated point
     * \param angle angle of the bisector from pointX, pointY origin on [ab-cd]
     * \returns TRUE if the bisector exists (A B and C D are not collinear)
     *
     * \since QGIS 3.18
     */

    static bool angleBisector( double aX, double aY, double bX, double bY, double cX, double cY, double dX, double dY,
                               double &pointX SIP_OUT, double &pointY SIP_OUT, double &angle SIP_OUT ) SIP_HOLDGIL;

    /**
     * Returns the point (\a pointX, \a pointY) forming the bisector from point (\a aX, \a aY) to the segment (\a bX, \a bY) (\a cX, \a cY).
     * The bisector segment of ABC is (A-point)
     *
     * \param aX x-coordinate of first vertex in triangle
     * \param aY y-coordinate of first vertex in triangle
     * \param bX x-coordinate of second vertex in triangle
     * \param bY y-coordinate of second vertex in triangle
     * \param cX x-coordinate of third vertex in triangle
     * \param cY y-coordinate of third vertex in triangle
     * \param pointX x-coordinate of generated point
     * \param pointY y-coordinate of generated point
     * \returns TRUE if the bisector exists (A B and C are not collinear)
     *
     * \since QGIS 3.18
     */
    static bool bisector( double aX, double aY, double bX, double bY, double cX, double cY,
                          double &pointX SIP_OUT, double &pointY SIP_OUT ) SIP_HOLDGIL;


    /**
     * Computes the intersection between two lines. Z dimension is
     * supported and is retrieved from the first 3D point amongst \a p1 and
     * \a p2.
     * \param p1x x-coordinate of point on the first line
     * \param p1y y-coordinate of point on the first line
     * \param v1 Direction vector of the first line
     * \param p2x x-coordinate of second point on the first line
     * \param p2y y-coordinate of second point on the first line
     * \param v2 Direction vector of the second line
     * \param intersectionX x-coordinate of the intersection point
     * \param intersectionY y-coordinate of the intersection point
     * \returns Whether the lines intersect
     */
    static bool lineIntersection( double p1x, double p1y, QgsVector v1, double p2x, double p2y, QgsVector v2, double &intersectionX SIP_OUT, double &intersectionY SIP_OUT ) SIP_HOLDGIL;

    /**
     * \brief Compute the intersection between two segments
     * \param p1x x-coordinate of the first segment start point
     * \param p1y y-coordinate of the first segment start point
     * \param p2x x-coordinate of the first segment end point
     * \param p2y y-coordinate of the first segment end point
     * \param q1x x-coordinate of the second segment start point
     * \param q1y y-coordinate of the second segment start point
     * \param q2x x-coordinate of the second segment end point
     * \param q2y y-coordinate of the second segment end point
     * \param intersectionPointX Output parameter, x-coordinate of the intersection point
     * \param intersectionPointY Output parameter, y-coordinate of the intersection point
     * \param isIntersection Output parameter, return TRUE if an intersection is found
     * \param tolerance The tolerance to use
     * \param acceptImproperIntersection By default, this method returns TRUE only if segments have proper intersection. If set true, returns also TRUE if segments have improper intersection (end of one segment on other segment ; continuous segments).
     * \returns  Whether the segments intersect
     *
     */
    static bool segmentIntersection( double p1x, double p1y, double p2x, double p2y, double q1x, double q1y, double q2x, double q2y, double &intersectionPointX SIP_OUT, double &intersectionPointY SIP_OUT, bool &isIntersection SIP_OUT, double tolerance = 1e-8, bool acceptImproperIntersection = false ) SIP_HOLDGIL;

    /**
     * Returns coordinates of a point which corresponds to this point projected by a specified distance
     * with specified angles (azimuth and inclination), using Cartesian mathematics.
     * M value is preserved.
     * resultX, resultY, resultZ are coordinates of the point projected.
     *  If a 2D point is projected a 3D point will be returned except if
     *  inclination is 90. A 3D point is always returned if a 3D point is projected.
     * \param aX x-coordinate of the point to project
     * \param aY y-coordinate of the point to project
     * \param aZ z-coordinate of the point to project
     * \param distance distance to project
     * \param azimuth angle to project in X Y, clockwise in degrees starting from north
     * \param inclination angle to project in Z (3D). If the point is 2D, the Z value is assumed to be 0.
     * \param resultX Output parameter, x-coordinates of the point projected.
     * \param resultY Output parameter, y-coordinates of the point projected.
     * \param resultZ Output parameter, z-coordinates of the point projected.
     * \since QGIS 3.34
     */
    static void project( double aX, double aY, double aZ, double distance, double azimuth, double inclination, double &resultX SIP_OUT, double &resultY SIP_OUT, double &resultZ SIP_OUT ) SIP_HOLDGIL;

    /**
     * Calculates Cartesian azimuth between points (\a x1, \a y1) and (\a x2, \a y2)  (clockwise in degree, starting from north)
     * \param x1 x-coordinate of the start point
     * \param y1 y-coordinate of the start point
     * \param x2 x-coordinate of the end point
     * \param y2 y-coordinate of the end point
     * \since QGIS 3.34
     */
    static double azimuth( double x1, double y1, double x2, double y2 ) SIP_HOLDGIL;

#ifndef SIP_RUN

    /**
     * Performs fuzzy comparison between pairs of values within a specified epsilon.
     *
     * This function compares a variable number of pairs of values to check if their differences
     * fall within a specified epsilon range using qgsNumberNear. It returns true if all the differences
     * are within the given epsilon range; otherwise, it returns false.
     *
     * \tparam T Floating-point type (double or float) for the values to be compared.
     * \tparam Args Type of arguments for the values to be compared.
     * \param epsilon The range within which the differences are checked.
     * \param args Variadic list of values to be compared in pairs.
     *             The number of arguments must be greater than 0 and even.
     *             It must follow the pattern: x1, y1, x2, y2, or x1, y1, z1, x2, y2, z2, ...
     * \return true if all the differences between pairs of values are within epsilon, false otherwise.
     *
     * \see fuzzyDistanceEqual
     *
     * \since QGIS 3.36
     */
    template<typename T, typename... Args>
    static bool fuzzyEqual( T epsilon, const Args &... args ) noexcept
    {
      static_assert( ( sizeof...( args ) % 2 == 0 || sizeof...( args ) != 0 ), "The number of arguments must be greater than 0 and even" );
      constexpr size_t numArgs = sizeof...( args );
      bool result = true;
      T values[] = {static_cast<T>( args )...};

      for ( size_t i = 0; i < numArgs / 2; ++i )
      {
        result = result && qgsNumberNear( values[i], values[i + numArgs / 2], epsilon );
      }

      return result;
    }

    /**
     * Compare equality between multiple pairs of values with a specified epsilon.
     *
     * Traditionally, the comparison is done by examining the specific values (such as x and y) that define the location of points.
     * It focuses on the numerical differences or relationships between these values.
     * On the other hand, comparing distances between points considers the actual spatial separation or length between the points, regardless of their coordinate values.
     * This comparison involves measuring the distance between two points using formulas like the distance formula. Here, it's the "distance comparison" (fuzzyDistanceEqual).
     *
     * \tparam T Floating-point type (double or float) for the values to be compared.
     * \tparam Args Type of arguments for the values to be compared.
     * \param epsilon The range within which the differences are checked.
     * \param args Variadic list of values to be compared in pairs.
     *             The number of arguments must be greater than or equal to 4.
     *             It must follow the pattern: x1, y1, x2, y2, or x1, y1, z1, x2, y2, z2, ...
     * \return true if the squares of differences between pairs of values sum up to less than epsilon squared, false otherwise.
     *
     * \see fuzzyEqual
     *
     * \since QGIS 3.36
     */
    template<typename T, typename... Args>
    static bool fuzzyDistanceEqual( T epsilon, const Args &... args ) noexcept
    {
      static_assert( ( sizeof...( args ) % 2 == 0 || sizeof...( args ) >= 4 ), "The number of arguments must be greater than 4 and even" );
      constexpr size_t numArgs = sizeof...( args );
      const T squaredEpsilon = epsilon * epsilon;
      T sum = 0;

      T values[] = {static_cast<T>( args )...};

      for ( size_t i = 0; i < numArgs / 2; ++i )
      {
        const T diff = values[i] - values[i + numArgs / 2];
        sum += diff * diff;
      }

      return sum < squaredEpsilon;
    }
#endif

};
