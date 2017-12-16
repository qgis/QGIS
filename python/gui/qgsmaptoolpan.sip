/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolpan.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapToolPan : QgsMapTool
{
%Docstring
A map tool for panning the map.

.. seealso:: :py:class:`QgsMapTool`
%End

%TypeHeaderCode
#include "qgsmaptoolpan.h"
%End
  public:
    QgsMapToolPan( QgsMapCanvas *canvas );
%Docstring
constructor
%End
    ~QgsMapToolPan();

    virtual void activate();

    virtual void deactivate();


    virtual Flags flags() const;
    virtual void canvasPressEvent( QgsMapMouseEvent *e );

    virtual void canvasMoveEvent( QgsMapMouseEvent *e );

    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );

    virtual void canvasDoubleClickEvent( QgsMapMouseEvent *e );

    virtual bool gestureEvent( QGestureEvent *e );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolpan.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
