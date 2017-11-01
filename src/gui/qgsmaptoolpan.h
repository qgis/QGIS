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
 * A map tool for panning the map.
 * \see QgsMapTool
 */
class GUI_EXPORT QgsMapToolPan : public QgsMapTool
{
    Q_OBJECT

  public:
    //! constructor
    QgsMapToolPan( QgsMapCanvas *canvas );
    ~QgsMapToolPan();

    void activate() override;
    void deactivate() override;

    virtual Flags flags() const override { return QgsMapTool::Transient | QgsMapTool::AllowZoomRect; }
    virtual void canvasPressEvent( QgsMapMouseEvent *e ) override;
    virtual void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    virtual void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;
    bool gestureEvent( QGestureEvent *e ) override;

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate a pinch gesture is taking place
    bool mPinching = false;

    void pinchTriggered( QPinchGesture *gesture );
};

#endif
