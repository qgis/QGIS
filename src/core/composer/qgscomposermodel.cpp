/***************************************************************************
                         qgscomposermodel.cpp
                         -----------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgsapplication.h"
#include "qgscomposermodel.h"
#include "qgscomposition.h"
#include "qgscomposeritem.h"
#include "qgspaperitem.h"
#include "qgslogger.h"
#include <QApplication>
#include <QGraphicsItem>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>
#include <QSettings>
#include <QMessageBox>
#include <QIcon>

QgsComposerModel::QgsComposerModel( QgsComposition* composition , QObject *parent )
    : QAbstractItemModel( parent )
    , mComposition( composition )
{

}

QgsComposerModel::~QgsComposerModel()
{
}

QgsComposerItem* QgsComposerModel::itemFromIndex( const QModelIndex &index ) const
{
  //try to return the QgsComposerItem corresponding to a QModelIndex
  if ( !index.isValid() )
  {
    return 0;
  }

  QgsComposerItem * item = static_cast<QgsComposerItem*>( index.internalPointer() );
  return item;
}

QModelIndex QgsComposerModel::index( int row, int column,
                                     const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() )
  {
    //column out of bounds
    return QModelIndex();
  }

  if ( !parent.isValid() && row >= 0 && row < mItemsInScene.size() )
  {
    //return an index for the composer item at this position
    return createIndex( row, column, mItemsInScene.at( row ) );
  }

  //only top level supported for now
  return QModelIndex();
}

void QgsComposerModel::refreshItemsInScene()
{
  mItemsInScene.clear();

  //filter deleted and paper items from list
  //TODO - correctly handle grouped item z order placement
  QList<QgsComposerItem *>::const_iterator itemIt = mItemZList.constBegin();
  for ( ; itemIt != mItemZList.constEnd(); ++itemIt )
  {
    if ((( *itemIt )->type() != QgsComposerItem::ComposerPaper ) && !( *itemIt )->isRemoved() )
    {
      mItemsInScene.push_back(( *itemIt ) );
    }
  }
}

QModelIndex QgsComposerModel::parent( const QModelIndex &index ) const
{
  Q_UNUSED( index );

  //all items are top level for now
  return QModelIndex();
}

int QgsComposerModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
  {
    return mItemsInScene.size();
  }

  QGraphicsItem * parentItem = itemFromIndex( parent );

  if ( !parentItem )
  {
    return mItemsInScene.size();
  }
  else
  {
    //no children for now
    return 0;
  }
}

int QgsComposerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 3;
}

QVariant QgsComposerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsComposerItem *item = itemFromIndex( index );
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

    case Qt::TextAlignmentRole:
      return Qt::AlignLeft & Qt::AlignVCenter;

    case Qt::CheckStateRole:
      switch ( index.column() )
      {
        case Visibility:
          //column 0 is visibility of item
          return item->isVisible() ? Qt::Checked : Qt::Unchecked;
        case LockStatus:
          //column 1 is locked state of item
          return item->positionLock() ? Qt::Checked : Qt::Unchecked;
        default:
          return QVariant();
      }

    case Qt::FontRole:
      if ( index.column() == ItemId && item->isSelected() )
      {
        //draw name of selected items in bold
        QFont boldFont;
        boldFont.setBold( true );
        return boldFont;
      }
      else
      {
        return QVariant();
      }
      break;

    default:
      return QVariant();
  }
}

bool QgsComposerModel::setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole )
{
  Q_UNUSED( role );

  if ( !index.isValid() )
    return false;

  QgsComposerItem *item = itemFromIndex( index );
  if ( !item )
  {
    return false;
  }

  switch ( index.column() )
  {
    case Visibility:
      //first column is item visibility
      item->setVisibility( value.toBool() );
      emit dataChanged( index, index );
      return true;

    case LockStatus:
      //second column is item lock state
      item->setPositionLock( value.toBool() );
      emit dataChanged( index, index );
      return true;

    case ItemId:
      //last column is item id
      item->setId( value.toString() );
      emit dataChanged( index, index );
      return true;
  }

  return false;
}

QVariant QgsComposerModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  static QIcon lockIcon;
  if ( lockIcon.isNull() )
    lockIcon = QgsApplication::getThemeIcon( "/locked.svg" );
  static QIcon showIcon;
  if ( showIcon.isNull() )
    showIcon = QgsApplication::getThemeIcon( "/mActionShowAllLayers.png" );

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( section == ItemId )
      {
        return tr( "Item" );
      }
      else
      {
        return QVariant();
      }
      break;
    }

    case Qt::DecorationRole:
    {
      if ( section == Visibility )
      {
        return qVariantFromValue( showIcon );
      }
      else if ( section == LockStatus )
      {
        return qVariantFromValue( lockIcon );
      }
      else
      {
        return QVariant();
      }
      break;
    }

    case Qt::TextAlignmentRole:
      return Qt::AlignLeft & Qt::AlignVCenter;

    default:
      return QAbstractItemModel::headerData( section, orientation, role );
  }

}

void QgsComposerModel::clear()
{
  //totally reset model
  beginResetModel();
  mItemZList.clear();
  refreshItemsInScene();
  endResetModel();
}

int QgsComposerModel::zOrderListSize() const
{
  return mItemZList.size();
}

void QgsComposerModel::rebuildZList()
{
  QList<QgsComposerItem*> sortedList;
  //rebuild the item z order list based on the current zValues of items in the scene

  //get items in descending zValue order
  QList<QGraphicsItem*> itemList = mComposition->items( Qt::DescendingOrder );
  QList<QGraphicsItem*>::iterator itemIt = itemList.begin();
  for ( ; itemIt != itemList.end(); ++itemIt )
  {
    QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem*>( *itemIt );
    if ( composerItem )
    {
      if ( composerItem->type() != QgsComposerItem::ComposerPaper )
      {
        sortedList.append( composerItem );
      }
    }
  }

  mItemZList = sortedList;
  rebuildSceneItemList();
}

void QgsComposerModel::rebuildSceneItemList()
{
  //step through the z list and rebuild the items in scene list,
  //emitting signals as required
  QList<QgsComposerItem*>::iterator zListIt = mItemZList.begin();
  int row = 0;
  for ( ; zListIt != mItemZList.end(); ++zListIt )
  {
    if ((( *zListIt )->type() == QgsComposerItem::ComposerPaper ) || ( *zListIt )->isRemoved() )
    {
      //item not in scene, skip it
      continue;
    }

    int sceneListPos = mItemsInScene.indexOf( *zListIt );
    if ( sceneListPos == row )
    {
      //already in list in correct position, nothing to do

    }
    else if ( sceneListPos != -1 )
    {
      //in list, but in wrong spot
      beginMoveRows( QModelIndex(), sceneListPos, sceneListPos, QModelIndex(), row );
      mItemsInScene.removeAt( sceneListPos );
      mItemsInScene.insert( row, *zListIt );
      endMoveRows();
    }
    else
    {
      //needs to be inserted into list
      beginInsertRows( QModelIndex(), row, row );
      mItemsInScene.insert( row, *zListIt );
      endInsertRows();
    }
    row++;
  }
}

void QgsComposerModel::addItemAtTop( QgsComposerItem *item )
{
  beginInsertRows( QModelIndex(), 0, 0 );
  mItemZList.push_front( item );
  refreshItemsInScene();
  item->setZValue( mItemZList.size() );
  endInsertRows();
}

void QgsComposerModel::removeItem( QgsComposerItem * item )
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
    //removing an item not in the scene (eg, deleted item)
    //we need to remove it from the list, but don't need to call
    //beginRemoveRows or endRemoveRows since the item was not used by the model
    mItemZList.removeAt( pos );
    refreshItemsInScene();
    return;
  }

  //remove item from model
  int row = itemIndex.row();
  beginRemoveRows( QModelIndex() , row, row );
  mItemZList.removeAt( pos );
  refreshItemsInScene();
  endRemoveRows();
}

void QgsComposerModel::setItemRemoved( QgsComposerItem *item )
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
  beginRemoveRows( QModelIndex() , row, row );
  item->setIsRemoved( true );
  refreshItemsInScene();
  endRemoveRows();
}

void QgsComposerModel::setItemRestored( QgsComposerItem *item )
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

  item->setIsRemoved( false );
  rebuildSceneItemList();
}

void QgsComposerModel::updateItemDisplayName( QgsComposerItem *item )
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

void QgsComposerModel::updateItemLockStatus( QgsComposerItem *item )
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

void QgsComposerModel::updateItemVisibility( QgsComposerItem *item )
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

void QgsComposerModel::updateItemSelectStatus( QgsComposerItem *item )
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

bool QgsComposerModel::reorderItemUp( QgsComposerItem *item )
{
  if ( mItemsInScene.first() == item )
  {
    //item is already topmost item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsComposerItem*> it( mItemZList );
  if ( ! it.findNext( item ) )
  {
    //can't find item in z list, nothing to do
    return false;
  }

  it.remove();
  while ( it.hasPrevious() )
  {
    //search through item z list to find previous item which is present in the scene
    //(deleted items still exist in the z list so that they can be restored to their correct stacking order,
    //but since they are not in the scene they should be ignored here)
    it.previous();
    if ( it.value() && !( it.value()->isRemoved() ) )
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

bool QgsComposerModel::reorderItemDown( QgsComposerItem *item )
{
  if ( mItemsInScene.last() == item )
  {
    //item is already lowest item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsComposerItem*> it( mItemZList );
  if ( ! it.findNext( item ) )
  {
    //can't find item in z list, nothing to do
    return false;
  }

  it.remove();
  while ( it.hasNext() )
  {
    //search through item z list to find next item which is present in the scene
    //(deleted items still exist in the z list so that they can be restored to their correct stacking order,
    //but since they are not in the scene they should be ignored here)
    it.next();
    if ( it.value() && !( it.value()->isRemoved() ) )
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

bool QgsComposerModel::reorderItemToTop( QgsComposerItem *item )
{
  if ( mItemsInScene.first() == item )
  {
    //item is already topmost item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsComposerItem*> it( mItemZList );
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
  beginMoveRows( QModelIndex(), row, row, QModelIndex(), 0 );
  refreshItemsInScene();
  endMoveRows();
  return true;
}

bool QgsComposerModel::reorderItemToBottom( QgsComposerItem *item )
{
  if ( mItemsInScene.last() == item )
  {
    //item is already lowest item present in scene, nothing to do
    return false;
  }

  //move item in z list
  QMutableListIterator<QgsComposerItem*> it( mItemZList );
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

QgsComposerItem* QgsComposerModel::getComposerItemAbove( QgsComposerItem* item ) const
{
  //search item z list for selected item
  QListIterator<QgsComposerItem*> it( mItemZList );
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
  return 0;
}

QgsComposerItem* QgsComposerModel::getComposerItemBelow( QgsComposerItem* item ) const
{
  //search item z list for selected item
  QListIterator<QgsComposerItem*> it( mItemZList );
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
  return 0;
}

QList<QgsComposerItem *>* QgsComposerModel::zOrderList()
{
  return &mItemZList;
}


Qt::ItemFlags QgsComposerModel::flags( const QModelIndex & index ) const
{
  if ( ! index.isValid() )
  {
    return 0;
  }

  switch ( index.column() )
  {
    case Visibility:
    case LockStatus:
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    case ItemId:
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    default:
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

QModelIndex QgsComposerModel::indexForItem( QgsComposerItem *item , const int column )
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

  return index( row, column );
}

void QgsComposerModel::setSelected( const QModelIndex &index )
{
  QgsComposerItem *item = itemFromIndex( index );
  if ( !item )
  {
    return;
  }

  mComposition->setSelectedItem( item );
}
