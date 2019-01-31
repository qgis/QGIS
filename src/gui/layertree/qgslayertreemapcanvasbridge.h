/***************************************************************************
  qgslayertreemapcanvasbridge.h
  --------------------------------------
  Date                 : May 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEMAPCANVASBRIDGE_H
#define QGSLAYERTREEMAPCANVASBRIDGE_H

#include <QObject>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMapLayer;
class QgsMapOverviewCanvas;
class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class QgsLayerTree;

/**
 * \ingroup gui
 * The QgsLayerTreeMapCanvasBridge class takes care of updates of layer set
 * for QgsMapCanvas from a layer tree. The class listens to the updates in the layer tree
 * and updates the list of layers for rendering whenever some layers are added, removed,
 * or their visibility changes.
 *
 * The update of layers is not done immediately - it is postponed, so a series of updates
 * to the layer tree will trigger just one update of canvas layers.
 *
 * Also allows the client to override the default order of layers. This is useful
 * in advanced cases where the grouping in layer tree should be independent from the actual
 * order in the canvas.
 *
 * \since QGIS 2.4
 */
class GUI_EXPORT QgsLayerTreeMapCanvasBridge : public QObject
{
    Q_OBJECT
  public:
    //! Constructor: does not take ownership of the layer tree nor canvas
    QgsLayerTreeMapCanvasBridge( QgsLayerTree *root, QgsMapCanvas *canvas, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QgsLayerTree *rootGroup() const { return mRoot; }
    QgsMapCanvas *mapCanvas() const { return mCanvas; }

    /**
     * Associates overview canvas with the bridge, so the overview will be updated whenever main canvas is updated
     * \since QGIS 3.6
     */
    void setOverviewCanvas( QgsMapOverviewCanvas *overviewCanvas ) { mOverviewCanvas = overviewCanvas; }

#ifdef SIP_RUN

    /**
     * Associates overview canvas with the bridge, so the overview will be updated whenever main canvas is updated
     * \since QGIS 3.0
     * \deprecated use setOverviewCanvas instead
     */
    void setOvervewCanvas( QgsMapOverviewCanvas *overviewCanvas ) SIP_DEPRECATED; // TODO QGIS 4.0 remove
    % MethodCode
    sipCpp->setOverviewCanvas( a0 );
    % End
#endif

    /**
     * Returns associated overview canvas (may be null)
     * \since QGIS 3.0
     */
    QgsMapOverviewCanvas *overviewCanvas() const { return mOverviewCanvas; }

    /**
     * if enabled, will automatically set full canvas extent and destination CRS + map units
     * when first layer(s) are added
     */
    void setAutoSetupOnFirstLayer( bool enabled ) { mAutoSetupOnFirstLayer = enabled; }
    bool autoSetupOnFirstLayer() const { return mAutoSetupOnFirstLayer; }

    //! force update of canvas layers from the layer tree. Normally this should not be needed to be called.
    Q_INVOKABLE void setCanvasLayers();

  signals:

    /**
     * Emitted when the set of layers (or order of layers) visible in the
     * canvas changes.
     * \since QGIS 3.0
     */
    void canvasLayersChanged( const QList< QgsMapLayer * > &layers );

  private slots:
    void nodeVisibilityChanged();
    void nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );

  private:
    //! Fill canvasLayers and overviewLayers lists from node and its descendants
    void setCanvasLayers( QgsLayerTreeNode *node, QList<QgsMapLayer *> &canvasLayers, QList<QgsMapLayer *> &overviewLayers,
                          QList<QgsMapLayer *> &allLayers );

    void deferredSetCanvasLayers();

    QgsLayerTree *mRoot = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapOverviewCanvas *mOverviewCanvas = nullptr;

    bool mPendingCanvasUpdate;

    bool mAutoSetupOnFirstLayer;

    bool mHasFirstLayer;
    bool mHasLayersLoaded;
    bool mUpdatingProjectLayerOrder = false;

    QgsCoordinateReferenceSystem mFirstCRS;
};

#endif // QGSLAYERTREEMAPCANVASBRIDGE_H
