/***************************************************************************
                         qgstiledmeshboundingvolume.h
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTILEDMESHBOUNDINGVOLUME_H
#define QGSTILEDMESHBOUNDINGVOLUME_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsbox3d.h"
#include "qgsmatrix4x4.h"
#include "qgssphere.h"
#include "qgsorientedbox3d.h"
#include "qgscoordinatetransform.h"

class QgsMatrix4x4;

/**
 * \ingroup core
 * \brief Abstract base class for bounding volumes for tiled mesh nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAbstractTiledMeshNodeBoundingVolume
{
  public:

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case Qgis::TiledMeshBoundingVolumeType::Region:
        sipType = sipType_QgsTiledMeshNodeBoundingVolumeRegion;
        break;
      case Qgis::TiledMeshBoundingVolumeType::OrientedBox:
        sipType = sipType_QgsTiledMeshNodeBoundingVolumeBox;
        break;
      case Qgis::TiledMeshBoundingVolumeType::Sphere:
        sipType = sipType_QgsTiledMeshNodeBoundingVolumeSphere;
        break;
      default:
        sipType = 0;
        break;
    };
    SIP_END
#endif

    virtual ~QgsAbstractTiledMeshNodeBoundingVolume();

    /**
     * Returns the type of the volume;
     */
    virtual Qgis::TiledMeshBoundingVolumeType type() const = 0;

    /**
     * Returns the axis aligned bounding box of the volume.
     *
     * The optional \a transform and \a direction arguments should be used whenever the volume needs
     * to be transformed into a specific destination CRS, in order to correctly handle 3D coordinate transforms.
     */
    virtual QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const = 0;

    /**
     * Returns a clone of the volume.
     */
    virtual QgsAbstractTiledMeshNodeBoundingVolume *clone() const = 0 SIP_FACTORY;

    /**
     * Returns a new geometry representing the 2-dimensional X/Y center slice of the volume.
     *
     * Caller takes ownership of the returned geometry.
     *
     * The optional \a transform and \a direction arguments should be used whenever the volume needs
     * to be transformed into a specific destination CRS, in order to correctly handle 3D coordinate transforms.
     */
    virtual QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const = 0 SIP_FACTORY;

    /**
     * Sets the bounding volume's \a transform.
     *
     * \see transform()
     */
    void setTransform( const QgsMatrix4x4 &transform ) { mTransform = transform; }

    /**
     * Returns the bounding volume's transform.
     *
     * This represents the transformation which must be applied to all geometries from the tile
     * in order to transform them to the dataset's coordinate reference system.
     *
     * \see transform()
     */
    const QgsMatrix4x4 &transform() const { return mTransform; }

  protected:

    QgsMatrix4x4 mTransform;

};

/**
 * \ingroup core
 * \brief A region bounding volume for tiled mesh nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshNodeBoundingVolumeRegion : public QgsAbstractTiledMeshNodeBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledMeshNodeBoundingVolumeRegion, with the specified \a region.
     */
    QgsTiledMeshNodeBoundingVolumeRegion( const QgsBox3D &region );

    Qgis::TiledMeshBoundingVolumeType type() const FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledMeshNodeBoundingVolumeRegion *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;

    /**
     * Returns the volume's region.
     */
    QgsBox3D region() const { return mRegion; }

  private:
    QgsBox3D mRegion;
};

/**
 * \ingroup core
 * \brief A oriented box bounding volume for tiled mesh nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshNodeBoundingVolumeBox : public QgsAbstractTiledMeshNodeBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledMeshNodeBoundingVolumeBox, with the specified oriented \a box.
     */
    QgsTiledMeshNodeBoundingVolumeBox( const QgsOrientedBox3D &box );

    Qgis::TiledMeshBoundingVolumeType type() const FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledMeshNodeBoundingVolumeBox *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;

    /**
     * Returns the volume's oriented box.
     */
    QgsOrientedBox3D box() const { return mBox; }

  private:

    QgsOrientedBox3D mBox;

};

/**
 * \ingroup core
 * \brief A spherical bounding volume for tiled mesh nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshNodeBoundingVolumeSphere: public QgsAbstractTiledMeshNodeBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledMeshNodeBoundingVolumeBox, with the specified \a sphere.
     */
    QgsTiledMeshNodeBoundingVolumeSphere( const QgsSphere &sphere );

    Qgis::TiledMeshBoundingVolumeType type() const FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledMeshNodeBoundingVolumeSphere *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;

    /**
     * Returns the volume's sphere.
     */
    QgsSphere sphere() const { return mSphere; }

  private:

    QgsSphere mSphere;

};


#endif // QGSTILEDMESHBOUNDINGVOLUME_H
