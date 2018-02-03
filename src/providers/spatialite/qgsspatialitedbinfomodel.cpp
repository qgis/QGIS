/***************************************************************************
                        qgsspatialitedbinfomodel.cpp  -  description
                         -------------------
    begin                : December 2017
    copyright            : (C) 2017 by Mark Johnson
    email                : mj10777@googlemail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtWidgets>

#include <QStringList>
#include <QList>
#include "qgslogger.h"
#include "qgsbox3d.h"
#include "qgsspatialitedbinfomodel.h"
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::QgsSpatialiteDbInfoModel
//-----------------------------------------------------------------
QgsSpatialiteDbInfoModel::QgsSpatialiteDbInfoModel( QObject *parent )
  : QAbstractItemModel( parent )
{
  // QTableView {selection-background-color: #308cc6;selection-color: #ffffff;}
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setModelType
// Will setup the Model with a specific Type
// - Based on this Type, different initalization Tasks can be compleated
// -> setting the default columns with Text being one of these tasks
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem *QgsSpatialiteDbInfoModel::setModelType( SpatialiteDbInfoModelType modelType )
{
  mModelType  = modelType;
  // The rootItem [nullptr, ItemTypeRoot] will set the Column-Header text and store the positions
  mModelRootItem = new QgsSpatialiteDbInfoItem( nullptr, QgsSpatialiteDbInfoItem::ItemTypeRoot, modelType );
  if ( ( mModelRootItem ) && ( mModelRootItem->isValid() ) )
  {
    // Store Column-Header positions numbers, which can be retrieved from the outside
    mColumnTable = mModelRootItem->getTableNameIndex();
    mColumnGeometryName = mModelRootItem->getGeometryNameIndex();
    mColumnGeometryType = mModelRootItem->getGeometryTypeIndex();
    mColumnSql = mModelRootItem->getSqlQueryIndex();
    mColumnSortHidden = mModelRootItem->getColumnSortHidden();
    // Connect willAdd/Added/Remove/Removed Children Events
    connectToRootItem();
    switch ( getModelType() )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        // Display text for the User, giving some information about what the Model is used for
        loadItemHelp();
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
      }
      break;
      default:
        break;
    }
  }
  return mModelRootItem;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::loadItemHelp
// Display text for the User, giving some information about what the Model is used for
// - Will be removed when the first Database is loaded
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::loadItemHelp()
{
  // This can only be called after 'connectToRootItem' has run
  mModelHelpItem = new QgsSpatialiteDbInfoItem( mModelRootItem, QgsSpatialiteDbInfoItem::ItemTypeHelpRoot );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getExpandToDepth
// Based on the Item-Type, a different can be returned
// - ItemTypeHelpRoot: all level should be shown
// - ItemTypeDb: down to Table/View name, but not the Columns
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoModel::getExpandToDepth()
{
  if ( ( mModelRootItem ) && ( mModelRootItem->childCount() > 0 ) )
  {
    switch ( mModelRootItem->child( 0 )->getItemType() )
    {
      case QgsSpatialiteDbInfoItem::ItemTypeHelpRoot:
        mExpandToDepth = 3;
        break;
      case QgsSpatialiteDbInfoItem::ItemTypeDb:
        // Only the Database-Name and the Group should be expanded, so that Layer-Names can be seen
        // - Database-Name[0]/Group[1]/Layer-Name[2]
        mExpandToDepth = 1;
        break;
      case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorGroup:
      case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterGroup:
        // Only the Group should be expanded, so that Layer-Names can be seen
        // - Group[0]/Layer-Name[1]
        mExpandToDepth = 0;
        break;
      default:
        mExpandToDepth = 3;
        break;
    }
  }
  return mExpandToDepth;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::~QgsSpatialiteDbInfoModel
//-----------------------------------------------------------------
QgsSpatialiteDbInfoModel::~QgsSpatialiteDbInfoModel()
{
  if ( mModelRootItem )
  {
    removeRows( 0, rowCount() );
    delete mModelRootItem;
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getTableNameText
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getTableNameText() const
{
  return mModelRootItem->text( getTableNameIndex() );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getGeometryNameText
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getGeometryNameText() const
{
  return mModelRootItem->text( getGeometryNameIndex() );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getGeometryTypeText
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getGeometryTypeText() const
{
  return mModelRootItem->text( getGeometryTypeIndex() );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getSqlQueryText
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getSqlQueryText() const
{
  return mModelRootItem->text( getSqlQueryIndex() );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::connectToRootItem()
//-----------------------------------------------------------------
// - Based on QgsLayerTreeModel::connectToRootNode
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::connectToRootItem()
{
  Q_ASSERT( mModelRootItem );
  connect( mModelRootItem, &QgsSpatialiteDbInfoItem::willAddChildren, this, &QgsSpatialiteDbInfoModel::nodeWillAddChildren );
  connect( mModelRootItem, &QgsSpatialiteDbInfoItem::addedChildren, this, &QgsSpatialiteDbInfoModel::nodeAddedChildren );
  connect( mModelRootItem, &QgsSpatialiteDbInfoItem::willRemoveChildren, this, &QgsSpatialiteDbInfoModel::nodeWillRemoveChildren );
  connect( mModelRootItem, &QgsSpatialiteDbInfoItem::removedChildren, this, &QgsSpatialiteDbInfoModel::nodeRemovedChildren );
  // connect( mModelRootItem, &QgsSpatialiteDbInfoItem::visibilityChanged, this, &QgsSpatialiteDbInfoModel::nodeVisibilityChanged );
  // connect( mModelRootItem, &QgsSpatialiteDbInfoItem::nameChanged, this, &QgsSpatialiteDbInfoModel::nodeNameChanged );
  // connect( mModelRootItem, &QgsSpatialiteDbInfoItem::customPropertyChanged, this, &QgsSpatialiteDbInfoModel::nodeCustomPropertyChanged );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::nodeWillAddChildren
// - node is the Item the Children will be added to
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::nodeWillAddChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo )
{
  if ( !node )
  {
    return;
  }
  beginInsertRows( node2index( node ), indexFrom, indexTo );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::nodeAddedChildren
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::nodeAddedChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );
  Q_UNUSED( indexFrom );
  Q_UNUSED( indexTo );
  endInsertRows();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::nodeWillRemoveChildren
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::nodeWillRemoveChildren( QgsSpatialiteDbInfoItem *node, int indexFrom, int indexTo )
{
  Q_ASSERT( node );
  beginRemoveRows( node2index( node ), indexFrom, indexTo );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::index2node
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem *QgsSpatialiteDbInfoModel::index2node( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return mModelRootItem;
  QObject *obj = reinterpret_cast<QObject *>( index.internalPointer() );
  return qobject_cast<QgsSpatialiteDbInfoItem *>( obj );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::node2index
//-----------------------------------------------------------------
QModelIndex QgsSpatialiteDbInfoModel::node2index( QgsSpatialiteDbInfoItem *node ) const
{
  if ( ( !node )  || ( !node->parent() ) )
  {
    return QModelIndex(); // this is the only root item -> invalid index
  }
  QModelIndex parentIndex = node2index( node->parent() );
  // int row = node->parent()->children().indexOf( node );
  int row = node->row();
  Q_ASSERT( row >= 0 );
  return index( row, 0, parentIndex );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::nodeRemovedChildren
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::nodeRemovedChildren()
{
  endRemoveRows();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::disconnectFromModelRootItem
//-----------------------------------------------------------------
// - Based on QgsLayerTreeModel::disconnectFromRootNode
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::disconnectFromRootItem()
{
  disconnect( mModelRootItem, nullptr, this, nullptr );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::columnCount
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoModel::columnCount( const QModelIndex & /* parent */ ) const
{
  return mModelRootItem->columnCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::data
//-----------------------------------------------------------------
QVariant QgsSpatialiteDbInfoModel::data( const QModelIndex &index, int role ) const
{
  // QgsDebugMsgLevelLiteral( QStringLiteral( "QgsSpatialiteDbInfoModel::data -0- index.row/column[%1,%2] role[%3] index.valid[%4]" ).arg( index.row() ).arg( index.column() ).arg( role ).arg(index.isValid()), 7 );
  if ( !index.isValid() )
  {
    return QVariant();
  }
  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::DecorationRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
    case Qt::StatusTipRole:
    case Qt::TextAlignmentRole:
    case Qt::BackgroundRole:
    case Qt::ForegroundRole:
    // flags
    case ( Qt::UserRole-1 ):
    {
      // 0=Qt::DisplayRole ,1=Qt::DecorationRole, 2=Qt::EditRole, 3=Qt::ToolTipRole, 4=Qt::StatusTipRole
      // 7=Qt::TextAlignmentRole, 8=Qt::BackgroundRole,9=Qt::ForegroundRole, 255=Qt::UserRole-1
      QgsSpatialiteDbInfoItem *item = getItem( index );
      return item->data( index.column(), role );
    }
    break;
    case Qt::FontRole:
    case Qt::CheckStateRole:
    case Qt::SizeHintRole:
    default:
      // 6=Qt::FontRole, 10=Qt::CheckStateRole,13=Qt::SizeHintRole
      break;
  }
  return QVariant();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::flags
// will return the set flag for the given Item's column
// 0=Qt::NoItemFlags will be returned on error
//-----------------------------------------------------------------
Qt::ItemFlags QgsSpatialiteDbInfoModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return Qt::NoItemFlags;
  }
  // return Qt::ItemIsEditable | QAbstractItemModel::flags( index );
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    Qt::ItemFlags itemFlags = item->flags( index.column() );
    return itemFlags;
  }
  return Qt::NoItemFlags;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getItem
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem *QgsSpatialiteDbInfoModel::getItem( const QModelIndex &index ) const
{
  if ( index.isValid() )
  {
    QgsSpatialiteDbInfoItem *item = static_cast<QgsSpatialiteDbInfoItem *>( index.internalPointer() );
    if ( ( item ) && ( item->isValid() ) )
    {
      return item;
    }
  }
  return mModelRootItem;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::headerData
//-----------------------------------------------------------------
QVariant QgsSpatialiteDbInfoModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( ( orientation == Qt::Horizontal ) &&
       ( ( role == Qt::DisplayRole ) || ( role == Qt::EditRole ) || ( role == Qt::DecorationRole ) || ( role == Qt::TextAlignmentRole ) ||
         ( role == Qt::BackgroundRole ) || ( role == Qt::ForegroundRole ) ) )
  {
    // 0=Qt::DisplayRole ,1=Qt::DecorationRole, , 2=Qt::EditRole, 7=Qt::TextAlignmentRole, 8=Qt::BackgroundRole,9=Qt::ForegroundRole
    return mModelRootItem->data( section, role );
  }
  // 2=Qt::EditRole, 6=Qt::FontRole,13=Qt::SizeHintRole
  return QVariant();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::index
//-----------------------------------------------------------------
QModelIndex QgsSpatialiteDbInfoModel::index( int row, int column, const QModelIndex &parent ) const
{
  // https://forum.qt.io/topic/41977/solved-how-to-find-a-child-in-a-qabstractitemmodel/5
  // if ( parent.isValid() && parent.column() != 0 )
  if ( !hasIndex( row, column, parent ) )
  {
    return QModelIndex();
  }
  QgsSpatialiteDbInfoItem *parentItem = getItem( parent );
  QgsSpatialiteDbInfoItem *childItem = parentItem->child( row );
  if ( childItem )
  {
    return createIndex( row, column, childItem );
  }
  else
  {
    return QModelIndex();
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::insertColumns
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::insertColumns( int position, int columns, const QModelIndex &parent )
{
  bool success;
  beginInsertColumns( parent, position, position + columns - 1 );
  success = mModelRootItem->insertColumns( position, columns );
  endInsertColumns();
  return success;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::insertRows
// Defaults: ItemType=ItemTypeUnknown, columnCount()
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::insertRows( int position, int rows, const QModelIndex &parent )
{
  QgsSpatialiteDbInfoItem *parentItem = getItem( parent );
  bool success;
  beginInsertRows( parent, position, position + rows - 1 );
  // success = parentItem->insertChildren( position, rows, mModelRootItem->getItemType(), mModelRootItem->columnCount() );
  success = parentItem->insertChildren( position, rows );
  endInsertRows();
  return success;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::parent
//-----------------------------------------------------------------
QModelIndex QgsSpatialiteDbInfoModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
  {
    return QModelIndex();
  }
  QgsSpatialiteDbInfoItem *childItem = getItem( index );
  QgsSpatialiteDbInfoItem *parentItem = childItem->parent();
  if ( parentItem == mModelRootItem )
  {
    return QModelIndex();
  }
  return createIndex( parentItem->childNumber(), 0, parentItem );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::removeColumns
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::removeColumns( int position, int columns, const QModelIndex &parent )
{
  bool success;
  beginRemoveColumns( parent, position, position + columns - 1 );
  success = mModelRootItem->removeColumns( position, columns );
  endRemoveColumns();
  if ( mModelRootItem->columnCount() == 0 )
  {
    removeRows( 0, rowCount() );
  }
  return success;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::removeRows
// Remove the Children from Root
// - removeChildren will delete the Children
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::removeRows( int position, int rows, const QModelIndex &parent )
{
  QgsSpatialiteDbInfoItem *parentItem = getItem( parent );
  bool success = true;
  beginRemoveRows( parent, position, position + rows - 1 );
  success = parentItem->removeChildren( position, rows );
  endRemoveRows();
  return success;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::rowCount
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoModel::rowCount( const QModelIndex &parent ) const
{
  QgsSpatialiteDbInfoItem *parentItem = getItem( parent );
  return parentItem->childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setData
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  bool bRc =  false;
  if ( ( role != Qt::DisplayRole ) && ( role != Qt::EditRole ) && ( role != Qt::DecorationRole ) && ( role != ( Qt::UserRole - 1 ) ) )
  {
    // Not: Text, QIcon or flags
    return bRc;
  }
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    bRc = item->setData( index.column(), value, role );
    if ( bRc )
    {
      emit dataChanged( index, index );
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setHeaderData
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::setHeaderData( int section, Qt::Orientation orientation, const QVariant &value, int role )
{
  if ( role != Qt::EditRole || orientation != Qt::Horizontal )
  {
    // return false;
  }
  bool result = mModelRootItem->setData( section, value, role );
  if ( result )
  {
    emit headerDataChanged( orientation, section, section );
  }
  return result;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setLayerOrderData
// returns amount of Items affected
// If nothing has changed (iRc== 0), no need for the View to do anything
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoModel::setLayerOrderData( bool bRemoveItems )
{
  int iCountItems = 0;
  if ( ( getModelRootItem() ) && ( getModelType() == ModelTypeLayerOrder ) )
  {
    bool bResetModel = false;
    if ( getModelRootItem()->getPrepairedChildItems().count() > 0 )
    {
      bResetModel = true;
    }
    if ( bResetModel )
    {
      // Inform the TreeView to stop painting, we are goint to rebuild everything
      beginResetModel();
    }
    else
    {
      emit layoutAboutToBeChanged();
    }
    iCountItems = getModelRootItem()->handelSelectedPrepairedChildren( bRemoveItems );
    if ( bResetModel )
    {
      // We have finished rebuilding, inform the TreeView to repaint everything
      endResetModel();
    }
    else
    {
      // We have changed something [Item internaly: such as moveChild, setSeletedStyle]
      // -  inform the TreeView to repaint everything
      emit layoutChanged();
    }
  }
  return iCountItems;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setSpatialiteDbInfo
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo )
{
  if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbValid() ) )
  {
    // Inform the TablesTreeView to stop painting, we are goint to rebuild everything
    beginResetModel();
    if ( getSpatialiteDbInfo() )
    {
      mTableCount = 0;
      if ( mModelDataItem )
      {
        // remove Children from mModelRootItem
        // - the children will be deleted
        removeRows( 0, rowCount() );
        mModelDataItem = nullptr;
      }
      if ( getSpatialiteDbInfo()->getQSqliteHandle() )
      {
        mSpatialiteDbInfo = nullptr;
      }
      else
      {
        delete spatialiteDbInfo;
        mSpatialiteDbInfo = nullptr;
      }
    }
    else
    {
      if ( mModelHelpItem )
      {
        // remove Children from mModelRootItem
        // - the children will be deleted
        removeRows( 0, rowCount() );
        mModelHelpItem = nullptr;
      }
    }
    mSpatialiteDbInfo = spatialiteDbInfo;
    mDbLayersDataSourceUris = getDataSourceUris();
    mTableCounter = 0;
    mNonSpatialTablesCounter = 0;
    mStylesCounter = 0;
    mSridInfoCounter = 0;
    mModelDataItem = new QgsSpatialiteDbInfoItem( mModelRootItem, mSpatialiteDbInfo );
    if ( ( mModelDataItem ) && ( mModelDataItem->isValid() ) )
    {
      if ( mModelRootItem->getSpatialiteDbInfo() )
      {
        mTableCounter = mModelRootItem->getTableCounter();
        mNonSpatialTablesCounter = mModelRootItem->getNonSpatialTablesCounter();
        mStylesCounter = mModelRootItem->getStylesCounter();
        mSridInfoCounter = mModelRootItem->getSridInfoCounter();
      }
    }
    // We have finished rebuilding, inform the TablesTreeView to repaint everything
    endResetModel();
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::findItems
// - taken from QStandardItemModel
// -> uses getItem() instead of 'itemFromIndex' as in QStandardItemModel
// TODO: theis may not be needed
//-----------------------------------------------------------------
QList<QgsSpatialiteDbInfoItem *> QgsSpatialiteDbInfoModel::findItems( const QString &text, Qt::MatchFlags flags, int column ) const
{
  QModelIndexList indexes = match( index( 0, column, QModelIndex() ), Qt::EditRole, text, -1, flags );
  QList<QgsSpatialiteDbInfoItem *> items;
  const int numIndexes = indexes.count();
  items.reserve( numIndexes );
  for ( int i = 0; i < numIndexes; ++i )
  {
    items.append( getItem( indexes.at( i ) ) );
  }
  return items;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getTableName
// - General retriever of text from the 'TableName' Column
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getTableName( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    return item->data( getTableNameIndex() ).toString();
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getGeometryName
// - General retriever of text from the 'GeometryName' Column
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getGeometryName( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    return item->data( getGeometryNameIndex() ).toString();
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getLayerName
// - to retrieve Item use
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getLayerName( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) && ( item->getDbLayer() ) )
  {
    return item->getDbLayer()->getLayerName();
  }
  QString sLayerName = getTableName( index );
  QString sGeometryName = getGeometryName( index );
  if ( !sGeometryName.isEmpty() )
  {
    // Note: for Rasters, this must be empty!
    sLayerName = QStringLiteral( "%1(%2)" ).arg( sLayerName ).arg( sGeometryName );
  }
  if ( mDbLayersDataSourceUris.contains( sLayerName ) )
  {
    return sLayerName;
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getLayerItem
// - to retrieve only a LayerName use getLayerName
// iParm:
// - 0: any ItemType that contains a QgsSpatialiteDbLayer
// - 1: as 0, but represents a Layer (not a Sub-Type of a Layer)
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem *QgsSpatialiteDbInfoModel::getLayerItem( const QModelIndex &index, int iParm )
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) && ( item->getDbLayer() ) )
  {
    switch ( iParm )
    {
      case 1:
        if ( item->isLayerSelectable() )
        {
          return item;
        }
        break;
      default:
        return item;
    }
  }
  return nullptr;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::removeLayerOrderChild
// - for use with ModelTypeLayerOrder only
// LayerOrder can contain up to 2 Groups (Vector/Raster)
// - with selected Layers to be displayed on MapCanvas
// If a Layer is removed, the next possible Layer will be selected
// - default next Layer. If last Layer in Group: the previous
// If the Group is empty, the Group will be removed
// - if a second Group exists, it will be selected
// QModelIndex will be returned with selected Layer or Group
// - an invalid QModelIndex will be returned if no Layers/Groups exists
//-----------------------------------------------------------------
QModelIndex QgsSpatialiteDbInfoModel::removeLayerOrderChild( QgsSpatialiteDbInfoItem *layerChild )
{
  QModelIndex returnIndex;
  if ( ( layerChild ) && ( layerChild->isValid() ) && ( layerChild->parent() ) && ( layerChild->getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    QgsSpatialiteDbInfoItem *parentItem = layerChild->getParent();
    QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemType itemType = layerChild->getItemType();
    QModelIndex parentIndex = node2index( parentItem );
    if ( ( parentItem ) && ( parentItem->isValid() ) )
    {
      int iMoveFrom = layerChild->childNumber();
      int iNextChildRow = -1;
      beginRemoveRows( parentIndex, iMoveFrom, iMoveFrom );
      // This can be a single Layer or a Group with Layers
      iNextChildRow = parentItem->removeChild( iMoveFrom );
      if ( iNextChildRow >= -1 )
      {
        // Any children will be deleted
        delete layerChild;
        layerChild = nullptr;
      }
      endRemoveRows();
      if ( iNextChildRow < 0 )
      {
        if ( iNextChildRow == -1 )
        {
          // parent/Group is empty
          layerChild = parentItem;
          parentItem = layerChild->getParent();
          if ( ( parentItem ) && ( parentItem->isValid() ) )
          {
            // Remove Group
            beginRemoveRows( parentIndex, iMoveFrom, iMoveFrom );
            iNextChildRow = parentItem->removeChild( iMoveFrom );
            if ( iNextChildRow >= -1 )
            {
              delete layerChild;
              layerChild = nullptr;
            }
            endRemoveRows();
            // if < 0: no other Groups exist
            if ( iNextChildRow >= 0 )
            {
              // An Vector/Raster-Group-Item has been deleted, select next Group
              layerChild = parentItem->child( iNextChildRow );
              // If no further Groups exist, this will be invalid
              returnIndex = node2index( layerChild );
            }
          }
        }
        // else -2: invalid range
      }
      else
      {
        // An Layer-Item inside a Vector/Raster-Group-Item has been deleted, reset Counter-Text of Group
        layerChild = parentItem->child( iNextChildRow );
        returnIndex = node2index( layerChild );
        QString sCounterText = QStringLiteral( "Layers: %1" ).arg( parentItem->childCount() );
        QString sLayerType = QString();
        switch ( itemType )
        {
          case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorLayer:
            sLayerType = QStringLiteral( "Vector" );
            break;
          case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterLayer:
            sLayerType = QStringLiteral( "Raster" );
            break;
          default:
            break;
        }
        if ( !sLayerType.isEmpty() )
        {
          parentItem->setText( getGeometryNameIndex(), sCounterText );
          parentItem->setToolTip( getGeometryNameIndex(), QStringLiteral( "There are %1 %2-Layers contained in this Group " ).arg( parentItem->childCount() ).arg( sLayerType ) );
        }
      }
    }
  }
  // If valid, View should call setCurrentIndex(returnIndex)
  return returnIndex;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::moveLayerOrderChild
// LayerOrder can contain up to 2 Groups (Vector/Raster)
// - with selected Layers to be displayed on MapCanvas
// Move a Item, inside its Parent/Group, to a new position
// - moving outside a Group is not forseen (Vector/Raster specific)
// Up/Down ; First to Last ; Last to First or a specific position inside the Parent
// - moving to a specific position is not implemented by the calling gui
//  iMoveCount is a +/- number from the present position inside the parent
// A negative number resulting in < 0: First to Last
// A positive number resulting in >= count: Last to First
// - 0: will do nothing
// QModelIndex will be returned with selected Layer at its new position
// - with multiple Up/Down pressing, the same Layer should remain selected
//-----------------------------------------------------------------
QModelIndex QgsSpatialiteDbInfoModel::moveLayerOrderChild( QgsSpatialiteDbInfoItem *layerChild, int iMoveCount )
{
  QModelIndex returnIndex;
  if ( ( layerChild ) && ( layerChild->isValid() ) && ( layerChild->parent() ) && ( layerChild->getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    QgsSpatialiteDbInfoItem *parentItem = layerChild->getParent();
    QModelIndex parentIndex = node2index( parentItem );
    if ( ( parentItem ) && ( parentItem->isValid() ) )
    {
      int iMoveFrom = layerChild->childNumber();
      //-----------------------------------------------------------------
      // Will return value needed for 'moveChild'
      // For Down[iMoveCount > 0], but not LastToFirst[iMoveToRows > 0], iMoveToRows is correct for beginMoveRows
      // - but not for moveChild: adapt value [iMoveToChild]
      // -> moveChild[QList.take] need to know the old position (removeAt), and after removal,  position to insert
      // For Up[iMoveCount < 0], and for FirstToLast[iMoveToRows >= parentItem->childCount()], iMoveToRows is correct for beginMoveRows
      // - but not for moveChild: adapt value  [iMoveToChild]
      // For the other conditions, the values are the same
      //-----------------------------------------------------------------
      int iMoveToChild = 0;
      // Will return value needed for 'beginMoveRows'
      int iMoveToRows = layerChild->moveChildPosition( iMoveCount, iMoveToChild );
      if ( iMoveToChild != iMoveFrom )
      {
        // Using beginMoveRows and endMoveRows is an alternative to
        //  emitting layoutAboutToBeChanged and layoutChanged directly along with changePersistentIndexes.
        // Note that if sourceParent and destinationParent are the same,
        // you must ensure that the destinationChild is not within the range of sourceFirst and sourceLast + 1.
        // -> beginMoveRows need to know the new position at a time when the old position still exists, thus +1
        if ( beginMoveRows( parentIndex, iMoveFrom, iMoveFrom, parentIndex, iMoveToRows ) )
        {
          parentItem->moveChild( iMoveFrom, iMoveToChild );
          endMoveRows();
          returnIndex = node2index( parentItem->child( iMoveToChild ) );
        }
#if 0
        else
        {
          // Note that if sourceParent and destinationParent are the same,
          // you must ensure that the destinationChild is not within the range of sourceFirst and sourceLast + 1.
          // You must also ensure that you do not attempt to move a row to one of its own children or ancestors.
          // This method returns false if either condition is true, in which case you should abort your move operation.
          QgsDebugMsgLevel( QString( "-E-> QgsSpatialiteDbInfoModel::moveLayerOrderChild rc=failed  -3-  beginMoveRows( parentIndex,[%1], [%1], parentIndex, [%2]) MoveToChild[%3] LayerName[%4]" ).arg( iMoveFrom ).arg( iMoveToRows ).arg( iMoveToChild ).arg( layerChild->getLayerName() ), 7 );
        }
#endif
      }
    }
  }
  // If valid, View should call setCurrentIndex(returnIndex)
  return returnIndex;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setSelectedStyle
// The Item, when the correct Item-Type, will set the
// selected text() to the proper Layer column as selected Style
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::setSelectedStyle( QgsSpatialiteDbInfoItem *layerChild )
{
  bool bRc = false;
  if ( ( layerChild ) && ( layerChild->isValid() ) && ( layerChild->getGrandParent() ) )
  {
    // Since no changes in the Item-Position, but only changed text, use layoutAboutToBeChanged
    emit layoutAboutToBeChanged();
    bRc = layerChild->setSelectedStyle();
    // Text has been changed, repaint
    emit layoutChanged();
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getLayerNameUris
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getLayerNameUris( const QModelIndex &index ) const
{
  QString sLayerUris = QString();
  QString sLayerName = getLayerName( index );
  if ( !sLayerName.isEmpty() )
  {
    sLayerUris = mDbLayersDataSourceUris.value( sLayerName );
  }
  return sLayerUris;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getGeometryType
// - General retriever of text from the 'GeometryType' Column
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getGeometryType( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    return item->data( getGeometryTypeIndex() ).toString();
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getSqlQuery
// - General retriever of text from the 'SqlQuery' Column
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getSqlQuery( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    return item->data( getSqlQueryIndex() ).toString();
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getDbName
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getDbName( bool withPath ) const
{
  QString sDatabaseName = QString();
  if ( ( getSpatialiteDbInfo() ) && ( getSpatialiteDbInfo()->isDbValid() ) )
  {
    if ( withPath )
    {
      sDatabaseName = getSpatialiteDbInfo()->getDatabaseFileName();
    }
    else
    {
      sDatabaseName = getSpatialiteDbInfo()->getFileName();
    }
  }
  return sDatabaseName;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getDatabaseUri
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getDatabaseUri() const
{
  QString sDatabaseUri = QString();
  if ( ( getSpatialiteDbInfo() ) && ( getSpatialiteDbInfo()->isDbValid() ) )
  {
    sDatabaseUri = getSpatialiteDbInfo()->getDatabaseUri();
  }
  return sDatabaseUri;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::getSqlQuery
// - Special retriever of LayerSqlQuery
// This will return Empty if not isKayerSelectable
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::getLayerSqlQuery( const QModelIndex &index ) const
{
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isValid() ) )
  {
    return item->getLayerSqlQuery();
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::setLayerSqlQuery
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoModel::setLayerSqlQuery( const QModelIndex &index, const QString &sLayerSqlQuery )
{
  if ( !index.isValid() || !index.parent().isValid() )
  {
    return;
  }
  //find out table name
  QModelIndex tableSibling = index.sibling( index.row(), getTableNameIndex() );
  QModelIndex geomSibling = index.sibling( index.row(), getGeometryNameIndex() );

  if ( !tableSibling.isValid() || !geomSibling.isValid() )
  {
    return;
  }
  QgsSpatialiteDbInfoItem *item = getItem( index );
  if ( ( item ) && ( item->isLayerSelectable() ) )
  {
    item->setLayerSqlQuery( sLayerSqlQuery );
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::UpdateLayerStatistics
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::UpdateLayerStatistics( QStringList saLayers )
{
  if ( ( getSpatialiteDbInfo() ) && ( getSpatialiteDbInfo()->isDbValid() ) && ( getSpatialiteDbInfo()->isDbSpatialite() ) )
  {
    return getSpatialiteDbInfo()->UpdateLayerStatistics( saLayers );
  }
  return false;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::createDatabase
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoModel::createDatabase( QString sDatabaseFileName, QgsSpatialiteDbInfo::SpatialMetadata dbCreateOption )
{
  bool bRc = false;
  bool bLoadLayers = false;
  bool bShared = false;
  //----------------------------------------------------------
  QgsSpatiaLiteConnection connectionInfo( sDatabaseFileName );
  QgsSpatialiteDbInfo *spatialiteDbInfo = connectionInfo.CreateSpatialiteConnection( QString(), bLoadLayers, bShared, dbCreateOption );
  if ( spatialiteDbInfo )
  {
    if ( spatialiteDbInfo->isDbSqlite3() )
    {
      switch ( dbCreateOption )
      {
        case QgsSpatialiteDbInfo::Spatialite40:
        case QgsSpatialiteDbInfo::Spatialite50:
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case QgsSpatialiteDbInfo::SpatialiteLegacy:
            case QgsSpatialiteDbInfo::Spatialite40:
            case QgsSpatialiteDbInfo::Spatialite50:
              // this is a Database that can be used for QgsSpatiaLiteProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        case QgsSpatialiteDbInfo::SpatialiteGpkg:
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case QgsSpatialiteDbInfo::SpatialiteGpkg:
              // this is a Database that can be used for QgsOgrProvider or QgsGdalProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        case QgsSpatialiteDbInfo::SpatialiteMBTiles:
          switch ( spatialiteDbInfo->dbSpatialMetadata() )
          {
            case QgsSpatialiteDbInfo::SpatialiteMBTiles:
              // this is a Database that can be used for QgsGdalProvider
              bRc = true;
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
    if ( bRc )
    {
      setSpatialiteDbInfo( spatialiteDbInfo );
    }
    else
    {
      delete spatialiteDbInfo;
      spatialiteDbInfo = nullptr;
    }
  }
  //----------------------------------------------------------
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem constructor
// - to Build a Database
// OnInit will call sniffSpatialiteDbInfo
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem::QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, QgsSpatialiteDbInfo *spatialiteDbInfo )
{
  mParentItem = parent;
  mSpatialiteDbInfo = spatialiteDbInfo;
  if ( getSpatialiteDbInfo() )
  {
    mItemType = ItemTypeDb;
  }
  // Sets default Values/Settings, will only return true when further tasks must be called
  if ( onInit() )
  {
    // Pre-Conditions for this Item-Type are fulfilled
    switch ( mModelType )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        switch ( mItemType )
        {
          case ItemTypeDb:
            if ( buildDbInfoItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::SpatialiteDbInfo type not handeled  ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
        mIsItemValid = true;
      }
      break;
      default:
        break;
    }
  }
  if ( !isValid() )
  {
    // This should never happen
    QgsDebugMsgLevel( QStringLiteral( "-E--> QgsSpatialiteDbInfoItem::SpatialiteDbInfo -not valid- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem constructor
// - to Build a SpatialLayer
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem::QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, QgsSpatialiteDbLayer *dbLayer )
{
  mParentItem = parent;
  mDbLayer = dbLayer;
  if ( ( getDbLayer() ) && ( mParentItem ) )
  {
    if ( mParentItem->getModelType() == QgsSpatialiteDbInfoModel::ModelTypeConnections )
    {
      mItemType = ItemTypeLayer;
    }
    else
    {
      if ( mParentItem->getItemType() == ItemTypeLayerOrderVectorGroup )
      {
        mItemType = ItemTypeLayerOrderVectorLayer;
      }
      else if ( mParentItem->getItemType() == ItemTypeLayerOrderRasterGroup )
      {
        mItemType = ItemTypeLayerOrderRasterLayer;
      }
    }
  }
  // Sets default Values/Settings, will only return true when further tasks must be called
  if ( onInit() )
  {
    // Pre-Conditions for this Item-Type are fulfilled
    switch ( mModelType )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        switch ( mItemType )
        {
          case ItemTypeLayer:
            if ( buildDbLayerItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::SpatialiteDbLayer type not handeled  ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
        switch ( mItemType )
        {
          case ItemTypeLayerOrderVectorLayer:
          case ItemTypeLayerOrderRasterLayer:
            if ( buildLayerOrderItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::SpatialiteDbLayer type not handeled  ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      default:
        break;
    }
  }
  if ( !isValid() )
  {
    // This should never happen
    QgsDebugMsgLevel( QStringLiteral( "-E--> QgsSpatialiteDbInfoItem::SpatialiteDbLayer -not valid- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
  }
}
// -- ---------------------------------- --
// class QgsSpatialiteDbInfoItem
// - for Item Rows [with children columns]
// -- ---------------------------------- --
QgsSpatialiteDbInfoItem::QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType, QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType modelType )
{
  mParentItem = parent;
  mItemType = itemType;
  mModelType = modelType;
  // Sets default Values/Settings, will only return true when further tasks must be called
  if ( onInit() )
  {
    // Pre-Conditions for this Item-Type are fulfilled
    switch ( mModelType )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        switch ( mItemType )
        {
          case ItemTypeHelpRoot:
            if ( buildHelpItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::ItemType -type not handeled- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
        mIsItemValid = true;
      }
      break;
      default:
        break;
    }
  }
  if ( !isValid() )
  {
    // This should never happen
    QgsDebugMsgLevel( QStringLiteral( "-E-> QgsSpatialiteDbInfoItem::ItemType -not valid- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
  }
}
// -- ---------------------------------- --
// class QgsSpatialiteDbInfoItem
// - for Item Rows [with children columns]
// -- ---------------------------------- --
QgsSpatialiteDbInfoItem::QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType )
{
  mParentItem = parent;
  mItemType = itemType;
  // Sets default Values/Settings, will only return true when further tasks must be called
  if ( onInit() )
  {
    // Pre-Conditions for this Item-Type are fulfilled
    switch ( mModelType )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        switch ( mItemType )
        {
          case ItemTypeHelpRoot:
            if ( buildHelpItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeMetadataRoot:
            if ( buildMetadataRootItem() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeGroupNonSpatialTables:
            // called from createGroupLayerItem
            if ( buildNonSpatialTables() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeSpatialRefSysAuxGroups:
            // called from createGroupLayerItem
            if ( buildSpatialRefSysAuxGroup() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeGroupSpatialTable:
          case ItemTypeGroupSpatialView:
          case ItemTypeGroupVirtualShape:
          case ItemTypeGroupTopologyExport:
          case ItemTypeGroupGeoPackageVector:
          case ItemTypeGroupGdalFdoOgr:
          case ItemTypeGroupRasterLite2Raster:
          case ItemTypeGroupGeoPackageRaster:
          case ItemTypeGroupRasterLite1:
          // Either MBTilesTable or MBTilesView can be used [same result]
          case ItemTypeGroupMBTilesTable:
          case ItemTypeGroupMBTilesView:
            // called from createGroupLayerItem
            if ( buildSpatialSubGroups() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          // Either StyleVector or StyleRaster can be used [same result]
          case ItemTypeGroupStyleVector:
          case ItemTypeGroupStyleRaster:
            // called from createGroupLayerItem
            if ( buildStylesSubGroups() > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::ItemType -type not handeled- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
        mIsItemValid = true;
      }
      break;
      default:
        break;
    }
  }
  if ( !isValid() )
  {
    // This should never happen
    QgsDebugMsgLevel( QStringLiteral( "-E-> QgsSpatialiteDbInfoItem::ItemType -not valid- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
  }
}
// -- ---------------------------------- --
// class QgsSpatialiteDbInfoItem
// - for Item Rows [with children columns]
// -- ---------------------------------- --
QgsSpatialiteDbInfoItem::QgsSpatialiteDbInfoItem( QgsSpatialiteDbInfoItem *parent, SpatialiteDbInfoItemType itemType, QString sItemName, QStringList saItemInfos )
{
  mParentItem = parent;
  mItemType = itemType;
  // Sets default Values/Settings, will only return true when further tasks must be called
  if ( onInit() )
  {
    // Pre-Conditions for this Item-Type are fulfilled
    switch ( mModelType )
    {
      case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      {
        switch ( mItemType )
        {
          case ItemTypeNonSpatialTablesSubGroups:
            // called from buildNonSpatialTables
            if ( buildNonSpatialTablesSubGroups( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeNonSpatialTablesSubGroup:
            // called from buildNonSpatialTablesSubGroups
            if ( buildNonSpatialTablesSubGroup( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeNonSpatialTable:
            // called from buildNonSpatialTables or buildNonSpatialTablesSubGroups
            if ( buildNonSpatialTable( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeSpatialRefSysAuxSubGroup:
            // called from buildNonSpatialTables or buildNonSpatialTablesSubGroups
            if ( buildSpatialRefSysAuxSubGroup( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeGroupSpatialTable:
          case ItemTypeGroupSpatialView:
          case ItemTypeGroupVirtualShape:
          case ItemTypeGroupTopologyExport:
          case ItemTypeGroupGeoPackageVector:
          case ItemTypeGroupGdalFdoOgr:
          case ItemTypeGroupRasterLite2Raster:
          case ItemTypeGroupGeoPackageRaster:
          case ItemTypeGroupRasterLite1:
          // Either MBTilesTable or MBTilesView can be used [same result]
          case ItemTypeGroupMBTilesTable:
          case ItemTypeGroupMBTilesView:
            // called from createGroupLayerItem or buildSpatialSubGroups
            if ( buildSpatialSubGroup( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          // Either StyleVector or StyleRaster can be used [same result]
          case ItemTypeGroupStyleVector:
          case ItemTypeGroupStyleRaster:
            if ( buildStylesGroupItem( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          case ItemTypeMetadataGroup:
            if ( buildMetadataGroupItem( sItemName, saItemInfos ) > 0 )
            {
              mIsItemValid = true;
            }
            break;
          default:
            // This should never happen
            QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::ItemType,ItemName,ItemInfos -type not handeled- ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
            break;
        }
      }
      break;
      case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      {
      }
      break;
      default:
        break;
    }
  }
  if ( !isValid() )
  {
    // This should never happen
    QgsDebugMsgLevel( QStringLiteral( "-E-> QgsSpatialiteDbInfoItem::ItemType,ItemName,ItemInfos -not valid- ItemType[%1] ItemName[%2] saItemInfos.count[%3]" ).arg( getItemTypeString() ).arg( sItemName ).arg( saItemInfos .count() ), 7 );
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setSpatialiteDbInfo
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::getCopyCellText( int iColumn, int &iCommandRole, QString &sMenuText, QString &sToolTip )
{
  bool bRc = false;
  if ( ( iColumn < 0 ) && ( iColumn >= getColumnSortHidden() ) )
  {
    iColumn = -1;
  }
  iCommandRole = -1; // 0=copy ; 1=edit ; 2=remove
  // sMenuText = sToolTip;
  switch ( getItemType() )
  {
    case ItemTypeRoot:
    case ItemTypeMetadataGroup:
    case ItemTypeGroupSpatialTable:
    case ItemTypeGroupSpatialView:
    case ItemTypeGroupVirtualShape:
    case ItemTypeGroupRasterLite1:
    case ItemTypeGroupRasterLite2Vector:
    case ItemTypeGroupRasterLite2Raster:
    case ItemTypeGroupSpatialiteTopology:
    case ItemTypeGroupTopologyExport:
    case ItemTypeGroupStyleVector:
    case ItemTypeGroupStyleRaster:
    case ItemTypeGroupGdalFdoOgr:
    case ItemTypeGroupGeoPackageVector:
    case ItemTypeGroupGeoPackageRaster:
    case ItemTypeGroupMBTilesTable:
    case ItemTypeGroupMBTilesView:
    case ItemTypeGroupMetadata:
    case ItemTypeGroupAllSpatialLayers:
    case ItemTypeGroupNonSpatialTables:
    case ItemTypeSpatialRefSysAuxGroups:
    case ItemTypeSpatialRefSysAuxSubGroup:
    case ItemTypeNonSpatialTablesSubGroups:
    case ItemTypeNonSpatialTablesSubGroup:
    case ItemTypeGroupWarning:
    case ItemTypeGroupError:
    {
      // Ignore for all Columns, no copy
      return bRc;
    }
    case ItemTypeDb:
    case ItemTypeLayer:
    case ItemTypeStylesMetadata:
    case ItemTypeWarning:
    case ItemTypeError:
    {
      // Allow for all Columns, copy
      iCommandRole = 0;
      sMenuText = QStringLiteral( "Copy Cell text" );
      sToolTip = QStringLiteral( "Copy Cell text for ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() );
      bRc = true;
      return bRc;
    }
    break;
    case ItemTypeLayerOrderVectorGroup:
    case ItemTypeLayerOrderRasterGroup:
      // allow remove switch to GroupItem
      iCommandRole = 2;
      sMenuText = QStringLiteral( "Remove all Items is this Group" );
      sToolTip = QStringLiteral( "Remove all Items is this Group ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() );
      break;
    case ItemTypeLayerOrderVectorLayer:
    case ItemTypeLayerOrderRasterLayer:
      // allow copy and remove switch to LayerItem
      iCommandRole = 20;
      sMenuText = QStringLiteral( "Remove this Item from this Group" );
      sToolTip = QStringLiteral( "Remove this Item from this Group ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() );
      break;
    default:
      break;
  }
  // TODO: SpatialRefSysAux
  if ( iColumn == getTableNameIndex() )
  {
    // Column 0: getTableNameIndex()
    bRc = true;
    switch ( getItemType() )
    {
      case ItemTypeLayer:
      case ItemTypeNonSpatialTable:
      {
        bRc = true;
      }
      break;
      default:
        break;
    }
    switch ( getItemType() )
    {
      case ItemTypeCommonMetadata:
      case ItemTypeMBTilesMetadata:
      case ItemTypePointMetadata:
      case ItemTypeRectangleMetadata:
      case ItemTypeSpatialRefSysAux:
      {
        // Description only, no copy
        bRc = false;
      }
      break;
      default:
        break;
    }
  }
  else if ( iColumn == getGeometryNameIndex() )
  {
    // Column 1: getGeometryNameIndex()
    bRc = true;
    switch ( getItemType() )
    {
      case ItemTypeCommonMetadata:
      case ItemTypeMBTilesMetadata:
      case ItemTypePointMetadata:
      case ItemTypeRectangleMetadata:
      case ItemTypeNonSpatialTable:
      case ItemTypeSpatialRefSysAux:
      {
        // Extent only, copy and change postion
        bRc = true;
      }
      break;
      default:
        break;
    }
    switch ( getItemType() )
    {
      case ItemTypeMBTilesMetadata:
      {
        // allow switch to Editable [MBTiles-Metadata, values]
        iCommandRole = 1;
      }
      break;
      default:
        break;
    }
  }
  else if ( iColumn == getGeometryTypeIndex() )
  {
    // Column 2: getGeometryTypeIndex()
    bRc = true;
    switch ( getItemType() )
    {
      case ItemTypeCommonMetadata:
      case ItemTypeMBTilesMetadata:
      case ItemTypePointMetadata:
      case ItemTypeRectangleMetadata:
      case ItemTypeNonSpatialTable:
      case ItemTypeSpatialRefSysAux:
      {
        // Empty, no copy
        bRc = false;
      }
      break;
      default:
        break;
    }
  }
  else if ( iColumn == getSqlQueryIndex() )
  {
    // Column 3: getSqlQueryIndex()
    bRc = true;
    switch ( getItemType() )
    {
      case ItemTypeCommonMetadata:
      case ItemTypeMBTilesMetadata:
      case ItemTypePointMetadata:
      case ItemTypeRectangleMetadata:
      case ItemTypeColumn:
      case ItemTypeNonSpatialTable:
      case ItemTypeSpatialRefSysAux:
      {
        // Empty, no copy
        bRc = false;
      }
      break;
      default:
        break;
    }
  }
  else
  {
    // Column 4 [should always be  used]: getColumnSortHidden()
    bRc = false;
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setSpatialiteDbInfo
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setSpatialiteDbInfo( QgsSpatialiteDbInfo *spatialiteDbInfo )
{
  if ( ( spatialiteDbInfo ) && ( spatialiteDbInfo->isDbValid() ) )
  {
    mSpatialiteDbInfo = spatialiteDbInfo;
    mSpatialMetadata = getSpatialiteDbInfo()->dbSpatialMetadata();
    mSpatialMetadataString = getSpatialiteDbInfo()->dbSpatialMetadataString();
    return getSpatialiteDbInfo()->isDbValid();
  }
  return false;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setDbLayer
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setDbLayer( QgsSpatialiteDbLayer *dbLayer )
{
  bool bRc = false;
  if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
  {
    mDbLayer = dbLayer;
    setSpatialiteDbInfo( getDbLayer()->getSpatialiteDbInfo() );
    if ( mParentItem )
    {
      mParentItem->setDbLayer( getDbLayer() );
    }
    bRc = true;
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::getSpatialiteDbInfoErrors
//-----------------------------------------------------------------
// - only when geometryTypes
// - this does not include the geometry of the Layer
//-----------------------------------------------------------------
QMap<QString, QString> QgsSpatialiteDbInfoItem::getSpatialiteDbInfoErrors( bool bWarnings ) const
{
  QMap<QString, QString> dbErrors;
  if ( getSpatialiteDbInfo() )
  {
    if ( getDbLayer() )
    {
      if ( bWarnings )
      {
        dbErrors = getDbLayer()->getLayerWarnings();
      }
      else
      {
        dbErrors = getDbLayer()->getLayerErrors();
      }
    }
    else
    {
      if ( bWarnings )
      {
        dbErrors = getSpatialiteDbInfo()->getDbWarnings();
      }
      else
      {
        dbErrors = getSpatialiteDbInfo()->getDbErrors();
      }
    }
  }
  return dbErrors;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::addColumnItems()
//-----------------------------------------------------------------
// - only when geometryTypes
// - this does not include the geometry of the Layer
// Called from buildMetadataGroupItem
// - List other columns of the Layer, excluding the used geometry
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addColumnItems()
{
  if ( getDbLayer() )
  {
    QColor fg_color = QColor( "yellow" ); // gold "khaki"
    QColor bg_color = QColor( "lightyellow" );
    QColor bg_color_Primary_Key = QColor( "darkcyan" );
    QgsFields attributeFields = getDbLayer()->getAttributeFields();
    // Column 1: getGeometryNameIndex()
    QString sGeometryColumn = QStringLiteral( "Fields count : %1" ).arg( ( attributeFields.count() + 1 ) );
    setText( getGeometryNameIndex(), sGeometryColumn );
    sGeometryColumn += QStringLiteral( " [including active geometry]" );
    setToolTip( getGeometryNameIndex(), sGeometryColumn );
    QString sPrimaryKey = getDbLayer()->getPrimaryKey();
    for ( int i = 0; i < attributeFields.count(); i++ )
    {
      QString sName = attributeFields.at( i ).name();
      QString sType = attributeFields.at( i ).typeName();
      QgsWkbTypes::Type geometryType = QgsSpatialiteDbLayer::GetGeometryTypeLegacy( sType );
      QIcon iconType;
      if ( geometryType != QgsWkbTypes::Unknown )
      {
        iconType = QgsSpatialiteDbInfo::SpatialGeometryTypeIcon( geometryType );
      }
      else
      {
        iconType = QgsSpatialiteDbInfo::QVariantTypeIcon( attributeFields.at( i ).type() );
      }
      QString sComment = attributeFields.at( i ).comment();
      QString sSortTag = QStringLiteral( "AZZC_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( i );
      QString sCommentGeometryType = QString();
      if ( getDbLayer()->getCapabilitiesString().startsWith( "SpatialView(" ) )
      {
        if ( getDbLayer()->getCapabilitiesString().startsWith( "SpatialView(ReadOnly)" ) )
        {
          sCommentGeometryType = QStringLiteral( "Readonly-SpatialView: Default-Values from from CREATE TABLE-Sql of underling Table and WHERE conditions of View" );
        }
        else
        {
          sCommentGeometryType = QStringLiteral( "Writable-SpatialView: Default-Values from CREATE TABLE-Sql of underling Table and defaults set in Triggers " );
        }
      }
      else if ( getDbLayer()->getCapabilitiesString().startsWith( "SpatialTable(" ) )
      {
        sCommentGeometryType = QStringLiteral( "SpatialTable: Default-Values from CREATE TABLE-Sql " );
      }
      QgsSpatialiteDbInfoItem *dbColumnItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeColumn );
      if ( dbColumnItem )
      {
        // Column 0: getTableNameIndex()
        dbColumnItem->setText( getTableNameIndex(), sName );
        dbColumnItem->setBackground( getTableNameIndex(), bg_color );
        // Column 1: getGeometryNameIndex()
        dbColumnItem->setText( getGeometryNameIndex(), sType );
        dbColumnItem->setIcon( getGeometryNameIndex(), iconType );
        dbColumnItem->setBackground( getGeometryNameIndex(), bg_color );
        // Column 2: getGeometryTypeIndex()
        if ( sName == sPrimaryKey )
        {
          dbColumnItem->setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::QVariantTypeIcon( QVariant::KeySequence ) );
          dbColumnItem->setBackground( getGeometryTypeIndex(), bg_color_Primary_Key );
          dbColumnItem->setForeground( getGeometryTypeIndex(), fg_color );
          if ( sComment.isEmpty() )
          {
            sComment = QString( "Primary-Key" );
          }
          else
          {
            if ( sComment !=  QStringLiteral( "PRIMARY KEY" ) )
            {
              sComment = QString( "[Primary-Key] %1" ).arg( sComment );
            }
          }
        }
        else
        {
          dbColumnItem->setBackground( getGeometryTypeIndex(), bg_color );
        }
        dbColumnItem->setText( getGeometryTypeIndex(), sComment );
        if ( !sCommentGeometryType.isEmpty() )
        {
          dbColumnItem->setToolTip( getGeometryTypeIndex(), sCommentGeometryType );
        }
        // Column 3 [not used]: getSqlQueryIndex()
        // Column 4 [should always be  used]: getColumnSortHidden()
        dbColumnItem->setText( getColumnSortHidden(), sSortTag );
        mPrepairedChildItems.append( dbColumnItem );
      }
    }
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::addLayerStylesItems
//-----------------------------------------------------------------
// - QgsSpatialiteDbInfo::ParseSeparatorCoverage ('€') is used a separator
// - Key: StyleInfo retrieved from SE_vector_styled_layers_view / SE_raster_styled_layers_view
// - Value: StyleName, StyleTitle, StyleAbstract,TestStylesForQgis
// -> style_type: StyleVector,StyleRaster
// Called from buildMetadataGroupItem
// List registered Styles of the Layer
// - not of the Database [Database is done in buildStylesSubGroups]
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addLayerStylesItems()
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) && ( getDbLayer()->hasLayerStyle() ) )
  {
    QString sStyleName;
    QString sStyleTitle;
    QString sStyleAbstract;
    QString sTestStylesForQGis;
    QString sSortTag;
    QColor fg_color = QColor( "yellow" );
    QColor bg_color = QColor( "lightyellow" );
    QColor bg_color_Selected = QColor( "darkcyan" );
    QgsSpatialiteDbInfo::SpatialiteLayerType styleType = QgsSpatialiteDbInfo::Metadata;
    if ( mIsVectorType )
    {
      styleType = QgsSpatialiteDbInfo::StyleVector;
    }
    else
    {
      styleType = QgsSpatialiteDbInfo::StyleRaster;
    }
    //-----------------------------------------------------------------
    QIcon iconType = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( styleType );
    QString sStyleType = QgsSpatialiteDbInfo::SpatialiteLayerTypeName( styleType );
    int iStyleIdSelected = getDbLayer()->getLayerStyleIdSelected();
    QMap<int, QString> layerStyles =  getDbLayer()->getLayerCoverageStylesInfo();
    // Column 1: getGeometryNameIndex()
    QString sGeometryColumn = QString( "Styles count : %1" ).arg( ( layerStyles.count() ) );
    setText( getGeometryNameIndex(), sGeometryColumn );
    sGeometryColumn += QString( " [the first style will be used as the default style]" );
    setToolTip( getGeometryNameIndex(), sGeometryColumn );
    for ( QMap<int, QString>::iterator itLayers = layerStyles.begin(); itLayers != layerStyles.end(); ++itLayers )
    {
      int iStyleId = itLayers.key();
      if ( QgsSpatialiteDbInfo::parseStyleInfo( itLayers.value(), sStyleName, sStyleTitle, sStyleAbstract, sTestStylesForQGis ) )
      {
        sSortTag = QString( "AZZL_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( sStyleName );
        QgsSpatialiteDbInfoItem *dbStyleItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
        if ( dbStyleItem )
        {
          // Column 0: getTableNameIndex()
          dbStyleItem->setText( getTableNameIndex(), sStyleName );
          if ( iStyleIdSelected == iStyleId )
          {
            QString sToolTipText = QStringLiteral( "default Style [%1]" ).arg( sStyleName );
            dbStyleItem->setToolTip( getTableNameIndex(), sToolTipText );
            dbStyleItem->setForeground( getTableNameIndex(), fg_color );
            dbStyleItem->setBackground( getTableNameIndex(), bg_color_Selected );
            dbStyleItem->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::QVariantTypeIcon( QVariant::KeySequence ) );
          }
          else
          {
            dbStyleItem->setToolTip( getTableNameIndex(), sStyleName );
            dbStyleItem->setBackground( getTableNameIndex(), bg_color );
            dbStyleItem->setIcon( getTableNameIndex(), iconType );
          }
          // Column 1: getGeometryNameIndex()
          dbStyleItem->setText( getGeometryNameIndex(), sStyleTitle );
          dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
          // These can be long string descriptions, add ToolTip
          dbStyleItem->setToolTip( getGeometryNameIndex(), sStyleTitle );
          // dbStyleItem->setEditable( getGeometryTypeIndex(), true );
          dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
          // Column 2: getGeometryTypeIndex()
          dbStyleItem->setText( getGeometryTypeIndex(), sStyleType );
          dbStyleItem->setToolTip( getGeometryTypeIndex(), sStyleType );
          dbStyleItem->setIcon( getGeometryTypeIndex(), iconType );
          dbStyleItem->setBackground( getGeometryTypeIndex(), bg_color );
          // Column 3: getSqlQueryIndex()
          dbStyleItem->setText( getSqlQueryIndex(), sStyleAbstract );
          dbStyleItem->setToolTip( getSqlQueryIndex(), sStyleAbstract );
          // Column 4 [should always be  used]: getColumnSortHidden()
          dbStyleItem->setText( getColumnSortHidden(), sSortTag );
          mPrepairedChildItems.append( dbStyleItem );
        }
      }
    }
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::addCommonMetadataItems
//-----------------------------------------------------------------
// This List will be shown after the Columns [Vector-only]
// - not all Meta informaton is supported by all Layer-Types
// Called from buildMetadataGroupItem
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addCommonMetadataItems()
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) )
  {
    QColor bg_color_base = QColor( "paleturquoise" );
    QColor bg_color = bg_color_base;
    QColor bg_color_srid = QColor( "khaki" );
    QColor fg_color_base = QColor( "black" );
    QColor fg_color = fg_color_base;
    QColor bg_color_extent = QColor( "lightyellow" );
    QColor bg_color_srid_default = QColor( "steelblue" );
    QColor fg_color_srid_default = QColor( "yellow" );
    bool bMetadataExtended = false;
    if ( mIsRasterLite2 )
    {
      bMetadataExtended = true;
    }
    QgsLayerMetadata layerMetadata = getDbLayer()->getLayerMetadata();
    QString sToolTipText = QStringLiteral( "Metaddata [%1]" ).arg( getLayerTypeString() );
    QList<QPair<QString, QString>> metaData;
    metaData.append( qMakePair( QStringLiteral( "Capabilities" ), getDbLayer()->getCapabilitiesString() ) );
    if ( mIsVectorType )
    {
      metaData.append( qMakePair( QStringLiteral( "Features" ), QString::number( getDbLayer()->getFeaturesCount() ) ) );
    }
    if ( ( !mIsMbTiles ) && ( !mIsRasterLite1 ) )
    {
      metaData.append( qMakePair( QStringLiteral( "Title" ), getDbLayer()->getTitle() ) );
      metaData.append( qMakePair( QStringLiteral( "Abstract" ), getDbLayer()->getAbstract() ) );
    }
    if ( bMetadataExtended )
    {
      metaData.append( qMakePair( QStringLiteral( "Copyright" ), getDbLayer()->getCopyright() ) );
    }
    if ( !mIsVectorType )
    {
      metaData.append( qMakePair( QStringLiteral( "Resolution - X" ), QString::number( getDbLayer()->getLayerImageResolutionX(), 'f', 7 ) ) );
      metaData.append( qMakePair( QStringLiteral( "Resolution - Y" ), QString::number( getDbLayer()->getLayerImageResolutionY(), 'f', 7 ) ) );
      metaData.append( qMakePair( QStringLiteral( "Image-Size" ), QString( "%1x%2" ).arg( getDbLayer()->getLayerImageWidth() ).arg( getDbLayer()->getLayerImageHeight() ) ) );
      metaData.append( qMakePair( QStringLiteral( "Bands" ), QString::number( getDbLayer()->getLayerNumBands() ) ) );
      //if ( mIsRasterLite2 )
      metaData.append( qMakePair( QStringLiteral( "Data Type" ), getDbLayer()->getLayerRasterDataTypeString() ) );
      metaData.append( qMakePair( QStringLiteral( "Pixel Type" ), getDbLayer()->getLayerRasterPixelTypeString() ) );
      metaData.append( qMakePair( QStringLiteral( "Compression Type" ), getDbLayer()->getLayerRasterCompressionType() ) );
      metaData.append( qMakePair( QStringLiteral( "Tile Size" ), QString( "%1x%2" ).arg( getDbLayer()->getLayerTileWidth() ).arg( getDbLayer()->getLayerTileHeight() ) ) );
    }
    for ( int i = 0; i < metaData.count(); i++ )
    {
      QPair<QString, QString> pair = metaData.at( i );
      if ( !pair.second.isEmpty() )
      {
        QString sName = pair.first;
        QString sValue = pair.second;
        // Make sure that this does not get re-sorted when more that 10 entries ['0001']
        QString sSortTag = QStringLiteral( "AZZM_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( QStringLiteral( "%1" ).arg( i, 5, 10, QChar( '0' ) ) );
        QgsSpatialiteDbInfoItem *dbColumnItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeCommonMetadata );
        if ( dbColumnItem )
        {
          // Column 0: getTableNameIndex()
          dbColumnItem->setText( getTableNameIndex(), sName );
          dbColumnItem->setToolTip( getTableNameIndex(), sToolTipText );
          dbColumnItem->setBackground( getTableNameIndex(), bg_color );
          // Column 1: getGeometryNameIndex()
          dbColumnItem->setText( getGeometryNameIndex(), sValue );
          // These can be long string descriptions, add ToolTip
          dbColumnItem->setToolTip( getGeometryNameIndex(), sValue );
          // dbColumnItem->setEditable( getGeometryTypeIndex(), true );
          dbColumnItem->setBackground( getGeometryNameIndex(), bg_color );
          // Column 2 [not used]: getGeometryTypeIndex()
          // Column 3 [not used]: getSqlQueryIndex()
          // Column 4 [should always be  used]: getColumnSortHidden()
          dbColumnItem->setText( getColumnSortHidden(), sSortTag );
          mPrepairedChildItems.append( dbColumnItem );
        }
      }
    }
    metaData.clear();
    int iDefaultSrid = getDbLayer()->getSrid();
    QMap<int, QgsBox3d> layerExtents = getDbLayer()->getLayerExtents();
    QString sName;
    QString sValue;
    for ( QMap<int, QgsBox3d>::iterator itLayers = layerExtents.begin(); itLayers != layerExtents.end(); ++itLayers )
    {
      int iSrid = itLayers.key();
      QgsBox3d sridExtent = itLayers.value();
      // Make sure that this does not get re-sorted when more that 10 entries ['0001']
      QString sSortTag = QStringLiteral( "AZZM_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( QStringLiteral( "%1" ).arg( iSrid, 5, 10, QChar( '0' ) ) );
      SpatialiteDbInfoItemType itemTypeMetadata = ItemTypeRectangleMetadata;
      QgsSpatialiteDbInfoItem *dbSridItem = new QgsSpatialiteDbInfoItem( this, itemTypeMetadata );
      if ( dbSridItem )
      {
        // Column 0: getTableNameIndex()
        sName = QStringLiteral( "Extent EPSG:%1" ).arg( iSrid );
        sValue = sridExtent.toRectangle().asWktCoordinates();
        dbSridItem->setText( getTableNameIndex(), sName );
        // Column 1: getGeometryNameIndex()
        dbSridItem->setText( getGeometryNameIndex(), sValue );
        sToolTipText = QStringLiteral( "Alternative Srid EPSG:%1" ).arg( iSrid );
        bg_color = bg_color_base; // paleturquoise
        fg_color = fg_color_base; // black
        if ( iSrid == iDefaultSrid )
        {
          dbSridItem->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::QVariantTypeIcon( QVariant::KeySequence ) );
          bg_color = bg_color_srid_default; // blue
          fg_color = fg_color_srid_default;
          sToolTipText = QStringLiteral( "Native Srid EPSG:%1" ).arg( iSrid );
        }
        dbSridItem->setForeground( getTableNameIndex(), fg_color );
        dbSridItem->setForeground( getGeometryNameIndex(), fg_color );
        dbSridItem->setToolTip( getTableNameIndex(), sToolTipText );
        dbSridItem->setBackground( getTableNameIndex(), bg_color );
        dbSridItem->setToolTip( getGeometryNameIndex(), sValue );
        dbSridItem->setBackground( getGeometryNameIndex(), bg_color );
        // Column 2 [not used]: getGeometryTypeIndex()
        // Column 3 [not used]: getSqlQueryIndex()
        // Column 4 [should always be  used]: getColumnSortHidden()
        dbSridItem->setText( getColumnSortHidden(), sSortTag );
        metaData.append( qMakePair( QStringLiteral( "-> Min X, Y" ), QStringLiteral( "%1, %2" ).arg( QString::number( sridExtent.xMinimum(), 'f', 7 ) ).arg( QString::number( sridExtent.yMinimum(), 'f', 7 ) ) ) );
        metaData.append( qMakePair( QStringLiteral( "-> Max X, Y" ), QStringLiteral( "%1, %2" ).arg( QString::number( sridExtent.xMaximum(), 'f', 7 ) ).arg( QString::number( sridExtent.yMaximum(), 'f', 7 ) ) ) );
        metaData.append( qMakePair( QStringLiteral( "-> Center" ), sridExtent.toRectangle().center().toString( 7 ) ) );
        metaData.append( qMakePair( QStringLiteral( "-> Width, Height" ), QStringLiteral( "%1, %2" ).arg( QString::number( sridExtent.width(), 'f', 7 ) ).arg( QString::number( sridExtent.height(), 'f', 7 ) ) ) );
        bg_color = bg_color_extent;
        if ( iSrid == iDefaultSrid )
        {
          bg_color = bg_color_srid ;
        }
        for ( int i = 0; i < metaData.count(); i++ )
        {
          QPair<QString, QString> pair = metaData.at( i );
          if ( !pair.first.isEmpty() )
          {
            itemTypeMetadata = ItemTypePointMetadata;
            if ( i == ( metaData.count() - 1 ) )
            {
              itemTypeMetadata = ItemTypeCommonMetadata;
            }
            QString sName = pair.first;
            QString sValue = pair.second;
            // Make sure that this does not get re-sorted when more that 10 entries ['0001']
            QString sSortTag = QString( "AZZM_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( QStringLiteral( "%1" ).arg( i, 5, 10, QChar( '0' ) ) );
            QgsSpatialiteDbInfoItem *dbExtentItem = new QgsSpatialiteDbInfoItem( dbSridItem, itemTypeMetadata );
            if ( dbExtentItem )
            {
              // Column 0: getTableNameIndex()
              dbExtentItem->setText( getTableNameIndex(), sName );
              dbExtentItem->setToolTip( getTableNameIndex(), sToolTipText );
              dbExtentItem->setBackground( getTableNameIndex(), bg_color );
              // Column 1: getGeometryNameIndex()
              dbExtentItem->setText( getGeometryNameIndex(), sValue );
              // These can be long string descriptions, add ToolTip
              dbExtentItem->setToolTip( getGeometryNameIndex(), sValue );
              // dbColumnItem->setEditable( getGeometryTypeIndex(), true );
              dbExtentItem->setBackground( getGeometryNameIndex(), bg_color );
              // Column 2 [not used]: getGeometryTypeIndex()
              // Column 3 [not used]: getSqlQueryIndex()
              // Column 4 [should always be  used]: getColumnSortHidden()
              dbExtentItem->setText( getColumnSortHidden(), sSortTag );
              dbSridItem->mPrepairedChildItems.append( dbExtentItem );
            }
          }
        }
        metaData.clear();
        // mPrepairedChildItems will be emptied
        dbSridItem->insertPrepairedChildren();
        mPrepairedChildItems.append( dbSridItem );
      }
    }
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::addMbTilesMetadataItems
//-----------------------------------------------------------------
// - List the entries in the MbTiles specifice metadata TABLE
// Called from buildMetadataGroupItem
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addMbTilesMetadataItems()
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) )
  {
    QColor bg_color = QColor( "lightyellow" );
    QColor bg_edit = QColor( "darkseagreen" );
    QMap<QString, QString> metaData = getSpatialiteDbInfo()->getDbMBTileMetaData();
    // Column 1: getGeometryNameIndex()
    QString sGeometryColumn = QStringLiteral( "metadata entries : %1" ).arg( ( metaData.count() ) );
    setText( getGeometryNameIndex(), sGeometryColumn );
    sGeometryColumn += QStringLiteral( " [from the MBTiles specific 'metadata' TABLE]" );
    setToolTip( getGeometryNameIndex(), sGeometryColumn );
    for ( QMap<QString, QString>::iterator itLayers = metaData.begin(); itLayers != metaData.end(); ++itLayers )
    {
      if ( !itLayers.key().isEmpty() )
      {
        QString sName = itLayers.key();
        QString sValue = itLayers.value();
        QString sSortTag = QStringLiteral( "AZZC_%1_%2" ).arg( getDbLayer()->getLayerName() ).arg( sName );
        QgsSpatialiteDbInfoItem *dbMetadataItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeMBTilesMetadata );
        if ( dbMetadataItem )
        {
          // Column 0: getTableNameIndex()
          dbMetadataItem->setText( getTableNameIndex(), sName );
          dbMetadataItem->setToolTip( getTableNameIndex(), QStringLiteral( "Entry of MbTiles 'metadata' TABLE" ) );
          dbMetadataItem->setBackground( getTableNameIndex(), bg_color );
          // Column 1: getGeometryNameIndex()
          dbMetadataItem->setText( getGeometryNameIndex(), sValue );
          // These can be long string descriptions, add ToolTip
          dbMetadataItem->setToolTip( getGeometryNameIndex(), sValue );
          dbMetadataItem->setEditable( getGeometryNameIndex(), true );
          dbMetadataItem->setBackground( getGeometryNameIndex(), bg_edit );
          // Column 2 [not used]: getGeometryTypeIndex()
          // Column 3 [not used]: getSqlQueryIndex()
          // Column 4 [should always be  used]: getColumnSortHidden()
          dbMetadataItem->setText( getColumnSortHidden(), sSortTag );
          mPrepairedChildItems.append( dbMetadataItem );
        }
      }
    }
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::~QgsSpatialiteDbInfoItem
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem::~QgsSpatialiteDbInfoItem()
{
  qDeleteAll( mChildItems );
  mItemData.clear();
  if ( getDbLayer() )
  {
    // Do not delete, may be used elsewhere
    mDbLayer = nullptr;
  }
  if ( getSpatialiteDbInfo() )
  {
    // Do not delete, may be used elsewhere
    mSpatialiteDbInfo = nullptr;
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::onInit
// Common Tasks:
// - Set initial Values, such as Column-Headers and Positions [mModelRootItem]
// - retrieve values from Parent
// Only when bRc=true, will certain (mItemType specific tasks) will be called
// - returning bRc=false is not an error, only a sign that all tasks are compleated
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::onInit()
{
  bool bRc = false;
  // Qt::ItemFlags flags = ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
  Qt::ItemFlags flags = ( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
  bool bSpatialiteDbInfo = false;
  bool bDbLayer = false;
  if ( !mParentItem )
  {
    if ( mItemType == ItemTypeRoot )
    {
      // This is the mModelRootItem
      mColumnTable = 0;
      if ( mModelType == QgsSpatialiteDbInfoModel::ModelTypeConnections )
      {
        mColumnGeometryName = getTableNameIndex() + 1;
        mColumnGeometryType = getGeometryNameIndex() + 1;
        mColumnSql = getGeometryTypeIndex() + 1;
        mColumnSortHidden = getSqlQueryIndex() + 1; // Must be the last, since (mColumnSortHidden+1) == columnCount
        // The 'mColumn*' values must be set before setText and setFlags can be used
        setText( getTableNameIndex(), QStringLiteral( "Table" ) );
        setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialTable ) );
        setAlignment( getTableNameIndex(), Qt::AlignCenter ); //Qt::AlignJustify
        setText( getGeometryNameIndex(), QStringLiteral( "Geometry-Column" ) );
        setAlignment( getGeometryNameIndex(), Qt::AlignCenter );
        setText( getGeometryTypeIndex(), QStringLiteral( "Geometry-Type" ) );
        setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialGeometryTypeIcon( QgsWkbTypes::Polygon ) );
        setAlignment( getGeometryTypeIndex(), Qt::AlignCenter );
        setText( getSqlQueryIndex(), QStringLiteral( "Sql" ) );
        setAlignment( getSqlQueryIndex(), Qt::AlignCenter );
        setText( getColumnSortHidden(), QStringLiteral( "ColumnSortHidden" ) );
      }
      if ( mModelType == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder )
      {
        mColumnGeometryName = getTableNameIndex() + 1;
        mColumnGeometryType = getGeometryNameIndex() + 1;
        mColumnSql = getGeometryTypeIndex() + 1;
        mColumnSortHidden = getSqlQueryIndex() + 1; // Must be the last, since (mColumnSortHidden+1) == columnCount
        // The 'mColumn*' values must be set before setText and setFlags can be used
        setText( getTableNameIndex(), QStringLiteral( "Layer-Name" ) );
        setAlignment( getTableNameIndex(), Qt::AlignCenter ); //Qt::AlignJustify
        setText( getGeometryNameIndex(), QStringLiteral( "Layer-Style" ) );
        setAlignment( getGeometryNameIndex(), Qt::AlignCenter );
        setText( getGeometryTypeIndex(), QStringLiteral( "Title" ) );
        setAlignment( getGeometryTypeIndex(), Qt::AlignCenter );
        setText( getSqlQueryIndex(), QStringLiteral( "Abstract" ) );
        setAlignment( getSqlQueryIndex(), Qt::AlignCenter );
        setText( getColumnSortHidden(), QStringLiteral( "ColumnSortHidden" ) );
      }
      setFlags( -1, flags );
    }
  }
  else
  {
    // Retrieve the ModelType from Parent [from mModelRootItem]
    mModelType = mParentItem->getModelType();
    // Retrieve Column-Number Information from Parent [from mModelRootItem]
    mColumnTable = mParentItem->getTableNameIndex();
    mColumnGeometryName = mParentItem->getGeometryNameIndex();
    mColumnGeometryType = mParentItem->getGeometryTypeIndex();
    mColumnSql = mParentItem->getSqlQueryIndex();
    mColumnSortHidden = mParentItem->getColumnSortHidden();
    // The 'mColumn*' values must be set before setText and setFlags can be used
    // Set default values for Columns [Role may be ItemType specific for Qt::EditRole]
    int iRole = Qt::EditRole;
    // -1: set all columns with the given Text and Role
    setText( -1, QStringLiteral( "" ), iRole );
    setText( getColumnSortHidden(), QStringLiteral( "" ) ); // allways Qt::EditRole
    // Retrieve SpatialiteDbInfo from Parent, if exists and not done allready [there can only be one]
    if ( !getSpatialiteDbInfo() )
    {
      // This may be nullptr
      mSpatialiteDbInfo = mParentItem->getSpatialiteDbInfo();
    }
    // Retrieve DbLayer from Parent, if exists and not done allready [there can only be one]
    if ( !getDbLayer() )
    {
      // This may be nullptr
      mDbLayer = mParentItem->getDbLayer();
    }
    switch ( mItemType )
    {
      //   Qt::ItemFlags flags = ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled );
      case ItemTypeHelpRoot:
      case ItemTypeHelpText:
      case ItemTypeGroupWarning:
      case ItemTypeGroupError:
      case ItemTypeWarning:
      case ItemTypeError:
      case ItemTypeUnknown:
      case ItemTypeMetadataRoot:
      case ItemTypeMetadataGroup:
      case ItemTypeGroupSpatialTable:
      case ItemTypeGroupSpatialView:
      case ItemTypeGroupVirtualShape:
      case ItemTypeGroupRasterLite1:
      case ItemTypeGroupRasterLite2Vector:
      case ItemTypeGroupRasterLite2Raster:
      case ItemTypeGroupSpatialiteTopology:
      case ItemTypeGroupTopologyExport:
      case ItemTypeGroupStyleVector:
      case ItemTypeGroupStyleRaster:
      case ItemTypeGroupGdalFdoOgr:
      case ItemTypeGroupGeoPackageVector:
      case ItemTypeGroupGeoPackageRaster:
      case ItemTypeGroupMBTilesTable:
      case ItemTypeGroupMBTilesView:
      case ItemTypeGroupMetadata:
      case ItemTypeGroupAllSpatialLayers:
      case ItemTypeGroupNonSpatialTables:
      case ItemTypeSpatialRefSysAuxGroups:
      case ItemTypeSpatialRefSysAuxSubGroup:
      case ItemTypeSpatialRefSysAux:
      case ItemTypeCommonMetadata:
      case ItemTypeMBTilesMetadata:
      case ItemTypeStylesMetadata:
      case ItemTypePointMetadata:
      case ItemTypeRectangleMetadata:
      case ItemTypeColumn:
      case ItemTypeNonSpatialTablesSubGroups:
      case ItemTypeNonSpatialTablesSubGroup:
      case ItemTypeNonSpatialTable:
      case ItemTypeLayerOrderRasterGroup:
      case ItemTypeLayerOrderVectorGroup:
      case ItemTypeLayerOrderRasterItem:
      case ItemTypeLayerOrderVectorItem:
        // Removing IsSelectable
        flags &= ~Qt::ItemIsSelectable;
        break;
      case ItemTypeDb:
      case ItemTypeLayer:
        break;
      default:
        break;
    }
    // -1: set all columns with the given Flags
    setFlags( -1, flags );
    // forward the signal towards the root
    // connectToChildItem( this );
  }
  if ( getDbLayer() )
  {
    if ( !getSpatialiteDbInfo() )
    {
      // Retrieve SpatialiteDbInfo from Parent, if exists and not done allready [there can only be one]
      mSpatialiteDbInfo = getDbLayer()->getSpatialiteDbInfo();
    }
    mLayerType =  getDbLayer()->getLayerType();
    mLayerTypeString = getDbLayer()->getLayerTypeString();
    mLayerName = getDbLayer()->getLayerName();
    bDbLayer = getDbLayer()->isLayerValid();
    switch ( mItemType )
    {
      case ItemTypeLayer:
      case ItemTypeLayerOrderRasterLayer:
      case ItemTypeLayerOrderVectorLayer:
        if ( ( isSelectable() ) && ( bDbLayer ) )
        {
          mIsLayerSelectable = true;
        }
        break;
      default:
        break;
    }
    switch ( mLayerType )
    {
      case QgsSpatialiteDbInfo::SpatialTable:
      case QgsSpatialiteDbInfo::SpatialView:
      case QgsSpatialiteDbInfo::TopologyExport:
      case QgsSpatialiteDbInfo::GeoPackageVector:
      case QgsSpatialiteDbInfo::GdalFdoOgr:
      case QgsSpatialiteDbInfo::VirtualShape:
        if ( mLayerType ==  QgsSpatialiteDbInfo::SpatialView )
        {
          mIsSpatialView = true;
        }
        if ( mLayerType ==  QgsSpatialiteDbInfo::GeoPackageVector )
        {
          mIsGeoPackage = true;
        }
        mIsVectorType = true;
        break;
      case QgsSpatialiteDbInfo::MBTilesTable:
      case QgsSpatialiteDbInfo::MBTilesView:
        mIsMbTiles = true;
        mIsRasterType = true;
        mIsVectorType = false;
        break;
      case QgsSpatialiteDbInfo::RasterLite2Raster:
      case QgsSpatialiteDbInfo::RasterLite1:
      case QgsSpatialiteDbInfo::GeoPackageRaster:
        mIsVectorType = false;
        if ( mLayerType ==  QgsSpatialiteDbInfo::RasterLite1 )
        {
          mIsRasterLite1 = true;
          mIsRasterType = true;
        }
        if ( mLayerType ==  QgsSpatialiteDbInfo::RasterLite2Raster )
        {
          mIsRasterLite2 = true;
          mIsRasterType = true;
        }
        if ( mLayerType ==  QgsSpatialiteDbInfo::GeoPackageRaster )
        {
          mIsGeoPackage = true;
          mIsRasterType = true;
        }
        break;
      default:
        mIsVectorType = false;
        mIsGeoPackage = false;
        mIsMbTiles = false;
        mIsRasterLite2 = false;
        mIsRasterLite2 = false;
        mIsSpatialView = false;
        break;
    }
  }
  if ( getSpatialiteDbInfo() )
  {
    bSpatialiteDbInfo = true;
    mSpatialMetadata = getSpatialiteDbInfo()->dbSpatialMetadata();
    mSpatialMetadataString = getSpatialiteDbInfo()->dbSpatialMetadataString();
    if ( mItemType == ItemTypeDb )
    {
      if ( mParentItem )
      {
        mParentItem->setTableCounter( 0 );
        mParentItem->setNonSpatialTablesCounter( 0 );
        mParentItem->setStylesCounter( 0 );
        mParentItem->setSridInfoCounter( 0 );
      }
      // Determin amount of Layers, and Layer-Types
      // Add Groups and fill with Layers
      if ( sniffSpatialiteDbInfo() )
      {
        if ( mParentItem )
        {
          // Tell mRootItem the amount of Tables
          mParentItem->setTableCounter( getTableCounter() );
          mParentItem->setNonSpatialTablesCounter( getNonSpatialTablesCounter() );
          mParentItem->setStylesCounter( getStylesCounter() );
          mParentItem->setSridInfoCounter( getSridInfoCounter() );
          mParentItem->setSpatialiteDbInfo( getSpatialiteDbInfo() );
        }
        bRc = true;
      }
    }
    else
    {
      if ( mParentItem )
      {
        setTableCounter( mParentItem->getTableCounter() ) ;
        setNonSpatialTablesCounter( mParentItem->getNonSpatialTablesCounter() );
        setStylesCounter( mParentItem->getStylesCounter() ) ;
        setSridInfoCounter( mParentItem->getSridInfoCounter() ) ;
      }
    }
    switch ( mItemType )
    {
      case ItemTypeLayer:
      case ItemTypeGroupNonSpatialTables:
      case ItemTypeNonSpatialTablesSubGroups:
      case ItemTypeNonSpatialTablesSubGroup:
      case ItemTypeSpatialRefSysAuxGroups:
      case ItemTypeSpatialRefSysAuxSubGroup:
      case ItemTypeNonSpatialTable:
        // These Item-Types should call tasks after onInit()
        bRc = true;
        break;
      // Everything that can return a QgsSpatialiteDbLayer;
      case ItemTypeGroupSpatialTable:
      case ItemTypeGroupSpatialView:
      case ItemTypeGroupVirtualShape:
      case ItemTypeGroupTopologyExport:
      case ItemTypeGroupGeoPackageVector:
      case ItemTypeGroupGdalFdoOgr:
      case ItemTypeGroupRasterLite2Raster:
      case ItemTypeGroupGeoPackageRaster:
      case ItemTypeGroupRasterLite1:
      // Either MBTilesTable or MBTilesView can be used [same result]
      case ItemTypeGroupMBTilesTable:
      case ItemTypeGroupMBTilesView:
      // Either StyleVector or StyleRaster can be used [same result]
      case ItemTypeGroupStyleVector:
      case ItemTypeGroupStyleRaster:
        if ( ( bSpatialiteDbInfo ) && ( !bDbLayer ) )
        {
          // These Item-Types should call tasks after onInit()
          bRc = true;
        }
        break;
      case ItemTypeMetadataRoot:
      case ItemTypeMetadataGroup:
      case ItemTypeLayerOrderRasterLayer:
      case ItemTypeLayerOrderVectorLayer:
      case ItemTypeLayerOrderRasterItem:
      case ItemTypeLayerOrderVectorItem:
        // These Item-Types should call tasks after onInit()
        if ( ( bSpatialiteDbInfo ) && ( bDbLayer ) )
        {
          // These Item-Types should call tasks after onInit()
          bRc = true;
        }
        break;
      case ItemTypeSpatialRefSysAux:
      default:
        break;
    }
  }
  else
  {
    // do not have an active mSpatialiteDbInfo
    if ( mParentItem )
    {
      setTableCounter( mParentItem->getTableCounter() ) ;
      setNonSpatialTablesCounter( mParentItem->getNonSpatialTablesCounter() );
      setStylesCounter( mParentItem->getStylesCounter() );
      setSridInfoCounter( mParentItem->getSridInfoCounter() );
    }
    switch ( mItemType )
    {
      case ItemTypeHelpRoot:
        // These Item-Types should call tasks after onInit()
        bRc = true;
        break;
      default:
        break;
    }
  }
  if ( mParentItem )
  {
    switch ( mItemType )
    {
      case ItemTypeHelpRoot:
      case ItemTypeDb:
        // Adding this Item to the Parent
        mParentItem->addChild( this, mParentItem->childCount() );
        break;
      // Must be done later, with insertPrepairedChildren [stack overflow in thread #1: can't grow stack to 0x1ffe801000]
      case ItemTypeHelpText:
      case ItemTypeGroupWarning:
      case ItemTypeGroupError:
      case ItemTypeWarning:
      case ItemTypeError:
      case ItemTypeUnknown:
      case ItemTypeLayer:
      default:
        break;
    }
#if 0
    QString sMessage = QStringLiteral( "QgsSpatialiteDbInfoItem::onInit " );
    sMessage += QStringLiteral( "bRc[%1] " ).arg( bRc );
    sMessage += QStringLiteral( "ColumnsCount[%1] " ).arg( columnCount() );
    sMessage += QStringLiteral( "TableCounter[%1] " ).arg( getTableCounter() );
    sMessage += QStringLiteral( "ItemType[this=%1,parent=%2] " ).arg( getItemTypeString() ).arg( mParentItem->getItemTypeString() );
    sMessage += QStringLiteral( "ModelType[this=%1,parent=%2] " ).arg( getModelTypeString() ).arg( mParentItem->getModelTypeString() );
    sMessage += QStringLiteral( "Parent-childCount[%1] " ).arg( mParentItem->childCount() );
    sMessage += QStringLiteral( "DbContainterType[%1] " ).arg( dbSpatialMetadataString() );
    sMessage += QStringLiteral( "LayerType[%1] " ).arg( getLayerTypeString() );
    sMessage += QStringLiteral( "LayerName[%1] " ).arg( getLayerName() );
    sMessage += QStringLiteral( "bSpatialiteDbInfo[%1] bDbLayer[%2] " ).arg( bSpatialiteDbInfo ).arg( bDbLayer );
    QgsDebugMsgLevel( sMessage, 7 );
#endif
  }
  if ( !bRc )
  {
    // No tasks after onInit. consider this valid
    mIsItemValid = true;
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::connectToChildItem()
//-----------------------------------------------------------------
// - Based on QgsLayerTreeNode::insertChildrenPrivate
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::connectToChildItem( QgsSpatialiteDbInfoItem *node )
{
  // forward the signal towards the root
  connect( node, &QgsSpatialiteDbInfoItem::willAddChildren, this, &QgsSpatialiteDbInfoItem::willAddChildren );
  connect( node, &QgsSpatialiteDbInfoItem::addedChildren, this, &QgsSpatialiteDbInfoItem::addedChildren );
  connect( node, &QgsSpatialiteDbInfoItem::willRemoveChildren, this, &QgsSpatialiteDbInfoItem::willRemoveChildren );
  connect( node, &QgsSpatialiteDbInfoItem::removedChildren, this, &QgsSpatialiteDbInfoItem::removedChildren );
  // connect( node, &QgsSpatialiteDbInfoItem::customPropertyChanged, this, &QgsSpatialiteDbInfoItem::customPropertyChanged );
  // connect( node, &QgsSpatialiteDbInfoItem::visibilityChanged, this, &QgsSpatialiteDbInfoItem::visibilityChanged );
  // connect( node, &QgsSpatialiteDbInfoItem::expandedChanged, this, &QgsSpatialiteDbInfoItem::expandedChanged );
  // connect( node, &QgsSpatialiteDbInfoItem::nameChanged, this, &QgsSpatialiteDbInfoItem::nameChanged );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::child
// - 0 based index
//-----------------------------------------------------------------
QgsSpatialiteDbInfoItem *QgsSpatialiteDbInfoItem::child( int number )
{
  if ( number >= childCount() )
  {
    number = childCount() - 1;
  }
  if ( number < 0 )
  {
    number = 0;
  }
  return mChildItems.at( number );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::moveChild
// - 0 based index
// - returns true parameter logic is correct
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::moveChild( int iFrom, int iTo )
{
  if ( ( iFrom ==  iTo ) || ( ( iFrom < 0 ) || ( iFrom >= mChildItems.count() ) ) || ( ( iTo < 0 ) || ( iTo >= mChildItems.count() ) ) )
  {
    return false;
  }
  mChildItems.move( iFrom, iTo );
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::removeChild
// - 0 based index
// - returns thw following child row number ; -1 = empty ; -2 invalid range
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::removeChild( int iFrom )
{
  int iNextChildRow = -2;
  if ( ( iFrom < 0 ) || ( iFrom >= mChildItems.count() ) )
  {
    return iNextChildRow;
  }
  mChildItems.removeAt( iFrom );
  // iFrom is now the next child
  iNextChildRow = iFrom;
  // Was iFrom the last child?
  if ( iFrom >= mChildItems.count() )
  {
    // iFrom is now (again) the last Child
    iNextChildRow = mChildItems.count() - 1;
    // if empty (count == 0) :-1 will be returned, Parent is empty
  }
  return iNextChildRow;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::childCount
// - 1 based index
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::childCount() const
{
  return mChildItems.count();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::columnCount
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::columnCount() const
{
  return ( mColumnSortHidden + 1 );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::addChild
// Inserting 1 child to the Parent at given 0-based position
// Note: the value of position and indexTo are the same (from/to)
// - emit willAddChildren will crash when used [reason unknown]
// -> works without [also not used in sample 'editabletreemodel']
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::addChild( QgsSpatialiteDbInfoItem *child, int position, bool bWillAddChildren )
{
  if ( ( position < 0 ) || ( position > childCount() ) )
  {
    position = row(); // Insert at end;
  }
  int indexTo = position;
  if ( bWillAddChildren )
  {
    connectToChildItem( child );
    emit willAddChildren( this, position, indexTo );
  }
  mChildItems.insert( position, child );
  if ( bWillAddChildren )
  {
    emit addedChildren( this, position, indexTo );
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::insertPrepairedChildren
// Adding Childrent to this node/item
// - The ModelType must be the same
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::insertPrepairedChildren()
{
  int iCountValid = 0;
  if ( mPrepairedChildItems.count() < 1 )
  {
    return  iCountValid;
  }
  bool bWillAddChildren = false;
  int indexFrom = row(); // Insert at end
  int iPosition = indexFrom;
  for ( int i = 0; i < mPrepairedChildItems.count(); i++ )
  {
    QgsSpatialiteDbInfoItem *childItem = mPrepairedChildItems.at( i );
    if ( childItem )
    {
      if ( ( childItem->isValid() ) && ( childItem->getModelType()  == getModelType() ) )
      {
        iCountValid++;
        addChild( childItem, ++iPosition, bWillAddChildren );
      }
    }
  }
  mPrepairedChildItems.clear();
  return iCountValid;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::moveChild
// For support of Up/down Buttion in Tab 'Layer Order'
// - Out of range values will place the Item in the First or Last position
// It will return the new position for beginMoveRows to use
// Note that if sourceParent and destinationParent are the same,
// you must ensure that the destinationChild is not within the range of sourceFirst and sourceLast + 1.
//  -> beginMoveRows need to know the new position at a time when the old position still exists, thus +1
//-----------------------------------------------------------------
// Will return value needed for 'moveChild'
// For Down[iMoveCount > 0], but not LastToFirst[iMoveToRows > 0], iMoveToRows is correct for beginMoveRows
// - but not for moveChild: adapt value [iMoveToChild]
// -> moveChild[QList.take] needs to know the old position (removeAt), and after removal,  position to insert
// For Up[iMoveCount < 0], and for FirstToLast[iMoveToRows >= parentItem->childCount()], iMoveToRows is correct for beginMoveRows
// - but not for moveChild: adapt value  [iMoveToChild]
// For the other conditions, the values are the same
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::moveChildPosition( int iMoveCount, int &iMoveToChild )
{
  int iMoveToRows = childNumber(); // Present position
  iMoveToChild = iMoveToRows;
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) && ( parent() ) )
  {
    switch ( getItemType() )
    {
      case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorLayer:
      case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterLayer:
        if ( ( iMoveCount != 0 ) && ( parent()->childCount() > 0 ) )
        {
          if ( iMoveCount > 0 )
          {
            if ( ( childNumber() + iMoveCount ) >= parent()->childCount() )
            {
              iMoveToRows = 0; // From Last to First
              iMoveToChild = iMoveToRows; // No difference
            }
            else
            {
              // 4 = (2+1)+1 [Note: this will be the correct result for beginMoveRows, but not for moveChild where '3' is needed]
              // beginMoveRows the new position includes the old row that has not yet been deleted
              iMoveToRows = ( childNumber() + 1 ) + iMoveCount;
              // moveChild needs the the new position, where the old row has already been deleted (removeAt)
              iMoveToChild = iMoveToRows - 1;
            }
          }
          else
          {
            // iMoveCount is minus, thus we must add
            if ( ( childNumber() + iMoveCount ) < 0 )
            {
              // beginMoveRows needs the position after the last row
              iMoveToRows = parent()->childCount(); // From First to Last
              // moveChild needs the position of the last row
              iMoveToChild = iMoveToRows - 1;
            }
            else
            {
              iMoveToRows = childNumber() + iMoveCount;
              iMoveToChild = iMoveToRows; // No difference
            }
          }
        }
        break;
      default:
        break;
    }
  }
  return iMoveToRows;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::addDbMapLayers
//-----------------------------------------------------------------
// Collecting selected Map-Layers to be added to MapCanvas
// - The ModelType must be ModelTypeLayerOrder and ItemTypeRoot
// Intended for use with QgsSpatialiteDbInfo::addDbMapLayers
// - which will first send the collected Rasters, the the collected Vectors
// --> irrespective of the order given here
// Creating 2 major Lists [1 Vectors, 1 Rasters]
// For the Vectors List 3 Minor Lists will be created
// - POINT's (2D and Multi)
// - LINESTRING's (2D and Multi)
// - POLYGON's (2D and Multi)
//-----------------------------------------------------------------
// First load the Rasters, then the Vectors
// - so that the Vectors will be on top of the Rasters
// -> the last appended, is rendered first
// --> POINT's will be on top of LINESTRING's
// --> LINESTRING's will be on top of POLYGON's
// --> POLYGON's will be on top of Rasters
//-----------------------------------------------------------------
// This function will add the Selected Layers
// - from Bottom to Top. so that when rendered in the MapCanvas
// -> Top will be over Bottom
//-----------------------------------------------------------------
// For the 'LayerOrder' List, Rasters are shown before Vectors
// - since the list is often shorter
// No Sub-Groups are shown for POINT's, LINESTRING's and POLYGON's
// - since this sorting is done in QgsSpatialiteDbInfo::addDbMapLayers
//-----------------------------------------------------------------
// The given order here, therefore, effects only in which order the
// - POINT's, then LINESTRING's, then POLYGON's and finaly the Rasters are shown
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addSelectedDbLayers( QStringList &saSelectedLayers, QStringList &saSelectedLayersStyles, QStringList &saSelectedLayersSql )
{
  saSelectedLayers.clear();
  saSelectedLayersStyles.clear();
  saSelectedLayersSql.clear();
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) && ( getItemType() == ItemTypeRoot ) )
  {
    if ( childCount() > 0 )
    {
      for ( int iGroup = 0; iGroup < childCount(); iGroup++ )
      {
        QgsSpatialiteDbInfoItem *groupItem = child( iGroup );
        if ( ( groupItem ) &&
             ( ( groupItem->getItemType() == ItemTypeLayerOrderVectorGroup ) ||
               ( groupItem->getItemType() == ItemTypeLayerOrderRasterGroup ) ) )
        {
          // From Bottom to Top
          for ( int iLayer = groupItem->childCount() - 1; iLayer >= 0; iLayer-- )
          {
            QgsSpatialiteDbInfoItem *layerItem = groupItem->child( iLayer );
            // We do not care about the Vector/Raster-Type
            // - QgsSpatialiteDbInfo::addDbMapLayers will deal with the proper ordering
            if ( ( layerItem ) && ( layerItem->isLayerSelectable() ) )
            {
              saSelectedLayers.append( layerItem->getLayerName() );
              saSelectedLayersStyles.append( layerItem->getSelectedStyle() );
              saSelectedLayersSql.append( layerItem->getLayerSqlQuery() );
            }
          }
        }
      }
    }
  }
  return saSelectedLayers.count();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::insertPrepairedChildren
// Adding/Removing Childrens to this node/item
// - The ModelType must be ModelTypeLayerOrder and ItemTypeRoot
// Note: this will be called from 'setLayerOrderData',
// - inside beginResetModel/endResetModel statements
// -> therefore we can add and Remove while the View is not painting
// for ItemTypeLayer: text() will return a TableName, use getLayerName
// for ItemTypeLayerOrder: text() will return the LayerName
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::handelSelectedPrepairedChildren( bool bRemoveItems )
{
  int iCountValid = 0;
  if ( mPrepairedChildItems.count() > 0 )
  {
    if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) && ( getItemType() == ItemTypeRoot ) )
    {
      // with 'bRemoveItems=true', we are recieving a list of Items NOT to be deleted
      QList<int> listRetainVectors;
      QList<int> listRetainRasters;
      QgsSpatialiteDbInfoItem *groupVectors = nullptr;
      QgsSpatialiteDbInfoItem *groupRasters = nullptr;
      for ( int iChild = 0; iChild < mPrepairedChildItems.count(); iChild++ )
      {
        QgsSpatialiteDbInfoItem *childItem = mPrepairedChildItems.at( iChild );
        if ( childItem )
        {
          // 'addPrepairedChildItem' has checked for acceptable Types [Model/Item] and if unique inside PrepairedChildItems
          if ( childItem->isValid() )
          {
            bool bChildUnique = true;
            if ( childCount() > 0 )
            {
              // Check the childItem for Vector or Raster
              SpatialiteDbInfoItemType itemType = ItemTypeUnknown;
              if ( childItem->isVector() )
              {
                itemType = ItemTypeLayerOrderVectorGroup;
              }
              if ( childItem->isRaster() )
              {
                itemType = ItemTypeLayerOrderRasterGroup;
              }
              if ( itemType != ItemTypeUnknown )
              {
                // This should always be true
                QgsSpatialiteDbInfoItem *groupItem = nullptr;
                for ( int iGroup = 0; iGroup < childCount(); iGroup++ )
                {
                  if ( ( child( iGroup ) ) && ( child( iGroup )->isValid() ) && ( child( iGroup )->getItemType() == itemType ) )
                  {
                    groupItem = child( iGroup );
                  }
                }
                if ( groupItem )
                {
                  if ( ( !groupVectors ) && ( itemType == ItemTypeLayerOrderVectorGroup ) )
                  {
                    // Set the pointer to be used later
                    groupVectors = groupItem;
                  }
                  if ( ( !groupRasters ) && ( itemType == ItemTypeLayerOrderRasterGroup ) )
                  {
                    // Set the pointer to be used later
                    groupRasters = groupItem;
                  }
                  // We have found the Vector or Raster-Group [Group may not exist at all]
                  for ( int iGroup = 0; iGroup < groupItem->childCount(); iGroup++ )
                  {
                    if ( groupItem->child( iGroup )->getLayerName() == childItem->getLayerName() )
                    {
                      // The Child-Item exists, addConnectionLayer will not be called
                      bChildUnique = false;
                      if ( bRemoveItems )
                      {
                        if ( itemType == ItemTypeLayerOrderVectorGroup )
                        {
                          // Add to the list of Items NOT to be removed
                          listRetainVectors.append( iGroup );
                        }
                        if ( itemType == ItemTypeLayerOrderRasterGroup )
                        {
                          // Add to the list of Items NOT to be removed
                          listRetainRasters.append( iGroup );
                        }
                      }
                    }
                  }
                }
              }
            }
            if ( ( !bRemoveItems ) && ( bChildUnique ) )
            {
              // Tell the calling function that something has changed
              iCountValid += addConnectionLayer( childItem );
            }
          }
        }
      }
      if ( ( bRemoveItems ) && ( ( groupVectors ) || ( groupRasters ) ) )
      {
        if ( groupVectors )
        {
          for ( int iGroup = 0; iGroup < groupVectors->childCount(); iGroup++ )
          {
            if ( !listRetainVectors.contains( iGroup ) )
            {
              // Remove the Child-Item and delete
              QgsSpatialiteDbInfoItem *removeItem = groupVectors->mChildItems.takeAt( iGroup );
              iCountValid++; // Tell the calling function that something has changed
              if ( removeItem )
              {
                // Any Children it may contain will also be deleted
                delete removeItem;
                removeItem = nullptr;
              }
            }
          }
          // While removing, check if the Group is empty
          if ( groupVectors->childCount() == 0 )
          {
            // We have removed the last child of the Group, remove the Group
            mChildItems.removeAt( groupVectors->childNumber() );
            delete groupVectors;
          }
          groupVectors = nullptr;
        }
        if ( groupRasters )
        {
          for ( int iGroup = 0; iGroup < groupRasters->childCount(); iGroup++ )
          {
            if ( !listRetainRasters.contains( iGroup ) )
            {
              // Remove the Child-Item and delete
              QgsSpatialiteDbInfoItem *removeItem = groupRasters->mChildItems.takeAt( iGroup );
              iCountValid++; // Tell the calling function that something has changed
              if ( removeItem )
              {
                // Any Children it may contain will also be deleted
                delete removeItem;
                removeItem = nullptr;
              }
            }
          }
          // While removing, check if the Group is empty
          if ( groupRasters->childCount() == 0 )
          {
            // We have removed the last child of the Group, remove the Group
            mChildItems.removeAt( groupRasters->childNumber() );
            delete groupRasters;
          }
          groupRasters = nullptr;
        }
      }
    }
    mPrepairedChildItems.clear();
  }
  return iCountValid;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setLayerSqlQuery
//-----------------------------------------------------------------
// The storing of the Layer-Sql is only done in ModelTypeConnections
// Upon creation of the Selected Layer for ModelTypeLayerOrder
// - the pointer of this ModelTypeConnections will be stored
// With getLayerSqlQuery
// - the latest version of the Layer-Sql is returned
// -> from the ModelTypeConnections version
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setLayerSqlQuery( QString sLayerSqlQuery )
{
  bool bRc = false;
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeConnections ) )
  {
    if ( ( isLayerSelectable() ) && ( !getCousinLayerItem()->isLayerSelectable() ) )
    {
      // This is where the created Layer-Sql is stored
      setText( getSqlQueryIndex(), sLayerSqlQuery );
      bRc = true;
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setSelectedStyle
// Note:
// - QgsSpatialiteDbInfoModel::setSelectedStyle
// -> must be called for the change to be rendered properly
// --> where layoutAboutToBeChanged and layoutChanged will be emitted
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setSelectedStyle()
{
  bool bRc = false;
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    if ( ( getItemType() == ItemTypeStylesMetadata ) && ( getGrandParent() ) )
    {
      // Copy the Style-Name [text()], to where the Layer-Style to be used is stored
      // Note: Parent is the Style-Group, GrandParent is the Layer-Item of LayerOrder
      getGrandParent()->setText( getGrandParent()->getGeometryNameIndex(), text() );
      bRc = true;
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::getSelectedStyle
// Note:
// The User can change the default style to be used
// - set with QgsSpatialiteDbInfoModel::setSelectedStyle
// -> value 'nostyle': no style will be applied [or no registered style available]
// --> retrieved during addSelectedDbLayers
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoItem::getSelectedStyle()
{
  QString sSelectedStyle = QString();
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    if ( ( getItemType() == ItemTypeLayerOrderRasterLayer ) || ( getItemType() == ItemTypeLayerOrderVectorLayer ) )
    {
      // if empty, the stored default value in the Database will be used
      // if 'nostyle' no Style will be applied [or no registered style available]
      sSelectedStyle = text( getGeometryNameIndex() );
    }
  }
  return sSelectedStyle;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::getLayerSqlQuery
//-----------------------------------------------------------------
// Between the creation of the Selected Layer for ModelTypeLayerOrder
// - the created Layer-Sql may be changed in ModelTypeConnections
// Upon creation, the point to ModelTypeConnections will be stored
// - the latest version of the Layer-Sql is returned
// There is a special hack in 'data()' to insure that when displaying a LayerOrder-Item, that does not store the Layer-Sql-Query
// - to show it as a ToolTip, when not empty [overriding the otherwise set ToolTip of Layer-Abstract]
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoItem::getLayerSqlQuery() const
{
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    if ( ( isLayerSelectable() ) && ( getCousinLayerItem() ) )
    {
      // This will insure that the latest version of the Layer-Sql is returned
      return getCousinLayerItem()->getLayerSqlQuery();
    }
  }
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeConnections ) )
  {
    if ( ( isLayerSelectable() ) && ( !getCousinLayerItem() ) )
    {
      // This is where the created Layer-Sql is stored
      return text( getSqlQueryIndex() );
    }
  }
  return QString();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::getLayerTypeIcon()
// For Vectors: the Geometry-Type Icon will returned
// For other Layer-Types the Layer-Type
// otherwise (Styles etc.) default Icons for non-Layer types
//-----------------------------------------------------------------
QIcon QgsSpatialiteDbInfoItem::getLayerTypeIcon() const
{
  if ( isLayerSelectable() )
  {
    if ( isVector() )
    {
      return getDbLayer()->getGeometryTypeIcon();
    }
    return getDbLayer()->getLayerTypeIcon();
  }
  return QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( getLayerType() );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setCousinLayerItem
// When a LayerOrder-LayerItem is created from a Connections-Layer
// -> in 'addConnectionLayer'
// -> both must be isLayerSelectable() [i.e. Root of QgsSpatialiteDbLayer-Item]
// The creating Connections-Layer must be stored so the latest version
// version of the (possibly create) Sql-Query can be retrieved.
// This will be used for the 'addDbMapLayers' logic
// See
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setCousinLayerItem( QgsSpatialiteDbInfoItem *cousinLayerItem )
{
  bool bRc = false;
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) )
  {
    if ( ( isLayerSelectable() ) && ( cousinLayerItem->isLayerSelectable() ) )
    {
      mCousinLayerItem = cousinLayerItem;
      bRc = true;
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::addPrepairedChildItem
// Only a PrepairedChildItem with a unique layerName will be added
// Note:
// for ItemTypeLayer: text() will return a TableName, use getLayerName
// for ItemTypeLayerOrder: text() will return the LayerName
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addPrepairedChildItem( QgsSpatialiteDbInfoItem *childItem )
{
  if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) && ( getItemType() == ItemTypeRoot ) )
  {
    // The LayerOrder Model [as ItemRoot] will only accept Items that are Layers and not the same ModelType
    if ( ( childItem ) && ( childItem->isValid() ) && ( childItem->getItemType() == ItemTypeLayer )  && ( childItem->getModelType() != getModelType() ) )
    {
      QString sLayerName = childItem->getLayerName();
      bool bChildUnique = true;
      for ( int i = 0; i < mPrepairedChildItems.count(); i++ )
      {
        QgsSpatialiteDbInfoItem *prepairedItem = mPrepairedChildItems.at( i );
        if ( prepairedItem )
        {
          if ( prepairedItem->getLayerName() == sLayerName )
          {
            bChildUnique = false;
            break;
          }
        }
      }
      if ( bChildUnique )
      {
        // The Layer-Name in the Prepaired list is unique [don't add doubles]
        mPrepairedChildItems.append( childItem );
      }
    }
  }
  return mPrepairedChildItems.count();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::addConnectionLayer
// Input is of the ModelTypeConnections Type
// - and must contain a QgsSpatialiteDbLayer
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::addConnectionLayer( QgsSpatialiteDbInfoItem *childItem )
{
  if ( childItem )
  {
    // If Valid and a Raster or Vector Type
    if ( ( childItem->isValid() ) && ( childItem->isLayerSelectable() ) )
    {
      setDbLayer( childItem->getDbLayer() );
      setCousinLayerItem( childItem );
      SpatialiteDbInfoItemType itemGroupType = ItemTypeUnknown;
      QColor bg_color = QColor( "steelblue" );
      QColor bg_black = QColor( "black" );
      QColor fg_color = QColor( "yellow" );
      QIcon itemIcon = childItem->getLayerTypeIcon();
      QString sName;
      QString sTitel = QString();
      QString sAbstract = QString();
      QString sToolTipText = QStringLiteral( "Rasters will be loaded first, then the Vectors so that the Vectors will be on top of the Rasters" );
      QString sSortTag = QStringLiteral( "AAAAA" );
      QString sGroupType = QString();
      if ( childItem->isVector() )
      {
        itemGroupType = ItemTypeLayerOrderVectorGroup;
        sGroupType = QStringLiteral( "Vector" );
        sSortTag = QStringLiteral( "1AAAAA_%1" ).arg( sName );
        sTitel = QStringLiteral( "Top to Bottom, Vectors will be over the Rasters" );
        sAbstract = QStringLiteral( "Ordered by POINT's, LINESTRING's then POLYGON's" );
        sToolTipText = QStringLiteral( "POINT's will be on top of the LINESTRING's\n" );
        sToolTipText += QStringLiteral( "LINESTRING's will be on top of the POLYGON's\n" );
        sToolTipText += QStringLiteral( "POLYGON's will be on top of the Rasters" );
      }
      else if ( childItem->isRaster() )
      {
        itemGroupType = ItemTypeLayerOrderRasterGroup;
        sGroupType = QStringLiteral( "Raster" );
        sTitel = QStringLiteral( "Top to Bottom, Rasters will be covered by the Vectors" );
        sAbstract = QStringLiteral( "The first Raster will cover the second Raster" );
        sToolTipText = QStringLiteral( "Vectors will be on top of the Raster(s)\n" );
        sToolTipText += QStringLiteral( "The first Raster will over the second Raster\n" );
        sToolTipText += QStringLiteral( "The last Raster will be covered by the previous Raster" );
        sSortTag = QStringLiteral( "1AAAAB_%1" ).arg( sName );
      }
      sName = QStringLiteral( "%1-Data" ).arg( sGroupType );
      if ( itemGroupType != ItemTypeUnknown )
      {
        QgsSpatialiteDbInfoItem *groupItem = nullptr;
        for ( int i = 0; i < mChildItems.count(); i++ )
        {
          if ( mChildItems.at( i )->getItemType() == itemGroupType )
          {
            groupItem = mChildItems.at( i );
            break;
          }
        }
        if ( !groupItem )
        {
          // Create the GroupItem
          groupItem = new QgsSpatialiteDbInfoItem( this, itemGroupType );
          if ( groupItem )
          {
            // Column 0: getTableNameIndex()
            groupItem->setText( getTableNameIndex(), sName );
            // For Geometries, this will be the Geometry-Type-Icon
            groupItem->setIcon( getTableNameIndex(), groupItem->getGroupIcon() );
            groupItem->setToolTip( getTableNameIndex(), sToolTipText );
            groupItem->setBackground( getTableNameIndex(), bg_color );
            groupItem->setForeground( getTableNameIndex(), fg_color );
            // Column 1 [used later]: getGeometryNameIndex()
            // Note: the 'Layers: count' text are set during 'addConnectionLayer' amd QgsSpatialiteDbInfoModel::removeLayerOrderChild
            groupItem->setBackground( getGeometryNameIndex(), bg_color );
            groupItem->setForeground( getGeometryNameIndex(), fg_color );
            // Column 2 [possibly used]: getGeometryTypeIndex()
            if ( !sTitel.isEmpty() )
            {
              groupItem->setText( getGeometryTypeIndex(), sTitel );
              groupItem->setForeground( getGeometryTypeIndex(), fg_color );
              groupItem->setBackground( getGeometryTypeIndex(), bg_black );
            }
            // Column 3 [possibly used]: getSqlQueryIndex()
            if ( !sAbstract.isEmpty() )
            {
              groupItem->setText( getSqlQueryIndex(), sAbstract );
              groupItem->setForeground( getSqlQueryIndex(), fg_color );
              groupItem->setBackground( getSqlQueryIndex(), bg_black );
              groupItem->setToolTip( getSqlQueryIndex(), sToolTipText );
            }
            // Column 4 [should always be  used]: getColumnSortHidden()
            groupItem->setText( getColumnSortHidden(), sSortTag );
            mPrepairedChildItems.append( groupItem );
            insertPrepairedChildren();
          }
        }
        if ( groupItem )
        {
          // Calls buildLayerOrderItem(), which will add the Item to the Group
          QgsSpatialiteDbInfoItem *layerItem = new QgsSpatialiteDbInfoItem( groupItem, childItem->getDbLayer() );
          if ( ( layerItem ) && ( layerItem->isValid() ) )
          {
            groupItem->mPrepairedChildItems.append( layerItem );
            groupItem->insertPrepairedChildren();
            // Set (the possibly changed) counter Text and ToolTip
            groupItem->setText( groupItem->getGeometryNameIndex(), QStringLiteral( "Layers: %1" ).arg( groupItem->childCount() ) );
            groupItem->setToolTip( groupItem->getGeometryNameIndex(), QStringLiteral( "There are %1 %2-Layers contained in this Group " ).arg( groupItem->childCount() ).arg( sGroupType ) );
          }
        }
      }
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::insertChildren
// this may not be needed
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::insertChildren( int position, int count, SpatialiteDbInfoItemType itemType, int columns )
{
  if ( ( position < 0 ) || ( position > childCount() ) )
  {
    position = childCount();
  }
  if ( ( columns < 0 ) || ( columns > columnCount() ) )
  {
    columns = columnCount();
  }
  switch ( itemType )
  {
    case ItemTypeGroupWarning:
    case ItemTypeGroupError:
    case ItemTypeWarning:
    case ItemTypeError:
    case ItemTypeUnknown:
      break;
    // Type that call addChild in onInit, should not be supported
    case ItemTypeHelpRoot:
    case ItemTypeDb:
    default:
      itemType = ItemTypeUnknown;
      break;
  }
  int indexTo = ( position + count ) - 1;
  emit willAddChildren( this, position, indexTo );
  // TODO loop through layers and send
  for ( int row = 0; row < count; ++row )
  {
    QgsSpatialiteDbInfoItem *child = new QgsSpatialiteDbInfoItem( this, itemType );
    connectToChildItem( child );
    mChildItems.insert( position, child );
  }
  emit addedChildren( this, position, indexTo );
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::insertColumns
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::insertColumns( int position, int columns )
{
  if ( position < 0 || position > mItemData.count() )
  {
    return false;
  }
  for ( int column = 0; column < columns; ++column )
  {
    setText( column, QString() );
  }
  foreach ( QgsSpatialiteDbInfoItem *child, mChildItems )
  {
    child->insertColumns( position, columns );
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::removeChildren
// - Children will be deleted
// -> starting at the given position, for the given count
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::removeChildren( int position, int count )
{
  if ( position < 0 || position + count > mChildItems.count() )
  {
    return false;
  }
  for ( int row = 0; row < count; ++row )
  {
    delete mChildItems.takeAt( position );
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::removeColumns
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::removeColumns( int position, int columns )
{
  if ( position < 0 || position + columns > mItemData.count() )
  {
    return false;
  }
  for ( int column = 0; column < columns; ++column )
  {
    mItemData.remove( position );
  }
  foreach ( QgsSpatialiteDbInfoItem *child, mChildItems )
  {
    child->removeColumns( position, columns );
  }
  return true;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setData
// - Emulating QStandardItem
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::setData( int column, const QVariant &value, int role )
{
  bool bRc = false; // when true dataChanged should be omitted
  if ( ( column < 0 ) || ( column >= columnCount() ) )
  {
    column = 0;
  }
  // Change any Qt::DisplayRole to Qt::EditRole
  role = ( role == Qt::EditRole ) ? Qt::DisplayRole : role;
  QVector<QgsSpatialiteDbInfoData>::iterator it;
  for ( it = mItemData.begin(); it != mItemData.end(); ++it )
  {
    // Replacing, if found
    if ( ( ( *it ).role == role ) && ( ( *it ).column == column ) )
    {
      if ( value.isValid() )
      {
        if ( ( *it ).value.type() == value.type() && ( *it ).value == value )
        {
          return bRc; // Value has not changed
        }
        ( *it ).value = value;
      }
      else
      {
        mItemData.erase( it );
      }
      bRc = true;
      return bRc;
    }
  }
  // Adding
  mItemData.append( QgsSpatialiteDbInfoData( role, value, column ) );
  bRc = true;
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::data
// - Emulating QStandardItem
// Used by:
// - text(), icon(), flags()
// There is a special hack to insure that when displaying a LayerOrder-Item, that does not store the Layer-Sql-Query
// - to show it as a ToolTip, when not empty [overriding the otherwise set ToolTip of Layer-Abstract]
//-----------------------------------------------------------------
QVariant QgsSpatialiteDbInfoItem::data( int column, int role ) const
{
  if ( ( column < 0 ) || ( column >= columnCount() ) )
  {
    column = 0;
  }
  if ( ( column >= 0 ) && ( column < columnCount() ) )
  {
    // Change any Qt::DisplayRole to Qt::EditRole
    role = ( role == Qt::EditRole ) ? Qt::DisplayRole : role;
    QVector<QgsSpatialiteDbInfoData>::const_iterator it;
    for ( it = mItemData.begin(); it != mItemData.end(); ++it )
    {
      if ( ( *it ).role == role )
      {
        if ( ( *it ).column == column )
        {
          switch ( role )
          {
            case Qt::DisplayRole:
            case Qt::DecorationRole:
            case Qt::EditRole:

            case Qt::StatusTipRole:
            case Qt::TextAlignmentRole:
            case Qt::BackgroundRole:
            case Qt::ForegroundRole:
            // flags
            case ( Qt::UserRole-1 ):
            {
              // 0=Qt::DisplayRole ,1=Qt::DecorationRole, 2=Qt::EditRole, 3=Qt::ToolTipRole, 4=Qt::StatusTipRole
              // 7=Qt::TextAlignmentRole, 8=Qt::BackgroundRole,9=Qt::ForegroundRole, 255=Qt::UserRole-1
              return ( *it ).value;
            }
            break;
            case Qt::ToolTipRole:
            {
              //  3=Qt::ToolTipRole
              if ( ( getModelType() == QgsSpatialiteDbInfoModel::ModelTypeLayerOrder ) && ( column == getSqlQueryIndex() ) && ( isLayerSelectable() ) )
              {
                QString sLayerSqlQuery = getLayerSqlQuery();
                if ( !sLayerSqlQuery.isEmpty() )
                {
                  // This is a special hack to insure that when displaying a LayerOrder-Item, that does not store the Layer-Sql-Query
                  // - to show it as a ToolTip, when not empty [overriding the otherwise set ToolTip of Layer-Abstract]
                  return QVariant( sLayerSqlQuery );
                }
              }
              return ( *it ).value;
            }
            break;
            case Qt::FontRole:
            case Qt::CheckStateRole:
            case Qt::SizeHintRole:
            default:
              // 6=Qt::FontRole, 10=Qt::CheckStateRole,13=Qt::SizeHintRole
              break;
          }
        }
      }
    }
  }
  return QVariant();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setFlags
// - Emulating QStandardItem
// Used by:
// - setText, setIcon,setEnabled,setEditable,setSelectable
// - when column < 0: all columns will be set with the given value
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::setFlags( int column, Qt::ItemFlags flags )
{
  if ( column < 0 )
  {
    // Insure all colums have a flag [called in onInit()]
    for ( column = 0; column < columnCount(); column++ )
    {
      setData( column, ( int )flags, Qt::UserRole - 1 );
    }
    return;
  }
  setData( column, ( int )flags, Qt::UserRole - 1 );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setText
// - Emulating QStandardItem
// - when column < 0: all columns will be set with the given value
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::setText( int column, const QString &sText, int role )
{
  if ( column < 0 )
  {
    // Insure all columns have a text [called in onInit()]
    for ( column = 0; column < columnCount(); column++ )
    {
      setData( column, sText, role );
    }
    return;
  }
  setData( column, sText, role );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setFlags
// - Emulating QStandardItem
// Used by:
// - isEnabled,isEditable,isSelectable
// -> calling them here causes a loop
//-----------------------------------------------------------------
Qt::ItemFlags QgsSpatialiteDbInfoItem::flags( int column )  const
{
  QVariant v = data( column, Qt::UserRole - 1 );
  if ( !v.isValid() )
  {
    // Note: this should no loger happen, since setFlags(-1,...) is called in onInit()
    // 1=Qt::ItemIsSelectable, 2=Qt::ItemIsEditable, 32=Qt::ItemIsEnabled
    // 4=Qt::ItemIsDragEnabled, 8=Qt::ItemIsDropEnabled
    Qt::ItemFlags itemFlags = ( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
#if 1
    bool bEditable = ( itemFlags & Qt::ItemIsEditable ) != 0;
    bool bSelectable = ( itemFlags & Qt::ItemIsSelectable ) != 0;
    bool bEnabled = ( itemFlags & Qt::ItemIsEnabled ) != 0;
    QgsDebugMsgLevel( QStringLiteral( "-W-> QgsSpatialiteDbInfoItem::flags(%1,isEditable=%4,isSelectable=%5,isEnabled=%6) item.row/column[%2,%3]" ).arg( itemFlags ).arg( row() ).arg( column ).arg( bEditable ).arg( bSelectable ).arg( bEnabled ), 7 );
#endif
    return  itemFlags;
  }
  Qt::ItemFlags itemFlags = Qt::ItemFlags( v.toInt() );
#if 0
  bool bEditable = ( itemFlags & Qt::ItemIsEditable ) != 0;
  bool bSelectable = ( itemFlags & Qt::ItemIsSelectable ) != 0;
  bool bEnabled = ( itemFlags & Qt::ItemIsEnabled ) != 0;
  QgsDebugMsgLevel( QStringLiteral( "-I-> QgsSpatialiteDbInfoItem::flags(%1,isEditable=%4,isSelectable=%5,isEnabled=%6) item.row/column[%2,%3]" ).arg( itemFlags ).arg( row() ).arg( column ).arg( bEditable ).arg( bSelectable ).arg( bEnabled ), 7 );
#endif
  return  itemFlags;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setEnabled
// - Emulating QStandardItem
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::setEnabled( int column, bool value )
{
  Qt::ItemFlags flag = flags( column );
  if ( value )
  {
    flag |= Qt::ItemIsEditable;
  }
  else
  {
    flag &= ~Qt::ItemIsEditable;
  }
  setFlags( column, flag );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setEditable
// - Emulating QStandardItem
// Item-Column must be Enabled to be Editable
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::setEditable( int column, bool value )
{
  Qt::ItemFlags flag = flags( column );
  if ( value )
  {
    if ( !isEnabled( column ) )
    {
      setEnabled( column, true );
    }
    flag |= Qt::ItemIsEditable;
  }
  else
  {
    flag &= ~Qt::ItemIsEditable;
  }
  setFlags( column, flag );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::setSelectable
// - Emulating QStandardItem
//-----------------------------------------------------------------
void QgsSpatialiteDbInfoItem::setSelectable( int column, bool value )
{
  Qt::ItemFlags flag = flags( column );
  if ( value )
  {
    flag |= Qt::ItemIsSelectable;
  }
  else
  {
    flag &= ~Qt::ItemIsSelectable;
  }
  setFlags( column, flag );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::childNumber
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::childNumber() const
{
  if ( mParentItem )
  {
    return mParentItem->mChildItems.indexOf( const_cast<QgsSpatialiteDbInfoItem *>( this ) );
  }
  return 0;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::position
// - Emulating QStandardItem
//-----------------------------------------------------------------
QPair<int, int> QgsSpatialiteDbInfoItem::position() const
{
  if ( parent() )
  {
    // int idx = parent()->childNumber(); // in Grand-Parent
    int idx = childNumber(); // within parent plus position Grand-Parent
    if ( idx == -1 )
    {
      return QPair<int, int>( -1, -1 );
    }
    // QgsSpatialiteDbInfoItem contains the column in 1 Item and not an Item for each column
    return QPair<int, int>( idx, parent()->columnCount() );
    // return QPair<int, int>( idx / parent()->columnCount(), idx % parent()->columnCount() );
  }
  // ### support header items?
  return QPair<int, int>( -1, -1 );
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::sniffSpatialiteDbInfo
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::sniffSpatialiteDbInfo()
{
  bool bRc = false;
  if ( ( getSpatialiteDbInfo() ) && ( getSpatialiteDbInfo()->isDbValid() ) )
  {
    mTableCounter = 0; // Count the Total amount of entries for all Spatial-Types.
    mNonSpatialTablesCounter = 0; // Count the Total amount of entries for all non-Spatial-Types.
    if ( getSpatialiteDbInfo()->dbSpatialTablesLayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::SpatialTable ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbSpatialTablesLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbSpatialViewsLayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::SpatialView ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbSpatialViewsLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbVirtualShapesLayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::VirtualShape ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbVirtualShapesLayersCount();
      }
    }
    // these values will be -1 if no admin-tables were found
    if ( getSpatialiteDbInfo()->dbRasterCoveragesLayersCount() > 0 )
    {
      // RasterLite2
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::RasterLite2Raster ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbRasterCoveragesLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbRasterLite1LayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::RasterLite1 ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbRasterLite1LayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbMBTilesLayersCount() > 0 )
    {
      // Either MBTilesTable or MBTilesView can be used [same result]
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::MBTilesTable ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbMBTilesLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbGeoPackageVectorsCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::GeoPackageVector ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbGeoPackageLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbGeoPackageRastersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::GeoPackageRaster ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbGeoPackageLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbFdoOgrLayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::GdalFdoOgr ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbFdoOgrLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbTopologyExportLayersCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::TopologyExport ) )
      {
        mTableCounter += getSpatialiteDbInfo()->dbTopologyExportLayersCount();
      }
    }
    if ( getSpatialiteDbInfo()->dbNonSpatialTablesCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::NonSpatialTables ) )
      {
        mNonSpatialTablesCounter = getSpatialiteDbInfo()->dbNonSpatialTablesCount();
      }
    }
    if ( getSpatialiteDbInfo()->getDbSridInfoCount() > 0 )
    {
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::SpatialRefSysAux ) )
      {
        mSridInfoCounter = getSpatialiteDbInfo()->getDbSridInfoCount();
      }
    }
    if ( ( getSpatialiteDbInfo()->dbVectorStylesCount() > 0 ) || ( getSpatialiteDbInfo()->dbRasterStylesCount() > 0 ) )
    {
      // Either StyleVector or StyleRaster can be used [same result]
      if ( createGroupLayerItem( QgsSpatialiteDbInfo::StyleVector ) )
      {
        if ( getSpatialiteDbInfo()->dbVectorStylesCount() > 0 )
        {
          mStylesCounter = getSpatialiteDbInfo()->dbVectorStylesCount();
        }
        if ( getSpatialiteDbInfo()->dbRasterStylesCount() > 0 )
        {
          mStylesCounter = getSpatialiteDbInfo()->dbRasterStylesCount();
        }
      }
    }
    if ( mTableCounter > 0 )
    {
      bRc = true;
#if 1
      // Only during development
      createErrorWarningsGroups();
#endif
    }
    else
    {
      createErrorWarningsGroups();
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::createErrorWarningsGroups
// When called from 'sniffSpatialiteDbInfo'
// - should be the last entry
//-----------------------------------------------------------------
// Note:
// -> Such Errors/Warnings should be reported and resolved
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::createErrorWarningsGroups()
{
  bool bRc = false;
  QString sGroupName;
  QString sGroupSort;
  QString sText;
  int iCounter = 0;
  QMap<QString, QString> dbErrors = getSpatialiteDbInfoErrors( false ); // Errors [default]
  if ( dbErrors.count() > 0 )
  {
    QgsSpatialiteDbInfoItem *dbErrorGroup = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeError );
    if ( dbErrorGroup )
    {
      if ( dbErrorGroup->isValid() )
      {
        // Column 0: getTableNameIndex()
        sGroupName = QStringLiteral( "SpatialiteDbInfo-Errors" );
        sGroupSort = QStringLiteral( "ZZZZZZE_%1" ).arg( sGroupName );
        dbErrorGroup->setText( getTableNameIndex(),  sGroupName );
        dbErrorGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialiteUnknown ) );
        // Column 1: getGeometryNameIndex()
        dbErrorGroup->setText( getGeometryNameIndex(), QStringLiteral( "Count: %1" ).arg( dbErrors.count() ) );
        // Column 2 [not used]: getGeometryTypeIndex()
        // Column 3: getSqlQueryIndex()
        sText = QStringLiteral( "Cause [%1]" ).arg( QStringLiteral( "reported during the creation of SpatialiteDbInfo" ) );
        dbErrorGroup->setText( getSqlQueryIndex(), sText );
        // Column 4 [should always be  used]: getColumnSortHidden()
        dbErrorGroup->setText( getColumnSortHidden(), sGroupSort );
        for ( QMap<QString, QString>::iterator itErrors = dbErrors.begin(); itErrors != dbErrors.end(); ++itErrors )
        {
          QgsSpatialiteDbInfoItem *dbErrorItem = new QgsSpatialiteDbInfoItem( dbErrorGroup, QgsSpatialiteDbInfoItem::ItemTypeGroupError );
          if ( dbErrorItem )
          {
            if ( dbErrorItem->isValid() )
            {
              // Column 0: getTableNameIndex()
              sText = QStringLiteral( "%1" ).arg( ++iCounter, 5, 10, QChar( '0' ) );
              dbErrorItem->setText( getTableNameIndex(), sText );
              sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sText );
              // Column 1: getGeometryNameIndex()
              dbErrorItem->setText( getGeometryNameIndex(), itErrors.key() );
              sText = QStringLiteral( "Function: %1" ).arg( itErrors.key() );
              dbErrorItem->setToolTip( getGeometryNameIndex(), sText );
              // Column 2: getGeometryTypeIndex()
              dbErrorItem->setText( getGeometryTypeIndex(), itErrors.value() );
              dbErrorItem->setToolTip( getGeometryTypeIndex(), itErrors.value() );
              // Column 3 [not used]: getSqlQueryIndex()
              // Column 4 [should always be  used]: getColumnSortHidden()
              dbErrorItem->setText( getColumnSortHidden(), sGroupSort );
              dbErrorGroup->mPrepairedChildItems.append( dbErrorItem );
            }
          }
        }
        // mPrepairedChildItems will be emptied
        dbErrorGroup->insertPrepairedChildren();
        mPrepairedChildItems.append( dbErrorGroup );
        bRc = true;
      }
    }
  }
  dbErrors = getSpatialiteDbInfoErrors( true ); // Warnings
  if ( dbErrors.count() > 0 )
  {
    iCounter = 0;
    QgsSpatialiteDbInfoItem *dbWarningGroup = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeGroupWarning );
    if ( dbWarningGroup )
    {
      if ( dbWarningGroup->isValid() )
      {
        // Column 0: getTableNameIndex()
        sGroupName = QStringLiteral( "SpatialiteDbInfo-Warnings" );
        sGroupSort = QStringLiteral( "ZZZZZZW_%1" ).arg( sGroupName );
        dbWarningGroup->setText( getTableNameIndex(),  sGroupName );
        dbWarningGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialiteUnknown ) );
        // Column 1: getGeometryNameIndex()
        dbWarningGroup->setText( getGeometryNameIndex(), QStringLiteral( "Count: %1" ).arg( dbErrors.count() ) );
        // Column 2 [not used]: getGeometryTypeIndex()
        sText = QStringLiteral( "Cause [%1]" ).arg( QStringLiteral( "reported during the creation of SpatialiteDbInfo" ) );
        dbWarningGroup->setText( getSqlQueryIndex(), sText );
        // Column 4 [should always be  used]: getColumnSortHidden()
        dbWarningGroup->setText( getColumnSortHidden(), sGroupSort );
        for ( QMap<QString, QString>::iterator itWarnings = dbErrors.begin(); itWarnings != dbErrors.end(); ++itWarnings )
        {
          QgsSpatialiteDbInfoItem *dbWarningItem = new QgsSpatialiteDbInfoItem( dbWarningGroup, QgsSpatialiteDbInfoItem::ItemTypeWarning );
          if ( dbWarningItem )
          {
            if ( dbWarningItem->isValid() )
            {
              // Column 0: getTableNameIndex()
              sText = QStringLiteral( "%1" ).arg( ++iCounter, 5, 10, QChar( '0' ) );
              dbWarningItem->setText( getTableNameIndex(), sText );
              sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sText );
              // Column 1: getGeometryNameIndex()
              dbWarningItem->setText( getGeometryNameIndex(), itWarnings.key() );
              sText = QStringLiteral( "Function: %1" ).arg( itWarnings.key() );
              dbWarningItem->setToolTip( getGeometryNameIndex(), sText );
              // Column 2: getGeometryTypeIndex()
              dbWarningItem->setText( getGeometryTypeIndex(), itWarnings.value() );
              dbWarningItem->setToolTip( getGeometryTypeIndex(), itWarnings.value() );
              // Column 3 [not used]: getSqlQueryIndex()
              // Column 4 [not used]: getColumnSortHidden()
              dbWarningItem->setText( getColumnSortHidden(), sGroupSort );
              dbWarningGroup->mPrepairedChildItems.append( dbWarningItem );
            }
          }
        }
        // mPrepairedChildItems will be emptied
        dbWarningGroup->insertPrepairedChildren();
        mPrepairedChildItems.append( dbWarningGroup );
        bRc = true;
      }
    }
  }
  if ( bRc )
  {
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::createGroupLayerItem
//-----------------------------------------------------------------
// Sub-Groups logic.
// A Layer-Name with at least 2 '_' in the TableName AND
// - there is more than 1 entry with the same base name
// -> will be added to a Sub-Group
// - other Layers will be added after the Sub-Group
//-----------------------------------------------------------------
// Sample: 'berlin_street' as Sub-Group [base name] in 'SpatialTable' Group
// -> 'berlin_street_geometries' with 'soldner_center' as geometry
// -> 'berlin_street_geometries' with 'soldner_linestring' as geometry
// -> ... 2 more ...
// -> 'berlin_street_nr' with 'soldner_center' as geometry
// -> 'berlin_street_segments' with 'soldner_segment' as geometry
//-----------------------------------------------------------------
// Sample: 'berlin_streets' as Sub-Group [base name] in 'SpatialView' Group
// -> 'berlin_streets_1650'
// -> 'berlin_streets_1700'
// -> ... 12 more ...
// -> 'berlin_streets_1990'
// -> 'berlin_streets_3000'
//-----------------------------------------------------------------
bool QgsSpatialiteDbInfoItem::createGroupLayerItem( QgsSpatialiteDbInfo::SpatialiteLayerType layerType )
{
  bool bRc = false;
  QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemType infoType = ItemTypeUnknown;
  QString sGroupCount = QString();
  QIcon iconGroup = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( layerType );
  QString sTableType =  QString();
  QString sGroupName =  QString();
  QString sParentGroupName =  QString();
  QString sGroupSort =  QString();
  QString sTableName =  QString();
  QString sGroupFilter = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QgsSpatialiteDbInfo::SpatialiteLayerTypeName( layerType ) ) ;
  if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupFilter, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
  {
    sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sGroupName );
    QString sTableTypeText = QStringLiteral( "Table" );
    switch ( layerType )
    {
      case QgsSpatialiteDbInfo::SpatialTable:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupSpatialTable;
        break;
      case QgsSpatialiteDbInfo::SpatialView:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupSpatialView;
        sTableTypeText = QStringLiteral( "View" );
        break;
      case QgsSpatialiteDbInfo::VirtualShape:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupVirtualShape;
        break;
      case QgsSpatialiteDbInfo::RasterLite1:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupRasterLite1;
        break;
      case QgsSpatialiteDbInfo::RasterLite2Raster:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupRasterLite2Raster;
        break;
      case QgsSpatialiteDbInfo::TopologyExport:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupTopologyExport;
        break;
      // Either MBTilesTable or MBTilesView can be used [same result]
      case QgsSpatialiteDbInfo::MBTilesTable:
        sGroupName = QStringLiteral( "MbTiles" ); // Instead of 'MBTilesTable'
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupMBTilesTable;
        break;
      case QgsSpatialiteDbInfo::MBTilesView:
        sGroupName = QStringLiteral( "MbTiles" ); // Instead of 'MBTilesView'
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupMBTilesView;
        break;
      case QgsSpatialiteDbInfo::GeoPackageVector:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupGeoPackageVector;
        break;
      case QgsSpatialiteDbInfo::GeoPackageRaster:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupGeoPackageRaster;
        break;
      case QgsSpatialiteDbInfo::GdalFdoOgr:
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupGdalFdoOgr;
        break;
      // Non-Spatialite-Types
      // Either StyleVector or StyleRaster can be used [same result]
      case QgsSpatialiteDbInfo::StyleVector:
        sGroupName = QStringLiteral( "Se/Sld-Styles" ); // Instead of 'StyleVector'
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupStyleVector;
        break;
      case QgsSpatialiteDbInfo::StyleRaster:
        sGroupName = QStringLiteral( "Se/Sld-Styles" ); // Instead of 'Raster'
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupStyleRaster;
        break;
      case QgsSpatialiteDbInfo::SpatialRefSysAux:
        // when shown after Styles
        infoType = QgsSpatialiteDbInfoItem::ItemTypeSpatialRefSysAuxGroups;
        break;
      case QgsSpatialiteDbInfo::NonSpatialTables:
        // when shown at end of list
        infoType = QgsSpatialiteDbInfoItem::ItemTypeGroupNonSpatialTables;
        break;
      case QgsSpatialiteDbInfo::SpatialiteTopology:
      case QgsSpatialiteDbInfo::Metadata:
      case QgsSpatialiteDbInfo::RasterLite2Vector:
      case QgsSpatialiteDbInfo::AllSpatialLayers:
      default:
        sGroupSort = QStringLiteral( "AAZZ_%1" ).arg( sGroupName ); // Unknown at end
        break;
    }
    if ( infoType != QgsSpatialiteDbInfoItem::ItemTypeUnknown )
    {
      QMap<QString, QString> mapLayers = getSpatialiteDbInfo()->getDbLayersType( layerType );
      if ( mapLayers.count() > 0 )
      {
        // StyleVector or StyleRasterwill be empty
        if ( infoType != QgsSpatialiteDbInfoItem::ItemTypeGroupNonSpatialTables )
        {
          sGroupCount = QStringLiteral( "%1 Layers" ).arg( mapLayers.count() );
          if ( mapLayers.count() < 2 )
          {
            sGroupCount = QStringLiteral( "%1 Layer" ).arg( mapLayers.count() );
          }
        }
        else
        {
          if ( mapLayers.count() > 1 )
          {
            sTableTypeText += QStringLiteral( "s" );
          }
          sGroupCount = QStringLiteral( "%1 %2" ).arg( mapLayers.count() ).arg( sTableTypeText );
        }
      }
      // Add Group and any Sub-Groups, with the Sub-Group-Layers
      QgsSpatialiteDbInfoItem *dbGroupLayerItem = new QgsSpatialiteDbInfoItem( this, infoType );
      if ( dbGroupLayerItem )
      {
        if ( dbGroupLayerItem->isValid() )
        {
          // Column 0: getTableNameIndex()
          dbGroupLayerItem->setText( getTableNameIndex(), sGroupName );
          dbGroupLayerItem->setIcon( getTableNameIndex(), iconGroup );
          // Column 1 [notused]: getGeometryNameIndex()
          // Column 2 [not used]: getGeometryTypeIndex()
          // Column 3: getSqlQueryIndex()
          // Note: total sum of group [right]
          if ( !sGroupCount.isEmpty() )
          {
            // StyleVector or StyleRasterwill be empty [set in buildStylesGroupItem]
            dbGroupLayerItem->setText( getSqlQueryIndex(), sGroupCount );
          }
          // Column 4 [should always be  used]: getColumnSortHidden()
          dbGroupLayerItem->setText( getColumnSortHidden(), sGroupSort );
          // mPrepairedChildItems will be emptied
          dbGroupLayerItem->insertPrepairedChildren();
          mPrepairedChildItems.append( dbGroupLayerItem );
          bRc = true;
        }
        else
        {
          // For some reason, an error as occured. Basic Database information should be shown.
          // - give out a message impling a possible cause [when known]
          QgsSpatialiteDbInfoItem *dbErrorItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeError );
          if ( dbErrorItem )
          {
            if ( dbErrorItem->isValid() )
            {
              // Column 0: getTableNameIndex()
              dbErrorItem->setText( getTableNameIndex(), sGroupName );
              dbErrorItem->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialiteUnknown ) );
              // Column 1: getGeometryNameIndex()
              dbErrorItem->setText( getGeometryNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeName( layerType ) );
              dbErrorItem->setIcon( getGeometryNameIndex(), iconGroup );
              // Column 2: getGeometryTypeIndex()
              sTableTypeText = QStringLiteral( "Error during creation of TableType[%1]" ).arg( sTableTypeText );
              dbErrorItem->setText( getGeometryTypeIndex(), sTableTypeText );
              // Column 3: getSqlQueryIndex()
              sTableTypeText = QStringLiteral( "Cause [%1]" ).arg( QStringLiteral( "Unknown internal error" ) );
              dbErrorItem->setText( getSqlQueryIndex(), sTableTypeText );
              // Column 4 [should always be  used]: getColumnSortHidden()
              dbErrorItem->setText( getColumnSortHidden(), sGroupSort );
              // mPrepairedChildItems will be emptied
              dbErrorItem->insertPrepairedChildren();
              mPrepairedChildItems.append( dbErrorItem );
              // Allthough an error, this will allow the collected information to be shown.
              bRc = true;
            }
          }
        }
      }
    }
  }
  return bRc;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildDbInfoItem
// To Build the Database Item
// - setting Text and Icons
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildDbInfoItem()
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeDb ) )
  {
    // Column 0: getTableNameIndex()
    setText( getTableNameIndex(), getSpatialiteDbInfo()->getFileName() );
    setIcon( getTableNameIndex(), getSpatialiteDbInfo()->getSpatialMetadataIcon() );
    setToolTip( getTableNameIndex(), QStringLiteral( "in Directory: %1" ).arg( getSpatialiteDbInfo()->getDirectoryName() ) );
    // Column 1: getGeometryNameIndex()
    setText( getGeometryNameIndex(), getSpatialiteDbInfo()->dbSpatialMetadataString() );
    setIcon( getGeometryNameIndex(), getSpatialiteDbInfo()->getSpatialMetadataIcon() );
    setToolTip( getGeometryNameIndex(), QStringLiteral( "the sqlite3 container type found was : %1" ).arg( getSpatialiteDbInfo()->dbSpatialMetadataString() ) );
    // Column 2: getGeometryTypeIndex()
    QString sProvider = QStringLiteral( "QgsSpatiaLiteProvider" );
    QString sInfoText = "";
    switch ( dbSpatialMetadata() )
    {
      case QgsSpatialiteDbInfo::SpatialiteFdoOgr:
        sProvider = QStringLiteral( "QgsOgrProvider" );
        sInfoText = QStringLiteral( "The 'SQLite' Driver is NOT active and cannot be displayed" );
        if ( getSpatialiteDbInfo()->hasDbFdoOgrDriver() )
        {
          sInfoText = QStringLiteral( "The 'SQLite' Driver is active and can be displayed" );
        }
        break;
      case QgsSpatialiteDbInfo::SpatialiteGpkg:
        sProvider = QStringLiteral( "QgsOgrProvider" );
        sInfoText = QStringLiteral( "The 'GPKG' Driver is NOT active and cannot be displayed" );
        if ( getSpatialiteDbInfo()->hasDbGdalGeoPackageDriver() )
        {
          sInfoText = QStringLiteral( "The 'GPKG' Driver is active and can be displayed" );
        }
        break;
      case QgsSpatialiteDbInfo::SpatialiteMBTiles:
        sProvider = QStringLiteral( "QgsGdalProvider" );
        sInfoText = QStringLiteral( "The 'MBTiles' Driver is NOT active and cannot be displayed" );
        if ( getSpatialiteDbInfo()->hasDbGdalMBTilesDriver() )
        {
          sInfoText = QStringLiteral( "The 'MBTiles' Driver is active and can be displayed" );
        }
        break;
      case QgsSpatialiteDbInfo::SpatialiteLegacy:
        if ( getSpatialiteDbInfo()->dbRasterLite1LayersCount() > 0 )
        {
          sProvider = QStringLiteral( "QgsGdalProvider" );
          sInfoText = QStringLiteral( "The 'RASTERLITE' Driver is NOT active and cannot be displayed" );
          if ( getSpatialiteDbInfo()->hasDbGdalRasterLite1Driver() )
          {
            sInfoText = QStringLiteral( "The 'RASTERLITE' Driver is active and can be displayed" );
          }
        }
        break;
      default:
        break;
    }
    if ( !sInfoText.isEmpty() )
    {
      sProvider = QStringLiteral( "%1 [%2]" ).arg( sProvider ).arg( sInfoText );
      setToolTip( getGeometryTypeIndex(), QStringLiteral( "Driver information : %1" ).arg( sInfoText ) );
    }
    setText( getGeometryTypeIndex(), sProvider );
    setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( getSpatialiteDbInfo()->dbSpatialMetadata() ) );
    // Column 3: getSqlQueryIndex()
    QString sLayerText = QStringLiteral( "%1 Spatial-Layers, %2 NonSpatial-Tables/Views" ).arg( getTableCounter() ).arg( getNonSpatialTablesCounter() );
    if ( mTableCounter == 1 )
    {
      sLayerText = QStringLiteral( "%1 Spatial-Layer, %2 NonSpatial-Tables/Views" ).arg( getTableCounter() ).arg( getNonSpatialTablesCounter() );
    }
    setText( getSqlQueryIndex(), sLayerText );
    // Column 4 [should always be  used]: getColumnSortHidden()
    QString sSortTag = QStringLiteral( "AAAA_%1" ).arg( getSpatialiteDbInfo()->getFileName() );
    setText( getColumnSortHidden(), sSortTag );
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildLayerOrderItem
// Tab 'Layer Order'
// - ItemTypeLayerOrderVectorLayer
// - ItemTypeLayerOrderRasterLayer
// Note: creating a Sort-Tag is not stricly needed,
// - since 'LayerOrder' is not sorted
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildLayerOrderItem()
{
  int iCountGroup = 0;
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) )
  {
    if ( ( mItemType == ItemTypeLayerOrderVectorLayer ) || ( mItemType == ItemTypeLayerOrderRasterLayer ) )
    {
      iCountGroup = 1;
      QColor fg_color = QColor( "yellow" );
      QColor fg_style = QColor( "khaki" );
      QColor bg_color = QColor( "lightyellow" );
      QColor bg_color_group = QColor( "paleturquoise" );
      QColor bg_color_Selected = QColor( "darkcyan" );
      QString sLayerName = getDbLayer()->getLayerName();
      QString sSortTag = QStringLiteral( "1AAAAA_%1" ).arg( sLayerName );
      QString sStyleType = QStringLiteral( "Raster" );
      QStringList dbStyles;
      QString sNoStyle = QStringLiteral( "nostyle" );
      QIcon iconStyleType;
      //-----------------------------------------------------------------
      if ( mItemType == ItemTypeLayerOrderVectorLayer )
      {
        sSortTag = QStringLiteral( "1AAAAB_%1" ).arg( sLayerName );
        sStyleType = QStringLiteral( "Vector" );
        iconStyleType = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleVector );
        dbStyles = QStringList( getSpatialiteDbInfo()->getDbVectorStylesInfo().values() );
      }
      else
      {
        dbStyles = QStringList( getSpatialiteDbInfo()->getDbRasterStylesInfo().values() );
        iconStyleType = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleRaster );
      }
      QString sTitle  = getDbLayer()->getTitle();
      QString sAbstract  = getDbLayer()->getAbstract();
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sLayerName );
      setToolTip( getTableNameIndex(), sLayerName );
      setIcon( getTableNameIndex(), getLayerTypeIcon() );
      // Column 1: getGeometryNameIndex()
      // Note: this may be empty
      QString sStyleNameSelected = getDbLayer()->getLayerStyleSelected();
      setText( getGeometryNameIndex(), sStyleNameSelected );
      setBackground( getGeometryNameIndex(), fg_style );
      setToolTip( getGeometryNameIndex(), getDbLayer()->getLayerStyleSelectedTitle() );
      // getLayerStyleSelectedAbstract()
      // Column 2: getGeometryTypeIndex()
      // Note: this may be empty
      setText( getGeometryTypeIndex(), sTitle );
      setToolTip( getGeometryTypeIndex(), sTitle );
      // Column 3: getSqlQueryIndex()
      // Note: this may be empty
      setText( getSqlQueryIndex(), sAbstract );
      setToolTip( getSqlQueryIndex(), sAbstract );
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sSortTag );
      if ( dbStyles.count() > 0 )
      {
        QString sStyleName;
        QString sStyleTitle;
        QString sStyleAbstract;
        QString sTestStylesForQGis;
        QString sGroupSortTag;
        QString sItemSortTag;
        QStringList layerStyles;
        if ( getDbLayer()->hasLayerStyle() )
        {
          layerStyles =  QStringList( getDbLayer()->getLayerCoverageStylesInfo().values() );
          for ( int i = 0; i < layerStyles.count(); i++ )
          {
            if ( dbStyles.contains( layerStyles.at( i ) ) )
            {
              // Remove the Layer-Styles from the Database-Styles
              dbStyles.removeAll( layerStyles.at( i ) );
            }
          }
        }
        QString sGroupName = QStringLiteral( "Non-Registered Se/Sld-Styles [%1]" ).arg( sStyleType );
        QgsSpatialiteDbInfoItem *dbStyleGroup = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
        if ( dbStyleGroup )
        {
          dbStyleGroup->setText( getTableNameIndex(), sGroupName );
          dbStyleGroup->setIcon( getTableNameIndex(), dbStyleGroup->getGroupIcon() );
          dbStyleGroup->setBackground( getTableNameIndex(), bg_color_group );
          QString sToolTipText = QStringLiteral( "Styles: [%1]" ).arg( dbStyles.count() );
          dbStyleGroup->setText( getGeometryNameIndex(), sToolTipText );
          dbStyleGroup->setToolTip( getGeometryNameIndex(), sToolTipText );
          dbStyleGroup->setBackground( getGeometryNameIndex(), bg_color_group );
          sGroupSortTag = QStringLiteral( "%1_%2" ).arg( sSortTag ).arg( sGroupName );
          // Column 4 [should always be  used]: getColumnSortHidden()
          dbStyleGroup->setText( getColumnSortHidden(), sGroupSortTag );
          for ( int i = 0; i < dbStyles.count(); i++ )
          {
            QString sStyleInfo = dbStyles.at( i );
            if ( QgsSpatialiteDbInfo::parseStyleInfo( sStyleInfo, sStyleName, sStyleTitle, sStyleAbstract, sTestStylesForQGis ) )
            {
              sItemSortTag = QStringLiteral( "%1_%2" ).arg( sGroupSortTag ).arg( sStyleName );
              QgsSpatialiteDbInfoItem *dbStyleItem = new QgsSpatialiteDbInfoItem( dbStyleGroup, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
              if ( dbStyleItem )
              {
                // Column 0: getTableNameIndex()
                dbStyleItem->setText( getTableNameIndex(), sStyleName );
                dbStyleItem->setToolTip( getTableNameIndex(), sStyleName );
                dbStyleItem->setBackground( getTableNameIndex(), bg_color );
                dbStyleItem->setIcon( getTableNameIndex(), iconStyleType );
                // Column 1: getGeometryNameIndex()
                dbStyleItem->setText( getGeometryNameIndex(), sStyleTitle );
                dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
                // These can be long string descriptions, add ToolTip
                dbStyleItem->setToolTip( getGeometryNameIndex(), sStyleTitle );
                // dbStyleItem->setEditable( getGeometryTypeIndex(), true );
                dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
                // Column 2: getGeometryTypeIndex()
                dbStyleItem->setText( getGeometryTypeIndex(), sStyleType );
                dbStyleItem->setToolTip( getGeometryTypeIndex(), sStyleType );
                dbStyleItem->setIcon( getGeometryTypeIndex(), iconStyleType );
                dbStyleItem->setBackground( getGeometryTypeIndex(), bg_color );
                // Column 3: getSqlQueryIndex()
                dbStyleItem->setText( getSqlQueryIndex(), sStyleAbstract );
                dbStyleItem->setToolTip( getSqlQueryIndex(), sStyleAbstract );
                // Column 4 [should always be  used]: getColumnSortHidden()
                dbStyleItem->setText( getColumnSortHidden(), sItemSortTag );
                dbStyleGroup->mPrepairedChildItems.append( dbStyleItem );
              }
            }
          }
          // mPrepairedChildItems will be emptied
          dbStyleGroup->insertPrepairedChildren();
          mPrepairedChildItems.append( dbStyleGroup );
          // mPrepairedChildItems will be emptied
          insertPrepairedChildren();
        }
        //-----------------------------------------------------------------
        // Note: this View will not be sorted
        // The 'Registered Styles' should be shown first,
        //  so must be added after the 'Non-Registered Styles'
        //-----------------------------------------------------------------
        if ( getDbLayer()->hasLayerStyle() )
        {
          QString sGroupName = QStringLiteral( "Registered Se/Sld-Styles [%1]" ).arg( sStyleType );
          QgsSpatialiteDbInfoItem *dbStyleGroup = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
          if ( dbStyleGroup )
          {
            dbStyleGroup->setText( getTableNameIndex(), sGroupName );
            dbStyleGroup->setIcon( getTableNameIndex(), dbStyleGroup->getGroupIcon() );
            dbStyleGroup->setBackground( getTableNameIndex(), bg_color_group );
            dbStyleGroup->setBackground( getGeometryNameIndex(), bg_color_group );
            QString sToolTipText = QStringLiteral( "Styles: [%1]" ).arg( layerStyles.count() );
            dbStyleGroup->setText( getGeometryNameIndex(), sToolTipText );
            dbStyleGroup->setToolTip( getGeometryNameIndex(), sToolTipText );
            // Column 4 [should always be  used]: getColumnSortHidden()
            sGroupSortTag = QStringLiteral( "%1_%2" ).arg( sSortTag ).arg( sGroupName );
            dbStyleGroup->setText( getColumnSortHidden(), sSortTag );
            QgsSpatialiteDbInfoItem *dbStyleItem = new QgsSpatialiteDbInfoItem( dbStyleGroup, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
            if ( dbStyleItem )
            {
              sStyleName = sNoStyle;
              sItemSortTag = QStringLiteral( "%1_A%2" ).arg( sGroupSortTag ).arg( sStyleName );
              sToolTipText = QStringLiteral( "override any Styling" );
              dbStyleItem->setText( getTableNameIndex(), sStyleName );
              dbStyleItem->setToolTip( getTableNameIndex(), sToolTipText );
              dbStyleItem->setBackground( getTableNameIndex(), bg_color );
              // Column 1: getGeometryNameIndex()
              sStyleTitle = QStringLiteral( "Do not render any Style" );
              dbStyleItem->setText( getGeometryNameIndex(), sStyleTitle );
              dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
              // Column 4 [should always be  used]: getColumnSortHidden()
              dbStyleItem->setText( getColumnSortHidden(), sItemSortTag );
              dbStyleGroup->mPrepairedChildItems.append( dbStyleItem );
            }
            for ( int i = 0; i < layerStyles.count(); i++ )
            {
              QString sStyleInfo = layerStyles.at( i );
              if ( QgsSpatialiteDbInfo::parseStyleInfo( sStyleInfo, sStyleName, sStyleTitle, sStyleAbstract, sTestStylesForQGis ) )
              {
                sItemSortTag = QStringLiteral( "%1_%2" ).arg( sGroupSortTag ).arg( sStyleName );
                QgsSpatialiteDbInfoItem *dbStyleItem = new QgsSpatialiteDbInfoItem( dbStyleGroup, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
                if ( dbStyleItem )
                {
                  if ( ( i == 0 ) && ( sStyleNameSelected.isEmpty() ) )
                  {
                    sStyleNameSelected = sStyleName;
                  }
                  // Column 0: getTableNameIndex()
                  dbStyleItem->setText( getTableNameIndex(), sStyleName );
                  if ( sStyleNameSelected == sStyleName )
                  {
                    sToolTipText = QStringLiteral( "default Style [%1]" ).arg( sStyleName );
                    dbStyleItem->setToolTip( getTableNameIndex(), sToolTipText );
                    dbStyleItem->setForeground( getTableNameIndex(), fg_color );
                    dbStyleItem->setBackground( getTableNameIndex(), bg_color_Selected );
                    dbStyleItem->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::QVariantTypeIcon( QVariant::KeySequence ) );
                    setText( getGeometryNameIndex(), sStyleName );
                    setToolTip( getGeometryNameIndex(), sStyleTitle );
                  }
                  else
                  {
                    dbStyleItem->setToolTip( getTableNameIndex(), sStyleName );
                    dbStyleItem->setBackground( getTableNameIndex(), bg_color );
                    dbStyleItem->setIcon( getTableNameIndex(), iconStyleType );
                  }
                  // Column 1: getGeometryNameIndex()
                  dbStyleItem->setText( getGeometryNameIndex(), sStyleTitle );
                  dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
                  // These can be long string descriptions, add ToolTip
                  dbStyleItem->setToolTip( getGeometryNameIndex(), sStyleTitle );
                  // dbStyleItem->setEditable( getGeometryTypeIndex(), true );
                  dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
                  // Column 2: getGeometryTypeIndex()
                  dbStyleItem->setText( getGeometryTypeIndex(), sStyleType );
                  dbStyleItem->setToolTip( getGeometryTypeIndex(), sStyleType );
                  dbStyleItem->setIcon( getGeometryTypeIndex(), iconStyleType );
                  dbStyleItem->setBackground( getGeometryTypeIndex(), bg_color );
                  // Column 3: getSqlQueryIndex()
                  dbStyleItem->setText( getSqlQueryIndex(), sStyleAbstract );
                  dbStyleItem->setToolTip( getSqlQueryIndex(), sStyleAbstract );
                  // Column 4 [should always be  used]: getColumnSortHidden()
                  dbStyleItem->setText( getColumnSortHidden(), sItemSortTag );
                  dbStyleGroup->mPrepairedChildItems.append( dbStyleItem );
                }
              }
            }
            // mPrepairedChildItems will be emptied
            dbStyleGroup->insertPrepairedChildren();
            mPrepairedChildItems.append( dbStyleGroup );
            // mPrepairedChildItems will be emptied
            insertPrepairedChildren();
          }
        }
        iCountGroup = childCount();
      }
      else
      {
        // If there are no Styles: set 'nostyle' text
        setText( getGeometryNameIndex(), sNoStyle );
      }
    }
  }
  return iCountGroup;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildDbLayerItem
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildDbLayerItem()
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) && ( mItemType == ItemTypeLayer ) )
  {
    QString sTableName = getDbLayer()->getTableName();
    QString sGeometryColumn = getDbLayer()->getGeometryColumn();
    QString sSortTag = QStringLiteral( "AZZA_%1" ).arg( getDbLayer()->getLayerName() );
    QString sGeometryTypeString = getDbLayer()->getGeometryTypeString();
    QString sFeatureCount = QString();
    QString sViewTableName = QString();
    QString sTitle  = getDbLayer()->getTitle();
    QString sAbstract  = getDbLayer()->getAbstract();
    if ( mIsVectorType )
    {
      // "SpatialView(Add, Insert, Delete | ReadOnly) 123 features"
      sFeatureCount = QStringLiteral( "%1 %2 features" ).arg( getDbLayer()->getCapabilitiesString() ).arg( getDbLayer()->getFeaturesCount() );
      if ( mLayerType == QgsSpatialiteDbInfo::SpatialView )
      {
        sViewTableName = QStringLiteral( " ViewTableName(%1)" ).arg( getDbLayer()->getViewTableName() );
      }
    }
    if ( !mIsVectorType )
    {
      sGeometryTypeString = sGeometryColumn;
      sGeometryTypeString = sAbstract;
      if ( mIsMbTiles )
      {
        sFeatureCount = QStringLiteral( "Table-based MbTiles" );
        if ( mLayerType == QgsSpatialiteDbInfo::MBTilesView )
        {
          sFeatureCount = QStringLiteral( "View-based MbTiles" );
        }
      }
    }
    // Column 0: getTableNameIndex()
    setText( getTableNameIndex(), sTableName );
    setIcon( getTableNameIndex(), getDbLayer()->getLayerTypeIcon() );
    // Column 1: getGeometryNameIndex()
    // Note: for Rasters this will be the Title
    setText( getGeometryNameIndex(), sGeometryColumn );
    if ( mIsVectorType )
    {
      // Add column information [this does not include the geometry of the Layer]
      setToolTip( getGeometryNameIndex(), sFeatureCount );
      setToolTip( getGeometryTypeIndex(), sTitle );
      setIcon( getGeometryTypeIndex(), getDbLayer()->getGeometryTypeIcon() );
    }
    else
    {
      // These can be long string descriptions, add ToolTip
      setToolTip( getTableNameIndex(), sTableName );
      setToolTip( getGeometryNameIndex(), sGeometryColumn );
      setToolTip( getGeometryTypeIndex(), sGeometryTypeString );
      if ( mIsMbTiles )
      {
        mParentItem->setText( getGeometryNameIndex(), sFeatureCount );
      }
    }
    // Column 2: getGeometryTypeIndex()
    // Note: for Rasters this will be the Abstract
    setText( getGeometryTypeIndex(), sGeometryTypeString );
    // Column 3: getSqlQueryIndex()
    if ( mIsVectorType )
    {
      setToolTip( getSqlQueryIndex(), sAbstract );
    }
    // Column 4 [should always be  used]: getColumnSortHidden()
    setText( getColumnSortHidden(), sSortTag );
    QgsSpatialiteDbInfoItem *dbMetadataRootItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeMetadataRoot );
    if ( ( dbMetadataRootItem ) && ( dbMetadataRootItem->isValid() ) )
    {
      // Note;
      // - if this is a Vector-Layer: a 'AttributeFields' Sub-Group with the Column information will be added
      // - if this is a MbTiles-Layer: a 'MbTiles.Metadata' Sub-Group with entries of the metadata-table (name,value) will be listed
      // - if this is a Layer with Styles [Spatialite/RasterLite2]: TODO
      // - A 'General Metadata' will be added for all Layers
      // Retrieve all of the created Children
      mPrepairedChildItems.append( dbMetadataRootItem->getChildItems() );
      // mPrepairedChildItems will be emptied
      insertPrepairedChildren();
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildMetadataRootItem
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildMetadataRootItem()
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) && ( parent() ) && ( mItemType == ItemTypeMetadataRoot ) )
  {
    QString sGroupName;
    QStringList saParmValues;
    if ( mIsVectorType )
    {
      if ( mIsSpatialView )
      {
      }
      else
      {
      }
      sGroupName = QStringLiteral( "AttributeFields" );
      saParmValues.append( QStringLiteral( "addColumnItems()" ) );
      // This will be retrieved from and added to the calling Parent-Item
      QgsSpatialiteDbInfoItem *dbMetadataGroupItem = new QgsSpatialiteDbInfoItem( parent(), QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup, sGroupName, saParmValues );
      if ( ( dbMetadataGroupItem ) && ( dbMetadataGroupItem->isValid() ) )
      {
        mPrepairedChildItems.append( dbMetadataGroupItem );
      }
      saParmValues.clear();
      if ( getDbLayer()->hasLayerStyle() )
      {
        sGroupName = QStringLiteral( "Se/Sld-Styles [Vector]" );
        saParmValues.append( QStringLiteral( "addLayerStylesItems()" ) );
        // This will be retrieved from and added to the calling Parent-Item
        QgsSpatialiteDbInfoItem *dbMetadataGroupItem = new QgsSpatialiteDbInfoItem( parent(), QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup, sGroupName, saParmValues );
        if ( ( dbMetadataGroupItem ) && ( dbMetadataGroupItem->isValid() ) )
        {
          mPrepairedChildItems.append( dbMetadataGroupItem );
        }
        saParmValues.clear();
      }
    }
    else
    {
      if ( mIsMbTiles )
      {
        sGroupName = QStringLiteral( "MbTiles.Metadata" );
        saParmValues.append( QStringLiteral( "addMbTilesMetadataItems()" ) );
        // This will be retrieved from and added to the calling Parent-Item
        QgsSpatialiteDbInfoItem *dbMetadataGroupItem = new QgsSpatialiteDbInfoItem( parent(), QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup, sGroupName, saParmValues );
        if ( ( dbMetadataGroupItem ) && ( dbMetadataGroupItem->isValid() ) )
        {
          mPrepairedChildItems.append( dbMetadataGroupItem );
        }
        saParmValues.clear();
      }
      else if ( mIsRasterLite1 )
      {
      }
      else if ( mIsRasterLite2 )
      {
      }
      else if ( mIsGeoPackage )
      {
      }
      if ( getDbLayer()->hasLayerStyle() )
      {
        sGroupName = QStringLiteral( "Se/Sld-Styles [Raster]" );
        saParmValues.append( QStringLiteral( "addLayerStylesItems()" ) );
        // This will be retrieved from and added to the calling Parent-Item
        QgsSpatialiteDbInfoItem *dbMetadataGroupItem = new QgsSpatialiteDbInfoItem( parent(), QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup, sGroupName, saParmValues );
        if ( ( dbMetadataGroupItem ) && ( dbMetadataGroupItem->isValid() ) )
        {
          mPrepairedChildItems.append( dbMetadataGroupItem );
        }
        saParmValues.clear();
      }
    }
    sGroupName = QStringLiteral( "General Metadata" );
    saParmValues.append( QStringLiteral( "addCommonMetadataItems()" ) );
    // This will be retrieved from and added to the calling Parent-Item
    QgsSpatialiteDbInfoItem *dbMetadataGroupItem = new QgsSpatialiteDbInfoItem( parent(), QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup, sGroupName, saParmValues );
    if ( ( dbMetadataGroupItem ) && ( dbMetadataGroupItem->isValid() ) )
    {
      mPrepairedChildItems.append( dbMetadataGroupItem );
    }
    saParmValues.clear();
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildStylesSubGroups
// Build Group of Vector/Raster-Styles defined in the Database
// - not of the Layer [Layer is done in addLayerStylesItems]
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildStylesSubGroups( )
{
  if ( getSpatialiteDbInfo() )
  {
    if ( ( mItemType == ItemTypeGroupStyleVector ) || ( mItemType == ItemTypeGroupStyleRaster ) )
    {
      QString sTableType =  QString();
      QString sGroupName =  QString();
      QString sParentGroupName =  QString();
      QString sGroupSort =  QString();
      QString sTableName =  QString();
      QString sGroupFilter = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QgsSpatialiteDbInfo::SpatialiteLayerTypeName( QgsSpatialiteDbInfo::StyleVector ) ) ;
      if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupFilter, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
      {
        sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sGroupName );
        // Column 0: getTableNameIndex()
        setText( getTableNameIndex(), sGroupName );
        setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleVector ) );
        // Column 1[not used]: getGeometryNameIndex()
        // Column 2[not used]: getGeometryTypeIndex()
        // Column 3: getSqlQueryIndex()
        sTableType = QStringLiteral( "Style" );
        int iCountStyles = getSpatialiteDbInfo()->dbVectorStylesCount() + getSpatialiteDbInfo()->dbRasterStylesCount();
        if ( iCountStyles > 1 )
        {
          sTableType += QStringLiteral( "s" );
        }
        sParentGroupName = QStringLiteral( "%1: %2" ).arg( sTableType ).arg( iCountStyles );
        setText( getSqlQueryIndex(), sParentGroupName );
        // Column 4 [should always be  used]: getColumnSortHidden()
        setText( getColumnSortHidden(), sGroupSort );
        QStringList saParmValues;
        if ( getSpatialiteDbInfo()->dbVectorStylesCount() > 0 )
        {
          sTableType = QgsSpatialiteDbInfo::SpatialiteLayerTypeName( QgsSpatialiteDbInfo::StyleVector );
          QgsSpatialiteDbInfoItem *dbStyleGroupItem = new QgsSpatialiteDbInfoItem( this, ItemTypeGroupStyleVector, sTableType, saParmValues );
          if ( ( dbStyleGroupItem ) && ( dbStyleGroupItem->isValid() ) )
          {
            mPrepairedChildItems.append( dbStyleGroupItem );
          }
          saParmValues.clear();
        }
        if ( getSpatialiteDbInfo()->dbRasterStylesCount() > 0 )
        {
          sTableType = QgsSpatialiteDbInfo::SpatialiteLayerTypeName( QgsSpatialiteDbInfo::StyleRaster );
          QgsSpatialiteDbInfoItem *dbStyleGroupItem = new QgsSpatialiteDbInfoItem( this, ItemTypeGroupStyleRaster, sTableType, saParmValues );
          if ( ( dbStyleGroupItem ) && ( dbStyleGroupItem->isValid() ) )
          {
            mPrepairedChildItems.append( dbStyleGroupItem );
          }
          saParmValues.clear();
        }
        // mPrepairedChildItems will be emptied
        insertPrepairedChildren();
      }
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildStylesGroupItem
// - Key: StyleId as retrieved from SE_vector_styled_layers_view
// - Value: StyleName, StyleTitle, StyleAbstract,TestStylesForQgis
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildStylesGroupItem( QString sGroupName, QStringList saParmValues )
{
  Q_UNUSED( saParmValues );
  if ( getSpatialiteDbInfo() )
  {
    QgsSpatialiteDbInfo::SpatialiteLayerType styleType = QgsSpatialiteDbInfo::Metadata;
    QMap<int, QString> mapStyles;
    switch ( mItemType )
    {
      case ItemTypeGroupStyleVector:
        styleType = QgsSpatialiteDbInfo::StyleVector;
        mapStyles = getSpatialiteDbInfo()->getDbVectorStylesInfo();
        break;
      case ItemTypeGroupStyleRaster:
        styleType = QgsSpatialiteDbInfo::StyleRaster;
        mapStyles = getSpatialiteDbInfo()->getDbRasterStylesInfo();
        break;
      default:
        return childCount();
        break;
    }
    QIcon iconType = QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( styleType );
    QString sTableType =  QString();
    QString sGroupTypeName =  QString();
    QString sParentGroupName =  QString();
    QString sGroupSort =  QString();
    QString sTableName =  QString();
    QString sGroupFilter = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QgsSpatialiteDbInfo::SpatialiteLayerTypeName( styleType ) ) ;
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupFilter, sTableType, sGroupTypeName, sParentGroupName, sGroupSort, sTableName ) )
    {
      sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sGroupName );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sGroupName );
      setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleVector ) );
      // Column 1: getGeometryNameIndex()
      sTableName = QStringLiteral( "Style" );
      int iCountStyles = mapStyles.count();
      if ( iCountStyles > 1 )
      {
        sTableName += QStringLiteral( "s" );
      }
      sParentGroupName = QStringLiteral( "%1: %2" ).arg( sTableName ).arg( iCountStyles );
      setText( getGeometryNameIndex(), sParentGroupName );
      // Column 2[not used]: getGeometryTypeIndex()
      // Column 3[not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sGroupSort );
      QColor bg_color = QColor( "lightyellow" );
      QString sStyleName;
      QString sStyleTitle;
      QString sStyleAbstract;
      QString sStyleSort;
      for ( QMap<int, QString>::iterator itStyles = mapStyles.begin(); itStyles != mapStyles.end(); ++itStyles )
      {
        if ( !itStyles.value().isEmpty() )
        {
          int iStyleId = itStyles.key();
          if ( QgsSpatialiteDbInfo::parseStyleInfo( itStyles.value(), sStyleName, sStyleTitle, sStyleAbstract, sStyleSort ) )
          {
            sStyleSort = QStringLiteral( "%1_%2_%3" ).arg( sGroupSort ).arg( sGroupName ).arg( sStyleName );
            QgsSpatialiteDbInfoItem *dbStyleItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata );
            if ( dbStyleItem )
            {
              // Column 0: getTableNameIndex()
              dbStyleItem->setText( getTableNameIndex(), sStyleName );
              dbStyleItem->setToolTip( getTableNameIndex(), sStyleName );
              QString sToolTipText = QStringLiteral( "StyleId [%1]" ).arg( iStyleId );
              dbStyleItem->setBackground( getTableNameIndex(), bg_color );
              dbStyleItem->setIcon( getTableNameIndex(), iconType );
              // Column 1: getGeometryNameIndex()
              dbStyleItem->setText( getGeometryNameIndex(), sStyleTitle );
              dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
              // These can be long string descriptions, add ToolTip
              dbStyleItem->setToolTip( getGeometryNameIndex(), sStyleTitle );
              // dbStyleItem->setEditable( getGeometryTypeIndex(), true );
              dbStyleItem->setBackground( getGeometryNameIndex(), bg_color );
              // Column 2: getGeometryTypeIndex()
              sToolTipText = QStringLiteral( "StyleId=%1 [%2]" ).arg( iStyleId ).arg( sTableType ) ;
              dbStyleItem->setText( getGeometryTypeIndex(), sToolTipText );
              dbStyleItem->setToolTip( getGeometryTypeIndex(), sToolTipText );
              dbStyleItem->setIcon( getGeometryTypeIndex(), iconType );
              dbStyleItem->setBackground( getGeometryTypeIndex(), bg_color );
              // Column 3: getSqlQueryIndex()
              dbStyleItem->setText( getSqlQueryIndex(), sStyleAbstract );
              dbStyleItem->setToolTip( getSqlQueryIndex(), sStyleAbstract );
              dbStyleItem->setBackground( getSqlQueryIndex(), bg_color );
              // Column 4 [should always be  used]: getColumnSortHidden()
              dbStyleItem->setText( getColumnSortHidden(), sStyleSort );
              mPrepairedChildItems.append( dbStyleItem );
            }
          }
        }
      }
      // mPrepairedChildItems will be emptied
      insertPrepairedChildren();
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildMetadataGroupItem
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildMetadataGroupItem( QString sGroupName, QStringList saParmValues )
{
  if ( ( getSpatialiteDbInfo() ) && ( getDbLayer() ) && ( parent() ) && ( mItemType == ItemTypeMetadataGroup ) )
  {
    if ( saParmValues.count() > 0 )
    {
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sGroupName );
      setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleVector ) );
      QString sCommand = saParmValues.at( 0 );
      if ( mIsVectorType )
      {
        if ( sCommand == QStringLiteral( "addColumnItems()" ) )
        {
          // Column 1[not used]: getGeometryNameIndex()
          if ( addColumnItems()  > 0 )
          {
            // will now have children tables
          }
        }
      }
      else
      {
        if ( mIsMbTiles )
        {
          if ( sCommand == QStringLiteral( "addMbTilesMetadataItems()" ) )
          {
            if ( addMbTilesMetadataItems() > 0 )
            {
              // will now have children tables
            }
          }
        }
      }
      if ( sCommand == QStringLiteral( "addLayerStylesItems()" ) )
      {
        if ( addLayerStylesItems() > 0 )
        {
          // will now have children tables
        }
      }
      if ( sCommand == QStringLiteral( "addCommonMetadataItems()" ) )
      {
        if ( addCommonMetadataItems() > 0 )
        {
          // will now have children tables
        }
      }
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildHelpItem
// Will create a 'help' text to explain the basic structure of
// what is shown in the QgsSpatiaLiteSourceSelect after a
// Database has been loaded.
// Will be remove after the first Database has been loaded.
// Note:
// If a 'double free or corruption (fasttop)' turns up when QGis is closed
// - look for a created Item that has NOT been added with 'addChild'
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildHelpItem()
{
  // Create text for Help
  if ( ( !getSpatialiteDbInfo() ) && ( mItemType == ItemTypeHelpRoot ) )
  {
    // Column 0: getTableNameIndex()
    int iSortCounter = 0;
    QString sText = QStringLiteral( "Help text for QgsSpatiaLiteSourceSelect" );
    setText( getTableNameIndex(), sText );
    setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::Spatialite50 ) );
    sText = QStringLiteral( "Browser for sqlite3 containers supported by" );
    // Column 1: getGeometryNameIndex()
    setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Spatialite, RasterLite2, Gdal and Ogr Providers" );
    setText( getGeometryTypeIndex(), sText );
    QString sSortTag = QStringLiteral( "AAAA_%1" ).arg( iSortCounter++ );
    setText( getColumnSortHidden(), sSortTag );
    // new Child to be added to Help Text
    QgsSpatialiteDbInfoItem *dbHelpItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "Listing of Items" );
    dbHelpItem->setText( getTableNameIndex(), sText );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Tables/Views will be listed in Sub-Groups" );
    dbHelpItem->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "A Sub-Group will only be shown when valid entries exist" );
    dbHelpItem->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpItem->setText( getColumnSortHidden(), sSortTag );
    QgsSpatialiteDbInfoItem *dbHelpSubGroups = new QgsSpatialiteDbInfoItem( dbHelpItem, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "Sub-Groups" );
    dbHelpSubGroups->setText( getTableNameIndex(), sText );
    sText = QStringLiteral( "based on Provider-Type" );
    dbHelpSubGroups->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Listing the Tables/Views togeather, with the amount found" );
    dbHelpSubGroups->setText( getGeometryTypeIndex(), sText );
    iSortCounter = 0;
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroups->setText( getColumnSortHidden(), sSortTag );
    QgsSpatialiteDbInfoItem *dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "SpatialTables" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialTable ) );
    sText = QStringLiteral( "Spatialite and RasterLite2" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the Table/Geometry-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "SpatialViews" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialView ) );
    sText = QStringLiteral( "Spatialite, RasterLite2 and RasterLite1" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the View/Geometry-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "VirtualShapes" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::VirtualShape ) );
    sText = QStringLiteral( "Spatialite, RasterLite2" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the VirtualShape-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite2Rasters" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::RasterLite2Raster ) );
    sText = QStringLiteral( "RasterLite2" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the RasterCoverage-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "GeoPackageVectors" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::GeoPackageVector ) );
    sText = QStringLiteral( "GeoPackage" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the Table/Geometry-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "GeoPackageRasters" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::GeoPackageRaster ) );
    sText = QStringLiteral( "GeoPackage" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the Table-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "OgrFdo" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::GdalFdoOgr ) );
    sText = QStringLiteral( "Ogr" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the Table/Geometry-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "MBTiles" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::MBTilesTable ) );
    sText = QStringLiteral( "Gdal" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the value entered in metadata.name" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite1" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::RasterLite1 ) );
    sText = QStringLiteral( "Gdal" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the RasterCoverage-Names" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACA_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    dbHelpSubGroup = new QgsSpatialiteDbInfoItem( dbHelpSubGroups, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "NonSpatialTables" );
    dbHelpSubGroup->setText( getTableNameIndex(), sText );
    dbHelpSubGroup->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::NonSpatialTables ) );
    sText = QStringLiteral( "with Data, Administration Tables" );
    dbHelpSubGroup->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "These entries will always be shown, when any exist and are Provider specific" );
    dbHelpSubGroup->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACB_%1" ).arg( iSortCounter++ );
    dbHelpSubGroup->setText( getColumnSortHidden(), sSortTag );
    QgsSpatialiteDbInfoItem *dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "Table-Data" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatiale Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Showing the Table-Names" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite2-Metadata" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatial Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite2-Tiles" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "Tile-Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Tables containing Raster-Tiles" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite1-Metadata" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatial Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "GeoPackage" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatial Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "MBTiles" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatial Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "RasterLite1-Tiles" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "Tile-Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Tables containing Raster-Tiles" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "Data" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "non-Spatial Tables/Views" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "used as Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "Spatialite" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialTable ) );
    sText = QStringLiteral( "internal Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "without SpatialIndex" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "SpatialIndex" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "index Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "for SpatialIndex" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "Geometries-Admin" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "internal Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "for FdoOgr" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "Sqlite3" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "internal Administration Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "for sqlite3" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    dbHelpNonSpatialTable = new QgsSpatialiteDbInfoItem( dbHelpSubGroup, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    sText = QStringLiteral( "EPSG-Admin" );
    dbHelpNonSpatialTable->setText( getTableNameIndex(), sText );
    dbHelpNonSpatialTable->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sText ) );
    sText = QStringLiteral( "Spatial-Reference Tables" );
    dbHelpNonSpatialTable->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "for Spatialite, FdoOgr, GeoPackage" );
    dbHelpNonSpatialTable->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AACC_%1" ).arg( iSortCounter++ );
    dbHelpNonSpatialTable->setText( getColumnSortHidden(), sSortTag );
    // Add NonSpatialTable to SubGroup NonSpatialTables
    dbHelpSubGroup->addChild( dbHelpNonSpatialTable );
    // Add Sub-Group to SubGroups
    dbHelpSubGroups->addChild( dbHelpSubGroup );
    // Add Sub-Group to the main Item
    dbHelpItem->addChild( dbHelpSubGroups );
    // Column 3 [not used]: getSqlQueryIndex()
    // Add Text about Provider [will be shown as first child]
    mPrepairedChildItems.append( dbHelpItem );
    // new Child to be added to Help Text
    dbHelpItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "Providers" );
    dbHelpItem->setText( getTableNameIndex(), sText );
    iSortCounter = 0;
    sSortTag = QStringLiteral( "AABA_%1" ).arg( iSortCounter++ );
    dbHelpItem->setText( getColumnSortHidden(), sSortTag );
    QgsSpatialiteDbInfoItem *dbHelpProviders = new QgsSpatialiteDbInfoItem( dbHelpItem, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "Spatialite" );
    dbHelpProviders->setText( getTableNameIndex(), sText );
    dbHelpProviders->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::Spatialite50 ) );
    dbHelpProviders->setBackground( getTableNameIndex(), QColor( "cadetblue" ) );
    dbHelpProviders->setForeground( getTableNameIndex(), QColor( "yellow" ) );
    dbHelpProviders->setToolTip( getTableNameIndex(), QStringLiteral( "QgsSpatialiteProvider" ) );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Versions 2.4, 3.* [Legacy] until present" );
    dbHelpProviders->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Vectors (Geometries)" );
    dbHelpProviders->setText( getGeometryTypeIndex(), sText );
    dbHelpProviders->setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::SpatialTable ) );
    sSortTag = QStringLiteral( "AABA_%1" ).arg( iSortCounter++ );
    // Column 4 [should always be  used]: getColumnSortHidden()
    dbHelpProviders->setText( getColumnSortHidden(), sSortTag );
    dbHelpItem->addChild( dbHelpProviders );
    // new Child to be added to Provider
    dbHelpProviders = new QgsSpatialiteDbInfoItem( dbHelpItem, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "RasterLite2" );
    dbHelpProviders->setText( getTableNameIndex(), sText );
    dbHelpProviders->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::Spatialite50 ) );
    dbHelpProviders->setBackground( getTableNameIndex(), QColor( "lightyellow" ) );
    dbHelpProviders->setToolTip( getTableNameIndex(), QStringLiteral( "QgsRasterite2Provider" ) );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Versions supported by QgsRasterite2Provider" );
    dbHelpProviders->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Rasters" );
    dbHelpProviders->setText( getGeometryTypeIndex(), sText );
    dbHelpProviders->setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::RasterLite2Raster ) );
    // Column 3 [not used]: getSqlQueryIndex()
    sSortTag = QStringLiteral( "AABA_%1" ).arg( iSortCounter++ );
    // Column 4 [should always be  used]: getColumnSortHidden()
    dbHelpProviders->setText( getColumnSortHidden(), sSortTag );
    dbHelpItem->addChild( dbHelpProviders );
    // new Child to be added to Provider
    dbHelpProviders = new QgsSpatialiteDbInfoItem( dbHelpItem, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "RasterLite1, GeoPackage-Rasters, MBTiles" );
    dbHelpProviders->setText( getTableNameIndex(), sText );
    dbHelpProviders->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::SpatialiteGpkg ) );
    dbHelpProviders->setBackground( getTableNameIndex(), QColor( "khaki" ) );
    dbHelpProviders->setToolTip( getTableNameIndex(), QStringLiteral( "QgsGdalProvider" ) );
    dbHelpProviders->setToolTip( getGeometryNameIndex(), QStringLiteral( "The structure will be shown, even if the if the needed driver is not active and cannot be shown" ) );
    dbHelpProviders->setToolTip( getGeometryTypeIndex(), QStringLiteral( "A message will be shown with the name of the needed driver, if it is active and can be shown" ) );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Versions supported by QgsGdalProvider" );
    dbHelpProviders->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Rasters, when the needed Driver is installed" );
    dbHelpProviders->setText( getGeometryTypeIndex(), sText );
    dbHelpProviders->setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::RasterLite2Raster ) );
    dbHelpProviders->setBackground( getGeometryTypeIndex(), QColor( "indianred" ) );
    dbHelpProviders->setForeground( getGeometryTypeIndex(), QColor( "white" ) );
    // Column 3 [not used]: getSqlQueryIndex()
    sSortTag = QStringLiteral( "AABA_%1" ).arg( iSortCounter++ );
    // Column 4 [should always be  used]: getColumnSortHidden()
    dbHelpProviders->setText( getColumnSortHidden(), sSortTag );
    dbHelpItem->addChild( dbHelpProviders );
    // new Child to be added to Provider
    dbHelpProviders = new QgsSpatialiteDbInfoItem( dbHelpItem, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "FdoOgr, GeoPackage-Vectors" );
    dbHelpProviders->setText( getTableNameIndex(), sText );
    dbHelpProviders->setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::SpatialiteFdoOgr ) );
    dbHelpProviders->setBackground( getTableNameIndex(), QColor( "powderblue" ) );
    dbHelpProviders->setToolTip( getTableNameIndex(), QStringLiteral( "QgsOgrProvider" ) );
    dbHelpProviders->setToolTip( getGeometryNameIndex(), QStringLiteral( "The structure will be shown, even if the if the needed driver is not active and cannot be shown" ) );
    dbHelpProviders->setToolTip( getGeometryTypeIndex(), QStringLiteral( "A message will be shown with the name of the needed driver, if it is active and can be shown" ) );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Versions supported by QgsOgrProvider" );
    dbHelpProviders->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Vectors, when the needed Driver is installed" );
    dbHelpProviders->setText( getGeometryTypeIndex(), sText );
    dbHelpProviders->setIcon( getGeometryTypeIndex(), QgsSpatialiteDbInfo::SpatialMetadataTypeIcon( QgsSpatialiteDbInfo::SpatialiteFdoOgr ) );
    dbHelpProviders->setBackground( getGeometryTypeIndex(), QColor( "indianred" ) );
    dbHelpProviders->setForeground( getGeometryTypeIndex(), QColor( "white" ) );
    // Column 3 [not used]: getSqlQueryIndex()
    sSortTag = QStringLiteral( "AABA_%1" ).arg( iSortCounter++ );
    // Column 4 [should always be  used]: getColumnSortHidden()
    dbHelpProviders->setText( getColumnSortHidden(), sSortTag );
    dbHelpItem->addChild( dbHelpProviders );
    // Add Provider with Children [will be shown as second (and last) child]
    mPrepairedChildItems.append( dbHelpItem );
    // new Child to be added to Help Text
    dbHelpItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "The Total amount of tables in the Database will be shown, " );
    dbHelpItem->setText( getTableNameIndex(), sText );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "togeather with the Container-Type" );
    dbHelpItem->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "with information about the needed Driver (name and if active)" );
    dbHelpItem->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AAEA_%1" ).arg( iSortCounter++ );
    dbHelpItem->setText( getColumnSortHidden(), sSortTag );
    // Add Provider with Children [will be shown as second (and last) child]
    mPrepairedChildItems.append( dbHelpItem );
    // new Child to be added to Help Text
    dbHelpItem = new QgsSpatialiteDbInfoItem( this, QgsSpatialiteDbInfoItem::ItemTypeHelpText );
    // Column 0: getTableNameIndex()
    sText = QStringLiteral( "When a Layer is added, the corresponding Provider will be used" );
    dbHelpItem->setText( getTableNameIndex(), sText );
    // Column 1: getGeometryNameIndex()
    sText = QStringLiteral( "Only sqlite3 containers are supported" );
    dbHelpItem->setText( getGeometryNameIndex(), sText );
    // Column 2: getGeometryTypeIndex()
    sText = QStringLiteral( "Raster and Vector Layers can be added togeather, when the needed Drivers are installed" );
    dbHelpItem->setText( getGeometryTypeIndex(), sText );
    sSortTag = QStringLiteral( "AADA_%1" ).arg( iSortCounter++ );
    dbHelpItem->setText( getColumnSortHidden(), sSortTag );
    // Add Provider with Children [will be shown as second (and last) child]
    mPrepairedChildItems.append( dbHelpItem );
    mPrepairedChildItems.count();
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildSpatialGroupd
// - Adding and filling NonSpatialTables Group and Sub-Groups
// -> placing the table into logical catagories
// called from createGroupLayerItem
// - calls buildSpatialSubGroup
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildSpatialSubGroups()
{
  int iCountGroups = 0;
  if ( getSpatialiteDbInfo() )
  {
    QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::SpatialiteUnknown;
    switch ( mItemType )
    {
      // Vectors that can return QgsSpatialiteDbLayer
      case ItemTypeGroupSpatialTable:
        layerType = QgsSpatialiteDbInfo::SpatialTable;
        break;
      case ItemTypeGroupSpatialView:
        layerType = QgsSpatialiteDbInfo::SpatialView;
        break;
      case ItemTypeGroupVirtualShape:
        layerType = QgsSpatialiteDbInfo::VirtualShape;
        break;
      case ItemTypeGroupTopologyExport:
        layerType = QgsSpatialiteDbInfo::TopologyExport;
        break;
      case ItemTypeGroupGeoPackageVector:
        layerType = QgsSpatialiteDbInfo::GeoPackageVector;
        break;
      case ItemTypeGroupGdalFdoOgr:
        layerType = QgsSpatialiteDbInfo::GdalFdoOgr;
        break;
      // Rasters that can return QgsSpatialiteDbLayer
      case ItemTypeGroupRasterLite2Raster:
        layerType = QgsSpatialiteDbInfo::RasterLite2Raster;
        break;
      case ItemTypeGroupRasterLite1:
        layerType = QgsSpatialiteDbInfo::RasterLite1;
        break;
      case ItemTypeGroupGeoPackageRaster:
        layerType = QgsSpatialiteDbInfo::GeoPackageRaster;
        break;
      case ItemTypeGroupMBTilesTable:
        layerType = QgsSpatialiteDbInfo::MBTilesTable;
        break;
      case ItemTypeGroupMBTilesView:
        layerType = QgsSpatialiteDbInfo::MBTilesView;
        break;
      case ItemTypeGroupStyleVector:
        layerType = QgsSpatialiteDbInfo::StyleVector;
        break;
      case ItemTypeGroupStyleRaster:
        layerType = QgsSpatialiteDbInfo::StyleRaster;
        break;
      default:
        return childCount();
        break;
    }
    // We have a supported Type
    QString sGroupName = QgsSpatialiteDbInfo::SpatialiteLayerTypeName( layerType );
    int iGroupSeperatorCount = 0;
    QStringList saSubGroupNames = getSpatialiteDbInfo()->getListSubGroupNames( sGroupName, iGroupSeperatorCount );
    // Retrieve the compleate list of all LayerNames of this Layer-Type [will be filtered]
    QStringList saLayerKeys = QStringList( getSpatialiteDbInfo()->getDbLayersType( layerType ).keys() );
    // We must work with a copy, since entries will be removed from saNonSubGroupLayerNames during the loop of saLayerKeys.
    QStringList saNonSubGroupLayerNames = saLayerKeys;
    if ( saSubGroupNames.count() > 0 )
    {
      QStringList saLayerNames;
      for ( int iGroup = 0; iGroup < saSubGroupNames.count(); iGroup++ )
      {
        // For each Sub-Group ...
        QString sSubGroupName = saSubGroupNames.at( iGroup );
        for ( int iLayers = 0; iLayers < saLayerKeys.count(); iLayers++ )
        {
          QString sLayerName = saLayerKeys.at( iLayers );
          // missing 'berlin_admin_segments'
          // Layer-Name must start with the Group-Name [avoid Layers of other Groups to be defined as a NonGroupTable Layer]
          if ( ( !sLayerName.isEmpty() ) && ( sLayerName.toLower().startsWith( sSubGroupName ) ) )
          {
            // Check if the given Layer-Name belongs to this Sub-Group [filtering]
            if ( getSpatialiteDbInfo()->checkSubGroupNames( sLayerName, sSubGroupName, iGroupSeperatorCount, saSubGroupNames ) )
            {
              // ... collect the LayerNames of that Sub-Group ...
              saLayerNames.append( sLayerName );
              if ( saNonSubGroupLayerNames.contains( sLayerName ) )
              {
                saNonSubGroupLayerNames.removeAll( sLayerName );
              }
            }
          }
        }
        if ( saSubGroupNames.count() == 1 )
        {
          // Determine the need of a Sub-Group
          if ( ( saNonSubGroupLayerNames.count() == 0 ) && ( saLayerNames.count() > 0 ) )
          {
            // if there is only 1 Sub-Group and all entries belong to that Sub-Group [i.e no NonGroupTableTypeInfo entries]
            // - then the Sub-Group should not be created and all it's entries shown as NonGroupTableTypeInfo entries
            saNonSubGroupLayerNames.append( saLayerNames ); // Add it back, will be sorted
            saLayerNames.clear();
          }
        }
        if ( saLayerNames.count() > 0 )
        {
          // ... create the Sub-Group, giving the List of Tables/Layers  ...
          QgsSpatialiteDbInfoItem *dbSubGroupItem = new QgsSpatialiteDbInfoItem( this, mItemType, sSubGroupName, saLayerNames );
          if ( dbSubGroupItem )
          {
            // ... add the (valid) Sub-Group to this Parent-Group
            mPrepairedChildItems.append( dbSubGroupItem );
          }
          saLayerNames.clear();
        }
      }
      saSubGroupNames.clear();
    }
    saLayerKeys.clear();
    if ( saNonSubGroupLayerNames.count() > 0 )
    {
      saNonSubGroupLayerNames.sort();
      for ( int iLayer = 0; iLayer < saNonSubGroupLayerNames.count(); iLayer++ )
      {
        bool bLoadLayer = true;
        // These Items are not contained in any of the Sub-Groups, add them after the Sub-Groups entries
        QgsSpatialiteDbLayer *dbLayer = getSpatialiteDbInfo()->getQgsSpatialiteDbLayer( saNonSubGroupLayerNames.at( iLayer ), bLoadLayer );
        if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
        {
          QgsSpatialiteDbInfoItem *dbLayerItem = new QgsSpatialiteDbInfoItem( this, dbLayer );
          if ( dbLayerItem )
          {
            mPrepairedChildItems.append( dbLayerItem );
          }
        }
      }
      saNonSubGroupLayerNames.clear();
    }
    // mPrepairedChildItems will be emptied
    iCountGroups = insertPrepairedChildren();
  }
  return iCountGroups;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildSpatialSubGroup
// - Adding and filling Spatial Layers of a Sub-Groups
// -> placing the table into logical catagories
// called from buildSpatialSubGroups
// calls addTableItemLayer
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildSpatialSubGroup( QString sSubGroupName, QStringList saLayerNames )
{
  int iCountGroups = 0;
  if ( getSpatialiteDbInfo() )
  {
    QgsSpatialiteDbInfo::SpatialiteLayerType layerType = QgsSpatialiteDbInfo::SpatialiteUnknown;
    QString sTableTypeText = QStringLiteral( "Table" );
    switch ( mItemType )
    {
      // Vectors that can return QgsSpatialiteDbLayer
      case ItemTypeGroupSpatialTable:
        layerType = QgsSpatialiteDbInfo::SpatialTable;
        break;
      case ItemTypeGroupSpatialView:
        layerType = QgsSpatialiteDbInfo::SpatialView;
        sTableTypeText = QStringLiteral( "View" );
        break;
      case ItemTypeGroupVirtualShape:
        layerType = QgsSpatialiteDbInfo::VirtualShape;
        break;
      case ItemTypeGroupTopologyExport:
        layerType = QgsSpatialiteDbInfo::TopologyExport;
        break;
      case ItemTypeGroupGeoPackageVector:
        layerType = QgsSpatialiteDbInfo::GeoPackageVector;
        break;
      case ItemTypeGroupGdalFdoOgr:
        layerType = QgsSpatialiteDbInfo::GdalFdoOgr;
        break;
      // Rasters that can return QgsSpatialiteDbLayer
      case ItemTypeGroupRasterLite2Raster:
        layerType = QgsSpatialiteDbInfo::RasterLite2Raster;
        break;
      case ItemTypeGroupRasterLite1:
        layerType = QgsSpatialiteDbInfo::RasterLite1;
        break;
      case ItemTypeGroupGeoPackageRaster:
        layerType = QgsSpatialiteDbInfo::GeoPackageRaster;
        break;
      case ItemTypeGroupMBTilesTable:
        layerType = QgsSpatialiteDbInfo::MBTilesTable;
        QgsDebugMsgLevel( QStringLiteral( "-W--> QgsSpatialiteDbInfoItem::buildSpatialSubGroup: MbTiles -1-, returning  ItemType[%1] ModelType[%2]" ).arg( getItemTypeString() ).arg( getModelTypeString() ), 7 );
        break;
      case ItemTypeGroupMBTilesView:
        layerType = QgsSpatialiteDbInfo::MBTilesView;
        break;
      default:
        return childCount();
        break;
    }
    QString sGroupFilter = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QgsSpatialiteDbInfo::SpatialiteLayerTypeName( layerType ) ) ;
    QString sTableType =  QString();
    QString sGroupName =  QString();
    QString sParentGroupName =  QString();
    QString sGroupSort =  QString();
    QString sTableName =  QString();
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupFilter, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
    {
      sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sSubGroupName );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sSubGroupName );
      setIcon( getTableNameIndex(), getGroupIcon() );
      if ( saLayerNames.count() > 1 )
      {
        sTableTypeText += QStringLiteral( "s" );
      }
      QString sGroupMessage = QStringLiteral( "Contains %1 %2s that starts with '%3_'" ).arg( saLayerNames.count() ).arg( sTableType ).arg( sSubGroupName );
      setToolTip( getTableNameIndex(), sGroupMessage );
      // Column 1 [not used]: getGeometryNameIndex()
      // Column 2 [not used]: getGeometryTypeIndex()
      // Note: sum of sub-group [left of total sum]
      QString sGroupCount = QStringLiteral( "%1 %2" ).arg( saLayerNames.count() ).arg( sTableTypeText );
      setText( getGeometryTypeIndex(), sGroupCount );
      // Column 3 [not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      // QString sGroupSort = QString( "AAYA_NonSpatialTables_%1" ).arg( sGroupName );
      setText( getColumnSortHidden(), sGroupSort );
      QStringList saParmValues;
      // saParmValues.append( sTableType );
      // saParmValues.append( sGroupSort );
      bool bLoadLayer = true;
      for ( int i = 0; i < saLayerNames.count(); i++ )
      {
        QString sLayerName = saLayerNames.at( i );
        // QgsDebugMsgLevel( QString( "-I-> QgsSpatialiteDbInfoItem::buildSpatialSubGroup adding_to[%1] LayerName[%2]" ).arg( sSubGroupName ).arg( sLayerName ), 7 );
        QgsSpatialiteDbLayer *dbLayer = getSpatialiteDbInfo()->getQgsSpatialiteDbLayer( sLayerName, bLoadLayer );
        if ( ( dbLayer ) && ( dbLayer->isLayerValid() ) )
        {
          QgsSpatialiteDbInfoItem *dbLayerItem = new QgsSpatialiteDbInfoItem( this, dbLayer );
          if ( dbLayerItem )
          {
            mPrepairedChildItems.append( dbLayerItem );
          }
        }
      }
      // mPrepairedChildItems will be emptied
      iCountGroups = insertPrepairedChildren();
    }
  }
  return iCountGroups;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildNonSpatialTables
// - Adding and filling NonSpatialTables Group and Sub-Groups
// -> placing the table into logical catagories
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildNonSpatialTables()
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeGroupNonSpatialTables ) )
  {
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort;
    QString sTableName;
    // Switch from a 'TableName' based QMap to a 'GroupName' based QMap
    bool bNonSpatialTablesGroups = true;
    // key: contains  GroupName; value: contains TableType,GroupName,ParentGroup-Name,SortKey,TableName
    QMultiMap<QString, QString> mapNonSpatialTables = getSpatialiteDbInfo()->getDbLayersType( QgsSpatialiteDbInfo::NonSpatialTables, bNonSpatialTablesGroups );
    // Extract the Unique List of Keys for create the Groups inside the 'NonSpatialTables' Group
    QStringList saNonSpatialUniqueKeys = QStringList( mapNonSpatialTables.uniqueKeys() );
    for ( int i = 0; i < saNonSpatialUniqueKeys.count(); i++ )
    {
      // Extract a a list of Tables belonging to the new Group
      QStringList saLayerTableInfo = QStringList( mapNonSpatialTables.values( saNonSpatialUniqueKeys.at( i ) ) );
      if ( saLayerTableInfo.count() > 0 )
      {
        //  -2- GroupName[SpatialIndex-Admin] count_tables[220]
        if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( saLayerTableInfo.at( 0 ), sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
        {
          if ( sParentGroupName.isEmpty() )
          {
            // -3a- Value[SpatialIndex-Data;SpatialIndex-Admin;;ZZZZA;rtree_middle_earth_towns_geom_update4] GroupName[SpatialIndex-Admin] ParentGroupName[] GroupSort[ZZZZA] TableType[SpatialIndex-Data] TableName[rtree_middle_earth_towns_geom_update4] saLayerTableInfo.count[220]
            // will add a Group inside  the 'NonSpatialTables' Group, with a list of Tables belonging to that Group
            QgsSpatialiteDbInfoItem *dbSubGroupsItem = new QgsSpatialiteDbInfoItem( this, ItemTypeNonSpatialTablesSubGroups, saLayerTableInfo.at( 0 ), saLayerTableInfo );
            if ( dbSubGroupsItem )
            {
              mPrepairedChildItems.append( dbSubGroupsItem );
            }
          }
          else
          {
          }
        }
      }
    }
    mPrepairedChildItems.count();
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildNonSpatialTablesGroups
// - Adding List of Admin-Tables within a Group
// called from buildNonSpatialTables
// calls buildNonSpatialTablesSubGroup when there is a Sub-Group to be filled
// calls buildNonSpatialTable when an Item does not belong to a Sub-Group
//-----------------------------------------------------------------
// TableTypeInfo[SpatialIndex-Data;SpatialIndex-Admin;;ZZZZA;rtree_middle_earth_towns_geom_update4]
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildNonSpatialTablesSubGroups( QString sGroupInfo, QStringList saTableTypeInfo )
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeNonSpatialTablesSubGroups ) )
  {
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort;
    QString sTableName;
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupInfo, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
    {
      sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sGroupName );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sGroupName );
      setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sGroupName ) );
      // Column 1 [not used]: getGeometryNameIndex()
      // Column 2: getGeometryTypeIndex()
      // Note: sum of sub-group [left of total sum of NonSpatialTables]
      QString sGroupCount = QStringLiteral( "%1 Tables/Views" ).arg( saTableTypeInfo.count() );
      setText( getGeometryTypeIndex(), sGroupCount );
      // Column 3 [not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sGroupSort );
      QStringList saParmValues;
      // saParmValues.append( sTableType );
      // saParmValues.append( sGroupSort );
      int iGroupSeperatorCount = 0;
      QStringList saSubGroupNames = getSpatialiteDbInfo()->getListSubGroupNames( sGroupName, iGroupSeperatorCount );
      // We must work with a copy, since entries will be removed from saNonGroupTableTypeInfo during the loop of saTableTypeInfo.
      QStringList saNonGroupTableTypeInfo = saTableTypeInfo;
      if ( saSubGroupNames.count() > 0 )
      {
        QStringList saGroupTableTypeInfo;
        QString sTableType;
        QString sGroupName;
        QString sParentGroupName;
        QString sGroupSort;
        QString sTableName;
        for ( int iGroup = 0; iGroup < saSubGroupNames.count(); iGroup++ )
        {
          // For each Sub-Group ...
          QString sSubGroupName = saSubGroupNames.at( iGroup );
          for ( int iTableTypeInfo = 0; iTableTypeInfo < saTableTypeInfo.count(); iTableTypeInfo++ )
          {
            QString sTableTypeInfo = saTableTypeInfo.at( iTableTypeInfo );
            if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sTableTypeInfo, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
            {
              // Table-Name must start with the Group-Name [avoid tables of other Groups to be defined as a NonGroupTable table]
              if ( sTableName.toLower().startsWith( sSubGroupName ) )
              {
                // Check if the given TableName belongs to this Sub-Group [filtering]
                // TableTypeInfo[SpatialIndex-Data;SpatialIndex-Admin;;ZZZZA;rtree_middle_earth_towns_geom_update4]
                // TableType[SpatialIndex-Data]
                // GroupName[SpatialIndex-Admin]
                // Sub-GroupName[rtree_middle_earth]
                // TableName[rtree_middle_earth_towns_geom_update4]
                // Note: 'checkSubGroupNames' will add a '_' to the Sub-GroupName [rtree_middle_earth_],
                // - checking if contains the correct amout [iGroupSeperatorCount] of '' and if TableName.startWith(GroupName)
                if ( getSpatialiteDbInfo()->checkSubGroupNames( sTableName, sSubGroupName, iGroupSeperatorCount, saSubGroupNames ) )
                {
                  // ... collect the TableTypeInfo of that Sub-Group ...
                  saGroupTableTypeInfo.append( sTableTypeInfo );
                  if ( saNonGroupTableTypeInfo.contains( sTableTypeInfo ) )
                  {
                    saNonGroupTableTypeInfo.removeAll( sTableTypeInfo );
                  }
                }
              }
            }
          }
          if ( saSubGroupNames.count() == 1 )
          {
            // Determine the need of a Sub-Group
            if ( ( saNonGroupTableTypeInfo.count() == 0 ) && ( saGroupTableTypeInfo.count() == saTableTypeInfo.count() ) )
            {
              // if there is only 1 Sub-Group and all entries belong to that Sub-Group [i.e no NonGroupTableTypeInfo entries]
              // - then the Sub-Group should not be created and all it's entries shown as NonGroupTableTypeInfo entries
              saNonGroupTableTypeInfo.append( saGroupTableTypeInfo ); // Add it back, will be sorted
              saGroupTableTypeInfo.clear();
            }
          }
          if ( saGroupTableTypeInfo.count() > 0 )
          {
            // ... create the Sub-Group, giving the List of Tables/Layers  ...
            QgsSpatialiteDbInfoItem *dbSubGroupItem = new QgsSpatialiteDbInfoItem( this, ItemTypeNonSpatialTablesSubGroup, sSubGroupName, saGroupTableTypeInfo );
            if ( dbSubGroupItem )
            {
              // ... add the (valid) Sub-Group to this Parent-Group
              mPrepairedChildItems.append( dbSubGroupItem );
            }
            saGroupTableTypeInfo.clear();
          }
        }
        // mPrepairedChildItems will be emptied
        insertPrepairedChildren();
      }
      saTableTypeInfo.clear();
      if ( saNonGroupTableTypeInfo.count() > 0 )
      {
        saNonGroupTableTypeInfo.sort();
        // These Items are not contained in any of the Sub-Groups, add them after the Sub-Groups entries
        for ( int i = 0; i < saNonGroupTableTypeInfo.count(); i++ )
        {
          QString sTableTypeInfo = saNonGroupTableTypeInfo.at( i );
          QgsSpatialiteDbInfoItem *dbNonSpatialTable = new QgsSpatialiteDbInfoItem( this, ItemTypeNonSpatialTable, sTableTypeInfo, saParmValues );
          if ( dbNonSpatialTable )
          {
            mPrepairedChildItems.append( dbNonSpatialTable );
          }
        }
      }
      // mPrepairedChildItems will be emptied
      insertPrepairedChildren();
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildSpatialSubGroup
// - Adding and filling NonSpatialTables Group and Sub-Groups
// -> placing the table into logical catagories
// TODO: compleate
// calls buildNonSpatialTable for each Item in the Sub-Group
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildNonSpatialTablesSubGroup( QString sSubGroupName, QStringList saTableTypeInfo )
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeNonSpatialTablesSubGroup ) )
  {
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort = sSubGroupName;
    QString sTableName;
    QString sTableTypeText = QStringLiteral( "Table" );
    if ( saTableTypeInfo.count() > 0 )
    {
      if ( saTableTypeInfo.count() > 1 )
      {
        sTableTypeText += QStringLiteral( "s" );
      }
      if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( saTableTypeInfo.at( 0 ), sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
      {
        // With added 'A', insure that the Sub-Groups come before the Non-Sub-Group Items
        sGroupSort = QStringLiteral( "%1A_%2" ).arg( sGroupSort ).arg( sSubGroupName );
      }
    }
    QString sGroupMessage = QStringLiteral( "Contains %1 %2s that starts with '%3_'" ).arg( saTableTypeInfo.count() ).arg( sTableType ).arg( sSubGroupName );
    if ( sSubGroupName.startsWith( "idx_" ) )
    {
      sSubGroupName = sSubGroupName.replace( "idx_", "" );
    }
    else if ( sSubGroupName.startsWith( "rtree_" ) )
    {
      sSubGroupName = sSubGroupName.replace( "rtree", "" );
    }
    else if ( sSubGroupName.startsWith( "vgpkg_" ) )
    {
      sSubGroupName = sSubGroupName.replace( "vgpkg_", "" );
    }
    else if ( sSubGroupName.startsWith( "fdo_" ) )
    {
      sSubGroupName = sSubGroupName.replace( "fdo_", "" );
    }
    // Column 0: getTableNameIndex()
    setText( getTableNameIndex(), sSubGroupName );
    setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::SpatialiteLayerTypeIcon( QgsSpatialiteDbInfo::StyleVector ) );
    setToolTip( getTableNameIndex(), sGroupMessage );
    // Column 1: getGeometryNameIndex()
    // Note: sum of sub-group [left of total sum of sub-group NonSpatialTables]
    QString sGroupCount = QStringLiteral( "%1 %2" ).arg( saTableTypeInfo.count() ).arg( sTableTypeText );
    setText( getGeometryNameIndex(), sGroupCount );
    // Column 2 [not used]: getGeometryTypeIndex()
    // Column 3 [not used]: getSqlQueryIndex()
    // Column 4 [should always be  used]: getColumnSortHidden()
    setText( getColumnSortHidden(), sGroupSort );
    QStringList saParmValues;
    for ( int i = 0; i < saTableTypeInfo.count(); i++ )
    {
      QString sTableTypeInfo = saTableTypeInfo.at( i );
      QgsSpatialiteDbInfoItem *dbNonSpatialTable = new QgsSpatialiteDbInfoItem( this, ItemTypeNonSpatialTable, sTableTypeInfo, saParmValues );
      if ( dbNonSpatialTable )
      {
        mPrepairedChildItems.append( dbNonSpatialTable );
      }
    }
    // mPrepairedChildItems will be emptied
    insertPrepairedChildren();
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildSpatialRefSysAuxGroup
// - Adding Admin-Table with text and Icon
// called from buildNonSpatialTables when an Item does not belong to a Sub-Group
// called from buildNonSpatialTablesSubGroup for the Item that belongs to that Sub-Group
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildSpatialRefSysAuxGroup()
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeSpatialRefSysAuxGroups ) && ( getSpatialiteDbInfo()->getDbSridInfoCount() > 0 ) )
  {
    QString sGroupInfo = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QStringLiteral( "SridInfo" ) );
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort;
    QString sTableName;
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupInfo, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
    {
      sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sGroupName );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sGroupName );
      setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sGroupName ) );
      QString sGroupCount = QStringLiteral( "The Srid's  being used with this Database" );
      setToolTip( getTableNameIndex(), sGroupCount );
      // Column 1: getGeometryNameIndex()
      // Note: sum of sub-group [left of total sum of NonSpatialTables]
      sGroupCount = QStringLiteral( "%1 Srid's" ).arg( getSpatialiteDbInfo()->getDbSridInfoCount() );
      setText( getGeometryNameIndex(), sGroupCount );
      sGroupCount += QStringLiteral( " are being used with this Database" );
      setToolTip( getGeometryNameIndex(), sGroupCount );
      // Column 2 [not used]: getGeometryTypeIndex()
      // Column 3 [not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sGroupSort );
      //-----------------------------------------------------------------
      QStringList saSridInfo = QStringList( getSpatialiteDbInfo()->getDbSridInfo().values() );
      int iSrid = -1;
      QString sAuthName = QString();
      QString sRefSysName = QString();
      QString sProjText = QString();
      QString sSrsWkt = QString();
      int iIsGeographic = -1;
      int iHasFlippedAxes = -1;
      QString sSpheroid = QString();
      QString sPrimeMeridian = QString();
      QString sDatum = QString();
      QString sProjection = QString();
      QString sMapUnit = QString();
      QString sAxis1Name = QString();
      QString sAxis1Orientation = QString();
      QString sAxis2Name = QString();
      QString sAxis2Orientation = QString();
      QString sProjectionParameters = QString();
      QStringList saParmValues;
      for ( int i = 0; i < saSridInfo.count(); i++ )
      {
        if ( QgsSpatialiteDbInfo::parseSridInfo( saSridInfo.at( i ), iSrid, sAuthName, sRefSysName, sProjText, sSrsWkt,
             iIsGeographic, iHasFlippedAxes, sSpheroid, sPrimeMeridian, sDatum, sProjection, sMapUnit,
             sAxis1Name, sAxis1Orientation, sAxis2Name, sAxis2Orientation, sProjectionParameters ) )
        {
          saParmValues.append( saSridInfo.at( i ) );
          if ( sAuthName.isEmpty() )
          {
            sAuthName = QStringLiteral( "EPSG" );
          }
          // this will be the subGroup-Name and 'saParmValues' will contain the Data to be shown
          sAuthName = QStringLiteral( "%1:%2" ).arg( sAuthName ).arg( iSrid );
          QgsSpatialiteDbInfoItem *dbSridInfoSubGroup = new QgsSpatialiteDbInfoItem( this, ItemTypeSpatialRefSysAuxSubGroup, sAuthName, saParmValues );
          if ( ( dbSridInfoSubGroup ) && ( dbSridInfoSubGroup->isValid() ) )
          {
            mPrepairedChildItems.append( dbSridInfoSubGroup );
          }
          saParmValues.clear();
        }
      }
      // mPrepairedChildItems will be emptied
      insertPrepairedChildren();
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildSpatialRefSysAux
// - Adding Admin-Table with text and Icon
// called from buildNonSpatialTables when an Item does not belong to a Sub-Group
// called from buildNonSpatialTablesSubGroup for the Item that belongs to that Sub-Group
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildSpatialRefSysAuxSubGroup( QString sTableTypeInfo, QStringList saParmValues )
{
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeSpatialRefSysAuxSubGroup ) && ( saParmValues.count() > 0 ) )
  {
    QString sGroupInfo = QgsSpatialiteDbInfo::getSpatialiteTypesFilter( QStringLiteral( "SridInfo" ) );
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort;
    QString sTableName;
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sGroupInfo, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
    {
      QColor bg_color = QColor( "lightyellow" );
      QColor bg_value = QColor( "paleturquoise" );
      // sTableTypeInfo will contain something like 'EPSG:3035'
      sGroupSort = QStringLiteral( "%1_%2" ).arg( sGroupSort ).arg( sTableTypeInfo );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sTableTypeInfo );
      setToolTip( getGeometryNameIndex(), sTableTypeInfo );
      setIcon( getTableNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sGroupName ) );
      // Column 1[done later]: getGeometryNameIndex()
      // - with something like 'ETRS89 / LAEA Europe'
      // Column 2 [not used]: getGeometryTypeIndex()
      // Column 3 [not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sGroupSort );
      //-----------------------------------------------------------------
      int iSrid = -1;
      QString sAuthName = QString();
      QString sRefSysName = QString();
      QString sProjText = QString();
      QString sSrsWkt = QString();
      int iIsGeographic = -1;
      int iHasFlippedAxes = -1;
      QString sSpheroid = QString();
      QString sPrimeMeridian = QString();
      QString sDatum = QString();
      QString sProjection = QString();
      QString sMapUnit = QString();
      QString sAxis1Name = QString();
      QString sAxis1Orientation = QString();
      QString sAxis2Name = QString();
      QString sAxis2Orientation = QString();
      QString sProjectionParameters = QString();
      for ( int i = 0; i < saParmValues.count(); i++ )
      {
        if ( QgsSpatialiteDbInfo::parseSridInfo( saParmValues.at( i ), iSrid, sAuthName, sRefSysName, sProjText, sSrsWkt,
             iIsGeographic, iHasFlippedAxes, sSpheroid, sPrimeMeridian, sDatum, sProjection, sMapUnit,
             sAxis1Name, sAxis1Orientation, sAxis2Name, sAxis2Orientation, sProjectionParameters ) )
        {
          // Column 1: getGeometryNameIndex()
          setText( getGeometryNameIndex(), sRefSysName );
          setToolTip( getGeometryNameIndex(), sRefSysName );
          // Create Items
          QList<QPair<QString, QString>> metaData;
          metaData.append( qMakePair( QStringLiteral( "Srid" ), QStringLiteral( "%1" ).arg( iSrid ) ) );
          metaData.append( qMakePair( QStringLiteral( "Authority Name" ), sAuthName ) );
          metaData.append( qMakePair( QStringLiteral( "Referece System Name" ), sRefSysName ) );
          metaData.append( qMakePair( QStringLiteral( "Spheroid" ), sSpheroid ) );
          metaData.append( qMakePair( QStringLiteral( "Prime Meridian" ), sPrimeMeridian ) );
          metaData.append( qMakePair( QStringLiteral( "Datum" ), sDatum ) );
          metaData.append( qMakePair( QStringLiteral( "Projection" ), sProjection ) );
          metaData.append( qMakePair( QStringLiteral( "Map Unit" ), sMapUnit ) );
          metaData.append( qMakePair( QStringLiteral( "Axis 1 Name" ), sAxis1Name ) );
          metaData.append( qMakePair( QStringLiteral( "Axis 1 Orientation" ), sAxis1Orientation ) );
          metaData.append( qMakePair( QStringLiteral( "Axis 2 Name" ), sAxis2Name ) );
          metaData.append( qMakePair( QStringLiteral( "Axis 2 Orientation" ), sAxis2Orientation ) );
          metaData.append( qMakePair( QStringLiteral( "Is Geographic" ), QStringLiteral( "%1" ).arg( QString::number( ( bool )iIsGeographic ) ) ) );
          metaData.append( qMakePair( QStringLiteral( "Has Flipped Axes" ), QStringLiteral( "%1" ).arg( QString::number( ( bool )iHasFlippedAxes ) ) ) );
          metaData.append( qMakePair( QStringLiteral( "Proj Text" ), sProjText ) );
          metaData.append( qMakePair( QStringLiteral( "Srs: Well Known Text" ), sSrsWkt ) );

          for ( int iPair = 0; iPair < metaData.count(); iPair++ )
          {
            QPair<QString, QString> pair = metaData.at( iPair );
            // Insert Items, when not empty
            if ( !pair.second.isEmpty() )
            {
              QString sName = pair.first;
              QString sValue = pair.second;
              // Make sure that this does not get re-sorted when more that 10 entries ['0001']
              QgsSpatialiteDbInfoItem *dbSridInfoItem = new QgsSpatialiteDbInfoItem( this, ItemTypeSpatialRefSysAux );
              if ( ( dbSridInfoItem ) && ( dbSridInfoItem->isValid() ) )
              {
                QString sItemSort = QStringLiteral( "%1_%2_%3" ).arg( sGroupSort ).arg( sTableTypeInfo ).arg( QStringLiteral( "%1" ).arg( iPair, 5, 10, QChar( '0' ) ) );
                // Column 0: getTableNameIndex()
                dbSridInfoItem->setText( getTableNameIndex(), sName );
                dbSridInfoItem->setToolTip( getTableNameIndex(), sName );
                dbSridInfoItem->setBackground( getTableNameIndex(), bg_color );
                // Column 1: getGeometryNameIndex()
                dbSridInfoItem->setText( getGeometryNameIndex(), sValue );
                dbSridInfoItem->setToolTip( getGeometryNameIndex(), sValue );
                dbSridInfoItem->setBackground( getGeometryNameIndex(), bg_value );
                // Column 2 [not used]: getGeometryTypeIndex()
                // Column 3 [not used]: getSqlQueryIndex()
                // Column 4 [should always be  used]: getColumnSortHidden()
                dbSridInfoItem->setText( getColumnSortHidden(), sItemSort );
                if ( ( sName == QStringLiteral( "Projection" ) ) && ( sProjectionParameters.count() > 0 ) )
                {
                  QStringList saParms = sProjectionParameters.split( QgsSpatialiteDbInfo::ParseSeparatorGeneral );
                  if ( saParms.count() > 0 )
                  {
                    dbSridInfoItem->setBackground( getTableNameIndex(), QColor( "cadetblue" ) );
                    dbSridInfoItem->setForeground( getTableNameIndex(), QColor( "yellow" ) );
                    QgsSpatialiteDbInfoItem *dbProjectionParmGroup = new QgsSpatialiteDbInfoItem( dbSridInfoItem, ItemTypeSpatialRefSysAux );
                    if ( ( dbProjectionParmGroup ) && ( dbProjectionParmGroup->isValid() ) )
                    {
                      sItemSort = QStringLiteral( "%1_%2_%3_Parms" ).arg( sGroupSort ).arg( sTableTypeInfo ).arg( QStringLiteral( "%1" ).arg( iPair, 5, 10, QChar( '0' ) ) );
                      // Column 0: getTableNameIndex()
                      sName = QStringLiteral( "Projection-Parms" );
                      dbProjectionParmGroup->setText( getTableNameIndex(), sName );
                      dbProjectionParmGroup->setToolTip( getTableNameIndex(), sName );
                      dbProjectionParmGroup->setBackground( getTableNameIndex(), bg_value );
                      dbProjectionParmGroup->setToolTip( getGeometryNameIndex(), sValue );
                      dbProjectionParmGroup->setBackground( getGeometryNameIndex(), bg_color );
                      // Column 2 [not used]: getGeometryTypeIndex()
                      // Column 3 [not used]: getSqlQueryIndex()
                      // Column 4 [should always be  used]: getColumnSortHidden()
                      dbProjectionParmGroup->setText( getColumnSortHidden(), sItemSort );
                      for ( int iParm = 0; iParm < saParms.count(); iParm++ )
                      {
                        QStringList saParmPairs = saParms.at( iParm ).split( QStringLiteral( "=" ) );
                        if ( saParmPairs.count() == 2 )
                        {
                          sName = saParmPairs.at( 0 );
                          sValue = saParmPairs.at( 1 );
                          QgsSpatialiteDbInfoItem *dbProjectionParmItem = new QgsSpatialiteDbInfoItem( dbProjectionParmGroup, ItemTypeSpatialRefSysAux );
                          if ( ( dbProjectionParmItem ) && ( dbProjectionParmItem->isValid() ) )
                          {
                            sItemSort = QStringLiteral( "%1_%2_%3_Parms_%4" ).arg( sGroupSort ).arg( sTableTypeInfo ).arg( QStringLiteral( "%1" ).arg( iPair, 5, 10, QChar( '0' ) ) ).arg( QStringLiteral( "%1" ).arg( iParm, 5, 10, QChar( '0' ) ) );
                            dbProjectionParmItem->setText( getTableNameIndex(), sName );
                            dbProjectionParmItem->setToolTip( getTableNameIndex(), sName );
                            dbProjectionParmItem->setBackground( getTableNameIndex(), bg_color );
                            // Column 1: getGeometryNameIndex()
                            dbProjectionParmItem->setText( getGeometryNameIndex(), sValue );
                            dbProjectionParmItem->setToolTip( getGeometryNameIndex(), sValue );
                            dbProjectionParmItem->setBackground( getGeometryNameIndex(), bg_value );
                            // Column 2 [not used]: getGeometryTypeIndex()
                            // Column 3 [not used]: getSqlQueryIndex()
                            // Column 4 [should always be  used]: getColumnSortHidden()
                            dbProjectionParmItem->setText( getColumnSortHidden(), sItemSort );
                            dbProjectionParmGroup->mPrepairedChildItems.append( dbProjectionParmItem );
                          }
                        }
                      }
                      // mPrepairedChildItems will be emptied
                      dbProjectionParmGroup->insertPrepairedChildren();
                      // Column 1: getGeometryNameIndex()
                      sValue = QStringLiteral( "%1 Parameters" ).arg( dbProjectionParmGroup->childCount() );
                      dbProjectionParmGroup->setText( getGeometryNameIndex(), sValue );
                      dbSridInfoItem->mPrepairedChildItems.append( dbProjectionParmGroup );
                      // mPrepairedChildItems will be emptied
                      dbSridInfoItem->insertPrepairedChildren();
                    }
                  }
                }
                mPrepairedChildItems.append( dbSridInfoItem );
              }
            }
          }
          // mPrepairedChildItems will be emptied
          insertPrepairedChildren();
        }
      }
    }
  }
  return childCount();
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::buildNonSpatialAdminTable
// - Adding Admin-Table with text and Icon
// called from buildNonSpatialTables when an Item does not belong to a Sub-Group
// called from buildNonSpatialTablesSubGroup for the Item that belongs to that Sub-Group
//-----------------------------------------------------------------
int QgsSpatialiteDbInfoItem::buildNonSpatialTable( QString sTableTypeInfo, QStringList saParmValues )
{
  int iGroupCount = 0;
  Q_UNUSED( saParmValues );
  if ( ( getSpatialiteDbInfo() ) && ( mItemType == ItemTypeNonSpatialTable ) )
  {
    QString sTableType;
    QString sGroupName;
    QString sParentGroupName;
    QString sGroupSort;
    QString sTableName;
    if ( QgsSpatialiteDbInfo::parseSpatialiteTableTypes( sTableTypeInfo, sTableType, sGroupName, sParentGroupName, sGroupSort, sTableName ) )
    {
      sGroupSort = QStringLiteral( "%1_%2_%3" ).arg( sGroupSort ).arg( sGroupName ).arg( sTableName );
      // Column 0: getTableNameIndex()
      setText( getTableNameIndex(), sTableName );
      // Some of the Table-Names are long, use TooTip the see the name
      setToolTip( getTableNameIndex(), sTableName );
      // Column 1 [not used]: getGeometryNameIndex()
      setText( getGeometryNameIndex(), sTableType );
      setIcon( getGeometryNameIndex(), QgsSpatialiteDbInfo::NonSpatialTablesTypeIcon( sTableType ) );
      // Column 2 [not used]: getGeometryTypeIndex()
      // Column 3 [not used]: getSqlQueryIndex()
      // Column 4 [should always be  used]: getColumnSortHidden()
      setText( getColumnSortHidden(), sGroupSort );
      iGroupCount = 1;
    }
  }
  return iGroupCount;
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelTypeName
// emum to String
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelTypeName( QgsSpatialiteDbInfoModel::SpatialiteDbInfoModelType modelType )
{
  //---------------------------------------------------------------
  switch ( modelType )
  {
    case QgsSpatialiteDbInfoModel::ModelTypeConnections:
      return QStringLiteral( "ModelTypeConnections" );
    case QgsSpatialiteDbInfoModel::ModelTypeLayerOrder:
      return QStringLiteral( "ModelTypeLayerOrder" );
    case QgsSpatialiteDbInfoModel::ModelTypeUnknown:
      return QStringLiteral( "ModelTypeUnknown" );
    default:
      return QStringLiteral( "ModelTypeUnknown[%1]" ).arg( modelType );
  }
}
//-----------------------------------------------------------------
// QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemTypeName
// emum to String
//-----------------------------------------------------------------
QString QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemTypeName( QgsSpatialiteDbInfoItem::SpatialiteDbInfoItemType itemType )
{
  //---------------------------------------------------------------
  switch ( itemType )
  {
    case QgsSpatialiteDbInfoItem::ItemTypeRoot:
      return QStringLiteral( "ItemTypeRoot" );
    case QgsSpatialiteDbInfoItem::ItemTypeDb:
      return QStringLiteral( "ItemTypeDb" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayer:
      return QStringLiteral( "ItemTypeLayer" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterGroup:
      return QStringLiteral( "ItemTypeLayerOrderRasterGroup" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterLayer:
      return QStringLiteral( "ItemTypeLayerOrderRasterLayer" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderRasterItem:
      return QStringLiteral( "ItemTypeLayerOrderRasterItem" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorGroup:
      return QStringLiteral( "ItemTypeLayerOrderVectorGroup" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorLayer:
      return QStringLiteral( "ItemTypeLayerOrderVectorLayer" );
    case QgsSpatialiteDbInfoItem::ItemTypeLayerOrderVectorItem:
      return QStringLiteral( "ItemTypeLayerOrderVectorItem" );
    case QgsSpatialiteDbInfoItem::ItemTypeColumn:
      return QStringLiteral( "ItemTypeColumn" );
    case QgsSpatialiteDbInfoItem::ItemTypeCommonMetadata:
      return QStringLiteral( "ItemTypeCommonMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypeMBTilesMetadata:
      return QStringLiteral( "ItemTypeMBTilesMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypeStylesMetadata:
      return QStringLiteral( "ItemTypeStylesMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypePointMetadata:
      return QStringLiteral( "ItemTypePointMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypeRectangleMetadata:
      return QStringLiteral( "ItemTypeRectangleMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypeNonSpatialTable:
      return QStringLiteral( "ItemTypeNonSpatialTable" );
    case QgsSpatialiteDbInfoItem::ItemTypeSpatialRefSysAuxGroups:
      return QStringLiteral( "ItemTypeSpatialRefSysAuxGroups" );
    case QgsSpatialiteDbInfoItem::ItemTypeSpatialRefSysAuxSubGroup:
      return QStringLiteral( "ItemTypeSpatialRefSysAuxSubGroup" );
    case QgsSpatialiteDbInfoItem::ItemTypeSpatialRefSysAux:
      return QStringLiteral( "ItemTypeSpatialRefSysAux" );
    case QgsSpatialiteDbInfoItem::ItemTypeNonSpatialTablesSubGroups:
      return QStringLiteral( "ItemTypeNonSpatialTablesSubGroups" );
    case QgsSpatialiteDbInfoItem::ItemTypeNonSpatialTablesSubGroup:
      return QStringLiteral( "ItemTypeNonSpatialTablesSubGroup" );
    case QgsSpatialiteDbInfoItem::ItemTypeHelpText:
      return QStringLiteral( "ItemTypeHelpText" );
    case QgsSpatialiteDbInfoItem::ItemTypeHelpRoot:
      return QStringLiteral( "ItemTypeHelpRoot" );
    case QgsSpatialiteDbInfoItem::ItemTypeMetadataRoot:
      return QStringLiteral( "ItemTypeMetadataRoot" );
    case QgsSpatialiteDbInfoItem::ItemTypeMetadataGroup:
      return QStringLiteral( "ItemTypeMetadataGroup" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupSpatialTable:
      return QStringLiteral( "ItemTypeGroupSpatialTable" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupSpatialView:
      return QStringLiteral( "ItemTypeGroupSpatialView" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupVirtualShape:
      return QStringLiteral( "ItemTypeGroupVirtualShape" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupRasterLite1:
      return QStringLiteral( "ItemTypeGroupRasterLite1" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupRasterLite2Vector:
      return QStringLiteral( "ItemTypeGroupRasterLite2Vector" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupRasterLite2Raster:
      return QStringLiteral( "ItemTypeGroupRasterLite2Raster" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupSpatialiteTopology:
      return QStringLiteral( "ItemTypeGroupSpatialiteTopology" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupTopologyExport:
      return QStringLiteral( "ItemTypeGroupTopologyExport" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupStyleVector:
      return QStringLiteral( "ItemTypeGroupStyleVector" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupStyleRaster:
      return QStringLiteral( "ItemTypeGroupStyleRaster" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupGdalFdoOgr:
      return QStringLiteral( "ItemTypeGroupGdalFdoOgr" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupGeoPackageVector:
      return QStringLiteral( "ItemTypeGroupGeoPackageVector" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupGeoPackageRaster:
      return QStringLiteral( "ItemTypeGroupGeoPackageRaster" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupMBTilesTable:
      return QStringLiteral( "ItemTypeGroupMBTilesTable" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupMBTilesView:
      return QStringLiteral( "ItemTypeGroupMBTilesView" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupMetadata:
      return QStringLiteral( "ItemTypeGroupMetadata" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupAllSpatialLayers:
      return QStringLiteral( "ItemTypeGroupAllSpatialLayers" );
    case QgsSpatialiteDbInfoItem::ItemTypeGroupNonSpatialTables:
      return QStringLiteral( "ItemTypeGroupNonSpatialTables" );
    case QgsSpatialiteDbInfoItem::ItemTypeWarning:
      return QStringLiteral( "ItemTypeWarning" );
    case QgsSpatialiteDbInfoItem::ItemTypeError:
      return QStringLiteral( "ItemTypeError" );
    case QgsSpatialiteDbInfoItem::ItemTypeUnknown:
      return QStringLiteral( "ItemTypeUnknown" );
    default:
      return QStringLiteral( "ItemTypeUnknown[%1]" ).arg( itemType );
  }
  //---------------------------------------------------------------
}
