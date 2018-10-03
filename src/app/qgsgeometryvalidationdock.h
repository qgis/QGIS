/***************************************************************************
                      qgsgeometryvalidationdock.h
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYVALIDATIONPANEL_H
#define QGSGEOMETRYVALIDATIONPANEL_H

#include "ui_qgsgeometryvalidationdockbase.h"
#include "qgsdockwidget.h"
#include "qgscoordinatetransform.h"

class QgsMapCanvas;
class QgsGeometryValidationModel;
class QgsGeometryValidationService;
class QgsRubberBand;
class QgisApp;

/**
 * @brief The QgsGeometryValidationDock class
 */
class QgsGeometryValidationDock : public QgsDockWidget, public Ui_QgsGeometryValidationDockBase
{
    Q_OBJECT

  public:
    QgsGeometryValidationDock( const QString &title, QgsMapCanvas *mapCanvas, QgisApp *parent = nullptr, Qt::WindowFlags flags = nullptr );

    QgsGeometryValidationModel *geometryValidationModel() const;
    void setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel );

    QgsGeometryValidationService *geometryValidationService() const;
    void setGeometryValidationService( QgsGeometryValidationService *geometryValidationService );

  private slots:
    void updateCurrentError();
    void onCurrentErrorChanged( const QModelIndex &current, const QModelIndex &previous );
    void onCurrentLayerChanged( QgsMapLayer *layer );
    void gotoNextError();
    void gotoPreviousError();
    void zoomToProblem();
    void zoomToFeature();
    void triggerTopologyChecks();
    void updateLayerTransform();
    void onRowsInserted();

  private:

    enum ZoomToAction
    {
      ZoomToFeature,
      ZoomToProblem
    };

    void showHighlight( const QModelIndex &current );

    ZoomToAction mLastZoomToAction = ZoomToFeature;
    QgsGeometryValidationModel *mGeometryValidationModel = nullptr;
    QgsGeometryValidationService *mGeometryValidationService = nullptr;
    QButtonGroup *mZoomToButtonGroup = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgisApp *mApp = nullptr;
    QgsCoordinateTransform mLayerTransform;
    QModelIndex currentIndex() const;
    QgsRubberBand *mFeatureRubberband = nullptr;
    QgsRubberBand *mErrorRubberband = nullptr;
    QgsRubberBand *mErrorLocationRubberband = nullptr;
};

#endif // QGSGEOMETRYVALIDATIONPANEL_H
