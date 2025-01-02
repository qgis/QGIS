/***************************************************************************
    qgsmaptoolpan.h  -  map tool for panning in map canvas
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

#ifndef QGSMAPTOOLPAN_H
#define QGSMAPTOOLPAN_H

#include "qgsmaptool.h"
#include "qgis_gui.h"

class QgsMapCanvas;


/**
 * \ingroup gui
 * \brief A map tool for panning the map.
 * \see QgsMapTool
 */
class GUI_EXPORT QgsMapToolPan : public QgsMapTool
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolPan( QgsMapCanvas *canvas );
    ~QgsMapToolPan() override;

    void activate() override;
    void deactivate() override;

    Flags flags() const override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;
    bool gestureEvent( QGestureEvent *e ) override;

    /**
     * Returns TRUE if a drag operation is in progress.
     *
     * \since QGIS 3.12
     */
    bool isDragging() const { return mDragging; }

  signals:

    /**
     * Emitted whenever the distance or bearing of an in-progress panning
     * operation is changed.
     *
     * This signal will be emitted during a pan operation as the user moves the map,
     * giving the total distance and bearing between the map position at the
     * start of the pan and the current pan position.
     *
     * \since QGIS 3.12
     */
    void panDistanceBearingChanged( double distance, Qgis::DistanceUnit unit, double bearing );

  private:
    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate a pinch gesture is taking place
    bool mPinching = false;

    void pinchTriggered( QPinchGesture *gesture );
};

#endif
