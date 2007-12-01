/***************************************************************************
                        qgsrasterlayer.h  -  description
                              -------------------
	begin                : Fri Jun 28 2002
	copyright            : (C) 2004 by T.Sutton, Gary E.Sherman, Steve Halasz
	email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

/** \file qgsrasterlayer.h
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas
 *
 *  The qgsrasterlayer class makes use of gdal for data io, and thus supports
 *  any gdal supported format. The constructor attemtps to infer what type of
 *  file (RASTER_LAYER_TYPE) is being opened - not in terms of the file format (tif, ascii grid etc.)
 *  but rather in terms of whether the image is a GRAYSCALE, PALETTED or MULTIBAND,
 *
 *  Within the three allowable raster layer types, there are 8 permutations of 
 *  how a layer can actually be rendered. These are defined in the DRAWING_STYLE enum
 *  and consist of:
 *
 *  SINGLE_BAND_GRAY -> a GRAYSCALE layer drawn as a range of gray colors (0-255)
 *  SINGLE_BAND_PSEUDO_COLOR -> a GRAYSCALE layer drawn using a pseudocolor algorithm
 *  PALETTED_SINGLE_BAND_GRAY -> a PALLETED layer drawn in gray scale (using only one of the color components)
 *  PALETTED_SINGLE_BAND_PSEUDO_COLOR -> a PALLETED layer having only one of its color components rendered as psuedo color
 *  PALETTED_MULTI_BAND_COLOR -> a PALLETED image where the bands contains 24bit color info and 8 bits is pulled out per color
 *  MULTI_BAND_SINGLE_BAND_GRAY -> a layer containing 2 or more bands, but using only one band to produce a grayscale image
 *  MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR -> a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
 *  MULTI_BAND_COLOR -> a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
 *
 *  Each of the above mentioned drawing styles is implemented in its own draw* function.
 *  Some of the drawing styles listed above require statistics about the layer such 
 *  as the min / max / mean / stddev etc. Statics for a band can be gathered using the 
 *  getRasterBandStats function. Note that statistics gathering is a slow process and 
 *  evey effort should be made to call this function as few times as possible. For this
 *  reason, qgsraster has a vector class member to store stats for each band. The 
 *  constructor initialises this vector on startup, but only populates the band name and
 *  number fields.
 *  
 *  Note that where bands are of gdal 'undefined' type, their values may exceed the 
 *  renderable range of 0-255. Because of this a linear scaling histogram stretch is
 *  applied to undefined layers to normalise the data into the 0-255 range.
 *
 *  A qgsrasterlayer band can be referred to either by name or by number (base=1). It
 *  should be noted that band names as stored in datafiles may not be uniqe, and 
 *  so the rasterlayer class appends the band number in brackets behind each band name.
 *  
 *  Sample useage of the QgsRasterLayer class:
 *
 *     QString myFileNameQString = "/path/to/file";
 *     QFileInfo myFileInfo(myFileNameQString);
 *     QString myBaseNameQString = myFileInfo.baseName();
 *     QgsRasterLayer *myRasterLayer = new QgsRasterLayer(myFileNameQString, myBaseNameQString);
 *
 *  In order to automate redrawing of a raster layer, you should like it to a map canvas like this :
 *  
 *     QObject::connect( myRasterLayer, SIGNAL(repaintRequested()), mapCanvas, SLOT(refresh()) );
 *
 *  A raster layer can also export its legend as a pixmap:
 *
 *     QPixmap myQPixmap = myRasterLayer->legendPixmap();
 *
 * Once a layer has been created you can find out what type of layer it is (GRAY_OR_UNDEFINED, PALETTE or MULTIBAND):
 *
 *    if (rasterLayer->getRasterLayerType()==QgsRasterLayer::MULTIBAND)
 *    {
 *      //do something
 *    }
 *    else if (rasterLayer->getRasterLayerType()==QgsRasterLayer::PALETTE)
 *    {
 *      //do something
 *    }
 *    else // QgsRasterLayer::GRAY_OR_UNDEFINED
 *    {
 *      //do something.
 *    }
 *
 * You can combine layer type detection with the setDrawingStyle method to override the default drawing style assigned
 * when a layer is loaded.:
 *
 *    if (rasterLayer->getRasterLayerType()==QgsRasterLayer::MULTIBAND)
 *    {
 *       myRasterLayer->setDrawingStyle(QgsRasterLayer::MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR);
 *    }
 *    else if (rasterLayer->getRasterLayerType()==QgsRasterLayer::PALETTE)
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::PALETTED_SINGLE_BAND_PSEUDO_COLOR);
 *    }
 *    else // QgsRasterLayer::GRAY_OR_UNDEFINED
 *    {
 *      myRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
 *    }
 * 
 *  Raster layers can also have an aribitary level of transparency defined, and have their
 *  colour palettes inverted using the setTransparency and setInvertHistogramFlag methods. 
 * 
 *  Pseudocolour images can have their output adjusted to a given number of standard
 *  deviations using the setStdDevsToPlot method.
 * 
 *  The final area of functionality you may be interested in is band mapping. Band mapping
 *  allows you to choose arbitary band -> colour mappings and is applicable only to PALETTE
 *  and MULTIBAND rasters, There are four mappings that can be made : red, green, blue and gray.
 *  Mappings are non exclusive. That is a given band can be assigned to no, some or all 
 *  colour mappings. The constructor sets sensible defaults for band mappings but these can be
 *  overridden at run time using the setRedBandName,setGreenBandName,setBlueBandName and setGrayBandName 
 *  methods.
 */
 
/*
 * 
 * PROGRAMMERS NOTES:
 * 
 * 
 Please observe the following variable naming guidelines when editing this class:
----------------
In my opinion, clarity of code is more important than brevity, so variables should be given clear, 
unambiguous names. Variables names should be written in mixed case, with a lowercase first letter. 
Each variable name should include a scope resolution indicator and a type indicator, in the form:

[scope]+[name]+[type]

Where scope resolution indicators are:

- global vars and class members : [none]
- variables passed as parameters to a function/method: the
- variables declared locally in a method or function: my

For example:

class FooClass {
  int fooInt;  //class var has no prefix

  public void FooClass::fooMethod (int theBarInt)  //function parameter prefixed by 'the'
  {
    fooInt=1;
    int myLocalInt=0; //function members prefixed by 'my'
    myLocalInt=fooInt+theBarInt;
  }
}

Using this scope resolution naming scheme makes the origin of each variable unambiguous and the 
code easy to read (especially by people who did not write it!).

The [name] part of the variable should be short and descriptive, usually a noun.

The [type] part of the variable should be the type class of the variable written out in full.

*/ 
 
 
#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

//
// Includes
// 
 
#include <QDateTime>
#include <QVector>
#include <QList>

#include "qgis.h"
#include "qgspoint.h"
#include "qgsmaplayer.h"

/*
 * 
 * New includes that will convert this class to a data provider interface
 * (B Morley)
 *
 */ 
 
#include "qgsrasterdataprovider.h"

/*
 * END
 */

#include <gdal_priv.h>

//
// Forward declarations
//
class QgsColorTable;
class QgsRect;
class QgsRasterBandStats;
class QgsRasterPyramid;
class QgsRasterLayerProperties;
struct QgsRasterViewPort;
class QImage;
class QPixmap;

class GDALDataset;
class GDALRasterBand;
class QSlider;
class QLibrary;


  
  
/*! \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class CORE_EXPORT QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
public:
    //
    // Static methods:
    //
        
    static void buildSupportedRasterFileFilter(QString & fileFilters);
    static bool isSupportedRasterDriver(QString const &driverName);

    /** This helper checks to see whether the filename appears to be a valid
       raster file name */
    static bool isValidRasterFileName(QString const & theFileNameQString);

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
    QgsRasterLayer(QString const & path = QString::null, 
                   QString const &  baseName = QString::null);

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
    void identify(const QgsPoint& point, std::map<QString,QString>& results);

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

    /** \brief Returns the number of raster units per each raster pixel. For rasters with world file, this is
     normally the first row (without the sign) in that file */
    double rasterUnitsPerPixel();

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
    const int getRasterXDim() {return rasterXDimInt;};

    /** \brief Accessor that returns the height of the (unclipped) raster  */
    const int getRasterYDim() {return rasterYDimInt;};

    //
    // Accessor and mutator for no data double
    //
    /** \brief  Accessor that returns the NO_DATA entry for this raster. */
    const double getNoDataValue() {return noDataValueDouble;}

    /** \brief  Mutator that allows the  NO_DATA entry for this raster to be overridden. */
    void setNoDataValue(double theNoDataDouble) { noDataValueDouble=theNoDataDouble; return;};

    //
    // Accessor and mutator for invertHistogramFlag
    //
    /** \brief Accessor to find out whether the histogram should be inverted.   */
    bool getInvertHistogramFlag()
    {
        return invertHistogramFlag;
    }
    /** \brief Mutator to alter the state of the invert histogram flag.  */
    void setInvertHistogramFlag(bool theFlag)
    {
        invertHistogramFlag=theFlag;
    }
    //
    // Accessor and mutator for stdDevsToPlotDouble
    //
    /** \brief Accessor to find out how many standard deviations are being plotted.  */
    double getStdDevsToPlot()
    {
        return stdDevsToPlotDouble;
    };
    /** \brief Mutator to alter the number of standard deviations that should be plotted.  */
    void setStdDevsToPlot(double theDouble)
    {
        stdDevsToPlotDouble = theDouble;
    };
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
    const  QgsRasterBandStats getRasterBandStats(QString const &);
    /** \brief Get the number of a band given its name. Note this will be the rewritten name set 
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    const  int getRasterBandNumber (QString const & theBandNameQString);
    /** \brief Get the name of a band given its number.  */
    const  QString getRasterBandName(int theBandNoInt);
    /** \brief Find out whether a given band exists.    */
    bool hasBand(QString const &  theBandName);
    /** \brief Call any inline image manipulation filters */
    void filterLayer(QImage * theQImage);
    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red colour). */
    QString getRedBandName()
    {
        return redBandNameQString;
    }
    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red colour). */
    void setRedBandName(QString const & theBandNameQString);
    // 
    // Accessor and mutator for green band name
    // 
    /** \brief Accessor for green band name mapping.  */
    QString getGreenBandName()
    {
        return greenBandNameQString;
    }
    /** \brief Mutator for green band name mapping.  */
    void setGreenBandName(QString const & theBandNameQString);
    //
    // Accessor and mutator for blue band name
    // 
    /** \brief  Accessor for blue band name mapping. */
    QString getBlueBandName()
    {
        return blueBandNameQString;
    }
    /** \brief Mutator for blue band name mapping.  */
    void setBlueBandName(QString const & theBandNameQString);
    //
    // Accessor and mutator for transparent band name
    // 
    /** \brief  Accessor for transparent band name mapping. */
    QString getTransparentBandName()
    {
        return transparentBandNameQString;
    }
    /** \brief Mutator for transparent band name mapping.  */
    void setTransparentBandName(QString const & theBandNameQString);
    //
    // Accessor and mutator for gray band name
    //
    /** \brief Accessor for gray band name mapping.  */
    QString getGrayBandName()
    {
        return grayBandNameQString;
    }
    /** \brief Mutator for gray band name mapping.  */
    void setGrayBandName(QString const & theBandNameQString);
    // 
    // Accessor and mutator for showDebugOverlayFlag
    // 
    /** \brief Accessor for a flag that determines whether to show some debug info on the image.  */
    bool getShowDebugOverlayFlag()
    {
        return showDebugOverlayFlag;
    }
    /** \brief Mutator for a flag that determines whether to show some debug info on the image.  */
    void setShowDebugOverlayFlag(bool theFlag)
    {
        showDebugOverlayFlag=theFlag;
    }
    // 
    // Accessor and mutator for min and max red
    // 
    /** \brief Accessor for minimum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinRedDouble()
    {
        return minRedDouble;
    }
    /** \brief Mutator for minimum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinRedDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        minRedDouble=theDouble;
    }
    /** \brief Accessor for maximum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxRedDouble()
    {
        return maxRedDouble;
    }
    /** \brief Mutator for maximum clipping range for red.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxRedDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        maxRedDouble=theDouble;
    }
    // 
    // Accessor and mutator for min and max green
    // 
    /** \brief Accessor for minimum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinGreenDouble()
    {
        return minGreenDouble;
    }
    /** \brief Mutator for minimum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinGreenDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        minGreenDouble=theDouble;
    }
    /** \brief Accessor for maximum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxGreenDouble()
    {
        return maxGreenDouble;
    }
    /** \brief Mutator for maximum clipping range for green.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxGreenDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        maxGreenDouble=theDouble;
    }
    // 
    // Accessor and mutator for min and max blue
    // 
    /** \brief Accessor for minimum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    /** \brief   */
    double getMinBlueDouble()
    {
        return minBlueDouble;
    }
    /** \brief Mutator for minimum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinBlueDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        minBlueDouble=theDouble;
    }
    /** \brief Accessor for maximum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxBlueDouble()
    {
        return maxBlueDouble;
    }
    /** \brief Mutator for maximum clipping range for blue.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxBlueDouble(double theDouble)
    {
        userDefinedColorMinMax = true;
        maxBlueDouble=theDouble;
    }
    // 
    // Accessor and mutator for min and max gray
    // 
    /** \brief Accessor for minimum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMinGrayDouble()
    {
        return minGrayDouble;
    }
    /** \brief Mutator for minimum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMinGrayDouble(double theDouble)
    {
        userDefinedGrayMinMax = true;
        minGrayDouble=theDouble;
    }
    /** \brief Accessor for maximum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    double getMaxGrayDouble()
    {
        return maxGrayDouble;
    }
    /** \brief Mutator for maximum clipping range for gray.
     *
     * The clipping range can have different interpretations - it can either be used to perform
     * a histogram stretch between the minimum and maximum clipping values, or to exclude data
     * that falls outside the clipping range.*/
    void setMaxGrayDouble(double theDouble)
    {
        userDefinedGrayMinMax = true;
        maxGrayDouble=theDouble;
    }
    //
    /** \brief This enumerator describes the types of histogram scaling algorithms that can be used.  */
    enum COLOR_SCALING_ALGORITHM
    {
        NO_STRETCH, //this should be the default color scaling algorithm, will allow for the display of images without calling QgsRasterBandStats unless needed
        STRETCH_TO_MINMAX, //linear histogram stretch
        STRETCH_AND_CLIP_TO_MINMAX,
        CLIP_TO_MINMAX
    } colorScalingAlgorithm;
    //
    // Accessor and mutator for the color scaling algorithm
    //
    /** \brief Accessor for colour scaling algorithm. */
    COLOR_SCALING_ALGORITHM getColorScalingAlgorithm()
    {
        return colorScalingAlgorithm;
    }
    /** \brief Mutator for color scaling algorithm. */
    void setColorScalingAlgorithm(COLOR_SCALING_ALGORITHM theAlgorithm)
    {
        colorScalingAlgorithm=theAlgorithm;
    }
    
    /** \brief This enumerator describes the types of histogram colour ramping that can be used.  */
    enum COLOR_RAMPING_TYPE
    {
        BLUE_GREEN_RED, 
        FREAK_OUT //it will scare your granny!
    } colorRampingType;
    //
    // Accessor and mutator for the color ramping type
    //
    /** \brief Accessor for colour ramping type. */
    COLOR_RAMPING_TYPE getColorRampingType()
    {
        return colorRampingType;
    }
    /** \brief Mutator for color scaling algorithm. */
    void setColorRampingType(COLOR_RAMPING_TYPE theRamping)
    {
        colorRampingType=theRamping;
    }
    
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
    } drawingStyle;    
    //
    // Accessor and mutator for drawing style.
    //
    /** \brief Accessor for drawing style.  */
    DRAWING_STYLE getDrawingStyle() {return drawingStyle;}
    /** \brief Returns a string representation of drawing style.
     *
     * Implementaed mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future!. Use alternate implementation above rather.
     * */
    QString getDrawingStyleAsQString();
    /** \brief Mutator for drawing style.  */
    void setDrawingStyle(DRAWING_STYLE const &  theDrawingStyle) {drawingStyle=theDrawingStyle;}
    /** \brief Overloaded version of the above function for convenience when restoring from xml.
     *
     * Implementaed mainly for serialisation / deserialisation of settings to xml.
     * NOTE: May be deprecated in the future! Use alternate implementation above rather.
     * */
    void setDrawingStyle(QString  const & theDrawingStyleQString);




    /** \brief This enumerator describes the type of raster layer.  */
    enum RASTER_LAYER_TYPE
    {
        GRAY_OR_UNDEFINED,
	PALETTE,
	MULTIBAND    
    } rasterLayerType;
    //
    //accessor and for raster layer type (READ ONLY)
    //
    /** \brief  Accessor for raster layer type (which is a read only property) */
    RASTER_LAYER_TYPE getRasterLayerType() { return rasterLayerType; }
    /** \brief Accessor for hasPyramidsFlag (READ ONLY) */
    bool getHasPyramidsFlag() {return hasPyramidsFlag;}
     
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
    virtual void setLayerOrder(QStringList const & layers);
    
    /**
     * Set the visibility of the given sublayer name
     */
    virtual void setSubLayerVisibility(QString const & name, bool vis);

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
    static QDateTime lastModified ( QString const &  name );

    /**Copies the symbology settings from another layer. Returns true in case of success*/
    bool copySymbologySettings(const QgsMapLayer& other) {
      //preventwarnings
      if (other.type() < 0) 
      {
        return false;
      }
      return false;
    } //todo

    bool isSymbologyCompatible(const QgsMapLayer& other) const
    {
      UNUSED(other);
      return false;
    } //todo

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
    const QgsRasterDataProvider* getDataProvider() const;

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
    QString buildPyramids(RasterPyramidList const &, 
                          QString const &  theResamplingMethod="NEAREST");
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
    
private:

    //
    // Private methods
    //
    /** \brief Paint debug information onto the output image.  */
    void showDebugOverlay(QPainter * theQPainter, QgsRasterViewPort * theRasterViewPort);

    //
    // Grayscale Imagery
    //

    /** \brief Drawing routine for single band grayscale image.  */
    void drawSingleBandGray(QPainter * theQPainter, 
                            QgsRasterViewPort * theRasterViewPort,
                            QgsMapToPixel * theQgsMapToPixel,
                            int theBandNoInt);

    /** \brief Drawing routine for single band grayscale image, rendered in pseudocolor.  */
    void drawSingleBandPseudoColor(QPainter * theQPainter, 
                                   QgsRasterViewPort * theRasterViewPort,
                                   QgsMapToPixel * theQgsMapToPixel,
                                   int theBandNoInt);


    //
    // Paletted Layers
    //
    
    /** \brief Drawing routine for paletted image, rendered as a single band image in color.  */
    void drawPalettedSingleBandColor(QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort,
                                     QgsMapToPixel * theQgsMapToPixel,
                                     int theBandNoInt);
    
    /** \brief Drawing routine for paletted image, rendered as a single band image in grayscale.  */
    void drawPalettedSingleBandGray(QPainter * theQPainter,
                                    QgsRasterViewPort * theRasterViewPort,
                                    QgsMapToPixel * theQgsMapToPixel,
                                    int theBandNoInt,
                                    QString const &  theColorQString);

    /** \brief Drawing routine for paletted image, rendered as a single band image in pseudocolor.  */
    void drawPalettedSingleBandPseudoColor(QPainter * theQPainter,
                                           QgsRasterViewPort * theRasterViewPort,
                                           QgsMapToPixel * theQgsMapToPixel,
                                           int theBandNoInt,
                                           QString const &  theColorQString);

    /** \brief Drawing routine for paletted multiband image.  */
    void drawPalettedMultiBandColor(QPainter * theQPainter,
                                    QgsRasterViewPort * theRasterViewPort,
                                    QgsMapToPixel * theQgsMapToPixel,                                
                                    int theBandNoInt);

    //
    // Multiband Layers
    //
    
    /** \brief Drawing routine for multiband image, rendered as a single band image in grayscale.  */
    void drawMultiBandSingleBandGray(QPainter * theQPainter,
                                     QgsRasterViewPort * theRasterViewPort, 
                                     QgsMapToPixel * theQgsMapToPixel,
                                     int theBandNoInt);

    /** \brief Drawing routine for multiband image, rendered as a single band image in pseudocolor.  */
    void drawMultiBandSingleBandPseudoColor(QPainter * theQPainter, 
                                            QgsRasterViewPort * theRasterViewPort, 
                                            QgsMapToPixel * theQgsMapToPixel,
                                            int theBandNoInt);

    /** \brief Drawing routine for multiband image  */
    void drawMultiBandColor(QPainter * theQPainter, 
                            QgsRasterViewPort * theRasterViewPort,
                            QgsMapToPixel * theQgsMapToPixel);

    /** \brief Read color table from GDAL raster band */
    void readColorTable ( GDALRasterBand *gdalBand, QgsColorTable *theColorTable );

    /** \brief Allocate memory and load data to that allocated memory, data type is the same
     *         as raster band. The memory must be released later!
     *  \return pointer to the memory
     */
    void *readData ( GDALRasterBand *gdalBand, QgsRasterViewPort *viewPort );

    /** \brief Read a raster value on given position from memory block created by readData() 
     *  \param index index in memory block
     */
    inline double readValue ( void *data, GDALDataType type, int index );


    /**
       Load the given raster file

       @returns true if successfully read file

       @note
       
       Called from ctor if a raster image given there
     */
    bool readFile( QString const & fileName );
    
    /** \brief Close data set and release related data */
    void closeDataset ();

    /** \brief Update the layer if it is outdated */
    bool update ();

    //
    // Private member vars
    //
    /** \brief  Raster width. */
    int rasterXDimInt;
    /** \brief  Raster Height. */
    int rasterYDimInt;
    /** \brief Cell value representing no data. e.g. -9999  */
    double noDataValueDouble;
    /** \brief Flag to indicate whether debug infor overlay should be rendered onto the raster.  */
    bool showDebugOverlayFlag;
    /** \brief Pointer to the gdaldataset.  */
    GDALDataset * gdalDataset;
    /** \brief Values for mapping pixel to world coordinates.  */
    double adfGeoTransform[6];
    /** \brief Flag indicating whether the histogram should be inverted or not.  */
    bool invertHistogramFlag;
    /** \brief Number of stddev to plot (0) to ignore. Not applicable to all layer types.  */
    double stdDevsToPlotDouble;
    /** \brief A collection of stats - one for each band in the layer.
     * The typedef for this is defined above before class declaration
     */
    RasterStatsVector rasterStatsVector;
    /** \brief The band to be associated with the color red - usually 1.  */
    QString redBandNameQString;
    /** \brief The band to be associated with the color green - usually 2.  */
    QString greenBandNameQString;
    /** \brief The band to be associated with the color blue - usually 3.  */
    QString blueBandNameQString;
    /** \brief The band to be associated with transparency.  */
    QString transparentBandNameQString;
    /** \brief The band to be associated with the grayscale only ouput - usually 1.  */
    QString grayBandNameQString;
    /** \brief Minimum red value - used in scaling procedure.  */
    double minRedDouble;
    /** \brief Maximum red value - used in scaling procedure.  */
    double maxRedDouble;
    /** \brief Minimum green value - used in scaling procedure.  */
    double minGreenDouble;
    /** \brief Maximum green value - used in scaling procedure.  */
    double maxGreenDouble;
    /** \brief Minimum blue value - used in scaling procedure.  */
    double minBlueDouble;
    /** \brief Maximum blue value - used in scaling procedure.  */
    double maxBlueDouble;
    /** \brief Minimum gray value - used in scaling procedure.  */
    double minGrayDouble;
    /** \brief Maximum gray value - used in scaling procedure.  */
    double maxGrayDouble;
    /** \brief Whether this raster has overviews / pyramids or not */
    bool hasPyramidsFlag;
    //Since QgsRasterBandStats deos not set the minRedDouble maxRedDouble etc., it is benificial to know if the user as set these values. Default = false
    bool userDefinedColorMinMax;
    bool userDefinedGrayMinMax;
    /** \brief This list holds a series of RasterPyramid structs
     * which store infomation for each potential pyramid level for this raster.*/
    RasterPyramidList mPyramidList;


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
                 QString const & baseName = QString(),
                 QString const & path = QString(),
                 QString const & providerLib = QString(),
                 QStringList const & layers = QStringList(),
                 QStringList const & styles = QStringList(),
                 QString const & format = QString(),
                 QString const & crs = QString(),
                 QString const & proxyHost = QString(),
                 int proxyPort = 80,
                 QString const & proxyUser = QString(),
                 QString const & proxyPass = QString());

  void setDataProvider( QString const & provider,
                        QStringList const & layers,
                        QStringList const & styles,
                        QString const & format,
                        QString const & crs,
                        QString const & proxyHost,
                        int proxyPort,
                        QString const & proxyUser,
                        QString const & proxyPass );

  //! Does this layer use a provider for setting/retrieving data?
  bool usesProvider();

  /**
   * Sets a proxy for the path given in the constructor
   *
   * \retval TRUE if proxy setting is successful (if indeed it is supported)
   */
  bool setProxy(QString const & host = 0,
                            int port = 80,
                QString const & user = 0,
                QString const & pass = 0);

  //! Which provider is being used for this Raster Layer?
  QString providerKey();

public slots:

  void showStatusMessage(QString const & theMessage);


private:

  //! Data provider key
  QString mProviderKey;
  
  //! pointer for loading the provider library
  QLibrary *myLib;

  //! Pointer to data provider derived from the abstract base class QgsDataProvider
  QgsRasterDataProvider *dataProvider;

  /**Flag indicating wheter the layer is in editing mode or not*/
  bool mEditable;
  
  /**Flag indicating wheter the layer has been modified since the last commit*/
  bool mModified;

  //! Timestamp, the last modified time of the data source when the layer was created
  QDateTime mLastModified;

  /**
   * The error caption associated with the last error.
   */
  QString mErrorCaption;

  /**
   * The error message associated with the last error.
   */
  QString mError;

};

#endif
