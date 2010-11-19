/***************************************************************************
                          CloughTocherInterpolator.cc  -  description
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

#include "CloughTocherInterpolator.h"
#include "qgslogger.h"

#include <cmath>

double CloughTocherInterpolator::calcBernsteinPoly( int n, int i, int j, int k, double u, double v, double w )
{
  double result = MathUtils::faculty( n ) * MathUtils::power( u, i ) * MathUtils::power( v, j ) * MathUtils::power( w, k ) / ( MathUtils::faculty( i ) * MathUtils::faculty( j ) * MathUtils::faculty( k ) );
  return result;
}

bool CloughTocherInterpolator::calcNormVec( double x, double y, Vector3D* result )
{
  if ( result )
  {
    init( x, y );
    Point3D barycoord( 0, 0, 0 );//barycentric coordinates of (x,y) whith respect to the triangle
    Point3D endpointUXY( 0, 0, 0 );//endpoint of the derivative in u-direction (in xy-coordinates)
    Point3D endpointV( 0, 0, 0 );//endpoint of the derivative in v-direction (in barycentric coordinates)
    Point3D endpointVXY( 0, 0, 0 );//endpoint of the derivative in v-direction (in xy-coordinates)

    //find out, in which subtriangle the point (x,y) is
    //is the point in the first subtriangle (point1,point2,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point1, &point2, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double zu = point1.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp1.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp2.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp3.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp4.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zv = cp1.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp2.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point2.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp4.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp5.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zw = cp3.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp4.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp5.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp10.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      MathUtils::BarycentricToXY( barycoord.getX() + 1, barycoord.getY() - 1, barycoord.getZ(), &point1, &point2, &cp10, &endpointUXY );
      endpointUXY.setZ( 3*( zu - zv ) );
      MathUtils::BarycentricToXY( barycoord.getX(), barycoord.getY() + 1, barycoord.getZ() - 1, &point1, &point2, &cp10, &endpointVXY );
      endpointVXY.setZ( 3*( zv - zw ) );
      Vector3D v1( endpointUXY.getX() - x, endpointUXY.getY() - y, endpointUXY.getZ() );
      Vector3D v2( endpointVXY.getX() - x, endpointVXY.getY() - y, endpointVXY.getZ() );
      result->setX( v1.getY()*v2.getZ() - v1.getZ()*v2.getY() );
      result->setY( v1.getZ()*v2.getX() - v1.getX()*v2.getZ() );
      result->setZ( v1.getX()*v2.getY() - v1.getY()*v2.getX() );
      result->standardise();
      return true;
    }
    //is the point in the second subtriangle (point2,point3,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point2, &point3, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double zu = point2.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp9.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp16.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp5.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp13.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zv = cp9.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp16.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point3.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp13.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp15.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zw = cp5.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp13.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp15.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp10.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      MathUtils::BarycentricToXY( barycoord.getX() + 1, barycoord.getY() - 1, barycoord.getZ(), &point2, &point3, &cp10, &endpointUXY );
      endpointUXY.setZ( 3*( zu - zv ) );
      MathUtils::BarycentricToXY( barycoord.getX(), barycoord.getY() + 1, barycoord.getZ() - 1, &point2, &point3, &cp10, &endpointVXY );
      endpointVXY.setZ( 3*( zv - zw ) );
      Vector3D v1( endpointUXY.getX() - x, endpointUXY.getY() - y, endpointUXY.getZ() );
      Vector3D v2( endpointVXY.getX() - x, endpointVXY.getY() - y, endpointVXY.getZ() );
      result->setX( v1.getY()*v2.getZ() - v1.getZ()*v2.getY() );
      result->setY( v1.getZ()*v2.getX() - v1.getX()*v2.getZ() );
      result->setZ( v1.getX()*v2.getY() - v1.getY()*v2.getX() );
      result->standardise();
      return true;

    }
    //is the point in the third subtriangle (point3,point1,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point3, &point1, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double zu = point3.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp14.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp6.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp15.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp11.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zv = cp14.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp6.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point1.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp11.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp3.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      double zw = cp15.getZ() * calcBernsteinPoly( 2, 2, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp11.getZ() * calcBernsteinPoly( 2, 1, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp3.getZ() * calcBernsteinPoly( 2, 0, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 2, 1, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 2, 0, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp10.getZ() * calcBernsteinPoly( 2, 0, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      MathUtils::BarycentricToXY( barycoord.getX() + 1, barycoord.getY() - 1, barycoord.getZ(), &point3, &point1, &cp10, &endpointUXY );
      endpointUXY.setZ( 3*( zu - zv ) );
      MathUtils::BarycentricToXY( barycoord.getX(), barycoord.getY() + 1, barycoord.getZ() - 1, &point3, &point1, &cp10, &endpointVXY );
      endpointVXY.setZ( 3*( zv - zw ) );
      Vector3D v1( endpointUXY.getX() - x, endpointUXY.getY() - y, endpointUXY.getZ() );
      Vector3D v2( endpointVXY.getX() - x, endpointVXY.getY() - y, endpointVXY.getZ() );
      result->setX( v1.getY()*v2.getZ() - v1.getZ()*v2.getY() );
      result->setY( v1.getZ()*v2.getX() - v1.getX()*v2.getZ() );
      result->setZ( v1.getX()*v2.getY() - v1.getY()*v2.getX() );
      result->standardise();
      return true;
    }

    //the point is in none of the subtriangles, test if point has the same position as one of the vertices
    if ( x == point1.getX() && y == point1.getY() )
    {
      result->setX( -der1X );
      result->setY( -der1Y );
      result->setZ( 1 );
      result->standardise();
      return true;
    }
    else if ( x == point2.getX() && y == point2.getY() )
    {
      result->setX( -der2X );
      result->setY( -der2Y );
      result->setZ( 1 );
      result->standardise();
      return true;
    }
    else if ( x == point3.getX() && y == point3.getY() )
    {
      result->setX( -der3X );
      result->setY( -der3Y );
      result->setZ( 1 );
      result->standardise();
      return true;
    }

    QgsDebugMsg( "warning, point outside the triangle" );
    result->setX( 0 );//return a vertical normal if failed
    result->setY( 0 );
    result->setZ( 1 );
    return false;

  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool CloughTocherInterpolator::calcPoint( double x, double y, Point3D* result )
{
  if ( result )
  {
    init( x, y );
    //find out, in which subtriangle the point (x,y) is
    Point3D barycoord( 0, 0, 0 );
    //is the point in the first subtriangle (point1,point2,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point1, &point2, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double z = point1.getZ() * calcBernsteinPoly( 3, 3, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp1.getZ() * calcBernsteinPoly( 3, 2, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp2.getZ() * calcBernsteinPoly( 3, 1, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point2.getZ() * calcBernsteinPoly( 3, 0, 3, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp3.getZ() * calcBernsteinPoly( 3, 2, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp4.getZ() * calcBernsteinPoly( 3, 1, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp5.getZ() * calcBernsteinPoly( 3, 0, 2, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 3, 1, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 3, 0, 1, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp10.getZ() * calcBernsteinPoly( 3, 0, 0, 3, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      result->setX( x );
      result->setY( y );
      result->setZ( z );
      return true;
    }
    //is the point in the second subtriangle (point2,point3,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point2, &point3, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double z = cp10.getZ() * calcBernsteinPoly( 3, 0, 0, 3, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp8.getZ() * calcBernsteinPoly( 3, 1, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp5.getZ() * calcBernsteinPoly( 3, 2, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point2.getZ() * calcBernsteinPoly( 3, 3, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 3, 0, 1, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp13.getZ() * calcBernsteinPoly( 3, 1, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp9.getZ() * calcBernsteinPoly( 3, 2, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp15.getZ() * calcBernsteinPoly( 3, 0, 2, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp16.getZ() * calcBernsteinPoly( 3, 1, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point3.getZ() * calcBernsteinPoly( 3, 0, 3, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      result->setX( x );
      result->setY( y );
      result->setZ( z );
      return true;
    }
    //is the point in the third subtriangle (point3,point1,cp10)?
    MathUtils::calcBarycentricCoordinates( x, y, &point3, &point1, &cp10, &barycoord );
    if ( barycoord.getX() >= -mEdgeTolerance && barycoord.getX() <= ( 1 + mEdgeTolerance ) && barycoord.getY() >= -mEdgeTolerance && barycoord.getY() <= ( 1 + mEdgeTolerance ) )
    {
      double z = point1.getZ() * calcBernsteinPoly( 3, 0, 3, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp3.getZ() * calcBernsteinPoly( 3, 0, 2, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp7.getZ() * calcBernsteinPoly( 3, 0, 1, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp10.getZ() * calcBernsteinPoly( 3, 0, 0, 3, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp6.getZ() * calcBernsteinPoly( 3, 1, 2, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp11.getZ() * calcBernsteinPoly( 3, 1, 1, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp12.getZ() * calcBernsteinPoly( 3, 1, 0, 2, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp14.getZ() * calcBernsteinPoly( 3, 2, 1, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + cp15.getZ() * calcBernsteinPoly( 3, 2, 0, 1, barycoord.getX(), barycoord.getY(), barycoord.getZ() ) + point3.getZ() * calcBernsteinPoly( 3, 3, 0, 0, barycoord.getX(), barycoord.getY(), barycoord.getZ() );
      result->setX( x );
      result->setY( y );
      result->setZ( z );
      return true;
    }

    //the point is in none of the subtriangles, test if point has the same position as one of the vertices
    if ( x == point1.getX() && y == point1.getY() )
    {
      result->setX( x );
      result->setY( y );
      result->setZ( point1.getZ() );
      return true;
    }
    else if ( x == point2.getX() && y == point2.getY() )
    {
      result->setX( x );
      result->setY( y );
      result->setZ( point2.getZ() );
      return true;
    }
    else if ( x == point3.getX() && y == point3.getY() )
    {
      result->setX( x );
      result->setY( y );
      result->setZ( point3.getZ() );
      return true;
    }
    else
    {
      QgsDebugMsg( "warning, point outside the triangle" );
      result->setX( x );
      result->setY( y );
      result->setZ( 0 );
    }

    return false;

  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

void CloughTocherInterpolator::init( double x, double y )//version, which has the unintended breaklines within the macrotriangles
{
  Vector3D v1, v2, v3;//normals at the three data points
  int ptn1, ptn2, ptn3;//numbers of the vertex points
  NormVecDecorator::pointState state1, state2, state3;//states of the vertex points (NORMAL, BREAKLINE, ENDPOINT possible)

  if ( mTIN )
  {
    mTIN->getTriangle( x, y, &point1, &ptn1, &v1, &state1, &point2, &ptn2, &v2, &state2, &point3, &ptn3, &v3, &state3 );

    if ( point1 == lpoint1 && point2 == lpoint2 && point3 == lpoint3 )//if we are in the same triangle as at the last run, we can leave 'init'
    {
      return;
    }

    //calculate the partial derivatives at the data points
    der1X = -v1.getX() / v1.getZ();
    der1Y = -v1.getY() / v1.getZ();
    der2X = -v2.getX() / v2.getZ();
    der2Y = -v2.getY() / v2.getZ();
    der3X = -v3.getX() / v3.getZ();
    der3Y = -v3.getY() / v3.getZ();

    //calculate the control points
    cp1.setX( point1.getX() + ( point2.getX() - point1.getX() ) / 3 );
    cp1.setY( point1.getY() + ( point2.getY() - point1.getY() ) / 3 );
    cp1.setZ( point1.getZ() + ( cp1.getX() - point1.getX() )*der1X + ( cp1.getY() - point1.getY() )*der1Y );

    cp2.setX( point2.getX() + ( point1.getX() - point2.getX() ) / 3 );
    cp2.setY( point2.getY() + ( point1.getY() - point2.getY() ) / 3 );
    cp2.setZ( point2.getZ() + ( cp2.getX() - point2.getX() )*der2X + ( cp2.getY() - point2.getY() )*der2Y );

    cp9.setX( point2.getX() + ( point3.getX() - point2.getX() ) / 3 );
    cp9.setY( point2.getY() + ( point3.getY() - point2.getY() ) / 3 );
    cp9.setZ( point2.getZ() + ( cp9.getX() - point2.getX() )*der2X + ( cp9.getY() - point2.getY() )*der2Y );

    cp16.setX( point3.getX() + ( point2.getX() - point3.getX() ) / 3 );
    cp16.setY( point3.getY() + ( point2.getY() - point3.getY() ) / 3 );
    cp16.setZ( point3.getZ() + ( cp16.getX() - point3.getX() )*der3X + ( cp16.getY() - point3.getY() )*der3Y );

    cp14.setX( point3.getX() + ( point1.getX() - point3.getX() ) / 3 );
    cp14.setY( point3.getY() + ( point1.getY() - point3.getY() ) / 3 );
    cp14.setZ( point3.getZ() + ( cp14.getX() - point3.getX() )*der3X + ( cp14.getY() - point3.getY() )*der3Y );

    cp6.setX( point1.getX() + ( point3.getX() - point1.getX() ) / 3 );
    cp6.setY( point1.getY() + ( point3.getY() - point1.getY() ) / 3 );
    cp6.setZ( point1.getZ() + ( cp6.getX() - point1.getX() )*der1X + ( cp6.getY() - point1.getY() )*der1Y );

    //set x- and y-coordinates of the central point
    cp10.setX(( point1.getX() + point2.getX() + point3.getX() ) / 3 );
    cp10.setY(( point1.getY() + point2.getY() + point3.getY() ) / 3 );

    //set the derivatives of the points to new values if they are on a breakline
    if ( state1 == NormVecDecorator::BREAKLINE )
    {
      Vector3D target1;
      if ( mTIN->calcNormalForPoint( x, y, ptn1, &target1 ) )
      {
        der1X = -target1.getX() / target1.getZ();
        der1Y = -target1.getY() / target1.getZ();

        if ( state2 == NormVecDecorator::NORMAL )
        {
          //recalculate cp1
          cp1.setZ( point1.getZ() + ( cp1.getX() - point1.getX() )*der1X + ( cp1.getY() - point1.getY() )*der1Y );
        }
        if ( state3 == NormVecDecorator::NORMAL )
        {
          //recalculate cp6
          cp6.setZ( point1.getZ() + ( cp6.getX() - point1.getX() )*der1X + ( cp6.getY() - point1.getY() )*der1Y );
        }
      }
    }

    if ( state2 == NormVecDecorator::BREAKLINE )
    {
      Vector3D target2;
      if ( mTIN->calcNormalForPoint( x, y, ptn2, &target2 ) )
      {
        der2X = -target2.getX() / target2.getZ();
        der2Y = -target2.getY() / target2.getZ();

        if ( state1 == NormVecDecorator::NORMAL )
        {
          //recalculate cp2
          cp2.setZ( point2.getZ() + ( cp2.getX() - point2.getX() )*der2X + ( cp2.getY() - point2.getY() )*der2Y );
        }
        if ( state3 == NormVecDecorator::NORMAL )
        {
          //recalculate cp9
          cp9.setZ( point2.getZ() + ( cp9.getX() - point2.getX() )*der2X + ( cp9.getY() - point2.getY() )*der2Y );
        }
      }
    }

    if ( state3 == NormVecDecorator::BREAKLINE )
    {
      Vector3D target3;
      if ( mTIN->calcNormalForPoint( x, y, ptn3, &target3 ) )
      {
        der3X = -target3.getX() / target3.getZ();
        der3Y = -target3.getY() / target3.getZ();

        if ( state1 == NormVecDecorator::NORMAL )
        {
          //recalculate cp14
          cp14.setZ( point3.getZ() + ( cp14.getX() - point3.getX() )*der3X + ( cp14.getY() - point3.getY() )*der3Y );
        }
        if ( state2 == NormVecDecorator::NORMAL )
        {
          //recalculate cp16
          cp16.setZ( point3.getZ() + ( cp16.getX() - point3.getX() )*der3X + ( cp16.getY() - point3.getY() )*der3Y );
        }
      }
    }


    cp3.setX( point1.getX() + ( cp10.getX() - point1.getX() ) / 3 );
    cp3.setY( point1.getY() + ( cp10.getY() - point1.getY() ) / 3 );
    cp3.setZ( point1.getZ() + ( cp3.getX() - point1.getX() )*der1X + ( cp3.getY() - point1.getY() )*der1Y );

    cp5.setX( point2.getX() + ( cp10.getX() - point2.getX() ) / 3 );
    cp5.setY( point2.getY() + ( cp10.getY() - point2.getY() ) / 3 );
    cp5.setZ( point2.getZ() + ( cp5.getX() - point2.getX() )*der2X + ( cp5.getY() - point2.getY() )*der2Y );

    cp15.setX( point3.getX() + ( cp10.getX() - point3.getX() ) / 3 );
    cp15.setY( point3.getY() + ( cp10.getY() - point3.getY() ) / 3 );
    cp15.setZ( point3.getZ() + ( cp15.getX() - point3.getX() )*der3X + ( cp15.getY() - point3.getY() )*der3Y );


    cp4.setX(( point1.getX() + cp10.getX() + point2.getX() ) / 3 );
    cp4.setY(( point1.getY() + cp10.getY() + point2.getY() ) / 3 );

    Point3D midpoint3(( cp1.getX() + cp2.getX() ) / 2, ( cp1.getY() + cp2.getY() ) / 2, ( cp1.getZ() + cp2.getZ() ) / 2 );
    Vector3D cp1cp2( cp2.getX() - cp1.getX(), cp2.getY() - cp1.getY(), cp2.getZ() - cp1.getZ() );
    Vector3D odir3( 0, 0, 0 );//direction orthogonal to the line from point1 to point2
    if (( point2.getY() - point1.getY() ) != 0 )//avoid division through zero
    {
      odir3.setX( 1 );
      odir3.setY( -( point2.getX() - point1.getX() ) / ( point2.getY() - point1.getY() ) );
      odir3.setZ(( der1X + der2X ) / 2 + ( der1Y + der2Y ) / 2*odir3.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir3.setY( 1 );
      odir3.setX( -( point2.getY() - point1.getY() ) / ( point2.getX() - point1.getX() ) );
      odir3.setZ(( der1X + der2X ) / 2*odir3.getX() + ( der1Y + der2Y ) / 2 );
    }
    Vector3D midpoint3cp4( 0, 0, 0 );
    MathUtils::derVec( &cp1cp2, &odir3, &midpoint3cp4, cp4.getX() - midpoint3.getX(), cp4.getY() - midpoint3.getY() );
    cp4.setZ( midpoint3.getZ() + midpoint3cp4.getZ() );

    cp13.setX(( point2.getX() + cp10.getX() + point3.getX() ) / 3 );
    cp13.setY(( point2.getY() + cp10.getY() + point3.getY() ) / 3 );
    //find the point in the middle of the bezier curve between point2 and point3
    Point3D midpoint1(( cp9.getX() + cp16.getX() ) / 2, ( cp9.getY() + cp16.getY() ) / 2, ( cp9.getZ() + cp16.getZ() ) / 2 );
    Vector3D cp9cp16( cp16.getX() - cp9.getX(), cp16.getY() - cp9.getY(), cp16.getZ() - cp9.getZ() );
    Vector3D odir1( 0, 0, 0 );//direction orthogonal to the line from point2 to point3
    if (( point3.getY() - point2.getY() ) != 0 )//avoid division through zero
    {
      odir1.setX( 1 );
      odir1.setY( -( point3.getX() - point2.getX() ) / ( point3.getY() - point2.getY() ) );
      odir1.setZ(( der2X + der3X ) / 2 + ( der2Y + der3Y ) / 2*odir1.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir1.setY( 1 );
      odir1.setX( -( point3.getY() - point2.getY() ) / ( point3.getX() - point2.getX() ) );
      odir1.setZ(( der2X + der3X ) / 2*odir1.getX() + ( der2Y + der3Y ) / 2 );
    }
    Vector3D midpoint1cp13( 0, 0, 0 );
    MathUtils::derVec( &cp9cp16, &odir1, &midpoint1cp13, cp13.getX() - midpoint1.getX(), cp13.getY() - midpoint1.getY() );
    cp13.setZ( midpoint1.getZ() + midpoint1cp13.getZ() );


    cp11.setX(( point3.getX() + cp10.getX() + point1.getX() ) / 3 );
    cp11.setY(( point3.getY() + cp10.getY() + point1.getY() ) / 3 );
    //find the point in the middle of the bezier curve between point3 and point2
    Point3D midpoint2(( cp14.getX() + cp6.getX() ) / 2, ( cp14.getY() + cp6.getY() ) / 2, ( cp14.getZ() + cp6.getZ() ) / 2 );
    Vector3D cp14cp6( cp6.getX() - cp14.getX(), cp6.getY() - cp14.getY(), cp6.getZ() - cp14.getZ() );
    Vector3D odir2( 0, 0, 0 );//direction orthogonal to the line from point 1 to point3
    if (( point3.getY() - point1.getY() ) != 0 )//avoid division through zero
    {
      odir2.setX( 1 );
      odir2.setY( -( point3.getX() - point1.getX() ) / ( point3.getY() - point1.getY() ) );
      odir2.setZ(( der3X + der1X ) / 2 + ( der3Y + der1Y ) / 2*odir2.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir2.setY( 1 );
      odir2.setX( -( point3.getY() - point1.getY() ) / ( point3.getX() - point1.getX() ) );
      odir2.setZ(( der3X + der1X ) / 2*odir2.getX() + ( der3Y + der1Y ) / 2 );
    }
    Vector3D midpoint2cp11( 0, 0, 0 );
    MathUtils::derVec( &cp14cp6, &odir2, &midpoint2cp11, cp11.getX() - midpoint2.getX(), cp11.getY() - midpoint2.getY() );
    cp11.setZ( midpoint2.getZ() + midpoint2cp11.getZ() );


    cp7.setX( cp10.getX() + ( point1.getX() - cp10.getX() ) / 3 );
    cp7.setY( cp10.getY() + ( point1.getY() - cp10.getY() ) / 3 );
    //cp7 has to be in the same plane as cp4, cp3 and cp11
    Vector3D cp4cp3( cp3.getX() - cp4.getX(), cp3.getY() - cp4.getY(), cp3.getZ() - cp4.getZ() );
    Vector3D cp4cp11( cp11.getX() - cp4.getX(), cp11.getY() - cp4.getY(), cp11.getZ() - cp4.getZ() );
    Vector3D cp4cp7( 0, 0, 0 );
    MathUtils::derVec( &cp4cp3, &cp4cp11, &cp4cp7, cp7.getX() - cp4.getX(), cp7.getY() - cp4.getY() );
    cp7.setZ( cp4.getZ() + cp4cp7.getZ() );

    cp8.setX( cp10.getX() + ( point2.getX() - cp10.getX() ) / 3 );
    cp8.setY( cp10.getY() + ( point2.getY() - cp10.getY() ) / 3 );
    //cp8 has to be in the same plane as cp4, cp5 and cp13
    Vector3D cp4cp5( cp5.getX() - cp4.getX(), cp5.getY() - cp4.getY(), cp5.getZ() - cp4.getZ() );
    Vector3D cp4cp13( cp13.getX() - cp4.getX(), cp13.getY() - cp4.getY(), cp13.getZ() - cp4.getZ() );
    Vector3D cp4cp8( 0, 0, 0 );
    MathUtils::derVec( &cp4cp5, &cp4cp13, &cp4cp8, cp8.getX() - cp4.getX(), cp8.getY() - cp4.getY() );
    cp8.setZ( cp4.getZ() + cp4cp8.getZ() );

    cp12.setX( cp10.getX() + ( point3.getX() - cp10.getX() ) / 3 );
    cp12.setY( cp10.getY() + ( point3.getY() - cp10.getY() ) / 3 );
    //cp12 has to be in the same plane as cp13, cp15 and cp11
    Vector3D cp13cp11( cp11.getX() - cp13.getX(), cp11.getY() - cp13.getY(), cp11.getZ() - cp13.getZ() );
    Vector3D cp13cp15( cp15.getX() - cp13.getX(), cp15.getY() - cp13.getY(), cp15.getZ() - cp13.getZ() );
    Vector3D cp13cp12( 0, 0, 0 );
    MathUtils::derVec( &cp13cp11, &cp13cp15, &cp13cp12, cp12.getX() - cp13.getX(), cp12.getY() - cp13.getY() );
    cp12.setZ( cp13.getZ() + cp13cp12.getZ() );

    //cp10 has to be in the same plane as cp7, cp8 and cp12
    Vector3D cp7cp8( cp8.getX() - cp7.getX(), cp8.getY() - cp7.getY(), cp8.getZ() - cp7.getZ() );
    Vector3D cp7cp12( cp12.getX() - cp7.getX(), cp12.getY() - cp7.getY(), cp12.getZ() - cp7.getZ() );
    Vector3D cp7cp10( 0, 0, 0 );
    MathUtils::derVec( &cp7cp8, &cp7cp12, &cp7cp10, cp10.getX() - cp7.getX(), cp10.getY() - cp7.getY() );
    cp10.setZ( cp7.getZ() + cp7cp10.getZ() );

    lpoint1 = point1;
    lpoint2 = point2;
    lpoint3 = point3;


  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}


#if 0
void CloughTocherInterpolator::init( double x, double y )//version which has unintended breaklines similar to the Coons interpolator
{
  Vector3D v1, v2, v3;//normals at the three data points
  int ptn1, ptn2, ptn3;//numbers of the vertex points
  NormVecDecorator::pointState state1, state2, state3;//states of the vertex points (NORMAL, BREAKLINE, ENDPOINT possible)

  if ( mTIN )
  {
    mTIN->getTriangle( x, y, &point1, &ptn1, &v1, &state1, &point2, &ptn2, &v2, &state2, &point3, &ptn3, &v3, &state3 );

    if ( point1 == lpoint1 && point2 == lpoint2 && point3 == lpoint3 )//if we are in the same triangle as at the last run, we can leave 'init'
    {
      return;
    }

    //calculate the partial derivatives at the data points
    der1X = -v1.getX() / v1.getZ();
    der1Y = -v1.getY() / v1.getZ();
    der2X = -v2.getX() / v2.getZ();
    der2Y = -v2.getY() / v2.getZ();
    der3X = -v3.getX() / v3.getZ();
    der3Y = -v3.getY() / v3.getZ();

    //calculate the control points
    cp1.setX( point1.getX() + ( point2.getX() - point1.getX() ) / 3 );
    cp1.setY( point1.getY() + ( point2.getY() - point1.getY() ) / 3 );
    cp1.setZ( point1.getZ() + ( cp1.getX() - point1.getX() )*der1X + ( cp1.getY() - point1.getY() )*der1Y );

    cp2.setX( point2.getX() + ( point1.getX() - point2.getX() ) / 3 );
    cp2.setY( point2.getY() + ( point1.getY() - point2.getY() ) / 3 );
    cp2.setZ( point2.getZ() + ( cp2.getX() - point2.getX() )*der2X + ( cp2.getY() - point2.getY() )*der2Y );

    cp9.setX( point2.getX() + ( point3.getX() - point2.getX() ) / 3 );
    cp9.setY( point2.getY() + ( point3.getY() - point2.getY() ) / 3 );
    cp9.setZ( point2.getZ() + ( cp9.getX() - point2.getX() )*der2X + ( cp9.getY() - point2.getY() )*der2Y );

    cp16.setX( point3.getX() + ( point2.getX() - point3.getX() ) / 3 );
    cp16.setY( point3.getY() + ( point2.getY() - point3.getY() ) / 3 );
    cp16.setZ( point3.getZ() + ( cp16.getX() - point3.getX() )*der3X + ( cp16.getY() - point3.getY() )*der3Y );

    cp14.setX( point3.getX() + ( point1.getX() - point3.getX() ) / 3 );
    cp14.setY( point3.getY() + ( point1.getY() - point3.getY() ) / 3 );
    cp14.setZ( point3.getZ() + ( cp14.getX() - point3.getX() )*der3X + ( cp14.getY() - point3.getY() )*der3Y );

    cp6.setX( point1.getX() + ( point3.getX() - point1.getX() ) / 3 );
    cp6.setY( point1.getY() + ( point3.getY() - point1.getY() ) / 3 );
    cp6.setZ( point1.getZ() + ( cp6.getX() - point1.getX() )*der1X + ( cp6.getY() - point1.getY() )*der1Y );

    //set x- and y-coordinates of the central point
    cp10.setX(( point1.getX() + point2.getX() + point3.getX() ) / 3 );
    cp10.setY(( point1.getY() + point2.getY() + point3.getY() ) / 3 );

    //do the necessary adjustments in case of breaklines--------------------------------------------------------------------

    //temporary normals and derivatives
    double tmpx = 0;
    double tmpy = 0;
    Vector3D tmp( 0, 0, 0 );

    //point1
    if ( state1 == NormVecDecorator::BREAKLINE )
    {
      if ( mTIN->calcNormalForPoint( x, y, ptn1, &tmp ) )
      {
        tmpx = -tmp.getX() / tmp.getZ();
        tmpy = -tmp.getY() / tmp.getZ();
        if ( state3 == NormVecDecorator::NORMAL )
        {
          cp6.setZ( point1.getZ() + (( point3.getX() - point1.getX() ) / 3 )*tmpx + (( point3.getY() - point1.getY() ) / 3 )*tmpy );
        }
        if ( state2 == NormVecDecorator::NORMAL )
        {
          cp1.setZ( point1.getZ() + (( point2.getX() - point1.getX() ) / 3 )*tmpx + (( point2.getY() - point1.getY() ) / 3 )*tmpy );
        }
      }
    }

    if ( state2 == NormVecDecorator::BREAKLINE )
    {
      if ( mTIN->calcNormalForPoint( x, y, ptn2, &tmp ) )
      {
        tmpx = -tmp.getX() / tmp.getZ();
        tmpy = -tmp.getY() / tmp.getZ();
        if ( state1 == NormVecDecorator::NORMAL )
        {
          cp2.setZ( point2.getZ() + (( point1.getX() - point2.getX() ) / 3 )*tmpx + (( point1.getY() - point2.getY() ) / 3 )*tmpy );
        }
        if ( state3 == NormVecDecorator::NORMAL )
        {
          cp9.setZ( point2.getZ() + (( point3.getX() - point2.getX() ) / 3 )*tmpx + (( point3.getY() - point2.getY() ) / 3 )*tmpy );
        }
      }
    }

    if ( state3 == NormVecDecorator::BREAKLINE )
    {
      if ( mTIN->calcNormalForPoint( x, y, ptn3, &tmp ) )
      {
        tmpx = -tmp.getX() / tmp.getZ();
        tmpy = -tmp.getY() / tmp.getZ();
        if ( state1 == NormVecDecorator::NORMAL )
        {
          cp14.setZ( point3.getZ() + (( point1.getX() - point3.getX() ) / 3 )*tmpx + (( point1.getY() - point3.getY() ) / 3 )*tmpy );
        }
        if ( state2 == NormVecDecorator::NORMAL )
        {
          cp16.setZ( point3.getZ() + (( point2.getX() - point3.getX() ) / 3 )*tmpx + (( point2.getY() - point3.getY() ) / 3 )*tmpy );
        }
      }
    }

    //calculate cp3, cp 5 and cp15
    Vector3D normal;//temporary normal for the vertices on breaklines

    cp3.setX( point1.getX() + ( cp10.getX() - point1.getX() ) / 3 );
    cp3.setY( point1.getY() + ( cp10.getY() - point1.getY() ) / 3 );
    if ( state1 == NormVecDecorator::BREAKLINE )
    {
      MathUtils::normalFromPoints( &point1, &cp1, &cp6, &normal );
      //recalculate der1X and der1Y
      der1X = -normal.getX() / normal.getZ();
      der1Y = -normal.getY() / normal.getZ();
    }

    cp3.setZ( point1.getZ() + ( cp3.getX() - point1.getX() )*der1X + ( cp3.getY() - point1.getY() )*der1Y );



    cp5.setX( point2.getX() + ( cp10.getX() - point2.getX() ) / 3 );
    cp5.setY( point2.getY() + ( cp10.getY() - point2.getY() ) / 3 );
    if ( state2 == NormVecDecorator::BREAKLINE )
    {
      MathUtils::normalFromPoints( &point2, &cp9, &cp2, &normal );
      //recalculate der2X and der2Y
      der2X = -normal.getX() / normal.getZ();
      der2Y = -normal.getY() / normal.getZ();
    }

    cp5.setZ( point2.getZ() + ( cp5.getX() - point2.getX() )*der2X + ( cp5.getY() - point2.getY() )*der2Y );


    cp15.setX( point3.getX() + ( cp10.getX() - point3.getX() ) / 3 );
    cp15.setY( point3.getY() + ( cp10.getY() - point3.getY() ) / 3 );
    if ( state3 == NormVecDecorator::BREAKLINE )
    {
      MathUtils::normalFromPoints( &point3, &cp14, &cp16, &normal );
      //recalculate der3X and der3Y
      der3X = -normal.getX() / normal.getZ();
      der3Y = -normal.getY() / normal.getZ();
    }

    cp15.setZ( point3.getZ() + ( cp15.getX() - point3.getX() )*der3X + ( cp15.getY() - point3.getY() )*der3Y );


    cp4.setX(( point1.getX() + cp10.getX() + point2.getX() ) / 3 );
    cp4.setY(( point1.getY() + cp10.getY() + point2.getY() ) / 3 );

    Point3D midpoint3(( cp1.getX() + cp2.getX() ) / 2, ( cp1.getY() + cp2.getY() ) / 2, ( cp1.getZ() + cp2.getZ() ) / 2 );
    Vector3D cp1cp2( cp2.getX() - cp1.getX(), cp2.getY() - cp1.getY(), cp2.getZ() - cp1.getZ() );
    Vector3D odir3( 0, 0, 0 );//direction orthogonal to the line from point1 to point2
    if (( point2.getY() - point1.getY() ) != 0 )//avoid division through zero
    {
      odir3.setX( 1 );
      odir3.setY( -( point2.getX() - point1.getX() ) / ( point2.getY() - point1.getY() ) );
      odir3.setZ(( der1X + der2X ) / 2 + ( der1Y + der2Y ) / 2*odir3.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir3.setY( 1 );
      odir3.setX( -( point2.getY() - point1.getY() ) / ( point2.getX() - point1.getX() ) );
      odir3.setZ(( der1X + der2X ) / 2*odir3.getX() + ( der1Y + der2Y ) / 2 );
    }
    Vector3D midpoint3cp4( 0, 0, 0 );
    MathUtils::derVec( &cp1cp2, &odir3, &midpoint3cp4, cp4.getX() - midpoint3.getX(), cp4.getY() - midpoint3.getY() );
    cp4.setZ( midpoint3.getZ() + midpoint3cp4.getZ() );

    cp13.setX(( point2.getX() + cp10.getX() + point3.getX() ) / 3 );
    cp13.setY(( point2.getY() + cp10.getY() + point3.getY() ) / 3 );
    //find the point in the middle of the bezier curve between point2 and point3
    Point3D midpoint1(( cp9.getX() + cp16.getX() ) / 2, ( cp9.getY() + cp16.getY() ) / 2, ( cp9.getZ() + cp16.getZ() ) / 2 );
    Vector3D cp9cp16( cp16.getX() - cp9.getX(), cp16.getY() - cp9.getY(), cp16.getZ() - cp9.getZ() );
    Vector3D odir1( 0, 0, 0 );//direction orthogonal to the line from point2 to point3
    if (( point3.getY() - point2.getY() ) != 0 )//avoid division through zero
    {
      odir1.setX( 1 );
      odir1.setY( -( point3.getX() - point2.getX() ) / ( point3.getY() - point2.getY() ) );
      odir1.setZ(( der2X + der3X ) / 2 + ( der2Y + der3Y ) / 2*odir1.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir1.setY( 1 );
      odir1.setX( -( point3.getY() - point2.getY() ) / ( point3.getX() - point2.getX() ) );
      odir1.setZ(( der2X + der3X ) / 2*odir1.getX() + ( der2Y + der3Y ) / 2 );
    }
    Vector3D midpoint1cp13( 0, 0, 0 );
    MathUtils::derVec( &cp9cp16, &odir1, &midpoint1cp13, cp13.getX() - midpoint1.getX(), cp13.getY() - midpoint1.getY() );
    cp13.setZ( midpoint1.getZ() + midpoint1cp13.getZ() );


    cp11.setX(( point3.getX() + cp10.getX() + point1.getX() ) / 3 );
    cp11.setY(( point3.getY() + cp10.getY() + point1.getY() ) / 3 );
    //find the point in the middle of the bezier curve between point3 and point2
    Point3D midpoint2(( cp14.getX() + cp6.getX() ) / 2, ( cp14.getY() + cp6.getY() ) / 2, ( cp14.getZ() + cp6.getZ() ) / 2 );
    Vector3D cp14cp6( cp6.getX() - cp14.getX(), cp6.getY() - cp14.getY(), cp6.getZ() - cp14.getZ() );
    Vector3D odir2( 0, 0, 0 );//direction orthogonal to the line from point 1 to point3
    if (( point3.getY() - point1.getY() ) != 0 )//avoid division through zero
    {
      odir2.setX( 1 );
      odir2.setY( -( point3.getX() - point1.getX() ) / ( point3.getY() - point1.getY() ) );
      odir2.setZ(( der3X + der1X ) / 2 + ( der3Y + der1Y ) / 2*odir2.getY() );//take the linear interpolated cross-derivative
    }
    else
    {
      odir2.setY( 1 );
      odir2.setX( -( point3.getY() - point1.getY() ) / ( point3.getX() - point1.getX() ) );
      odir2.setZ(( der3X + der1X ) / 2*odir2.getX() + ( der3Y + der1Y ) / 2 );
    }
    Vector3D midpoint2cp11( 0, 0, 0 );
    MathUtils::derVec( &cp14cp6, &odir2, &midpoint2cp11, cp11.getX() - midpoint2.getX(), cp11.getY() - midpoint2.getY() );
    cp11.setZ( midpoint2.getZ() + midpoint2cp11.getZ() );


    cp7.setX( cp10.getX() + ( point1.getX() - cp10.getX() ) / 3 );
    cp7.setY( cp10.getY() + ( point1.getY() - cp10.getY() ) / 3 );
    //cp7 has to be in the same plane as cp4, cp3 and cp11
    Vector3D cp4cp3( cp3.getX() - cp4.getX(), cp3.getY() - cp4.getY(), cp3.getZ() - cp4.getZ() );
    Vector3D cp4cp11( cp11.getX() - cp4.getX(), cp11.getY() - cp4.getY(), cp11.getZ() - cp4.getZ() );
    Vector3D cp4cp7( 0, 0, 0 );
    MathUtils::derVec( &cp4cp3, &cp4cp11, &cp4cp7, cp7.getX() - cp4.getX(), cp7.getY() - cp4.getY() );
    cp7.setZ( cp4.getZ() + cp4cp7.getZ() );

    cp8.setX( cp10.getX() + ( point2.getX() - cp10.getX() ) / 3 );
    cp8.setY( cp10.getY() + ( point2.getY() - cp10.getY() ) / 3 );
    //cp8 has to be in the same plane as cp4, cp5 and cp13
    Vector3D cp4cp5( cp5.getX() - cp4.getX(), cp5.getY() - cp4.getY(), cp5.getZ() - cp4.getZ() );
    Vector3D cp4cp13( cp13.getX() - cp4.getX(), cp13.getY() - cp4.getY(), cp13.getZ() - cp4.getZ() );
    Vector3D cp4cp8( 0, 0, 0 );
    MathUtils::derVec( &cp4cp5, &cp4cp13, &cp4cp8, cp8.getX() - cp4.getX(), cp8.getY() - cp4.getY() );
    cp8.setZ( cp4.getZ() + cp4cp8.getZ() );

    cp12.setX( cp10.getX() + ( point3.getX() - cp10.getX() ) / 3 );
    cp12.setY( cp10.getY() + ( point3.getY() - cp10.getY() ) / 3 );
    //cp12 has to be in the same plane as cp13, cp15 and cp11
    Vector3D cp13cp11( cp11.getX() - cp13.getX(), cp11.getY() - cp13.getY(), cp11.getZ() - cp13.getZ() );
    Vector3D cp13cp15( cp15.getX() - cp13.getX(), cp15.getY() - cp13.getY(), cp15.getZ() - cp13.getZ() );
    Vector3D cp13cp12( 0, 0, 0 );
    MathUtils::derVec( &cp13cp11, &cp13cp15, &cp13cp12, cp12.getX() - cp13.getX(), cp12.getY() - cp13.getY() );
    cp12.setZ( cp13.getZ() + cp13cp12.getZ() );

    //cp10 has to be in the same plane as cp7, cp8 and cp12
    Vector3D cp7cp8( cp8.getX() - cp7.getX(), cp8.getY() - cp7.getY(), cp8.getZ() - cp7.getZ() );
    Vector3D cp7cp12( cp12.getX() - cp7.getX(), cp12.getY() - cp7.getY(), cp12.getZ() - cp7.getZ() );
    Vector3D cp7cp10( 0, 0, 0 );
    MathUtils::derVec( &cp7cp8, &cp7cp12, &cp7cp10, cp10.getX() - cp7.getX(), cp10.getY() - cp7.getY() );
    cp10.setZ( cp7.getZ() + cp7cp10.getZ() );

    lpoint1 = point1;
    lpoint2 = point2;
    lpoint3 = point3;
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
  }
}
#endif







