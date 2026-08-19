/***************************************************************************
  qgsvectorlayerchunkloader_p.h
  --------------------------------------
  Date                 : July 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERCHUNKLOADER_P_H
#define QGSVECTORLAYERCHUNKLOADER_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include "qgs3drendercontext.h"
#include "qgsabstractfeaturebasedchunkedentity.h"
#include "qgschunkloader.h"
#include "qgschunknode.h"
#include "qgscoordinatetransform.h"
#include "qgsvector3d.h"

#define SIP_NO_FILE

class QgsVectorLayer;
class QgsVectorLayer3DTilingSettings;
class QgsVectorLayerFeatureSource;
class QgsAbstract3DSymbol;
class QgsFeature3DHandler;

namespace Qt3DCore
{
  class QTransform;
}

#include <QFutureWatcher>


/**
 * \ingroup qgis_3d
 * \brief This loader is responsible for creation of individual tiles of
 * QgsVectorLayerChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.12
 */
class QgsVectorLayerChunkLoader : public QgsQuadtreeChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader
    QgsVectorLayerChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, double zMin, double zMax, int maxFeatures );

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    QgsChunkNode *createRootNode() const override;
    QFuture<QVector<QgsChunkNode *>> createChildren( QgsChunkNode *node ) override;

    //! Returns the extent of the quadtree node with the given \a id, in a fixed tilling scheme
    static QgsRectangle nodeIdToLonLatRect( QgsChunkNodeId id );
    //! Returns the id of the smallest tile that fully contains \a lonLatExtent
    static QgsChunkNodeId rootTileIdForExtent( const QgsRectangle &lonLatExtent );
    //! Returns the exact ECEF world-space bounding box of the quadtree tile with the given \a id
    QgsBox3D tileIdToBox3D( QgsChunkNodeId id ) const;
    //! Returns the XY bounding rectangle of a 3d bounding box
    static QgsRectangle box3DTransformedExtent( const QgsBox3D &box3D, const QgsCoordinateTransform &transform, Qgis::TransformDirection direction = Qgis::TransformDirection::Forward );

    Qgs3DRenderContext mRenderContext;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
    //! Contains loaded nodes and whether they are leaf nodes or not
    QHash< QString, bool > mNodesAreLeafs;
    QMutex mNodesAreLeafsMutex;
    int mMaxFeatures;

    bool mIsGeocentric = false;

    // below only used for geocentric case
    QgsChunkNodeId mRootNodeId;
    QgsCoordinateTransform mCrsToLatLon;

    QgsVector3D mRadius;
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of vector layers with a single 3D symbol for all features.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it
 * uses QgsVectorLayerChunkLoader to do the actual work of loading and creating
 * 3D sub-entities for each tile.
 *
 * \since QGIS 3.12
 */
class QgsVectorLayerChunkedEntity : public QgsAbstractFeatureBasedChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsVectorLayerChunkedEntity( Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol );

    ~QgsVectorLayerChunkedEntity() override;

  private:
    bool applyTerrainOffset() const override;
};

/// @endcond

#endif // QGSVECTORLAYERCHUNKLOADER_P_H
