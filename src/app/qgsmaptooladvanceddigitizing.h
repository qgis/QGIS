/***************************************************************************
    qgsmaptooladvanceddigitizing.h  - map tool with event in map coordinates
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMAPTOOLADVANCEDDIGITIZE_H
#define QGSMAPTOOLADVANCEDDIGITIZE_H

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsmaptool.h"

class QgsMapMouseEvent;

/**
 * @brief The QgsMapToolAdvancedDigitizing class is a QgsMapTool whcih gives event directly in map coordinates and allows filtering its events.
 * Events from QgsMapTool are caught and their QMouseEvent are transformed into QgsMapMouseEvent (with map coordinates).
 * Events are then forwarded to corresponding virtual methods which can be reimplemented in subclasses.
 * An event filter can be set on the map tool to filter and modify the events in map coordinates (@see QgsMapToolMapEventFilter).
 * @note at the momemt, the event filter is used by the CAD tools (@see QgsCadDocWidget).
 * @note the event filter definition is not exposed in python API to avoid any unexpected behavior.
 */
class APP_EXPORT QgsMapToolAdvancedDigitizing : public QgsMapTool
{
    Q_OBJECT
  public:
    enum CaptureMode
    {
      CaptureNone,
      CapturePoint,
      CaptureLine,
      CapturePolygon
    };

    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas* canvas );

    ~QgsMapToolAdvancedDigitizing();

    //! catch the mouse press event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasPressEvent( QMouseEvent* e ) override;
    //! catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasReleaseEvent( QMouseEvent* e ) override;
    //! catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasMoveEvent( QMouseEvent* e ) override;
    //! catch the mouse double click event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasDoubleClickEvent( QMouseEvent* e ) override;
    //! catch the key press event, filters it and send it to virtual method
    void keyPressEvent( QKeyEvent* event ) override;
    //! catch the key release event, filters it and send it to virtual method
    void keyReleaseEvent( QKeyEvent* event ) override;

    //! mouse press event in map coordinates (eventually filtered) to be redefined in subclass
    virtual void canvasMapPressEvent( QgsMapMouseEvent* e );
    //! mouse release event in map coordinates (eventually filtered) to be redefined in subclass
    virtual void canvasMapReleaseEvent( QgsMapMouseEvent* e );
    //! mouse move event in map coordinates (eventually filtered) to be redefined in subclass
    virtual void canvasMapMoveEvent( QgsMapMouseEvent* e );
    //! mouse double click event in map coordinates (eventually filtered) to be redefined in subclass
    virtual void canvasMapDoubleClickEvent( QgsMapMouseEvent* e );
    //! key press event (eventually filtered) to be redefined in subclass
    virtual void canvasKeyPressEvent( QKeyEvent* e );
    //! key press release (eventually filtered) to be redefined in subclass
    virtual void canvasKeyReleaseEvent( QKeyEvent* e );

    //! return if CAD is allowed in the map tool
    bool cadAllowed() { return mCadAllowed; }

    //! return the capture mode of the map tool
    CaptureMode mode() { return mCaptureMode; }

  protected:

    QgsAdvancedDigitizingDockWidget* mCadDockWidget;

    bool mCadAllowed;

    CaptureMode mCaptureMode;

    bool mSnapOnPress;
    bool mSnapOnRelease;
    bool mSnapOnMove;
    bool mSnapOnDoubleClick;
};

#endif // QGSMAPTOOLADVANCEDDIGITIZE_H
