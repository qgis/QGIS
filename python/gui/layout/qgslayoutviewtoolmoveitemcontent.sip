/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolmoveitemcontent.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutViewToolMoveItemContent : QgsLayoutViewTool
{
%Docstring
 Layout view tool for moving and zooming item content.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtoolmoveitemcontent.h"
%End
  public:

    QgsLayoutViewToolMoveItemContent( QgsLayoutView *view /TransferThis/ );
%Docstring
 Constructor for QgsLayoutViewToolMoveItemContent.
%End

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );

    virtual void wheelEvent( QWheelEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolmoveitemcontent.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
