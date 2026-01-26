/***************************************************************************
  qgsrastertilereader.cpp - Fast tile reader for tiled raster datasets
  --------------------------------------
  Date                 : January 2026
  Copyright            : (C) 2026 by Wietze Suijker
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastertilereader.h"
#include "qgslogger.h"
#include "qgsrectangle.h"

#include <gdal.h>
#include <cpl_conv.h>
#include <algorithm>

QgsRasterTileReader::QgsRasterTileReader( GDALDatasetH dataset )
  : mDataset( dataset )
{
  mValid = initialize( dataset );
}

bool QgsRasterTileReader::initialize( GDALDatasetH dataset )
{
  if ( !dataset )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: NULL dataset" ) );
    return false;
  }

  mWidth = GDALGetRasterXSize( dataset );
  mHeight = GDALGetRasterYSize( dataset );

  if ( mWidth <= 0 || mHeight <= 0 )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Invalid dataset dimensions" ) );
    return false;
  }

  // Get geotransform for extent calculation
  if ( GDALGetGeoTransform( dataset, mGeoTransform ) != CE_None )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Failed to get geotransform" ) );
    // Set identity transform as fallback
    mGeoTransform[0] = 0.0;  // top left x
    mGeoTransform[1] = 1.0;  // w-e pixel resolution
    mGeoTransform[2] = 0.0;  // rotation, 0 if north up
    mGeoTransform[3] = 0.0;  // top left y
    mGeoTransform[4] = 0.0;  // rotation, 0 if north up
    mGeoTransform[5] = -1.0; // n-s pixel resolution (negative)
  }

  // Calculate extent
  const double xMin = mGeoTransform[0];
  const double yMax = mGeoTransform[3];
  const double xMax = xMin + mWidth * mGeoTransform[1];
  const double yMin = yMax + mHeight * mGeoTransform[5];
  mExtent = QgsRectangle( xMin, yMin, xMax, yMax );

  // Calculate base resolution
  mBaseResolutionX = std::abs( mGeoTransform[1] );
  mBaseResolutionY = std::abs( mGeoTransform[5] );

  // Get first band for overview and tile info
  GDALRasterBandH band = GDALGetRasterBand( dataset, 1 );
  if ( !band )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: No bands in dataset" ) );
    return false;
  }

  // Cache base level tile info
  if ( !cacheTileInfo( band, 0 ) )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Failed to cache base tile info" ) );
    return false;
  }

  // Cache overview tile info
  mOverviewCount = GDALGetOverviewCount( band );
  for ( int i = 0; i < mOverviewCount; ++i )
  {
    GDALRasterBandH overview = GDALGetOverview( band, i );
    if ( overview && !cacheTileInfo( overview, i + 1 ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "QgsRasterTileReader: Failed to cache overview %1 tile info" ).arg( i ), 2 );
    }
  }

  QgsDebugMsgLevel( QStringLiteral( "QgsRasterTileReader: Initialized with %1 overviews, base tile size %2x%3" )
                    .arg( mOverviewCount )
                    .arg( mTileInfoCache[0].width )
                    .arg( mTileInfoCache[0].height ), 2 );

  return true;
}

bool QgsRasterTileReader::cacheTileInfo( GDALRasterBandH band, int overviewIndex )
{
  if ( !band )
    return false;

  TileInfo info;

  // Get block (tile) size
  int blockXSize = 0, blockYSize = 0;
  GDALGetBlockSize( band, &blockXSize, &blockYSize );

  info.width = blockXSize;
  info.height = blockYSize;
  info.dataType = GDALGetRasterDataType( band );
  info.bytesPerPixel = GDALGetDataTypeSizeBytes( info.dataType );
  info.bandCount = GDALGetRasterCount( mDataset );

  // Calculate number of tiles
  const int rasterXSize = GDALGetRasterBandXSize( band );
  const int rasterYSize = GDALGetRasterBandYSize( band );

  if ( rasterXSize <= 0 || rasterYSize <= 0 )
    return false;

  info.tilesX = ( rasterXSize + blockXSize - 1 ) / blockXSize;
  info.tilesY = ( rasterYSize + blockYSize - 1 ) / blockYSize;

  // Check if dataset is tiled (block size indicates tiling)
  // Tiled datasets have block sizes smaller than raster and height > 1
  // Strip-based formats have block size = width x 1
  info.isTiled = ( blockXSize < rasterXSize ) && ( blockYSize > 1 );

  // Ensure cache has enough space
  while ( mTileInfoCache.size() <= overviewIndex )
  {
    mTileInfoCache.append( TileInfo() );
  }

  mTileInfoCache[overviewIndex] = info;

  return true;
}

QgsRasterTileReader::TileInfo QgsRasterTileReader::tileInfo( int overviewLevel ) const
{
  if ( overviewLevel < 0 || overviewLevel >= mTileInfoCache.size() )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Invalid overview level %1" ).arg( overviewLevel ) );
    return TileInfo();
  }

  return mTileInfoCache[overviewLevel];
}

bool QgsRasterTileReader::readTile( int overviewLevel, int tileX, int tileY, int bandNumber, QByteArray &outBuffer )
{
  if ( !mValid || !mDataset )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Reader not valid" ) );
    return false;
  }

  if ( overviewLevel < 0 || overviewLevel >= mTileInfoCache.size() )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Invalid overview level %1" ).arg( overviewLevel ) );
    return false;
  }

  const TileInfo &info = mTileInfoCache[overviewLevel];

  if ( tileX < 0 || tileX >= info.tilesX || tileY < 0 || tileY >= info.tilesY )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Tile indices out of range: %1,%2 (max %3,%4)" )
                   .arg( tileX ).arg( tileY ).arg( info.tilesX ).arg( info.tilesY ) );
    return false;
  }

  // Get the band
  GDALRasterBandH band = GDALGetRasterBand( mDataset, bandNumber );
  if ( !band )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Invalid band number %1" ).arg( bandNumber ) );
    return false;
  }

  // Get overview if requested
  if ( overviewLevel > 0 )
  {
    band = GDALGetOverview( band, overviewLevel - 1 );
    if ( !band )
    {
      QgsDebugError( QStringLiteral( "QgsRasterTileReader: Failed to get overview %1" ).arg( overviewLevel - 1 ) );
      return false;
    }
  }

  // Allocate buffer
  const int bufferSize = info.width * info.height * info.bytesPerPixel;
  outBuffer.resize( bufferSize );

  // Use GDALReadBlock for optimal tile reading on tiled datasets
  if ( info.isTiled )
  {
    const CPLErr err = GDALReadBlock( band, tileX, tileY, outBuffer.data() );
    if ( err != CE_None )
    {
      QgsDebugError( QStringLiteral( "QgsRasterTileReader: GDALReadBlock failed for tile %1,%2 overview %3" )
                     .arg( tileX ).arg( tileY ).arg( overviewLevel ) );
      return false;
    }
  }
  else
  {
    // Fallback to RasterIO for non-tiled datasets
    const int xOff = tileX * info.width;
    const int yOff = tileY * info.height;
    const int xSize = std::min( info.width, GDALGetRasterBandXSize( band ) - xOff );
    const int ySize = std::min( info.height, GDALGetRasterBandYSize( band ) - yOff );

    const CPLErr err = GDALRasterIO(
                         band,
                         GF_Read,
                         xOff, yOff,      // source offset
                         xSize, ySize,    // source size
                         outBuffer.data(),
                         info.width, info.height,  // destination size
                         info.dataType,
                         0, 0  // pixel/line spacing (auto)
                       );

    if ( err != CE_None )
    {
      QgsDebugError( QStringLiteral( "QgsRasterTileReader: GDALRasterIO failed for tile %1,%2 overview %3" )
                     .arg( tileX ).arg( tileY ).arg( overviewLevel ) );
      return false;
    }
  }

  return true;
}

bool QgsRasterTileReader::readTileMultiBand( int overviewLevel, int tileX, int tileY, const QList<int> &bandNumbers, QByteArray &outBuffer )
{
  if ( bandNumbers.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: No band numbers provided" ) );
    return false;
  }

  if ( !mValid || !mDataset )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Reader not valid" ) );
    return false;
  }

  if ( overviewLevel < 0 || overviewLevel >= mTileInfoCache.size() )
  {
    QgsDebugError( QStringLiteral( "QgsRasterTileReader: Invalid overview level %1" ).arg( overviewLevel ) );
    return false;
  }

  const TileInfo &info = mTileInfoCache[overviewLevel];
  const int pixelCount = info.width * info.height;
  const int bandCount = bandNumbers.size();

  // Read each band separately
  QVector<QByteArray> bandBuffers( bandCount );
  for ( int i = 0; i < bandCount; ++i )
  {
    if ( !readTile( overviewLevel, tileX, tileY, bandNumbers[i], bandBuffers[i] ) )
    {
      QgsDebugError( QStringLiteral( "QgsRasterTileReader: Failed to read band %1" ).arg( bandNumbers[i] ) );
      return false;
    }
  }

  // Interleave bands
  outBuffer.resize( pixelCount * info.bytesPerPixel * bandCount );

  // Build source pointers for interleaving
  QVector<const char *> srcPointers( bandCount );
  for ( int i = 0; i < bandCount; ++i )
  {
    srcPointers[i] = bandBuffers[i].constData();
  }

  char *dst = outBuffer.data();

  for ( int pixel = 0; pixel < pixelCount; ++pixel )
  {
    for ( int band = 0; band < bandCount; ++band )
    {
      std::memcpy( dst, srcPointers[band] + pixel * info.bytesPerPixel, info.bytesPerPixel );
      dst += info.bytesPerPixel;
    }
  }

  return true;
}

int QgsRasterTileReader::selectBestOverview( double targetMupp ) const
{
  if ( !mValid || mTileInfoCache.isEmpty() )
    return 0;

  if ( targetMupp <= 0.0 )
    return 0;

  // Start with base resolution
  double bestResolution = std::max( mBaseResolutionX, mBaseResolutionY );
  int bestLevel = 0;

  // Find the coarsest overview that still provides enough resolution
  for ( int i = 1; i < mTileInfoCache.size(); ++i )
  {
    // Calculate resolution at this overview level
    // Each overview typically has resolution = baseResolution * 2^level
    const double factor = std::pow( 2.0, i );
    const double levelResolution = bestResolution * factor;

    // Select this overview if its resolution is still finer than or equal to target
    // but coarser than the previous best
    if ( levelResolution <= targetMupp * 1.5 && levelResolution > bestResolution )
    {
      bestResolution = levelResolution;
      bestLevel = i;
    }
  }

  return bestLevel;
}
