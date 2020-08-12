/***************************************************************************
    qgsmaptoolextent.h  -  map tool that emits an extent
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEXTENT_H
#define QGSMAPTOOLEXTENT_H

#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgsrubberband.h"
#include "qgis_gui.h"
#include "qobjectuniqueptr.h"

class QgsMapCanvas;


/**
 * \ingroup gui
 * A map tool that emits an extent from a rectangle drawn onto the map canvas.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsMapToolExtent : public QgsMapTool
{
    Q_OBJECT

  public:

    //! constructor
    QgsMapToolExtent( QgsMapCanvas *canvas );

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void activate() override;
    void deactivate() override;

    /**
     * Sets a fixed aspect ratio to be used when dragging extent onto the canvas.
     * To unset a fixed aspect ratio, set the width and height to zero.
     * \param ratio aspect ratio's width and height
     */
    void setRatio( QSize ratio ) { mRatio = ratio; }

    /**
     * Returns the current fixed aspect ratio to be used when dragging extent onto the canvas.
     * If the aspect ratio isn't fixed, the width and height will be set to zero.
     */
    QSize ratio() const { return mRatio; }

    /**
     * Returns the current extent drawn onto the canvas.
     */
    QgsRectangle extent() const;

  signals:

    //! signal emitted on extent change
    void extentChanged( const QgsRectangle &extent );

  private:

    void calculateEndPoint( QgsPointXY &point );

    void drawExtent();

    QObjectUniquePtr< QgsRubberBand > mRubberBand;

    QgsPointXY mStartPoint;
    QgsPointXY mEndPoint;

    bool mDraw = false;

    QSize mRatio;

};

#endif
