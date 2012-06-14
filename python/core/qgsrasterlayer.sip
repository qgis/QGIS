/*! \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class QgsRasterLayer : QgsMapLayer
{
%TypeHeaderCode
#include <qgsrasterpyramid.h>
#include <qgsrasterlayer.h>
#include <qgscontrastenhancement.h>
#include <qgsrastertransparency.h>
#include <qgsrastershader.h>
%End

public:
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
    QgsRasterLayer( const QString & path = QString::null,
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
      PalettedColor,                  //a "Palette" image drawn using color table
      PalettedSingleBandGray,        // a "Palette" layer drawn in gray scale
      PalettedSingleBandPseudoColor, // a "Palette" layerdrawn using a pseudocolor algorithm
      PalettedMultiBandColor,         // currently not supported
      MultiBandSingleBandGray,        // a layer containing 2 or more bands, but a single band drawn as a range of gray colors
      MultiBandSingleBandPseudoColor, //a layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
      MultiBandColor                  //a layer containing 2 or more bands, mapped to RGB color space.
                                      //In the case of a multiband with only two bands, one band will be mapped to more than one color.
    };

    /** \brief This enumerator describes the type of raster layer */
    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband
    } ;

    /** \brief A list containing on ContrastEnhancement object per raster band in this raster layer */
    typedef QList<QgsContrastEnhancement> ContrastEnhancementList;

    /** \brief  A list containing one RasterPyramid struct per raster band in this raster layer.
     * POTENTIAL pyramid layer. This works by dividing the height
     * and width of the raster by an incrementing number. As soon as the result
     * of the division is <=256 we stop allowing RasterPyramid structs
     * to be added to the list. Each time a RasterPyramid is created
     * we will check to see if a pyramid matching these dimensions already exists
     * in the raster layer, and if so mark the exists flag as true */
    typedef QList<QgsRasterPyramid> RasterPyramidList;

    /** \brief  A list containing one RasterBandStats struct per raster band in this raster layer.
     * Note that while every RasterBandStats element will have the name and number of its associated
     * band populated, any additional stats are calculated on a need to know basis.*/
    typedef QList<QgsRasterBandStats> RasterStatsList;





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

    /** Return time stamp for given file name */
    static QDateTime lastModified( const QString &  name );

    /** \brief ensures that GDAL drivers are registered, but only once */
    static void registerGdalDrivers();




    //
    // Non Static inline methods
    //

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

    /** \brief Accessor for gray band name mapping */
    QString grayBandName() const;

    /** \brief Accessor for green band name mapping */
    QString greenBandName() const;

    /** \brief Accessor for mHasPyramids (READ ONLY) */
    bool hasPyramids();

    /** \brief Accessor for mUserDefinedGrayMinimumMaximum */
    bool hasUserDefinedGrayMinimumMaximum() const;

    /** \brief Accessor for mUserDefinedRGBMinimumMaximum */
    bool hasUserDefinedRGBMinimumMaximum() const;

    /** \brief Accessor that returns the height of the (unclipped) raster */
    int height();

    /** \brief Accessor to find out whether the histogram should be inverted  */
    bool invertHistogram() const;

    /** \brief Is the NoDataValue Valid */
    bool isNoDataValueValid() const;

    /** \brief Accessor for mGrayMinimumMaximumEstimated */
    bool isGrayMinimumMaximumEstimated() const;

    /** \brief Accessor for mRGBMinimumMaximumEstimated */
    bool isRGBMinimumMaximumEstimated() const;

    /** \brief Accessor that returns the NO_DATA entry for this raster */
    double noDataValue( bool* isValid = 0 );

    /** \brief Returns a pointer to the transparency object */
    QgsRasterTransparency* rasterTransparency();

    /** \brief Accessor for raster shader */
    QgsRasterShader* rasterShader();

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType();

    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red color) */
    QString redBandName();

    /**  [ data provider interface ] Set the data provider */
    void setDataProvider( const QString & provider );

    /** \brief Mutator for drawing style */
    void setDrawingStyle( const DrawingStyle &  theDrawingStyle );

    /** \brief Mutator for mGrayMinimumMaximumEstimated */
    void setGrayMinimumMaximumEstimated( bool theBool );

    /** \brief Mutator to alter the state of the invert histogram flag  */
    void setInvertHistogram( bool theFlag );

    /** \brief Mutator for mRGBMinimumMaximumEstimated */
    void setRGBMinimumMaximumEstimated( bool theBool );

    /** \brief Mutator to alter the number of standard deviations that should be plotted */
    void setStandardDeviations( double theStdDevsToPlot );

    /** \brief Mutator for mUserDefinedGrayMinimumMaximum */
    void setUserDefinedGrayMinimumMaximum( bool theBool );

    /** \brief Mutator for mUserDefinedRGBMinimumMaximum */
    void setUserDefinedRGBMinimumMaximum( bool theBool );

    /**Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* renderer );
    QgsRasterRenderer* renderer();

    /** \brief Accessor to find out how many standard deviations are being plotted */
    double standardDeviations() const;

    /** \brief  Accessor for transparent band name mapping */
    QString transparentBandName() const;

    /**  \brief [ data provider interface ] Does this layer use a provider for setting/retrieving data? 
     * @deprecated in 2.0
     */
    bool usesProvider() /Deprecated/;

    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int width();





    //
    // Non Static methods
    //
    /** \brief Get the number of bands in this layer  */
    unsigned int bandCount();

    /** \brief Get the name of a band given its number  */
    const  QString bandName( int theBandNoInt );

    /** \brief Get the number of a band given its name. The name is the rewritten name set
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    int bandNumber( const QString & theBandName );

    /** \brief Get RasterBandStats for a band given its number (read only)  */
    const  QgsRasterBandStats bandStatistics( int );

    /** \brief Get RasterBandStats for a band given its name (read only)  */
    const  QgsRasterBandStats bandStatistics( const QString & );

    /** \brief Accessor for ths raster layers pyramid list. A pyramid list defines the
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the
     * list.
     */
    RasterPyramidList buildPyramidList();

    /** \brief Accessor for color shader algorithm */
    QString colorShadingAlgorithmAsString() const;

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    // (would need MethodCode directive) void computeMinimumMaximumEstimates( int theBand, double* theMinMax );

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    // (would need MethodCode directive) void computeMinimumMaximumEstimates( QString theBand, double* theMinMax );

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option
      \note added in v1.6 */
    void computeMinimumMaximumEstimates( int theBand, double& theMin /Out/, double& theMax /Out/ );
    
    /** \brief Compute the actual minimum maximum pixel values based on the current (last) display extent */
    // (would need MethodCode directive) void computeMinimumMaximumFromLastExtent( int theBand, double* theMinMax );

    /** \brief Compute the actual minimum maximum pixel values based on the current (last) display extent */
    // (would need MethodCode directive) void computeMinimumMaximumFromLastExtent( QString theBand, double* theMinMax );

    /**  \brief Compute the actual minimum maximum pixel values based on the current (last) display extent 
      \note added in v1.6 */
    void computeMinimumMaximumFromLastExtent( int theBand, double& theMin /Out/, double& theMax /Out/ );

    /** \brief Get a pointer to the contrast enhancement for the selected band */
    QgsContrastEnhancement* contrastEnhancement( unsigned int theBand );

    /**Copies the symbology settings from another layer. Returns true in case of success*/
    bool copySymbologySettings( const QgsMapLayer& theOther );

    /** \brief Get a pointer to the color table */
//QList<QgsColorRampShader::ColorRampItem>* colorTable( int theBandNoInt );

    /** Returns the data provider */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner */
    //const QgsRasterDataProvider* dataProvider() const;

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

    /** \brief Checks if symbology is the same as another layers */
    bool hasCompatibleSymbology( const QgsMapLayer& theOther ) const;

    /** \brief  Check whether a given band number has stats associated with it */
    bool hasStatistics( int theBandNoInt );

    /** \brief Identify raster value(s) found on the point position */
    bool identify( const QgsPoint & point, QMap<QString, QString>& results /Out/ );

    /** \brief Identify arbitrary details from the WMS server found on the point position */
    QString identifyAsText( const QgsPoint & point );

    /** \brief Identify arbitrary details from the WMS server found on the point position
     * @added in 1.5
     */
    QString identifyAsHtml( const QgsPoint & point );

    /** \brief Currently returns always false */
    bool isEditable() const;

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure  */
    QString lastError();

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure */
    QString lastErrorTitle();

    /** \brief Get a legend image for this layer */
    QPixmap legendAsPixmap();

    /** \brief  Overloaded version of above function that can print layer name onto legend */
//QPixmap legendAsPixmap( bool );

    /** \brief Use this method when you want an annotated legend suitable for print output etc */
//QPixmap legendAsPixmap( int theLabelCount );

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
    QPixmap paletteAsPixmap( int theBand = 1 );

    /**  \brief [ data provider interface ] Which provider is being used for this Raster Layer? 
     * @note added in 2.0
     */
    QString providerType() const;

    /**  \brief [ data provider interface ] Which provider is being used for this Raster Layer? 
     * @deprecated in 2.0
     */
    QString providerKey() const /Deprecated/;

    /** \brief Returns the number of raster units per each raster pixel. In a world file, this is normally the first row (without the sign) */
    double rasterUnitsPerPixel();

    /** \brief Read color table from GDAL raster band */
//bool readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList );

    /** \brief Simple reset function that set the noDataValue back to the value stored in the first raster band */
    void resetNoDataValue();

    /** \brief Mutator for blue band name mapping */
    void setBlueBandName( const QString & theBandName );

    /** \brief Mutator for color shader algorithm */
    void setColorShadingAlgorithm( QgsRasterLayer::ColorShadingAlgorithm theShaderAlgorithm );

    /** \brief Mutator for color shader algorithm */
    void setColorShadingAlgorithm( QString theShaderAlgorithm );

    /** \brief Mutator for contrast enhancement algorithm */
    void setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm,
                                          bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for contrast enhancement algorithm */
    void setContrastEnhancementAlgorithm( QString theAlgorithm, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for contrast enhancement function */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction* theFunction );

    /** \brief Overloaded version of the above function for convenience when restoring from xml */
    void setDrawingStyle( const QString & theDrawingStyleQString );

    /** \brief Mutator for gray band name mapping  */
    void setGrayBandName( const QString & theBandName );

    /** \brief Mutator for green band name mapping  */
    void setGreenBandName( const QString & theBandName );

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Sets the minimum and maximum values for the band(s) currently being displayed using the only pixel values from the last/current extent */
    void setMinimumMaximumUsingLastExtent();

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    void setMinimumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    void setMinimumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator that allows the  NO_DATA entry for this raster to be overridden */
    void setNoDataValue( double theNoData );

    /** \brief Set the raster shader function to a user defined function
      \note ownership of the shader function is transfered to raster shader */
    void setRasterShaderFunction( QgsRasterShaderFunction* theFunction /Transfer/ );

    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red color) */
    void setRedBandName( const QString & theBandName );

    /** \brief Mutator for transparent band name mapping  */
    void setTransparentBandName( const QString & theBandName );

    /**  \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int theValue );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    QStringList subLayers() const;

    /** \brief Draws a thumbnail of the rasterlayer into the supplied pixmap pointer */
    void thumbnailAsPixmap( QPixmap * theQPixmap );

    /** \brief Emit a signal asking for a repaint. (inherited from maplayer) */
    void triggerRepaint();





    //
    // Virtural methods
    //
    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    virtual void setLayerOrder( const QStringList & layers );

    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility( const QString & name, bool vis );





  public slots:
    /** \brief Create GDAL pyramid overviews */
    QString buildPyramids( const RasterPyramidList &,
                           const QString &  theResamplingMethod = "NEAREST",
                           bool theTryInternalFlag = false );

    /** \brief Populate the histogram vector for a given band */
    void populateHistogram( int theBandNoInt,
                            int theBinCountInt = 256,
                            bool theIgnoreOutOfRangeFlag = true,
                            bool theThoroughBandScanFlag = false );

    void showStatusMessage( const QString & theMessage );

    /** \brief Propagate progress updates from GDAL up to the parent app */
    void updateProgress( int, int );





  signals:
    /** \brief Signal for notifying listeners of long running processes */
    void progressUpdate( int theValue );




  protected:

    /** \brief Read the symbology for the current layer from the Dom node supplied */
    bool readSymbology( const QDomNode& node, QString& errorMessage );

    /** \brief Reads layer specific state from project file Dom node */
    bool readXml( QDomNode & layer_node );

    /** \brief Write the symbology for the layer into the docment provided */
    bool writeSymbology( QDomNode&, QDomDocument& doc, QString& errorMessage ) const;

    /** \brief Write layer specific state to project file Dom node */
    bool writeXml( QDomNode & layer_node, QDomDocument & doc );

};

