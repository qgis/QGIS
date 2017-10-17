/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtooleditnodes.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutViewToolEditNodes : QgsLayoutViewTool
{
%Docstring
 Layout view tool for edit node based items in the layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtooleditnodes.h"
%End
  public:

    QgsLayoutViewToolEditNodes( QgsLayoutView *view /TransferThis/ );
%Docstring
 Constructor for QgsLayoutViewToolEditNodes.
%End

    void deleteSelectedNode();
%Docstring
 Deletes the selected node from the item.
%End

    virtual void activate();

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutDoubleClickEvent( QgsLayoutViewMouseEvent *event );

    virtual void keyPressEvent( QKeyEvent *event );

    virtual void deactivate();

    virtual QList< QgsLayoutItem * > ignoredSnapItems() const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtooleditnodes.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
