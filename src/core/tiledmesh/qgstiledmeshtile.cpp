/***************************************************************************
                         qgstiledmeshtile.cpp
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

#include "qgstiledmeshtile.h"
#include "qgstiledmeshboundingvolume.h"

QgsTiledMeshTile::QgsTiledMeshTile()
{

}

QgsTiledMeshTile::QgsTiledMeshTile( const QString &id )
  : mId( id )
{

}

QgsTiledMeshTile::~QgsTiledMeshTile() = default;

QgsTiledMeshTile::QgsTiledMeshTile( const QgsTiledMeshTile &other )
  : mId( other.mId )
  , mRefinementProcess( other.mRefinementProcess )
  , mResources( other.mResources )
  , mGeometricError( other.mGeometricError )
{
  mBoundingVolume.reset( other.mBoundingVolume ? other.mBoundingVolume->clone() : nullptr );
  mTransform.reset( other.mTransform ? new QgsMatrix4x4( *other.mTransform.get() ) : nullptr );
}

QgsTiledMeshTile &QgsTiledMeshTile::operator=( const QgsTiledMeshTile &other )
{
  mId = other.mId;
  mRefinementProcess = other.mRefinementProcess;
  mTransform.reset( other.mTransform ? new QgsMatrix4x4( *other.mTransform.get() ) : nullptr );
  mResources = other.mResources;
  mGeometricError = other.mGeometricError;
  mBoundingVolume.reset( other.mBoundingVolume ? other.mBoundingVolume->clone() : nullptr );
  return *this;
}

void QgsTiledMeshTile::setRefinementProcess( Qgis::TileRefinementProcess process )
{
  mRefinementProcess = process;
}

void QgsTiledMeshTile::setBoundingVolume( QgsAbstractTiledMeshNodeBoundingVolume *volume )
{
  mBoundingVolume.reset( volume );
}

const QgsAbstractTiledMeshNodeBoundingVolume *QgsTiledMeshTile::boundingVolume() const
{
  return mBoundingVolume.get();
}

void QgsTiledMeshTile::setTransform( const QgsMatrix4x4 &transform )
{
  if ( transform.isIdentity() )
    mTransform.reset();
  else
    mTransform = std::make_unique< QgsMatrix4x4 >( transform );
}

QVariantMap QgsTiledMeshTile::resources() const
{
  return mResources;
}

void QgsTiledMeshTile::setResources( const QVariantMap &resources )
{
  mResources = resources;
}

void QgsTiledMeshTile::setGeometricError( double error )
{
  mGeometricError = error;
}

