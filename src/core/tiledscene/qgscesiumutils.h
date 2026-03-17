/***************************************************************************
                         qgscesiumutils.h
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

#ifndef QGSCESIUMUTILS_H
#define QGSCESIUMUTILS_H

#include <nlohmann/json_fwd.hpp>
#include <optional>

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbox3d.h"
#include "qgsvector3d.h"

#include <QQuaternion>
#include <QVector>
#include <QVector3D>

#ifndef SIP_RUN
using namespace nlohmann;
#endif

class QgsCoordinateTransformContext;
class QgsSphere;
class QgsOrientedBox3D;
class QgsMatrix4x4;
class QgsTiledSceneBoundingVolume;

#ifndef SIP_RUN
namespace tinygltf
{
  class Model;
}
#include "qgsgltfutils.h"
#endif

/**
 * \brief Contains utilities for working with Cesium data.
 *
 * \ingroup core
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsCesiumUtils
{
  public:
#ifndef SIP_RUN

    /**
    * Parses a \a region object from a Cesium JSON object to a 3D box.
    *
    * \note Not available in Python bindings.
    */
    static QgsBox3D parseRegion( const json &region );
#endif

    /**
     * Parses a \a region object from a Cesium JSON document to a 3D box.
     */
    static QgsBox3D parseRegion( const QVariantList &region );

#ifndef SIP_RUN

    /**
    * Parses a \a box object from a Cesium JSON document to an oriented bounding box.
    *
    * \note Not available in Python bindings.
    */
    static QgsOrientedBox3D parseBox( const json &box );
#endif

    /**
    * Parses a \a box object from a Cesium JSON document to an oriented bounding box.
    */
    static QgsOrientedBox3D parseBox( const QVariantList &box );

#ifndef SIP_RUN

    /**
    * Parses a \a sphere object from a Cesium JSON document.
    *
    * \note Not available in Python bindings.
    */
    static QgsSphere parseSphere( const json &sphere );
#endif

    /**
    * Parses a \a sphere object from a Cesium JSON document.
    */
    static QgsSphere parseSphere( const QVariantList &sphere );

    /**
     * Applies a \a transform to a sphere.
     */
    static QgsSphere transformSphere( const QgsSphere &sphere, const QgsMatrix4x4 &transform );

    /**
     * Encapsulates the contents of a B3DM file.
     */
    struct B3DMContents
    {
        //! GLTF binary content
        QByteArray gltf;

        //! Optional RTC center
        QgsVector3D rtcCenter;
    };

    /**
     * Extracts GLTF binary data and other contents from the legacy b3dm (Batched 3D Model) tile format.
     * Returns empty byte array on error.
     */
    static B3DMContents extractGltfFromB3dm( const QByteArray &tileContent );

#ifndef SIP_RUN
    /**
     * \brief Raw per-instance data parsed from an i3dm feature table of a single tile
     *
     * This struct transports i3dm instance data from the binary parser through
     * TileContents to resolveInstancing() - to handle 3D Tiles 1.0 spec.
     *
     * In 3D Tiles 1.1 spec, instanced meshes are handled by EXT_mesh_gpu_instancing
     * glTF extension, and this struct is not used at all, because instancing is parsed
     * directly from glTF nodes inside resolveInstancing().
     *
     * All positions are in i3dm tile space (Z-up), relative to RTC_CENTER.
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 4.2
     */
    struct TileI3dmData
    {
        //! Number of instances
        int instanceCount = 0;
        //! ECEF-relative positions (Z-up), relative to RTC_CENTER
        QVector<QVector3D> translations;
        //! Quaternion (x,y,z,w) - identity if unspecified
        QVector<QQuaternion> rotations;
        //! Per-axis scale - (1,1,1) if unspecified
        QVector<QVector3D> scales;
        //! Whether EAST_NORTH_UP rotations should be computed (deferred until tile transform is available)
        bool eastNorthUp = false;
    };
#endif

    /**
     * Encapsulates the contents of a 3D tile.
     */
    struct TileContents
    {
        //! GLTF binary content
        QByteArray gltf;

        //! Center position of relative-to-center coordinates (when used)
        QgsVector3D rtcCenter;

#ifndef SIP_RUN
        //! Optional instancing data, populated for i3dm tiles
        std::optional<TileI3dmData> instancing;
#endif
    };

    /**
     * Parses tile content.
     * Returns empty byte array on error.
     *
     * \note cmpt, pnts, i3dm tile types are currently not supported
     *
     * \deprecated QGIS 4.2. : use extractTileContent() which can handle composite tiles as well.
     */
    Q_DECL_DEPRECATED static TileContents extractGltfFromTileContent( const QByteArray &tileContent ) SIP_DEPRECATED;

    /**
     * Parses tile content and returns a list of TileContents.
     *
     * For b3dm and glTF tiles, the returned list will contain a single entry.
     * For cmpt (composite) tiles, the returned list will contain one entry
     * per inner tile that could be successfully parsed.
     * Returns an empty list on error or for unsupported tile types (pnts, i3dm).
     *
     * \since QGIS 4.2
     */
    static QVector<QgsCesiumUtils::TileContents> extractTileContent( const QByteArray &tileContent, const QString &baseUri = QString() );

    /**
     * Calculates oriented bounding box in EPSG:4978 from "region" defined with min/max lat/lon coordinates in EPSG:4978.
     * \note added in QGIS 4.2
     */
    static QgsTiledSceneBoundingVolume boundingVolumeFromRegion( const QgsBox3D &region, const QgsCoordinateTransformContext &transformContext );

    /**
     * Copies any query items from the base URL to the content URI - to replicate undocumented
     * Cesium JS behavior that is used at least by Google Tiles.
     *
     * \note added in QGIS 4.2
     */
    static QString appendQueryFromBaseUrl( const QString &contentUri, const QUrl &baseUrl );

#ifndef SIP_RUN

    /**
     * Resolves instancing from either i3dm data or EXT_mesh_gpu_instancing.
     *
     * Walks the glTF node tree, decomposes multi-node models into independent
     * mesh primitives, and computes per-instance TRS in a common "tile space".
     *
     * Returns an empty vector if no instancing data is found, meaning the model
     * should be rendered through the existing non-instanced code path.
     *
     * \param model the loaded tinygltf model
     * \param tileInstancing optional i3dm instance data (nullopt for EXT/b3dm)
     * \param gltfUpAxis the up axis used in the glTF model
     * \param tileTransform tile transform from tileset.json (tile-local → ECEF), needed for deferred EAST_NORTH_UP computation
     * \param rtcCenter RTC_CENTER offset in tile-local space
     * \returns flat list of instanced primitives with tile-space transforms
     * \since QGIS 4.2
     */
    static QVector<QgsGltfUtils::InstancedPrimitive> resolveInstancing(
      const tinygltf::Model &model, const std::optional<TileI3dmData> &tileInstancing, Qgis::Axis gltfUpAxis, const QgsMatrix4x4 &tileTransform, const QgsVector3D &rtcCenter
    );

#endif // SIP_RUN
};

#endif // QGSCESIUMUTILS_H
