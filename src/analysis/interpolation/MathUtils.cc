/***************************************************************************
                          MathUtils.cc  -  description
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

#include "MathUtils.h"
#include "qgslogger.h"

bool MathUtils::calcBarycentricCoordinates( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result )
{
  if ( p1 && p2 && p3 && result )
  {
    Point3D p( x, y, 0 );
    double area = triArea( p1, p2, p3 );
    if ( area == 0 )//p1, p2, p3 are in a line
    {
      QgsDebugMsg( "warning, triangle area should not be 0" );
      return false;
    }
    double area1 = triArea( &p, p2, p3 );
    double area2 = triArea( p1, &p, p3 );
    double area3 = triArea( p1, p2, &p );
    double u = area1 / area;
    double v = area2 / area;
    double w = area3 / area;
    result->setX( u );
    result->setY( v );
    result->setZ( w );
    return true;
  }
  else//null pointer
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool MathUtils::BarycentricToXY( double u, double v, double w, Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result )//this is wrong at the moment. Furthermore, the case, where the denominators are 0 have to be treated (two ways of calculating px and py)
{
  Q_UNUSED( w );

  double px, py;
  if ( p1 && p2 && p3 && result )
  {
    double area = triArea( p1, p2, p3 );

    if ( area == 0 )
    {
      QgsDebugMsg( "warning, p1, p2 and p3 are in a line" );
      return false;
    }

    double denominator = (( p2->getY() - p3->getY() ) * ( p1->getX() - p3->getX() ) - ( p3->getY() - p1->getY() ) * ( p3->getX() - p2->getX() ) );
    if ( denominator != 0 )//drop out py in the two equations
    {
      px = ( 2 * u * area * ( p1->getX() - p3->getX() ) - 2 * v * area * ( p3->getX() - p2->getX() ) - p2->getX() * p3->getY() * ( p1->getX() - p3->getX() ) + p3->getX() * p1->getY() * ( p3->getX() - p2->getX() ) + p3->getX() * p2->getY() * ( p1->getX() - p3->getX() ) - p1->getX() * p3->getY() * ( p3->getX() - p2->getX() ) ) / denominator;
      if (( p3->getX() - p2->getX() ) != 0 )
      {
        py = ( 2 * u * area - px * ( p2->getY() - p3->getY() ) - p2->getX() * p3->getY() + p3->getX() * p2->getY() ) / ( p3->getX() - p2->getX() );
      }
      else
      {
        py = ( 2 * v * area - px * ( p3->getY() - p1->getY() ) - p3->getX() * p1->getY() + p1->getX() * p3->getY() ) / ( p1->getX() - p3->getX() );
      }
    }
    else//dorp out px in the two equations(maybe this possibility occurs only, if p1, p2 and p3 are coplanar
    {
      py = ( 2 * u * area * ( p3->getY() - p1->getY() ) - 2 * v * area * ( p2->getY() - p3->getY() ) - p2->getX() * p3->getY() * ( p3->getY() - p1->getY() ) + p3->getX() * p1->getY() * ( p2->getY() - p3->getY() ) + p3->getX() * p2->getY() * ( p3->getY() - p1->getY() ) - p1->getX() * p3->getY() * ( p2->getY() - p3->getY() ) ) / (( p3->getX() - p2->getX() ) * ( p3->getY() - p1->getY() ) - ( p1->getX() - p3->getX() ) * ( p2->getY() - p3->getY() ) );
      if (( p2->getY() - p3->getY() ) != 0 )
      {
        px = ( 2 * u * area - py * ( p3->getX() - p2->getX() ) - p2->getX() * p3->getY() + p3->getX() * p2->getY() ) / ( p2->getY() - p3->getY() );
      }
      else
      {
        px = ( 2 * v * area - py * ( p1->getX() - p3->getX() ) - p3->getX() * p1->getY() + p1->getX() * p3->getY() ) / ( p3->getY() - p1->getY() );
      }
    }
    result->setX( px );
    result->setY( py );
    return true;
  }
  else//null pointer
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

double MathUtils::calcBernsteinPoly( int n, int i, double t )
{
  if ( i < 0 )
  {
    return 0;
  }

  return lower( n, i )*power( t, i )*power(( 1 - t ), ( n - i ) );
}

double MathUtils::cFDerBernsteinPoly( int n, int i, double t )
{
  return n*( calcBernsteinPoly( n - 1, i - 1, t ) - calcBernsteinPoly( n - 1, i, t ) );
}

bool MathUtils::circumcenter( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result )//version using the property that the distances from p1, p2, p3 to the circumcenter have to be equal. Possibly there is a bug
{
  if ( p1 && p2 && p3 && result )
  {
    double distp1p2 = sqrt(( p1->getX() - p2->getX() ) * ( p1->getX() - p2->getX() ) + ( p1->getY() - p2->getY() ) * ( p1->getY() - p2->getY() ) );
    double distp2p3 = sqrt(( p2->getX() - p3->getX() ) * ( p2->getX() - p3->getX() ) + ( p2->getY() - p3->getY() ) * ( p2->getY() - p3->getY() ) );
    if ( distp1p2 > distp2p3 )
    {
      //swap p1 and p3 to avoid round-off errors
      Point3D* temp = p1;
      p1 = p3;
      p3 = temp;
    }
    double denominator = -p3->getX() * p2->getY() + p3->getX() * p1->getY() + p1->getX() * p2->getY() + p2->getX() * p3->getY() - p2->getX() * p1->getY() - p1->getX() * p3->getY();
    //if one of the denominator is zero we will have problems
    if ( denominator == 0 )
    {
      QgsDebugMsg( "error: the three points are in a line" );
      return false;
    }
    else
    {
      result->setX( 0.5*( p1->getX()*p1->getX()*p2->getY() - p1->getX()*p1->getX()*p3->getY() - p3->getX()*p3->getX()*p2->getY() - p1->getY()*p2->getX()*p2->getX() - p1->getY()*p1->getY()*p3->getY() - p3->getY()*p3->getY()*p2->getY() + p1->getY()*p1->getY()*p2->getY() + p3->getY()*p2->getX()*p2->getX() - p1->getY()*p2->getY()*p2->getY() + p1->getY()*p3->getY()*p3->getY() + p1->getY()*p3->getX()*p3->getX() + p3->getY()*p2->getY()*p2->getY() ) / denominator );
      result->setY( -0.5*( p3->getX()*p2->getX()*p2->getX() + p2->getX()*p1->getY()*p1->getY() + p3->getX()*p2->getY()*p2->getY() - p3->getX()*p1->getX()*p1->getX() + p1->getX()*p3->getY()*p3->getY() - p3->getX()*p1->getY()*p1->getY() - p1->getX()*p2->getX()*p2->getX() - p2->getX()*p3->getY()*p3->getY() - p1->getX()*p2->getY()*p2->getY() - p2->getX()*p3->getX()*p3->getX() + p1->getX()*p3->getX()*p3->getX() + p2->getX()*p1->getX()*p1->getX() ) / denominator );

#if 0
      //debugging: test, if the distances from p1, p2, p3 to result are equal
      double dist1 = sqrt(( p1->getX() - result->getX() ) * ( p1->getX() - result->getX() ) + ( p1->getY() - result->getY() ) * ( p1->getY() - result->getY() ) );
      double dist2 = sqrt(( p2->getX() - result->getX() ) * ( p2->getX() - result->getX() ) + ( p2->getY() - result->getY() ) * ( p2->getY() - result->getY() ) );
      double dist3 = sqrt(( p3->getX() - result->getX() ) * ( p3->getX() - result->getX() ) + ( p3->getY() - result->getY() ) * ( p3->getY() - result->getY() ) );

      if ( dist1 - dist2 > 1 || dist2 - dist1 > 1 || dist1 - dist3 > 1 || dist3 - dist1 > 1 )
      {
        bool debug = true;
      }
#endif

      return true;
    }
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

#if 0
bool MathUtils::circumcenter( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* result )//version imitating the geometric construction
{
  if ( p1 && p2 && p3 && result )
  {
    Point3D midpoint12(( p1->getX() + p2->getX() ) / 2, ( p1->getY() + p2->getY() ) / 2, 0 );
    Point3D midpoint23(( p2->getX() + p3->getX() ) / 2, ( p2->getY() + p3->getY() ) / 2, 0 );
    Vector3D v12( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D v23( p3->getX() - p2->getX(), p3->getY() - p2->getY(), 0 );
    Vector3D n12;
    MathUtils::normalLeft( &v12, &n12, 10000 );
    Vector3D n23;
    MathUtils::normalLeft( &v23, &n23, 10000 );
    Point3D helppoint1( midpoint12.getX() + n12.getX(), midpoint12.getY() + n12.getY(), 0 );
    Point3D helppoint2( midpoint23.getX() + n23.getX(), midpoint23.getY() + n23.getY(), 0 );
    MathUtils::lineIntersection( &midpoint12, &helppoint1, &midpoint23, &helppoint2, result );

    //debugging: test, if the distances from p1, p2, p3 to result are equal
    double dist1 = sqrt(( p1->getX() - result->getX() ) * ( p1->getX() - result->getX() ) + ( p1->getY() - result->getY() ) * ( p1->getY() - result->getY() ) );
    double dist2 = sqrt(( p2->getX() - result->getX() ) * ( p2->getX() - result->getX() ) + ( p2->getY() - result->getY() ) * ( p2->getY() - result->getY() ) );
    double dist3 = sqrt(( p3->getX() - result->getX() ) * ( p3->getX() - result->getX() ) + ( p3->getY() - result->getY() ) * ( p3->getY() - result->getY() ) );

    if ( dist1 - dist2 > 1 || dist2 - dist1 > 1 || dist1 - dist3 > 1 || dist3 - dist1 > 1 )
    {
      bool debug = true;
    }

    return true;

  }
  else
  {
    cout << "null pointer in method MathUtils::circumcenter" << endl << flush;
    return false;
  }
}
#endif // 0

double MathUtils::distPointFromLine( Point3D* thepoint, Point3D* p1, Point3D* p2 )
{
  if ( thepoint && p1 && p2 )
  {
    Vector3D normal( 0, 0, 0 );
    Vector3D line( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    MathUtils::normalLeft( &line, &normal, 1 );
    double a = normal.getX();
    double b = normal.getY();
    double c = -( normal.getX() * p2->getX() + normal.getY() * p2->getY() );
    double distance = qAbs(( a * thepoint->getX() + b * thepoint->getY() + c ) / ( sqrt( a * a + b * b ) ) );
    return distance;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

int MathUtils::faculty( int n )
{
  if ( n < 0 )//Is faculty also defined for negative integers?
  {
    QgsDebugMsg( "Error, faculty of a negative integer requested!" );
    return 0;
  }
  int i;
  int result = n;

  if ( n == 0 || n == 1 )
    {return 1;}//faculty of 0 is 1!

  for ( i = n - 1; i >= 2; i-- )
  {
    result *= i;
  }
  return result;
}

bool MathUtils::inCircle( Point3D* testp, Point3D* p1, Point3D* p2, Point3D* p3 )
{
  double tolerance = 0.0001;//if the amount of aValue is below this, testp is approximately on the circle and we have to define another criterion to tell, if it is inside or outside.

  if ( testp && p1 && p2 && p3 )
  {
    double ax = p1->getX();
    double ay = p1->getY();
    double bx = p2->getX();
    double by = p2->getY();
    double cx = p3->getX();
    double cy = p3->getY();
    double px = testp->getX();
    double py = testp->getY();

    double xmin = qMin( qMin( ax, px ), qMin( bx, cx ) );
    double ymin = qMin( qMin( ay, py ), qMin( by, cy ) );
    ax -= xmin;
    bx -= xmin;
    cx -= xmin;
    px -= xmin;
    ay -= ymin;
    by -= ymin;
    cy -= ymin;
    py -= ymin;

    double aValue = ( ax * ax + ay * ay ) * triArea( p2, p3, testp );
    aValue = aValue - (( bx * bx + by * by ) * triArea( p1, p3, testp ) );
    aValue = aValue + (( cx * cx + cy * cy ) * triArea( p1, p2, testp ) );
    aValue = aValue - (( px * px + py * py ) * triArea( p1, p2, p3 ) );

    return aValue > tolerance;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool MathUtils::inDiametral( Point3D* p1, Point3D* p2, Point3D* point )
{
  return angle( p1, point, p2, point ) > 90;
}

#if 0
bool MathUtils::inDiametral( Point3D* p1, Point3D* p2, Point3D* point )
{
  if ( p1 && p2 && point )
  {
    Vector3D p1p2( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D orthogonalvec;//vector orthogonal to p1p2 (length radius)
    Point3D midpoint(( p1->getX() + p2->getX() ) / 2, ( p1->getY() + p2->getY() ) / 2, 0 );
    double radius = p1p2.getLength() / 2;
    normalLeft( &p1p2, &orthogonalvec, radius );
    Point3D p3( midpoint.getX() + orthogonalvec.getX(), midpoint.getY() + orthogonalvec.getY(), 0 );
    return inCircle( point, p1, p2, &p3 );
  }
  else
  {
    cout << "null pointer in MathUtils::inDiametral" << endl << flush;
    return false;
  }
}
#endif // 0

double MathUtils::leftOf( Point3D* thepoint, Point3D* p1, Point3D* p2 )
{
  if ( thepoint && p1 && p2 )
  {
    //just for debugging
    double f1 = thepoint->getX() - p1->getX();
    double f2 = p2->getY() - p1->getY();
    double f3 = thepoint->getY() - p1->getY();
    double f4 = p2->getX() - p1->getX();
    return f1*f2 - f3*f4;
    //return thepoint->getX()-p1->getX())*(p2->getY()-p1->getY())-(thepoint->getY()-p1->getY())*(p2->getX()-p1->getX();//calculating the vectorproduct
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

bool MathUtils::lineIntersection( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4 )
{
  if ( p1 && p2 && p3 && p4 )
  {
    double t1, t2;
    Vector3D p1p2( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D p3p4( p4->getX() - p3->getX(), p4->getY() - p3->getY(), 0 );
    if (( p3p4.getX()*p1p2.getY() - p3p4.getY()*p1p2.getX() ) != 0 && p1p2.getX() != 0 )//avoid division through zero
    {
      t2 = ( p1->getX() * p1p2.getY() - p1->getY() * p1p2.getX() + p3->getY() * p1p2.getX() - p3->getX() * p1p2.getY() ) / ( p3p4.getX() * p1p2.getY() - p3p4.getY() * p1p2.getX() );
      t1 = ( p3->getX() - p1->getX() + t2 * p3p4.getX() ) / p1p2.getX();
    }
    else if (( p1p2.getX()*p3p4.getY() - p1p2.getY()*p3p4.getX() ) != 0 && p3p4.getX() != 0 )
    {
      t1 = ( p3->getX() * p3p4.getY() - p3->getY() * p3p4.getX() - p1->getX() * p3p4.getY() + p1->getY() * p3p4.getX() ) / ( p1p2.getX() * p3p4.getY() - p1p2.getY() * p3p4.getX() );
      t2 = ( p1->getX() + t1 * p1p2.getX() - p3->getX() ) / p3p4.getX();
    }
    else//the lines are parallel
    {
      return false;
    }

    if ( t1 > 0 && t1<1 && t2>0 && t2 < 1 )
    {
      if (( *p1 ) == ( *p3 ) || ( *p1 ) == ( *p4 ) || ( *p2 ) == ( *p3 ) || ( *p2 ) == ( *p4 ) )//the lines touch each other, so they do not cross
      {
        return false;
      }
      return true;
    }
    else
    {
      return false;
    }
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool MathUtils::lineIntersection( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4, Point3D* intersection_point )
{
  if ( p1 && p2 && p3 && p4 )
  {
    double t1, t2;
    Vector3D p1p2( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D p3p4( p4->getX() - p3->getX(), p4->getY() - p3->getY(), 0 );
    if (( p3p4.getX()*p1p2.getY() - p3p4.getY()*p1p2.getX() ) != 0 && p1p2.getX() != 0 )//avoid division through zero
    {
      t2 = ( p1->getX() * p1p2.getY() - p1->getY() * p1p2.getX() + p3->getY() * p1p2.getX() - p3->getX() * p1p2.getY() ) / ( p3p4.getX() * p1p2.getY() - p3p4.getY() * p1p2.getX() );
      t1 = ( p3->getX() - p1->getX() + t2 * p3p4.getX() ) / p1p2.getX();
    }
    else if (( p1p2.getX()*p3p4.getY() - p1p2.getY()*p3p4.getX() ) != 0 && p3p4.getX() != 0 )
    {
      t1 = ( p3->getX() * p3p4.getY() - p3->getY() * p3p4.getX() - p1->getX() * p3p4.getY() + p1->getY() * p3p4.getX() ) / ( p1p2.getX() * p3p4.getY() - p1p2.getY() * p3p4.getX() );
      t2 = ( p1->getX() + t1 * p1p2.getX() - p3->getX() ) / p3p4.getX();
    }
    else//the lines are parallel
    {
      intersection_point->setX( 0 );
      intersection_point->setY( 0 );
      intersection_point->setZ( 0 );
      return false;
    }

    if ( t1 > 0 && t1<1 && t2>0 && t2 < 1 )
    {
      if (( *p1 ) == ( *p3 ) || ( *p1 ) == ( *p4 ) || ( *p2 ) == ( *p3 ) || ( *p2 ) == ( *p4 ) )//the lines touch each other, so they do not cross
      {
        intersection_point->setX( 0 );
        intersection_point->setY( 0 );
        intersection_point->setZ( 0 );
        return false;
      }
      //calculate the intersection point
      intersection_point->setX( p1->getX()*( 1 - t1 ) + p2->getX()*t1 );
      intersection_point->setY( p1->getY()*( 1 - t1 ) + p2->getY()*t1 );
      intersection_point->setZ( 0 );
      return true;
    }
    else
    {
      return false;
    }
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

int MathUtils::lower( int n, int i )
{
  if ( i >= 0 && i <= n )
  {
    return faculty( n ) / ( faculty( i )*faculty( n - i ) );
  }
  else
  {
    return 0;
  }
}

double MathUtils::max( double x, double y )
{
  if ( x > y )
  {
    return x;
  }
  else if ( y > x )
  {
    return y;
  }
  else//if both are equal
  {
    return x;
  }
}
double MathUtils::min( double x, double y )
{
  if ( x < y )
  {
    return x;
  }
  else if ( y < x )
  {
    return y;
  }
  else//if both are equal
  {
    return x;
  }
}

double MathUtils::power( double a, int b )
{
  if ( b == 0 )
  {
    return 1;
  }
  double tmp = a;
  for ( int i = 2; i <= qAbs(( double )b ); i++ )
  {

    a *= tmp;
  }
  if ( b > 0 )
  {
    return a;
  }
  else
  {
    return ( 1.0 / a );
  }
}

double MathUtils::triArea( Point3D* pa, Point3D* pb, Point3D* pc )
{
  if ( pa && pb && pc )
  {
    double deter = ( pa->getX() * pb->getY() + pb->getX() * pc->getY() + pc->getX() * pa->getY() - pa->getX() * pc->getY() - pb->getX() * pa->getY() - pc->getX() * pb->getY() );
    return 0.5*deter;
  }
  else//null pointer
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

double MathUtils::calcCubicHermitePoly( int n, int i, double t )
{
  if ( n != 3 || i > n )
  {
    QgsDebugMsg( "error, can't calculate hermite polynom" );
  }

  if ( n == 3 && i == 0 )
  {
    return ( calcBernsteinPoly( 3, 0, t ) + calcBernsteinPoly( 3, 1, t ) );
  }

  if ( n == 3 && i == 1 )
  {
    return ( calcBernsteinPoly( 3, 1, t )*0.33333333 );
  }

  if ( n == 3 && i == 2 )
  {
    return ( calcBernsteinPoly( 3, 2, t )* -0.33333333 );
  }

  if ( n == 3 && i == 3 )
  {
    return ( calcBernsteinPoly( 3, 2, t ) + calcBernsteinPoly( 3, 3, t ) );
  }
  else//something went wrong
  {
    QgsDebugMsg( "unexpected error" );
    return 0;
  }
}

double MathUtils::cFDerCubicHermitePoly( int n, int i, double t )
{
  if ( n != 3 || i > n )
  {
    QgsDebugMsg( "error, can't calculate hermite polynom" );
  }

  if ( n == 3 && i == 0 )
  {
    //cout << "cFDerBernsteinPoly(3,0,t): " << cFDerBernsteinPoly(3,0,t) << endl << flush;
    //cout << "cFDerBernsteinPoly(3,1,t): " << cFDerBernsteinPoly(3,1,t) << endl << flush;
    return ( cFDerBernsteinPoly( 3, 0, t ) + cFDerBernsteinPoly( 3, 1, t ) );
  }

  if ( n == 3 && i == 1 )
  {
    //cout << "cFDerBernsteinPoly(3,1,t)/3: " << cFDerBernsteinPoly(3,1,t)/3 << endl << flush;
    return ( cFDerBernsteinPoly( 3, 1, t ) / 3 );
  }

  if ( n == 3 && i == 2 )
  {
    //cout << "cFDerBernsteinPoly(3,2,t)/3: " << cFDerBernsteinPoly(3,2,t)/3 << endl << flush;
    return ( -cFDerBernsteinPoly( 3, 2, t ) / 3 );
  }

  if ( n == 3 && i == 3 )
  {
    //cout << "cFDerBernsteinPoly(3,2,t): " << cFDerBernsteinPoly(3,2,t) << endl << flush;
    //cout << "cFDerBernsteinPoly(3,3,t): " << cFDerBernsteinPoly(3,3,t) << endl << flush;
    return ( cFDerBernsteinPoly( 3, 2, t ) + cFDerBernsteinPoly( 3, 3, t ) );
  }
  else
  {
    QgsDebugMsg( "unexpected error" );
    return 0;
  }
}

bool MathUtils::derVec( const Vector3D* v1, const Vector3D* v2, Vector3D* result, double x, double y )
{
  if ( v1 && v2 && result )//no null pointers
  {
    double u = ( x * v2->getY() - y * v2->getX() ) / ( v1->getX() * v2->getY() - v1->getY() * v2->getX() );
    double v = ( x * v1->getY() - y * v1->getX() ) / ( v2->getX() * v1->getY() - v2->getY() * v1->getX() );
    result->setX( x );
    result->setY( y );
    result->setZ( u*v1->getZ() + v*v2->getZ() );
    return true;
  }
  return false;
}


bool MathUtils::normalLeft( Vector3D* v1, Vector3D* result, double length )
{
  if ( v1 && result )//we don't like null pointers
  {
    if ( v1->getY() == 0 )//this would cause a division with zero
    {
      result->setX( 0 );

      if ( v1->getX() < 0 )
      {
        result->setY( -length );
      }

      else
      {
        result->setY( length );
      }
      return true;
    }

    //coefficients for the quadratic equation
    double a = 1 + ( v1->getX() * v1->getX() ) / ( v1->getY() * v1->getY() );
    double b = 0;
    double c = -( length * length );
    double d = b * b - 4 * a * c;

    if ( d < 0 )//no solution in R
    {
      QgsDebugMsg( "Determinant Error" );
      return false;
    }

    result->setX(( -b + sqrt( d ) ) / ( 2*a ) );//take one of the two solutions of the quadratic equation
    result->setY(( -result->getX()*v1->getX() ) / v1->getY() );

    Point3D point1( 0, 0, 0 );
    Point3D point2( v1->getX(), v1->getY(), 0 );
    Point3D point3( result->getX(), result->getY(), 0 );

    if ( !( leftOf( &point1, &point2, &point3 ) < 0 ) )//if we took the solution on the right side, change the sign of the components of the result
    {
      result->setX( -result->getX() );
      result->setY( -result->getY() );
    }

    return true;
  }

  else
  {
    return false;
  }
}



bool MathUtils::normalRight( Vector3D* v1, Vector3D* result, double length )
{
  if ( v1 && result )//we don't like null pointers
  {

    if ( v1->getY() == 0 )//this would cause a division with zero
    {
      result->setX( 0 );

      if ( v1->getX() < 0 )
      {
        result->setY( length );
      }

      else
      {
        result->setY( -length );
      }
      return true;
    }

    //coefficients for the quadratic equation
    double a = 1 + ( v1->getX() * v1->getX() ) / ( v1->getY() * v1->getY() );
    double b = 0;
    double c = -( length * length );
    double d = b * b - 4 * a * c;

    if ( d < 0 )//no solution in R
    {
      QgsDebugMsg( "Determinant Error" );
      return false;
    }

    result->setX(( -b + sqrt( d ) ) / ( 2*a ) );//take one of the two solutions of the quadratic equation
    result->setY(( -result->getX()*v1->getX() ) / v1->getY() );

    Point3D point1( 0, 0, 0 );
    Point3D point2( v1->getX(), v1->getY(), 0 );
    Point3D point3( result->getX(), result->getY(), 0 );

    if (( leftOf( &point1, &point2, &point3 ) < 0 ) )//if we took the solution on the right side, change the sign of the components of the result
    {
      result->setX( -result->getX() );
      result->setY( -result->getY() );
    }

    return true;
  }

  else
  {
    return false;
  }
}


void MathUtils::normalFromPoints( Point3D* p1, Point3D* p2, Point3D* p3, Vector3D* vec )
{
  if ( p1 && p2 && p3 && vec )//no null pointers
  {
    double ax = p2->getX() - p1->getX();
    double ay = p2->getY() - p1->getY();
    double az = p2->getZ() - p1->getZ();
    double bx = p3->getX() - p1->getX();
    double by = p3->getY() - p1->getY();
    double bz = p3->getZ() - p1->getZ();

    vec->setX( ay*bz - az*by );
    vec->setY( az*bx - ax*bz );
    vec->setZ( ax*by - ay*bx );
  }

}

double MathUtils::crossVec( Point3D* first, Vector3D* vec1, Point3D* second, Vector3D* vec2 )
{
  if ( first && vec1 && second && vec2 )
  {
    if (( vec2->getX()*vec1->getY() - vec2->getY()*vec1->getX() ) != 0 )
    {
      /*cout << "first: " << first->getX() << "//" << first->getY() << "//" << first->getZ() << endl << flush;
      cout << "vec1: " << vec1->getX() << "//" << vec1->getY() << "//" << vec1->getZ() << endl << flush;
      cout << "second: " << second->getX() << "//" << second->getY() << "//" << second->getZ() << endl << flush;
      cout << "vec2: " << vec2->getX() << "//" << vec2->getY() << "//" << vec2->getZ() << endl << flush;
      cout << "t2: " << ((first->getX()*vec1->getY()-first->getY()*vec1->getX()-second->getX()*vec1->getY()+second->getY()*vec1->getX())/(vec2->getX()*vec1->getY()-vec2->getY()*vec1->getX())) << endl << flush;*/

      return (( first->getX()*vec1->getY() - first->getY()*vec1->getX() - second->getX()*vec1->getY() + second->getY()*vec1->getX() ) / ( vec2->getX()*vec1->getY() - vec2->getY()*vec1->getX() ) );

    }
    else//if a division by zero would occur
    {
      QgsDebugMsg( "warning: vectors are parallel" );
      return 0;
    }
  }


  else//null pointer
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

bool MathUtils::pointInsideTriangle( double x, double y, Point3D* p1, Point3D* p2, Point3D* p3 )
{
  Point3D thepoint( x, y, 0 );
  if ( MathUtils::leftOf( &thepoint, p1, p2 ) > 0 )
  {
    return false;
  }
  if ( MathUtils::leftOf( &thepoint, p2, p3 ) > 0 )
  {
    return false;
  }
  if ( MathUtils::leftOf( &thepoint, p3, p1 ) > 0 )
  {
    return false;
  }
  return true;
}

bool MathUtils::normalMinDistance( Vector3D* tangent, Vector3D* target, Vector3D* result )
{
  if ( tangent && target && result )
  {
    double xt = tangent->getX();
    double yt = tangent->getY();
    double zt = tangent->getZ();

    double xw = target->getX();
    double yw = target->getY();
    double zw = target->getZ();

    double xg1, yg1, zg1;//the coordinates of the first result
    double xg2, yg2, zg2;//the coordinates of the second result

    //calculate xg
    double xgalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( xgalpha1 < 0 )
    {
      QgsDebugMsg( "warning, only complex solution of xg" );
      return false;
    }
    xg1 = sqrt( xgalpha1 ) * ( -yt * yw * xt + yt * yt * xw + xw * zt * zt - zt * xt * zw );
    xg2 = -sqrt( xgalpha1 ) * ( -yt * yw * xt + yt * yt * xw + xw * zt * zt - zt * xt * zw );

    //calculate yg
    double ygalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( ygalpha1 < 0 )
    {
      QgsDebugMsg( "warning, only complex solution of yg" );
      return false;
    }
    yg1 = -sqrt( ygalpha1 ) * ( -yw * xt * xt - zt * zt * yw + zt * yt * zw + yt * xw * xt );
    yg2 = sqrt( ygalpha1 ) * ( -yw * xt * xt - zt * zt * yw + zt * yt * zw + yt * xw * xt );

    //calculate zg
    double zgalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( zgalpha1 < 0 )
    {
      QgsDebugMsg( "warning, only complex solution of zg" );
      return false;
    }
    zg1 = -sqrt( zgalpha1 ) * ( yt * yw * zt - yt * yt * zw + xw * zt * xt - xt * xt * zw );
    zg2 = sqrt( zgalpha1 ) * ( yt * yw * zt - yt * yt * zw + xw * zt * xt - xt * xt * zw );

    double distance1 = sqrt(( xw - xg1 ) * ( xw - xg1 ) + ( yw - yg1 ) * ( yw - yg1 ) + ( zw - zg1 ) * ( zw - zg1 ) );
    double distance2 = sqrt(( xw - xg2 ) * ( xw - xg2 ) + ( yw - yg2 ) * ( yw - yg2 ) + ( zw - zg2 ) * ( zw - zg2 ) );

    if ( distance1 <= distance2 )//find out, which solution is the maximum and which the minimum
    {
      result->setX( xg1 );
      result->setY( yg1 );
      result->setZ( zg1 );
    }
    else
    {
      result->setX( xg2 );
      result->setY( yg2 );
      result->setZ( zg2 );
    }
    return true;
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}


double MathUtils::planeTest( Point3D* test, Point3D* pt1, Point3D* pt2, Point3D* pt3 )
{
  if ( test && pt1 && pt2 && pt3 )
  {
    double a = ( pt1->getZ() * ( pt2->getY() - pt3->getY() ) + pt2->getZ() * ( pt3->getY() - pt1->getY() ) + pt3->getZ() * ( pt1->getY() - pt2->getY() ) ) / (( pt1->getX() - pt2->getX() ) * ( pt2->getY() - pt3->getY() ) - ( pt2->getX() - pt3->getX() ) * ( pt1->getY() - pt2->getY() ) );
    double b = ( pt1->getZ() * ( pt2->getX() - pt3->getX() ) + pt2->getZ() * ( pt3->getX() - pt1->getX() ) + pt3->getZ() * ( pt1->getX() - pt2->getX() ) ) / (( pt1->getY() - pt2->getY() ) * ( pt2->getX() - pt3->getX() ) - ( pt2->getY() - pt3->getY() ) * ( pt1->getX() - pt2->getX() ) );
    double c = pt1->getZ() - a * pt1->getX() - b * pt1->getY();
    double zpredicted = test->getX() * a + test->getY() * b + c;
    return ( test->getZ() - zpredicted );
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}

double MathUtils::angle( Point3D* p1, Point3D* p2, Point3D* p3, Point3D* p4 )
{
  if ( p1 && p2 && p3 && p4 )
  {
    Vector3D v1( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D v2( p4->getX() - p3->getX(), p4->getY() - p3->getY(), 0 );
    double value = acos(( v1.getX() * v2.getX() + v1.getY() * v2.getY() ) / ( v1.getLength() * v2.getLength() ) ) * 180 / M_PI;
    return value;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return 0;
  }
}
