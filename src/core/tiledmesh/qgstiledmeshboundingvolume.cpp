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
#include "qgsmatrix4x4.h"
#include "qgsvector3d.h"
#include "qgsmultipoint.h"
#include "qgsgeos.h"

QgsAbstractTiledMeshNodeBoundingVolume::~QgsAbstractTiledMeshNodeBoundingVolume() = default;

//
// QgsTiledMeshNodeBoundingVolumeRegion
//

QgsTiledMeshNodeBoundingVolumeRegion::QgsTiledMeshNodeBoundingVolumeRegion( const QgsBox3D &region )
  : mRegion( region )
{

}

Qgis::TiledMeshBoundingVolumeType QgsTiledMeshNodeBoundingVolumeRegion::type() const
{
  return Qgis::TiledMeshBoundingVolumeType::Region;
}

QgsBox3D QgsTiledMeshNodeBoundingVolumeRegion::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !mTransform.isIdentity() || ( transform.isValid() && !transform.isShortCircuited() ) )
  {
    // transform each corner of the box, then collect the min/max x/y/z values of the result
    QVector<QgsVector3D > corners = mRegion.corners();
    QVector< double > x;
    x.reserve( 8 );
    QVector< double > y;
    y.reserve( 8 );
    QVector< double > z;
    z.reserve( 8 );
    for ( int i = 0; i < 8; ++i )
    {
      QgsVector3D corner = corners[i];
      if ( !mTransform.isIdentity() )
        corner = mTransform.map( corner );

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
    return mRegion;
  }
}

QgsTiledMeshNodeBoundingVolumeRegion *QgsTiledMeshNodeBoundingVolumeRegion::clone() const
{
  return new QgsTiledMeshNodeBoundingVolumeRegion( *this );
}

QgsAbstractGeometry *QgsTiledMeshNodeBoundingVolumeRegion::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !mTransform.isIdentity() || ( transform.isValid() && !transform.isShortCircuited() ) )
  {
    const QVector< QgsVector3D > corners = mRegion.corners();
    QVector< double > x;
    x.reserve( 8 );
    QVector< double > y;
    y.reserve( 8 );
    QVector< double > z;
    z.reserve( 8 );
    for ( int i = 0; i < 8; ++i )
    {
      if ( !mTransform.isIdentity() )
      {
        for ( int i = 0; i < 8; ++i )
        {
          const QgsVector3D transformed = mTransform.map( QgsVector3D( corners[i] ) );
          x.append( transformed.x() );
          y.append( transformed.y() );
          z.append( transformed.z() );
        }
      }
      else
      {
        x.append( corners[i].x() );
        y.append( corners[i].y() );
        z.append( corners[i].z() );
      }
    }

    if ( transform.isValid() && !transform.isShortCircuited() )
    {
      transform.transformInPlace( x, y, z, direction );
    }

    std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >( x, y );
    QgsGeos geosMp( mp.get() );
    return geosMp.convexHull();
  }
  else
  {
    std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();
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
        << mRegion.yMinimum() );

    polygon->setExteriorRing( ext.release() );
    return polygon.release();
  }
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

QgsBox3D QgsTiledMeshNodeBoundingVolumeBox::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !mTransform.isIdentity() || ( transform.isValid() && !transform.isShortCircuited() ) )
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
      QgsVector3D corner = corners[i];
      if ( !mTransform.isIdentity() )
        corner = mTransform.map( corner );

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
    if ( !mTransform.isIdentity() )
    {
      for ( int i = 0; i < 8; ++i )
      {
        const QgsVector3D transformed = mTransform.map( QgsVector3D( corners[i] ) );
        x.append( transformed.x() );
        y.append( transformed.y() );
        z.append( transformed.z() );
      }
    }
    else
    {
      x.append( corners[i].x() );
      y.append( corners[i].y() );
      z.append( corners[i].z() );
    }
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

QgsBox3D QgsTiledMeshNodeBoundingVolumeSphere::bounds( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !mTransform.isIdentity() || ( transform.isValid() && !transform.isShortCircuited() ) )
  {
    // TODO -- what happens with the sphere radius when a transform is present? is it also transformed?
    const QVector< QgsVector3D > corners = mSphere.boundingBox().corners();
    QVector< double > x;
    x.reserve( 8 );
    QVector< double > y;
    y.reserve( 8 );
    QVector< double > z;
    z.reserve( 8 );
    for ( int i = 0; i < 8; ++i )
    {
      QgsVector3D corner = corners[i];
      if ( !mTransform.isIdentity() )
        corner = mTransform.map( corner );

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
    return mSphere.boundingBox();
  }
}

QgsTiledMeshNodeBoundingVolumeSphere *QgsTiledMeshNodeBoundingVolumeSphere::clone() const
{
  return new QgsTiledMeshNodeBoundingVolumeSphere( *this );
}

QgsAbstractGeometry *QgsTiledMeshNodeBoundingVolumeSphere::as2DGeometry( const QgsCoordinateTransform &transform, Qgis::TransformDirection direction ) const
{
  if ( !mTransform.isIdentity() || ( transform.isValid() && !transform.isShortCircuited() ) )
  {
    // TODO -- what happens with the sphere radius when a transform is present? is it also transformed?
    const QgsVector3D sphereCenter = mTransform.map( mSphere.centerVector() );
    QgsVector3D normal = sphereCenter;
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
      circleXInPlane.append( sphereCenter.x() + mSphere.radius() * ( axis1.x() * std::cos( alpha ) + axis2.x()* std::sin( alpha ) ) );
      circleYInPlane.append( sphereCenter.y() + mSphere.radius() * ( axis1.y() * std::cos( alpha ) + axis2.y()* std::sin( alpha ) ) );
      circleZInPlane.append( sphereCenter.z() + mSphere.radius() * ( axis1.z() * std::cos( alpha ) + axis2.z()* std::sin( alpha ) ) );
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
