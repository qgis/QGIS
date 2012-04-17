/***************************************************************************
               qgsdataitem.cpp  - Data items
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataitem.h"

#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsconfig.h"

// use internal quazip for /vsizip support
#ifdef HAVE_ZLIB
#define QUAZIP_STATIC
#include <quazip/quazip.h>
#endif

// shared icons
const QIcon &QgsLayerItem::iconPoint()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconPointLayer.png" ) );

  return icon;
}

const QIcon &QgsLayerItem::iconLine()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconLineLayer.png" ) );

  return icon;
}

const QIcon &QgsLayerItem::iconPolygon()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconPolygonLayer.png" ) );

  return icon;
}

const QIcon &QgsLayerItem::iconTable()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconTableLayer.png" ) );

  return icon;
}

const QIcon &QgsLayerItem::iconRaster()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconRaster.png" ) );

  return icon;
}

const QIcon &QgsLayerItem::iconDefault()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconLayer.png" ) );

  return icon;
}

const QIcon &QgsDataCollectionItem::iconDataCollection()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconDbSchema.png" ) );

  return icon;
}

const QIcon &QgsDataCollectionItem::iconDir()
{
  static QIcon icon;

  if ( icon.isNull() )
  {
    // initialize shared icons
    QStyle *style = QApplication::style();
    icon = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
    icon.addPixmap( style->standardPixmap( QStyle::SP_DirOpenIcon ),
                    QIcon::Normal, QIcon::On );
  }

  return icon;
}

const QIcon &QgsFavouritesItem::iconFavourites()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconFavourites.png" ) );

  return icon;
}

const QIcon &QgsZipItem::iconZip()
{
  static QIcon icon;

  if ( icon.isNull() )
    icon = QIcon( getThemePixmap( "/mIconZip.png" ) );
// icon from http://www.softicons.com/free-icons/application-icons/mega-pack-icons-1-by-nikolay-verin/winzip-folder-icon

  return icon;
}


QgsDataItem::QgsDataItem( QgsDataItem::Type type, QgsDataItem* parent, QString name, QString path )
    : QObject( parent ), mType( type ), mParent( parent ), mPopulated( false ), mName( name ), mPath( path )
{
}

// TODO: This is copy from QgisApp, bad
// TODO: add some caching mechanism ?
QPixmap QgsDataItem::getThemePixmap( const QString theName )
{
  QString myPreferredPath = QgsApplication::activeThemePath()  + QDir::separator() + theName;
  QString myDefaultPath = QgsApplication::defaultThemePath()  + QDir::separator() + theName;

  // QgsDebugMsg( "theName = " + theName );
  // QgsDebugMsg( "myPreferredPath = " + myPreferredPath );
  // QgsDebugMsg( "myDefaultPath = " + myDefaultPath );
  if ( QFile::exists( myPreferredPath ) )
  {
    return QPixmap( myPreferredPath );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QPixmap( myDefaultPath );
  }
}

void QgsDataItem::emitBeginInsertItems( QgsDataItem* parent, int first, int last )
{
  emit beginInsertItems( parent, first, last );
}
void QgsDataItem::emitEndInsertItems()
{
  emit endInsertItems();
}
void QgsDataItem::emitBeginRemoveItems( QgsDataItem* parent, int first, int last )
{
  emit beginRemoveItems( parent, first, last );
}
void QgsDataItem::emitEndRemoveItems()
{
  emit endRemoveItems();
}

QVector<QgsDataItem*> QgsDataItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  return children;
}

void QgsDataItem::populate()
{
  if ( mPopulated )
    return;

  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsDataItem*> children = createChildren( );
  foreach( QgsDataItem *child, children )
  {
    // initialization, do not refresh! That would result in infinite loop (beginInsertItems->rowCount->populate)
    addChildItem( child );
  }
  mPopulated = true;

  QApplication::restoreOverrideCursor();
}

int QgsDataItem::rowCount()
{
  if ( !mPopulated )
    populate();
  return mChildren.size();
}
bool QgsDataItem::hasChildren()
{
  return ( mPopulated ? mChildren.count() > 0 : true );
}

void QgsDataItem::addChildItem( QgsDataItem * child, bool refresh )
{
  QgsDebugMsg( QString( "add child #%1 - %2 - %3" ).arg( mChildren.size() ).arg( child->mName ).arg( child->mType ) );

  int i;
  if ( type() == Directory )
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

  connect( child, SIGNAL( beginInsertItems( QgsDataItem*, int, int ) ),
           this, SLOT( emitBeginInsertItems( QgsDataItem*, int, int ) ) );
  connect( child, SIGNAL( endInsertItems() ),
           this, SLOT( emitEndInsertItems() ) );
  connect( child, SIGNAL( beginRemoveItems( QgsDataItem*, int, int ) ),
           this, SLOT( emitBeginRemoveItems( QgsDataItem*, int, int ) ) );
  connect( child, SIGNAL( endRemoveItems() ),
           this, SLOT( emitEndRemoveItems() ) );

  if ( refresh )
    emit endInsertItems();
}
void QgsDataItem::deleteChildItem( QgsDataItem * child )
{
  QgsDebugMsg( "mName = " + child->mName );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  delete child;
  emit endRemoveItems();
}

int QgsDataItem::findItem( QVector<QgsDataItem*> items, QgsDataItem * item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    QgsDebugMsg( QString::number( i ) + " : " + items[i]->mPath + " x " + item->mPath );
    if ( items[i]->equal( item ) )
      return i;
  }
  return -1;
}

void QgsDataItem::refresh()
{
  QgsDebugMsg( "mPath = " + mPath );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QVector<QgsDataItem*> items = createChildren( );

  // Remove no more present items
  QVector<QgsDataItem*> remove;
  foreach( QgsDataItem *child, mChildren )
  {
    if ( findItem( items, child ) >= 0 )
      continue;
    remove.append( child );
  }
  foreach( QgsDataItem *child, remove )
  {
    deleteChildItem( child );
  }

  // Add new items
  foreach( QgsDataItem *item, items )
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

bool QgsDataItem::equal( const QgsDataItem *other )
{
  if ( metaObject()->className() == other->metaObject()->className() &&
       mPath == other->path() )
  {
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------

QgsLayerItem::QgsLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType, QString providerKey )
    : QgsDataItem( Layer, parent, name, path )
    , mProviderKey( providerKey )
    , mUri( uri )
    , mLayerType( layerType )
{
  switch ( layerType )
  {
    case Point:      mIcon = iconPoint(); break;
    case Line:       mIcon = iconLine(); break;
    case Polygon:    mIcon = iconPolygon(); break;
      // TODO add a new icon for generic Vector layers
    case Vector :    mIcon = iconPolygon(); break;
    case TableLayer: mIcon = iconTable(); break;
    case Raster:     mIcon = iconRaster(); break;
    default:         mIcon = iconDefault(); break;
  }
}

QgsMapLayer::LayerType QgsLayerItem::mapLayerType()
{
  if ( mLayerType == QgsLayerItem::Raster )
    return QgsMapLayer::RasterLayer;
  return QgsMapLayer::VectorLayer;
}

bool QgsLayerItem::equal( const QgsDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  //const QgsLayerItem *o = qobject_cast<const QgsLayerItem *> ( other );
  const QgsLayerItem *o = dynamic_cast<const QgsLayerItem *>( other );
  return ( mPath == o->mPath && mName == o->mName && mUri == o->mUri && mProviderKey == o->mProviderKey );
}

// ---------------------------------------------------------------------
QgsDataCollectionItem::QgsDataCollectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataItem( Collection, parent, name, path )
{
  mIcon = iconDataCollection();
}

QgsDataCollectionItem::~QgsDataCollectionItem()
{
  foreach( QgsDataItem* i, mChildren )
  delete i;
}

//-----------------------------------------------------------------------
// QVector<QgsDataProvider*> QgsDirectoryItem::mProviders = QVector<QgsDataProvider*>();
QVector<QLibrary*> QgsDirectoryItem::mLibraries = QVector<QLibrary*>();


QgsDirectoryItem::QgsDirectoryItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mType = Directory;
  mIcon = iconDir();

  if ( mLibraries.size() == 0 )
  {
    QStringList keys = QgsProviderRegistry::instance()->providerList();
    QStringList::const_iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i )
    {
      QString k( *i );
      // some providers hangs with empty uri (Postgis) etc...
      // -> using libraries directly
      QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( k );
      if ( library )
      {
        dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
        if ( !dataCapabilities )
        {
          QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
          continue;
        }
        if ( dataCapabilities() == QgsDataProvider::NoDataCapabilities )
        {
          QgsDebugMsg( library->fileName() + " has NoDataCapabilities" );
          continue;
        }

        QgsDebugMsg( QString( "%1 dataCapabilities : %2" ).arg( library->fileName() ).arg( dataCapabilities() ) );
        mLibraries.append( library );
      }
      else
      {
        //QgsDebugMsg ( "Cannot get provider " + k );
      }
    }
  }
}

QgsDirectoryItem::~QgsDirectoryItem()
{
}

QVector<QgsDataItem*> QgsDirectoryItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  QDir dir( mPath );
  QSettings settings;
  bool scanZip = ( settings.value( "/qgis/scanZipInBrowser", 1 ).toInt() != 0 );

  QStringList entries = dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  foreach( QString subdir, entries )
  {
    QString subdirPath = dir.absoluteFilePath( subdir );
    QgsDebugMsg( QString( "creating subdir: %1" ).arg( subdirPath ) );

    QgsDirectoryItem *item = new QgsDirectoryItem( this, subdir, subdirPath );
    // propagate signals up to top

    children.append( item );
  }

  QStringList fileEntries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
  foreach( QString name, fileEntries )
  {
    QString path = dir.absoluteFilePath( name );
    QFileInfo fileInfo( path );

    // vsizip support was added to GDAL/OGR 1.6 but this symbol not available here
// #if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1600
    if ( fileInfo.suffix() == "zip" && scanZip )
    {
      QgsDataItem * item = QgsZipItem::itemFromPath( this, path, name );
      if ( item )
      {
        children.append( item );
        continue;
      }
    }
// #endif

    foreach( QLibrary *library, mLibraries )
    {
      // we could/should create separate list of providers for each purpose

      // TODO: use existing fileVectorFilters(),directoryDrivers() ?
      dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
      if ( !dataCapabilities )
      {
        continue;
      }

      int capabilities = dataCapabilities();

      if ( !(( fileInfo.isFile() && ( capabilities & QgsDataProvider::File ) ) ||
             ( fileInfo.isDir() && ( capabilities & QgsDataProvider::Dir ) ) ) )
      {
        continue;
      }

      dataItem_t * dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
      if ( ! dataItem )
      {
        QgsDebugMsg( library->fileName() + " does not have dataItem" );
        continue;
      }

      QgsDataItem * item = dataItem( path, this );
      if ( item )
      {
        children.append( item );
      }
    }
  }

  return children;
}

bool QgsDirectoryItem::equal( const QgsDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  return ( path() == other->path() );
}

QWidget * QgsDirectoryItem::paramWidget()
{
  return new QgsDirectoryParamWidget( mPath );
}

QgsDirectoryParamWidget::QgsDirectoryParamWidget( QString path, QWidget* parent )
    : QTreeWidget( parent )
{
  setRootIsDecorated( false );

  // name, size, date, permissions, owner, group, type
  setColumnCount( 7 );
  QStringList labels;
  labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
  setHeaderLabels( labels );

  QStyle* style = QApplication::style();
  QIcon iconDirectory = QIcon( style->standardPixmap( QStyle::SP_DirClosedIcon ) );
  QIcon iconFile = QIcon( style->standardPixmap( QStyle::SP_FileIcon ) );
  QIcon iconLink = QIcon( style->standardPixmap( QStyle::SP_FileLinkIcon ) ); // TODO: symlink to directory?

  QList<QTreeWidgetItem *> items;

  QDir dir( path );
  QStringList entries = dir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  foreach( QString name, entries )
  {
    QFileInfo fi( dir.absoluteFilePath( name ) );
    QStringList texts;
    texts << name;
    QString size;
    if ( fi.size() > 1024 )
    {
      size = size.sprintf( "%.1f KiB", fi.size() / 1024.0 );
    }
    else if ( fi.size() > 1.048576e6 )
    {
      size = size.sprintf( "%.1f MiB", fi.size() / 1.048576e6 );
    }
    else
    {
      size = QString( "%1 B" ).arg( fi.size() );
    }
    texts << size;
    texts << fi.lastModified().toString( Qt::SystemLocaleShortDate );
    QString perm;
    perm += fi.permission( QFile::ReadOwner ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOwner ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOwner ) ? 'x' : '-';
    // QFile::ReadUser, QFile::WriteUser, QFile::ExeUser
    perm += fi.permission( QFile::ReadGroup ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteGroup ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeGroup ) ? 'x' : '-';
    perm += fi.permission( QFile::ReadOther ) ? 'r' : '-';
    perm += fi.permission( QFile::WriteOther ) ? 'w' : '-';
    perm += fi.permission( QFile::ExeOther ) ? 'x' : '-';
    texts << perm;

    texts << fi.owner();
    texts << fi.group();

    QString type;
    QIcon icon;
    if ( fi.isDir() )
    {
      type = tr( "folder" );
      icon = iconDirectory;
    }
    else if ( fi.isFile() )
    {
      type = tr( "file" );
      icon = iconFile;
    }
    else if ( fi.isSymLink() )
    {
      type = tr( "link" );
      icon = iconLink;
    }

    texts << type;

    QTreeWidgetItem *item = new QTreeWidgetItem( texts );
    item->setIcon( 0, icon );
    items << item;
  }

  addTopLevelItems( items );

  // hide columns that are not requested
  QSettings settings;
  QList<QVariant> lst = settings.value( "/dataitem/directoryHiddenColumns" ).toList();
  foreach( QVariant colVariant, lst )
  {
    setColumnHidden( colVariant.toInt(), true );
  }
}

void QgsDirectoryParamWidget::mousePressEvent( QMouseEvent* event )
{
  if ( event->button() == Qt::RightButton )
  {
    // show the popup menu
    QMenu popupMenu;

    QStringList labels;
    labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
    for ( int i = 0; i < labels.count(); i++ )
    {
      QAction* action = popupMenu.addAction( labels[i], this, SLOT( showHideColumn() ) );
      action->setObjectName( QString::number( i ) );
      action->setCheckable( true );
      action->setChecked( !isColumnHidden( i ) );
    }

    popupMenu.exec( event->globalPos() );
  }
}

void QgsDirectoryParamWidget::showHideColumn()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  if ( !action )
    return; // something is wrong

  int columnIndex = action->objectName().toInt();
  setColumnHidden( columnIndex, !isColumnHidden( columnIndex ) );

  // save in settings
  QSettings settings;
  QList<QVariant> lst;
  for ( int i = 0; i < columnCount(); i++ )
  {
    if ( isColumnHidden( i ) )
      lst.append( QVariant( i ) );
  }
  settings.setValue( "/dataitem/directoryHiddenColumns", lst );
}


QgsErrorItem::QgsErrorItem( QgsDataItem* parent, QString error, QString path )
    : QgsDataItem( QgsDataItem::Error, parent, error, path )
{
  mIcon = QIcon( getThemePixmap( "/mIconDelete.png" ) );

  mPopulated = true; // no more children
}

QgsErrorItem::~QgsErrorItem()
{
}

QgsFavouritesItem::QgsFavouritesItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mType = Favourites;
  mIcon = iconFavourites();
}

QgsFavouritesItem::~QgsFavouritesItem()
{
}

QVector<QgsDataItem*> QgsFavouritesItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  QgsDataItem* item;

  QSettings settings;
  QStringList favDirs = settings.value( "/browser/favourites", QVariant() ).toStringList();

  foreach( QString favDir, favDirs )
  {
    item = new QgsDirectoryItem( this, favDir, favDir );
    if ( item )
    {
      children.append( item );
    }
  }

  return children;
}

//-----------------------------------------------------------------------
QStringList QgsZipItem::mProviderNames = QStringList();
QVector<dataItem_t *> QgsZipItem::mDataItemPtr = QVector<dataItem_t*>();


QgsZipItem::QgsZipItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mType = Collection; //Zip??
  mIcon = iconZip();

  if ( mProviderNames.size() == 0 )
  {
    // QStringList keys = QgsProviderRegistry::instance()->providerList();
    // only use GDAL and OGR providers as we use the VSIFILE mechanism
    QStringList keys;
    // keys << "ogr" << "gdal";
    keys << "gdal" << "ogr";

    QStringList::const_iterator i;
    for ( i = keys.begin(); i != keys.end(); ++i )
    {
      QString k( *i );
      QgsDebugMsg( "provider " + k );
      // some providers hangs with empty uri (Postgis) etc...
      // -> using libraries directly
      QLibrary *library = QgsProviderRegistry::instance()->providerLibrary( k );
      if ( library )
      {
        dataCapabilities_t * dataCapabilities = ( dataCapabilities_t * ) cast_to_fptr( library->resolve( "dataCapabilities" ) );
        if ( !dataCapabilities )
        {
          QgsDebugMsg( library->fileName() + " does not have dataCapabilities" );
          continue;
        }
        if ( dataCapabilities() == QgsDataProvider::NoDataCapabilities )
        {
          QgsDebugMsg( library->fileName() + " has NoDataCapabilities" );
          continue;
        }
        QgsDebugMsg( QString( "%1 dataCapabilities : %2" ).arg( library->fileName() ).arg( dataCapabilities() ) );

        dataItem_t * dataItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
        if ( ! dataItem )
        {
          QgsDebugMsg( library->fileName() + " does not have dataItem" );
          continue;
        }

        // mLibraries.append( library );
        mDataItemPtr.append( dataItem );
        mProviderNames.append( k );
      }
      else
      {
        //QgsDebugMsg ( "Cannot get provider " + k );
      }
    }
  }

}

QgsZipItem::~QgsZipItem()
{
}

QVector<QgsDataItem*> QgsZipItem::createChildren( )
{
  QVector<QgsDataItem*> children;
  QStringList zipFileList;
  QString tmpPath;
  QString childPath;

  QSettings settings;
  int scanZipSetting = settings.value( "/qgis/scanZipInBrowser", 1 ).toInt();

  QgsDebugMsg( QString( "path = %1 name= %2 scanZipSetting= %3" ).arg( path() ).arg( name() ).arg( scanZipSetting ) );

  // if scanZipBrowser == 0 (No): skip to the next file
  if ( scanZipSetting == 0 )
  {
    return children;
  }

#ifndef HAVE_ZLIB
  // if zlib not available, only support Passthru
  if ( scanZipSetting == 2 || scanZipSetting == 3 )
  {
    scanZipSetting = 1;
    settings.setValue( "/qgis/scanZipInBrowser", 1 );
  }
#endif

  // if scanZipBrowser == 1 (Passthru): do not scan zip and allow to open directly with /vsigzip/
  if ( scanZipSetting == 1 )
  {
    mPath = "/vsizip/" + path(); // should check for extension
    QgsDebugMsg( "set path to " + path() );
    return children;
  }

#ifdef HAVE_ZLIB
  QgsDebugMsg( QString( "Open file %1 with quazip" ).arg( path() ) );
  // get list of files inside zip file
  QuaZip zip( path() );
  if ( ! zip.open( QuaZip::mdUnzip ) || ! zip.isOpen() )
  {
    QgsDebugMsg( QString( "Zip error: %1" ).arg( zip.getZipError() ) );
  }
  else
  {
    for ( bool more = zip.goToFirstFile(); more; more = zip.goToNextFile() )
    {
      tmpPath = zip.getCurrentFileName();
      // skip directories (files ending with /)
      if ( tmpPath.right( 1 ) != "/" )
        zipFileList << tmpPath;
    }
  }
  zip.close();
  if ( zip.getZipError() != UNZ_OK )
  {
    QgsDebugMsg( QString( "Zip error: %1" ).arg( zip.getZipError() ) );
  }
#endif

  // loop over files inside zip
  foreach( QString fileName, zipFileList )
  {
    QFileInfo info( fileName );
    tmpPath = "/vsizip/" + path() + "/" + fileName;
    QgsDebugMsg( "tmpPath = " + tmpPath );

    // foreach( dataItem_t *dataItem, mDataItemPtr )
    for ( int i = 0; i < mProviderNames.size(); i++ )
    {
      // ugly hack to remove .dbf file if there is a .shp file
      if ( mProviderNames[i] == "ogr" )
      {
        if ( info.suffix() == "dbf" )
        {
          if ( zipFileList.indexOf( fileName.left( fileName.count() - 4 ) + ".shp" ) != -1 )
            continue;
        }
        if ( info.completeSuffix().toLower() == "shp.xml" )
        {
          continue;
        }
      }

      // try to get data item from provider
      dataItem_t *dataItem = mDataItemPtr[i];
      if ( dataItem )
      {
        QgsDebugMsg( QString( "trying to load item %1 with %2" ).arg( tmpPath ).arg( mProviderNames[i] ) );
        QgsDataItem * item = dataItem( tmpPath, this );
        if ( item )
        {
          QgsDebugMsg( "loaded item" );
          childPath = tmpPath;
          children.append( item );
          break;
        }
        else
        {
          QgsDebugMsg( "not loaded item" );
        }
      }
    }

  }

  if ( children.size() == 1 )
  {
    // save the name of the only child so we can get a normal data item from it
    mPath = childPath;
  }

  return children;
}



QgsDataItem* QgsZipItem::itemFromPath( QgsDataItem* parent, QString path, QString name )
{

  QSettings settings;
  int scanZipSetting = settings.value( "/qgis/scanZipInBrowser", 1 ).toInt();
  QFileInfo fileInfo( path );
  QString tmpPath = path;
  QgsZipItem * zipItem = 0;

  QgsDebugMsg( QString( "path = %1 name= %2 scanZipSetting= %3" ).arg( path ).arg( name ).arg( scanZipSetting ) );

  // if scanZipBrowser == 0 (No): skip to the next file
  if ( scanZipSetting == 0 )
  {
    return 0;
  }
  // if scanZipBrowser == 1 (Passthru): do not scan zip and allow to open directly with /vsigzip/
  else if ( scanZipSetting == 1 )
  {
    tmpPath = "/vsizip/" + path;
    zipItem = 0;
  }
  else
  {
    zipItem = new QgsZipItem( parent, name, path );
  }

  if ( zipItem )
  {
    // force populate zipItem
    zipItem->populate();
    QgsDebugMsg( QString( "Got zipItem with %1 children, path=%2, name=%3" ).arg( zipItem->rowCount() ).arg( zipItem->path() ).arg( zipItem->name() ) );
  }

// only display if has children
// other option would be to delay until item is opened, but we would be polluting the tree with empty items
  if ( zipItem && zipItem->rowCount() > 1 )
  {
    QgsDebugMsg( "returning zipItem" );
    return zipItem;
  }
// if 1 or 0 child found, create a data item using the full path given by QgsZipItem
  else
  {
    if ( zipItem )
    {
      tmpPath = zipItem->path();
      delete zipItem;
    }

    QgsDebugMsg( QString( "will try to create a normal dataItem from path= %2" ).arg( tmpPath ) );

    // try to open using registered providers (gdal and ogr)
    for ( int i = 0; i < mProviderNames.size(); i++ )
    {
      dataItem_t *dataItem = mDataItemPtr[i];
      if ( dataItem )
      {
        QgsDataItem *item = dataItem( tmpPath, parent );
        if ( item )
          return item;
      }
    }
  }

  return 0;
}
