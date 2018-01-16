/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolemitpoint.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsMapToolEmitPoint : QgsMapTool
{
%Docstring
A map tool that simply emits a point when clicking on the map.
Connecting a slot to its canvasClicked() signal will
let you implement custom behavior for the passed in point.
%End

%TypeHeaderCode
#include "qgsmaptoolemitpoint.h"
%End
  public:
    QgsMapToolEmitPoint( QgsMapCanvas *canvas );
%Docstring
constructor
%End

    virtual Flags flags() const;
    virtual void canvasMoveEvent( QgsMapMouseEvent *e );

    virtual void canvasPressEvent( QgsMapMouseEvent *e );

    virtual void canvasReleaseEvent( QgsMapMouseEvent *e );


  signals:
    void canvasClicked( const QgsPointXY &point, Qt::MouseButton button );
%Docstring
signal emitted on canvas click
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolemitpoint.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
