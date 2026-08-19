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

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    bool canCreateChildren( QgsChunkNode *node ) override;
    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

    Qgs3DRenderContext mRenderContext;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
    //! Contains loaded nodes and whether they are leaf nodes or not
    mutable QHash< QString, bool > mNodesAreLeafs;
    mutable QMutex mNodesAreLeafsMutex;
    int mMaxFeatures;
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of vector layers with a single 3D symbol for all features.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsVectorLayerChunkLoaderFactory and QgsVectorLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
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
