/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef MAPCOORDSDIALOG_H
#define MAPCOORDSDIALOG_H

#include <QDialog>

#include "qgsmaptoolemitpoint.h"
#include "qgssnapindicator.h"
#include "qgssnappingutils.h"
#include "qgspointxy.h"
#include "qgsmapcanvas.h"
#include "qgspointlocator.h"
#include "qgsprojectionselectionwidget.h"
#include "qgscoordinatereferencesystem.h"

#include "ui_qgsmapcoordsdialogbase.h"

class QgsGCPCanvasItem;

class QPushButton;

class QgsGeorefMapToolEmitPoint : public QgsMapTool
{
    Q_OBJECT

  public:
    explicit QgsGeorefMapToolEmitPoint( QgsMapCanvas *canvas );

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    void canvasPressEvent( QgsMapMouseEvent *e ) override;

    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    void deactivate() override;

  signals:
    void canvasClicked( const QgsPointXY &point, Qt::MouseButton button );
    void mouseReleased();

  private:

    QgsPointLocator::Match mapPointMatch( QMouseEvent *e );

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
};

class QgsMapCoordsDialog : public QDialog, private Ui::QgsMapCoordsDialogBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMapCoordsDialog.
     * \param qgisCanvas
     * \param sourceCoordinates must be in source layer coordinates, NOT pixels (unless source image is completely non-referenced)!
     * \param rasterCrs
     * \param parent
     */
    QgsMapCoordsDialog( QgsMapCanvas *qgisCanvas, const QgsPointXY &sourceCoordinates, QgsCoordinateReferenceSystem &rasterCrs, QWidget *parent = nullptr );
    ~QgsMapCoordsDialog() override;

  private slots:
    void buttonBox_accepted();

    void setToolEmitPoint( bool isEnable );

    void maybeSetXY( const QgsPointXY &, Qt::MouseButton );
    void updateOK();
    void setPrevTool();

  signals:

    /**
     * Emitted when a point should be added through the dialog.
     * \param sourceCoordinate source point, which MUST be in source layer coordinates not pixels
     * \param destinationCoordinate
     * \param destinationCrs
     */
    void pointAdded( const QgsPointXY &sourceCoordinate, const QgsPointXY &destinationCoordinate, const QgsCoordinateReferenceSystem &destinationCrs );

  private:
    double dmsToDD( const QString &dms );

    QPushButton *mPointFromCanvasPushButton = nullptr;

    //QgsProjectionSelectionWidget *mProjSelect = nullptr;

    QgsGeorefMapToolEmitPoint *mToolEmitPoint = nullptr;
    QgsMapTool *mPrevMapTool = nullptr;
    QgsMapCanvas *mQgisCanvas = nullptr;

    QgsGCPCanvasItem *mNewlyAddedPointItem = nullptr;

    QgsCoordinateReferenceSystem mRasterCrs;

    //! Source layer coordinates -- must be in source layer coordinates, not pixels (unless source image is completely non-referenced)
    QgsPointXY mSourceLayerCoordinates;
};

#endif
