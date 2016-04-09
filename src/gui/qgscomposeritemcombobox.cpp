/***************************************************************************
   qgscomposeritemcombobox.cpp
    --------------------------------------
   Date                 : August 2014
   Copyright            : (C) 2014 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgscomposeritemcombobox.h"
#include "qgscomposermodel.h"

QgsComposerItemComboBox::QgsComposerItemComboBox( QWidget *parent, QgsComposition* composition )
    : QComboBox( parent )
    , mProxyModel( nullptr )
{
  setComposition( composition );

  setModelColumn( QgsComposerModel::ItemId );
  connect( this, SIGNAL( activated( int ) ), this, SLOT( indexChanged( int ) ) );
  connect( mProxyModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ), this, SLOT( rowsChanged() ) );
  connect( mProxyModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( rowsChanged() ) );
}

void QgsComposerItemComboBox::setComposition( QgsComposition *composition )
{
  delete mProxyModel;
  mProxyModel = new QgsComposerProxyModel( composition, this );
  connect( mProxyModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ), this, SLOT( rowsChanged() ) );
  connect( mProxyModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ), this, SLOT( rowsChanged() ) );
  setModel( mProxyModel );
  setModelColumn( QgsComposerModel::ItemId );
  mProxyModel->sort( 0, Qt::AscendingOrder );
}

void QgsComposerItemComboBox::setItem( const QgsComposerItem* item )
{
  if ( !mProxyModel->sourceLayerModel() )
    return;

  QModelIndex idx = mProxyModel->sourceLayerModel()->indexForItem( const_cast< QgsComposerItem* >( item ) );
  if ( idx.isValid() )
  {
    QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      emit itemChanged( currentItem() );
      return;
    }
  }
  setCurrentIndex( -1 );
  emit itemChanged( currentItem() );
}

QgsComposerItem* QgsComposerItemComboBox::currentItem() const
{
  return item( currentIndex() );
}

void QgsComposerItemComboBox::indexChanged( int i )
{
  Q_UNUSED( i );
  emit itemChanged( currentItem() );
}

void QgsComposerItemComboBox::rowsChanged()
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

void QgsComposerItemComboBox::setItemType( QgsComposerItem::ItemType itemType )
{
  mProxyModel->setFilterType( itemType );
}

QgsComposerItem::ItemType QgsComposerItemComboBox::itemType() const
{
  return mProxyModel->filterType();
}

void QgsComposerItemComboBox::setExceptedItemList( const QList< QgsComposerItem*>& exceptList )
{
  mProxyModel->setExceptedItemList( exceptList );
}

QList< QgsComposerItem*> QgsComposerItemComboBox::exceptedItemList() const
{
  return mProxyModel->exceptedItemList();
}
QgsComposerItem* QgsComposerItemComboBox::item( int index ) const
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
