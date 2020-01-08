/***************************************************************************
  qgspostgresrastershareddata.cpp - QgsPostgresRasterSharedData

 ---------------------
 begin                : 8.1.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresrastershareddata.h"
#include "qgspostgresrasterutils.h"

QgsPostgresRasterSharedData::~QgsPostgresRasterSharedData()
{
  for ( auto idx : mSpatialIndexes )
  {
    delete idx.second;
  }
}

QList<QgsPostgresRasterSharedData::Tile *> QgsPostgresRasterSharedData::tiles( unsigned int overviewFactor, const QgsRectangle &extent )
{
  QList<QgsPostgresRasterSharedData::Tile *> result;
  QMutexLocker locker( &mMutex );
  Q_ASSERT( mSpatialIndexes.find( overviewFactor ) != mSpatialIndexes.end() );
  mSpatialIndexes[ overviewFactor ]->intersects( extent, [ &result ]( Tile * tilePtr ) -> bool
  {
    result.push_back( tilePtr );
    return true;
  } );
  return result;
}

QgsPostgresRasterSharedData::Tile *QgsPostgresRasterSharedData::addToIndex( unsigned int overviewFactor, QgsPostgresRasterSharedData::Tile *tile )
{
  QMutexLocker locker( &mMutex );
  Q_ASSERT( mSpatialIndexes.find( overviewFactor ) != mSpatialIndexes.end() );
  mTiles.push_back( std::unique_ptr<Tile>( tile ) );
  mSpatialIndexes[ overviewFactor ]->insert( tile, tile->extent );
  return tile;
}

void QgsPostgresRasterSharedData::initIndexes( const std::list<unsigned int> &overviewFactors )
{
  QMutexLocker locker( &mMutex );
  if ( mSpatialIndexes.size() == 0 )
  {
    // Create index for full resolution data
    mSpatialIndexes.emplace( 1, new QgsGenericSpatialIndex<Tile>() );
    // Create indexes for overviews
    for ( const auto &ovFactor : qgis::as_const( overviewFactors ) )
    {
      mSpatialIndexes.emplace( ovFactor, new QgsGenericSpatialIndex<Tile>() );
    }
  }
}

bool QgsPostgresRasterSharedData::indexIsEmpty( unsigned int overviewFactor )
{
  QMutexLocker locker( &mMutex );
  Q_ASSERT( mSpatialIndexes.find( overviewFactor ) != mSpatialIndexes.end() );
  return mSpatialIndexes[ overviewFactor ]->isEmpty();
}

QgsPostgresRasterSharedData::Tile::Tile( long tileId, int srid, QgsRectangle extent, double upperLeftX, double upperLeftY, long width, long height, double scaleX, double scaleY, double skewX, double skewY, int numBands )
  : tileId( tileId )
  , srid( srid )
  , extent( extent )
  , upperLeftX( upperLeftX )
  , upperLeftY( upperLeftY )
  , width( width )
  , height( height )
  , scaleX( scaleX )
  , scaleY( scaleY )
  , skewX( skewX )
  , skewY( skewY )
  , numBands( numBands )
{

}

QByteArray QgsPostgresRasterSharedData::Tile::bandData( int bandNo )
{
  return QgsPostgresRasterUtils::parseWkb( data, bandNo )[ QStringLiteral( "data" )].toByteArray();
}
