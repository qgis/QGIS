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

#include "cpl_string.h"
#include "gdal.h"

#include "qgspostgresrastershareddata.h"
#include "qgspostgresrasterutils.h"
#include "qgspostgresconn.h"
#include "qgsmessagelog.h"
#include "qgspolygon.h"
#include "qgslogger.h"

QgsPostgresRasterSharedData::~QgsPostgresRasterSharedData()
{
  for ( const auto &idx : mSpatialIndexes )
  {
    delete idx.second;
  }
}

QgsPostgresRasterSharedData::TilesResponse QgsPostgresRasterSharedData::tiles( const QgsPostgresRasterSharedData::TilesRequest &request )
{
  const QMutexLocker locker( &mMutex );

  const QString cacheKey { keyFromRequest( request ) };

  QgsPostgresRasterSharedData::TilesResponse result;

  // First check for index existence
  if ( mSpatialIndexes.find( cacheKey ) == mSpatialIndexes.end() )
  {
    // Create the index
    mSpatialIndexes.emplace( cacheKey, new QgsGenericSpatialIndex<Tile>() );
    mTiles.emplace( cacheKey, std::map<TileIdType, std::unique_ptr<Tile>>() );
    mLoadedIndexBounds[ cacheKey] = QgsGeometry();
  }

  // Now check if the requested extent was completely downloaded
  const QgsGeometry requestedRect { QgsGeometry::fromRect( request.extent ) };

  // Fast track for first tile (where index is empty)
  if ( mLoadedIndexBounds[ cacheKey ].isNull() )
  {
    return fetchTilesIndexAndData( requestedRect, request );
  }
  else if ( ! mLoadedIndexBounds[ cacheKey].contains( requestedRect ) )
  {
    // Fetch index
    const QgsGeometry geomDiff { requestedRect.difference( mLoadedIndexBounds[ cacheKey ] ) };
    if ( ! fetchTilesIndex( geomDiff.isEmpty() ? requestedRect : geomDiff, request ) )
    {
      return result;
    }
  }

  // Stores tile IDs to be fetched
  QStringList missingTileIds;

  // Get intersecting tiles from the index
  mSpatialIndexes[ cacheKey ]->intersects( request.extent, [ & ]( Tile * tilePtr ) -> bool
  {
    if ( tilePtr->data.size() == 0 )
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

    const QString sql { QStringLiteral( "SELECT %1, ENCODE( ST_AsBinary( %2, TRUE ), 'hex') "
                                        "FROM %3 WHERE %4 %1 IN ( %5 )" )
                        .arg( request.pk,
                              request.rasterColumn,
                              request.tableToQuery,
                              request.whereClause,
                              missingTileIds.join( ',' ) ) };

    QgsPostgresResult dataResult( request.conn->PQexec( sql ) );
    if ( dataResult.PQresultStatus() != PGRES_TUPLES_OK )
    {
      QgsMessageLog::logMessage( QObject::tr( "Unable to get tile data.\nThe error message from the database was:\n%1.\nSQL: %2" )
                                 .arg( dataResult.PQresultErrorMessage(),
                                       sql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );
    }

    if ( dataResult.PQntuples() != missingTileIds.size() )
    {
      QgsMessageLog::logMessage( QObject::tr( "Missing tiles were not found while fetching tile data from backend.\nSQL: %1" )
                                 .arg( sql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );
    }

    for ( int row = 0; row < dataResult.PQntuples(); ++row )
    {
      // Note: if we change tile id type we need to sync this
      const TileIdType tileId { dataResult.PQgetvalue( row, 0 ) };
      if ( tileId.isEmpty() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Tile with ID (%1) is empty while fetching tile data from backend.\nSQL: %2" )
                                   .arg( dataResult.PQgetvalue( row, 0 ) )
                                   .arg( sql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );
      }

      int dataRead;
      GByte *binaryData { CPLHexToBinary( dataResult.PQgetvalue( row, 1 ).toLatin1().constData(), &dataRead ) };
      Tile const *tilePtr { setTileData( cacheKey, tileId, QByteArray::fromRawData( reinterpret_cast<char *>( binaryData ), dataRead ) ) };
      CPLFree( binaryData );

      if ( ! tilePtr )
      {
        // This should never happen!
        QgsMessageLog::logMessage( QObject::tr( "Tile with ID (%1) could not be found in provider storage while fetching tile data "
                                                "from backend.\nSQL: %2" )
                                   .arg( tileId )
                                   .arg( sql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );
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

void QgsPostgresRasterSharedData::invalidateCache()
{
  const QMutexLocker locker( &mMutex );
  mSpatialIndexes.clear();
  mTiles.clear();
  mLoadedIndexBounds.clear();
}


QgsPostgresRasterSharedData::Tile const *QgsPostgresRasterSharedData::setTileData( const QString &cacheKey, TileIdType tileId, const QByteArray &data )
{
  Q_ASSERT( ! data.isEmpty() );
  if ( mTiles.find( cacheKey ) == mTiles.end() ||
       mTiles[ cacheKey ].find( tileId ) == mTiles[ cacheKey ].end() )
  {
    return nullptr;
  }

  Tile *const tile { mTiles[ cacheKey ][ tileId ].get() };
  const QVariantMap parsedData = QgsPostgresRasterUtils::parseWkb( data );
  for ( int bandCnt = 1; bandCnt <= tile->numBands; ++bandCnt )
  {
    tile->data.emplace_back( parsedData[ QStringLiteral( "band%1" ).arg( bandCnt ) ].toByteArray() );
  }
  return tile;
}

QString QgsPostgresRasterSharedData::keyFromRequest( const QgsPostgresRasterSharedData::TilesRequest &request )
{
  return QStringLiteral( "%1 - %2" ).arg( QString::number( request.overviewFactor ), request.whereClause );
}

bool QgsPostgresRasterSharedData::fetchTilesIndex( const QgsGeometry &requestPolygon, const TilesRequest &request )
{
  const QString indexSql { QStringLiteral( "SELECT %1, (ST_Metadata( %2 )).* FROM %3 "
                           "WHERE %6 %2 && ST_GeomFromText( '%5', %4 )" )
                           .arg( request.pk,
                                 request.rasterColumn,
                                 request.tableToQuery,
                                 request.srid,
                                 requestPolygon.asWkt(),
                                 request.whereClause ) };

  QgsPostgresResult result( request.conn->PQexec( indexSql ) );

  if ( result.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error fetching tile index from backend.\nSQL: %1" )
                               .arg( indexSql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );

    return false;
  }

  const QString cacheKey { keyFromRequest( request ) };

  if ( mLoadedIndexBounds[ cacheKey ].isNull() )
  {
    mLoadedIndexBounds[ cacheKey ] = requestPolygon;
  }
  else
  {
    mLoadedIndexBounds[ cacheKey ] = mLoadedIndexBounds[ cacheKey ].combine( requestPolygon );
  }

  QgsRectangle overallExtent;

  for ( int i = 0; i < result.PQntuples(); ++i )
  {
    // rid | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands
    const TileIdType tileId { result.PQgetvalue( i, 0 ) };

    if ( mTiles[ cacheKey ].find( tileId ) == mTiles[ cacheKey ].end() )
    {
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
      double minY { upperlefty + tileHeight * scaley };
      double maxY { upperlefty };
      // Southing Y?
      if ( scaley > 0 )
      {
        std::swap( minY, maxY );
      }
      const QgsRectangle extent( upperleftx, minY,  upperleftx + tileWidth * scalex, maxY );

      overallExtent.combineExtentWith( extent );

      std::unique_ptr<QgsPostgresRasterSharedData::Tile> tile = std::make_unique<QgsPostgresRasterSharedData::Tile>(
            tileId,
            srid,
            extent,
            upperleftx,
            maxY,
            tileWidth,
            tileHeight,
            scalex,
            scaley,
            skewx,
            skewy,
            numbands
          );
      mSpatialIndexes[ cacheKey ]->insert( tile.get(), tile->extent );
      mTiles[ cacheKey ][ tileId ] = std::move( tile );
      QgsDebugMsgLevel( QStringLiteral( "Tile added: %1, ID: %2" )
                        .arg( cacheKey )
                        .arg( tileId ), 3 );
      //qDebug() << "Tile added:" << cacheKey << " ID: " << tileId << "extent " << extent.toString( 4 ) << upperleftx << upperlefty << tileWidth  << tileHeight <<  extent.width() << extent.height();
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Tile already indexed: %1, ID: %2" )
                        .arg( cacheKey )
                        .arg( tileId ), 3 );
    }
  }

  // Include actual bounds
  mLoadedIndexBounds[ cacheKey ] = requestPolygon.combine( QgsGeometry::fromWkt( overallExtent.asWktPolygon() ) );

  return true;
}

QgsPostgresRasterSharedData::TilesResponse QgsPostgresRasterSharedData::fetchTilesIndexAndData( const QgsGeometry &requestPolygon, const QgsPostgresRasterSharedData::TilesRequest &request )
{
  QgsPostgresRasterSharedData::TilesResponse response;
  const QString indexSql { QStringLiteral( "SELECT %1, (ST_Metadata( %2 )).*, ENCODE( ST_AsBinary( %2, TRUE ), 'hex') FROM %3 "
                           "WHERE %6 %2 && ST_GeomFromText( '%5', %4 )" )
                           .arg( request.pk,
                                 request.rasterColumn,
                                 request.tableToQuery,
                                 request.srid,
                                 requestPolygon.asWkt(),
                                 request.whereClause ) };

  QgsPostgresResult dataResult( request.conn->PQexec( indexSql ) );

  if ( dataResult.PQresultStatus() != PGRES_TUPLES_OK )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error fetching tile index from backend.\nSQL: %1" )
                               .arg( indexSql ), QObject::tr( "PostGIS" ), Qgis::MessageLevel::Critical );

    return response;
  }

  const QString cacheKey { keyFromRequest( request ) };

  for ( int row = 0; row < dataResult.PQntuples(); ++row )
  {
    // rid | upperleftx | upperlefty | width | height | scalex | scaley | skewx | skewy | srid | numbands | data
    const TileIdType tileId { dataResult.PQgetvalue( row, 0 ) };

    if ( mTiles[ cacheKey ].find( tileId ) == mTiles[ cacheKey ].end() )
    {
      const double upperleftx { dataResult.PQgetvalue( row, 1 ).toDouble() };
      const double upperlefty { dataResult.PQgetvalue( row, 2 ).toDouble() };
      const long int tileWidth { dataResult.PQgetvalue( row, 3 ).toLong( ) };
      const long int tileHeight { dataResult.PQgetvalue( row, 4 ).toLong( ) };
      const double scalex { dataResult.PQgetvalue( row, 5 ).toDouble( ) };
      const double scaley { dataResult.PQgetvalue( row, 6 ).toDouble( ) };
      const double skewx { dataResult.PQgetvalue( row, 7 ).toDouble( ) };
      const double skewy { dataResult.PQgetvalue( row, 8 ).toDouble( ) };
      const int srid {dataResult.PQgetvalue( row, 9 ).toInt() };
      const int numbands {dataResult.PQgetvalue( row, 10 ).toInt() };

      double minY { upperlefty + tileHeight * scaley };
      double maxY { upperlefty };
      // Southing Y?
      if ( scaley > 0 )
      {
        std::swap( minY, maxY );
      }
      const QgsRectangle extent( upperleftx, minY,  upperleftx + tileWidth * scalex, maxY );

      std::unique_ptr<QgsPostgresRasterSharedData::Tile> tile = std::make_unique<QgsPostgresRasterSharedData::Tile>(
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

      int dataRead;
      GByte *binaryData { CPLHexToBinary( dataResult.PQgetvalue( row, 11 ).toLatin1().constData(), &dataRead ) };
      const QVariantMap parsedData = QgsPostgresRasterUtils::parseWkb( QByteArray::fromRawData( reinterpret_cast<char *>( binaryData ), dataRead ) );
      CPLFree( binaryData );
      for ( int bandCnt = 1; bandCnt <= tile->numBands; ++bandCnt )
      {
        tile->data.emplace_back( parsedData[ QStringLiteral( "band%1" ).arg( bandCnt ) ].toByteArray() );
      }
      mSpatialIndexes[ cacheKey ]->insert( tile.get(), tile->extent );

      response.tiles.push_back( TileBand
      {
        tile->tileId,
        tile->srid,
        tile->extent,
        tile->upperLeftX,
        tile->upperLeftY,
        tile->width,
        tile->height,
        tile->scaleX,
        tile->scaleY,
        tile->skewX,
        tile->skewY,
        tile->bandData( request.bandNo )
      } );

      response.extent.combineExtentWith( tile->extent );

      mTiles[ cacheKey ][ tileId ] = std::move( tile );
      QgsDebugMsgLevel( QStringLiteral( "Tile added: %1, ID: %2" )
                        .arg( cacheKey )
                        .arg( tileId ), 3 );
      //qDebug() << "Tile data added:" << cacheKey << " ID: " << tileId << "extent " << extent.toString( 4 ) << upperleftx << upperlefty << tileWidth  << tileHeight <<  extent.width() << extent.height();
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Tile and data already indexed: %1, ID: %2" )
                        .arg( cacheKey )
                        .arg( tileId ), 2 );
    }
  }

  // Include actual bounds
  mLoadedIndexBounds[ cacheKey ] = requestPolygon.combine( QgsGeometry::fromWkt( response.extent.asWktPolygon() ) );

  return response;
}

QgsPostgresRasterSharedData::Tile::Tile( const QgsPostgresRasterSharedData::TileIdType tileId, int srid, QgsRectangle extent, double upperLeftX, double upperLeftY, long width, long height, double scaleX, double scaleY, double skewX, double skewY, int numBands )
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

const QByteArray QgsPostgresRasterSharedData::Tile::bandData( int bandNo ) const
{
  Q_ASSERT( 0 < bandNo && bandNo <= static_cast<int>( data.size() ) );
  return data.at( static_cast<unsigned int>( bandNo ) - 1 );
}
