/* **************************************************************************
   qgsrasterlayer.cpp -  description
   -------------------
begin                : Sat Jun 22 2002
copyright            : (C) 2003 by Tim Sutton, Steve Halasz and Gary E.Sherman
email                : tim at linfiniti.com
 ***************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

/*
   Please observe the following variable naming guidelines when editing this class:
   ---------------------------------------------------------------------------------
   In my opinion, clarity of code is more important than brevity, so variables should be 
   given clear, unambiguous names. Variables names should be written in mixed case, with 
   a lowercase first letter. Each variable name should include a scope resolution 
   indicator and a type indicator, in the form:

   [scope]+[name]+[type]

   Where scope resolution indicators are:

   - global vars and class members : [none]
   - variables passed as parameters to a function/method: the
   - variables declared locally in a method or function: my

   For example:

   class FooClass {
   int fooInt;  //class var has no prefix

   void FooClass::fooMethod (int theBarInt)  //function parameter prefixed by 'the'
   {
   fooInt=1;
   int myLocalInt=0; //function members prefixed by 'my'
   myLocalInt=fooInt+theBarInt;
   }
   }

   Using this scope resolution naming scheme makes the origin of each variable unambiguous
   and the code easy to read (especially by people who did not write it!).

   The [name] part of the variable should be short and descriptive, usually a noun.

   The [type] part of the variable should be the type class of the variable written out in full.


   DEBUG DIRECTIVES:

   When compiling you can make sure DEBUG is defined by including -DDEBUG in the gcc command (e.g. gcc -DDEBUG myprog.c ) if you 
   wish to see edbug messages printed to stdout.

*/




#include <qpainter.h>
#include <qimage.h>
#include <qfont.h>
#include <qfile.h>
#include <qfontmetrics.h>
#include <qwmatrix.h>
#include <qpopupmenu.h>
#include <stdio.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qslider.h>
#include <qlabel.h>

#include "qgsrasterlayer.h"
#include "qgsrect.h"
#include "qgisapp.h"
#include "qgsrasterlayerproperties.h"
#include "gdal_priv.h"
#include <math.h>

QgsRasterLayer::QgsRasterLayer(QString path, QString baseName):QgsMapLayer(RASTER, baseName, path)
{
  //we need to do the tr() stuff outside of the loop becauses tr() is a
  //time consuming operation nd we dont want to do it in the loop!
  redTranslatedQString=tr("Red");
  greenTranslatedQString=tr("Green");
  blueTranslatedQString=tr("Blue");


  //popMenu is the contextmenu obtained by right clicking on the legend
  //it is a member of the qgsmaplayer base class
  popMenu = 0;

  // set the layer name (uppercase first character)
  QString layerTitle = baseName;
  layerTitle = layerTitle.left(1).upper() + layerTitle.mid(1);
  setLayerName(layerTitle);

  GDALAllRegister();
  gdalDataset = (GDALDataset *) GDALOpen(path, GA_ReadOnly);
  if (gdalDataset == NULL)
  {
    valid = FALSE;
    return;
  }
  //check f this file has pyramids
  GDALRasterBandH myGDALBand = GDALGetRasterBand( gdalDataset, 1 ); //just use the first band
  if( GDALGetOverviewCount(myGDALBand) > 0 )
  {
    hasPyramidsFlag=true;
  }
  else
  {
    hasPyramidsFlag=false;
  }
  //populate the list of what pyramids exist
  buildRasterPyramidList(); 
  //load  up the pyramid icons
  mPyramidPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/pyramid.png"));
  mNoPyramidPixmap.load(QString(PKGDATAPATH) + QString("/images/icons/no_pyramid.png"));
  //just testing remove this later
  getMetadata();

  double myXMaxDouble = adfGeoTransform[0] + gdalDataset->GetRasterXSize() * adfGeoTransform[1] +
      gdalDataset->GetRasterYSize() * adfGeoTransform[2];
  double myYMinDouble = adfGeoTransform[3] + gdalDataset->GetRasterXSize() * adfGeoTransform[4] +
      gdalDataset->GetRasterYSize() * adfGeoTransform[5];

  layerExtent.setXmax(myXMaxDouble);
  layerExtent.setXmin(adfGeoTransform[0]);
  layerExtent.setYmax(adfGeoTransform[3]);
  layerExtent.setYmin(myYMinDouble);
  //
  // Set up the x and y dimensions of this raster layer
  //
  rasterXDimInt = gdalDataset->GetRasterXSize();
  rasterYDimInt = gdalDataset->GetRasterYSize();
  //
  // Determin the no data value
  //
  noDataValueDouble = gdalDataset->GetRasterBand(1)->GetNoDataValue();

  //initialise the raster band stats vector
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(i);
    QString myColorQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    RasterBandStats myRasterBandStats;
    myRasterBandStats.bandName = myColorQString + " (" + QString::number(i) + ")";
    //myRasterBandStats.bandName=QString::number(i) + " : " + myColorQString;
    myRasterBandStats.bandNoInt = i;
    myRasterBandStats.statsGatheredFlag = false;
    rasterStatsVector.push_back(myRasterBandStats);
  }
  //decide what type of layer this is...
  //note that multiband images can have one or more 'undefindd' bands,
  //so we must do this check first!
  if ((gdalDataset->GetRasterCount() > 1))
  {
    rasterLayerType = MULTIBAND;
  }
  else if (hasBand("Palette")) //dont tr() this its a gdal word!
  {
    rasterLayerType = PALETTE;
  } else
  {
    rasterLayerType = GRAY_OR_UNDEFINED;
  }

  if (rasterLayerType == PALETTE)
  {
    redBandNameQString = redTranslatedQString; // sensible default
    greenBandNameQString = greenTranslatedQString; // sensible default
    blueBandNameQString = blueTranslatedQString; // sensible default
    grayBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = PALETTED_MULTI_BAND_COLOR; //sensible default
  }
  else if (rasterLayerType == MULTIBAND)
  {
    //we know we have at least 2 layers...
    redBandNameQString = getRasterBandName(1);  // sensible default
    greenBandNameQString = getRasterBandName(2);  // sensible default
    //for the third layer we cant be sure so..
    if (gdalDataset->GetRasterCount() > 2)
    {
      blueBandNameQString = getRasterBandName(3); // sensible default
    }
    else
    {
      blueBandNameQString = tr("Not Set");  // sensible default
    }
    grayBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = MULTI_BAND_COLOR;  //sensible default
  }
  else                        //GRAY_OR_UNDEFINED
  {
    getRasterBandStats(1);
    redBandNameQString = tr("Not Set"); //sensible default
    greenBandNameQString = tr("Not Set"); //sensible default
    blueBandNameQString = tr("Not Set");  //sensible default
    drawingStyle = SINGLE_BAND_GRAY;  //sensible default
    if (hasBand("Gray"))
    {
      grayBandNameQString = "Gray"; // sensible default //dont tr() this its a gdal word!
    }
    else if (hasBand("Undefined")) //dont tr() this its a gdal word!
    {
      grayBandNameQString = "Undefined";  // sensible default
    }
  }

  invertHistogramFlag = false;  // sensible default
  stdDevsToPlotDouble = 0;      // sensible default
  transparencyLevelInt = 255;   //sensible default 0 is transparent
  showDebugOverlayFlag = false; //sensible default
  //  Transparency slider for popup meni
  //  QSlider ( int minValue, int maxValue, int pageStep, int value, Orientation orientation, QWidget * parent, const char * name = 0 )
  mTransparencySlider = new QSlider(0,255,5,0,QSlider::Horizontal,popMenu);
  mTransparencySlider->setTickmarks(QSlider::Both);
  mTransparencySlider->setTickInterval(25);
  mTransparencySlider->setTracking(false); //stop slider emmitting a signal until mouse released
  connect(mTransparencySlider, SIGNAL(valueChanged(int)), this, SLOT(popupTransparencySliderMoved(int)));
  // emit a signal asking for a repaint
  emit repaintRequested();
}

QgsRasterLayer::~QgsRasterLayer()
{
  GDALClose(gdalDataset);
}

void QgsRasterLayer::showLayerProperties()
{
  QgsRasterLayerProperties *myRasterLayerProperties = new QgsRasterLayerProperties(this);

}

// emit a signal asking for a repaint
void QgsRasterLayer::triggerRepaint()
{
  emit repaintRequested();
}


QString QgsRasterLayer::getDrawingStyleAsQString()
{
  switch (drawingStyle)
  {
      case SINGLE_BAND_GRAY:
          return QString("SINGLE_BAND_GRAY"); //no need to tr() this its not shown in ui
          break;
      case SINGLE_BAND_PSEUDO_COLOR:
          return QString("SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
          break;
      case PALETTED_SINGLE_BAND_GRAY:
          return QString("PALETTED_SINGLE_BAND_GRAY");//no need to tr() this its not shown in ui
          break;
      case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
          return QString("PALETTED_SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
          break;
      case PALETTED_MULTI_BAND_COLOR:
          return QString("PALETTED_MULTI_BAND_COLOR");//no need to tr() this its not shown in ui
          break;
      case MULTI_BAND_SINGLE_BAND_GRAY:
          return QString("MULTI_BAND_SINGLE_BAND_GRAY");//no need to tr() this its not shown in ui
          break;
      case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
          return QString("MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR");//no need to tr() this its not shown in ui
          break;
      case MULTI_BAND_COLOR:
          return QString("MULTI_BAND_COLOR");//no need to tr() this its not shown in ui
          break;
      default:
          break;
  }
}

void QgsRasterLayer::setDrawingStyle(QString theDrawingStyleQString)
{
  if (theDrawingStyleQString == "SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "PALETTED_MULTI_BAND_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = PALETTED_MULTI_BAND_COLOR;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_GRAY")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_GRAY;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR;
    return;
  }
  if (theDrawingStyleQString == "MULTI_BAND_COLOR")//no need to tr() this its not shown in ui
  {
    drawingStyle = MULTI_BAND_COLOR;
    return;
  }
}


/** This method looks to see if a given band name exists. Note
  that in muliband layers more than one "Undefined" band can exist! */
bool QgsRasterLayer::hasBand(QString theBandName)
{
#ifdef QGISDEBUG
  std::cout << "Looking for band : " << theBandName << std::endl;
#endif

  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(i);
    QString myColorQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
#ifdef QGISDEBUG

    std::cout << "band : " << i << std::endl;
#endif

    if (myColorQString == theBandName)
    {
#ifdef QGISDEBUG
      std::cout << "band : " << i << std::endl;
      std::cout << "Found band : " << theBandName << std::endl;
#endif

      return true;
    }
#ifdef QGISDEBUG
    std::cout << "Found unmatched band : " << i << " " << myColorQString << std::endl;
#endif

  }
  return false;
}

void QgsRasterLayer::drawThumbnail(QPixmap * theQPixmap)
{
  theQPixmap->fill(); //defaults to white
  RasterViewPort *myRasterViewPort = new RasterViewPort();
  myRasterViewPort->rectXOffsetInt = 0;
  myRasterViewPort->rectYOffsetInt = 0;
  myRasterViewPort->clippedXMinDouble = 0;
  myRasterViewPort->clippedXMaxDouble = rasterXDimInt;
  myRasterViewPort->clippedYMinDouble = rasterYDimInt;
  myRasterViewPort->clippedYMaxDouble = 0;
  myRasterViewPort->clippedWidthInt   = rasterXDimInt;
  myRasterViewPort->clippedHeightInt  = rasterYDimInt;
  myRasterViewPort->topLeftPoint = QgsPoint(0,0);
  myRasterViewPort->bottomRightPoint = QgsPoint(theQPixmap->width(), theQPixmap->height());
  myRasterViewPort->drawableAreaXDimInt = theQPixmap->width();
  myRasterViewPort->drawableAreaYDimInt = theQPixmap->height();

  QPainter * myQPainter=new QPainter(theQPixmap);
  draw(myQPainter,myRasterViewPort);
  delete myRasterViewPort;
  myQPainter->end();
  delete myQPainter;
}

QPixmap QgsRasterLayer::getPaletteAsPixmap()
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::getPaletteAsPixmap" << std::endl;
#endif
  if (hasBand("Palette")) //dont tr() this its a gdal word!
  {
#ifdef QGISDEBUG
  std::cout << "....found paletted image" << std::endl;
#endif
    GDALRasterBandH myGdalBand = gdalDataset->GetRasterBand(1);
    if( GDALGetRasterColorInterpretation(myGdalBand) == GCI_PaletteIndex )
    {
#ifdef QGISDEBUG
  std::cout << "....found GCI_PaletteIndex" << std::endl;
#endif
      GDALColorTableH myTable;
      myTable = GDALGetRasterColorTable( myGdalBand );
      int myColourCountInt = GDALGetColorEntryCount( myTable ) ;

      //we would like to have the palette drawn onto a more or less
      //square pixmap. So we need to determine the square root of the 
      //total palette entry count and handle any non int remainders
      //by adding an extra row if neccessary
      //
      int myWidth=(int) sqrt(myColourCountInt);
      int myHeight;
      if ((myWidth*myWidth) < myColourCountInt)
      {
        //add an extra row for the remainder
        myHeight=myWidth+1;
      }
      else
      {
        //pixmap is exactly proporational
        myHeight=myWidth;
      }
      QPixmap myPalettePixmap(myWidth,myHeight);
      QPainter myQPainter(&myPalettePixmap);

      QImage myQImage = QImage(myWidth,myHeight, 32);
      myQImage.fill(0);
      myQImage.setAlphaBuffer(true);
      myPalettePixmap.fill();

      for( int myIteratorInt = 0; myIteratorInt < myColourCountInt; myIteratorInt++ )
      {
        GDALColorEntry myEntry;
        GDALGetColorEntryAsRGB( myTable, myIteratorInt, &myEntry );
        myQImage.setPixel(myIteratorInt % myWidth,(int)(myIteratorInt/myWidth)
                          ,qRgba(myEntry.c1, myEntry.c2, myEntry.c3, myEntry.c4));
      }
      myQPainter.drawImage(0,0,myQImage);
      return myPalettePixmap; 
    }
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
  else
  {
    //invalid layer  was requested
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
}

void QgsRasterLayer::draw(QPainter * theQPainter, QgsRect * theViewExtent, QgsCoordinateTransform * theQgsCoordinateTransform, QPaintDevice* dst)
{
  //Dont waste time drawing if transparency is at 0 (completely transparent)
  if (transparencyLevelInt == 0)
    return;


  // clip raster extent to view extent
  QgsRect myRasterExtent = theViewExtent->intersect(&layerExtent);
  if (myRasterExtent.isEmpty())
  {
    // nothing to do
    return;
  }
  //
  // The first thing we do is set up the RasterViewPort. This struct stores all the settings
  // relating to the size (in pixels and coordinate system units) of the raster part that is
  // in view in the map window. It also stores the origin.
  //
  //this is not a class level member because every time the user pans or zooms
  //the contents of the rasterViewPort will change
  RasterViewPort *myRasterViewPort = new RasterViewPort();

  // calculate raster pixel offsets from origin to clipped rect
  // we're only interested in positive offsets where the origin of the raster
  // is northwest of the origin of the view
  myRasterViewPort->rectXOffsetInt = static_cast < int >((theViewExtent->xMin() - layerExtent.xMin()) / fabs(adfGeoTransform[1]));
  myRasterViewPort->rectXOffsetInt = myRasterViewPort->rectXOffsetInt >? 0;
  myRasterViewPort->rectYOffsetInt = static_cast < int >((layerExtent.yMax() - theViewExtent->yMax()) / fabs(adfGeoTransform[5]));
  myRasterViewPort->rectYOffsetInt = myRasterViewPort->rectYOffsetInt >? 0;

  //std::cout << "Nodata value for band " << i << " is " << noDataDouble << "\n" << std::endl;
  //std::cout << "myGdalBand->GetOverviewCount(): " << myGdalBand->GetOverviewCount() <<std::endl;

  // get dimensions of clipped raster image in raster pixel space/ RasterIO will do the scaling for us.
  // So for example, if the user is zoomed in a long way, there may only be e.g. 5x5 pixels retrieved from
  // the raw raster data, but rasterio will seamlessly scale the up to whatever the screen coordinats are
  // e.g. a 600x800 display window (see next section below)
  myRasterViewPort->clippedXMinDouble = (myRasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedXMaxDouble = (myRasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
  myRasterViewPort->clippedYMinDouble = (myRasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
  myRasterViewPort->clippedYMaxDouble = (myRasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];
  myRasterViewPort->clippedWidthInt =
      abs(static_cast < int >(myRasterViewPort->clippedXMaxDouble - myRasterViewPort->clippedXMinDouble));
  myRasterViewPort->clippedHeightInt =
      abs(static_cast < int >(myRasterViewPort->clippedYMaxDouble - myRasterViewPort->clippedYMinDouble));
  // make sure we don't exceed size of raster
  myRasterViewPort->clippedWidthInt = myRasterViewPort->clippedWidthInt <? rasterXDimInt;
  myRasterViewPort->clippedHeightInt = myRasterViewPort->clippedHeightInt <? rasterYDimInt;

  // get dimensions of clipped raster image in device coordinate space (this is the size of the viewport)
  myRasterViewPort->topLeftPoint = theQgsCoordinateTransform->transform(myRasterExtent.xMin(), myRasterExtent.yMax());
  myRasterViewPort->bottomRightPoint = theQgsCoordinateTransform->transform(myRasterExtent.xMax(), myRasterExtent.yMin());
  myRasterViewPort->drawableAreaXDimInt = myRasterViewPort->bottomRightPoint.xToInt() - myRasterViewPort->topLeftPoint.xToInt();
  myRasterViewPort->drawableAreaYDimInt = myRasterViewPort->bottomRightPoint.yToInt() - myRasterViewPort->topLeftPoint.yToInt();

  draw(theQPainter,myRasterViewPort);
  delete myRasterViewPort;
}

void QgsRasterLayer::draw (QPainter * theQPainter, RasterViewPort * myRasterViewPort)
{
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //
  switch (drawingStyle)
  {
    // a "Gray" or "Undefined" layer drawn as a range of gray colors
      case SINGLE_BAND_GRAY:
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
            break;
          }
          else
          {
            drawSingleBandGray(theQPainter, myRasterViewPort, getRasterBandNumber(grayBandNameQString));
            break;
          }
          // a "Gray" or "Undefined" layer drawn using a pseudocolor algorithm
      case SINGLE_BAND_PSEUDO_COLOR:
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
            break;
          }
          else
          {
            drawSingleBandPseudoColor(theQPainter, myRasterViewPort, getRasterBandNumber(grayBandNameQString));
            break;
          }
          // a "Palette" layer drawn in gray scale (using only one of the color components)
      case PALETTED_SINGLE_BAND_GRAY:
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
            break;
          }
          else
          {
#ifdef QGISDEBUG
            std::cout << "PALETTED_SINGLE_BAND_GRAY drawing type detected..." << std::endl;
#endif

            int myBandNoInt = 1;
            drawPalettedSingleBandGray(theQPainter, myRasterViewPort, myBandNoInt, grayBandNameQString);

            break;
          }
          // a "Palette" layer having only one of its color components rendered as psuedo color
      case PALETTED_SINGLE_BAND_PSEUDO_COLOR:
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
            break;
          }
          else
          {

            int myBandNoInt = 1;
            drawPalettedSingleBandPseudoColor(theQPainter, myRasterViewPort, myBandNoInt, grayBandNameQString);
            break;
          }
          //a "Palette" image where the bands contains 24bit color info and 8 bits is pulled out per color
      case PALETTED_MULTI_BAND_COLOR:
          drawPalettedMultiBandColor(theQPainter, myRasterViewPort, 1);
          break;
          // a layer containing 2 or more bands, but using only one band to produce a grayscale image
      case MULTI_BAND_SINGLE_BAND_GRAY:
#ifdef QGISDEBUG

          std::cout << "MULTI_BAND_SINGLE_BAND_GRAY drawing type detected..." << std::endl;
#endif
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
#ifdef QGISDEBUG
            std::cout << "MULTI_BAND_SINGLE_BAND_GRAY Not Set detected..." << grayBandNameQString << std::endl;
#endif

            break;
          }
          else
          {

            //get the band number for the mapped gray band
            drawMultiBandSingleBandGray(theQPainter, myRasterViewPort, getRasterBandNumber(grayBandNameQString));
            break;
          }
          //a layer containing 2 or more bands, but using only one band to produce a pseudocolor image
      case MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR:
          //check the band is set!
          if (grayBandNameQString == tr("Not Set"))
          {
            break;
          }
          else
          {

            drawMultiBandSingleBandPseudoColor(theQPainter, myRasterViewPort, getRasterBandNumber(grayBandNameQString));
            break;
          }
          //a layer containing 2 or more bands, mapped to the three RGBcolors.
          //In the case of a multiband with only two bands, one band will have to be mapped to more than one color
      case MULTI_BAND_COLOR:
          drawMultiBandColor(theQPainter, myRasterViewPort);
          break;

      default:
          break;

  }

  //see if debug info is wanted
  if (showDebugOverlayFlag)
  {
    showDebugOverlay(theQPainter, myRasterViewPort);
  };

}                               //end of draw method

void QgsRasterLayer::drawSingleBandGray(QPainter * theQPainter, RasterViewPort * theRasterViewPort, int theBandNoInt)
{
#ifdef QGISDEBUG
  std::cerr << "QgsRasterLayer::drawSingleBandGray called for layer " << theBandNoInt << std::endl;
#endif
  //create the outout image that the layer will be drawn on before placing it in the qcanvas
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  // QPixmap * myQPixmap=new QPixmap(theRasterViewPort->drawableAreaXDimInt,theRasterViewPort->drawableAreaYDimInt);

  // read entire clipped area of raster band
  // treat myGdalScanData as a pseudo-multidimensional array
  // RasterIO() takes care of scaling down image
  uint *myGdalScanData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read, theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalScanData,
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);

  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  //std::cout << "Colour Interpretation for this band is : " << myColorInterpretation << std::endl;

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
#ifdef QGISDEBUG

  std::cout << "draw gray band Retrieving stats" << std::endl;
#endif

  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
#ifdef QGISDEBUG

  std::cout << "draw gray band Git stats" << std::endl;
#endif

  double myRangeDouble = myRasterBandStats.rangeDouble;
  // print each point in myGdalScanData with equal parts R, G ,B o make it show as gray
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      int myGrayValInt = myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt];
      //remove these lines!
      //if (myColumnInt==0)
      //std::cout << "Checking if " << myGrayValInt << " = " << noDataValueDouble << std::endl;

      //dont draw this point if it is no data !
      //gdal should return -9999 when a cell is null, but it seems to return 0 rather
      //when this is resolved the first clause below should be removed.
      if ((myGrayValInt != 0) && (myGrayValInt != noDataValueDouble))
      {
        // We need to make sure the values are 0-255
        myGrayValInt = static_cast < int >((255 / myRangeDouble) * myGrayValInt);

        if (invertHistogramFlag)
        {
          myGrayValInt = 255 - myGrayValInt;
        }
        myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValInt, myGrayValInt, myGrayValInt, transparencyLevelInt));
      }
      else                //render no data as 100% transparent
      {
        //0 alpha = completely transparent
        //myQImage.setPixel( myRowInt, myColumnInt, qRgba( 255, 127, 255, 127 ));
      }
    }
  }
  //render any inline filters
  filterLayer(&myQImage);
  //part of the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);
}


void QgsRasterLayer::drawSingleBandPseudoColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort, int theBandNoInt)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawSingleBandPseudoColor called" << std::endl;
#endif

  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  //create the outout image that the layer will be drawn on before placing it in the qcanvas

  // read entire clipped area of raster band
  // treat myGdalScanData as a pseudo-multidimensional array
  // RasterIO() takes care of scaling down image
  uint *myGdalScanData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read, theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalScanData,
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);

  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  //std::cout << "Colour Interpretation for this band is : " << myColorInterpretation << std::endl;

  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  double myRangeDouble = myRasterBandStats.rangeDouble;
  int myRedInt = 0;
  int myGreenInt = 0;
  int myBlueInt = 0;
  //calculate the adjusted matrix stats - which come into affect if the user has chosen
  RasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);
  //to histogram stretch to a given number of std deviations
  //see if we are using histogram stretch using stddev and plot only within the selected number of deviations if we are
  //cout << "stdDevsToPlotDouble: " << cboStdDev->currentText() << " converted to " << stdDevsToPlotDouble << endl;
  if (stdDevsToPlotDouble > 0)
  {
    //work out how far on either side of the mean we should include data
    float myTotalDeviationDouble = stdDevsToPlotDouble * myAdjustedRasterBandStats.stdDevDouble;
    //printf("myTotalDeviationDouble: %i\n" , myTotalDeviationDouble );
    //adjust min and max accordingly
    //only change min if it is less than mean  -  (n  x  deviations)
    if (noDataValueDouble < (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble))
    {
      noDataValueDouble = (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble);
      //cout << "Adjusting minValDouble to: " << noDataValueDouble << endl;
    }
    //only change max if it is greater than mean  +  (n  x  deviations)
    if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble))
    {
      myAdjustedRasterBandStats.maxValDouble = (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble);
      //cout << "Adjusting maxValDouble to: " << myAdjustedRasterBandStats.maxValDouble << endl;
    }
    //update the range
    myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble - noDataValueDouble;
  }
  //set up the three class breaks for pseudocolour mapping
  double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
  double myClassBreakMin1 = myRasterBandStats.minValDouble;
  double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
  double myClassBreakMin2 = myClassBreakMax1;
  double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
  double myClassBreakMin3 = myClassBreakMax2;
  double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;


  myQImage.setAlphaBuffer(true);

  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      //hardcoding to white to start with
      myRedInt = 255;
      myBlueInt = 255;
      myGreenInt = 255;
      int myInt = myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt];
      // draw this point if it is not  no_data !
      //gdal should return -9999 when a cell is null, but it seems to return 0 rather
      if ((myInt != noDataValueDouble) && (myInt != 0)) //should not need the second clause!
      {
        //double check that myInt >= min and <= max
        //this is relevant if we are plotting within stddevs
        if (myInt < noDataValueDouble)
        {
          myInt = static_cast < int >(noDataValueDouble);
        }
        if (myInt > myAdjustedRasterBandStats.maxValDouble)
        {
          myInt = static_cast < int >(myAdjustedRasterBandStats.maxValDouble);
        }
        if (!invertHistogramFlag)
        {
          //check if we are in the first class break
          if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
          {
            //std::cout << "Class break 1 value : " << myInt << endl;
            myRedInt = 0;
            myBlueInt = 255;
            myGreenInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * (myInt - myClassBreakMin1)) * 3);
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
          }
          //check if we are in the second class break
          else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
          {
            //std::cout << "Class break 2 value : " << myInt << endl;
            myRedInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3);
            myBlueInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3));
            myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
          }
          //otherwise we must be in the third classbreak
          else
          {
            //std::cout << "Class break 3 value : " << myInt << endl;
            myRedInt = 255;
            myBlueInt = 0;
            myGreenInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin3) / 1) * 3)));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=myGreenInt;
              myGreenInt=255-myGreenInt;
            }
          }
        }
        else            //invert histogram toggle is on
        {
          //check if we are in the first class break
          if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
          {
            myRedInt = 255;
            myBlueInt = 0;
            myGreenInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin1) / 1) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
          }
          //check if we are in the second class break
          else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
          {
            myRedInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3));
            myBlueInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3);
            myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
          }
          //otherwise we must be in the third classbreak
          else
          {
            myRedInt = 0;
            myBlueInt = 255;
            myGreenInt =
                static_cast < int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * (myInt - myClassBreakMin3)) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=myGreenInt;
              myGreenInt=255-myGreenInt;
            }
          }
        }
        myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, transparencyLevelInt));
      }                   //end of nodata=false check
      else                  //nodata so draw transparent
      {
        //0 alpha = completely transparent
        //myQImage.setPixel( myRowInt, myColumnInt, qRgba( 255, 255, 255, 0 ));
      }
    }                       //end of columnwise loop
  }                           //end of towwise loop
  //render any inline filters
  filterLayer(&myQImage);
  //draw with the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);
}

/**
 * This method is used to render a paletted raster layer as a gray image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
 */
void QgsRasterLayer::drawPalettedSingleBandGray(QPainter * theQPainter,
        RasterViewPort * theRasterViewPort, int theBandNoInt, QString theColorQString)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedSingleBandGray called" << std::endl;
#endif
  // read entire clipped area of raster band
  // treat myGdalScanData as a pseudo-multidimensional array
  // RasterIO() takes care of scaling down image
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  uint *myGdalScanData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read, theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalScanData,
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);

  // print each point in myGdalScanData using color looked up in color table
  GDALColorTable *colorTable = myGdalBand->GetColorTable();
  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);



  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      const GDALColorEntry *myColorEntry =
          GDALGetColorEntry(colorTable, myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt]);
      //dont draw this point if it is no data !
      if (noDataValueDouble != myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt])
      {
        int myGrayValueInt = 0; //color 1 int
        //check colorEntry is valid
        if (myColorEntry != NULL)
        {
          //check for alternate color mappings
          if (theColorQString == redTranslatedQString)
          {
            myGrayValueInt = myColorEntry->c1;
          }
          if (theColorQString == greenTranslatedQString)
          {
            myGrayValueInt = myColorEntry->c2;
          }
          if (theColorQString == blueTranslatedQString)
          {
            myGrayValueInt = myColorEntry->c3;
          }
        }
        else
        {
          //there is no guarantee that there will be a matching palette entry for
          //every cell in the raster. If there is no match, do nothing.
          myGrayValueInt = 0;
        }
        if (invertHistogramFlag)
        {
          myGrayValueInt = 255 - myGrayValueInt;
        }
        //set the pixel based on the above color mappings
        myQImage.setPixel(myRowInt, myColumnInt, qRgba(myGrayValueInt, myGrayValueInt, myGrayValueInt, transparencyLevelInt));
      }
    }
  }
  //render any inline filters
  filterLayer(&myQImage);
  //part of the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);
}


/**
 * This method is used to render a paletted raster layer as a pseudocolor image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 * @param theColorQString - QString containing either 'Red' 'Green' or 'Blue' indicating which part of the rgb triplet will be used to render gray.
 */
void QgsRasterLayer::drawPalettedSingleBandPseudoColor(QPainter * theQPainter,
        RasterViewPort * theRasterViewPort, int theBandNoInt, QString theColorQString)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedSingleBandPseudoColor called" << std::endl;
#endif
  // read entire clipped area of raster band
  // treat myGdalScanData as a pseudo-multidimensional array
  // RasterIO() takes care of scaling down image
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1); //always one!
  uint *myGdalScanData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read, theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalScanData,
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);

  // print each point in myGdalScanData using color looked up in color table
  GDALColorTable *colorTable = myGdalBand->GetColorTable();
  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);


  RasterBandStats myRasterBandStats = getRasterBandStats(theBandNoInt);
  double myRangeDouble = myRasterBandStats.rangeDouble;
  int myRedInt = 0;
  int myGreenInt = 0;
  int myBlueInt = 0;
  //calculate the adjusted matrix stats - which come into affect if the user has chosen
  RasterBandStats myAdjustedRasterBandStats = getRasterBandStats(theBandNoInt);

  //to histogram stretch to a given number of std deviations
  //see if we are using histogram stretch using stddev and plot only within the selected number of deviations if we are
  //cout << "stdDevsToPlotDouble: " << cboStdDev->currentText() << " converted to " << stdDevsToPlotDouble << endl;
  if (stdDevsToPlotDouble > 0)
  {
    //work out how far on either side of the mean we should include data
    float myTotalDeviationDouble = stdDevsToPlotDouble * myAdjustedRasterBandStats.stdDevDouble;
    //printf("myTotalDeviationDouble: %i\n" , myTotalDeviationDouble );
    //adjust min and max accordingly
    //only change min if it is less than mean  -  (n  x  deviations)
    if (noDataValueDouble < (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble))
    {
      noDataValueDouble = (myAdjustedRasterBandStats.meanDouble - myTotalDeviationDouble);
      //cout << "Adjusting minValDouble to: " << noDataValueDouble << endl;
    }
    //only change max if it is greater than mean  +  (n  x  deviations)
    if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble))
    {
      myAdjustedRasterBandStats.maxValDouble = (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble);
      //cout << "Adjusting maxValDouble to: " << myAdjustedRasterBandStats.maxValDouble << endl;
    }
    //update the range
    myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble - noDataValueDouble;
  }
  //set up the three class breaks for pseudocolour mapping
  double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
  double myClassBreakMin1 = myRasterBandStats.minValDouble;
  double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
  double myClassBreakMin2 = myClassBreakMax1;
  double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
  double myClassBreakMin3 = myClassBreakMax2;
  double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;



  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      int myInt = 0;
      const GDALColorEntry *myColorEntry =
          GDALGetColorEntry(colorTable, myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt]);
      //check colorEntry is valid
      if (myColorEntry != NULL)
      {
        //check for alternate color mappings
        if (theColorQString == redTranslatedQString)
        {
          myInt = myColorEntry->c1;
        }
        if (theColorQString == ("Green"))
        {
          myInt = myColorEntry->c2;
        }
        if (theColorQString == ("Blue"))
        {
          myInt = myColorEntry->c3;
        }
        //dont draw this point if it is no data !
        //double check that myInt >= min and <= max
        //this is relevant if we are plotting within stddevs
        if ((myInt < noDataValueDouble) && (myInt != noDataValueDouble))
        {
          myInt = static_cast < int >(noDataValueDouble);
        }
        if ((myInt > myAdjustedRasterBandStats.maxValDouble) && (myInt != noDataValueDouble))
        {
          myInt = static_cast < int >(myAdjustedRasterBandStats.maxValDouble);
        }
        if (myInt == noDataValueDouble)
        {
          //hardcoding to white for now
          myRedInt = 255;
          myBlueInt = 255;
          myGreenInt = 255;
        }
        else if (!invertHistogramFlag)
        {
          //check if we are in the first class break
          if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
          {
            myRedInt = 0;
            myBlueInt = 255;
            myGreenInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * (myInt - myClassBreakMin1)) * 3);
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
          }
          //check if we are in the second class break
          else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
          {
            myRedInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3);
            myBlueInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3));
            myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
          }
          //otherwise we must be in the third classbreak
          else
          {
            myRedInt = 255;
            myBlueInt = 0;
            myGreenInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin3) / 1) * 3)));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=myGreenInt;
              myGreenInt=255-myGreenInt;
            }
          }
        }
        else            //invert histogram toggle is on
        {
          //check if we are in the first class break
          if ((myInt >= myClassBreakMin1) && (myInt < myClassBreakMax1))
          {
            myRedInt = 255;
            myBlueInt = 0;
            myGreenInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin1) / 1) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
            }
          }
          //check if we are in the second class break
          else if ((myInt >= myClassBreakMin2) && (myInt < myClassBreakMax2))
          {
            myRedInt =
                static_cast <
                int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3));
            myBlueInt =
                static_cast < int >(((255 / myAdjustedRasterBandStats.rangeDouble) * ((myInt - myClassBreakMin2) / 1)) * 3);
            myGreenInt = 255;
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myGreenInt=myBlueInt;
            }
          }
          //otherwise we must be in the third classbreak
          else
          {
            myRedInt = 0;
            myBlueInt = 255;
            myGreenInt =
                static_cast < int >(255 - (((255 / myAdjustedRasterBandStats.rangeDouble) * (myInt - myClassBreakMin3)) * 3));
            // testing this stuff still ...
            if (colorRampingType==FREAK_OUT)
            {
              myRedInt=255-myGreenInt;
              myGreenInt=myGreenInt;
            }
          }


        }
      }                   //end of color palette is null check
      else
      {
        //there is no guarantee that there will be a matching palette entry for
        //every cell in the raster. If there is no match, do nothing.
        myRedInt = 255;
        myBlueInt = 255;
        myGreenInt = 255;
      }
      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedInt, myGreenInt, myBlueInt, transparencyLevelInt));
    }
  }

  //render any inline filters
  filterLayer(&myQImage);
  //part of the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);
}

/**
 * This method is used to render a paletted raster layer as a colour image.
 * @param theQPainter - pointer to the QPainter onto which the layer should be drawn.
 * @param theRasterViewPort - pointer to the ViewPort struct containing dimensions of viewable area and subset area to be extracted from data file.
 * @param theGdalBand - pointer to the GDALRasterBand which should be rendered.
 */
void QgsRasterLayer::drawPalettedMultiBandColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort, int theBandNoInt)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawPalettedMultiBandColor called" << std::endl;
#endif

  // read entire clipped area of raster band
  // treat myGdalScanData as a pseudo-multidimensional array
  // RasterIO() takes care of scaling down image
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
  uint *myGdalScanData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read,
          theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalScanData,
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);

  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  //std::cout << "Colour Interpretation for this band is : " << myColorInterpretation << std::endl;

  // print each point in myGdalScanData using color looked up in color table
  GDALColorTable *colorTable = myGdalBand->GetColorTable();
  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      const GDALColorEntry *colorEntry =
          GDALGetColorEntry(colorTable, myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt]);
      //dont draw this point if it is no data !
      if (noDataValueDouble != myGdalScanData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt])
      {
        int myRedValueInt = 0;  //color 1 int
        int myGreenValueInt = 0;  //color 2 int
        int myBlueValueInt = 0; //color 3 int
        //check colorEntry is valid
        if (colorEntry != NULL)
        {
          //check for alternate color mappings
          if (redBandNameQString == redTranslatedQString)
            myRedValueInt = colorEntry->c1;
          if (redBandNameQString == greenTranslatedQString)
            myRedValueInt = colorEntry->c2;
          if (redBandNameQString == blueTranslatedQString)
            myRedValueInt = colorEntry->c3;
          if (greenBandNameQString == redTranslatedQString)
            myGreenValueInt = colorEntry->c1;
          if (greenBandNameQString == greenTranslatedQString)
            myGreenValueInt = colorEntry->c2;
          if (greenBandNameQString == blueTranslatedQString)
            myGreenValueInt = colorEntry->c3;
          if (blueBandNameQString == redTranslatedQString)
            myBlueValueInt = colorEntry->c1;
          if (blueBandNameQString == greenTranslatedQString)
            myBlueValueInt = colorEntry->c2;
          if (blueBandNameQString == blueTranslatedQString)
            myBlueValueInt = colorEntry->c3;
        }
        else
        {
          //there is no guarantee that there will be a matching palette entry for
          //every cell in the raster. If there is no match, do nothing.
        }
        if (invertHistogramFlag)
        {
          myRedValueInt = 255 - myRedValueInt;
          myGreenValueInt = 255 - myGreenValueInt;
          myBlueValueInt = 255 - myBlueValueInt;

        }
        //set the pixel based on the above color mappings
        myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, transparencyLevelInt));
        //old method for painting directly to canvas with no alpha rendering
        //theQPainter->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
        //theQPainter->drawPoint(myTopLeftPoint.xToInt() + myRowInt, myTopLeftPoint.yToInt() + myColumnInt);
      }
    }
  }
  //render any inline filters
  filterLayer(&myQImage);
  //part of the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);

}
void QgsRasterLayer::drawMultiBandSingleBandGray(QPainter * theQPainter, RasterViewPort * theRasterViewPort, int theBandNoInt)
{
  //delegate to drawSingleBandGray!
  drawSingleBandGray(theQPainter, theRasterViewPort, theBandNoInt);
}

void QgsRasterLayer::drawMultiBandSingleBandPseudoColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort, int theBandNoInt)
{
  //delegate to drawSinglePseudocolor!
  drawSingleBandPseudoColor(theQPainter, theRasterViewPort, theBandNoInt);
}

void QgsRasterLayer::drawMultiBandColor(QPainter * theQPainter, RasterViewPort * theRasterViewPort)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::drawMultiBandColor called" << std::endl;
#endif

  int myRedBandNoInt = getRasterBandNumber(redBandNameQString);
  int myGreenBandNoInt = getRasterBandNumber(greenBandNameQString);
  int myBlueBandNoInt = getRasterBandNumber(blueBandNameQString);
  GDALRasterBand *myGdalRedBand = gdalDataset->GetRasterBand(myRedBandNoInt);
  GDALRasterBand *myGdalGreenBand = gdalDataset->GetRasterBand(myGreenBandNoInt);
  GDALRasterBand *myGdalBlueBand = gdalDataset->GetRasterBand(myBlueBandNoInt);
  //
  // I dont know how much overhead there is in opening three matricies at the same time....
  // but this is a replacement of the old method implemented by Steve which xored the color components
  // into the pixels
  // RasterIO() takes care of scaling down image
  uint *myGdalRedData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  uint *myGdalGreenData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);
  uint *myGdalBlueData =
      (uint *) CPLMalloc(sizeof(uint) * theRasterViewPort->drawableAreaXDimInt * sizeof(uint) * theRasterViewPort->drawableAreaYDimInt);

  CPLErr myRedCPLerr = myGdalRedBand->RasterIO(GF_Read,
          theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalRedData, // <----- Red Layer
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);
  CPLErr myGreenCPLerr = myGdalGreenBand->RasterIO(GF_Read,
          theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalGreenData, // <----- Green Layer
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);
  CPLErr myBlueCPLerr = myGdalBlueBand->RasterIO(GF_Read,
          theRasterViewPort->rectXOffsetInt,
          theRasterViewPort->rectYOffsetInt,
          theRasterViewPort->clippedWidthInt,
          theRasterViewPort->clippedHeightInt,
          myGdalBlueData,  // <----- Blue Layer
          theRasterViewPort->drawableAreaXDimInt,
          theRasterViewPort->drawableAreaYDimInt,
          GDT_UInt32, 0, 0);
  //std::cout << "Colour Interpretation for this band is : " << myColorInterpretation << std::endl;
  int myRedInt, myGreenInt, myBlueInt;
  QImage myQImage = QImage(theRasterViewPort->drawableAreaXDimInt, theRasterViewPort->drawableAreaYDimInt, 32);
  myQImage.fill(0);
  myQImage.setAlphaBuffer(true);
  for (int myColumnInt = 0; myColumnInt < theRasterViewPort->drawableAreaYDimInt; ++myColumnInt)
  {
    for (int myRowInt = 0; myRowInt < theRasterViewPort->drawableAreaXDimInt; ++myRowInt)
    {
      //pull the rgb values from each band
      int myRedValueInt = myGdalRedData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt];
      int myGreenValueInt = myGdalGreenData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt];
      int myBlueValueInt = myGdalBlueData[myColumnInt * theRasterViewPort->drawableAreaXDimInt + myRowInt];

      //TODO check for nodata values
      if (invertHistogramFlag)
      {
        myRedValueInt = 255 - myRedValueInt;
        myGreenValueInt = 255 - myGreenValueInt;
        myBlueValueInt = 255 - myBlueValueInt;
      }
      //set the pixel based on the above color mappings
      myQImage.setPixel(myRowInt, myColumnInt, qRgba(myRedValueInt, myGreenValueInt, myBlueValueInt, transparencyLevelInt));
    }
  }
  //render any inline filters
  filterLayer(&myQImage);
  //part of the experimental transaparency support
  theQPainter->drawImage(theRasterViewPort->topLeftPoint.xToInt(), theRasterViewPort->topLeftPoint.yToInt(), myQImage);

  //free the scanline memory
  CPLFree(myGdalRedData);
  CPLFree(myGdalGreenData);
  CPLFree(myGdalBlueData);
}

/**
 * Call any inline filters
 */
void QgsRasterLayer::filterLayer(QImage * theQImage)
{
  //do stuff here....
  return;
}

/**
  Print some debug info to the qpainter
  */

void QgsRasterLayer::showDebugOverlay(QPainter * theQPainter, RasterViewPort * theRasterViewPort)
{


  QFont myQFont("arial", 10, QFont::Bold);
  theQPainter->setFont(myQFont);
  theQPainter->setPen(Qt::black);
  QBrush myQBrush(qRgba(128, 128, 164, 50), Dense6Pattern); //semi transparent
  theQPainter->setBrush(myQBrush);  // set the yellow brush
  theQPainter->drawRect(5, 5, theQPainter->window().width() - 10, 60);
  theQPainter->setBrush(NoBrush); // do not fill

  theQPainter->drawText(10, 20, "QPainter: "
          + QString::number(theQPainter->window().width()) + " x " + QString::number(theQPainter->window().height()));
  theQPainter->drawText(10, 32, tr("Raster Extent: ")
          + QString::number(theRasterViewPort->drawableAreaXDimInt)
          + "," + QString::number(theRasterViewPort->drawableAreaYDimInt));
  theQPainter->drawText(10, 44, tr("Clipped area: ")
          + QString::number(theRasterViewPort->clippedXMinDouble)
          + "," + QString::number(theRasterViewPort->clippedYMinDouble)
          + " - " + QString::number(theRasterViewPort->clippedXMaxDouble)
          + "," + QString::number(theRasterViewPort->clippedYMinDouble));

  return;


}                               //end of main draw method



/** Return the statistics for a given band name.
  WARDNING::: THERE IS NO GUARANTEE THAT BAND NAMES ARE UNIQE
  THE FIRST MATCH WILL BE RETURNED!!!!!!!!!!!!
  */
const RasterBandStats QgsRasterLayer::getRasterBandStats(QString theBandNameQString)
{

  //we cant use a vector iterator because the iterator is astruct not a class
  //and the qvector model does not like this.
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++)
  {
    RasterBandStats myRasterBandStats = getRasterBandStats(i);
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      return myRasterBandStats;
    }
  }



}

//get the number of a band given its name
//note this should be the rewritten name set up in the constructor,
//not the name retrieved directly from gdal!
//if no matching band is found zero will be returned!
const int QgsRasterLayer::getRasterBandNumber(QString theBandNameQString)
{
  for (int myIteratorInt = 0; myIteratorInt <= rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
#ifdef QGISDEBUG

    std::cout << "myRasterBandStats.bandName: " << myRasterBandStats.bandName << "  :: theBandNameQString: " << theBandNameQString << std::endl;
#endif

    if (myRasterBandStats.bandName == theBandNameQString)
    {
#ifdef QGISDEBUG
      std::cerr << "********** band " << myRasterBandStats.bandNoInt << " was found in getRasterBandNumber " << theBandNameQString << std::endl;
#endif

      return myRasterBandStats.bandNoInt;
    }
  }
#ifdef QGISDEBUG
  std::cerr << "********** no band was found in getRasterBandNumber " << theBandNameQString << std::endl;
#endif

  return 0;                     //no band was found
}

// get the name of a band given its number
const QString QgsRasterLayer::getRasterBandName(int theBandNoInt)
{

  if (theBandNoInt <= rasterStatsVector.size())
  {
    //vector starts at base 0, band counts at base1 !
    return rasterStatsVector[theBandNoInt - 1].bandName;
  }
  else
  {
    return QString("");
  }
}

/** Check whether a given band number has stats associated with it */
const bool QgsRasterLayer::hasStats(int theBandNoInt)
{
  if (theBandNoInt <= rasterStatsVector.size())
  {
    //vector starts at base 0, band counts at base1 !
    return rasterStatsVector[theBandNoInt - 1].statsGatheredFlag;
  }
  else
  {
    return false;
  }
}
/** Private method to calculate statistics for a band. Populates rasterStatsMemArray.
    Note that this is a cpu intensive /slow task!*/
const RasterBandStats QgsRasterLayer::getRasterBandStats(int theBandNoInt)
{
emit setStatus(QString("Calculating stats for ")+layerName);
//reset the main app progress bar
emit setProgress(0,0);
#ifdef QGISDEBUG
    std::cout << "QgsRasterLayer::calculate stats for band " << theBandNoInt << std::endl;
#endif
    //check if we have received a valid band number
    if ((gdalDataset->GetRasterCount() < theBandNoInt) && rasterLayerType != PALETTE)
    {
        //invalid band id, return nothing
        RasterBandStats myNullReturnStats;
        return myNullReturnStats;
    }
    if (rasterLayerType == PALETTE && (theBandNoInt > 3))
    {
        //invalid band id, return nothing
        RasterBandStats myNullReturnStats;
        return myNullReturnStats;
    }
    //check if we have previously gathered stats for this band...

    RasterBandStats myRasterBandStats = rasterStatsVector[theBandNoInt - 1];
    myRasterBandStats.bandNoInt = theBandNoInt;
    //dont bother with this if we already have stats
    if (myRasterBandStats.statsGatheredFlag)
    {
        return myRasterBandStats;
    }
    GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(theBandNoInt);
    QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());

    //declare a colorTable to hold a palette - will only be used if the layer color interp is palette
    GDALColorTable *colorTable;
    if (rasterLayerType == PALETTE)
    {
        //get the palette colour table
        colorTable = myGdalBand->GetColorTable();
        //override the band name - palette images are really only one band
        //so we are faking three band behaviour
        switch (theBandNoInt)
        {
            // a "Red" layer
        case 1:
            myRasterBandStats.bandName = redTranslatedQString;
            break;
        case 2:
            myRasterBandStats.bandName = blueTranslatedQString;
            break;
        case 3:
            myRasterBandStats.bandName = greenTranslatedQString;
            break;
        default:
            //invalid band id so return
            RasterBandStats myNullReturnStats;
            return myNullReturnStats;
            break;
        }
    }
    else if (rasterLayerType==GRAY_OR_UNDEFINED)
    {
        myRasterBandStats.bandName = myColorInterpretation;
    }
    else //rasterLayerType is MULTIBAND
    {
        //do nothing
    }
    myRasterBandStats.elementCountInt = rasterXDimInt * rasterYDimInt;

    //allocate a buffer to hold one row of ints
    int myAllocationSizeInt = sizeof(uint) * rasterXDimInt;
    uint *myScanlineAllocInt = (uint *) CPLMalloc(myAllocationSizeInt);
    bool myFirstIterationFlag = true;
    //unfortunately we need to make two passes through the data to calculate stddev
    for (int myCurrentRowInt = 0; myCurrentRowInt < rasterYDimInt; myCurrentRowInt++)
    {
       //we loop through the dataset twice for stats so ydim is doubled!
       emit setProgress(myCurrentRowInt,rasterYDimInt*2);
        CPLErr myResult =
            myGdalBand->RasterIO(GF_Read, 0, myCurrentRowInt, rasterXDimInt, 1, myScanlineAllocInt, rasterXDimInt, 1, GDT_UInt32, 0, 0);
        for (int myCurrentColInt = 0; myCurrentColInt < rasterXDimInt; myCurrentColInt++)
        {
	
            double myDouble = 0;
            //get the nth element from the current row
            if (myColorInterpretation != "Palette") //dont translate this its a gdal string
            {
                myDouble = myScanlineAllocInt[myCurrentColInt];
            } else
            {
                //this is a palette layer so red / green / blue 'layers are 'virtual'
                //in that we need to obtain the palette entry and then get the r,g or g
                //component from that palette entry
                const GDALColorEntry *myColorEntry = GDALGetColorEntry(colorTable, myScanlineAllocInt[myCurrentColInt]);
                //check colorEntry is valid
                if (myColorEntry != NULL)
                {
                    //check for alternate color mappings
                    if (theBandNoInt == 1)  //"Red"
                    {
                        myDouble = static_cast < double >(myColorEntry->c1);
                    }
                    if (theBandNoInt == 2)  //"Green"
                    {
                        myDouble = static_cast < double >(myColorEntry->c2);
                    }
                    if (theBandNoInt == 3)  //"Blue"
                    {
                        myDouble = static_cast < double >(myColorEntry->c3);
                    }
                }


            }
            //only use this element if we have a non null element
            if (myDouble != noDataValueDouble)
            {
                if (myFirstIterationFlag)
                {
                    //this is the first iteration so initialise vars
                    myFirstIterationFlag = false;
                    myRasterBandStats.minValDouble = myDouble;
                    myRasterBandStats.maxValDouble = myDouble;
                }               //end of true part for first iteration check
                else
                {
                    //this is done for all subsequent iterations
                    if (myDouble < myRasterBandStats.minValDouble)
                    {
                        myRasterBandStats.minValDouble = myDouble;
                    }
                    if (myDouble > myRasterBandStats.maxValDouble)
                    {
                        myRasterBandStats.maxValDouble = myDouble;
                    }
                    //only increment the running total if it is not a nodata value
                    if (myDouble != noDataValueDouble)
                    {
                        myRasterBandStats.sumDouble += myDouble;
                        ++myRasterBandStats.elementCountInt;
                    }
                }               //end of false part for first iteration check
            }                   //end of nodata chec
        }                       //end of column wise loop
    }                           //end of row wise loop
    //
    //end of first pass through data now calculate the range
    myRasterBandStats.rangeDouble = myRasterBandStats.maxValDouble - myRasterBandStats.minValDouble;
    //calculate the mean
    myRasterBandStats.meanDouble = myRasterBandStats.sumDouble / myRasterBandStats.elementCountInt;
    //for the second pass we will get the sum of the squares / mean
    for (int myCurrentRowInt = 0; myCurrentRowInt < rasterYDimInt; myCurrentRowInt++)
    {
        //we loop through the dataset twice for stats so ydim is doubled (this is loop2)!
        emit setProgress(myCurrentRowInt+rasterYDimInt,rasterYDimInt*2);
        CPLErr myResult =
            myGdalBand->RasterIO(GF_Read, 0, myCurrentRowInt, rasterXDimInt, 1, myScanlineAllocInt, rasterXDimInt, 1, GDT_UInt32, 0, 0);
        for (int myCurrentColInt = 0; myCurrentColInt < rasterXDimInt; myCurrentColInt++)
        {
            double myDouble = 0;
            //get the nth element from the current row
            if (myColorInterpretation != "Palette") //dont translate this its a gdal string
            {
                myDouble = myScanlineAllocInt[myCurrentColInt];
            } else
            {
                //this is a palette layer so red / green / blue 'layers are 'virtual'
                //in that we need to obtain the palette entry and then get the r,g or g
                //component from that palette entry
                const GDALColorEntry *myColorEntry = GDALGetColorEntry(colorTable, myScanlineAllocInt[myCurrentColInt]);
                //check colorEntry is valid
                if (myColorEntry != NULL)
                {
                    //check for alternate color mappings
                    if (theBandNoInt == 1)  //red
                    {
                        myDouble = myColorEntry->c1;
                    }
                    if (theBandNoInt == 1)  //green
                    {
                        myDouble = myColorEntry->c2;
                    }
                    if (theBandNoInt == 3)  //blue
                    {
                        myDouble = myColorEntry->c3;
                    }
                }


            }
            myRasterBandStats.sumSqrDevDouble += static_cast < double >(pow(myDouble - myRasterBandStats.meanDouble, 2));
        }                       //end of column wise loop
    }                           //end of row wise loop
    //divide result by sample size - 1 and get square root to get stdev
    myRasterBandStats.stdDevDouble = static_cast < double >(sqrt(myRasterBandStats.sumSqrDevDouble /
                                     (myRasterBandStats.elementCountInt - 1)));
    CPLFree(myScanlineAllocInt);
    myRasterBandStats.statsGatheredFlag = true;
    //add this band to the class stats map
    rasterStatsVector[theBandNoInt - 1] = myRasterBandStats;
    emit setProgress(rasterYDimInt, rasterYDimInt); //reset progress
    return myRasterBandStats;
}                               //end of getRasterBandStats


//mutator for red band name (allows alternate mappings e.g. map blue as red colour)
void QgsRasterLayer::setRedBandName(QString theBandNameQString)
{
#ifdef QGISDEBUG
  std::cout << "setRedBandName :  " << theBandNameQString << std::endl;
#endif
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    redBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      redBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  redBandNameQString = tr("Not Set");
  return;
}

//mutator for green band name
void QgsRasterLayer::setGreenBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    greenBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      greenBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  greenBandNameQString = tr("Not Set");
  return;
}

//mutator for blue band name
void QgsRasterLayer::setBlueBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    blueBandNameQString = theBandNameQString;
    return;
  }
  //check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      blueBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  blueBandNameQString = tr("Not Set");
  return;
}

//mutator for gray band name
void QgsRasterLayer::setGrayBandName(QString theBandNameQString)
{
  //check if the band is unset
  if (theBandNameQString == tr("Not Set"))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //check if the image is paletted
  if (rasterLayerType == PALETTE && (theBandNameQString == redTranslatedQString || theBandNameQString == greenTranslatedQString || theBandNameQString == blueTranslatedQString))
  {
    grayBandNameQString = theBandNameQString;
    return;
  }
  //otherwise check that a valid band name was passed

  for (int myIteratorInt = 0; myIteratorInt < rasterStatsVector.size(); ++myIteratorInt)
  {
    //find out the name of this band
    RasterBandStats myRasterBandStats = rasterStatsVector[myIteratorInt];
    if (myRasterBandStats.bandName == theBandNameQString)
    {
      grayBandNameQString = theBandNameQString;
      return;
    }
  }

  //if no matches were found default to not set
  grayBandNameQString = tr("Not Set");
  return;
}

/** Return a pixmap representing a legend image. This is an overloaded
 * version of the method below and assumes false for the legend name flag.
 */
QPixmap QgsRasterLayer::getLegendQPixmap()
{
  return getLegendQPixmap(false);
}

/** Return a pixmap representing a legend image
 * @param theWithNameFlag - boolena flag whether to overlay the legend name in the text
 */
QPixmap QgsRasterLayer::getLegendQPixmap(bool theWithNameFlag)
{
#ifdef QGISDEBUG
  std::cout << "QgsRasterLayer::getLegendQPixmap called (" << getDrawingStyleAsQString() << ")" << std::endl;
#endif
  //
  // Get the adjusted matrix stats
  //
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
  double noDataDouble = myGdalBand->GetNoDataValue();
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  QPixmap myLegendQPixmap;      //will be initialised once we know what drawing style is active
  QPainter myQPainter;
  //
  // Create the legend pixmap - note it is generated on the preadjusted stats
  //
  if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
  {

    myLegendQPixmap = QPixmap(100, 1);
    myQPainter.begin(&myLegendQPixmap);
    int myPosInt = 0;
    for (double myDouble = 0; myDouble < 255; myDouble += 2.55)
    {
      if (!invertHistogramFlag) //histogram is not inverted
      {
        //draw legend as grayscale
        int myGrayInt = static_cast < int >(myDouble);
        myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
      } else                //histogram is inverted
      {
        //draw legend as inverted grayscale
        int myGrayInt = 255 - static_cast < int >(myDouble);
        myQPainter.setPen(QPen(QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0));
      }                   //end of invert histogram  check
      myQPainter.drawPoint(myPosInt++, 0);
    }
  }                           //end of gray check
  else if (drawingStyle == MULTI_BAND_SINGLE_BAND_PSEUDO_COLOR ||
          drawingStyle == PALETTED_SINGLE_BAND_PSEUDO_COLOR || drawingStyle == SINGLE_BAND_PSEUDO_COLOR)
  {

    //set up the three class breaks for pseudocolour mapping
    double myRangeSizeDouble = 90;  //hard coded for now
    double myBreakSizeDouble = myRangeSizeDouble / 3;
    double myClassBreakMin1 = 0;
    double myClassBreakMax1 = myClassBreakMin1 + myBreakSizeDouble;
    double myClassBreakMin2 = myClassBreakMax1;
    double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
    double myClassBreakMin3 = myClassBreakMax2;
    double myClassBreakMax3 = myClassBreakMin3 + myBreakSizeDouble;

    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    myLegendQPixmap = QPixmap(100, 1);
    myQPainter.begin(&myLegendQPixmap);
    int myPosInt = 0;
    for (double myDouble = 0; myDouble < myRangeSizeDouble; myDouble += myRangeSizeDouble / 100.0)
    {
      //draw pseudocolor legend
      if (!invertHistogramFlag)
      {
        //check if we are in the first class break
        if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
        {
          int myRedInt = 0;
          int myBlueInt = 255;
          int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin1)) * 3);
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }
        //check if we are in the second class break
        else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
        {
          int myRedInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
          int myBlueInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
          int myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }
        //otherwise we must be in the third classbreak
        else
        {
          int myRedInt = 255;
          int myBlueInt = 0;
          int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin3) / 1) * 3)));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=myGreenInt;
            myGreenInt=255-myGreenInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }
      }                   //end of invert histogram == false check
      else                  //invert histogram toggle is off
      {
        //check if we are in the first class break
        if ((myDouble >= myClassBreakMin1) && (myDouble < myClassBreakMax1))
        {
          int myRedInt = 255;
          int myBlueInt = 0;
          int myGreenInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin1) / 1) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }
        //check if we are in the second class break
        else if ((myDouble >= myClassBreakMin2) && (myDouble < myClassBreakMax2))
        {
          int myRedInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3));
          int myBlueInt = static_cast < int >(((255 / myRangeSizeDouble) * ((myDouble - myClassBreakMin2) / 1)) * 3);
          int myGreenInt = 255;
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myGreenInt=myBlueInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }
        //otherwise we must be in the third classbreak
        else
        {
          int myRedInt = 0;
          int myBlueInt = 255;
          int myGreenInt = static_cast < int >(255 - (((255 / myRangeSizeDouble) * (myDouble - myClassBreakMin3)) * 3));
          // testing this stuff still ...
          if (colorRampingType==FREAK_OUT)
          {
            myRedInt=255-myGreenInt;
          }
          myQPainter.setPen(QPen(QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0));
        }

      }                   //end of invert histogram check
      myQPainter.drawPoint(myPosInt++, 0);
    }

  }                           //end of pseudocolor check
  else if (drawingStyle == PALETTED_MULTI_BAND_COLOR || drawingStyle == MULTI_BAND_COLOR)
  {
    //
    // Create the legend pixmap showing red green and blue band mappings
    //
    // TODO update this so it actually shows the mappings for paletted images
    myLegendQPixmap = QPixmap(3, 1);
    myQPainter.begin(&myLegendQPixmap);
    //draw legend red part
    myQPainter.setPen(QPen(QColor(224, 103, 103, QColor::Rgb), 0));
    myQPainter.drawPoint(0, 0);
    //draw legend green part
    myQPainter.setPen(QPen(QColor(132, 224, 127, QColor::Rgb), 0));
    myQPainter.drawPoint(1, 0);
    //draw legend blue part
    myQPainter.setPen(QPen(QColor(127, 160, 224, QColor::Rgb), 0));
    myQPainter.drawPoint(2, 0);
  }


  myQPainter.end();


  //see if the caller wants the name of the layer in the pixmap (used for legend bar
  if (theWithNameFlag)
  {
    QFont myQFont("times", 12, QFont::Normal);
    QFontMetrics myQFontMetrics(myQFont);

    int myWidthInt = 40 + myQFontMetrics.width(this->name());
    int myHeightInt = (myQFontMetrics.height() + 10 > 35) ? myQFontMetrics.height() + 10 : 35;

    //create a matrix to
    QWMatrix myQWMatrix;
    //scale the raster legend up a bit bigger to the legend item size
    //note that scaling parameters are factors, not absolute values,
    // so scale (0.25,1) scales the painter to a quarter of its size in the x direction
    //TODO We need to decide how much to scale by later especially for rgb images which are only 3x1 pix
    //hard coding thes values for now.
    if (myLegendQPixmap.width() == 3)
    {
      //scale width by factor of 50 (=150px wide)
      myQWMatrix.scale(60, myHeightInt);
    }
    else                    
    {
      //assume 100px so scale by factor of 1.5 (=150px wide)
      myQWMatrix.scale(1.8, myHeightInt);
    }
    //apply the matrix
    QPixmap myQPixmap2 = myLegendQPixmap.xForm(myQWMatrix);
    QPainter myQPainter(&myQPixmap2);
    //
    // Overlay a pyramid icon
    //
    if (hasPyramidsFlag)
    {
      myQPainter.drawPixmap(0,myHeightInt-mPyramidPixmap.height(),mPyramidPixmap);
    }
    else
    {
      myQPainter.drawPixmap(0,myHeightInt-mNoPyramidPixmap.height(),mNoPyramidPixmap);
    }
    //
    // Overlay the layername
    //
    if (drawingStyle == MULTI_BAND_SINGLE_BAND_GRAY || drawingStyle == PALETTED_SINGLE_BAND_GRAY || drawingStyle == SINGLE_BAND_GRAY)
    {
      myQPainter.setPen(Qt::white);
    }
    else
    {
      myQPainter.setPen(Qt::black);
    }
    myQPainter.setFont(myQFont);
    myQPainter.drawText(25, myHeightInt - 10, this->name());
    //
    // finish up
    //
    myLegendQPixmap = myQPixmap2;
    myQPainter.end();
  }
  //finish up

  return myLegendQPixmap;

}                               //end of getLegendQPixmap function

//similar to above but returns a pointer. Implemented for qgsmaplayer interface
QPixmap *QgsRasterLayer::legendPixmap()
{
  QPixmap myQPixmap = getLegendQPixmap(true);
  return new QPixmap(myQPixmap);
}

/** Accessor for the superclass popmenu var*/
QPopupMenu *QgsRasterLayer::contextMenu()
{
  return popMenu;
}

void QgsRasterLayer::initContextMenu(QgisApp * theApp)
{
  popMenu = new QPopupMenu();
  popMenu->setCheckable ( true );
  //create a heading label for the menu:
  //If a widget is not focus-enabled (see QWidget::isFocusEnabled()), the menu treats it as a separator; 
  //this means that the item is not selectable and will never get focus. In this way you can, for example, 
  //simply insert a QLabel if you need a popup menu with a title. 
  QLabel *myPopupLabel = new QLabel( popMenu );
  myPopupLabel->setFrameStyle( QFrame::Panel | QFrame::Raised );
  myPopupLabel->setText( tr("<center><b>Raster Layer</b></center>") );
  popMenu->insertItem(myPopupLabel);
  //
  popMenu->insertItem(tr("&Zoom to extent of selected layer"), theApp, SLOT(zoomToLayerExtent()));
  popMenu->insertItem(tr("&Properties"), theApp, SLOT(layerProperties()));
  //show in overview slot is implemented in maplayer superclass!
  mShowInOverviewItemId = popMenu->insertItem(tr("Show In &Overview"), this, SLOT(toggleShowInOverview()));
  popMenu->insertItem(tr("&Remove"), theApp, SLOT(removeLayer()));
  popMenu->insertSeparator();
  QLabel * myTransparencyLabel = new QLabel( popMenu );
  myTransparencyLabel->setFrameStyle( QFrame::Panel | QFrame::Raised );
  myTransparencyLabel->setText( tr("<center><b>Transparency</b></center>") );
  popMenu->insertItem(myTransparencyLabel);
  popMenu->insertItem(mTransparencySlider);

}

void QgsRasterLayer::popupTransparencySliderMoved(int theInt)
{
#ifdef QGISDEBUG
  std::cout << "popupTransparencySliderMoved called with : " << theInt << std::endl;
#endif
  if (theInt > 255)
  {
    transparencyLevelInt = 255;
  }
  else if (theInt < 0)
  {
    transparencyLevelInt = 0;
  }
  else
  {
    transparencyLevelInt = 255-theInt;
  }
  triggerRepaint();
}
//
//should be between 0 and 255
void QgsRasterLayer::setTransparency(int theInt)
{
#ifdef QGISDEBUG
  std::cout << "Set transparency called with : " << theInt << std::endl;
#endif
  mTransparencySlider->setValue(255-theInt);
  //delegate rest to transparency slider
  
}
unsigned int QgsRasterLayer::getTransparency()
{
  return transparencyLevelInt;
}

QString QgsRasterLayer::getMetadata()
{
  QString myMetadataQString = "<html><body>";
  myMetadataQString += "<table width=\"100%\">";
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Driver:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += QString(gdalDataset->GetDriver()->GetDescription());
  myMetadataQString += "<br>";
  myMetadataQString += QString(gdalDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));
  myMetadataQString += "</td></tr>";

  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Dimensions:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  myMetadataQString += tr("X: ") + QString::number(gdalDataset->GetRasterXSize()) +
      tr(" Y: ") + QString::number(gdalDataset->GetRasterYSize()) + tr(" Bands: ") + QString::number(gdalDataset->GetRasterCount());
  myMetadataQString += "</td></tr>";
  
  //just use the first band
  GDALRasterBand *myGdalBand = gdalDataset->GetRasterBand(1);
  
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Data Type:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";
  switch (myGdalBand->GetRasterDataType())
  {
      case GDT_Byte:
          myMetadataQString += tr("GDT_Byte - Eight bit unsigned integer");
          break;
      case GDT_UInt16:
          myMetadataQString += tr("GDT_UInt16 - Sixteen bit unsigned integer ");
          break;
      case GDT_Int16:
          myMetadataQString += tr("GDT_Int16 - Sixteen bit signed integer ");
          break;
      case GDT_UInt32:
          myMetadataQString += tr("GDT_UInt32 - Thirty two bit unsigned integer ");
          break;
      case GDT_Int32:
          myMetadataQString += tr("GDT_Int32 - Thirty two bit signed integer ");
          break;
      case GDT_Float32:
          myMetadataQString += tr("GDT_Float32 - Thirty two bit floating point ");
          break;
      case GDT_Float64:
          myMetadataQString += tr("GDT_Float64 - Sixty four bit floating point ");
          break;
      case GDT_CInt16:
          myMetadataQString += tr("GDT_CInt16 - Complex Int16 ");
          break;
      case GDT_CInt32:
          myMetadataQString += tr("GDT_CInt32 - Complex Int32 ");
          break;
      case GDT_CFloat32:
          myMetadataQString += tr("GDT_CFloat32 - Complex Float32 ");
          break;
      case GDT_CFloat64:
          myMetadataQString += tr("GDT_CFloat64 - Complex Float64 ");
          break;
      default:
          myMetadataQString += tr("Could not determine raster data type.");
  }
  myMetadataQString += "</td></tr>";
  
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Pyramid overviews:");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";

  if( GDALGetOverviewCount(myGdalBand) > 0 )
  {
    int myOverviewInt;
    for( myOverviewInt = 0;
            myOverviewInt < GDALGetOverviewCount(myGdalBand);
            myOverviewInt++ )
    {
      GDALRasterBandH myOverview;
      myOverview = GDALGetOverview( myGdalBand, myOverviewInt );
      myMetadataQString += "<p>X : " + QString::number(GDALGetRasterBandXSize( myOverview ));
      myMetadataQString += ",Y " + QString::number(GDALGetRasterBandYSize( myOverview ) ) + "</p>";
    }
  }
  myMetadataQString += "</td></tr>";

  if (gdalDataset->GetProjectionRef() != NULL)
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Projection: ");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    //note we inject some white space so the projection wraps better in the <td>
    QString myProjectionQString = QString(gdalDataset->GetProjectionRef());
    myProjectionQString = myProjectionQString.replace(QRegExp("\"")," \"");
    myMetadataQString += myProjectionQString ;
    myMetadataQString += "</td></tr>";
  }
  if (gdalDataset->GetGeoTransform(adfGeoTransform) == CE_None)
  {
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Origin:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += QString::number(adfGeoTransform[0]);
    myMetadataQString += ",";
    myMetadataQString += QString::number(adfGeoTransform[3]);
    myMetadataQString += "</td></tr>";

    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Pixel Size:");
    myMetadataQString += "</td></tr>";
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += QString::number(adfGeoTransform[1]);
    myMetadataQString += ",";
    myMetadataQString += QString::number(adfGeoTransform[5]);
    myMetadataQString += "</td></tr>";
  }
  //
  // Add the stats for each band to the output table
  //
  myMetadataQString += "<tr><td bgcolor=\"gray\">";
  myMetadataQString += tr("Band Statistics (if gathered):");
  myMetadataQString += "</td></tr>";
  myMetadataQString += "<tr><td bgcolor=\"white\">";

  // Start a nested table in this trow
  myMetadataQString += "<table width=\"100%\">";
  myMetadataQString += "<tr><th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Property") + "</font>";
  myMetadataQString += "</th>";
  myMetadataQString += "<th bgcolor=\"black\">";
  myMetadataQString += "<font color=\"white\">" + tr("Value") + "</font>";
  myMetadataQString += "</th><tr>";

  int myRowInt = 0;
  int myBandCountInt = getBandCount();
  for (int myIteratorInt = 1; myIteratorInt <= myBandCountInt; ++myIteratorInt)
  {
#ifdef QGISDEBUG
    std::cout << "Raster properties : checking if band " << myIteratorInt << " has stats? ";
#endif
    //band name
    myMetadataQString += "<tr><td bgcolor=\"gray\">";
    myMetadataQString += tr("Band");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"gray\">";
    myMetadataQString += getRasterBandName(myIteratorInt);
    myMetadataQString += "</td></tr>";
    //band number
    myMetadataQString += "<tr><td bgcolor=\"white\">";
    myMetadataQString += tr("Band No");
    myMetadataQString += "</td>";
    myMetadataQString += "<td bgcolor=\"white\">";
    myMetadataQString += QString::number(myIteratorInt); 
    myMetadataQString += "</td></tr>";

    //check if full stats for this layer have already been collected
    if (!hasStats(myIteratorInt))  //not collected
    {
#ifdef QGISDEBUG
      std::cout << ".....no" << std::endl;
#endif

      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("No Stats");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += tr("No stats collected yet"); 
      myMetadataQString += "</td></tr>";
    } 
    else                    // collected - show full detail
    {
#ifdef QGISDEBUG
      std::cout << ".....yes" << std::endl;
#endif

      RasterBandStats myRasterBandStats = getRasterBandStats(myIteratorInt);
      //Min Val
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Min Val");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.minValDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      // Max Val
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Max Val");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.maxValDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      // Range
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Range");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.rangeDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      // Mean
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Mean");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.meanDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      //sum of squares
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Sum of squares");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.sumSqrDevDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      //standard deviation
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Standard Deviation");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.stdDevDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      //sum of all cells
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Sum of all cells");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.sumDouble, 'f',10); 
      myMetadataQString += "</td></tr>";

      //number of cells
      myMetadataQString += "<tr><td bgcolor=\"white\">";
      myMetadataQString += tr("Cell Count");
      myMetadataQString += "</td>";
      myMetadataQString += "<td bgcolor=\"white\">";
      myMetadataQString += QString::number(myRasterBandStats.elementCountInt); 
      myMetadataQString += "</td></tr>";
    }
  }
  myMetadataQString += "</table>"; //end of nested table
  myMetadataQString += "</td></tr>"; //end of stats container table row


  //
  // Close the table
  //

  myMetadataQString += "</table>";
  myMetadataQString += "</body></html>";
  return myMetadataQString;
}

void QgsRasterLayer::buildPyramids(RasterPyramidList theRasterPyramidList)
{
  emit setProgress(0,0);
  //first test if the file is writeable
  QFile myQFile(dataSource);
  if (!myQFile.open(IO_WriteOnly| IO_Append))
  {

    QMessageBox myMessageBox( tr("Write access denied"),
            tr("Write access denied. Adjust the file permissions and try again.\n\n"),
            QMessageBox::Warning,
            QMessageBox::Ok,
            QMessageBox::NoButton,
            QMessageBox::NoButton );
    myMessageBox.exec();

    return;
  }
  myQFile.close();

  GDALAllRegister();
  //close the gdal dataset and reopen it in read / write mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(dataSource, GA_Update);
  //
  // Iterate through the Raster Layer Pyramid Vector, building any pyramid 
  // marked as exists in eaxh RasterPyramid struct.
  //
  int myCountInt=1;
  int myTotalInt=theRasterPyramidList.count();
  RasterPyramidList::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator=theRasterPyramidList.begin(); 
          myRasterPyramidIterator != theRasterPyramidList.end(); 
          ++myRasterPyramidIterator )
  {
    std::cout << "Buld pyramids:: Level " << (*myRasterPyramidIterator).levelInt
        << "x :" << (*myRasterPyramidIterator).xDimInt
        << "y :" << (*myRasterPyramidIterator).yDimInt
        << "exists :" << (*myRasterPyramidIterator).existsFlag
        << std::endl;
    if ((*myRasterPyramidIterator).existsFlag)
    {
      std::cout << "Building....." << std::endl;
      emit setProgress(myCountInt,myTotalInt);
      int myOverviewLevelsIntArray[1] = {(*myRasterPyramidIterator).levelInt };
      /* From : http://remotesensing.org/gdal/classGDALDataset.html#a23
       * pszResampling : one of "NEAREST", "AVERAGE" or "MODE" controlling the downsampling method applied.
       * nOverviews : number of overviews to build. 
       * panOverviewList : the list of overview decimation factors to build. 
       * nBand : number of bands to build overviews for in panBandList. Build for all bands if this is 0. 
       * panBandList : list of band numbers. 
       * pfnProgress : a function to call to report progress, or NULL. 
       * pProgressData : application data to pass to the progress function.
       */
#ifdef QGISDEBUG 
      //build the pyramid and show progress to console
      gdalDataset->BuildOverviews( "NEAREST", 1, myOverviewLevelsIntArray, 0, NULL,
              GDALTermProgress, NULL );
#else      
      //build the pyramid and suppress  progress to console
      gdalDataset->BuildOverviews( "NEAREST", 1, myOverviewLevelsIntArray, 0, NULL,
              GDALDummyProgress, NULL );
#endif      
      myCountInt++;
      //make sure the raster knows it has pyramids
      hasPyramidsFlag=true;
    }

  }
  std::cout << "Pyramid overviews built" << std::endl;
  //close the gdal dataset and reopen it in read only mode
  delete gdalDataset;
  gdalDataset = (GDALDataset *) GDALOpen(dataSource, GA_ReadOnly);
  emit setProgress(0,0);
}
RasterPyramidList  QgsRasterLayer::buildRasterPyramidList()
{
  //
  // First we build up a list of potential pyramid layers
  //
  int myWidth=rasterXDimInt;
  int myHeight=rasterYDimInt;
  int myDivisorInt=2;
  GDALRasterBandH myGDALBand = GDALGetRasterBand( gdalDataset, 1 ); //just use the first band

  mPyramidList.clear();
  std::cout << "Building initial pyramid list" << std::endl;
  while((myWidth/myDivisorInt > 32) && ((myHeight/myDivisorInt)>32)) 
  {

    RasterPyramid myRasterPyramid;
    myRasterPyramid.levelInt=myDivisorInt;
    myRasterPyramid.xDimInt = (int)(0.5 + (myWidth/(double)myDivisorInt));
    myRasterPyramid.yDimInt = (int)(0.5 + (myHeight/(double)myDivisorInt));
    myRasterPyramid.existsFlag=false;
    std::cout << "Pyramid:  " << myRasterPyramid.levelInt << " "
        << myRasterPyramid.xDimInt << " "
        << myRasterPyramid.yDimInt << " "
        << std::endl;




    //
    // Now we check if it actually exists in the raster layer
    // and also adjust the dimensions if the dimensions calculated
    // above are only a near match.
    //
    const int myNearMatchLimitInt=5;
    if( GDALGetOverviewCount(myGDALBand) > 0 )
    {
      int myOverviewInt;
      for( myOverviewInt = 0;
              myOverviewInt < GDALGetOverviewCount(myGDALBand);
              myOverviewInt++ )
      {
        GDALRasterBandH myOverview;
        myOverview = GDALGetOverview( myGDALBand, myOverviewInt );
        int myOverviewXDim = GDALGetRasterBandXSize( myOverview );
        int myOverviewYDim = GDALGetRasterBandYSize( myOverview );
        //
        // here is where we check if its a near match:
        // we will see if its within 5 cells either side of
        //
        std::cout << "Checking whether " <<
            myRasterPyramid.xDimInt << " x " <<
            myRasterPyramid.yDimInt << " matches " << 
            myOverviewXDim << " x " << myOverviewYDim ;

        if ((myOverviewXDim <= (myRasterPyramid.xDimInt+ myNearMatchLimitInt)) && 
                (myOverviewXDim >= (myRasterPyramid.xDimInt- myNearMatchLimitInt)) &&
                (myOverviewYDim <= (myRasterPyramid.yDimInt+ myNearMatchLimitInt)) &&
                (myOverviewYDim >= (myRasterPyramid.yDimInt- myNearMatchLimitInt)))
        {
          //right we have a match so adjust the a / y before they get added to the list
          myRasterPyramid.xDimInt=myOverviewXDim;
          myRasterPyramid.yDimInt=myOverviewYDim;
          myRasterPyramid.existsFlag=true;
          std::cout << ".....YES!" << std::endl;
        }
        else
        {
          //no match
          std::cout << ".....no." << std::endl;
        }
      }
    }
    mPyramidList.append(myRasterPyramid);
    //sqare the divisor each step
    myDivisorInt=(myDivisorInt *2);
  }

  return mPyramidList;
}
/*
   RasterPyramid QgsRasterLayer::getRasterPyramid(int thePyramidNoInt)
   {
   if (thePyramidNoInt < 0 || thePyramidNoInt > mPyramidList.size())
   {
   return NULL;
   }
   else
   {
   return mPyramidList[thePyramidNoInt];
   }
   }
   */
//currently not used! See also typedef by same name near top of rasterlayer header file.
/*
   int QgsRasterLayer::showTextProgress( double theProgressDouble,
   const char *theMessageCharArray,
   void *theData)
   {
   if( theMessageCharArray != NULL )
   printf( "%d%% complete: %s\n", (int) (theProgressDouble*100), theMessageCharArray );
   else if( theData != NULL )
   printf( "%d%% complete:%s\n", (int) (theProgressDouble*100),
   (char) theData );
   else
   printf( "%d%% complete.\n", (int) (theProgressDouble*100) );

   return TRUE;
   }
   */
