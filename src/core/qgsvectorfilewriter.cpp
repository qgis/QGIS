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

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgslogger.h"
#include "qgsspatialrefsys.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"

#include <iostream>
#include <QFile>
#include <QString>
#include <QTextCodec>

// Includes for ogr shaprewriting
#include <ogr_srs_api.h>
#include <ogrsf_frmts.h>

//  needed for OFTDate (supported from GDAL 1.3.2)
#include <gdal_version.h>

QgsVectorFileWriter::QgsVectorFileWriter(QString theOutputFileName, QString fileEncoding, QgsVectorLayer * theVectorLayer)
{
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName.toLocal8Bit().data() << 
      " and vector layer : " << theVectorLayer->getLayerID().toLocal8Bit().data() << std::endl;
  //const char *mOutputFormat = "ESRI Shapefile";
  mOutputFormat = "ESRI Shapefile";
  //const char *theOutputFileName = "ogrtest.shp";
  mOutputFileName = theOutputFileName;

  QTextCodec* ncodec=QTextCodec::codecForName(fileEncoding.toLocal8Bit().data());
  if(ncodec)
    {
      mEncoding=ncodec;
    }
  else
    {
#ifdef QGISDEBUG
      qWarning("error finding QTextCodec in QgsVectorFileWriter ctor");
#endif
    }

  //
  // We should retrieve the geometry type from the vector layer!
  //



  mInitialisedFlag = false;
  
}

QgsVectorFileWriter::QgsVectorFileWriter(QString theOutputFileName, QString fileEncoding, OGRwkbGeometryType theGeometryType)
{
  std::cout << "QgsVectorFileWriter constructor called with " << theOutputFileName.toLocal8Bit().data() <<  " and no input vector layer "  << std::endl;
  mOutputFormat = "ESRI Shapefile"; //hard coded for now!
  QTextCodec* ncodec=QTextCodec::codecForName(fileEncoding.toLocal8Bit().data());
  if(ncodec)
    {
      mEncoding=ncodec;
    }
  else
    {
#ifdef QGISDEBUG
      qWarning("error finding QTextCodec in QgsVectorFileWriter ctor");
#endif
    }
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
  OGRSFDriverH myDriverHandle = OGRGetDriverByName( mOutputFormat.toLocal8Bit().data() );

  if( myDriverHandle == NULL )
  {
    std::cout << "Unable to find format driver named " << mOutputFormat.toLocal8Bit().data() << std::endl;
    return false;
  }

  // Filename needs to be UTF-8 for Mac but local8Bit otherwise
  mDataSourceHandle = OGR_Dr_CreateDataSource( myDriverHandle, QFile::encodeName(mOutputFileName).constData(), NULL );
  if( mDataSourceHandle == NULL )
  {
    std::cout << "Datasource handle is null! " << std::endl;
    return false;
  }

  //define the spatial ref system
  OGRSpatialReferenceH mySpatialReferenceSystemHandle = NULL;
  QgsSpatialRefSys mySpatialRefSys;
  mySpatialRefSys.validate();
  QString myWKT = mySpatialRefSys.toWkt();

 
  //sample below shows how to extract srs from a raster
  //    const char *myWKT = GDALGetProjectionRef( hBand );


  if( !myWKT.isNull()  &&  myWKT.length() != 0 )
  {
    mySpatialReferenceSystemHandle = OSRNewSpatialReference( myWKT.toLocal8Bit().data() );
  }
  //change 'contour' to something more useful!
#ifdef QGISDEBUG
  qWarning(("mOutputFileName: "+mOutputFileName).toLocal8Bit().data());
#endif //QGISDEBUG

#ifdef WIN32 
  QString outname=mOutputFileName.mid(mOutputFileName.findRev("\\")+1,mOutputFileName.length());
#else
  QString outname=mOutputFileName.mid(mOutputFileName.findRev("/")+1,mOutputFileName.length());
#endif
  

#ifdef QGISDEBUG
  qWarning(("outname: "+outname).toLocal8Bit().data());
#endif //QGISDEBUG

  // Unsure if this will be a filename or not. Use custom encoding for now.
  mLayerHandle = OGR_DS_CreateLayer( mDataSourceHandle, mEncoding->fromUnicode(outname), 
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

  // Check and fall back on encoding.
  if (mEncoding == NULL)
  {
    qWarning("Failed to initialize VectorFileWriter with encoding. Falling back on utf8");
    mEncoding = QTextCodec::codecForName("utf8");
    Q_ASSERT(mEncoding);
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
  // XXX Is the layer encoding set here?
  Q_ASSERT(mEncoding != NULL);
  myFieldDefinitionHandle = OGR_Fld_Create( mEncoding->fromUnicode(theName), theType );
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
#if GDAL_VERSION_NUM >= 1320
      case OFTDate:
      case OFTTime:
      case OFTDateTime:
          //XXX Find out how this is used!!!!!!!
          break;
#endif
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
  return QgsApplication::endian();
}


QString QgsVectorFileWriter::writeVectorLayerAsShapefile(QString shapefileName, QString enc, QgsVectorLayer* layer)
{
  // save the layer as a shapefile
  QString driverName = "ESRI Shapefile";
  OGRSFDriver *poDriver;
  OGRRegisterAll();
  poDriver =
      OGRSFDriverRegistrar::GetRegistrar()->
      GetDriverByName((const char *)driverName.toLocal8Bit().data());
  
  if (poDriver == NULL)
  {
    return "DRIVER_NOT_FOUND";
  }

  OGRDataSource *poDS;
  // create the data source
  poDS = poDriver->CreateDataSource((const char *) shapefileName, NULL);
  if (poDS == NULL)
  {
    return "ERROR_CREATE_SOURCE";
  }
  
  QgsDebugMsg("Created data source");
  
  QTextCodec* saveCodec = QTextCodec::codecForName(enc.toLocal8Bit().data());
  if(!saveCodec)
  {
      QgsDebugMsg("error finding QTextCodec for " + enc);
      saveCodec = QTextCodec::codecForLocale();
  }
 
  // datasource created, now create the output layer, use utf8() for now.
  OGRLayer *poLayer;
  poLayer =
      poDS->CreateLayer((const char *) (shapefileName.
      left(shapefileName.find(".shp"))).utf8(), NULL,
  static_cast < OGRwkbGeometryType > (1), NULL);
  
  if (poLayer == NULL)
  {
    return "ERROR_CREATE_LAYER";
  }
  
  QgsDebugMsg("created layer");
  
  const QgsFieldMap & attributeFields = layer->fields();
    
  // TODO: calculate the field lengths
  //int *lengths = getFieldLengths();
  int lengths[] = { 1,2,3 }; // dummy lengths!
  
  // create the fields
  QgsDebugMsg("creating " + QString("%d").arg(attributeFields.size()) + " fields");

  for (uint i = 0; i < attributeFields.size(); i++)
  {
    // check the field length - if > 10 we need to truncate it
    QgsField attrField = attributeFields[i];
    if (attrField.name().length() > 10)
    {
      attrField = attrField.name().left(10);
    }

    // all fields are created as string (for now)
    // TODO: make it type-aware
    OGRFieldDefn fld(saveCodec->fromUnicode(attrField.name()), OFTString);

    // set the length for the field -- but we don't know what it is...
    fld.SetWidth(lengths[i]);

    // create the field
    QgsDebugMsg("creating field " + attrField.name() + " width length " + QString("%d").arg(lengths[i]));
    if (poLayer->CreateField(&fld) != OGRERR_NONE)
    {
      QgsDebugMsg("error creating field " + attrField.name());
    }
  }

  QgsDebugMsg("Done creating fields, saving features");

  QgsVectorDataProvider* provider = layer->getDataProvider();
  QgsFeature fet;
  QgsAttributeList allAttr = provider->allAttributesList();

  provider->reset();

  while (provider->getNextFeature(fet, true, allAttr))
  {
    // create the feature
    OGRFeature *poFeature = new OGRFeature(poLayer->GetLayerDefn());

    QgsDebugMsg("Setting the field values");

    const QgsAttributeMap& attributes = fet.attributeMap();
    QgsAttributeMap::const_iterator it;
    
    for (it = attributes.begin(); it != attributes.end(); it++)
    {
      QString value = it.value().fieldValue();
      uint i = it.key();
      if (!value.isNull())
      {
        QgsDebugMsg("Setting " + attributeFields[i].name() + " to " + value);
        poFeature->SetField(saveCodec->fromUnicode(attributeFields[i].name()).data(),
                            saveCodec->fromUnicode(value).data());
      }
      else
      {
        poFeature->SetField(saveCodec->fromUnicode(attributeFields[i].name()).data(), "");
      }
    }
    
    
    // TODO: get the geometry and save it
    OGRPoint *poPoint = new OGRPoint();
    poPoint->setX(0);
    poPoint->setY(0);
//    QString sX = parts[fieldPositions[mXField]];
//    QString sY = parts[fieldPositions[mYField]];
//    poPoint->setX(sX.toDouble());
//    poPoint->setY(sY.toDouble());

    QgsDebugMsg("Setting geometry");

    poFeature->SetGeometryDirectly(poPoint);
    if (poLayer->CreateFeature(poFeature) != OGRERR_NONE)
    {
      QgsDebugMsg("Failed to create feature in shapefile");
    }
    else
    {
      QgsDebugMsg("Added feature");
    }

    delete poFeature;
  }
  
  delete poDS;

  return NULL;
}
