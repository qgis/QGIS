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

    QgsRasterFileWriter( const QString &outputUrl );

    QgsRasterDataProvider *createOneBandRaster( Qgis::DataType dataType,
        int width, int height,
        const QgsRectangle &extent,
        const QgsCoordinateReferenceSystem &crs ) /Factory/;
%Docstring
 Create a raster file with one band without initializing the pixel data.
 Returned provider may be used to initialize the raster using writeBlock() calls.
 Ownership of the returned provider is passed to the caller.
.. note::

   Does not work with tiled mode enabled.
 \returns Instance of data provider in editing mode (on success) or null on error.
.. versionadded:: 3.0
%End

    WriterError writeRaster( const QgsRasterPipe *pipe, int nCols, int nRows, const QgsRectangle &outputExtent,
                             const QgsCoordinateReferenceSystem &crs, QgsRasterBlockFeedback *feedback = 0 );
%Docstring
 Write raster file
\param pipe raster pipe
\param nCols number of output columns
\param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
\param outputExtent extent to output
\param crs crs to reproject to
\param feedback optional feedback object for progress reports
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

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterfilewriter.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
