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

#include "qgscoordinatetransform.h"

class QMatrix4x4;
class QImage;

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

    /**
     * Helper structure with additional context for conversion of a Draco-encoded
     * geometry of a I3S node.
     * \since QGIS 4.0
     */
    struct I3SNodeContext
    {

      /**
       * Material parsed from I3S material definition of the node. See
       * loadMaterialFromMetadata() for more details about its content.
       */
      QVariantMap materialInfo;

      /**
       * A flag whether we are in "global" mode, i.e. the geometry's XY
       * coordinates are lat/lon decimal degrees (in EPSG:4326).
       * When not in global mode, we are using a projected CRS.
       */
      bool isGlobalMode = false;

      /**
       * Only applies when in global mode: transform from dataset's native CRS
       * (lat/lon in degrees) to the scene CRS (ECEF - used in scene index).
       */
      QgsCoordinateTransform datasetToSceneTransform;

      /**
       * Only applies when in global mode: origin of the node's geometry
       * (ECEF coordinates).
       */
      QgsVector3D nodeCenterEcef;
    };

    /**
     * Loads a GLTF 2.0 model from I3S node's geometry in Draco file format.
     * The function additionally needs the context of the I3S node, especially
     * information about the material to be used.
     *
     * The function implements I3S-specific behaviors:
     *
     * - if position attribute contains "i3s-scale_x" and "i3s-scale_y" metadata,
     *   they will be used to scale XY position coordinates (used when XY are in degrees)
     * - if there is a generic attribute with "i3s-attribute-type" metadata being "uv-region",
     *   the UV coordinates of each vertex are updated accordingly
     *
     * \since QGIS 4.0
     */
    static bool loadDracoModel( const QByteArray &data, const I3SNodeContext &context, tinygltf::Model &model, QString *errors = nullptr );

    /**
     * Loads a material into a model (including additions of texture and image objects)
     * from a variant map representing GLTF 2.0 material. The following subset of properties
     * is supported:
     *
     * - "pbrBaseColorFactor" - a list of 4 doubles (RGBA color)
     * - "pbrBaseColorTexture" - a string with URI of a texture
     * - "doubleSided" - a boolean indicating whether the material is double sided (no culling)
     *
     * \since QGIS 4.0
     */
    static int loadMaterialFromMetadata( const QVariantMap &materialInfo, tinygltf::Model &model );

    /**
     * Writes a model to a binary GLTF file (.glb)
     * \since QGIS 4.0
     */
    static bool writeGltfModel( const tinygltf::Model &model, const QString &outputFilename );
};

///@endcond

#endif // QGSGLTFUTILS_H
