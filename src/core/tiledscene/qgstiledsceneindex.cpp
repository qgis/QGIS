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
  QMutexLocker locker( &mCacheMutex );
  if ( QByteArray *cachedData = mContentCache.object( uri ) )
  {
    return *cachedData;
  }
  locker.unlock();

  const QByteArray res = fetchContent( uri, feedback );
  if ( feedback && feedback->isCanceled() )
    return QByteArray();

  locker.relock();
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

QgsTiledSceneTile QgsTiledSceneIndex::getTile( long long id )
{
  if ( !mIndex )
    return QgsTiledSceneTile();

  return mIndex->getTile( id );
}

long long QgsTiledSceneIndex::parentTileId( long long id ) const
{
  if ( !mIndex )
    return -1;

  return mIndex->parentTileId( id );
}

QVector< long long > QgsTiledSceneIndex::childTileIds( long long id ) const
{
  if ( !mIndex )
    return {};

  return mIndex->childTileIds( id );
}

QVector< long long > QgsTiledSceneIndex::getTiles( const QgsTiledSceneRequest &request )
{
  if ( !mIndex )
    return {};

  return mIndex->getTiles( request );
}

Qgis::TileChildrenAvailability QgsTiledSceneIndex::childAvailability( long long id ) const
{
  if ( !mIndex )
    return Qgis::TileChildrenAvailability::NoChildren;

  return mIndex->childAvailability( id );
}

bool QgsTiledSceneIndex::fetchHierarchy( long long id, QgsFeedback *feedback )
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

