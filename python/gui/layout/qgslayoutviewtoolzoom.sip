/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolzoom.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutViewToolZoom : QgsLayoutViewTool
{
%Docstring
Layout view tool for zooming into and out of the layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtoolzoom.h"
%End
  public:

    QgsLayoutViewToolZoom( QgsLayoutView *view /TransferThis/ );
%Docstring
Constructor for QgsLayoutViewToolZoom.
%End

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

    virtual void keyReleaseEvent( QKeyEvent *event );

    virtual void deactivate();


  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolzoom.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
