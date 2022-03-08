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
     * \returns TRUE on success
     * \since QGIS 3.8
     */
    static bool resampleSingleBandRaster( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, GDALResampleAlg resampleAlg, const char *pszCoordinateOperation );

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

    /**
     * Converts an \a image to a GDAL memory dataset by borrowing image data.
     *
     * \warning The \a image must exist unchanged for the lifetime of the returned gdal dataset!
     *
     * \since QGIS 3.12
     */
    static gdal::dataset_unique_ptr imageToMemoryDataset( const QImage &image );

    /**
     * Converts an raster \a block to a  single band GDAL memory dataset.
     *
     * \warning The \a block must stay allocated for the lifetime of the returned gdal dataset.
     *
     * \since QGIS 3.26
     */
    static gdal::dataset_unique_ptr blockToSingleBandMemoryDataset( int pixelWidth, int pixelHeight, const QgsRectangle &extent, void *block,  GDALDataType dataType );

    /**
     * This is a copy of GDALAutoCreateWarpedVRT optimized for imagery using RPC georeferencing
     * that also sets RPC_HEIGHT in GDALCreateGenImgProjTransformer2 based on HEIGHT_OFF.
     * By default GDAL would assume that the imagery has zero elevation - if that is not the case,
     * the image would not be shown in the correct location.
     *
     * \since QGIS 3.14
     */
    static GDALDatasetH rpcAwareAutoCreateWarpedVrt(
      GDALDatasetH hSrcDS,
      const char *pszSrcWKT,
      const char *pszDstWKT,
      GDALResampleAlg eResampleAlg,
      double dfMaxError,
      const GDALWarpOptions *psOptionsIn );

    /**
     * This is a wrapper around GDALCreateGenImgProjTransformer2() that takes into account RPC
     * georeferencing (it sets RPC_HEIGHT in GDALCreateGenImgProjTransformer2 based on HEIGHT_OFF).
     * By default GDAL would assume that the imagery has zero elevation - if that is not the case,
     * the image would not be shown in the correct location.
     *
     * \since QGIS 3.16
     */
    static void *rpcAwareCreateTransformer( GDALDatasetH hSrcDS, GDALDatasetH hDstDS = nullptr, char **papszOptions = nullptr );

#ifndef QT_NO_NETWORKPROXY
    //! Sets the gdal proxy variables
    static void setupProxy();
#endif

    /**
     * Returns TRUE if the dataset at the specified \a path is considered "cheap" to open.
     *
     * Datasets which are considered cheap to open may correspond to very small file sizes, or data types
     * which only require some inexpensive header parsing to open.
     *
     * One use case for this method is to test whether a remote dataset can be safely opened
     * to resolve the geometry types and other metadata without causing undue network traffic.
     *
     * The \a smallFileSizeLimit argument specifies the maximum file size (in bytes) which will
     * be considered as small.
     *
     * \since QGIS 3.22
     */
    static bool pathIsCheapToOpen( const QString &path, int smallFileSizeLimit = 50000 );

    /**
     * Returns a list of file extensions which potentially contain multiple layers representing
     * GDAL raster or vector layers.
     *
     * \since QGIS 3.22
     */
    static QStringList multiLayerFileExtensions();

    /**
     * Returns TRUE if the VRT file at the specified path is a VRT matching
     * the given layer \a type.
     *
     * \since QGIS 3.22
     */
    static bool vrtMatchesLayerType( const QString &vrtPath, QgsMapLayerType type );

    friend class TestQgsGdalUtils;
};

#endif // QGSGDALUTILS_H
