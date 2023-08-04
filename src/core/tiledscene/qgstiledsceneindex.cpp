/***************************************************************************
                         qgstiledsceneindex.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledsceneindex.h"
#include "qgsfeedback.h"
#include "qgstiledscenetile.h"
#include "qgsreadwritelocker.h"

//
// QgsAbstractTiledSceneIndex
//

QgsAbstractTiledSceneIndex::QgsAbstractTiledSceneIndex()
{
  mContentCache.setMaxCost( 10000 );
}

QgsAbstractTiledSceneIndex::~QgsAbstractTiledSceneIndex() = default;

QByteArray QgsAbstractTiledSceneIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mCacheLock, QgsReadWriteLocker::Read );
  if ( QByteArray *cachedData = mContentCache.object( uri ) )
  {
    return *cachedData;
  }
  locker.unlock();

  const QByteArray res = fetchContent( uri, feedback );
  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  locker.changeMode( QgsReadWriteLocker::Write );
  mContentCache.insert( uri, new QByteArray( res ) );
  return res;
}

//
// QgsTiledSceneIndex
//

QgsTiledSceneIndex::QgsTiledSceneIndex( QgsAbstractTiledSceneIndex *index )
  : mIndex( index )
{

}

QgsTiledSceneIndex::~QgsTiledSceneIndex() = default;

QgsTiledSceneIndex::QgsTiledSceneIndex( const QgsTiledSceneIndex &other )
  : mIndex( other.mIndex )
{

}

QgsTiledSceneIndex &QgsTiledSceneIndex::operator=( const QgsTiledSceneIndex &other )
{
  if ( this == &other )
    return *this;

  mIndex = other.mIndex;
  return *this;
}

bool QgsTiledSceneIndex::isValid() const
{
  return static_cast< bool >( mIndex.get() );
}

QgsTiledSceneTile QgsTiledSceneIndex::rootTile() const
{
  if ( !mIndex )
    return QgsTiledSceneTile();

  return mIndex->rootTile();
}

QgsTiledSceneTile QgsTiledSceneIndex::getTile( const QString &id )
{
  if ( !mIndex )
    return QgsTiledSceneTile();

  return mIndex->getTile( id );
}

QString QgsTiledSceneIndex::parentTileId( const QString &id ) const
{
  if ( !mIndex )
    return QString();

  return mIndex->parentTileId( id );
}

QStringList QgsTiledSceneIndex::childTileIds( const QString &id ) const
{
  if ( !mIndex )
    return QStringList();

  return mIndex->childTileIds( id );
}

QStringList QgsTiledSceneIndex::getTiles( const QgsTiledSceneRequest &request )
{
  if ( !mIndex )
    return {};

  return mIndex->getTiles( request );
}

Qgis::TileChildrenAvailability QgsTiledSceneIndex::childAvailability( const QString &id ) const
{
  if ( !mIndex )
    return Qgis::TileChildrenAvailability::NoChildren;

  return mIndex->childAvailability( id );
}

bool QgsTiledSceneIndex::fetchHierarchy( const QString &id, QgsFeedback *feedback )
{
  if ( !mIndex )
    return {};

  return mIndex->fetchHierarchy( id, feedback );
}

QByteArray QgsTiledSceneIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
{
  if ( !mIndex )
    return QByteArray();

  return mIndex->retrieveContent( uri, feedback );
}

