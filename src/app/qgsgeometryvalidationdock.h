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
class QgsVectorLayer;

/**
 * \brief The QgsGeometryValidationDock class
 */
class QgsGeometryValidationDock : public QgsDockWidget, public Ui_QgsGeometryValidationDockBase
{
    Q_OBJECT

  public:
    QgsGeometryValidationDock( const QString &title, QgsMapCanvas *mapCanvas, QgisApp *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    QgsGeometryValidationModel *geometryValidationModel() const;
    void setGeometryValidationModel( QgsGeometryValidationModel *geometryValidationModel );

    QgsGeometryValidationService *geometryValidationService() const;
    void setGeometryValidationService( QgsGeometryValidationService *geometryValidationService );

  private slots:
    void updateCurrentError();
    void onCurrentErrorChanged( const QModelIndex &current, const QModelIndex &previous );
    void updateMapCanvasExtent();
    void onCurrentLayerChanged( QgsMapLayer *layer );
    void onLayerEditingStatusChanged();
    void onLayerDestroyed( QObject *layer );
    void gotoNextError();
    void gotoPreviousError();
    void zoomToProblem();
    void zoomToFeature();
    void onDataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles );
    void onRowsInserted();
    void showErrorContextMenu( const QPoint &pos );

  private:

    enum ZoomToAction
    {
      ZoomToFeature,
      ZoomToProblem
    };

    void showHighlight( const QModelIndex &current );

    QgsCoordinateTransform layerTransform() const;

    ZoomToAction mLastZoomToAction = ZoomToFeature;
    QgsGeometryValidationModel *mGeometryValidationModel = nullptr;
    QgsGeometryValidationService *mGeometryValidationService = nullptr;
    QButtonGroup *mZoomToButtonGroup = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgisApp *mQgisApp = nullptr;
    QModelIndex currentIndex() const;
    QgsRubberBand *mFeatureRubberband = nullptr;
    QgsRubberBand *mErrorRubberband = nullptr;
    QgsRubberBand *mErrorLocationRubberband = nullptr;
    QgsVectorLayer *mCurrentLayer = nullptr;
    bool mPreventZoomToError = false;
    QMenu *mGeometryErrorContextMenu = nullptr;
};

#endif // QGSGEOMETRYVALIDATIONPANEL_H
