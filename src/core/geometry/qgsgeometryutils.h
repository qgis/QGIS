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

#include "qgspointv2.h"
#include <limits>

class QgsLineStringV2;

/** \ingroup core
 * \class QgsGeometryUtils
 * \brief Contains various geometry utility functions.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsGeometryUtils
{
  public:

    /** Returns list of linestrings extracted from the passed geometry. The returned objects
     *  have to be deleted by the caller.
     */
    static QList<QgsLineStringV2*> extractLineStrings( const QgsAbstractGeometryV2* geom );

    /** Returns the closest vertex to a geometry for a specified point
     */
    static QgsPointV2 closestVertex( const QgsAbstractGeometryV2& geom, const QgsPointV2& pt, QgsVertexId& id );

    /** Returns the distance along a geometry from its first vertex to the specified vertex.
     * @param geom geometry
     * @param id vertex id to find distance to
     * @returns distance to vertex (following geometry)
     * @note added in QGIS 2.16
     */
    static double distanceToVertex( const QgsAbstractGeometryV2& geom, const QgsVertexId& id );

    /** Returns vertices adjacent to a specified vertex within a geometry.
     */
    static void adjacentVertices( const QgsAbstractGeometryV2& geom, QgsVertexId atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex );

    /** Returns the squared 2D distance between two points.
     */
    static double sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 );

    /** Returns the squared distance between a point and a line.
     */
    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX, double& minDistY, double epsilon );

    /**
     * @brief Compute the intersection between two lines
     * @param p1 Point on the first line
     * @param v Direction vector of the first line
     * @param q1 Point on the second line
     * @param w Direction vector of the second line
     * @param inter Output parameter, the intersection point
     * @return Whether the lines intersect
     */
    static bool lineIntersection( const QgsPointV2& p1, QgsVector v, const QgsPointV2& q1, QgsVector w, QgsPointV2& inter );

    /**
     * @brief Compute the intersection between two segments
     * @param p1 First segment start point
     * @param p2 First segment end point
     * @param q1 Second segment start point
     * @param q2 Second segment end point
     * @param inter Output parameter, the intersection point
     * @param tolerance The tolerance to use
     * @return  Whether the segments intersect
     */
    static bool segmentIntersection( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &q1, const QgsPointV2 &q2, QgsPointV2& inter, double tolerance );

    /**
     * @brief Project the point on a segment
     * @param p The point
     * @param s1 The segment start point
     * @param s2 The segment end point
     * @return The projection of the point on the segment
     */
    static QgsPointV2 projPointOnSegment( const QgsPointV2& p, const QgsPointV2& s1, const QgsPointV2& s2 )
    {
      double nx = s2.y() - s1.y();
      double ny = -( s2.x() - s1.x() );
      double t = ( p.x() * ny - p.y() * nx - s1.x() * ny + s1.y() * nx ) / (( s2.x() - s1.x() ) * ny - ( s2.y() - s1.y() ) * nx );
      return t < 0. ? s1 : t > 1. ? s2 : QgsPointV2( s1.x() + ( s2.x() - s1.x() ) * t, s1.y() + ( s2.y() - s1.y() ) * t );
    }

    struct SelfIntersection
    {
      int segment1;
      int segment2;
      QgsPointV2 point;
    };

    /**
     * @brief Find self intersections in a polyline
     * @param geom The geometry to check
     * @param part The part of the geometry to check
     * @param ring The ring of the geometry part to check
     * @param tolerance The tolerance to use
     * @return The list of self intersections
     * @note added in QGIS 2.12
     */
    static QList<SelfIntersection> getSelfIntersections( const QgsAbstractGeometryV2* geom, int part, int ring, double tolerance );

    /** Returns < 0 if point(x/y) is left of the line x1,y1 -> x2,y2*/
    static double leftOfLine( double x, double y, double x1, double y1, double x2, double y2 );

    /** Returns a point a specified distance toward a second point.
     */
    static QgsPointV2 pointOnLineWithDistance( const QgsPointV2& startPoint, const QgsPointV2& directionPoint, double distance );

    /** Returns the counter clockwise angle between a line with components dx, dy and the line with dx > 0 and dy = 0*/
    static double ccwAngle( double dy, double dx );

    /** Returns radius and center of the circle through pt1, pt2, pt3*/
    static void circleCenterRadius( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3, double& radius,
                                    double& centerX, double& centerY );

    /** Returns true if circle is ordered clockwise*/
    static bool circleClockwise( double angle1, double angle2, double angle3 );

    /** Returns true if, in a circle, angle is between angle1 and angle2*/
    static bool circleAngleBetween( double angle, double angle1, double angle2, bool clockwise );

    /** Returns true if an angle is between angle1 and angle3 on a circle described by
     * angle1, angle2 and angle3.
     */
    static bool angleOnCircle( double angle, double angle1, double angle2, double angle3 );

    /** Length of a circular string segment defined by pt1, pt2, pt3*/
    static double circleLength( double x1, double y1, double x2, double y2, double x3, double y3 );

    /** Calculates angle of a circular string part defined by pt1, pt2, pt3*/
    static double sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 );

    /** Calculates midpoint on circle passing through p1 and p2, closest to given coordinate*/
    static bool segmentMidPoint( const QgsPointV2& p1, const QgsPointV2& p2, QgsPointV2& result, double radius, const QgsPointV2& mousePos );

    /** Calculates the direction angle of a circle tangent (clockwise from north in radians)*/
    static double circleTangentDirection( const QgsPointV2& tangentPoint, const QgsPointV2& cp1, const QgsPointV2& cp2, const QgsPointV2& cp3 );

    /** Returns a list of points contained in a WKT string.
     */
    static QgsPointSequenceV2 pointsFromWKT( const QString& wktCoordinateList, bool is3D, bool isMeasure );
    /** Returns a LinearRing { uint32 numPoints; Point points[numPoints]; } */
    static void pointsToWKB( QgsWkbPtr &wkb, const QgsPointSequenceV2 &points, bool is3D, bool isMeasure );
    /** Returns a WKT coordinate list */
    static QString pointsToWKT( const QgsPointSequenceV2 &points, int precision, bool is3D, bool isMeasure );
    /** Returns a gml::coordinates DOM element */
    static QDomElement pointsToGML2( const QgsPointSequenceV2 &points, QDomDocument &doc, int precision, const QString& ns );
    /** Returns a gml::posList DOM element */
    static QDomElement pointsToGML3( const QgsPointSequenceV2 &points, QDomDocument &doc, int precision, const QString& ns, bool is3D );
    /** Returns a geoJSON coordinates string */
    static QString pointsToJSON( const QgsPointSequenceV2 &points, int precision );

    /** Ensures that an angle is in the range 0 <= angle < 2 pi.
     * @param angle angle in radians
     * @returns equivalent angle within the range [0, 2 pi)
     */
    static double normalizedAngle( double angle );

    /** Calculates the direction of line joining two points in radians, clockwise from the north direction.
     * @param x1 x-coordinate of line start
     * @param y1 y-coordinate of line start
     * @param x2 x-coordinate of line end
     * @param y2 y-coordinate of line end
     * @returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double lineAngle( double x1, double y1, double x2, double y2 );

    /** Calculates the perpendicular angle to a line joining two points. Returned angle is in radians,
     * clockwise from the north direction.
     * @param x1 x-coordinate of line start
     * @param y1 y-coordinate of line start
     * @param x2 x-coordinate of line end
     * @param y2 y-coordinate of line end
     * @returns angle in radians. Returned value is undefined if start and end point are the same.
     */
    static double linePerpendicularAngle( double x1, double y1, double x2, double y2 );

    /** Angle between two linear segments*/
    static double averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 );

    /** Averages two angles, correctly handling negative angles and ensuring the result is between 0 and 2 pi.
     * @param a1 first angle (in radians)
     * @param a2 second angle (in radians)
     * @returns average angle (in radians)
     */
    static double averageAngle( double a1, double a2 );

    /** Parses a WKT block of the format "TYPE( contents )" and returns a pair of geometry type to contents ("Pair(wkbType, "contents")")
     */
    static QPair<QgsWKBTypes::Type, QString> wktReadBlock( const QString& wkt );

    /** Parses a WKT string and returns of list of blocks contained in the WKT.
     * @param wkt WKT string in the format "TYPE1 (contents1), TYPE2 (TYPE3 (contents3), TYPE4 (contents4))"
     * @param defaultType default geometry type for childen
     * @returns list of WKT child block strings, eg List("TYPE1 (contents1)", "TYPE2 (TYPE3 (contents3), TYPE4 (contents4))")
     */
    static QStringList wktGetChildBlocks( const QString& wkt , const QString &defaultType = "" );

    enum componentType
    {
      VERTEX,
      RING,
      PART
    };

    template<class T> static double closestSegmentFromComponents( T& container, componentType ctype, const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon )
    {
      double minDist = std::numeric_limits<double>::max();
      double minDistSegmentX = 0.0, minDistSegmentY = 0.0;
      QgsVertexId minDistVertexAfter;
      bool minDistLeftOf = false;
      double sqrDist = 0.0;
      int vertexOffset = 0;
      int ringOffset = 0;
      int partOffset = 0;

      for ( int i = 0; i < container.size(); ++i )
      {
        sqrDist = container.at( i )->closestSegment( pt, segmentPt, vertexAfter, leftOf, epsilon );
        if ( sqrDist < minDist )
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

        if ( ctype == VERTEX )
        {
          //-1 because compoundcurve counts duplicated vertices of neighbour curves as one node
          vertexOffset += container.at( i )->nCoordinates() - 1;
        }
        else if ( ctype == RING )
        {
          ringOffset += 1;
        }
        else if ( ctype == PART )
        {
          partOffset += 1;
        }
      }

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
