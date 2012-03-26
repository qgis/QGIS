#include <QDir>
#include <QApplication>
#include <QStyle>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsmimedatautils.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"

#include "qgsbrowsermodel.h"

#include <QSettings>

QgsBrowserModel::QgsBrowserModel( QObject *parent ) :
    QAbstractItemModel( parent )
{
  addRootItems();
}

QgsBrowserModel::~QgsBrowserModel()
{
  removeRootItems();
}

void QgsBrowserModel::addRootItems()
{
  // give the home directory a prominent first place
  QgsDirectoryItem *item = new QgsDirectoryItem( NULL, tr( "Home" ), QDir::homePath() );
  QStyle *style = QApplication::style();
  QIcon homeIcon( style->standardPixmap( QStyle::SP_DirHomeIcon ) );
  item->setIcon( homeIcon );
  connectItem( item );
  mRootItems << item;

  // add favourite directories
  QSettings settings;
  QStringList favDirs = settings.value( "/browser/favourites", QVariant() ).toStringList();
  // if there are 5 or more items, create a "Favourites" Root item
  // perhaps this should be the default?
  if ( favDirs.count() >= 5 )
  {
    QgsFavouritesItem *item = new QgsFavouritesItem( NULL, tr( "Favourites" ), "" );
    if ( item )
    {
      connectItem( item );
      mRootItems << item;
    }
  }
  else
  {
    foreach( QString favDir, favDirs )
    {
      item = new QgsDirectoryItem( NULL, favDir, favDir );
      if ( item )
      {
        item->setIcon( QgsFavouritesItem::iconFavourites() );
        mRootItems << item;
        connectItem( item );
      }
    }
  }

  foreach( QFileInfo drive, QDir::drives() )
  {
    QString path = drive.absolutePath();
    QgsDirectoryItem *item = new QgsDirectoryItem( NULL, path, path );

    connectItem( item );
    mRootItems << item;
  }

  // Add non file top level items
  QStringList providersList = QgsProviderRegistry::instance()->providerList();
  providersList.sort();
  foreach( QString key, providersList )
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

    dataItem_t * dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
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
      mRootItems << item;
    }
  }
}

void QgsBrowserModel::removeRootItems()
{
  foreach( QgsDataItem* item, mRootItems )
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
    QgsLayerItem *layer = ( QgsLayerItem* ) ptr;
    if ( layer->providerKey() != "wms" )
    {
      flags |= Qt::ItemIsDragEnabled;
    }
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
  //qDebug("rowCount: idx: (valid %d) %d %d", parent.isValid(), parent.row(), parent.column());

  if ( !parent.isValid() )
  {
    // root item: its children are top level items
    return mRootItems.count(); // mRoot
  }
  else
  {
    // ordinary item: number of its children
    QgsDataItem *item = dataItem( parent );
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

QModelIndex QgsBrowserModel::findPath( QString path )
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

      if ( path.startsWith( item->path() ) )
      {
        // we have found a preceding item: stop searching on this level and go deeper
        foundChild = true;
        theIndex = idx;
        break;
      }
    }
  }

  return QModelIndex(); // not found
}

void QgsBrowserModel::reload()
{
  removeRootItems();
  addRootItems();
  reset(); // Qt4.6 brings better methods beginResetModel + endResetModel
}

/* Refresh dir path */
void QgsBrowserModel::refresh( QString path )
{
  QModelIndex idx = findPath( path );
  if ( idx.isValid() )
  {
    QgsDataItem* item = dataItem( idx );
    if ( item )
      item->refresh();
  }
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

/* Refresh item */
void QgsBrowserModel::refresh( const QModelIndex& theIndex )
{
  QgsDataItem *item = dataItem( theIndex );
  if ( !item )
    return;

  QgsDebugMsg( "Refresh " + item->path() );
  item->refresh();
}

void QgsBrowserModel::beginInsertItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsg( "valid" );
  beginInsertRows( idx, first, last );
  QgsDebugMsg( "end" );
}
void QgsBrowserModel::endInsertItems()
{
  QgsDebugMsg( "Entered" );
  endInsertRows();
}
void QgsBrowserModel::beginRemoveItems( QgsDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsBrowserModel::endRemoveItems()
{
  QgsDebugMsg( "Entered" );
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
  foreach( const QModelIndex &index, indexes )
  {
    if ( index.isValid() )
    {
      QgsDataItem* ptr = ( QgsDataItem* ) index.internalPointer();
      if ( ptr->type() != QgsDataItem::Layer ) continue;
      QgsLayerItem *layer = ( QgsLayerItem* ) ptr;
      if ( layer->providerKey() == "wms" ) continue;
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
