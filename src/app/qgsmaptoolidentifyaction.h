/***************************************************************************
    qgsmaptoolidentify.h  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLIDENTIFYACTION_H
#define QGSMAPTOOLIDENTIFYACTION_H

#include "qgis.h"
#include "qgsmaptoolidentify.h"

#include <QObject>
#include <QPointer>
#include "qgis_app.h"

class QgsIdentifyResultsDialog;
class QgsMapLayer;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsFeatureStore;
class QgsRubberBand;

/**
  \brief Map tool for identifying features layers and showing results

  after selecting a point shows dialog with identification results
  - for raster layers shows value of underlying pixel
  - for vector layers shows feature attributes within search radius
    (allows editing values when vector layer is in editing mode)
*/
class APP_EXPORT QgsMapToolIdentifyAction : public QgsMapToolIdentify
{
    Q_OBJECT

  public:

    QgsMapToolIdentifyAction( QgsMapCanvas *canvas );

    ~QgsMapToolIdentifyAction() override;

    //! Overridden mouse move event
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse press event
    void canvasPressEvent( QgsMapMouseEvent *e ) override;

    //! Overridden mouse release event
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void activate() override;

    void deactivate() override;

    void handleOnCanvasRelease( QgsMapMouseEvent *e );

    void initRubberBand();

  public slots:
    void handleCopyToClipboard( QgsFeatureStore & );
    void handleChangedRasterResults( QList<QgsMapToolIdentify::IdentifyResult> &results );

  signals:

    void copyToClipboard( QgsFeatureStore & );

  private slots:
    void showAttributeTable( QgsMapLayer *layer, const QList<QgsFeature> &featureList );

  private:
    //! Pointer to the identify results dialog for name/value pairs
    QPointer<QgsIdentifyResultsDialog> mResultsDialog;

    QgsIdentifyResultsDialog *resultsDialog();

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    bool mSelectionActive = false;

    std::unique_ptr< QgsRubberBand > mSelectionRubberBand;

    QColor mFillColor;

    QColor mStrokeColor;

    bool mJustFinishedSelection = false;

    //! Center point for the radius
    QgsPointXY mRadiusCenter;

    QPoint mInitDragPos;

    QgsUnitTypes::DistanceUnit displayDistanceUnits() const override;
    QgsUnitTypes::AreaUnit displayAreaUnits() const override;
    void setClickContextScope( const QgsPointXY &point );

    void selectFeaturesMoveEvent( QgsMapMouseEvent *e );
    void selectFeaturesReleaseEvent( QgsMapMouseEvent *e );

    void selectPolygonMoveEvent( QgsMapMouseEvent *e );
    void selectPolygonReleaseEvent( QgsMapMouseEvent *e );

    void selectFreehandMoveEvent( QgsMapMouseEvent *e );
    void selectFreehandReleaseEvent( QgsMapMouseEvent *e );

    void selectRadiusMoveEvent( QgsMapMouseEvent *e );
    void selectRadiusReleaseEvent( QgsMapMouseEvent *e );

    void updateRadiusFromEdge( QgsPointXY &radiusEdge );

    void keyReleaseEvent( QKeyEvent *e ) override;

    friend class TestQgsMapToolIdentifyAction;
};

#endif
