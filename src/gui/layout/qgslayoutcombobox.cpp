/***************************************************************************
   qgslayoutcombobox.cpp
    --------------------------------------
   Date                 : March 2019
   Copyright            : (C) 2019 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include "qgslayoutcombobox.h"
#include "qgslayoutmodel.h"

QgsLayoutComboBox::QgsLayoutComboBox( QWidget *parent, QgsLayoutManager *manager )
  : QComboBox( parent )
{
  setLayoutManager( manager );

  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutComboBox::indexChanged );
}

void QgsLayoutComboBox::setLayoutManager( QgsLayoutManager *manager )
{
  if ( mModel )
    mModel->deleteLater();
  if ( mProxyModel )
    mProxyModel->deleteLater();

  mModel = new QgsLayoutManagerModel( manager, this );
  mProxyModel = new QgsLayoutManagerProxyModel( this );
  mProxyModel->setSourceModel( mModel );

  connect( mProxyModel, &QAbstractItemModel::rowsInserted, this, &QgsLayoutComboBox::rowsChanged );
  connect( mProxyModel, &QAbstractItemModel::rowsRemoved, this, &QgsLayoutComboBox::rowsChanged );
  setModel( mProxyModel );
  mProxyModel->sort( 0, Qt::AscendingOrder );
}

QgsLayoutManagerProxyModel::Filters QgsLayoutComboBox::filters() const
{
  return mProxyModel->filters();
}

void QgsLayoutComboBox::setFilters( QgsLayoutManagerProxyModel::Filters filters )
{
  mProxyModel->setFilters( filters );
}

void QgsLayoutComboBox::setAllowEmptyLayout( bool allowEmpty )
{
  mModel->setAllowEmptyLayout( allowEmpty );
}

bool QgsLayoutComboBox::allowEmptyLayout() const
{
  return mModel->allowEmptyLayout();
}

void QgsLayoutComboBox::setCurrentLayout( QgsMasterLayoutInterface *layout )
{
  if ( !mModel )
    return;

  const QModelIndex idx = mModel->indexFromLayout( layout );
  if ( idx.isValid() )
  {
    const QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      return;
    }
  }
  setCurrentIndex( allowEmptyLayout() ? 0 : -1 );
}

QgsMasterLayoutInterface *QgsLayoutComboBox::currentLayout() const
{
  return layout( currentIndex() );
}

void QgsLayoutComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  emit layoutChanged( currentLayout() );
}

void QgsLayoutComboBox::rowsChanged()
{
  if ( count() == 1 )
  {
    //currently selected item has changed
    emit layoutChanged( currentLayout() );
  }
  else if ( count() == 0 )
  {
    emit layoutChanged( nullptr );
  }
}

QgsMasterLayoutInterface *QgsLayoutComboBox::layout( int index ) const
{
  const QModelIndex proxyIndex = mProxyModel->index( index, 0 );
  if ( !proxyIndex.isValid() )
  {
    return nullptr;
  }

  const QModelIndex sourceIndex = mProxyModel->mapToSource( proxyIndex );
  if ( !sourceIndex.isValid() )
  {
    return nullptr;
  }

  return mModel->layoutFromIndex( sourceIndex );
}
