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

#include "qgsmaptooledit.h"
#include "qgis_gui.h"

class QgsMapMouseEvent;
class QgsAdvancedDigitizingDockWidget;
class QgsSnapToGridCanvasItem;

/**
 * \ingroup gui
 * \brief The QgsMapToolAdvancedDigitizing class is a QgsMapTool which gives event directly in map coordinates and allows filtering its events.
 * Events from QgsMapTool are caught and their QMouseEvent are transformed into QgsMapMouseEvent (with map coordinates).
 * Events are then forwarded to corresponding virtual methods which can be reimplemented in subclasses.
 * An event filter can be set on the map tool to filter and modify the events in map coordinates (\see QgsMapToolMapEventFilter).
 * \note at the moment, the event filter is used by the CAD tools (\see QgsCadDocWidget).
 * \note the event filter definition is not exposed in Python API to avoid any unexpected behavior.
 */
class GUI_EXPORT QgsMapToolAdvancedDigitizing : public QgsMapToolEdit
{
    Q_OBJECT
  public:

    /**
     * Creates an advanced digitizing maptool
     * \param canvas         The map canvas on which the tool works
     * \param cadDockWidget  The cad dock widget which will be used to adjust mouse events
     */
    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    //! Catch the mouse press event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    //! Catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    //! Catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    /**
     * Registers this maptool with the cad dock widget
     */
    void activate() override;

    /**
     * Unregisters this maptool from the cad dock widget
     */
    void deactivate() override;

    QgsAdvancedDigitizingDockWidget *cadDockWidget() const { return mCadDockWidget; }

    /**
     * Returns the layer associated with the map tool.
     *
     * By default this returns the map canvas' QgsMapCanvas::currentLayer().
     *
     * \since QGIS 3.22
     */
    virtual QgsMapLayer *layer() const;

    /**
     * Returns whether functionality of advanced digitizing dock widget is currently allowed.
     *
     * Tools may decide to switch this support on/off based on the current state of the map tool.
     * For example, in vertex tool before user picks a vertex to move, advanced digitizing dock
     * widget should be disabled and only enabled once a vertex is being moved. Other map tools
     * may keep advanced digitizing allowed all the time.
     *
     * If TRUE is returned, that does not mean that advanced digitizing is actually active,
     * because it is up to the user to enable/disable it when it is allowed.
     * \sa setAdvancedDigitizingAllowed()
     * \since QGIS 3.0
     */
    bool isAdvancedDigitizingAllowed() const { return mAdvancedDigitizingAllowed; }

    /**
     * Returns whether mouse events (press/move/release) should automatically try to snap mouse position
     * (according to the snapping configuration of map canvas) before passing the mouse coordinates
     * to the tool. This may be desirable default behavior for some map tools, but not for other map tools.
     * It is therefore possible to configure the behavior by the map tool.
     * \sa isAutoSnapEnabled()
     * \since QGIS 3.0
     */
    bool isAutoSnapEnabled() const { return mAutoSnapEnabled; }

  protected:

    /**
     * Sets whether functionality of advanced digitizing dock widget is currently allowed.
     * This method is protected because it should be a decision of the map tool and not from elsewhere.
     * \sa isAdvancedDigitizingAllowed()
     * \since QGIS 3.0
     */
    void setAdvancedDigitizingAllowed( bool allowed ) { mAdvancedDigitizingAllowed = allowed; }

    /**
     * Sets whether mouse events (press/move/release) should automatically try to snap mouse position
     * This method is protected because it should be a decision of the map tool and not from elsewhere.
     * \sa isAutoSnapEnabled()
     * \since QGIS 3.0
     */
    void setAutoSnapEnabled( bool enabled ) { mAutoSnapEnabled = enabled; }


    QgsAdvancedDigitizingDockWidget *mCadDockWidget = nullptr;

  public:

    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasPressEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * \param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasPressEvent( QgsMapMouseEvent *e ) { Q_UNUSED( e ) }


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasReleaseEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * \param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) { Q_UNUSED( e ) }


    /**
     * Override this method when subclassing this class.
     * This will receive adapted events from the cad system whenever a
     * canvasMoveEvent is triggered and it's not hidden by the cad's
     * construction mode.
     *
     * \param e Mouse events prepared by the cad system
     */
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e ) { Q_UNUSED( e ) }

    /**
     * Enables or disables snap to grid of mouse events.
     * The snapping will occur in the layer's CRS.
     *
     * \since QGIS 3.4
     */
    bool snapToLayerGridEnabled() const;

    /**
     * Enables or disables snap to grid of mouse events.
     * The snapping will occur in the layer's CRS.
     *
     * \since QGIS 3.4
     */
    void setSnapToLayerGridEnabled( bool snapToLayerGridEnabled );

  private slots:

    /**
     * Is to be called by the cad system whenever a point changes outside of a
     * mouse event. E.g. when additional constraints are toggled.
     * The specified point will be used to generate a fake mouse event which will
     * be sent as move event to cadCanvasMoveEvent.
     *
     * \param point The last point known to the cad system.
     */
    void cadPointChanged( const QgsPointXY &point );

    void onCurrentLayerChanged();

  private:

    //! Whether to allow use of advanced digitizing dock at this point
    bool mAdvancedDigitizingAllowed = true;
    //! Whether to snap mouse cursor to map before passing coordinates to cadCanvas*Event()
    bool mAutoSnapEnabled = true;
    //! Whether to snap to grid before passing coordinates to cadCanvas*Event()
    bool mSnapToLayerGridEnabled = true;
    QgsSnapToGridCanvasItem *mSnapToGridCanvasItem = nullptr;
};

#endif // QGSMAPTOOLADVANCEDDIGITIZE_H
