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

/** \ingroup gui
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
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;
    //! catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;
    //! catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    /**
     * The capture mode
     *
     * @return Capture mode
     */
    CaptureMode mode() const { return mCaptureMode; }

    /**
     * Set capture mode. This should correspond to the layer on which the digitizing
     * happens.
     *
     * @param mode Capture Mode
     */
    void setMode( CaptureMode mode ) { mCaptureMode = mode; }

    /**
     * Registers this maptool with the cad dock widget
     */
    virtual void activate() override;

    /**
     * Unregisters this maptool from the cad dock widget
     */
    virtual void deactivate() override;

    QgsAdvancedDigitizingDockWidget* cadDockWidget() const { return mCadDockWidget; }

  protected:
    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasPressEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasPressEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasReleaseEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasMoveEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * @param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent* e ) { Q_UNUSED( e ) }

    //! The capture mode in which this tool operates
    CaptureMode mCaptureMode;

    bool mSnapOnPress;       //!< snap on press
    bool mSnapOnRelease;     //!< snap on release
    bool mSnapOnMove;        //!< snap on move
    bool mSnapOnDoubleClick; //!< snap on double click

  private slots:
    /**
     * Is to be called by the cad system whenever a point changes outside of a
     * mouse event. E.g. when additional constraints are toggled.
     * The specified point will be used to generate a fake mouse event which will
     * be sent as move event to cadCanvasMoveEvent.
     *
     * @param point The last point known to the cad system.
     */
    void cadPointChanged( const QgsPoint& point );

  private:
    QgsAdvancedDigitizingDockWidget* mCadDockWidget;

    void snap( QgsMapMouseEvent* e );
};

#endif // QGSMAPTOOLADVANCEDDIGITIZE_H
