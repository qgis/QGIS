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

/**
 * @brief The QgsMapToolAdvancedDigitizing class is a QgsMapTool whcih gives event directly in map coordinates and allows filtering its events.
 * Events from QgsMapTool are caught and their QMouseEvent are transformed into QgsMapMouseEvent (with map coordinates).
 * Events are then forwarded to corresponding virtual methods which can be reimplemented in subclasses.
 * An event filter can be set on the map tool to filter and modify the events in map coordinates (@see QgsMapToolMapEventFilter).
 * @note at the moment, the event filter is used by the CAD tools (@see QgsCadDocWidget).
 * @note the event filter definition is not exposed in python API to avoid any unexpected behavior.
 */
class QgsMapToolAdvancedDigitizing : QgsMapToolEdit
{
%TypeHeaderCode
#include "qgsmaptooladvanceddigitizing.h"
%End
  public:
    //! Different capture modes
    enum CaptureMode
    {
      CaptureNone,    //!< Do not capture
      CapturePoint,   //!< Capture points
      CaptureLine,    //!< Capture lines
      CapturePolygon  //!< Capture polygons
    };

    /**
     * Creates an advanced digitizing maptool
     * @param canvas         The map canvas on which the tool works
     * @param cadDockWidget  The cad dock widget which will be used to adjust mouse events
     */
    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas* canvas, QgsAdvancedDigitizingDockWidget* cadDockWidget );

    ~QgsMapToolAdvancedDigitizing();

    //! catch the mouse press event, filters it, transforms it to map coordinates and send it to virtual method
    virtual void canvasPressEvent( QgsMapMouseEvent* e );
    //! catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e );
    //! catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
    virtual void canvasMoveEvent( QgsMapMouseEvent* e );

    /**
     * The capture mode
     *
     * @return Capture mode
     */
    CaptureMode mode() const;

    /**
     * Set capture mode. This should correspond to the layer on which the digitizing
     * happens.
     *
     * @param mode Capture Mode
     */
    void setMode( CaptureMode mode );

    /**
     * Registers this maptool with the cad dock widget
     */
    virtual void activate();

    /**
     * Unregisters this maptool from the cad dock widget
     */
    virtual void deactivate();

    QgsAdvancedDigitizingDockWidget* cadDockWidget() const;

  protected:
    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasPressEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasPressEvent( QgsMapMouseEvent* e );


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasReleaseEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent* e );


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasMoveEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent* e );
};
