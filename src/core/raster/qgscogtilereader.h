/***************************************************************************
  qgscogtilereader.h - Fast tile reader for Cloud Optimized GeoTIFFs
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

#ifndef QGSCOGTILEREADER_H
#define QGSCOGTILEREADER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrectangle.h"

#include <QSize>
#include <QByteArray>
#include <gdal.h>

/**
 * \ingroup core
 * \class QgsCOGTileReader
 * \brief Fast, direct tile reader for Cloud Optimized GeoTIFFs (COGs).
 *
 * This class provides high-performance tile reading by bypassing QgsRasterBlock
 * overhead and using GDAL's native tile reading APIs (ReadBlock) when possible.
 *
 * Key features:
 * - Direct tile access via GDALReadBlock for optimal COG performance
 * - Overview level support for multi-resolution rendering
 * - Metadata caching to minimize GDAL calls
 * - Pre-allocated buffers to avoid memory allocation overhead
 * - GPU-friendly raw data output
 *
 * Usage:
 * \code{.cpp}
 * QgsCOGTileReader reader(gdalDataset);
 * if (reader.isValid()) {
 *   QByteArray tileData;
 *   if (reader.readTile(0, 5, 10, tileData)) {
 *     // Upload tileData directly to GPU texture
 *   }
 * }
 * \endcode
 *
 * \note This class is optimized for COG files. For non-tiled or non-COG rasters,
 * performance may not differ significantly from standard QgsRasterBlock.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsCOGTileReader
{
  public:

    /**
     * \brief Tile metadata structure
     */
    struct TileInfo
    {
      int width = 0;          //!< Tile width in pixels
      int height = 0;         //!< Tile height in pixels
      int tilesX = 0;         //!< Number of tiles in X direction
      int tilesY = 0;         //!< Number of tiles in Y direction
      GDALDataType dataType = GDT_Unknown;  //!< GDAL data type
      int bytesPerPixel = 0;  //!< Bytes per pixel
      int bandCount = 1;      //!< Number of bands
      bool isTiled = false;   //!< Whether the dataset is tiled
    };

    /**
     * \brief Constructor from GDAL dataset handle
     * \param dataset GDAL dataset handle (ownership is NOT transferred)
     */
    explicit QgsCOGTileReader( GDALDatasetH dataset );

    //! Default destructor
    ~QgsCOGTileReader() = default;

    /**
     * \brief Returns TRUE if the reader was successfully initialized
     */
    bool isValid() const { return mValid; }

    /**
     * \brief Returns the number of overview levels (0 = base resolution)
     */
    int overviewCount() const { return mOverviewCount; }

    /**
     * \brief Returns tile metadata for the specified overview level
     * \param overviewLevel Overview level (0 = base resolution)
     */
    TileInfo tileInfo( int overviewLevel = 0 ) const;

    /**
     * \brief Returns the dataset extent in the dataset's CRS
     */
    QgsRectangle extent() const { return mExtent; }

    /**
     * \brief Returns the width of the dataset at base resolution
     */
    int width() const { return mWidth; }

    /**
     * \brief Returns the height of the dataset at base resolution
     */
    int height() const { return mHeight; }

    /**
     * \brief Read a single tile into a pre-allocated buffer
     * \param overviewLevel Overview level (0 = base resolution, 1+ = overviews)
     * \param tileX Tile X index (0-based)
     * \param tileY Tile Y index (0-based)
     * \param bandNumber Band number (1-based, following GDAL convention)
     * \param outBuffer Output buffer (will be resized if needed)
     * \returns TRUE on success, FALSE on error
     *
     * The output buffer will contain raw pixel data in the native GDAL data type,
     * suitable for direct GPU texture upload. The buffer size will be:
     * tileWidth * tileHeight * bytesPerPixel
     */
    bool readTile( int overviewLevel, int tileX, int tileY, int bandNumber, QByteArray &outBuffer );

    /**
     * \brief Read multiple bands for a single tile (for RGB/RGBA)
     * \param overviewLevel Overview level (0 = base resolution)
     * \param tileX Tile X index (0-based)
     * \param tileY Tile Y index (0-based)
     * \param bandNumbers List of band numbers to read (1-based)
     * \param outBuffer Output buffer with interleaved band data
     * \returns TRUE on success, FALSE on error
     *
     * The output buffer will contain interleaved band data:
     * [B1_P0, B2_P0, B3_P0, B1_P1, B2_P1, B3_P1, ...]
     * where Bn_Pm = band n, pixel m
     */
    bool readTileMultiBand( int overviewLevel, int tileX, int tileY, const QList<int> &bandNumbers, QByteArray &outBuffer );

    /**
     * \brief Select the best overview level for a given map units per pixel value
     * \param targetMupp Target map units per pixel
     * \returns Best overview level (0 = base resolution)
     *
     * This method selects the coarsest overview that still provides sufficient
     * resolution for the target display resolution, minimizing data transfer.
     */
    int selectBestOverview( double targetMupp ) const;

  private:

    /**
     * \brief Initialize reader from GDAL dataset
     */
    bool initialize( GDALDatasetH dataset );

    /**
     * \brief Cache tile metadata for an overview level
     */
    bool cacheTileInfo( GDALRasterBandH band, int overviewIndex );

    //! GDAL dataset handle (NOT owned)
    GDALDatasetH mDataset = nullptr;

    //! Whether reader is valid
    bool mValid = false;

    //! Dataset dimensions
    int mWidth = 0;
    int mHeight = 0;

    //! Dataset extent
    QgsRectangle mExtent;

    //! Number of overviews
    int mOverviewCount = 0;

    //! Cached tile info per overview level (index 0 = base resolution)
    QVector<TileInfo> mTileInfoCache;

    //! Geotransform coefficients
    double mGeoTransform[6];

    //! Resolution (map units per pixel) at base level
    double mBaseResolutionX = 0.0;
    double mBaseResolutionY = 0.0;
};

#endif // QGSCOGTILEREADER_H
