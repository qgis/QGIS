/***************************************************************************
		 gsrasterlayer.h  -  description
			 -------------------
	begin                : Fri Jun 28 2002
	copyright            : (C) 2004 by T.Sutton, Gary E.Sherman, Steve Halsatz
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

/** \file qgsrasterlayer.h
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
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
 *     myRasterLayer->initContextMenu(this); //prepare the right click pop up menu
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
 
#include <qvaluevector.h>
#include "qgspoint.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"

//
// Forward declarations
//
class QgsRect;
class GDALDataset;
class GDALRasterBand;


//
// Structs
//

/** \brief The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
struct RasterBandStats
{
    /** \brief The name of the band that these stats belong to. */
    QString bandName;
    /** \brief The gdal band number (starts at 1)*/
    int bandNoInt; 
    /** \brief A flag to indicate whether this RasterBandStats struct 
     * is completely populated */
    bool statsGatheredFlag; 
    /** \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function. */
    double minValDouble;
    /** \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function. */
    double maxValDouble;
    /** \brief The range is the distance between min & max. */
    double rangeDouble;
    /** \brief The mean cell value for the band. NO_DATA values are excluded. */
    double meanDouble;
    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumSqrDevDouble; 
    /** \brief The standard deviation of the cell values. */
    double stdDevDouble;
    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sumDouble;
    /** \brief The number of cells in the band. Equivalent to height x width. 
     * TODO: check if NO_DATA are excluded!*/
    int elementCountInt;    
};

/** \brief The RasterViewPort describes the area of the raster layer that will be
 * rendered during a draw operation.
 */
struct RasterViewPort
{
    /** \brief  The offset from the left hand edge of the raster for the rectangle that will be drawn to screen.
     * TODO Check this explanation is correc!*/
    int rectXOffsetInt;
    /** \brief  The offset from the bottom edge of the raster for the rectangle that will be drawn to screen.
     * TODO Check this explanation is correc!*/
    int rectYOffsetInt;
    /** \brief Lower left X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedXMinDouble;
    /** \brief Top Right X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedXMaxDouble;
    /** \brief Lower left Y dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedYMinDouble;
    /** \brief Top Right X dimension of clipped raster image in raster pixel space.
     *  RasterIO will do the scaling for us, so for example, if the user is zoomed in a long way, there may only 
     *  be e.g. 5x5 pixels retrieved from the raw raster data, but rasterio will seamlessly scale the up to 
     *  whatever the screen coordinates are (e.g. a 600x800 display window) */
    double clippedYMaxDouble;
    /** \brief  Distance in pixels from clippedXMinDouble to clippedXMaxDouble. */
    int clippedWidthInt;
    /** \brief Distance in pixels from clippedYMinDouble to clippedYMaxDouble  */
    int clippedHeightInt;
    /** \brief Coordinate (in geographic coordinate system) of top left corner of the part of the raster that 
     * is to be rendered.*/
    QgsPoint topLeftPoint;
    /** \brief Coordinate (in geographic coordinate system) of bottom right corner of the part of the raster that 
     * is to be rendered.*/
    QgsPoint bottomRightPoint;
    /** \brief Distance in map units from left edge to right edge for the part of the raster that 
     * is to be rendered.*/
    int drawableAreaXDimInt;
    /** \brief Distance in map units from bottom edge to top edge for the part of the raster that 
     * is to be rendered.*/
    int drawableAreaYDimInt;
};


/** \brief  A vector containing one RasterBandStats struct per raster band in this raster layer.
 * Note that while very RasterBandStats element will have the name and number of its associated
 * band populated, any additional stats are calculated on a need to know basis.*/
typedef QValueVector<RasterBandStats> RasterStatsVector;

/*! \class QgsRasterLayer
 *  \brief This class provides qgis with the ability to render raster datasets
 *  onto the mapcanvas..
 */

class QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
public:
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
    QgsRasterLayer(QString path = 0, QString baseName = 0);
    /** \brief The destuctor.  */
    ~QgsRasterLayer();
    /** \brief This method is called when the properties for this layer needs to be modified. 
     * invokes an instance of the QgsRasterLayerProperties dialog box.*/
     void showLayerProperties();
    /** \brief This is called when the view on the rasterlayer needs to be refreshed (redrawn).  */
    void draw(QPainter * theQPainter, QgsRect * theViewExtent, QgsCoordinateTransform * theQgsCoordinateTransform);
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
    const double getNoDataValue() {return noDataValueDouble;};
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
    const unsigned int getBandCount()
    {
        return rasterStatsVector.size();
    };
    /** \brief Get RasterBandStats for a band given its number (read only)  */
    const  RasterBandStats getRasterBandStats(int);
    /** \brief  Check whether a given band number has stats associated with it */
    const bool hasStats(int theBandNoInt);
    /** \brief Overloaded method that also returns stats for a band, but uses the band colour name
    *    Note this approach is not recommeneded because it is possible for two gdal raster
    *    bands to have the same name!
    */
    const  RasterBandStats getRasterBandStats(QString);
    /** \brief Get the number of a band given its name. Note this will be the rewritten name set 
    *   up in the constructor, and will not necessarily be the same as the name retrieved directly from gdal!
    *   If no matching band is found zero will be returned! */
    const  int getRasterBandNumber (QString theBandNameQString);
    /** \brief Get the name of a band given its number.  */
    const  QString getRasterBandName(int theBandNoInt);
    /** \brief Find out whether a given band exists.    */
    bool hasBand(QString theBandName);
    /** \brief accessor for transparency level.  */
    unsigned int getTransparency();
    /** \brief Mutator for transparency level. Should be between 0 and 255 */
    void setTransparency(unsigned int); //
    /** \brief Accessor for red band name (allows alternate mappings e.g. map blue as red colour). */
    QString getRedBandName()
    {
        return redBandNameQString;
    };
    /** \brief Mutator for red band name (allows alternate mappings e.g. map blue as red colour). */
    void setRedBandName(QString theBandNameQString);
    // Accessor and mutator for green band name
    /** \brief   */
    QString getGreenBandName()
    {
        return greenBandNameQString;
    };
    /** \brief   */
    void setGreenBandName(QString theBandNameQString);
    // Accessor and mutator for blue band name
    /** \brief   */
    QString getBlueBandName()
    {
        return blueBandNameQString;
    };
    /** \brief   */
    void setBlueBandName(QString theBandNameQString);
    // Accessor and mutator for gray band name
    /** \brief   */
    QString getGrayBandName()
    {
        return grayBandNameQString;
    };
    /** \brief   */
    void setGrayBandName(QString theBandNameQString);
    // Accessor and mutator for showDebugOverlayFlag
    /** \brief   */
    bool getShowDebugOverlayFlag()
    {
        return showDebugOverlayFlag;
    };
    /** \brief   */
    void setShowDebugOverlayFlag(bool theFlag)
    {
        showDebugOverlayFlag=theFlag;
    };
    // Accessor and mutator for min and max red
    /** \brief   */
    double getMinRedDouble()
    {
        return minRedDouble;
    };
    /** \brief   */
    void setMinRedDouble(double theDouble)
    {
        minRedDouble=theDouble;
    };
    /** \brief   */
    double getMaxRedDouble()
    {
        return maxRedDouble;
    };
    /** \brief   */
    void setMaxRedDouble(double theDouble)
    {
        maxRedDouble=theDouble;
    };
    // Accessor and mutator for min and max green
    /** \brief   */
    double getMinGreenDouble()
    {
        return minGreenDouble;
    };
    /** \brief   */
    void setMinGreenDouble(double theDouble)
    {
        minGreenDouble=theDouble;
    };
    /** \brief   */
    double getMaxGreenDouble()
    {
        return maxGreenDouble;
    };
    /** \brief   */
    void setMaxGreenDouble(double theDouble)
    {
        maxGreenDouble=theDouble;
    };
    // Accessor and mutator for min and max red
    /** \brief   */
    double getMinBlueDouble()
    {
        return minBlueDouble;
    };
    /** \brief   */
    void setMinBlueDouble(double theDouble)
    {
        minBlueDouble=theDouble;
    };
    /** \brief   */
    double getMaxBlueDouble()
    {
        return maxBlueDouble;
    };
    /** \brief   */
    void setMaxBlueDouble(double theDouble)
    {
        maxBlueDouble=theDouble;
    };
    // Accessor and mutator for min and max red
    /** \brief   */
    double getMinGrayDouble()
    {
        return minGrayDouble;
    };
    /** \brief   */
    void setMinGrayDouble(double theDouble)
    {
        minGrayDouble=theDouble;
    };
    /** \brief   */
    double getMaxGrayDouble()
    {
        return maxGrayDouble;
    };
    /** \brief   */
    void setMaxGrayDouble(double theDouble)
    {
        maxGrayDouble=theDouble;
    };
    //this enumerator describes the types of scaling algorithms that can be used
    /** \brief   */
    enum COLOR_SCALING_ALGORITHM
    {
        STRETCH_TO_MINMAX, //linear histogram stretch
        STRETCH_AND_CLIP_TO_MINMAX,
        CLIP_TO_MINMAX
    } colorScalingAlgorithm;
    //Accessor and mutator for the color scaling algorithm
    /** \brief   */
    COLOR_SCALING_ALGORITHM getColorScalingAlgorithm()
    {
        return colorScalingAlgorithm;
    };
    /** \brief   */
    void setColorScalingAlgorithm(COLOR_SCALING_ALGORITHM theAlgorithm)
    {
        colorScalingAlgorithm=theAlgorithm;
    };
    //this enumerator describes the different kinds of drawing we can do
    /** \brief   */
    enum DRAWING_STYLE
    {
        SINGLE_BAND_GRAY, // a "Gray" or "Undefined" layer drawn as a range of gray colors
        SINGLE_BAND_PSEUDO_COLOR,// a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
        PALETTED_SINGLE_BAND_GRAY,// a "Palette" layer drawn in gray scale (using only one of the color components)
        PALETTED_SINGLE_BAND_PSEUDO_COLOR, // a "Palette" layer having only one of its color components rendered as psuedo color
        PALETTED_MULTI_BAND_COLOR, //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
        MULTI_BAND_SINGLE_BAND_GRAY, // a layer containing 2 or more bands, but using only one band to produce a grayscale image
        MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR, //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
        MULTI_BAND_COLOR //a layer containing 2 or more bands, mapped to the three RGBcolors. In the case of a multiband with only two bands, one band will have to be mapped to more than one color
    } drawingStyle;    
    //accessor and mutator for drawins style
    /** \brief   */
    DRAWING_STYLE getDrawingStyle() {return drawingStyle;};
    /** \brief   */
    QString getDrawingStyleAsQString();
    /** \brief   */
    void setDrawingStyle(DRAWING_STYLE theDrawingStyle) {drawingStyle=theDrawingStyle;};
    //overloaded version of the above function for convenience when restoring from xml
    /** \brief   */
    void setDrawingStyle(QString theDrawingStyleQString);
    //this enumerator describes the type of raster layer
    /** \brief   */
    enum RASTER_LAYER_TYPE
    {
        GRAY_OR_UNDEFINED,
	PALETTE,
	MULTIBAND    
    } rasterLayerType;
    //accessor and mutator for raster layer type
    /** \brief   */
    RASTER_LAYER_TYPE getRasterLayerType() { return rasterLayerType; };
    /** \brief   */
    void setRasterLayerType( RASTER_LAYER_TYPE theRasterLayerType ) { rasterLayerType=theRasterLayerType; };
    //get a legend image for this layer
    /** \brief   */
    QPixmap getLegendQPixmap();
    /** \brief   */
    QPixmap getLegendQPixmap(bool);
    //similar to above but returns a pointer. Implemented for qgsmaplayer interface
    /** \brief   */
    QPixmap * legendPixmap(); 
    // Initialise the right click popup menu
    /** \brief   */
    void initContextMenu(QgisApp *);
    /** Accessor for the superclass popmenu var - implements pure virtual fn*/
    /** \brief   */
    QPopupMenu *contextMenu();
    // emit a signal asking for a repaint
    /** \brief   */
    void triggerRepaint();
    

private:

    //
    // Private methods
    //
    /** \brief   */
    void showDebugOverlay(QPainter * theQPainter, RasterViewPort * theRasterViewPort);

    /** \brief   */
    void drawSingleBandGray(QPainter * theQPainter, RasterViewPort * theRasterViewPort,int theBandNoInt);

    /** \brief   */
    void drawSingleBandPseudoColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort,int theBandNoInt);

    /** \brief   */
    void drawPalettedSingleBandGray(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                int theBandNoInt,
                                QString theColorQString);

    /** \brief   */
    void drawPalettedSingleBandPseudoColor(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                int theBandNoInt,
                                QString theColorQString);

    /** \brief   */
    void drawPalettedMultiBandColor(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                int theBandNoInt);

    /** \brief   */
    void drawMultiBandSingleBandGray(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort, 
                                int theBandNoInt);

    /** \brief   */
    void drawMultiBandSingleBandPseudoColor(QPainter * theQPainter, 
                                RasterViewPort * theRasterViewPort, 
                                int theBandNoInt);

    /** \brief   */
    void drawMultiBandColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort);


    //
    // Private member vars
    //
    /** \brief   */
    int rasterXDimInt;
    /** \brief   */
    int rasterYDimInt;
    /** \brief   */
    double noDataValueDouble;
    //flag to indicate whether debug infor overlay should be rendered onto the raster
    /** \brief   */
    bool showDebugOverlayFlag;
    /** \brief   */
    GDALDataset * gdalDataset;
    // values for mapping pixel to world coordinates
    /** \brief   */
    double adfGeoTransform[6];
    // flag indicating whether the histogram should be inverted or not
    /** \brief   */
    bool invertHistogramFlag;
    // Number of stddev to plot (0) to ignore
    /** \brief   */
    double stdDevsToPlotDouble;
    // a collection of stats - one for each band in the layer
    // the typedef for this is defined above before class declaration
    /** \brief   */
    RasterStatsVector rasterStatsVector;
    // transparency for this layer should be 0-255
    /** \brief   */
    unsigned int transparencyLevelInt;
    //the band to be associated with the color red - usually 1
    /** \brief   */
    QString redBandNameQString;
    //the band to be associated with the color green - usually 2
    /** \brief   */
    QString greenBandNameQString;
    //the band to be associated with the color blue - usually 3
    /** \brief   */
    QString blueBandNameQString;
    //the band to be associated with the grayscale only ouput - usually 1
    /** \brief   */
    QString grayBandNameQString;
    // minimum red value - used in scaling procedure
    /** \brief   */
    double minRedDouble;
    // maximum red value - used in scaling procedure
    /** \brief   */
    double maxRedDouble;
    // minimum green value - used in scaling procedure
    /** \brief   */
    double minGreenDouble;
    // maximum green value - used in scaling procedure
    /** \brief   */
    double maxGreenDouble;
    // minimum blue value - used in scaling procedure
    /** \brief   */
    double minBlueDouble;
    // maximum blue value - used in scaling procedure
    /** \brief   */
    double maxBlueDouble;
    // minimum gray value - used in scaling procedure
    /** \brief   */
    double minGrayDouble;
    // maximum gray value - used in scaling procedure
    /** \brief   */
    double maxGrayDouble;

};

#endif
