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
 * \note QML Type: MapCanvasMap
 *
 * \sa QgsQuickMapCanvas
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMapCanvasMap : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY( QgsCoordinateReferenceSystem destinationCrs READ destinationCrs WRITE setDestinationCrs NOTIFY destinationCrsChanged )
    Q_PROPERTY( QgsQuickMapSettings *mapSettings READ mapSettings )
    Q_PROPERTY( bool freeze READ freeze WRITE setFreeze NOTIFY freezeChanged )
    Q_PROPERTY( bool isRendering READ isRendering NOTIFY isRenderingChanged )

    /**
     * Interval in milliseconds after which the map canvas will be updated while a rendering job is ongoing.
     * This only has an effect if incrementalRendering is activated.
     * Default is 250 [ms].
     */
    Q_PROPERTY( int mapUpdateInterval READ mapUpdateInterval WRITE setMapUpdateInterval NOTIFY mapUpdateIntervalChanged )
    Q_PROPERTY( bool incrementalRendering READ incrementalRendering WRITE setIncrementalRendering NOTIFY incrementalRenderingChanged )

  public:
    QgsQuickMapCanvasMap( QQuickItem *parent = 0 );
    ~QgsQuickMapCanvasMap();

    QgsPoint toMapCoordinates( QPoint canvasCoordinates );

    QgsQuickMapSettings *mapSettings() const;

    QgsUnitTypes::DistanceUnit mapUnits() const;
    void setMapUnits( const QgsUnitTypes::DistanceUnit &mapUnits );

    QList<QgsMapLayer *> layerSet() const;
    void setLayerSet( const QList<QgsMapLayer *> &layerSet );

    bool hasCrsTransformEnabled() const;
    void setCrsTransformEnabled( bool hasCrsTransformEnabled );

    QgsCoordinateReferenceSystem destinationCrs() const;
    void setDestinationCrs( const QgsCoordinateReferenceSystem &destinationCrs );

    QgsRectangle extent() const;
    void setExtent( const QgsRectangle &extent );

    virtual QSGNode *updatePaintNode( QSGNode *oldNode, QQuickItem::UpdatePaintNodeData * );

    bool freeze() const;
    void setFreeze( bool freeze );

    bool isRendering() const;

    int mapUpdateInterval() const;
    void setMapUpdateInterval( int mapUpdateInterval );

    bool incrementalRendering() const;
    void setIncrementalRendering( bool incrementalRendering );

  signals:
    void renderStarting();

    void mapCanvasRefreshed();

    void extentChanged();

    void destinationCrsChanged();

    void freezeChanged();

    void isRenderingChanged();

    void mapUpdateIntervalChanged();

    void incrementalRenderingChanged();

  protected:
    void geometryChanged( const QRectF &newGeometry, const QRectF &oldGeometry );

  public slots:
    void stopRendering();

    void zoomToFullExtent();

    void zoom( QPointF center, qreal scale );
    void pan( QPointF oldPos, QPointF newPos );
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

    QgsQuickMapSettings *mMapSettings;

    bool mPinching;
    QPoint mPinchStartPoint;
    QgsMapRendererParallelJob *mJob;
    QgsMapRendererCache *mCache;
    QgsLabelingResults *mLabelingResults;
    QImage mImage;
    QgsMapSettings mImageMapSettings;
    QTimer mRefreshTimer;
    bool mDirty;
    bool mFreeze;
    QList<QMetaObject::Connection> mLayerConnections;
    QFutureSynchronizer<void> mZombieJobs;
    QTimer mMapUpdateTimer;
    int mMapUpdateInterval;
    bool mIncrementalRendering;
};

#endif // QGSQUICKMAPCANVASMAP_H
