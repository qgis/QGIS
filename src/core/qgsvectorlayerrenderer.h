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

#define SIP_NO_FILE

#include <QList>
#include <QPainter>

typedef QList<int> QgsAttributeList;

#include "qgis.h"
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
 * Interruption checker used by QgsVectorLayerRenderer::render()
 * \note not available in Python bindings
 */
class QgsVectorLayerRendererInterruptionChecker: public QgsFeedback
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsVectorLayerRendererInterruptionChecker( const QgsRenderContext &context );

  private:
    const QgsRenderContext &mContext;
    QTimer *mTimer = nullptr;
};

/**
 * \ingroup core
 * Implementation of threaded rendering for vector layers.
 *
 * \note not available in Python bindings
 * \since QGIS 2.4
 */
class QgsVectorLayerRenderer : public QgsMapLayerRenderer
{
  public:
    QgsVectorLayerRenderer( QgsVectorLayer *layer, QgsRenderContext &context );
    ~QgsVectorLayerRenderer() override;

    bool render() override;

  private:

    /**
     * Registers label and diagram layer
      \param layer diagram layer
      \param attributeNames attributes needed for labeling and diagrams will be added to the list
     */
    void prepareLabeling( QgsVectorLayer *layer, QSet<QString> &attributeNames );
    void prepareDiagrams( QgsVectorLayer *layer, QSet<QString> &attributeNames );

    /**
     * Draw layer with renderer V2. QgsFeatureRenderer::startRender() needs to be called before using this method
     */
    void drawRenderer( QgsFeatureIterator &fit );

    /**
     * Draw layer with renderer V2 using symbol levels. QgsFeatureRenderer::startRender() needs to be called before using this method
     */
    void drawRendererLevels( QgsFeatureIterator &fit );

    //! Stop version 2 renderer and selected renderer (if required)
    void stopRenderer( QgsSingleSymbolRenderer *selRenderer );


  protected:

    QgsRenderContext &mContext;

    QgsVectorLayerRendererInterruptionChecker mInterruptionChecker;

    //! The rendered layer
    QgsVectorLayer *mLayer = nullptr;

    QgsFields mFields; // TODO: use fields from mSource

    QgsFeatureIds mSelectedFeatureIds;

    QgsVectorLayerFeatureSource *mSource = nullptr;

    QgsFeatureRenderer *mRenderer = nullptr;

    bool mDrawVertexMarkers;
    bool mVertexMarkerOnlyForSelection;
    int mVertexMarkerStyle, mVertexMarkerSize;

    QgsWkbTypes::GeometryType mGeometryType;

    QSet<QString> mAttrNames;

    //! used with old labeling engine (QgsPalLabeling): whether labeling is enabled
    bool mLabeling;
    //! used with new labeling engine (QgsPalLabeling): whether diagrams are enabled
    bool mDiagrams;

    /**
     * used with new labeling engine (QgsLabelingEngine): provider for labels.
     * may be null. no need to delete: if exists it is owned by labeling engine
     */
    QgsVectorLayerLabelProvider *mLabelProvider = nullptr;

    /**
     * used with new labeling engine (QgsLabelingEngine): provider for diagrams.
     * may be null. no need to delete: if exists it is owned by labeling engine
     */
    QgsVectorLayerDiagramProvider *mDiagramProvider = nullptr;

    QPainter::CompositionMode mFeatureBlendMode;

    QgsVectorSimplifyMethod mSimplifyMethod;
    bool mSimplifyGeometry;
};


#endif // QGSVECTORLAYERRENDERER_H
