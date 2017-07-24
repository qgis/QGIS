/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutnewitempropertiesdialog.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemPropertiesDialog : QDialog
{
%Docstring
 A dialog for configuring properties like the size and position of layout items.

 This is usually used only when constructing new layout items, allowing users to precisely
 enter their sizes and positions.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutnewitempropertiesdialog.h"
%End
  public:

    QgsLayoutItemPropertiesDialog( QWidget *parent = 0, Qt::WindowFlags flags = 0 );
%Docstring
 Constructor for QgsLayoutNewItemPropertiesDialog.
%End

    void setItemPosition( QgsLayoutPoint position );
%Docstring
 Sets the item ``position`` to show in the dialog.
.. seealso:: itemPosition()
%End

    QgsLayoutPoint itemPosition() const;
%Docstring
 Returns the current item position defined by the dialog.
.. seealso:: setItemPosition()
 :rtype: QgsLayoutPoint
%End

    void setItemSize( QgsLayoutSize size );
%Docstring
 Sets the item ``size`` to show in the dialog.
.. seealso:: itemSize()
%End

    QgsLayoutSize itemSize() const;
%Docstring
 Returns the item size defined by the dialog.
.. seealso:: setItemSize()
 :rtype: QgsLayoutSize
%End

    QgsLayoutItem::ReferencePoint referencePoint() const;
%Docstring
 Returns the item reference point defined by the dialog.
.. seealso:: setReferencePoint()
 :rtype: QgsLayoutItem.ReferencePoint
%End

    void setReferencePoint( QgsLayoutItem::ReferencePoint point );
%Docstring
 Sets the item reference ``point`` defined to show in the dialog.
.. seealso:: referencePoint()
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutnewitempropertiesdialog.h                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
