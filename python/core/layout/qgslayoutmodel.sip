/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsLayoutModel: QAbstractItemModel
{
%Docstring

 A model for items attached to a layout. The model also maintains the z-order for the
 layout, and must be notified whenever item stacking changes.

 Internally, QgsLayoutModel maintains two lists. One contains a complete list of all items for
 the layout, ordered by their position within the z-order stack.

 The second list contains only items which are currently displayed in the layout's scene.
 It is used as a cache of the last known stacking order, so that the model can compare the current
 stacking of items in the layout to the last known state, and emit the corresponding signals
 as required.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutmodel.h"
%End
  public:

    enum Columns
    {
      Visibility,
      LockStatus,
      ItemId,
    };

    explicit QgsLayoutModel( QgsLayout *layout, QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for a QgsLayoutModel attached to the specified ``layout``.
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
%End

    int zOrderListSize() const;
%Docstring
 Returns the size of the z-order list.
 :rtype: int
%End

    void rebuildZList();
%Docstring
 Rebuilds the z-order list, based on the current stacking of items in the layout.
 This method should be called after adding multiple items to the layout.
%End

    void addItemAtTop( QgsLayoutItem *item );
%Docstring
 Adds an ``item`` to the top of the layout z stack. The item must not already exist in the z-order list.
.. seealso:: reorderItemToTop()
%End

    void removeItem( QgsLayoutItem *item );
%Docstring
 Removes an ``item`` from the z-order list.
%End

    bool reorderItemUp( QgsLayoutItem *item );
%Docstring
 Moves an ``item`` up the z-order list.

 Returns true if ``item`` was moved. Returns false if ``item`` was not found
 in z-order list or was already at the top of the z-order list.

.. seealso:: reorderItemDown()
.. seealso:: reorderItemToTop()
.. seealso:: reorderItemToBottom()
 :rtype: bool
%End

    bool reorderItemDown( QgsLayoutItem *item );
%Docstring
 Moves an ``item`` down the z-order list.

 Returns true if ``item`` was moved. Returns false if ``item`` was not found
 in z-order list or was already at the bottom of the z-order list.

.. seealso:: reorderItemUp()
.. seealso:: reorderItemToTop()
.. seealso:: reorderItemToBottom()
 :rtype: bool
%End

    bool reorderItemToTop( QgsLayoutItem *item );
%Docstring
 Moves an ``item`` to the top of the z-order list.

 Returns true if ``item`` was moved. Returns false if ``item`` was not found
 in z-order list or was already at the top of the z-order list.

.. seealso:: reorderItemUp()
.. seealso:: reorderItemDown()
.. seealso:: reorderItemToBottom()
 :rtype: bool
%End

    bool reorderItemToBottom( QgsLayoutItem *item );
%Docstring
 Moves an ``item`` to the bottom of the z-order list.

 Returns true if ``item`` was moved. Returns false if ``item`` was not found
 in z-order list or was already at the bottom of the z-order list.

.. seealso:: reorderItemUp()
.. seealso:: reorderItemDown()
.. seealso:: reorderItemToTop()
 :rtype: bool
%End

    QgsLayoutItem *findItemAbove( QgsLayoutItem *item ) const;
%Docstring
 Finds the next layout item above an ``item``, where ``item`` is
 the item to search above.

 If no items were found, a None will be returned.

.. seealso:: findItemBelow()
 :rtype: QgsLayoutItem
%End

    QgsLayoutItem *findItemBelow( QgsLayoutItem *item ) const;
%Docstring
 Finds the next layout item below an ``item``, where ``item``
 is the item to search below.

 If no items were found, a None will be returned.

.. seealso:: findItemAbove()
 :rtype: QgsLayoutItem
%End

    QList<QgsLayoutItem *> &zOrderList();
%Docstring
 Returns the item z-order list.
 :rtype: list of QgsLayoutItem
%End

    void setItemRemoved( QgsLayoutItem *item );
%Docstring
 Marks an ``item`` as removed from the layout. This must be called whenever an item
 is about to be removed from the layout.
%End


    void updateItemDisplayName( QgsLayoutItem *item );
%Docstring
 Must be called when an ``item``'s display name is modified.

.. seealso:: updateItemLockStatus()
.. seealso:: updateItemVisibility()
.. seealso:: updateItemSelectStatus()
%End

    void updateItemLockStatus( QgsLayoutItem *item );
%Docstring
 Must be called when an ``item``'s lock status changes.
.. seealso:: updateItemDisplayName()
.. seealso:: updateItemVisibility()
.. seealso:: updateItemSelectStatus()
%End

    void updateItemVisibility( QgsLayoutItem *item );
%Docstring
 Must be called when an ``item``'s visibility changes.
.. seealso:: updateItemDisplayName()
.. seealso:: updateItemLockStatus()
.. seealso:: updateItemSelectStatus()
%End

    void updateItemSelectStatus( QgsLayoutItem *item );
%Docstring
 Must be called when an ``item``'s selection status changes.
.. seealso:: updateItemDisplayName()
.. seealso:: updateItemVisibility()
.. seealso:: updateItemLockStatus()
%End

    QModelIndex indexForItem( QgsLayoutItem *item, const int column = 0 );
%Docstring
 Returns the QModelIndex corresponding to a QgsLayoutItem ``item`` and \column, if possible.
 :rtype: QModelIndex
%End

  public slots:

    void setSelected( const QModelIndex &index );
%Docstring
 Sets an item as the current selection from a QModelIndex ``index``.
%End

};


class QgsLayoutProxyModel: QSortFilterProxyModel
{
%Docstring
 Allows for filtering a QgsLayoutModel by item type.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutmodel.h"
%End
  public:

    QgsLayoutProxyModel( QgsLayout *layout, QObject *parent /TransferThis/ = 0 );
%Docstring
 Constructor for QgsLayoutProxyModelm, attached to the specified ``layout``.
%End

    QgsLayoutItemRegistry::ItemType filterType() const;
%Docstring
 Returns the current item type filter, or QgsLayoutItemRegistry.LayoutItem if no
 item type filter is set.
.. seealso:: setFilterType()
 :rtype: QgsLayoutItemRegistry.ItemType
%End

    void setFilterType( QgsLayoutItemRegistry::ItemType filter );
%Docstring
 Sets the item type ``filter``. Only matching item types will be shown.
 Set ``filter`` to QgsLayoutItemRegistry.LayoutItem to show all
 item types.
.. seealso:: filterType()
%End

    void setExceptedItemList( const QList< QgsLayoutItem * > &items );
%Docstring
 Sets a list of specific ``items`` to exclude from the model.
.. seealso:: exceptedItemList()
%End

    QList< QgsLayoutItem * > exceptedItemList() const;
%Docstring
 Returns the list of specific items excluded from the model.
.. seealso:: setExceptedItemList()
 :rtype: list of QgsLayoutItem
%End

    QgsLayoutModel *sourceLayerModel() const;
%Docstring
 Returns the QgsLayoutModel used in this proxy model.
 :rtype: QgsLayoutModel
%End

    QgsLayoutItem *itemFromSourceIndex( const QModelIndex &sourceIndex ) const;
%Docstring
 Returns the QgsLayoutItem corresponding to an index from the source
 QgsLayoutModel model.
 :rtype: QgsLayoutItem
%End

  protected:
    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmodel.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
