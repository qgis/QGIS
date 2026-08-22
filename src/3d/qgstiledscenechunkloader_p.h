/***************************************************************************
  qgstiledscenechunkloader_p.h
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

#ifndef QGSTILEDSCENECHUNKLOADER_P_H
#define QGSTILEDSCENECHUNKLOADER_P_H

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
#include "qgschunkedentity.h"
#include "qgschunkloader.h"
#include "qgschunknode.h"
#include "qgscoordinatetransform.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenetile.h"

#include <QFutureWatcher>

#define SIP_NO_FILE

class Qgs3DMapSettings;
class QgsTiledSceneChunkLoaderFactory;


/**
 * \ingroup qgis_3d
 * \brief This loader is responsible for creation of individual tiles of tiled
 * scene chunk entity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.34
 */
class QgsTiledSceneChunkLoader : public QgsChunkLoader
{
    Q_OBJECT
  public:
    QgsTiledSceneChunkLoader(
      const Qgs3DRenderContext &context, const QgsTiledSceneIndex &index, QgsCoordinateReferenceSystem tileCrs, QgsCoordinateReferenceSystem layerCrs, double zValueScale, double zValueOffset
    );

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    QgsChunkNode *createRootNode() const override;
    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

    bool canCreateChildren( QgsChunkNode *node ) override;
    QFuture<void> prepareChildren( QgsChunkNode *node ) override;

    QgsChunkNode *nodeForTile( const QgsTiledSceneTile &t, const QgsChunkNodeId &nodeId, QgsChunkNode *parent ) const;
    QFuture<void> fetchHierarchyForNode( long long nodeId );

    Qgs3DRenderContext mRenderContext;
    QString mRelativePathBase;
    mutable QgsTiledSceneIndex mIndex;
    double mZValueScale = 1.0;
    double mZValueOffset = 0;
    QgsCoordinateTransform mBoundsTransform;
    QgsCoordinateReferenceSystem mLayerCrs;
    QSet<long long> mPendingHierarchyFetches;
    QSet<long long> mFutureHierarchyFetches;
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of tiled scene layers.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it
 * uses QgsTiledSceneChunkLoader to do the actual work of loading and creating
 * 3D sub-entities for each tile.
 *
 * \since QGIS 3.34
 */
class QgsTiledSceneLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    explicit QgsTiledSceneLayerChunkedEntity(
      Qgs3DMapSettings *map,
      const QgsTiledSceneIndex &index,
      QgsCoordinateReferenceSystem tileCrs,
      QgsCoordinateReferenceSystem layerCrs,
      double maximumScreenError,
      bool showBoundingBoxes,
      double zValueScale,
      double zValueOffset
    );

    ~QgsTiledSceneLayerChunkedEntity() override;

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

    int pendingJobsCount() const override;

  private:
    mutable QgsTiledSceneIndex mIndex;
};

/// @endcond

#endif // QGSTILEDSCENECHUNKLOADER_P_H
