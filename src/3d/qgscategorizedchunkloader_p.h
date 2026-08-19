/***************************************************************************
  qgscategorizedchunkloader_p.h
  --------------------------------------
  Date                 : November 2025
  Copyright            : (C) 2025 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCATEGORIZEDCHUNKLOADER_H
#define QGSCATEGORIZEDCHUNKLOADER_H

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
#include "qgscategorized3drenderer.h"
#include "qgschunkloader.h"

#include <QFutureWatcher>
#include <QMutex>

#define SIP_NO_FILE

class Qgs3DMapSettings;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsAbstract3DSymbol;
class QgsFeature3DHandler;

namespace Qt3DCore
{
  class QTransform;
}

/**
 * \ingroup qgis_3d
 * \brief This loader is responsible for loading individual tiles of
 * QgsCategorizedChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 4.2
 */
class QgsCategorizedChunkLoader : public QgsQuadtreeChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader (vectorLayer and renderer must not be null)
    QgsCategorizedChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vectorLayer, const QgsCategorized3DRenderer *renderer, double zMin, double zMax, int maxFeatures );
    ~QgsCategorizedChunkLoader() override;

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    QFuture<QVector<QgsChunkNode *>> createChildren( QgsChunkNode *node ) override;

  private:
    Qgs3DRenderContext mRenderContext;
    const QgsVectorLayer *mLayer = nullptr;
    const Qgs3DCategoryList *mCategories = nullptr;
    QString mAttributeName;
    //! Contains loaded nodes and whether they are leaf nodes or not
    QHash< QString, bool > mNodesAreLeafs;
    QMutex mNodesAreLeafsMutex;
    int mMaxFeatures = 0;

    friend class QgsCategorizedChunkedEntity;
};

/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of vector layers using categories (just like
 * in case of 2D categories rendering).
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it
 * uses QgsCategorizedChunkLoader to do the actual work of loading and creating
 * 3D sub-entities for each tile.
 *
 * \since QGIS 4.2
 */
class QgsCategorizedChunkedEntity : public QgsAbstractFeatureBasedChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity.
    explicit QgsCategorizedChunkedEntity(
      Qgs3DMapSettings *mapSettings, QgsVectorLayer *vectorLayer, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, const QgsCategorized3DRenderer *renderer
    );

    ~QgsCategorizedChunkedEntity() override;

  private:
    bool applyTerrainOffset() const override;
};

/// @endcond

#endif // QGSCATEGORIZEDCHUNKLOADER_H
