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

QgsVectorFileWriter::QgsVectorFileWriter(QString theOutputFileName, QgsVectorLayer * theVectorLayer)
{
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName << 
      " and vector layer : " << theVectorLayer->getLayerID() << std::endl;
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
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName <<  " and no input vector layer "  << std::endl;
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
    std::cout << "Unable to find format driver named " << mOutputFormat << std::endl;
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

  QString myWKT = NULL; //WKT = Well Known Text
  //sample below shows how to extract srs from a raster
  //    const char *myWKT = GDALGetProjectionRef( hBand );


  if( myWKT != NULL && strlen(myWKT) != 0 )
  {
    mySpatialReferenceSystemHandle = OSRNewSpatialReference( myWKT );
  }
  //change 'contour' to something more useful!
  mLayerHandle = OGR_DS_CreateLayer( mDataSourceHandle, 
          "contour", 
          mySpatialReferenceSystemHandle, 
          mGeometryType,
          NULL );
  
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
  OGRFieldDefnH myFieldDefinitionHandle;;

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

  

  //
  //
  // Do Stuff
  //

  return true;
}

//
// I think this stuff illustrates how to write features to a vector file.
//

/*
  CPLErr eErr;
   CPLErr OGRContourWriter( double dfLevel,
   int nPoints, double *padfX, double *padfY,
   void *pInfo )

   {
   OGRContourWriterInfo *poInfo = (OGRContourWriterInfo *) pInfo;
   OGRFeatureH hFeat;
   OGRGeometryH hGeom;
   int iPoint;

   hFeat = OGR_F_Create( OGR_L_GetLayerDefn( poInfo->myLayerHandle ) );

   if( poInfo->nIDField != -1 )
   OGR_F_SetFieldInteger( hFeat, poInfo->nIDField, poInfo->nNextID++ );

   if( poInfo->nElevField != -1 )
   OGR_F_SetFieldDouble( hFeat, poInfo->nElevField, dfLevel );

   hGeom = OGR_G_CreateGeometry( wkbLineString );

   for( iPoint = nPoints-1; iPoint >= 0; iPoint-- )
   {
   OGR_G_SetPoint( hGeom, iPoint,
   poInfo->adfGeoTransform[0]
   + poInfo->adfGeoTransform[1] * padfX[iPoint]
   + poInfo->adfGeoTransform[2] * padfY[iPoint],
   poInfo->adfGeoTransform[3]
   + poInfo->adfGeoTransform[4] * padfX[iPoint]
   + poInfo->adfGeoTransform[5] * padfY[iPoint],
   dfLevel );
   }

   OGR_F_SetGeometryDirectly( hFeat, hGeom );

   OGR_L_CreateFeature( poInfo->myLayerHandle, hFeat );
   OGR_F_Destroy( hFeat );

   return CE_None;
   }
   */
