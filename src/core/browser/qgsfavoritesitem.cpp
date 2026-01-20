/***************************************************************************
                           qgsfavoritesitem.cpp
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

#include "qgsfavoritesitem.h"

#include "qgsapplication.h"
#include "qgsdataitemprovider.h"
#include "qgsdataitemproviderregistry.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include "moc_qgsfavoritesitem.cpp"

//
// QgsFavoritesItem
//

QgsFavoritesItem::QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, u"favorites:"_s, u"special:Favorites"_s )
{
  Q_UNUSED( path )
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mType = Qgis::BrowserItemType::Favorites;
  mIconName = u"/mIconFavorites.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QgsSettings settings;

  const QStringList favDirs = settings.value( u"browser/favourites"_s, QVariant() ).toStringList();

  children.reserve( favDirs.size() );
  for ( const QString &favDir : favDirs )
  {
    const QStringList parts = favDir.split( u"|||"_s );
    if ( parts.empty() )
      continue;

    const QString dir = parts.at( 0 );
    QString name = dir;
    if ( parts.count() > 1 )
      name = parts.at( 1 );

    children << createChildren( dir, name );
  }

  return children;
}

void QgsFavoritesItem::addDirectory( const QString &favDir, const QString &n )
{
  const QString name = n.isEmpty() ? favDir : n;

  QgsSettings settings;
  QStringList favDirs = settings.value( u"browser/favourites"_s ).toStringList();
  favDirs.append( u"%1|||%2"_s.arg( favDir, name ) );
  settings.setValue( u"browser/favourites"_s, favDirs );

  if ( state() == Qgis::BrowserItemState::Populated )
  {
    const QVector<QgsDataItem *> items = createChildren( favDir, name );
    for ( QgsDataItem *item : items )
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
  QStringList favDirs = settings.value( u"browser/favourites"_s ).toStringList();
  for ( int i = favDirs.count() - 1; i >= 0; --i )
  {
    const QStringList parts = favDirs.at( i ).split( u"|||"_s );
    if ( parts.empty() )
      continue;

    const QString dir = parts.at( 0 );
    if ( dir == item->dirPath() )
      favDirs.removeAt( i );
  }
  settings.setValue( u"browser/favourites"_s, favDirs );

  const int idx = findItem( mChildren, item );
  if ( idx < 0 )
  {
    QgsDebugError( u"favorites item %1 not found"_s.arg( item->path() ) );
    return;
  }

  if ( state() == Qgis::BrowserItemState::Populated )
    deleteChildItem( mChildren.at( idx ) );
}

void QgsFavoritesItem::renameFavorite( const QString &path, const QString &name )
{
  // update stored name
  QgsSettings settings;
  QStringList favDirs = settings.value( u"browser/favourites"_s ).toStringList();
  for ( int i = 0; i < favDirs.count(); ++i )
  {
    const QStringList parts = favDirs.at( i ).split( u"|||"_s );
    if ( parts.empty() )
      continue;

    const QString dir = parts.at( 0 );
    if ( dir == path )
    {
      const QStringList newParts { path, name };
      favDirs[i] = newParts.join( "|||"_L1 );
      break;
    }
  }
  settings.setValue( u"browser/favourites"_s, favDirs );

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

QIcon QgsFavoritesItem::iconFavorites()
{
  return QgsApplication::getThemeIcon( u"/mIconFavorites.svg"_s );
}

QVariant QgsFavoritesItem::sortKey() const
{
  return u" 0"_s;
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren( const QString &directory, const QString &name )
{
  const QString pathName = pathComponent( directory );
  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  QVector<QgsDataItem *> children;
  children.reserve( providers.size() );
  for ( QgsDataItemProvider *provider : providers )
  {
    if ( provider->capabilities() & Qgis::DataItemProviderCapability::Directories )
    {
      if ( QgsDataItem *item = provider->createDataItem( directory, this ) )
      {
        item->setName( name );
        children.append( item );
      }
    }
  }
  if ( children.isEmpty() )
  {
    children.append( new QgsFavoriteItem( this, name, directory, mPath + '/' + pathName ) );
  }
  return children;
}

//
// QgsFavoriteItem
//

QgsFavoriteItem::QgsFavoriteItem( QgsFavoritesItem *parent, const QString &name, const QString &dirPath, const QString &path )
  : QgsDirectoryItem( parent, name, dirPath, path, u"special:Favorites"_s )
  , mFavorites( parent )
{
  mCapabilities |= Qgis::BrowserItemCapability::Rename;
}

bool QgsFavoriteItem::rename( const QString &name )
{
  mFavorites->renameFavorite( dirPath(), name );
  return true;
}


