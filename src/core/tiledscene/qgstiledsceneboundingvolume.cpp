/***************************************************************************
                         qgstiledsceneboundingvolume.cpp
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneboundingvolume.h"
#include "qgscoordinatetransform.h"
#include "qgsmatrix4x4.h"
#include "qgsvector3d.h"
#include "qgsmultipoint.h"
#include "qgsgeos.h"
#include "qgspolygon.h"

QgsTiledSceneBoundingVolume::QgsTiledSceneBoundingVolume( const QgsOrientedBox3D &box )
  : mBox( box )
{
}

void QgsTiledSceneBoundingVolume::transform( const QgsMatrix4x4 &transform )
{
  mBox = mBox.transformed( transform );
}

QgsBox3D QgsTiledSceneBoundingVolume::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    const QVector< QgsVector3D > corners = mBox.corners();
    QVector< double > x;
    x.reserve( 8 );
    QVector< double > y;
    y.reserve( 8 );
    QVector< double > z;
    z.reserve( 8 );
    for ( int i = 0; i < 8; ++i )
    {
      const QgsVector3D &corner = corners[i];
      x.append( corner.x() );
      y.append( corner.y() );
      z.append( corner.z() );
    }
    transform.transformInPlace( x, y, z, direction );

    const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
    const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
    const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
    return QgsBox3D( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second );
  }
  else
  {
    return mBox.extent();
  }
}

QgsAbstractGeometry *QgsTiledSceneBoundingVolume::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();

  const QVector< QgsVector3D > corners = mBox.corners();
  QVector< double > x;
  x.reserve( 8 );
  QVector< double > y;
  y.reserve( 8 );
  QVector< double > z;
  z.reserve( 8 );
  for ( int i = 0; i < 8; ++i )
  {
    const QgsVector3D &corner = corners[i];
    x.append( corner.x() );
    y.append( corner.y() );
    z.append( corner.z() );
  }

  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    transform.transformInPlace( x, y, z, direction );
  }

  std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >( x, y );
  QgsGeos geosMp( mp.get() );
  return geosMp.convexHull();
}

bool QgsTiledSceneBoundingVolume::intersects( const QgsOrientedBox3D &box ) const
{
  return mBox.intersects( box );
}

