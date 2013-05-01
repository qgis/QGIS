
/** Base class for raster data providers
 *
 *  \note  This class has been copied and pasted from
 *         QgsVectorDataProvider, and does not yet make
 *         sense for Raster layers.
 */

class QgsRasterDataProvider : QgsDataProvider, QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterdataprovider.h>
#include <qgsrasterinterface.h>
#include <qgsrasteridentifyresult.h>
%End

  public:

    QgsRasterDataProvider();

    QgsRasterDataProvider( const QString & uri );

    virtual ~QgsRasterDataProvider();

    virtual QgsRasterInterface * clone() const = 0;

    /* It makes no sense to set input on provider */
    bool setInput( QgsRasterInterface* input );

    // TODO: Document this better.
    /** \brief   Renders the layer as an image
     */
    virtual QImage* draw( const QgsRectangle & viewExtent, int pixelWidth, int pixelHeight ) = 0;

    // TODO: Get the supported formats by this provider

    // TODO: Get the file masks supported by this provider, suitable for feeding into the file open dialog box

    virtual QgsRectangle extent() = 0;

    /** Returns data type for the band specified by number */
    virtual QGis::DataType dataType( int bandNo ) const = 0;

    /** Returns source data type for the band specified by number,
     *  source data type may be shorter than dataType
     */
    virtual QGis::DataType srcDataType( int bandNo ) const = 0;

    /** Returns data type for the band specified by number */
    virtual int colorInterpretation( int theBandNo ) const;

    QString colorName( int colorInterpretation ) const;

    /** Reload data (data could change) */
    virtual bool reload();

    virtual QString colorInterpretationName( int theBandNo ) const;

    /** Get block size */
    virtual int xBlockSize() const;
    virtual int yBlockSize() const;

    /** Get raster size */
    virtual int xSize() const;
    virtual int ySize() const;

    /** read block of data  */
    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) / Factory /;

    /* Return true if source band has no data value */
    virtual bool srcHasNoDataValue( int bandNo ) const;

    /** \brief Get source nodata value usage */
    virtual bool useSrcNoDataValue( int bandNo ) const;

    /** \brief Set source nodata value usage */
    virtual void setUseSrcNoDataValue( int bandNo, bool use );

    /** Value representing no data value. */
    virtual double srcNoDataValue( int bandNo ) const;

    virtual void setUserNoDataValue( int bandNo, QgsRasterRangeList noData );

    /** Get list of user no data value ranges */
    virtual  QgsRasterRangeList userNoDataValues( int bandNo ) const;

    virtual QList<QgsColorRampShader::ColorRampItem> colorTable( int bandNo ) const;

    // Defined in parent
    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Create pyramid overviews */
    virtual QString buildPyramids( const QList<QgsRasterPyramid> & thePyramidList,
                                   const QString & theResamplingMethod = "NEAREST",
                                   QgsRaster::RasterPyramidsFormat theFormat = QgsRaster::PyramidsGTiff,
                                   const QStringList & theConfigOptions = QStringList() );
 
    /** \brief Accessor for ths raster layers pyramid list.
     * @param overviewList used to construct the pyramid list (optional), when empty the list is defined by the provider.
     * A pyramid list defines the
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the
     * list.
     */
    virtual QList<QgsRasterPyramid> buildPyramidList( QList<int> overviewList = QList<int>() );

    /** \brief Returns true if raster has at least one populated histogram. */
    bool hasPyramids();

    /**
     * Get metadata in a format suitable for feeding directly
     * into a subset of the GUI raster properties "Metadata" tab.
     */
    virtual QString metadata() = 0;

    virtual QgsRasterIdentifyResult identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 );

    //QMap<QString, QString> identify( const QgsPoint & thePoint, const QgsRectangle &theExtent = QgsRectangle(), int theWidth = 0, int theHeight = 0 );

    /**
     * \brief   Returns the caption error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     */
    virtual QString lastErrorTitle() = 0;

    /**
     * \brief   Returns the verbose error text for the last error in this provider
     *
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     *
     */
    virtual QString lastError() = 0;

    /**
     * \brief   Returns the format of the error text for the last error in this provider
     *
     * \note added in 1.6
     */
    virtual QString lastErrorFormat();

    /**Returns the dpi of the output device.
      @note: this method was added in version 1.2*/
    int dpi() const;

    /**Sets the output device resolution.
      @note: this method was added in version 1.2*/
    void setDpi( int dpi );

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const;

    /** Current time stamp of data source */
    virtual QDateTime dataTimestamp() const;

    /**Writes into the provider datasource*/
    // TODO: add data type (may be defferent from band type)
    virtual bool write( void* data, int band, int width, int height, int xOffset, int yOffset );

    /** Creates a new dataset with mDataSourceURI
        @return true in case of success*/
    static QgsRasterDataProvider* create( const QString &providerKey,
                         const QString &uri,
                         const QString& format, int nBands,
                         QGis::DataType type,
                         int width, int height, double* geoTransform,
                         const QgsCoordinateReferenceSystem& crs,
                         QStringList createOptions = QStringList() /*e.v. color table*/ );

    /** Set no data value on created dataset
     *  @param bandNo band number
     *  @param noDataValue no data value
     */
    virtual bool setNoDataValue( int bandNo, double noDataValue );

    /** Remove dataset*/
    virtual bool remove();

    /** Returns a list of pyramid resampling method names for given provider */
    static QList<QPair<QString,QString> > pyramidResamplingMethods( QString providerKey );

    /** Validates creation options for a specific dataset and destination format.
     * @note used by GDAL provider only
     * @note see also validateCreationOptionsFormat() in gdal provider for validating options based on format only */
     virtual QString validateCreationOptions( const QStringList& createOptions, QString format );

    /** Validates pyramid creation options for a specific dataset and destination format
     * @note used by GDAL provider only */
    virtual QString validatePyramidsConfigOptions( QgsRaster::RasterPyramidsFormat pyramidsFormat,
        const QStringList & theConfigOptions, const QString & fileFormat );

   signals:
    /** Emit a signal to notify of the progress event.
      * Emited theProgress is in percents (0.0-100.0) */
    void progress( int theType, double theProgress, QString theMessage );
    void progressUpdate( int theProgress );

  protected:

    /** Fill in histogram defaults if not specified */
    void initHistogram( QgsRasterHistogram &theHistogram, int theBandNo,
                        int theBinCount,
                        double theMinimum,
                        double theMaximum,
                        const QgsRectangle & theExtent = QgsRectangle(),
                        int theSampleSize = 0,
                        bool theIncludeOutOfRange = false );

    /** Fill in statistics defaults if not specified */
    void initStatistics( QgsRasterBandStats &theStatistics, int theBandNo,
                         int theStats = QgsRasterBandStats::All,
                         const QgsRectangle & theExtent = QgsRectangle(),
                         int theBinCount = 0 );

};
