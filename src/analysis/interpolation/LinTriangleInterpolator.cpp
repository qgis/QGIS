/***************************************************************************
                          LinTriangleInterpolator.cpp
                          ---------------------------
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

bool LinTriangleInterpolator::calcFirstDerX( double x, double y, Vector3D *vec )
{

  if ( vec && mTIN )
  {
    QgsPoint pt1( 0, 0, 0 );
    QgsPoint pt2( 0, 0, 0 );
    QgsPoint pt3( 0, 0, 0 );

    if ( !mTIN->triangleVertices( x, y, pt1, pt2, pt3 ) )
    {
      return false;//point outside the convex hull or numerical problems
    }

    vec->setX( 1.0 );
    vec->setY( 0.0 );
    vec->setZ( ( pt1.z() * ( pt2.y() - pt3.y() ) + pt2.z() * ( pt3.y() - pt1.y() ) + pt3.z() * ( pt1.y() - pt2.y() ) ) / ( ( pt1.x() - pt2.x() ) * ( pt2.y() - pt3.y() ) - ( pt2.x() - pt3.x() ) * ( pt1.y() - pt2.y() ) ) );
    return true;
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool LinTriangleInterpolator::calcFirstDerY( double x, double y, Vector3D *vec )
{
  if ( vec && mTIN )
  {
    QgsPoint pt1( 0, 0, 0 );
    QgsPoint pt2( 0, 0, 0 );
    QgsPoint pt3( 0, 0, 0 );

    if ( !mTIN->triangleVertices( x, y, pt1, pt2, pt3 ) )
    {
      return false;
    }

    vec->setX( 0 );
    vec->setY( 1.0 );
    vec->setZ( ( pt1.z() * ( pt2.x() - pt3.x() ) + pt2.z() * ( pt3.x() - pt1.x() ) + pt3.z() * ( pt1.x() - pt2.x() ) ) / ( ( pt1.y() - pt2.y() ) * ( pt2.x() - pt3.x() ) - ( pt2.y() - pt3.y() ) * ( pt1.x() - pt2.x() ) ) );
    return true;
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }
}

bool LinTriangleInterpolator::calcNormVec( double x, double y, QgsPoint &vec )
{
//calculate vector product of the two derivative vectors in x- and y-direction and set the length to 1
  if ( mTIN )
  {
    Vector3D vec1;
    Vector3D vec2;
    if ( !calcFirstDerX( x, y, &vec1 ) )
    {return false;}
    if ( !calcFirstDerY( x, y, &vec2 ) )
    {return false;}
    const Vector3D vec3( vec1.getY()*vec2.getZ() - vec1.getZ()*vec2.getY(), vec1.getZ()*vec2.getX() - vec1.getX()*vec2.getZ(), vec1.getX()*vec2.getY() - vec1.getY()*vec2.getX() );//calculate vector product
    const double absvec3 = std::sqrt( vec3.getX() * vec3.getX() + vec3.getY() * vec3.getY() + vec3.getZ() * vec3.getZ() );//length of vec3
    vec.setX( vec3.getX() / absvec3 );//standardize vec3 and assign it to vec
    vec.setY( vec3.getY() / absvec3 );
    vec.setZ( vec3.getZ() / absvec3 );
    return true;
  }

  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }

}

bool LinTriangleInterpolator::calcPoint( double x, double y, QgsPoint &point )
{
  if ( mTIN )
  {
    QgsPoint pt1( 0, 0, 0 );
    QgsPoint pt2( 0, 0, 0 );
    QgsPoint pt3( 0, 0, 0 );

    if ( !mTIN->triangleVertices( x, y, pt1, pt2, pt3 ) )
    {
      return false;//point is outside the convex hull or numerical problems
    }

    const double a = ( pt1.z() * ( pt2.y() - pt3.y() ) + pt2.z() * ( pt3.y() - pt1.y() ) + pt3.z() * ( pt1.y() - pt2.y() ) ) / ( ( pt1.x() - pt2.x() ) * ( pt2.y() - pt3.y() ) - ( pt2.x() - pt3.x() ) * ( pt1.y() - pt2.y() ) );
    const double b = ( pt1.z() * ( pt2.x() - pt3.x() ) + pt2.z() * ( pt3.x() - pt1.x() ) + pt3.z() * ( pt1.x() - pt2.x() ) ) / ( ( pt1.y() - pt2.y() ) * ( pt2.x() - pt3.x() ) - ( pt2.y() - pt3.y() ) * ( pt1.x() - pt2.x() ) );
    const double c = pt1.z() - a * pt1.x() - b * pt1.y();

    point.setX( x );
    point.setY( y );
    point.setZ( a * x + b * y + c );
    return true;
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "warning, null pointer" ) );
    return false;
  }

}










