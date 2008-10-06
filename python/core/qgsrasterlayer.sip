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

    //
    // Static methods:
    //
    static void buildSupportedRasterFileFilter(QString & fileFilters);
    static void registerGdalDrivers();
    
    /** This helper checks to see whether the file name appears to be a valid
       raster file name */
    static bool isValidRasterFileName(const QString & theFileNameQString);

    //
    // Non Static methods:
    //
    /** \brief This is the constructor for the RasterLayer class.
     *
     * The main tasks carried out by the constructor are:
     *
     * -Populate the RasterStatsVector with initial values for each band.
     *
     * -Calculate the layer extents
     *
     * -Determine whether the layer is gray, paletted or multiband.
     *
     * -Assign sensible defaults for the red,green, blue and gray bands.
     *
     * -
     * */
    QgsRasterLayer(const QString & path = QString::null, 
                   const QString &  baseName = QString::null);

    /** \brief The destuctor.  */
    ~QgsRasterLayer();

    /** \brief  A list containing one RasterBandStats struct per raster band in this raster layer.
     * Note that while very RasterBandStats element will have the name and number of its associated
     * band populated, any additional stats are calculated on a need to know basis.*/
    typedef QList<QgsRasterBandStats> RasterStatsList;


    /** \brief  A list containing one RasterPyramid struct per raster band in this raster layer.
     * POTENTIAL pyramid layer. How this works is we divide the height
     * and width of the raster by an incrementing number. As soon as the result
     * of the division is <=256 we stop allowing RasterPyramid stracuts
     * to be added to the list. Each time a RasterPyramid is created
     * we will check to see if a pyramid matching these dimensions already exists
     * in the raster layer, and if so mark the exists flag as true. */
      
    typedef QList<QgsRasterPyramid> RasterPyramidList;
    
    /** \brief A list containing on ContrastEnhancement object per raster band in this raster layer. */
    typedef QList<QgsContrastEnhancement> ContrastEnhancementList;

    /** \brief This typedef is used when the showProgress function is passed to gdal as a function
    pointer. */
    //  typedef  int (QgsRasterLayer::*showTextProgress)( double theProgress,
    //                                      const char *theMessageCharArray,
    //                                      void *theData);

    /** \brief Identify raster value(s) found on the point position 
     *
     * \param point[in]  a coordinate in the CRS of this layer.
     */
    bool identify(const QgsPoint & point, QMap<QString,QString>& results /Out/);

    /** \brief Identify arbitrary details from the WMS server found on the point position
     *
     * \param point[in]  an image pixel coordinate in the last requested extent of layer.
     *
     * \return  A text document containing the return from the WMS server
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     */
    QString identifyAsText(const QgsPoint & point);

    /** \brief Query gdal to find out the WKT projection string for this layer. This implements the virtual method of the same name defined in QgsMapLayer*/
    QString getProjectionWKT();

    /** \brief Returns the number of raster units per each raster pixel. For rasters with world file, this is normally the first row (without the sign) in that file */
    double rasterUnitsPerPixel();

    /** \brief Draws a thumbnail of the rasterlayer into the supplied pixmap pointer */
     void drawThumbnail(QPixmap * theQPixmap);

    /** \brief Get an 8x8 pixmap of the colour palette. If the layer has no palette a white pixmap will be returned. */
     QPixmap getPaletteAsPixmap(int theBand=1);
     
    /** \brief This is called when the view on the rasterlayer needs to be refreshed (redrawn).   
     */
    bool draw(QgsRenderContext& rendererContext);

    /** \brief This is an overloaded version of the above function that is called by both draw above and drawThumbnail */
    void draw(QPainter * theQPainter, QgsRasterViewPort * myRasterViewPort,
              QgsMapToPixel * theQgsMapToPixel = 0);
    
    //
    // Accessors for image height and width
    //
    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int getRasterXDim();

    /** \brief Accessor that returns the height of the (unclipped) raster  */
    int getRasterYDim();

    //
    // Accessor and mutator for no data double
    //
    /** \brief Is the NoDataValue Valid */
    bool isNoDataValueValid();
    
    /** \brief  Accessor that returns the NO_DATA entry for this raster. */
    double getNoDataValue(bool* isValid=0);

    /** \brief  Mutator that allows the  NO_DATA entry for this raster to be overridden. */
    void setNoDataValue(double theNoData);

    /** \brief Simple reset function that set the noDataValue back to the value stored in the first raster band */
    void resetNoDataValue();
    //
    // Accessor and mutator for mInvertPixelsFlag
    //
    /** \brief Accessor to find out whether the histogram should be inverted.   */
    bool getInvertHistogramFlag() const;
    
    /** \brief Mutator to alter the state of the invert histogram flag.  */
    void setInvertHistogramFlag(bool theFlag);
    
    //
    // Accessor and mutator for mStandardDeviations
    //
    /** \brief Accessor to find out how many standard deviations are being plotted.  */
    double getStdDevsToPlot() const;
    
    /** \brief Mutator to alter the number of standard deviations that should be plotted.  */
    void setStdDevsToPlot(double the);
    
    /** \brief Get the number of bands in this layer  */
    unsigned int getBandCount();
    /** \brief Get RasterBandStats for a band given its number (read only)  */
    const  QgsRasterBandStats getRasterBandStats(int);
    /** \brief  Check whether a given band number has stats associated with it */
    bool hasStats(int theBandNoInt);
    /** \brief Overloaded method that also returns stats for a band, but uses the band colour name
    *    Note this approach is not recommeneded because it is possible for two gdal raster
    *    bands to have the same name!
    */
    const  QgsRasterBandStats getRasterBandStats(const QString &);
    /** \brief Get the number of a band given its name. Note this will be the rewritten name set 
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    int getRasterBandNumber (const QString & theBandNameQString);
    /** \brief Get the name of a band given its number.  */
    const  QString getRasterBandName(int theBandNoInt);
    /** \brief Find out whether a given band exists.    */
    bool hasBand(const QString &  theBandName);
    /** \brief Call any inline image manipulation filters */
    void filterLayer(QImage * theQImage);
    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red colour). */
    QString getRedBandName() const;
    
    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red colour). */
    void setRedBandName(const QString & theBandNameQString);
    // 
    // Accessor and mutator for green band name
    // 
    /** \brief Accessor for green band name mapping.  */
    QString getGreenBandName() const;
    
    /** \brief Mutator for green band name mapping.  */
    void setGreenBandName(const QString & theBandNameQString);
    //
    // Accessor and mutator for blue band name
    // 
    /** \brief  Accessor for blue band name mapping. */
    QString getBlueBandName() const;
    
    /** \brief Mutator for blue band name mapping.  */
    void setBlueBandName(const QString & theBandNameQString);
    
    //
    // Accessor transparency container.
    //
    QgsRasterTransparency* getRasterTransparency();
    
    //
    // Accessor and mutator for transparent band name
    // 
    /** \brief  Accessor for transparent band name mapping. */
    QString getTransparentBandName() const;
    
    /** \brief Mutator for transparent band name mapping.  */
    void setTransparentBandName(const QString & theBandNameQString);
    
    //
    // Accessor and mutator for gray band name
    //
    /** \brief Accessor for gray band name mapping.  */
    QString getGrayBandName() const;
    
    /** \brief Mutator for gray band name mapping.  */
    void setGrayBandName(const QString & theBandNameQString);
    
    // Accessor and mutator for min max values 
    double getMinimumValue(unsigned int theBand);

    double getMinimumValue(QString theBand);

    void setMinimumValue(unsigned int theBand, double theValue);

    void setMinimumValue(QString theBand, double theValue, bool theGenerateLookupTableFlag=true);
    
    double getMaximumValue(unsigned int theBand);

    double getMaximumValue(QString theBand);
    
    void setMaximumValue(unsigned int theBand, double theValue);

    void setMaximumValue(QString theBand, double theValue, bool theGenerateLookupTableFlag=true);

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    void computeMinimumMaximumEstimates(int theBand, double* theMinMax);

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    void computeMinimumMaximumEstimates(QString theBand, double* theMinMax);

    QgsContrastEnhancement* getContrastEnhancement(unsigned int theBand);

    //
    // Accessor and mutator for the contrast enhancement algorithm
    //
    /** \brief Accessor for contrast enhancement algorithm. */
    QgsContrastEnhancement::CONTRAST_ENHANCEMENT_ALGORITHM getContrastEnhancementAlgorithm();
    
    /** \brief Accessor for contrast enhancement algorithm. */
    QString getContrastEnhancementAlgorithmAsString() const;
    
    /** \brief Mutator for contrast enhancement algorithm. */
    void setContrastEnhancementAlgorithm(QgsContrastEnhancement::CONTRAST_ENHANCEMENT_ALGORITHM theAlgorithm, bool theGenerateLookupTableFlag=true);
    
    /** \brief Mutator for contrast enhancement algorithm. */
    void setContrastEnhancementAlgorithm(QString theAlgorithm, bool theGenerateLookupTableFlag=true);
    
    //
    // Mutator for the contrast enhancement function
    //
    /** \brief Mutator for contrast enhancement function. */
    void setContrastEnhancementFunction(QgsContrastEnhancementFunction* theFunction);
    
    /** \brief This enumerator describes the types of shading that can be used.  */
    enum COLOR_SHADING_ALGORITHM
    {
        UNDEFINED_SHADING_ALGORITHM,
        PSEUDO_COLOR, 
        FREAK_OUT, //it will scare your granny!
        COLOR_RAMP,
        USER_DEFINED
    };
    //
    // Accessor and mutator for the color shader algorithm
    //
    /** \brief Accessor for colour shader algorithm. */
    COLOR_SHADING_ALGORITHM getColorShadingAlgorithm() const;

    /** \brief Accessor for colour shader algorithm. */
    QString getColorShadingAlgorithmAsString() const;
    
    /** \brief Mutator for color shader algorithm. */
    void setColorShadingAlgorithm(COLOR_SHADING_ALGORITHM theShaderAlgorithm);
    
    /** \brief Mutator for color shader algorithm. */
    void setColorShadingAlgorithm(QString theShaderAlgorithm);
    
    /** \brief Accessor for raster shader */
    QgsRasterShader* getRasterShader();
    
    /** \brief Set the raster shader function to a user defined function */
    void setRasterShaderFunction(QgsRasterShaderFunction* theFunction);
    
    /** \brief This enumerator describes the different kinds of drawing we can do.  */
    enum DRAWING_STYLE
    {
        UNDEFINED_DRAWING_STYLE,
        SINGLE_BAND_GRAY, // a "Gray" or "Undefined" layer drawn as a range of gray colors
        SINGLE_BAND_PSEUDO_COLOR,// a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
        PALETTED_COLOR, //a "Palette" image drawn using color table
        PALETTED_SINGLE_BAND_GRAY,// a "Palette" layer drawn in gray scale (using only one of the color components)
        PALETTED_SINGLE_BAND_PSEUDO_COLOR, // a "Palette" layer having only one of its color components rendered as psuedo color
        PALETTED_MULTI_BAND_COLOR, // a "Palette" image is decomposed to 3 channels (RGB) and drawn 
                             // as multiband 
        MULTI_BAND_SINGLE_BAND_GRAY, // a layer containing 2 or more bands, but using only one band to produce a grayscale image
        MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR, //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
        MULTI_BAND_COLOR //a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
    };
    DRAWING_STYLE drawingStyle;    
    //
    // Accessor and mutator for drawing style.
    //
    /** \brief Accessor for drawing style.  */
    DRAWING_STYLE getDrawingStyle();
    /** \brief Returns a string representation of drawing style.
     *
     * Implementaed mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future!. Use alternate implementation above rather.
     * */
    QString getDrawingStyleAsString() const;
    
    /** \brief Mutator for drawing style.  */
    void setDrawingStyle(const DRAWING_STYLE &  theDrawingStyle);
    
    /** \brief Overloaded version of the above function for convenience when restoring from xml.
     *
     * Implementaed mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future! Use alternate implementation above rather.
     * */
    void setDrawingStyle(const QString & theDrawingStyleQString);

    /** \brief This enumerator describes the type of raster layer.  */
    enum RASTER_LAYER_TYPE
    {
      GRAY_OR_UNDEFINED,
      PALETTE,
      MULTIBAND    
    };
    RASTER_LAYER_TYPE rasterLayerType;
    //
    //accessor and for raster layer type (READ ONLY)
    //
    /** \brief  Accessor for raster layer type (which is a read only property) */
    RASTER_LAYER_TYPE getRasterLayerType();
    /** \brief Accessor for hasPyramidsFlag (READ ONLY) */
    bool getHasPyramidsFlag();
     
    /** \brief Get a legend image for this layer.  */
    QPixmap getLegendQPixmap();
    /** \brief  Overloaded version of above function that can print layer name onto legend. */
    QPixmap getLegendQPixmap(bool); 
    
    /** \brief Use this method when you want an annotated legend suitable for print output etc. 
     * @param int theLabelCountInt Number of vertical labels to display (defaults to 3)
     * */
    QPixmap getDetailedLegendQPixmap(int theLabelCount);
    
    /**
     * Returns the sublayers of this layer
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    QStringList subLayers() const;
    
    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     *
     * (Useful for providers that manage their own layers, such as WMS)
     *
     */
    virtual void setLayerOrder(const QStringList & layers);
    
    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility(const QString & name, bool vis);

    /** \brief Emit a signal asking for a repaint. (inherited from maplayer) */
    void triggerRepaint();
    /** \brief Obtain GDAL Metadata for this layer */
    QString getMetadata(); 
    /** \brief Accessor for ths raster layers pyramid list. A pyramid list defines the 
     * POTENTIAL pyramids that can be in a raster. To know which of the pyramid layers 
     * ACTUALLY exists you need to look at the existsFlag member in each struct stored in the 
     * list.*/
    RasterPyramidList buildRasterPyramidList();
    /** \brief Helper method to retrieve the nth pyramid layer struct from the PyramidList. 
     * If the nth layer does not exist, NULL will be returned. */
//   RasterPyramid getRasterPyramid(int thePyramidNo);

    /**Currently returns always false*/
    bool isEditable() const;
    
    /** Return time stamp for given file name */
    static QDateTime lastModified ( const QString &  name );

    /**Copies the symbology settings from another layer. Returns true in case of success*/
    bool copySymbologySettings(const QgsMapLayer& other);

    bool isSymbologyCompatible(const QgsMapLayer& other) const;

    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorCaptionString();
  
    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    QString errorString();

    /** Returns the data provider
     *
     *  \retval 0 if not using the data provider model (i.e. directly using GDAL)
     */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
     *
     *  \retval 0 if not using the data provider model (i.e. directly using GDAL)
     */
    // PyQGIS: commented out, has the same signature as the function above
    // const QgsRasterDataProvider* dataProvider() const;

     /** \brief Mutator for mUserDefinedRGBMinMaxFlag */
    void setUserDefinedRGBMinMax(bool theBool);

    /** \brief Accessor for mUserDefinedRGBMinMaxFlag.  */
    bool getUserDefinedRGBMinMax() const;

    /** \brief Mutator for mUserDefinedGrayMinMaxFlag */
    void setUserDefinedGrayMinMax(bool theBool);

    /** \brief Accessor for mUserDefinedGrayMinMaxFlag.  */
    bool getUserDefinedGrayMinMax() const;

public slots:    
    /**
     * Convert this raster to another format
     */
    //void const convertTo();
    /**
     * Mainly inteded for use in propogating progress updates from gdal up to the parent app.
     **/
    void updateProgress(int,int);

    /** \brief Create  gdal pyramid overviews  for this layer.
    * This will speed up performance at the expense of hard drive space.
    * Also, write access to the file is required for creating internal pyramids,
    * and to the directory in which the files exists if external 
    * pyramids (.ovr) are to be created. If no paramter is passed in
    * it will default to nearest neighbor resampling.
    * @param theTryInternalFlag - Try to make the pyramids internal to 
    * the raster file if supported (e.g. geotiff). If not supported it 
    * will revert to creating external .ovr file anyway.
    * \return null string on success, otherwise a string specifying error
    */
    QString buildPyramids(const RasterPyramidList &, 
                          const QString &  theResamplingMethod="NEAREST",
                          bool theTryInternalFlag=false);
    /** \brief Used at the moment by the above function but hopefully will later
    be useable by any operation that needs to notify the user of its progress. */
/*
    int showTextProgress( double theProgress,
                          const char *theMessageCharArray,
                          void *theData);    
*/

  /** Populate the histogram vector for a given layer
  * @param theBandNoInt - which band to compute the histogram for
  * @param theBinCountInt - how many 'bins' to categorise the data into
  * @param theIgnoreOutOfRangeFlag - whether to ignore values that are out of range (default=true)
  * @param theThoroughBandScanFlag - whether to visit each cell when computing the histogram (default=false)
  */
  void populateHistogram(int theBandNoInt, 
                         int theBinCountInt=256,
                         bool theIgnoreOutOfRangeFlag=true,
                         bool theThoroughBandScanFlag=false);

    /** \brief Color table 
     *  \param band number
     *  \return pointer to color table
     */
    //QList<QgsColorRampShader::ColorRampItem>* getColorTable ( int theBandNoInt );
 protected:

    /** reads vector layer specific state from project file Dom node.

        @note

        Called by QgsMapLayer::readXML().

    */
    /* virtual */ bool readXml( QDomNode & layer_node );



  /** write vector layer specific state to project file Dom node.

      @note

      Called by QgsMapLayer::writeXML().

  */
  /* virtual */ bool writeXml( QDomNode & layer_node, QDomDocument & doc );
    

/*
 * 
 * New functions that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 
 
public:
 
  //! Constructor in provider mode
  // TODO Rename into a general constructor when the old raster interface is retired
  // \param  dummy  is just there to distinguish this function signature from the old non-provider one.
  QgsRasterLayer(int dummy, 
                 const QString & baseName = QString(),
                 const QString & path = QString(),
                 const QString & providerLib = QString(),
                 const QStringList & layers = QStringList(),
                 const QStringList & styles = QStringList(),
                 const QString & format = QString(),
                 const QString & crs = QString());

  void setDataProvider( const QString & provider,
                        const QStringList & layers,
                        const QStringList & styles,
                        const QString & format,
                        const QString & crs);

  //! Does this layer use a provider for setting/retrieving data?
  bool usesProvider();

  //! Which provider is being used for this Raster Layer?
  QString providerKey();

public slots:

  void showStatusMessage(const QString & theMessage);

};

