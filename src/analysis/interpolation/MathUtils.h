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

#ifndef Q_OS_MACX
#include <cmath>
#else
#include <math.h>
#endif
#include "Vector3D.h"
#include "Point3D.h"


namespace ANALYSIS_EXPORT MathUtils
{
  /**Calculates the barycentric coordinates of a point (x,y) with respect to p1, p2, p3 and stores the three barycentric coordinates in 'result'. Thus the u-coordinate is stored in result::x, the v-coordinate in result::y and the w-coordinate in result::z. Attention: p1, p2 and p3 have to be ordered counterclockwise*/
  bool calcBarycentricCoordinates( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result );
  bool BarycentricToXY( double u, double v, double w, Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result );
  /**calculates the value of a Bernstein polynomial*/
  double calcBernsteinPoly( int n, int i, double t );
  /**calculates the first derivative of a Bernstein polynomial with respect to the parameter t*/
  double cFDerBernsteinPoly( int n, int i, double t );
  /**calculates the value of a cubic Hermite polynomial*/
  double calcCubicHermitePoly( int n, int i, double t );
  /**calculates the first derivative of a cubic Hermite polynomial with respect to the parameter t*/
  double cFDerCubicHermitePoly( int n, int i, double t );
  /**Calculates the center of the circle passing through p1, p2 and p3. Returns true in case of success and false otherwise (e.g. all three points on a line)*/
  bool circumcenter( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result );
  /**Calculates the (2 dimensional) distance from 'thepoint' to the line defined by p1 and p2*/
  double distPointFromLine( Point3D* thepoint, Point3D* p1, Point3D* p2 );
  /**faculty function*/
  int faculty( int n );
  /**Tests, wheter 'testp' is inside the circle through 'p1', 'p2' and 'p3'*/
  bool inCircle( Point3D* testp, Point3D* p1, Point3D* p2, Point3D* p3 );
  /**Tests, whether 'point' is inside the diametral circle through 'p1' and 'p2'*/
  bool inDiametral( Point3D* p1, Point3D* p2, Point3D* point );
  /**Returns whether 'thepoint' is left or right of the line from 'p1' to 'p2'. Negativ values mean left and positiv values right. There may be numerical instabilities, so a treshold may be useful*/
  double leftOf( Point3D* thepoint, Point3D* p1, Point3D* p2 );
  /**Returns true, if line1 (p1 to p2) and line2 (p3 to p4) intersect. If the lines have an endpoint in common, false is returned*/
  bool lineIntersection( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4 );
  /**Returns true, if line1 (p1 to p2) and line2 (p3 to p4) intersect. If the lines have an endpoint in common, false is returned. The intersecting point is stored in 'intersection_point.*/
  bool lineIntersection( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4, Point3D* intersection_point );
  /**lower function*/
  int lower( int n, int i );
  /**returns the maximum of two doubles or the first argument if both are equal*/
  double max( double x, double y );
  /**returns the minimum of two doubles or the first argument if both are equal*/
  double min( double x, double y );
  /**power function for integer coefficients*/
  double power( double a, int b );//calculates a power b
  /**returns the area of a triangle. If the points are ordered counterclockwise, the value will be positiv. If they are ordered clockwise, the value will be negativ*/
  double triArea( Point3D* pa, Point3D* pb, Point3D* pc );
  /**Calculates the z-component of a vector with coordinates 'x' and 'y'which is in the same tangent plane as the tangent vectors 'v1' and 'v2'. The result is assigned to 'result' */
  bool derVec( const Vector3D* v1, const Vector3D* v2, Vector3D* result, double x, double y );
  /**Calculates the intersection of the two vectors vec1 and vec2, which start at first(vec1) and second(vec2) end. The return value is t2(multiplication of v2 with t2 and adding to 'second' results the intersection point)*/
  double crossVec( Point3D* first, Vector3D* vec1, Point3D* second, Vector3D* vec2 );
  /**Assigns the vector 'result', which is normal to the vector 'v1', on the left side of v1 and has length 'length'. This method works only with two dimensions.*/
  bool normalLeft( Vector3D* v1, Vector3D* result, double length );
  /**Assigns the vector 'result', which is normal to the vector 'v1', on the right side of v1 and has length 'length'. The calculation is only in two dimensions*/
  bool normalRight( Vector3D* v1, Vector3D* result, double length );
  /**Calculates the normal vector of the plane through the points p1, p2 and p3 and assigns the result to vec. If the points are ordered counterclockwise, the normal will have a positive z-coordinate;*/
  void normalFromPoints( Point3D* p1, Point3D* p2, Point3D* p3, Vector3D* vec );
  /**Returns true, if the point with coordinates x and y is inside (or at the edge) of the triangle p1,p2,p3 and false, if it is outside. p1, p2 and p3 have to be ordered counterclockwise*/
  bool pointInsideTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 );
  /**Calculates a Vector orthogonal to 'tangent' with length 1 and closest possible to result. Returns true in case of success and false otherwise*/
  bool normalMinDistance( Vector3D* tangent, Vector3D* target, Vector3D* result );
  /**Tests, if 'test' is in the same plane as 'p1', 'p2' and 'p3' and returns the z-difference from the plane to 'test*/
  double planeTest( Point3D* test, Point3D* pt1, Point3D* pt2, Point3D* pt3 );
  /**Calculates the angle between two segments (in 2 dimension, z-values are ignored)*/
  double angle( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4 );
}

#endif
