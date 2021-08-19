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
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"

//
// QgsFavoritesItem
//

QgsFavoritesItem::QgsFavoritesItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, QStringLiteral( "favorites:" ), QStringLiteral( "special:Favorites" ) )
{
  Q_UNUSED( path )
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mType = Qgis::BrowserItemType::Favorites;
  mIconName = QStringLiteral( "/mIconFavorites.svg" );
  populate();
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QgsSettings settings;

  const QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ), QVariant() ).toStringList();

  children.reserve( favDirs.size() );
  for ( const QString &favDir : favDirs )
  {
    const QStringList parts = favDir.split( QStringLiteral( "|||" ) );
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
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  favDirs.append( QStringLiteral( "%1|||%2" ).arg( favDir, name ) );
  settings.setValue( QStringLiteral( "browser/favourites" ), favDirs );

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
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  for ( int i = favDirs.count() - 1; i >= 0; --i )
  {
    const QStringList parts = favDirs.at( i ).split( QStringLiteral( "|||" ) );
    if ( parts.empty() )
      continue;

    const QString dir = parts.at( 0 );
    if ( dir == item->dirPath() )
      favDirs.removeAt( i );
  }
  settings.setValue( QStringLiteral( "browser/favourites" ), favDirs );

  const int idx = findItem( mChildren, item );
  if ( idx < 0 )
  {
    QgsDebugMsg( QStringLiteral( "favorites item %1 not found" ).arg( item->path() ) );
    return;
  }

  if ( state() == Qgis::BrowserItemState::Populated )
    deleteChildItem( mChildren.at( idx ) );
}

void QgsFavoritesItem::renameFavorite( const QString &path, const QString &name )
{
  // update stored name
  QgsSettings settings;
  QStringList favDirs = settings.value( QStringLiteral( "browser/favourites" ) ).toStringList();
  for ( int i = 0; i < favDirs.count(); ++i )
  {
    const QStringList parts = favDirs.at( i ).split( QStringLiteral( "|||" ) );
    if ( parts.empty() )
      continue;

    const QString dir = parts.at( 0 );
    if ( dir == path )
    {
      const QStringList newParts { path, name };
      favDirs[i] = newParts.join( QLatin1String( "|||" ) );
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

QIcon QgsFavoritesItem::iconFavorites()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFavorites.svg" ) );
}

QVariant QgsFavoritesItem::sortKey() const
{
  return QStringLiteral( " 0" );
}

QVector<QgsDataItem *> QgsFavoritesItem::createChildren( const QString &directory, const QString &name )
{
  const QString pathName = pathComponent( directory );
  const QList<QgsDataItemProvider *> providers = QgsApplication::dataItemProviderRegistry()->providers();

  QVector<QgsDataItem *> children;
  children.reserve( providers.size() );
  for ( QgsDataItemProvider *provider : providers )
  {
    if ( provider->capabilities() & QgsDataProvider::Dir )
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
  : QgsDirectoryItem( parent, name, dirPath, path, QStringLiteral( "special:Favorites" ) )
  , mFavorites( parent )
{
  mCapabilities |= Qgis::BrowserItemCapability::Rename;
}

bool QgsFavoriteItem::rename( const QString &name )
{
  mFavorites->renameFavorite( dirPath(), name );
  return true;
}


