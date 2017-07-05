/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolselect.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutViewToolSelect : QgsLayoutViewTool
{
%Docstring
 Layout view tool for selecting items in the layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtoolselect.h"
%End
  public:

    QgsLayoutViewToolSelect( QgsLayoutView *view );
%Docstring
 Constructor for QgsLayoutViewToolSelect.
%End

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );

    virtual void deactivate();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtoolselect.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
