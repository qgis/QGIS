/***************************************************************************
  qgsaabb.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaabb.h"

QgsAABB::QgsAABB( float xMin, float yMin, float zMin, float xMax, float yMax, float zMax )
  : xMin( xMin )
  , yMin( yMin )
  , zMin( zMin )
  , xMax( xMax )
  , yMax( yMax )
  , zMax( zMax )
{
  // normalize coords
  if ( this->xMax < this->xMin )
    std::swap( this->xMin, this->xMax );
  if ( this->yMax < this->yMin )
    std::swap( this->yMin, this->yMax );
  if ( this->zMax < this->zMin )
    std::swap( this->zMin, this->zMax );
}

bool QgsAABB::intersects( const QgsAABB &other ) const
{
  return xMin < other.xMax && other.xMin < xMax &&
         yMin < other.yMax && other.yMin < yMax &&
         zMin < other.zMax && other.zMin < zMax;
}

bool QgsAABB::intersects( float x, float y, float z ) const
{
  return xMin <= x && xMax >= x &&
         yMin <= y && yMax >= y &&
         zMin <= z && zMax >= z;
}


float QgsAABB::distanceFromPoint( float x, float y, float z ) const
{
  const float dx = std::max( xMin - x, std::max( 0.f, x - xMax ) );
  const float dy = std::max( yMin - y, std::max( 0.f, y - yMax ) );
  const float dz = std::max( zMin - z, std::max( 0.f, z - zMax ) );
  return sqrt( dx * dx + dy * dy + dz * dz );
}

float QgsAABB::distanceFromPoint( QVector3D v ) const
{
  return distanceFromPoint( v.x(), v.y(), v.z() );
}

QList<QVector3D> QgsAABB::verticesForLines() const
{
  QList<QVector3D> vertices;
  for ( int i = 0; i < 2; ++i )
  {
    const float x = i ? xMax : xMin;
    for ( int j = 0; j < 2; ++j )
    {
      const float y = j ? yMax : yMin;
      for ( int k = 0; k < 2; ++k )
      {
        const float z = k ? zMax : zMin;
        if ( i == 0 )
        {
          vertices.append( QVector3D( xMin, y, z ) );
          vertices.append( QVector3D( xMax, y, z ) );
        }
        if ( j == 0 )
        {
          vertices.append( QVector3D( x, yMin, z ) );
          vertices.append( QVector3D( x, yMax, z ) );
        }
        if ( k == 0 )
        {
          vertices.append( QVector3D( x, y, zMin ) );
          vertices.append( QVector3D( x, y, zMax ) );
        }
      }
    }
  }
  return vertices;
}

QString QgsAABB::toString() const
{
  return QStringLiteral( "X %1 - %2  Y %3 - %4  Z %5 - %6" ).arg( xMin ).arg( xMax ).arg( yMin ).arg( yMax ).arg( zMin ).arg( zMax );
}
