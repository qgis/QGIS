/***************************************************************************
  qgsquickmapcanvasmap.h
  --------------------------------------
  Date                 : 10.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKMAPCANVASMAP_H
#define QGSQUICKMAPCANVASMAP_H

#include <QtQuick/QQuickItem>
#include <QFutureSynchronizer>
#include <QTimer>

#include "qgsmapsettings.h"
#include "qgspoint.h"

#include "qgis_quick.h"
#include "qgsquickmapsettings.h"

class QgsMapRendererParallelJob;
class QgsMapRendererCache;
class QgsLabelingResults;

/**
 * \ingroup quick
 * This class implements a visual Qt Quick Item that does map rendering
 * according to the current map settings. Client code is expected to use
 * MapCanvas item rather than using this class directly.
 *
 * QgsQuickMapCanvasMap instance internally creates QgsQuickMapSettings in
 * constructor. The map settings for other QgsQuick components should
 * be initialized from  QgsQuickMapCanvasMap's mapSettings
 *
 * \note QML Type: MapCanvasMap
 *
 * \sa QgsQuickMapCanvas
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMapCanvasMap : public QQuickItem
{
    Q_OBJECT

    //! map settings. The map settings should be source of map settings for all other components
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings )
    //! do not refresh canvas
    Q_PROPERTY( bool freeze READ freeze WRITE setFreeze NOTIFY freezeChanged )
    //! QgsMapRendererParallelJob is running
    Q_PROPERTY( bool isRendering READ isRendering NOTIFY isRenderingChanged )

    /**
     * Interval in milliseconds after which the map canvas will be updated while a rendering job is ongoing.
     * This only has an effect if incrementalRendering is activated.
     * Default is 250 [ms].
     */
    Q_PROPERTY( int mapUpdateInterval READ mapUpdateInterval WRITE setMapUpdateInterval NOTIFY mapUpdateIntervalChanged )
    //! allow increamental rendering - automatic refresh of map canvas while a rendering job is ongoing
    Q_PROPERTY( bool incrementalRendering READ incrementalRendering WRITE setIncrementalRendering NOTIFY incrementalRenderingChanged )

  public:
    //! create map canvas map
    QgsQuickMapCanvasMap( QQuickItem *parent = nullptr );
    ~QgsQuickMapCanvasMap() = default;

    //! Return map settings associated
    QgsQuickMapSettings *mapSettings() const;

    //! Return map canvas for Qt Quick scene graph changes
    virtual QSGNode *updatePaintNode( QSGNode *oldNode, QQuickItem::UpdatePaintNodeData * );

    //! Return whether canvas refresh is frozen
    bool freeze() const;

    //! Freeze canvas refresh
    void setFreeze( bool freeze );

    //! Whether renderer job is running
    bool isRendering() const;

    //! Return current map update interval for incremental rendering
    int mapUpdateInterval() const;

    //! Set map update interval for incremental rendering
    void setMapUpdateInterval( int mapUpdateInterval );

    //! Return whether incremental rendering is on
    bool incrementalRendering() const;
    //! Set incremental rendering
    void setIncrementalRendering( bool incrementalRendering );

  signals:
    //! rendering starting
    void renderStarting();

    //! canvas refreshed
    void mapCanvasRefreshed();

    //! freeze changed
    void freezeChanged();

    //! is rendering changed
    void isRenderingChanged();

    //! map update interval for incremental rendering changed
    void mapUpdateIntervalChanged();

    //! incremental rendering changed
    void incrementalRenderingChanged();

  protected:
    //! geometry changed
    void geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry );

  public slots:
    //! stop rendering
    void stopRendering();

    //! Zoom the map by adjusting map settings extent
    void zoom( QPointF center, qreal scale );

    //! Pan the map
    void pan( QPointF oldPos, QPointF newPos );

    //! Refresh the map
    void refresh();

  private slots:
    void refreshMap();
    void renderJobUpdated();
    void renderJobFinished();
    void onWindowChanged( QQuickWindow *window );
    void onScreenChanged( QScreen *screen );
    void onExtentChanged();
    void onLayersChanged();

  private:

    /**
     * Should only be called ba stopRendering()!
     */
    void destroyJob( QgsMapRendererJob *job );
    QgsMapSettings prepareMapSettings() const;
    void updateTransform();
    void zoomToFullExtent();

    QgsQuickMapSettings *mMapSettings;

    bool mPinching = false;
    QPoint mPinchStartPoint;
    QgsMapRendererParallelJob *mJob = nullptr;
    QgsMapRendererCache *mCache = nullptr;
    QgsLabelingResults *mLabelingResults = nullptr;
    QImage mImage;
    QgsMapSettings mImageMapSettings;
    QTimer mRefreshTimer;
    bool mDirty = false;
    bool mFreeze = false;
    QList<QMetaObject::Connection> mLayerConnections;
    QTimer mMapUpdateTimer;
    bool mIncrementalRendering = false;
};

#endif // QGSQUICKMAPCANVASMAP_H
