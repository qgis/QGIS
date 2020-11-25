/***************************************************************************
  qgspointcloudlayerchunkloader_p.h
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYERCHUNKLOADER_P_H
#define QGSPOINTCLOUDLAYERCHUNKLOADER_P_H

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
#include "qgspointcloud3dsymbol.h"

#define SIP_NO_FILE

class Qgs3DMapSettings;
class QgsPointCloudLayer;
class IndexedPointCloudNode;
class QgsPointCloudIndex;

#include <memory>

#include <QFutureWatcher>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QMaterial>
#include <QVector3D>


class QgsPointCloud3DSymbolHandler // : public QgsFeature3DHandler
{
  public:
    QgsPointCloud3DSymbolHandler( QgsPointCloud3DSymbol *symbol );

    bool prepare( const Qgs3DRenderContext &context );// override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const Qgs3DRenderContext &context ); // override;
    void finalize( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context );// override;

    float zMinimum() const { return mZMin; }
    float zMaximum() const { return mZMax; }

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
      QVector<float> parameter;
      QVector<QVector3D> colors;
    };

  protected:
    float mZMin = std::numeric_limits<float>::max();
    float mZMax = std::numeric_limits<float>::lowest();

  private:

    //static void addSceneEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent );
    //static void addMeshEntities( const Qgs3DMapSettings &map, const QVector<QVector3D> &positions, const QgsPoint3DSymbol *symbol, Qt3DCore::QEntity *parent, bool are_selected );
    //static Qt3DCore::QTransform *transform( QVector3D position, const QgsPoint3DSymbol *symbol );

  private:
    Qt3DRender::QMaterial *constructMaterial( QgsNoRenderingPointCloud3DSymbol *symbol );
    Qt3DRender::QMaterial *constructMaterial( QgsSingleColorPointCloud3DSymbol *symbol );
    Qt3DRender::QMaterial *constructMaterial( QgsColorRampPointCloud3DSymbol *symbol );
    Qt3DRender::QMaterial *constructMaterial( QgsRGBPointCloud3DSymbol *symbol );

    void makeEntity( Qt3DCore::QEntity *parent, const Qgs3DRenderContext &context, PointData &out, bool selected );

    // input specific for this class
    //std::unique_ptr< QgsPoint3DSymbol > mSymbol;
    // inputs - generic
    //QgsFeatureIds mSelectedIds;

    // outputs
    PointData outNormal;  //!< Features that are not selected
    // PointData outSelected;  //!< Features that are selected

    std::unique_ptr<QgsPointCloud3DSymbol> mSymbol;
};

class QgsPointCloud3DGeometry: public Qt3DRender::QGeometry
{
  public:
    QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, QgsPointCloud3DSymbol *symbol );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data );

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mParameterAttribute = nullptr;
    Qt3DRender::QAttribute *mColorAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    int mVertexCount = 0;

    QgsPointCloud3DSymbol::RenderingStyle mRenderingStyle;
};

/**
 * \ingroup 3d
 * This loader factory is responsible for creation of loaders for individual tiles
 * of QgsQgsPointCloudLayerChunkedEntity whenever a new tile is requested by the entity.
 *
 * \since QGIS 3.18
 */
class QgsPointCloudLayerChunkLoaderFactory : public QgsChunkLoaderFactory
{
  public:
    /*
     * Constructs the factory
     * The factory takes ownership over the passed \a symbol
     */
    QgsPointCloudLayerChunkLoaderFactory( const Qgs3DMapSettings &map, QgsPointCloudIndex *pc, QgsPointCloud3DSymbol *symbol );

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;
    virtual QgsChunkNode *createRootNode() const override;
    virtual QVector<QgsChunkNode *> createChildren( QgsChunkNode *node ) const override;
    const Qgs3DMapSettings &mMap;
    QgsPointCloudIndex *mPointCloudIndex;
    std::unique_ptr< QgsPointCloud3DSymbol > mSymbol;
};


/**
 * \ingroup 3d
 * This loader class is responsible for async loading of data for a single tile
 * of QgsPointCloudLayerChunkedEntity and creation of final 3D entity from the data
 * previously prepared in a worker thread.
 *
 * \since QGIS 3.18
 */
class QgsPointCloudLayerChunkLoader : public QgsChunkLoader
{
  public:

    /**
     * Constructs the loader
     * QgsPointCloudLayerChunkLoader takes ownership over symbol
     */
    QgsPointCloudLayerChunkLoader( const QgsPointCloudLayerChunkLoaderFactory *factory, QgsChunkNode *node, QgsPointCloud3DSymbol *symbol );
    ~QgsPointCloudLayerChunkLoader() override;

    virtual void cancel() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    const QgsPointCloudLayerChunkLoaderFactory *mFactory;
    std::unique_ptr<QgsPointCloud3DSymbolHandler> mHandler;
    Qgs3DRenderContext mContext;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
};


/**
 * \ingroup 3d
 * 3D entity used for rendering of point cloud layers with a single 3D symbol for all points.
 *
 * It is implemented using tiling approach with QgsChunkedEntity. Internally it uses
 * QgsPointCloudLayerChunkLoaderFactory and QgsPointCloudLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for each tile.
 *
 * \since QGIS 3.18
 */
class QgsPointCloudLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    explicit QgsPointCloudLayerChunkedEntity( QgsPointCloudIndex *pc, const Qgs3DMapSettings &map, QgsPointCloud3DSymbol *symbol );

    ~QgsPointCloudLayerChunkedEntity();
};

/// @endcond

#endif // QGSPOINTCLOUDLAYERCHUNKLOADER_P_H
