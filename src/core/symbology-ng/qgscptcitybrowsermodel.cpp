/***************************************************************************
    qgscptcitybrowsermodel.cpp
    ---------------------
    begin                : August 2012
    copyright            : (C) 2009 by Martin Dobias
    copyright            : (C) 2011 Radim Blazek
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QStyle>
#include <QSettings>

#include "qgscptcitybrowsermodel.h"
#include "qgis.h"

#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsconfig.h"
#include "qgsmimedatautils.h"


QgsCptCityDataItem::QgsCptCityDataItem( QgsCptCityDataItem::Type type, QgsCptCityDataItem* parent,
                                        QString name, QString path, QString info )
// Do not pass parent to QObject, Qt would delete this when parent is deleted
    : QObject(), mType( type ), mParent( parent ), mPopulated( false ),
    mName( name ), mPath( path ), mInfo( info ), mValid( true )
{
}

QgsCptCityDataItem::~QgsCptCityDataItem()
{
  // QgsDebugMsg( "mName = " + mName + " mPath = " + mPath );
}

void QgsCptCityDataItem::emitBeginInsertItems( QgsCptCityDataItem* parent, int first, int last )
{
  emit beginInsertItems( parent, first, last );
}
void QgsCptCityDataItem::emitEndInsertItems()
{
  emit endInsertItems();
}
void QgsCptCityDataItem::emitBeginRemoveItems( QgsCptCityDataItem* parent, int first, int last )
{
  emit beginRemoveItems( parent, first, last );
}
void QgsCptCityDataItem::emitEndRemoveItems()
{
  emit endRemoveItems();
}

QVector<QgsCptCityDataItem*> QgsCptCityDataItem::createChildren( )
{
  QVector<QgsCptCityDataItem*> children;
  return children;
}

void QgsCptCityDataItem::populate()
{
  if ( mPopulated )
    return;

  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsCptCityDataItem*> children = createChildren( );
  foreach ( QgsCptCityDataItem *child, children )
  {
    // initialization, do not refresh! That would result in infinite loop (beginInsertItems->rowCount->populate)
    addChildItem( child );
  }
  mPopulated = true;

  QApplication::restoreOverrideCursor();
}

int QgsCptCityDataItem::rowCount()
{
  // if ( !mPopulated )
  //   populate();
  return mChildren.size();
}
bool QgsCptCityDataItem::hasChildren()
{
  return ( mPopulated ? mChildren.count() > 0 : true );
}

void QgsCptCityDataItem::addChildItem( QgsCptCityDataItem * child, bool refresh )
{
  QgsDebugMsg( QString( "add child #%1 - %2 - %3" ).arg( mChildren.size() ).arg( child->mName ).arg( child->mType ) );

  int i;
  if ( type() == Directory || type() == Category )
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      // sort items by type, so directories are before data items
      if ( mChildren[i]->mType == child->mType &&
           mChildren[i]->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }
  else
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      if ( mChildren[i]->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }

  if ( refresh )
    emit beginInsertItems( this, i, i );

  mChildren.insert( i, child );

  connect( child, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( emitBeginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  connect( child, SIGNAL( endInsertItems() ),
           this, SLOT( emitEndInsertItems() ) );
  connect( child, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( emitBeginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  connect( child, SIGNAL( endRemoveItems() ),
           this, SLOT( emitEndRemoveItems() ) );

  if ( refresh )
    emit endInsertItems();
}
void QgsCptCityDataItem::deleteChildItem( QgsCptCityDataItem * child )
{
  // QgsDebugMsg( "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  delete child;
  emit endRemoveItems();
}

QgsCptCityDataItem * QgsCptCityDataItem::removeChildItem( QgsCptCityDataItem * child )
{
  // QgsDebugMsg( "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  emit endRemoveItems();
  disconnect( child, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
              this, SLOT( emitBeginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  disconnect( child, SIGNAL( endInsertItems() ),
              this, SLOT( emitEndInsertItems() ) );
  disconnect( child, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
              this, SLOT( emitBeginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  disconnect( child, SIGNAL( endRemoveItems() ),
              this, SLOT( emitEndRemoveItems() ) );
  child->setParent( 0 );
  return child;
}

int QgsCptCityDataItem::findItem( QVector<QgsCptCityDataItem*> items, QgsCptCityDataItem * item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    // QgsDebugMsg( QString::number( i ) + " : " + items[i]->mPath + " x " + item->mPath );
    if ( items[i]->equal( item ) )
      return i;
  }
  return -1;
}

void QgsCptCityDataItem::refresh()
{
  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsCptCityDataItem*> items = createChildren( );

  // Remove no more present items
  QVector<QgsCptCityDataItem*> remove;
  foreach ( QgsCptCityDataItem *child, mChildren )
  {
    if ( findItem( items, child ) >= 0 )
      continue;
    remove.append( child );
  }
  foreach ( QgsCptCityDataItem *child, remove )
  {
    deleteChildItem( child );
  }

  // Add new items
  foreach ( QgsCptCityDataItem *item, items )
  {
    // Is it present in childs?
    if ( findItem( mChildren, item ) >= 0 )
    {
      delete item;
      continue;
    }
    addChildItem( item, true );
  }

  QApplication::restoreOverrideCursor();
}

bool QgsCptCityDataItem::equal( const QgsCptCityDataItem *other )
{
  if ( metaObject()->className() == other->metaObject()->className() &&
       mPath == other->path() )
  {
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------

QgsCptCityColorRampItem::QgsCptCityColorRampItem( QgsCptCityDataItem* parent,
    QString name, QString path,
    QString info, QString variantName )
    : QgsCptCityDataItem( ColorRamp, parent, name, path, info ),
    mRamp( path, variantName )
{
  mPopulated = true;

  // TODO how to get iconSize from treeView?
  QSize iconSize( 100, 15 );

  // make preview from variant if exists
  QStringList variantList = mRamp.variantList();
  if ( variantName.isNull() && ! variantList.isEmpty() )
  {
    mRamp.setVariantName( variantList[ variantList.count() / 2 ] );
    mRamp.loadFile();
  }

  // is this item valid? this might fail when there are variants, check
  if ( ! QFile::exists( mRamp.fileName() ) )
    mValid = false;
  else
    mValid = true;

  // load file and set info
  if ( mRamp.count() > 0 )
  {
    if ( variantList.isEmpty() )
    {
      int count = mRamp.count();
      QgsCptCityColorRampV2::GradientType type = mRamp.gradientType();
      if ( type == QgsCptCityColorRampV2::Discrete )
        count--;
      mInfo = QString::number( count ) + " " + tr( "colors" ) + " - ";
      if ( type == QgsCptCityColorRampV2::Continuous )
        mInfo += tr( "continuous" );
      else if ( type == QgsCptCityColorRampV2::ContinuousMulti )
        mInfo += tr( "continuous (multi)" );
      else if ( type == QgsCptCityColorRampV2::Discrete )
        mInfo += tr( "discrete" );
    }
    else
    {
      mInfo = QString::number( variantList.count() ) + " " + tr( "variants" );
    }
    setIcon( QgsSymbolLayerV2Utils::colorRampPreviewIcon( &mRamp, iconSize ) );
  }
  else
  {
    QPixmap blankPixmap( iconSize );
    blankPixmap.fill( Qt::white );
    setIcon( QIcon( blankPixmap ) );
    mInfo = "";
  }

}

bool QgsCptCityColorRampItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  //const QgsCptCityColorRampItem *o = qobject_cast<const QgsCptCityColorRampItem *> ( other );
  const QgsCptCityColorRampItem *o = dynamic_cast<const QgsCptCityColorRampItem *>( other );
  return ( mPath == o->mPath && mName == o->mName &&
           ramp().variantName() == o->ramp().variantName() );
}

// ---------------------------------------------------------------------
QgsCptCityCollectionItem::QgsCptCityCollectionItem( QgsCptCityDataItem* parent,
    QString name, QString path, QString info )
    : QgsCptCityDataItem( Collection, parent, name, path, info )
{

}

QgsCptCityCollectionItem::~QgsCptCityCollectionItem()
{
  // QgsDebugMsg( "Entered" );
  foreach ( QgsCptCityDataItem* i, mChildren )
  {
    // QgsDebugMsg( QString( "delete child = 0x%0" ).arg(( qlonglong )i, 8, 16, QLatin1Char( '0' ) ) );
    delete i;
  }
}

//-----------------------------------------------------------------------
QgsCptCityDirectoryItem::QgsCptCityDirectoryItem( QgsCptCityDataItem* parent,
    QString name, QString path, QString info )
    : QgsCptCityCollectionItem( parent, name, path, info )
{
  mType = Directory;
  mValid = QDir( QgsCptCityCollection::defaultBaseDir() ).exists();
  if ( ! mValid )
    QgsDebugMsg( "created invalid dir item, path = " + QgsCptCityCollection::defaultBaseDir() + "/" + mPath );
  // populate();
}

QgsCptCityDirectoryItem::~QgsCptCityDirectoryItem()
{
}

QVector<QgsCptCityDataItem*> QgsCptCityDirectoryItem::createChildren( )
{
  QgsCptCityCollection* collection = QgsCptCityCollection::defaultCollection();
  QgsCptCityDataItem* item = 0;
  QVector<QgsCptCityDataItem*> children;

  if ( ! mValid || ! collection )
    return children;

  QgsDebugMsg( "name= " + mName + " path= " + mPath );

  // add children dirs
  foreach ( QString childPath, collection->listDirNames( mPath ) )
  {
    QgsDebugMsg( "childPath = " + childPath + " name= " + QFileInfo( childPath ).baseName() );
    item = new QgsCptCityDirectoryItem( this, QFileInfo( childPath ).baseName(), childPath,
                                        collection->dirNamesMap().value( childPath ) );
    if ( item->isValid() )
      children << item;
    else
      delete item;
  }

  // add children schemes
  foreach ( QString schemeName, collection->schemeMap().value( mPath ) )
  {
    // QgsDebugMsg( "schemeName = " + schemeName );
    item = new QgsCptCityColorRampItem( this, schemeName, mPath + "/" + schemeName );
    if ( item->isValid() )
      children << item;
    else
      delete item;
  }

  return children;
}

bool QgsCptCityDirectoryItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}


//-----------------------------------------------------------------------
QgsCptCityCategoryItem::QgsCptCityCategoryItem( QgsCptCityDataItem* parent,
    QString name, QString path, QString info )
    : QgsCptCityCollectionItem( parent, name, path, info )
{
  mType = Category;
}

QgsCptCityCategoryItem::~QgsCptCityCategoryItem()
{
}

QVector<QgsCptCityDataItem*> QgsCptCityCategoryItem::createChildren( )
{
  QgsCptCityCollection* collection = QgsCptCityCollection::defaultCollection();
  QgsCptCityDataItem* item = 0;
  QVector<QgsCptCityDataItem*> children;

  if ( ! mValid || ! collection )
    return children;

  QgsDebugMsg( "name= " + mName + " path= " + mPath );

  // add children collections
  foreach ( QString childPath, collection->selectionsMap().value( mPath ) )
  {
    QgsDebugMsg( "childPath = " + childPath + " name= " + QFileInfo( childPath ).baseName() );
    if ( childPath.endsWith( "/" ) )
    {
      childPath.chop( 1 );
      item = new QgsCptCityDirectoryItem( this, childPath, childPath,
                                          collection->dirNamesMap().value( childPath ) );
      if ( item->isValid() )
        children << item;
      else
        delete item;
    }
    else
    {
      item = new QgsCptCityColorRampItem( this, childPath, childPath );
      if ( item->isValid() )
        children << item;
      else
        delete item;
    }
  }

  QgsDebugMsg( QString( "path= %1 inserted %2 children" ).arg( mPath ).arg( children.count() ) );

  return children;
}

bool QgsCptCityCategoryItem::equal( const QgsCptCityDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

//-----------------------------------------------------------------------

QgsCptCityBrowserModel::QgsCptCityBrowserModel( QObject *parent, QgsCptCityCollection* collection, QString viewName )
    : QAbstractItemModel( parent ), mCollection( collection ), mViewName( viewName )
{
  if ( mCollection == NULL )
  {
    QgsDebugMsg( "Error: collection invalid" );
    Q_ASSERT( mCollection != NULL );
  }
  QgsDebugMsg( "collectionName = " + collection->collectionName() + " viewName=" + viewName );
  addRootItems();
}

QgsCptCityBrowserModel::~QgsCptCityBrowserModel()
{
  removeRootItems();
}

void QgsCptCityBrowserModel::addRootItems( )
{
  if ( mViewName == "authors" )
  {
    QgsCptCityDirectoryItem* item = 0;
    foreach ( QString path, mCollection->listDirNames() )
    {
      QgsDebugMsg( "path= " + path );
      item = new QgsCptCityDirectoryItem( NULL, QFileInfo( path ).baseName(), path,
                                          mCollection->dirNamesMap().value( path ) );
      if ( item->isValid() )
        mRootItems << item;
      else
        delete item;
    }
  }
  else if ( mViewName == "selections" )
  {
    QgsCptCityCategoryItem* item = 0;
    QMapIterator< QString, QStringList> it( mCollection->selectionsMap() );
    while ( it.hasNext() )
    {
      it.next();
      QString path = it.key();
      QString info = mCollection->dirNamesMap().value( path );
      QgsDebugMsg( "path= " + path + " info= " + info );
      item = new QgsCptCityCategoryItem( NULL, path, path, info );
      //TODO remove item if there are no children (e.g. esri in qgis-sel)
      if ( item->isValid() )
        mRootItems << item;
      else
        delete item;
    }
  }

}

void QgsCptCityBrowserModel::removeRootItems()
{
  foreach ( QgsCptCityDataItem* item, mRootItems )
  {
    delete item;
  }

  mRootItems.clear();
}


Qt::ItemFlags QgsCptCityBrowserModel::flags( const QModelIndex & index ) const
{
  if ( !index.isValid() )
    return 0;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  return flags;
}

QVariant QgsCptCityBrowserModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  QgsCptCityDataItem *item = dataItem( index );
  if ( !item )
  {
    return QVariant();
  }
  else if ( role == Qt::DisplayRole )
  {
    if ( index.column() == 0 )
      return item->name();
    if ( index.column() == 1 )
    {
      if ( item->type() == QgsCptCityDataItem::ColorRamp )
        return "   " + item->info();
      else
        return item->info();
    }
  }
  else if ( role == Qt::ToolTipRole )
  {
    return item->toolTip();
  }
  else if ( role == Qt::DecorationRole && index.column() == 1 &&
            item->type() == QgsCptCityDataItem::ColorRamp )
  {
    return item->icon();
  }
  else if ( role == Qt::FontRole &&
            ( item->type() == QgsCptCityDataItem::Directory ||
              item->type() == QgsCptCityDataItem::Category ) )
  {
    // collectionitems are larger and bold
    QFont font;
    font.setPointSize( font.pointSize() + 1 );
    font.setBold( true );
    return font;
  }
  else
  {
    // unsupported role
    return QVariant();
  }
  return QVariant();
}

QVariant QgsCptCityBrowserModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  Q_UNUSED( section );
  if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
  {
    if ( section == 0 )
      return QVariant( tr( "Name" ) );
    else if ( section == 1 )
      return QVariant( tr( "Info" ) );
  }
  return QVariant();
}

int QgsCptCityBrowserModel::rowCount( const QModelIndex &parent ) const
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
    QgsCptCityDataItem *item = dataItem( parent );
    return item ? item->rowCount() : 0;
  }
}

bool QgsCptCityBrowserModel::hasChildren( const QModelIndex &parent ) const
{
  if ( !parent.isValid() )
    return true; // root item: its children are top level items

  QgsCptCityDataItem *item = dataItem( parent );
  return item && item->hasChildren();
}

int QgsCptCityBrowserModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

QModelIndex QgsCptCityBrowserModel::findPath( QString path )
{
  QModelIndex theIndex; // starting from root
  bool foundParent = false, foundChild = true;
  QString itemPath;

  QgsDebugMsg( "path = " + path );

  while ( foundChild )
  {
    foundChild = false; // assume that the next child item will not be found

    for ( int i = 0; i < rowCount( theIndex ); i++ )
    {
      QModelIndex idx = index( i, 0, theIndex );
      QgsCptCityDataItem *item = dataItem( idx );
      if ( !item )
        return QModelIndex(); // an error occurred

      itemPath = item->path();

      if ( itemPath == path )
      {
        QgsDebugMsg( "Arrived " + itemPath );
        return idx; // we have found the item we have been looking for
      }

      foundParent = false;
      // if we are using a selection collection, search for target in the mapping in this group
      if ( mViewName == "selections" )
      {
        itemPath = item->name();
        foreach ( QString childPath, mCollection->selectionsMap().value( item->path() ) )
        {
          if ( childPath == path )
          {
            foundParent = true;
            break;
          }
        }
      }
      // search for target in parent directory
      else if ( path.startsWith( item->path() ) )
        foundParent = true;

      if ( foundParent )
      {
        // we have found a preceding item: stop searching on this level and go deeper
        foundChild = true;
        theIndex = idx;
        if ( canFetchMore( theIndex ) )
          fetchMore( theIndex );
        break;
      }
    }
  }

  return QModelIndex(); // not found
}

void QgsCptCityBrowserModel::reload()
{
  removeRootItems();
  addRootItems();
  reset(); // Qt4.6 brings better methods beginResetModel + endResetModel
}

/* Refresh dir path */
void QgsCptCityBrowserModel::refresh( QString path )
{
  QModelIndex idx = findPath( path );
  if ( idx.isValid() )
  {
    QgsCptCityDataItem* item = dataItem( idx );
    if ( item )
      item->refresh();
  }
}

QModelIndex QgsCptCityBrowserModel::index( int row, int column, const QModelIndex &parent ) const
{
  QgsCptCityDataItem *p = dataItem( parent );
  const QVector<QgsCptCityDataItem*> &items = p ? p->children() : mRootItems;
  QgsCptCityDataItem *item = items.value( row, 0 );
  return item ? createIndex( row, column, item ) : QModelIndex();
}

QModelIndex QgsCptCityBrowserModel::parent( const QModelIndex &index ) const
{
  QgsCptCityDataItem *item = dataItem( index );
  if ( !item )
    return QModelIndex();

  return findItem( item->parent() );
}

QModelIndex QgsCptCityBrowserModel::findItem( QgsCptCityDataItem *item, QgsCptCityDataItem *parent ) const
{
  const QVector<QgsCptCityDataItem*> &items = parent ? parent->children() : mRootItems;

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
void QgsCptCityBrowserModel::refresh( const QModelIndex& theIndex )
{
  QgsCptCityDataItem *item = dataItem( theIndex );
  if ( !item )
    return;

  QgsDebugMsg( "Refresh " + item->path() );
  item->refresh();
}

void QgsCptCityBrowserModel::beginInsertItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  QgsDebugMsg( "valid" );
  beginInsertRows( idx, first, last );
  QgsDebugMsg( "end" );
}
void QgsCptCityBrowserModel::endInsertItems()
{
  QgsDebugMsg( "Entered" );
  endInsertRows();
}
void QgsCptCityBrowserModel::beginRemoveItems( QgsCptCityDataItem *parent, int first, int last )
{
  QgsDebugMsg( "parent mPath = " + parent->path() );
  QModelIndex idx = findItem( parent );
  if ( !idx.isValid() )
    return;
  beginRemoveRows( idx, first, last );
}
void QgsCptCityBrowserModel::endRemoveItems()
{
  QgsDebugMsg( "Entered" );
  endRemoveRows();
}
void QgsCptCityBrowserModel::connectItem( QgsCptCityDataItem* item )
{
  connect( item, SIGNAL( beginInsertItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( beginInsertItems( QgsCptCityDataItem*, int, int ) ) );
  connect( item, SIGNAL( endInsertItems() ),
           this, SLOT( endInsertItems() ) );
  connect( item, SIGNAL( beginRemoveItems( QgsCptCityDataItem*, int, int ) ),
           this, SLOT( beginRemoveItems( QgsCptCityDataItem*, int, int ) ) );
  connect( item, SIGNAL( endRemoveItems() ),
           this, SLOT( endRemoveItems() ) );
}

bool QgsCptCityBrowserModel::canFetchMore( const QModelIndex & parent ) const
{
  QgsCptCityDataItem* item = dataItem( parent );
  // if ( item )
  //   QgsDebugMsg( QString( "path = %1 canFetchMore = %2" ).arg( item->path() ).arg( item && ! item->isPopulated()) );
  return ( item && ! item->isPopulated() );
}

void QgsCptCityBrowserModel::fetchMore( const QModelIndex & parent )
{
  QgsCptCityDataItem* item = dataItem( parent );
  if ( item )
    item->populate();
  QgsDebugMsg( "path = " + item->path() );
}


#if 0
QStringList QgsCptCityBrowserModel::mimeTypes() const
{
  QStringList types;
  // In theory the mime type convention is: application/x-vnd.<vendor>.<application>.<type>
  // but it seems a bit over formalized. Would be an application/x-qgis-uri better?
  types << "application/x-vnd.qgis.qgis.uri";
  return types;
}

QMimeData * QgsCptCityBrowserModel::mimeData( const QModelIndexList &indexes ) const
{
  QgsMimeDataUtils::UriList lst;
  foreach ( const QModelIndex &index, indexes )
  {
    if ( index.isValid() )
    {
      QgsCptCityDataItem* ptr = ( QgsCptCityDataItem* ) index.internalPointer();
      if ( ptr->type() != QgsCptCityDataItem::Layer ) continue;
      QgsLayerItem *layer = ( QgsLayerItem* ) ptr;
      lst.append( QgsMimeDataUtils::Uri( ayer ) );
    }
  }
  return QgsMimeDataUtils::encodeUriList( lst );
}

bool QgsCptCityBrowserModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  QgsCptCityDataItem* destItem = dataItem( parent );
  if ( !destItem )
  {
    QgsDebugMsg( "DROP PROBLEM!" );
    return false;
  }

  return destItem->handleDrop( data, action );
}
#endif

QgsCptCityDataItem *QgsCptCityBrowserModel::dataItem( const QModelIndex &idx ) const
{
  void *v = idx.internalPointer();
  QgsCptCityDataItem *d = reinterpret_cast<QgsCptCityDataItem*>( v );
  Q_ASSERT( !v || d );
  return d;
}
