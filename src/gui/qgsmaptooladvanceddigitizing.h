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

#include "qgsmaptool.h"
#include "qgsmaptooledit.h"
#include "qgsadvanceddigitizingdockwidget.h"

class QgsMapMouseEvent;

/**
 * @brief The QgsMapToolAdvancedDigitizing class is a QgsMapTool whcih gives event directly in map coordinates and allows filtering its events.
 * Events from QgsMapTool are caught and their QMouseEvent are transformed into QgsMapMouseEvent (with map coordinates).
 * Events are then forwarded to corresponding virtual methods which can be reimplemented in subclasses.
 * An event filter can be set on the map tool to filter and modify the events in map coordinates (@see QgsMapToolMapEventFilter).
 * @note at the moment, the event filter is used by the CAD tools (@see QgsCadDocWidget).
 * @note the event filter definition is not exposed in python API to avoid any unexpected behavior.
 */
class GUI_EXPORT QgsMapToolAdvancedDigitizing : public QgsMapToolEdit
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

    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget );

    ~QgsMapToolAdvancedDigitizing();

    //! catch the mouse press event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasPressEvent( QgsMapMouseEvent* e ) override;
    //! catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasReleaseEvent( QgsMapMouseEvent* e ) override;
    //! catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    CaptureMode mode() const { return mCaptureMode; }

    void activate();

    void deactivate();

    QgsAdvancedDigitizingDockWidget* cadDockWidget() const { return mCadDockWidget; }

  protected:
    virtual void cadCanvasPressEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }
    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }

    CaptureMode mCaptureMode;

    bool mSnapOnPress;
    bool mSnapOnRelease;
    bool mSnapOnMove;
    bool mSnapOnDoubleClick;

  private slots:
    void cadPointChanged( const QgsPoint& point );

  private:
    QgsAdvancedDigitizingDockWidget* mCadDockWidget;
};

#endif // QGSMAPTOOLADVANCEDDIGITIZE_H
