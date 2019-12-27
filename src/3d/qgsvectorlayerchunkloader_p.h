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

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"

class Qgs3DMapSettings;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;
class QgsAbstract3DSymbol;
class QgsFeature3DHandler;

#include <QFutureWatcher>


class QgsVectorLayerChunkLoaderFactory : public QgsChunkLoaderFactory
{
  public:
    QgsVectorLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, int leafLevel );

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const;

    const Qgs3DMapSettings &mMap;
    QgsVectorLayer *mLayer;
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;
    int mLeafLevel;
};


class QgsVectorLayerChunkLoader : public QgsChunkLoader
{
  public:
    QgsVectorLayerChunkLoader( const QgsVectorLayerChunkLoaderFactory *factory, QgsChunkNode *node );
    ~QgsVectorLayerChunkLoader();

    virtual void cancel();
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent );

  private:
    const QgsVectorLayerChunkLoaderFactory *mFactory;
    QgsFeature3DHandler *mHandler = nullptr;
    Qgs3DRenderContext mContext;
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
};

#include "qgschunkedentity_p.h"
class QgsVectorLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity. The argument maxLevel determines how deep the tree of tiles will be
    explicit QgsVectorLayerChunkedEntity( QgsVectorLayer *vl, QgsAbstract3DSymbol *symbol, const Qgs3DMapSettings &map );

    ~QgsVectorLayerChunkedEntity();
};

#endif // QGSVECTORLAYERCHUNKLOADER_P_H
