/***************************************************************************
  qgsrasterlayer.cpp -  description
  -------------------
begin                : Sat Jun 22 2002
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
/* $Id$ */
#include <qpainter.h>
#include <qimage.h>
#include <stdio.h>

#include "qgspoint.h"
#include "qgsrect.h"
#include "qgsrasterlayer.h"
#include "gdal_priv.h"

    QgsRasterLayer::QgsRasterLayer(QString path, QString baseName)
:QgsMapLayer(RASTER, baseName, path)
{
  //std::cout << "QgsRasterLayer::QgsRasterLayer()" << std::endl;

  GDALAllRegister();
  gdalDataset = (GDALDataset *) GDALOpen( path, GA_ReadOnly );
  if ( gdalDataset == NULL ) {
    valid = FALSE;
    return;
  }
  //std::cout << "Raster Count: " << gdalDataset->GetRasterCount() << std::endl;

  if( gdalDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
  {
    printf( "Origin = (%.6f,%.6f)\n",
            adfGeoTransform[0], adfGeoTransform[3] );

    printf( "Pixel Size = (%.6f,%.6f)\n",
            adfGeoTransform[1], adfGeoTransform[5] );
  }

  double myXMaxDouble = adfGeoTransform[0] + gdalDataset->GetRasterXSize() * adfGeoTransform[1] +  
      gdalDataset->GetRasterYSize() * adfGeoTransform[2];
  double myYMinDouble  = adfGeoTransform[3] + gdalDataset->GetRasterXSize() * adfGeoTransform[4] + 
      gdalDataset->GetRasterYSize() * adfGeoTransform[5];

  layerExtent.setXmax(myXMaxDouble);
  layerExtent.setXmin(adfGeoTransform[0]);
  layerExtent.setYmax(adfGeoTransform[3]);
  layerExtent.setYmin(myYMinDouble);
  calculateStats();
  redBandNameQString="Red"; // sensible default
  greenBandNameQString="Green"; // sensible default
  blueBandNameQString="Blue"; // sensible default
  showGrayAsColorFlag=false; // sensible default
  invertHistogramFlag=false; // sensible default
  stdDevsToPlotDouble = 0; // sensible default
  transparencyLevelInt = 255; //sensible default 0 is transparent  
}

QgsRasterLayer::~QgsRasterLayer()
{
  GDALClose(gdalDataset);
}
// emit a signal asking for a repaint
void QgsRasterLayer::triggerRepaint()
{
  emit repaintRequested(); 
}
//
//should be between 0 and 255
void QgsRasterLayer::slot_setTransparency(unsigned int theInt)
{
  if (theInt > 255) 
    transparencyLevelInt= 255;
  else
    transparencyLevelInt=theInt;
}
unsigned int QgsRasterLayer::getTransparency()
{
  return  transparencyLevelInt;
}

bool QgsRasterLayer::hasBand(QString theBandName)
{
  RasterStatsMap::Iterator myIterator;
  std::cout << "Looking for band : " << theBandName << std::endl;
  for(myIterator  = rasterStatsMap.begin(); myIterator != rasterStatsMap.end(); ++myIterator )
  {
    if  (myIterator.key()==theBandName) 
    {
      std::cout << "Found band : " << theBandName << std::endl;
      return true;
    }
    std::cout << "Found unmatched band : " << myIterator.key().latin1() << std::endl;
  }
  return false;
}

void QgsRasterLayer::draw(QPainter * theQPainter, QgsRect * theViewExtent, QgsCoordinateTransform * theQgsCoordinateTransform)
{
  //std::cout << "QgsRasterLayer::draw()" << std::endl;
  //std::cout << "gdalDataset->GetRasterCount(): " << gdalDataset->GetRasterCount() << std::endl;
  //std::cout << "Layer extent: " << layerExtent.stringRep() << std::endl;

  // clip raster extent to view extent
  QgsRect myRasterExtent = theViewExtent->intersect(&layerExtent);
  if (myRasterExtent.isEmpty()) {
    // nothing to do
    return;
  }

  // calculate raster pixel offsets from origin to clipped rect
  // we're only interested in positive offsets where the origin of the raster
  // is northwest of the origin of the view
  int myRectXOffsetInt = static_cast<int>((theViewExtent->xMin() - layerExtent.xMin()) / fabs(adfGeoTransform[1]));
  myRectXOffsetInt = myRectXOffsetInt >? 0;
  int myRectYOffsetInt = static_cast<int>((layerExtent.yMax() - theViewExtent->yMax()) / fabs(adfGeoTransform[5]));
  myRectYOffsetInt = myRectYOffsetInt >? 0;

  // get dimensions of clipped raster image in raster pixel space
  double myClippedXMinDouble = (myRasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
  double myClippedXMaxDouble = (myRasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
  double myClippedYMinDouble = (myRasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
  double myClippedYMaxDouble = (myRasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];
  int myClippedWidthInt = abs(static_cast<int>(myClippedXMaxDouble - myClippedXMinDouble));
  int myClippedHeightInt = abs(static_cast<int>(myClippedYMaxDouble - myClippedYMinDouble));

  // get dimensions of clipped raster image in device coordinate space
  QgsPoint myTopLeftPoint = theQgsCoordinateTransform->transform(myRasterExtent.xMin(), myRasterExtent.yMax());
  QgsPoint myBottomRightPoint = theQgsCoordinateTransform->transform(myRasterExtent.xMax(), myRasterExtent.yMin());
  int myLayerXDimInt = myBottomRightPoint.xToInt() - myTopLeftPoint.xToInt();
  int myLayerYDimInt = myBottomRightPoint.yToInt() - myTopLeftPoint.yToInt();

  // loop through raster bands
  // a band can have color values that correspond to colors in a palette
  // or it can contain the red, green or blue value for an rgb image
  // HLS, CMYK, or RGB alpha bands are silently ignored for now
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) 
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( i );
    double myNoDataDouble = myGdalBand->GetNoDataValue();
    //std::cout << "Nodata value for band " << i << " is " << myNoDataDouble << "\n" << std::endl;
    //std::cout << "myGdalBand->GetOverviewCount(): " << myGdalBand->GetOverviewCount() <<std::endl;
    // make sure we don't exceed size of raster
    myClippedWidthInt = myClippedWidthInt <? myGdalBand->GetXSize();
    myClippedHeightInt = myClippedHeightInt <? myGdalBand->GetYSize();

    // read entire clipped area of raster band
    // treat scandata as a pseudo-multidimensional array
    // RasterIO() takes care of scaling down image
    uint *scandata = (uint*) CPLMalloc(sizeof(uint)*myLayerXDimInt * sizeof(uint)*myLayerYDimInt);
    CPLErr result = myGdalBand->RasterIO( 
            GF_Read, myRectXOffsetInt, myRectYOffsetInt, myClippedWidthInt, myClippedHeightInt, scandata, myLayerXDimInt, myLayerYDimInt, GDT_UInt32, 0, 0 );

    QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    //std::cout << "Colour Interpretation for this band is : " << myColorInterpretation << std::endl;
    if ( myColorInterpretation == "Palette") 
    {
      // print each point in scandata using color looked up in color table
      GDALColorTable *colorTable = myGdalBand->GetColorTable();
      QImage myQImage=QImage(myLayerXDimInt,myLayerYDimInt,32);
      myQImage.setAlphaBuffer(true);
      for (int y = 0; y < myLayerYDimInt; y++) 
      {
        for (int x =0; x < myLayerXDimInt; x++) 
        {
          const GDALColorEntry *colorEntry = GDALGetColorEntry(colorTable, scandata[y*myLayerXDimInt + x]);
          //dont draw this point if it is no data !
          if (myNoDataDouble != scandata[y*myLayerXDimInt + x])
          {
            int myRedValueInt=0; //color 1 int
            int myGreenValueInt=0; //color 2 int
            int myBlueValueInt=0; //color 3 int
            //check colorEntry is valid
            if (colorEntry!=NULL)
            { 
	      //check for alternate color mappings
              if (redBandNameQString=="Red") myRedValueInt=colorEntry->c1;
              if (redBandNameQString=="Green") myRedValueInt=colorEntry->c2;
              if (redBandNameQString=="Blue") myRedValueInt=colorEntry->c3;
              if (greenBandNameQString=="Red") myGreenValueInt=colorEntry->c1;
              if (greenBandNameQString=="Green") myGreenValueInt=colorEntry->c2;
              if (greenBandNameQString=="Blue") myGreenValueInt=colorEntry->c3;
              if (blueBandNameQString=="Red") myBlueValueInt=colorEntry->c1;
              if (blueBandNameQString=="Green") myBlueValueInt=colorEntry->c2;
              if (blueBandNameQString=="Blue") myBlueValueInt=colorEntry->c3;
            }
            else
            {
              //there is no guarantee that there will be a matching palette entry for
              //every cell in the raster. If there is no match, do nothing.
            }
            if (invertHistogramFlag)
            {
              myRedValueInt=255-myRedValueInt; 
              myGreenValueInt=255-myGreenValueInt;
              myBlueValueInt=255-myBlueValueInt; 

            }
            //set the pixel based on the above color mappings
            myQImage.setPixel( x, y, qRgba( myRedValueInt,myGreenValueInt,myBlueValueInt, transparencyLevelInt )); 
            //old method for painting directly to canvas with no alpha rendering  
            //theQPainter->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
            //theQPainter->drawPoint(myTopLeftPoint.xToInt() + x, myTopLeftPoint.yToInt() + y);
          }
        }
      }
      //part of the experimental transaparency support
      theQPainter->drawImage(myTopLeftPoint.xToInt(), myTopLeftPoint.yToInt(),myQImage);
    } else if ( myColorInterpretation == "Red" ) {
      // print each point in scandata as the red part of an rgb value
      // this assumes that the red band will always be first
      // is that necessarily the case?
      for (int y = 0; y < myLayerYDimInt; y++) {
        for (int x =0; x < myLayerXDimInt; x++) {
          //dont draw this point if it is no data !
          if (myNoDataDouble != scandata[y*myLayerXDimInt + x])
          {
            theQPainter->setPen(QColor(scandata[y*myLayerXDimInt + x], 0, 0));
            theQPainter->drawPoint(myTopLeftPoint.xToInt() + x, myTopLeftPoint.yToInt() + y);
          }
        }
      }
    } else if ( myColorInterpretation == "Green" ) {
      // print each point in scandata as the green part of an rgb value
      theQPainter->setRasterOp(Qt::XorROP);
      for (int y = 0; y < myLayerYDimInt; y++) {
        for (int x =0; x < myLayerXDimInt; x++) {
          //dont draw this point if it is no data !
          if (myNoDataDouble != scandata[y*myLayerXDimInt + x])
          {
            theQPainter->setPen(QColor(0, scandata[y*myLayerXDimInt + x], 0));
            theQPainter->drawPoint(myTopLeftPoint.xToInt() + x, myTopLeftPoint.yToInt() + y);
          }
        }
      }
      theQPainter->setRasterOp(Qt::CopyROP);
    } else if ( myColorInterpretation == "Blue" ) {
      // print each point in scandata as the blue part of an rgb value
      theQPainter->setRasterOp(Qt::XorROP);
      for (int y = 0; y < myLayerYDimInt; y++) {
        for (int x =0; x < myLayerXDimInt; x++) {
          //dont draw this point if it is no data !
          if (myNoDataDouble != scandata[y*myLayerXDimInt + x])
          {
            theQPainter->setPen(QColor(0, 0, scandata[y*myLayerXDimInt + x]));
            theQPainter->drawPoint(myTopLeftPoint.xToInt() + x, myTopLeftPoint.yToInt() + y);
          }
        }
      }
      theQPainter->setRasterOp(Qt::CopyROP);
    } else if ( myColorInterpretation == "Gray" || myColorInterpretation == "Undefined" ) {
      if (!showGrayAsColorFlag)
      {
        //ensure we are not still xoring
        theQPainter->setRasterOp(Qt::CopyROP);  
        //experimental transparency support for grayscale
        //if this works ok, I will move it into other layertype renderings
        QImage myQImage=QImage(myLayerXDimInt,myLayerYDimInt,32);
        myQImage.setAlphaBuffer(true);
        RasterBandStats myRasterBandStats = rasterStatsMap[myColorInterpretation];
        double myRangeDouble=myRasterBandStats.rangeDouble;
        // print each point in scandata with equal parts R, G ,B o make it show as gray
        for (int y = 0; y < myLayerYDimInt; y++) {
          for (int x =0; x < myLayerXDimInt; x++) {
            int myGrayValInt=scandata[y*myLayerXDimInt + x];
            //dont draw this point if it is no data !
            if (myGrayValInt != myRasterBandStats.noDataDouble)
            {
              //if band is undefined, we need to scale the values to 0-255
              if (myColorInterpretation=="Undefined")
              {
                myGrayValInt = static_cast<int>((255/myRangeDouble) * myGrayValInt);
              }
              if (invertHistogramFlag) myGrayValInt=255-myGrayValInt;
              myQImage.setPixel( x, y, qRgba( myGrayValInt, myGrayValInt, myGrayValInt, transparencyLevelInt )); 
            }
            else //render no data as 100% transparent
            {
              //0 alpha = completely transparent
              myQImage.setPixel( x, y, qRgba( 255, 255, 255, 0 )); 
            }
          }
        }
        //part of the experimental transaparency support
        theQPainter->drawImage(myTopLeftPoint.xToInt(), myTopLeftPoint.yToInt(),myQImage);
      }
      else { //show the gray layer as pseudocolor
        //ensure we are not still xoring

        theQPainter->setRasterOp(Qt::CopyROP); 
        int myRedInt=0;
        int myGreenInt=0;
        int myBlueInt=0;
        //calculate the adjusted matrix stats - which come into affect if the user has chosen
        RasterBandStats myAdjustedRasterBandStats = rasterStatsMap[myColorInterpretation];
        myAdjustedRasterBandStats.noDataDouble=0;//hard coding for now
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
          if (myAdjustedRasterBandStats.minValDouble < (myAdjustedRasterBandStats.meanDouble-myTotalDeviationDouble)) 
          {         
            myAdjustedRasterBandStats.minValDouble=(myAdjustedRasterBandStats.meanDouble-myTotalDeviationDouble);
            //cout << "Adjusting minValDouble to: " << myAdjustedRasterBandStats.minValDouble << endl;
          }
          //only change max if it is greater than mean  +  (n  x  deviations)
          if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble)) 
          {
            myAdjustedRasterBandStats.maxValDouble=(myAdjustedRasterBandStats.meanDouble+myTotalDeviationDouble);
            //cout << "Adjusting maxValDouble to: " << myAdjustedRasterBandStats.maxValDouble << endl;    
          }
          //update the range
          myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble-myAdjustedRasterBandStats.minValDouble;
        }
        //set up the three class breaks for pseudocolour mapping
        double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
        double myClassBreakMin1 = myAdjustedRasterBandStats.minValDouble;
        double myClassBreakMax1 = myAdjustedRasterBandStats.minValDouble + myBreakSizeDouble;
        double myClassBreakMin2 = myClassBreakMax1;
        double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
        double myClassBreakMin3 = myClassBreakMax2;
        double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;

        QImage myQImage=QImage(myLayerXDimInt,myLayerYDimInt,32);
        myQImage.setAlphaBuffer(true);

        for (int y = 0; y < myLayerYDimInt; y++) {
          for (int x =0; x < myLayerXDimInt; x++) {
            int myInt=scandata[y*myLayerXDimInt + x];
            //dont draw this point if it is no data !
            //double check that myInt >= min and <= max
            //this is relevant if we are plotting within stddevs
            if ((myInt < myAdjustedRasterBandStats.minValDouble ) && (myInt != myAdjustedRasterBandStats.noDataDouble))
            {
              myInt = static_cast<int>(myAdjustedRasterBandStats.minValDouble);
            }
            if ((myInt > myAdjustedRasterBandStats.maxValDouble)   && (myInt != myAdjustedRasterBandStats.noDataDouble))
            {
              myInt = static_cast<int>(myAdjustedRasterBandStats.maxValDouble);
            }      
            if (myInt==myAdjustedRasterBandStats.noDataDouble)
            {
              //hardcoding to white for now
              myRedInt = 255;
              myBlueInt = 255;
              myGreenInt =255;          
            }
            else if(!invertHistogramFlag)
            { 
              //check if we are in the first class break
              if ((myInt >= myClassBreakMin1) &&  (myInt < myClassBreakMax1) )
              {
                myRedInt = 0;
                myBlueInt = 255;
                myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * (myInt-myClassBreakMin1))*3);
              }
              //check if we are in the second class break
              else if ((myInt >= myClassBreakMin2) &&  (myInt < myClassBreakMax2) )
              {
                myRedInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin2)/1))*3);
                myBlueInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin2)/1))*3));
                myGreenInt = 255;
              }
              //otherwise we must be in the third classbreak
              else
              {
                myRedInt = 255;
                myBlueInt = 0;
                myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin3)/1)*3)));
              }
            }
            else  //invert histogram toggle is on
            {
              //check if we are in the first class break      
              if ((myInt >= myClassBreakMin1) &&  (myInt < myClassBreakMax1) )
              {
                myRedInt = 255;
                myBlueInt = 0;
                myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin1)/1)*3));
              }
              //check if we are in the second class break
              else if ((myInt >= myClassBreakMin2) &&  (myInt < myClassBreakMax2) )
              {
                myRedInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin2)/1))*3));
                myBlueInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myInt-myClassBreakMin2)/1))*3);
                myGreenInt = 255;
              }
              //otherwise we must be in the third classbreak
              else 
              {
                myRedInt = 0;
                myBlueInt = 255;
                myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * (myInt-myClassBreakMin3))*3));
              }   
            }
            myQImage.setPixel( x, y, qRgba( myRedInt, myGreenInt, myBlueInt, transparencyLevelInt )); 
            //these are the old way of drawing direct to canvase, no transparency
            //theQPainter->setPen(QColor(myRedInt, myGreenInt, myBlueInt));
            //theQPainter->drawPoint(myTopLeftPoint.xToInt() + x, myTopLeftPoint.yToInt() + y);
          }
        }
        //draw with the experimental transaparency support
        theQPainter->drawImage(myTopLeftPoint.xToInt(), myTopLeftPoint.yToInt(),myQImage);
      }
    } else {
      // do nothing
    }

    CPLFree(scandata);
  }
}

//void QgsRasterLayer::identify(QgsRect * r)
//{
//}

/**  Return the statistics for a given band number */
const  RasterBandStats QgsRasterLayer::getRasterBandStats(int theBandNo) 
{        
  // make sure the band stats requested correspond to a valid band number
  if (rasterStatsMap.size() >= theBandNo) 
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( theBandNo );
    QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    return rasterStatsMap[myColorInterpretation];
  }
}
/** Return the statistics for a given band name */
const  RasterBandStats QgsRasterLayer::getRasterBandStats(QString theBandName) 
{        
  return rasterStatsMap[theBandName];
}

/** Private method to calculate statistics for each band. Populates rasterStatsMap. */
void QgsRasterLayer::calculateStats()
{
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) 
  {
    RasterBandStats myRasterBandStats; 
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( i );
    QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    myRasterBandStats.bandName=myColorInterpretation;
    myRasterBandStats.bandNo=i;
    std::cout << "Getting stats for band " << i << " (" << myColorInterpretation << ")" << std::endl;
    // get the dimensions of the raster
    int myColsInt = myGdalBand->GetXSize();
    int myRowsInt = myGdalBand->GetYSize();

    myRasterBandStats.elementCountInt=myColsInt*myRowsInt;
    myRasterBandStats.noDataDouble=myGdalBand->GetNoDataValue();
    //avoid collecting stats for rgb images for now
    if (myColorInterpretation=="Gray" || myColorInterpretation=="Undefined") 
    {
      //allocate a buffer to hold one row of ints
      int myAllocationSizeInt = sizeof(uint)*myColsInt; 
      uint * myScanlineAllocInt = (uint*) CPLMalloc(myAllocationSizeInt);
      bool myFirstIterationFlag = true;
      //unfortunately we need to make two passes through the data to calculate stddev
      for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
      {  
        CPLErr myResult = myGdalBand->RasterIO( 
                GF_Read, 0, myCurrentRowInt, myColsInt, 1, myScanlineAllocInt, myColsInt, 1, GDT_UInt32, 0, 0 );
        for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
        {
          //get the nth element from the current row
          double myDouble=myScanlineAllocInt[myCurrentColInt];
          //only use this element if we have a non null element
          if (myDouble != myRasterBandStats.noDataDouble )
          {                  
            if (myFirstIterationFlag)
            {
              //this is the first iteration so initialise vars
              myFirstIterationFlag=false;
              myRasterBandStats.minValDouble=myDouble;
              myRasterBandStats.maxValDouble=myDouble;
            } //end of true part for first iteration check
            else
            {
              //this is done for all subsequent iterations
              if (myDouble < myRasterBandStats.minValDouble) 
              {
                myRasterBandStats.minValDouble=myDouble;
              }
              if (myDouble > myRasterBandStats.maxValDouble)
              {
                myRasterBandStats.maxValDouble=myDouble;
              }
              //only increment the running total if it is not a nodata value
              if (myDouble != myRasterBandStats.noDataDouble)
              {
                myRasterBandStats.sumDouble += myDouble;
                ++myRasterBandStats.elementCountInt;  
              }
            } //end of false part for first iteration check
          } //end of nodata chec
        } //end of column wise loop 
      } //end of row wise loop 
      //
      //end of first pass through data now calculate the range
      myRasterBandStats.rangeDouble = myRasterBandStats.maxValDouble-myRasterBandStats.minValDouble;
      //calculate the mean
      myRasterBandStats.meanDouble = myRasterBandStats.sumDouble / myRasterBandStats.elementCountInt;         
      //for the second pass we will get the sum of the squares / mean
      for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
      {  
        CPLErr myResult = myGdalBand->RasterIO( 
                GF_Read, 0, myCurrentRowInt, myColsInt, 1, myScanlineAllocInt, myColsInt, 1, GDT_UInt32, 0, 0 );
        for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
        {
          //get the nth element from the current row
          double myDouble=myScanlineAllocInt[myCurrentColInt];
          myRasterBandStats.sumSqrDevDouble += static_cast<double>(pow(myDouble - myRasterBandStats.meanDouble,2));
        } //end of column wise loop
      } //end of row wise loop                            
      //divide result by sample size - 1 and get square root to get stdev
      myRasterBandStats.stdDevDouble = static_cast<double>(sqrt(myRasterBandStats.sumSqrDevDouble /
                  (myRasterBandStats.elementCountInt - 1)));
      CPLFree(myScanlineAllocInt);
    }//end of gray / undefined raster color interp check
    //add this band to the class stats map
    rasterStatsMap[myColorInterpretation]=myRasterBandStats;
  }//end of band loop
}


//mutator for red band name (allows alternate mappings e.g. map blue as red colour)
void QgsRasterLayer::setRedBandName(QString theBandNameQString)
{

  //check that a valid band name was passed, or if this is a paletted image, 
  //Red, Green or Blue are also allowed.
  for (int myInt = 1; myInt <= gdalDataset->GetRasterCount(); ++myInt)
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( myInt );
    QString myBandNameQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    if (theBandNameQString==myBandNameQString)
    { 
      //set the class member red band with the name passed by the calling fn
      redBandNameQString=theBandNameQString;
    }
    else if ((myBandNameQString=="Palette") && (theBandNameQString=="Red" || theBandNameQString=="Green" ||theBandNameQString=="Blue") )
    {
      //this is a special case where we allow the calling fn to set the band name to rd green or blue
      //if the layre is palette based. This allows us to plot RGB onto RBG or BGR for example. See the 
      //Palette section in the paint routine for more info.
      redBandNameQString=theBandNameQString;
    }
  }
}
//mutator for green band name
void QgsRasterLayer::setGreenBandName(QString theBandNameQString)
{
  //check that a valid band name was passed, or if this is a paletted image, 
  //Red, Green or Blue are also allowed.
  for (int myInt = 1; myInt <= gdalDataset->GetRasterCount(); ++myInt)
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( myInt );
    QString myBandNameQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    if (theBandNameQString==myBandNameQString)
    { 
      //set the class member red band with the name passed by the calling fn
      greenBandNameQString=theBandNameQString;
    }
    else if ((myBandNameQString=="Palette") && (theBandNameQString=="Red" || theBandNameQString=="Green" ||theBandNameQString=="Blue") )
    {
      //this is a special case where we allow the calling fn to set the band name to rd green or blue
      //if the layre is palette based. This allows us to plot RGB onto RBG or BGR for example. See the 
      //Palette section in the paint routine for more info.
      greenBandNameQString=theBandNameQString;
    }
  }

}
//mutator for blue band name
void QgsRasterLayer::setBlueBandName(QString theBandNameQString)
{
  //check that a valid band name was passed, or if this is a paletted image, 
  //Red, Green or Blue are also allowed.
  for (int myInt = 1; myInt <= gdalDataset->GetRasterCount(); ++myInt)
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( myInt );
    QString myBandNameQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    if (theBandNameQString==myBandNameQString)
    { 
      //set the class member red band with the name passed by the calling fn
      blueBandNameQString=theBandNameQString;
    }
    else if ((myBandNameQString=="Palette") && (theBandNameQString=="Red" || theBandNameQString=="Green" ||theBandNameQString=="Blue") )
    {
      //this is a special case where we allow the calling fn to set the band name to rd green or blue
      //if the layre is palette based. This allows us to plot RGB onto RBG or BGR for example. See the 
      //Palette section in the paint routine for more info.
      blueBandNameQString=theBandNameQString;
    }
  }
}
//mutator for gray band name
void QgsRasterLayer::setGrayBandName(QString theBandNameQString)
{
  //check that a valid band name was passed, or if this is a paletted image, 
  //Red, Green or Blue are also allowed.
  for (int myInt = 1; myInt <= gdalDataset->GetRasterCount(); ++myInt)
  {
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( myInt );
    QString myBandNameQString = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    if (theBandNameQString==myBandNameQString)
    { 
      //set the class member gray band with the name passed by the calling fn
      grayBandNameQString=theBandNameQString;
    }
  }
}

//return a pixmap representing a legend image
QPixmap QgsRasterLayer::getLegendQPixmap()
{
  //
  // Get the adjusted matrix stats
  //
  GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( 1 );
  double myNoDataDouble = myGdalBand->GetNoDataValue();
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  if ( myColorInterpretation == "Gray" || myColorInterpretation == "Undefined")
  {
    RasterBandStats myAdjustedRasterBandStats = rasterStatsMap[myColorInterpretation];
    myAdjustedRasterBandStats.noDataDouble=0;//hard coding for now
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
      if (myAdjustedRasterBandStats.minValDouble < (myAdjustedRasterBandStats.meanDouble-myTotalDeviationDouble)) 
      {         
        myAdjustedRasterBandStats.minValDouble=(myAdjustedRasterBandStats.meanDouble-myTotalDeviationDouble);
        //cout << "Adjusting minValDouble to: " << myAdjustedRasterBandStats.minValDouble << endl;
      }
      //only change max if it is greater than mean  +  (n  x  deviations)
      if (myAdjustedRasterBandStats.maxValDouble > (myAdjustedRasterBandStats.meanDouble + myTotalDeviationDouble)) 
      {
        myAdjustedRasterBandStats.maxValDouble=(myAdjustedRasterBandStats.meanDouble+myTotalDeviationDouble);
        //cout << "Adjusting maxValDouble to: " << myAdjustedRasterBandStats.maxValDouble << endl;    
      }
      //update the range
      myAdjustedRasterBandStats.rangeDouble = myAdjustedRasterBandStats.maxValDouble-myAdjustedRasterBandStats.minValDouble;
    }
    //set up the three class breaks for pseudocolour mapping
    double myBreakSizeDouble = myAdjustedRasterBandStats.rangeDouble / 3;
    double myClassBreakMin1 = myAdjustedRasterBandStats.minValDouble;
    double myClassBreakMax1 = myAdjustedRasterBandStats.minValDouble + myBreakSizeDouble;
    double myClassBreakMin2 = myClassBreakMax1;
    double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
    double myClassBreakMin3 = myClassBreakMax2;
    double myClassBreakMax3 = myAdjustedRasterBandStats.maxValDouble;

    //
    // Create the legend pixmap - note it is generated on the preadjusted stats
    //
    QPixmap myLegendQPixmap = QPixmap(100,1);
    QPainter myQPainter; 
    myQPainter.begin(&myLegendQPixmap);
    int myPosInt = 0;
    for (double myDouble=myAdjustedRasterBandStats.minValDouble ;
            myDouble < myAdjustedRasterBandStats.maxValDouble ;
            myDouble+=myAdjustedRasterBandStats.rangeDouble/100)
    {
      if (!showGrayAsColorFlag)
      {
        //draw legend as grayscale
        int myGrayInt = static_cast<int>((255/myAdjustedRasterBandStats.rangeDouble) * myDouble);
        myQPainter.setPen( QPen( QColor(myGrayInt, myGrayInt, myGrayInt, QColor::Rgb), 0) );
      }
      else
      {
        //draw pseudocolor legend
        if (!invertHistogramFlag)
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) &&  (myDouble < myClassBreakMax1) )
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * (myDouble-myClassBreakMin1))*3);
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) &&  (myDouble < myClassBreakMax2) )
          {
            int myRedInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3);
            int myBlueInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3));
            int myGreenInt = 255;
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin3)/1)*3)));
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
        }//end of invert histogram == false check
        else  //invert histogram toggle is off
        {
          //check if we are in the first class break
          if ((myDouble >= myClassBreakMin1) &&  (myDouble < myClassBreakMax1) )
          {
            int myRedInt = 255;
            int myBlueInt = 0;
            int myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin1)/1)*3));
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
          //check if we are in the second class break
          else if ((myDouble >= myClassBreakMin2) &&  (myDouble < myClassBreakMax2) )
          {
            int myRedInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3));
            int myBlueInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3);
            int myGreenInt = 255;
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
          //otherwise we must be in the third classbreak
          else
          {
            int myRedInt = 0;
            int myBlueInt = 255;
            int myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * (myDouble-myClassBreakMin3))*3));
            myQPainter.setPen( QPen( QColor(myRedInt, myGreenInt, myBlueInt, QColor::Rgb), 0) );
          }
        }
      } //end of show as gray check
      myQPainter.drawPoint( myPosInt++,0);
    }
    myQPainter.end();
    return myLegendQPixmap;
  } //end of colour interpretation is palette or gray or undefined check

  else if (myColorInterpretation == "Palette")
  {
    //
    // Create the legend pixmap showing red green and blue band mappings 
    //
    // TODO update this so it actually shows the mappings for paletted images
    QPixmap myLegendQPixmap = QPixmap(3,1);
    QPainter myQPainter; 
    myQPainter.begin(&myLegendQPixmap);
    //draw legend red part
    myQPainter.setPen( QPen( QColor(255,0,0, QColor::Rgb), 0) );
    myQPainter.drawPoint( 0,0);
    //draw legend green part
    myQPainter.setPen( QPen( QColor(0,255,0, QColor::Rgb), 0) );
    myQPainter.drawPoint( 1,0);
    //draw legend blue part
    myQPainter.setPen( QPen( QColor(0,0,255, QColor::Rgb), 0) );
    myQPainter.drawPoint( 2,0);
    myQPainter.end();
    return myLegendQPixmap;
  }

  else
  {
    //
    // Create the legend pixmap showing red green and blue band mappings 
    //
    //TODO update this so it show the colour band mappins rather than just an rgb swatch
    QPixmap myLegendQPixmap = QPixmap(3,1);
    QPainter myQPainter; 
    myQPainter.begin(&myLegendQPixmap);
    //draw legend red part
    myQPainter.setPen( QPen( QColor(255,0,0, QColor::Rgb), 0) );
    myQPainter.drawPoint( 0,0);
    //draw legend green part
    myQPainter.setPen( QPen( QColor(0,255,0, QColor::Rgb), 0) );
    myQPainter.drawPoint( 1,0);
    //draw legend blue part
    myQPainter.setPen( QPen( QColor(0,0,255, QColor::Rgb), 0) );
    myQPainter.drawPoint( 2,0);
    myQPainter.end();
    return myLegendQPixmap;

  } //end of mulitband

}//end of getLegendQPixmap function
