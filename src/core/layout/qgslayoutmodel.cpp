/***************************************************************************
                         qgslayoutmodel.cpp
                         ------------------
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

#include "qgslayoutmodel.h"
#include "qgslayout.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgslayoutitemgroup.h"
#include <QApplication>
#include <QGraphicsItem>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QSettings>
#include <QIcon>
#include <QIODevice>

QgsLayoutModel::QgsLayoutModel( QgsLayout *layout, QObject *parent )
  : QAbstractItemModel( parent )
  , mLayout( layout )
{

}

QgsLayoutItem *QgsLayoutModel::itemFromIndex( const QModelIndex &index ) const
{
  //try to return the QgsLayoutItem corresponding to a QModelIndex
  if ( !index.isValid() || index.row() == 0 )
  {
    return nullptr;
  }

  QgsLayoutItem *item = static_cast<QgsLayoutItem *>( index.internalPointer() );
  return item;
}

QModelIndex QgsLayoutModel::index( int row, int column,
                                   const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row == 0 )
  {
    return createIndex( row, column, nullptr );
  }
  else if ( !parent.isValid() && row >= 1 && row < mItemsInScene.size() + 1 )
  {
    //return an index for the layout item at this position
    return createIndex( row, column, mItemsInScene.at( row - 1 ) );
  }

  //only top level supported for now
  return QModelIndex();
}

void QgsLayoutModel::refreshItemsInScene()
{
  mItemsInScene.clear();

  const QList< QGraphicsItem * > items = mLayout->items();
  //filter paper items from list
  //TODO - correctly handle grouped item z order placement
  for ( QgsLayoutItem *item : std::as_const( mItemZList ) )
  {
    if ( item->type() != QgsLayoutItemRegistry::LayoutPage && items.contains( item ) )
    {
      mItemsInScene.push_back( item );
    }
  }
}

QModelIndex QgsLayoutModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index )

  //all items are top level for now
  return QModelIndex();
}

int QgsLayoutModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mItemsInScene.size() + 1;
  }

#if 0
  QGraphicsItem *parentItem = itemFromIndex( parent );

  if ( parentItem )
  {
    // return child count for item
    return 0;
  }
#endif

  //no children for now
  return 0;
}

int QgsLayoutModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 3;
}

QVariant QgsLayoutModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsLayoutItem *item = itemFromIndex( index );
  if ( !item )
  {
    return QVariant();
  }

  switch ( role )
  {
    case Qt::DisplayRole:
      if ( index.column() == ItemId )
      {
        return item->displayName();
      }
      else
      {
        return QVariant();
      }

    case Qt::DecorationRole:
      if ( index.column() == ItemId )
      {
        return item->icon();
      }
      else
      {
        return QVariant();
      }

    case Qt::EditRole:
      if ( index.column() == ItemId )
      {
        return item->id();
      }
      else
      {
        return QVariant();
      }

    case Qt::UserRole:
      //store item uuid in userrole so we can later get the QModelIndex for a specific item
      return item->uuid();
    case Qt::UserRole+1:
      //user role stores reference in column object
      return QVariant::fromValue( qobject_cast<QObject *>( item ) );

    case Qt::TextAlignmentRole:
      return static_cast<Qt::Alignment::Int>( Qt::AlignLeft & Qt::AlignVCenter );

    case Qt::CheckStateRole:
      switch ( index.column() )
      {
        case Visibility:
          //column 0 is visibility of item
          return item->isVisible() ? Qt::Checked : Qt::Unchecked;
        case LockStatus:
          //column 1 is locked state of item
          return item->isLocked() ? Qt::Checked : Qt::Unchecked;
        default:
          return QVariant();
      }

    default:
      return QVariant();
  }
}

bool QgsLayoutModel::setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole )
{
  Q_UNUSED( role )

  if ( !index.isValid() )
    return false;

  QgsLayoutItem *item = itemFromIndex( index );
  if ( !item )
  {
    return false;
  }

  switch ( index.column() )
  {
    case Visibility:
      //first column is item visibility
      item->setVisibility( value.toBool() );
      return true;

    case LockStatus:
      //second column is item lock state
      item->setLocked( value.toBool() );
      return true;

    case ItemId:
      //last column is item id
      item->setId( value.toString() );
      return true;
  }

  return false;
}

QVariant QgsLayoutModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( section == ItemId )
      {
        return tr( "Item" );
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( section == Visibility )
      {
        return QVariant::fromValue( QgsApplication::getThemeIcon( QStringLiteral( "/mActionShowAllLayersGray.svg" ) ) );
      }
      else if ( section == LockStatus )
      {
        return QVariant::fromValue( QgsApplication::getThemeIcon( QStringLiteral( "/lockedGray.svg" ) ) );
      }

      return QVariant();
    }

    case Qt::TextAlignmentRole:
      return static_cast<Qt::Alignment::Int>( Qt::AlignLeft & Qt::AlignVCenter );

    default:
      return QAbstractItemModel::headerData( section, orientation, role );
  }

}

Qt::DropActions QgsLayoutModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QStringList QgsLayoutModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/x-vnd.qgis.qgis.composeritemid" );
  return types;
}

QMimeData *QgsLayoutModel::mimeData( const QModelIndexList &indexes ) const
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );

  for ( const QModelIndex &index : indexes )
  {
    if ( index.isValid() && index.column() == ItemId )
    {
      QgsLayoutItem *item = itemFromIndex( index );
      if ( !item )
      {
        continue;
      }
      QString text = item->uuid();
      stream << text;
    }
  }

  mimeData->setData( QStringLiteral( "application/x-vnd.qgis.qgis.composeritemid" ), encodedData );
  return mimeData;
}

bool zOrderDescending( QgsLayoutItem *item1, QgsLayoutItem *item2 )
{
  return item1->zValue() > item2->zValue();
}

bool QgsLayoutModel::dropMimeData( const QMimeData *data,
                                   Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( column != ItemId && column != -1 )
  {
    return false;
  }

  if ( action == Qt::IgnoreAction )
  {
    return true;
  }

  if ( !data->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.composeritemid" ) ) )
  {
    return false;
  }

  if ( parent.isValid() )
  {
    return false;
  }

  int beginRow = row != -1 ? row : rowCount( QModelIndex() );

  QByteArray encodedData = data->data( QStringLiteral( "application/x-vnd.qgis.qgis.composeritemid" ) );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  QList<QgsLayoutItem *> droppedItems;
  int rows = 0;

  while ( !stream.atEnd() )
  {
    QString text;
    stream >> text;
    QgsLayoutItem *item = mLayout->itemByUuid( text );
    if ( item )
    {
      droppedItems << item;
      ++rows;
    }
  }

  if ( droppedItems.empty() )
  {
    //no dropped items
    return false;
  }

  //move dropped items

  //first sort them by z-order
  std::sort( droppedItems.begin(), droppedItems.end(), zOrderDescending );

  //calculate position in z order list to drop items at
  int destPos = 0;
  if ( beginRow < rowCount() )
  {
    QgsLayoutItem *itemBefore = mItemsInScene.at( beginRow - 1 );
    destPos = mItemZList.indexOf( itemBefore );
  }
  else
  {
    //place items at end
    destPos = mItemZList.size();
  }

  //calculate position to insert moved rows to
  int insertPos = destPos;
  for ( QgsLayoutItem *item : std::as_const( droppedItems ) )
  {
    int listPos = mItemZList.indexOf( item );
    if ( listPos == -1 )
    {
      //should be impossible
      continue;
    }

    if ( listPos < destPos )
    {
      insertPos--;
    }
  }

  //remove rows from list
  auto itemIt = droppedItems.begin();
  for ( ; itemIt != droppedItems.end(); ++itemIt )
  {
    mItemZList.removeOne( *itemIt );
  }

  //insert items
  itemIt = droppedItems.begin();
  for ( ; itemIt != droppedItems.end(); ++itemIt )
  {
    mItemZList.insert( insertPos, *itemIt );
    insertPos++;
  }

  rebuildSceneItemList();

  mLayout->updateZValues( true );

  return true;
}

bool QgsLayoutModel::removeRows( int row, int count, const QModelIndex &parent )
{
  Q_UNUSED( count )
  if ( parent.isValid() )
  {
    return false;
  }

  if ( row >= rowCount() )
  {
    return false;
  }

  //do nothing - moves are handled by the dropMimeData method
  return true;
}

///@cond PRIVATE
void QgsLayoutModel::clear()
{
  //totally reset model
  beginResetModel();
  mItemZList.clear();
  refreshItemsInScene();
  endResetModel();
}

int QgsLayoutModel::zOrderListSize() const
{
  return mItemZList.size();
}

void QgsLayoutModel::rebuildZList()
{
  QList<QgsLayoutItem *> sortedList;
  //rebuild the item z order list based on the current zValues of items in the scene

  //get items in descending zValue order
  const QList<QGraphicsItem *> itemList = mLayout->items( Qt::DescendingOrder );
  for ( QGraphicsItem *item : itemList )
  {
    if ( QgsLayoutItem *layoutItem = dynamic_cast<QgsLayoutItem *>( item ) )
    {
      if ( layoutItem->type() != QgsLayoutItemRegistry::LayoutPage )
      {
        sortedList.append( layoutItem );
      }
    }
  }

  mItemZList = sortedList;
  rebuildSceneItemList();
}
///@endcond

void QgsLayoutModel::rebuildSceneItemList()
{
  //step through the z list and rebuild the items in scene list,
  //emitting signals as required
  int row = 0;
  const QList< QGraphicsItem * > items = mLayout->items();
  for ( QgsLayoutItem *item : std::as_const( mItemZList ) )
  {
    if ( item->type() == QgsLayoutItemRegistry::LayoutPage || !items.contains( item ) )
    {
      //item not in scene, skip it
      continue;
    }

    int sceneListPos = mItemsInScene.indexOf( item );
    if ( sceneListPos == row )
    {
      //already in list in correct position, nothing to do

    }
    else if ( sceneListPos != -1 )
    {
      //in list, but in wrong spot
      beginMoveRows( QModelIndex(), sceneListPos + 1, sceneListPos + 1, QModelIndex(), row + 1 );
      mItemsInScene.removeAt( sceneListPos );
      mItemsInScene.insert( row, item );
      endMoveRows();
    }
    else
    {
      //needs to be inserted into list
      beginInsertRows( QModelIndex(), row + 1, row + 1 );
      mItemsInScene.insert( row, item );
      endInsertRows();
    }
    row++;
  }
}
///@cond PRIVATE
void QgsLayoutModel::addItemAtTop( QgsLayoutItem *item )
{
  mItemZList.push_front( item );
  refreshItemsInScene();
  item->setZValue( mItemZList.size() );
}

void QgsLayoutModel::removeItem( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  int pos = mItemZList.indexOf( item );
  if ( pos == -1 )
  {
    //item not in z list, nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    //removing an item not in the scene (e.g., deleted item)
    //we need to remove it from the list, but don't need to call
    //beginRemoveRows or endRemoveRows since the item was not used by the model
    mItemZList.removeAt( pos );
    refreshItemsInScene();
    return;
  }

  //remove item from model
  int row = itemIndex.row();
  beginRemoveRows( QModelIndex(), row, row );
  mItemZList.removeAt( pos );
  refreshItemsInScene();
  endRemoveRows();
}

void QgsLayoutModel::setItemRemoved( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  int pos = mItemZList.indexOf( item );
  if ( pos == -1 )
  {
    //item not in z list, nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    return;
  }

  //removing item
  int row = itemIndex.row();
  beginRemoveRows( QModelIndex(), row, row );
  mLayout->removeItem( item );
  refreshItemsInScene();
  endRemoveRows();
}

void QgsLayoutModel::updateItemDisplayName( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item, ItemId );
  if ( !itemIndex.isValid() )
  {
    return;
  }

  //emit signal for item id change
  emit dataChanged( itemIndex, itemIndex );
}

void QgsLayoutModel::updateItemLockStatus( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item, LockStatus );
  if ( !itemIndex.isValid() )
  {
    return;
  }

  //emit signal for item lock status change
  emit dataChanged( itemIndex, itemIndex );
}

void QgsLayoutModel::updateItemVisibility( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item, Visibility );
  if ( !itemIndex.isValid() )
  {
    return;
  }

  //emit signal for item visibility change
  emit dataChanged( itemIndex, itemIndex );
}

void QgsLayoutModel::updateItemSelectStatus( QgsLayoutItem *item )
{
  if ( !item )
  {
    //nothing to do
    return;
  }

  //need to get QModelIndex of item
  QModelIndex itemIndex = indexForItem( item, ItemId );
  if ( !itemIndex.isValid() )
  {
    return;
  }

  //emit signal for item visibility change
  emit dataChanged( itemIndex, itemIndex );
}

bool QgsLayoutModel::reorderItemUp( QgsLayoutItem *item )
{
  if ( !item )
  {
    return false;
  }

  if ( mItemsInScene.at( 0 ) == item )
  {
    //item is already topmost item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsLayoutItem *> it( mItemZList );
  if ( ! it.findNext( item ) )
  {
    //can't find item in z list, nothing to do
    return false;
  }

  const QList< QGraphicsItem * > sceneItems = mLayout->items();

  it.remove();
  while ( it.hasPrevious() )
  {
    //search through item z list to find previous item which is present in the scene
    it.previous();
    if ( it.value() && sceneItems.contains( it.value() ) )
    {
      break;
    }
  }
  it.insert( item );

  //also move item in scene items z list and notify of model changes
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    return true;
  }

  //move item up in scene list
  int row = itemIndex.row();
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), row - 1 );
  refreshItemsInScene();
  endMoveRows();
  return true;
}

bool QgsLayoutModel::reorderItemDown( QgsLayoutItem *item )
{
  if ( !item )
  {
    return false;
  }

  if ( mItemsInScene.last() == item )
  {
    //item is already lowest item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsLayoutItem *> it( mItemZList );
  if ( ! it.findNext( item ) )
  {
    //can't find item in z list, nothing to do
    return false;
  }

  const QList< QGraphicsItem * > sceneItems = mLayout->items();
  it.remove();
  while ( it.hasNext() )
  {
    //search through item z list to find next item which is present in the scene
    //(deleted items still exist in the z list so that they can be restored to their correct stacking order,
    //but since they are not in the scene they should be ignored here)
    it.next();
    if ( it.value() && sceneItems.contains( it.value() ) )
    {
      break;
    }
  }
  it.insert( item );

  //also move item in scene items z list and notify of model changes
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    return true;
  }

  //move item down in scene list
  int row = itemIndex.row();
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), row + 2 );
  refreshItemsInScene();
  endMoveRows();
  return true;
}

bool QgsLayoutModel::reorderItemToTop( QgsLayoutItem *item )
{
  if ( !item || !mItemsInScene.contains( item ) )
  {
    return false;
  }

  if ( mItemsInScene.at( 0 ) == item )
  {
    //item is already topmost item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsLayoutItem *> it( mItemZList );
  if ( it.findNext( item ) )
  {
    it.remove();
  }
  mItemZList.push_front( item );

  //also move item in scene items z list and notify of model changes
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    return true;
  }

  //move item to top
  int row = itemIndex.row();
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), 1 );
  refreshItemsInScene();
  endMoveRows();
  return true;
}

bool QgsLayoutModel::reorderItemToBottom( QgsLayoutItem *item )
{
  if ( !item || !mItemsInScene.contains( item ) )
  {
    return false;
  }

  if ( mItemsInScene.last() == item )
  {
    //item is already lowest item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsLayoutItem *> it( mItemZList );
  if ( it.findNext( item ) )
  {
    it.remove();
  }
  mItemZList.push_back( item );

  //also move item in scene items z list and notify of model changes
  QModelIndex itemIndex = indexForItem( item );
  if ( !itemIndex.isValid() )
  {
    return true;
  }

  //move item to bottom
  int row = itemIndex.row();
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), rowCount() );
  refreshItemsInScene();
  endMoveRows();
  return true;
}

QgsLayoutItem *QgsLayoutModel::findItemAbove( QgsLayoutItem *item ) const
{
  //search item z list for selected item
  QListIterator<QgsLayoutItem *> it( mItemZList );
  it.toBack();
  if ( it.findPrevious( item ) )
  {
    //move position to before selected item
    while ( it.hasPrevious() )
    {
      //now find previous item, since list is sorted from lowest->highest items
      if ( it.hasPrevious() && !it.peekPrevious()->isGroupMember() )
      {
        return it.previous();
      }
      it.previous();
    }
  }
  return nullptr;
}

QgsLayoutItem *QgsLayoutModel::findItemBelow( QgsLayoutItem *item ) const
{
  //search item z list for selected item
  QListIterator<QgsLayoutItem *> it( mItemZList );
  if ( it.findNext( item ) )
  {
    //return next item (list is sorted from lowest->highest items)
    while ( it.hasNext() )
    {
      if ( !it.peekNext()->isGroupMember() )
      {
        return it.next();
      }
      it.next();
    }
  }
  return nullptr;
}

QList<QgsLayoutItem *> &QgsLayoutModel::zOrderList()
{
  return mItemZList;
}

///@endcond

Qt::ItemFlags QgsLayoutModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  if ( ! index.isValid() )
  {
    return flags | Qt::ItemIsDropEnabled;
  }

  if ( index.row() == 0 )
  {
    return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  else
  {
    switch ( index.column() )
    {
      case Visibility:
      case LockStatus:
        return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
      case ItemId:
        return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
      default:
        return flags | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
  }
}

QModelIndex QgsLayoutModel::indexForItem( QgsLayoutItem *item, const int column )
{
  if ( !item )
  {
    return QModelIndex();
  }

  int row = mItemsInScene.indexOf( item );
  if ( row == -1 )
  {
    //not found
    return QModelIndex();
  }

  return index( row + 1, column );
}

///@cond PRIVATE
void QgsLayoutModel::setSelected( const QModelIndex &index )
{
  QgsLayoutItem *item = itemFromIndex( index );
  if ( !item )
  {
    return;
  }

  // find top level group this item is contained within, and mark the group as selected
  QgsLayoutItemGroup *group = item->parentGroup();
  while ( group && group->parentGroup() )
  {
    group = group->parentGroup();
  }

  // but the actual main selected item is the item itself (allows editing of item properties)
  mLayout->setSelectedItem( item );

  if ( group && group != item )
    group->setSelected( true );
}
///@endcond

//
// QgsLayoutProxyModel
//

QgsLayoutProxyModel::QgsLayoutProxyModel( QgsLayout *layout, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLayout( layout )
  , mItemTypeFilter( QgsLayoutItemRegistry::LayoutItem )
{
  if ( mLayout )
    setSourceModel( mLayout->itemsModel() );

  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  sort( QgsLayoutModel::ItemId );
}

bool QgsLayoutProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const QString leftText = sourceModel()->data( left, Qt::DisplayRole ).toString();
  const QString rightText = sourceModel()->data( right, Qt::DisplayRole ).toString();
  if ( leftText.isEmpty() )
    return true;
  if ( rightText.isEmpty() )
    return false;

  //sort by item id
  const QgsLayoutItem *item1 = itemFromSourceIndex( left );
  const QgsLayoutItem *item2 = itemFromSourceIndex( right );
  if ( !item1 )
    return false;

  if ( !item2 )
    return true;

  return QString::localeAwareCompare( item1->displayName(), item2->displayName() ) < 0;
}

QgsLayoutItem *QgsLayoutProxyModel::itemFromSourceIndex( const QModelIndex &sourceIndex ) const
{
  if ( !mLayout )
    return nullptr;

  //get column corresponding to an index from the source model
  QVariant itemAsVariant = sourceModel()->data( sourceIndex, Qt::UserRole + 1 );
  return qobject_cast<QgsLayoutItem *>( itemAsVariant.value<QObject *>() );
}

void QgsLayoutProxyModel::setAllowEmptyItem( bool allowEmpty )
{
  mAllowEmpty = allowEmpty;
  invalidateFilter();
}

bool QgsLayoutProxyModel::allowEmptyItem() const
{
  return mAllowEmpty;
}

void QgsLayoutProxyModel::setItemFlags( QgsLayoutItem::Flags flags )
{
  mItemFlags = flags;
  invalidateFilter();
}

QgsLayoutItem::Flags QgsLayoutProxyModel::itemFlags() const
{
  return mItemFlags;
}

void QgsLayoutProxyModel::setFilterType( QgsLayoutItemRegistry::ItemType filter )
{
  mItemTypeFilter = filter;
  invalidate();
}

void QgsLayoutProxyModel::setExceptedItemList( const QList< QgsLayoutItem *> &items )
{
  if ( mExceptedList == items )
    return;

  mExceptedList = items;
  invalidateFilter();
}

bool QgsLayoutProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  //get QgsComposerItem corresponding to row
  QModelIndex index = sourceModel()->index( sourceRow, 0, sourceParent );
  QgsLayoutItem *item = itemFromSourceIndex( index );

  if ( !item )
    return mAllowEmpty;

  // specific exceptions
  if ( mExceptedList.contains( item ) )
    return false;

  // filter by type
  if ( mItemTypeFilter != QgsLayoutItemRegistry::LayoutItem && item->type() != mItemTypeFilter )
    return false;

  if ( mItemFlags && !( item->itemFlags() & mItemFlags ) )
  {
    return false;
  }

  return true;
}
