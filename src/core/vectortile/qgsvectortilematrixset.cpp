/***************************************************************************
  qgsvectortilematrixset.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortilematrixset.h"
#include "qgstiles.h"
#include "qgsarcgisrestutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

QgsVectorTileMatrixSet QgsVectorTileMatrixSet::fromWebMercator( int minimumZoom, int maximumZoom )
{
  QgsVectorTileMatrixSet res;
  res.addGoogleCrs84QuadTiles( minimumZoom, maximumZoom );
  return res;
}

bool QgsVectorTileMatrixSet::fromEsriJson( const QVariantMap &json, const QVariantMap &rootTileMap )
{
  setScaleToTileZoomMethod( Qgis::ScaleToTileZoomLevelMethod::Esri );

  const QVariantMap tileInfo = json.value( QStringLiteral( "tileInfo" ) ).toMap();

  const QVariantMap origin = tileInfo.value( QStringLiteral( "origin" ) ).toMap();
  const double originX = origin.value( QStringLiteral( "x" ) ).toDouble();
  const double originY = origin.value( QStringLiteral( "y" ) ).toDouble();

  const int rows = tileInfo.value( QStringLiteral( "rows" ), QStringLiteral( "512" ) ).toInt();
  const int cols = tileInfo.value( QStringLiteral( "cols" ), QStringLiteral( "512" ) ).toInt();
  if ( rows != cols )
  {
    QgsDebugError( QStringLiteral( "row/col size mismatch: %1 vs %2 - tile misalignment may occur" ).arg( rows ).arg( cols ) );
  }

  const QgsCoordinateReferenceSystem crs = QgsArcGisRestUtils::convertSpatialReference( tileInfo.value( QStringLiteral( "spatialReference" ) ).toMap() );

  const QVariantList lodList = tileInfo.value( QStringLiteral( "lods" ) ).toList();
  bool foundLevel0 = false;
  double z0Dimension = 0;

  for ( const QVariant &lod : lodList )
  {
    const QVariantMap lodMap = lod.toMap();
    const int level = lodMap.value( QStringLiteral( "level" ) ).toInt();
    if ( level == 0 )
    {
      z0Dimension = lodMap.value( QStringLiteral( "resolution" ) ).toDouble() * rows;
      foundLevel0 = true;
      break;
    }
  }

  if ( !foundLevel0 )
    return false;

  for ( const QVariant &lod : lodList )
  {
    const QVariantMap lodMap = lod.toMap();
    const int level = lodMap.value( QStringLiteral( "level" ) ).toInt();

    // TODO -- we shouldn't be using z0Dimension here, but rather the actual dimension and properties of
    // this exact LOD
    QgsTileMatrix tm = QgsTileMatrix::fromCustomDef(
                         level,
                         crs,
                         QgsPointXY( originX, originY ),
                         z0Dimension );
    tm.setScale( lodMap.value( QStringLiteral( "scale" ) ).toDouble() );
    addMatrix( tm );
  }

  setRootMatrix( QgsTileMatrix::fromCustomDef( 0, crs, QgsPointXY( originX, originY ), z0Dimension, 1, 1 ) );

  const QVariantList tileMap = rootTileMap.value( QStringLiteral( "index" ) ).toList();
  if ( !tileMap.isEmpty() )
  {
    // QUESTION: why do things this way?
    // ANSWERS:
    // - While we could do an upfront interpretation of the tilemap once, we'd end up storing
    //   the tile availability for every single tile in the matrix. This will quickly end up
    //   with a stupidly large amount of data, even for relatively small zoom level ranges.
    // - Even if we only store the availability for "not available" and "no child tiles" tiles,
    //   we can still end up storing a huge amount of tile information. Testing with relatively
    //   confined tileset extents still resulted in storage of 200k+ tile status due to the number
    //   of tiles which are skipped during the indexing.
    // - It's quite cheap to just pass the tile map every time we want to find the desired
    //   tile for a given extent.
    // - I don't want virtual methods in QgsTileMatrixSet and the complexity of handling copies
    //   of matrix sets when there's an inheritance involved

    mTileReplacementFunction = [tileMap]( QgsTileXYZ id, QgsTileXYZ & replacement ) -> Qgis::TileAvailability
    {
      /*
       Punch holes in matrix set according to tile map.
       From the ESRI documentation:
       "The tilemap resource describes a quadtree of tiles and can be used to avoid
       requesting tiles that don't exist in the server. Each node of the tree
       has an associated tile. The root node (lod 0) covers the entire extent of the
       data. Children are identified by their position with NW, NE, SW, and SE. Tiles
       are identified by lod/h/v, where h and v are indexes on a 2^lod by 2^lod grid .
       These values are derived from the position in the tree. The tree has a variable
       depth. A node doesn’t have children if the complexity of the data in the
       associated tile is below a threshold. This threshold is based on a combination
       of number of features, attributes, and vertices."

       And

       "Where <node> is : [<node>,<node>,<node>,<node>] in order NW,NE,SW,SE with possible values:
       1 // tile with no children (referred to as a leaf tile)
       0 // no tile (because there's no data here, so the tile file does not exist)
       2 // subtree defined in a different index file (to mitigate the index being too large)"
      */

      replacement = id;
      std::vector< QgsTileXYZ > bottomToTopQueue;
      bottomToTopQueue.reserve( id.zoomLevel() );
      int column = id.column();
      int row = id.row();
      // to handle the "tile with no children" states we need to start at the lowest zoom level
      // and then zoom in to the target zoom level. So let's first build up a queue of the lower level
      // tiles covering the tile at the target zoom level.
      for ( int zoomLevel = id.zoomLevel(); zoomLevel > 0; zoomLevel-- )
      {
        bottomToTopQueue.emplace_back( QgsTileXYZ( column, row, zoomLevel ) );
        column /= 2;
        row /= 2;
      }

      // now we'll zoom back in, checking the tilemap information for each tile as we go
      // in order to catch "tile with no children" states
      QVariantList node = tileMap;
      for ( int index = static_cast<int>( bottomToTopQueue.size() ) - 1; index >= 0; --index )
      {
        const QgsTileXYZ &tile = bottomToTopQueue[ index ];
        int childColumn = tile.column() - column;
        int childRow = tile.row() - row;
        int childIndex = 0;
        if ( childColumn == 1 && childRow == 0 )
          childIndex = 1;
        else if ( childColumn == 0 && childRow == 1 )
          childIndex = 2;
        else if ( childColumn == 1 && childRow == 1 )
          childIndex = 3;

        const QVariant childNode = node.at( childIndex );
        if ( childNode.userType() == QMetaType::Type::QVariantList )
        {
          node = childNode.toList();
          column = tile.column() * 2;
          row = tile.row() * 2;
          continue;
        }
        else
        {
          bool ok = false;
          const long long nodeInt = childNode.toLongLong( &ok );

          if ( !ok )
          {
            QgsDebugError( QStringLiteral( "Found tile index node with unsupported value: %1" ).arg( childNode.toString() ) );
          }
          else if ( nodeInt == 0 )
          {
            // "no tile (because there's no data here, so the tile file does not exist)"
            return Qgis::TileAvailability::NotAvailable;
          }
          else if ( nodeInt == 1 )
          {
            // "tile with no children (referred to as a leaf tile)"
            replacement = tile;
            return tile.zoomLevel() == id.zoomLevel() ? Qgis::TileAvailability::AvailableNoChildren : Qgis::TileAvailability::UseLowerZoomLevelTile;
          }
          else if ( nodeInt == 2 )
          {
            // "subtree defined in a different index file (to mitigate the index being too large)"
            // I've never seen this in the wild, and don't know how it's actually structured. There's no documentation
            // which describes where this "different index file" will be! I suspect it's something which was added to the
            // specification as a "just in case we need in future" thing which isn't actually in use right now.
            QgsMessageLog::logMessage( QObject::tr( "Found tile index node with subtree defined in a different index file -- this is not yet supported!" ), QObject::tr( "Vector Tiles" ), Qgis::MessageLevel::Critical );
            // assume available
            return Qgis::TileAvailability::Available;
          }
        }
      }
      return Qgis::TileAvailability::Available;
    };

    // we explicitly need to capture a copy of the member variable here
    const QMap< int, QgsTileMatrix > tileMatrices = mTileMatrices;
    mTileAvailabilityFunction = [this, tileMatrices]( QgsTileXYZ id ) -> Qgis::TileAvailability
    {
      // find zoom level matrix
      const auto it = tileMatrices.constFind( id.zoomLevel() );
      if ( it == tileMatrices.constEnd() )
        return Qgis::TileAvailability::NotAvailable;

      // check if column/row is within matrix
      if ( id.column() >= it->matrixWidth() || id.row() >= it->matrixHeight() )
        return Qgis::TileAvailability::NotAvailable;

      QgsTileXYZ replacement;
      return mTileReplacementFunction( id, replacement );
    };
  }
  return true;
}
