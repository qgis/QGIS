/***************************************************************************
                         qgstiledscenetile.cpp
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

#include "qgstiledscenetile.h"
#include "qgstiledsceneboundingvolume.h"

QgsTiledSceneTile::QgsTiledSceneTile()
  : mBoundingVolume( QgsTiledSceneBoundingVolume( QgsOrientedBox3D() ) )
{

}

QgsTiledSceneTile::QgsTiledSceneTile( long long id )
  : mId( id )
  , mBoundingVolume( QgsTiledSceneBoundingVolume( QgsOrientedBox3D() ) )
{

}

QgsTiledSceneTile::~QgsTiledSceneTile() = default;

QgsTiledSceneTile::QgsTiledSceneTile( const QgsTiledSceneTile &other )
  : mId( other.mId )
  , mRefinementProcess( other.mRefinementProcess )
  , mBoundingVolume( other.mBoundingVolume )
  , mResources( other.mResources )
  , mGeometricError( other.mGeometricError )
  , mBaseUrl( other.mBaseUrl )
  , mMetadata( other.mMetadata )
{
  mTransform.reset( other.mTransform ? new QgsMatrix4x4( *other.mTransform.get() ) : nullptr );
}

QgsTiledSceneTile &QgsTiledSceneTile::operator=( const QgsTiledSceneTile &other )
{
  mId = other.mId;
  mRefinementProcess = other.mRefinementProcess;
  mTransform.reset( other.mTransform ? new QgsMatrix4x4( *other.mTransform.get() ) : nullptr );
  mResources = other.mResources;
  mGeometricError = other.mGeometricError;
  mBoundingVolume = other.mBoundingVolume;
  mBaseUrl = other.mBaseUrl;
  mMetadata = other.mMetadata;
  return *this;
}

void QgsTiledSceneTile::setRefinementProcess( Qgis::TileRefinementProcess process )
{
  mRefinementProcess = process;
}

void QgsTiledSceneTile::setBoundingVolume( const QgsTiledSceneBoundingVolume &volume )
{
  mBoundingVolume = volume;
}

const QgsTiledSceneBoundingVolume &QgsTiledSceneTile::boundingVolume() const
{
  return mBoundingVolume;
}

void QgsTiledSceneTile::setTransform( const QgsMatrix4x4 &transform )
{
  if ( transform.isIdentity() )
    mTransform.reset();
  else
    mTransform = std::make_unique< QgsMatrix4x4 >( transform );
}

QVariantMap QgsTiledSceneTile::resources() const
{
  return mResources;
}

void QgsTiledSceneTile::setResources( const QVariantMap &resources )
{
  mResources = resources;
}

void QgsTiledSceneTile::setGeometricError( double error )
{
  mGeometricError = error;
}

QUrl QgsTiledSceneTile::baseUrl() const
{
  return mBaseUrl;
}

void QgsTiledSceneTile::setBaseUrl( const QUrl &baseUrl )
{
  mBaseUrl = baseUrl;
}

QVariantMap QgsTiledSceneTile::metadata() const
{
  return mMetadata;
}

void QgsTiledSceneTile::setMetadata( const QVariantMap &metadata )
{
  mMetadata = metadata;
}
