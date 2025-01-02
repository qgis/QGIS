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

#include "qgis_core.h"
#include "qgsbox3d.h"
#include "qgsvector3d.h"
#include "qgis_sip.h"
#include <nlohmann/json_fwd.hpp>

#ifndef SIP_RUN
using namespace nlohmann;
#endif

class QgsSphere;
class QgsOrientedBox3D;
class QgsMatrix4x4;

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

    /**
     * Encapsulates the contents of a 3D tile.
     */
    struct TileContents
    {
      //! GLTF binary content
      QByteArray gltf;

      //! Center position of relative-to-center coordinates (when used)
      QgsVector3D rtcCenter;
    };

    /**
     * Parses tile content.
     * Returns empty byte array on error.
     *
     * \note cmpt, pnts, i3dm tile types are currently not supported
     */
    static TileContents extractGltfFromTileContent( const QByteArray &tileContent );

};

#endif // QGSCESIUMUTILS_H
