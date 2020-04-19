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

#include "qgsvectortilemvtencoder.h"
#include "qgsvectortileutils.h"

#include "qgstiles.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

#include <QtDebug>
#include "qgsvectorlayer.h"

QgsVectorTileWriter::QgsVectorTileWriter()
{
  mExtent = QgsTileMatrix::fromWebMercator( 0 ).tileExtent( QgsTileXYZ( 0, 0, 0 ) );
}


bool QgsVectorTileWriter::writeTiles()
{
  if ( mMinZoom < 0 )
  {
    mErrorMessage = "Invalid min. zoom level";
    return false;
  }
  if ( mMaxZoom > 24 )
  {
    mErrorMessage = "Invalid max. zoom level";
    return false;
  }

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDestinationUri );

  QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType != QStringLiteral( "xyz" ) )
  {
    mErrorMessage = "Unsupported source type for writing: " + sourceType;
    return false;
  }

  // remove the initial file:// scheme
  sourcePath = QUrl( sourcePath ).toLocalFile();

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
          encoder.addLayer( layer.layer() );
        }

        QByteArray tileData = encoder.encode();

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
            mErrorMessage = "Cannot create directory " + fileDir.path();
            return false;
          }
        }

        QFile f( filePath );
        if ( !f.open( QIODevice::WriteOnly ) )
        {
          mErrorMessage = "Cannot open file for writing " + filePath;
          return false;
        }

        f.write( tileData );
        f.close();
      }
    }
  }

  return true;
}
