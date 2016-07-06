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

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    //! Overridden Mouse double click event.
    virtual void canvasDoubleClickEvent( QgsMapMouseEvent* e ) override;

    Q_DECL_DEPRECATED virtual bool isTransient() const override { return true; }

    bool gestureEvent( QGestureEvent *event ) override;

  private:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate a pinch gesture is taking place
    bool mPinching;
    void pinchTriggered( QPinchGesture *gesture );
};

#endif
