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

#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QPair>
#include <QVector>

#include "qgis.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgscolorrampshader.h"
#include "qgscontrastenhancement.h"
#include "qgshuesaturationfilter.h"
#include "qgsmaplayer.h"
#include "qgspoint.h"
#include "qgsraster.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"
#include "qgsrasterpipe.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrastershaderfunction.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"

class QgsMapToPixel;
class QgsRasterRenderer;
class QgsRectangle;
class QImage;
class QLibrary;
class QPixmap;
class QSlider;

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

    /**  [ data provider interface ] Set the data provider */
    void setDataProvider( const QString & provider );

    /** \brief Accessor for drawing style */
    DrawingStyle drawingStyle() { return mDrawingStyle; }

    /** \brief  Accessor for raster layer type (which is a read only property) */
    LayerType rasterType() { return mRasterType; }

    /** \brief Mutator for drawing style */
    void setDrawingStyle( const DrawingStyle &  theDrawingStyle ) { mDrawingStyle = theDrawingStyle; setRendererForDrawingStyle( theDrawingStyle ); }

    /**Set raster renderer. Takes ownership of the renderer object*/
    void setRenderer( QgsRasterRenderer* theRenderer );
    QgsRasterRenderer* renderer() const { return mPipe.renderer(); }

    /**Set raster resample filter. Takes ownership of the resample filter object*/
    QgsRasterResampleFilter * resampleFilter() const { return mPipe.resampleFilter(); }

    QgsBrightnessContrastFilter * brightnessFilter() const { return mPipe.brightnessFilter(); }
    QgsHueSaturationFilter * hueSaturationFilter() const { return mPipe.hueSaturationFilter(); }

    /** Get raster pipe */
    QgsRasterPipe * pipe() { return &mPipe; }

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

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure  */
    QString lastError();

    /** \brief [ data provider interface ] If an operation returns 0 (e.g. draw()), this function returns the text of the error associated with the failure */
    QString lastErrorTitle();

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
    double rasterUnitsPerPixel();

    /** \brief Set contrast enhancement algorithm
     *  @param theAlgorithm Contrast enhancement algorithm
     *  @param theLimits Limits
     *  @param theExtent Extent used to calculate limits, if empty, use full layer extent
     *  @param theSampleSize Size of data sample to calculate limits, if 0, use full resolution
     *  @param theGenerateLookupTableFlag Generate llokup table. */


    void setContrastEnhancementAlgorithm( QgsContrastEnhancement::ContrastEnhancementAlgorithm theAlgorithm,
                                          QgsRaster::ContrastEnhancementLimits theLimits = QgsRaster::ContrastEnhancementMinMax,
                                          QgsRectangle theExtent = QgsRectangle(),
                                          int theSampleSize = SAMPLE_SIZE,
                                          bool theGenerateLookupTableFlag = true );

    /** \brief Set default contrast enhancement */
    void setDefaultContrastEnhancement();

    /** \brief Overloaded version of the above function for convenience when restoring from xml */
    void setDrawingStyle( const QString & theDrawingStyleQString );

    /**  \brief [ data provider interface ] A wrapper function to emit a progress update signal */
    void showProgress( int theValue );

    /** \brief Returns the sublayers of this layer - Useful for providers that manage their own layers, such as WMS */
    virtual QStringList subLayers() const;

    /** \brief Draws a preview of the rasterlayer into a pixmap */
    QPixmap previewAsPixmap( QSize size, QColor bgColor = Qt::white );

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
    virtual QDateTime timestamp() const { return mDataProvider->timestamp() ; }

  public slots:
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
    /** \brief Initialize default values */
    void init();

    /** \brief Close data provider and clear related members */
    void closeDataProvider();

    /** \brief Update the layer if it is outdated */
    bool update();

    /**Sets corresponding renderer for style*/
    void setRendererForDrawingStyle( const DrawingStyle &  theDrawingStyle );

    /** \brief  Constant defining flag for XML and a constant that signals property not used */
    const QString QSTRING_NOT_SET;
    const QString TRSTRING_NOT_SET;

    /** Pointer to data provider */
    QgsRasterDataProvider* mDataProvider;

    DrawingStyle mDrawingStyle;

    /** [ data provider interface ]The error message associated with the last error */
    QString mError;

    /** [ data provider interface ] The error caption associated with the last error */
    QString mErrorCaption;

    /**  [ data provider interface ] Timestamp, the last modified time of the data source when the layer was created */
    QDateTime mLastModified;

    QgsRasterViewPort mLastViewPort;

    /**  [ data provider interface ] Data provider key */
    QString mProviderKey;

    LayerType mRasterType;

    QgsRasterPipe mPipe;
};

#endif
