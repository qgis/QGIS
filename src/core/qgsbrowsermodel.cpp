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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsmimedatautils.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"

#include "qgsbrowsermodel.h"
#include "qgsproject.h"

#include <QSettings>

QgsBrowserWatcher::QgsBrowserWatcher( QgsDataItem * item )
    : mItem( item )
{
}

QgsBrowserWatcher::~QgsBrowserWatcher()
{
}

// sort function for QList<QgsDataItem*>, e.g. sorted/grouped provider listings
static bool cmpByDataItemName_( QgsDataItem* a, QgsDataItem* b )
{
  return QString::localeAwareCompare( a->name(), b->name() ) < 0;
}

QgsBrowserModel::QgsBrowserModel( QObject *parent )
    : QAbstractItemModel( parent )
    , mFavourites( 0 )
    , mProjectHome( 0 )
{
  connect( QgsProject::instance(), SIGNAL( readProject( const QDomDocument & ) ), this, SLOT( updateProjectHome() ) );
  connect( QgsProject::instance(), SIGNAL( writeProject( QDomDocument & ) ), this, SLOT( updateProjectHome() ) );
  mLoadingMovie.setFileName( QgsApplication::iconPath( "/mIconLoading.gif" ) );
  mLoadingMovie.setCacheMode( QMovie::CacheAll );
  connect( &mLoadingMovie, SIGNAL( frameChanged( int ) ), SLOT( loadingFrameChanged() ) );
  addRootItems();
}

QgsBrowserModel::~QgsBrowserModel()
{
  removeRootItems();
}

void QgsBrowserModel::updateProjectHome()
{
  QString home = QgsProject::instance()->homePath();
  if ( mProjectHome && mProjectHome->path() == home )
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
  mProjectHome = home.isNull() ? 0 : new QgsDirectoryItem( NULL, tr( "Project home" ), home, "project:" + home );
  if ( mProjectHome )
  {
    connectItem( mProjectHome );

    beginInsertRows( QModelIndex(), 0, 0 );
    mRootItems.insert( 0, mProjectHome );
    endInsertRows();
  }
}

void QgsBrowserModel::addRootItems()
{
  updateProjectHome();

  // give the home directory a prominent second place
  QgsDirectoryItem *item = new QgsDirectoryItem( NULL, tr( "Home" ), QDir::homePath(), "home:" + QDir::homePath() );
  QStyle *style = QApplication::style();
  QIcon homeIcon( style->standardPixmap( QStyle::SP_DirHomeIcon ) );
  item->setIcon( homeIcon );
  connectItem( item );
  mRootItems << item;

  // add favourite directories
  mFavourites = new QgsFavouritesItem( NULL, tr( "Favourites" ) );
  if ( mFavourites )
  {
    connectItem( mFavourites );
    mRootItems << mFavourites;
  }

  // add drives
  foreach ( QFileInfo drive, QDir::drives() )
  {
    QString path = drive.absolutePath();
    QgsDirectoryItem *item = new QgsDirectoryItem( NULL, path, path );

    connectItem( item );
    mRootItems << item;
  }

#ifdef Q_WS_MAC
  QString path = QString( "/Volumes" );
  QgsDirectoryItem *vols = new QgsDirectoryItem( NULL, path, path );
  connectItem( vols );
  mRootItems << vols;
#endif

  // Add non file top level items
  QStringList providersList = QgsProviderRegistry::instance()->providerList();

  // container for displaying providers as sorted groups (by QgsDataProvider::DataCapability enum)
  QMap<int, QgsDataItem *> providerMap;

  foreach ( QString key, providersList )
  {
    QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( key );
    if ( !library )
      continue;

    dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
    if ( !dataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
      continue;
    }

    int capabilities = dataCapabilities();
    if ( capabilities == QgsDataProvider::NoDataCapabilities )
    {
      QgsDebugMsg( library->fileName() + " does not have any dataCapabilities" );
      continue;
    }

    dataItem_t *dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dataItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    QgsDataItem *item = dataItem( "", NULL );  // empty path -> top level
    if ( item )
    {
      QgsDebugMsg( "Add new top level item : " + item->name() );
      connectItem( item );
      providerMap.insertMulti( capabilities, item );
    }
  }

  // add as sorted groups by QgsDataProvider::DataCapability enum
  foreach ( int key, providerMap.uniqueKeys() )
  {
    QList<QgsDataItem *> providerGroup = providerMap.values( key );
    if ( providerGroup.size() > 1 )
    {
      qSort( providerGroup.begin(), providerGroup.end(), cmpByDataItemName_ );
    }

    foreach ( QgsDataItem * ditem, providerGroup )
    {
      mRootItems << ditem;
    }
  }
}

void QgsBrowserModel::removeRootItems()
{
  foreach ( QgsDataItem* item, mRootItems )
  {
    delete item;
  }

  mRootItems.clear();
}


Qt::ItemFlags QgsBrowserModel::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() )
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  QgsDataItem* ptr = ( QgsDataItem* ) index.internalPointer();
  if ( ptr->type() == QgsDataItem::Layer )
  {
    flags |= Qt::ItemIsDragEnabled;
  }
  if ( ptr->acceptDrop() )
    flags |= Qt::ItemIsDropEnabled;
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
  else if ( role == Qt::DisplayRole )
  {
    return item->name();
  }
  else if ( role == Qt::ToolTipRole )
  {
    return item->toolTip();
  }
  else if ( role == Qt::DecorationRole && index.column() == 0 )
  {
    if ( fetching( item ) )
    {
      return mLoadingIcon;
    }
    return item->icon();
  }
  else
  {
    // unsupported role
    return QVariant();
  }
}

QVariant QgsBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section );
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
    return true; // root item: its children are top level items

  QgsDataItem *item = dataItem( parent );
  return item && item->hasChildren();
}

int QgsBrowserModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 1;
}

QModelIndex QgsBrowserModel::findPath( QString path, Qt::MatchFlag matchFlag )
{
  QModelIndex theIndex; // starting from root
  bool foundChild = true;

  while ( foundChild )
  {
    foundChild = false; // assume that the next child item will not be found

    for ( int i = 0; i < rowCount( theIndex ); i++ )
    {
      QModelIndex idx = index( i, 0, theIndex );
      QgsDataItem *item = dataItem( idx );
      if ( !item )
        return QModelIndex(); // an error occurred

      if ( item->path() == path )
      {
        QgsDebugMsg( "Arrived " + item->path() );
        return idx; // we have found the item we have been looking for
      }

      // paths are slash separated identifier
      if ( path.startsWith( item->path() + "/" ) )
      {
        foundChild = true;
        theIndex = idx;
        break;
      }
    }
  }

  if ( matchFlag == Qt::MatchStartsWith )
    return theIndex;

  QgsDebugMsg( "path not found" );
  return QModelIndex(); // not found
}

void QgsBrowserModel::reload()
{
  beginResetModel();
  removeRootItems();
  addRootItems();
  endResetModel();
}

QModelIndex QgsBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsDataItem *p = dataItem( parent );
  const QVector<QgsDataItem*> &items = p ? p->children() : mRootItems;
  QgsDataItem *item = items.value( row, 0 );
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
  const QVector<QgsDataItem*> &items = parent ? parent->children() : mRootItems;

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
  QgsDebugMsgLevel( "valid", 3 );
  beginInsertRows( idx, first, last );
  QgsDebugMsgLevel( "end", 3 );
}
void QgsBrowserModel::endInsertItems()
{
  QgsDebugMsgLevel( "Entered", 3 );
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
  QgsDebugMsgLevel( "Entered", 3 );
  endRemoveRows();
}
void QgsBrowserModel::connectItem( QgsDataItem* item )
{
  connect( item, SIGNAL( beginInsertItems( QgsDataItem*, int, int ) ),
           this, SLOT( beginInsertItems( QgsDataItem*, int, int ) ) );
  connect( item, SIGNAL( endInsertItems() ),
           this, SLOT( endInsertItems() ) );
  connect( item, SIGNAL( beginRemoveItems( QgsDataItem*, int, int ) ),
           this, SLOT( beginRemoveItems( QgsDataItem*, int, int ) ) );
  connect( item, SIGNAL( endRemoveItems() ),
           this, SLOT( endRemoveItems() ) );
}

QStringList QgsBrowserModel::mimeTypes() const
{
  QStringList types;
  // In theory the mime type convention is: application/x-vnd.<vendor>.<application>.<type>
  // but it seems a bit over formalized. Would be an application/x-qgis-uri better?
  types << "application/x-vnd.qgis.qgis.uri";
  return types;
}

QMimeData * QgsBrowserModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsMimeDataUtils::UriList lst;
  foreach ( const QModelIndex &index, indexes )
  {
    if ( index.isValid() )
    {
      QgsDataItem* ptr = ( QgsDataItem* ) index.internalPointer();
      if ( ptr->type() != QgsDataItem::Layer ) continue;
      QgsLayerItem *layer = ( QgsLayerItem* ) ptr;
      lst.append( QgsMimeDataUtils::Uri( layer ) );
    }
  }
  return QgsMimeDataUtils::encodeUriList( lst );
}

bool QgsBrowserModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  QgsDataItem* destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsg( "DROP PROBLEM!" );
    return false;
  }

  return destItem->handleDrop( data, action );
}

QgsDataItem *QgsBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsDataItem *d = reinterpret_cast<QgsDataItem*>( v );
  Q_ASSERT( !v || d );
  return d;
}

bool QgsBrowserModel::canFetchMore( const QModelIndex & parent ) const
{
  QgsDataItem* item = dataItem( parent );
  // if ( item )
  //   QgsDebugMsg( QString( "path = %1 canFetchMore = %2" ).arg( item->path() ).arg( item && ! item->isPopulated() ) );
  return ( item && ! item->isPopulated() );
}

void QgsBrowserModel::fetchMore( const QModelIndex & parent )
{
  QgsDebugMsg( "Entered" );
  QgsDataItem* item = dataItem( parent );

  if ( !item || fetching( item ) )
    return;

  QgsDebugMsg( "path = " + item->path() );

  if ( item->isPopulated() )
    return;

  QList<QgsDataItem*> itemList;
  itemList << item;
  QgsBrowserWatcher * watcher = new QgsBrowserWatcher( item );
  connect( watcher, SIGNAL( finished() ), SLOT( childrenCreated() ) );
  watcher->setFuture( QtConcurrent::mapped( itemList, QgsBrowserModel::createChildren ) );
  mWatchers.append( watcher );
  mLoadingMovie.setPaused( false );
  emit dataChanged( parent, parent );
}

/* Refresh dir path */
void QgsBrowserModel::refresh( QString path )
{
  QModelIndex index = findPath( path );
  refresh( index );
}

/* Refresh item */
void QgsBrowserModel::refresh( const QModelIndex& theIndex )
{
  QgsDataItem *item = dataItem( theIndex );
  if ( !item )
    return;

  QgsDebugMsg( "Refresh " + item->path() );

  QList<QgsDataItem*> itemList;
  itemList << item;
  QgsBrowserWatcher * watcher = new QgsBrowserWatcher( item );
  connect( watcher, SIGNAL( finished() ), SLOT( refreshChildrenCreated() ) );
  watcher->setFuture( QtConcurrent::mapped( itemList, QgsBrowserModel::createChildren ) );
  mWatchers.append( watcher );
  mLoadingMovie.setPaused( false );
  emit dataChanged( theIndex, theIndex );
}

// This is expected to be run in a separate thread
QVector<QgsDataItem*> QgsBrowserModel::createChildren( QgsDataItem* item )
{
  QgsDebugMsg( "Entered" );
  QTime time;
  time.start();
  QVector <QgsDataItem*> children = item->createChildren();
  QgsDebugMsg( QString( "%1 children created in %2 ms" ).arg( children.size() ).arg( time.elapsed() ) );
  // Children objects must be pushed to main thread.
  foreach ( QgsDataItem* child, children )
  {
    if ( !child ) // should not happen
      continue;
    // However it seems to work without resetting parent, the Qt doc says that
    // "The object cannot be moved if it has a parent."
    QgsDebugMsg( "moveToThread child" + child->path() );
    child->setParent( 0 );
    child->moveToThread( QApplication::instance()->thread() );
    child->setParent( item );
  }
  return children;
}

void QgsBrowserModel::childrenCreated()
{
  QgsBrowserWatcher *watcher = dynamic_cast<QgsBrowserWatcher *>( sender() );
  if ( !watcher )
    return;
  QgsDataItem* item = watcher->item();
  QVector <QgsDataItem*> children = watcher->result();
  QgsDebugMsg( QString( "path = %1 children.size() = %2" ).arg( item->path() ).arg( children.size() ) );
  QModelIndex index = findItem( item );
  if ( !index.isValid() ) // check if item still exists
    return;
  item->populate( children );
  emit dataChanged( index, index );
  emit fetchFinished( index );
}

void QgsBrowserModel::refreshChildrenCreated()
{
  QgsBrowserWatcher *watcher = dynamic_cast<QgsBrowserWatcher *>( sender() );
  if ( !watcher )
    return;
  QgsDataItem* item = watcher->item();
  QVector <QgsDataItem*> children = watcher->result();
  QgsDebugMsg( QString( "path = %1 children.size() = %2" ).arg( item->path() ).arg( children.size() ) );
  QModelIndex index = findItem( item );
  if ( !index.isValid() ) // check if item still exists
    return;
  item->refresh( children );
  emit dataChanged( index, index );
}

bool QgsBrowserModel::fetching( QgsDataItem* item ) const
{
  foreach ( QgsBrowserWatcher * watcher, mWatchers )
  {
    if ( !watcher->isFinished() && watcher->item() == item )
      return true;
  }
  return false;
}

void QgsBrowserModel::loadingFrameChanged()
{
  mLoadingIcon = QIcon( mLoadingMovie.currentPixmap() );
  int notFinished = 0;
  foreach ( QgsBrowserWatcher * watcher, mWatchers )
  {
    if ( watcher->isFinished() )
    {
      delete watcher;
      mWatchers.removeOne( watcher );
      continue;
    }
    QModelIndex index = findItem( watcher->item() );
    QgsDebugMsg( QString( "path = %1 not finished" ).arg( watcher->item()->path() ) );
    emit dataChanged( index, index );
    notFinished++;
  }
  if ( notFinished == 0 )
    mLoadingMovie.setPaused( true );
}

void QgsBrowserModel::addFavouriteDirectory( QString favDir )
{
  Q_ASSERT( mFavourites );
  mFavourites->addDirectory( favDir );
}

void QgsBrowserModel::removeFavourite( const QModelIndex &index )
{
  QgsDirectoryItem *item = dynamic_cast<QgsDirectoryItem *>( dataItem( index ) );
  if ( !item )
    return;

  mFavourites->removeDirectory( item );
}
