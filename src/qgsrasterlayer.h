/***************************************************************************
		 gsrasterlayer.h  -  description
			 -------------------
	begin                : Fri Jun 28 2002
	copyright            : (C) 2002 by Gary E.Sherman
	email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 Please observe the following variable naming guidelines when editing this class:
----------------
In my opinion, clarity of code is more important than brevity, so variables should be given clear, unambiguous names. Variables names should be written in mixed case, with a lowercase first letter. Each variable name should include a scope resolution indicator and a type indicator, in the form:

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

Using this scope resolution naming scheme makes the origin of each variable unambiguous and the code easy to read (especially by people who did not write it!).

The [name] part of the variable should be short and descriptive, usually a noun.

The [type] part of the variable should be the type class of the variable written out in full.
*/ 
 
 
#ifndef QGSRASTERLAYER_H
#define QGSRASTERLAYER_H

#include <qvaluevector.h>

#include "qgspoint.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"

class QgsRect;
class GDALDataset;
class GDALRasterBand;


struct RasterBandStats
{
    QString bandName;
    int bandNo;
    double minValDouble;
    double maxValDouble;
    //the distance between min & max
    double rangeDouble;
    double meanDouble;
    double sumSqrDevDouble; //used to calculate stddev
    double stdDevDouble;
    double sumDouble;
    int elementCountInt;
    double noDataDouble;
};

struct RasterViewPort
{
    int rectXOffsetInt;
    int rectYOffsetInt;
    double clippedXMinDouble;
    double clippedXMaxDouble;
    double clippedYMinDouble;
    double clippedYMaxDouble;
    int clippedWidthInt;
    int clippedHeightInt;
    QgsPoint topLeftPoint;
    QgsPoint bottomRightPoint;
    int drawableAreaXDimInt;
    int drawableAreaYDimInt;
    double noDataDouble;
};


typedef QMap<QString, RasterBandStats> RasterStatsMap;

/*! \class QgsRasterLayer
 * \brief Raster layer class
 */

class QgsRasterLayer : public QgsMapLayer
{
    Q_OBJECT
public:
    //! Constructor
    QgsRasterLayer(QString path = 0, QString baseName = 0);
    //! Destructor
    ~QgsRasterLayer();
    //this is called when the properties for this layer needs to be modified
    
     void showLayerProperties();
    //this is called when the view on the rasterlayer needs to be refreshed
    void draw(QPainter * theQPainter, QgsRect * theViewExtent, QgsCoordinateTransform * theQgsCoordinateTransform);


    //
    // Accessor and mutator for invertHistogramFlag
    //
    bool getInvertHistogramFlag()
    {
        return invertHistogramFlag;
    }
    void setInvertHistogramFlag(bool theFlag)
    {
        invertHistogramFlag=theFlag;
    }
    //
    // Accessor and mutator for stdDevsToPlotDouble
    //
    double getStdDevsToPlot()
    {
        return stdDevsToPlotDouble;
    };
    void setStdDevsToPlot(double theDouble)
    {
        stdDevsToPlotDouble = theDouble;
    };
    //get the number of bands in this layer
    const unsigned int getBandCount()
    {
        return rasterStatsMap.size();
    };
    // Get RasterBandStats for a band given its numer (read only)
    const  RasterBandStats getRasterBandStats(int);
    // Overloaded method that also returns stats for a band, but uses the band color name
    const  RasterBandStats getRasterBandStats(QString theBandName);
    // Find out whether a given band exists
    bool hasBand(QString theBandName);
    //accessor for transparency level
    unsigned int getTransparency();
    //mutator for transparency
    void setTransparency(unsigned int); //should be between 0 and 255
    // Accessor and mutator for red band name (allows alternate mappings e.g. map blue as red colour)
    QString getRedBandName()
    {
        return redBandNameQString;
    };
    void setRedBandName(QString theBandNameQString);
    // Accessor and mutator for green band name
    QString getGreenBandName()
    {
        return greenBandNameQString;
    };
    void setGreenBandName(QString theBandNameQString);
    // Accessor and mutator for blue band name
    QString getBlueBandName()
    {
        return blueBandNameQString;
    };
    void setBlueBandName(QString theBandNameQString);
    // Accessor and mutator for gray band name
    QString getGrayBandName()
    {
        return grayBandNameQString;
    };
    void setGrayBandName(QString theBandNameQString);
    // Accessor and mutator for showDebugOverlayFlag
    bool getShowDebugOverlayFlag()
    {
        return showDebugOverlayFlag;
    };
    void setShowDebugOverlayFlag(bool theFlag)
    {
        showDebugOverlayFlag=theFlag;
    };
    // Accessor and mutator for min and max red
    double getMinRedDouble()
    {
        return minRedDouble;
    };
    void setMinRedDouble(double theDouble)
    {
        minRedDouble=theDouble;
    };
    double getMaxRedDouble()
    {
        return maxRedDouble;
    };
    void setMaxRedDouble(double theDouble)
    {
        maxRedDouble=theDouble;
    };
    // Accessor and mutator for min and max green
    double getMinGreenDouble()
    {
        return minGreenDouble;
    };
    void setMinGreenDouble(double theDouble)
    {
        minGreenDouble=theDouble;
    };
    double getMaxGreenDouble()
    {
        return maxGreenDouble;
    };
    void setMaxGreenDouble(double theDouble)
    {
        maxGreenDouble=theDouble;
    };
    // Accessor and mutator for min and max red
    double getMinBlueDouble()
    {
        return minBlueDouble;
    };
    void setMinBlueDouble(double theDouble)
    {
        minBlueDouble=theDouble;
    };
    double getMaxBlueDouble()
    {
        return maxBlueDouble;
    };
    void setMaxBlueDouble(double theDouble)
    {
        maxBlueDouble=theDouble;
    };
    // Accessor and mutator for min and max red
    double getMinGrayDouble()
    {
        return minGrayDouble;
    };
    void setMinGrayDouble(double theDouble)
    {
        minGrayDouble=theDouble;
    };
    double getMaxGrayDouble()
    {
        return maxGrayDouble;
    };
    void setMaxGrayDouble(double theDouble)
    {
        maxGrayDouble=theDouble;
    };
    //this enumerator describes the types of scaling algorithms that can be used
    enum COLOR_SCALING_ALGORITHM
    {
        STRETCH_TO_MINMAX, //linear histogram stretch
        STRETCH_AND_CLIP_TO_MINMAX,
        CLIP_TO_MINMAX
    } colorScalingAlgorithm;
    //Accessor and mutator for the color scaling algorithm
    COLOR_SCALING_ALGORITHM getColorScalingAlgorithm()
    {
        return colorScalingAlgorithm;
    };
    void setColorScalingAlgorithm(COLOR_SCALING_ALGORITHM theAlgorithm)
    {
        colorScalingAlgorithm=theAlgorithm;
    };
    //this enumerator describes the different kinds of drawing we can do
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
    DRAWING_STYLE getDrawingStyle() {return drawingStyle;};
    void setDrawingStyle(DRAWING_STYLE theDrawingStyle) {drawingStyle=theDrawingStyle;};
    //this enumerator describes the type of raster layer
    enum RASTER_LAYER_TYPE
    {
        GRAY_OR_UNDEFINED,
	PALETTE,
	MULTIBAND    
    } rasterLayerType;
    //accessor and mutator for raster layer type
    RASTER_LAYER_TYPE getRasterLayerType() { return rasterLayerType; };
    void setRasterLayerType( RASTER_LAYER_TYPE theRasterLayerType ) { rasterLayerType=theRasterLayerType; };
    //get a legend image for this layer
    QPixmap getLegendQPixmap();
    // emit a signal asking for a repaint
    void triggerRepaint();
    

private:

    //
    // Private methods
    //
    void showDebugOverlay(QPainter * theQPainter, RasterViewPort * theRasterViewPort);

    void drawSingleBandGray(QPainter * theQPainter, RasterViewPort * theRasterViewPort, GDALRasterBand * theGdalBand);

    void drawSingleBandPseudoColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort,  GDALRasterBand * theGdalBand);

    void drawPalettedSingleBandGray(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                GDALRasterBand * theGdalBand,
                                QString theColorQString);

    void drawPalettedSingleBandPseudoColor(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                GDALRasterBand * theGdalBand,
                                QString theColorQString);

    void drawPalettedMultiBandColor(QPainter * theQPainter,
                                RasterViewPort * theRasterViewPort,
                                GDALRasterBand * theGdalBand);

    void drawMultiBandSingleBandGray(QPainter * theQPainter, RasterViewPort * theRasterViewPort);

    void drawMultiBandSingleBandPseudoColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort);

    void drawMultiBandColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort);


    //
    // Private member vars
    //

    //flag to indicate whether debug infor overlay should be rendered onto the raster
    bool showDebugOverlayFlag;
    //private method to calculate various stats about this layers band. If none is specified it will try to calc
    // stats for any Undefined, Gray or Palette layers it finds.
    void calculateStats(QString theBandNameQString);
    GDALDataset * gdalDataset;
    // values for mapping pixel to world coordinates
    double adfGeoTransform[6];
    // flag indicating whether the histogram should be inverted or not
    bool invertHistogramFlag;
    // Number of stddev to plot (0) to ignore
    double stdDevsToPlotDouble;
    // a collection of stats - one for each band in the layer
    // the map key corresonds to the gdal GetColorInterpretation for that band
    RasterStatsMap rasterStatsMap;
    // transparency for this layer should be 0-255
    unsigned int transparencyLevelInt;
    //the band to be associated with the color red - usually 1
    QString redBandNameQString;
    //the band to be associated with the color green - usually 2
    QString greenBandNameQString;
    //the band to be associated with the color blue - usually 3
    QString blueBandNameQString;
    //the band to be associated with the grayscale only ouput - usually 1
    QString grayBandNameQString;
    // minimum red value - used in scaling procedure
    double minRedDouble;
    // maximum red value - used in scaling procedure
    double maxRedDouble;
    // minimum green value - used in scaling procedure
    double minGreenDouble;
    // maximum green value - used in scaling procedure
    double maxGreenDouble;
    // minimum blue value - used in scaling procedure
    double minBlueDouble;
    // maximum blue value - used in scaling procedure
    double maxBlueDouble;
    // minimum gray value - used in scaling procedure
    double minGrayDouble;
    // maximum gray value - used in scaling procedure
    double maxGrayDouble;

    
signals:
    void repaintRequested();
};

#endif
