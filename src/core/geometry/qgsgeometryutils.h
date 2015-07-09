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

/**\ingroup core
 * \class QgsGeometryUtils
 * \brief Contains various geometry utility functions.
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsGeometryUtils
{
  public:

    /** Returns the closest vertex to a geometry for a specified point
     */
    static QgsPointV2 closestVertex( const QgsAbstractGeometryV2& geom, const QgsPointV2& pt, QgsVertexId& id );

    /** Returns vertices adjacent to a specified vertex within a geometry.
     */
    static void adjacentVertices( const QgsAbstractGeometryV2& geom, const QgsVertexId& atVertex, QgsVertexId& beforeVertex, QgsVertexId& afterVertex );

    /** Returns the squared 2D distance between two points.
     */
    static double sqrDistance2D( const QgsPointV2& pt1, const QgsPointV2& pt2 );

    /** Returns the squared distance between a point and a line.
     */
    static double sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double& minDistX, double& minDistY, double epsilon );

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

    /** Returns a list of points contained in a WKT string.
     */
    static QList<QgsPointV2> pointsFromWKT( const QString& wktCoordinateList, bool is3D, bool isMeasure );
    /** Returns a LinearRing { uint32 numPoints; Point points[numPoints]; } */
    static void pointsToWKB( QgsWkbPtr &wkb, const QList<QgsPointV2>& points, bool is3D, bool isMeasure );
    /** Returns a WKT coordinate list */
    static QString pointsToWKT( const QList<QgsPointV2>& points, int precision, bool is3D, bool isMeasure );
    /** Returns a gml::coordinates DOM element */
    static QDomElement pointsToGML2( const QList<QgsPointV2>& points, QDomDocument &doc, int precision, const QString& ns );
    /** Returns a gml::posList DOM element */
    static QDomElement pointsToGML3( const QList<QgsPointV2>& points, QDomDocument &doc, int precision, const QString& ns, bool is3D );
    /** Returns a geoJSON coordinates string */
    static QString pointsToJSON( const QList<QgsPointV2>& points, int precision );

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
