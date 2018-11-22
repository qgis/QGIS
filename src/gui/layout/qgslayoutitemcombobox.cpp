/***************************************************************************
   qgslayoutitemcombobox.cpp
    --------------------------------------
   Date                 : October 2017
   Copyright            : (C) 201\7 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgslayoutitemcombobox.h"
#include "qgslayoutmodel.h"

QgsLayoutItemComboBox::QgsLayoutItemComboBox( QWidget *parent, QgsLayout *layout )
  : QComboBox( parent )
{
  setCurrentLayout( layout );

  setModelColumn( QgsLayoutModel::ItemId );
  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutItemComboBox::indexChanged );
}

void QgsLayoutItemComboBox::setCurrentLayout( QgsLayout *layout )
{
  mProxyModel = qgis::make_unique< QgsLayoutProxyModel >( layout, this );
  connect( mProxyModel.get(), &QAbstractItemModel::rowsInserted, this, &QgsLayoutItemComboBox::rowsChanged );
  connect( mProxyModel.get(), &QAbstractItemModel::rowsRemoved, this, &QgsLayoutItemComboBox::rowsChanged );
  setModel( mProxyModel.get() );
  setModelColumn( QgsLayoutModel::ItemId );
  mProxyModel->sort( 0, Qt::AscendingOrder );
}

void QgsLayoutItemComboBox::setItem( const QgsLayoutItem *item )
{
  if ( !mProxyModel->sourceLayerModel() )
    return;

  QModelIndex idx = mProxyModel->sourceLayerModel()->indexForItem( const_cast< QgsLayoutItem * >( item ) );
  if ( idx.isValid() )
  {
    QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      return;
    }
  }
  setCurrentIndex( -1 );
}

QgsLayoutItem *QgsLayoutItemComboBox::currentItem() const
{
  return item( currentIndex() );
}

void QgsLayoutItemComboBox::indexChanged( int i )
{
  Q_UNUSED( i );
  emit itemChanged( currentItem() );
}

void QgsLayoutItemComboBox::rowsChanged()
{
  if ( count() == 1 )
  {
    //currently selected item has changed
    emit itemChanged( currentItem() );
  }
  else if ( count() == 0 )
  {
    emit itemChanged( nullptr );
  }
}

void QgsLayoutItemComboBox::setItemType( QgsLayoutItemRegistry::ItemType itemType )
{
  mProxyModel->setFilterType( itemType );
}

QgsLayoutItemRegistry::ItemType QgsLayoutItemComboBox::itemType() const
{
  return mProxyModel->filterType();
}

void QgsLayoutItemComboBox::setExceptedItemList( const QList<QgsLayoutItem *> &exceptList )
{
  mProxyModel->setExceptedItemList( exceptList );
}

QList< QgsLayoutItem *> QgsLayoutItemComboBox::exceptedItemList() const
{
  return mProxyModel->exceptedItemList();
}

QgsLayoutItem *QgsLayoutItemComboBox::item( int index ) const
{
  const QModelIndex proxyIndex = mProxyModel->index( index, 0 );
  if ( !proxyIndex.isValid() )
  {
    return nullptr;
  }

  QModelIndex sourceIndex = mProxyModel->mapToSource( proxyIndex );
  if ( !sourceIndex.isValid() )
  {
    return nullptr;
  }

  return mProxyModel->itemFromSourceIndex( sourceIndex );
}
