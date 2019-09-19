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

QgsGeoPdfLayerTreeModel::QgsGeoPdfLayerTreeModel( QgsLayerTree *rootNode, QObject *parent )
  : QgsLayerTreeModel( rootNode, parent )
{
  setFlags( nullptr ); // ideally we'd just show embedded legend nodes - but the api doesn't exist for this
}

int QgsGeoPdfLayerTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 2;
}

Qt::ItemFlags QgsGeoPdfLayerTreeModel::flags( const QModelIndex &idx ) const
{
  if ( idx.column() == LayerColumn )
  {
    return QgsLayerTreeModel::flags( idx ) | Qt::ItemIsUserCheckable;
  }

  QgsVectorLayer *vl = vectorLayer( idx );
  if ( !vl )
  {
    return Qt::NoItemFlags;
  }
  else
  {
    const QModelIndex layerIndex = sibling( idx.row(), LayerColumn, idx );
    if ( data( layerIndex, Qt::CheckStateRole ) == Qt::Checked )
    {
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
  }
  return Qt::NoItemFlags;
}

QgsVectorLayer *QgsGeoPdfLayerTreeModel::vectorLayer( const QModelIndex &idx ) const
{
  QgsLayerTreeNode *node = index2node( index( idx.row(), LayerColumn, idx.parent() ) );
  if ( !node || !QgsLayerTree::isLayer( node ) )
    return nullptr;

  return qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() );
}

QVariant QgsGeoPdfLayerTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Horizontal )
  {
    if ( role == Qt::DisplayRole )
    {
      switch ( section )
      {
        case 0:
          return tr( "Layer" );
        case 1:
          return tr( "PDF Group" );
        default:
          return QVariant();
      }
    }
  }
  return QgsLayerTreeModel::headerData( section, orientation, role );
}

QVariant QgsGeoPdfLayerTreeModel::data( const QModelIndex &idx, int role ) const
{
  switch ( idx.column() )
  {
    case LayerColumn:
    {
      if ( role == Qt::CheckStateRole )
      {
        QgsLayerTreeNode *node = index2node( index( idx.row(), LayerColumn, idx.parent() ) );
        QgsVectorLayer *vl = vectorLayer( idx );
        if ( vl )
        {
          const QVariant v = vl->customProperty( QStringLiteral( "geopdf/includeFeatures" ) );
          if ( v.isValid() )
          {
            return v.toBool() ? Qt::Checked : Qt::Unchecked;
          }
          else
          {
            // otherwise, we default to the layer's visibility
            return node->itemVisibilityChecked() ? Qt::Checked : Qt::Unchecked;
          }
        }
        return QVariant();
      }
      return QgsLayerTreeModel::data( idx, role );
    }
    case GroupColumn:
    {
      switch ( role )
      {
        case Qt::DisplayRole:
        case Qt::EditRole:
        {
          if ( QgsVectorLayer *vl = vectorLayer( idx ) )
          {
            return vl->customProperty( QStringLiteral( "geopdf/groupName" ) ).toString();
          }
          break;
        }
      }

      return QVariant();
    }
  }

  return QVariant();
}

bool QgsGeoPdfLayerTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  switch ( index.column() )
  {
    case LayerColumn:
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
        if ( QgsVectorLayer *vl = vectorLayer( index ) )
        {
          vl->setCustomProperty( QStringLiteral( "geopdf/groupName" ), value.toString() );
          emit dataChanged( index, index );
          return true;
        }
      }
      break;
    }
  }
  return false;
}
