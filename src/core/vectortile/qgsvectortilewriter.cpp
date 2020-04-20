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
#include "qgstiles.h"
#include "qgsvectortilemvtencoder.h"
#include "qgsvectortileutils.h"

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

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDestinationUri );

  QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType != QStringLiteral( "xyz" ) )
  {
    mErrorMessage = tr( "Unsupported source type for writing: " ) + sourceType;
    return false;
  }

  // remove the initial file:// scheme
  sourcePath = QUrl( sourcePath ).toLocalFile();

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
      }
    }
  }

  return true;
}
