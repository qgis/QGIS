/***************************************************************************
   qgsmaplayermodel.cpp
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

#include <QIcon>

#include "qgsdataitem.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerregistry.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"


const int QgsMapLayerModel::LayerIdRole = Qt::UserRole + 1;

QgsMapLayerModel::QgsMapLayerModel( QList<QgsMapLayer *> layers, QObject *parent )
    : QAbstractItemModel( parent )
    , mLayersChecked( QMap<QString, Qt::CheckState>() )
    , mItemCheckable( false )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( removeLayers( QStringList ) ) );
  addLayers( layers );
}

QgsMapLayerModel::QgsMapLayerModel( QObject *parent )
    : QAbstractItemModel( parent )
    , mLayersChecked( QMap<QString, Qt::CheckState>() )
    , mItemCheckable( false )
{
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersAdded( QList<QgsMapLayer*> ) ), this, SLOT( addLayers( QList<QgsMapLayer*> ) ) );
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layersWillBeRemoved( QStringList ) ), this, SLOT( removeLayers( QStringList ) ) );
  addLayers( QgsMapLayerRegistry::instance()->mapLayers().values() );
}

void QgsMapLayerModel::setItemsCheckable( bool checkable )
{
  mItemCheckable = checkable;
}

void QgsMapLayerModel::checkAll( Qt::CheckState checkState )
{
  Q_FOREACH ( const QString& key, mLayersChecked.keys() )
  {
    mLayersChecked[key] = checkState;
  }
  emit dataChanged( index( 0, 0 ), index( mLayers.length() - 1, 0 ) );
}

QList<QgsMapLayer *> QgsMapLayerModel::layersChecked( Qt::CheckState checkState )
{
  QList<QgsMapLayer *> layers;
  Q_FOREACH ( QgsMapLayer* layer, mLayers )
  {
    if ( mLayersChecked[layer->id()] == checkState )
    {
      layers.append( layer );
    }
  }
  return layers;
}

QModelIndex QgsMapLayerModel::indexFromLayer( QgsMapLayer *layer ) const
{
  int r = mLayers.indexOf( layer );
  return index( r, 0 );
}

void QgsMapLayerModel::removeLayers( const QStringList layerIds )
{
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    QModelIndex startIndex = index( 0, 0 );
    QModelIndexList list = match( startIndex, LayerIdRole, layerId, 1 );
    if ( list.count() )
    {
      QModelIndex index = list[0];
      beginRemoveRows( QModelIndex(), index.row(), index.row() );
      mLayersChecked.remove( layerId );
      mLayers.removeAt( index.row() );
      endRemoveRows();
    }
  }
}

void QgsMapLayerModel::addLayers( QList<QgsMapLayer *> layers )
{
  beginInsertRows( QModelIndex(), mLayers.count(), mLayers.count() + layers.count() - 1 );
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    mLayers.append( layer );
    mLayersChecked.insert( layer->id(), Qt::Unchecked );
  }
  endInsertRows();
}

QModelIndex QgsMapLayerModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, mLayers[row] );
  }

  return QModelIndex();

}

QModelIndex QgsMapLayerModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child );
  return QModelIndex();
}


int QgsMapLayerModel::rowCount( const QModelIndex &parent ) const
{
  return parent.isValid() ? 0 : mLayers.length();
}

int QgsMapLayerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}


QVariant QgsMapLayerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || !index.internalPointer() )
    return QVariant();

  if ( role == Qt::DisplayRole )
  {
    QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
    return layer->name();
  }

  if ( role == LayerIdRole )
  {
    QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
    return layer->id();
  }

  if ( role == Qt::CheckStateRole && mItemCheckable )
  {
    QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
    return mLayersChecked[layer->id()];
  }

  if ( role == Qt::DecorationRole )
  {
    QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
    QgsMapLayer::LayerType type = layer->type();
    if ( role == Qt::DecorationRole )
    {
      switch ( type )
      {
        case QgsMapLayer::RasterLayer:
        {
          return QgsLayerItem::iconRaster();
        }

        case QgsMapLayer::VectorLayer:
        {
          QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
          if ( !vl )
          {
            return QIcon();
          }
          QGis::GeometryType geomType = vl->geometryType();
          switch ( geomType )
          {
            case QGis::Point:
            {
              return QgsLayerItem::iconPoint();
            }
            case QGis::Polygon :
            {
              return QgsLayerItem::iconPolygon();
            }
            case QGis::Line :
            {
              return QgsLayerItem::iconLine();
            }
            case QGis::NoGeometry :
            {
              return QgsLayerItem::iconTable();
            }
            default:
            {
              return QIcon();
            }
          }
        }
        default:
        {
          return QIcon();
        }
      }
    }
  }

  return QVariant();
}


Qt::ItemFlags QgsMapLayerModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return 0;
  }

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if ( mItemCheckable )
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}


bool QgsMapLayerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( role == Qt::CheckStateRole )
  {
    QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
    mLayersChecked[layer->id()] = ( Qt::CheckState )value.toInt();
    emit dataChanged( index, index );
    return true;
  }

  return false;
}
