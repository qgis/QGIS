/***************************************************************************
                         qgslayoutmodel.h
                         ----------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTMODEL_H
#define QGSLAYOUTMODEL_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgslayoutitemregistry.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <QSet>

class QgsLayout;
class QGraphicsItem;
class QgsLayoutItem;

/**
 * \class QgsLayoutModel
 * \ingroup core
 *
 * A model for items attached to a layout. The model also maintains the z-order for the
 * layout, and must be notified whenever item stacking changes.
 *
 * Internally, QgsLayoutModel maintains two lists. One contains a complete list of all items for
 * the layout, ordered by their position within the z-order stack.
 *
 * The second list contains only items which are currently displayed in the layout's scene.
 * It is used as a cache of the last known stacking order, so that the model can compare the current
 * stacking of items in the layout to the last known state, and emit the corresponding signals
 * as required.
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsLayoutModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Columns returned by the model
    enum Columns
    {
      Visibility = 0, //!< Item visibility checkbox
      LockStatus, //!< Item lock status checkbox
      ItemId, //!< Item ID
    };

    /**
     * Constructor for a QgsLayoutModel attached to the specified \a layout.
     */
    explicit QgsLayoutModel( QgsLayout *layout, QObject *parent SIP_TRANSFERTHIS = nullptr );

    //reimplemented QAbstractItemModel methods
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData( const QModelIndexList &indexes ) const override;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

///@cond PRIVATE
#ifndef SIP_RUN

    /**
     * Clears all items from z-order list and resets the model
     */
    void clear();

    /**
     * Returns the size of the z-order list.
     */
    int zOrderListSize() const;

    /**
     * Rebuilds the z-order list, based on the current stacking of items in the layout.
     * This method should be called after adding multiple items to the layout.
     */
    void rebuildZList();

    /**
     * Adds an \a item to the top of the layout z stack. The item must not already exist in the z-order list.
     * \see reorderItemToTop()
     */
    void addItemAtTop( QgsLayoutItem *item );

    /**
     * Removes an \a item from the z-order list.
     */
    void removeItem( QgsLayoutItem *item );

    /**
     * Moves an \a item up the z-order list.
     *
     * Returns true if \a item was moved. Returns false if \a item was not found
     * in z-order list or was already at the top of the z-order list.
     *
     * \see reorderItemDown()
     * \see reorderItemToTop()
     * \see reorderItemToBottom()
     */
    bool reorderItemUp( QgsLayoutItem *item );

    /**
     * Moves an \a item down the z-order list.
     *
     * Returns true if \a item was moved. Returns false if \a item was not found
     * in z-order list or was already at the bottom of the z-order list.
     *
     * \see reorderItemUp()
     * \see reorderItemToTop()
     * \see reorderItemToBottom()
     */
    bool reorderItemDown( QgsLayoutItem *item );

    /**
     * Moves an \a item to the top of the z-order list.
     *
     * Returns true if \a item was moved. Returns false if \a item was not found
     * in z-order list or was already at the top of the z-order list.
     *
     * \see reorderItemUp()
     * \see reorderItemDown()
     * \see reorderItemToBottom()
     */
    bool reorderItemToTop( QgsLayoutItem *item );

    /**
     * Moves an \a item to the bottom of the z-order list.
     *
     * Returns true if \a item was moved. Returns false if \a item was not found
     * in z-order list or was already at the bottom of the z-order list.
     *
     * \see reorderItemUp()
     * \see reorderItemDown()
     * \see reorderItemToTop()
     */
    bool reorderItemToBottom( QgsLayoutItem *item );

    /**
     * Finds the next layout item above an \a item, where \a item is
     * the item to search above.
     *
     * If no items were found, a nullptr will be returned.
     *
     * \see findItemBelow()
     */
    QgsLayoutItem *findItemAbove( QgsLayoutItem *item ) const;

    /**
     * Finds the next layout item below an \a item, where \a item
     * is the item to search below.
     *
     * If no items were found, a nullptr will be returned.

     * \see findItemAbove()
     */
    QgsLayoutItem *findItemBelow( QgsLayoutItem *item ) const;

    /**
     * Returns the item z-order list.
     */
    QList<QgsLayoutItem *> &zOrderList();

    /**
     * Marks an \a item as removed from the layout. This must be called whenever an item
     * is about to be removed from the layout.
     */
    void setItemRemoved( QgsLayoutItem *item );

#if 0

    /**
     * Restores an item to the composition. This must be called whenever an item removed
     * from the composition is restored to the composition.
     * \param item to mark as restored to the composition
     * \see setItemRemoved
     * \since QGIS 2.5
     */
    void setItemRestored( QgsComposerItem *item );
#endif

    /**
     * Must be called when an \a item's display name is modified.
     *
     * \see updateItemLockStatus()
     * \see updateItemVisibility()
     * \see updateItemSelectStatus()
     */
    void updateItemDisplayName( QgsLayoutItem *item );

    /**
     * Must be called when an \a item's lock status changes.
     * \see updateItemDisplayName()
     * \see updateItemVisibility()
     * \see updateItemSelectStatus()
     */
    void updateItemLockStatus( QgsLayoutItem *item );

    /**
     * Must be called when an \a item's visibility changes.
     * \see updateItemDisplayName()
     * \see updateItemLockStatus()
     * \see updateItemSelectStatus()
     */
    void updateItemVisibility( QgsLayoutItem *item );

    /**
     * Must be called when an \a item's selection status changes.
     * \see updateItemDisplayName()
     * \see updateItemVisibility()
     * \see updateItemLockStatus()
     */
    void updateItemSelectStatus( QgsLayoutItem *item );
#endif
///@endcond

    /**
     * Returns the QgsLayoutItem corresponding to a QModelIndex \a index, if possible.
     * \see indexForItem()
     */
    QgsLayoutItem *itemFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the QModelIndex corresponding to a QgsLayoutItem \a item and \a column, if possible.
     * \see itemFromIndex()
     */
    QModelIndex indexForItem( QgsLayoutItem *item, int column = 0 );

  public slots:

///@cond PRIVATE
#ifndef SIP_RUN

    /**
     * Sets an item as the current selection from a QModelIndex \a index.
     */
    void setSelected( const QModelIndex &index );
#endif
///@endcond

  private:

    //! Maintains z-Order of items. Starts with item at position 1 (position 0 is always paper item)
    QList<QgsLayoutItem *> mItemZList;

    //! Cached list of items from mItemZList which are currently in the scene
    QList<QgsLayoutItem *> mItemsInScene;

    //! Parent layout
    QgsLayout *mLayout = nullptr;

    /**
     * Rebuilds the list of all layout items which are present in the layout. This is
     * called when the stacking of order changes or when items are removed/restored to the
     * layout. Unlike rebuildSceneItemList, this method clears the existing scene item
     * list and does not emit QAbstractItemModel signals. Accordingly, this method should
     * only be called when changes to the z-order list are known and QAbstractItemModel begin
     * signals have already been called.
     * \see rebuildSceneItemList()
     */
    void refreshItemsInScene();

    /**
     * Steps through the item z-order list and rebuilds the items in layout list,
     * emitting QAbstractItemModel signals as required.
     * \see refreshItemsInScene()
     */
    void rebuildSceneItemList();

    friend class TestQgsLayoutModel;
    friend class TestQgsLayoutGui;
};


/**
 * \class QgsLayoutProxyModel
 * \ingroup core
 * \brief Allows for filtering a QgsLayoutModel by item type.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutProxyModelm, attached to the specified \a layout.
     */
    QgsLayoutProxyModel( QgsLayout *layout, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the current item type filter, or QgsLayoutItemRegistry::LayoutItem if no
     * item type filter is set.
     * \see setFilterType()
     */
    QgsLayoutItemRegistry::ItemType filterType() const { return mItemTypeFilter; }

    /**
     * Sets the item type \a filter. Only matching item types will be shown.
     * Set \a filter to QgsLayoutItemRegistry::LayoutItem to show all
     * item types.
     * \see filterType()
     */
    void setFilterType( QgsLayoutItemRegistry::ItemType filter );

    /**
     * Sets a list of specific \a items to exclude from the model.
     * \see exceptedItemList()
     */
    void setExceptedItemList( const QList< QgsLayoutItem * > &items );

    /**
     * Returns the list of specific items excluded from the model.
     * \see setExceptedItemList()
     */
    QList< QgsLayoutItem * > exceptedItemList() const { return mExceptedList; }

    /**
     * Returns the QgsLayoutModel used in this proxy model.
     */
    QgsLayoutModel *sourceLayerModel() const { return static_cast< QgsLayoutModel * >( sourceModel() ); }

    /**
     * Returns the QgsLayoutItem corresponding to an index from the source
     * QgsLayoutModel model.
     */
    QgsLayoutItem *itemFromSourceIndex( const QModelIndex &sourceIndex ) const;

  protected:
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    QgsLayout *mLayout = nullptr;
    QgsLayoutItemRegistry::ItemType mItemTypeFilter;
    QList< QgsLayoutItem * > mExceptedList;

};



#endif //QGSLAYOUTMODEL_H
