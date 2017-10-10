/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtooladditem.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutViewToolAddItem : QgsLayoutViewTool
{
%Docstring
 Layout view tool for adding items to a layout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutviewtooladditem.h"
%End
  public:

    QgsLayoutViewToolAddItem( QgsLayoutView *view /TransferThis/ );

    int itemType() const;
%Docstring
 Returns the item type for items created by the tool.
.. seealso:: setItemType()
 :rtype: int
%End

    void setItemType( int type );
%Docstring
 Sets the item ``type`` for items created by the tool.
.. seealso:: itemType()
%End

    virtual void layoutPressEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutMoveEvent( QgsLayoutViewMouseEvent *event );

    virtual void layoutReleaseEvent( QgsLayoutViewMouseEvent *event );

    virtual void deactivate();


  signals:

    void createdItem();
%Docstring
 Emitted when an item has been created using the tool.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutviewtooladditem.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
