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
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QStyle>
#include <mutex>

#include "qgis.h"
#include "qgsdataitem.h"
#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsproviderregistry.h"
#include "qgsconfig.h"
#include "qgssettings.h"
#include "qgsanimatedicon.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

// use GDAL VSI mechanism
#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include "cpl_vsi.h"
#include "cpl_string.h"

// shared icons
QIcon QgsLayerItem::iconPoint()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) );
}

QIcon QgsLayerItem::iconLine()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) );
}

QIcon QgsLayerItem::iconPolygon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) );
}

QIcon QgsLayerItem::iconTable()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) );
}

QIcon QgsLayerItem::iconRaster()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRaster.svg" ) );
}

QIcon QgsLayerItem::iconMesh()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconMeshLayer.svg" ) );
}

QIcon QgsLayerItem::iconDefault()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

QIcon QgsDataCollectionItem::iconDataCollection()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconDbSchema.svg" ) );
}

QIcon QgsDataCollectionItem::openDirIcon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolderOpen.svg" ) );
}

QIcon QgsDataCollectionItem::homeDirIcon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderHome.svg" ) );
}

QIcon QgsDataCollectionItem::iconDir()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFolder.svg" ) );
}

QIcon QgsFavoritesItem::iconFavorites()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavourites.svg" ) );
}

QVariant QgsFavoritesItem::sortKey() const
{
  return QStringLiteral( " 0" );
}

QIcon QgsZipItem::iconZip()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconZip.svg" ) );
}

QgsAnimatedIcon *QgsDataItem::sPopulatingIcon = nullptr;

QgsDataItem::QgsDataItem( QgsDataItem::Type type, QgsDataItem *parent, const QString &name, const QString &path )
// Do not pass parent to QObject, Qt would delete this when parent is deleted
  : mType( type )
  , mCapabilities( NoCapabilities )
  , mParent( parent )
  , mState( NotPopulated )
  , mName( name )
  , mPath( path )
  , mDeferredDelete( false )
  , mFutureWatcher( nullptr )
{
}

QgsDataItem::~QgsDataItem()
{
  QgsDebugMsgLevel( QStringLiteral( "mName = %1 mPath = %2 mChildren.size() = %3" ).arg( mName, mPath ).arg( mChildren.size() ), 2 );
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( !child ) // should not happen
      continue;
    child->deleteLater();
  }
  mChildren.clear();

  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    // this should not usually happen (until the item was deleted directly when createChildren was running)
    QgsDebugMsg( QStringLiteral( "mFutureWatcher not finished (should not happen) -> waitForFinished()" ) );
    mDeferredDelete = true;
    mFutureWatcher->waitForFinished();
  }

  delete mFutureWatcher;
}

QString QgsDataItem::pathComponent( const QString &string )
{
  return QString( string ).replace( QRegExp( "[\\\\/]" ), QStringLiteral( "|" ) );
}

QVariant QgsDataItem::sortKey() const
{
  return mSortKey.isValid() ? mSortKey : name();
}

void QgsDataItem::setSortKey( const QVariant &key )
{
  mSortKey = key;
}

void QgsDataItem::deleteLater()
{
  QgsDebugMsgLevel( "path = " + path(), 3 );
  setParent( nullptr ); // also disconnects parent
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( !child ) // should not happen
      continue;
    child->deleteLater();
  }
  mChildren.clear();

  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    QgsDebugMsg( QStringLiteral( "mFutureWatcher not finished -> schedule to delete later" ) );
    mDeferredDelete = true;
  }
  else
  {
    QObject::deleteLater();
  }
}

void QgsDataItem::deleteLater( QVector<QgsDataItem *> &items )
{
  Q_FOREACH ( QgsDataItem *item, items )
  {
    if ( !item ) // should not happen
      continue;
    item->deleteLater();
  }
  items.clear();
}

void QgsDataItem::moveToThread( QThread *targetThread )
{
  // QObject::moveToThread() cannot move objects with parent, but QgsDataItem is not using paren/children from QObject
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( !child ) // should not happen
      continue;
    QgsDebugMsgLevel( "moveToThread child " + child->path(), 3 );
    child->QObject::setParent( nullptr ); // to be sure
    child->moveToThread( targetThread );
  }
  QObject::moveToThread( targetThread );
}

QIcon QgsDataItem::icon()
{
  if ( state() == Populating && sPopulatingIcon )
    return sPopulatingIcon->icon();

  if ( !mIcon.isNull() )
    return mIcon;

  if ( !mIconMap.contains( mIconName ) )
  {
    mIconMap.insert( mIconName, mIconName.startsWith( ':' ) ? QIcon( mIconName ) : QgsApplication::getThemeIcon( mIconName ) );
  }

  return mIconMap.value( mIconName );
}

void QgsDataItem::setName( const QString &name )
{
  mName = name;
  emit dataChanged( this );
}

QVector<QgsDataItem *> QgsDataItem::createChildren()
{
  return QVector<QgsDataItem *>();
}

void QgsDataItem::populate( bool foreground )
{
  if ( state() == Populated || state() == Populating )
    return;

  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  if ( capabilities2() & QgsDataItem::Fast || foreground )
  {
    populate( createChildren() );
  }
  else
  {
    setState( Populating );
    // The watcher must not be created with item (in constructor) because the item may be created in thread and the watcher created in thread does not work correctly.
    if ( !mFutureWatcher )
    {
      mFutureWatcher = new QFutureWatcher< QVector <QgsDataItem *> >( this );
    }

    connect( mFutureWatcher, &QFutureWatcherBase::finished, this, &QgsDataItem::childrenCreated );
    mFutureWatcher->setFuture( QtConcurrent::run( runCreateChildren, this ) );
  }
}

// This is expected to be run in a separate thread
QVector<QgsDataItem *> QgsDataItem::runCreateChildren( QgsDataItem *item )
{
  QgsDebugMsgLevel( "path = " + item->path(), 2 );
  QTime time;
  time.start();
  QVector <QgsDataItem *> children = item->createChildren();
  QgsDebugMsgLevel( QStringLiteral( "%1 children created in %2 ms" ).arg( children.size() ).arg( time.elapsed() ), 3 );
  // Children objects must be pushed to main thread.
  Q_FOREACH ( QgsDataItem *child, children )
  {
    if ( !child ) // should not happen
      continue;
    QgsDebugMsgLevel( "moveToThread child " + child->path(), 2 );
    if ( qApp )
      child->moveToThread( qApp->thread() ); // moves also children
  }
  QgsDebugMsgLevel( QStringLiteral( "finished path %1: %2 children" ).arg( item->path() ).arg( children.size() ), 3 );
  return children;
}

void QgsDataItem::childrenCreated()
{
  QgsDebugMsgLevel( QStringLiteral( "path = %1 children.size() = %2" ).arg( path() ).arg( mFutureWatcher->result().size() ), 3 );

  if ( deferredDelete() )
  {
    QgsDebugMsg( QStringLiteral( "Item was scheduled to be deleted later" ) );
    QObject::deleteLater();
    return;
  }

  if ( mChildren.isEmpty() ) // usually populating but may also be refresh if originally there were no children
  {
    populate( mFutureWatcher->result() );
  }
  else // refreshing
  {
    refresh( mFutureWatcher->result() );
  }
  disconnect( mFutureWatcher, &QFutureWatcherBase::finished, this, &QgsDataItem::childrenCreated );
  emit dataChanged( this ); // to replace loading icon by normal icon
}

void QgsDataItem::updateIcon()
{
  emit dataChanged( this );
}

void QgsDataItem::populate( const QVector<QgsDataItem *> &children )
{
  QgsDebugMsgLevel( "mPath = " + mPath, 3 );

  Q_FOREACH ( QgsDataItem *child, children )
  {
    if ( !child ) // should not happen
      continue;
    // update after thread finished -> refresh
    addChildItem( child, true );
  }
  setState( Populated );
}

void QgsDataItem::depopulate()
{
  QgsDebugMsgLevel( "mPath = " + mPath, 3 );

  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    QgsDebugMsgLevel( "remove " + child->path(), 3 );
    child->depopulate(); // recursive
    deleteChildItem( child );
  }
  setState( NotPopulated );
}

void QgsDataItem::refresh()
{
  if ( state() == Populating )
    return;

  QgsDebugMsgLevel( "mPath = " + mPath, 3 );

  if ( capabilities2() & QgsDataItem::Fast )
  {
    refresh( createChildren() );
  }
  else
  {
    setState( Populating );
    if ( !mFutureWatcher )
    {
      mFutureWatcher = new QFutureWatcher< QVector <QgsDataItem *> >( this );
    }
    connect( mFutureWatcher, &QFutureWatcherBase::finished, this, &QgsDataItem::childrenCreated );
    mFutureWatcher->setFuture( QtConcurrent::run( runCreateChildren, this ) );
  }
}

void QgsDataItem::refreshConnections()
{
  // Walk up until the root node is reached
  if ( mParent )
  {
    mParent->refreshConnections();
  }
  else
  {
    refresh();
    emit connectionsChanged();
  }
}

void QgsDataItem::refresh( const QVector<QgsDataItem *> &children )
{
  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  // Remove no more present children
  QVector<QgsDataItem *> remove;
  Q_FOREACH ( QgsDataItem *child, mChildren )
  {
    if ( !child ) // should not happen
      continue;
    if ( findItem( children, child ) >= 0 )
      continue;
    remove.append( child );
  }
  Q_FOREACH ( QgsDataItem *child, remove )
  {
    QgsDebugMsgLevel( "remove " + child->path(), 3 );
    deleteChildItem( child );
  }

  // Add new children
  Q_FOREACH ( QgsDataItem *child, children )
  {
    if ( !child ) // should not happen
      continue;

    int index = findItem( mChildren, child );
    if ( index >= 0 )
    {
      // Refresh recursively (some providers may create more generations of descendants)
      if ( !( child->capabilities2() & QgsDataItem::Fertile ) )
      {
        // The child cannot createChildren() itself
        mChildren.value( index )->refresh( child->children() );
      }

      child->deleteLater();
      continue;
    }
    addChildItem( child, true );
  }
  setState( Populated );
}

int QgsDataItem::rowCount()
{
  return mChildren.size();
}
bool QgsDataItem::hasChildren()
{
  return ( state() == Populated ? !mChildren.isEmpty() : true );
}

void QgsDataItem::setParent( QgsDataItem *parent )
{
  if ( mParent )
  {
    disconnect( this, nullptr, mParent, nullptr );
  }
  if ( parent )
  {
    connect( this, &QgsDataItem::beginInsertItems, parent, &QgsDataItem::beginInsertItems );
    connect( this, &QgsDataItem::endInsertItems, parent, &QgsDataItem::endInsertItems );
    connect( this, &QgsDataItem::beginRemoveItems, parent, &QgsDataItem::beginRemoveItems );
    connect( this, &QgsDataItem::endRemoveItems, parent, &QgsDataItem::endRemoveItems );
    connect( this, &QgsDataItem::dataChanged, parent, &QgsDataItem::dataChanged );
    connect( this, &QgsDataItem::stateChanged, parent, &QgsDataItem::stateChanged );
  }
  mParent = parent;
}

void QgsDataItem::addChildItem( QgsDataItem *child, bool refresh )
{
  Q_ASSERT( child );
  QgsDebugMsgLevel( QStringLiteral( "path = %1 add child #%2 - %3 - %4" ).arg( mPath ).arg( mChildren.size() ).arg( child->mName ).arg( child->mType ), 3 );

  //calculate position to insert child
  int i;
  if ( type() == Directory )
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      // sort items by type, so directories are before data items
      if ( mChildren.at( i )->mType == child->mType &&
           mChildren.at( i )->mName.localeAwareCompare( child->mName ) > 0 )
        break;
    }
  }
  else
  {
    for ( i = 0; i < mChildren.size(); i++ )
    {
      if ( mChildren.at( i )->mName.localeAwareCompare( child->mName ) >= 0 )
        break;
    }
  }

  if ( refresh )
    emit beginInsertItems( this, i, i );

  mChildren.insert( i, child );
  child->setParent( this );

  if ( refresh )
    emit endInsertItems();
}

void QgsDataItem::deleteChildItem( QgsDataItem *child )
{
  QgsDebugMsgLevel( "mName = " + child->mName, 2 );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  child->deleteLater();
  emit endRemoveItems();
}

QgsDataItem *QgsDataItem::removeChildItem( QgsDataItem *child )
{
  QgsDebugMsgLevel( "mName = " + child->mName, 2 );
  int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  if ( i < 0 )
  {
    child->setParent( nullptr );
    return nullptr;
  }

  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  emit endRemoveItems();
  return child;
}

int QgsDataItem::findItem( QVector<QgsDataItem *> items, QgsDataItem *item )
{
  for ( int i = 0; i < items.size(); i++ )
  {
    Q_ASSERT_X( items[i], "findItem", QStringLiteral( "item %1 is nullptr" ).arg( i ).toLatin1() );
    QgsDebugMsgLevel( QString::number( i ) + " : " + items[i]->mPath + " x " + item->mPath, 2 );
    if ( items[i]->equal( item ) )
      return i;
  }
  return -1;
}

bool QgsDataItem::equal( const QgsDataItem *other )
{
  return ( metaObject()->className() == other->metaObject()->className() &&
           mPath == other->path() );
}

QList<QAction *> QgsDataItem::actions( QWidget *parent )
{
  Q_UNUSED( parent );
  return QList<QAction *>();
}

bool QgsDataItem::handleDoubleClick()
{
  return false;
}

bool QgsDataItem::rename( const QString & )
{
  return false;
}

QgsDataItem::State QgsDataItem::state() const
{
  return mState;
}

void QgsDataItem::setState( State state )
{
  QgsDebugMsgLevel( QStringLiteral( "item %1 set state %2 -> %3" ).arg( path() ).arg( this->state() ).arg( state ), 3 );
  if ( state == mState )
    return;

  State oldState = mState;

  if ( state == Populating ) // start loading
  {
    if ( !sPopulatingIcon )
    {
      // TODO: ensure that QgsAnimatedIcon is created on UI thread only
      sPopulatingIcon = new QgsAnimatedIcon( QgsApplication::iconPath( QStringLiteral( "/mIconLoading.gif" ) ), QgsApplication::instance() );
    }

    sPopulatingIcon->connectFrameChanged( this, &QgsDataItem::updateIcon );
  }
  else if ( mState == Populating && sPopulatingIcon ) // stop loading
  {
    sPopulatingIcon->disconnectFrameChanged( this, &QgsDataItem::updateIcon );
  }


  mState = state;

  emit stateChanged( this, oldState );
  if ( state == Populated )
    updateIcon();
}

QList<QMenu *> QgsDataItem::menus( QWidget *parent )
{
  Q_UNUSED( parent );
  return QList<QMenu *>();
}

// ---------------------------------------------------------------------

QgsLayerItem::QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType, const QString &providerKey )
  : QgsDataItem( Layer, parent, name, path )
  , mProviderKey( providerKey )
  , mUri( uri )
  , mLayerType( layerType )
{
  mIconName = iconName( layerType );
}

QgsMapLayerType QgsLayerItem::mapLayerType() const
{
  switch ( mLayerType )
  {
    case QgsLayerItem::Raster:
      return QgsMapLayerType::RasterLayer;

    case QgsLayerItem::Mesh:
      return QgsMapLayerType::MeshLayer;

    case QgsLayerItem::Plugin:
      return QgsMapLayerType::PluginLayer;

    case QgsLayerItem::NoType:
    case QgsLayerItem::Vector:
    case QgsLayerItem::Point:
    case QgsLayerItem::Polygon:
    case QgsLayerItem::Line:
    case QgsLayerItem::TableLayer:
    case QgsLayerItem::Table:
    case QgsLayerItem::Database:
      return QgsMapLayerType::VectorLayer;
  }

  return QgsMapLayerType::VectorLayer; // no warnings
}

QgsLayerItem::LayerType QgsLayerItem::typeFromMapLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      switch ( qobject_cast< QgsVectorLayer * >( layer )->geometryType() )
      {
        case QgsWkbTypes::PointGeometry:
          return Point;

        case QgsWkbTypes::LineGeometry:
          return Line;

        case QgsWkbTypes::PolygonGeometry:
          return Polygon;

        case QgsWkbTypes::NullGeometry:
          return TableLayer;

        case QgsWkbTypes::UnknownGeometry:
          return Vector;
      }

      return Vector; // no warnings
    }

    case QgsMapLayerType::RasterLayer:
      return Raster;
    case QgsMapLayerType::PluginLayer:
      return Plugin;
    case QgsMapLayerType::MeshLayer:
      return Mesh;
  }
  return Vector; // no warnings
}

QString QgsLayerItem::layerTypeAsString( QgsLayerItem::LayerType layerType )
{
  static int enumIdx = staticMetaObject.indexOfEnumerator( "LayerType" );
  return staticMetaObject.enumerator( enumIdx ).valueToKey( layerType );
}

QString QgsLayerItem::iconName( QgsLayerItem::LayerType layerType )
{
  switch ( layerType )
  {
    case Point:
      return QStringLiteral( "/mIconPointLayer.svg" );
    case Line:
      return QStringLiteral( "/mIconLineLayer.svg" );
    case Polygon:
      return QStringLiteral( "/mIconPolygonLayer.svg" );
    // TODO add a new icon for generic Vector layers
    case Vector :
      return QStringLiteral( "/mIconVector.svg" );
    case TableLayer:
    case Table:
      return QStringLiteral( "/mIconTableLayer.svg" );
    case Raster:
      return QStringLiteral( "/mIconRaster.svg" );
    case Mesh:
      return QStringLiteral( "/mIconMeshLayer.svg" );
    default:
      return QStringLiteral( "/mIconLayer.png" );
  }
}

bool QgsLayerItem::deleteLayer()
{
  return false;
}

bool QgsLayerItem::equal( const QgsDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  //const QgsLayerItem *o = qobject_cast<const QgsLayerItem *> ( other );
  const QgsLayerItem *o = qobject_cast<const QgsLayerItem *>( other );
  if ( !o )
    return false;

  return ( mPath == o->mPath && mName == o->mName && mUri == o->mUri && mProviderKey == o->mProviderKey );
}

QgsMimeDataUtils::Uri QgsLayerItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;

  switch ( mapLayerType() )
  {
    case QgsMapLayerType::VectorLayer:
      u.layerType = QStringLiteral( "vector" );
      break;
    case QgsMapLayerType::RasterLayer:
      u.layerType = QStringLiteral( "raster" );
      break;
    case QgsMapLayerType::MeshLayer:
      u.layerType = QStringLiteral( "mesh" );
      break;
    case QgsMapLayerType::PluginLayer:
      u.layerType = QStringLiteral( "plugin" );
      break;
  }

  u.providerKey = providerKey();
  u.name = layerName();
  u.uri = uri();
  u.supportedCrs = supportedCrs();
  u.supportedFormats = supportedFormats();
  return u;
}

// ---------------------------------------------------------------------
QgsDataCollectionItem::QgsDataCollectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( Collection, parent, name, path )
{
  mCapabilities = Fertile;
  mIconName = QStringLiteral( "/mIconDbSchema.svg" );
}

QgsDataCollectionItem::~QgsDataCollectionItem()
{
  QgsDebugMsgLevel( "mName = " + mName + " mPath = " + mPath, 2 );

// Do not delete children, children are deleted by QObject parent
#if 0
  Q_FOREACH ( QgsDataItem *i, mChildren )
  {
    QgsDebugMsgLevel( QStringLiteral( "delete child = 0x%0" ).arg( static_cast<qlonglong>( i ), 8, 16, QLatin1Char( '0' ) ), 2 );
    delete i;
  }
#endif
}

//-----------------------------------------------------------------------

QgsDirectoryItem::QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, QDir::toNativeSeparators( name ), path )
  , mDirPath( path )
  , mRefreshLater( false )
{
  mType = Directory;
  init();
}

QgsDirectoryItem::QgsDirectoryItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path )
  : QgsDataCollectionItem( parent, QDir::toNativeSeparators( name ), path )
  , mDirPath( dirPath )
  , mRefreshLater( false )
{
  mType = Directory;
  init();
}

void QgsDirectoryItem::init()
{
  setToolTip( QDir::toNativeSeparators( mDirPath ) );
}

QIcon QgsDirectoryItem::icon()
{
  if ( mDirPath == QDir::homePath() )
    return homeDirIcon();

  // still loading? show the spinner
  if ( state() == Populating )
    return QgsDataItem::icon();

  // symbolic link? use link icon
  QFileInfo fi( mDirPath );
  if ( fi.isDir() && fi.isSymLink() )
  {
    return QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderLink.svg" ) );
  }

  // loaded? show the open dir icon
  if ( state() == Populated )
    return openDirIcon();

  // show the closed dir icon
  return iconDir();
}


QVector<QgsDataItem *> QgsDirectoryItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QDir dir( mDirPath );

  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  QStringList entries = dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  Q_FOREACH ( const QString &subdir, entries )
  {
    if ( mRefreshLater )
    {
      deleteLater( children );
      return children;
    }

    QString subdirPath = dir.absoluteFilePath( subdir );

    QgsDebugMsgLevel( QStringLiteral( "creating subdir: %1" ).arg( subdirPath ), 2 );

    QString path = mPath + '/' + subdir; // may differ from subdirPath
    if ( QgsDirectoryItem::hiddenPath( path ) )
      continue;

    bool handledByProvider = false;
    for ( QgsDataItemProvider *provider : providers )
    {
      if ( provider->handlesDirectoryPath( path ) )
      {
        handledByProvider = true;
        break;
      }
    }
    if ( handledByProvider )
      continue;

    QgsDirectoryItem *item = new QgsDirectoryItem( this, subdir, subdirPath, path );

    // we want directories shown before files
    item->setSortKey( QStringLiteral( "  %1" ).arg( subdir ) );

    // propagate signals up to top

    children.append( item );
  }

  QStringList fileEntries = dir.entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files, QDir::Name );
  Q_FOREACH ( const QString &name, fileEntries )
  {
    if ( mRefreshLater )
    {
      deleteLater( children );
      return children;
    }

    QString path = dir.absoluteFilePath( name );
    QFileInfo fileInfo( path );

    if ( fileInfo.suffix().compare( QLatin1String( "zip" ), Qt::CaseInsensitive ) == 0 ||
         fileInfo.suffix().compare( QLatin1String( "tar" ), Qt::CaseInsensitive ) == 0 )
    {
      QgsDataItem *item = QgsZipItem::itemFromPath( this, path, name, mPath + '/' + name );
      if ( item )
      {
        children.append( item );
        continue;
      }
    }

    bool createdItem = false;
    for ( QgsDataItemProvider *provider : providers )
    {
      int capabilities = provider->capabilities();

      if ( !( ( fileInfo.isFile() && ( capabilities & QgsDataProvider::File ) ) ||
              ( fileInfo.isDir() && ( capabilities & QgsDataProvider::Dir ) ) ) )
      {
        continue;
      }

      QgsDataItem *item = provider->createDataItem( path, this );
      if ( item )
      {
        children.append( item );
        createdItem = true;
      }
    }

    if ( !createdItem )
    {
      // if item is a QGIS project, and no specific item provider has overridden handling of
      // project items, then use the default project item behavior
      if ( fileInfo.suffix().compare( QLatin1String( "qgs" ), Qt::CaseInsensitive ) == 0 ||
           fileInfo.suffix().compare( QLatin1String( "qgz" ), Qt::CaseInsensitive ) == 0 )
      {
        QgsDataItem *item = new QgsProjectItem( this, fileInfo.completeBaseName(), path );
        children.append( item );
        continue;
      }
    }

  }
  return children;
}

void QgsDirectoryItem::setState( State state )
{
  QgsDataCollectionItem::setState( state );

  if ( state == Populated )
  {
    if ( !mFileSystemWatcher )
    {
      mFileSystemWatcher = new QFileSystemWatcher( this );
      mFileSystemWatcher->addPath( mDirPath );
      connect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
    }
    mLastScan = QDateTime::currentDateTime();
  }
  else if ( state == NotPopulated )
  {
    if ( mFileSystemWatcher )
    {
      delete mFileSystemWatcher;
      mFileSystemWatcher = nullptr;
    }
  }
}

void QgsDirectoryItem::directoryChanged()
{
  // If the last scan was less than 10 seconds ago, skip this
  if ( mLastScan.msecsTo( QDateTime::currentDateTime() ) < QgsSettings().value( QStringLiteral( "browser/minscaninterval" ), 10000 ).toInt() )
  {
    return;
  }
  if ( state() == Populating )
  {
    // schedule to refresh later, because refresh() simply returns if Populating
    mRefreshLater = true;
  }
  else
  {
    // We definintely don't want the temporary files created by sqlite
    // to re-trigger a refresh in an infinite loop.
    disconnect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
    // QFileSystemWhatcher::directoryChanged is emitted when a
    // file is created and not when it is closed/flushed.
    //
    // Delay to give to OS the time to complete writing the file
    // this happens when a new file appears in the directory and
    // the item's children thread will try to open the file with
    // GDAL or OGR even if it is still being written.
    QTimer::singleShot( 100, this, [ = ] { refresh(); } );
  }
}

bool QgsDirectoryItem::hiddenPath( const QString &path )
{
  QgsSettings settings;
  QStringList hiddenItems = settings.value( QStringLiteral( "browser/hiddenPaths" ),
                            QStringList() ).toStringList();
  int idx = hiddenItems.indexOf( path );
  return ( idx > -1 );
}

void QgsDirectoryItem::childrenCreated()
{
  QgsDebugMsgLevel( QStringLiteral( "mRefreshLater = %1" ).arg( mRefreshLater ), 3 );

  if ( mRefreshLater )
  {
    QgsDebugMsgLevel( QStringLiteral( "directory changed during createChidren() -> refresh() again" ), 3 );
    mRefreshLater = false;
    setState( Populated );
    refresh();
  }
  else
  {
    QgsDataCollectionItem::childrenCreated();
  }
  // Re-connect the file watcher after all children have been created
  connect( mFileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &QgsDirectoryItem::directoryChanged );
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

QWidget *QgsDirectoryItem::paramWidget()
{
  return new QgsDirectoryParamWidget( mPath );
}

QgsMimeDataUtils::Uri QgsDirectoryItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "directory" );
  u.name = mName;
  u.uri = mDirPath;
  return u;
}

QgsDirectoryParamWidget::QgsDirectoryParamWidget( const QString &path, QWidget *parent )
  : QTreeWidget( parent )
{
  setRootIsDecorated( false );

  // name, size, date, permissions, owner, group, type
  setColumnCount( 7 );
  QStringList labels;
  labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
  setHeaderLabels( labels );

  QIcon iconDirectory = QgsApplication::getThemeIcon( QStringLiteral( "mIconFolder.svg" ) );
  QIcon iconFile = QgsApplication::getThemeIcon( QStringLiteral( "mIconFile.svg" ) );
  QIcon iconDirLink = QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderLink.svg" ) );
  QIcon iconFileLink = QgsApplication::getThemeIcon( QStringLiteral( "mIconFileLink.svg" ) );

  QList<QTreeWidgetItem *> items;

  QDir dir( path );
  QStringList entries = dir.entryList( QDir::AllEntries | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase );
  Q_FOREACH ( const QString &name, entries )
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
      size = QStringLiteral( "%1 B" ).arg( fi.size() );
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
    if ( fi.isDir() && fi.isSymLink() )
    {
      type = tr( "folder" );
      icon = iconDirLink;
    }
    else if ( fi.isDir() )
    {
      type = tr( "folder" );
      icon = iconDirectory;
    }
    else if ( fi.isFile() && fi.isSymLink() )
    {
      type = tr( "file" );
      icon = iconFileLink;
    }
    else if ( fi.isFile() )
    {
      type = tr( "file" );
      icon = iconFile;
    }

    texts << type;

    QTreeWidgetItem *item = new QTreeWidgetItem( texts );
    item->setIcon( 0, icon );
    items << item;
  }

  addTopLevelItems( items );

  // hide columns that are not requested
  QgsSettings settings;
  QList<QVariant> lst = settings.value( QStringLiteral( "dataitem/directoryHiddenColumns" ) ).toList();
  Q_FOREACH ( const QVariant &colVariant, lst )
  {
    setColumnHidden( colVariant.toInt(), true );
  }
}

void QgsDirectoryParamWidget::mousePressEvent( QMouseEvent *event )
{
  if ( event->button() == Qt::RightButton )
  {
    // show the popup menu
    QMenu popupMenu;

    QStringList labels;
    labels << tr( "Name" ) << tr( "Size" ) << tr( "Date" ) << tr( "Permissions" ) << tr( "Owner" ) << tr( "Group" ) << tr( "Type" );
    for ( int i = 0; i < labels.count(); i++ )
    {
      QAction *action = popupMenu.addAction( labels[i], this, &QgsDirectoryParamWidget::showHideColumn );
      action->setObjectName( QString::number( i ) );
      action->setCheckable( true );
      action->setChecked( !isColumnHidden( i ) );
    }

    popupMenu.exec( event->globalPos() );
  }
}

void QgsDirectoryParamWidget::showHideColumn()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return; // something is wrong

  int columnIndex = action->objectName().toInt();
  setColumnHidden( columnIndex, !isColumnHidden( columnIndex ) );

  // save in settings
  QgsSettings settings;
  QList<QVariant> lst;
  for ( int i = 0; i < columnCount(); i++ )
  {
    if ( isColumnHidden( i ) )
      lst.append( QVariant( i ) );
  }
  settings.setValue( QStringLiteral( "dataitem/directoryHiddenColumns" ), lst );
}

QgsProjectItem::QgsProjectItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataItem( QgsDataItem::Project, parent, name, path )
{
  mIconName = QStringLiteral( ":/images/icons/qgis_icon.svg" );
  setToolTip( QDir::toNativeSeparators( path ) );
  setState( Populated ); // no more children
}

QgsMimeDataUtils::Uri QgsProjectItem::mimeUri() const
{
  QgsMimeDataUtils::Uri u;
  u.layerType = QStringLiteral( "project" );
  u.name = mName;
  u.uri = mPath;
  return u;
}

QgsErrorItem::QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path )
  : QgsDataItem( QgsDataItem::Error, parent, error, path )
{
  mIconName = QStringLiteral( "/mIconDelete.svg" );

  setState( Populated ); // no more children
}

QgsFavoritesItem::QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "favorites:" ) )
{
  Q_UNUSED( path );
  mCapabilities |= Fast;
  mType = Favorites;
  mIconName = QStringLiteral( "/mIconFavourites.svg" );
  populate();
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren()
{
  QVector<QgsDataItem *> children;

  QgsSettings settings;
  const QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ), QVariant() ).toStringList();

  for ( const QString &favDir : favDirs )
  {
    QStringList parts = favDir.split( QStringLiteral( "|||" ) );
    if ( parts.empty() )
      continue;

    QString dir = parts.at( 0 );
    QString name = dir;
    if ( parts.count() > 1 )
      name = parts.at( 1 );

    children << createChildren( dir, name );
  }

  return children;
}

void QgsFavoritesItem::addDirectory( const QString &favDir, const QString &n )
{
  QString name = n.isEmpty() ? favDir : n;

  QgsSettings settings;
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  favDirs.append( QStringLiteral( "%1|||%2" ).arg( favDir, name ) );
  settings.setValue( QStringLiteral( "browser/favourites" ), favDirs );

  if ( state() == Populated )
  {
    QVector<QgsDataItem *> items = createChildren( favDir, name );
    Q_FOREACH ( QgsDataItem *item, items )
    {
      addChildItem( item, true );
    }
  }
}

void QgsFavoritesItem::removeDirectory( QgsDirectoryItem *item )
{
  if ( !item )
    return;

  QgsSettings settings;
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  for ( int i = favDirs.count() - 1; i >= 0; --i )
  {
    QStringList parts = favDirs.at( i ).split( QStringLiteral( "|||" ) );
    if ( parts.empty() )
      continue;

    QString dir = parts.at( 0 );
    if ( dir == item->dirPath() )
      favDirs.removeAt( i );
  }
  settings.setValue( QStringLiteral( "browser/favourites" ), favDirs );

  int idx = findItem( mChildren, item );
  if ( idx < 0 )
  {
    QgsDebugMsg( QStringLiteral( "favorites item %1 not found" ).arg( item->path() ) );
    return;
  }

  if ( state() == Populated )
    deleteChildItem( mChildren.at( idx ) );
}

void QgsFavoritesItem::renameFavorite( const QString &path, const QString &name )
{
  // update stored name
  QgsSettings settings;
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  for ( int i = 0; i < favDirs.count(); ++i )
  {
    QStringList parts = favDirs.at( i ).split( QStringLiteral( "|||" ) );
    if ( parts.empty() )
      continue;

    QString dir = parts.at( 0 );
    if ( dir == path )
    {
      favDirs[i] = QStringLiteral( "%1|||%2" ).arg( path, name );
      break;
    }
  }
  settings.setValue( QStringLiteral( "browser/favourites" ), favDirs );

  // also update existing data item
  const QVector<QgsDataItem *> ch = children();
  for ( QgsDataItem *child : ch )
  {
    if ( QgsFavoriteItem *favorite = qobject_cast< QgsFavoriteItem * >( child ) )
    {
      if ( favorite->dirPath() == path )
      {
        favorite->setName( name );
        break;
      }
    }
  }
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren( const QString &favDir, const QString &name )
{
  QVector<QgsDataItem *> children;
  QString pathName = pathComponent( favDir );
  Q_FOREACH ( QgsDataItemProvider *provider, QgsApplication::dataItemProviderRegistry()->providers() )
  {
    int capabilities = provider->capabilities();

    if ( capabilities & QgsDataProvider::Dir )
    {
      QgsDataItem *item = provider->createDataItem( favDir, this );
      if ( item )
      {
        item->setName( name );
        children.append( item );
      }
    }
  }
  if ( children.isEmpty() )
  {
    QgsFavoriteItem *item = new QgsFavoriteItem( this, name, favDir, mPath + '/' + pathName );
    if ( item )
    {
      children.append( item );
    }
  }
  return children;
}

//-----------------------------------------------------------------------
QStringList QgsZipItem::sProviderNames = QStringList();


QgsZipItem::QgsZipItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
{
  mFilePath = path;
  init();
}

QgsZipItem::QgsZipItem( QgsDataItem *parent, const QString &name, const QString &filePath, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
  , mFilePath( filePath )
{
  init();
}

void QgsZipItem::init()
{
  mType = Collection; //Zip??
  mIconName = QStringLiteral( "/mIconZip.svg" );
  mVsiPrefix = vsiPrefix( mFilePath );

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]
  {
    sProviderNames << QStringLiteral( "OGR" ) << QStringLiteral( "GDAL" );
  } );
}

QVector<QgsDataItem *> QgsZipItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QString tmpPath;
  QgsSettings settings;
  QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();

  mZipFileList.clear();

  QgsDebugMsgLevel( QStringLiteral( "mFilePath = %1 path = %2 name= %3 scanZipSetting= %4 vsiPrefix= %5" ).arg( mFilePath, path(), name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == QLatin1String( "no" ) )
  {
    return children;
  }

  // first get list of files
  getZipFileList();

  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  // loop over files inside zip
  Q_FOREACH ( const QString &fileName, mZipFileList )
  {
    QFileInfo info( fileName );
    tmpPath = mVsiPrefix + mFilePath + '/' + fileName;
    QgsDebugMsgLevel( "tmpPath = " + tmpPath, 3 );

    for ( QgsDataItemProvider *provider : providers )
    {
      if ( !sProviderNames.contains( provider->name() ) )
        continue;

      // ugly hack to remove .dbf file if there is a .shp file
      if ( provider->name() == QStringLiteral( "OGR" ) )
      {
        if ( info.suffix().compare( QLatin1String( "dbf" ), Qt::CaseInsensitive ) == 0 )
        {
          if ( mZipFileList.indexOf( fileName.left( fileName.count() - 4 ) + ".shp" ) != -1 )
            continue;
        }
        if ( info.completeSuffix().compare( QLatin1String( "shp.xml" ), Qt::CaseInsensitive ) == 0 )
        {
          continue;
        }
      }

      QgsDebugMsgLevel( QStringLiteral( "trying to load item %1 with %2" ).arg( tmpPath, provider->name() ), 3 );
      QgsDataItem *item = provider->createDataItem( tmpPath, this );
      if ( item )
      {
        // the item comes with zipped file name, set the name to relative path within zip file
        item->setName( fileName );
        children.append( item );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "not loaded item" ), 3 );
      }
    }
  }

  return children;
}

QgsDataItem *QgsZipItem::itemFromPath( QgsDataItem *parent, const QString &path, const QString &name )
{
  return itemFromPath( parent, path, name, path );
}

QgsDataItem *QgsZipItem::itemFromPath( QgsDataItem *parent, const QString &filePath, const QString &name, const QString &path )
{
  QgsSettings settings;
  QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();
  QStringList zipFileList;
  QString vsiPrefix = QgsZipItem::vsiPrefix( filePath );
  QgsZipItem *zipItem = nullptr;
  bool populated = false;

  QgsDebugMsgLevel( QStringLiteral( "path = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4" ).arg( path, name, scanZipSetting, vsiPrefix ), 3 );

  // don't scan if scanZipBrowser == no
  if ( scanZipSetting == QLatin1String( "no" ) )
    return nullptr;

  // don't scan if this file is not a /vsizip/ or /vsitar/ item
  if ( ( vsiPrefix != QLatin1String( "/vsizip/" ) && vsiPrefix != QLatin1String( "/vsitar/" ) ) )
    return nullptr;

  zipItem = new QgsZipItem( parent, name, filePath, path );

  if ( zipItem )
  {
    // force populate zipItem if it has less than 10 items and is not a .tgz or .tar.gz file (slow loading)
    // for other items populating will be delayed until item is opened
    // this might be polluting the tree with empty items but is necessary for performance reasons
    // could also accept all files smaller than a certain size and add options for file count and/or size

    // first get list of files inside .zip or .tar files
    if ( path.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) ||
         path.endsWith( QLatin1String( ".tar" ), Qt::CaseInsensitive ) )
    {
      zipFileList = zipItem->getZipFileList();
    }
    // force populate if less than 10 items
    if ( !zipFileList.isEmpty() && zipFileList.count() <= 10 )
    {
      zipItem->populate( zipItem->createChildren() );
      populated = true; // there is no QgsDataItem::isPopulated() function
      QgsDebugMsgLevel( QStringLiteral( "Got zipItem with %1 children, path=%2, name=%3" ).arg( zipItem->rowCount() ).arg( zipItem->path(), zipItem->name() ), 3 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Delaying populating zipItem with path=%1, name=%2" ).arg( zipItem->path(), zipItem->name() ), 3 );
    }
  }

  // only display if has children or if is not populated
  if ( zipItem && ( !populated || zipItem->rowCount() > 0 ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "returning zipItem" ), 3 );
    return zipItem;
  }

  return nullptr;
}

QStringList QgsZipItem::getZipFileList()
{
  if ( ! mZipFileList.isEmpty() )
    return mZipFileList;

  QString tmpPath;
  QgsSettings settings;
  QString scanZipSetting = settings.value( QStringLiteral( "qgis/scanZipInBrowser2" ), "basic" ).toString();

  QgsDebugMsgLevel( QStringLiteral( "mFilePath = %1 name= %2 scanZipSetting= %3 vsiPrefix= %4" ).arg( mFilePath, name(), scanZipSetting, mVsiPrefix ), 3 );

  // if scanZipBrowser == no: skip to the next file
  if ( scanZipSetting == QLatin1String( "no" ) )
  {
    return mZipFileList;
  }

  // get list of files inside zip file
  QgsDebugMsgLevel( QStringLiteral( "Open file %1 with gdal vsi" ).arg( mVsiPrefix + mFilePath ), 3 );
  char **papszSiblingFiles = VSIReadDirRecursive( QString( mVsiPrefix + mFilePath ).toLocal8Bit().constData() );
  if ( papszSiblingFiles )
  {
    for ( int i = 0; papszSiblingFiles[i]; i++ )
    {
      tmpPath = papszSiblingFiles[i];
      QgsDebugMsgLevel( QStringLiteral( "Read file %1" ).arg( tmpPath ), 3 );
      // skip directories (files ending with /)
      if ( tmpPath.right( 1 ) != QLatin1String( "/" ) )
        mZipFileList << tmpPath;
    }
    CSLDestroy( papszSiblingFiles );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Error reading %1" ).arg( mFilePath ) );
  }

  return mZipFileList;
}

///@cond PRIVATE

QgsProjectHomeItem::QgsProjectHomeItem( QgsDataItem *parent, const QString &name, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, name, dirPath, path )
{
}

QIcon QgsProjectHomeItem::icon()
{
  if ( state() == Populating )
    return QgsDirectoryItem::icon();
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconFolderProject.svg" ) );
}

QVariant QgsProjectHomeItem::sortKey() const
{
  return QStringLiteral( " 1" );
}


QgsFavoriteItem::QgsFavoriteItem( QgsFavoritesItem *parent, const QString &name, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, name, dirPath, path )
  , mFavorites( parent )
{
  mCapabilities |= Rename;
}

bool QgsFavoriteItem::rename( const QString &name )
{
  mFavorites->renameFavorite( dirPath(), name );
  return true;
}


///@endcond
