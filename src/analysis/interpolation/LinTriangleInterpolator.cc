/***************************************************************************
                          LinTriangleInterpolator.cc  -  description
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

#include "LinTriangleInterpolator.h"
#include "qgslogger.h"

bool LinTriangleInterpolator::calcFirstDerX( double x, double y, Vector3D* vec )
{

  if ( vec && mTIN )
  {
    Point3D pt1( 0, 0, 0 );
    Point3D pt2( 0, 0, 0 );
    Point3D pt3( 0, 0, 0 );

    if ( !mTIN->getTriangle( x, y, &pt1, &pt2, &pt3 ) )
    {
      return false;//point outside the convex hull or numerical problems
    }

    vec->setX( 1.0 );
    vec->setY( 0.0 );
    vec->setZ(( pt1.getZ()*( pt2.getY() - pt3.getY() ) + pt2.getZ()*( pt3.getY() - pt1.getY() ) + pt3.getZ()*( pt1.getY() - pt2.getY() ) ) / (( pt1.getX() - pt2.getX() )*( pt2.getY() - pt3.getY() ) - ( pt2.getX() - pt3.getX() )*( pt1.getY() - pt2.getY() ) ) );
    return true;
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool LinTriangleInterpolator::calcFirstDerY( double x, double y, Vector3D* vec )
{
  if ( vec && mTIN )
  {
    Point3D pt1( 0, 0, 0 );
    Point3D pt2( 0, 0, 0 );
    Point3D pt3( 0, 0, 0 );

    if ( !mTIN->getTriangle( x, y, &pt1, &pt2, &pt3 ) )
    {
      return false;
    }

    vec->setX( 0 );
    vec->setY( 1.0 );
    vec->setZ(( pt1.getZ()*( pt2.getX() - pt3.getX() ) + pt2.getZ()*( pt3.getX() - pt1.getX() ) + pt3.getZ()*( pt1.getX() - pt2.getX() ) ) / (( pt1.getY() - pt2.getY() )*( pt2.getX() - pt3.getX() ) - ( pt2.getY() - pt3.getY() )*( pt1.getX() - pt2.getX() ) ) );
    return true;
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }
}

bool LinTriangleInterpolator::calcNormVec( double x, double y, Vector3D* vec )
{
//calculate vector product of the two derivative vectors in x- and y-direction and set the length to 1
  if ( vec && mTIN )
  {
    Vector3D vec1;
    Vector3D vec2;
    if ( !calcFirstDerX( x, y, &vec1 ) )
      {return false;}
    if ( !calcFirstDerY( x, y, &vec2 ) )
      {return false;}
    Vector3D vec3( vec1.getY()*vec2.getZ() - vec1.getZ()*vec2.getY(), vec1.getZ()*vec2.getX() - vec1.getX()*vec2.getZ(), vec1.getX()*vec2.getY() - vec1.getY()*vec2.getX() );//calculate vector product
    double absvec3 = sqrt( vec3.getX() * vec3.getX() + vec3.getY() * vec3.getY() + vec3.getZ() * vec3.getZ() );//length of vec3
    vec->setX( vec3.getX() / absvec3 );//standardize vec3 and assign it to vec
    vec->setY( vec3.getY() / absvec3 );
    vec->setZ( vec3.getZ() / absvec3 );
    return true;
  }

  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }

}


bool LinTriangleInterpolator::calcPoint( double x, double y, Point3D* point )
{
  if ( point && mTIN )
  {
    Point3D pt1( 0, 0, 0 );
    Point3D pt2( 0, 0, 0 );
    Point3D pt3( 0, 0, 0 );

    if ( !mTIN->getTriangle( x, y, &pt1, &pt2, &pt3 ) )
    {
      return false;//point is outside the convex hull or numerical problems
    }

    double a = ( pt1.getZ() * ( pt2.getY() - pt3.getY() ) + pt2.getZ() * ( pt3.getY() - pt1.getY() ) + pt3.getZ() * ( pt1.getY() - pt2.getY() ) ) / (( pt1.getX() - pt2.getX() ) * ( pt2.getY() - pt3.getY() ) - ( pt2.getX() - pt3.getX() ) * ( pt1.getY() - pt2.getY() ) );
    double b = ( pt1.getZ() * ( pt2.getX() - pt3.getX() ) + pt2.getZ() * ( pt3.getX() - pt1.getX() ) + pt3.getZ() * ( pt1.getX() - pt2.getX() ) ) / (( pt1.getY() - pt2.getY() ) * ( pt2.getX() - pt3.getX() ) - ( pt2.getY() - pt3.getY() ) * ( pt1.getX() - pt2.getX() ) );
    double c = pt1.getZ() - a * pt1.getX() - b * pt1.getY();

    point->setX( x );
    point->setY( y );
    point->setZ( a*x + b*y + c );
    return true;
  }
  else
  {
    QgsDebugMsg( "warning, null pointer" );
    return false;
  }

}










