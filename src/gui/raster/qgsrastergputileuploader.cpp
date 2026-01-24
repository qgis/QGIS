/***************************************************************************
  qgsrastergputileuploader.cpp - Fast GPU tile uploader for rasters
  --------------------------------------
  Date                 : January 2026
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastergputileuploader.h"
#include "qgslogger.h"

#include <QOpenGLContext>

QgsRasterGPUTileUploader::QgsRasterGPUTileUploader( QgsCOGTileReader *reader )
  : mReader( reader )
{
  if ( QOpenGLContext::currentContext() )
  {
    initializeOpenGLFunctions();
    mGLInitialized = true;
  }

  if ( mReader && mReader->isValid() )
  {
    const auto tileInfo = mReader->tileInfo( 0 );
    // Get format for the raster's data type
    mTextureFormat = QgsRasterTextureFormat::getFormat(
                       static_cast<Qgis::DataType>( tileInfo.dataType ),
                       tileInfo.bandCount
                     );
  }
}

QgsRasterGPUTileUploader::~QgsRasterGPUTileUploader()
{
  clearCache();
}

quint64 QgsRasterGPUTileUploader::makeTileKey( int overview, int tileX, int tileY, int band )
{
  // Pack into 64-bit key: [16-bit overview][16-bit band][16-bit Y][16-bit X]
  return ( static_cast<quint64>( overview & 0xFFFF ) << 48 ) |
         ( static_cast<quint64>( band & 0xFFFF ) << 32 ) |
         ( static_cast<quint64>( tileY & 0xFFFF ) << 16 ) |
         ( static_cast<quint64>( tileX & 0xFFFF ) );
}

QgsRasterGPUTileUploader::GPUTile QgsRasterGPUTileUploader::uploadTile(
  int overviewLevel, int tileX, int tileY, int bandNumber )
{
  GPUTile result;

  if ( !mReader || !mReader->isValid() || !mGLInitialized )
  {
    QgsDebugError( QStringLiteral( "GPU uploader not initialized" ) );
    return result;
  }

  // Read tile data using fast COG reader
  if ( !mReader->readTile( overviewLevel, tileX, tileY, bandNumber, mTileBuffer ) )
  {
    QgsDebugError( QStringLiteral( "Failed to read tile %1,%2 overview %3" )
                   .arg( tileX ).arg( tileY ).arg( overviewLevel ) );
    return result;
  }

  const auto tileInfo = mReader->tileInfo( overviewLevel );

  // Create OpenGL texture
  GLuint textureId = 0;
  glGenTextures( 1, &textureId );
  glBindTexture( GL_TEXTURE_2D, textureId );

  // Set texture parameters
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  // Upload data directly to GPU (zero-copy!)
  glTexImage2D(
    GL_TEXTURE_2D,
    0,  // mip level
    mTextureFormat.internalFormat,
    tileInfo.width,
    tileInfo.height,
    0,  // border (must be 0)
    mTextureFormat.format,
    mTextureFormat.type,
    mTileBuffer.constData()
  );

  glBindTexture( GL_TEXTURE_2D, 0 );

  // Check for errors
  const GLenum error = glGetError();
  if ( error != GL_NO_ERROR )
  {
    QgsDebugError( QStringLiteral( "OpenGL error uploading tile: %1" ).arg( error ) );
    glDeleteTextures( 1, &textureId );
    return result;
  }

  // Fill result
  result.textureId = textureId;
  result.width = tileInfo.width;
  result.height = tileInfo.height;
  result.lastUsedFrame = 0;
  result.isValid = true;

  return result;
}

QgsRasterGPUTileUploader::GPUTile QgsRasterGPUTileUploader::getTile(
  int overviewLevel, int tileX, int tileY, int bandNumber, quint64 frameNumber )
{
  const quint64 key = makeTileKey( overviewLevel, tileX, tileY, bandNumber );

  // Check cache
  if ( mTileCache.contains( key ) )
  {
    GPUTile &tile = mTileCache[key];
    tile.lastUsedFrame = frameNumber;
    return tile;
  }

  // Upload new tile
  GPUTile tile = uploadTile( overviewLevel, tileX, tileY, bandNumber );
  if ( tile.isValid )
  {
    tile.lastUsedFrame = frameNumber;
    mTileCache[key] = tile;
  }

  return tile;
}

void QgsRasterGPUTileUploader::clearCache()
{
  if ( !mGLInitialized )
    return;

  // Delete all textures
  for ( auto it = mTileCache.begin(); it != mTileCache.end(); ++it )
  {
    if ( it.value().textureId )
    {
      glDeleteTextures( 1, &it.value().textureId );
    }
  }

  mTileCache.clear();

  QgsDebugMsgLevel( QStringLiteral( "GPU tile cache cleared" ), 2 );
}

void QgsRasterGPUTileUploader::evictOldTiles( quint64 currentFrame, int maxAge )
{
  if ( !mGLInitialized )
    return;

  QList<quint64> toRemove;

  for ( auto it = mTileCache.begin(); it != mTileCache.end(); ++it )
  {
    const quint64 age = currentFrame - it.value().lastUsedFrame;
    if ( age > static_cast<quint64>( maxAge ) )
    {
      toRemove.append( it.key() );
    }
  }

  for ( const quint64 key : toRemove )
  {
    GPUTile &tile = mTileCache[key];
    if ( tile.textureId )
    {
      glDeleteTextures( 1, &tile.textureId );
    }
    mTileCache.remove( key );
  }

  if ( !toRemove.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Evicted %1 old tiles from GPU cache" ).arg( toRemove.size() ), 2 );
  }
}

void QgsRasterGPUTileUploader::getCacheStats( int &cachedCount, qint64 &memoryBytes ) const
{
  cachedCount = mTileCache.size();
  memoryBytes = 0;

  for ( auto it = mTileCache.begin(); it != mTileCache.end(); ++it )
  {
    const GPUTile &tile = it.value();
    if ( tile.isValid )
    {
      // Approximate memory: width * height * bytes per pixel
      memoryBytes += tile.width * tile.height * mTextureFormat.bytesPerPixel;
    }
  }
}

QgsCOGTileReader::TileInfo QgsRasterGPUTileUploader::tileInfo( int overviewLevel ) const
{
  if ( mReader && mReader->isValid() )
  {
    return mReader->tileInfo( overviewLevel );
  }
  return QgsCOGTileReader::TileInfo();
}

QgsRectangle QgsRasterGPUTileUploader::rasterExtent() const
{
  if ( mReader && mReader->isValid() )
  {
    return mReader->extent();
  }
  return QgsRectangle();
}

int QgsRasterGPUTileUploader::selectBestOverview( double targetMupp ) const
{
  if ( mReader && mReader->isValid() )
  {
    return mReader->selectBestOverview( targetMupp );
  }
  return 0;
}

QgsRectangle QgsRasterGPUTileUploader::tileExtent( int overviewLevel, int tileX, int tileY ) const
{
  if ( !mReader || !mReader->isValid() )
  {
    return QgsRectangle();
  }

  // Get tile info
  const auto info = mReader->tileInfo( overviewLevel );
  if ( !info.isTiled )
  {
    return QgsRectangle();
  }

  // Get full raster extent
  const QgsRectangle extent = mReader->extent();

  // Calculate tile size in georeferenced units
  const double tileWidth = extent.width() / info.tilesX;
  const double tileHeight = extent.height() / info.tilesY;

  // Calculate tile extent
  // Note: Y axis is inverted (tile 0 is at top)
  const double minX = extent.xMinimum() + ( tileX * tileWidth );
  const double maxX = minX + tileWidth;
  const double maxY = extent.yMaximum() - ( tileY * tileHeight );
  const double minY = maxY - tileHeight;

  return QgsRectangle( minX, minY, maxX, maxY );
}
