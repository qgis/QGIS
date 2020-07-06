/***************************************************************************
    qgsmeshdatasetgrouptreeview.cpp
    -------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshdatasetgrouptreeview.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmeshlayer.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgssettings.h"

#include <QList>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>


QgsMeshDatasetGroupTreeModel::QgsMeshDatasetGroupTreeModel( QObject *parent )
  : QAbstractItemModel( parent )
  ,  mRootItem( new QgsMeshDatasetGroupTreeItem() )
{
}

int QgsMeshDatasetGroupTreeModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

bool QgsMeshDatasetGroupTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  switch ( role )
  {
    case Qt::EditRole:
      if ( value != QString() )
      {
        item->setName( value.toString() );
        return true;
      }
      break;
    case Qt::CheckStateRole :
      item->setIsEnabled( value.toBool() );
      return true;
  }
  return false;
}

QVariant QgsMeshDatasetGroupTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Name:
      return item->name();
    case IsVector:
      return item->isVector();
    case DatasetGroupIndex:
      return item->datasetGroupIndex();
    case Qt::CheckStateRole :
      return static_cast< int >( item->isEnabled() ? Qt::Checked : Qt::Unchecked );
    case IsMemory:
      return item->storageType() == QgsMeshDatasetGroupTreeItem::Memory;
    case Qt::DecorationRole:
      if ( item->storageType() == QgsMeshDatasetGroupTreeItem::Memory )
        return QgsApplication::getThemeIcon( QStringLiteral( "mIndicatorMemory.svg" ) );
  }

  return QVariant();
}

Qt::ItemFlags QgsMeshDatasetGroupTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

QVariant QgsMeshDatasetGroupTreeModel::headerData( int section,
    Qt::Orientation orientation,
    int role ) const
{
  Q_UNUSED( section )

  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return tr( "Groups" );

  return QVariant();
}

QModelIndex QgsMeshDatasetGroupTreeModel::index( int row, int column, const QModelIndex &parent )
const
{
  if ( !hasIndex( row, column, parent ) )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *parentItem;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  QgsMeshDatasetGroupTreeItem *childItem = parentItem->child( row );
  if ( childItem )
    return createIndex( row, column, childItem );
  else
    return QModelIndex();
}

QModelIndex QgsMeshDatasetGroupTreeModel::parent( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QModelIndex();

  QgsMeshDatasetGroupTreeItem *childItem = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  QgsMeshDatasetGroupTreeItem *parentItem = childItem->parentItem();

  if ( parentItem == mRootItem.get() )
    return QModelIndex();

  return createIndex( parentItem->row(), 0, parentItem );
}

int QgsMeshDatasetGroupTreeModel::rowCount( const QModelIndex &parent ) const
{
  QgsMeshDatasetGroupTreeItem *parentItem;
  if ( parent.column() > 0 )
    return 0;

  if ( !parent.isValid() )
    parentItem = mRootItem.get();
  else
    parentItem = static_cast<QgsMeshDatasetGroupTreeItem *>( parent.internalPointer() );

  if ( parentItem )
    return parentItem->childCount();
  else
    return 0;
}

void QgsMeshDatasetGroupTreeModel::syncToLayer( QgsMeshLayer *layer )
{
  beginResetModel();
  if ( layer && layer->datasetGroupTreeRootItem() )
    mRootItem.reset( layer->datasetGroupTreeRootItem()->clone() );
  else
    mRootItem.reset();
  endResetModel();
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeModel::datasetGroupTreeRootItem()
{
  return mRootItem.get();
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeModel::datasetGroupTreeItem( int groupIndex )
{
  if ( mRootItem )
    return mRootItem->childFromDatasetGroupIndex( groupIndex );
  else
    return nullptr;
}

bool QgsMeshDatasetGroupTreeModel::isEnabled( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return false;

  QVariant checked = data( index, Qt::CheckStateRole );

  return checked != QVariant() && checked.toInt() == Qt::Checked;
}

void QgsMeshDatasetGroupTreeModel::resetDefault( QgsMeshLayer *meshLayer )
{
  if ( !meshLayer && !mRootItem )
    return;

  beginResetModel();
  meshLayer->resetDatasetGroupTreeItem();
  mRootItem.reset( meshLayer->datasetGroupTreeRootItem()->clone() );
  endResetModel();
}

void QgsMeshDatasetGroupTreeModel::setAllGroupsAsEnabled( bool isEnabled )
{
  if ( !mRootItem )
    return;

  for ( int i = 0; i < mRootItem->childCount(); ++i )
  {
    QgsMeshDatasetGroupTreeItem *item = mRootItem->child( i );
    item->setIsEnabled( isEnabled );
    for ( int j = 0; j < item->childCount(); ++j )
    {
      QgsMeshDatasetGroupTreeItem *child = item->child( j );
      child->setIsEnabled( isEnabled );
    }
  }
  dataChanged( index( 0, 0 ), index( mRootItem->childCount(), 0 ) );
}

void QgsMeshDatasetGroupTreeModel::removeItem( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item || item->storageType() == QgsMeshDatasetGroupTreeItem::File )
    return;

  beginRemoveRows( index.parent(), index.row(), index.row() );
  QgsMeshDatasetGroupTreeItem *parent = item->parentItem();
  parent->removeChild( item );
  endRemoveRows();
}

void QgsMeshDatasetGroupTreeModel::setStorageType( const QModelIndex &index, QgsMeshDatasetGroupTreeItem::StorageType type )
{
  if ( !index.isValid() )
    return;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return;
  item->setStorageType( type );
  dataChanged( index, index );
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupProxyModel::QgsMeshDatasetGroupProxyModel( QAbstractItemModel *sourceModel ):
  QSortFilterProxyModel( sourceModel )
{
  setSourceModel( sourceModel );
}

int QgsMeshDatasetGroupProxyModel::activeScalarGroup() const
{
  return mActiveScalarGroupIndex;
}

void QgsMeshDatasetGroupProxyModel::setActiveScalarGroup( int group )
{
  if ( mActiveScalarGroupIndex == group )
    return;

  int oldGroupIndex = mActiveScalarGroupIndex;
  mActiveScalarGroupIndex = group;

  if ( oldGroupIndex > -1  || group > -1 )
    invalidate();
}

int QgsMeshDatasetGroupProxyModel::activeVectorGroup() const
{
  return mActiveVectorGroupIndex;
}

void QgsMeshDatasetGroupProxyModel::setActiveVectorGroup( int group )
{
  if ( mActiveVectorGroupIndex == group )
    return;

  int oldGroupIndex = mActiveVectorGroupIndex;
  mActiveVectorGroupIndex = group;

  if ( oldGroupIndex > -1  || group > -1 )
    invalidate();
}

Qt::ItemFlags QgsMeshDatasetGroupProxyModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  return Qt::ItemIsEnabled;
}

QVariant QgsMeshDatasetGroupProxyModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QModelIndex sourceIndex = mapToSource( index );
  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( sourceIndex.internalPointer() );

  switch ( role )
  {
    case QgsMeshDatasetGroupTreeModel::IsActiveScalarDatasetGroup:
      return item->datasetGroupIndex() == mActiveScalarGroupIndex;
    case QgsMeshDatasetGroupTreeModel::IsActiveVectorDatasetGroup:
      return item->datasetGroupIndex() == mActiveVectorGroupIndex;
    case Qt::CheckStateRole :
      return QVariant();
    case Qt::DecorationRole:
      return QVariant();
  }

  return sourceModel()->data( sourceIndex, role );
}

void QgsMeshDatasetGroupProxyModel::syncToLayer( QgsMeshLayer *layer )
{
  static_cast<QgsMeshDatasetGroupTreeModel *>( sourceModel() )->syncToLayer( layer );
  mActiveScalarGroupIndex = layer->rendererSettings().activeScalarDatasetGroup();
  mActiveVectorGroupIndex = layer->rendererSettings().activeVectorDatasetGroup();
  invalidate();
}

bool QgsMeshDatasetGroupProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

  return static_cast<QgsMeshDatasetGroupTreeModel *>( sourceModel() )->isEnabled( sourceIndex );
}


/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshDatasetGroupTreeItemDelagate::QgsMeshDatasetGroupTreeItemDelagate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mScalarSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontours.svg" ) )
  , mScalarDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshcontoursoff.svg" ) )
  , mVectorSelectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectors.svg" ) )
  , mVectorDeselectedPixmap( QStringLiteral( ":/images/themes/default/propertyicons/meshvectorsoff.svg" ) )
{
}

void QgsMeshDatasetGroupTreeItemDelagate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !painter )
    return;

  QStyledItemDelegate::paint( painter, option, index );
  bool isVector = index.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
  if ( isVector )
  {
    bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveVectorDatasetGroup ).toBool();
    painter->drawPixmap( iconRect( option.rect, true ), isActive ? mVectorSelectedPixmap : mVectorDeselectedPixmap );
  }

  bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveScalarDatasetGroup ).toBool();
  painter->drawPixmap( iconRect( option.rect, false ), isActive ? mScalarSelectedPixmap : mScalarDeselectedPixmap );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect &rect, bool isVector ) const
{
  return  iconRect( rect, isVector ? 1 : 2 );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect &rect, int pos ) const
{
  int iw = mScalarSelectedPixmap.width();
  int ih = mScalarSelectedPixmap.height();
  int margin = ( rect.height() - ih ) / 2;
  return QRect( rect.right() - pos * ( iw + margin ), rect.top() + margin, iw, ih );
}

QSize QgsMeshDatasetGroupTreeItemDelagate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QSize hint = QStyledItemDelegate::sizeHint( option, index );
  if ( hint.height() < 16 )
    hint.setHeight( 16 );
  return hint;
}

/////////////////////////////////////////////////////////////////////////////////////////

QgsMeshActiveDatasetGroupTreeView::QgsMeshActiveDatasetGroupTreeView( QWidget *parent )
  : QTreeView( parent ),
    mProxyModel( new QgsMeshDatasetGroupProxyModel( new QgsMeshDatasetGroupTreeModel( this ) ) )
{
  setModel( mProxyModel );
  setItemDelegate( &mDelegate );
  setSelectionMode( QAbstractItemView::SingleSelection );
}

void QgsMeshActiveDatasetGroupTreeView::setLayer( QgsMeshLayer *layer )
{
  if ( layer != mMeshLayer )
  {
    mMeshLayer = layer;
  }
}

int QgsMeshActiveDatasetGroupTreeView::activeScalarGroup() const
{
  return mProxyModel->activeScalarGroup();
}

void QgsMeshActiveDatasetGroupTreeView::setActiveScalarGroup( int group )
{
  if ( mProxyModel->activeScalarGroup() != group )
  {
    mProxyModel->setActiveScalarGroup( group );
    mProxyModel->invalidate();
    emit activeScalarGroupChanged( group );
  }
}

int QgsMeshActiveDatasetGroupTreeView::activeVectorGroup() const
{
  return mProxyModel->activeVectorGroup();
}

void QgsMeshActiveDatasetGroupTreeView::setActiveVectorGroup( int group )
{
  if ( mProxyModel->activeVectorGroup() != group )
  {
    mProxyModel->setActiveVectorGroup( group );
    mProxyModel->invalidate();
    emit activeVectorGroupChanged( group );
  }
}

void QgsMeshActiveDatasetGroupTreeView::syncToLayer()
{
  mProxyModel->syncToLayer( mMeshLayer );
  setActiveGroup();
}

void QgsMeshActiveDatasetGroupTreeView::mousePressEvent( QMouseEvent *event )
{
  if ( !event )
    return;

  bool processed = false;
  const QModelIndex idx = indexAt( event->pos() ) ;
  if ( idx.isValid() )
  {
    const QRect vr = visualRect( idx );
    if ( mDelegate.iconRect( vr, true ).contains( event->pos() ) )
    {
      bool isVector = idx.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
      if ( isVector )
      {
        int datasetIndex = idx.data( QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt();
        if ( datasetIndex == activeVectorGroup() )
          datasetIndex = -1;
        setActiveVectorGroup( datasetIndex );
        processed = true;
      }
    }
    else if ( mDelegate.iconRect( vr, false ).contains( event->pos() ) )
    {
      int datasetIndex = idx.data( QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt();
      if ( datasetIndex == activeScalarGroup() )
        datasetIndex = -1;
      setActiveScalarGroup( datasetIndex );
      processed = true;
    }
  }

  // only if the user did not click one of the icons do usual handling
  if ( !processed )
    QTreeView::mousePressEvent( event );
}

void QgsMeshActiveDatasetGroupTreeView::setActiveGroup()
{
  int scalarGroup = -1;
  int vectorGroup = -1;

  // find active dataset
  if ( mMeshLayer )
  {
    const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();
    scalarGroup = rendererSettings.activeScalarDatasetGroup();
    vectorGroup = rendererSettings.activeVectorDatasetGroup();
  }

  setActiveScalarGroup( scalarGroup );
  setActiveVectorGroup( vectorGroup );
}

void QgsMeshDatasetGroupListModel::syncToLayer( QgsMeshLayer *layer )
{
  if ( layer )
    mRootItem = layer->datasetGroupTreeRootItem();
}

int QgsMeshDatasetGroupListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  if ( mRootItem )
    return mRootItem->totalChildCount();
  else
    return 0;
}

QVariant QgsMeshDatasetGroupListModel::data( const QModelIndex &index, int role ) const
{
  if ( !mRootItem || ! index.isValid() )
    return QVariant();

  if ( index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = mRootItem->childFromDatasetGroupIndex( index.row() );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
      if ( mDisplayProviderName )
        return item->providerName();
      else
        return item->name();
      break;
  }

  return QVariant();
}

void QgsMeshDatasetGroupListModel::setDisplayProviderName( bool displayProviderName )
{
  mDisplayProviderName = displayProviderName;
}

QgsMeshDatasetGroupTreeView::QgsMeshDatasetGroupTreeView( QWidget *parent ):
  QTreeView( parent )
  , mModel( new QgsMeshDatasetGroupTreeModel( this ) )
  , mSaveMenu( new QgsMeshDatasetGroupSaveMenu( this ) )
{
  setItemDelegate( &mDelegate );
  setModel( mModel );
  setSelectionMode( QAbstractItemView::SingleSelection );

  connect( mSaveMenu, &QgsMeshDatasetGroupSaveMenu::datasetGroupSaved, this, &QgsMeshDatasetGroupTreeView::onDatasetGroupSaved );
}

void QgsMeshDatasetGroupTreeView::syncToLayer( QgsMeshLayer *layer )
{
  if ( mModel )
    mModel->syncToLayer( layer );
  if ( mSaveMenu )
    mSaveMenu->setMeshLayer( layer );
}

void QgsMeshDatasetGroupTreeView::selectAllGroups()
{
  selectAllItem( true );
}

void QgsMeshDatasetGroupTreeView::deselectAllGroups()
{
  selectAllItem( false );
}

void QgsMeshDatasetGroupTreeView::contextMenuEvent( QContextMenuEvent *event )
{
  QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
    setCurrentIndex( QModelIndex() );

  std::unique_ptr<QMenu> menu( createContextMenu() );
  if ( menu && menu->actions().count() != 0 )
    menu->exec( mapToGlobal( event->pos() ) );
}

void QgsMeshDatasetGroupTreeView::removeCurrentItem()
{
  if ( QMessageBox::question( this, tr( "Remove Dataset Group" ), tr( "Remove dataset group?" ) ) == QMessageBox::Ok )
    mModel->removeItem( currentIndex() );
}

void QgsMeshDatasetGroupTreeView::onDatasetGroupSaved()
{
  mModel->setStorageType( currentIndex(), QgsMeshDatasetGroupTreeItem::File );
  emit apply();
}

QMenu *QgsMeshDatasetGroupTreeView::createContextMenu()
{
  QMenu *contextMenu = new QMenu;

  const QModelIndex &index = currentIndex();
  if ( !index.isValid() )
    return nullptr;

  int groupIndex = mModel->data( index, QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt();
  QgsMeshDatasetGroupTreeItem *item = mModel->datasetGroupTreeItem( groupIndex );

  if ( !item )
    return nullptr;

  switch ( item->storageType() )
  {
    case QgsMeshDatasetGroupTreeItem::None:
      break;
    case QgsMeshDatasetGroupTreeItem::File:
      break;
    case QgsMeshDatasetGroupTreeItem::Memory:
    case QgsMeshDatasetGroupTreeItem::OnTheFly:
      contextMenu->addAction( tr( "Remove Dataset Group" ), this, &QgsMeshDatasetGroupTreeView::removeCurrentItem );
      mSaveMenu->createSaveMenu( groupIndex, contextMenu );
      break;
  }
  return contextMenu;
}

void QgsMeshDatasetGroupTreeView::resetDefault( QgsMeshLayer *meshLayer )
{
  mModel->resetDefault( meshLayer );
}

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeView::datasetGroupTreeRootItem()
{
  return  mModel->datasetGroupTreeRootItem();
}

void QgsMeshDatasetGroupTreeView::selectAllItem( bool isChecked )
{
  mModel->setAllGroupsAsEnabled( isChecked );
}

QMenu *QgsMeshDatasetGroupSaveMenu::createSaveMenu( int groupIndex, QMenu *parentMenu )
{
  if ( !mMeshLayer )
    return  nullptr;

  QMenu *menu = new QMenu( parentMenu );
  menu->setTitle( QObject::tr( "Save Datasets Group as..." ) );
  QgsMeshDatasetGroupMetadata groupMeta = mMeshLayer->datasetGroupMetadata( groupIndex );

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( mMeshLayer->dataProvider()->name() );
  if ( providerMetadata )
  {
    const QList<QgsMeshDriverMetadata> allDrivers = providerMetadata->meshDriversMetadata();
    for ( const QgsMeshDriverMetadata driver : allDrivers )
    {
      QString driverName = driver.name();
      QString suffix = driver.writeDatasetOnFileSuffix();
      if ( ( driver.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteFaceDatasets )
             && groupMeta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces ) ||
           ( driver.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteVertexDatasets )
             && groupMeta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices ) ||
           ( driver.capabilities().testFlag( QgsMeshDriverMetadata::MeshDriverCapability::CanWriteEdgeDatasets )
             && groupMeta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges ) )
      {
        menu->addAction( driver.description(), [groupIndex, driverName, suffix, this]
        {
          this->saveDatasetGroup( groupIndex, driverName, suffix );
        } );
      }
    }
  }

  if ( menu->actions().isEmpty() )
  {
    menu->addAction( QObject::tr( "No Driver Available to Write this Dataset Group" ) );
    menu->actions().last()->setDisabled( true );
  }

  if ( parentMenu )
    parentMenu->addMenu( menu );

  return menu;
}

void QgsMeshDatasetGroupSaveMenu::setMeshLayer( QgsMeshLayer *meshLayer )
{
  mMeshLayer = meshLayer;
}

void QgsMeshDatasetGroupSaveMenu::saveDatasetGroup( int datasetGroup, const QString &driver, const QString &fileSuffix )
{
  if ( !mMeshLayer )
    return;

  QgsSettings settings;
  QString filter;
  if ( !fileSuffix.isEmpty() )
    filter = QStringLiteral( "%1 (*.%2)" ).arg( driver ).arg( fileSuffix );
  QString exportFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  QString saveFileName = QFileDialog::getSaveFileName( nullptr,
                         QObject::tr( "Save Mesh Datasets" ),
                         exportFileDir,
                         filter );

  if ( saveFileName.isEmpty() )
    return;

  QFileInfo openFileInfo( saveFileName );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );


  if ( mMeshLayer->saveDataset( saveFileName, datasetGroup, driver ) )
  {
    QMessageBox::warning( nullptr, QObject::tr( "Save Mesh Datasets" ), QObject::tr( "Saving datasets fails" ) );
  }
  else
  {
    emit datasetGroupSaved();
    QMessageBox::information( nullptr, QObject::tr( "Save Mesh Datasets" ), QObject::tr( "Datasets successfully saved on file" ) );
  }

}

QgsMeshDatasetGroupTreeView::Delegate::Delegate( QObject *parent ): QStyledItemDelegate( parent )
{
}

void QgsMeshDatasetGroupTreeView::Delegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyleOptionViewItem opt = option;
  opt.decorationPosition = QStyleOptionViewItem::Right;
  QStyledItemDelegate::paint( painter, opt, index );
}
