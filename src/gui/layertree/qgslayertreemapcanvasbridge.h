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
 * @note added in 2.4
 */
class GUI_EXPORT QgsLayerTreeMapCanvasBridge : public QObject
{
    Q_OBJECT
  public:
    //! Constructor: does not take ownership of the layer tree nor canvas
    QgsLayerTreeMapCanvasBridge( QgsLayerTreeGroup *root, QgsMapCanvas *canvas, QObject *parent = nullptr );

    void clear();

    QgsLayerTreeGroup *rootGroup() const { return mRoot; }
    QgsMapCanvas *mapCanvas() const { return mCanvas; }

    //! Associates overview canvas with the bridge, so the overview will be updated whenever main canvas is updated
    //! @note added in 3.0
    void setOvervewCanvas( QgsMapOverviewCanvas *overviewCanvas ) { mOverviewCanvas = overviewCanvas; }
    //! Returns associated overview canvas (may be null)
    //! @note added in 3.0
    QgsMapOverviewCanvas *overviewCanvas() const { return mOverviewCanvas; }

    bool hasCustomLayerOrder() const { return mHasCustomLayerOrder; }
    QStringList customLayerOrder() const { return mCustomLayerOrder; }

    QStringList defaultLayerOrder() const;

    //! if enabled, will automatically set full canvas extent and destination CRS + map units
    //! when first layer(s) are added
    void setAutoSetupOnFirstLayer( bool enabled ) { mAutoSetupOnFirstLayer = enabled; }
    bool autoSetupOnFirstLayer() const { return mAutoSetupOnFirstLayer; }

  public slots:
    void setHasCustomLayerOrder( bool state );
    void setCustomLayerOrder( const QStringList &order );

    //! force update of canvas layers from the layer tree. Normally this should not be needed to be called.
    void setCanvasLayers();

    void readProject( const QDomDocument &doc );
    void writeProject( QDomDocument &doc );

  signals:
    void hasCustomLayerOrderChanged( bool );
    void customLayerOrderChanged( const QStringList &order );

    /**
     * Emitted when the set of layers (or order of layers) visible in the
     * canvas changes.
     * @note added in QGIS 3.0
     */
    void canvasLayersChanged( const QList< QgsMapLayer * > &layers );

  protected:

    void defaultLayerOrder( QgsLayerTreeNode *node, QStringList &order ) const;

    //! Fill canvasLayers and overviewLayers lists from node and its descendants
    void setCanvasLayers( QgsLayerTreeNode *node, QList<QgsMapLayer *> &canvasLayers, QList<QgsMapLayer *> &overviewLayers,
                          QList<QgsMapLayer *> &allLayers );

    void deferredSetCanvasLayers();

  protected slots:
    void nodeAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void nodeRemovedChildren();
    void nodeVisibilityChanged();
    void nodeCustomPropertyChanged( QgsLayerTreeNode *node, const QString &key );

  private slots:

    void projectLayerOrderChanged();

  protected:
    QgsLayerTreeGroup *mRoot = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapOverviewCanvas *mOverviewCanvas = nullptr;

    bool mPendingCanvasUpdate;

    bool mHasCustomLayerOrder;
    QStringList mCustomLayerOrder;

    bool mAutoSetupOnFirstLayer;

    bool mHasFirstLayer;
    bool mLastLayerCount;
    bool mUpdatingProjectLayerOrder = false;

    QgsCoordinateReferenceSystem mFirstCRS;
};

#endif // QGSLAYERTREEMAPCANVASBRIDGE_H
