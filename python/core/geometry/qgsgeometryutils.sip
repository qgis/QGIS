/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometryutils.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGeometryUtils
{
%Docstring
 Contains various geometry utility functions.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsgeometryutils.h"
%End
  public:

    static QList<QgsLineString *> extractLineStrings( const QgsAbstractGeometry *geom ) /Factory/;
%Docstring
 Returns list of linestrings extracted from the passed geometry. The returned objects
  have to be deleted by the caller.
 :rtype: list of QgsLineString
%End

    static QgsPointV2 closestVertex( const QgsAbstractGeometry &geom, const QgsPointV2 &pt, QgsVertexId &id /Out/ );
%Docstring
 Returns the closest vertex to a geometry for a specified point.
 On error null point will be returned and "id" argument will be invalid.
 :rtype: QgsPointV2
%End

    static double distanceToVertex( const QgsAbstractGeometry &geom, QgsVertexId id );
%Docstring
 Returns the distance along a geometry from its first vertex to the specified vertex.
 \param geom geometry
 \param id vertex id to find distance to
 :return: distance to vertex (following geometry)
.. versionadded:: 2.16
 :rtype: float
%End

    static bool verticesAtDistance( const QgsAbstractGeometry &geometry,
                                    double distance,
                                    QgsVertexId &previousVertex /Out/,
                                    QgsVertexId &nextVertex /Out/ );
%Docstring
 Retrieves the vertices which are before and after the interpolated point at a specified distance along a linestring
 (or polygon boundary).
 \param geometry line or polygon geometry
 \param distance distance to traverse along geometry
 \param previousVertex will be set to previous vertex ID
 \param nextVertex will be set to next vertex ID
.. note::

   if the distance coincides exactly with a vertex, then both previousVertex and nextVertex will be set to this vertex
 :return: true if vertices were successfully retrieved
.. versionadded:: 3.0
 :rtype: bool
%End

    static void adjacentVertices( const QgsAbstractGeometry &geom, QgsVertexId atVertex, QgsVertexId &beforeVertex /Out/, QgsVertexId &afterVertex /Out/ );
%Docstring
 Returns vertices adjacent to a specified vertex within a geometry.
%End

    static double sqrDistance2D( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Returns the squared 2D distance between two points.
 :rtype: float
%End

    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double &minDistX /Out/, double &minDistY /Out/, double epsilon );
%Docstring
 Returns the squared distance between a point and a line.
 :rtype: float
%End

    static bool lineIntersection( const QgsPointV2 &p1, QgsVector v, const QgsPointV2 &q1, QgsVector w, QgsPointV2 &inter /Out/ );
%Docstring
 Compute the intersection between two lines
 \param p1 Point on the first line
 \param v Direction vector of the first line
 \param q1 Point on the second line
 \param w Direction vector of the second line
 \param inter Output parameter, the intersection point
 :return: Whether the lines intersect
 :rtype: bool
%End

    static bool segmentIntersection( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &q1, const QgsPointV2 &q2, QgsPointV2 &inter /Out/, double tolerance );
%Docstring
 Compute the intersection between two segments
 \param p1 First segment start point
 \param p2 First segment end point
 \param q1 Second segment start point
 \param q2 Second segment end point
 \param inter Output parameter, the intersection point
 \param tolerance The tolerance to use
 :return:  Whether the segments intersect
 :rtype: bool
%End

    static QgsPointV2 projPointOnSegment( const QgsPointV2 &p, const QgsPointV2 &s1, const QgsPointV2 &s2 );
%Docstring
 Project the point on a segment
 \param p The point
 \param s1 The segment start point
 \param s2 The segment end point
 :return: The projection of the point on the segment
 :rtype: QgsPointV2
%End



    static double leftOfLine( double x, double y, double x1, double y1, double x2, double y2 );
%Docstring
Returns < 0 if point(x/y) is left of the line x1,y1 -> x2,y2
 :rtype: float
%End

    static QgsPointV2 pointOnLineWithDistance( const QgsPointV2 &startPoint, const QgsPointV2 &directionPoint, double distance );
%Docstring
 Returns a point a specified distance toward a second point.
 :rtype: QgsPointV2
%End

    static double ccwAngle( double dy, double dx );
%Docstring
Returns the counter clockwise angle between a line with components dx, dy and the line with dx > 0 and dy = 0
 :rtype: float
%End

    static void circleCenterRadius( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3, double &radius /Out/,
                                    double &centerX /Out/, double &centerY /Out/ );
%Docstring
Returns radius and center of the circle through pt1, pt2, pt3
%End

    static bool circleClockwise( double angle1, double angle2, double angle3 );
%Docstring
Returns true if circle is ordered clockwise
 :rtype: bool
%End

    static bool circleAngleBetween( double angle, double angle1, double angle2, bool clockwise );
%Docstring
Returns true if, in a circle, angle is between angle1 and angle2
 :rtype: bool
%End

    static bool angleOnCircle( double angle, double angle1, double angle2, double angle3 );
%Docstring
 Returns true if an angle is between angle1 and angle3 on a circle described by
 angle1, angle2 and angle3.
 :rtype: bool
%End

    static double circleLength( double x1, double y1, double x2, double y2, double x3, double y3 );
%Docstring
Length of a circular string segment defined by pt1, pt2, pt3
 :rtype: float
%End

    static double sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 );
%Docstring
Calculates angle of a circular string part defined by pt1, pt2, pt3
 :rtype: float
%End

    static bool segmentMidPoint( const QgsPointV2 &p1, const QgsPointV2 &p2, QgsPointV2 &result /Out/, double radius, const QgsPointV2 &mousePos );
%Docstring
Calculates midpoint on circle passing through p1 and p2, closest to given coordinate
 :rtype: bool
%End

    static double circleTangentDirection( const QgsPointV2 &tangentPoint, const QgsPointV2 &cp1, const QgsPointV2 &cp2, const QgsPointV2 &cp3 );
%Docstring
Calculates the direction angle of a circle tangent (clockwise from north in radians)
 :rtype: float
%End

    static void segmentizeArc( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3,
                               QList<QgsPointV2>  &points /Out/, double tolerance = M_PI_2 / 90,
                               QgsAbstractGeometry::SegmentationToleranceType toleranceType = QgsAbstractGeometry::MaximumAngle,
                               bool hasZ = false, bool hasM = false );
%Docstring
 Convert circular arc defined by p1, p2, p3 (p1/p3 being start resp. end point, p2 lies on the arc) into a sequence of points.
.. note::

   added in 3.0
 :rtype: QgsPointSequence
%End

    static int segmentSide( const QgsPointV2 &pt1, const QgsPointV2 &pt3, const QgsPointV2 &pt2 );
%Docstring
 For line defined by points pt1 and pt3, find out on which side of the line is point pt3.
 Returns -1 if pt3 on the left side, 1 if pt3 is on the right side or 0 if pt3 lies on the line.
.. note::

   added in 3.0
 :rtype: int
%End

    static double interpolateArcValue( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 );
%Docstring
 Interpolate a value at given angle on circular arc given values (zm1, zm2, zm3) at three different angles (a1, a2, a3).
.. note::

   added in 3.0
 :rtype: float
%End







    static double normalizedAngle( double angle );
%Docstring
 Ensures that an angle is in the range 0 <= angle < 2 pi.
 \param angle angle in radians
 :return: equivalent angle within the range [0, 2 pi)
 :rtype: float
%End

    static double lineAngle( double x1, double y1, double x2, double y2 );
%Docstring
 Calculates the direction of line joining two points in radians, clockwise from the north direction.
 \param x1 x-coordinate of line start
 \param y1 y-coordinate of line start
 \param x2 x-coordinate of line end
 \param y2 y-coordinate of line end
 :return: angle in radians. Returned value is undefined if start and end point are the same.
 :rtype: float
%End

    static double angleBetweenThreePoints( double x1, double y1, double x2, double y2,
                                           double x3, double y3 );
%Docstring
 Calculates the angle between the lines AB and BC, where AB and BC described
 by points a, b and b, c.
 \param x1 x-coordinate of point a
 \param y1 y-coordinate of point a
 \param x2 x-coordinate of point b
 \param y2 y-coordinate of point b
 \param x3 x-coordinate of point c
 \param y3 y-coordinate of point c
 :return: angle between lines in radians. Returned value is undefined if two or more points are equal.
 :rtype: float
%End

    static double linePerpendicularAngle( double x1, double y1, double x2, double y2 );
%Docstring
 Calculates the perpendicular angle to a line joining two points. Returned angle is in radians,
 clockwise from the north direction.
 \param x1 x-coordinate of line start
 \param y1 y-coordinate of line start
 \param x2 x-coordinate of line end
 \param y2 y-coordinate of line end
 :return: angle in radians. Returned value is undefined if start and end point are the same.
 :rtype: float
%End

    static double averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 );
%Docstring
Angle between two linear segments
 :rtype: float
%End

    static double averageAngle( double a1, double a2 );
%Docstring
 Averages two angles, correctly handling negative angles and ensuring the result is between 0 and 2 pi.
 \param a1 first angle (in radians)
 \param a2 second angle (in radians)
 :return: average angle (in radians)
 :rtype: float
%End



    static QgsPointV2 midpoint( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Returns a middle point between points pt1 and pt2.
 Z value is computed if one of this point have Z.
 M value is computed if one of this point have M.
 \param pt1 first point.
 \param pt2 second point.
 :return: New point at middle between points pt1 and pt2.
 * Example:
 \code{.py}
   p = QgsPointV2( 4, 6 ) # 2D point
   pr = midpoint ( p, QgsPointV2( 2, 2 ) )
   # pr is a 2D point: 'Point (3 4)'
   pr = midpoint ( p, QgsPointV2( QgsWkbTypes.PointZ, 2, 2, 2 ) )
   # pr is a 3D point: 'PointZ (3 4 1)'
   pr = midpoint ( p, QgsPointV2( QgsWkbTypes.PointM, 2, 2, 0, 2 ) )
   # pr is a 3D point: 'PointM (3 4 1)'
   pr = midpoint ( p, QgsPointV2( QgsWkbTypes.PointZM, 2, 2, 2, 2 ) )
   # pr is a 3D point: 'PointZM (3 4 1 1)'
 \endcode
.. versionadded:: 3.0
 :rtype: QgsPointV2
%End

    static double gradient( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Return the gradient of a line defined by points ``pt1`` and ``pt2``.
 \param pt1 first point.
 \param pt2 second point.
 :return: The gradient of this linear entity, or infinity if vertical
.. versionadded:: 3.0
 :rtype: float
%End

    static void coefficients( const QgsPointV2 &pt1, const QgsPointV2 &pt2,
                              double &a /Out/, double &b /Out/, double &c /Out/ );
%Docstring
 Return the coefficients (a, b, c for equation "ax + by + c = 0") of a line defined by points ``pt1`` and ``pt2``.
 \param pt1 first point.
 \param pt2 second point.
 \param a Output parameter, a coefficient of the equation.
 \param b Output parameter, b coefficient of the equation.
 \param c Output parameter, c coefficient of the equation.
.. versionadded:: 3.0
%End

    static QgsLineString perpendicularSegment( const QgsPointV2 &p, const QgsPointV2 &s1, const QgsPointV2 &s2 );
%Docstring
 Create a perpendicular line segment from p to segment [s1, s2]
 \param p The point
 \param s1 The segment start point
 \param s2 The segment end point
 :return: A line (segment) from p to perpendicular point on segment [s1, s2]
 :rtype: QgsLineString
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometryutils.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
