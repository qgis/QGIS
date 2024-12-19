/***************************************************************************
  qgsterraindownloader.h
  --------------------------------------
  Date                 : March 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTERRAINDOWNLOADER_H
#define QGSTERRAINDOWNLOADER_H

#include "qgis_3d.h"

#include <memory>
#include <QByteArray>
#include <QImage>

#include "qgscoordinatetransformcontext.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsRasterLayer;
class QgsCoordinateTransformContext;

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Takes care of downloading terrain data from a publicly available data source.
 *
 * Currently using terrain tiles in Terrarium format hosted on AWS. More info:
 *
 * - data format: https://github.com/tilezen/joerd/blob/master/docs/formats.md
 * - data sources: https://github.com/tilezen/joerd/blob/master/docs/data-sources.md
 * - hosting: https://registry.opendata.aws/terrain-tiles/
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.8
 */
class _3D_EXPORT QgsTerrainDownloader
{

  public:

    /**
     * Constructs a QgsTerrainDownloader object
     * \param transformContext coordinate transform context
     */
    QgsTerrainDownloader( const QgsCoordinateTransformContext  &transformContext );

    ~QgsTerrainDownloader();

    //! Definition of data source for terrain tiles (assuming "terrarium" data encoding with usual XYZ tiling scheme)
    struct DataSource
    {
      QString uri;  //!< HTTP(S) template for XYZ tiles requests (e.g. http://example.com/{z}/{x}/{y}.png)
      int zMin = 0;  //!< Minimum zoom level (Z) with valid data
      int zMax = 0;  //!< Maximum zoom level (Z) with valid data
    };

    //! Returns the data source used by default
    static DataSource defaultDataSource();

    //! Configures data source to be used for download of terrain tiles
    void setDataSource( const DataSource &ds );

    //! Returns currently configured data source
    DataSource dataSource() const { return mDataSource; }

    /**
     * For given extent and resolution (number of pixels for width/height) in specified CRS, download necessary
     * tile images (if not cached already) and produce height map out of them (byte array of res*res float values)
     */
    QByteArray getHeightMap( const QgsRectangle &extentOrig, int res, const QgsCoordinateReferenceSystem &destCrs, const QgsCoordinateTransformContext &context = QgsCoordinateTransformContext(), QString tmpFilenameImg = QString(), QString tmpFilenameTif = QString() );

  private:

    /**
     * For the requested resolution given as map units per pixel, find out the best native tile resolution
     * (higher resolution = fewer map units per pixel)
     */
    double findBestTileResolution( double requestedMupp ) const;

    /**
     * Given extent and map units per pixels, adjust the extent and resolution
     */
    static void adjustExtentAndResolution( double mupp, const QgsRectangle &extentOrig, QgsRectangle &extent, int &res );

    /**
     * Takes an image tile with heights encoded using "terrarium" encoding and converts it to
     * an array of 32-bit floats with decoded elevations.
     */
    static void tileImageToHeightMap( const QImage &img, QByteArray &heightMap );

  private:
    DataSource mDataSource;
    std::unique_ptr<QgsRasterLayer> mOnlineDtm;
    double mXSpan = 0;   //!< Width of the tile at zoom level 0 in map units
};

#endif // QGSTERRAINDOWNLOADER_H
