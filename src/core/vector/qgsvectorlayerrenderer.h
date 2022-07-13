/***************************************************************************
  qgsvectorlayerrenderer.h
  --------------------------------------
  Date                 : December 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERRENDERER_H
#define QGSVECTORLAYERRENDERER_H

class QgsFeatureRenderer;
class QgsRenderContext;
class QgsVectorLayer;
class QgsVectorLayerFeatureSource;

class QgsDiagramRenderer;
class QgsDiagramLayerSettings;

class QgsFeatureIterator;
class QgsSingleSymbolRenderer;
class QgsMapClippingRegion;

#define SIP_NO_FILE

#include <QList>
#include <QPainter>
#include <QElapsedTimer>

typedef QList<int> QgsAttributeList;

#include "qgis_sip.h"
#include "qgsfields.h"  // QgsFields
#include "qgsfeatureiterator.h"
#include "qgsvectorsimplifymethod.h"
#include "qgsfeedback.h"
#include "qgsfeatureid.h"

#include "qgsmaplayerrenderer.h"

class QgsVectorLayerLabelProvider;
class QgsVectorLayerDiagramProvider;

/**
 * \ingroup core
 * \brief Implementation of threaded rendering for vector layers.
 *
 * \note not available in Python bindings
 * \since QGIS 2.4
 */
class QgsVectorLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsVectorLayerRenderer( QgsVectorLayer *layer, QgsRenderContext &context );
    ~QgsVectorLayerRenderer() override;
    QgsFeedback *feedback() const override;
    bool forceRasterRender() const override;

    /**
     * Returns the feature renderer.
     * This may be used for tweaking it before the actual rendering of the layer.
     * \since QGIS 3.12
     */
    QgsFeatureRenderer *featureRenderer() SIP_SKIP { return mRenderer; }

    bool render() override;

    void setLayerRenderingTimeHint( int time ) override;

  private:

    /**
     * Registers label and diagram layer
     * \param layer diagram layer
     * \param attributeNames attributes needed for labeling and diagrams will be added to the list
     */
    void prepareLabeling( QgsVectorLayer *layer, QSet<QString> &attributeNames );
    void prepareDiagrams( QgsVectorLayer *layer, QSet<QString> &attributeNames );

    /**
     * Draw layer with \a renderer. QgsFeatureRenderer::startRender() needs to be called before using this method
     */
    void drawRenderer( QgsFeatureRenderer *renderer, QgsFeatureIterator &fit );

    /**
     * Draw layer with \a renderer using symbol levels. QgsFeatureRenderer::startRender() needs to be called before using this method
     */
    void drawRendererLevels( QgsFeatureRenderer *renderer, QgsFeatureIterator &fit );

    //! Stop version 2 renderer and selected renderer (if required)
    void stopRenderer( QgsFeatureRenderer *renderer, QgsSingleSymbolRenderer *selRenderer );


    bool renderInternal( QgsFeatureRenderer *renderer );

  private:

    std::unique_ptr<QgsFeedback> mFeedback = nullptr;

    //! The rendered layer
    QgsVectorLayer *mLayer = nullptr;

    QgsFields mFields; // TODO: use fields from mSource

    QgsFeatureIds mSelectedFeatureIds;

    QString mTemporalFilter;

    std::unique_ptr< QgsVectorLayerFeatureSource > mSource;

    QgsFeatureRenderer *mRenderer = nullptr;
    std::vector< std::unique_ptr< QgsFeatureRenderer> > mRenderers;

    bool mDrawVertexMarkers;
    bool mVertexMarkerOnlyForSelection;
    Qgis::VertexMarkerType mVertexMarkerStyle = Qgis::VertexMarkerType::SemiTransparentCircle;
    double mVertexMarkerSize = 2.0;

    QgsWkbTypes::GeometryType mGeometryType;

    QSet<QString> mAttrNames;

    /**
     * used with new labeling engine (QgsLabelingEngine): provider for labels.
     * may be NULLPTR. no need to delete: if exists it is owned by labeling engine
     */
    QgsVectorLayerLabelProvider *mLabelProvider = nullptr;

    /**
     * used with new labeling engine (QgsLabelingEngine): provider for diagrams.
     * may be NULLPTR. no need to delete: if exists it is owned by labeling engine
     */
    QgsVectorLayerDiagramProvider *mDiagramProvider = nullptr;

    QPainter::CompositionMode mFeatureBlendMode;

    QgsVectorSimplifyMethod mSimplifyMethod;
    bool mSimplifyGeometry;

    QList< QgsMapClippingRegion > mClippingRegions;
    QgsGeometry mClipFilterGeom;
    bool mApplyClipFilter = false;
    QgsGeometry mClipFeatureGeom;
    bool mApplyClipGeometries = false;
    QgsGeometry mLabelClipFeatureGeom;
    bool mApplyLabelClipGeometries = false;
    bool mForceRasterRender = false;

    int mRenderTimeHint = 0;
    bool mBlockRenderUpdates = false;
    QElapsedTimer mElapsedTimer;

    bool mNoSetLayerExpressionContext = false;

};


#endif // QGSVECTORLAYERRENDERER_H
