/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolextent.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMapToolExtent : QgsMapTool
{
%Docstring
A map tool that emits an extent from a rectangle drawn onto the map canvas.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsmaptoolextent.h"
%End
  public:

    QgsMapToolExtent( QgsMapCanvas *canvas );
%Docstring
constructor
%End

    virtual Flags flags() const;
    virtual void canvasMoveEvent( QgsMapMouseEvent *e );

    virtual void canvasPressEvent( QgsMapMouseEvent *e );

    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );

    virtual void activate();

    virtual void deactivate();


    void setRatio( QSize ratio );
%Docstring
Sets a fixed aspect ratio to be used when dragging extent onto the canvas.
To unset a fixed aspect ratio, set the width and height to zero.

:param ratio: aspect ratio's width and height
%End

    QSize ratio() const;
%Docstring
Returns the current fixed aspect ratio to be used when dragging extent onto the canvas.
If the aspect ratio isn't fixed, the width and height will be set to zero.
%End

    QgsRectangle extent() const;
%Docstring
Returns the current extent drawn onto the canvas.
%End

  signals:

    void extentChanged( const QgsRectangle &extent );
%Docstring
signal emitted on extent change
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolextent.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
