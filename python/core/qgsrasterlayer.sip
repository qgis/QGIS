
  
  
/*! \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class QgsRasterLayer : QgsMapLayer
{
%TypeHeaderCode
#include <qgsrasterpyramid.h>
#include <qgsrasterlayer.h>
%End

public:
    //
    // Static methods:
    //
        
    static void buildSupportedRasterFileFilter(QString & fileFilters);

    static bool isSupportedRasterDriver(const QString & driverName);

    /** This helper checks to see whether the filename appears to be a valid
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

    /** \brief  A vector containing one RasterBandStats struct per raster band in this raster layer.
     * Note that while very RasterBandStats element will have the name and number of its associated
     * band populated, any additional stats are calculated on a need to know basis.*/
    typedef QVector<QgsRasterBandStats> RasterStatsVector;


    /** \brief  A list containing one RasterPyramid struct per 
     * POTENTIAL pyramid layer. How this works is we divide the height
     * and width of the raster by an incrementing number. As soon as the result
     * of the division is <=256 we stop allowing RasterPyramid stracuts
     * to be added to the list. Each time a RasterPyramid is created
     * we will check to see if a pyramid matching these dimensions already exists
     * in the raster layer, and if so mark the exists flag as true. */
      
    typedef QList<QgsRasterPyramid> RasterPyramidList;

    /** \brief This typedef is used when the showProgress function is passed to gdal as a function
    pointer. */
    //  typedef  int (QgsRasterLayer::*showTextProgress)( double theProgressDouble,
    //                                      const char *theMessageCharArray,
    //                                      void *theData);

    /** \brief Identify raster value(s) found on the point position 
     *
     * \param point[in]  a coordinate in the CRS of this layer.
     */
    // TODO: wrap
    //void identify(const QgsPoint& point, std::map<QString,QString>& results);

    /** \brief Identify arbitrary details from the WMS server found on the point position
     *
     * \param point[in]  an image pixel coordinate in the last requested extent of layer.
     *
     * \return  A text document containing the return from the WMS server
     *
     * \note  The arbitraryness of the returned document is enforced by WMS standards
     *        up to at least v1.3.0
     */
    QString identifyAsText(const QgsPoint& point);

    /** \brief Query gdal to find out the WKT projection string for this layer. This implements the virtual method of the same name defined in QgsMapLayer*/
    QString getProjectionWKT();

    /** \brief Draws a thumbnail of the rasterlayer into the supplied pixmap pointer */
     void drawThumbnail(QPixmap * theQPixmap);

    /** \brief Get an 8x8 pixmap of the colour palette. If the layer has no palette a white pixmap will be returned. */
     QPixmap getPaletteAsPixmap();
     
    /** \brief This is called when the view on the rasterlayer needs to be refreshed (redrawn).
         
        \param drawingToEditingCanvas  Are we drawing to an editable canvas? 
                                       currently not used, but retain to be similar to 
                                       the QgsVectorLayer interface 
     */
    bool draw(QPainter * theQPainter,
              QgsRect & theViewExtent, 
              QgsMapToPixel * theQgsMapToPixel,
              QgsCoordinateTransform* ct,
              bool drawingToEditingCanvas);

    /** \brief This is an overloaded version of the above function that is called by both draw above and drawThumbnail */
    void draw(QPainter * theQPainter, QgsRasterViewPort * myRasterViewPort,
              QgsMapToPixel * theQgsMapToPixel = 0);
    
    //
    // Accessors for image height and width
    //
    /** \brief Accessor that returns the width of the (unclipped) raster  */
    const int getRasterXDim();

    /** \brief Accessor that returns the height of the (unclipped) raster  */
    const int getRasterYDim();

    //
    // Accessor and mutator for no data double
    //
    /** \brief  Accessor that returns the NO_DATA entry for this raster. */
    const double getNoDataValue();

    /** \brief  Mutator that allows the  NO_DATA entry for this raster to be overridden. */
    void setNoDataValue(double theNoDataDouble);

    //
    // Accessor and mutator for invertHistogramFlag
    //
    /** \brief Accessor to find out whether the histogram should be inverted.   */
    bool getInvertHistogramFlag();
    /** \brief Mutator to alter the state of the invert histogram flag.  */
    void setInvertHistogramFlag(bool theFlag);
    //
    // Accessor and mutator for stdDevsToPlotDouble
    //
    /** \brief Accessor to find out how many standard deviations are being plotted.  */
    double getStdDevsToPlot();
    /** \brief Mutator to alter the number of standard deviations that should be plotted.  */
    void setStdDevsToPlot(double theDouble);
    /** \brief Get the number of bands in this layer  */
    const unsigned int getBandCount();
    /** \brief Get RasterBandStats for a band given its number (read only)  */
    const  QgsRasterBandStats getRasterBandStats(int);
    /** \brief  Check whether a given band number has stats associated with it */
    const bool hasStats(int theBandNoInt);
    /** \brief Overloaded method that also returns stats for a band, but uses the band colour name
    *    Note this approach is not recommeneded because it is possible for two gdal raster
    *    bands to have the same name!
    */
    const  QgsRasterBandStats getRasterBandStats(const QString &);
    /** \brief Get the number of a band given its name. Note this will be the rewritten name set 
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    const  int getRasterBandNumber (const QString & theBandNameQString);
    /** \brief Get the name of a band given its number.  */
    const  QString getRasterBandName(int theBandNoInt);
    /** \brief Find out whether a given band exists.    */
    bool hasBand(const QString &  theBandName);
    /** \brief Call any inline image manipulation filters */
    void filterLayer(QImage * theQImage);
    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red colour). */
    QString getRedBandName();
    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red colour). */
    void setRedBandName(const QString & theBandNameQString);
    // 
    // Accessor and mutator for green band name
    // 
    /** \brief Accessor for green band name mapping.  */
    QString getGreenBandName();
    /** \brief Mutator for green band name mapping.  */
    void setGreenBandName(const QString & theBandNameQString);
    //
    // Accessor and mutator for blue band name
    // 
    /** \brief  Accessor for blue band name mapping. */
    QString getBlueBandName();
    /** \brief Mutator for blue band name mapping.  */
    void setBlueBandName(const QString & theBandNameQString);
    //
    // Accessor and mutator for gray band name
    //
    /** \brief Accessor for gray band name mapping.  */
    QString getGrayBandName();
    /** \brief Mutator for gray band name mapping.  */
    void setGrayBandName(const QString & theBandNameQString);
    // 
    // Accessor and mutator for showDebugOverlayFlag
    // 
    /** \brief Accessor for a flag that determines whether to show some debug info on the image.  */
    bool getShowDebugOverlayFlag();
    /** \brief Mutator for a flag that determines whether to show some debug info on the image.  */
    void setShowDebugOverlayFlag(bool theFlag);
    // 
    // Accessor and mutator for min and max red
    // 
    /** \brief Accessor for minimum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinRedDouble();
    /** \brief Mutator for minimum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinRedDouble(double theDouble);
    /** \brief Accessor for maximum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxRedDouble();
    /** \brief Mutator for maximum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxRedDouble(double theDouble);
    // 
    // Accessor and mutator for min and max green
    // 
    /** \brief Accessor for minimum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinGreenDouble();
    /** \brief Mutator for minimum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinGreenDouble(double theDouble);
    /** \brief Accessor for maximum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxGreenDouble();
    /** \brief Mutator for maximum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxGreenDouble(double theDouble);
    // 
    // Accessor and mutator for min and max blue
    // 
    /** \brief Accessor for minimum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    /** \brief   */
    double getMinBlueDouble();
    /** \brief Mutator for minimum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinBlueDouble(double theDouble);
    /** \brief Accessor for maximum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxBlueDouble();
    /** \brief Mutator for maximum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxBlueDouble(double theDouble);
    // 
    // Accessor and mutator for min and max gray
    // 
    /** \brief Accessor for minimum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinGrayDouble();
    /** \brief Mutator for minimum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinGrayDouble(double theDouble);
    /** \brief Accessor for maximum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxGrayDouble();
    /** \brief Mutator for maximum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxGrayDouble(double theDouble);
    //
    /** \brief This enumerator describes the types of histogram scaling algorithms that can be used.  */
    enum COLOR_SCALING_ALGORITHM
    {
        STRETCH_TO_MINMAX, //linear histogram stretch
        STRETCH_AND_CLIP_TO_MINMAX,
        CLIP_TO_MINMAX
    };
    
    COLOR_SCALING_ALGORITHM colorScalingAlgorithm;
    //
    // Accessor and mutator for the color scaling algorithm
    //
    /** \brief Accessor for colour scaling algorithm. */
    COLOR_SCALING_ALGORITHM getColorScalingAlgorithm();
    /** \brief Mutator for color scaling algorithm. */
    void setColorScalingAlgorithm(COLOR_SCALING_ALGORITHM theAlgorithm);
    
    /** \brief This enumerator describes the types of histogram colour ramping that can be used.  */
    enum COLOR_RAMPING_TYPE
    {
        BLUE_GREEN_RED, 
        FREAK_OUT //it will scare your granny!
    };
    
    COLOR_RAMPING_TYPE colorRampingType;
    //
    // Accessor and mutator for the color ramping type
    //
    /** \brief Accessor for colour ramping type. */
    COLOR_RAMPING_TYPE getColorRampingType();
    /** \brief Mutator for color scaling algorithm. */
    void setColorRampingType(COLOR_RAMPING_TYPE theRamping);
    
    /** \brief This enumerator describes the different kinds of drawing we can do.  */
    enum DRAWING_STYLE
    {
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
    QString getDrawingStyleAsQString();
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
    QStringList subLayers();
    
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
    QgsRasterDataProvider* getDataProvider();

    /** Returns the data provider in a const-correct manner
     *
     *  \retval 0 if not using the data provider model (i.e. directly using GDAL)
     */
  // PyQGIS: commented out, has the same signature as the function above
  // const QgsRasterDataProvider* getDataProvider() const;

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
    * Also, write access to the file is required. If no paramter is passed in
    * it will default to nearest neighbor resampling.
    * \return null string on success, otherwise a string specifying error
    */
    QString buildPyramids(const RasterPyramidList &, 
                          const QString &  theResamplingMethod="NEAREST");
    /** \brief Used at the moment by the above function but hopefully will later
    be useable by any operation that needs to notify the user of its progress. */
/*
    int showTextProgress( double theProgressDouble,
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
    QgsColorTable *colorTable ( int theBandNoInt );
 protected:

    /** reads vector layer specific state from project file DOM node.

        @note

        Called by QgsMapLayer::readXML().

    */
    /* virtual */ bool readXML_( QDomNode & layer_node );



  /** write vector layer specific state to project file DOM node.

      @note

      Called by QgsMapLayer::writeXML().

  */
  /* virtual */ bool writeXML_( QDomNode & layer_node, QDomDocument & doc );
    

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
                 const QString & crs = QString(),
                 const QString & proxyHost = QString(),
                 int proxyPort = 80,
                 const QString & proxyUser = QString(),
                 const QString & proxyPass = QString());

  void setDataProvider( const QString & provider,
                        const QStringList & layers,
                        const QStringList & styles,
                        const QString & format,
                        const QString & crs,
                        const QString & proxyHost,
                        int proxyPort,
                        const QString & proxyUser,
                        const QString & proxyPass );


  //! Does this layer use a provider for setting/retrieving data?
  bool usesProvider();

  /**
   * Sets a proxy for the path given in the constructor
   *
   * \retval TRUE if proxy setting is successful (if indeed it is supported)
   */
  bool setProxy(const QString & host = 0,
                            int port = 80,
                const QString & user = 0,
                const QString & pass = 0);

  //! Which provider is being used for this Raster Layer?
  QString providerKey();

public slots:

  void showStatusMessage(const QString & theMessage);


};

