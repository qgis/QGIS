/***************************************************************************
  qgsannotationlayerchunkloader_p.h
  --------------------------------------
  Date                 : September 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSANNOTATIONLAYERCHUNKLOADER_P_H
#define QGSANNOTATIONLAYERCHUNKLOADER_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <functional>
#include <limits>
#include <memory>
#include <vector>

#include "qgs3drendercontext.h"
#include "qgsabstractfeaturebasedchunkedentity.h"
#include "qgsbillboardgeometry.h"
#include "qgschunkloader.h"
#include "qgscoordinatetransform.h"
#include "qgslinestring.h"
#include "qgstextformat.h"
#include "qgsvector3d.h"

#include <QImage>
#include <QPromise>
#include <QSize>
#include <QVector3D>

#define SIP_NO_FILE

class QgsAnnotationLayer;
class QgsAnnotationItem;

namespace Qt3DCore
{
  class QTransform;
}

#include <QFutureWatcher>


/**
 * \ingroup qgis_3d
 * \brief This loader is responsible for creation of QgsAnnotationLayerChunkedEntity chunks.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkLoader : public QgsQuadtreeChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader
    QgsAnnotationLayerChunkLoader(
      const Qgs3DRenderContext &context,
      QgsAnnotationLayer *layer,
      int leafLevel,
      Qgis::AltitudeClamping clamping,
      double zOffset,
      bool showCallouts,
      const QColor &calloutLineColor,
      double calloutLineWidth,
      const QgsTextFormat &textFormat,
      double zMin,
      double zMax
    );

    QFuture<QgsChunkLoaderResult> loadChunk( QgsChunkNode *node ) override;

    Qgs3DRenderContext mRenderContext;
    QgsAnnotationLayer *mLayer = nullptr;
    int mLeafLevel = 0;

    //! Settings for the loader, copied by each worker thread loading a chunk.
    struct LoaderData
    {
        Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Relative;
        double mZOffset = 0;
        bool mShowCallouts = false;
        QColor mCalloutLineColor;
        double mCalloutLineWidth = 2;
        QgsTextFormat mTextFormat;
    };

    // Each worker thread loading a chunk copies this data.
    LoaderData mData;

  private:
    //! A set of picture billboards sharing the same texture.
    struct PictureBillboards
    {
        QImage image;
        QVector< QVector3D > positions;
        QVector< QSizeF > sizes;
        Qgis::BillboardScaleMode scaleMode = Qgis::BillboardScaleMode::ViewIndependent;
    };

    //! Everything a worker thread loads for a chunk and is later used for entity creation.
    struct ChunkData
    {
        QgsChunkNode *node = nullptr;
        QString layerName;
        QgsVector3D chunkOrigin;
        Qgs3DRenderContext renderContext;
        QVector< QgsBillboardGeometry::BillboardAtlasData > billboardPositions;
        QVector< QgsBillboardGeometry::BillboardAtlasData > textBillboardPositions;
        QImage billboardAtlas;
        QImage textBillboardAtlas;
        QVector< PictureBillboards > pictureBillboards;
        QVector< QgsLineString > calloutLines;
        double zMin = std::numeric_limits< double >::max();
        double zMax = std::numeric_limits< double >::lowest();
    };

    static void loadChunkInWorker(
      QPromise<ChunkData> &promise,
      QgsChunkNode *node,
      const QgsRectangle &rect,
      const QgsCoordinateTransform &layerToMapTransform,
      const QString &layerName,
      const std::vector< std::unique_ptr< QgsAnnotationItem > > &itemsToRender,
      const QgsVector3D &chunkOrigin,
      const Qgs3DRenderContext &renderCtx,
      const LoaderData &data
    );

    Qt3DCore::QEntity *createEntity( const ChunkData &chunkData, Qt3DCore::QEntity *parent );
};

/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of annotation layers.
 *
 * Internally it uses QgsAnnotationLayerChunkLoader and
 * QgsAnnotationLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for the layer.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkedEntity : public QgsAbstractFeatureBasedChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity.
    explicit QgsAnnotationLayerChunkedEntity(
      Qgs3DMapSettings *map,
      QgsAnnotationLayer *layer,
      Qgis::AltitudeClamping clamping,
      double zOffset,
      bool showCallouts,
      const QColor &calloutLineColor,
      double calloutLineWidth,
      const QgsTextFormat &textFormat,
      double zMin,
      double zMax
    );
    ~QgsAnnotationLayerChunkedEntity() override;

    QList<QgsRayCastHit> rayIntersection( const QgsRay3D &ray, const QgsRayCastContext &context ) const override;

  private:
    bool applyTerrainOffset() const override;
};

/// @endcond

#endif // QGSANNOTATIONLAYERCHUNKLOADER_P_H
