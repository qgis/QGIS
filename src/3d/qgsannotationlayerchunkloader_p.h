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

#include "qgschunkloader.h"
#include "qgschunkedentity.h"
#include "qgs3drendercontext.h"
#include "qgsbillboardgeometry.h"
#include "qgstextformat.h"
#include <QImage>

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
 * \brief This loader factory is responsible for creation of loaders of QgsAnnotationLayerChunkedEntity.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkLoaderFactory : public QgsQuadtreeChunkLoaderFactory
{
    Q_OBJECT

  public:
    //! Constructs the factory
    QgsAnnotationLayerChunkLoaderFactory( const Qgs3DRenderContext &context, QgsAnnotationLayer *layer, int leafLevel, Qgis::AltitudeClamping clamping, double zOffset, bool showCallouts, const QColor &calloutLineColor, double calloutLineWidth, const QgsTextFormat &textFormat, double zMin, double zMax );

    //! Creates loader for the given chunk node. Ownership of the returned is passed to the caller.
    virtual QgsChunkLoader *createChunkLoader( QgsChunkNode *node ) const override;

    Qgs3DRenderContext mRenderContext;
    QgsAnnotationLayer *mLayer = nullptr;
    int mLeafLevel = 0;
    Qgis::AltitudeClamping mClamping = Qgis::AltitudeClamping::Relative;
    double mZOffset = 0;
    bool mShowCallouts = false;
    QColor mCalloutLineColor;
    double mCalloutLineWidth = 2;
    QgsTextFormat mTextFormat;
};


/**
 * \ingroup qgis_3d
 * \brief This loader class is responsible for async loading of data for QgsAnnotationLayerChunkedEntity
 * and creation of final 3D entity from the data previously prepared in a worker thread.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkLoader : public QgsChunkLoader
{
    Q_OBJECT

  public:
    //! Constructs the loader
    QgsAnnotationLayerChunkLoader( const QgsAnnotationLayerChunkLoaderFactory *factory, QgsChunkNode *node );
    ~QgsAnnotationLayerChunkLoader() override;

    void start() override;
    virtual void cancel() override;
    virtual Qt3DCore::QEntity *createEntity( Qt3DCore::QEntity *parent ) override;

  private:
    const QgsAnnotationLayerChunkLoaderFactory *mFactory = nullptr;
    Qgs3DRenderContext mRenderContext;
    bool mCanceled = false;
    QFutureWatcher<void> *mFutureWatcher = nullptr;
    QString mLayerName;
    QgsVector3D mChunkOrigin;

    std::vector< std::unique_ptr< QgsAnnotationItem > > mItemsToRender;

    QVector< QgsBillboardGeometry::BillboardAtlasData > mBillboardPositions;
    QVector< QgsBillboardGeometry::BillboardAtlasData > mTextBillboardPositions;
    QVector< QgsLineString > mCalloutLines;
    QImage mBillboardAtlas;
    QImage mTextBillboardAtlas;
    double mZMin = std::numeric_limits< double >::max();
    double mZMax = std::numeric_limits< double >::lowest();
};


/**
 * \ingroup qgis_3d
 * \brief 3D entity used for rendering of annotation layers.
 *
 * Internally it uses QgsAnnotationLayerChunkLoaderFactory and
 * QgsAnnotationLayerChunkLoader to do the actual work
 * of loading and creating 3D sub-entities for the layer.
 *
 * \since QGIS 4.0
 */
class QgsAnnotationLayerChunkedEntity : public QgsChunkedEntity
{
    Q_OBJECT
  public:
    //! Constructs the entity.
    explicit QgsAnnotationLayerChunkedEntity( Qgs3DMapSettings *map, QgsAnnotationLayer *layer, Qgis::AltitudeClamping clamping, double zOffset, bool showCallouts, const QColor &calloutLineColor, double calloutLineWidth, const QgsTextFormat &textFormat, double zMin, double zMax );
    ~QgsAnnotationLayerChunkedEntity();

  private slots:
    void onTerrainElevationOffsetChanged();

  private:
    Qt3DCore::QTransform *mTransform = nullptr;

    bool applyTerrainOffset() const;

    friend class TestQgsChunkedEntity;
};

/// @endcond

#endif // QGSANNOTATIONLAYERCHUNKLOADER_P_H
