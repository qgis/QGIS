/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemcombobox.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemComboBox : QComboBox
{
%Docstring
 The QgsLayoutItemComboBox class is a combo box which displays items of
a matching type from a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemcombobox.h"
%End
  public:

    explicit QgsLayoutItemComboBox( QWidget *parent /TransferThis/ = 0, QgsLayout *layout = 0 );
%Docstring
QgsLayoutItemComboBox creates a combo box to display a list of items in a
``layout``. The items can optionally be filtered by type.
If ``layout`` is not set, no items will be shown
until setCurrentLayout() is called
%End

    void setCurrentLayout( QgsLayout *layout );
%Docstring
Sets the ``layout`` containing the items to list in the combo box.
%End

    void setItemType( QgsLayoutItemRegistry::ItemType itemType );
%Docstring
Sets a filter for the item type to show in the combo box.

:param itemType: type of items to show. Set to QgsLayoutItemRegistry.LayoutItem to
show all items.

.. seealso:: :py:func:`itemType`
%End

    QgsLayoutItemRegistry::ItemType itemType() const;
%Docstring
Returns the filter for the item types to show in the combo box.

.. seealso:: :py:func:`setItemType`
%End

    void setExceptedItemList( const QList< QgsLayoutItem * > &exceptList );
%Docstring
Sets a list of specific items to exclude from the combo box.

.. seealso:: :py:func:`exceptedItemList`
%End

    QList< QgsLayoutItem * > exceptedItemList() const;
%Docstring
Returns the list of specific items excluded from the combo box.

.. seealso:: :py:func:`setExceptedItemList`
%End

    QgsLayoutItem *item( int index ) const;
%Docstring
Return the item currently shown at the specified ``index`` within the combo box.

.. seealso:: :py:func:`currentItem`
%End

    QgsLayoutItem *currentItem() const;
%Docstring
Returns the item currently selected in the combo box.
%End

  public slots:

    void setItem( const QgsLayoutItem *item );
%Docstring
Sets the currently selected ``item`` in the combo box.
%End

  signals:

    void itemChanged( QgsLayoutItem *item );
%Docstring
Emitted whenever the currently selected item changes
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemcombobox.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
