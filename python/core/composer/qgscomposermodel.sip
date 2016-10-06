/**
 * \class QgsComposerModel
 * \ingroup core
 *
 * A model for items attached to a composition. The model also maintains the z-order for the
 * composition, and must be notified whenever item stacking changes.
 *
 * Internally, QgsComposerModel maintains two lists. One contains a complete list of all items for
 * the composition, ordered by their position within the z-order stack. This list also contains
 * items which have been removed from the composition, so that undo/redo commands can restore
 * them to their correct position in the stacking order.
 *
 * The second list contains only items which are currently displayed in the composition's scene.
 * It is used as a cache of the last known stacking order, so that the model can compare the current
 * stacking of items in the composition to the last known state, and emit the corresponding signals
 * as required.
 */

class QgsComposerModel : QAbstractItemModel
{
%TypeHeaderCode
#include "qgscomposermodel.h"
%End
  public:

    //! Columns returned by the model
    enum Columns
    {
      Visibility, /*!< Item visibility check box */
      LockStatus, /*!< Item lock status check box */
      ItemId, /*!< Item ID */
    };

    /** Constructor
     * @param composition composition to attach to
     * @param parent parent object
     */
    explicit QgsComposerModel( QgsComposition* composition, QObject* parent /TransferThis/ = 0 );

    ~QgsComposerModel();

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role ) const;
    Qt::ItemFlags flags( const QModelIndex & index ) const;
    bool setData( const QModelIndex & index, const QVariant & value, int role );
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual QMimeData* mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
    bool removeRows( int row, int count, const QModelIndex & parent = QModelIndex() );

    /** Clears all items from z-order list and resets the model
     * @note added in QGIS 2.5
     */
    void clear();

    /** Returns the size of the z-order list, which includes items which may
     * have been removed from the composition.
     * @returns size of z-order list
     * @note added in QGIS 2.5
     */
    int zOrderListSize() const;

    /** Rebuilds the z-order list, based on the current stacking of items in the composition.
     * This method should be called after adding multiple items to the composition.
     * @note added in QGIS 2.5
     */
    void rebuildZList();

    /** Adds an item to the top of the composition z stack.
     * @param item item to add. The item must not already exist in the z-order list.
     * @note added in QGIS 2.5
     * @see reorderItemToTop
     */
    void addItemAtTop( QgsComposerItem *item );

    /** Removes an item from the z-order list.
     * @param item item to remove
     * @note added in QGIS 2.5
     */
    void removeItem( QgsComposerItem *item );

    /** Moves an item up the z-order list.
     * @param item item to move
     * @returns true if item was moved. Returns false if item was not found
     * in z-order list or was already at the top of the z-order list.
     * @see reorderItemDown
     * @see reorderItemToTop
     * @see reorderItemToBottom
     * @note added in QGIS 2.5
     */
    bool reorderItemUp( QgsComposerItem *item );

    /** Moves an item down the z-order list.
     * @param item item to move
     * @returns true if item was moved. Returns false if item was not found
     * in z-order list or was already at the bottom of the z-order list.
     * @see reorderItemUp
     * @see reorderItemToTop
     * @see reorderItemToBottom
     * @note added in QGIS 2.5
     */
    bool reorderItemDown( QgsComposerItem *item );

    /** Moves an item to the top of the z-order list.
     * @param item item to move
     * @returns true if item was moved. Returns false if item was not found
     * in z-order list or was already at the top of the z-order list.
     * @see reorderItemUp
     * @see reorderItemDown
     * @see reorderItemToBottom
     * @note added in QGIS 2.5
     */
    bool reorderItemToTop( QgsComposerItem *item );

    /** Moves an item to the bottom of the z-order list.
     * @param item item to move
     * @returns true if item was moved. Returns false if item was not found
     * in z-order list or was already at the bottom of the z-order list.
     * @see reorderItemUp
     * @see reorderItemDown
     * @see reorderItemToTop
     * @note added in QGIS 2.5
     */
    bool reorderItemToBottom( QgsComposerItem *item );

    /** Finds the next composer item above an item. This method only considers
     * items which are currently in the composition, and ignores items which have been
     * removed from the composition.
     * @param item item to search above
     * @returns item above specified item. If no items were found, no item
     * will be returned.
     * @see getComposerItemBelow
     * @note added in QGIS 2.5
     */
    QgsComposerItem* getComposerItemAbove( QgsComposerItem *item ) const;

    /** Finds the next composer item below an item. This method only considers
     * items which are currently in the composition, and ignores items which have been
     * removed from the composition.
     * @param item item to search above
     * @returns item below specified item. If no items were found, no item
     * will be returned.
     * @see getComposerItemAbove
     * @note added in QGIS 2.5
     */
    QgsComposerItem* getComposerItemBelow( QgsComposerItem *item ) const;

    /** Returns the item z-order list. This list includes both items currently in the
     * composition and items which have been removed from the composition.
     * @returns item z-order list
     * @note added in QGIS 2.5
     */
    QList<QgsComposerItem *>* zOrderList();

    /** Marks an item as removed from the composition. This must be called whenever an item
     * has been removed from the composition.
     * @param item to mark as removed from the composition
     * @see setItemRestored
     * @note added in QGIS 2.5
     */
    void setItemRemoved( QgsComposerItem *item );

    /** Restores an item to the composition. This must be called whenever an item removed
     * from the composition is restored to the composition.
     * @param item to mark as restored to the composition
     * @see setItemRemoved
     * @note added in QGIS 2.5
     */
    void setItemRestored( QgsComposerItem *item );

    /** Must be called when an item's display name is modified
     * @param item item to update
     * @see updateItemLockStatus
     * @see updateItemVisibility
     * @see updateItemSelectStatus
     * @note added in QGIS 2.5
     */
    void updateItemDisplayName( QgsComposerItem *item );

    /** Must be called when an item's lock status changes
     * @param item item to update
     * @see updateItemDisplayName
     * @see updateItemVisibility
     * @see updateItemSelectStatus
     * @note added in QGIS 2.5
     */
    void updateItemLockStatus( QgsComposerItem *item );

    /** Must be called when an item's visibility changes
     * @param item item to update
     * @see updateItemDisplayName
     * @see updateItemLockStatus
     * @see updateItemSelectStatus
     * @note added in QGIS 2.5
     */
    void updateItemVisibility( QgsComposerItem *item );

    /** Must be called when an item's selection status changes
     * @param item item to update
     * @see updateItemDisplayName
     * @see updateItemVisibility
     * @see updateItemLockStatus
     * @note added in QGIS 2.5
     */
    void updateItemSelectStatus( QgsComposerItem *item );

    /** Returns the QModelIndex corresponding to a QgsComposerItem, if possible
     * @param item QgsComposerItem to find index for
     * @param column column number for created QModelIndex
     * @returns QModelIndex corresponding to item and specified column
     */
    QModelIndex indexForItem( QgsComposerItem *item, const int column = 0 );

  public slots:

    /** Sets an item as the current selection from a QModelIndex
     * @param index QModelIndex of item to set as selected
     * @note added in QGIS 2.5
     */
    void setSelected( const QModelIndex &index );
};


/**
 * /class QgsComposerProxyModel
 * /ingroup core
 * /brief Allows for filtering a QgsComposerModel by item type.
 * /note added in 2.16
 */
class QgsComposerProxyModel: QSortFilterProxyModel
{
%TypeHeaderCode
#include "qgscomposermodel.h"
%End
  public:

    /** Constructor for QgsComposerProxyModel.
     * @param composition composition to attach model to
     * @param parent optional parent
     */
    QgsComposerProxyModel( QgsComposition* composition, QObject *parent /TransferThis/ = nullptr );

    /** Returns the current item type filter, or QgsComposerItem::ComposerItem if no
     * item type filter is set.
     * @see setFilterType()
     */
    QgsComposerItem::ItemType filterType() const;

    /** Sets the item type filter. Only matching item types will be shown.
     * @param itemType type to filter. Set to QgsComposerItem::ComposerItem to show all
     * item types.
     * @see filterType()
     */
    void setFilterType( QgsComposerItem::ItemType itemType );

    /** Sets a list of specific items to exclude from the model
     * @param exceptList list of items to exclude
     * @see exceptedItemList()
     */
    void setExceptedItemList( const QList< QgsComposerItem* >& exceptList );

    /** Returns the list of specific items excluded from the model.
     * @see setExceptedItemList()
     */
    QList< QgsComposerItem* > exceptedItemList() const;

    /** Returns the QgsComposerModel used in this proxy model.
     */
    QgsComposerModel* sourceLayerModel() const;

    /** Returns the QgsComposerItem corresponding to an index from the source
     * QgsComposerModel model.
     * @param sourceIndex a QModelIndex
     * @returns QgsComposerItem for specified index from QgsComposerModel
     */
    QgsComposerItem* itemFromSourceIndex( const QModelIndex& sourceIndex ) const;

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

};

