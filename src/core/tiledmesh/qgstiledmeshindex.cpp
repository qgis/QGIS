/***************************************************************************
                         qgstiledmeshindex.cpp
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

#include "qgstiledmeshindex.h"
#include "qgsfeedback.h"
#include "qgstiledmeshtile.h"
#include "qgsreadwritelocker.h"

//
// QgsAbstractTiledMeshIndex
//

QgsAbstractTiledMeshIndex::QgsAbstractTiledMeshIndex()
{
  mContentCache.setMaxCost( 10000 );
}

QgsAbstractTiledMeshIndex::~QgsAbstractTiledMeshIndex() = default;

QByteArray QgsAbstractTiledMeshIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
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
// QgsTiledMeshIndex
//

QgsTiledMeshIndex::QgsTiledMeshIndex( QgsAbstractTiledMeshIndex *index )
  : mIndex( index )
{

}

QgsTiledMeshIndex::~QgsTiledMeshIndex() = default;

QgsTiledMeshIndex::QgsTiledMeshIndex( const QgsTiledMeshIndex &other )
  : mIndex( other.mIndex )
{

}

QgsTiledMeshIndex &QgsTiledMeshIndex::operator=( const QgsTiledMeshIndex &other )
{
  mIndex = other.mIndex;
  return *this;
}

bool QgsTiledMeshIndex::isValid() const
{
  return static_cast< bool >( mIndex.get() );
}

QgsTiledMeshTile QgsTiledMeshIndex::rootTile() const
{
  if ( !mIndex )
    return QgsTiledMeshTile();

  return mIndex->rootTile();
}

QgsTiledMeshTile QgsTiledMeshIndex::getTile( const QString &id )
{
  if ( !mIndex )
    return QgsTiledMeshTile();

  return mIndex->getTile( id );
}

QString QgsTiledMeshIndex::parentTileId( const QString &id ) const
{
  if ( !mIndex )
    return QString();

  return mIndex->parentTileId( id );
}

QStringList QgsTiledMeshIndex::childTileIds( const QString &id ) const
{
  if ( !mIndex )
    return QStringList();

  return mIndex->childTileIds( id );
}

QStringList QgsTiledMeshIndex::getTiles( const QgsTiledMeshRequest &request )
{
  if ( !mIndex )
    return {};

  return mIndex->getTiles( request );
}

Qgis::TileChildrenAvailability QgsTiledMeshIndex::childAvailability( const QString &id ) const
{
  if ( !mIndex )
    return Qgis::TileChildrenAvailability::NoChildren;

  return mIndex->childAvailability( id );
}

bool QgsTiledMeshIndex::fetchHierarchy( const QString &id, QgsFeedback *feedback )
{
  if ( !mIndex )
    return {};

  return mIndex->fetchHierarchy( id, feedback );
}

QByteArray QgsTiledMeshIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
{
  if ( !mIndex )
    return QByteArray();

  return mIndex->retrieveContent( uri, feedback );
}

