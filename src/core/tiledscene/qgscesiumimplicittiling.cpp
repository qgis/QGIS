/***************************************************************************
                         qgscesiumimplicittiling.cpp
                         ---------------------------
    begin                : March 2026
    copyright            : (C) 2026 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumimplicittiling.h"

#include "qgscesiumutils.h"
#include "qgslogger.h"
#include "qgstiledscenenode.h"
#include "qgstiledscenetile.h"

#include <QString>

using namespace Qt::StringLiterals;

int QgsCesiumImplicitTiling::subtreeTileCount( int levels )
{
  // Total tiles in a quadtree is 1 + 4 + 16 + 64 + ... which is
  // a geometric series, with a bit of math we get closed form: (4^levels - 1) / 3
  return ( static_cast<int>( std::pow( 4, levels ) ) - 1 ) / 3;
}

int QgsCesiumImplicitTiling::mortonIndex( int x, int y )
{
  // this interleaves 16 bits of X and Y to a single 32-bit value
  // (bits from X end up at bit positions 0,2,4,... and from Y end
  // up at bit positions 1,3,5,...)
  int morton = 0;
  for ( int i = 0; i < 16; ++i )
  {
    morton |= ( ( x & ( 1 << i ) ) << i ) | ( ( y & ( 1 << i ) ) << ( i + 1 ) );
  }
  return morton;
}

int QgsCesiumImplicitTiling::subtreeBitIndex( int localLevel, int localX, int localY )
{
  int offsetForLevel = subtreeTileCount( localLevel );
  return offsetForLevel + mortonIndex( localX, localY );
}

QString QgsCesiumImplicitTiling::expandTemplateUri( const QString &templateUri, const QUrl &baseUrl, const TileCoordinate &coord )
{
  QString expanded = templateUri;
  expanded.replace( "{level}"_L1, QString::number( coord.level ) );
  expanded.replace( "{x}"_L1, QString::number( coord.x ) );
  expanded.replace( "{y}"_L1, QString::number( coord.y ) );

  const QString resolved = baseUrl.resolved( QUrl( expanded ) ).toString();
  if ( baseUrl.hasQuery() && QUrl( expanded ).isRelative() )
    return QgsCesiumUtils::appendQueryFromBaseUrl( resolved, baseUrl );
  return resolved;
}


QgsBox3D QgsCesiumImplicitTiling::computeImplicitRegionBoundingVolume( const QgsBox3D &rootRegion, const TileCoordinate &coord )
{
  const double divisor = std::pow( 2.0, coord.level );
  const double tileWidth = rootRegion.width() / divisor;
  const double tileHeight = rootRegion.height() / divisor;

  const double xMin = rootRegion.xMinimum() + coord.x * tileWidth;
  const double yMin = rootRegion.yMinimum() + coord.y * tileHeight;

  return QgsBox3D( xMin, yMin, rootRegion.zMinimum(), xMin + tileWidth, yMin + tileHeight, rootRegion.zMaximum() );
}


QgsTiledSceneBoundingVolume QgsCesiumImplicitTiling::computeImplicitBoundingVolume( const QgsTiledSceneBoundingVolume &rootVolume, const TileCoordinate &coord )
{
  const QgsOrientedBox3D &rootBox = rootVolume.box();
  if ( rootBox.isNull() )
    return rootVolume;

  const double *halfAxes = rootBox.halfAxes();
  const double divisor = std::pow( 2.0, coord.level );

  // Half-axes vectors (rows of the 3x3 matrix)
  // halfAxes[0..2] = first half-axis (u)
  // halfAxes[3..5] = second half-axis (v)
  // halfAxes[6..8] = third half-axis (w) — unchanged for quadtree

  // Child half-axes: u and v are divided by 2^level, w stays the same
  const double childHalfAxes[9]
    = { halfAxes[0] / divisor, halfAxes[1] / divisor, halfAxes[2] / divisor, halfAxes[3] / divisor, halfAxes[4] / divisor, halfAxes[5] / divisor, halfAxes[6], halfAxes[7], halfAxes[8] };

  // child tile's center relative to the tiling grid (with range [0,0] to [1,1])
  const double dx = ( coord.x + 0.5 ) / divisor;
  const double dy = ( coord.y + 0.5 ) / divisor;

  // turn the range from [0,1]-[1,1] to [-1,-1]-[1,1] so that we can use it
  // with half-axes in the next step
  const double fx = dx * 2 - 1;
  const double fy = dy * 2 - 1;

  const double childCenter[3] = { rootBox.centerX() + fx * halfAxes[0] + fy * halfAxes[3], rootBox.centerY() + fx * halfAxes[1] + fy * halfAxes[4], rootBox.centerZ() + fx * halfAxes[2] + fy * halfAxes[5] };

  return QgsTiledSceneBoundingVolume( QgsOrientedBox3D(
    QList<double>( { childCenter[0], childCenter[1], childCenter[2] } ),
    QList<double>( { childHalfAxes[0], childHalfAxes[1], childHalfAxes[2], childHalfAxes[3], childHalfAxes[4], childHalfAxes[5], childHalfAxes[6], childHalfAxes[7], childHalfAxes[8] } )
  ) );
}


QgsCesiumImplicitTiling::Subtree QgsCesiumImplicitTiling::parseSubtree( const Root &tilingData, const QByteArray &data )
{
  Subtree result;

  if ( data.isEmpty() )
    return result;

  // Subtree binary format:
  // Header (24 bytes): magic "subt" (4), version (4), jsonByteLength (8), binaryByteLength (8)
  if ( data.size() < 24 )
  {
    QgsDebugError( u"Subtree data too short"_s );
    return result;
  }

  const char *ptr = data.constData();
  if ( memcmp( ptr, "subt", 4 ) != 0 )
  {
    // Maybe it's a JSON subtree (the spec allows JSON format too)
    try
    {
      const auto subtreeJson = json::parse( data.toStdString() );
      // TODO: handle JSON-only subtree format if needed
      QgsDebugError( u"JSON subtree format not yet supported"_s );
      return result;
    }
    catch ( json::parse_error & )
    {
      QgsDebugError( u"Invalid subtree data (not binary or JSON)"_s );
      return result;
    }
  }

  // Parse header
  quint64 jsonByteLength = 0;
  quint64 binaryByteLength = 0;
  memcpy( &jsonByteLength, ptr + 8, 8 );
  memcpy( &binaryByteLength, ptr + 16, 8 );

  const int headerSize = 24;
  if ( static_cast<quint64>( data.size() ) < headerSize + jsonByteLength )
  {
    QgsDebugError( u"Subtree data truncated"_s );
    return result;
  }

  // Parse JSON chunk
  const QByteArray jsonChunk = data.mid( headerSize, static_cast<int>( jsonByteLength ) );
  json subtreeJson;
  try
  {
    subtreeJson = json::parse( jsonChunk.toStdString() );
  }
  catch ( json::parse_error & )
  {
    QgsDebugError( u"Cannot parse subtree JSON chunk"_s );
    return result;
  }

  // Binary chunk starts after JSON chunk (8-byte aligned)
  const quint64 binaryStart = headerSize + ( ( jsonByteLength + 7 ) & ~static_cast<quint64>( 7 ) );
  const QByteArray binaryChunk = ( binaryByteLength > 0 && static_cast<quint64>( data.size() ) >= binaryStart + binaryByteLength )
                                   ? data.mid( static_cast<int>( binaryStart ), static_cast<int>( binaryByteLength ) )
                                   : QByteArray();

  // Parse bufferViews
  std::vector<QPair<int, int>> bufferViews; // (byteOffset, byteLength)
  if ( subtreeJson.contains( "bufferViews" ) )
  {
    for ( const auto &bv : subtreeJson["bufferViews"] )
    {
      const int byteOffset = bv.value( "byteOffset", 0 );
      const int byteLength = bv["byteLength"].get<int>();
      bufferViews.push_back( { byteOffset, byteLength } );
    }
  }

  // Helper to parse an availability object into a QBitArray
  auto parseAvailability = [&]( const json &availJson, int bitCount ) -> QBitArray {
    QBitArray bits( bitCount, false );
    if ( availJson.contains( "constant" ) )
    {
      const int constant = availJson["constant"].get<int>();
      if ( constant == 1 )
        bits.fill( true );
      // constant == 0 means all false (already initialized)
    }
    else if ( availJson.contains( "bitstream" ) )
    {
      const int bitstreamIdx = availJson["bitstream"].get<int>();
      if ( bitstreamIdx >= 0 && bitstreamIdx < static_cast<int>( bufferViews.size() ) )
      {
        const auto &[byteOffset, byteLength] = bufferViews[bitstreamIdx];
        if ( byteOffset + byteLength <= binaryChunk.size() )
        {
          const unsigned char *bitstreamData = reinterpret_cast<const unsigned char *>( binaryChunk.constData() + byteOffset );
          for ( int i = 0; i < bitCount && ( i / 8 ) < byteLength; ++i )
          {
            if ( bitstreamData[i / 8] & ( 1 << ( i % 8 ) ) )
              bits.setBit( i );
          }
        }
      }
    }
    return bits;
  };

  // Total tile count in subtree: (4^S - 1) / 3
  const int totalTiles = subtreeTileCount( tilingData.subtreeLevels );

  // Child subtree count: 4^S (nodes at the bottom level)
  const int childSubtreeCount = static_cast<int>( std::pow( 4, tilingData.subtreeLevels ) );

  if ( subtreeJson.contains( "tileAvailability" ) )
    result.tileAvailability = parseAvailability( subtreeJson["tileAvailability"], totalTiles );
  else
    result.tileAvailability = QBitArray( totalTiles, true );

  if ( subtreeJson.contains( "contentAvailability" ) )
  {
    // contentAvailability can be an array (one per content) or a single object
    const auto &contentAvail = subtreeJson["contentAvailability"];
    if ( contentAvail.is_array() && !contentAvail.empty() )
      result.contentAvailability = parseAvailability( contentAvail[0], totalTiles ); // TODO: if there are multiple contents - multiple arrays
    else if ( contentAvail.is_object() )
      result.contentAvailability = parseAvailability( contentAvail, totalTiles );
    else
      result.contentAvailability = QBitArray( totalTiles, false );
  }
  else
  {
    result.contentAvailability = QBitArray( totalTiles, false );
  }

  if ( subtreeJson.contains( "childSubtreeAvailability" ) )
    result.childSubtreeAvailability = parseAvailability( subtreeJson["childSubtreeAvailability"], childSubtreeCount );
  else
    result.childSubtreeAvailability = QBitArray( childSubtreeCount, false );

  return result;
}


QgsCesiumImplicitTiling::TileCoordinate QgsCesiumImplicitTiling::tileCoordinateToParentSubtree( TileCoordinate coord, int subtreeLevels )
{
  const int subtreeRootLevel = ( coord.level / subtreeLevels ) * subtreeLevels;
  const int localLevel = coord.level - subtreeRootLevel;
  const int subtreeRootX = coord.x >> localLevel;
  const int subtreeRootY = coord.y >> localLevel;
  return QgsCesiumImplicitTiling::TileCoordinate { subtreeRootLevel, subtreeRootX, subtreeRootY };
}


QMap<QgsCesiumImplicitTiling::TileCoordinate, QgsTiledSceneNode *> QgsCesiumImplicitTiling::createImplicitTilingChildren(
  QgsTiledSceneNode *node, const TileCoordinate &coord, Root &tilingData, const TileCoordinate &subtreeCoord, QgsCoordinateTransformContext &transformContext, long long &nextTileId
)
{
  QMap<TileCoordinate, QgsTiledSceneNode *> children;
  const Subtree &subtree = tilingData.subtreeCache[subtreeCoord];

  // Generate up to 4 children
  const int childLevel = coord.level + 1;
  for ( int dy = 0; dy < 2; ++dy )
  {
    for ( int dx = 0; dx < 2; ++dx )
    {
      const int childX = 2 * coord.x + dx;
      const int childY = 2 * coord.y + dy;
      const int childLocalLevel = childLevel - subtreeCoord.level;
      const TileCoordinate childCoord { childLevel, childX, childY };

      bool childAvail = false;
      bool childHasContent = false;

      if ( childLocalLevel < tilingData.subtreeLevels )
      {
        const int bitIdx = subtreeBitIndex( childLocalLevel, childX - ( subtreeCoord.x << childLocalLevel ), childY - ( subtreeCoord.y << childLocalLevel ) );
        if ( bitIdx < subtree.tileAvailability.size() )
          childAvail = subtree.tileAvailability.testBit( bitIdx );
        if ( bitIdx < subtree.contentAvailability.size() )
          childHasContent = subtree.contentAvailability.testBit( bitIdx );
      }
      else
      {
        const int childSubtreeIdx = mortonIndex( childX - ( subtreeCoord.x << childLocalLevel ), childY - ( subtreeCoord.y << childLocalLevel ) );
        if ( childSubtreeIdx < subtree.childSubtreeAvailability.size() )
        {
          childAvail = subtree.childSubtreeAvailability.testBit( childSubtreeIdx );
          // Don't assume content exists — the child subtree root's content
          // availability is in its own subtree, not in the parent's.
          // Content will be resolved when the child's subtree is fetched.
          childHasContent = false;
        }
      }

      if ( !childAvail )
        continue;

      auto childTile = std::make_unique<QgsTiledSceneTile>( nextTileId++ );
      childTile->setGeometricError( tilingData.rootGeometricError / std::pow( 2.0, childLevel ) );
      childTile->setRefinementProcess( tilingData.refinementProcess );

      // Compute the child bounding volume. For region-based tilesets, subdivide in lat/lon
      // space and convert the sub-region to an oriented box only at the last step.
      if ( tilingData.rootRegion.has_value() )
      {
        const QgsBox3D childRegion = computeImplicitRegionBoundingVolume( *tilingData.rootRegion, childCoord );
        childTile->setBoundingVolume( QgsCesiumUtils::boundingVolumeFromRegion( childRegion, transformContext ) );
      }
      else
      {
        childTile->setBoundingVolume( computeImplicitBoundingVolume( tilingData.rootBoundingVolume, childCoord ) );
      }
      childTile->setBaseUrl( tilingData.baseUrl );
      childTile->setMetadata( {
        { u"gltfUpAxis"_s, static_cast<int>( tilingData.gltfUpAxis ) },
        { u"contentFormat"_s, u"cesiumtiles"_s },
      } );

      if ( tilingData.rootTransform.has_value() )
        childTile->setTransform( *tilingData.rootTransform );

      if ( childHasContent && !tilingData.contentUriTemplate.isEmpty() )
      {
        const QString contentUri = expandTemplateUri( tilingData.contentUriTemplate, tilingData.baseUrl, childCoord );
        childTile->setResources( { { u"content"_s, contentUri } } );
      }

      auto childNode = std::make_unique<QgsTiledSceneNode>( childTile.release() );
      children.insert( childCoord, childNode.get() );
      node->addChild( childNode.release() );
    }
  }
  return children;
}


Qgis::TileChildrenAvailability QgsCesiumImplicitTiling::childAvailability( const Root &tilingData, const TileCoordinate &coord )
{
  if ( coord.level + 1 >= tilingData.availableLevels )
    return Qgis::TileChildrenAvailability::NoChildren; // outside of our zoom levels

  // Check whether the subtree covering this tile has been fetched
  const TileCoordinate subtreeCoord = tileCoordinateToParentSubtree( coord, tilingData.subtreeLevels );
  const auto subtreeCacheIt = tilingData.subtreeCache.constFind( subtreeCoord );
  if ( subtreeCacheIt == tilingData.subtreeCache.constEnd() )
  {
    // Subtree not yet fetched — must fetch to determine children
    return Qgis::TileChildrenAvailability::NeedFetching;
  }

  const int localLevel = coord.level - subtreeCoord.level;
  if ( localLevel + 1 < tilingData.subtreeLevels )
  {
    // Children would be within this subtree; since none were created during subtree
    // population, they don't exist
    return Qgis::TileChildrenAvailability::NoChildren;
  }

  // Tile is at the deepest level of its subtree — check childSubtreeAvailability
  const Subtree &subtree = subtreeCacheIt.value();
  const int childLocalLevel = localLevel + 1;
  for ( int dy = 0; dy < 2; ++dy )
  {
    for ( int dx = 0; dx < 2; ++dx )
    {
      const int childX = 2 * coord.x + dx;
      const int childY = 2 * coord.y + dy;
      const int childSubtreeIdx = mortonIndex( childX - ( subtreeCoord.x << childLocalLevel ), childY - ( subtreeCoord.y << childLocalLevel ) );
      if ( childSubtreeIdx < subtree.childSubtreeAvailability.size() && subtree.childSubtreeAvailability.testBit( childSubtreeIdx ) )
        return Qgis::TileChildrenAvailability::NeedFetching;
    }
  }
  return Qgis::TileChildrenAvailability::NoChildren;
}

bool QgsCesiumImplicitTiling::parseImplicitTiling( const json &json, QgsTiledSceneNode *newNode, const QUrl &baseUrl, Qgis::Axis gltfUpAxis, Root &tilingData )
{
  const auto &implicit = json["implicitTiling"];
  const std::string scheme = implicit.value( "subdivisionScheme", "" );
  if ( scheme == "QUADTREE" )
  {
    if ( !implicit.contains( "availableLevels" ) || !implicit.contains( "subtreeLevels" ) )
    {
      QgsDebugError( u"Implicit tiling is missing required availableLevels or subtreeLevels"_s );
      return false;
    }
    tilingData.availableLevels = implicit["availableLevels"].get<int>();
    tilingData.subtreeLevels = implicit["subtreeLevels"].get<int>();

    if ( implicit.contains( "subtrees" ) && implicit["subtrees"].contains( "uri" ) )
      tilingData.subtreeUriTemplate = QString::fromStdString( implicit["subtrees"]["uri"].get<std::string>() );

    if ( tilingData.subtreeUriTemplate.isEmpty() )
    {
      QgsDebugError( u"Implicit tiling is missing required subtrees.uri"_s );
      return false;
    }

    if ( json.contains( "content" ) && json["content"].contains( "uri" ) )
      tilingData.contentUriTemplate = QString::fromStdString( json["content"]["uri"].get<std::string>() );

    // TODO: "contents" with multiple contents not supported yet

    tilingData.baseUrl = baseUrl;
    tilingData.rootBoundingVolume = newNode->tile()->boundingVolume();
    // If the root tile uses a "region" bounding volume, store the original lat/lon region
    // so that implicit tile subdivision can be done in geographic space rather than on
    // the derived oriented bounding box.
    if ( json.contains( "boundingVolume" ) && json["boundingVolume"].contains( "region" ) )
    {
      const QgsBox3D region = QgsCesiumUtils::parseRegion( json["boundingVolume"]["region"] );
      if ( !region.isNull() )
        tilingData.rootRegion = region;
    }
    tilingData.rootGeometricError = newNode->tile()->geometricError();
    tilingData.refinementProcess = newNode->tile()->refinementProcess();
    if ( newNode->tile()->transform() )
      tilingData.rootTransform = *newNode->tile()->transform();
    tilingData.gltfUpAxis = gltfUpAxis;

    // Clear the template content URI — it will be resolved after subtree fetch
    newNode->tile()->setResources( {} );
    return true;
  }
  else
  {
    QgsDebugError( u"Unsupported implicit tiling subdivision scheme: %1"_s.arg( QString::fromStdString( scheme ) ) );
    return false;
  }
}
