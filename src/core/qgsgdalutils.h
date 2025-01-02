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

class QgsRasterBlock;

/**
 * \ingroup core
 * \class QgsGdalOption
 * \brief Encapsulates the definition of a GDAL configuration option.
 *
 * \note not available in Python bindings
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsGdalOption
{
  public:

    /**
     * Option types
     */
    enum class Type
    {
      Invalid, //!< Invalid option
      Select, //!< Selection option
      Boolean, //!< Boolean option
      Text, //!< Text option
      Int, //!< Integer option
      Double, //!< Double option
    };

    //! Option name
    QString name;

    //! Option type
    Type type = Type::Invalid;

    //! Option description
    QString description;

    //! Available choices, for Select options
    QStringList options;

    //! Default value
    QVariant defaultValue;

    //! Minimum acceptable value
    QVariant minimum;

    //! Maximum acceptable value
    QVariant maximum;

    //! Option scope
    QString scope;

    /**
     * Creates a QgsGdalOption from an XML \a node.
     *
     * Returns an invalid option if the node could not be interpreted
     * as a GDAL option.
     */
    static QgsGdalOption fromXmlNode( const CPLXMLNode *node );

    /**
     * Returns a list of all GDAL options from an XML \a node.
     */
    static QList< QgsGdalOption > optionsFromXml( const CPLXMLNode *node );
};


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
     *
     * In case of different CRS, the parameter \a pszCoordinateOperation is the Proj coordinate operation string, that
     * can be obtained with QgsCoordinateTransformContext::calculateCoordinateOperation()
     *
     * \returns TRUE on success
     * \since QGIS 3.8
     */
    static bool resampleSingleBandRaster( GDALDatasetH hSrcDS, GDALDatasetH hDstDS, GDALResampleAlg resampleAlg, const char *pszCoordinateOperation );

    /**
     * Resamples a single band raster to the destination dataset with different resolution and different CRS.
     * Ideally the source dataset should cover the whole area or the destination dataset.
     *
     * \note If possible, it is preferable to use the overload method with parameter \a pszCoordinateOperation.
     *       But if it is not possible or it fails to obtain the Proj coordinate operation string,
     *       this function is an alternative.
     *
     * \returns TRUE on success
     * \since QGIS 3.30
     */
    static bool resampleSingleBandRaster( GDALDatasetH hSrcDS,
                                          GDALDatasetH hDstDS,
                                          GDALResampleAlg resampleAlg,
                                          const QgsCoordinateReferenceSystem &sourceCrs,
                                          const QgsCoordinateReferenceSystem &destinationCrs );

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
     * Converts a data \a block to a single band GDAL memory dataset.
     *
     * \warning The data \a block must stay allocated for the lifetime of the returned gdal dataset.
     *
     * \since QGIS 3.26
     */
    static gdal::dataset_unique_ptr blockToSingleBandMemoryDataset( int pixelWidth, int pixelHeight, const QgsRectangle &extent, void *block,  GDALDataType dataType );

    /**
     * Converts a raster \a block to a single band GDAL memory dataset.
     *
     * \warning The raster \a block must stay allocated for the lifetime of the returned gdal dataset.
     *
     * \since QGIS 3.30
     */
    static gdal::dataset_unique_ptr blockToSingleBandMemoryDataset( const QgsRectangle &extent, QgsRasterBlock *block );

    /**
     * Converts a raster \a block to a single band GDAL memory dataset with \a rotation angle,side sizes of the grid,
     * origin if the grid (top left if rotation == 0)
     *
     * \warning The raster \a block must stay allocated for the lifetime of the returned gdal dataset.
     *
     * \since QGIS 3.30
     */
    static gdal::dataset_unique_ptr blockToSingleBandMemoryDataset( double rotation, const QgsPointXY &origin, double gridXSize,  double gridYSize,   QgsRasterBlock *block );

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

    /**
     * Returns the GDAL data type corresponding to the QGIS data type \a dataType
     *
     * \since QGIS 3.30
     */
    static GDALDataType gdalDataTypeFromQgisDataType( Qgis::DataType dataType );

    /**
     * Returns the GDAL resampling method corresponding to the QGIS resampling  \a method
     *
     * \since QGIS 3.30
     */
    static GDALResampleAlg gdalResamplingAlgorithm( Qgis::RasterResamplingMethod method );

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
     * Returns a the vsi prefix which corresponds to a file \a path, or an empty
     * string if the path is not associated with a vsi prefix.
     *
     * \since QGIS 3.32
     */
    static QString vsiPrefixForPath( const QString &path );

    /**
     * Returns a list of vsi prefixes which correspond to archive style containers (eg vsizip).
     *
     * \since QGIS 3.32
     */
    static QStringList vsiArchivePrefixes();

    /**
     * Encapsulates details for a GDAL VSI network file system.
     *
     * \since QGIS 3.40
     */
    struct VsiNetworkFileSystemDetails
    {
      //! VSI handler identifier, eg "vsis3"
      QString identifier;

      //! Translated, user-friendly name.
      QString name;
    };

    /**
     * Returns a list of available GDAL VSI network file systems.
     *
     * \since QGIS 3.40
     */
    static QList< VsiNetworkFileSystemDetails > vsiNetworkFileSystems();

    /**
     * Returns TRUE if \a prefix is a supported archive style container prefix (e.g. "/vsizip/").
     *
     * \since QGIS 3.32
     */
    static bool isVsiArchivePrefix( const QString &prefix );

    /**
     * Returns a list of file extensions which correspond to archive style containers supported by GDAL (e.g. "zip").
     *
     * \since QGIS 3.32
     */
    static QStringList vsiArchiveFileExtensions();

    /**
     * Returns TRUE if a file \a extension is a supported archive style container (e.g. ".zip").
     *
     * \since QGIS 3.32
     */
    static bool isVsiArchiveFileExtension( const QString &extension );

    /**
     * Returns the VSI handler type for a given VSI \a prefix.
     *
     * \since QGIS 3.40
     */
    static Qgis::VsiHandlerType vsiHandlerType( const QString &prefix );

    /**
     * Attempts to apply VSI credential \a options.
     *
     * This method uses GDAL's VSISetPathSpecificOption, which will overrwrite any existing
     * options for the same VSI \a prefix and \a path.
     *
     * Returns TRUE if the options could be applied.
     *
     * \since QGIS 3.40
     */
    static bool applyVsiCredentialOptions( const QString &prefix, const QString &path, const QVariantMap &options );

    /**
     * Returns TRUE if the VRT file at the specified path is a VRT matching
     * the given layer \a type.
     *
     * \since QGIS 3.22
     */
    static bool vrtMatchesLayerType( const QString &vrtPath, Qgis::LayerType type );

    /**
     * Returns the URL for the GDAL documentation for the specified \a driver.
     *
     * \since QGIS 3.40
     */
    static QString gdalDocumentationUrlForDriver( GDALDriverH hDriver );

    friend class TestQgsGdalUtils;
};

#endif // QGSGDALUTILS_H
