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


#include "qgis_3d.h"
#include "qgsgltfutils.h"
#include "qgsmatrix4x4.h"

#include <QQuaternion>
#include <QVector3D>

#define SIP_NO_FILE

#define TINYGLTF_NO_STB_IMAGE       // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE // we don't need writing of images
#include "tiny_gltf.h"

class QgsCoordinateTransform;
class Qgs3DRenderContext;
class QgsMaterialContext;

namespace Qt3DCore
{
  class QEntity;
  class QGeometry;
} //namespace Qt3DCore

/**
 * \ingroup qgis_3d
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
    static Qt3DCore::QEntity *gltfToEntity( const QByteArray &data, const EntityTransform &transform, const QString &baseUri, const Qgs3DRenderContext &context, QStringList *errors = nullptr );

    /**
     * Converts a GLTF model into a Qt 3D entity.
     * \see gltfToEntity()
     */
    static Qt3DCore::QEntity *parsedGltfToEntity( tinygltf::Model &model, const QgsGltf3DUtils::EntityTransform &transform, QString baseUri, const Qgs3DRenderContext &context, QStringList *errors );

    /**
     * Per-instance chunk-local transform (ready for the shader buffer).
     * \since QGIS 4.2
     */
    struct InstanceChunkTransform
    {
        QVector3D translation; //!< Chunk-local position (map CRS - chunk origin)
        QQuaternion rotation;  //!< Chunk-local rotation
        QVector3D scale;       //!< Effective scale
    };

    /**
     * Computes a 3×3 rotation correction matrix that maps vectors from ECEF frame
     * to target CRS frame at a given ECEF position. This accounts for the fact that
     * local east/north/up directions in ECEF are not aligned with the target CRS axes.
     *
     * Returns identity if the reprojection of perturbed points fails.
     *
     * \param ecefPos ECEF position to compute the correction at
     * \param mapPos the same position already reprojected to target CRS
     * \param ecefToTargetCrs coordinate transform from ECEF to target CRS
     * \since QGIS 4.2
     */
    static QMatrix3x3 ecefToTargetCrsRotationCorrection( const QgsVector3D &ecefPos, const QgsVector3D &mapPos, const QgsCoordinateTransform &ecefToTargetCrs );

    /**
     * Converts tile-space per-instance matrices to chunk-local T/R/S.
     * \since QGIS 4.2
     */
    static QVector<InstanceChunkTransform> tileSpaceToChunkLocal( const QgsGltfUtils::InstancedPrimitive &primitive, const EntityTransform &transform );

    /**
     * Adds per-instance GPU attributes (translation, rotation, scale) to the geometry.
     * \since QGIS 4.2
     */
    static void createInstanceBuffer( Qt3DCore::QGeometry *geometry, const QVector<InstanceChunkTransform> &instances );

    /**
     * Creates Qt3D entities from instanced primitives resolved by resolveInstancing().
     * \since QGIS 4.2
     */
    static QVector<Qt3DCore::QEntity *> createInstancedEntities(
      tinygltf::Model &model, const QVector<QgsGltfUtils::InstancedPrimitive> &primitives, const EntityTransform &transform, const QString &baseUri, const QgsMaterialContext &context, QStringList *errors
    );
};

///@endcond

#endif // QGSGLTF3DUTILS_H
