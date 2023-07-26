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
#include "qgsreadwritelocker.h"
#include "qgstiledmeshnode.h"

//
// QgsAbstractTiledMeshIndex
//

QgsAbstractTiledMeshIndex::QgsAbstractTiledMeshIndex()
{
  mContentCache.setMaxCost( 50 );
}

QgsAbstractTiledMeshIndex::~QgsAbstractTiledMeshIndex() = default;

QByteArray QgsAbstractTiledMeshIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
{
  QgsReadWriteLocker locker( mLock, QgsReadWriteLocker::Read );
  if ( QByteArray *cachedData = mContentCache.object( uri ) )
  {
    return *cachedData;
  }

  locker.unlock();

  const QByteArray res = fetchContent( uri, feedback );
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

QgsTiledMeshNode QgsTiledMeshIndex::rootNode() const
{
  if ( !mIndex )
    return QgsTiledMeshNode();

  QgsReadWriteLocker locker( mIndex->mLock, QgsReadWriteLocker::Read );
  return *mIndex->rootNode();
}

QgsTiledMeshNode *QgsTiledMeshIndex::getNodes( const QgsTiledMeshRequest &request )
{
  if ( !mIndex )
    return nullptr;

  QgsReadWriteLocker locker( mIndex->mLock, QgsReadWriteLocker::Write );
  return mIndex->getNodes( request );
}

QByteArray QgsTiledMeshIndex::retrieveContent( const QString &uri, QgsFeedback *feedback )
{
  if ( !mIndex )
    return QByteArray();

  return mIndex->retrieveContent( uri, feedback );
}
