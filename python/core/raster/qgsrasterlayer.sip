/** \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class QgsRasterLayer : QgsMapLayer
{
%TypeHeaderCode
#include <qgsrasterlayer.h>
%End

  public:

    /** \brief Default sample size (number of pixels) for estimated statistics/histogram calculation */
    static const double SAMPLE_SIZE;

    /** \brief Constructor. Provider is not set. */
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
    QgsRasterLayer( const QString &uri,
                    const QString &baseName = QString(),
                    const QString &providerKey = "gdal",
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
    static bool isValidRasterFileName( const QString & fileNameQString, QString &retError );
    static bool isValidRasterFileName( const QString & fileNameQString );

    /** Return time stamp for given file name */
    static QDateTime lastModified( const QString &  name );

    /** [ data provider interface ] Set the data provider */
    void setDataProvider( const QString & provider );

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType();

    /** Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* renderer /Transfer/ );
    QgsRasterRenderer* renderer() const;

    /** Set raster resample filter. Takes ownership of the resample filter object*/
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
    QString bandName( int bandNoInt ) const;

    /** Returns the data provider */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
      @note available in python bindings as constDataProvider()
     */
    const QgsRasterDataProvider* dataProvider() const /PyName=constDataProvider/;

    /** Synchronises with changes in the datasource */
    virtual void reload();

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) /Factory/;

    /** \brief This is an overloaded version of the draw() function that is called by both draw() and thumbnailAsPixmap */
    void draw( QPainter * theQPainter,
               QgsRasterViewPort * myRasterViewPort,
               const QgsMapToPixel* qgsMapToPixel = 0 );

    /** Returns a list with classification items (Text and color) */
    QList< QPair< QString, QColor > > legendSymbologyItems() const;

    virtual bool isSpatial() const;

    /** \brief Obtain GDAL Metadata for this layer */
    QString metadata() const;

    /** \brief Get an 100x100 pixmap of the color palette. If the layer has no palette a white pixmap will be returned */
    QPixmap paletteAsPixmap( int bandNumber = 1 );

    /** \brief [ data provider interface ] Which provider is being used for this Raster Layer? */
    QString providerType() const;

    /** \brief Returns the number of raster units per each raster pixel. In a world file, this is normally the first row (without the sign) */
    double rasterUnitsPerPixelX();
    double rasterUnitsPerPixelY();

    /** \brief Set contrast enhancement algorithm
     *  @param algorithm Contrast enhancement algorithm
     *  @param limits Limits
     *  @param extent Extent used to calculate limits, if empty, use full layer extent
     *  @param sampleSize Size of data sample to calculate limits, if 0, use full resolution
     *  @param generateLookupTableFlag Generate lookup table. */


    void setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
                                 QgsRasterMinMaxOrigin::Limits limits = QgsRasterMinMaxOrigin::MinMax,
                                 const QgsRectangle& extent = QgsRectangle(),
                                 int sampleSize = QgsRasterLayer::SAMPLE_SIZE,
                                 bool generateLookupTableFlag = true );

    /** \brief Set default contrast enhancement */
    void setDefaultContrastEnhancement();

    /** \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int value );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Draws a preview of the rasterlayer into a QImage
     @note added in 2.4 */
    QImage previewAsImage( QSize size, const QColor& bgColor = QColor( 255, 255, 255 ),
                           QImage::Format format = QImage::Format_ARGB32_Premultiplied );

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
    virtual void setSubLayerVisibility( const QString& name, bool vis );

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const;

  public slots:
    void showStatusMessage( const QString & message );

    /** \brief receive progress signal from provider */
    void onProgress( int, double, const QString& );

  signals:
    /** \brief Signal for notifying listeners of long running processes */
    void progressUpdate( int value );

    /**
     *   This is emitted whenever data or metadata (e.g. color table, extent) has changed
     */
    void dataChanged();

  protected:
    /** \brief Read the symbology for the current layer from the Dom node supplied */
    bool readSymbology( const QDomNode& node, QString& errorMessage );

    /** \brief Read the style information for the current layer from the Dom node supplied */
    bool readStyle(const QDomNode &node, QString &errorMessage);

    /** \brief Reads layer specific state from project file Dom node */
    bool readXml( const QDomNode& layer_node );

    /** \brief Write the symbology for the layer into the docment provided */
    bool writeSymbology( QDomNode&, QDomDocument& doc, QString& errorMessage ) const;

    /** \brief Write the style for the layer into the docment provided */
    bool writeStyle(QDomNode &node, QDomDocument &doc, QString &errorMessage) const;

    /** \brief Write layer specific state to project file Dom node */
    bool writeXml( QDomNode & layer_node, QDomDocument & doc ) const;
};

