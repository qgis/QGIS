/***************************************************************************
                         qgscesiumimplicittiling.h
                         -------------------------
    begin                : March 2026
    copyright            : (C) 2026 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCESIUMIMPLICITTILING_H
#define QGSCESIUMIMPLICITTILING_H

#include <nlohmann/json.hpp>

#include "qgis_core.h"
#include "qgstiledsceneboundingvolume.h"

#include <QBitArray>
#include <QUrl>

#define SIP_NO_FILE

class QgsTiledSceneNode;

/**
 * \ingroup core
 * \brief Contains utilities for working with Cesium 3D Tiles implicit tiling.
 *
 * Currently only supporting QUADTREE subdivision scheme.
 *
 * \note Not available in Python bindings - private API that may change later
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsCesiumImplicitTiling
{
  public:
    //! Contains ZXY coordinates of a node within implicit tiling
    struct TileCoordinate
    {
        int level = 0;
        int x = 0;
        int y = 0;

        auto operator<=>( const TileCoordinate & ) const = default;
        bool operator==( const TileCoordinate & ) const = default;

        friend inline size_t qHash( const TileCoordinate &c, size_t seed = 0 ) { return qHashMulti( seed, c.level, c.x, c.y ); }
    };

    /**
     * Data about subtree of a node - there should be subtree at least for root
     * implicit tiling node, and there may be further subtrees if the hierarchy
     * is deeper.
     *
     * Number of zoom levels covered by a single subtree is given by
     * Root::subtreeLevels.
     *
     * For more details, see the spec, there are some diagrams about subtrees:
     * https://github.com/CesiumGS/3d-tiles/tree/main/specification/ImplicitTiling
     */
    struct Subtree
    {
        /**
       * Bit array whether a tile is available. Contains multiple zoom levels,
       * within a single zoom level tiles are ordered using Morton Z-order curve.
       *
       * If a tile is available, that means it may optionally also have content
       * assigned to it (check "contentAvailability" bit array) and/or it may have
       * some children.
       */
        QBitArray tileAvailability;
        /**
       * Bit array whether a tile has some content. Contains multiple zoom levels,
       * within a single zoom level tiles are ordered using Morton Z-order curve.
       *
       * If a content is available, we should be able to fetch it using the templat
       * content URI.
       */
        QBitArray contentAvailability;
        /**
       * Bit array to know where to look for more subtree definitions deeper
       * in the hierarchy. This bit array is for single zoom level one level deeper
       * than the deepest level of subtree data - for example, for subtreeLevels=3,
       * this will cover availability of subtrees at zoom level 4.
       */
        QBitArray childSubtreeAvailability;
    };

    /**
     * Definition of root implicit tiling node (typically root node of the whole
     * tileset, but it does not have to be - there could be even multiple root
     * implicit tiling nodes within a tileset).
     *
     * For more details, see the spec, there are some diagrams about subtrees:
     * https://github.com/CesiumGS/3d-tiles/tree/main/specification/ImplicitTiling
     */
    struct Root
    {
        // definition of the tiling scheme

        //! total number of available levels within the implicit tiling
        int availableLevels = 0;
        //! how many levels are stored in a single subtree
        int subtreeLevels = 0;
        QString contentUriTemplate;
        QString subtreeUriTemplate;
        QUrl baseUrl;

        // data related to the root node

        //! if the root node uses OBB as the bounding volume, we use it directly to create child volumes
        QgsTiledSceneBoundingVolume rootBoundingVolume;
        //! if the root node uses "region" bounding volume (in lat/lon), we use it to create child regions and then transform them to OBBs
        std::optional<QgsBox3D> rootRegion;

        double rootGeometricError = 0;
        Qgis::TileRefinementProcess refinementProcess = Qgis::TileRefinementProcess::Replacement;
        std::optional<QgsMatrix4x4> rootTransform;
        Qgis::Axis gltfUpAxis = Qgis::Axis::Y;

        QMap<TileCoordinate, Subtree> subtreeCache;
    };

    //! Parses JSON definition of implicit tiling into tilingData argument and returns TRUE on success
    static bool parseImplicitTiling( const json &tileJson, QgsTiledSceneNode *newNode, const QUrl &baseUrl, Qgis::Axis gltfUpAxis, Root &tilingData );

    //! Parses subtree definition and returns it
    static Subtree parseSubtree( const Root &tilingData, const QByteArray &data );

    //! Expands template URI (using {level}, {x}, {y} markers) to the final URI
    static QString expandTemplateUri( const QString &templateUri, const QUrl &baseUrl, const TileCoordinate &coord );

    //! Returns parent subtree's tile coordinates of the given tile
    static TileCoordinate tileCoordinateToParentSubtree( TileCoordinate coord, int subtreeLevels );

    //! Creates immediate children of a node according to implicit tiling
    static QMap<TileCoordinate, QgsTiledSceneNode *> createImplicitTilingChildren(
      QgsTiledSceneNode *node, const TileCoordinate &coord, Root &tilingData, const TileCoordinate &subtreeCoord, QgsCoordinateTransformContext &transformContext, long long &nextTileId
    );

    //! Returns tile availability of a child based on cached subtree data
    static Qgis::TileChildrenAvailability childAvailability( const Root &tilingData, const TileCoordinate &coord );

    //! Returns the bit index within a subtree's availability bitstream for a given local tile position
    static int subtreeBitIndex( int localLevel, int localX, int localY );

  private:
    static int subtreeTileCount( int levels );
    static int mortonIndex( int x, int y );
    static QgsTiledSceneBoundingVolume computeImplicitBoundingVolume( const QgsTiledSceneBoundingVolume &rootVolume, const TileCoordinate &coord );
    static QgsBox3D computeImplicitRegionBoundingVolume( const QgsBox3D &rootRegion, const TileCoordinate &coord );
};

#endif // QGSCESIUMIMPLICITTILING_H
