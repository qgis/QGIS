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
.. seealso:: QgsMapToolMapEventFilter).
.. seealso:: QgsCadDocWidget).
.. note::

   the event filter definition is not exposed in Python API to avoid any unexpected behavior.
%End

%TypeHeaderCode
#include "qgsmaptooladvanceddigitizing.h"
%End
  public:
    enum CaptureMode
    {
      CaptureNone,
      CapturePoint,
      CaptureSegment,
      CaptureLine,
      CapturePolygon
    };

    explicit QgsMapToolAdvancedDigitizing( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
%Docstring
 Creates an advanced digitizing maptool
 \param canvas         The map canvas on which the tool works
 \param cadDockWidget  The cad dock widget which will be used to adjust mouse events
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

    CaptureMode mode() const;
%Docstring
 The capture mode

 :return: Capture mode
 :rtype: CaptureMode
%End

    void setMode( CaptureMode mode );
%Docstring
 Set capture mode. This should correspond to the layer on which the digitizing
 happens.

 \param mode Capture Mode
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
%Docstring
 :rtype: QgsAdvancedDigitizingDockWidget
%End


  protected:

    virtual void cadCanvasPressEvent( QgsMapMouseEvent *e );
%Docstring
 Override this method when subclassing this class.
 This will receive adapted events from the cad system whenever a
 canvasPressEvent is triggered and it's not hidden by the cad's
 construction mode.

 \param e Mouse events prepared by the cad system
%End


    virtual void cadCanvasReleaseEvent( QgsMapMouseEvent *e );
%Docstring
 Override this method when subclassing this class.
 This will receive adapted events from the cad system whenever a
 canvasReleaseEvent is triggered and it's not hidden by the cad's
 construction mode.

 \param e Mouse events prepared by the cad system
%End


    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e );
%Docstring
 Override this method when subclassing this class.
 This will receive adapted events from the cad system whenever a
 canvasMoveEvent is triggered and it's not hidden by the cad's
 construction mode.

 \param e Mouse events prepared by the cad system
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptooladvanceddigitizing.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
