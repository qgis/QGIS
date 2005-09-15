/***************************************************************************
                          qgsvectorfilewriter.cpp  
                          generic vector file writer 
                             -------------------
    begin                : Sat Jun 16 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim at linfiniti.com
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
#include "qgsvectorfilewriter.h"

//#include <stdio.h>
//#include <qtextstream.h>
#include <iostream>
//#include <qfileinfo.h>
#include <qstring.h>

// Includes for ogr shaprewriting
//#include "gdal.h"
//#include "gdal_alg.h"
//#include "cpl_conv.h"
#include "ogr_srs_api.h"
#ifndef WIN32
#include <netinet/in.h>
#endif

QgsVectorFileWriter::QgsVectorFileWriter(QString theOutputFileName, QgsVectorLayer * theVectorLayer)
{
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName.local8Bit() << 
      " and vector layer : " << theVectorLayer->getLayerID().local8Bit() << std::endl;
  //const char *mOutputFormat = "ESRI Shapefile";
  mOutputFormat = "ESRI Shapefile";
  //const char *theOutputFileName = "ogrtest.shp";
  mOutputFileName = theOutputFileName;

  //
  // We should retrieve the geometry type from the vector layer!
  //



  mInitialisedFlag = false;
  
}

QgsVectorFileWriter::QgsVectorFileWriter(QString theOutputFileName, OGRwkbGeometryType theGeometryType)
{
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName.local8Bit() <<  " and no input vector layer "  << std::endl;
  mOutputFormat = "ESRI Shapefile"; //hard coded for now!
  mOutputFileName = theOutputFileName; 
  mGeometryType = theGeometryType;
  mInitialisedFlag = false;
}

QgsVectorFileWriter::~QgsVectorFileWriter()
{
  OGR_DS_Destroy( mDataSourceHandle );
  //GDALClose( hSrcDS );
}

bool QgsVectorFileWriter::initialise()
{
  // Valid OGRwkbGeometryType as defined in ogr_core.h
  //    wkbUnknown = 0,             /* non-standard */
  //    wkbPoint = 1,               /* rest are standard WKB type codes */
  //    wkbLineString = 2,
  //    wkbPolygon = 3,
  //    wkbMultiPoint = 4,
  //    wkbMultiLineString = 5,
  //    wkbMultiPolygon = 6,
  //    wkbGeometryCollection = 7,
  //    wkbNone = 100,              /* non-standard, for pure attribute records */
  //    wkbLinearRing = 101,        /* non-standard, just for createGeometry() */
  //    wkbPoint25D = 0x80000001,   /* 2.5D extensions as per 99-402 */
  //    wkbLineString25D = 0x80000002,
  //    wkbPolygon25D = 0x80000003,
  //    wkbMultiPoint25D = 0x80000004,
  //    wkbMultiLineString25D = 0x80000005,
  //    wkbMultiPolygon25D = 0x80000006,
  //    wkbGeometryCollection25D = 0x80000007

  //
  // The following stuff just sets up the shapefile - without writing any data to it!
  //

  //assume failure
  mInitialisedFlag=false; 
  
  OGRRegisterAll();
  OGRSFDriverH myDriverHandle = OGRGetDriverByName( mOutputFormat );

  if( myDriverHandle == NULL )
  {
    std::cout << "Unable to find format driver named " << mOutputFormat.local8Bit() << std::endl;
    return false;
  }

  mDataSourceHandle = OGR_Dr_CreateDataSource( myDriverHandle, mOutputFileName, NULL );
  if( mDataSourceHandle == NULL )
  {
    std::cout << "Datasource handle is null! " << std::endl;
    return false;
  }

  //define the spatial ref system
  OGRSpatialReferenceH mySpatialReferenceSystemHandle = NULL;

  QgsSpatialRefSys mySpatialRefSys;
  mySpatialRefSys.validate();
  char* WKT;
  QString myWKT = NULL;
  if(mySpatialRefSys.toOgrSrs().exportToWkt(&WKT)==OGRERR_NONE)
  {
#ifdef QGISDEBUG
      qWarning("export to WKT successful****************************************************************************************************");
#endif
      myWKT=WKT;
#ifdef QGISDEBUG
      qWarning("WKT is:WKT "+myWKT);
#endif    
  }
  else
  {
#ifdef QGISDEBUG
      qWarning("export to WKT failed*******************************************************************************************************3");
#endif      
  }

 
  //sample below shows how to extract srs from a raster
  //    const char *myWKT = GDALGetProjectionRef( hBand );


  if( !myWKT.isEmpty() )
  {
    mySpatialReferenceSystemHandle = OSRNewSpatialReference( myWKT );
  }
  //change 'contour' to something more useful!
#ifdef QGISDEBUG
  qWarning("mOutputFileName: "+mOutputFileName);
#endif //QGISDEBUG

#ifdef WIN32 
  QString outname=mOutputFileName.mid(mOutputFileName.findRev("\\")+1,mOutputFileName.length());
#else
  QString outname=mOutputFileName.mid(mOutputFileName.findRev("/")+1,mOutputFileName.length());
#endif
  

#ifdef QGISDEBUG
  qWarning("outname: "+outname);
#endif //QGISDEBUG

  mLayerHandle = OGR_DS_CreateLayer( mDataSourceHandle, outname, 
  mySpatialReferenceSystemHandle, mGeometryType, NULL );
  
  if( mLayerHandle == NULL )
  {
    std::cout << "Error layer handle is null!" << std::endl;
    return false;
  }
  else
  {
    std::cout << "File handle created!" << std::endl;
  }

  //let other methods know we have an initialised file
  mInitialisedFlag=true; 
  return true;
}

bool QgsVectorFileWriter::createField(QString theName, OGRFieldType theType, int theWidthInt, int thePrecisionInt)
{
  if (!mInitialisedFlag)
  {
    return false;
  }
  OGRFieldDefnH myFieldDefinitionHandle;

  //
  // OGRFieldType types as defined in  ogr_core.h :
  // 
  //        OFTInteger = 0,
  //        OFTIntegerList = 1,
  //        OFTReal = 2,
  //        OFTRealList = 3,
  //        OFTString = 4,
  //        OFTStringList = 5,
  //        OFTWideString = 6,
  //        OFTWideStringList = 7,
  //        OFTBinary = 8
  //
  myFieldDefinitionHandle = OGR_Fld_Create( theName, theType );
  OGR_Fld_SetWidth( myFieldDefinitionHandle, theWidthInt );
  //
  // Set the precision where applicable!
  //
  switch (theType)
  {
      case OFTInteger:
          //do nothing  (has no precision to set)
          break;
      case OFTIntegerList:
          //XXX Find out how this is used!!!!!!!
          break;
      case OFTReal:
          OGR_Fld_SetPrecision( myFieldDefinitionHandle, thePrecisionInt );
          break;
      case OFTRealList:
          //XXX Find out how this is used!!!!!!!
          break;
      case OFTString:
          break;
      case OFTStringList:
          //XXX Find out how this is used!!!!!!!
          break;
      case OFTWideString:
          break;
      case OFTWideStringList:
          //XXX Find out how this is used!!!!!!!
          break;
      case OFTBinary:
          break;
  }

  OGR_L_CreateField( mLayerHandle, myFieldDefinitionHandle, FALSE );
  OGR_Fld_Destroy( myFieldDefinitionHandle );
  
  return true;
}

bool QgsVectorFileWriter::writePoint(QgsPoint * thePoint)
{
    bool returnvalue=true;
    //check the output file has been initialised
    if (!mInitialisedFlag)
    {
	std::cout << "Vector file writer not initialised yet. Initialise first before calling writePoint!" << std::endl;
	return false;
    }
    //check the geomtry of the output file is compatible
    if (mGeometryType != wkbPoint)
    {
	std::cout << "Vector file writer geometry type is not compatible with writePoint!" << std::endl;
	return false;
    }
    
    OGRFeatureDefnH fdef=OGR_L_GetLayerDefn(mLayerHandle);
    OGRFeatureH fhand= OGR_F_Create( fdef );
    OGRGeometryH geometryh=OGR_G_CreateGeometry(wkbPoint);
    OGR_G_AddPoint( geometryh, thePoint->x(), thePoint->y(), 0 );
    
    if(OGR_F_SetGeometryDirectly(fhand, geometryh )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Set geometry failed");
#endif
	returnvalue=false;
    }
    
    if(OGR_L_CreateFeature( mLayerHandle, fhand )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Creation of the point failed");
#endif  
	returnvalue=false;
    }
    
    if(OGR_L_SyncToDisk( mLayerHandle )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Sync to disk failed");
#endif 
	returnvalue=false;
    }
    OGR_F_Destroy( fhand );  
    return returnvalue;
}

bool QgsVectorFileWriter::writeLine(unsigned char* wkb, int size)
{
    bool returnvalue=true;
    //add endianness
    int endianval = endian();
    memcpy(&wkb[0],&endianval,1);
    //check the output file has been initialised
    if (!mInitialisedFlag)
    {
	std::cout << "Vector file writer not initialised yet. Initialise first before calling writePoint!" << std::endl;
	return false;
    }
    //check the geomtry of the output file is compatible
    if (mGeometryType != wkbLineString)
    {
	std::cout << "Vector file writer geometry type is not compatible with writePoint!" << std::endl;
	return false;
    }

    OGRFeatureDefnH fdef=OGR_L_GetLayerDefn(mLayerHandle);
    OGRFeatureH fhand= OGR_F_Create( fdef );
    OGRGeometryH geometryh=OGR_G_CreateGeometry(wkbLineString);
    
    if(OGR_G_ImportFromWkb(geometryh, wkb, size)!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("wkb import failed");
#endif
	returnvalue=false;
    }

    if(OGR_F_SetGeometryDirectly(fhand, geometryh )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Set geometry failed");
#endif
	returnvalue=false;
    }

    if(OGR_L_CreateFeature( mLayerHandle, fhand )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Creation of the point failed");
#endif    
	returnvalue=false;
    }
    
    if(OGR_L_SyncToDisk( mLayerHandle )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Sync to disk failed");
#endif 
	returnvalue=false;
    }
    OGR_F_Destroy( fhand );  
    return returnvalue;
}

bool QgsVectorFileWriter::writePolygon(unsigned char* wkb, int size)
{
    bool returnvalue = true;
    //add endianness
    int endianval = endian();
    memcpy(&wkb[0],&endianval,1);
    //check the output file has been initialised
    if (!mInitialisedFlag)
    {
	std::cout << "Vector file writer not initialised yet. Initialise first before calling writePoint!" << std::endl;
	return false;
    }
    //check the geomtry of the output file is compatible
    if (mGeometryType != wkbPolygon)
    {
	std::cout << "Vector file writer geometry type is not compatible with writePoint!" << std::endl;
	return false;
    }

    OGRFeatureDefnH fdef=OGR_L_GetLayerDefn(mLayerHandle);
    OGRFeatureH fhand= OGR_F_Create( fdef );
    OGRGeometryH geometryh=OGR_G_CreateGeometry(wkbPolygon);
    
    if(OGR_G_ImportFromWkb(geometryh, wkb, size)!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("wkb import failed");
#endif
	returnvalue = false;
    }

    if(OGR_F_SetGeometryDirectly(fhand, geometryh )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Set geometry failed");
#endif
	returnvalue = false;
    }

    if(OGR_L_CreateFeature( mLayerHandle, fhand )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Creation of the point failed");
#endif      
	returnvalue = false;
    }
    
    if(OGR_L_SyncToDisk( mLayerHandle )!=OGRERR_NONE)
    {
#ifdef QGISDEBUG
	qWarning("Sync to disk failed");
#endif 
	returnvalue = false;
    }
    OGR_F_Destroy( fhand );  
    return true;
    
}

int QgsVectorFileWriter::endian()
{
#ifdef WIN32
  return NDR;
#else
    // XXX why re-calculate this all the time?  Why not just calculate this
    // XXX once and return the value?  For that matter, some machines have
    // XXX endian.h, which stores the constant variable for local endian-ness.
    if ( 23 == htons( 23 ) )
    {
        // if host byte order is same as network (big-endian) byte order, then
        // this is a big-endian environment
        return XDR;
    }
    
    // otherwise this must be little-endian

    return NDR;
#endif
}
