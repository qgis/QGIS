/***************************************************************************
    qgsbrowsermodel.cpp
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDir>
#include <QApplication>
#include <QStyle>
#include <QtConcurrentMap>
#include <QUrl>
#include <QStorageInfo>
#include <QFuture>
#include <QFutureWatcher>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdataprovider.h"
#include "qgsmimedatautils.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsbrowsermodel.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsdirectoryitem.h"
#include "qgsprojectitem.h"
#include "qgslayeritem.h"
#include "qgsfavoritesitem.h"
#include "qgslayermetadata.h"

#define PROJECT_HOME_PREFIX "project:"
#define HOME_PREFIX "home:"

/// @cond PRIVATE
class QgsBrowserWatcher : public QFutureWatcher<QVector <QgsDataItem *> >
{
    Q_OBJECT

  public:
    QgsBrowserWatcher( QgsDataItem *item )
      : QFutureWatcher( nullptr )
      , mItem( item )
    {
    }

    QgsDataItem *item() const { return mItem; }

  signals:
    void finished( QgsDataItem *item, const QVector <QgsDataItem *> &items );

  private:
    QgsDataItem *mItem = nullptr;
};
///@endcond

// sort function for QList<QgsDataItem*>, e.g. sorted/grouped provider listings
static bool cmpByDataItemName_( QgsDataItem *a, QgsDataItem *b )
{
  return QString::localeAwareCompare( a->name(), b->name() ) < 0;
}

QgsBrowserModel::QgsBrowserModel( QObject *parent )
  : QAbstractItemModel( parent )

{
  connect( QgsApplication::dataItemProviderRegistry(), &QgsDataItemProviderRegistry::providerAdded,
           this, &QgsBrowserModel::dataItemProviderAdded );
  connect( QgsApplication::dataItemProviderRegistry(), &QgsDataItemProviderRegistry::providerWillBeRemoved,
           this, &QgsBrowserModel::dataItemProviderWillBeRemoved );
}

QgsBrowserModel::~QgsBrowserModel()
{
  removeRootItems();
}

void QgsBrowserModel::updateProjectHome()
{
  QString home = QgsProject::instance()->homePath();
  if ( mProjectHome && mProjectHome->path().mid( QStringLiteral( PROJECT_HOME_PREFIX ).length() ) == home )
    return;

  int idx = mRootItems.indexOf( mProjectHome );

  // using layoutAboutToBeChanged() was messing expanded items
  if ( idx >= 0 )
  {
    beginRemoveRows( QModelIndex(), idx, idx );
    mRootItems.remove( idx );
    endRemoveRows();
  }
  delete mProjectHome;
  mProjectHome = home.isNull() ? nullptr : new QgsProjectHomeItem( nullptr, tr( "Project Home" ), home, QStringLiteral( PROJECT_HOME_PREFIX ) + home );
  if ( mProjectHome )
  {
    setupItemConnections( mProjectHome );

    beginInsertRows( QModelIndex(), 0, 0 );
    mRootItems.insert( 0, mProjectHome );
    endInsertRows();
  }
}

void QgsBrowserModel::addRootItems()
{
  updateProjectHome();

  // give the home directory a prominent third place
  QgsDirectoryItem *item = new QgsDirectoryItem( nullptr, tr( "Home" ), QDir::homePath(),
      QStringLiteral( HOME_PREFIX ) + QDir::homePath(),
      QStringLiteral( "special:Home" ) );
  item->setSortKey( QStringLiteral( " 2" ) );
  setupItemConnections( item );
  mRootItems << item;

  // add favorite directories
  mFavorites = new QgsFavoritesItem( nullptr, tr( "Favorites" ) );
  if ( mFavorites )
  {
    setupItemConnections( mFavorites );
    mRootItems << mFavorites;
  }

  // add drives
  const auto drives { QDir::drives() };
  for ( const QFileInfo &drive : drives )
  {
    const QString path = drive.absolutePath();

    if ( QgsDirectoryItem::hiddenPath( path ) )
      continue;

    const QString driveName = QStorageInfo( path ).displayName();
    const QString name = driveName.isEmpty() || driveName == path ? path : QStringLiteral( "%1 (%2)" ).arg( path, driveName );

    QgsDirectoryItem *item = new QgsDirectoryItem( nullptr, name, path, path, QStringLiteral( "special:Drives" ) );
    item->setSortKey( QStringLiteral( " 3 %1" ).arg( path ) );
    mDriveItems.insert( path, item );

    setupItemConnections( item );
    mRootItems << item;
  }

#ifdef Q_OS_MAC
  QString path = QString( "/Volumes" );
  QgsDirectoryItem *vols = new QgsDirectoryItem( nullptr, path, path, path, QStringLiteral( "special:Volumes" ) );
  setupItemConnections( vols );
  mRootItems << vols;
#endif

  // container for displaying providers as sorted groups (by QgsDataProvider::DataCapability enum)
  QMultiMap<int, QgsDataItem *> providerMap;

  const auto constProviders = QgsApplication::dataItemProviderRegistry()->providers();
  for ( QgsDataItemProvider *pr : constProviders )
  {
    if ( QgsDataItem *item = addProviderRootItem( pr ) )
    {
      providerMap.insert( pr->capabilities(), item );
    }
  }

  // add as sorted groups by QgsDataProvider::DataCapability enum
  const auto constUniqueKeys = providerMap.uniqueKeys();
  for ( int key : constUniqueKeys )
  {
    QList<QgsDataItem *> providerGroup = providerMap.values( key );
    if ( providerGroup.size() > 1 )
    {
      std::sort( providerGroup.begin(), providerGroup.end(), cmpByDataItemName_ );
    }

    const auto constProviderGroup = providerGroup;
    for ( QgsDataItem *ditem : constProviderGroup )
    {
      mRootItems << ditem;
    }
  }
}

void QgsBrowserModel::removeRootItems()
{
  const auto constMRootItems = mRootItems;
  for ( QgsDataItem *item : constMRootItems )
  {
    delete item;
  }

  mRootItems.clear();
  mDriveItems.clear();
}

void QgsBrowserModel::dataItemProviderAdded( QgsDataItemProvider *provider )
{
  if ( !mInitialized )
    return;

  if ( QgsDataItem *item = addProviderRootItem( provider ) )
  {
    beginInsertRows( QModelIndex(), rowCount(), rowCount() );
    mRootItems << item;
    endInsertRows();
  }
}

void QgsBrowserModel::dataItemProviderWillBeRemoved( QgsDataItemProvider *provider )
{
  const auto constMRootItems = mRootItems;
  for ( QgsDataItem *item : constMRootItems )
  {
    if ( item->providerKey() == provider->name() )
    {
      removeRootItem( item );
      break;  // assuming there is max. 1 root item per provider
    }
  }
}

void QgsBrowserModel::onConnectionsChanged( const QString &providerKey )
{
  // refresh the matching provider
  for ( QgsDataItem *item : std::as_const( mRootItems ) )
  {
    if ( item->providerKey() == providerKey )
    {
      item->refresh();
      break;  // assuming there is max. 1 root item per provider
    }
  }

  emit connectionsChanged( providerKey );
}

QMap<QString, QgsDirectoryItem *> QgsBrowserModel::driveItems() const
{
  return mDriveItems;
}


void QgsBrowserModel::initialize()
{
  if ( ! mInitialized )
  {
    connect( QgsProject::instance(), &QgsProject::homePathChanged, this, &QgsBrowserModel::updateProjectHome );
    addRootItems();
    mInitialized = true;
  }
}

Qt::ItemFlags QgsBrowserModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  QgsDataItem *ptr = dataItem( index );

  if ( !ptr )
  {
    QgsDebugMsgLevel( QStringLiteral( "FLAGS PROBLEM!" ), 4 );
    return Qt::ItemFlags();
  }

  if ( ptr->hasDragEnabled() )
    flags |= Qt::ItemIsDragEnabled;

  Q_NOWARN_DEPRECATED_PUSH
  if ( ptr->acceptDrop() )
    flags |= Qt::ItemIsDropEnabled;
  Q_NOWARN_DEPRECATED_POP

  if ( ( ptr->capabilities2() & Qgis::BrowserItemCapability::Rename )
       || ( ptr->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
    flags |= Qt::ItemIsEditable;

  return flags;
}

QVariant QgsBrowserModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsDataItem *item = dataItem( index );
  if ( !item )
  {
    return QVariant();
  }
  else if ( role == Qt::DisplayRole || role == Qt::EditRole )
  {
    return item->name();
  }
  else if ( role == QgsBrowserModel::SortRole )
  {
    return item->sortKey();
  }
  else if ( role == Qt::ToolTipRole )
  {
    return item->toolTip();
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    return item->icon();
  }
  else if ( role == QgsBrowserModel::PathRole )
  {
    return item->path();
  }
  else if ( role == QgsBrowserModel::CommentRole )
  {
    if ( item->type() == Qgis::BrowserItemType::Layer )
    {
      QgsLayerItem *lyrItem = qobject_cast<QgsLayerItem *>( item );
      return lyrItem->comments();
    }
    return QVariant();
  }
  else if ( role == QgsBrowserModel::LayerMetadataRole )
  {
    if ( item->type() == Qgis::BrowserItemType::Layer )
    {
      QgsLayerItem *lyrItem = qobject_cast<QgsLayerItem *>( item );
      return QVariant::fromValue( lyrItem->layerMetadata() );
    }
    return QVariant();
  }
  else if ( role == QgsBrowserModel::ProviderKeyRole )
  {
    return item->providerKey();
  }
  else
  {
    // unsupported role
    return QVariant();
  }
}

bool QgsBrowserModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;


  QgsDataItem *item = dataItem( index );
  if ( !item )
  {
    return false;
  }

  if ( !( item->capabilities2() & Qgis::BrowserItemCapability::Rename )
       && !( item->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile ) )
    return false;

  switch ( role )
  {
    case Qt::EditRole:
    {
      Q_NOWARN_DEPRECATED_PUSH
      return item->rename( value.toString() );
      Q_NOWARN_DEPRECATED_POP
    }
  }
  return false;
}

QVariant QgsBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section )
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    return QVariant( "header" );
  }

  return QVariant();
}

int QgsBrowserModel::rowCount( const QModelIndex &parent ) const
{
  //QgsDebugMsg(QString("isValid = %1 row = %2 column = %3").arg(parent.isValid()).arg(parent.row()).arg(parent.column()));

  if ( !parent.isValid() )
  {
    // root item: its children are top level items
    return mRootItems.count(); // mRoot
  }
  else
  {
    // ordinary item: number of its children
    QgsDataItem *item = dataItem( parent );
    //if ( item ) QgsDebugMsg(QString("path = %1 rowCount = %2").arg(item->path()).arg(item->rowCount()) );
    return item ? item->rowCount() : 0;
  }
}

bool QgsBrowserModel::hasChildren( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return !mRootItems.isEmpty(); // root item: its children are top level items

  QgsDataItem *item = dataItem( parent );
  return item && item->hasChildren();
}

int QgsBrowserModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QModelIndex QgsBrowserModel::findPath( const QString &path, Qt::MatchFlag matchFlag )
{
  return findPath( this, path, matchFlag );
}

QModelIndex QgsBrowserModel::findPath( QAbstractItemModel *model, const QString &path, Qt::MatchFlag matchFlag )
{
  if ( !model )
    return QModelIndex();

  QModelIndex index; // starting from root
  bool foundChild = true;

  while ( foundChild )
  {
    foundChild = false; // assume that the next child item will not be found

    for ( int i = 0; i < model->rowCount( index ); i++ )
    {
      QModelIndex idx = model->index( i, 0, index );

      QString itemPath = model->data( idx, PathRole ).toString();
      if ( itemPath == path )
      {
        QgsDebugMsgLevel( "Arrived " + itemPath, 4 );
        return idx; // we have found the item we have been looking for
      }

      // paths are slash separated identifier
      if ( path.startsWith( itemPath + '/' ) )
      {
        foundChild = true;
        index = idx;
        break;
      }
    }
  }

  if ( matchFlag == Qt::MatchStartsWith )
    return index;

  QgsDebugMsgLevel( QStringLiteral( "path not found" ), 4 );
  return QModelIndex(); // not found
}

QModelIndex QgsBrowserModel::findUri( const QString &uri, QModelIndex index )
{
  for ( int i = 0; i < this->rowCount( index ); i++ )
  {
    QModelIndex idx = this->index( i, 0, index );

    if ( qobject_cast<QgsLayerItem *>( dataItem( idx ) ) )
    {
      QString itemUri = qobject_cast<QgsLayerItem *>( dataItem( idx ) )->uri();

      if ( itemUri == uri )
      {
        QgsDebugMsgLevel( "Arrived " + itemUri, 4 );
        return idx; // we have found the item we have been looking for
      }
    }

    QModelIndex childIdx = findUri( uri, idx );
    if ( childIdx.isValid() )
      return childIdx;
  }
  return QModelIndex();
}

void QgsBrowserModel::connectItem( QgsDataItem * )
{
  // deprecated, no use
}

void QgsBrowserModel::reload()
{
  // TODO: put items creating currently children in threads to deleteLater (does not seem urget because reload() is not used in QGIS)
  beginResetModel();
  removeRootItems();
  addRootItems();
  endResetModel();
}

void QgsBrowserModel::refreshDrives()
{
  const QList< QFileInfo > drives = QDir::drives();
  // remove any removed drives
  const QStringList existingDrives = mDriveItems.keys();
  for ( const QString &drivePath : existingDrives )
  {
    bool stillExists = false;
    for ( const QFileInfo &drive : drives )
    {
      if ( drivePath == drive.absolutePath() )
      {
        stillExists = true;
        break;
      }
    }

    if ( stillExists )
      continue;

    // drive has been removed, remove corresponding item
    if ( QgsDirectoryItem *driveItem = mDriveItems.value( drivePath ) )
      removeRootItem( driveItem );
  }

  for ( const QFileInfo &drive : drives )
  {
    const QString path = drive.absolutePath();

    if ( QgsDirectoryItem::hiddenPath( path ) )
      continue;

    // does an item for this drive already exist?
    if ( !mDriveItems.contains( path ) )
    {
      const QString driveName = QStorageInfo( path ).displayName();
      const QString name = driveName.isEmpty() || driveName == path ? path : QStringLiteral( "%1 (%2)" ).arg( path, driveName );

      QgsDirectoryItem *item = new QgsDirectoryItem( nullptr, name, path, path, QStringLiteral( "special:Drives" ) );
      item->setSortKey( QStringLiteral( " 3 %1" ).arg( path ) );

      mDriveItems.insert( path, item );
      setupItemConnections( item );

      beginInsertRows( QModelIndex(), mRootItems.count(), mRootItems.count() );
      mRootItems << item;
      endInsertRows();
    }
  }
}

QModelIndex QgsBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( column < 0 || column >= columnCount() || row < 0 )
    return QModelIndex();

  QgsDataItem *p = dataItem( parent );
  const QVector<QgsDataItem *> &items = p ? p->children() : mRootItems;
  QgsDataItem *item = items.value( row, nullptr );
  return item ? createIndex( row, column, item ) : QModelIndex();
}

QModelIndex QgsBrowserModel::parent( const QModelIndex &index ) const
{
  QgsDataItem *item = dataItem( index );
  if ( !item )
    return QModelIndex();

  return findItem( item->parent() );
}

QModelIndex QgsBrowserModel::findItem( QgsDataItem *item, QgsDataItem *parent ) const
{
  const QVector<QgsDataItem *> &items = parent ? parent->children() : mRootItems;

  for ( int i = 0; i < items.size(); i++ )
  {
    if ( items[i] == item )
      return createIndex( i, 0, item );

    QModelIndex childIndex = findItem( item, items[i] );
    if ( childIndex.isValid() )
      return childIndex;
  }
  return QModelIndex();
}

void QgsBrowserModel::beginInsertItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsgLevel( "parent mPath = " + parent->path(), 3 );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsgLevel( QStringLiteral( "valid" ), 3 );
  beginInsertRows( idx, first, last );
  QgsDebugMsgLevel( QStringLiteral( "end" ), 3 );
}
void QgsBrowserModel::endInsertItems()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 3 );
  endInsertRows();
}
void QgsBrowserModel::beginRemoveItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsgLevel( "parent mPath = " + parent->path(), 3 );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsBrowserModel::endRemoveItems()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 3 );
  endRemoveRows();
}
void QgsBrowserModel::itemDataChanged( QgsDataItem *item )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 3 );
  QModelIndex idx = findItem( item );
  if ( !idx.isValid() )
    return;
  emit dataChanged( idx, idx );
}
void QgsBrowserModel::itemStateChanged( QgsDataItem *item, Qgis::BrowserItemState oldState )
{
  if ( !item )
    return;
  QModelIndex idx = findItem( item );
  if ( !idx.isValid() )
    return;
  QgsDebugMsgLevel( QStringLiteral( "item %1 state changed %2 -> %3" ).arg( item->path() ).arg( qgsEnumValueToKey< Qgis::BrowserItemState >( oldState ) ).arg( qgsEnumValueToKey< Qgis::BrowserItemState >( item->state() ) ), 4 );
  emit stateChanged( idx, oldState );
}

void QgsBrowserModel::setupItemConnections( QgsDataItem *item )
{
  connect( item, &QgsDataItem::beginInsertItems,
           this, &QgsBrowserModel::beginInsertItems );
  connect( item, &QgsDataItem::endInsertItems,
           this, &QgsBrowserModel::endInsertItems );
  connect( item, &QgsDataItem::beginRemoveItems,
           this, &QgsBrowserModel::beginRemoveItems );
  connect( item, &QgsDataItem::endRemoveItems,
           this, &QgsBrowserModel::endRemoveItems );
  connect( item, &QgsDataItem::dataChanged,
           this, &QgsBrowserModel::itemDataChanged );
  connect( item, &QgsDataItem::stateChanged,
           this, &QgsBrowserModel::itemStateChanged );

  // if it's a collection item, also forwards connectionsChanged
  QgsDataCollectionItem *collectionItem = qobject_cast<QgsDataCollectionItem *>( item );
  if ( collectionItem )
    connect( collectionItem, &QgsDataCollectionItem::connectionsChanged, this, &QgsBrowserModel::onConnectionsChanged );
}

QStringList QgsBrowserModel::mimeTypes() const
{
  QStringList types;
  // In theory the mime type convention is: application/x-vnd.<vendor>.<application>.<type>
  // but it seems a bit over formalized. Would be an application/x-qgis-uri better?
  types << QStringLiteral( "application/x-vnd.qgis.qgis.uri" );
  return types;
}

QMimeData *QgsBrowserModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsMimeDataUtils::UriList lst;
  const auto constIndexes = indexes;
  for ( const QModelIndex &index : constIndexes )
  {
    if ( index.isValid() )
    {
      QgsDataItem *ptr = reinterpret_cast< QgsDataItem * >( index.internalPointer() );
      const QgsMimeDataUtils::UriList uris = ptr->mimeUris();
      for ( QgsMimeDataUtils::Uri uri : std::as_const( uris ) )
      {
        if ( ptr->capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
        {
          uri.filePath = ptr->path();
        }
        lst.append( uri );
      }
    }
  }
  return QgsMimeDataUtils::encodeUriList( lst );
}

bool QgsBrowserModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &parent )
{
  QgsDataItem *destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsgLevel( QStringLiteral( "DROP PROBLEM!" ), 4 );
    return false;
  }

  Q_NOWARN_DEPRECATED_PUSH
  return destItem->handleDrop( data, action );
  Q_NOWARN_DEPRECATED_POP
}

QgsDataItem *QgsBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsDataItem *d = reinterpret_cast<QgsDataItem *>( v );
  Q_ASSERT( !v || d );
  return d;
}

bool QgsBrowserModel::canFetchMore( const QModelIndex &parent ) const
{
  QgsDataItem *item = dataItem( parent );
  // if ( item )
  //   QgsDebugMsg( QStringLiteral( "path = %1 canFetchMore = %2" ).arg( item->path() ).arg( item && ! item->isPopulated() ) );
  return ( item && item->state() == Qgis::BrowserItemState::NotPopulated );
}

void QgsBrowserModel::fetchMore( const QModelIndex &parent )
{
  QgsDataItem *item = dataItem( parent );

  if ( !item || item->state() == Qgis::BrowserItemState::Populating || item->state() == Qgis::BrowserItemState::Populated )
    return;

  QgsDebugMsgLevel( "path = " + item->path(), 4 );

  item->populate();
}

/* Refresh dir path */
void QgsBrowserModel::refresh( const QString &path )
{
  QModelIndex index = findPath( path );
  refresh( index );
}

/* Refresh item */
void QgsBrowserModel::refresh( const QModelIndex &index )
{
  QgsDataItem *item = dataItem( index );
  if ( !item || item->state() == Qgis::BrowserItemState::Populating )
    return;

  QgsDebugMsgLevel( "Refresh " + item->path(), 4 );

  item->refresh();
}

void QgsBrowserModel::addFavoriteDirectory( const QString &directory, const QString &name )
{
  Q_ASSERT( mFavorites );
  mFavorites->addDirectory( directory, name );
}

void QgsBrowserModel::removeFavorite( const QModelIndex &index )
{
  QgsDirectoryItem *item = qobject_cast<QgsDirectoryItem *>( dataItem( index ) );
  if ( !item )
    return;

  mFavorites->removeDirectory( item );
}

void QgsBrowserModel::removeFavorite( QgsFavoriteItem *favorite )
{
  if ( !favorite )
    return;

  mFavorites->removeDirectory( favorite );
}

void QgsBrowserModel::hidePath( QgsDataItem *item )
{
  QgsSettings settings;
  QStringList hiddenItems = settings.value( QStringLiteral( "browser/hiddenPaths" ),
                            QStringList() ).toStringList();
  int idx = hiddenItems.indexOf( item->path() );
  if ( idx != -1 )
  {
    hiddenItems.removeAt( idx );
  }
  else
  {
    hiddenItems << item->path();
  }
  settings.setValue( QStringLiteral( "browser/hiddenPaths" ), hiddenItems );
  if ( item->parent() )
  {
    item->parent()->deleteChildItem( item );
  }
  else
  {
    removeRootItem( item );
  }
}


void QgsBrowserModel::removeRootItem( QgsDataItem *item )
{
  int i = mRootItems.indexOf( item );
  beginRemoveRows( QModelIndex(), i, i );
  mRootItems.remove( i );
  QgsDirectoryItem *dirItem = qobject_cast< QgsDirectoryItem * >( item );
  if ( !mDriveItems.key( dirItem ).isEmpty() )
  {
    mDriveItems.remove( mDriveItems.key( dirItem ) );
  }
  item->deleteLater();
  endRemoveRows();
}

QgsDataItem *QgsBrowserModel::addProviderRootItem( QgsDataItemProvider *pr )
{
  int capabilities = pr->capabilities();
  if ( capabilities == QgsDataProvider::NoDataCapabilities )
  {
    QgsDebugMsgLevel( pr->name() + " does not have any dataCapabilities", 4 );
    return nullptr;
  }

  QgsDataItem *item = pr->createDataItem( QString(), nullptr );  // empty path -> top level
  if ( item )
  {
    // make sure the top level key is always set
    item->setProviderKey( pr->name() );
    // Forward the signal from the root items to the model (and then to the app)
    connect( item, &QgsDataItem::connectionsChanged, this, &QgsBrowserModel::onConnectionsChanged );
    QgsDebugMsgLevel( "Add new top level item : " + item->name(), 4 );
    setupItemConnections( item );
  }
  return item;
}

// For QgsBrowserWatcher
#include "qgsbrowsermodel.moc"
