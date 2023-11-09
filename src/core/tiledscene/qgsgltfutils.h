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

#ifndef QGSGLTFUTILS_H
#define QGSGLTFUTILS_H

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

#include "qgis_core.h"
#include "qgis.h"

#include <memory>
#include <QVector>

class QMatrix4x4;
class QImage;

class QgsCoordinateTransform;
class QgsMatrix4x4;
class QgsVector3D;

namespace tinygltf
{
  struct Image;
  class Model;
  class Node;
  class TinyGLTF;
}


/**
 * \ingroup core
 *
 * Utility functions for dealing with GLTF models.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsGltfUtils
{
  public:

    /**
     * Reads model's accessor given by \a accessorIndex and applies a couple of transforms
     * to convert the raw coordinates to map coordinates (assuming this is a GLTF to be used
     * in a 3D Tiles dataset). Sets \a vx, \a vy, \a vz vectors with final X,Y,Z coordinates.
     *
     * The chain of transform goes like this: P_SRC -> P_GLTF -> P_ECEF -> P_MAP.
     * P_SRC are raw coordinates as stored in GLTF vertex buffer. P_GLTF are final
     * coordinates stored by the GLTF file ( \a nodeTransform * P_SRC ).
     * P_ECEF are Earth-centered earth-fixed (ECEF) coordinates: X,Y define equator plane
     * and Z is point towards North pole (EPSG:4978). P_ECEF are calculated as
     * \a tileTransform * (flip_ZY(P_GLTF) + \a tileTranslationEcef).
     * Finally, P_MAP is calculated by optionally doing a coordinate transform with PROJ
     * using \a ecefToTargetCrs
     */
    static bool accessorToMapCoordinates( const tinygltf::Model &model,
                                          int accessorIndex,
                                          const QgsMatrix4x4 &tileTransform,
                                          const QgsCoordinateTransform *ecefToTargetCrs,
                                          const QgsVector3D &tileTranslationEcef,
                                          const QMatrix4x4 *nodeTransform,
                                          Qgis::Axis gltfUpAxis,
                                          QVector<double> &vx, QVector<double> &vy, QVector<double> &vz );

    /**
     * Types of resources referenced by GLTF models.
     */
    enum class ResourceType
    {
      Embedded, //!< Embedded resource
      Linked, //!< Linked (external) resource
    };

    /**
     * Returns the resource type of the image with specified \a index from a \a model.
     *
     * \see extractEmbeddedImage()
     * \see linkedImagePath()
     */
    static ResourceType imageResourceType( const tinygltf::Model &model, int index );

    /**
     * Extracts the embedded image with specified \a index from a \a model.
     *
     * Returns a null QImage if no embedded image exists with the given \a index.
     *
     * \see imageResourceType()
     * \see linkedImagePath()
     */
    static QImage extractEmbeddedImage( const tinygltf::Model &model, int index );

    /**
     * Extracts the path to a linked image with specified \a index from a \a model.
     *
     * Returns a empty string if no linked image exists with the given \a index.
     *
     * \see imageResourceType()
     * \see extractEmbeddedImage()
     */
    static QString linkedImagePath( const tinygltf::Model &model, int index );

    /**
     * Parses transform of a node - either by reading the 4x4 transform matrix,
     * or by reading translation, rotation and scale, combining all to the final matrix.
     * Returns null pointer if no transform is attached.
     */
    static std::unique_ptr<QMatrix4x4> parseNodeTransform( const tinygltf::Node &node );

    /**
     * Try to extract translation of the model: by either using CESIUM_RTC extension
     * or by taking translation from the root node. The returned offset should be
     * in ECEF coordinates.
     */
    static QgsVector3D extractTileTranslation( tinygltf::Model &model, Qgis::Axis upAxis = Qgis::Axis::Y );

    /**
     * Helper function to allow tinygltf to read images, based on QImage readers.
     */
    static bool loadImageDataWithQImage(
      tinygltf::Image *image, const int image_idx, std::string *err,
      std::string *warn, int req_width, int req_height,
      const unsigned char *bytes, int size, void *user_data );

    /**
     * Extracts the texture coordinates from a \a model, and stores the results in the \a x, \a y vectors.
     */
    static bool extractTextureCoordinates( const tinygltf::Model &model, int accessorIndex,
                                           QVector<float> &x, QVector<float> &y );

    /**
     * Loads a GLTF model from \a data (both binary and text format are supported)
     * and stores the result in the given \a model. Returns true on success.
     * May set \a errors and/or \a warnings if they are not null pointers.
     */
    static bool loadGltfModel( const QByteArray &data, tinygltf::Model &model, QString *errors, QString *warnings );

    /**
     * Returns the index for the scene to load from a \a model.
     *
     * This will be the model's default scene, unless the default scene is invalid in which case
     * it will just be the first scene found in the model.
     *
     * If no scene is available, \a ok will be set to FALSE.
     */
    static std::size_t sourceSceneForModel( const tinygltf::Model &model, bool &ok );
};

///@endcond

#endif // QGSGLTFUTILS_H
