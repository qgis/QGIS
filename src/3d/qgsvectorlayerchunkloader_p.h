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

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"
#include "qgschunkedentity_p.h"

#define SIP_NO_FILE

class Qgs3DMapSettings;
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
 * \ingroup 3d
 * \brief This loader factory is responsible for creation of loaders for individual tiles
 * of QgsVectorLayerChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.12
 */
class QgsVectorLayerChunkLoaderFactory : public QgsQuadtreeChunkLoaderFactory
{
    Q_OBJECT

  public:
    //! Constructs the factory
    QgsVectorLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, int leafLevel, double zMin, double zMax );

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    const Qgs3DMapSettings &mMap;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
    int mLeafLevel;
};


/**
 * \ingroup 3d
 * \brief This loader class is responsible for async loading of data for a single tile
 * of QgsVectorLayerChunkedEntity and creation of final 3D entity from the data
 * previously prepared in a worker thread.
 *
 * \since QGIS 3.12
 */
class QgsVectorLayerChunkLoader : public QgsChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader
    QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node );
    ~QgsVectorLayerChunkLoader() override;

    virtual void cancel() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    const QgsVectorLayerChunkLoaderFactory *mFactory;
    std::unique_ptr<QgsFeature3DHandler> mHandler;
    Qgs3DRenderContext mContext;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
};


/**
 * \ingroup 3d
 * \brief 3D entity used for rendering of vector layers with a single 3D symbol for all features.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsVectorLayerChunkLoaderFactory and QgsVectorLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
 *
 * \since QGIS 3.12
 */
class QgsVectorLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsVectorLayerChunkedEntity( QgsVectorLayer *vl, double zMin, double zMax, const QgsVectorLayer3DTilingSettings &tilingSettings, QgsAbstract3DSymbol *symbol, const Qgs3DMapSettings &map );

    ~QgsVectorLayerChunkedEntity();
  private slots:
    void onTerrainElevationOffsetChanged( float newOffset );

  private:
    Qt3DCore::QTransform *mTransform = nullptr;
};

/// @endcond

#endif // QGSVECTORLAYERCHUNKLOADER_P_H
