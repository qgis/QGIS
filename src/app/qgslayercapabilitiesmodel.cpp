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

QgsLayerCapabilitiesModel::QgsLayerCapabilitiesModel( QgsProject *project, QObject *parent )
  : QSortFilterProxyModel( parent )
{
  mNonIdentifiableLayers = project->nonIdentifiableLayers();
  mRequiredLayers = project->requiredLayers();

  const QMap<QString, QgsMapLayer *> &mapLayers = QgsProject::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    mReadOnlyLayers.insert( it.value(), it.value()->readOnly() );
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( it.value() );
    mSearchableLayers.insert( it.value(), vl && vl->searchable() );
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
  QVector<int> roles = QVector<int>() << Qt::CheckStateRole;
  for ( const QModelIndex &index : checkedIndexes )
  {
    bool isChecked = data( index, Qt::CheckStateRole ) == Qt::Checked;
    if ( setData( index, isChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole ) )
      emit dataChanged( index, index );
  }
}

QStringList QgsLayerCapabilitiesModel::nonIdentifiableLayers() const
{
  return mNonIdentifiableLayers;
}

QSet<QgsMapLayer *> QgsLayerCapabilitiesModel::requiredLayers() const
{
  return mRequiredLayers;
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
  Q_UNUSED( parent );
  return 5;
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
        default:
          return QVariant();
      }
    }
    if ( role == Qt::ToolTipRole )
    {
      switch ( section )
      {
        case LayerColumn:
        case IdentifiableColumn:
        case ReadOnlyColumn:
        case SearchableColumn:
          return QVariant();
        case RequiredColumn:
          return tr( "Layers which are protected from inadvertent removal from the project." );
        default:
          return QVariant();
      }
    }
  }
  return mLayerTreeModel->headerData( section, orientation, role );
}

Qt::ItemFlags QgsLayerCapabilitiesModel::flags( const QModelIndex &idx ) const
{
  if ( idx.column() == LayerColumn )
  {
    return Qt::ItemIsEnabled;
  }

  QgsMapLayer *layer = mapLayer( idx );
  if ( !layer )
  {
    return Qt::NoItemFlags;
  }
  else
  {
    if ( idx.column() == IdentifiableColumn )
    {
      if ( layer->isSpatial() )
      {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
      }
      else
      {
        return nullptr;
      }
    }
    else if ( idx.column() == ReadOnlyColumn )
    {
      if ( layer->type() == QgsMapLayer::VectorLayer )
      {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
      }
      else
      {
        return nullptr;
      }
    }
    else if ( idx.column() == SearchableColumn )
    {
      if ( layer->type() == QgsMapLayer::VectorLayer )
      {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
      }
      else
      {
        return nullptr;
      }
    }
    else if ( idx.column() == RequiredColumn )
    {
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
    }
  }
  return nullptr;
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
  QModelIndex parent = idx.parent();
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
      QVariant trueValue = role == Qt::CheckStateRole ? Qt::Checked : true;
      QVariant falseValue = role == Qt::CheckStateRole ? Qt::Unchecked : false;
      if ( idx.column() == IdentifiableColumn )
      {
        if ( layer->isSpatial() )
          return !mNonIdentifiableLayers.contains( layer->id() ) ? trueValue : falseValue;
      }
      else if ( idx.column() == ReadOnlyColumn )
      {
        if ( layer->type() == QgsMapLayer::VectorLayer )
          return mReadOnlyLayers.value( layer, true ) ? trueValue : falseValue;
      }
      else if ( idx.column() == SearchableColumn )
      {
        if ( layer->type() == QgsMapLayer::VectorLayer )
          return mSearchableLayers.value( layer, true ) ? trueValue : falseValue;
      }
      else if ( idx.column() == RequiredColumn )
      {
        return mRequiredLayers.contains( layer ) ? trueValue : falseValue;
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
      if ( index.column() == IdentifiableColumn )
      {
        if ( layer->isSpatial() )
        {
          bool nonIdentifiable = value == Qt::Unchecked;
          bool containsLayer = mNonIdentifiableLayers.contains( layer->id() );
          if ( containsLayer && !nonIdentifiable )
            mNonIdentifiableLayers.removeAll( layer->id() );
          if ( !containsLayer && nonIdentifiable )
            mNonIdentifiableLayers.append( layer->id() );
          emit dataChanged( index, index );
          return true;
        }
      }
      else if ( index.column() == ReadOnlyColumn )
      {
        if ( layer->type() == QgsMapLayer::VectorLayer )
        {
          mReadOnlyLayers.insert( layer, value == Qt::Checked );
          emit dataChanged( index, index );
          return true;
        }
      }
      else if ( index.column() == SearchableColumn )
      {
        if ( layer->type() == QgsMapLayer::VectorLayer )
        {
          mSearchableLayers.insert( layer, value == Qt::Checked );
          emit dataChanged( index, index );
          return true;
        }
      }
      else if ( index.column() == RequiredColumn )
      {
        bool required = value == Qt::Checked;
        bool containsLayer = mRequiredLayers.contains( layer );
        if ( containsLayer && !required )
          mRequiredLayers.remove( layer );
        if ( !containsLayer && required )
          mRequiredLayers.insert( layer );
        emit dataChanged( index, index );
        return true;
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
    Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
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
