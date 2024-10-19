/***************************************************************************
  qgsvectortilelayerrenderer.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILELAYERRENDERER_H
#define QGSVECTORTILELAYERRENDERER_H

#define SIP_NO_FILE

#include "qgsmaplayerrenderer.h"
#include "qgsvectortilerenderer.h"
#include "qgsmapclippingregion.h"
#include "qgsvectortilematrixset.h"

class QgsVectorTileLayer;
class QgsVectorTileRawData;
class QgsVectorTileLabelProvider;
class QgsVectorTileDataProvider;


/**
 * \ingroup core
 * \brief This class provides map rendering functionality for vector tile layers.
 * In render() function (assumed to be run in a worker thread) it will:
 *
 * - fetch vector tiles using QgsVectorTileLoader
 * - decode raw tiles into QgsFeature objects using QgsVectorTileDecoder
 * - render tiles using a class derived from QgsVectorTileRenderer
 *
 * \since QGIS 3.14
 */
class QgsVectorTileLayerRenderer : public QgsMapLayerRenderer
{
  public:
    //! Creates the renderer. Always called from main thread, should copy whatever necessary from the layer
    QgsVectorTileLayerRenderer( QgsVectorTileLayer *layer, QgsRenderContext &context );
    ~QgsVectorTileLayerRenderer() override;

    virtual bool render() override;
    virtual QgsFeedback *feedback() const override { return mFeedback.get(); }
    bool forceRasterRender() const override;

  private:
    void decodeAndDrawTile( const QgsVectorTileRawData &rawTile );

    // data coming from the vector tile layer

    QString mLayerName;

    std::unique_ptr< QgsVectorTileDataProvider > mDataProvider;

    //! Tile renderer object to do rendering of individual tiles
    std::unique_ptr<QgsVectorTileRenderer> mRenderer;

    QPainter::CompositionMode mLayerBlendMode = QPainter::CompositionMode::CompositionMode_SourceOver;

    /**
     * Label provider that handles registration of labels.
     * No need to delete: if exists it is owned by labeling engine.
     */
    QgsVectorTileLabelProvider *mLabelProvider = nullptr;

    //! Whether to draw boundaries of tiles (useful for debugging)
    bool mDrawTileBoundaries = false;

    //! True if labels are enabled
    bool mLabelsEnabled = true;

    // temporary data used during rendering process

    //! Feedback object that may be used by the caller to cancel the rendering
    std::unique_ptr<QgsFeedback> mFeedback;
    //! Zoom level used to fetch tiles
    int mTileZoomToFetch = 0;
    //! Zoom level at which we will be rendering
    int mTileZoomToRender = 0;

    //! Definition of the tile matrix for our zoom level
    QgsTileMatrix mTileMatrix;
    //!< Block of tiles we will be rendering in that zoom level
    QgsTileRange mTileRange;
    //! Cached QgsFields object for each sub-layer that will be rendered
    QMap<QString, QgsFields> mPerLayerFields;

    //! Cached list of layers required for renderer and labeling
    QSet< QString > mRequiredLayers;

    //! Selected features, to draw on top in a selected style
    QList< QgsFeature > mSelectedFeatures;

    //! Counter of total elapsed time to decode tiles (ms)
    int mTotalDecodeTime = 0;
    //! Counter of total elapsed time to render tiles (ms)
    int mTotalDrawTime = 0;

    QList< QgsMapClippingRegion > mClippingRegions;
    double mLayerOpacity = 1.0;

    QgsVectorTileMatrixSet mTileMatrixSet;

    bool mEnableProfile = false;
    quint64 mPreparationTime = 0;

};


#endif // QGSVECTORTILELAYERRENDERER_H
