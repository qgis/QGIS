/***************************************************************************
  qgslayertreefilterproxymodel.cpp

 ---------------------
 begin                : 05.06.2020
 copyright            : (C) 2020 by Denis Rouzaud
 email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreefilterproxymodel.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgssymbol.h"

QgsLayerTreeFilterProxyModel::QgsLayerTreeFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
  connect( QgsProject::instance(), &QgsProject::readProject, this, [ = ]
  {
    beginResetModel();
    endResetModel();
  } );
}

void QgsLayerTreeFilterProxyModel::setCheckedLayers( const QList<QgsMapLayer *> layers )
{
  // do not use invalidate() since it's not the filter which changes but the data
  beginResetModel();
  mCheckedLayers = layers;
  endResetModel();
}

int QgsLayerTreeFilterProxyModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

Qt::ItemFlags QgsLayerTreeFilterProxyModel::flags( const QModelIndex &idx ) const
{
  if ( idx.column() == 0 )
  {
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  }
  return Qt::NoItemFlags;
}

QModelIndex QgsLayerTreeFilterProxyModel::index( int row, int column, const QModelIndex &parent ) const
{
  QModelIndex newIndex = QSortFilterProxyModel::index( row, 0, parent );
  if ( column == 0 )
    return newIndex;

  return createIndex( row, column, newIndex.internalId() );
}

QModelIndex QgsLayerTreeFilterProxyModel::parent( const QModelIndex &child ) const
{
  return QSortFilterProxyModel::parent( createIndex( child.row(), 0, child.internalId() ) );
}

QModelIndex QgsLayerTreeFilterProxyModel::sibling( int row, int column, const QModelIndex &idx ) const
{
  const QModelIndex parent = idx.parent();
  return index( row, column, parent );
}

QgsMapLayer *QgsLayerTreeFilterProxyModel::mapLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *node = nullptr;
  if ( idx.column() == 0 )
  {
    node = mLayerTreeModel->index2node( mapToSource( idx ) );
  }

  if ( !node || !QgsLayerTree::isLayer( node ) )
    return nullptr;

  return QgsLayerTree::toLayer( node )->layer();
}

void QgsLayerTreeFilterProxyModel::setFilterText( const QString &filterText )
{
  if ( filterText == mFilterText )
    return;

  mFilterText = filterText;
  invalidateFilter();
}

QgsLayerTreeModel *QgsLayerTreeFilterProxyModel::layerTreeModel() const
{
  return mLayerTreeModel;
}

void QgsLayerTreeFilterProxyModel::setLayerTreeModel( QgsLayerTreeModel *layerTreeModel )
{
  mLayerTreeModel = layerTreeModel;
  QSortFilterProxyModel::setSourceModel( layerTreeModel );
}

void QgsLayerTreeFilterProxyModel::setFilters( const QgsMapLayerProxyModel::Filters &filters )
{
  mFilters = filters;
  invalidateFilter();
}

bool QgsLayerTreeFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayerTreeNode *node = mLayerTreeModel->index2node( mLayerTreeModel->index( sourceRow, 0, sourceParent ) );
  return nodeShown( node );
}

bool QgsLayerTreeFilterProxyModel::isLayerChecked( QgsMapLayer *layer ) const
{
  return mCheckedLayers.contains( layer );
}

void QgsLayerTreeFilterProxyModel::setLayerChecked( QgsMapLayer *layer, bool checked )
{
  if ( checked )
  {
    mCheckedLayers << layer;
  }
  else
  {
    mCheckedLayers.removeAll( layer );
  }
}

void QgsLayerTreeFilterProxyModel::setLayerCheckedPrivate( QgsMapLayer *layer, bool checked )
{
  if ( checked && isLayerChecked( layer ) )
    return;
  if ( !checked && !isLayerChecked( layer ) )
    return;

  QgsLayerTreeNode *node = mLayerTreeModel->rootGroup()->findLayer( layer );
  const QModelIndex index = mapFromSource( mLayerTreeModel->node2index( node ) );

  setLayerChecked( layer, checked );

  emit dataChanged( index, index );
}

bool QgsLayerTreeFilterProxyModel::layerShown( QgsMapLayer *layer ) const
{
  Q_UNUSED( layer )
  return true;
}

bool QgsLayerTreeFilterProxyModel::nodeShown( QgsLayerTreeNode *node ) const
{
  if ( !node )
    return false;
  if ( node->nodeType() == QgsLayerTreeNode::NodeGroup )
  {
    const auto constChildren = node->children();
    for ( QgsLayerTreeNode *child : constChildren )
    {
      if ( nodeShown( child ) )
      {
        return true;
      }
    }
    return false;
  }
  else
  {
    QgsMapLayer *layer = QgsLayerTree::toLayer( node )->layer();
    if ( !layer )
      return false;
    if ( !mFilterText.isEmpty() && !layer->name().contains( mFilterText, Qt::CaseInsensitive ) )
      return false;
    if ( !QgsMapLayerProxyModel::layerMatchesFilters( layer, mFilters ) )
      return false;

    return layerShown( layer );
  }
}

QVariant QgsLayerTreeFilterProxyModel::data( const QModelIndex &idx, int role ) const
{
  if ( idx.column() == 0 )
  {
    if ( role == Qt::CheckStateRole )
    {
      QgsMapLayer *layer = mapLayer( idx );
      if ( layer )
      {
        if ( isLayerChecked( layer ) )
        {
          return Qt::Checked;
        }
        else
        {
          return Qt::Unchecked;
        }
      }
      else
      {
        // i.e. this is a group, analyze its children
        bool hasChecked = false, hasUnchecked = false;
        int n;
        for ( n = 0; !hasChecked || !hasUnchecked; n++ )
        {
          const QVariant v = data( index( n, 0, idx ), role );
          if ( !v.isValid() )
            break;

          switch ( v.toInt() )
          {
            case Qt::PartiallyChecked:
              // parent of partially checked child shared state
              return Qt::PartiallyChecked;

            case Qt::Checked:
              hasChecked = true;
              break;

            case Qt::Unchecked:
              hasUnchecked = true;
              break;
          }
        }

        // unchecked leaf
        if ( n == 0 )
          return Qt::Unchecked;

        // both
        if ( hasChecked &&  hasUnchecked )
          return Qt::PartiallyChecked;

        if ( hasChecked )
          return Qt::Checked;

        Q_ASSERT( hasUnchecked );
        return Qt::Unchecked;
      }
    }
    else
    {
      return mLayerTreeModel->data( mapToSource( idx ), role );
    }
  }
  return QVariant();
}

bool QgsLayerTreeFilterProxyModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( index.column() == 0 )
  {
    if ( role == Qt::CheckStateRole )
    {
      int i = 0;
      for ( i = 0; ; i++ )
      {
        const QModelIndex child = QgsLayerTreeFilterProxyModel::index( i, 0, index );
        if ( !child.isValid() )
          break;

        setData( child, value, role );
      }

      if ( i == 0 )
      {
        QgsMapLayer *layer = mapLayer( index );
        if ( !layer )
        {
          return false;
        }
        if ( value.toInt() == Qt::Checked )
          setLayerCheckedPrivate( layer, true );
        else if ( value.toInt() == Qt::Unchecked )
          setLayerCheckedPrivate( layer, false );
        else
          Q_ASSERT( false ); // expected checked or unchecked
      }
      emit dataChanged( index, index );
      return true;
    }

    return mLayerTreeModel->setData( mapToSource( index ), value, role );
  }

  return false;
}
