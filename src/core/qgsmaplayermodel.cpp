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

#include "qgsmaplayermodel.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsiconutils.h"
#include "qgsmaplayerlistutils_p.h"
#include <QMimeData>

QgsMapLayerModel::QgsMapLayerModel( const QList<QgsMapLayer *> &layers, QObject *parent, QgsProject *project )
  : QAbstractItemModel( parent )
  , mProject( project ? project : QgsProject::instance() )
{
  connect( mProject, static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  addLayers( layers );
}

QgsMapLayerModel::QgsMapLayerModel( QObject *parent, QgsProject *project )
  : QAbstractItemModel( parent )
  , mProject( project ? project : QgsProject::instance() )
{
  connect( mProject, &QgsProject::layersAdded, this, &QgsMapLayerModel::addLayers );
  connect( mProject, static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  addLayers( mProject->mapLayers().values() );
}

void QgsMapLayerModel::setProject( QgsProject *project )
{

  // remove layers from previous project
  if ( mProject )
  {
    removeLayers( mProject->mapLayers().keys() );
    disconnect( mProject, &QgsProject::layersAdded, this, &QgsMapLayerModel::addLayers );
    disconnect( mProject, static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  }

  mProject = project ? project : QgsProject::instance();

  connect( mProject, &QgsProject::layersAdded, this, &QgsMapLayerModel::addLayers );
  connect( mProject, static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsMapLayerModel::removeLayers );
  addLayers( mProject->mapLayers().values() );
}


void QgsMapLayerModel::setItemsCheckable( bool checkable )
{
  mItemCheckable = checkable;
}

void QgsMapLayerModel::setItemsCanBeReordered( bool allow )
{
  mCanReorder = allow;
}

bool QgsMapLayerModel::itemsCanBeReordered() const
{
  return mCanReorder;
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

void QgsMapLayerModel::setAllowEmptyLayer( bool allowEmpty, const QString &text, const QIcon &icon )
{
  mEmptyText = text;
  mEmptyIcon = icon;
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
  const auto constMLayers = mLayers;
  for ( QgsMapLayer *layer : constMLayers )
  {
    if ( mLayersChecked[layer->id()] == checkState )
    {
      layers.append( layer );
    }
  }
  return layers;
}

void QgsMapLayerModel::setLayersChecked( const QList<QgsMapLayer *> &layers )
{
  QMap<QString, Qt::CheckState>::iterator i = mLayersChecked.begin();
  for ( ; i != mLayersChecked.end(); ++i )
  {
    *i = Qt::Unchecked;
  }
  for ( const QgsMapLayer *layer : layers )
  {
    mLayersChecked[ layer->id() ] = Qt::Checked;
  }
  emit dataChanged( index( 0, 0 ), index( rowCount() - 1, 0 ), QVector<int>() << Qt::CheckStateRole );
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
  return mProject->mapLayer( index.data( LayerIdRole ).toString() );
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

void QgsMapLayerModel::setAdditionalLayers( const QList<QgsMapLayer *> &layers )
{
  if ( layers == _qgis_listQPointerToRaw( mAdditionalLayers ) )
    return;

  QStringList layerIdsToRemove;
  for ( QgsMapLayer *layer : std::as_const( mAdditionalLayers ) )
  {
    if ( layer )
      layerIdsToRemove << layer->id();
  }
  removeLayers( layerIdsToRemove );

  for ( QgsMapLayer *layer : layers )
  {
    if ( layer )
    {
      addLayers( { layer } );
      const QString layerId = layer->id();
      connect( layer, &QgsMapLayer::willBeDeleted, this, [this, layerId] { removeLayers( {layerId} ); } );
    }
  }

  mAdditionalLayers = _qgis_listRawToQPointer( layers );
}

QList<QgsMapLayer *> QgsMapLayerModel::additionalLayers() const
{
  return _qgis_listQPointerToRaw( mAdditionalLayers );
}

void QgsMapLayerModel::removeLayers( const QStringList &layerIds )
{
  int offset = 0;
  if ( mAllowEmpty )
    offset++;

  for ( const QString &layerId : layerIds )
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
    const auto constLayers = layers;
    for ( QgsMapLayer *layer : constLayers )
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
  Q_UNUSED( child )
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
  Q_UNUSED( parent )
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
    case Qt::EditRole:
    {
      if ( index.row() == 0 && mAllowEmpty )
        return mEmptyText;

      if ( additionalIndex >= 0 )
        return mAdditionalItems.at( additionalIndex );

      QgsMapLayer *layer = mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
      if ( !layer )
        return QVariant();

      if ( !mShowCrs || !layer->isSpatial() || role == Qt::EditRole )
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

      QgsMapLayer *layer = mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
      return layer ? layer->id() : QVariant();
    }

    case LayerRole:
    {
      if ( isEmpty || additionalIndex >= 0 )
        return QVariant();

      return QVariant::fromValue<QgsMapLayer *>( mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) ) );
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

        QgsMapLayer *layer = mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
        return layer ? mLayersChecked[layer->id()] : QVariant();
      }

      return QVariant();
    }

    case Qt::ToolTipRole:
    {
      QgsMapLayer *layer = mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
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
        return parts.join( QLatin1String( "<br/>" ) );
      }
      return QVariant();
    }

    case Qt::DecorationRole:
    {
      if ( isEmpty )
        return mEmptyIcon.isNull() ? QVariant() : mEmptyIcon;

      if ( additionalIndex >= 0 )
        return QVariant();

      QgsMapLayer *layer = mLayers.value( index.row() - ( mAllowEmpty ? 1 : 0 ) );
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
    if ( mCanReorder )
      return Qt::ItemIsDropEnabled;
    else
      return Qt::ItemFlags();
  }

  bool isEmpty = index.row() == 0 && mAllowEmpty;
  int additionalIndex = index.row() - ( mAllowEmpty ? 1 : 0 ) - mLayers.count();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( mCanReorder && !isEmpty && additionalIndex < 0 )
  {
    flags |= Qt::ItemIsDragEnabled;
  }

  if ( mItemCheckable && !isEmpty && additionalIndex < 0 )
  {
    flags |= Qt::ItemIsUserCheckable;
  }
  return flags;
}

bool QgsMapLayerModel::insertRows( int row, int count, const QModelIndex &parent )
{
  if ( parent.isValid() )
    return false;

  int offset = 0;
  if ( mAllowEmpty )
    offset++;

  beginInsertRows( parent, row, row + count - 1 );
  for ( int i = row; i < row + count; ++i )
    mLayers.insert( i - offset, nullptr );
  endInsertRows();

  return true;
}

bool QgsMapLayerModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( parent.isValid() || row < 0 )
    return false;

  int offset = 0;
  if ( mAllowEmpty )
  {
    if ( row == 0 )
      return false;

    offset++;
  }

  if ( row - offset > mLayers.count() - 1 )
  {
    return false;
  }

  beginRemoveRows( parent, row, row + count - 1 );
  for ( int i = 0; i != count; ++i )
    mLayers.removeAt( row - offset );
  endRemoveRows();

  return true;
}

QStringList QgsMapLayerModel::mimeTypes() const
{
  QStringList types;
  types << QStringLiteral( "application/qgis.layermodeldata" );
  return types;
}

bool QgsMapLayerModel::canDropMimeData( const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex & ) const
{
  if ( !mCanReorder || action != Qt::MoveAction || !data->hasFormat( QStringLiteral( "application/qgis.layermodeldata" ) ) )
    return false;
  return true;
}

QMimeData *QgsMapLayerModel::mimeData( const QModelIndexList &indexes ) const
{
  std::unique_ptr< QMimeData > mimeData = std::make_unique< QMimeData >();

  QByteArray encodedData;
  QDataStream stream( &encodedData, QIODevice::WriteOnly );
  QSet< QString > addedLayers;

  for ( const QModelIndex &i : indexes )
  {
    if ( i.isValid() )
    {
      const QString id = data( index( i.row(), 0, i.parent() ), LayerIdRole ).toString();
      if ( !addedLayers.contains( id ) )
      {
        addedLayers.insert( id );
        stream << id;
      }
    }
  }
  mimeData->setData( QStringLiteral( "application/qgis.layermodeldata" ), encodedData );
  return mimeData.release();
}

bool QgsMapLayerModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent )
{
  if ( !canDropMimeData( data, action, row, column, parent ) || row < 0 )
    return false;

  if ( action == Qt::IgnoreAction )
    return true;
  else if ( action != Qt::MoveAction )
    return false;

  QByteArray encodedData = data->data( QStringLiteral( "application/qgis.layermodeldata" ) );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  QStringList newItems;
  int rows = 0;

  while ( !stream.atEnd() )
  {
    QString text;
    stream >> text;
    newItems << text;
    ++rows;
  }

  insertRows( row, rows, QModelIndex() );
  for ( const QString &text : std::as_const( newItems ) )
  {
    QModelIndex idx = index( row, 0, QModelIndex() );
    setData( idx, text, LayerIdRole );
    row++;
  }

  return true;
}

Qt::DropActions QgsMapLayerModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

QIcon QgsMapLayerModel::iconForLayer( QgsMapLayer *layer )
{
  return QgsIconUtils::iconForLayer( layer );
}

bool QgsMapLayerModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  bool isEmpty = index.row() == 0 && mAllowEmpty;
  int additionalIndex = index.row() - ( mAllowEmpty ? 1 : 0 ) - mLayers.count();

  switch ( role )
  {
    case Qt::CheckStateRole:
    {
      if ( !isEmpty && additionalIndex < 0 )
      {
        QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
        mLayersChecked[layer->id()] = ( Qt::CheckState )value.toInt();
        emit dataChanged( index, index, QVector< int >() << Qt::CheckStateRole );
        return true;
      }
      break;
    }

    case LayerIdRole:
      if ( !isEmpty && additionalIndex < 0 )
      {
        mLayers[index.row() - ( mAllowEmpty ? 1 : 0 )] = mProject->mapLayer( value.toString() );
        emit dataChanged( index, index );
        return true;
      }
      break;
  }

  return false;
}
