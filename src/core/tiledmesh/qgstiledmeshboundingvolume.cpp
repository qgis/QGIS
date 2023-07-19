/***************************************************************************
                         qgstiledmeshboundingvolume.cpp
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

#include "qgstiledmeshboundingvolume.h"
#include "qgscircle.h"
#include "qgscoordinatetransform.h"
#include "qgsvector3d.h"
#include "qgsmultipoint.h"
#include "qgsgeos.h"

QgsAbstractTiledMeshNodeBoundingVolume::~QgsAbstractTiledMeshNodeBoundingVolume() = default;

//
// QgsTiledMeshNodeBoundingVolumeRegion
//

QgsTiledMeshNodeBoundingVolumeRegion::QgsTiledMeshNodeBoundingVolumeRegion( const QgsBox3d &region )
  : mRegion( region )
{

}

Qgis::TiledMeshBoundingVolumeType QgsTiledMeshNodeBoundingVolumeRegion::type() const
{
  return Qgis::TiledMeshBoundingVolumeType::Region;
}

QgsBox3d QgsTiledMeshNodeBoundingVolumeRegion::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !transform.isValid() || transform.isShortCircuited() )
  {
    return mRegion;
  }
  else
  {
    // transform each corner of the box, then collect the min/max x/y/z values of the result
    QVector< double > x{ mRegion.xMinimum(), mRegion.xMinimum(), mRegion.xMaximum(), mRegion.xMaximum(), mRegion.xMinimum(), mRegion.xMinimum(), mRegion.xMaximum(), mRegion.xMaximum() };
    QVector< double > y{ mRegion.yMinimum(), mRegion.yMaximum(), mRegion.yMinimum(), mRegion.yMaximum(), mRegion.yMinimum(), mRegion.yMaximum(), mRegion.yMinimum(), mRegion.yMaximum() };
    QVector< double > z{ mRegion.zMinimum(), mRegion.zMinimum(), mRegion.zMinimum(), mRegion.zMinimum(), mRegion.zMaximum(), mRegion.zMaximum(), mRegion.zMaximum(), mRegion.zMaximum() };
    transform.transformInPlace( x, y, z, direction );

    const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
    const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
    const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
    return QgsBox3d( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second );
  }
}

QgsTiledMeshNodeBoundingVolumeRegion *QgsTiledMeshNodeBoundingVolumeRegion::clone() const
{
  return new QgsTiledMeshNodeBoundingVolumeRegion( *this );
}

QgsAbstractGeometry *QgsTiledMeshNodeBoundingVolumeRegion::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();

  // transform using the region center point z
  const double centerZ = 0.5 * ( mRegion.zMaximum() + mRegion.zMinimum() );
  std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString >(
      QVector< double >() << mRegion.xMinimum()
      << mRegion.xMaximum()
      << mRegion.xMaximum()
      << mRegion.xMinimum()
      << mRegion.xMinimum(),
      QVector< double >() << mRegion.yMinimum()
      << mRegion.yMinimum()
      << mRegion.yMaximum()
      << mRegion.yMaximum()
      << mRegion.yMinimum(),
      QVector< double >() << centerZ
      << centerZ
      << centerZ
      << centerZ
      << centerZ );

  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    ext->transform( transform, direction, true );
  }
  ext->dropZValue();
  polygon->setExteriorRing( ext.release() );

  return polygon.release();
}


//
// QgsTiledMeshNodeBoundingVolumeBox
//

QgsTiledMeshNodeBoundingVolumeBox::QgsTiledMeshNodeBoundingVolumeBox( const QgsOrientedBox3D &box )
  : mBox( box )
{
}

Qgis::TiledMeshBoundingVolumeType QgsTiledMeshNodeBoundingVolumeBox::type() const
{
  return Qgis::TiledMeshBoundingVolumeType::OrientedBox;
}

QgsBox3d QgsTiledMeshNodeBoundingVolumeBox::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
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
      x.append( corners[i].x() );
      y.append( corners[i].y() );
      z.append( corners[i].z() );
    }
    transform.transformInPlace( x, y, z, direction );

    const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
    const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
    const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
    return QgsBox3d( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second );
  }
  else
  {
    return mBox.extent();
  }
}

QgsTiledMeshNodeBoundingVolumeBox *QgsTiledMeshNodeBoundingVolumeBox::clone() const
{
  return new QgsTiledMeshNodeBoundingVolumeBox( *this );
}

QgsAbstractGeometry *QgsTiledMeshNodeBoundingVolumeBox::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
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
    x.append( corners[i].x() );
    y.append( corners[i].y() );
    z.append( corners[i].z() );
  }

  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    transform.transformInPlace( x, y, z, direction );
  }

  std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >( x, y );
  QgsGeos geosMp( mp.get() );
  return geosMp.convexHull();
}

//
// QgsTiledMeshNodeBoundingVolumeSphere
//

QgsTiledMeshNodeBoundingVolumeSphere::QgsTiledMeshNodeBoundingVolumeSphere( const QgsSphere &sphere )
  : mSphere( sphere )
{

}

Qgis::TiledMeshBoundingVolumeType QgsTiledMeshNodeBoundingVolumeSphere::type() const
{
  return Qgis::TiledMeshBoundingVolumeType::Sphere;
}

QgsBox3d QgsTiledMeshNodeBoundingVolumeSphere::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    std::unique_ptr< QgsAbstractGeometry > centerSlice( as2DGeometry( transform, direction ) );

    // a line through the z range of sphere, passing through center
    QVector< double > x { mSphere.centerX(), mSphere.centerX() };
    QVector< double > y { mSphere.centerY(), mSphere.centerY() };
    QVector< double > z { mSphere.centerZ() - mSphere.radius(), mSphere.centerZ() + mSphere.radius() };
    transform.transformInPlace( x, y, z, direction );

    const QgsRectangle bounds2d( centerSlice->boundingBox() );
    return QgsBox3d( bounds2d.xMinimum(), bounds2d.yMinimum(), std::min( z[0], z[1] ), bounds2d.xMaximum(), bounds2d.yMaximum(), std::max( z[0], z[1] ) );
  }
  else
  {
    return mSphere.boundingBox();
  }
}

QgsTiledMeshNodeBoundingVolumeSphere *QgsTiledMeshNodeBoundingVolumeSphere::clone() const
{
  return new QgsTiledMeshNodeBoundingVolumeSphere( *this );
}

QgsAbstractGeometry *QgsTiledMeshNodeBoundingVolumeSphere::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( transform.isValid() && !transform.isShortCircuited() )
  {
    QgsVector3D normal = mSphere.centerVector();
    normal.normalize();

    QgsVector3D axis1 = QgsVector3D::crossProduct( normal, QgsVector3D( 1, 0, 0 ) );
    if ( axis1.length() == 0 )
    {
      axis1 = QgsVector3D::crossProduct( normal, QgsVector3D( 0, 1, 0 ) );
    }
    axis1.normalize();
    const QgsVector3D axis2 = QgsVector3D::crossProduct( normal, axis1 );
    QVector< double > circleXInPlane;
    QVector< double > circleYInPlane;
    QVector< double > circleZInPlane;

    for ( int i = 0; i < 48; ++i )
    {
      const double alpha = 2 * i / 48.0 * M_PI;
      circleXInPlane.append( mSphere.centerX() + mSphere.radius() * ( axis1.x() * std::cos( alpha ) + axis2.x()* std::sin( alpha ) ) );
      circleYInPlane.append( mSphere.centerY() + mSphere.radius() * ( axis1.y() * std::cos( alpha ) + axis2.y()* std::sin( alpha ) ) );
      circleZInPlane.append( mSphere.centerZ() + mSphere.radius() * ( axis1.z() * std::cos( alpha ) + axis2.z()* std::sin( alpha ) ) );
    }
    transform.transformInPlace( circleXInPlane, circleYInPlane, circleZInPlane, direction );

    std::unique_ptr< QgsLineString > exterior = std::make_unique< QgsLineString>( circleXInPlane, circleYInPlane );
    exterior->close();
    std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();
    polygon->setExteriorRing( exterior.release() );
    return polygon.release();
  }
  else
  {
    std::unique_ptr< QgsCurvePolygon > polygon = std::make_unique< QgsCurvePolygon >();
    std::unique_ptr< QgsCircularString > exterior( mSphere.toCircle().toCircularString() );
    polygon->setExteriorRing( exterior.release() );
    return polygon.release();
  }
}
