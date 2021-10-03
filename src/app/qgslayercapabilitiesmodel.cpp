/***************************************************************************
                        qgslayercapabilitiesmodel.h
                        ----------------------------
   begin                : August 2018
   copyright            : (C) 2018 by Denis Rouzaud
   email                : denis@opengis.ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayercapabilitiesmodel.h"

#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

QgsLayerCapabilitiesModel::QgsLayerCapabilitiesModel( QgsProject *project, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  const QMap<QString, QgsMapLayer *> &mapLayers = project->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    mReadOnlyLayers.insert( it.value(), it.value()->readOnly() );
    mSearchableLayers.insert( it.value(), it.value()->type() == QgsMapLayerType::VectorLayer && it.value()->flags().testFlag( QgsMapLayer::Searchable ) );
    mIdentifiableLayers.insert( it.value(), it.value()->flags().testFlag( QgsMapLayer::Identifiable ) );
    mRemovableLayers.insert( it.value(), it.value()->flags().testFlag( QgsMapLayer::Removable ) );
    mPrivateLayers.insert( it.value(), it.value()->flags().testFlag( QgsMapLayer::Private ) );
  }
}


QgsLayerTreeModel *QgsLayerCapabilitiesModel::layerTreeModel() const
{
  return mLayerTreeModel;
}

void QgsLayerCapabilitiesModel::setLayerTreeModel( QgsLayerTreeModel *layerTreeModel )
{
  mLayerTreeModel = layerTreeModel;
  QSortFilterProxyModel::setSourceModel( layerTreeModel );
}

void QgsLayerCapabilitiesModel::setFilterText( const QString &filterText )
{
  if ( filterText == mFilterText )
    return;

  mFilterText = filterText;
  invalidateFilter();
}

void QgsLayerCapabilitiesModel::toggleSelectedItems( const QModelIndexList &checkedIndexes )
{
  for ( const QModelIndex &index : checkedIndexes )
  {
    const bool isChecked = data( index, Qt::CheckStateRole ) == Qt::Checked;
    if ( setData( index, isChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole ) )
      emit dataChanged( index, index );
  }
}

bool QgsLayerCapabilitiesModel::identifiable( QgsMapLayer *layer ) const
{
  return mIdentifiableLayers.value( layer, true );
}

bool QgsLayerCapabilitiesModel::removable( QgsMapLayer *layer ) const
{
  return mRemovableLayers.value( layer, true );
}

bool QgsLayerCapabilitiesModel::privateLayer( QgsMapLayer *layer ) const
{
  return mPrivateLayers.value( layer, true );
}

bool QgsLayerCapabilitiesModel::readOnly( QgsMapLayer *layer ) const
{
  return mReadOnlyLayers.value( layer, true );
}

bool QgsLayerCapabilitiesModel::searchable( QgsMapLayer *layer ) const
{
  return mSearchableLayers.value( layer, false );
}

int QgsLayerCapabilitiesModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 6;
}

QVariant QgsLayerCapabilitiesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    if ( role == Qt::DisplayRole )
    {
      switch ( section )
      {
        case LayerColumn:
          return tr( "Layer" );
        case IdentifiableColumn:
          return tr( "Identifiable" );
        case ReadOnlyColumn:
          return tr( "Read-only" );
        case SearchableColumn:
          return tr( "Searchable" );
        case RequiredColumn:
          return tr( "Required" );
        case PrivateColumn:
          return tr( "Private" );
        default:
          return QVariant();
      }
    }
    if ( role == Qt::ToolTipRole )
    {
      switch ( section )
      {
        case LayerColumn:
          return QVariant();
        case IdentifiableColumn:
          return tr( "Layers which can be queried with the \"Identify features\" tool." );
        case ReadOnlyColumn:
          return tr( "Layers which are protected from edit." );
        case SearchableColumn:
          return tr( "Layers which can be queried with the locator search tool." );
        case RequiredColumn:
          return tr( "Layers which are protected from inadvertent removal from the project." );
        case PrivateColumn:
          return tr( "Layers which are not shown in the legend." );
        default:
          return QVariant();
      }
    }
  }
  return mLayerTreeModel->headerData( section, orientation, role );
}

Qt::ItemFlags QgsLayerCapabilitiesModel::flags( const QModelIndex &idx ) const
{
  QgsMapLayer *layer = mapLayer( idx );
  if ( !layer && idx.column() != LayerColumn )
  {
    return Qt::NoItemFlags;
  }
  else
  {
    switch ( static_cast<Columns>( idx.column() ) )
    {
      case LayerColumn:
      {
        return Qt::ItemIsEnabled;
      }
      case IdentifiableColumn:
      {
        if ( layer->isSpatial() )
        {
          return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        }
        else
        {
          return Qt::ItemFlags();
        }
      }
      case ReadOnlyColumn:
      {
        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        }
        else
        {
          return Qt::ItemFlags();
        }
      }
      case SearchableColumn:
      {
        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        }
        else
        {
          return Qt::ItemFlags();
        }
      }
      case RequiredColumn:
      {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
      }
      case PrivateColumn:
      {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
      }
    }
  }
  return Qt::ItemFlags();
}

QgsMapLayer *QgsLayerCapabilitiesModel::mapLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *node = nullptr;
  if ( idx.column() == LayerColumn )
  {
    node = mLayerTreeModel->index2node( mapToSource( idx ) );
  }
  else
  {
    node = mLayerTreeModel->index2node( mapToSource( index( idx.row(), LayerColumn, idx.parent() ) ) );
  }

  if ( !node || !QgsLayerTree::isLayer( node ) )
    return nullptr;

  return QgsLayerTree::toLayer( node )->layer();
}

void QgsLayerCapabilitiesModel::setShowSpatialLayersOnly( bool only )
{
  if ( only == mShowSpatialLayersOnly )
    return;

  mShowSpatialLayersOnly = only;
  invalidateFilter();
}

QModelIndex QgsLayerCapabilitiesModel::index( int row, int column, const QModelIndex &parent ) const
{
  QModelIndex newIndex = QSortFilterProxyModel::index( row, LayerColumn, parent );
  if ( column == LayerColumn )
    return newIndex;

  return createIndex( row, column, newIndex.internalId() );
}

QModelIndex QgsLayerCapabilitiesModel::parent( const QModelIndex &child ) const
{
  return QSortFilterProxyModel::parent( createIndex( child.row(), LayerColumn, child.internalId() ) );
}

QModelIndex QgsLayerCapabilitiesModel::sibling( int row, int column, const QModelIndex &idx ) const
{
  const QModelIndex parent = idx.parent();
  return index( row, column, parent );
}

QVariant QgsLayerCapabilitiesModel::data( const QModelIndex &idx, int role ) const
{
  if ( idx.column() == LayerColumn )
  {
    return mLayerTreeModel->data( mapToSource( idx ), role );
  }
  else
  {
    QgsMapLayer *layer = mapLayer( idx );

    if ( !layer )
    {
      return QVariant();
    }

    if ( role == Qt::CheckStateRole || role == Qt::UserRole )
    {
      const QVariant trueValue = role == Qt::CheckStateRole ? QVariant( Qt::Checked ) : QVariant( true );
      const QVariant falseValue = role == Qt::CheckStateRole ? QVariant( Qt::Unchecked ) : QVariant( false );
      switch ( idx.column() )
      {
        case LayerColumn:
          break;

        case IdentifiableColumn:
          if ( layer->isSpatial() )
            return mIdentifiableLayers.value( layer, true ) ? trueValue : falseValue;
          break;

        case ReadOnlyColumn:

          if ( layer->type() == QgsMapLayerType::VectorLayer )
            return mReadOnlyLayers.value( layer, true ) ? trueValue : falseValue;
          break;

        case SearchableColumn:

          if ( layer->type() == QgsMapLayerType::VectorLayer )
            return mSearchableLayers.value( layer, true ) ? trueValue : falseValue;
          break;

        case RequiredColumn:
          return !mRemovableLayers.value( layer, true ) ? trueValue : falseValue;

        case PrivateColumn:
          return mPrivateLayers.value( layer, false ) ? trueValue : falseValue;
      }
    }
  }

  return QVariant();
}

bool QgsLayerCapabilitiesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role == Qt::CheckStateRole )
  {
    QgsMapLayer *layer = mapLayer( index );
    if ( layer )
    {
      switch ( index.column() )
      {
        case LayerColumn:
          break;
        case IdentifiableColumn:
        {
          if ( layer->isSpatial() )
          {
            const bool identifiable = value == Qt::Checked;
            if ( identifiable != mIdentifiableLayers.value( layer, true ) )
            {
              mIdentifiableLayers.insert( layer, identifiable );
              emit dataChanged( index, index );
              return true;
            }
          }
          break;
        }
        case ReadOnlyColumn:
        {
          if ( layer->type() == QgsMapLayerType::VectorLayer )
          {
            const bool readOnly = value == Qt::Checked;
            if ( readOnly != mReadOnlyLayers.value( layer, true ) )
            {
              mReadOnlyLayers.insert( layer, readOnly );
              emit dataChanged( index, index );
              return true;
            }
          }
          break;
        }
        case SearchableColumn:
        {
          if ( layer->type() == QgsMapLayerType::VectorLayer )
          {
            const bool searchable = value == Qt::Checked;
            if ( searchable != mSearchableLayers.value( layer, true ) )
            {
              mSearchableLayers.insert( layer, searchable );
              emit dataChanged( index, index );
              return true;
            }
          }
          break;
        }
        case RequiredColumn:
        {
          const bool removable = value == Qt::Unchecked;
          if ( removable != mRemovableLayers.value( layer, true ) )
          {
            mRemovableLayers.insert( layer, removable );
            emit dataChanged( index, index );
            return true;
          }
          break;
        }
        case PrivateColumn:
        {
          const bool isPrivate = value == Qt::Checked;
          if ( isPrivate != mPrivateLayers.value( layer, false ) )
          {
            mPrivateLayers.insert( layer, isPrivate );
            emit dataChanged( index, index );
            return true;
          }
          break;
        }
      }
    }
  }
  return false;
}

bool QgsLayerCapabilitiesModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QgsLayerTreeNode *node = mLayerTreeModel->index2node( mLayerTreeModel->index( sourceRow, LayerColumn, sourceParent ) );
  return nodeShown( node );
}

bool QgsLayerCapabilitiesModel::nodeShown( QgsLayerTreeNode *node ) const
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
    return layer
           && ( mFilterText.isEmpty() || layer->name().contains( mFilterText, Qt::CaseInsensitive ) )
           && ( !mShowSpatialLayersOnly || layer->isSpatial() );
  }
}
