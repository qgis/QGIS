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
#include "qgscategorized3drenderer.h"
#include "qgschunkedentity.h"
#include "qgschunkloader.h"

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
 * \brief This loader factory is responsible for creation of loaders for individual tiles
 * of QgsCategorizedChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 4.0
 */
class QgsCategorizedChunkLoaderFactory : public QgsQuadtreeChunkLoaderFactory
{
    Q_OBJECT

  public:
    //! Constructs the factory (vectorLayer and renderer must not be null)
    QgsCategorizedChunkLoaderFactory( const Qgs3DRenderContext &context, QgsVectorLayer *vectorLayer, const QgsCategorized3DRenderer *renderer, double zMin, double zMax, int maxFeatures );
    ~QgsCategorizedChunkLoaderFactory() override;

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    bool canCreateChildren( QgsChunkNode *node ) override;
    QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;

    Qgs3DRenderContext mRenderContext;
    QgsVectorLayer *mLayer;
    Qgs3DCategoryList mCategories;
    QString mAttributeName;
    //! Contains loaded nodes and whether they are leaf nodes or not
    mutable QHash< QString, bool > mNodesAreLeafs;
    int mMaxFeatures;
};


/**
 * \ingroup qgis_3d
 * \brief This loader class is responsible for async loading of data for a single tile
 * of QgsCategorizedChunkedEntity and creation of final 3D entity from the data
 * previously prepared in a worker thread.
 *
 * \since QGIS 4.0
 */
class QgsCategorizedChunkLoader : public QgsChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader (factory and node must not be null)
    QgsCategorizedChunkLoader( const QgsCategorizedChunkLoaderFactory *factory, QgsChunkNode *node );
    ~QgsCategorizedChunkLoader() override;

    void start() override;
    virtual void cancel() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    const QSet<QString> prepareHandlers( const QgsBox3D &chunkExtent );
    void processFeature( const QgsFeature &feature ) const;

  private:
    const QgsCategorizedChunkLoaderFactory *mFactory;
    Qgs3DRenderContext mContext;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
    bool mNodeIsLeaf = false;

    //! hashtable for faster access to symbols
    QHash<QString, QgsFeature3DHandler *> mFeaturesHandlerHash;
    std::unique_ptr<QgsExpression> mExpression;
    int mAttributeIdx = -1;
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of vector layers using categories (just like
 * in case of 2D categories rendering).
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsCategorizedChunkLoaderFactory and QgsCategorizedChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
 *
 * \since QGIS 4.0
 */
class QgsCategorizedChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsCategorizedChunkedEntity( Qgs3DMapSettings *mapSettings, QgsVectorLayer *vectorLayer, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, const QgsCategorized3DRenderer *renderer );

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

    ~QgsCategorizedChunkedEntity() override;
  private slots:
    void onTerrainElevationOffsetChanged();

  private:
    Qt3DCore::QTransform *mTransform = nullptr;

    bool applyTerrainOffset() const;
};

/// @endcond

#endif // QGSCATEGORIZEDCHUNKLOADER_H
