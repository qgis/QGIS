/***************************************************************************
  qgsvectortilewriter.cpp
  --------------------------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilewriter.h"

#include "qgsdatasourceuri.h"
#include "qgsfeedback.h"
#include "qgsjsonutils.h"
#include "qgsmbtiles.h"
#include "qgstiles.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilemvtencoder.h"
#include "qgsvectortileutils.h"

#include <nlohmann/json.hpp>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>


QgsVectorTileWriter::QgsVectorTileWriter()
{
  mExtent = QgsTileMatrix::fromWebMercator( 0 ).tileExtent( QgsTileXYZ( 0, 0, 0 ) );
}


bool QgsVectorTileWriter::writeTiles( QgsFeedback *feedback )
{
  if ( mMinZoom < 0 )
  {
    mErrorMessage = tr( "Invalid min. zoom level" );
    return false;
  }
  if ( mMaxZoom > 24 )
  {
    mErrorMessage = tr( "Invalid max. zoom level" );
    return false;
  }

  std::unique_ptr<QgsMbTiles> mbtiles;

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDestinationUri );

  QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QStringLiteral( "xyz" ) )
  {
    // remove the initial file:// scheme
    sourcePath = QUrl( sourcePath ).toLocalFile();
  }
  else if ( sourceType == QStringLiteral( "mbtiles" ) )
  {
    mbtiles.reset( new QgsMbTiles( sourcePath ) );
  }
  else
  {
    mErrorMessage = tr( "Unsupported source type for writing: " ) + sourceType;
    return false;
  }

  // figure out how many tiles we will need to do
  int tilesToCreate = 0;
  for ( int zoomLevel = mMinZoom; zoomLevel <= mMaxZoom; ++zoomLevel )
  {
    QgsTileMatrix tileMatrix = QgsTileMatrix::fromWebMercator( zoomLevel );

    QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( mExtent );
    tilesToCreate += ( tileRange.endRow() - tileRange.startRow() + 1 ) *
                     ( tileRange.endColumn() - tileRange.startColumn() + 1 );
  }

  if ( tilesToCreate == 0 )
  {
    mErrorMessage = tr( "No tiles to generate" );
    return false;
  }

  if ( mbtiles )
  {
    if ( !mbtiles->create() )
    {
      mErrorMessage = tr( "Failed to create MBTiles file: " ) + sourcePath;
      return false;
    }

    // required metadata
    mbtiles->setMetadataValue( "name", "???" );  // TODO: custom name?
    mbtiles->setMetadataValue( "format", "pbf" );

    // optional metadata
    mbtiles->setMetadataValue( "bounds", "-180.0,-85,180,85" );
    mbtiles->setMetadataValue( "minzoom", QString::number( mMinZoom ) );
    mbtiles->setMetadataValue( "maxzoom", QString::number( mMaxZoom ) );
    // TODO: "center"? initial view with [lon,lat,zoom]

    // required metadata for vector tiles: "json" with schema of layers
    mbtiles->setMetadataValue( "json", mbtilesJsonSchema() );
  }

  int tilesCreated = 0;
  for ( int zoomLevel = mMinZoom; zoomLevel <= mMaxZoom; ++zoomLevel )
  {
    QgsTileMatrix tileMatrix = QgsTileMatrix::fromWebMercator( zoomLevel );

    QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( mExtent );
    for ( int row = tileRange.startRow(); row <= tileRange.endRow(); ++row )
    {
      for ( int col = tileRange.startColumn(); col <= tileRange.endColumn(); ++col )
      {
        QgsTileXYZ tileID( col, row, zoomLevel );
        QgsVectorTileMVTEncoder encoder( tileID );

        for ( const Layer &layer : qgis::as_const( mLayers ) )
        {
          encoder.addLayer( layer.layer(), feedback );
        }

        if ( feedback && feedback->isCanceled() )
        {
          mErrorMessage = tr( "Operation has been canceled" );
          return false;
        }

        QByteArray tileData = encoder.encode();

        ++tilesCreated;
        if ( feedback )
        {
          feedback->setProgress( static_cast<double>( tilesCreated ) / tilesToCreate * 100 );
        }

        if ( tileData.isEmpty() )
        {
          // skipping empty tile - no need to write it
          continue;
        }

        if ( sourceType == QStringLiteral( "xyz" ) )
        {
          if ( !writeTileFileXYZ( sourcePath, tileID, tileData ) )
            return false;  // error message already set
        }
        else  // mbtiles
        {
          QByteArray gzipTileData;
          QgsMbTiles::encodeGzip( tileData, gzipTileData );
          int rowTMS = pow( 2, tileID.zoomLevel() ) - tileID.row() - 1;
          mbtiles->setTileData( tileID.zoomLevel(), tileID.column(), rowTMS, gzipTileData );
        }
      }
    }
  }

  return true;
}

bool QgsVectorTileWriter::writeTileFileXYZ( const QString &sourcePath, QgsTileXYZ tileID, const QByteArray &tileData )
{
  QString filePath = QgsVectorTileUtils::formatXYZUrlTemplate( sourcePath, tileID );

  // make dirs if needed
  QFileInfo fi( filePath );
  QDir fileDir = fi.dir();
  if ( !fileDir.exists() )
  {
    if ( !fileDir.mkpath( "." ) )
    {
      mErrorMessage = tr( "Cannot create directory " ) + fileDir.path();
      return false;
    }
  }

  QFile f( filePath );
  if ( !f.open( QIODevice::WriteOnly ) )
  {
    mErrorMessage = tr( "Cannot open file for writing " ) + filePath;
    return false;
  }

  f.write( tileData );
  f.close();
  return true;
}


QString QgsVectorTileWriter::mbtilesJsonSchema()
{
  QVariantList arrayLayers;
  for ( const Layer &layer : qgis::as_const( mLayers ) )
  {
    QgsVectorLayer *vl = layer.layer();
    const QgsFields fields = vl->fields();

    QVariantMap fieldsObj;
    for ( const QgsField &field : fields )
    {
      QString fieldTypeStr;
      if ( field.type() == QVariant::Bool )
        fieldTypeStr = QStringLiteral( "Boolean" );
      else if ( field.type() == QVariant::Int || field.type() == QVariant::Double )
        fieldTypeStr = QStringLiteral( "Number" );
      else
        fieldTypeStr = QStringLiteral( "String" );

      fieldsObj[field.name()] = fieldTypeStr;
    }

    QVariantMap layerObj;
    layerObj["id"] = vl->name();
    layerObj["fields"] = fieldsObj;
    arrayLayers.append( layerObj );
  }

  QVariantMap rootObj;
  rootObj["vector_layers"] = arrayLayers;
  return QString::fromStdString( QgsJsonUtils::jsonFromVariant( rootObj ).dump() );
}
