/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptooladvanceddigitizing.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapToolAdvancedDigitizing : QgsMapToolEdit
{
%Docstring
 The QgsMapToolAdvancedDigitizing class is a QgsMapTool which gives event directly in map coordinates and allows filtering its events.
Events from QgsMapTool are caught and their QMouseEvent are transformed into QgsMapMouseEvent (with map coordinates).
Events are then forwarded to corresponding virtual methods which can be reimplemented in subclasses.
An event filter can be set on the map tool to filter and modify the events in map coordinates (:py:class:`QgsMapToolMapEventFilter`).

.. note::

   at the moment, the event filter is used by the CAD tools (:py:class:`QgsCadDocWidget`).

.. note::

   the event filter definition is not exposed in Python API to avoid any unexpected behavior.
%End

%TypeHeaderCode
#include "qgsmaptooladvanceddigitizing.h"
%End
  public:

    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
%Docstring
Creates an advanced digitizing maptool

:param canvas:         The map canvas on which the tool works
:param cadDockWidget:  The cad dock widget which will be used to adjust mouse events
%End

    virtual void canvasPressEvent( QgsMapMouseEvent *e );

%Docstring
Catch the mouse press event, filters it, transforms it to map coordinates and send it to virtual method
%End
    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );

%Docstring
Catch the mouse release event, filters it, transforms it to map coordinates and send it to virtual method
%End
    virtual void canvasMoveEvent( QgsMapMouseEvent *e );

%Docstring
Catch the mouse move event, filters it, transforms it to map coordinates and send it to virtual method
%End

    virtual void activate();

%Docstring
Registers this maptool with the cad dock widget
%End

    virtual void deactivate();

%Docstring
Unregisters this maptool from the cad dock widget
%End

    QgsAdvancedDigitizingDockWidget *cadDockWidget() const;

    bool isAdvancedDigitizingAllowed() const;
%Docstring
Returns whether functionality of advanced digitizing dock widget is currently allowed.

Tools may decide to switch this support on/off based on the current state of the map tool.
For example, in node tool before user picks a vertex to move, advanced digitizing dock
widget should be disabled and only enabled once a vertex is being moved. Other map tools
may keep advanced digitizing allowed all the time.

If true is returned, that does not mean that advanced digitizing is actually active,
because it is up to the user to enable/disable it when it is allowed.
\sa setAdvancedDigitizingAllowed()

.. versionadded:: 3.0
%End

    bool isAutoSnapEnabled() const;
%Docstring
Returns whether mouse events (press/move/release) should automatically try to snap mouse position
(according to the snapping configuration of map canvas) before passing the mouse coordinates
to the tool. This may be desirable default behavior for some map tools, but not for other map tools.
It is therefore possible to configure the behavior by the map tool.
\sa isAutoSnapEnabled()

.. versionadded:: 3.0
%End

  protected:

    void setAdvancedDigitizingAllowed( bool allowed );
%Docstring
Sets whether functionality of advanced digitizing dock widget is currently allowed.
This method is protected because it should be a decision of the map tool and not from elsewhere.
\sa isAdvancedDigitizingAllowed()

.. versionadded:: 3.0
%End

    void setAutoSnapEnabled( bool enabled );
%Docstring
Sets whether mouse events (press/move/release) should automatically try to snap mouse position
This method is protected because it should be a decision of the map tool and not from elsewhere.
\sa isAutoSnapEnabled()

.. versionadded:: 3.0
%End

  public:

    virtual void cadCanvasPressEvent( QgsMapMouseEvent *e );
%Docstring
Override this method when subclassing this class.
This will receive adapted events from the cad system whenever a
canvasPressEvent is triggered and it's not hidden by the cad's
construction mode.

:param e: Mouse events prepared by the cad system
%End


    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent *e );
%Docstring
Override this method when subclassing this class.
This will receive adapted events from the cad system whenever a
canvasReleaseEvent is triggered and it's not hidden by the cad's
construction mode.

:param e: Mouse events prepared by the cad system
%End


    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e );
%Docstring
Override this method when subclassing this class.
This will receive adapted events from the cad system whenever a
canvasMoveEvent is triggered and it's not hidden by the cad's
construction mode.

:param e: Mouse events prepared by the cad system
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptooladvanceddigitizing.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
