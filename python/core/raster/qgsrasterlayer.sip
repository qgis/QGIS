/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterlayer.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




typedef QList < QPair< QString, QColor > > QgsLegendColorList;


class QgsRasterLayer : QgsMapLayer
{
%Docstring
  This class provides qgis with the ability to render raster datasets
  onto the mapcanvas.

  The qgsrasterlayer class makes use of gdal for data io, and thus supports
  any gdal supported format. The constructor attempts to infer what type of
  file (LayerType) is being opened - not in terms of the file format (tif, ascii grid etc.)
  but rather in terms of whether the image is a GRAYSCALE, PaletteD or Multiband,

  Within the three allowable raster layer types, there are 8 permutations of
  how a layer can actually be rendered. These are defined in the DrawingStyle enum
  and consist of:

  SingleBandGray -> a GRAYSCALE layer drawn as a range of gray colors (0-255)
  SingleBandPseudoColor -> a GRAYSCALE layer drawn using a pseudocolor algorithm
  PalettedSingleBandGray -> a PaletteD layer drawn in gray scale (using only one of the color components)
  PalettedSingleBandPseudoColor -> a PaletteD layer having only one of its color components rendered as pseudo color
  PalettedMultiBandColor -> a PaletteD image where the bands contains 24bit color info and 8 bits is pulled out per color
  MultiBandSingleBandGray -> a layer containing 2 or more bands, but using only one band to produce a grayscale image
  MultiBandSingleBandPseudoColor -> a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
  MultiBandColor -> a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color

  Each of the above mentioned drawing styles is implemented in its own draw* function.
  Some of the drawing styles listed above require statistics about the layer such
  as the min / max / mean / stddev etc. statistics for a band can be gathered using the
  bandStatistics function. Note that statistics gathering is a slow process and
  every effort should be made to call this function as few times as possible. For this
  reason, qgsraster has a vector class member to store stats for each band. The
  constructor initializes this vector on startup, but only populates the band name and
  number fields.

  Note that where bands are of gdal 'undefined' type, their values may exceed the
  renderable range of 0-255. Because of this a linear scaling histogram enhanceContrast is
  applied to undefined layers to normalise the data into the 0-255 range.

  A qgsrasterlayer band can be referred to either by name or by number (base=1). It
  should be noted that band names as stored in datafiles may not be unique, and
  so the rasterlayer class appends the band number in brackets behind each band name.

  Sample usage of the QgsRasterLayer class:

 \code
     QString myFileNameQString = "/path/to/file";
     QFileInfo myFileInfo(myFileNameQString);
     QString myBaseNameQString = myFileInfo.baseName();
     QgsRasterLayer *myRasterLayer = new QgsRasterLayer(myFileNameQString, myBaseNameQString);

 \endcode

  In order to automate redrawing of a raster layer, you should like it to a map canvas like this :

 \code
     QObject.connect( myRasterLayer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()) );
 \endcode

 Once a layer has been created you can find out what type of layer it is (GrayOrUndefined, Palette or Multiband):

 \code
    if (rasterLayer->rasterType()==QgsRasterLayer.Multiband)
    {
      //do something
    }
    else if (rasterLayer->rasterType()==QgsRasterLayer.Palette)
    {
      //do something
    }
    else // QgsRasterLayer.GrayOrUndefined
    {
      //do something.
    }
 \endcode

  Raster layers can also have an arbitrary level of transparency defined, and have their
  color palettes inverted using the setTransparency and setInvertHistogram methods.

  Pseudocolor images can have their output adjusted to a given number of standard
  deviations using the setStandardDeviations method.

  The final area of functionality you may be interested in is band mapping. Band mapping
  allows you to choose arbitrary band -> color mappings and is applicable only to Palette
  and Multiband rasters, There are four mappings that can be made: red, green, blue and gray.
  Mappings are non-exclusive. That is a given band can be assigned to no, some or all
  color mappings. The constructor sets sensible defaults for band mappings but these can be
  overridden at run time using the setRedBandName, setGreenBandName, setBlueBandName and setGrayBandName
  methods.
%End

%TypeHeaderCode
#include "qgsrasterlayer.h"
%End
  public:

    static const double SAMPLE_SIZE;
%Docstring
 Default sample size (number of pixels) for estimated statistics/histogram calculation
%End

    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm SINGLE_BAND_ENHANCEMENT_ALGORITHM;
%Docstring
 Default enhancement algorithm for single band raster
%End

    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM;
%Docstring
 Default enhancement algorithm for multiple band raster of type Byte
%End

    static const QgsContrastEnhancement::ContrastEnhancementAlgorithm MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM;
%Docstring
 Default enhancement algorithm for multiple band raster of type different from Byte
%End

    static const QgsRasterMinMaxOrigin::Limits SINGLE_BAND_MIN_MAX_LIMITS;
%Docstring
 Default enhancement limits for single band raster
%End

    static const QgsRasterMinMaxOrigin::Limits MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS;
%Docstring
 Default enhancement limits for multiple band raster of type Byte
%End

    static const QgsRasterMinMaxOrigin::Limits MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS;
%Docstring
 Default enhancement limits for multiple band raster of type different from Byte
%End

    QgsRasterLayer();
%Docstring
 Constructor. Provider is not set.
%End

    struct LayerOptions
    {

      explicit LayerOptions( bool loadDefaultStyle = true );
%Docstring
 Constructor for LayerOptions.
%End

      bool loadDefaultStyle;
%Docstring
Set to true if the default layer style should be loaded
%End
    };

    explicit QgsRasterLayer( const QString &uri,
                             const QString &baseName = QString(),
                             const QString &providerKey = "gdal",
                             const QgsRasterLayer::LayerOptions &options = QgsRasterLayer::LayerOptions() );
%Docstring
 This is the constructor for the RasterLayer class.

 The main tasks carried out by the constructor are:

 -Load the rasters default style (.qml) file if it exists

 -Populate the RasterStatsVector with initial values for each band.

 -Calculate the layer extents

 -Determine whether the layer is gray, paletted or multiband.

 -Assign sensible defaults for the red, green, blue and gray bands.

 -
 *
%End

    ~QgsRasterLayer();

    virtual QgsRasterLayer *clone() const /Factory/;
%Docstring
 Returns a new instance equivalent to this one. A new provider is
  created for the same data source and renderer is cloned too.
 :return: a new layer instance
.. versionadded:: 3.0
 :rtype: QgsRasterLayer
%End

    enum ColorShadingAlgorithm
    {
      UndefinedShader,
      PseudoColorShader,
      FreakOutShader,
      ColorRampShader,
      UserDefinedShader
    };

    enum LayerType
    {
      GrayOrUndefined,
      Palette,
      Multiband,
      ColorLayer
    };

    static bool isValidRasterFileName( const QString &fileNameQString, QString &retError );
%Docstring
 This helper checks to see whether the file name appears to be a valid
  raster file name.  If the file name looks like it could be valid,
  but some sort of error occurs in processing the file, the error is
  returned in retError.
 :rtype: bool
%End
    static bool isValidRasterFileName( const QString &fileNameQString );
%Docstring
 :rtype: bool
%End

    static QDateTime lastModified( const QString   &name );
%Docstring
Return time stamp for given file name
 :rtype: QDateTime
%End

    void setDataProvider( const QString &provider );
%Docstring
[ data provider interface ] Set the data provider
%End

    LayerType rasterType();
%Docstring
  Accessor for raster layer type (which is a read only property)
 :rtype: LayerType
%End

    void setRenderer( QgsRasterRenderer *renderer /Transfer/ );
%Docstring
Set raster renderer. Takes ownership of the renderer object
%End
    QgsRasterRenderer *renderer() const;
%Docstring
 :rtype: QgsRasterRenderer
%End

    QgsRasterResampleFilter *resampleFilter() const;
%Docstring
Set raster resample filter. Takes ownership of the resample filter object
 :rtype: QgsRasterResampleFilter
%End

    QgsBrightnessContrastFilter *brightnessFilter() const;
%Docstring
 :rtype: QgsBrightnessContrastFilter
%End
    QgsHueSaturationFilter *hueSaturationFilter() const;
%Docstring
 :rtype: QgsHueSaturationFilter
%End

    QgsRasterPipe *pipe();
%Docstring
Get raster pipe
 :rtype: QgsRasterPipe
%End

    int width() const;
%Docstring
 Accessor that returns the width of the (unclipped) raster
 :rtype: int
%End

    int height() const;
%Docstring
 Accessor that returns the height of the (unclipped) raster
 :rtype: int
%End

    int bandCount() const;
%Docstring
 Get the number of bands in this layer
 :rtype: int
%End

    QString bandName( int bandNoInt ) const;
%Docstring
 Get the name of a band given its number
 :rtype: str
%End

    virtual QgsRasterDataProvider *dataProvider();


    virtual const QgsRasterDataProvider *dataProvider() const /PyName=constDataProvider/;

%Docstring
 Returns the data provider in a const-correct manner
.. note::

   available in Python bindings as constDataProvider()
 :rtype: QgsRasterDataProvider
%End

    virtual void reload();
%Docstring
Synchronises with changes in the datasource
%End

    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) /Factory/;
%Docstring
 Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
.. versionadded:: 2.4
 :rtype: QgsMapLayerRenderer
%End

    void draw( QPainter *theQPainter,
               QgsRasterViewPort *myRasterViewPort,
               const QgsMapToPixel *qgsMapToPixel = 0 );
%Docstring
 This is an overloaded version of the draw() function that is called by both draw() and thumbnailAsPixmap
%End

    QgsLegendColorList legendSymbologyItems() const;
%Docstring
Returns a list with classification items (Text and color)
 :rtype: QgsLegendColorList
%End

    virtual bool isSpatial() const;

    virtual QString htmlMetadata() const;


    QPixmap paletteAsPixmap( int bandNumber = 1 );
%Docstring
 Get an 100x100 pixmap of the color palette. If the layer has no palette a white pixmap will be returned
 :rtype: QPixmap
%End

    QString providerType() const;
%Docstring
 [ data provider interface ] Which provider is being used for this Raster Layer?
 :rtype: str
%End

    double rasterUnitsPerPixelX() const;
%Docstring
 Returns the number of raster units per each raster pixel in X axis. In a world file, this is normally the first row (without the sign)
 :rtype: float
%End
    double rasterUnitsPerPixelY() const;
%Docstring
 Returns the number of raster units per each raster pixel in Y axis. In a world file, this is normally the first row (without the sign)
 :rtype: float
%End



    void setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
                                 QgsRasterMinMaxOrigin::Limits limits = QgsRasterMinMaxOrigin::MinMax,
                                 const QgsRectangle &extent = QgsRectangle(),
                                 int sampleSize = QgsRasterLayer::SAMPLE_SIZE,
                                 bool generateLookupTableFlag = true );
%Docstring
 Set contrast enhancement algorithm
  \param algorithm Contrast enhancement algorithm
  \param limits Limits
  \param extent Extent used to calculate limits, if empty, use full layer extent
  \param sampleSize Size of data sample to calculate limits, if 0, use full resolution
  \param generateLookupTableFlag Generate lookup table. *
%End




    void setDefaultContrastEnhancement();
%Docstring
 Set default contrast enhancement
 :rtype: bool
%End

    virtual QStringList subLayers() const;
%Docstring
 Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS
 :rtype: list of str
%End

    QImage previewAsImage( QSize size, const QColor &bgColor = Qt::white,
                           QImage::Format format = QImage::Format_ARGB32_Premultiplied );
%Docstring
 Draws a preview of the rasterlayer into a QImage
.. versionadded:: 2.4
 :rtype: QImage
%End

    virtual void setLayerOrder( const QStringList &layers );
%Docstring
 Reorders the *previously selected* sublayers of this layer from bottom to top

 (Useful for providers that manage their own layers, such as WMS)
%End

    virtual void setSubLayerVisibility( const QString &name, bool vis );
%Docstring
 Set the visibility of the given sublayer name
%End

    virtual QDateTime timestamp() const;
%Docstring
Time stamp of data source in the moment when data/metadata were loaded by provider
 :rtype: QDateTime
%End

  public slots:
    void showStatusMessage( const QString &message );

  protected:
    virtual bool readSymbology( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context );

%Docstring
 Read the symbology for the current layer from the Dom node supplied
 :rtype: bool
%End

    virtual bool readStyle( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context );

%Docstring
 Read the style information for the current layer from the Dom node supplied
 :rtype: bool
%End

    virtual bool readXml( const QDomNode &layer_node, const QgsReadWriteContext &context );

%Docstring
 Reads layer specific state from project file Dom node
 :rtype: bool
%End

    virtual bool writeSymbology( QDomNode &, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;

%Docstring
 Write the symbology for the layer into the docment provided
 :rtype: bool
%End

    virtual bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;

%Docstring
 Write the style for the layer into the docment provided
 :rtype: bool
%End

    virtual bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const;

%Docstring
 Write layer specific state to project file Dom node
 :rtype: bool
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterlayer.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
