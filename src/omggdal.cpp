/**************************************************************************
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
/**
 * This class is a wrapper for gdal functions to give them a more qt like interface.
 * 
 * @author Tim Sutton
 * 
 * @note This class was written for the openmodeller gui, and will be periodically
 * synchronised with the code from that project, hence the different naming convention.
 */
#include "omggdal.h"

#if QT_VERSION < 0x040000

#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qregexp.h>

#else

#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDir>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//gdal includes
#include <gdal_priv.h>
#include <cpl_string.h>
#include <gdal.h>
#include <gdal_alg.h>
#include <cpl_conv.h>
#include <gdalwarper.h>

//ogr includes (used e.g. for contouring output)
#include <cpl_conv.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

OmgGdal::OmgGdal() : QObject()
{
  GDALAllRegister();
  OGRRegisterAll();
}

OmgGdal::~OmgGdal()
{}


const QString  OmgGdal::getWorldFile(const QString theFileName)
{

#if QT_VERSION < 0x040000
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
#else
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.toLocal8Bit(), GA_ReadOnly );
#endif
  if ( gdalDataset == NULL )
  {
    return QString("");
  }
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
  myHeader += "Pixel XDim " + QString::number(myTransform[1]) + "\r\n";
  myHeader += "Rot 0 \r\n";
  myHeader += "Rot 0 \r\n";
  myHeader += "Pixel YDim " + QString::number(myTransform[5]) + "\r\n";
  myHeader += "Origin X   " + QString::number(myTransform[0]) + "\r\n";
  myHeader += "Origin Y   " + QString::number(myTransform[3]) + "\r\n";
  return myHeader;
}

const QString  OmgGdal::getAsciiHeader(const QString theFileName)
{
#if QT_VERSION < 0x040000
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
#else
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.toLocal8Bit(), GA_ReadOnly );
#endif
  if ( gdalDataset == NULL )
  {
    return QString("");
  }

  //get dimesnions and no data value
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


/*
Builds the list of file filter strings to later be used by
QgisApp::addRasterLayer()
 
We query GDAL for a list of supported raster formats; we then build
a list of file filter strings from that list.  We return a string
that contains this list that is suitable for use in a a
QFileDialog::getOpenFileNames() call.
 
This method was filched from QGIS QgsRasterLayer class and tweaked a bit
*/
void OmgGdal::buildSupportedRasterFileFilter(QString & theFileFiltersString)
{
  // first get the GDAL driver manager

  GDALDriverManager *myGdalDriverManager = GetGDALDriverManager();

  if (!myGdalDriverManager)
  {
    std::cerr << "unable to get GDALDriverManager\n";
    return;                   // XXX good place to throw exception if we
  }                           // XXX decide to do exceptions

  // then iterate through all of the supported drivers, adding the
  // corresponding file filter

  GDALDriver *myGdalDriver;           // current driver

  char **myGdalDriverMetadata;        // driver metadata strings

  QString myGdalDriverLongName("");   // long name for the given driver
  QString myGdalDriverExtension("");  // file name extension for given driver
  QString myGdalDriverDescription;    // QString wrapper of GDAL driver description

  QStringList metadataTokens;   // essentially the metadata string delimited by '='

  QString catchallFilter;       // for Any file(*.*), but also for those
  // drivers with no specific file
  // filter

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  // Note that file name extension strings are of the form
  // "DMD_EXTENSION=.*".  We'll also store the long name of the
  // driver, which will be found in DMD_LONGNAME, which will have the
  // same form.

  for (int i = 0; i < myGdalDriverManager->GetDriverCount(); ++i)
  {
    myGdalDriver = myGdalDriverManager->GetDriver(i);

    Q_CHECK_PTR(myGdalDriver);

    if (!myGdalDriver)
    {
      qWarning("unable to get driver %d", i);
      continue;
    }
    // now we need to see if the driver is for something currently
    // supported; if not, we give it a miss for the next driver

    myGdalDriverDescription = myGdalDriver->GetDescription();

    // std::cerr << "got driver string " << myGdalDriver->GetDescription() << "\n";

    myGdalDriverMetadata = myGdalDriver->GetMetadata();

    // presumably we know we've run out of metadta if either the
    // address is 0, or the first character is null
    while (myGdalDriverMetadata && '\0' != myGdalDriverMetadata[0])
    {
#if QT_VERSION < 0x040000
      metadataTokens = QStringList::split("=",*myGdalDriverMetadata);
#else
      metadataTokens = QString(*myGdalDriverMetadata).split("=");
#endif
      // std::cerr << "\t" << *myGdalDriverMetadata << "\n";

      // XXX add check for malformed metadataTokens

      // Note that it's oddly possible for there to be a
      // DMD_EXTENSION with no corresponding defined extension
      // string; so we check that there're more than two tokens.

      if (metadataTokens.count() > 1)
      {
        if ("DMD_EXTENSION" == metadataTokens[0])
        {
          myGdalDriverExtension = metadataTokens[1];

        }
        else if ("DMD_LONGNAME" == metadataTokens[0])
        {
          myGdalDriverLongName = metadataTokens[1];

          // remove any superfluous (.*) strings at the end as
          // they'll confuse QFileDialog::getOpenFileNames()

          myGdalDriverLongName.remove(QRegExp("\\(.*\\)$"));
        }
      }
      // if we have both the file name extension and the long name,
      // then we've all the information we need for the current
      // driver; therefore emit a file filter string and move to
      // the next driver
      if (!(myGdalDriverExtension.isEmpty() || myGdalDriverLongName.isEmpty()))
      {
        // XXX add check for SDTS; in that case we want (*CATD.DDF)
        QString glob = "*." + myGdalDriverExtension;
#if QT_VERSION < 0x040000
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
#else
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif

        break;            // ... to next driver, if any.
      }

      ++myGdalDriverMetadata;

    }                       // each metadata item

    if (myGdalDriverExtension.isEmpty() && !myGdalDriverLongName.isEmpty())
    {
      // Then what we have here is a driver with no corresponding
      // file extension; e.g., GRASS.  In which case we append the
      // string to the "catch-all" which will match all file types.
      // (I.e., "*.*") We use the driver description intead of the
      // long time to prevent the catch-all line from getting too
      // large.

      // ... OTOH, there are some drivers with missing
      // DMD_EXTENSION; so let's check for them here and handle
      // them appropriately

      // USGS DEMs use "*.dem"
      if (myGdalDriverDescription.startsWith("USGSDEM"))
      {
        QString glob = "*.dem";
#if QT_VERSION < 0x040000
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
#else
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif
      }
      else if (myGdalDriverDescription.startsWith("DTED"))
      {
        // DTED use "*.dt0"
        QString glob = "*.dt0";
#if QT_VERSION < 0x040000
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
#else
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif
      }
      else if (myGdalDriverDescription.startsWith("MrSID"))
      {
        // MrSID use "*.sid"
        QString glob = "*.sid";
#if QT_VERSION < 0x040000
        theFileFiltersString += myGdalDriverLongName + " (" + glob.lower() + " " + glob.upper() + ");;";
#else
        theFileFiltersString += myGdalDriverLongName + " (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif
      }
      else
      {
        catchallFilter += QString(myGdalDriver->GetDescription()) + " ";
      }
    }

    // A number of drivers support JPEG 2000. Add it in for those.
    if (  myGdalDriverDescription.startsWith("MrSID")
          || myGdalDriverDescription.startsWith("ECW")
          || myGdalDriverDescription.startsWith("JPEG2000")
          || myGdalDriverDescription.startsWith("JP2KAK") )
    {
      QString glob = "*.jp2 *.j2k";
#if QT_VERSION < 0x040000
      theFileFiltersString += "JPEG 2000 (" + glob.lower() + " " + glob.upper() + ");;";
#else
      theFileFiltersString += "JPEG 2000 (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif
    }

    // A number of drivers support JPEG 2000. Add it in for those.
    if (  myGdalDriverDescription.startsWith("MrSID")
          || myGdalDriverDescription.startsWith("ECW")
          || myGdalDriverDescription.startsWith("JPEG2000")
          || myGdalDriverDescription.startsWith("JP2KAK") )
    {
      QString glob = "*.jp2 *.j2k";
#if QT_VERSION < 0x040000
      theFileFiltersString += "JPEG 2000 (" + glob.lower() + " " + glob.upper() + ");;";
#else
      theFileFiltersString += "JPEG 2000 (" + glob.toLower() + " " + glob.toUpper() + ");;";
#endif
    }

    myGdalDriverExtension = myGdalDriverLongName = "";  // reset for next driver

  }                           // each loaded GDAL driver

  // can't forget the default case
  theFileFiltersString += catchallFilter + "All other files (*)";
#ifdef QGISDEBUG
  std::cout << "Raster filter list built: " << theFileFiltersString.local8Bit() << std::endl;
#endif
}  // buildSupportedRasterFileFilter

const QString OmgGdal::contour(const QString theInputFile)
{
  GDALDatasetH hSrcDS;
  int i, b3D = FALSE, bNoDataSet = FALSE, bIgnoreNoData = FALSE;
  int myBandNumber = 1;
  double dfInterval = 1.0;
  double  dfNoData = 0.0;
  double dfOffset = 0.0;

  const char *pszDstFilename = "/tmp";
  const char *pszElevAttrib = NULL;
  QString myFormat = "ESRI Shapefile";
  double adfFixedLevels[1000];
  int    nFixedLevelCount = 0;

  //      Open source raster file.

  GDALRasterBandH hBand;

#if QT_VERSION < 0x040000
  hSrcDS = GDALOpen( theInputFile.local8Bit(), GA_ReadOnly );
#else
  hSrcDS = GDALOpen( theInputFile.toLocal8Bit(), GA_ReadOnly );
#endif
  if( hSrcDS == NULL )
  {
    emit error ("Unable to open source file");
  }

  hBand = GDALGetRasterBand( hSrcDS, myBandNumber );
  if( hBand == NULL )
  {
    CPLError( CE_Failure, CPLE_AppDefined,
              "Band %d does not exist on dataset.",
              myBandNumber );
  }
  dfNoData = GDALGetRasterNoDataValue( hBand, &bNoDataSet );
  //     Try to get a coordinate system from the raster.

  OGRSpatialReferenceH hSRS = NULL;

  const char *pszWKT = GDALGetProjectionRef( hBand );

  if( pszWKT != NULL && strlen(pszWKT) != 0 )
    hSRS = OSRNewSpatialReference( pszWKT );

  //      Create the outputfile
  OGRDataSourceH hDS;
#if QT_VERSION < 0x040000
  OGRSFDriverH hDriver = OGRGetDriverByName( myFormat.local8Bit() );
#else
  OGRSFDriverH hDriver = OGRGetDriverByName( myFormat.toLocal8Bit() );
#endif
  OGRFieldDefnH hFld;
  OGRLayerH hLayer;
  int nElevField = -1;

  if( hDriver == NULL )
  {
#if QT_VERSION < 0x040000    
    fprintf( stderr, "Unable to find format driver named %s.\n",
             myFormat.local8Bit().data() );
#else
    fprintf( stderr, "Unable to find format driver named %s.\n",
             myFormat.toLocal8Bit().data() );
#endif
    return QString();
  }

  hDS = OGR_Dr_CreateDataSource( hDriver, pszDstFilename, NULL );
  if( hDS == NULL )
    exit( 1 );

  hLayer = OGR_DS_CreateLayer( hDS, "contour", hSRS,
                               b3D ? wkbLineString25D : wkbLineString,
                               NULL );
  if( hLayer == NULL )
  {
    return QString();
  }


  hFld = OGR_Fld_Create( "ID", OFTInteger );
  OGR_Fld_SetWidth( hFld, 8 );
  OGR_L_CreateField( hLayer, hFld, FALSE );
  OGR_Fld_Destroy( hFld );

  if( pszElevAttrib )
  {
    hFld = OGR_Fld_Create( pszElevAttrib, OFTReal );
    OGR_Fld_SetWidth( hFld, 12 );
    OGR_Fld_SetPrecision( hFld, 3 );
    OGR_L_CreateField( hLayer, hFld, FALSE );
    OGR_Fld_Destroy( hFld );
    nElevField = 1;
  }
  // -------------------------------------------------------------------- */
  //      Invoke.                                                         */
  // -------------------------------------------------------------------- */
  CPLErr eErr;

  eErr = GDALContourGenerate( hBand, dfInterval, dfOffset,
                              nFixedLevelCount, adfFixedLevels,
                              bNoDataSet, dfNoData,
                              hLayer, 0, nElevField,
                              GDALTermProgress, NULL );
//                              progressCallback, NULL );

  OGR_DS_Destroy( hDS );
  GDALClose( hSrcDS );
}

const QString OmgGdal::convert(const QString theInputFile, const QString theOutputPath, const FileType theOutputFileType)
{
  QString myResult("");
  switch (theOutputFileType)
  {
  case GeoTiff :
    myResult=gdal2Tiff (theInputFile, theOutputPath);
    break;
  case ArcInfoAscii :
    myResult=gdal2Ascii (theInputFile, theOutputPath);
    break;
  default :
    emit error( tr("Output file type not supported!"));
    break;
  }
  return myResult;
}


//
// Private methods only below this point please!
//

const QString OmgGdal::gdal2Tiff(const QString theFileName, const QString theOutputPath)
{
  //Compile output filename from input file and output path
  QFileInfo myFileInfo(theFileName);
  QString myBaseString = theOutputPath +QString("/")+myFileInfo.baseName();  // excludes any extension

  QString myFileName(myBaseString+".tif");
  const QString myFormat = "GTiff";
  GDALDriver * mypDriver;
  char **papszMetadata;

#if QT_VERSION < 0x040000
  mypDriver = GetGDALDriverManager()->GetDriverByName(myFormat.local8Bit());
#else
  mypDriver = GetGDALDriverManager()->GetDriverByName(myFormat.toLocal8Bit());
#endif

  if( mypDriver == NULL )
  {
    emit error ("Unknown GDAL Driver!");
    return QString();
  }

  papszMetadata = mypDriver->GetMetadata();
  if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
  {
    printf( "Driver supports Create() method.\n" );
  }
  else
  {
    printf( "Driver does not support Create() method.\n" );
  }
  if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
  {
    printf( "Driver supports CreateCopy() method.\n" );
  }
  else
  {
    printf( "Driver %s does not support CreateCopy() method.\n" );
  }

#if QT_VERSION < 0x040000
  GDALDataset *mypSourceDataset =
    (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
  GDALDataset *mypDestinationDataset = mypDriver->CreateCopy( myFileName.local8Bit(),
                          mypSourceDataset, FALSE,
                          NULL, progressCallback, this );
#else
  GDALDataset *mypSourceDataset =
    (GDALDataset *) GDALOpen( theFileName.toLocal8Bit(), GA_ReadOnly );
  GDALDataset *mypDestinationDataset = mypDriver->CreateCopy( myFileName.toLocal8Bit(),
                          mypSourceDataset, FALSE,
                          NULL, progressCallback, this );
#endif

  if( mypDestinationDataset != NULL )
    delete mypDestinationDataset;

  return myFileName;

}

const QString OmgGdal::gdal2Ascii(const QString theFileName, const QString theOutputPath)
{
  //Compile output filename from input file and output path
  QFileInfo myFileInfo(theFileName);
  QString myBaseString = theOutputPath +QString("/")+myFileInfo.baseName();  // excludes any extension

  QString myFileName(myBaseString+".asc");
  QFile myFile( myFileName );
#if QT_VERSION < 0x040000
  if ( !myFile.open( IO_WriteOnly ) )
#else
  if ( !myFile.open( QIODevice::WriteOnly ) )
#endif
  {
    printf("Opening output file for write failed");
    return QString("");
  }
  else
  {
#if QT_VERSION < 0x040000
    qDebug("Writing " + myFileName.local8Bit());
#else
    qDebug("Writing " + myFileName.toLocal8Bit());
#endif
  }
  QTextStream myStream( &myFile );
  GDALAllRegister();
#if QT_VERSION < 0x040000
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.local8Bit(), GA_ReadOnly );
#else
  GDALDataset  *gdalDataset = (GDALDataset *) GDALOpen( theFileName.toLocal8Bit(), GA_ReadOnly );
#endif
  if ( gdalDataset == NULL )
  {
    return QString("");
  }

  //assume to be working with first band in dataset only
  GDALRasterBand  *myGdalBand = gdalDataset->GetRasterBand( 1 );
  //find out the name of the band if any
  QString myColorInterpretation = GDALGetColorInterpretationName(myGdalBand->GetColorInterpretation());
  // get the dimensions of the raster
  int myColsInt = myGdalBand->GetXSize();
  int myRowsInt = myGdalBand->GetYSize();
  //write the header to the file
  myStream << getAsciiHeader(theFileName);
  //allocate a buffer to hold one row of ints
  int myAllocationSizeInt = sizeof(uint)*myColsInt;
  uint * myScanlineAllocInt = (uint*) CPLMalloc(myAllocationSizeInt);
  //for the second pass we will get the sum of the squares / mean
  for (int myCurrentRowInt=0; myCurrentRowInt < myRowsInt;myCurrentRowInt++)
  {
    //get a scanline
    CPLErr myResult = myGdalBand->RasterIO(
                        GF_Read, 0, myCurrentRowInt, myColsInt,
                        1, myScanlineAllocInt, myColsInt,
                        1, GDT_UInt32, 0, 0 );
    if (myResult==CE_Failure)
    {
      std::cout << "Error reading scanline from gdal! Aborting ascii conversion" << std::endl;
      CPLFree(myScanlineAllocInt);
      myFile.close();
      return ("");
    }
    for (int myCurrentColInt=0; myCurrentColInt < myColsInt; myCurrentColInt++)
    {
      //get the nth element from the current row
      double myDouble=myScanlineAllocInt[myCurrentColInt];
      myStream << myDouble << " "; //pixel value
    } //end of column wise loop
    myStream << "\r\n"; //dos style new line
    showProgress (myCurrentRowInt,myRowsInt);
  } //end of row wise loop
  CPLFree(myScanlineAllocInt);
  myFile.close();
  return myFileName;
}





void OmgGdal::showProgress (int theProgress,int theMaximum)
{
  emit updateProgress (theProgress,theMaximum);
}

int progressCallback( double dfComplete, const char * pszMessage,
                      void * pProgressArg)
{
  static double dfLastComplete = -1.0;

  OmgGdal * mypOmgGdal = (OmgGdal *) pProgressArg;

  if( dfLastComplete > dfComplete )
  {
    if( dfLastComplete >= 1.0 )
      dfLastComplete = -1.0;
    else
      dfLastComplete = dfComplete;
  }

  if( floor(dfLastComplete*10) != floor(dfComplete*10) )
  {
    int    nPercent = (int) floor(dfComplete*100);

    if( nPercent == 0 && pszMessage != NULL )
    {
      fprintf( stdout, "%s:", pszMessage );
    }

    if( nPercent == 100 )
    {
      fprintf( stdout, "%d - done.\n", (int) floor(dfComplete*100) );
      mypOmgGdal->showProgress(100,100);
    }
    else
    {
      int myProgress = (int) floor(dfComplete*100);
      fprintf( stdout, "%d.", myProgress);
      mypOmgGdal->showProgress(myProgress,100);
      fflush( stdout );
    }
  }
  else if( floor(dfLastComplete*30) != floor(dfComplete*30) )
  {
    fprintf( stdout, "." );
    fflush( stdout );
  }

  dfLastComplete = dfComplete;

  return TRUE;
}
