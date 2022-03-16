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
#include "qgsprovidermetadata.h"

#include <QApplication>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QDateTime>
#include <QElapsedTimer>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QStyle>
#include <QTimer>
#include <mutex>
#include <QRegularExpression>

// use GDAL VSI mechanism
#define CPL_SUPRESS_CPLUSPLUS  //#spellok
#include "cpl_vsi.h"
#include "cpl_string.h"

QgsAnimatedIcon *QgsDataItem::sPopulatingIcon = nullptr;

QgsDataItem::QgsDataItem( Qgis::BrowserItemType type, QgsDataItem *parent, const QString &name, const QString &path, const QString &providerKey )
// Do not pass parent to QObject, Qt would delete this when parent is deleted
  : mType( type )
  , mParent( parent )
  , mName( name )
  , mProviderKey( providerKey )
  , mPath( path )
{
}

QgsDataItem::~QgsDataItem()
{
  QgsDebugMsgLevel( QStringLiteral( "mName = %1 mPath = %2 mChildren.size() = %3" ).arg( mName, mPath ).arg( mChildren.size() ), 2 );
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
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
  const thread_local QRegularExpression rx( "[\\\\/]" );
  return QString( string ).replace( rx, QStringLiteral( "|" ) );
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
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
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
  const auto constItems = items;
  for ( QgsDataItem *item : constItems )
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
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    if ( !child ) // should not happen
      continue;
    QgsDebugMsgLevel( "moveToThread child " + child->path(), 3 );
    child->QObject::setParent( nullptr ); // to be sure
    child->moveToThread( targetThread );
  }
  QObject::moveToThread( targetThread );
}

QgsAbstractDatabaseProviderConnection *QgsDataItem::databaseConnection() const
{
  return nullptr;
}

QIcon QgsDataItem::icon()
{
  if ( state() == Qgis::BrowserItemState::Populating && sPopulatingIcon )
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
  if ( state() == Qgis::BrowserItemState::Populated || state() == Qgis::BrowserItemState::Populating )
    return;

  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  if ( capabilities2() & Qgis::BrowserItemCapability::Fast || foreground )
  {
    populate( createChildren() );
  }
  else
  {
    setState( Qgis::BrowserItemState::Populating );
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
  QElapsedTimer time;
  time.start();
  QVector <QgsDataItem *> children = item->createChildren();
  QgsDebugMsgLevel( QStringLiteral( "%1 children created in %2 ms" ).arg( children.size() ).arg( time.elapsed() ), 3 );
  // Children objects must be pushed to main thread.
  const auto constChildren = children;
  for ( QgsDataItem *child : constChildren )
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

  const auto constChildren = children;
  for ( QgsDataItem *child : constChildren )
  {
    if ( !child ) // should not happen
      continue;
    // update after thread finished -> refresh
    addChildItem( child, true );
  }
  setState( Qgis::BrowserItemState::Populated );
}

void QgsDataItem::depopulate()
{
  QgsDebugMsgLevel( "mPath = " + mPath, 3 );

  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    QgsDebugMsgLevel( "remove " + child->path(), 3 );
    child->depopulate(); // recursive
    deleteChildItem( child );
  }
  setState( Qgis::BrowserItemState::NotPopulated );
}

void QgsDataItem::refresh()
{
  if ( state() == Qgis::BrowserItemState::Populating )
    return;

  QgsDebugMsgLevel( "mPath = " + mPath, 3 );

  if ( capabilities2() & Qgis::BrowserItemCapability::Fast )
  {
    refresh( createChildren() );
  }
  else
  {
    setState( Qgis::BrowserItemState::Populating );
    if ( !mFutureWatcher )
    {
      mFutureWatcher = new QFutureWatcher< QVector <QgsDataItem *> >( this );
    }
    connect( mFutureWatcher, &QFutureWatcherBase::finished, this, &QgsDataItem::childrenCreated );
    mFutureWatcher->setFuture( QtConcurrent::run( runCreateChildren, this ) );
  }
}

void QgsDataItem::refreshConnections( const QString &key )
{
  // Walk up until the root node is reached
  if ( mParent )
  {
    mParent->refreshConnections( key );
  }
  else
  {
    // if a specific key was specified then we use that -- otherwise we assume the connections
    // changed belong to the same provider as this item
    emit connectionsChanged( key.isEmpty() ? providerKey() : key );
  }
}

void QgsDataItem::refresh( const QVector<QgsDataItem *> &children )
{
  QgsDebugMsgLevel( "mPath = " + mPath, 2 );

  // Remove no more present children
  QVector<QgsDataItem *> remove;
  const auto constMChildren = mChildren;
  for ( QgsDataItem *child : constMChildren )
  {
    if ( !child ) // should not happen
      continue;
    if ( findItem( children, child ) >= 0 )
      continue;
    remove.append( child );
  }
  const auto constRemove = remove;
  for ( QgsDataItem *child : constRemove )
  {
    QgsDebugMsgLevel( "remove " + child->path(), 3 );
    deleteChildItem( child );
  }

  // Add new children
  const auto constChildren = children;
  for ( QgsDataItem *child : constChildren )
  {
    if ( !child ) // should not happen
      continue;

    const int index = findItem( mChildren, child );
    if ( index >= 0 )
    {
      // Refresh recursively (some providers may create more generations of descendants)
      if ( !( child->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
      {
        // The child cannot createChildren() itself
        mChildren.value( index )->refresh( child->children() );
      }
      else if ( mChildren.value( index )->state() == Qgis::BrowserItemState::Populated
                && ( child->capabilities2() & Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed ) )
      {
        mChildren.value( index )->refresh();
      }

      child->deleteLater();
      continue;
    }
    addChildItem( child, true );
  }
  setState( Qgis::BrowserItemState::Populated );
}

QString QgsDataItem::providerKey() const
{
  return mProviderKey;
}

void QgsDataItem::setProviderKey( const QString &value )
{
  mProviderKey = value;
}

int QgsDataItem::rowCount()
{
  return mChildren.size();
}
bool QgsDataItem::hasChildren()
{
  return ( state() == Qgis::BrowserItemState::Populated ? !mChildren.isEmpty() : true );
}

bool QgsDataItem::layerCollection() const
{
  return false;
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
  QgsDebugMsgLevel( QStringLiteral( "path = %1 add child #%2 - %3 - %4" ).arg( mPath ).arg( mChildren.size() ).arg( child->mName ).arg( qgsEnumValueToKey< Qgis::BrowserItemType >( child->mType ) ), 3 );

  //calculate position to insert child
  int i;
  if ( type() == Qgis::BrowserItemType::Directory )
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
  const int i = mChildren.indexOf( child );
  Q_ASSERT( i >= 0 );
  emit beginRemoveItems( this, i, i );
  mChildren.remove( i );
  child->deleteLater();
  emit endRemoveItems();
}

QgsDataItem *QgsDataItem::removeChildItem( QgsDataItem *child )
{
  QgsDebugMsgLevel( "mName = " + child->mName, 2 );
  const int i = mChildren.indexOf( child );
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
  Q_UNUSED( parent )
  return QList<QAction *>();
}

bool QgsDataItem::handleDoubleClick()
{
  return false;
}

QgsMimeDataUtils::Uri QgsDataItem::mimeUri() const
{
  return mimeUris().isEmpty() ? QgsMimeDataUtils::Uri() : mimeUris().constFirst();
}

QgsMimeDataUtils::UriList QgsDataItem::mimeUris() const
{
  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    QgsMimeDataUtils::Uri uri;
    uri.uri = path();
    uri.filePath = path();
    return { uri };
  }

  return {};
}

bool QgsDataItem::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  Q_UNUSED( crs )
  return false;
}

bool QgsDataItem::rename( const QString & )
{
  return false;
}

void QgsDataItem::setCapabilities( int capabilities )
{
  setCapabilities( static_cast< Qgis::BrowserItemCapabilities >( capabilities ) );
}

Qgis::BrowserItemState QgsDataItem::state() const
{
  return mState;
}

void QgsDataItem::setState( Qgis::BrowserItemState state )
{
  QgsDebugMsgLevel( QStringLiteral( "item %1 set state %2 -> %3" ).arg( path() ).arg( qgsEnumValueToKey< Qgis::BrowserItemState >( this->state() ) ).arg( qgsEnumValueToKey< Qgis::BrowserItemState >( state ) ), 3 );
  if ( state == mState )
    return;

  const Qgis::BrowserItemState oldState = mState;

  if ( state == Qgis::BrowserItemState::Populating ) // start loading
  {
    if ( !sPopulatingIcon )
    {
      // TODO: ensure that QgsAnimatedIcon is created on UI thread only
      sPopulatingIcon = new QgsAnimatedIcon( QgsApplication::iconPath( QStringLiteral( "/mIconLoading.gif" ) ), QgsApplication::instance() );
    }

    sPopulatingIcon->connectFrameChanged( this, &QgsDataItem::updateIcon );
  }
  else if ( mState == Qgis::BrowserItemState::Populating && sPopulatingIcon ) // stop loading
  {
    sPopulatingIcon->disconnectFrameChanged( this, &QgsDataItem::updateIcon );
  }


  mState = state;

  emit stateChanged( this, oldState );
  if ( state == Qgis::BrowserItemState::Populated )
    updateIcon();
}

QList<QMenu *> QgsDataItem::menus( QWidget *parent )
{
  Q_UNUSED( parent )
  return QList<QMenu *>();
}

QgsErrorItem::QgsErrorItem( QgsDataItem *parent, const QString &error, const QString &path )
  : QgsDataItem( Qgis::BrowserItemType::Error, parent, error, path )
{
  mIconName = QStringLiteral( "/mIconDelete.svg" );

  setState( Qgis::BrowserItemState::Populated ); // no more children
}

