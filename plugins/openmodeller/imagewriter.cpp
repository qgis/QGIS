#include "imagewriter.h"

#include <qimage.h>
#include <qmap.h>


#include <stdio.h>
#include <stdlib.h>
#include <iostream>

ImageWriter::ImageWriter()
{
}


ImageWriter::~ImageWriter()
{
}





/** Private method to calculate statistics for each band. Populates rasterStatsMap. */
void ImageWriter::calculateStats(RasterBandStats * theRasterBandStats,GDALDataset * gdalDataset)
{
        std::cout << "Calculating statistics..." << std::endl;
	GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( 1 );
	QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
	theRasterBandStats->bandName=myColorInterpretation;
	theRasterBandStats->bandNo=1;
	// get the dimensions of the raster
	int myColsInt = myGdalBand->GetXSize();
	int myRowsInt = myGdalBand->GetYSize();

	theRasterBandStats->elementCountInt=myColsInt*myRowsInt;
	theRasterBandStats->noDataDouble=myGdalBand->GetNoDataValue();
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
			if (myDouble != theRasterBandStats->noDataDouble )
			{
				if (myFirstIterationFlag)
				{
					//this is the first iteration so initialise vars
					myFirstIterationFlag=false;
					theRasterBandStats->minValDouble=myDouble;
					theRasterBandStats->maxValDouble=myDouble;
				} //end of true part for first iteration check
				else
				{
					//this is done for all subsequent iterations
					if (myDouble < theRasterBandStats->minValDouble)
					{
						theRasterBandStats->minValDouble=myDouble;
					}
					if (myDouble > theRasterBandStats->maxValDouble)
					{
					//	printf ("Maxval updated to %f\n",myDouble);
						theRasterBandStats->maxValDouble=myDouble;
					}
					//only increment the running total if it is not a nodata value
					if (myDouble != theRasterBandStats->noDataDouble)
					{
						theRasterBandStats->sumDouble += myDouble;
						++theRasterBandStats->elementCountInt;
					}
				} //end of false part for first iteration check
			} //end of nodata chec
		} //end of column wise loop
	} //end of row wise loop
	//
	//end of first pass through data now calculate the range
	theRasterBandStats->rangeDouble = theRasterBandStats->maxValDouble-theRasterBandStats->minValDouble;
	//calculate the mean
	theRasterBandStats->meanDouble = theRasterBandStats->sumDouble / theRasterBandStats->elementCountInt;
	//for the second pass we will get the sum of the squares / mean
	for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
	{
		CPLErr myResult = myGdalBand->RasterIO(
				GF_Read, 0, myCurrentRowInt, myColsInt, 1, myScanlineAllocInt, myColsInt, 1, GDT_UInt32, 0, 0 );
		for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
		{
			//get the nth element from the current row
			double myDouble=myScanlineAllocInt[myCurrentColInt];
			theRasterBandStats->sumSqrDevDouble += static_cast<double>(pow(myDouble - theRasterBandStats->meanDouble,2));
		} //end of column wise loop
	} //end of row wise loop
	//divide result by sample size - 1 and get square root to get stdev
	theRasterBandStats->stdDevDouble = static_cast<double>(sqrt(theRasterBandStats->sumSqrDevDouble /
				(theRasterBandStats->elementCountInt - 1)));
	CPLFree(myScanlineAllocInt);
	//printf("CalculateStats::\n");
	//std::cout << "Band Name   : " << theRasterBandStats->bandName << std::endl;
	//printf("Band No   : %i\n",theRasterBandStats->bandNo);
	//printf("Band min  : %f\n",theRasterBandStats->minValDouble);
	//printf("Band max  : %f\n",theRasterBandStats->maxValDouble);
	//printf("Band range: %f\n",theRasterBandStats->rangeDouble);
	//printf("Band mean : %f\n",theRasterBandStats->meanDouble);
	//printf("Band sum : %f\n",theRasterBandStats->sumDouble);
	return ;
}

void ImageWriter::writeImage(QString theInputFileString, QString theOutputFileString)
{
	//printf("Started with input :  %s output: %s \n",theInputFileString,theOutputFileString);
	GDALAllRegister();
	GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theInputFileString, GA_ReadOnly );
	if ( gdalDataset == NULL )
	{
		//valid = FALSE;
		return ;
	}
	int myXDimInt = gdalDataset->GetRasterXSize();
	int myYDimInt = gdalDataset->GetRasterYSize();
	printf("Raster is %i x %i cells\n", myXDimInt, myYDimInt);
	GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( 1 );
	// RasterIO() takes care of scaling down image
	uint *myGdalScanData = (uint*) CPLMalloc(sizeof(uint)*myXDimInt * sizeof(uint)*myYDimInt);
	CPLErr myResultCPLerr = myGdalBand->RasterIO(GF_Read, 0, 0, myXDimInt, myYDimInt, myGdalScanData, myXDimInt, myYDimInt, GDT_UInt32, 0, 0 );
	QImage myQImage=QImage(myXDimInt,myYDimInt,32);
	myQImage.setAlphaBuffer(true);
	uint stdDevsToPlotDouble=0;
	bool invertHistogramFlag=false;
	int transparencyLevelInt=255;
	//calculate the adjusted matrix stats - which come into affect if the user has chosen

	RasterBandStats * myAdjustedRasterBandStats = new RasterBandStats();
	calculateStats(myAdjustedRasterBandStats, gdalDataset);

	myAdjustedRasterBandStats->noDataDouble=0;//hard coding for now
	//to histogram stretch to a given number of std deviations
	//see if we are using histogram stretch using stddev and plot only within the selected number of deviations if we are
	//cout << "stdDevsToPlotDouble: " << cboStdDev->currentText() << " converted to " << stdDevsToPlotDouble << endl;
	if (stdDevsToPlotDouble > 0)
	{
		//work out how far on either side of the mean we should include data
		float myTotalDeviationDouble = stdDevsToPlotDouble * myAdjustedRasterBandStats->stdDevDouble;
		//printf("myTotalDeviationDouble: %i\n" , myTotalDeviationDouble );
		//adjust min and max accordingly
		//only change min if it is less than mean  -  (n  x  deviations)
		if (myAdjustedRasterBandStats->minValDouble < (myAdjustedRasterBandStats->meanDouble-myTotalDeviationDouble))
		{
			myAdjustedRasterBandStats->minValDouble=(myAdjustedRasterBandStats->meanDouble-myTotalDeviationDouble);
			//cout << "Adjusting minValDouble to: " << myAdjustedRasterBandStats->minValDouble << endl;
		}
		//only change max if it is greater than mean  +  (n  x  deviations)
		if (myAdjustedRasterBandStats->maxValDouble > (myAdjustedRasterBandStats->meanDouble + myTotalDeviationDouble))
		{
			myAdjustedRasterBandStats->maxValDouble=(myAdjustedRasterBandStats->meanDouble+myTotalDeviationDouble);
			//cout << "Adjusting maxValDouble to: " << myAdjustedRasterBandStats->maxValDouble << endl;
		}
		//update the range
		myAdjustedRasterBandStats->rangeDouble = myAdjustedRasterBandStats->maxValDouble-myAdjustedRasterBandStats->minValDouble;
	}

	printf("Main ::\n");
	std::cout << "Band Name   : " << myAdjustedRasterBandStats->bandName << std::endl;
	printf("Band No   : %i\n",myAdjustedRasterBandStats->bandNo);
	printf("Band min  : %f\n",myAdjustedRasterBandStats->minValDouble);
	printf("Band max  : %f\n",myAdjustedRasterBandStats->maxValDouble);
	printf("Band range: %f\n",myAdjustedRasterBandStats->rangeDouble);
	printf("Band mean : %f\n",myAdjustedRasterBandStats->meanDouble);
	printf("Band sum : %f\n",myAdjustedRasterBandStats->sumDouble);

	//double sumSqrDevDouble; //used to calculate stddev
	//double stdDevDouble;
	//double sumDouble;
	//int elementCountInt;
	//double noDataDouble;

	//set up the three class breaks for pseudocolour mapping
	double myBreakSizeDouble = myAdjustedRasterBandStats->rangeDouble / 3;
	double myClassBreakMin1 = myAdjustedRasterBandStats->minValDouble;
	double myClassBreakMax1 = myAdjustedRasterBandStats->minValDouble + myBreakSizeDouble;
	double myClassBreakMin2 = myClassBreakMax1;
	double myClassBreakMax2 = myClassBreakMin2 + myBreakSizeDouble;
	double myClassBreakMin3 = myClassBreakMax2;
	double myClassBreakMax3 = myAdjustedRasterBandStats->maxValDouble;

	printf ("ClassBreak size : %f \n",myBreakSizeDouble);
	printf ("ClassBreak 1 : %f - %f\n",myClassBreakMin1,myClassBreakMax1);
	printf ("ClassBreak 2 : %f - %f\n",myClassBreakMin2,myClassBreakMax2);
	printf ("ClassBreak 3 : %f - %f\n",myClassBreakMin3,myClassBreakMax3);

	int myRedInt=0;
	int myGreenInt=0;
	int myBlueInt=0;
	for (int myColumnInt = 0; myColumnInt < myYDimInt; myColumnInt++)
	{
		for (int myRowInt =0; myRowInt < myXDimInt; myRowInt++)
		{
			int myInt=myGdalScanData[myColumnInt*myXDimInt + myRowInt];
			//dont draw this point if it is no data !
			//double check that myInt >= min and <= max
			//this is relevant if we are plotting within stddevs
			if ((myInt < myAdjustedRasterBandStats->minValDouble ) && (myInt != myAdjustedRasterBandStats->noDataDouble))
			{
				myInt = static_cast<int>(myAdjustedRasterBandStats->minValDouble);
			}
			if ((myInt > myAdjustedRasterBandStats->maxValDouble)   && (myInt != myAdjustedRasterBandStats->noDataDouble))
			{
				myInt = static_cast<int>(myAdjustedRasterBandStats->maxValDouble);
			}
			if (myInt==myAdjustedRasterBandStats->noDataDouble)
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
					myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats->rangeDouble) * (myInt-myClassBreakMin1))*3);
				}
				//check if we are in the second class break
				else if ((myInt >= myClassBreakMin2) &&  (myInt < myClassBreakMax2) )
				{
					myRedInt = static_cast<int>(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin2)/1))*3);
					myBlueInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin2)/1))*3));
					myGreenInt = 255;
				}
				//otherwise we must be in the third classbreak
				else
				{
					myRedInt = 255;
					myBlueInt = 0;
					myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin3)/1)*3)));
				}
			}
			else  //invert histogram toggle is on
			{
				//check if we are in the first class break
				if ((myInt >= myClassBreakMin1) &&  (myInt < myClassBreakMax1) )
				{
					myRedInt = 255;
					myBlueInt = 0;
					myGreenInt = static_cast<int>(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin1)/1)*3));
				}
				//check if we are in the second class break
				else if ((myInt >= myClassBreakMin2) &&  (myInt < myClassBreakMax2) )
				{
					myRedInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin2)/1))*3));
					myBlueInt = static_cast<int>(((255/myAdjustedRasterBandStats->rangeDouble) * ((myInt-myClassBreakMin2)/1))*3);
					myGreenInt = 255;
				}
				//otherwise we must be in the third classbreak
				else
				{
					myRedInt = 0;
					myBlueInt = 255;
					myGreenInt = static_cast<int>(255-(((255/myAdjustedRasterBandStats->rangeDouble) * (myInt-myClassBreakMin3))*3));
				}
			}
			myQImage.setPixel( myRowInt, myColumnInt, qRgba( myRedInt, myGreenInt, myBlueInt, transparencyLevelInt ));
		}
	}
	//draw with the experimental transaparency support
	CPLFree(myGdalScanData);
	GDALClose(gdalDataset);
	printf("Saving image...\n");
	myQImage.save(theOutputFileString,"PNG");
	return ;
}


