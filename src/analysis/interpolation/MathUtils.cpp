/***************************************************************************
                             MathUtils.cpp
                             -------------
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
#include "qgspoint.h"
#include "Vector3D.h"

bool MathUtils::calcBarycentricCoordinates( double x, double y, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result )
{
  if ( p1 && p2 && p3 && result )
  {
    QgsPoint p( x, y, 0 );
    const double area = triArea( p1, p2, p3 );
    if ( area == 0 )//p1, p2, p3 are in a line
    {
      return false;
    }
    const double area1 = triArea( &p, p2, p3 );
    const double area2 = triArea( p1, &p, p3 );
    const double area3 = triArea( p1, p2, &p );
    const double u = area1 / area;
    const double v = area2 / area;
    const double w = area3 / area;
    result->setX( u );
    result->setY( v );
    result->setZ( w );
    return true;
  }
  else//null pointer
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool MathUtils::BarycentricToXY( double u, double v, double w, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result )//this is wrong at the moment. Furthermore, the case, where the denominators are 0 have to be treated (two ways of calculating px and py)
{
  Q_UNUSED( w )

  double px, py;
  if ( p1 && p2 && p3 && result )
  {
    const double area = triArea( p1, p2, p3 );

    if ( area == 0 )
    {
      QgsDebugMsg( QStringLiteral( "warning, p1, p2 and p3 are in a line" ) );
      return false;
    }

    const double denominator = ( ( p2->y() - p3->y() ) * ( p1->x() - p3->x() ) - ( p3->y() - p1->y() ) * ( p3->x() - p2->x() ) );
    if ( denominator != 0 )//drop out py in the two equations
    {
      px = ( 2 * u * area * ( p1->x() - p3->x() ) - 2 * v * area * ( p3->x() - p2->x() ) - p2->x() * p3->y() * ( p1->x() - p3->x() ) + p3->x() * p1->y() * ( p3->x() - p2->x() ) + p3->x() * p2->y() * ( p1->x() - p3->x() ) - p1->x() * p3->y() * ( p3->x() - p2->x() ) ) / denominator;
      if ( ( p3->x() - p2->x() ) != 0 )
      {
        py = ( 2 * u * area - px * ( p2->y() - p3->y() ) - p2->x() * p3->y() + p3->x() * p2->y() ) / ( p3->x() - p2->x() );
      }
      else
      {
        py = ( 2 * v * area - px * ( p3->y() - p1->y() ) - p3->x() * p1->y() + p1->x() * p3->y() ) / ( p1->x() - p3->x() );
      }
    }
    else//drop out px in the two equations(maybe this possibility occurs only, if p1, p2 and p3 are coplanar
    {
      py = ( 2 * u * area * ( p3->y() - p1->y() ) - 2 * v * area * ( p2->y() - p3->y() ) - p2->x() * p3->y() * ( p3->y() - p1->y() ) + p3->x() * p1->y() * ( p2->y() - p3->y() ) + p3->x() * p2->y() * ( p3->y() - p1->y() ) - p1->x() * p3->y() * ( p2->y() - p3->y() ) ) / ( ( p3->x() - p2->x() ) * ( p3->y() - p1->y() ) - ( p1->x() - p3->x() ) * ( p2->y() - p3->y() ) );
      if ( ( p2->y() - p3->y() ) != 0 )
      {
        px = ( 2 * u * area - py * ( p3->x() - p2->x() ) - p2->x() * p3->y() + p3->x() * p2->y() ) / ( p2->y() - p3->y() );
      }
      else
      {
        px = ( 2 * v * area - py * ( p1->x() - p3->x() ) - p3->x() * p1->y() + p1->x() * p3->y() ) / ( p3->y() - p1->y() );
      }
    }
    result->setX( px );
    result->setY( py );
    return true;
  }
  else//null pointer
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

double MathUtils::calcBernsteinPoly( int n, int i, double t )
{
  if ( i < 0 )
  {
    return 0;
  }

  return lower( n, i ) * std::pow( t, i ) * std::pow( ( 1 - t ), ( n - i ) );
}

double MathUtils::cFDerBernsteinPoly( int n, int i, double t )
{
  return n * ( calcBernsteinPoly( n - 1, i - 1, t ) - calcBernsteinPoly( n - 1, i, t ) );
}

bool MathUtils::circumcenter( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result )//version using the property that the distances from p1, p2, p3 to the circumcenter have to be equal. Possibly there is a bug
{
  if ( p1 && p2 && p3 && result )
  {
    const double distp1p2 = std::sqrt( ( p1->x() - p2->x() ) * ( p1->x() - p2->x() ) + ( p1->y() - p2->y() ) * ( p1->y() - p2->y() ) );
    const double distp2p3 = std::sqrt( ( p2->x() - p3->x() ) * ( p2->x() - p3->x() ) + ( p2->y() - p3->y() ) * ( p2->y() - p3->y() ) );
    if ( distp1p2 > distp2p3 )
    {
      //swap p1 and p3 to avoid round-off errors
      QgsPoint *temp = p1;
      p1 = p3;
      p3 = temp;
    }
    const double denominator = -p3->x() * p2->y() + p3->x() * p1->y() + p1->x() * p2->y() + p2->x() * p3->y() - p2->x() * p1->y() - p1->x() * p3->y();
    //if one of the denominator is zero we will have problems
    if ( denominator == 0 )
    {
      QgsDebugMsg( QStringLiteral( "error: the three points are in a line" ) );
      return false;
    }
    else
    {
      result->setX( 0.5 * ( p1->x()*p1->x()*p2->y() - p1->x()*p1->x()*p3->y() - p3->x()*p3->x()*p2->y() - p1->y()*p2->x()*p2->x() - p1->y()*p1->y()*p3->y() - p3->y()*p3->y()*p2->y() + p1->y()*p1->y()*p2->y() + p3->y()*p2->x()*p2->x() - p1->y()*p2->y()*p2->y() + p1->y()*p3->y()*p3->y() + p1->y()*p3->x()*p3->x() + p3->y()*p2->y()*p2->y() ) / denominator );
      result->setY( -0.5 * ( p3->x()*p2->x()*p2->x() + p2->x()*p1->y()*p1->y() + p3->x()*p2->y()*p2->y() - p3->x()*p1->x()*p1->x() + p1->x()*p3->y()*p3->y() - p3->x()*p1->y()*p1->y() - p1->x()*p2->x()*p2->x() - p2->x()*p3->y()*p3->y() - p1->x()*p2->y()*p2->y() - p2->x()*p3->x()*p3->x() + p1->x()*p3->x()*p3->x() + p2->x()*p1->x()*p1->x() ) / denominator );

#if 0
      //debugging: test, if the distances from p1, p2, p3 to result are equal
      double dist1 = std::sqrt( ( p1->getX() - result->getX() ) * ( p1->getX() - result->getX() ) + ( p1->getY() - result->getY() ) * ( p1->getY() - result->getY() ) );
      double dist2 = std::sqrt( ( p2->getX() - result->getX() ) * ( p2->getX() - result->getX() ) + ( p2->getY() - result->getY() ) * ( p2->getY() - result->getY() ) );
      double dist3 = std::sqrt( ( p3->getX() - result->getX() ) * ( p3->getX() - result->getX() ) + ( p3->getY() - result->getY() ) * ( p3->getY() - result->getY() ) );

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
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

#if 0
bool MathUtils::circumcenter( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *result )//version imitating the geometric construction
{
  if ( p1 && p2 && p3 && result )
  {
    QgsPoint midpoint12( ( p1->getX() + p2->getX() ) / 2, ( p1->getY() + p2->getY() ) / 2, 0 );
    QgsPoint midpoint23( ( p2->getX() + p3->getX() ) / 2, ( p2->getY() + p3->getY() ) / 2, 0 );
    Vector3D v12( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D v23( p3->getX() - p2->getX(), p3->getY() - p2->getY(), 0 );
    Vector3D n12;
    MathUtils::normalLeft( &v12, &n12, 10000 );
    Vector3D n23;
    MathUtils::normalLeft( &v23, &n23, 10000 );
    QgsPoint helppoint1( midpoint12.getX() + n12.getX(), midpoint12.getY() + n12.getY(), 0 );
    QgsPoint helppoint2( midpoint23.getX() + n23.getX(), midpoint23.getY() + n23.getY(), 0 );
    MathUtils::lineIntersection( &midpoint12, &helppoint1, &midpoint23, &helppoint2, result );

    //debugging: test, if the distances from p1, p2, p3 to result are equal
    double dist1 = std::sqrt( ( p1->getX() - result->getX() ) * ( p1->getX() - result->getX() ) + ( p1->getY() - result->getY() ) * ( p1->getY() - result->getY() ) );
    double dist2 = std::sqrt( ( p2->getX() - result->getX() ) * ( p2->getX() - result->getX() ) + ( p2->getY() - result->getY() ) * ( p2->getY() - result->getY() ) );
    double dist3 = std::sqrt( ( p3->getX() - result->getX() ) * ( p3->getX() - result->getX() ) + ( p3->getY() - result->getY() ) * ( p3->getY() - result->getY() ) );

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

double MathUtils::distPointFromLine( QgsPoint *thepoint, QgsPoint *p1, QgsPoint *p2 )
{
  if ( thepoint && p1 && p2 )
  {
    Vector3D normal( 0, 0, 0 );
    Vector3D line( p2->x() - p1->x(), p2->y() - p1->y(), 0 );
    MathUtils::normalLeft( &line, &normal, 1 );
    const double a = normal.getX();
    const double b = normal.getY();
    const double c = -( normal.getX() * p2->x() + normal.getY() * p2->y() );
    const double distance = std::fabs( ( a * thepoint->x() + b * thepoint->y() + c ) / ( std::sqrt( a * a + b * b ) ) );
    return distance;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

int MathUtils::faculty( int n )
{
  return std::tgamma( n + 1 );
}

bool MathUtils::inCircle( QgsPoint *testp, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3 )
{
  const double tolerance = 0.0001;//if the amount of aValue is below this, testp is approximately on the circle and we have to define another criterion to tell, if it is inside or outside.

  if ( testp && p1 && p2 && p3 )
  {
    double ax = p1->x();
    double ay = p1->y();
    double bx = p2->x();
    double by = p2->y();
    double cx = p3->x();
    double cy = p3->y();
    double px = testp->x();
    double py = testp->y();

    const double xmin = std::min( std::min( ax, px ), std::min( bx, cx ) );
    const double ymin = std::min( std::min( ay, py ), std::min( by, cy ) );
    ax -= xmin;
    bx -= xmin;
    cx -= xmin;
    px -= xmin;
    ay -= ymin;
    by -= ymin;
    cy -= ymin;
    py -= ymin;

    double aValue = ( ax * ax + ay * ay ) * triArea( p2, p3, testp );
    aValue = aValue - ( ( bx * bx + by * by ) * triArea( p1, p3, testp ) );
    aValue = aValue + ( ( cx * cx + cy * cy ) * triArea( p1, p2, testp ) );
    aValue = aValue - ( ( px * px + py * py ) * triArea( p1, p2, p3 ) );

    return aValue > tolerance;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool MathUtils::inDiametral( QgsPoint *p1, QgsPoint *p2, QgsPoint *point )
{
  return angle( p1, point, p2, point ) > 90;
}

#if 0
bool MathUtils::inDiametral( QgsPoint *p1, QgsPoint *p2, QgsPoint *point )
{
  if ( p1 && p2 && point )
  {
    Vector3D p1p2( p2->getX() - p1->getX(), p2->getY() - p1->getY(), 0 );
    Vector3D orthogonalvec;//vector orthogonal to p1p2 (length radius)
    QgsPoint midpoint( ( p1->getX() + p2->getX() ) / 2, ( p1->getY() + p2->getY() ) / 2, 0 );
    double radius = p1p2.getLength() / 2;
    normalLeft( &p1p2, &orthogonalvec, radius );
    QgsPoint p3( midpoint.getX() + orthogonalvec.getX(), midpoint.getY() + orthogonalvec.getY(), 0 );
    return inCircle( point, p1, p2, &p3 );
  }
  else
  {
    cout << "null pointer in MathUtils::inDiametral" << endl << flush;
    return false;
  }
}
#endif // 0

double MathUtils::leftOf( const QgsPoint &thepoint, const QgsPoint *p1, const QgsPoint *p2 )
{
  if ( p1 && p2 )
  {
    //just for debugging
    const double f1 = thepoint.x() - p1->x();
    const double f2 = p2->y() - p1->y();
    const double f3 = thepoint.y() - p1->y();
    const double f4 = p2->x() - p1->x();
    return f1 * f2 - f3 * f4;
    //return thepoint->getX()-p1->getX())*(p2->getY()-p1->getY())-(thepoint->getY()-p1->getY())*(p2->getX()-p1->getX();//calculating the vectorproduct
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

bool MathUtils::lineIntersection( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4 )
{
  if ( p1 && p2 && p3 && p4 )
  {
    double t1, t2;
    const Vector3D p1p2( p2->x() - p1->x(), p2->y() - p1->y(), 0 );
    const Vector3D p3p4( p4->x() - p3->x(), p4->y() - p3->y(), 0 );
    if ( ( p3p4.getX()*p1p2.getY() - p3p4.getY()*p1p2.getX() ) != 0 && p1p2.getX() != 0 ) //avoid division through zero
    {
      t2 = ( p1->x() * p1p2.getY() - p1->y() * p1p2.getX() + p3->y() * p1p2.getX() - p3->x() * p1p2.getY() ) / ( p3p4.getX() * p1p2.getY() - p3p4.getY() * p1p2.getX() );
      t1 = ( p3->x() - p1->x() + t2 * p3p4.getX() ) / p1p2.getX();
    }
    else if ( ( p1p2.getX()*p3p4.getY() - p1p2.getY()*p3p4.getX() ) != 0 && p3p4.getX() != 0 )
    {
      t1 = ( p3->x() * p3p4.getY() - p3->y() * p3p4.getX() - p1->x() * p3p4.getY() + p1->y() * p3p4.getX() ) / ( p1p2.getX() * p3p4.getY() - p1p2.getY() * p3p4.getX() );
      t2 = ( p1->x() + t1 * p1p2.getX() - p3->x() ) / p3p4.getX();
    }
    else//the lines are parallel
    {
      return false;
    }

    if ( t1 > 0 && t1 < 1 && t2 > 0 && t2 < 1 )
    {
      if ( ( *p1 ) == ( *p3 ) || ( *p1 ) == ( *p4 ) || ( *p2 ) == ( *p3 ) || ( *p2 ) == ( *p4 ) ) //the lines touch each other, so they do not cross
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
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool MathUtils::lineIntersection( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4, QgsPoint *intersection_point )
{
  if ( p1 && p2 && p3 && p4 )
  {
    double t1, t2;
    const Vector3D p1p2( p2->x() - p1->x(), p2->y() - p1->y(), 0 );
    const Vector3D p3p4( p4->x() - p3->x(), p4->y() - p3->y(), 0 );
    if ( ( p3p4.getX()*p1p2.getY() - p3p4.getY()*p1p2.getX() ) != 0 && p1p2.getX() != 0 ) //avoid division through zero
    {
      t2 = ( p1->x() * p1p2.getY() - p1->y() * p1p2.getX() + p3->y() * p1p2.getX() - p3->x() * p1p2.getY() ) / ( p3p4.getX() * p1p2.getY() - p3p4.getY() * p1p2.getX() );
      t1 = ( p3->x() - p1->x() + t2 * p3p4.getX() ) / p1p2.getX();
    }
    else if ( ( p1p2.getX()*p3p4.getY() - p1p2.getY()*p3p4.getX() ) != 0 && p3p4.getX() != 0 )
    {
      t1 = ( p3->x() * p3p4.getY() - p3->y() * p3p4.getX() - p1->x() * p3p4.getY() + p1->y() * p3p4.getX() ) / ( p1p2.getX() * p3p4.getY() - p1p2.getY() * p3p4.getX() );
      t2 = ( p1->x() + t1 * p1p2.getX() - p3->x() ) / p3p4.getX();
    }
    else//the lines are parallel
    {
      intersection_point->setX( 0 );
      intersection_point->setY( 0 );
      intersection_point->setZ( 0 );
      return false;
    }

    if ( t1 > 0 && t1 < 1 && t2 > 0 && t2 < 1 )
    {
      if ( ( *p1 ) == ( *p3 ) || ( *p1 ) == ( *p4 ) || ( *p2 ) == ( *p3 ) || ( *p2 ) == ( *p4 ) ) //the lines touch each other, so they do not cross
      {
        intersection_point->setX( 0 );
        intersection_point->setY( 0 );
        intersection_point->setZ( 0 );
        return false;
      }
      //calculate the intersection point
      intersection_point->setX( p1->x() * ( 1 - t1 ) + p2->x()*t1 );
      intersection_point->setY( p1->y() * ( 1 - t1 ) + p2->y()*t1 );
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
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

int MathUtils::lower( int n, int i )
{
  if ( i >= 0 && i <= n )
  {
    return faculty( n ) / ( faculty( i ) * faculty( n - i ) );
  }
  else
  {
    return 0;
  }
}

double MathUtils::triArea( QgsPoint *pa, QgsPoint *pb, QgsPoint *pc )
{
  if ( pa && pb && pc )
  {
    const double deter = ( pa->x() * pb->y() + pb->x() * pc->y() + pc->x() * pa->y() - pa->x() * pc->y() - pb->x() * pa->y() - pc->x() * pb->y() );
    return 0.5 * deter;
  }
  else//null pointer
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

double MathUtils::calcCubicHermitePoly( int n, int i, double t )
{
  if ( n != 3 || i > n )
  {
    QgsDebugMsg( QStringLiteral( "error, can't calculate hermite polynom" ) );
  }

  if ( n == 3 && i == 0 )
  {
    return ( calcBernsteinPoly( 3, 0, t ) + calcBernsteinPoly( 3, 1, t ) );
  }

  if ( n == 3 && i == 1 )
  {
    return ( calcBernsteinPoly( 3, 1, t ) * 0.33333333 );
  }

  if ( n == 3 && i == 2 )
  {
    return ( calcBernsteinPoly( 3, 2, t ) * -0.33333333 );
  }

  if ( n == 3 && i == 3 )
  {
    return ( calcBernsteinPoly( 3, 2, t ) + calcBernsteinPoly( 3, 3, t ) );
  }
  else//something went wrong
  {
    QgsDebugMsg( QStringLiteral( "unexpected error" ) );
    return 0;
  }
}

double MathUtils::cFDerCubicHermitePoly( int n, int i, double t )
{
  if ( n != 3 || i > n )
  {
    QgsDebugMsg( QStringLiteral( "error, can't calculate hermite polynom" ) );
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
    QgsDebugMsg( QStringLiteral( "unexpected error" ) );
    return 0;
  }
}

bool MathUtils::derVec( const Vector3D *v1, const Vector3D *v2, Vector3D *result, double x, double y )
{
  if ( v1 && v2 && result )//no null pointers
  {
    const double u = ( x * v2->getY() - y * v2->getX() ) / ( v1->getX() * v2->getY() - v1->getY() * v2->getX() );
    const double v = ( x * v1->getY() - y * v1->getX() ) / ( v2->getX() * v1->getY() - v2->getY() * v1->getX() );
    result->setX( x );
    result->setY( y );
    result->setZ( u * v1->getZ() + v * v2->getZ() );
    return true;
  }
  return false;
}


bool MathUtils::normalLeft( Vector3D *v1, Vector3D *result, double length )
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
    const double a = 1 + ( v1->getX() * v1->getX() ) / ( v1->getY() * v1->getY() );
    const double b = 0;
    const double c = -( length * length );
    const double d = b * b - 4 * a * c;

    if ( d < 0 )//no solution in R
    {
      QgsDebugMsg( QStringLiteral( "Determinant Error" ) );
      return false;
    }

    result->setX( ( -b + std::sqrt( d ) ) / ( 2 * a ) ); //take one of the two solutions of the quadratic equation
    result->setY( ( -result->getX()*v1->getX() ) / v1->getY() );

    const QgsPoint point1( 0, 0, 0 );
    const QgsPoint point2( v1->getX(), v1->getY(), 0 );
    const QgsPoint point3( result->getX(), result->getY(), 0 );

    if ( !( leftOf( point1, &point2, &point3 ) < 0 ) )//if we took the solution on the right side, change the sign of the components of the result
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



bool MathUtils::normalRight( Vector3D *v1, Vector3D *result, double length )
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
    const double a = 1 + ( v1->getX() * v1->getX() ) / ( v1->getY() * v1->getY() );
    const double b = 0;
    const double c = -( length * length );
    const double d = b * b - 4 * a * c;

    if ( d < 0 )//no solution in R
    {
      QgsDebugMsg( QStringLiteral( "Determinant Error" ) );
      return false;
    }

    result->setX( ( -b + std::sqrt( d ) ) / ( 2 * a ) ); //take one of the two solutions of the quadratic equation
    result->setY( ( -result->getX()*v1->getX() ) / v1->getY() );

    const QgsPoint point1( 0, 0, 0 );
    const QgsPoint point2( v1->getX(), v1->getY(), 0 );
    const QgsPoint point3( result->getX(), result->getY(), 0 );

    if ( ( leftOf( point1, &point2, &point3 ) < 0 ) ) //if we took the solution on the right side, change the sign of the components of the result
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


void MathUtils::normalFromPoints( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, Vector3D *vec )
{
  if ( p1 && p2 && p3 && vec )//no null pointers
  {
    const double ax = p2->x() - p1->x();
    const double ay = p2->y() - p1->y();
    const double az = p2->z() - p1->z();
    const double bx = p3->x() - p1->x();
    const double by = p3->y() - p1->y();
    const double bz = p3->z() - p1->z();

    vec->setX( ay * bz - az * by );
    vec->setY( az * bx - ax * bz );
    vec->setZ( ax * by - ay * bx );
  }

}

double MathUtils::crossVec( QgsPoint *first, Vector3D *vec1, QgsPoint *second, Vector3D *vec2 )
{
  if ( first && vec1 && second && vec2 )
  {
    if ( ( vec2->getX()*vec1->getY() - vec2->getY()*vec1->getX() ) != 0 )
    {
      /*cout << "first: " << first->getX() << "//" << first->getY() << "//" << first->getZ() << endl << flush;
      cout << "vec1: " << vec1->getX() << "//" << vec1->getY() << "//" << vec1->getZ() << endl << flush;
      cout << "second: " << second->getX() << "//" << second->getY() << "//" << second->getZ() << endl << flush;
      cout << "vec2: " << vec2->getX() << "//" << vec2->getY() << "//" << vec2->getZ() << endl << flush;
      cout << "t2: " << ((first->getX()*vec1->getY()-first->getY()*vec1->getX()-second->getX()*vec1->getY()+second->getY()*vec1->getX())/(vec2->getX()*vec1->getY()-vec2->getY()*vec1->getX())) << endl << flush;*/

      return ( ( first->x() * vec1->getY() - first->y() * vec1->getX() - second->x() * vec1->getY() + second->y() * vec1->getX() ) / ( vec2->getX() * vec1->getY() - vec2->getY() * vec1->getX() ) );

    }
    else//if a division by zero would occur
    {
      QgsDebugMsg( QStringLiteral( "warning: vectors are parallel" ) );
      return 0;
    }
  }


  else//null pointer
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

bool MathUtils::pointInsideTriangle( double x, double y, QgsPoint *p1, QgsPoint *p2, QgsPoint *p3 )
{
  const QgsPoint thepoint( x, y, 0 );
  if ( MathUtils::leftOf( thepoint, p1, p2 ) > 0 )
  {
    return false;
  }
  if ( MathUtils::leftOf( thepoint, p2, p3 ) > 0 )
  {
    return false;
  }
  if ( MathUtils::leftOf( thepoint, p3, p1 ) > 0 )
  {
    return false;
  }
  return true;
}

bool MathUtils::normalMinDistance( Vector3D *tangent, Vector3D *target, Vector3D *result )
{
  if ( tangent && target && result )
  {
    const double xt = tangent->getX();
    const double yt = tangent->getY();
    const double zt = tangent->getZ();

    const double xw = target->getX();
    const double yw = target->getY();
    const double zw = target->getZ();

    double xg1, yg1, zg1;//the coordinates of the first result
    double xg2, yg2, zg2;//the coordinates of the second result

    //calculate xg
    const double xgalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( xgalpha1 < 0 )
    {
      QgsDebugMsg( QStringLiteral( "warning, only complex solution of xg" ) );
      return false;
    }
    xg1 = std::sqrt( xgalpha1 ) * ( -yt * yw * xt + yt * yt * xw + xw * zt * zt - zt * xt * zw );
    xg2 = -sqrt( xgalpha1 ) * ( -yt * yw * xt + yt * yt * xw + xw * zt * zt - zt * xt * zw );

    //calculate yg
    const double ygalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( ygalpha1 < 0 )
    {
      QgsDebugMsg( QStringLiteral( "warning, only complex solution of yg" ) );
      return false;
    }
    yg1 = -sqrt( ygalpha1 ) * ( -yw * xt * xt - zt * zt * yw + zt * yt * zw + yt * xw * xt );
    yg2 = std::sqrt( ygalpha1 ) * ( -yw * xt * xt - zt * zt * yw + zt * yt * zw + yt * xw * xt );

    //calculate zg
    const double zgalpha1 = 1 / ( 2 * xt * xt * yw * yw * zt * zt - 2 * zt * zt * zt * xt * zw * xw + yt * yt * yt * yt * zw * zw + yt * yt * zw * zw * zt * zt + xt * xt * yt * yt * xw * xw + xt * xt * yw * yw * yt * yt - 2 * xt * xt * xt * zt * zw * xw + yt * yt * yt * yt * xw * xw + yt * yt * yw * yw * zt * zt + 2 * xt * xt * yt * yt * zw * zw - 2 * yt * yt * yt * yw * zt * zw + zt * zt * xt * xt * zw * zw + zt * zt * zt * zt * xw * xw + xt * xt * zt * zt * xw * xw + 2 * zt * zt * xw * xw * yt * yt - 2 * xt * xt * yw * zt * yt * zw - 2 * xt * yt * yt * yt * xw * yw - 2 * xt * xt * xt * yw * yt * xw - 2 * xt * zt * zt * xw * yt * yw - 2 * xt * zt * xw * yt * yt * zw - 2 * yw * zt * zt * zt * yt * zw + xt * xt * xt * xt * yw * yw + yw * yw * zt * zt * zt * zt + xt * xt * xt * xt * zw * zw );
    if ( zgalpha1 < 0 )
    {
      QgsDebugMsg( QStringLiteral( "warning, only complex solution of zg" ) );
      return false;
    }
    zg1 = -sqrt( zgalpha1 ) * ( yt * yw * zt - yt * yt * zw + xw * zt * xt - xt * xt * zw );
    zg2 = std::sqrt( zgalpha1 ) * ( yt * yw * zt - yt * yt * zw + xw * zt * xt - xt * xt * zw );

    const double distance1 = std::sqrt( ( xw - xg1 ) * ( xw - xg1 ) + ( yw - yg1 ) * ( yw - yg1 ) + ( zw - zg1 ) * ( zw - zg1 ) );
    const double distance2 = std::sqrt( ( xw - xg2 ) * ( xw - xg2 ) + ( yw - yg2 ) * ( yw - yg2 ) + ( zw - zg2 ) * ( zw - zg2 ) );

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
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}


double MathUtils::planeTest( QgsPoint *test, QgsPoint *pt1, QgsPoint *pt2, QgsPoint *pt3 )
{
  if ( test && pt1 && pt2 && pt3 )
  {
    const double a = ( pt1->z() * ( pt2->y() - pt3->y() ) + pt2->z() * ( pt3->y() - pt1->y() ) + pt3->z() * ( pt1->y() - pt2->y() ) ) / ( ( pt1->x() - pt2->x() ) * ( pt2->y() - pt3->y() ) - ( pt2->x() - pt3->x() ) * ( pt1->y() - pt2->y() ) );
    const double b = ( pt1->z() * ( pt2->x() - pt3->x() ) + pt2->z() * ( pt3->x() - pt1->x() ) + pt3->z() * ( pt1->x() - pt2->x() ) ) / ( ( pt1->y() - pt2->y() ) * ( pt2->x() - pt3->x() ) - ( pt2->y() - pt3->y() ) * ( pt1->x() - pt2->x() ) );
    const double c = pt1->z() - a * pt1->x() - b * pt1->y();
    const double zpredicted = test->x() * a + test->y() * b + c;
    return ( test->z() - zpredicted );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}

double MathUtils::angle( QgsPoint *p1, QgsPoint *p2, QgsPoint *p3, QgsPoint *p4 )
{
  if ( p1 && p2 && p3 && p4 )
  {
    const Vector3D v1( p2->x() - p1->x(), p2->y() - p1->y(), 0 );
    const Vector3D v2( p4->x() - p3->x(), p4->y() - p3->y(), 0 );
    const double value = acos( ( v1.getX() * v2.getX() + v1.getY() * v2.getY() ) / ( v1.getLength() * v2.getLength() ) ) * 180 / M_PI;
    return value;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return 0;
  }
}
