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
	
	double XMax = adfGeoTransform[0] + gdalDataset->GetRasterXSize() * adfGeoTransform[1] +  
				  gdalDataset->GetRasterYSize() * adfGeoTransform[2];
	double YMin  = adfGeoTransform[3] + gdalDataset->GetRasterXSize() * adfGeoTransform[4] + 
				   gdalDataset->GetRasterYSize() * adfGeoTransform[5];
	
	layerExtent.setXmax(XMax);
	layerExtent.setXmin(adfGeoTransform[0]);
	layerExtent.setYmax(adfGeoTransform[3]);
	layerExtent.setYmin(YMin);
        calculateStats();
        showGrayAsColorFlag=true; //hard coding for now
        invertHistogramFlag=true; //hard coding for now
        stdDevsToPlotDouble = 0; // hard coding for now!
        
}

QgsRasterLayer::~QgsRasterLayer()
{
	GDALClose(gdalDataset);
}

void QgsRasterLayer::draw(QPainter * p, QgsRect * viewExtent, QgsCoordinateTransform * cXf)
{
	//std::cout << "QgsRasterLayer::draw()" << std::endl;
	//std::cout << "gdalDataset->GetRasterCount(): " << gdalDataset->GetRasterCount() << std::endl;
	//std::cout << "Layer extent: " << layerExtent.stringRep() << std::endl;
	
	// clip raster extent to view extent
	QgsRect rasterExtent = viewExtent->intersect(&layerExtent);
	if (rasterExtent.isEmpty()) {
		// nothing to do
		return;
	}
	
	// calculate raster pixel offsets from origin to clipped rect
	// we're only interested in positive offsets where the origin of the raster
	// is northwest of the origin of the view
	int rXOff = static_cast<int>((viewExtent->xMin() - layerExtent.xMin()) / fabs(adfGeoTransform[1]));
	rXOff = rXOff >? 0;
	int rYOff = static_cast<int>((layerExtent.yMax() - viewExtent->yMax()) / fabs(adfGeoTransform[5]));
	rYOff = rYOff >? 0;
	
	// get dimensions of clipped raster image in raster pixel space
	double rXmin = (rasterExtent.xMin() - adfGeoTransform[0]) / adfGeoTransform[1];
	double rXmax = (rasterExtent.xMax() - adfGeoTransform[0]) / adfGeoTransform[1];
	double rYmin = (rasterExtent.yMin() - adfGeoTransform[3]) / adfGeoTransform[5];
	double rYmax = (rasterExtent.yMax() - adfGeoTransform[3]) / adfGeoTransform[5];
	int rXSize = abs(static_cast<int>(rXmax - rXmin));
	int rYSize = abs(static_cast<int>(rYmax - rYmin));
	
	// get dimensions of clipped raster image in device coordinate space
	QgsPoint topLeft = cXf->transform(rasterExtent.xMin(), rasterExtent.yMax());
	QgsPoint bottomRight = cXf->transform(rasterExtent.xMax(), rasterExtent.yMin());
	int lXSize = bottomRight.xToInt() - topLeft.xToInt();
	int lYSize = bottomRight.yToInt() - topLeft.yToInt();
	
	// loop through raster bands
	// a band can have color values that correspond to colors in a palette
	// or it can contain the red, green or blue value for an rgb image
	// HLS, CMYK, or RGB alpha bands are silently ignored for now
	for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) {
		GDALRasterBand  *gdalBand = gdalDataset->GetRasterBand( i );
		double myNoDataDouble = gdalBand->GetNoDataValue();
                //std::cout << "Nodata value for band " << i << " is " << myNoDataDouble << "\n" << std::endl;
		//std::cout << "gdalBand->GetOverviewCount(): " << gdalBand->GetOverviewCount() <<std::endl;
		
		// make sure we don't exceed size of raster
		rXSize = rXSize <? gdalBand->GetXSize();
		rYSize = rYSize <? gdalBand->GetYSize();		
		
		// read entire clipped area of raster band
		// treat scandata as a pseudo-multidimensional array
		// RasterIO() takes care of scaling down image
		uint *scandata = (uint*) CPLMalloc(sizeof(uint)*lXSize * sizeof(uint)*lYSize);
		CPLErr result = gdalBand->RasterIO( 
				GF_Read, rXOff, rYOff, rXSize, rYSize, scandata, lXSize, lYSize, GDT_UInt32, 0, 0 );
							
		QString colorInterp = GDALGetColorInterpretationName(gdalBand->GetColorInterpretation());
                //std::cout << "Colour Interpretation for this band is : " << colorInterp << std::endl;
		if ( colorInterp == "Palette") {
			// print each point in scandata using color looked up in color table
			GDALColorTable *colorTable = gdalBand->GetColorTable();
			
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {
					const GDALColorEntry *colorEntry = GDALGetColorEntry(colorTable, scandata[y*lXSize + x]);
                                        //dont draw this point if it is no data !
                                        if (myNoDataDouble != scandata[y*lXSize + x])
                                        {
					  p->setPen(QColor(colorEntry->c1, colorEntry->c2, colorEntry->c3));
					  p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
                                        }
				}
			}			
		} else if ( colorInterp == "Red" ) {
			// print each point in scandata as the red part of an rgb value
			// this assumes that the red band will always be first
			// is that necessarily the case?
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {		
                                        //dont draw this point if it is no data !
                                        if (myNoDataDouble != scandata[y*lXSize + x])
                                        {			
					        p->setPen(QColor(scandata[y*lXSize + x], 0, 0));
					        p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
                                         }
				}
			}			
		} else if ( colorInterp == "Green" ) {
			// print each point in scandata as the green part of an rgb value
			p->setRasterOp(Qt::XorROP);
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {		
                                        //dont draw this point if it is no data !
                                        if (myNoDataDouble != scandata[y*lXSize + x])
                                        {			
					        p->setPen(QColor(0, scandata[y*lXSize + x], 0));
					        p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
                                         }
				}
			}
			p->setRasterOp(Qt::CopyROP);
		} else if ( colorInterp == "Blue" ) {
			// print each point in scandata as the blue part of an rgb value
			p->setRasterOp(Qt::XorROP);
			for (int y = 0; y < lYSize; y++) {
				for (int x =0; x < lXSize; x++) {		
                                        //dont draw this point if it is no data !
                                        if (myNoDataDouble != scandata[y*lXSize + x])
                                        {			
					        p->setPen(QColor(0, 0, scandata[y*lXSize + x]));
					        p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
                                         }
				}
			}
			p->setRasterOp(Qt::CopyROP);
		} else if ( colorInterp == "Gray" || colorInterp == "Undefined" ) {
                       if (!showGrayAsColorFlag)
                       {
                              //ensure we are not still xoring
                              p->setRasterOp(Qt::CopyROP);  
			      // print each point in scandata with equal parts R, G ,B o make it show as gray
			      for (int y = 0; y < lYSize; y++) {
				    for (int x =0; x < lXSize; x++) {	
                                            int myGrayValInt=scandata[y*lXSize + x];			
                                        //dont draw this point if it is no data !
                                        if (myNoDataDouble != myGrayValInt)
                                        {				
					    p->setPen(QColor(myGrayValInt, myGrayValInt, myGrayValInt));
					    p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);
                                         }
				    }
			     }
                       }
                       else { //show the gray layer as pseudocolor
                         //ensure we are not still xoring

                         p->setRasterOp(Qt::CopyROP); 
                         int myRedInt=0;
                         int myGreenInt=0;
                         int myBlueInt=0;
                         //calculate the adjusted matrix stats - which come into affect if the user has chosen
                         RasterBandStats myAdjustedRasterBandStats = rasterStatsMap[colorInterp];
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
                         // print each point in scandata with equal parts R, G ,B o make it show as gray
                         for (int y = 0; y < lYSize; y++) {
                           for (int x =0; x < lXSize; x++) {	
                             int myDouble=scandata[y*lXSize + x];			
                             //dont draw this point if it is no data !

                             //double check that myDouble >= min and <= max
                             //this is relevant if we are plotting within stddevs
                             if ((myDouble < myAdjustedRasterBandStats.minValDouble ) && (myDouble != myAdjustedRasterBandStats.noDataDouble))
                             {
                               myDouble = myAdjustedRasterBandStats.minValDouble;
                             }
                             if ((myDouble > myAdjustedRasterBandStats.maxValDouble)   && (myDouble != myAdjustedRasterBandStats.noDataDouble))
                             {
                               myDouble = myAdjustedRasterBandStats.maxValDouble;
                             }      
                             if (myDouble==myAdjustedRasterBandStats.noDataDouble)
                             {
                                 //hardcoding to white for now
                                 myRedInt = 255;
                                 myBlueInt = 255;
                                 myGreenInt =255;          
                             }
                             else if(!invertHistogramFlag)
                             { 
                               //check if we are in the first class break
                               if ((myDouble >= myClassBreakMin1) &&  (myDouble < myClassBreakMax1) )
                               {
                                 myRedInt = 0;
                                 myBlueInt = 255;
                                 myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * (myDouble-myClassBreakMin1))*3);
                               }
                               //check if we are in the second class break
                               else if ((myDouble >= myClassBreakMin2) &&  (myDouble < myClassBreakMax2) )
                               {
                                 myRedInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3);
                                 myBlueInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3));
                                 myGreenInt = 255;
                               }
                               //otherwise we must be in the third classbreak
                               else
                               {
                                 myRedInt = 255;
                                 myBlueInt = 0;
                                 myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin3)/1)*3)));
                               }
                             }
                             else  //invert histogram toggle is on
                             {
                               //check if we are in the first class break      
                               if ((myDouble >= myClassBreakMin1) &&  (myDouble < myClassBreakMax1) )
                               {
                                 myRedInt = 255;
                                 myBlueInt = 0;
                                 myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin1)/1)*3));
                               }
                               //check if we are in the second class break
                               else if ((myDouble >= myClassBreakMin2) &&  (myDouble < myClassBreakMax2) )
                               {
                                 myRedInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3));
                                 myBlueInt = static_cast<int>(((255/myAdjustedRasterBandStats.rangeDouble) * ((myDouble-myClassBreakMin2)/1))*3);
                                 myGreenInt = 255;
                               }
                               //otherwise we must be in the third classbreak
                               else 
                               {
                                 myRedInt = 0;
                                 myBlueInt = 255;
                                 myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats.rangeDouble) * (myDouble-myClassBreakMin3))*3));
                               }   
                             }
                             //uncomment the next line to write each pixel value to a file
                             //can be useful if you want to make an export to single grid routine
                             //int myResultInt =fprintf ( myFilePointer, "%f  ",myInt);
                             p->setPen(QColor(myRedInt, myGreenInt, myBlueInt));
                             p->drawPoint(topLeft.xToInt() + x, topLeft.yToInt() + y);


                           }
                         }
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
    QString colorInterp = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    return rasterStatsMap[colorInterp];
  }
}
/** Return the statistics for a given band name */
const  RasterBandStats QgsRasterLayer::getRasterBandStats(QString theBandName) 
{        
    return rasterStatsMap[theBandName];
}

/** Private method to calculate statistics fro each band. Populates rasterStatsMap. */
void QgsRasterLayer::calculateStats()
{
  for (int i = 1; i <= gdalDataset->GetRasterCount(); i++) 
  {
    RasterBandStats myRasterBandStats; 
    GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( i );
    QString colorInterp = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
    //avoid collecting stats for rgb images for now
    if (colorInterp=="Gray" || colorInterp=="Undefined") 
    {
      // get the dimensions of the raster
      int myColsInt = myGdalBand->GetXSize();
      int myRowsInt = myGdalBand->GetYSize();

      //allocate a buffer to hold one row of ints
      int myAllocationSizeInt = sizeof(uint)*myColsInt; 
      uint * myScanlineAllocInt = (uint*) CPLMalloc(myAllocationSizeInt);
      myRasterBandStats.elementCountInt=myColsInt*myRowsInt;
      myRasterBandStats.noDataDouble=myGdalBand->GetNoDataValue();
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
      //add this band to the class stats map
      rasterStatsMap[colorInterp]=myRasterBandStats;
    }//end of band loop
  }//end of gray / undefined raster color interp check
}
