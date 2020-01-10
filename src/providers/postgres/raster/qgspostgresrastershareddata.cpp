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

#include <QDebug>
#include <QObject>

#include "qgspostgresrastershareddata.h"
#include "qgspostgresrasterutils.h"
#include "qgspostgresconn.h"
#include "qgsmessagelog.h"
#include "qgspolygon.h"

QgsPostgresRasterSharedData::~QgsPostgresRasterSharedData()
{
  for ( auto idx : mSpatialIndexes )
  {
    delete idx.second;
  }
}

QgsPostgresRasterSharedData::TilesResponse QgsPostgresRasterSharedData::tiles( const QgsPostgresRasterSharedData::TilesRequest &request )
{
  QMutexLocker locker( &mMutex );

  QgsPostgresRasterSharedData::TilesResponse result;

  // First check for index existence
  if ( mSpatialIndexes.find( request.overviewFactor ) == mSpatialIndexes.end() )
  {
    // Create the index
    mSpatialIndexes.emplace( request.overviewFactor, new QgsGenericSpatialIndex<Tile>() );
    mTiles.emplace( request.overviewFactor, std::map<TileIdType, std::unique_ptr<Tile>>() );
    mLoadedIndexBounds[ request.overviewFactor] = QgsGeometry::fromRect( QgsRectangle() );
  }

  // Now check if the requested extent was completely downloaded
  const QgsGeometry requestedRect { QgsGeometry::fromRect( request.extent ) };
  if ( ! mLoadedIndexBounds[ request.overviewFactor].contains( requestedRect ) )
  {
    // Fetch index
    const QgsGeometry geomDiff { requestedRect.difference( mLoadedIndexBounds[ request.overviewFactor] ) };
    if ( ! fetchTilesIndex( geomDiff.isEmpty() ? requestedRect : geomDiff, request ) )
    {
      return result;
    }
  }

  // Stores tile IDs to be fetched
  QStringList missingTileIds;

  // Get intersecting tiles from the index
  mSpatialIndexes[ request.overviewFactor ]->intersects( request.extent, [ & ]( Tile * tilePtr ) -> bool
  {
    if ( tilePtr->data.isEmpty() )
    {
      missingTileIds.push_back( QStringLiteral( "'%1'" ).arg( tilePtr->tileId ) );
    }
    else
    {
      result.tiles.push_back( TileBand
      {
        tilePtr->tileId,
        tilePtr->srid,
        tilePtr->extent,
        tilePtr->upperLeftX,
        tilePtr->upperLeftY,
        tilePtr->width,
        tilePtr->height,
        tilePtr->scaleX,
        tilePtr->scaleY,
        tilePtr->skewX,
        tilePtr->skewY,
        tilePtr->bandData( request.bandNo )
      } );
      result.extent.combineExtentWith( tilePtr->extent );
    }
    return true;
  } );

  // Fetch missing tile data in one single query
  if ( ! missingTileIds.isEmpty() )
  {

    const QString sql { request.dataSql + '(' + missingTileIds.join( ',' ) + ')' };
    QgsPostgresResult dataResult( request.conn->PQexec( sql ) );
    if ( dataResult.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( QObject::tr( "Unable to get tile data.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( dataResult.PQresultErrorMessage(),
                                       sql ), QObject::tr( "PostGIS" ), Qgis::Critical );
    }

    if ( dataResult.PQntuples() != missingTileIds.size() )
    {
      QgsMessageLog::logMessage( QObject::tr( "Missing tiles where not found while fetching tile data from backend.\nSQL: %1" )
                                 .arg( sql ), QObject::tr( "PostGIS" ), Qgis::Critical );
    }

    for ( int row = 0; row < dataResult.PQntuples(); ++row )
    {
      bool ok;
      // Note: if we change tile id type we need to sync this
      const int tileId { dataResult.PQgetvalue( row, 0 ).toInt( &ok ) };
      if ( ! ok )
      {
        QgsMessageLog::logMessage( QObject::tr( "TileID (%1) could not be converted to integer while fetching tile data from backend.\nSQL: %2" )
                                   .arg( dataResult.PQgetvalue( row, 0 ) )
                                   .arg( sql ), QObject::tr( "PostGIS" ), Qgis::Critical );
      }

      Tile const *tilePtr { setTileData( request.overviewFactor, tileId, QByteArray::fromHex( dataResult.PQgetvalue( row, 1 ).toAscii() ) ) };

      if ( ! tilePtr )
      {
        // This should never happen!
        QgsMessageLog::logMessage( QObject::tr( "Tile with ID (%1) could not be found in provider storage while fetching tile data "
                                                "from backend.\nSQL: %2" )
                                   .arg( tileId )
                                   .arg( sql ), QObject::tr( "PostGIS" ), Qgis::Critical );
        Q_ASSERT( tilePtr );  // Abort
      }
      else  // Add to result
      {
        result.tiles.push_back( TileBand
        {
          tilePtr->tileId,
          tilePtr->srid,
          tilePtr->extent,
          tilePtr->upperLeftX,
          tilePtr->upperLeftY,
          tilePtr->width,
          tilePtr->height,
          tilePtr->scaleX,
          tilePtr->scaleY,
          tilePtr->skewX,
          tilePtr->skewY,
          tilePtr->bandData( request.bandNo )
        } );
        result.extent.combineExtentWith( tilePtr->extent );
      }
    }
  }

  return result;
}


QgsPostgresRasterSharedData::Tile const *QgsPostgresRasterSharedData::setTileData( unsigned int overviewFactor, int tileId, const QByteArray &data )
{
  Q_ASSERT( ! data.isEmpty() );
  if ( mTiles.find( overviewFactor ) == mTiles.end() ||
       mTiles[ overviewFactor ].find( tileId ) == mTiles[ overviewFactor ].end() )
  {
    return nullptr;
  }
  mTiles[ overviewFactor ][ tileId ]->data = data;
  return mTiles[ overviewFactor ][ tileId ].get();
}

bool QgsPostgresRasterSharedData::fetchTilesIndex( const QgsGeometry &requestPolygon, const TilesRequest &request )
{
  QString indexSql { request.indexSql };

  indexSql.replace( QStringLiteral( "###__POLYGON_WKT__###" ), requestPolygon.asWkt() );

  QgsPostgresResult result( request.conn->PQexec( indexSql ) );

  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error fetching tile index from backend.\nSQL: %1" )
                               .arg( indexSql ), QObject::tr( "PostGIS" ), Qgis::Critical );

    return false;
  }

  mLoadedIndexBounds[ request.overviewFactor ].addPart( requestPolygon );

  for ( int i = 0; i < result.PQntuples(); ++i )
  {
    // rid | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands
    const TileIdType tileId { result.PQgetvalue( i, 0 ).toInt( ) };
    const double upperleftx { result.PQgetvalue( i, 1 ).toDouble() };
    const double upperlefty { result.PQgetvalue( i, 2 ).toDouble() };
    const long int tileWidth { result.PQgetvalue( i, 3 ).toLong( ) };
    const long int tileHeight { result.PQgetvalue( i, 4 ).toLong( ) };
    const double scalex { result.PQgetvalue( i, 5 ).toDouble( ) };
    const double scaley { result.PQgetvalue( i, 6 ).toDouble( ) };
    const double skewx { result.PQgetvalue( i, 7 ).toDouble( ) };
    const double skewy { result.PQgetvalue( i, 8 ).toDouble( ) };
    const int srid {result.PQgetvalue( i, 9 ).toInt() };
    const int numbands {result.PQgetvalue( i, 10 ).toInt() };
    const QgsRectangle extent( upperleftx, upperlefty - tileHeight * std::abs( scaley ), upperleftx + tileWidth * scalex, upperlefty );

    std::unique_ptr<QgsPostgresRasterSharedData::Tile> tile = qgis::make_unique<QgsPostgresRasterSharedData::Tile>(
          tileId,
          srid,
          extent,
          upperleftx,
          upperlefty,
          tileWidth,
          tileHeight,
          scalex,
          scaley,
          skewx,
          skewy,
          numbands
        );

    if ( mTiles[ request.overviewFactor ].find( tileId ) == mTiles[ request.overviewFactor ].end() )
    {
      mSpatialIndexes[ request.overviewFactor ]->insert( tile.get(), tile->extent );
      mTiles[ request.overviewFactor ][ tileId ] = std::move( tile );
      qDebug() << "Tile added:" << request.overviewFactor << " ID: " << tileId << "extent " << extent.toString( 4 ) << upperleftx << upperlefty << tileWidth  << tileHeight <<  extent.width() << extent.height();
    }
    else
    {
      qDebug() << "Tile already indexed:" << request.overviewFactor << " ID: " << tileId << "extent " << extent.toString( 4 ) << upperleftx << upperlefty << tileWidth  << tileHeight <<  extent.width() << extent.height();
    }
  }
  return true;
}

QgsPostgresRasterSharedData::Tile::Tile( int tileId, int srid, QgsRectangle extent, double upperLeftX, double upperLeftY, long width, long height, double scaleX, double scaleY, double skewX, double skewY, int numBands )
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

QByteArray QgsPostgresRasterSharedData::Tile::bandData( int bandNo ) const
{
  return QgsPostgresRasterUtils::parseWkb( data, bandNo )[ QStringLiteral( "data" )].toByteArray();
}
