/***************************************************************************
   qgsmaplayercombobox.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaplayercombobox.h"
#include "qgsmaplayermodel.h"


QgsMapLayerComboBox::QgsMapLayerComboBox( QWidget *parent )
  : QComboBox( parent )
{
  mProxyModel = new QgsMapLayerProxyModel( this );
  setModel( mProxyModel );

  connect( this, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsMapLayerComboBox::indexChanged );
  connect( mProxyModel, &QAbstractItemModel::rowsInserted, this, &QgsMapLayerComboBox::rowsChanged );
  connect( mProxyModel, &QAbstractItemModel::rowsRemoved, this, &QgsMapLayerComboBox::rowsChanged );
}

void QgsMapLayerComboBox::setExcludedProviders( const QStringList &providers )
{
  mProxyModel->setExcludedProviders( providers );
}

QStringList QgsMapLayerComboBox::excludedProviders() const
{
  return mProxyModel->excludedProviders();
}

void QgsMapLayerComboBox::setAllowEmptyLayer( bool allowEmpty )
{
  mProxyModel->sourceLayerModel()->setAllowEmptyLayer( allowEmpty );
}

bool QgsMapLayerComboBox::allowEmptyLayer() const
{
  return mProxyModel->sourceLayerModel()->allowEmptyLayer();
}

void QgsMapLayerComboBox::setShowCrs( bool showCrs )
{
  mProxyModel->sourceLayerModel()->setShowCrs( showCrs );
}

bool QgsMapLayerComboBox::showCrs() const
{
  return mProxyModel->sourceLayerModel()->showCrs();
}

void QgsMapLayerComboBox::setAdditionalItems( const QStringList &items )
{
  mProxyModel->sourceLayerModel()->setAdditionalItems( items );
}

QStringList QgsMapLayerComboBox::additionalItems() const
{
  return mProxyModel->sourceLayerModel()->additionalItems();
}

void QgsMapLayerComboBox::setLayer( QgsMapLayer *layer )
{
  if ( !layer )
  {
    setCurrentIndex( -1 );
    return;
  }

  QModelIndex idx = mProxyModel->sourceLayerModel()->indexFromLayer( layer );
  if ( idx.isValid() )
  {
    QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      emit layerChanged( currentLayer() );
      return;
    }
  }
  setCurrentIndex( -1 );
  emit layerChanged( currentLayer() );
}

QgsMapLayer *QgsMapLayerComboBox::currentLayer() const
{
  return layer( currentIndex() );
}

QgsMapLayer *QgsMapLayerComboBox::layer( int layerIndex ) const
{
  const QModelIndex proxyIndex = mProxyModel->index( layerIndex, 0 );
  if ( !proxyIndex.isValid() )
  {
    return nullptr;
  }

  const QModelIndex index = mProxyModel->mapToSource( proxyIndex );
  if ( !index.isValid() )
  {
    return nullptr;
  }

  QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
  if ( layer )
  {
    return layer;
  }
  return nullptr;
}

void QgsMapLayerComboBox::indexChanged( int i )
{
  Q_UNUSED( i );
  QgsMapLayer *layer = currentLayer();
  emit layerChanged( layer );
}

void QgsMapLayerComboBox::rowsChanged()
{
  if ( count() == 1 )
  {
    //currently selected layer item has changed
    emit layerChanged( currentLayer() );
  }
  else if ( count() == 0 )
  {
    emit layerChanged( nullptr );
  }
}

