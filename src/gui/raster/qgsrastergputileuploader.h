/***************************************************************************
  qgsrastergputileuploader.h - Fast GPU tile uploader for rasters
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

#ifndef QGSRASTERGPUTILEUPLOADER_H
#define QGSRASTERGPUTILEUPLOADER_H

#include "qgis_gui.h"
#include "qgscogtilereader.h"
#include "qgsrastertextureformats.h"

#include <QOpenGLFunctions>
#include <QHash>
#include <QByteArray>

/**
 * \ingroup gui
 * \class QgsRasterGPUTileUploader
 * \brief High-performance GPU tile uploader using QgsCOGTileReader.
 *
 * This class combines fast COG tile reading (via QgsCOGTileReader) with
 * efficient GPU texture uploads, providing the complete fast path for
 * GPU-accelerated raster rendering.
 *
 * Key features:
 * - Zero-copy texture uploads (raw data → GPU)
 * - Format-aware texture creation
 * - Tile cache management
 * - Automatic overview selection
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsRasterGPUTileUploader : protected QOpenGLFunctions
{
  public:

    /**
     * \brief GPU tile structure
     */
    struct GPUTile
    {
      GLuint textureId = 0;          //!< OpenGL texture ID
      int width = 0;                 //!< Tile width in pixels
      int height = 0;                //!< Tile height in pixels
      quint64 lastUsedFrame = 0;     //!< Frame number when last used
      bool isValid = false;          //!< Whether texture is valid
    };

    /**
     * \brief Constructor
     * \param reader COG tile reader (ownership NOT transferred)
     */
    explicit QgsRasterGPUTileUploader( QgsCOGTileReader *reader );

    //! Destructor - releases GPU resources
    ~QgsRasterGPUTileUploader();

    /**
     * \brief Upload a tile to GPU
     * \param overviewLevel Overview level
     * \param tileX Tile X index
     * \param tileY Tile Y index
     * \param bandNumber Band number (1-based)
     * \returns GPU tile info, or invalid tile on failure
     */
    GPUTile uploadTile( int overviewLevel, int tileX, int tileY, int bandNumber = 1 );

    /**
     * \brief Get or create GPU tile (with caching)
     * \param overviewLevel Overview level
     * \param tileX Tile X index
     * \param tileY Tile Y index
     * \param bandNumber Band number
     * \param frameNumber Current frame number (for LRU)
     * \returns GPU tile from cache or newly uploaded
     */
    GPUTile getTile( int overviewLevel, int tileX, int tileY, int bandNumber, quint64 frameNumber );

    /**
     * \brief Clear tile cache and release GPU resources
     */
    void clearCache();

    /**
     * \brief Evict old tiles from cache
     * \param currentFrame Current frame number
     * \param maxAge Max age in frames before eviction
     */
    void evictOldTiles( quint64 currentFrame, int maxAge = 120 );

    /**
     * \brief Get cache statistics
     * \param[out] cachedCount Number of cached tiles
     * \param[out] memoryBytes Approximate GPU memory used
     */
    void getCacheStats( int &cachedCount, qint64 &memoryBytes ) const;

  private:

    /**
     * \brief Create tile cache key
     */
    static quint64 makeTileKey( int overview, int tileX, int tileY, int band );

    //! COG tile reader
    QgsCOGTileReader *mReader = nullptr;

    //! Texture format info
    QgsRasterTextureFormat::FormatInfo mTextureFormat;

    //! Tile cache
    QHash<quint64, GPUTile> mTileCache;

    //! Temporary buffer for tile data
    QByteArray mTileBuffer;

    //! Whether OpenGL is initialized
    bool mGLInitialized = false;
};

#endif // QGSRASTERGPUTILEUPLOADER_H
