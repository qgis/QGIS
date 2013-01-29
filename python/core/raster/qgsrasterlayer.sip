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


    //
    // Enums, structs and typedefs
    //
    /** \brief This enumerator describes the types of shading that can be used */
    enum ColorShadingAlgorithm
    {
      UndefinedShader,
      PseudoColorShader,
      FreakOutShader,
      ColorRampShader,
      UserDefinedShader
    };

    /** \brief This enumerator describes the different kinds of drawing we can do */
    enum DrawingStyle
    {
      UndefinedDrawingStyle,
      SingleBandGray,                 // a single band image drawn as a range of gray colors
      SingleBandPseudoColor,          // a single band image drawn using a pseudocolor algorithm
      PalettedColor,                  // a "Palette" image drawn using color table
      PalettedSingleBandGray,         // a "Palette" layer drawn in gray scale
      PalettedSingleBandPseudoColor,  // a "Palette" layerdrawn using a pseudocolor algorithm
      PalettedMultiBandColor,         // currently not supported
      MultiBandSingleBandGray,        // a layer containing 2 or more bands, but a single band drawn as a range of gray colors
      MultiBandSingleBandPseudoColor, // a layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
      MultiBandColor,                 // a layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color.
      SingleBandColorDataStyle        // ARGB values rendered directly
    };

    /** \brief This enumerator describes the type of raster layer */
    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband,
      ColorLayer
    };

    /** \brief Contrast enhancement limits */
    enum ContrastEnhancementLimits
    {
      ContrastEnhancementNone,
      ContrastEnhancementMinMax,
      ContrastEnhancementStdDev,
      ContrastEnhancementCumulativeCut
    };

    /** \brief A list containing on ContrastEnhancement object per raster band in this raster layer */
    typedef QList<QgsContrastEnhancement> ContrastEnhancementList;

    /** \brief  A list containing one RasterPyramid struct per raster band in this raster layer.
     * POTENTIAL pyramid layer. This works by dividing the height
     * and width of the raster by an incrementing number. As soon as the result
     * of the division is <=256 we stop allowing RasterPyramid structs
     * to be added to the list. Each time a RasterPyramid is created
     * we will check to see if a pyramid matching these dimensions already exists
     * in the raster layer, and if so mark the exists flag as true */
    /* typedef QList<QgsRasterPyramid> RasterPyramidList; */

    /** \brief  A list containing one RasterBandStats struct per raster band in this raster layer.
     * Note that while every RasterBandStats element will have the name and number of its associated
     * band populated, any additional stats are calculated on a need to know basis.*/
    /* typedef QList<QgsRasterBandStats> RasterStatsList; */

    //
    // Static methods:
    //
    static void buildSupportedRasterFileFilter( QString & fileFilters );

    /** This helper checks to see whether the file name appears to be a valid
     *  raster file name.  If the file name looks like it could be valid,
     *  but some sort of error occurs in processing the file, the error is
     *  returned in retError.
     */
    static bool isValidRasterFileName( const QString & theFileNameQString, QString &retError );
    static bool isValidRasterFileName( const QString & theFileNameQString );
    //static QStringList subLayers( GDALDatasetH dataset );

    /** Return time stamp for given file name */
    static QDateTime lastModified( const QString &  name );

    // Keep this for now, it is used by Python interface!!!
    /** \brief ensures that GDAL drivers are registered, but only once */
    static void registerGdalDrivers();

    //
    // Non Static inline methods
    //

    /** \brief Initialize default values */
    void init();

    /**  [ data provider interface ] Set the data provider */
    void setDataProvider( const QString & provider );

    /** \brief  Accessor for blue band name mapping */
    QString blueBandName() const;

    /** \brief Accessor for color shader algorithm */
    QgsRasterLayer::ColorShadingAlgorithm colorShadingAlgorithm() const;

    /** \brief Accessor for contrast enhancement algorithm */
    QgsContrastEnhancement::ContrastEnhancementAlgorithm contrastEnhancementAlgorithm();

    /** \brief Returns contrast enhancement algorithm as a string */
    QString contrastEnhancementAlgorithmAsString() const;

    /** \brief Accessor for drawing style */
    DrawingStyle drawingStyle();

    /** \brief Accessor for mHasPyramids (READ ONLY) */
    /* bool hasPyramids(); */

    /** \brief Accessor for mUserDefinedGrayMinimumMaximum */
    bool hasUserDefinedGrayMinimumMaximum() const;

    /** \brief Accessor for mUserDefinedRGBMinimumMaximum */
    bool hasUserDefinedRGBMinimumMaximum() const;

    /** \brief Accessor that returns the height of the (unclipped) raster */
    int height();

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType();


    /** \brief Mutator for drawing style */
    void setDrawingStyle( const DrawingStyle &  theDrawingStyle );
    /**Sets corresponding renderer for style*/
    void setRendererForDrawingStyle( const DrawingStyle &  theDrawingStyle );

    /** \brief Mutator to alter the number of standard deviations that should be plotted */
    void setStandardDeviations( double theStandardDeviations );

    /** \brief Mutator for mUserDefinedGrayMinimumMaximum */
    void setUserDefinedGrayMinimumMaximum( bool theBool );

    /** \brief Mutator for mUserDefinedRGBMinimumMaximum */
    void setUserDefinedRGBMinimumMaximum( bool theBool );

    /**Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* theRenderer /Transfer/ );
    QgsRasterRenderer* renderer() const;

    /**Set raster resample filter. Takes ownership of the resample filter object*/
    //void setResampleFilter( QgsRasterResampleFilter* resampleFilter /Transfer/ );
    QgsRasterResampleFilter * resampleFilter() const;

    /** Get raster pipe */
    QgsRasterPipe * pipe();

    /** \brief Accessor to find out how many standard deviations are being plotted */
    double standardDeviations() const;

    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int width();

    //
    // Non Static methods
    //
    /** \brief Get the number of bands in this layer  */
    unsigned int bandCount() const;

    /** \brief Get the name of a band given its number  */
    const  QString bandName( int theBandNoInt );

    /** \brief Get the number of a band given its name. The name is the rewritten name set
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    int bandNumber( const QString & theBandName ) const;

    /** \brief Accessor for ths raster layers pyramid list. A pyramid list defines the
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the
     * list.
     */
    // RasterPyramidList buildPyramidList();

    /** \brief Accessor for color shader algorithm */
    QString colorShadingAlgorithmAsString() const;

    /**  \brief Compute the actual minimum maximum pixel values based on the current (last) display extent
      \note added in v1.6 */
    //void computeMinimumMaximumFromLastExtent( int theBand, double& theMin /Out/, double& theMax /Out/ );

    /** \brief Get a pointer to the contrast enhancement for the selected band */
    QgsContrastEnhancement* contrastEnhancement( unsigned int theBand );

    const QgsContrastEnhancement* constContrastEnhancement( unsigned int theBand ) const;

    /** \brief Get a pointer to the color table */
    QList<QgsColorRampShader::ColorRampItem> colorTable( int theBandNoInt );

    /** Returns the data provider */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner */
    const QgsRasterDataProvider* constDataProvider() const;
%MethodCode
      sipRes = sipCpp->dataProvider();
%End

    /**Synchronises with changes in the datasource
    @note added in version 1.6*/
    virtual void reload();

    /** \brief This is called when the view on the raster layer needs to be redrawn */
    bool draw( QgsRenderContext& rendererContext );

    /** \brief This is an overloaded version of the draw() function that is called by both draw() and thumbnailAsPixmap */
    void draw( QPainter * theQPainter,
               QgsRasterViewPort * myRasterViewPort,
               const QgsMapToPixel* theQgsMapToPixel = 0 );

    /** \brief Returns a string representation of drawing style
     *
     * Implemented mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future!. DrawingStyle drawingStyle() instead.
     * */
    QString drawingStyleAsString() const;

    /** \brief Identify raster value(s) found on the point position */
    //bool identify( const QgsPoint & point, QMap<QString, QString>& results /Out/ );

    /** \brief Identify raster value(s) found on the point position */
//    bool identifyMap( const QgsPoint & point, QMap<int, QString>& results /Out/ );
//%MethodCode
//      sipRes = sipCpp->identify( *a0, *a1 );
//%End

    /** \brief Identify arbitrary details from the WMS server found on the point position */
    //QString identifyAsText( const QgsPoint & point );

    /** \brief Identify arbitrary details from the WMS server found on the point position
     * @note added in 1.5
     */
    //QString identifyAsHtml( const QgsPoint & point );

    /** \brief Currently returns always false */
    bool isEditable() const;

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure  */
    QString lastError();

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure */
    QString lastErrorTitle();

    /**Returns a list with classification items (Text and color)
      @note this method was added in version 1.8*/
    QList< QPair< QString, QColor > > legendSymbologyItems() const;

    /** \brief Accessor for maximum value user for contrast enhancement */
    double maximumValue( unsigned int theBand );

    /** \brief Accessor for maximum value user for contrast enhancement */
    double maximumValue( QString theBand );

    /** \brief Obtain GDAL Metadata for this layer */
    QString metadata();

    /** \brief Accessor for minimum value user for contrast enhancement */
    double minimumValue( unsigned int theBand );

    /** \brief Accessor for minimum value user for contrast enhancement */
    double minimumValue( QString theBand );

    /** \brief Get an 100x100 pixmap of the color palette. If the layer has no palette a white pixmap will be returned */
    QPixmap paletteAsPixmap( int theBandNumber = 1 );

    /**  \brief [ data provider interface ] Which provider is being used for this Raster Layer?
     * @note added in 2.0
     */
    QString providerType() const;

    /** \brief Returns the number of raster units per each raster pixel. In a world file, this is normally the first row (without the sign) */
    double rasterUnitsPerPixel();

    /** \brief Read color table from GDAL raster band */
    // bool readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList );

    /** \brief Simple reset function that set the noDataValue back to the value stored in the first raster band */
    //void resetNoDataValue();

    static QString contrastEnhancementLimitsAsString( QgsRasterLayer::ContrastEnhancementLimits theLimits );
    static ContrastEnhancementLimits contrastEnhancementLimitsFromString( QString theLimits );

    /** \brief Mutator for contrast enhancement algorithm using min/max */
    // TODO: remove in 2.0, replaced by following
    // void setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm,
    //                                       bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for contrast enhancement algorithm
     *  @param theAlgorithm Contrast enhancement algorithm
     *  @param theLimits Limits
     *  @param theExtent Extent used to calculate limits, if empty, use full layer extent
     *  @param theSampleSize Size of data sample to calculate limits, if 0, use full resolution
     *  @param theGenerateLookupTableFlag Generate llokup table. */


    void setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm,
                                          ContrastEnhancementLimits theLimits = ContrastEnhancementMinMax,
                                          QgsRectangle theExtent = QgsRectangle(),
                                          int theSampleSize = QgsRasterLayer::SAMPLE_SIZE,
                                          bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for contrast enhancement algorithm */
    void setContrastEnhancementAlgorithm( QString theAlgorithm, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for contrast enhancement function */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction );

    /** \brief Set default contrast enhancement */
    void setDefaultContrastEnhancement();

    /** \brief Overloaded version of the above function for convenience when restoring from xml */
    void setDrawingStyle( const QString & theDrawingStyleQString );

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Sets the minimum and maximum values for the band(s) currently
     * being displayed using the only pixel values from the last/current extent
     * */
    void setMinimumMaximumUsingLastExtent();

    /** \brief Sets the minimum and maximum values for the band(s) currently
     * being displayed using the only pixel values from the dataset min/max */
    void setMinimumMaximumUsingDataset();

    /**  \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int theValue );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Draws a preview of the rasterlayer into a pixmap */
    QPixmap previewAsPixmap( QSize size, QColor bgColor = QColor( 255, 255, 255 ) );

    /** \brief Emit a signal asking for a repaint. (inherited from maplayer) */
    void triggerRepaint();

    //
    // Virtual methods
    //
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
    /** \brief Create GDAL pyramid overviews */
    // QString buildPyramids( const RasterPyramidList &,
    //                        const QString &  theResamplingMethod = "NEAREST",
    //                        bool theTryInternalFlag = false );

    void showStatusMessage( const QString & theMessage );

    /** \brief Propagate progress updates from GDAL up to the parent app */
    void updateProgress( int, int );

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

