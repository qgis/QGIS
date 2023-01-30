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

#include "qgsraster.h"

class QgsRasterBlockFeedback;
class QgsRasterIterator;
class QgsRasterPipe;
class QgsRectangle;
class QgsRasterDataProvider;
class QgsRasterInterface;

/**
 * \ingroup core
 * \brief The raster file writer which allows you to save a raster to a new file.
 */
class CORE_EXPORT QgsRasterFileWriter
{
  public:
    enum Mode
    {
      Raw = 0, //!< Raw data
      Image = 1 //!< Rendered image
    };
    enum WriterError
    {
      NoError = 0,
      SourceProviderError = 1,
      DestProviderError = 2,
      CreateDatasourceError = 3,
      WriteError = 4,
      NoDataConflict = 5, //!< Internal error if a value used for 'no data' was found in input
      WriteCanceled = 6, //!< Writing was manually canceled
    };

    /**
     * Options for sorting and filtering raster formats.
     * \since QGIS 3.0
     */
    enum RasterFormatOption
    {
      SortRecommended = 1 << 1, //!< Use recommended sort order, with extremely commonly used formats listed first
    };
    Q_DECLARE_FLAGS( RasterFormatOptions, RasterFormatOption )

    QgsRasterFileWriter( const QString &outputUrl );

    /**
     * Create a raster file with one band without initializing the pixel data.
     * Returned provider may be used to initialize the raster using writeBlock() calls.
     * Ownership of the returned provider is passed to the caller.
     * \returns Instance of data provider in editing mode (on success) or NULLPTR on error.
     * \note Does not work with tiled mode enabled.
     * \since QGIS 3.0
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
     * \since QGIS 3.0
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
     * \deprecated since QGIS 3.8, use version with transformContext instead
    */
    Q_DECL_DEPRECATED WriterError writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
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
    WriterError writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
                             const QgsCoordinateReferenceSystem &crs,
                             const QgsCoordinateTransformContext &transformContext,
                             QgsRasterBlockFeedback *feedback = nullptr );


    /**
     * Returns the output URL for the raster.
     * \since QGIS 3.0
     */
    QString outputUrl() const { return mOutputUrl; }

    void setOutputFormat( const QString &format ) { mOutputFormat = format; }
    QString outputFormat() const { return mOutputFormat; }

    void setOutputProviderKey( const QString &key ) { mOutputProviderKey = key; }
    QString outputProviderKey() const { return mOutputProviderKey; }

    void setTiledMode( bool t ) { mTiledMode = t; }
    bool tiledMode() const { return mTiledMode; }

    void setMaxTileWidth( int w ) { mMaxTileWidth = w; }
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

    QList< int > pyramidsList() const { return mPyramidsList; }
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

    void setMaxTileHeight( int h ) { mMaxTileHeight = h; }
    int maxTileHeight() const { return mMaxTileHeight; }

    void setCreateOptions( const QStringList &list ) { mCreateOptions = list; }
    QStringList createOptions() const { return mCreateOptions; }

    void setPyramidsConfigOptions( const QStringList &list ) { mPyramidsConfigOptions = list; }
    QStringList pyramidsConfigOptions() const { return mPyramidsConfigOptions; }

    //! Creates a filter for an GDAL driver key
    static QString filterForDriver( const QString &driverName );

    /**
     * Details of available filters and formats.
     * \since QGIS 3.0
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
     * \since QGIS 3.0
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
     * \since QGIS 3.0
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
     * \since QGIS 3.0
     */
    static QStringList extensionsForFormat( const QString &format );

  private:
    QgsRasterFileWriter(); //forbidden
    WriterError writeDataRaster( const QgsRasterPipe *pipe, QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
                                 const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext,
                                 QgsRasterBlockFeedback *feedback = nullptr );

    // Helper method used by previous one
    WriterError writeDataRaster( const QgsRasterPipe *pipe,
                                 QgsRasterIterator *iter,
                                 int nCols, int nRows,
                                 const QgsRectangle &outputExtent,
                                 const QgsCoordinateReferenceSystem &crs,
                                 Qgis::DataType destDataType,
                                 const QList<bool> &destHasNoDataValueList,
                                 const QList<double> &destNoDataValueList,
                                 QgsRasterDataProvider *destProvider,
                                 QgsRasterBlockFeedback *feedback = nullptr );

    WriterError writeImageRaster( QgsRasterIterator *iter, int nCols, int nRows, const QgsRectangle &outputExtent,
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

    Mode mMode = Raw;
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
