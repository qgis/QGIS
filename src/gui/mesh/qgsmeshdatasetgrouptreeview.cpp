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
#include <QHeaderView>


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

QVariant QgsMeshDatasetGroupTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Name:
      return item->name();
      break;
    case Qt::CheckStateRole :
      if ( index.column() == 0 )
        return static_cast< int >( item->isEnabled() ? Qt::Checked : Qt::Unchecked );
      break;
    case DatasetGroupIndex:
      return item->datasetGroupIndex();
      break;
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

QgsMeshDatasetGroupTreeItem *QgsMeshDatasetGroupTreeModel::datasetGroupTreeItem( QModelIndex index )
{
  if ( !index.isValid() || !hasIndex( index.row(), index.column(), index.parent() ) )
    return nullptr;

  return static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
}

bool QgsMeshDatasetGroupTreeModel::isEnabled( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return false;

  const QVariant checked = data( index, Qt::CheckStateRole );

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
  if ( !item || item->datasetGroupType() == QgsMeshDatasetGroup::Persistent )
    return;

  beginRemoveRows( index.parent(), index.row(), index.row() );
  QgsMeshDatasetGroupTreeItem *parent = item->parentItem();
  parent->removeChild( item );
  endRemoveRows();
}

void QgsMeshDatasetGroupTreeModel::setPersistentDatasetGroup( const QModelIndex &index, const QString &uri )
{
  if ( !index.isValid() )
    return;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return;
  item->setPersistentDatasetGroup( uri );
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

  const int oldGroupIndex = mActiveScalarGroupIndex;
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

  const int oldGroupIndex = mActiveVectorGroupIndex;
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

  const QModelIndex sourceIndex = mapToSource( index );
  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( sourceIndex.internalPointer() );

  switch ( role )
  {
    case QgsMeshDatasetGroupTreeModel::IsVector:
      return item->isVector();
    case QgsMeshDatasetGroupTreeModel::IsActiveScalarDatasetGroup:
      return item->datasetGroupIndex() == mActiveScalarGroupIndex;
    case QgsMeshDatasetGroupTreeModel::IsActiveVectorDatasetGroup:
      return item->datasetGroupIndex() == mActiveVectorGroupIndex;
    case Qt::CheckStateRole :
      return QVariant();
    case Qt::DecorationRole:
      return QVariant();

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    case Qt::BackgroundColorRole:
#else
    case Qt::BackgroundRole:
#endif
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
  const QModelIndex sourceIndex = sourceModel()->index( source_row, 0, source_parent );

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
  const bool isVector = index.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
  if ( isVector )
  {
    const bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveVectorDatasetGroup ).toBool();
    painter->drawPixmap( iconRect( option.rect, true ), isActive ? mVectorSelectedPixmap : mVectorDeselectedPixmap );
  }

  const bool isActive = index.data( QgsMeshDatasetGroupTreeModel::IsActiveScalarDatasetGroup ).toBool();
  painter->drawPixmap( iconRect( option.rect, false ), isActive ? mScalarSelectedPixmap : mScalarDeselectedPixmap );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect &rect, bool isVector ) const
{
  return  iconRect( rect, isVector ? 1 : 2 );
}

QRect QgsMeshDatasetGroupTreeItemDelagate::iconRect( const QRect &rect, int pos ) const
{
  const int iw = mScalarSelectedPixmap.width();
  const int ih = mScalarSelectedPixmap.height();
  const int margin = ( rect.height() - ih ) / 2;
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
      const bool isVector = idx.data( QgsMeshDatasetGroupTreeModel::IsVector ).toBool();
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

QgsMeshDatasetGroupListModel::QgsMeshDatasetGroupListModel( QObject *parent ): QAbstractListModel( parent )
{}

void QgsMeshDatasetGroupListModel::syncToLayer( QgsMeshLayer *layer )
{
  beginResetModel();
  if ( layer )
    mRootItem = layer->datasetGroupTreeRootItem();
  endResetModel();
}

int QgsMeshDatasetGroupListModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  if ( mRootItem )
    return mRootItem->enabledDatasetGroupIndexes().count();
  else
    return 0;
}

QVariant QgsMeshDatasetGroupListModel::data( const QModelIndex &index, int role ) const
{
  if ( !mRootItem || ! index.isValid() )
    return QVariant();

  if ( index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  const QList<int> list = mRootItem->enabledDatasetGroupIndexes();
  if ( index.row() >= list.count() )
    return QVariant();

  QgsMeshDatasetGroupTreeItem *item = mRootItem->childFromDatasetGroupIndex( list.at( index.row() ) );

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
    case Qt::DecorationRole:
      return QVariant();
      break;
  }

  return QVariant();
}

void QgsMeshDatasetGroupListModel::setDisplayProviderName( bool displayProviderName )
{
  mDisplayProviderName = displayProviderName;
}

QStringList QgsMeshDatasetGroupListModel::variableNames() const
{
  const int varCount = rowCount( QModelIndex() );
  QStringList variableNames;
  for ( int i = 0; i < varCount; ++i )
    variableNames.append( data( createIndex( i, 0 ), Qt::DisplayRole ).toString() );

  return variableNames;
}

QgsMeshDatasetGroupTreeView::QgsMeshDatasetGroupTreeView( QWidget *parent ):
  QTreeView( parent )
  , mModel( new QgsMeshAvailableDatasetGroupTreeModel( this ) )
  , mSaveMenu( new QgsMeshDatasetGroupSaveMenu( this ) )
{
  // To avoid the theme style overrides the background defined by the model
  setStyleSheet( "QgsMeshDatasetGroupTreeView::item {background:none}" );

  setModel( mModel );
  setSelectionMode( QAbstractItemView::SingleSelection );
  header()->setSectionResizeMode( QHeaderView::ResizeToContents );

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
  const QModelIndex idx = indexAt( event->pos() );
  if ( !idx.isValid() )
    setCurrentIndex( QModelIndex() );

  std::unique_ptr<QMenu> menu( createContextMenu() );
  if ( menu && menu->actions().count() != 0 )
    menu->exec( mapToGlobal( event->pos() ) );
}

void QgsMeshDatasetGroupTreeView::removeCurrentItem()
{
  QgsMeshDatasetGroupTreeItem *item = mModel->datasetGroupTreeItem( currentIndex() );

  if ( item )
  {
    const QList<int> dependencies = item->groupIndexDependencies();
    if ( !dependencies.isEmpty() )
    {
      QString varList;
      for ( const int dependentGroupIndex : dependencies )
      {
        QgsMeshDatasetGroupTreeItem *item = mModel->datasetGroupTreeItem( dependentGroupIndex );
        if ( item )
        {
          varList.append( item->name() );
          varList.append( QStringLiteral( "\n" ) );
        }
      }
      QMessageBox::information( this, tr( "Remove Dataset Group" ), tr( "This dataset group can't be removed because other dataset groups depend on it:\n%1" )
                                .arg( varList ) );
      return;
    }
  }

  if ( QMessageBox::question( this, tr( "Remove Dataset Group" ), tr( "Remove dataset group?" ) ) == QMessageBox::Yes )
    mModel->removeItem( currentIndex() );
}

void QgsMeshDatasetGroupTreeView::onDatasetGroupSaved( const QString &uri )
{
  mModel->setPersistentDatasetGroup( currentIndex(), uri );
  emit apply();
}

QMenu *QgsMeshDatasetGroupTreeView::createContextMenu()
{
  QMenu *contextMenu = new QMenu;

  const QModelIndex &index = currentIndex();
  if ( !index.isValid() )
    return nullptr;

  const int groupIndex = mModel->data( index, QgsMeshDatasetGroupTreeModel::DatasetGroupIndex ).toInt();
  QgsMeshDatasetGroupTreeItem *item = mModel->datasetGroupTreeItem( groupIndex );

  if ( !item )
    return nullptr;

  switch ( item->datasetGroupType() )
  {
    case QgsMeshDatasetGroup::None:
      break;
    case QgsMeshDatasetGroup::Persistent:
      break;
    case QgsMeshDatasetGroup::Memory:
    case QgsMeshDatasetGroup::Virtual:
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
  const QgsMeshDatasetGroupMetadata groupMeta = mMeshLayer->datasetGroupMetadata( groupIndex );

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( mMeshLayer->dataProvider()->name() );
  if ( providerMetadata )
  {
    const QList<QgsMeshDriverMetadata> allDrivers = providerMetadata->meshDriversMetadata();
    for ( const QgsMeshDriverMetadata &driver : allDrivers )
    {
      const QString driverName = driver.name();
      const QString suffix = driver.writeDatasetOnFileSuffix();
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
  const QString exportFileDir = settings.value( QStringLiteral( "lastMeshDatasetDir" ), QDir::homePath(), QgsSettings::App ).toString();
  const QString saveFileName = QFileDialog::getSaveFileName( nullptr,
                               QObject::tr( "Save Mesh Datasets" ),
                               exportFileDir,
                               filter );

  if ( saveFileName.isEmpty() )
    return;

  const QFileInfo openFileInfo( saveFileName );
  settings.setValue( QStringLiteral( "lastMeshDatasetDir" ), openFileInfo.absolutePath(), QgsSettings::App );


  if ( mMeshLayer->saveDataset( saveFileName, datasetGroup, driver ) )
  {
    QMessageBox::warning( nullptr, QObject::tr( "Save Mesh Datasets" ), QObject::tr( "Saving datasets fails" ) );
  }
  else
  {
    emit datasetGroupSaved( saveFileName );
    QMessageBox::information( nullptr, QObject::tr( "Save Mesh Datasets" ), QObject::tr( "Datasets successfully saved on file" ) );
  }

}

QgsMeshAvailableDatasetGroupTreeModel::QgsMeshAvailableDatasetGroupTreeModel( QObject *parent ): QgsMeshDatasetGroupTreeModel( parent )
{}

QVariant QgsMeshAvailableDatasetGroupTreeModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Name:
      return textDisplayed( index );

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    case Qt::BackgroundColorRole:
#else
    case Qt::BackgroundRole:
#endif
      return backGroundColor( index );
  }
  return QgsMeshDatasetGroupTreeModel::data( index, role );
}

bool QgsMeshAvailableDatasetGroupTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return false;

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

Qt::ItemFlags QgsMeshAvailableDatasetGroupTreeModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::NoItemFlags;

  if ( index.column() > 0 )
    return Qt::ItemIsEnabled;

  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

QVariant QgsMeshAvailableDatasetGroupTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )

  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    if ( section == 0 )
      return tr( "Group" );
    if ( section == 1 )
      return  tr( "Type" );
    if ( section == 2 )
      return  tr( "Description" );
  }

  return QVariant();
}

int QgsMeshAvailableDatasetGroupTreeModel::columnCount( const QModelIndex &parent ) const {Q_UNUSED( parent ); return 3;}

QString QgsMeshAvailableDatasetGroupTreeModel::textDisplayed( const QModelIndex &index ) const
{
  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return QString();

  switch ( index.column() )
  {
    case 0:
      return item->name();
    case 1:
      if ( item->isVector() )
        return tr( "Vector" );
      else
        return tr( "Scalar" );
    case 2 :
      return item->description();
  }
  return QString();
}

QColor QgsMeshAvailableDatasetGroupTreeModel::backGroundColor( const QModelIndex &index ) const
{
  QgsMeshDatasetGroupTreeItem *item = static_cast<QgsMeshDatasetGroupTreeItem *>( index.internalPointer() );
  if ( !item )
    return QColor();

  if ( item->datasetGroupType() == QgsMeshDatasetGroup::Virtual )
    return QColor( 103, 0, 243, 44 );
  else if ( item->datasetGroupType() == QgsMeshDatasetGroup::Memory )
    return QColor( 252, 155, 79, 44 );
  else
    return QColor( 252, 255, 79, 44 );
}
