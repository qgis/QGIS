/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterfilewriter.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterFileWriter
{
%Docstring
The raster file writer which allows you to save a raster to a new file.
%End

%TypeHeaderCode
#include "qgsrasterfilewriter.h"
%End
  public:
    enum Mode
    {
      Raw,
      Image
    };
    enum WriterError
    {
      NoError,
      SourceProviderError,
      DestProviderError,
      CreateDatasourceError,
      WriteError,
      NoDataConflict,
      WriteCanceled,
    };

    enum RasterFormatOption
    {
      SortRecommended,
    };
    typedef QFlags<QgsRasterFileWriter::RasterFormatOption> RasterFormatOptions;


    QgsRasterFileWriter( const QString &outputUrl );

    QgsRasterDataProvider *createOneBandRaster( Qgis::DataType dataType,
        int width, int height,
        const QgsRectangle &extent,
        const QgsCoordinateReferenceSystem &crs ) /Factory/;
%Docstring
Create a raster file with one band without initializing the pixel data.
Returned provider may be used to initialize the raster using writeBlock() calls.
Ownership of the returned provider is passed to the caller.

:return: Instance of data provider in editing mode (on success) or None on error.

.. note::

   Does not work with tiled mode enabled.

.. versionadded:: 3.0
%End

    QgsRasterDataProvider *createMultiBandRaster( Qgis::DataType dataType,
        int width, int height,
        const QgsRectangle &extent,
        const QgsCoordinateReferenceSystem &crs,
        int nBands ) /Factory/;
%Docstring
Create a raster file with given number of bands without initializing the pixel data.
Returned provider may be used to initialize the raster using writeBlock() calls.
Ownership of the returned provider is passed to the caller.

:return: Instance of data provider in editing mode (on success) or None on error.

.. note::

   Does not work with tiled mode enabled.

.. versionadded:: 3.0
%End

    WriterError writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
                             const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback = 0 );
%Docstring
Write raster file

:param pipe: raster pipe
:param nCols: number of output columns
:param nRows: number of output rows (or -1 to automatically calculate row number to have square pixels)
:param outputExtent: extent to output
:param crs: crs to reproject to
:param feedback: optional feedback object for progress reports
%End

    QString outputUrl() const;
%Docstring
Returns the output URL for the raster.

.. versionadded:: 3.0
%End

    void setOutputFormat( const QString &format );
    QString outputFormat() const;

    void setOutputProviderKey( const QString &key );
    QString outputProviderKey() const;

    void setTiledMode( bool t );
    bool tiledMode() const;

    void setMaxTileWidth( int w );
    int maxTileWidth() const;

    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
    void setBuildPyramidsFlag( QgsRaster::RasterBuildPyramids f );

    QList< int > pyramidsList() const;
    void setPyramidsList( const QList< int > &list );

    QString pyramidsResampling() const;
    void setPyramidsResampling( const QString &str );

    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat f );

    void setMaxTileHeight( int h );
    int maxTileHeight() const;

    void setCreateOptions( const QStringList &list );
    QStringList createOptions() const;

    void setPyramidsConfigOptions( const QStringList &list );
    QStringList pyramidsConfigOptions() const;

    static QString filterForDriver( const QString &driverName );
%Docstring
Creates a filter for an GDAL driver key
%End

    struct FilterFormatDetails
    {
      QString driverName;

      QString filterString;
    };

    static QList< QgsRasterFileWriter::FilterFormatDetails > supportedFiltersAndFormats( RasterFormatOptions options = SortRecommended );
%Docstring
Returns a list or pairs, with format filter string as first element and GDAL format key as second element.
Relies on GDAL_DMD_EXTENSIONS metadata, if it is empty corresponding driver will be skipped even if supported.

The ``options`` argument can be used to control the sorting and filtering of
returned formats.

.. seealso:: :py:func:`supportedOutputRasterLayerExtensions`
%End

    static QStringList supportedFormatExtensions( RasterFormatOptions options = SortRecommended );
%Docstring
Returns a list of file extensions for supported formats.

The ``options`` argument can be used to control the sorting and filtering of
returned formats.

.. versionadded:: 3.0

.. seealso:: :py:func:`supportedFiltersAndFormats`
%End

    static QString driverForExtension( const QString &extension );
%Docstring
Returns the GDAL driver name for a specified file ``extension``. E.g. the
driver name for the ".tif" extension is "GTiff".
If no suitable drivers are found then an empty string is returned.

.. versionadded:: 3.0
%End

    static QStringList extensionsForFormat( const QString &format );
%Docstring
Returns a list of known file extensions for the given GDAL driver ``format``.
E.g. returns "tif", "tiff" for the format "GTiff".

If no matching format driver is found an empty list will be returned.

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterfilewriter.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
