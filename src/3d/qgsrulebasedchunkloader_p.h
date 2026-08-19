/***************************************************************************
  qgsrulebasedchunkloader_p.h
  --------------------------------------
  Date                 : November 2019
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

#ifndef QGSRULEBASEDCHUNKLOADER_H
#define QGSRULEBASEDCHUNKLOADER_H

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
#include "qgsrulebased3drenderer.h"

#include <QFutureWatcher>

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
 * \brief This loader is responsible for creation of individual tiles of
 * QgsRuleBasedChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.12
 */
class QgsRuleBasedChunkLoader : public QgsQuadtreeChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the factory (vl and rootRule must not be null)
    QgsRuleBasedChunkLoader( const Qgs3DRenderContext &context, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, double zMin, double zMax, int maxFeatures );
    ~QgsRuleBasedChunkLoader() override;

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;
    QFuture<QVector<QgsChunkNode *>> createChildren( QgsChunkNode *node ) override;

    Qgs3DRenderContext mRenderContext;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsRuleBased3DRenderer::Rule> mRootRule;
    //! Contains loaded nodes and whether they are leaf nodes or not
    QHash< QString, bool > mNodesAreLeafs;
    mutable QMutex mNodesAreLeafsMutex;
    int mMaxFeatures;
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of vector layers using a hierarchy of rules (just like
 * in case of 2D rule-based rendering or labeling).
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it
 * uses QgsRuleBasedChunkLoader to do the actual work of loading and creating
 * 3D sub-entities for each tile.
 *
 * \since QGIS 3.12
 */
class QgsRuleBasedChunkedEntity : public QgsAbstractFeatureBasedChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsRuleBasedChunkedEntity( Qgs3DMapSettings *map, QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsRuleBased3DRenderer::Rule *rootRule );

    ~QgsRuleBasedChunkedEntity() override;

  private:
    bool applyTerrainOffset() const override;
};

/// @endcond

#endif // QGSRULEBASEDCHUNKLOADER_H
