/***************************************************************************
    qgsmaptooltouch.h  -  map tool for zooming and panning using qgestures
    ----------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco at bernawebdesign.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLTOUCH_H
#define QGSMAPTOOLTOUCH_H

#include "qgsmaptool.h"
#include <QGestureEvent>
#include <QPinchGesture>
#include "qgis_gui.h"

class QgsMapCanvas;


/** \ingroup gui
 * A map tool for panning the map.
 * @see QgsMapTool
 * @note may not be available in Python bindings on all platforms
 */
class GUI_EXPORT QgsMapToolTouch : public QgsMapTool
{
  public:
    //! constructor
    QgsMapToolTouch( QgsMapCanvas* canvas );

    ~QgsMapToolTouch();

    void activate() override;
    void deactivate() override;
    virtual void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    virtual void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;
    virtual Flags flags() const override { return QgsMapTool::Transient | QgsMapTool::AllowZoomRect; }
    bool gestureEvent( QGestureEvent *e ) override;

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate a pinch gesture is taking place
    bool mPinching;
    void pinchTriggered( QPinchGesture *gesture );
};

#endif
