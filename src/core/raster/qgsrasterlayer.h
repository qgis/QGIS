/***************************************************************************
                        qgsrasterlayer.h  -  description
                              -------------------
 begin                : Fri Jun 28 2002
 copyright            : (C) 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
 email                : tim@linfiniti.com
***************************************************************************/
/*
 * Peter J. Ersts - contributed to the refactoring and maintenance of this class
 * B. Morley - added functions to convert this class to a data provider interface
 * Frank Warmerdam - contributed bug fixes and migrated class to use all GDAL_C_API calls
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

//
// Includes
//

#include <QColor>
#include <QDateTime>
#include <QVector>
#include <QList>
#include <QMap>
#include <QPair>

#include "qgis.h"
#include "qgspoint.h"
#include "qgsmaplayer.h"
#include "qgsrasterviewport.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgsrastershader.h"
#include "qgscolorrampshader.h"
#include "qgsrastershaderfunction.h"
#include "qgsrasterdataprovider.h"

//
// Forward declarations
//
class QgsMapToPixel;
class QgsRectangle;
class QgsRasterBandStats;
class QgsRasterPyramid;
class QgsRasterRenderer;
class QImage;
class QPixmap;
class QSlider;
class QLibrary;

/** \ingroup core
 *  This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas.
 *
 *  The qgsrasterlayer class makes use of gdal for data io, and thus supports
 *  any gdal supported format. The constructor attempts to infer what type of
 *  file (LayerType) is being opened - not in terms of the file format (tif, ascii grid etc.)
 *  but rather in terms of whether the image is a GRAYSCALE, PaletteD or Multiband,
 *
 *  Within the three allowable raster layer types, there are 8 permutations of
 *  how a layer can actually be rendered. These are defined in the DrawingStyle enum
 *  and consist of:
 *
 *  SingleBandGray -> a GRAYSCALE layer drawn as a range of gray colors (0-255)
 *  SingleBandPseudoColor -> a GRAYSCALE layer drawn using a pseudocolor algorithm
 *  PalettedSingleBandGray -> a PaletteD layer drawn in gray scale (using only one of the color components)
 *  PalettedSingleBandPseudoColor -> a PaletteD layer having only one of its color components rendered as psuedo color
 *  PalettedMultiBandColor -> a PaletteD image where the bands contains 24bit color info and 8 bits is pulled out per color
 *  MultiBandSingleBandGray -> a layer containing 2 or more bands, but using only one band to produce a grayscale image
 *  MultiBandSingleBandPseudoColor -> a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
 *  MultiBandColor -> a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
 *
 *  Each of the above mentioned drawing styles is implemented in its own draw* function.
 *  Some of the drawing styles listed above require statistics about the layer such
 *  as the min / max / mean / stddev etc. statistics for a band can be gathered using the
 *  bandStatistics function. Note that statistics gathering is a slow process and
 *  every effort should be made to call this function as few times as possible. For this
 *  reason, qgsraster has a vector class member to store stats for each band. The
 *  constructor initialises this vector on startup, but only populates the band name and
 *  number fields.
 *
 *  Note that where bands are of gdal 'undefined' type, their values may exceed the
 *  renderable range of 0-255. Because of this a linear scaling histogram enhanceContrast is
 *  applied to undefined layers to normalise the data into the 0-255 range.
 *
 *  A qgsrasterlayer band can be referred to either by name or by number (base=1). It
 *  should be noted that band names as stored in datafiles may not be unique, and
 *  so the rasterlayer class appends the band number in brackets behind each band name.
 *
 *  Sample usage of the QgsRasterLayer class:
 *
 * \code
 *     QString myFileNameQString = "/path/to/file";
 *     QFileInfo myFileInfo(myFileNameQString);
 *     QString myBaseNameQString = myFileInfo.baseName();
 *     QgsRasterLayer *myRasterLayer = new QgsRasterLayer(myFileNameQString, myBaseNameQString);
 *
 * \endcode
 *
 *  In order to automate redrawing of a raster layer, you should like it to a map canvas like this :
 *
 * \code
 *     QObject::connect( myRasterLayer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()) );
 * \endcode
 *
 *  A raster layer can also export its legend as a pixmap:
 *
 * \code
 *     QPixmap myQPixmap = myRasterLayer->legendPixmap();
 * \endcode
 *
 * Once a layer has been created you can find out what type of layer it is (GrayOrUndefined, Palette or Multiband):
 *
 * \code
 *    if (rasterLayer->rasterType()==QgsRasterLayer::Multiband)
 *    {
 *      //do something
 *    }
 *    else if (rasterLayer->rasterType()==QgsRasterLayer::Palette)
 *    {
 *      //do something
 *    }
 *    else // QgsRasterLayer::GrayOrUndefined
 *    {
 *      //do something.
 *    }
 * \endcode
 *
 * You can combine layer type detection with the setDrawingStyle method to override the default drawing style assigned
 * when a layer is loaded:
 *
  * \code
 *    if (rasterLayer->rasterType()==QgsRasterLayer::Multiband)
 *    {
 *       myRasterLayer->setDrawingStyle(QgsRasterLayer::MultiBandSingleBandPseudoColor);
 *    }
 *    else if (rasterLayer->rasterType()==QgsRasterLayer::Palette)
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::PalettedSingleBandPseudoColor);
 *    }
 *    else // QgsRasterLayer::GrayOrUndefined
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::SingleBandPseudoColor);
 *    }
 * \endcode
 *
 *  Raster layers can also have an arbitrary level of transparency defined, and have their
 *  color palettes inverted using the setTransparency and setInvertHistogram methods.
 *
 *  Pseudocolor images can have their output adjusted to a given number of standard
 *  deviations using the setStandardDeviations method.
 *
 *  The final area of functionality you may be interested in is band mapping. Band mapping
 *  allows you to choose arbitrary band -> color mappings and is applicable only to Palette
 *  and Multiband rasters, There are four mappings that can be made: red, green, blue and gray.
 *  Mappings are non-exclusive. That is a given band can be assigned to no, some or all
 *  color mappings. The constructor sets sensible defaults for band mappings but these can be
 *  overridden at run time using the setRedBandName, setGreenBandName, setBlueBandName and setGrayBandName
 *  methods.
 */

class CORE_EXPORT QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
  public:
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
      PalettedColor,                  //a "Palette" image drawn using color table
      PalettedSingleBandGray,        // a "Palette" layer drawn in gray scale
      PalettedSingleBandPseudoColor, // a "Palette" layerdrawn using a pseudocolor algorithm
      PalettedMultiBandColor,         // currently not supported
      MultiBandSingleBandGray, // a layer containing 2 or more bands, but a single band drawn as a range of gray colors
      MultiBandSingleBandPseudoColor, //a layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
      MultiBandColor,                  //a layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color.
      SingleBandColorDataStyle         // ARGB values rendered directly
    };

    /** \brief This enumerator describes the type of raster layer */
    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband,
      ColorLayer
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

    static QLibrary* loadProviderLibrary( QString theProviderKey );
    static QgsRasterDataProvider* loadProvider( QString theProviderKey, QString theDataSource = 0 );


    /** \brief  Accessor for blue band name mapping */
    QString blueBandName() const { return mBlueBandName; }

    /** \brief Accessor for color shader algorithm */
    QgsRasterLayer::ColorShadingAlgorithm colorShadingAlgorithm() const { return mColorShadingAlgorithm; }

    /** \brief Accessor for contrast enhancement algorithm */
    QgsContrastEnhancement::ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() { return mContrastEnhancementAlgorithm; }

    /** \brief Returns contrast enhancement algorithm as a string */
    QString contrastEnhancementAlgorithmAsString() const;

    /** \brief Accessor for drawing style */
    DrawingStyle drawingStyle() { return mDrawingStyle; }

    /** \brief Accessor for mHasPyramids (READ ONLY) */
    bool hasPyramids() { return mHasPyramids; }

    /** \brief Accessor for mUserDefinedGrayMinimumMaximum */
    bool hasUserDefinedGrayMinimumMaximum() const { return mUserDefinedGrayMinimumMaximum; }

    /** \brief Accessor for mUserDefinedRGBMinimumMaximum */
    bool hasUserDefinedRGBMinimumMaximum() const { return mUserDefinedRGBMinimumMaximum; }

    /** \brief Accessor that returns the height of the (unclipped) raster */
    int height() { return mHeight; }

    /** \brief Is the NoDataValue Valid */
    bool isNoDataValueValid() const { return mValidNoDataValue; }

    /** \brief Accessor that returns the NO_DATA entry for this raster */
    double noDataValue( bool* isValid = 0 ) { if ( isValid ) { *isValid = mValidNoDataValue;} return mNoDataValue; }

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType() { return mRasterType; }


    /** \brief Mutator for drawing style */
    void setDrawingStyle( const DrawingStyle &  theDrawingStyle ) { mDrawingStyle = theDrawingStyle; setRendererForDrawingStyle( theDrawingStyle ); }
    /**Sets corresponding renderer for style*/
    void setRendererForDrawingStyle( const DrawingStyle &  theDrawingStyle );

    /** \brief Mutator to alter the number of standard deviations that should be plotted */
    void setStandardDeviations( double theStandardDeviations ) { mStandardDeviations = theStandardDeviations; }

    /** \brief Mutator for mUserDefinedGrayMinimumMaximum */
    void setUserDefinedGrayMinimumMaximum( bool theBool ) { mUserDefinedGrayMinimumMaximum = theBool; }

    /** \brief Mutator for mUserDefinedRGBMinimumMaximum */
    void setUserDefinedRGBMinimumMaximum( bool theBool ) { mUserDefinedRGBMinimumMaximum = theBool; }

    /**Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* renderer );
    const QgsRasterRenderer* renderer() const { return mRenderer; }
    QgsRasterRenderer* renderer() { return mRenderer; }

    /** \brief Accessor to find out how many standard deviations are being plotted */
    double standardDeviations() const { return mStandardDeviations; }

    /**  \brief [ data provider interface ] Does this layer use a provider for setting/retrieving data?
     * @deprecated in 2.0
     */
    Q_DECL_DEPRECATED bool usesProvider();

    /** \brief Accessor that returns the width of the (unclipped) raster  */
    int width() { return mWidth; }

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
    void computeMinimumMaximumEstimates( int theBand, double* theMinMax );

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option */
    void computeMinimumMaximumEstimates( QString theBand, double* theMinMax );

    /** \brief Wrapper for GDALComputeRasterMinMax with the estimate option
      \note added in v1.6 */
    void computeMinimumMaximumEstimates( int theBand, double& theMin, double& theMax );

    /** \brief Compute the actual minimum maximum pixel values based on the current (last) display extent */
    void computeMinimumMaximumFromLastExtent( int theBand, double* theMinMax );

    /** \brief Compute the actual minimum maximum pixel values based on the current (last) display extent */
    void computeMinimumMaximumFromLastExtent( QString theBand, double* theMinMax );

    /**  \brief Compute the actual minimum maximum pixel values based on the current (last) display extent
      \note added in v1.6 */
    void computeMinimumMaximumFromLastExtent( int theBand, double& theMin, double& theMax );

    /** \brief Get a pointer to the contrast enhancement for the selected band */
    QgsContrastEnhancement* contrastEnhancement( unsigned int theBand );

    const QgsContrastEnhancement* constContrastEnhancement( unsigned int theBand ) const;

    /**Copies the symbology settings from another layer. Returns true in case of success*/
    bool copySymbologySettings( const QgsMapLayer& theOther );

    /** \brief Get a pointer to the color table */
    QList<QgsColorRampShader::ColorRampItem>* colorTable( int theBandNoInt );

    /** Returns the data provider */
    QgsRasterDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner */
    const QgsRasterDataProvider* dataProvider() const;

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
    bool identify( const QgsPoint & point, QMap<QString, QString>& results );

    /** \brief Identify raster value(s) found on the point position */
    bool identify( const QgsPoint & point, QMap<int, QString>& results );

    /** \brief Identify arbitrary details from the WMS server found on the point position */
    QString identifyAsText( const QgsPoint & point );

    /** \brief Identify arbitrary details from the WMS server found on the point position
     * @note added in 1.5
     */
    QString identifyAsHtml( const QgsPoint & point );

    /** \brief Currently returns always false */
    bool isEditable() const;

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure  */
    QString lastError();

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure */
    QString lastErrorTitle();

    /**Returns a list with classification items (Text and color)
      @note this method was added in version 1.8*/
    QList< QPair< QString, QColor > > legendSymbologyItems() const;

    /** \brief Get a legend image for this layer */
    QPixmap legendAsPixmap();

    /** \brief  Overloaded version of above function that can print layer name onto legend */
    Q_DECL_DEPRECATED QPixmap legendAsPixmap( bool );

    /** \brief Use this method when you want an annotated legend suitable for print output etc */
    Q_DECL_DEPRECATED QPixmap legendAsPixmap( int theLabelCount );

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

    /**  \brief [ data provider interface ] Which provider is being used for this Raster Layer?
     * @deprecated use providerType()
     */
    Q_DECL_DEPRECATED QString providerKey() const { return providerType(); }

    /** \brief Returns the number of raster units per each raster pixel. In a world file, this is normally the first row (without the sign) */
    double rasterUnitsPerPixel();

    const RasterStatsList rasterStatsList() const { return mRasterStatsList; }

    /** \brief Read color table from GDAL raster band */
    // Keep this for QgsRasterLayerProperties
    bool readColorTable( int theBandNumber, QList<QgsColorRampShader::ColorRampItem>* theList );

    /** \brief Simple reset function that set the noDataValue back to the value stored in the first raster band */
    void resetNoDataValue();

    /** \brief Mutator for blue band name mapping */
    void setBlueBandName( const QString & theBandName );

    /** \brief Mutator for color shader algorithm */
    void setColorShadingAlgorithm( QgsRasterLayer::ColorShadingAlgorithm theShaderAlgorithm );

    /** \brief Mutator for color shader algorithm */
    Q_DECL_DEPRECATED void setColorShadingAlgorithm( QString theShaderAlgorithm );

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
    Q_DECL_DEPRECATED void setGrayBandName( const QString & theBandName );

    /** \brief Mutator for green band name mapping  */
    Q_DECL_DEPRECATED void setGreenBandName( const QString & theBandName );

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    void setMaximumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for setting the maximum value for contrast enhancement */
    Q_DECL_DEPRECATED void setMaximumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Sets the minimum and maximum values for the band(s) currently
     * being displayed using the only pixel values from the last/current extent
     * */
    void setMinimumMaximumUsingLastExtent();

    /** \brief Sets the minimum and maximum values for the band(s) currently
     * being displayed using the only pixel values from the dataset min/max */
    void setMinimumMaximumUsingDataset();

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    Q_DECL_DEPRECATED void setMinimumValue( unsigned int theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator for setting the minimum value for contrast enhancement */
    Q_DECL_DEPRECATED void setMinimumValue( QString theBand, double theValue, bool theGenerateLookupTableFlag = true );

    /** \brief Mutator that allows the  NO_DATA entry for this raster to be overridden */
    void setNoDataValue( double theNoData );

    /** \brief Set the raster shader function to a user defined function
      \note ownership of the shader function is transfered to raster shader */
    Q_DECL_DEPRECATED void setRasterShaderFunction( QgsRasterShaderFunction* theFunction );

    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red color) */
    Q_DECL_DEPRECATED void setRedBandName( const QString & theBandName );

    /** \brief Mutator for transparent band name mapping  */
    Q_DECL_DEPRECATED void setTransparentBandName( const QString & theBandName );

    /**  \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int theValue );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Draws a thumbnail of the rasterlayer into the supplied pixmap pointer */
    void thumbnailAsPixmap( QPixmap * theQPixmap );
    /** \brief Draws a thumbnail of the rasterlayer into the supplied QImage pointer
     * @note added in QGIS 1.6
     * */
    void thumbnailAsImage( QImage * thepImage );

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
    virtual QDateTime timestamp() const { return mDataProvider->timestamp() ; }

  public slots:
    /** \brief Create GDAL pyramid overviews */
    QString buildPyramids( const RasterPyramidList &,
                           const QString &  theResamplingMethod = "NEAREST",
                           bool theTryInternalFlag = false );

    /** \brief test if the requested histogram is already available */

    bool hasCachedHistogram( int theBandNoInt,
                             int theBinCountInt = RASTER_HISTOGRAM_BINS );

    /** \brief Populate the histogram vector for a given band */

    void populateHistogram( int theBandNoInt,
                            int theBinCountInt = RASTER_HISTOGRAM_BINS,
                            bool theIgnoreOutOfRangeFlag = true,
                            bool theThoroughBandScanFlag = false );

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

  private:
    //
    // Private methods
    //
    /** \brief Drawing routine for color type data  */
    void drawSingleBandColorData( QPainter * theQPainter,
                                  QgsRasterViewPort * theRasterViewPort,
                                  const QgsMapToPixel* theQgsMapToPixel,
                                  int theBandNoInt );

    /** \brief Drawing routine for multiband image  */
    void drawMultiBandColor( QPainter * theQPainter,
                             QgsRasterViewPort * theRasterViewPort,
                             const QgsMapToPixel* theQgsMapToPixel );

    /** \brief Drawing routine for multiband image, rendered as a single band image in grayscale */
    void drawMultiBandSingleBandGray( QPainter * theQPainter,
                                      QgsRasterViewPort * theRasterViewPort,
                                      const QgsMapToPixel* theQgsMapToPixel,
                                      int theBandNoInt );

    /** \brief Drawing routine for multiband image, rendered as a single band image in pseudocolor */
    void drawMultiBandSingleBandPseudoColor( QPainter * theQPainter,
        QgsRasterViewPort * theRasterViewPort,
        const QgsMapToPixel* theQgsMapToPixel,
        int theBandNoInt );

    /** \brief Drawing routine for single band with a color map */
    void drawPalettedSingleBandColor( QPainter * theQPainter,
                                      QgsRasterViewPort * theRasterViewPort,
                                      const QgsMapToPixel* theQgsMapToPixel,
                                      int theBandNoInt );

    /** \brief Drawing routine for paletted image, rendered as a single band image in grayscale */
    void drawPalettedSingleBandGray( QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort,
                                     const QgsMapToPixel* theQgsMapToPixel,
                                     int theBandNoInt );

    /** \brief Drawing routine for paletted image, rendered as a single band image in pseudocolor */
    void drawPalettedSingleBandPseudoColor( QPainter * theQPainter,
                                            QgsRasterViewPort * theRasterViewPort,
                                            const QgsMapToPixel* theQgsMapToPixel,
                                            int theBandNoInt );

    /** \brief Drawing routine for paletted multiband image */
    void drawPalettedMultiBandColor( QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort,
                                     const QgsMapToPixel* theQgsMapToPixel,
                                     int theBandNoInt );

    /** \brief Drawing routine for single band grayscale image */
    void drawSingleBandGray( QPainter * theQPainter,
                             QgsRasterViewPort * theRasterViewPort,
                             const QgsMapToPixel* theQgsMapToPixel,
                             int theBandNoInt );

    /** \brief Drawing routine for single band grayscale image, rendered in pseudocolor */
    void drawSingleBandPseudoColor( QPainter * theQPainter,
                                    QgsRasterViewPort * theRasterViewPort,
                                    const QgsMapToPixel* theQgsMapToPixel,
                                    int theBandNoInt );

    /** \brief Close data provider and clear related members */
    void closeDataProvider();

    /** \brief helper function to create zero padded band names */
    QString  generateBandName( int );

    /** \brief Find out whether a given band exists.    */
    bool hasBand( const QString &  theBandName );

    /** \brief Query GDAL to find out the Wkt projection string for this layer.*/
    QString projectionWkt();

    /** \brief Allocate memory and load data to that allocated memory */
    //void* readData( GDALRasterBandH gdalBand, QgsRasterViewPort *viewPort );
    void* readData( int bandNo, QgsRasterViewPort *viewPort );

    /** \brief Load the given raster file */
    bool readFile( const QString & fileName );

    /** \brief Read a raster value given position from memory block created by readData() */
    //inline double readValue( void *data, GDALDataType type, int index );
    inline double readValue( void *data, int type, int index );

    /** \brief Update the layer if it is outdated */
    bool update();

    /** \brief Verify and transform band name for internal consistency. Return 'Not Set' on any type of failure */
    QString validateBandName( const QString & theBandName );

    //
    // Private member vars
    //
    /** \brief  Constant defining flag for XML and a constant that signals property not used */
    const QString QSTRING_NOT_SET;
    const QString TRSTRING_NOT_SET;

    /** \brief The number of bands in the dataset */
    int mBandCount;

    /** \brief The band to be associated with the color blue - usually 3 */
    QString mBlueBandName;

    /** \brief The raster shading algorithm being used */
    ColorShadingAlgorithm mColorShadingAlgorithm;

    /** \brief The contrast enhancement algorithm being used */
    QgsContrastEnhancement::ContrastEnhancementAlgorithm mContrastEnhancementAlgorithm;

    /** \brief List containing the contrast enhancements for each band */
    ContrastEnhancementList mContrastEnhancementList;

    /** \brief Number of stddev to plot (0) to ignore. Not applicable to all layer types */
    double mStandardDeviations;

    /**  [ data provider interface ] Pointer to data provider derived from the abstract base class QgsDataProvider */
    QgsRasterDataProvider* mDataProvider;

    DrawingStyle mDrawingStyle;

    /**  [ data provider interface ] Flag indicating whether the layer is in editing mode or not*/
    bool mEditable;

    /** [ data provider interface ]The error message associated with the last error */
    QString mError;

    /** [ data provider interface ] The error caption associated with the last error */
    QString mErrorCaption;

    /** \brief Pointer to the gdaldataset */
    //GDALDatasetH mGdalBaseDataset;

    /** \brief Pointer to the gdaldataset (possibly warped vrt) */
    //GDALDatasetH mGdalDataset;

    /** \brief Values for mapping pixel to world coordinates. Contents of this array are the same as the GDAL adfGeoTransform */
    double mGeoTransform[6];

    /** \brief Whether this raster has overviews / pyramids or not */
    bool mHasPyramids;

    /** \brief  Raster width */
    int mWidth;

    /** \brief  Raster height */
    int mHeight;

    /**  [ data provider interface ] Timestamp, the last modified time of the data source when the layer was created */
    QDateTime mLastModified;

    QgsRasterViewPort mLastViewPort;

    /**  [ data provider interface ] pointer for loading the provider library */
    //QLibrary* mLib;

    /**  [ data provider interface ] Flag indicating whether the layer has been modified since the last commit*/
    bool mModified;

    /** \brief Cell value representing no data. e.g. -9999  */
    double mNoDataValue;

    /**  [ data provider interface ] Data provider key */
    QString mProviderKey;

    /** \brief This list holds a series of RasterPyramid structs which store information for each potential pyramid level */
    RasterPyramidList mPyramidList;

    /** \brief A collection of stats - one for each band in the layer */
    RasterStatsList mRasterStatsList;

    LayerType mRasterType;

    /** \brief Flag to indicate if the user entered custom min max values */
    bool mUserDefinedGrayMinimumMaximum;

    /** \brief Flag to indicate if the user entered custom min max values */
    bool mUserDefinedRGBMinimumMaximum;

    /** \brief Flag indicating if the nodatavalue is valid*/
    bool mValidNoDataValue;

    QgsRasterRenderer* mRenderer;
};

#endif
