/***************************************************************************
                         qgspointcloudlayerrenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYERRENDERER_H
#define QGSPOINTCLOUDLAYERRENDERER_H

#include "qgis_core.h"
#include "qgscolorramp.h"
#include "qgsmaplayerrenderer.h"
#include "qgsreadwritecontext.h"
#include "qgspointcloudindex.h"
#include "qgsgeometry.h"

#include "qgserror.h"
#include "qgspointcloudindex.h"
#include "qgsidentifycontext.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudextentrenderer.h"
#include "qgsmapclippingregion.h"
#include "qgsrasterinterface.h"

#include <QDomElement>
#include <QString>
#include <QPainter>
#include <QElapsedTimer>

class QgsRenderContext;
class QgsPointCloudLayer;
class QgsPointCloudRenderer;
class QgsPointCloudRenderContext;
class QgsPointCloudSubIndex;

#define SIP_NO_FILE

/**
 * \ingroup core
 *
 * \brief Implementation of threaded rendering for point cloud layers.
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 * \note Not available in Python bindings
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudLayerRenderer: public QgsMapLayerRenderer
{
  public:

    //! Ctor
    explicit QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context );
    ~QgsPointCloudLayerRenderer();

    bool render() override;
    Qgis::MapLayerRendererFlags flags() const override;
    bool forceRasterRender() const override;
    void setLayerRenderingTimeHint( int time ) override;

    QgsFeedback *feedback() const override { return mFeedback.get(); }

  private:
    QVector<QgsPointCloudNodeId> traverseTree( const QgsPointCloudIndex &pc, const QgsRenderContext &context, QgsPointCloudNodeId n, double maxErrorPixels, double nodeErrorPixels );
    int renderNodesSync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled );
    int renderNodesAsync( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled );
    int renderNodesSorted( const QVector<QgsPointCloudNodeId> &nodes, QgsPointCloudIndex &pc, QgsPointCloudRenderContext &context, QgsPointCloudRequest &request, bool &canceled, Qgis::PointCloudDrawOrder order );
    void renderTriangulatedSurface( QgsPointCloudRenderContext &context );
    bool renderIndex( QgsPointCloudIndex &pc );

    QgsPointCloudLayer *mLayer = nullptr;
    QString mLayerName;

    std::unique_ptr< QgsPointCloudRenderer > mRenderer;
    std::unique_ptr< QgsPointCloudExtentRenderer > mSubIndexExtentRenderer;

    QgsVector3D mScale;
    QgsVector3D mOffset;
    double mZOffset = 0;
    double mZScale = 1.0;

    QgsPointCloudAttributeCollection mLayerAttributes;
    QgsPointCloudAttributeCollection mAttributes;
    QgsGeometry mCloudExtent;
    QList< QgsMapClippingRegion > mClippingRegions;
    const QVector< QgsPointCloudSubIndex > mSubIndexes;

    int mRenderTimeHint = 0;
    bool mBlockRenderUpdates = false;
    QElapsedTimer mElapsedTimer;

    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    bool mEnableProfile = false;
    quint64 mPreparationTime = 0;
};

#endif // QGSPOINTCLOUDLAYERRENDERER_H
