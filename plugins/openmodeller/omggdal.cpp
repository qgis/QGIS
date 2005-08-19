/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   tim@linfiniti.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "omggdal.h"

#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qdir.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <gdal_priv.h>


OmgGdal::OmgGdal()
{}

OmgGdal::~OmgGdal()
{}

const QString OmgGdal::Gdal2Ascii(const QString theFileName)
{
  std::cout << "Running gdal to ascii conversion..." << std::endl;
  QFileInfo myFileInfo(theFileName);
  QString myExt("asc");
  QString myOutFileName(QDir::convertSeparators(myFileInfo.dirPath(true)+"/"+myFileInfo.baseName() + "." + myExt));
  QFile myFile( myOutFileName );
  if ( !myFile.open( IO_WriteOnly ) )
  {
    printf("Opening output file for write failed");
    return QString("");
  }
  QTextStream myStream( &myFile );
  GDALAllRegister();
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
  if ( gdalDataset == NULL )
  {
    std::cout <<  "Error couldn't open file: " << theFileName << std::endl;
    return QString("");
  }

  //Write the ascii headers
  myStream << getAsciiHeader(theFileName);

  //assume to be working with first band in dataset only
  GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( 1 );
  //find out the name of the band if any
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  // get the dimensions of the raster
  int myColsInt = myGdalBand->GetXSize();
  int myRowsInt = myGdalBand->GetYSize();
  double myNullValue=myGdalBand->GetNoDataValue();
  //allocate a buffer to hold one row of ints
  int myAllocationSizeInt = sizeof(uint)*myColsInt;
  uint * myScanlineAllocInt = (uint*) CPLMalloc(myAllocationSizeInt);
  for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
  {
    //get a scanline
    CPLErr myResult = myGdalBand->RasterIO(
                        GF_Read, 0,
                        myCurrentRowInt,
                        myColsInt,
                        1,
                        myScanlineAllocInt,
                        myColsInt,
                        1,
                        GDT_UInt32,
                        0,
                        0 );
    for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
    {
      //get the nth element from the current row
      double myDouble=myScanlineAllocInt[myCurrentColInt];
      myStream << myDouble << " "; //pixel value
    } //end of column wise loop
    myStream << "\r\n"; //dos style new line
  } //end of row wise loop
  CPLFree(myScanlineAllocInt);
  myFile.close();
  std::cout << "The output ascii file is: " << myOutFileName << std::endl;
  return myOutFileName;
}

const QString OmgGdal::getAsciiHeader(const QString theFileName)
{
  GDALAllRegister();
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
  if ( gdalDataset == NULL )
  {
    return QString("");
  }

  //get dimensions and no data value
  int myColsInt = gdalDataset->GetRasterXSize();
  int myRowsInt = gdalDataset->GetRasterYSize();
  double myNullValue=gdalDataset->GetRasterBand(1)->GetNoDataValue();
  //get the geotransform stuff from gdal
  double myTransform[6];
  if (gdalDataset->GetGeoTransform(myTransform) != CE_None)
  {
    std::cout << "Failed to get geo transform from GDAL, aborting" << std::endl;
    GDALClose(gdalDataset);
    return QString("");
  }
  else

  {
    GDALClose(gdalDataset);
  }

  QString myHeader;

  myHeader =  "NCOLS "        + QString::number(myColsInt) + "\r\n";
  myHeader += "NROWS "        + QString::number(myRowsInt) + "\r\n";
  myHeader += "XLLCORNER "    + QString::number(myTransform[0]) +  "\r\n";;
  //negative y is bodged for now
  myHeader += "YLLCORNER -"   + QString::number(myTransform[3]) +  "\r\n"; 
  myHeader += "CELLSIZE "     + QString::number(myTransform[1]) +  "\r\n";
  myHeader += "NODATA_VALUE " + QString::number(myNullValue) +  "\r\n";

  return myHeader;
}