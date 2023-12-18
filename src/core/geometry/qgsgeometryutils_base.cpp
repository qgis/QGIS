/***************************************************************************
                        qgsgeometryutils_base.cpp
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

#include "qgsgeometryutils_base.h"
#include "qgsvector3d.h"
#include "qgsvector.h"

double QgsGeometryUtilsBase::sqrDistToLine( double ptX, double ptY, double x1, double y1, double x2, double y2, double &minDistX, double &minDistY, double epsilon )
{
  minDistX = x1;
  minDistY = y1;

  double dx = x2 - x1;
  double dy = y2 - y1;

  if ( !qgsDoubleNear( dx, 0.0 ) || !qgsDoubleNear( dy, 0.0 ) )
  {
    const double t = ( ( ptX - x1 ) * dx + ( ptY - y1 ) * dy ) / ( dx * dx + dy * dy );
    if ( t > 1 )
    {
      minDistX = x2;
      minDistY = y2;
    }
    else if ( t > 0 )
    {
      minDistX += dx * t;
      minDistY += dy * t;
    }
  }

  dx = ptX - minDistX;
  dy = ptY - minDistY;

  const double dist = dx * dx + dy * dy;

  //prevent rounding errors if the point is directly on the segment
  if ( qgsDoubleNear( dist, 0.0, epsilon ) )
  {
    minDistX = ptX;
    minDistY = ptY;
    return 0.0;
  }

  return dist;
}

int QgsGeometryUtilsBase::leftOfLine( const double x, const double y, const double x1, const double y1, const double x2, const double y2 )
{
  const double f1 = x - x1;
  const double f2 = y2 - y1;
  const double f3 = y - y1;
  const double f4 = x2 - x1;
  const double test = ( f1 * f2 - f3 * f4 );
  // return -1, 0, or 1
  return qgsDoubleNear( test, 0.0 ) ? 0 : ( test < 0 ? -1 : 1 );
}

void QgsGeometryUtilsBase::pointOnLineWithDistance( double x1, double y1, double x2, double y2, double distance, double &x, double &y, double *z1, double *z2, double *z, double *m1, double *m2, double *m )
{
  const double dx = x2 - x1;
  const double dy = y2 - y1;
  const double length = std::sqrt( dx * dx + dy * dy );

  if ( qgsDoubleNear( length, 0.0 ) )
  {
    x = x1;
    y = y1;
    if ( z && z1 )
      *z = *z1;
    if ( m && m1 )
      *m = *m1;
  }
  else
  {
    const double scaleFactor = distance / length;
    x = x1 + dx * scaleFactor;
    y = y1 + dy * scaleFactor;
    if ( z && z1 && z2 )
      *z = *z1 + ( *z2 - *z1 ) * scaleFactor;
    if ( m && m1 && m2 )
      *m = *m1 + ( *m2 - *m1 ) * scaleFactor;
  }
}

void QgsGeometryUtilsBase::perpendicularOffsetPointAlongSegment( double x1, double y1, double x2, double y2, double proportion, double offset, double *x, double *y )
{
  // calculate point along segment
  const double mX = x1 + ( x2 - x1 ) * proportion;
  const double mY = y1 + ( y2 - y1 ) * proportion;
  const double pX = x1 - x2;
  const double pY = y1 - y2;
  double normalX = -pY;
  double normalY = pX;  //#spellok
  const double normalLength = sqrt( ( normalX * normalX ) + ( normalY * normalY ) );  //#spellok
  normalX /= normalLength;
  normalY /= normalLength;  //#spellok

  *x = mX + offset * normalX;
  *y = mY + offset * normalY;  //#spellok
}

double QgsGeometryUtilsBase::ccwAngle( double dy, double dx )
{
  const double angle = std::atan2( dy, dx ) * 180 / M_PI;
  if ( angle < 0 )
  {
    return 360 + angle;
  }
  else if ( angle > 360 )
  {
    return 360 - angle;
  }
  return angle;
}

bool QgsGeometryUtilsBase::circleClockwise( double angle1, double angle2, double angle3 )
{
  if ( angle3 >= angle1 )
  {
    return !( angle2 > angle1 && angle2 < angle3 );
  }
  else
  {
    return !( angle2 > angle1 || angle2 < angle3 );
  }
}

bool QgsGeometryUtilsBase::circleAngleBetween( double angle, double angle1, double angle2, bool clockwise )
{
  if ( clockwise )
  {
    if ( angle2 < angle1 )
    {
      return ( angle <= angle1 && angle >= angle2 );
    }
    else
    {
      return ( angle <= angle1 || angle >= angle2 );
    }
  }
  else
  {
    if ( angle2 > angle1 )
    {
      return ( angle >= angle1 && angle <= angle2 );
    }
    else
    {
      return ( angle >= angle1 || angle <= angle2 );
    }
  }
}

bool QgsGeometryUtilsBase::angleOnCircle( double angle, double angle1, double angle2, double angle3 )
{
  const bool clockwise = circleClockwise( angle1, angle2, angle3 );
  return circleAngleBetween( angle, angle1, angle3, clockwise );
}

void QgsGeometryUtilsBase::circleCenterRadius( double x1, double y1, double x2, double y2, double x3, double y3, double &radius, double &centerX, double &centerY )
{
  double dx21, dy21, dx31, dy31, h21, h31, d;

  //closed circle
  if ( qgsDoubleNear( x1, x3 ) && qgsDoubleNear( y1, y3 ) )
  {
    centerX = ( x1 + x2 ) / 2.0;
    centerY = ( y1 + y2 ) / 2.0;
    radius = std::sqrt( std::pow( centerX - x1, 2.0 ) + std::pow( centerY - y1, 2.0 ) );
    return;
  }

  // Using Cartesian circumcenter eguations from page https://en.wikipedia.org/wiki/Circumscribed_circle
  dx21 = x2 - x1;
  dy21 = y2 - y1;
  dx31 = x3 - x1;
  dy31 = y3 - y1;

  h21 = std::pow( dx21, 2.0 ) + std::pow( dy21, 2.0 );
  h31 = std::pow( dx31, 2.0 ) + std::pow( dy31, 2.0 );

  // 2*Cross product, d<0 means clockwise and d>0 counterclockwise sweeping angle
  d = 2 * ( dx21 * dy31 - dx31 * dy21 );

  // Check colinearity, Cross product = 0
  if ( qgsDoubleNear( std::fabs( d ), 0.0, 0.00000000001 ) )
  {
    radius = -1.0;
    return;
  }

  // Calculate centroid coordinates and radius
  centerX = x1 + ( h21 * dy31 - h31 * dy21 ) / d;
  centerY = y1 - ( h21 * dx31 - h31 * dx21 ) / d;
  radius = std::sqrt( std::pow( centerX - x1, 2.0 ) + std::pow( centerY - y1, 2.0 ) );
}

double QgsGeometryUtilsBase::circleLength( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  double centerX{0.0};
  double centerY{0.0};
  double radius{0.0};
  circleCenterRadius( x1, y1, x2, y2, x3, y3, radius, centerX, centerY );
  double length = M_PI / 180.0 * radius * sweepAngle( centerX, centerY, x1, y1, x2, y2, x3, y3 );
  if ( length < 0 )
  {
    length = -length;
  }
  return length;
}

double QgsGeometryUtilsBase::sweepAngle( double centerX, double centerY, double x1, double y1, double x2, double y2, double x3, double y3 )
{
  const double p1Angle = QgsGeometryUtilsBase::ccwAngle( y1 - centerY, x1 - centerX );
  const double p2Angle = QgsGeometryUtilsBase::ccwAngle( y2 - centerY, x2 - centerX );
  const double p3Angle = QgsGeometryUtilsBase::ccwAngle( y3 - centerY, x3 - centerX );

  if ( p3Angle >= p1Angle )
  {
    if ( p2Angle > p1Angle && p2Angle < p3Angle )
    {
      return ( p3Angle - p1Angle );
    }
    else
    {
      return ( - ( p1Angle + ( 360 - p3Angle ) ) );
    }
  }
  else
  {
    if ( p2Angle < p1Angle && p2Angle > p3Angle )
    {
      return ( -( p1Angle - p3Angle ) );
    }
    else
    {
      return ( p3Angle + ( 360 - p1Angle ) );
    }
  }
}

double QgsGeometryUtilsBase::interpolateArcValue( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 )
{
  /* Counter-clockwise sweep */
  if ( a1 < a2 )
  {
    if ( angle <= a2 )
      return zm1 + ( zm2 - zm1 ) * ( angle - a1 ) / ( a2 - a1 );
    else
      return zm2 + ( zm3 - zm2 ) * ( angle - a2 ) / ( a3 - a2 );
  }
  /* Clockwise sweep */
  else
  {
    if ( angle >= a2 )
      return zm1 + ( zm2 - zm1 ) * ( a1 - angle ) / ( a1 - a2 );
    else
      return zm2 + ( zm3 - zm2 ) * ( a2 - angle ) / ( a2 - a3 );
  }
}
double QgsGeometryUtilsBase::normalizedAngle( double angle )
{
  double clippedAngle = angle;
  if ( clippedAngle >= M_PI * 2 || clippedAngle <= -2 * M_PI )
  {
    clippedAngle = std::fmod( clippedAngle, 2 * M_PI );
  }
  if ( clippedAngle < 0.0 )
  {
    clippedAngle += 2 * M_PI;
  }
  return clippedAngle;
}


int QgsGeometryUtilsBase::closestSideOfRectangle( double right, double bottom, double left, double top, double x, double y )
{
  // point outside rectangle
  if ( x <= left && y <= bottom )
  {
    const double dx = left - x;
    const double dy = bottom - y;
    if ( qgsDoubleNear( dx, dy ) )
      return 6;
    else if ( dx < dy )
      return 5;
    else
      return 7;
  }
  else if ( x >= right && y >= top )
  {
    const double dx = x - right;
    const double dy = y - top;
    if ( qgsDoubleNear( dx, dy ) )
      return 2;
    else if ( dx < dy )
      return 1;
    else
      return 3;
  }
  else if ( x >= right && y <= bottom )
  {
    const double dx = x - right;
    const double dy = bottom - y;
    if ( qgsDoubleNear( dx, dy ) )
      return 4;
    else if ( dx < dy )
      return 5;
    else
      return 3;
  }
  else if ( x <= left && y >= top )
  {
    const double dx = left - x;
    const double dy = y - top;
    if ( qgsDoubleNear( dx, dy ) )
      return 8;
    else if ( dx < dy )
      return 1;
    else
      return 7;
  }
  else if ( x <= left )
    return 7;
  else if ( x >= right )
    return 3;
  else if ( y <= bottom )
    return 5;
  else if ( y >= top )
    return 1;

  // point is inside rectangle
  const double smallestX = std::min( right - x, x - left );
  const double smallestY = std::min( top - y, y - bottom );
  if ( smallestX < smallestY )
  {
    // closer to left/right side
    if ( right - x < x - left )
      return 3; // closest to right side
    else
      return 7;
  }
  else
  {
    // closer to top/bottom side
    if ( top - y < y - bottom )
      return 1; // closest to top side
    else
      return 5;
  }
}

void QgsGeometryUtilsBase::perpendicularCenterSegment( double pointx, double pointy, double segmentPoint1x, double segmentPoint1y, double segmentPoint2x, double segmentPoint2y, double &perpendicularSegmentPoint1x, double &perpendicularSegmentPoint1y, double &perpendicularSegmentPoint2x, double &perpendicularSegmentPoint2y, double desiredSegmentLength )
{
  QgsVector segmentVector =  QgsVector( segmentPoint2x - segmentPoint1x, segmentPoint2y - segmentPoint1y );
  QgsVector perpendicularVector = segmentVector.perpVector();
  if ( desiredSegmentLength != 0 )
  {
    perpendicularVector = perpendicularVector.normalized() * ( desiredSegmentLength ) / 2;
  }

  perpendicularSegmentPoint1x = pointx - perpendicularVector.x();
  perpendicularSegmentPoint1y = pointy - perpendicularVector.y();
  perpendicularSegmentPoint2x = pointx + perpendicularVector.x();
  perpendicularSegmentPoint2y = pointy + perpendicularVector.y();
}

double QgsGeometryUtilsBase::lineAngle( double x1, double y1, double x2, double y2 )
{
  const double at = std::atan2( y2 - y1, x2 - x1 );
  const double a = -at + M_PI_2;
  return normalizedAngle( a );
}

double QgsGeometryUtilsBase::angleBetweenThreePoints( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  const double angle1 = std::atan2( y1 - y2, x1 - x2 );
  const double angle2 = std::atan2( y3 - y2, x3 - x2 );
  return normalizedAngle( angle1 - angle2 );
}

double QgsGeometryUtilsBase::linePerpendicularAngle( double x1, double y1, double x2, double y2 )
{
  double a = lineAngle( x1, y1, x2, y2 );
  a += M_PI_2;
  return normalizedAngle( a );
}

double QgsGeometryUtilsBase::averageAngle( double x1, double y1, double x2, double y2, double x3, double y3 )
{
  // calc average angle between the previous and next point
  const double a1 = lineAngle( x1, y1, x2, y2 );
  const double a2 = lineAngle( x2, y2, x3, y3 );
  return averageAngle( a1, a2 );
}

double QgsGeometryUtilsBase::averageAngle( double a1, double a2 )
{
  a1 = normalizedAngle( a1 );
  a2 = normalizedAngle( a2 );
  double clockwiseDiff = 0.0;
  if ( a2 >= a1 )
  {
    clockwiseDiff = a2 - a1;
  }
  else
  {
    clockwiseDiff = a2 + ( 2 * M_PI - a1 );
  }
  const double counterClockwiseDiff = 2 * M_PI - clockwiseDiff;

  double resultAngle = 0;
  if ( clockwiseDiff <= counterClockwiseDiff )
  {
    resultAngle = a1 + clockwiseDiff / 2.0;
  }
  else
  {
    resultAngle = a1 - counterClockwiseDiff / 2.0;
  }
  return normalizedAngle( resultAngle );
}

double QgsGeometryUtilsBase::skewLinesDistance( const QgsVector3D &P1, const QgsVector3D &P12,
    const QgsVector3D &P2, const QgsVector3D &P22 )
{
  const QgsVector3D u1 = P12 - P1;
  const QgsVector3D u2 = P22 - P2;
  QgsVector3D u3 = QgsVector3D::crossProduct( u1, u2 );
  if ( u3.length() == 0 ) return 1;
  u3.normalize();
  const QgsVector3D dir = P1 - P2;
  return std::fabs( ( QgsVector3D::dotProduct( dir, u3 ) ) ); // u3 is already normalized
}

bool QgsGeometryUtilsBase::skewLinesProjection( const QgsVector3D &P1, const QgsVector3D &P12,
    const QgsVector3D &P2, const QgsVector3D &P22,
    QgsVector3D &X1, double epsilon )
{
  const QgsVector3D d = P2 - P1;
  QgsVector3D u1 = P12 - P1;
  u1.normalize();
  QgsVector3D u2 = P22 - P2;
  u2.normalize();
  const QgsVector3D u3 = QgsVector3D::crossProduct( u1, u2 );

  if ( std::fabs( u3.x() ) <= epsilon &&
       std::fabs( u3.y() ) <= epsilon &&
       std::fabs( u3.z() ) <= epsilon )
  {
    // The rays are almost parallel.
    return false;
  }

  // X1 and X2 are the closest points on lines
  // we want to find X1 (lies on u1)
  // solving the linear equation in r1 and r2: Xi = Pi + ri*ui
  // we are only interested in X1 so we only solve for r1.
  const double a1 = QgsVector3D::dotProduct( u1, u1 ), b1 = QgsVector3D::dotProduct( u1, u2 ), c1 = QgsVector3D::dotProduct( u1, d );
  const double a2 = QgsVector3D::dotProduct( u1, u2 ), b2 = QgsVector3D::dotProduct( u2, u2 ), c2 = QgsVector3D::dotProduct( u2, d );
  if ( !( std::fabs( b1 ) > epsilon ) )
  {
    // Denominator is close to zero.
    return false;
  }
  if ( !( a2 != -1 && a2 != 1 ) )
  {
    // Lines are parallel
    return false;
  }

  const double r1 = ( c2 - b2 * c1 / b1 ) / ( a2 - b2 * a1 / b1 );
  X1 = P1 + u1 * r1;

  return true;
}

bool QgsGeometryUtilsBase::lineIntersection( double p1x, double p1y, QgsVector v1, double p2x, double p2y, QgsVector v2, double &intersectionX, double &intersectionY )
{
  const double d = v1.y() * v2.x() - v1.x() * v2.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  const double dx = p2x - p1x;
  const double dy = p2y - p1y;
  const double k = ( dy * v2.x() - dx * v2.y() ) / d;

  intersectionX = p1x + v1.x() * k;
  intersectionY = p1y + v1.y() * k;

  return true;
}

static bool equals( double p1x, double p1y, double p2x, double p2y, double epsilon = 1e-8 )
{
  return qgsDoubleNear( p1x, p2x, epsilon ) && qgsDoubleNear( p1y, p2y, epsilon );
}

bool QgsGeometryUtilsBase::segmentIntersection( double p1x, double p1y, double p2x, double p2y, double q1x, double q1y, double q2x, double q2y, double &intersectionPointX, double &intersectionPointY, bool &isIntersection, double tolerance, bool acceptImproperIntersection )
{
  isIntersection = false;
  intersectionPointX = intersectionPointY = std::numeric_limits<double>::quiet_NaN();

  QgsVector v( p2x - p1x, p2y - p1y );
  QgsVector w( q2x - q1x, q2y - q1y );
  const double vl = v.length();
  const double wl = w.length();

  if ( qgsDoubleNear( vl, 0.0, tolerance ) || qgsDoubleNear( wl, 0.0, tolerance ) )
  {
    return false;
  }
  v = v / vl;
  w = w / wl;

  if ( !lineIntersection( p1x, p1y, v, q1x, q1y, w, intersectionPointX, intersectionPointY ) )
  {
    return false;
  }

  isIntersection = true;
  if ( acceptImproperIntersection )
  {
    if ( ( equals( p1x, p1y, q1x, q1y ) ) || ( equals( p1x, p1y, q2x, q2y ) ) )
    {
      intersectionPointX = p1x;
      intersectionPointY = p1y;
      return true;
    }
    else if ( ( equals( p1x, p1y, q2x, q2y ) ) || ( equals( p2x, p2y, q2x, q2y ) ) )
    {
      intersectionPointX = p2x;
      intersectionPointY = p2y;
      return true;
    }

    double x, y;
    if (
      // intersectionPoint = p1
      qgsDoubleNear( sqrDistToLine( p1x, p1y, q1x, q1y, q2x, q2y, x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = p2
      qgsDoubleNear( sqrDistToLine( p2x, p2y, q1x, q1y, q2x, q2y, x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = q1
      qgsDoubleNear( sqrDistToLine( q1x, q1y, p1x, p1y, p2x, p2y, x, y, tolerance ), 0.0, tolerance ) ||
      // intersectionPoint = q2
      qgsDoubleNear( sqrDistToLine( q2x, q2y, p1x, p1y, p2x, p2y, x, y, tolerance ), 0.0, tolerance )
    )
    {
      return true;
    }
  }

  const double lambdav = QgsVector( intersectionPointX - p1x, intersectionPointY - p1y ) *  v;
  if ( lambdav < 0. + tolerance || lambdav > vl - tolerance )
    return false;

  const double lambdaw = QgsVector( intersectionPointX - q1x, intersectionPointY - q1y ) * w;
  return !( lambdaw < 0. + tolerance || lambdaw >= wl - tolerance );

}


bool QgsGeometryUtilsBase::linesIntersection3D( const QgsVector3D &La1, const QgsVector3D &La2,
    const QgsVector3D &Lb1, const QgsVector3D &Lb2,
    QgsVector3D &intersection )
{

  // if all Vector are on the same plane (have the same Z), use the 2D intersection
  // else return a false result
  if ( qgsDoubleNear( La1.z(), La2.z() ) && qgsDoubleNear( La1.z(), Lb1.z() ) && qgsDoubleNear( La1.z(), Lb2.z() ) )
  {
    double ptInterX = 0.0, ptInterY = 0.0;
    bool isIntersection = false;
    segmentIntersection( La1.x(), La1.y(),
                         La2.x(), La2.y(),
                         Lb1.x(), Lb1.y(),
                         Lb2.x(), Lb2.y(),
                         ptInterX, ptInterY,
                         isIntersection,
                         1e-8,
                         true );
    intersection.set( ptInterX, ptInterY, La1.z() );
    return true;
  }

  // first check if lines have an exact intersection point
  // do it by checking if the shortest distance is exactly 0
  const double distance = skewLinesDistance( La1, La2, Lb1, Lb2 );
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    // 3d lines have exact intersection point.
    const QgsVector3D C = La2;
    const QgsVector3D D = Lb2;
    const QgsVector3D e = La1 - La2;
    const QgsVector3D f = Lb1 - Lb2;
    const QgsVector3D g = D - C;
    if ( qgsDoubleNear( ( QgsVector3D::crossProduct( f, g ) ).length(), 0.0 ) || qgsDoubleNear( ( QgsVector3D::crossProduct( f, e ) ).length(), 0.0 ) )
    {
      // Lines have no intersection, are they parallel?
      return false;
    }

    QgsVector3D fgn = QgsVector3D::crossProduct( f, g );
    fgn.normalize();

    QgsVector3D fen = QgsVector3D::crossProduct( f, e );
    fen.normalize();

    int di = -1;
    if ( fgn == fen ) // same direction?
      di *= -1;

    intersection = C + e * di * ( QgsVector3D::crossProduct( f, g ).length() / QgsVector3D::crossProduct( f, e ).length() );
    return true;
  }

  // try to calculate the approximate intersection point
  QgsVector3D X1, X2;
  const bool firstIsDone = skewLinesProjection( La1, La2, Lb1, Lb2, X1 );
  const bool secondIsDone = skewLinesProjection( Lb1, Lb2, La1, La2, X2 );

  if ( !firstIsDone || !secondIsDone )
  {
    // Could not obtain projection point.
    return false;
  }

  intersection = ( X1 + X2 ) / 2.0;
  return true;
}

double QgsGeometryUtilsBase::triangleArea( double aX, double aY, double bX, double bY, double cX, double cY )
{
  return 0.5 * std::abs( ( aX - cX ) * ( bY - aY ) - ( aX - bX ) * ( cY - aY ) );
}

double QgsGeometryUtilsBase::pointFractionAlongLine( double x1, double y1, double x2, double y2, double px, double py )
{
  const double dxp = px - x1;
  const double dyp = py - y1;

  const double dxl = x2 - x1;
  const double dyl = y2 - y1;

  return std::sqrt( ( dxp * dxp ) + ( dyp * dyp ) ) / std::sqrt( ( dxl * dxl ) + ( dyl * dyl ) );
}

void QgsGeometryUtilsBase::weightedPointInTriangle( const double aX, const double aY, const double bX, const double bY, const double cX, const double cY,
    double weightB, double weightC, double &pointX, double &pointY )
{
  // if point will be outside of the triangle, invert weights
  if ( weightB + weightC > 1 )
  {
    weightB = 1 - weightB;
    weightC = 1 - weightC;
  }

  const double rBx = weightB * ( bX - aX );
  const double rBy = weightB * ( bY - aY );
  const double rCx = weightC * ( cX - aX );
  const double rCy = weightC * ( cY - aY );

  pointX = rBx + rCx + aX;
  pointY = rBy + rCy + aY;
}

bool QgsGeometryUtilsBase::pointsAreCollinear( double x1, double y1, double x2, double y2, double x3, double y3, double epsilon )
{
  return qgsDoubleNear( x1 * ( y2 - y3 ) + x2 * ( y3 - y1 ) + x3 * ( y1 - y2 ), 0, epsilon );
};

double QgsGeometryUtilsBase::azimuth( double x1, double y1, double x2, double y2 )
{
  const double dx = x2 - x1;
  const double dy = y2 - y1;
  return ( std::atan2( dx, dy ) * 180.0 / M_PI );
}

bool QgsGeometryUtilsBase::angleBisector( double aX, double aY, double bX, double bY, double cX, double cY, double dX, double dY,
    double &pointX, double &pointY, double &angle )
{
  angle = ( azimuth( aX, aY, bX, bY ) + azimuth( cX, cY, dX, dY ) ) / 2.0;

  bool intersection = false;
  QgsGeometryUtilsBase::segmentIntersection( aX, aY, bX, bY, cX, cY, dX, dY, pointX, pointY, intersection );

  return intersection;
}

void QgsGeometryUtilsBase::project( double aX, double aY, double aZ, double distance, double azimuth, double inclination, double &resultX, double &resultY, double &resultZ )
{
  const double radsXy = azimuth * M_PI / 180.0;
  double dx = 0.0, dy = 0.0, dz = 0.0;

  inclination = std::fmod( inclination, 360.0 );

  if ( std::isnan( aZ ) && qgsDoubleNear( inclination, 90.0 ) )
  {
    dx = distance * std::sin( radsXy );
    dy = distance * std::cos( radsXy );
  }
  else
  {
    const double radsZ = inclination * M_PI / 180.0;
    dx = distance * std::sin( radsZ ) * std::sin( radsXy );
    dy = distance * std::sin( radsZ ) * std::cos( radsXy );
    dz = distance * std::cos( radsZ );
  }

  resultX = aX + dx;
  resultY = aY + dy;
  resultZ = aZ + dz;
}

bool QgsGeometryUtilsBase::bisector( double aX, double aY, double bX, double bY, double cX, double cY,
                                     double &pointX, double &pointY )
{
  const double angle = ( azimuth( aX, aY, bX, bY ) + azimuth( aX, aY, cX, cY ) ) / 2.0;

  bool intersection = false;
  double dX = 0.0, dY = 0.0, dZ = 0.0;
  project( aX, aY, std::numeric_limits<double>::quiet_NaN(), 1.0, angle, 90.0, dX, dY, dZ );
  segmentIntersection( bX, bY, cX, cY, aX, aY, dX, dY, pointX, pointY, intersection );

  return intersection;
}
