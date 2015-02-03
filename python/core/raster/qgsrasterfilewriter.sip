
/**  Raster file writer */

class QgsRasterFileWriter
{
%TypeHeaderCode
#include <qgsrasterfilewriter.h>
#include <qgsrasterdataprovider.h>
%End
  public:
    enum Mode
    {
      Raw,   // Raw data
      Image, // Rendered image
    };
    enum WriterError
    {
      NoError,
      SourceProviderError,
      DestProviderError,
      CreateDatasourceError,
      WriteError,
      // Internal error if a value used for 'no data' was found in input
      NoDataConflict,
    };

    QgsRasterFileWriter( const QString& outputUrl );
    ~QgsRasterFileWriter();

    /**Write raster file
        @param pipe raster pipe
        @param nCols number of output columns
        @param nRows number of output rows (or -1 to automatically calculate row number to have square pixels)
        @param outputExtent extent to output
        @param crs crs to reproject to
        @param p dialog to show progress in */
    WriterError writeRaster( const QgsRasterPipe* pipe, int nCols, int nRows, QgsRectangle outputExtent,
                             const QgsCoordinateReferenceSystem& crs, QProgressDialog* p = 0 );

    void setOutputFormat( const QString& format );
    QString outputFormat() const;

    void setOutputProviderKey( const QString& key );
    QString outputProviderKey() const;

    void setTiledMode( bool t );
    bool tiledMode() const;

    void setMaxTileWidth( int w );
    int maxTileWidth() const;

    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
    void setBuildPyramidsFlag( QgsRaster::RasterBuildPyramids f );

    QList< int > pyramidsList() const;
    void setPyramidsList( const QList< int > & list );

    QString pyramidsResampling() const;
    void setPyramidsResampling( const QString & str );

    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
    void setPyramidsFormat( QgsRaster::RasterPyramidsFormat f );

    void setMaxTileHeight( int h );
    int maxTileHeight() const;

    void setCreateOptions( const QStringList& list );
    QStringList createOptions() const;

    void setPyramidsConfigOptions( const QStringList& list );
    QStringList pyramidsConfigOptions() const;

};
