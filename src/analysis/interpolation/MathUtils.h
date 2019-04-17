/***************************************************************************
                          MathUtils.h  -  description
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

#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <cmath>
#include "qgis_analysis.h"

class QgsPoint;
class Vector3D;

#define SIP_NO_FILE

namespace MathUtils
{
  //! Calculates the barycentric coordinates of a point (x,y) with respect to p1, p2, p3 and stores the three barycentric coordinates in 'result'. Thus the u-coordinate is stored in result::x, the v-coordinate in result::y and the w-coordinate in result::z. Attention: p1, p2 and p3 have to be ordered counterclockwise
  bool ANALYSIS_EXPORT calcBarycentricCoordinates( double x, double y, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result );
  bool ANALYSIS_EXPORT BarycentricToXY( double u, double v, double w, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result );
  //! Calculates the value of a Bernstein polynomial
  double ANALYSIS_EXPORT calcBernsteinPoly( int n, int i, double t );
  //! Calculates the first derivative of a Bernstein polynomial with respect to the parameter t
  double ANALYSIS_EXPORT cFDerBernsteinPoly( int n, int i, double t );
  //! Calculates the value of a cubic Hermite polynomial
  double ANALYSIS_EXPORT calcCubicHermitePoly( int n, int i, double t );
  //! Calculates the first derivative of a cubic Hermite polynomial with respect to the parameter t
  double ANALYSIS_EXPORT cFDerCubicHermitePoly( int n, int i, double t );
  //! Calculates the center of the circle passing through p1, p2 and p3. Returns TRUE in case of success and FALSE otherwise (e.g. all three points on a line)
  bool ANALYSIS_EXPORT circumcenter( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result );
  //! Calculates the (2 dimensional) distance from 'thepoint' to the line defined by p1 and p2
  double ANALYSIS_EXPORT distPointFromLine( QgsPoint *thepoint, QgsPoint *p1, QgsPoint *p2 );
  //! Faculty function
  int ANALYSIS_EXPORT faculty( int n );
  //! Tests, whether 'testp' is inside the circle through 'p1', 'p2' and 'p3'
  bool ANALYSIS_EXPORT inCircle( QgsPoint *testp, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3 );
  //! Tests, whether 'point' is inside the diametral circle through 'p1' and 'p2'
  bool ANALYSIS_EXPORT inDiametral( QgsPoint *p1, QgsPoint *p2, QgsPoint *point );
  //! Returns whether 'thepoint' is left or right of the line from 'p1' to 'p2'. Negativ values mean left and positiv values right. There may be numerical instabilities, so a threshold may be useful
  double ANALYSIS_EXPORT leftOf( const QgsPoint &thepoint, const QgsPoint *p1, const QgsPoint *p2 );
  //! Returns TRUE, if line1 (p1 to p2) and line2 (p3 to p4) intersect. If the lines have an endpoint in common, FALSE is returned
  bool ANALYSIS_EXPORT lineIntersection( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4 );
  //! Returns TRUE, if line1 (p1 to p2) and line2 (p3 to p4) intersect. If the lines have an endpoint in common, FALSE is returned. The intersecting point is stored in 'intersection_point.
  bool ANALYSIS_EXPORT lineIntersection( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4, QgsPoint *intersection_point );
  //! Lower function
  int ANALYSIS_EXPORT lower( int n, int i );
  //! Returns the area of a triangle. If the points are ordered counterclockwise, the value will be positiv. If they are ordered clockwise, the value will be negativ
  double ANALYSIS_EXPORT triArea( QgsPoint *pa, QgsPoint *pb, QgsPoint *pc );
  //! Calculates the z-component of a vector with coordinates 'x' and 'y'which is in the same tangent plane as the tangent vectors 'v1' and 'v2'. The result is assigned to 'result'
  bool ANALYSIS_EXPORT derVec( const Vector3D *v1, const Vector3D *v2, Vector3D *result, double x, double y );
  //! Calculates the intersection of the two vectors vec1 and vec2, which start at first(vec1) and second(vec2) end. The return value is t2(multiplication of v2 with t2 and adding to 'second' results the intersection point)
  double ANALYSIS_EXPORT crossVec( QgsPoint *first, Vector3D *vec1, QgsPoint *second, Vector3D *vec2 );
  //! Assigns the vector 'result', which is normal to the vector 'v1', on the left side of v1 and has length 'length'. This method works only with two dimensions.
  bool ANALYSIS_EXPORT normalLeft( Vector3D *v1, Vector3D *result, double length );
  //! Assigns the vector 'result', which is normal to the vector 'v1', on the right side of v1 and has length 'length'. The calculation is only in two dimensions
  bool ANALYSIS_EXPORT normalRight( Vector3D *v1, Vector3D *result, double length );
  //! Calculates the normal vector of the plane through the points p1, p2 and p3 and assigns the result to vec. If the points are ordered counterclockwise, the normal will have a positive z-coordinate;
  void ANALYSIS_EXPORT normalFromPoints( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, Vector3D *vec );
  //! Returns TRUE, if the point with coordinates x and y is inside (or at the edge) of the triangle p1,p2,p3 and FALSE, if it is outside. p1, p2 and p3 have to be ordered counterclockwise
  bool ANALYSIS_EXPORT pointInsideTriangle( double x, double y, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3 );
  //! Calculates a Vector orthogonal to 'tangent' with length 1 and closest possible to result. Returns TRUE in case of success and FALSE otherwise
  bool ANALYSIS_EXPORT normalMinDistance( Vector3D *tangent, Vector3D *target, Vector3D *result );
  //! Tests, if 'test' is in the same plane as 'p1', 'p2' and 'p3' and returns the z-difference from the plane to 'test
  double ANALYSIS_EXPORT planeTest( QgsPoint *test, QgsPoint *pt1, QgsPoint *pt2, QgsPoint *pt3 );
  //! Calculates the angle between two segments (in 2 dimension, z-values are ignored)
  double ANALYSIS_EXPORT angle( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4 );
}

#endif
