/***************************************************************************
  qgsgltfutils.h
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLTF3DUTILS_H
#define QGSGLTF3DUTILS_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#define SIP_NO_FILE

#include "qgis_3d.h"

#include "qgsmatrix4x4.h"

#define TINYGLTF_NO_STB_IMAGE       // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE // we don't need writing of images
#include "tiny_gltf.h"

class QgsCoordinateTransform;

namespace Qt3DCore
{
  class QEntity;
}

/**
 * \ingroup 3d
 *
 * Utility functions for dealing with GLTF models in 3D map views.
 *
 * \since QGIS 3.34
 */
class _3D_EXPORT QgsGltf3DUtils
{
  public:
    //! Helper struct to keep track of transforms to be applied to positions
    struct EntityTransform
    {
        //! chunk's origin in coordinates of the target CRS
        QgsVector3D chunkOriginTargetCrs;
        //! Tile's matrix to transform GLTF model coordinates to ECEF (normally EPSG:4978)
        QgsMatrix4x4 tileTransform;
        //! Transform from ECEF (normally EPSG:4978) to the target CRS
        const QgsCoordinateTransform *ecefToTargetCrs = nullptr;

        //! Axis to treat as up axis in the GLTF model
        Qgis::Axis gltfUpAxis = Qgis::Axis::Y;

        double zValueScale = 1;
        double zValueOffset = 0;
    };

    /**
     * Parses a GLTF model from \a data and returns a valid 3D entity or nullptr on error.
     * If \a errors is not a null pointer, error messages may be returned in the given object
     * (there may be errors also when a valid 3D entity is returned, if some bits of the model
     * were not fully/correctly parsed).
     *
     * The \a baseUri should be URI of the source model, so if GLTF model references some
     * external data (e.g. textures), relative URIs can be resolved correctly.
     *
     * In the process, it applies transforms to the position coordinates: in addition to
     * what QgsGltfUtils::accessorToMapCoordinates() does, it does an extra step of converting
     * map coordinates to 3D scene coordinates: P_SCENE = flip_ZY(P_MAP - sceneOriginTargetCrs)
     */
    static Qt3DCore::QEntity *gltfToEntity( const QByteArray &data, const EntityTransform &transform, const QString &baseUri, QStringList *errors = nullptr );

    /**
     * Converts a GLTF model into a Qt 3D entity.
     * \see gltfToEntity()
     */
    static Qt3DCore::QEntity *parsedGltfToEntity( tinygltf::Model &model, const QgsGltf3DUtils::EntityTransform &transform, QString baseUri, QStringList *errors );
};

///@endcond

#endif // QGSGLTF3DUTILS_H
