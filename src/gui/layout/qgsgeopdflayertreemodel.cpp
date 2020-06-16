/***************************************************************************
  qgsgeopdflayertreemodel.cpp
 ---------------------
 begin                : August 2019
 copyright            : (C) 2019 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QComboBox>
#include <QDoubleSpinBox>

#include "qgsgeopdflayertreemodel.h"
#include "qgslayertree.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

QgsGeoPdfLayerTreeModel::QgsGeoPdfLayerTreeModel( const QList<QgsMapLayer *> &layers, QObject *parent )
  : QgsMapLayerModel( layers, parent )
{
  setItemsCanBeReordered( true );
}

int QgsGeoPdfLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 4;
}

Qt::ItemFlags QgsGeoPdfLayerTreeModel::flags( const QModelIndex &idx ) const
{
  if ( !idx.isValid() )
    return Qt::ItemIsDropEnabled;

  if ( idx.column() == IncludeVectorAttributes )
  {
    if ( vectorLayer( idx ) )
      return QgsMapLayerModel::flags( idx ) | Qt::ItemIsUserCheckable;
    else
      return QgsMapLayerModel::flags( idx );
  }

  if ( idx.column() == InitiallyVisible )
  {
    return QgsMapLayerModel::flags( idx ) | Qt::ItemIsUserCheckable;
  }

  if ( !mapLayer( idx ) )
  {
    return nullptr;
  }
  else
  {
    return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled;
  }
  return Qt::NoItemFlags;
}

QgsMapLayer *QgsGeoPdfLayerTreeModel::mapLayer( const QModelIndex &idx ) const
{
  return layerFromIndex( index( idx.row(), LayerColumn, idx.parent() ) );
}

QgsVectorLayer *QgsGeoPdfLayerTreeModel::vectorLayer( const QModelIndex &idx ) const
{
  return qobject_cast<QgsVectorLayer *>( mapLayer( idx ) );
}

QVariant QgsGeoPdfLayerTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    if ( role == Qt::DisplayRole )
    {
      switch ( section )
      {
        case LayerColumn:
          return tr( "Layer" );
        case GroupColumn:
          return tr( "PDF Group" );
        case InitiallyVisible:
          return tr( "Initially Visible" );
        case IncludeVectorAttributes:
          return tr( "Include Attributes" );
        default:
          return QVariant();
      }
    }
  }
  return QgsMapLayerModel::headerData( section, orientation, role );
}

QVariant QgsGeoPdfLayerTreeModel::data( const QModelIndex &idx, int role ) const
{
  switch ( idx.column() )
  {
    case LayerColumn:
      if ( role == Qt::CheckStateRole )
        return QVariant();

      return QgsMapLayerModel::data( idx, role );

    case GroupColumn:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
          if ( QgsMapLayer *ml = mapLayer( idx ) )
          {
            return ml->customProperty( QStringLiteral( "geopdf/groupName" ) ).toString();
          }
          break;
        }
      }

      return QVariant();
    }

    case InitiallyVisible:
    {
      if ( role == Qt::CheckStateRole )
      {
        if ( QgsMapLayer *ml = mapLayer( idx ) )
        {
          const QVariant v = ml->customProperty( QStringLiteral( "geopdf/initiallyVisible" ) );
          if ( v.isValid() )
          {
            return v.toBool() ? Qt::Checked : Qt::Unchecked;
          }
          else
          {
            // otherwise, we default to showing by default
            return Qt::Checked;
          }
        }
        return QVariant();
      }
      return QVariant();
    }

    case IncludeVectorAttributes:
    {
      if ( role == Qt::CheckStateRole )
      {
        if ( QgsVectorLayer *vl = vectorLayer( idx ) )
        {
          const QVariant v = vl->customProperty( QStringLiteral( "geopdf/includeFeatures" ) );
          if ( v.isValid() )
          {
            return v.toBool() ? Qt::Checked : Qt::Unchecked;
          }
          else
          {
            // otherwise, we default to true
            return Qt::Checked;
          }
        }
        return QVariant();
      }
    }
  }

  return QVariant();
}

bool QgsGeoPdfLayerTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  switch ( index.column() )
  {
    case IncludeVectorAttributes:
    {
      if ( role == Qt::CheckStateRole )
      {
        if ( QgsVectorLayer *vl = vectorLayer( index ) )
        {
          vl->setCustomProperty( QStringLiteral( "geopdf/includeFeatures" ), value.toInt() == Qt::Checked );
          emit dataChanged( index, index );
          return true;
        }
      }
      break;
    }

    case GroupColumn:
    {
      if ( role == Qt::EditRole )
      {
        if ( QgsMapLayer *ml = mapLayer( index ) )
        {
          ml->setCustomProperty( QStringLiteral( "geopdf/groupName" ), value.toString() );
          emit dataChanged( index, index );
          return true;
        }
      }
      break;
    }

    case InitiallyVisible:
    {
      if ( role == Qt::CheckStateRole )
      {
        if ( QgsMapLayer *ml = mapLayer( index ) )
        {
          ml->setCustomProperty( QStringLiteral( "geopdf/initiallyVisible" ), value.toInt() == Qt::Checked );
          emit dataChanged( index, index );
          return true;
        }
      }
      break;
    }

    case LayerColumn:
      return QgsMapLayerModel::setData( index, value, role );
  }
  return false;
}

void QgsGeoPdfLayerTreeModel::checkAll( bool checked, const QModelIndex &parent, int column )
{
  for ( int row = 0; row < rowCount( parent ); ++row )
  {
    const QModelIndex childIndex = index( row, column, parent );
    setData( childIndex, checked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    checkAll( checked, childIndex );
  }
}


///@cond PRIVATE
QgsGeoPdfLayerFilteredTreeModel::QgsGeoPdfLayerFilteredTreeModel( QgsGeoPdfLayerTreeModel *sourceModel, QObject *parent )
  : QSortFilterProxyModel( parent )
  , mLayerTreeModel( sourceModel )
{
  setSourceModel( sourceModel );
}

bool QgsGeoPdfLayerFilteredTreeModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( QgsMapLayer *layer = mLayerTreeModel->layerFromIndex( sourceModel()->index( source_row, 0, source_parent ) ) )
  {
    // filter out non-spatial layers
    if ( !layer->isSpatial() )
      return false;

  }
  return true;
}

///@endcond
