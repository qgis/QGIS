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
#include "qgsorientedbox3d.h"
#include "qgscoordinatetransform.h"

class QgsMatrix4x4;

/**
 * \ingroup core
 * \brief Represents a bounding volume for a tiled scene.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneBoundingVolume
{
  public:

    /**
     * Constructor for QgsTiledSceneBoundingVolume, with the specified oriented \a box.
     */
    QgsTiledSceneBoundingVolume( const QgsOrientedBox3D &box = QgsOrientedBox3D() );

    /**
     * Returns the axis aligned bounding box of the volume.
     *
     * The optional \a transform and \a direction arguments should be used whenever the volume needs
     * to be transformed into a specific destination CRS, in order to correctly handle 3D coordinate transforms.
     */
    QgsBox3D bounds( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const;

    /**
     * Returns a new geometry representing the 2-dimensional X/Y center slice of the volume.
     *
     * Caller takes ownership of the returned geometry.
     *
     * The optional \a transform and \a direction arguments should be used whenever the volume needs
     * to be transformed into a specific destination CRS, in order to correctly handle 3D coordinate transforms.
     */
    QgsAbstractGeometry *as2DGeometry( const QgsCoordinateTransform &transform = QgsCoordinateTransform(), Qgis::TransformDirection direction = Qgis::TransformDirection::Forward ) const SIP_FACTORY;

    /**
     * Applies a \a transform to the bounding volume.
     */
    void transform( const QgsMatrix4x4 &transform );

    /**
     * Returns TRUE if this bounds intersects the specified \a box.
     */
    bool intersects( const QgsOrientedBox3D &box ) const;

    /**
     * Returns the volume's oriented box.
     */
    QgsOrientedBox3D box() const { return mBox; }

  private:

    QgsOrientedBox3D mBox;

};

#endif // QGSTILEDSCENEBOUNDINGVOLUME_H
