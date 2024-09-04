/***************************************************************************
    qgsrasterfilewriter.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERFILEWRITER_H
#define QGSRASTERFILEWRITER_H

#include "qgis_core.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QStringList>

class QgsRasterBlockFeedback;
class QgsRasterIterator;
class QgsRasterPipe;
class QgsRectangle;
class QgsRasterDataProvider;
class QgsRasterInterface;

/**
 * \ingroup core
 * \brief The raster file writer which allows you to save a raster to a new file.
 *
 * The writer defaults to creating GeoTIFF outputs using GDAL. Alternative formats and
 * data providers can be used by calling setOutputFormat() and setOutputProviderKey().
 */
class CORE_EXPORT QgsRasterFileWriter
{
  public:

    /**
     * Options for sorting and filtering raster formats.
     */
    enum RasterFormatOption SIP_ENUM_BASETYPE( IntFlag )
    {
      SortRecommended = 1 << 1, //!< Use recommended sort order, with extremely commonly used formats listed first
    };
    Q_DECLARE_FLAGS( RasterFormatOptions, RasterFormatOption )

    /**
     * Constructor for QgsRasterFileWriter, writing to the specified output URL/filename.
     */
    QgsRasterFileWriter( const QString &outputUrl );

    /**
     * Create a raster file with one band without initializing the pixel data.
     * Returned provider may be used to initialize the raster using writeBlock() calls.
     * Ownership of the returned provider is passed to the caller.
     * \returns Instance of data provider in editing mode (on success) or NULLPTR on error.
     * \note Does not work with tiled mode enabled.
     */
    QgsRasterDataProvider *createOneBandRaster( Qgis::DataType dataType,
        int width, int height,
        const QgsRectangle &extent,
        const QgsCoordinateReferenceSystem &crs ) SIP_FACTORY;

    /**
     * Create a raster file with given number of bands without initializing the pixel data.
     * Returned provider may be used to initialize the raster using writeBlock() calls.
     * Ownership of the returned provider is passed to the caller.
     * \returns Instance of data provider in editing mode (on success) or NULLPTR on error.
     * \note Does not work with tiled mode enabled.
     */
    QgsRasterDataProvider *createMultiBandRaster( Qgis::DataType dataType,
        int width, int height,
        const QgsRectangle &extent,
        const QgsCoordinateReferenceSystem &crs,
        int nBands ) SIP_FACTORY;


    /**
     * Write raster file
     * \param pipe raster pipe
     * \param nCols number of output columns
     * \param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
     * \param outputExtent extent to output
     * \param crs crs to reproject to
     * \param feedback optional feedback object for progress reports
     * \deprecated QGIS 3.8. Use version with transformContext instead.
    */
    Q_DECL_DEPRECATED Qgis::RasterFileWriterResult writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback = nullptr ) SIP_DEPRECATED;

    /**
     * Write raster file
     * \param pipe raster pipe
     * \param nCols number of output columns
     * \param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
     * \param outputExtent extent to output
     * \param crs crs to reproject to
     * \param transformContext coordinate transform context
     * \param feedback optional feedback object for progress reports
     * \since QGIS 3.8
    */
    Qgis::RasterFileWriterResult writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs,
        const QgsCoordinateTransformContext &transformContext,
        QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * Returns the output URL (filename) for the raster.
     */
    QString outputUrl() const { return mOutputUrl; }

    /**
     * Sets the output \a format.
     *
     * For GDAL disk based outputs this should match the GDAL driver name, e.g. "GTiff" for GeoTiff exports.
     *
     * \see outputFormat()
     */
    void setOutputFormat( const QString &format ) { mOutputFormat = format; }

    /**
     * Returns the output format.
     *
     * For GDAL disk based outputs this will match the GDAL driver name, e.g. "GTiff" for GeoTiff exports.
     *
     * \see setOutputFormat()
     */
    QString outputFormat() const { return mOutputFormat; }

    /**
     * Sets the name of the data provider for the raster output.
     *
     * E.g. set to "gdal" to use GDAL to create disk based raster files.
     *
     * \see outputProviderKey()
     */
    void setOutputProviderKey( const QString &key ) { mOutputProviderKey = key; }

    /**
     * Returns the name of the data provider for the raster output.
     *
     * \see setOutputProviderKey()
     */
    QString outputProviderKey() const { return mOutputProviderKey; }

    /**
     * Sets whether the output should be tiled.
     *
     * Tiled outputs will automatically split the raster into multiple parts, based on the
     * maxTileWidth() value.
     *
     * \see tiledMode()
     */
    void setTiledMode( bool t ) { mTiledMode = t; }

    /**
     * Returns whether the output will be tiled.
     *
     * \see setTiledMode()
     */
    bool tiledMode() const { return mTiledMode; }

    /**
     * Sets the maximum tile width (in pixels) for tiled outputs.
     *
     * \see maxTileWidth()
     * \see setMaxTileHeight()
     * \see tiledMode()
     */
    void setMaxTileWidth( int w ) { mMaxTileWidth = w; }

    /**
     * Returns the maximum tile width (in pixels) for tiled outputs.
     *
     * \see maxTileHeight()
     * \see setMaxTileWidth()
     * \see tiledMode()
     */
    int maxTileWidth() const { return mMaxTileWidth; }

    /**
     * Returns the pyramid building option.
     *
     * \see setBuildPyramidsFlag()
     */
    Qgis::RasterBuildPyramidOption buildPyramidsFlag() const { return mBuildPyramidsFlag; }

    /**
     * Sets the pyramid building option.
     *
     * \see buildPyramidsFlag()
     */
    void setBuildPyramidsFlag( Qgis::RasterBuildPyramidOption f ) { mBuildPyramidsFlag = f; }

    /**
     * Returns the list of pyramids which will be created for the output file.
     *
     * \see setPyramidsList()
     */
    QList< int > pyramidsList() const { return mPyramidsList; }

    /**
     * Sets the \a list of pyramids which will be created for the output file.
     *
     * \see pyramidsList()
     */
    void setPyramidsList( const QList< int > &list ) { mPyramidsList = list; }

    QString pyramidsResampling() const { return mPyramidsResampling; }
    void setPyramidsResampling( const QString &str ) { mPyramidsResampling = str; }

    /**
     * Returns the raster pyramid format.
     *
     * \see setPyramidsFormat()
     */
    Qgis::RasterPyramidFormat pyramidsFormat() const { return mPyramidsFormat; }

    /**
     * Sets the raster pyramid format.
     *
     * \see pyramidsFormat()
     */
    void setPyramidsFormat( Qgis::RasterPyramidFormat f ) { mPyramidsFormat = f; }

    /**
     * Sets the maximum tile height (in pixels) for tiled outputs.
     *
     * \see maxTileHeight()
     * \see setMaxTileWidth()
     * \see tiledMode()
     */
    void setMaxTileHeight( int h ) { mMaxTileHeight = h; }

    /**
     * Returns the maximum tile height (in pixels) for tiled outputs.
     *
     * \see maxTileWidth()
     * \see setMaxTileHeight()
     * \see tiledMode()
     */
    int maxTileHeight() const { return mMaxTileHeight; }

    /**
     * Sets a list of data source creation options to use when
     * creating the output raster file.
     *
     * \see createOptions()
     */
    void setCreateOptions( const QStringList &list ) { mCreateOptions = list; }

    /**
     * Returns the list of data source creation options which will be used when
     * creating the output raster file.
     *
     * \see setCreateOptions()
     */
    QStringList createOptions() const { return mCreateOptions; }

    /**
     * Sets a \a list of configuration options to use when
     * creating the pyramids for the output raster file.
     *
     * \see pyramidsConfigOptions()
     */
    void setPyramidsConfigOptions( const QStringList &list ) { mPyramidsConfigOptions = list; }

    /**
     * Returns the list of configuration options used when
     * creating the pyramids for the output raster file.
     *
     * \see setPyramidsConfigOptions()
     */
    QStringList pyramidsConfigOptions() const { return mPyramidsConfigOptions; }

    //! Creates a filter for an GDAL driver key
    static QString filterForDriver( const QString &driverName );

    /**
     * Details of available filters and formats.
     */
    struct FilterFormatDetails
    {
      //! Unique driver name
      QString driverName;

      //! Filter string for file picker dialogs
      QString filterString;
    };

    /**
     * Returns a list or pairs, with format filter string as first element and GDAL format key as second element.
     * Relies on GDAL_DMD_EXTENSIONS metadata, if it is empty corresponding driver will be skipped even if supported.
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned formats.
     *
     * \see supportedFormatExtensions()
     */
    static QList< QgsRasterFileWriter::FilterFormatDetails > supportedFiltersAndFormats( RasterFormatOptions options = SortRecommended );

    /**
     * Returns a list of file extensions for supported formats.
     *
     * The \a options argument can be used to control the sorting and filtering of
     * returned formats.
     *
     * \see supportedFiltersAndFormats()
     */
    static QStringList supportedFormatExtensions( RasterFormatOptions options = SortRecommended );

    /**
     * Returns the GDAL driver name for a specified file \a extension. E.g. the
     * driver name for the ".tif" extension is "GTiff".
     * If no suitable drivers are found then an empty string is returned.
     *
     * Note that this method works for all GDAL drivers, including those without create support
     * (and which are not supported by QgsRasterFileWriter).
     *
     */
    static QString driverForExtension( const QString &extension );

    /**
     * Returns a list of known file extensions for the given GDAL driver \a format.
     * E.g. returns "tif", "tiff" for the format "GTiff".
     *
     * If no matching format driver is found an empty list will be returned.
     *
     * Note that this method works for all GDAL drivers, including those without create support
     * (and which are not supported by QgsRasterFileWriter).
     *
     */
    static QStringList extensionsForFormat( const QString &format );

  private:
    QgsRasterFileWriter(); //forbidden
    Qgis::RasterFileWriterResult writeDataRaster( const QgsRasterPipe *pipe, QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext,
        QgsRasterBlockFeedback *feedback = nullptr );

    // Helper method used by previous one
    Qgis::RasterFileWriterResult writeDataRaster( const QgsRasterPipe *pipe,
        QgsRasterIterator *iter,
        int nCols, int nRows,
        const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs,
        Qgis::DataType destDataType,
        const QList<bool> &destHasNoDataValueList,
        const QList<double> &destNoDataValueList,
        QgsRasterDataProvider *destProvider,
        QgsRasterBlockFeedback *feedback = nullptr );

    Qgis::RasterFileWriterResult writeImageRaster( QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
        const QgsCoordinateReferenceSystem &crs,
        QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * \brief Initialize vrt member variables
     *  \param xSize width of vrt
     *  \param ySize height of vrt
     *  \param crs coordinate system of vrt
     *  \param geoTransform optional array of transformation matrix values
     *  \param type datatype of vrt
     *  \param destHasNoDataValueList TRUE if destination has no data value, indexed from 0
     *  \param destNoDataValueList no data value, indexed from 0
     */
    void createVRT( int xSize, int ySize, const QgsCoordinateReferenceSystem &crs, double *geoTransform, Qgis::DataType type, const QList<bool> &destHasNoDataValueList, const QList<double> &destNoDataValueList );
    //write vrt document to disk
    bool writeVRT( const QString &file );
    //add file entry to vrt
    void addToVRT( const QString &filename, int band, int xSize, int ySize, int xOffset, int yOffset );
    void buildPyramids( const QString &filename, QgsRasterDataProvider *destProviderIn = nullptr );

    //! Create provider and datasource for a part image (vrt mode)
    QgsRasterDataProvider *createPartProvider( const QgsRectangle &extent, int nCols, int iterCols, int iterRows,
        int iterLeft, int iterTop,
        const QString &outputUrl, int fileIndex, int nBands, Qgis::DataType type,
        const QgsCoordinateReferenceSystem &crs );

    /**
     * \brief Init VRT (for tiled mode) or create global output provider (single-file mode)
     *  \param nCols number of tile columns
     *  \param nRows number of tile rows
     *  \param crs coordinate system of vrt
     *  \param geoTransform optional array of transformation matrix values
     *  \param nBands number of bands
     *  \param type datatype of vrt
     *  \param destHasNoDataValueList TRUE if destination has no data value, indexed from 0
     *  \param destNoDataValueList no data value, indexed from 0
     */
    QgsRasterDataProvider *initOutput( int nCols, int nRows,
                                       const QgsCoordinateReferenceSystem &crs, double *geoTransform, int nBands,
                                       Qgis::DataType type,
                                       const QList<bool> &destHasNoDataValueList = QList<bool>(), const QList<double> &destNoDataValueList = QList<double>() );

    //! Calculate nRows, geotransform and pixel size for output
    void globalOutputParameters( const QgsRectangle &extent, int nCols, int &nRows, double *geoTransform, double &pixelSize );

    QString partFileName( int fileIndex );
    QString vrtFileName();

    Qgis::RasterExportType mMode = Qgis::RasterExportType::Raw;
    QString mOutputUrl;
    QString mOutputProviderKey = QStringLiteral( "gdal" );
    QString mOutputFormat = QStringLiteral( "GTiff" );
    QStringList mCreateOptions;
    QgsCoordinateReferenceSystem mOutputCRS;

    //! False: Write one file, TRUE: create a directory and add the files numbered
    bool mTiledMode = false;
    int mMaxTileWidth = 500;
    int mMaxTileHeight = 500;

    QList< int > mPyramidsList;
    QString mPyramidsResampling = QStringLiteral( "AVERAGE" );
    Qgis::RasterBuildPyramidOption mBuildPyramidsFlag = Qgis::RasterBuildPyramidOption::No;
    Qgis::RasterPyramidFormat mPyramidsFormat = Qgis::RasterPyramidFormat::GeoTiff;
    QStringList mPyramidsConfigOptions;

    QDomDocument mVRTDocument;
    QList<QDomElement> mVRTBands;

    QgsRasterBlockFeedback *mFeedback = nullptr;

    const QgsRasterPipe *mPipe = nullptr;
    const QgsRasterInterface *mInput = nullptr;
};

#endif // QGSRASTERFILEWRITER_H
