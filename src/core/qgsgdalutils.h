/***************************************************************************
                             qgsgdalutils.h
                             --------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGDALUTILS_H
#define QGSGDALUTILS_H

#define SIP_NO_FILE

#include "qgis_core.h"
#include <gdal.h>

#include "qgsogrutils.h"

/**
 * \ingroup core
 * \class QgsGdalUtils
 * \brief Utilities for working with GDAL
 *
 * \note not available in Python bindings
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsGdalUtils
{
  public:

    /**
     * Reads whether a driver supports GDALCreate() for raster purposes.
     * \param driver GDAL driver
     * \returns TRUE if a driver supports GDALCreate() for raster purposes.
     */
    static bool supportsRasterCreate( GDALDriverH driver );

    /**
     * Creates a new single band memory dataset with given parameters
     * \since QGIS 3.8
     */
    static gdal::dataset_unique_ptr createSingleBandMemoryDataset( GDALDataType dataType, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs );

    /**
     * Creates a new multi band memory dataset with given parameters
     * \since QGIS 3.12
     */
    static gdal::dataset_unique_ptr createMultiBandMemoryDataset( GDALDataType dataType, int bands, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs );

    /**
     * Creates a new single band TIFF dataset with given parameters
     * \since QGIS 3.8
     */
    static gdal::dataset_unique_ptr createSingleBandTiffDataset( const QString &filename, GDALDataType dataType, const QgsRectangle &extent, int width, int height, const QgsCoordinateReferenceSystem &crs );

    /**
     * Resamples a single band raster to the destination dataset with different resolution (and possibly with different CRS).
     * Ideally the source dataset should cover the whole area or the destination dataset.
     * \since QGIS 3.8
     */
    static void resampleSingleBandRaster( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, GDALResampleAlg resampleAlg );

    /**
     * Resamples a QImage \a image using GDAL resampler.
     * \since QGIS 3.12
     */
    static QImage resampleImage( const QImage &image, QSize outputSize, GDALRIOResampleAlg resampleAlg );

    /**
     * Gets creation options metadata for a given format
     * \since QGIS 3.10
     */
    static QString helpCreationOptionsFormat( const QString &format );

    /**
     * Validates creation options for a given format, regardless of layer.
     * \since QGIS 3.10
     */
    static QString validateCreationOptionsFormat( const QStringList &createOptions, const QString &format );

    /**
     * Helper function
     * \since QGIS 3.10
     */
    static char **papszFromStringList( const QStringList &list );

  private:

    /**
     * Converts an \a image to a GDAL memory dataset.
     *
     * \warning The \a image must exist unchanged for the lifetime of the returned gdal dataset!
     *
     * \since QGIS 3.12
     */
    static gdal::dataset_unique_ptr imageToMemoryDataset( const QImage &image );

    friend class TestQgsGdalUtils;
};

#endif // QGSGDALUTILS_H
