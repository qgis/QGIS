/***************************************************************************
    qgsmeasuretool.h  -  map tool for measuring distances and areas
    ---------------------
    begin                : April 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMEASURETOOL_H
#define QGSMEASURETOOL_H

#include "qgsmaptool.h"
#include "qgscoordinatereferencesystem.h"
#include "qgis_app.h"

class QgsDistanceArea;
class QgsMapCanvas;
class QgsMeasureDialog;
class QgsRubberBand;
class QgsSnapIndicator;


class APP_EXPORT QgsMeasureTool : public QgsMapTool
{
    Q_OBJECT

  public:

    QgsMeasureTool( QgsMapCanvas *canvas, bool measureArea );

    ~QgsMeasureTool() override;

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }

    //! returns whether measuring distance or area
    bool measureArea() const { return mMeasureArea; }

    //! When we have added our last point, and not following
    bool done() const { return mDone; }

    //! Reset and start new
    void restart();

    //! Add new point
    void addPoint( const QgsPointXY &point );

    //! Returns reference to array of the points
    QVector<QgsPointXY> points() const;

    // Inherited from QgsMapTool

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void deactivate() override;
    void keyPressEvent( QKeyEvent *e ) override;

  public slots:
    //! Updates the projections we're using
    void updateSettings();

  protected:

    QVector<QgsPointXY> mPoints;

    QgsMeasureDialog *mDialog = nullptr;

    //! Rubberband widget tracking the lines being drawn
    QgsRubberBand *mRubberBand = nullptr;

    //! Rubberband widget tracking the added nodes to line
    QgsRubberBand *mRubberBandPoints = nullptr;

    //! Indicates whether we're measuring distances or areas
    bool mMeasureArea = false;

    //! Indicates whether we've just done a right mouse click
    bool mDone = true;

    /**
     * Indicates whether we've recently warned the user about having the wrong
     * project projection.
     */
    bool mWrongProjectProjection = false;

    //! Destination CoordinateReferenceSystem used by the MapCanvas
    QgsCoordinateReferenceSystem mDestinationCrs;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

    //! Removes the last vertex from mRubberBand
    void undo();
};

#endif
