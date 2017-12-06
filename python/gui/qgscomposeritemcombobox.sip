/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposeritemcombobox.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerItemComboBox : QComboBox
{
%Docstring
 The QgsComposerItemComboBox class is a combo box which displays items of
 a matching type from a composition.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscomposeritemcombobox.h"
%End
  public:

    explicit QgsComposerItemComboBox( QWidget *parent /TransferThis/ = 0, QgsComposition *composition = 0 );
%Docstring
 QgsComposerItemComboBox creates a combo box to display a list of items in a
 composition. The items can optionally be filtered by type.
 \param parent parent widget
 \param composition composition to show items from. If not set, no items will be shown
 until setComposition() is called
%End

    void setComposition( QgsComposition *composition );
%Docstring
 Sets the composition containing the items to list in the combo box.
%End

    void setItemType( QgsComposerItem::ItemType itemType );
%Docstring
 Sets a filter for the item type to show in the combo box.
 \param itemType type of items to show. Set to QgsComposerItem.ComposerItem to
 show all items.
.. seealso:: :py:func:`itemType()`
%End

    QgsComposerItem::ItemType itemType() const;
%Docstring
 Returns the filter for the item types to show in the combo box.
.. seealso:: :py:func:`setItemType()`
 :rtype: QgsComposerItem.ItemType
%End

    void setExceptedItemList( const QList< QgsComposerItem * > &exceptList );
%Docstring
 Sets a list of specific items to exclude from the combo box.
 \param exceptList list of items to exclude
.. seealso:: :py:func:`exceptedItemList()`
%End

    QList< QgsComposerItem * > exceptedItemList() const;
%Docstring
 Returns the list of specific items excluded from the combo box.
.. seealso:: :py:func:`setExceptedItemList()`
 :rtype: list of QgsComposerItem
%End

    QgsComposerItem *item( int index ) const;
%Docstring
 Return the item currently shown at the specified index within the combo box.
 \param index position of item to return
.. seealso:: :py:func:`currentItem()`
 :rtype: QgsComposerItem
%End

    QgsComposerItem *currentItem() const;
%Docstring
 Returns the item currently selected in the combo box.
 :rtype: QgsComposerItem
%End

  public slots:

    void setItem( const QgsComposerItem *item );
%Docstring
 Sets the currently selected item in the combo box.
 \param item selected item
%End

  signals:

    void itemChanged( QgsComposerItem *item );
%Docstring
Emitted whenever the currently selected item changes
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgscomposeritemcombobox.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
