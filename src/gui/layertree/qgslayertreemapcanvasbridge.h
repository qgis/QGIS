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

class QgsMapCanvas;
class QgsMapCanvasLayer;
class QgsLayerTreeGroup;
class QgsLayerTreeNode;

class GUI_EXPORT QgsLayerTreeMapCanvasBridge : public QObject
{
    Q_OBJECT
  public:
    QgsLayerTreeMapCanvasBridge( QgsLayerTreeGroup* root, QgsMapCanvas* canvas, QObject* parent = 0 );

    void clear();

    QgsLayerTreeGroup* rootGroup() const { return mRoot; }
    QgsMapCanvas* mapCanvas() const { return mCanvas; }

    bool hasCustomLayerOrder() const { return mHasCustomLayerOrder; }
    QStringList customLayerOrder() const { return mCustomLayerOrder; }

    QStringList defaultLayerOrder() const;

    //! if enabled, will automatically set full canvas extent and destination CRS + map units
    //! when first layer(s) are added
    void setAutoSetupOnFirstLayer( bool enabled ) { mAutoSetupOnFirstLayer = enabled; }
    bool autoSetupOnFirstLayer() const { return mAutoSetupOnFirstLayer; }

    //! if enabled, will automatically turn on on-the-fly reprojection of layers if a layer
    //! with different source CRS is added
    void setAutoEnableCrsTransform( bool enabled ) { mAutoEnableCrsTransform = enabled; }
    bool autoEnableCrsTransform() const { return mAutoEnableCrsTransform; }

  public slots:
    void setHasCustomLayerOrder( bool override );
    void setCustomLayerOrder( const QStringList& order );

    //! force update of canvas layers from the layer tree. Normally this should not be needed to be called.
    void setCanvasLayers();

    void readProject( const QDomDocument& doc );
    void writeProject( QDomDocument& doc );

  signals:
    void hasCustomLayerOrderChanged( bool );
    void customLayerOrderChanged( const QStringList& order );

  protected:

    void defaultLayerOrder( QgsLayerTreeNode* node, QStringList& order ) const;

    void setCanvasLayers( QgsLayerTreeNode* node, QList<QgsMapCanvasLayer>& layers );

    void deferredSetCanvasLayers();

  protected slots:
    void nodeAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void nodeRemovedChildren();
    void nodeVisibilityChanged();
    void nodeCustomPropertyChanged( QgsLayerTreeNode* node, QString key );

  protected:
    QgsLayerTreeGroup* mRoot;
    QgsMapCanvas* mCanvas;

    bool mPendingCanvasUpdate;

    bool mHasCustomLayerOrder;
    QStringList mCustomLayerOrder;

    bool mAutoSetupOnFirstLayer;
    bool mAutoEnableCrsTransform;

    bool mHasFirstLayer;
    bool mLastLayerCount;
    QgsCoordinateReferenceSystem mFirstCRS;
};

#endif // QGSLAYERTREEMAPCANVASBRIDGE_H
