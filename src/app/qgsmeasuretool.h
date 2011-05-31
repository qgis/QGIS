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
#include "qgsmapcanvassnapper.h"

class QgsDistanceArea;
class QgsMapCanvas;
class QgsMeasureDialog;
class QgsRubberBand;



class QgsMeasureTool : public QgsMapTool
{
    Q_OBJECT

  public:

    QgsMeasureTool( QgsMapCanvas* canvas, bool measureArea );

    ~QgsMeasureTool();

    //! returns whether measuring distance or area
    bool measureArea() { return mMeasureArea; }

    //! Reset and start new
    void restart();

    //! Add new point
    void addPoint( QgsPoint &point );

    //! Returns reference to array of the points
    const QList<QgsPoint>& points();

    // Inherited from QgsMapTool

    //! Mouse move event for overriding
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Mouse press event for overriding
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Mouse release event for overriding
    virtual void canvasReleaseEvent( QMouseEvent * e );

    //! called when set as currently active map tool
    virtual void activate();

    //! called when map tool is being deactivated
    virtual void deactivate();

  public slots:
    //! updates the projections we're using
    void updateSettings();

  protected:

    QList<QgsPoint> mPoints;

    QgsMeasureDialog* mDialog;

    //! Rubberband widget tracking the lines being drawn
    QgsRubberBand *mRubberBand;

    //! indicates whether we're measuring distances or areas
    bool mMeasureArea;

    //! indicates whether we've just done a right mouse click
    bool mRightMouseClicked;

    //! indicates whether we've recently warned the user about having the wrong
    // project projection
    bool mWrongProjectProjection;

    QgsMapCanvasSnapper mSnapper;

    //! Returns the snapped (map) coordinate
    //@param p (pixel) coordinate
    QgsPoint snapPoint( const QPoint& p );

};

#endif
