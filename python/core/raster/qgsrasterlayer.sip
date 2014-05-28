/*! \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class QgsRasterLayer : QgsMapLayer
{
%TypeHeaderCode
#include <qgsrasterlayer.h>
%End

  public:
    /**  \brief Default cumulative cut lower limit */
    static const double CUMULATIVE_CUT_LOWER;

    /**  \brief Default cumulative cut upper limit */
    static const double CUMULATIVE_CUT_UPPER;

    /**  \brief Default sample size (number of pixels) for estimated statistics/histogram calculation */
    static const double SAMPLE_SIZE;

    /**  \brief Constructor. Provider is not set. */
    QgsRasterLayer();

    /** \brief This is the constructor for the RasterLayer class.
     *
     * The main tasks carried out by the constructor are:
     *
     * -Load the rasters default style (.qml) file if it exists
     *
     * -Populate the RasterStatsVector with initial values for each band.
     *
     * -Calculate the layer extents
     *
     * -Determine whether the layer is gray, paletted or multiband.
     *
     * -Assign sensible defaults for the red, green, blue and gray bands.
     *
     * -
     * */
    QgsRasterLayer( const QString & path,
                    const QString &  baseName = QString::null,
                    bool loadDefaultStyleFlag = true );

    /**  \brief [ data provider interface ] Constructor in provider mode */
    QgsRasterLayer( const QString & uri,
                    const QString & baseName,
                    const QString & providerKey,
                    bool loadDefaultStyleFlag = true );

    /** \brief The destructor */
    ~QgsRasterLayer();

    /** \brief This enumerator describes the types of shading that can be used */
    enum ColorShadingAlgorithm
    {
      UndefinedShader,
      PseudoColorShader,
      FreakOutShader,
      ColorRampShader,
      UserDefinedShader
    };

    /** \brief This enumerator describes the type of raster layer */
    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband,
      ColorLayer
    };

    /** This helper checks to see whether the file name appears to be a valid
     *  raster file name.  If the file name looks like it could be valid,
     *  but some sort of error occurs in processing the file, the error is
     *  returned in retError.
     */
    static bool isValidRasterFileName( const QString & theFileNameQString, QString &retError );
    static bool isValidRasterFileName( const QString & theFileNameQString );

    /** Return time stamp for given file name */
    static QDateTime lastModified( const QString &  name );

    /**  [ data provider interface ] Set the data provider */
    void setDataProvider( const QString & provider );

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType();

    /**Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* theRenderer /Transfer/ );
    QgsRasterRenderer* renderer() const;

    /**Set raster resample filter. Takes ownership of the resample filter object*/
    QgsRasterResampleFilter * resampleFilter() const;

    QgsBrightnessContrastFilter * brightnessFilter() const;
    QgsHueSaturationFilter * hueSaturationFilter() const;

    /** Get raster pipe */
    QgsRasterPipe * pipe();

    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int width() const;

    /** \brief Accessor that returns the height of the (unclipped) raster */
    int height() const;

    /** \brief Get the number of bands in this layer  */
    int bandCount() const;

    /** \brief Get the name of a band given its number  */
    const  QString bandName( int theBandNoInt );

    /** Returns the data provider */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
      @note available in python bindings as constDataProvider()
     */
    const QgsRasterDataProvider* dataProvider() const /PyName=constDataProvider/;

    /**Synchronises with changes in the datasource
    @note added in version 1.6*/
    virtual void reload();

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) /Factory/;

    /** \brief This is called when the view on the raster layer needs to be redrawn */
    bool draw( QgsRenderContext& rendererContext );

    /** \brief This is an overloaded version of the draw() function that is called by both draw() and thumbnailAsPixmap */
    void draw( QPainter * theQPainter,
               QgsRasterViewPort * myRasterViewPort,
               const QgsMapToPixel* theQgsMapToPixel = 0 );

    /**Returns a list with classification items (Text and color)
      @note this method was added in version 1.8*/
    QList< QPair< QString, QColor > > legendSymbologyItems() const;

    /** \brief Obtain GDAL Metadata for this layer */
    QString metadata();

    /** \brief Get an 100x100 pixmap of the color palette. If the layer has no palette a white pixmap will be returned */
    QPixmap paletteAsPixmap( int theBandNumber = 1 );

    /**  \brief [ data provider interface ] Which provider is being used for this Raster Layer?
     * @note added in 2.0
     */
    QString providerType() const;

    /** \brief Returns the number of raster units per each raster pixel. In a world file, this is normally the first row (without the sign) */
    double rasterUnitsPerPixelX();
    double rasterUnitsPerPixelY();

    /** \brief Set contrast enhancement algorithm
     *  @param theAlgorithm Contrast enhancement algorithm
     *  @param theLimits Limits
     *  @param theExtent Extent used to calculate limits, if empty, use full layer extent
     *  @param theSampleSize Size of data sample to calculate limits, if 0, use full resolution
     *  @param theGenerateLookupTableFlag Generate llokup table. */


    void setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm,
                                 QgsRaster::ContrastEnhancementLimits theLimits = QgsRaster::ContrastEnhancementMinMax,
                                 QgsRectangle theExtent = QgsRectangle(),
                                 int theSampleSize = QgsRasterLayer::SAMPLE_SIZE,
                                 bool theGenerateLookupTableFlag = true );

    /** \brief Set default contrast enhancement */
    void setDefaultContrastEnhancement();

    /** \brief Overloaded version of the above function for convenience when restoring from xml */
    void setDrawingStyle( const QString & theDrawingStyleQString );

    /**  \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int theValue );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Draws a preview of the rasterlayer into a pixmap
    @note - use previewAsImage() for rendering with QGIS>=2.4 */
    QPixmap previewAsPixmap( QSize size, QColor bgColor = QColor( 255, 255, 255 ) ) /Deprecated/;

    /** \brief Draws a preview of the rasterlayer into a QImage
     @note added in 2.4 */
    QImage previewAsImage( QSize size, QColor bgColor = QColor( 255, 255, 255 ),
                           QImage::Format format = QImage::Format_ARGB32_Premultiplied );

    /** \brief Emit a signal asking for a repaint. (inherited from maplayer) */
    void triggerRepaint();

    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    virtual void setLayerOrder( const QStringList &layers );

    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( QString name, bool vis );

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const;

  public slots:
    void showStatusMessage( const QString & theMessage );

    //! @deprecated since 2.4 - does nothing
    void updateProgress( int, int ) /Deprecated/;

    /** \brief receive progress signal from provider */
    void onProgress( int, double, QString );

  signals:
    /** \brief Signal for notifying listeners of long running processes */
    void progressUpdate( int theValue );

    /**
     *   This is emitted whenever data or metadata (e.g. color table, extent) has changed
     *   @note added in 1.7
     */
    void dataChanged();

  protected:
    /** \brief Read the symbology for the current layer from the Dom node supplied */
    bool readSymbology( const QDomNode& node, QString& errorMessage );

    /** \brief Reads layer specific state from project file Dom node */
    bool readXml( const QDomNode& layer_node );

    /** \brief Write the symbology for the layer into the docment provided */
    bool writeSymbology( QDomNode&, QDomDocument& doc, QString& errorMessage ) const;

    /** \brief Write layer specific state to project file Dom node */
    bool writeXml( QDomNode & layer_node, QDomDocument & doc );

};

