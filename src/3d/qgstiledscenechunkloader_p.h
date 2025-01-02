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

#include "qgscoordinatetransform.h"
#include "qgschunkedentity.h"
#include "qgschunkloader.h"
#include "qgschunknode.h"
#include "qgstiledsceneindex.h"
#include "qgstiledscenetile.h"
#include "qgs3drendercontext.h"

#include <QFutureWatcher>

#define SIP_NO_FILE

class Qgs3DMapSettings;
class QgsTiledSceneChunkLoaderFactory;


/**
 * \ingroup 3d
 * \brief This loader class is responsible for async loading of data for a single tile
 * of tiled scene chunk entity and creation of final 3D entity from the data
 * previously prepared in a worker thread.
 *
 * \since QGIS 3.34
 */
class QgsTiledSceneChunkLoader : public QgsChunkLoader
{
    Q_OBJECT
  public:
    QgsTiledSceneChunkLoader( QgsChunkNode *node, const QgsTiledSceneIndex &index, const QgsTiledSceneChunkLoaderFactory &factory, double zValueScale, double zValueOffset );

    ~QgsTiledSceneChunkLoader();

    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent );

  private:
    const QgsTiledSceneChunkLoaderFactory &mFactory;
    QgsTiledSceneIndex mIndex;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
    Qt3DCore::QEntity *mEntity = nullptr;
};


/**
 * \ingroup 3d
 * \brief This loader factory is responsible for creation of loaders for individual tiles
 * of tiled scene chunk entity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.34
 */
class QgsTiledSceneChunkLoaderFactory : public QgsChunkLoaderFactory
{
    Q_OBJECT
  public:
    QgsTiledSceneChunkLoaderFactory(
      const Qgs3DRenderContext &context, const QgsTiledSceneIndex &index, QgsCoordinateReferenceSystem tileCrs,
      double zValueScale, double zValueOffset
    );

    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    virtual QgsChunkNode *createRootNode() const override;
    virtual QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

    virtual bool canCreateChildren( QgsChunkNode *node ) override;
    virtual void prepareChildren( QgsChunkNode *node ) override;

    QgsChunkNode *nodeForTile( const QgsTiledSceneTile &t, const QgsChunkNodeId &nodeId, QgsChunkNode *parent ) const;
    void fetchHierarchyForNode( long long nodeId, QgsChunkNode *origNode );

    Qgs3DRenderContext mRenderContext;
    QString mRelativePathBase;
    mutable QgsTiledSceneIndex mIndex;
    double mZValueScale = 1.0;
    double mZValueOffset = 0;
    QgsCoordinateTransform mBoundsTransform;
    QSet<long long> mPendingHierarchyFetches;
    QSet<long long> mFutureHierarchyFetches;
};


/**
 * \ingroup 3d
 * \brief 3D entity used for rendering of tiled scene layers.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsTiledSceneChunkLoaderFactory and QgsTiledSceneChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
 *
 * \since QGIS 3.34
 */
class QgsTiledSceneLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    explicit QgsTiledSceneLayerChunkedEntity( Qgs3DMapSettings *map, const QgsTiledSceneIndex &index, QgsCoordinateReferenceSystem tileCrs, double maximumScreenError, bool showBoundingBoxes, double zValueScale, double zValueOffset );

    ~QgsTiledSceneLayerChunkedEntity();

    QVector<QgsRayCastingUtils::RayHit> rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QgsRayCastingUtils::RayCastContext &context ) const override;

    int pendingJobsCount() const override;

  private:
    mutable QgsTiledSceneIndex mIndex;
};

/// @endcond

#endif // QGSTILEDSCENECHUNKLOADER_P_H
