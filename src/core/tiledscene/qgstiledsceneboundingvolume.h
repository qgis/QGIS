/***************************************************************************
                         qgstiledsceneboundingvolume.h
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

#ifndef QGSTILEDSCENEBOUNDINGVOLUME_H
#define QGSTILEDSCENEBOUNDINGVOLUME_H

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
 * \brief Abstract base class for bounding volumes for tiled scene nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAbstractTiledSceneBoundingVolume
{
  public:

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->type() )
    {
      case Qgis::TiledSceneBoundingVolumeType::Region:
        sipType = sipType_QgsTiledSceneBoundingVolumeRegion;
        break;
      case Qgis::TiledSceneBoundingVolumeType::OrientedBox:
        sipType = sipType_QgsTiledSceneBoundingVolumeBox;
        break;
      case Qgis::TiledSceneBoundingVolumeType::Sphere:
        sipType = sipType_QgsTiledSceneBoundingVolumeSphere;
        break;
      default:
        sipType = 0;
        break;
    };
    SIP_END
#endif

    virtual ~QgsAbstractTiledSceneBoundingVolume();

    /**
     * Returns the type of the volume;
     */
    virtual Qgis::TiledSceneBoundingVolumeType type() const = 0;

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
    virtual QgsAbstractTiledSceneBoundingVolume *clone() const = 0 SIP_FACTORY;

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
     * Applies a \a transform to the bounding volume.
     *
     * The actual result of transforming a bounding volume depends on subclass specific logic. For instance:
     *
     * - transforming a QgsTiledSceneBoundingVolumeRegion results in no change to the region
     * - transforming a QgsTiledSceneBoundingVolumeSphere causes the radius to be multiplied by the maximum length of the transform scales
     */
    virtual void transform( const QgsMatrix4x4 &transform ) = 0;

    /**
     * Returns TRUE if this bounds intersects the specified \a box.
     */
    virtual bool intersects( const QgsOrientedBox3D &box ) const = 0;

};

/**
 * \ingroup core
 * \brief A region bounding volume for tiled scene nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneBoundingVolumeRegion : public QgsAbstractTiledSceneBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledSceneBoundingVolumeRegion, with the specified \a region.
     */
    QgsTiledSceneBoundingVolumeRegion( const QgsBox3D &region );

    Qgis::TiledSceneBoundingVolumeType type() const FINAL;
    void transform( const QgsMatrix4x4 &transform ) FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledSceneBoundingVolumeRegion *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;
    bool intersects( const QgsOrientedBox3D &box ) const FINAL;

    /**
     * Returns the volume's region.
     */
    QgsBox3D region() const { return mRegion; }

  private:
    QgsBox3D mRegion;
};

/**
 * \ingroup core
 * \brief A oriented box bounding volume for tiled scene nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneBoundingVolumeBox : public QgsAbstractTiledSceneBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledSceneBoundingVolumeBox, with the specified oriented \a box.
     */
    QgsTiledSceneBoundingVolumeBox( const QgsOrientedBox3D &box );

    Qgis::TiledSceneBoundingVolumeType type() const FINAL;
    void transform( const QgsMatrix4x4 &transform ) FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledSceneBoundingVolumeBox *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;
    bool intersects( const QgsOrientedBox3D &box ) const FINAL;

    /**
     * Returns the volume's oriented box.
     */
    QgsOrientedBox3D box() const { return mBox; }

  private:

    QgsOrientedBox3D mBox;

};

/**
 * \ingroup core
 * \brief A spherical bounding volume for tiled scene nodes.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneBoundingVolumeSphere: public QgsAbstractTiledSceneBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledSceneBoundingVolumeSphere, with the specified \a sphere.
     */
    QgsTiledSceneBoundingVolumeSphere( const QgsSphere &sphere );

    Qgis::TiledSceneBoundingVolumeType type() const FINAL;
    void transform( const QgsMatrix4x4 &transform ) FINAL;
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException );
    QgsTiledSceneBoundingVolumeSphere *clone() const FINAL SIP_FACTORY;
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const FINAL SIP_THROW( QgsCsException ) SIP_FACTORY;
    bool intersects( const QgsOrientedBox3D &box ) const FINAL;

    /**
     * Returns the volume's sphere.
     */
    QgsSphere sphere() const { return mSphere; }

  private:

    QgsSphere mSphere;

};


#endif // QGSTILEDSCENEBOUNDINGVOLUME_H
