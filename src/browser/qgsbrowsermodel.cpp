#include <QDir>
#include <QApplication>
#include <QStyle>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"

#include "qgsbrowsermodel.h"


QgsBrowserModel::QgsBrowserModel(QObject *parent) :
    QAbstractItemModel(parent)
{
  QStyle *style = QApplication::style();
  mIconDirectory = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
  mIconDirectory.addPixmap( style->standardPixmap( QStyle::SP_DirOpenIcon ),
                            QIcon::Normal, QIcon::On );

  foreach (QFileInfo drive, QDir::drives())
  {
    QString path = drive.absolutePath();
    QgsDirectoryItem *item = new QgsDirectoryItem(NULL, path, path);

    connectItem(item);
    mRootItems << item;
  }

  // Add non file top level items
  foreach ( QString key, QgsProviderRegistry::instance()->providerList() )
  {
    QLibrary *library = QgsProviderRegistry::instance()->getLibrary(key);
    if ( !library ) continue;

    dataCapabilities_t * dataCapabilities = (dataCapabilities_t *) cast_to_fptr( library->resolve ("dataCapabilities") );
    if ( !dataCapabilities ) {
      QgsDebugMsg ( library->fileName() + " does not have dataCapabilities" );
      continue;
    }

    int capabilities = dataCapabilities();
    if ( capabilities  == QgsDataProvider::NoDataCapabilities ) 
    {
      QgsDebugMsg ( library->fileName() + " does not have any dataCapabilities" );
      continue;
    }

    dataItem_t * dataItem = (dataItem_t *) cast_to_fptr( library->resolve ("dataItem" ) );
    if ( ! dataItem )
    {
      QgsDebugMsg ( library->fileName() + " does not have dataItem" );
      continue;
    }

    QgsDataItem * item = dataItem ( "", NULL ); // empty path -> top level
    if ( item )
    {
      QgsDebugMsg ( "Add new top level item : " + item->mName );
      connectItem(item);
      mRootItems << item;
    }
  }
}

QgsBrowserModel::~QgsBrowserModel()
{
  foreach (QgsDataItem* item, mRootItems)
    delete item;
}


Qt::ItemFlags QgsBrowserModel::flags( const QModelIndex & index ) const
{
  if (!index.isValid())
    return 0;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QgsBrowserModel::data( const QModelIndex & index, int role ) const
{
  if (!index.isValid())
    return QVariant();

  QgsDataItem* ptr = (QgsDataItem*) index.internalPointer();

  if (role == Qt::DisplayRole)
  {
     return QVariant(ptr->mName);
  }
  else if (role == Qt::DecorationRole && index.column() == 0)
  {
    return QVariant( ptr->icon() );
  }

  // unsupported role
  return QVariant();
}

QVariant QgsBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    return QVariant("header");
  }

  return QVariant();
}

int QgsBrowserModel::rowCount( const QModelIndex & parent ) const
{
    //qDebug("rowCount: idx: (valid %d) %d %d", parent.isValid(), parent.row(), parent.column());

    if (!parent.isValid())
    {
      // root item: its children are top level items
      return mRootItems.count(); // mRoot
    }
    else
    {
      // ordinary item: number of its children
      QgsDataItem* ptr = (QgsDataItem*) parent.internalPointer();

      return ptr->rowCount();
    }
}

bool QgsBrowserModel::hasChildren ( const QModelIndex & parent ) const
{
  if (!parent.isValid())
  {
    return true; // root item: its children are top level items
  }
  else
  {
    QgsDataItem* ptr = (QgsDataItem*) parent.internalPointer();

    return ptr->hasChildren();
  }

  return false;
}

int QgsBrowserModel::columnCount ( const QModelIndex & parent ) const
{
  return 1;
}

QModelIndex QgsBrowserModel::index( int row, int column, const QModelIndex & parent ) const
{
  //qDebug("index: idx: (valid %d) %d %d", parent.isValid(), parent.row(), parent.column());

  if (!parent.isValid())
  {
    // this is the root item, parent of the top level items
    Q_ASSERT(column == 0 && row >= 0 && row < mRootItems.count());
    return createIndex(row,column, mRootItems[row]);
  }
  else
  {
    // this is ordinary item: return a valid index if the requested child exists
    QgsDataItem* ptr = (QgsDataItem*) parent.internalPointer();
    if (ptr->mType == QgsDataItem::Directory || ptr->mType == QgsDataItem::Collection)
    {
      // this is a directory: get index of its subdir!
      QgsDirectoryItem* di = (QgsDirectoryItem*) ptr;
      return createIndex(row, column, di->mChildren.at(row));
    }
    if (ptr->mType == QgsDataItem::Layer)
    {
      return QModelIndex(); // has no children
    }

    Q_ASSERT(false && "unknown item in index()");
  }

  return QModelIndex(); // if the child does not exist
}

QModelIndex QgsBrowserModel::index( QgsDataItem *item )
{
  // Item index
  QModelIndex index = QModelIndex();

  const QVector<QgsDataItem*>& children = item->mParent ? item->mParent->mChildren : mRootItems;

  Q_ASSERT( children.size() > 0 );
  int row = -1;
  for ( int i = 0; i < children.size(); i++ )
  {
    if ( item == children[i] )
    {
      row = i;
      break;
    }
  }
  QgsDebugMsg(  QString ( "row = %1").arg(row) );
  Q_ASSERT( row >= 0 );
  index = createIndex( row, 0, item );

  return index;
}

QModelIndex QgsBrowserModel::parent( const QModelIndex & index ) const
{
  if (!index.isValid())
    return QModelIndex();

  // return QModelInde of parent, i.e. where the parent is within its parent :-)

  //qDebug("parent of: %d %d", index.row(), index.column());

  QgsDataItem* ptr = (QgsDataItem*) index.internalPointer();
  QgsDataItem* parentItem = ptr->mParent;

  if (parentItem == NULL)
  {
    // parent of our root is invalid index
    return QModelIndex();
  }

  const QVector<QgsDataItem*>& children =
      parentItem->mParent ? ((QgsDirectoryItem*)parentItem->mParent)->mChildren : mRootItems;
  Q_ASSERT(children.count() > 0);

  for (int i = 0; i < children.count(); i++)
  {
    if (children[i] == parentItem)
      return createIndex(i, 0, parentItem);
  }

  Q_ASSERT(false && "parent not found!");
  return QModelIndex();
}

/* Refresh dir path */
void QgsBrowserModel::refresh( QString path, const QModelIndex& theIndex )
{
  QStringList paths = path.split('/');
  for ( int i = 0; i < rowCount(theIndex); i++ )
  {
    QModelIndex idx = index(i, 0, theIndex);
    QgsDataItem* ptr = (QgsDataItem*) idx.internalPointer();
    if ( ptr->mPath == path )
    {
      QgsDebugMsg( "Arrived " + ptr->mPath );
      ptr->refresh();
      return;
    }
    if ( path.indexOf ( ptr->mPath ) == 0 )
    {
      refresh( path, idx );
      break;
    }
  }
}

/* Refresh item */
void QgsBrowserModel::refresh( const QModelIndex& theIndex )
{
  if ( !theIndex.isValid () ) // root
  {
    // Nothing to do I believe, mRootItems are always the same
  }
  else
  {
    QgsDataItem* ptr = (QgsDataItem*) theIndex.internalPointer();
    QgsDebugMsg( "Refresh " + ptr->mPath );
    ptr->refresh();
  }
}

void QgsBrowserModel::beginInsertItems( QgsDataItem* parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->mPath );
  QModelIndex idx = index( parent );
  if ( !idx.isValid() ) return;
  QgsDebugMsg( "valid");
  beginInsertRows( idx, first, last );
  QgsDebugMsg( "end");
}
void QgsBrowserModel::endInsertItems()
{
  QgsDebugMsg( "Entered");
  endInsertRows();
}
void QgsBrowserModel::beginRemoveItems( QgsDataItem* parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->mPath );
  QModelIndex idx = index( parent );
  if ( !idx.isValid() ) return;
  beginRemoveRows( idx, first, last );
}
void QgsBrowserModel::endRemoveItems()
{
  QgsDebugMsg( "Entered");
  endRemoveRows();
}
void QgsBrowserModel::connectItem ( QgsDataItem* item )
{
  connect ( item, SIGNAL(beginInsertItems ( QgsDataItem*, int, int )),
            this, SLOT(beginInsertItems( QgsDataItem*, int, int )) );
  connect ( item, SIGNAL(endInsertItems ()),
            this, SLOT(endInsertItems()) );
  connect ( item, SIGNAL(beginRemoveItems ( QgsDataItem*, int, int )),
            this, SLOT(beginRemoveItems( QgsDataItem*, int, int )) );	
  connect ( item, SIGNAL(endRemoveItems ()),
            this, SLOT(endRemoveItems()) );
}
