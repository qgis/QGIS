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
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"


QgsMapLayerModel::QgsMapLayerModel( const QList<QgsMapLayer *> &layers, QObject *parent )
  : QAbstractItemModel( parent )
{
  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  addLayers( layers );
}

QgsMapLayerModel::QgsMapLayerModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  connect( QgsProject::instance(), &QgsProject::layersAdded, this, &QgsMapLayerModel::addLayers );
  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  addLayers( QgsProject::instance()->mapLayers().values() );
}

void QgsMapLayerModel::setItemsCheckable( bool checkable )
{
  mItemCheckable = checkable;
}

void QgsMapLayerModel::checkAll( Qt::CheckState checkState )
{
  QMap<QString, Qt::CheckState>::iterator i = mLayersChecked.begin();
  for ( ; i != mLayersChecked.end(); ++i )
  {
    *i = checkState;
  }
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ) );
}

void QgsMapLayerModel::setAllowEmptyLayer( bool allowEmpty )
{
  if ( allowEmpty == mAllowEmpty )
    return;

  if ( allowEmpty )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mAllowEmpty = true;
    endInsertRows();
  }
  else
  {
    beginRemoveRows( QModelIndex(), 0, 0 );
    mAllowEmpty = false;
    endRemoveRows();
  }
}

void QgsMapLayerModel::setShowCrs( bool showCrs )
{
  if ( mShowCrs == showCrs )
    return;

  mShowCrs = showCrs;
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ), QVector<int>() << Qt::DisplayRole );
}

QList<QgsMapLayer *> QgsMapLayerModel::layersChecked( Qt::CheckState checkState )
{
  QList<QgsMapLayer *> layers;
  Q_FOREACH ( QgsMapLayer *layer, mLayers )
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
  if ( r >= 0 && mAllowEmpty )
    r++;
  return index( r, 0 );
}

QgsMapLayer *QgsMapLayerModel::layerFromIndex( const QModelIndex &index ) const
{
  return static_cast<QgsMapLayer *>( index.internalPointer() );
}

void QgsMapLayerModel::setAdditionalItems( const QStringList &items )
{
  if ( items == mAdditionalItems )
    return;

  int offset = 0;
  if ( mAllowEmpty )
    offset++;

  offset += mLayers.count();

  //remove existing
  if ( !mAdditionalItems.isEmpty() )
  {
    beginRemoveRows( QModelIndex(), offset, offset + mAdditionalItems.count() - 1 );
    mAdditionalItems.clear();
    endRemoveRows();
  }

  //add new
  beginInsertRows( QModelIndex(), offset, offset + items.count() - 1 );
  mAdditionalItems = items;
  endInsertRows();
}

void QgsMapLayerModel::removeLayers( const QStringList &layerIds )
{
  int offset = 0;
  if ( mAllowEmpty )
    offset++;

  Q_FOREACH ( const QString &layerId, layerIds )
  {
    QModelIndex startIndex = index( 0, 0 );
    QModelIndexList list = match( startIndex, LayerIdRole, layerId, 1 );
    if ( !list.isEmpty() )
    {
      QModelIndex index = list[0];
      beginRemoveRows( QModelIndex(), index.row(), index.row() );
      mLayersChecked.remove( layerId );
      mLayers.removeAt( index.row() - offset );
      endRemoveRows();
    }
  }
}

void QgsMapLayerModel::addLayers( const QList<QgsMapLayer *> &layers )
{
  if ( !layers.empty( ) )
  {
    int offset = 0;
    if ( mAllowEmpty )
      offset++;

    beginInsertRows( QModelIndex(), mLayers.count() + offset, mLayers.count() + layers.count() - 1  + offset );
    Q_FOREACH ( QgsMapLayer *layer, layers )
    {
      mLayers.append( layer );
      mLayersChecked.insert( layer->id(), Qt::Unchecked );
    }
    endInsertRows();
  }
}

QModelIndex QgsMapLayerModel::index( int row, int column, const QModelIndex &parent ) const
{
  int offset = 0;
  if ( mAllowEmpty )
    offset++;

  if ( hasIndex( row, column, parent ) )
  {
    QgsMapLayer *layer = nullptr;
    if ( row - offset >= 0 && row - offset < mLayers.count() )
      layer = mLayers.at( row - offset );

    return createIndex( row, column, layer );
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
  if ( parent.isValid() )
    return 0;

  return ( mAllowEmpty ? 1 : 0 ) + mLayers.length() + mAdditionalItems.count();
}

int QgsMapLayerModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}


QVariant QgsMapLayerModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  bool isEmpty = index.row() == 0 && mAllowEmpty;
  int additionalIndex = index.row() - ( mAllowEmpty ? 1 : 0 ) - mLayers.count();

  switch ( role )
  {
    case Qt::DisplayRole:
    {
      if ( index.row() == 0 && mAllowEmpty )
        return QVariant();

      if ( additionalIndex >= 0 )
        return mAdditionalItems.at( additionalIndex );

      QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
      if ( !layer )
        return QVariant();

      if ( !mShowCrs || !layer->isSpatial() )
      {
        return layer->name();
      }
      else
      {
        return tr( "%1 [%2]" ).arg( layer->name(), layer->crs().authid() );
      }
    }

    case LayerIdRole:
    {
      if ( isEmpty || additionalIndex >= 0 )
        return QVariant();

      QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
      return layer ? layer->id() : QVariant();
    }

    case LayerRole:
    {
      if ( isEmpty || additionalIndex >= 0 )
        return QVariant();

      return QVariant::fromValue<QgsMapLayer *>( static_cast<QgsMapLayer *>( index.internalPointer() ) );
    }

    case EmptyRole:
      return isEmpty;

    case AdditionalRole:
      return additionalIndex >= 0;

    case Qt::CheckStateRole:
    {
      if ( mItemCheckable )
      {
        if ( isEmpty || additionalIndex >= 0 )
          return QVariant();

        QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
        return layer ? mLayersChecked[layer->id()] : QVariant();
      }

      return QVariant();
    }

    case Qt::ToolTipRole:
    {
      QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
      if ( layer )
      {
        QStringList parts;
        QString title = layer->title().isEmpty() ? layer->shortName() : layer->title();
        if ( title.isEmpty() )
          title = layer->name();
        title = "<b>" + title + "</b>";
        if ( layer->isSpatial() && layer->crs().isValid() )
        {
          if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
            title = tr( "%1 (%2 - %3)" ).arg( title, QgsWkbTypes::displayString( vl->wkbType() ), layer->crs().authid() );
          else
            title = tr( "%1 (%2) " ).arg( title, layer->crs().authid() );
        }
        parts << title;

        if ( !layer->abstract().isEmpty() )
          parts << "<br/>" + layer->abstract().replace( QLatin1String( "\n" ), QLatin1String( "<br/>" ) );
        parts << "<i>" + layer->publicSource() + "</i>";
        return parts.join( QStringLiteral( "<br/>" ) );
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( isEmpty || additionalIndex >= 0 )
        return QVariant();

      QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
      if ( !layer )
        return QVariant();

      return iconForLayer( layer );
    }
  }

  return QVariant();
}

QHash<int, QByteArray> QgsMapLayerModel::roleNames() const
{
  QHash<int, QByteArray> roles  = QAbstractItemModel::roleNames();
  roles[LayerIdRole]  = "layerId";
  roles[LayerRole] = "layer";

  return roles;
}

Qt::ItemFlags QgsMapLayerModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return nullptr;
  }

  bool isEmpty = index.row() == 0 && mAllowEmpty;
  int additionalIndex = index.row() - ( mAllowEmpty ? 1 : 0 ) - mLayers.count();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  if ( mItemCheckable && !isEmpty && additionalIndex < 0 )
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}

QIcon QgsMapLayerModel::iconForLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::RasterLayer:
    {
      return QgsLayerItem::iconRaster();
    }

    case QgsMapLayerType::MeshLayer:
    {
      return QgsLayerItem::iconMesh();
    }

    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer *>( layer );
      if ( !vl )
      {
        return QIcon();
      }
      QgsWkbTypes::GeometryType geomType = vl->geometryType();
      switch ( geomType )
      {
        case QgsWkbTypes::PointGeometry:
        {
          return QgsLayerItem::iconPoint();
        }
        case QgsWkbTypes::PolygonGeometry :
        {
          return QgsLayerItem::iconPolygon();
        }
        case QgsWkbTypes::LineGeometry :
        {
          return QgsLayerItem::iconLine();
        }
        case QgsWkbTypes::NullGeometry :
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


bool QgsMapLayerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  bool isEmpty = index.row() == 0 && mAllowEmpty;
  int additionalIndex = index.row() - ( mAllowEmpty ? 1 : 0 ) - mLayers.count();

  if ( role == Qt::CheckStateRole && !isEmpty && additionalIndex < 0 )
  {
    QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
    mLayersChecked[layer->id()] = ( Qt::CheckState )value.toInt();
    emit dataChanged( index, index );
    return true;
  }

  return false;
}
