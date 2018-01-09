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

    int itemMetadataId() const;
%Docstring
Returns the item metadata id for items created by the tool.

.. seealso:: :py:func:`setItemMetadataId`
%End

    void setItemMetadataId( int metadataId );
%Docstring
Sets the item metadata ``metadataId`` for items created by the tool.

The ``metadataId`` associates the current tool behavior with a metadata entry
from :py:class:`QgsLayoutItemGuiRegistry`.

.. seealso:: :py:func:`itemMetadataId`
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
