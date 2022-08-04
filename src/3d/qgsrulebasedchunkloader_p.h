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

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"
#include "qgschunkedentity_p.h"
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
 * \ingroup 3d
 * \brief This loader factory is responsible for creation of loaders for individual tiles
 * of QgsRuleBasedChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.12
 */
class QgsRuleBasedChunkLoaderFactory : public QgsQuadtreeChunkLoaderFactory
{
    Q_OBJECT

  public:
    //! Constructs the factory (vl and rootRule must not be null)
    QgsRuleBasedChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsRuleBased3DRenderer::Rule *rootRule, int leafLevel, double zMin, double zMax );
    ~QgsRuleBasedChunkLoaderFactory() override;

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    const Qgs3DMapSettings &mMap;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsRuleBased3DRenderer::Rule> mRootRule;
    int mLeafLevel;
};


/**
 * \ingroup 3d
 * \brief This loader class is responsible for async loading of data for a single tile
 * of QgsRuleBasedChunkedEntity and creation of final 3D entity from the data
 * previously prepared in a worker thread.
 *
 * \since QGIS 3.12
 */
class QgsRuleBasedChunkLoader : public QgsChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader (factory and node must not be null)
    QgsRuleBasedChunkLoader( const QgsRuleBasedChunkLoaderFactory *factory, QgsChunkNode *node );
    ~QgsRuleBasedChunkLoader() override;

    virtual void cancel() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    const QgsRuleBasedChunkLoaderFactory *mFactory;
    QgsRuleBased3DRenderer::RuleToHandlerMap mHandlers;
    Qgs3DRenderContext mContext;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
    std::unique_ptr<QgsRuleBased3DRenderer::Rule> mRootRule;
};


/**
 * \ingroup 3d
 * \brief 3D entity used for rendering of vector layers using a hierarchy of rules (just like
 * in case of 2D rule-based rendering or labeling).
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsRuleBasedChunkLoaderFactory and QgsRuleBasedChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
 *
 * \since QGIS 3.12
 */
class QgsRuleBasedChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsRuleBasedChunkedEntity( QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsRuleBased3DRenderer::Rule *rootRule, const Qgs3DMapSettings &map );

    ~QgsRuleBasedChunkedEntity();
  private slots:
    void onTerrainElevationOffsetChanged( float newOffset );
  private:
    Qt3DCore::QTransform *mTransform = nullptr;
};

/// @endcond

#endif // QGSRULEBASEDCHUNKLOADER_H
