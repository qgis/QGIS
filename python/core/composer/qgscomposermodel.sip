/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermodel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsComposerModel: QAbstractItemModel
{
%Docstring

A model for items attached to a composition. The model also maintains the z-order for the
composition, and must be notified whenever item stacking changes.

Internally, QgsComposerModel maintains two lists. One contains a complete list of all items for
the composition, ordered by their position within the z-order stack. This list also contains
items which have been removed from the composition, so that undo/redo commands can restore
them to their correct position in the stacking order.

The second list contains only items which are currently displayed in the composition's scene.
It is used as a cache of the last known stacking order, so that the model can compare the current
stacking of items in the composition to the last known state, and emit the corresponding signals
as required.
%End

%TypeHeaderCode
#include "qgscomposermodel.h"
%End
  public:

    enum Columns
    {
      Visibility,
      LockStatus,
      ItemId,
    };

    explicit QgsComposerModel( QgsComposition *composition, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor

:param composition: composition to attach to
:param parent: parent object
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role );

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::DropActions supportedDropActions() const;

    virtual QStringList mimeTypes() const;

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );


    void clear();
%Docstring
Clears all items from z-order list and resets the model

.. versionadded:: 2.5
%End

    int zOrderListSize() const;
%Docstring
Returns the size of the z-order list, which includes items which may
have been removed from the composition.

:return: size of z-order list

.. versionadded:: 2.5
%End

    void rebuildZList();
%Docstring
Rebuilds the z-order list, based on the current stacking of items in the composition.
This method should be called after adding multiple items to the composition.

.. versionadded:: 2.5
%End

    void addItemAtTop( QgsComposerItem *item );
%Docstring
Adds an item to the top of the composition z stack.

:param item: item to add. The item must not already exist in the z-order list.

.. versionadded:: 2.5

.. seealso:: :py:func:`reorderItemToTop`
%End

    void removeItem( QgsComposerItem *item );
%Docstring
Removes an item from the z-order list.

:param item: item to remove

.. versionadded:: 2.5
%End

    bool reorderItemUp( QgsComposerItem *item );
%Docstring
Moves an item up the z-order list.

:param item: item to move

:return: true if item was moved. Returns false if item was not found
in z-order list or was already at the top of the z-order list.

.. seealso:: :py:func:`reorderItemDown`

.. seealso:: :py:func:`reorderItemToTop`

.. seealso:: :py:func:`reorderItemToBottom`

.. versionadded:: 2.5
%End

    bool reorderItemDown( QgsComposerItem *item );
%Docstring
Moves an item down the z-order list.

:param item: item to move

:return: true if item was moved. Returns false if item was not found
in z-order list or was already at the bottom of the z-order list.

.. seealso:: :py:func:`reorderItemUp`

.. seealso:: :py:func:`reorderItemToTop`

.. seealso:: :py:func:`reorderItemToBottom`

.. versionadded:: 2.5
%End

    bool reorderItemToTop( QgsComposerItem *item );
%Docstring
Moves an item to the top of the z-order list.

:param item: item to move

:return: true if item was moved. Returns false if item was not found
in z-order list or was already at the top of the z-order list.

.. seealso:: :py:func:`reorderItemUp`

.. seealso:: :py:func:`reorderItemDown`

.. seealso:: :py:func:`reorderItemToBottom`

.. versionadded:: 2.5
%End

    bool reorderItemToBottom( QgsComposerItem *item );
%Docstring
Moves an item to the bottom of the z-order list.

:param item: item to move

:return: true if item was moved. Returns false if item was not found
in z-order list or was already at the bottom of the z-order list.

.. seealso:: :py:func:`reorderItemUp`

.. seealso:: :py:func:`reorderItemDown`

.. seealso:: :py:func:`reorderItemToTop`

.. versionadded:: 2.5
%End

    QgsComposerItem *getComposerItemAbove( QgsComposerItem *item ) const;
%Docstring
Finds the next composer item above an item. This method only considers
items which are currently in the composition, and ignores items which have been
removed from the composition.

:param item: item to search above

:return: item above specified item. If no items were found, no item
will be returned.

.. seealso:: :py:func:`getComposerItemBelow`

.. versionadded:: 2.5
%End

    QgsComposerItem *getComposerItemBelow( QgsComposerItem *item ) const;
%Docstring
Finds the next composer item below an item. This method only considers
items which are currently in the composition, and ignores items which have been
removed from the composition.

:param item: item to search above

:return: item below specified item. If no items were found, no item
will be returned.

.. seealso:: :py:func:`getComposerItemAbove`

.. versionadded:: 2.5
%End

    QList<QgsComposerItem *> *zOrderList();
%Docstring
Returns the item z-order list. This list includes both items currently in the
composition and items which have been removed from the composition.

:return: item z-order list

.. versionadded:: 2.5
%End

    void setItemRemoved( QgsComposerItem *item );
%Docstring
Marks an item as removed from the composition. This must be called whenever an item
has been removed from the composition.

:param item: to mark as removed from the composition

.. seealso:: :py:func:`setItemRestored`

.. versionadded:: 2.5
%End

    void setItemRestored( QgsComposerItem *item );
%Docstring
Restores an item to the composition. This must be called whenever an item removed
from the composition is restored to the composition.

:param item: to mark as restored to the composition

.. seealso:: :py:func:`setItemRemoved`

.. versionadded:: 2.5
%End

    void updateItemDisplayName( QgsComposerItem *item );
%Docstring
Must be called when an item's display name is modified

:param item: item to update

.. seealso:: :py:func:`updateItemLockStatus`

.. seealso:: :py:func:`updateItemVisibility`

.. seealso:: :py:func:`updateItemSelectStatus`

.. versionadded:: 2.5
%End

    void updateItemLockStatus( QgsComposerItem *item );
%Docstring
Must be called when an item's lock status changes

:param item: item to update

.. seealso:: :py:func:`updateItemDisplayName`

.. seealso:: :py:func:`updateItemVisibility`

.. seealso:: :py:func:`updateItemSelectStatus`

.. versionadded:: 2.5
%End

    void updateItemVisibility( QgsComposerItem *item );
%Docstring
Must be called when an item's visibility changes

:param item: item to update

.. seealso:: :py:func:`updateItemDisplayName`

.. seealso:: :py:func:`updateItemLockStatus`

.. seealso:: :py:func:`updateItemSelectStatus`

.. versionadded:: 2.5
%End

    void updateItemSelectStatus( QgsComposerItem *item );
%Docstring
Must be called when an item's selection status changes

:param item: item to update

.. seealso:: :py:func:`updateItemDisplayName`

.. seealso:: :py:func:`updateItemVisibility`

.. seealso:: :py:func:`updateItemLockStatus`

.. versionadded:: 2.5
%End

    QModelIndex indexForItem( QgsComposerItem *item, const int column = 0 );
%Docstring
Returns the QModelIndex corresponding to a QgsComposerItem, if possible

:param item: QgsComposerItem to find index for
:param column: column number for created QModelIndex

:return: QModelIndex corresponding to item and specified column
%End

  public slots:

    void setSelected( const QModelIndex &index );
%Docstring
Sets an item as the current selection from a QModelIndex

:param index: QModelIndex of item to set as selected

.. versionadded:: 2.5
%End

  protected:



};


class QgsComposerProxyModel: QSortFilterProxyModel
{
%Docstring
 Allows for filtering a QgsComposerModel by item type.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscomposermodel.h"
%End
  public:

    QgsComposerProxyModel( QgsComposition *composition, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsComposerProxyModel.

:param composition: composition to attach model to
:param parent: optional parent
%End

    QgsComposerItem::ItemType filterType() const;
%Docstring
Returns the current item type filter, or QgsComposerItem.ComposerItem if no
item type filter is set.

.. seealso:: :py:func:`setFilterType()`
%End

    void setFilterType( QgsComposerItem::ItemType itemType );
%Docstring
Sets the item type filter. Only matching item types will be shown.

:param itemType: type to filter. Set to QgsComposerItem.ComposerItem to show all
item types.

.. seealso:: :py:func:`filterType()`
%End

    void setExceptedItemList( const QList< QgsComposerItem * > &exceptList );
%Docstring
Sets a list of specific items to exclude from the model

:param exceptList: list of items to exclude

.. seealso:: :py:func:`exceptedItemList()`
%End

    QList< QgsComposerItem * > exceptedItemList() const;
%Docstring
Returns the list of specific items excluded from the model.

.. seealso:: :py:func:`setExceptedItemList()`
%End

    QgsComposerModel *sourceLayerModel() const;
%Docstring
Returns the QgsComposerModel used in this proxy model.
%End

    QgsComposerItem *itemFromSourceIndex( const QModelIndex &sourceIndex ) const;
%Docstring
Returns the QgsComposerItem corresponding to an index from the source
QgsComposerModel model.

:param sourceIndex: a QModelIndex

:return: QgsComposerItem for specified index from QgsComposerModel
%End

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermodel.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
