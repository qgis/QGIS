/***************************************************************************
           qgsogrprovider.cpp Data provider for OGR supported formats
                    Formerly known as qgsshapefileprovider.cpp  
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
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

#include "qgsogrprovider.h"
#include "qgslogger.h"

#include <iostream>
#include <cassert>

#include <gdal.h>         // to collect version information

#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>
#include "ogr_api.h"//only for a test

#include <QtDebug>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>

//TODO Following ifndef can be removed once WIN32 GEOS support
//    is fixed
#ifndef NOWIN32GEOSXXX
//XXX GEOS support on windows is broken until we can get VC++ to
//    tolerate geos.h without throwing a bunch of type errors. It
//    appears that the windows version of GEOS may be compiled with 
//    MINGW rather than VC++.
#endif 


#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialrefsys.h"

static const QString TEXT_PROVIDER_KEY = "ogr";
static const QString TEXT_PROVIDER_DESCRIPTION = 
               QString("OGR data provider")
             + " (compiled against GDAL/OGR library version "
             + GDAL_RELEASE_NAME
             + ", running against GDAL/OGR library version "
             + GDALVersionInfo("RELEASE_NAME")
             + ")";



QgsOgrProvider::QgsOgrProvider(QString const & uri)
 : QgsVectorDataProvider(uri),
   ogrDataSource(0),
   extent_(0),
   ogrLayer(0),
   ogrDriver(0)
{
  OGRRegisterAll();

  // set the selection rectangle pointer to 0
  mSelectionRectangle = 0;
  // make connection to the data source

  QgsDebugMsg("Data source uri is " + uri);

  // try to open for update, but disable error messages to avoid a
  // message if the file is read only, because we cope with that
  // ourselves.
  CPLPushErrorHandler(&CPLQuietErrorHandler);
  ogrDataSource = OGRSFDriverRegistrar::Open(QFile::encodeName(uri).constData(), TRUE, &ogrDriver);
  CPLPopErrorHandler();

  if(ogrDataSource == NULL)
  {
    // try to open read-only
    ogrDataSource = OGRSFDriverRegistrar::Open(QFile::encodeName(uri).constData(), FALSE, &ogrDriver);

    //TODO Need to set a flag or something to indicate that the layer
    //TODO is in read-only mode, otherwise edit ops will fail
    //TODO: capabilities() should now reflect this; need to test.
  }
  if (ogrDataSource != NULL) {

    QgsDebugMsg("Data source is valid");
    QgsDebugMsg("OGR Driver was " + QString(ogrDriver->GetName()));

    valid = true;

    ogrDriverName = ogrDriver->GetName();

    ogrLayer = ogrDataSource->GetLayer(0);

    // get the extent_ (envelope) of the layer

    QgsDebugMsg("Starting get extent\n");

    extent_ = new OGREnvelope();
    ogrLayer->GetExtent(extent_);

    QgsDebugMsg("Finished get extent\n");

    // getting the total number of features in the layer
    numberFeatures = ogrLayer->GetFeatureCount();   
    // check the validity of the layer

    QgsDebugMsg("checking validity\n");
    loadFields();
    QgsDebugMsg("Done checking validity\n");
  } else {
    QgsLogger::critical("Data source is invalid");
    const char *er = CPLGetLastErrorMsg();
    QgsLogger::critical(er);
    valid = false;
  }

  // create the geos objects
  geometryFactory = new GEOS_GEOM::GeometryFactory();
  assert(geometryFactory!=0);

  mSupportedNativeTypes.insert("Integer");
  mSupportedNativeTypes.insert("Real");
  mSupportedNativeTypes.insert("String");
}

QgsOgrProvider::~QgsOgrProvider()
{
  OGRDataSource::DestroyDataSource(ogrDataSource);
  ogrDataSource = 0;
  delete extent_;
  extent_ = 0;
  delete geometryFactory;
  delete mSelectionRectangle;
}

void QgsOgrProvider::setEncoding(const QString& e)
{
    QgsVectorDataProvider::setEncoding(e);
    loadFields();
}

void QgsOgrProvider::loadFields()
{
    //the attribute fields need to be read again when the encoding changes
    mAttributeFields.clear();
    OGRFeatureDefn* fdef = ogrLayer->GetLayerDefn();
    if(fdef)
    {
      geomType = fdef->GetGeomType();

      //Some ogr drivers (e.g. GML) are not able to determine the geometry type of a layer like this.
      //In such cases, we examine the first feature 
      if(geomType == wkbUnknown) 
	{
	  ogrLayer->ResetReading();
	  OGRFeature* firstFeature = ogrLayer->GetNextFeature();
	  if(firstFeature)
	    {
	      OGRGeometry* firstGeometry = firstFeature->GetGeometryRef();
	      if(firstGeometry)
		{
		  geomType = firstGeometry->getGeometryType();
		}
	    }
	  ogrLayer->ResetReading();
	}
      
      for(int i=0;i<fdef->GetFieldCount();++i)
      {
        OGRFieldDefn *fldDef = fdef->GetFieldDefn(i);
        OGRFieldType ogrType = fldDef->GetType();
        QVariant::Type varType;
        switch (ogrType)
        {
          case OFTInteger: varType = QVariant::Int; break;
          case OFTReal: varType = QVariant::Double; break;
          // unsupported in OGR 1.3
          //case OFTDateTime: varType = QVariant::DateTime; break;
          case OFTString: varType = QVariant::String; break;
          default: varType = QVariant::String; // other unsupported, leave it as a string
        }
        
        mAttributeFields.insert(i, QgsField(
              mEncoding->toUnicode(fldDef->GetNameRef()), varType,
              mEncoding->toUnicode(fldDef->GetFieldTypeName(ogrType)),
              fldDef->GetWidth(),
              fldDef->GetPrecision() ));
      }
    }
}


QString QgsOgrProvider::storageType() const
{
  // Delegate to the driver loaded in by OGR
  return ogrDriverName;
}



bool QgsOgrProvider::getFeatureAtId(int featureId,
                                    QgsFeature& feature,
                                    bool fetchGeometry,
                                    QgsAttributeList fetchAttributes)
{
  OGRFeature *fet = ogrLayer->GetFeature(featureId);
  if (fet == NULL)
    return false;
  
  feature.setFeatureId(fet->GetFID());

  /* fetch geometry */
  if (fetchGeometry)
  {
    OGRGeometry *geom = fet->GetGeometryRef();
      
    // get the wkb representation
    unsigned char *wkb = new unsigned char[geom->WkbSize()];
    geom->exportToWkb((OGRwkbByteOrder) QgsApplication::endian(), wkb);
      
    feature.setGeometryAndOwnership(wkb, geom->WkbSize());
  }

  /* fetch attributes */
  for(QgsAttributeList::iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it)
  {
    getFeatureAttribute(fet,feature,*it);
  }
  
  return true;
}

bool QgsOgrProvider::getNextFeature(QgsFeature& feature)
{
  if (!valid)
    {
      QgsLogger::warning("Read attempt on an invalid shapefile data source");
      return false;
    }
  
  OGRFeature *fet;
  QgsRect selectionRect;
  
  while ((fet = ogrLayer->GetNextFeature()) != NULL)
    {
      // skip features without geometry
      if (fet->GetGeometryRef() == NULL && !mFetchFeaturesWithoutGeom)
	{
	  delete fet;
	  continue;
	}
      
      OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
      QString featureTypeName = featureDefinition ? QString(featureDefinition->GetName()) : QString("");
      feature.setFeatureId(fet->GetFID());
      feature.setTypeName(featureTypeName);
      
      /* fetch geometry */
      if (mFetchGeom)
	{
	  OGRGeometry *geom = fet->GetGeometryRef();
	  
	  // get the wkb representation
	  unsigned char *wkb = new unsigned char[geom->WkbSize()];
	  geom->exportToWkb((OGRwkbByteOrder) QgsApplication::endian(), wkb);
	  
	  feature.setGeometryAndOwnership(wkb, geom->WkbSize());
	  
	  if (mUseIntersect)
	    {
	      //precise test for intersection with search rectangle
	      //first make QgsRect from OGRPolygon
	      OGREnvelope env;
        if(mSelectionRectangle)
	        mSelectionRectangle->getEnvelope(&env);
	      if(env.IsInit()) //if envelope is invalid, skip the precise intersection test
		{
		  selectionRect.set(env.MinX, env.MinY, env.MaxX, env.MaxY);
		  if(!feature.geometry()->intersects(selectionRect))
		    {
		      delete fet;
		      continue;
		    }
		}
	      
	    }
	}
      
      /* fetch attributes */
      for(QgsAttributeList::iterator it = mAttributesToFetch.begin(); it != mAttributesToFetch.end(); ++it)
	{
	  getFeatureAttribute(fet,feature,*it);
	}
      
      /* we have a feature, end this cycle */
      break;
      
    } /* while */
  
  if (fet)
    {
      delete fet;
      return true;
    }
  else
    {
      QgsDebugMsg("Feature is null");  
      // probably should reset reading here
      ogrLayer->ResetReading();
      return false;
    }
}

void QgsOgrProvider::select(QgsAttributeList fetchAttributes, QgsRect rect, bool fetchGeometry, \
			    bool useIntersect)
{
  mUseIntersect = useIntersect;
  mAttributesToFetch = fetchAttributes;
  mFetchGeom = fetchGeometry;

  // spatial query to select features
  if(rect.isEmpty())
    {
        ogrLayer->SetSpatialFilter(0);
    }
  else
    {
      OGRGeometry *filter = 0;
      filter = OGRGeometryFactory::createGeometry(wkbPolygon);
      QString wktExtent = QString("POLYGON ((%1))").arg(rect.asPolygon());
      const char *wktText = (const char *)wktExtent;
      
      if(useIntersect)
	{
	  // store the selection rectangle for use in filtering features during
	  // an identify and display attributes
	  delete mSelectionRectangle;
	  mSelectionRectangle = new OGRPolygon();
	  mSelectionRectangle->importFromWkt((char **)&wktText);
	}
      
      // reset the extent for the ogr filter
      wktExtent = QString("POLYGON ((%1))").arg(rect.asPolygon());
      wktText = (const char *)wktExtent;
      
      OGRErr result = ((OGRPolygon *) filter)->importFromWkt((char **)&wktText);
      //TODO - detect an error in setting the filter and figure out what to
      //TODO   about it. If setting the filter fails, all records will be returned
      if (result == OGRERR_NONE) 
	{
	  QgsDebugMsg("Setting spatial filter using " + wktExtent);
	  ogrLayer->SetSpatialFilter(filter);
	  //ogrLayer->SetSpatialFilterRect(rect->xMin(), rect->yMin(), rect->xMax(), rect->yMax());
	}
      else
	{
	  QgsDebugMsg("Setting spatial filter failed!");
	}
      OGRGeometryFactory::destroyGeometry(filter);
    }  
}


unsigned char * QgsOgrProvider::getGeometryPointer(OGRFeature *fet)
{
  OGRGeometry *geom = fet->GetGeometryRef();
  unsigned char *gPtr=0;
  // get the wkb representation
  gPtr = new unsigned char[geom->WkbSize()];

  geom->exportToWkb((OGRwkbByteOrder) QgsApplication::endian(), gPtr);
  return gPtr;

}


QgsRect QgsOgrProvider::extent()
{
  mExtentRect.set(extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
  return mExtentRect;
}


size_t QgsOgrProvider::layerCount() const
{
    return ogrDataSource->GetLayerCount();
} // QgsOgrProvider::layerCount()


/** 
 * Return the feature type
 */
QGis::WKBTYPE QgsOgrProvider::geometryType() const
{
  return (QGis::WKBTYPE) geomType;
}

/** 
 * Return the feature type
 */
long QgsOgrProvider::featureCount() const
{
  return numberFeatures;
}

/**
 * Return the number of fields
 */
uint QgsOgrProvider::fieldCount() const
{
  return mAttributeFields.size();
}

void QgsOgrProvider::getFeatureAttribute(OGRFeature * ogrFet, QgsFeature & f, int attindex)
{
  OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(attindex);

  if ( ! fldDef )
  {
    QgsDebugMsg("ogrFet->GetFieldDefnRef(attindex) returns NULL");
    return;
  }

  //QString fld = mEncoding->toUnicode(fldDef->GetNameRef());
  QByteArray cstr(ogrFet->GetFieldAsString(attindex));
  QString str = mEncoding->toUnicode(cstr);
  QVariant value;
  
  switch (mAttributeFields[attindex].type())
  {
    case QVariant::String: value = QVariant(str); break;
    case QVariant::Int: value = QVariant(str.toInt()); break;
    case QVariant::Double: value = QVariant(str.toDouble()); break;
    //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
    default: assert(NULL && "unsupported field type");
  }

  f.addAttribute(attindex, value);
}


const QgsFieldMap & QgsOgrProvider::fields() const
{
  return mAttributeFields;
}

void QgsOgrProvider::reset()
{
  ogrLayer->ResetReading();
}


//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid()
{
  return valid;
}

bool QgsOgrProvider::addFeature(QgsFeature& f)
{ 
  bool returnValue = true;
  OGRFeatureDefn* fdef=ogrLayer->GetLayerDefn();
  OGRFeature* feature=new OGRFeature(fdef);
  QGis::WKBTYPE ftype = f.geometry()->wkbType();
  unsigned char* wkb = f.geometry()->wkbBuffer();
  OGRErr err;
  
  switch(ftype)
  {
    case QGis::WKBPoint:
      {
        OGRPoint* p=new OGRPoint();
        p->importFromWkb(wkb,1+sizeof(int)+2*sizeof(double));
        err = feature->SetGeometry(p);
	if(err != OGRERR_NONE)
	  {
	    delete p;
	    return false;
	  }
        break;
      }
    case QGis::WKBLineString:
      {
        OGRLineString* l=new OGRLineString();
        int length;
        memcpy(&length,wkb+1+sizeof(int),sizeof(int));
        l->importFromWkb(wkb,1+2*sizeof(int)+2*length*sizeof(double));
        err = feature->SetGeometry(l);
	if(err != OGRERR_NONE)
	  {
	    delete l;
	    return false;
	  }
        break;
      }
    case QGis::WKBPolygon:
      {
        OGRPolygon* pol=new OGRPolygon();
        int numrings;
        int totalnumpoints=0;
        int numpoints;//number of points in one ring
        unsigned char* ptr=wkb+1+sizeof(int);
        memcpy(&numrings,ptr,sizeof(int));
        ptr+=sizeof(int);
        for(int i=0;i<numrings;++i)
        {
          memcpy(&numpoints,ptr,sizeof(int));
          ptr+=sizeof(int);
          totalnumpoints+=numpoints;
          ptr+=(2*sizeof(double));
        }
        pol->importFromWkb(wkb,1+2*sizeof(int)+numrings*sizeof(int)+totalnumpoints*2*sizeof(double));
        err = feature->SetGeometry(pol);
	if(err != OGRERR_NONE)
	  {
	    delete pol;
	    return false;
	  }
        break;
      }
    case QGis::WKBMultiPoint:
      {
        OGRMultiPoint* multip= new OGRMultiPoint();
        int count;
        //determine how many points
        memcpy(&count,wkb+1+sizeof(int),sizeof(int));
        multip->importFromWkb(wkb,1+2*sizeof(int)+count*2*sizeof(double));
        err = feature->SetGeometry(multip);
	if(err != OGRERR_NONE)
	  {
	    delete multip;
	    return false;
	  }
        break;
      }
    case QGis::WKBMultiLineString:
      {
        OGRMultiLineString* multil=new OGRMultiLineString();
        int numlines;
        memcpy(&numlines,wkb+1+sizeof(int),sizeof(int));
        int totalpoints=0;
        int numpoints;//number of point in one line
        unsigned char* ptr=wkb+9;
        for(int i=0;i<numlines;++i)
        {
          memcpy(&numpoints,ptr,sizeof(int));
          ptr+=4;
          for(int j=0;j<numpoints;++j)
          {
            ptr+=16;
            totalpoints+=2;
          }
        }
        int size=1+2*sizeof(int)+numlines*sizeof(int)+totalpoints*2*sizeof(double);
        multil->importFromWkb(wkb,size);
        err = feature->SetGeometry(multil);
	if(err != OGRERR_NONE)
	  {
	    delete multil;
	    return false;
	  }
        break;
      }
    case QGis::WKBMultiPolygon:
      {
        OGRMultiPolygon* multipol=new OGRMultiPolygon();
        int numpolys;
        memcpy(&numpolys,wkb+1+sizeof(int),sizeof(int));
        int numrings;//number of rings in one polygon
        int totalrings=0;
        int totalpoints=0;
        int numpoints;//number of points in one ring
        unsigned char* ptr=wkb+9;

        for(int i=0;i<numpolys;++i)
        {
          memcpy(&numrings,ptr,sizeof(int));
          ptr+=4;
          for(int j=0;j<numrings;++j)
          {
            totalrings++;
            memcpy(&numpoints,ptr,sizeof(int));
            for(int k=0;k<numpoints;++k)
            {
              ptr+=16;
              totalpoints+=2;
            }
          }
        }
        int size=1+2*sizeof(int)+numpolys*sizeof(int)+totalrings*sizeof(int)+totalpoints*2*sizeof(double);
        multipol->importFromWkb(wkb,size);
        err = feature->SetGeometry(multipol);
	if(err != OGRERR_NONE)
	  {
	    delete multipol;
	    return false;
	  }
        break;
      }
    default:
      {
        QgsLogger::debug("Unknown feature type of: ", (int)(ftype), 1, 
                         __FILE__, __FUNCTION__, __LINE__);
	return false;
	break;
      }
  }
  
  QgsAttributeMap attrs = f.attributeMap();

  //add possible attribute information
  for(QgsAttributeMap::iterator it = attrs.begin(); it != attrs.end(); ++it)
  {
    int targetAttributeId = it.key();
    
    // don't try to set field from attribute map if it's not present in layer
    if (targetAttributeId >= fdef->GetFieldCount())
      continue;

    //if(!s.isEmpty())
    // continue;
      
    if(fdef->GetFieldDefn(targetAttributeId)->GetType()==OFTInteger)
    {
      feature->SetField(targetAttributeId,it->toInt());
    }
    else if(fdef->GetFieldDefn(targetAttributeId)->GetType()==OFTReal)
    {
      feature->SetField(targetAttributeId,it->toDouble());
    }
    else if(fdef->GetFieldDefn(targetAttributeId)->GetType()==OFTString)
    {
      QgsDebugMsg( QString("Writing string attribute %1 with %2, encoding %3")
	           .arg( targetAttributeId )
		   .arg( it->toString() )
		   .arg( mEncoding->name().data() ) );
      feature->SetField(targetAttributeId,mEncoding->fromUnicode(it->toString()).constData());
    }
    else
    {
      QgsLogger::warning("QgsOgrProvider::addFeature, no type found");
    }
  }

  if(ogrLayer->CreateFeature(feature)!=OGRERR_NONE)
  {
    QgsLogger::warning("Writing of the feature failed");
    returnValue = false;
  }
  ++numberFeatures;
  delete feature;
  return returnValue;
}


bool QgsOgrProvider::addFeatures(QgsFeatureList & flist)
{
  bool returnvalue=true;
  for(QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it)
  {
    if(!addFeature(*it))
    {
      returnvalue=false;
    }
  }

  // flush features
  ogrLayer->SyncToDisk();
  numberFeatures = ogrLayer->GetFeatureCount(); //new feature count
  return returnvalue;
}

bool QgsOgrProvider::addAttributes(const QgsNewAttributesMap & attributes)
{
    bool returnvalue=true;

    for(QgsNewAttributesMap::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
    {
	if(*iter=="OFTInteger")
	{
      OGRFieldDefn fielddefn(mEncoding->fromUnicode(iter.key()).data(),OFTInteger);
	    if(ogrLayer->CreateField(&fielddefn)!=OGRERR_NONE)
	    {
		QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTInteger field failed");	
		returnvalue=false;
	    }
	}
  else if(*iter=="OFTReal")
	{
      OGRFieldDefn fielddefn(mEncoding->fromUnicode(iter.key()).data(),OFTReal);
	    if(ogrLayer->CreateField(&fielddefn)!=OGRERR_NONE)
	    {
		QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTReal field failed");
		returnvalue=false;
	    }
	}
  else if(*iter=="OFTString")
	{
      OGRFieldDefn fielddefn(mEncoding->fromUnicode(iter.key()).data(),OFTString);
	    if(ogrLayer->CreateField(&fielddefn)!=OGRERR_NONE)
	    {
		QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTString field failed");
		returnvalue=false;
	    }
	}
	else
	{
	    QgsLogger::warning("QgsOgrProvider::addAttributes, type not found");
	    returnvalue=false;
	}
    }
    return returnvalue;
}

bool QgsOgrProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{   
  for(QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it)
  {
    long fid = (long) it.key();

    OGRFeature *of = ogrLayer->GetFeature ( fid );

    if ( !of )
    {
      QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Cannot read feature, cannot change attributes");
      return false;
    }

    const QgsAttributeMap& attr = it.value();

    for( QgsAttributeMap::const_iterator it2 = attr.begin(); it2 != attr.end(); ++it2 )
    {
      int f = it2.key();
		
      OGRFieldDefn *fd = of->GetFieldDefnRef ( f );
      if (fd == NULL)
      {
        QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Field " + QString::number(f) + " doesn't exist");
        continue;
      }
      
      OGRFieldType type = fd->GetType();
      switch ( type )
      {
        case OFTInteger:
          of->SetField ( f, it2->toInt() );
          break;
        case OFTReal:
          of->SetField ( f, it2->toDouble() );
          break;
        case OFTString:
          of->SetField ( f, mEncoding->fromUnicode(it2->toString()).constData() );
          break;
        default:
          QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Unknown field type, cannot change attribute");
          break;
      }

    }

    ogrLayer->SetFeature ( of );
  }

  ogrLayer->SyncToDisk();

  return true;
}

bool QgsOgrProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  OGRFeature* theOGRFeature = 0;
  OGRGeometry* theNewGeometry = 0;

  for (QgsGeometryMap::iterator it = geometry_map.begin(); it != geometry_map.end(); ++it)
  {
    theOGRFeature = ogrLayer->GetFeature(it.key());
    if(!theOGRFeature)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, cannot find feature");
      continue;
    }

    //create an OGRGeometry
    if (OGRGeometryFactory::createFromWkb(it->wkbBuffer(),
                                          ogrLayer->GetSpatialRef(),
                                          &theNewGeometry,
                                          it->wkbSize()) != OGRERR_NONE)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, error while creating new OGRGeometry");
      delete theNewGeometry;
      theNewGeometry = 0;
      continue;
    }

    if(!theNewGeometry)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, new geometry is NULL");
      continue;
    }
      
    //set the new geometry
    if(theOGRFeature->SetGeometryDirectly(theNewGeometry) != OGRERR_NONE)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, error while replacing geometry");
      delete theNewGeometry;
      theNewGeometry = 0;
      continue;
    }
    
    ogrLayer->SetFeature(theOGRFeature);
  }
  ogrLayer->SyncToDisk();
  return true;
}

bool QgsOgrProvider::createSpatialIndex()
{
    QString filename=dataSourceUri().section('/',-1,-1);//find out the filename from the uri
    QString layername=filename.section('.',0,0);
    QString sql="CREATE SPATIAL INDEX ON "+layername;
    ogrDataSource->ExecuteSQL (sql.ascii(), ogrLayer->GetSpatialFilter(),"");
    //find out, if the .qix file is there
    QString indexname = dataSourceUri();
    indexname.truncate(dataSourceUri().length()-filename.length());
    indexname=indexname+layername+".qix";
    QFile indexfile(indexname);
    if(indexfile.exists())
    {
	return true;
    }
    else
    {
	return false;
    }
}

bool QgsOgrProvider::deleteFeatures(const QgsFeatureIds & id)
{
  bool returnvalue=true;
  for (QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it)
  {
    if(!deleteFeature(*it))
    {
      returnvalue=false;
    }
  }

  ogrLayer->SyncToDisk();
  QString filename=dataSourceUri().section('/',-1,-1);//find out the filename from the uri
  QString layername=filename.section('.',0,0);
  QString sql="REPACK " + layername;
  ogrDataSource->ExecuteSQL(sql.toLocal8Bit().data(), NULL, NULL);
  numberFeatures = ogrLayer->GetFeatureCount(); //new feature count
  return returnvalue;
}

bool QgsOgrProvider::deleteFeature(int id)
{
  OGRErr res = ogrLayer->DeleteFeature(id);
  return (res == OGRERR_NONE);
}

int QgsOgrProvider::capabilities() const
{
  int ability = NoCapabilities;

  // collect abilities reported by OGR
  if (ogrLayer)
  {
    // Whilst the OGR documentation (e.g. at
    // http://www.gdal.org/ogr/classOGRLayer.html#a17) states "The capability
    // codes that can be tested are represented as strings, but #defined
    // constants exists to ensure correct spelling", we always use strings
    // here.  This is because older versions of OGR don't always have all
    // the #defines we want to test for here.

    if (ogrLayer->TestCapability("RandomRead"))
    // TRUE if the GetFeature() method works *efficiently* for this layer.
    // TODO: Perhaps influence if QGIS caches into memory 
    //       (vs read from disk every time) based on this setting.
    {
      ability |= QgsVectorDataProvider::RandomSelectGeometryAtId;
    }
    else
    {
      ability |= QgsVectorDataProvider::SequentialSelectGeometryAtId;
    }
    ability |= QgsVectorDataProvider::SelectGeometryAtId;

    if (ogrLayer->TestCapability("SequentialWrite"))
    // TRUE if the CreateFeature() method works for this layer.
    {
      ability |= QgsVectorDataProvider::AddFeatures;
    }

    if (ogrLayer->TestCapability("DeleteFeature"))
    // TRUE if this layer can delete its features
    {
      ability |= DeleteFeatures;
    }
    
    if (ogrLayer->TestCapability("RandomWrite"))
    // TRUE if the SetFeature() method is operational on this layer.
    {
      // TODO According to http://shapelib.maptools.org/ (Shapefile C Library V1.2)
      // TODO "You can't modify the vertices of existing structures".
      // TODO Need to work out versions of shapelib vs versions of GDAL/OGR
      // TODO And test appropriately.

      ability |= ChangeAttributeValues;
      ability |= QgsVectorDataProvider::ChangeGeometries;
    }

    if (ogrLayer->TestCapability("FastSpatialFilter"))
    // TRUE if this layer implements spatial filtering efficiently.
    // Layers that effectively read all features, and test them with the 
    // OGRFeature intersection methods should return FALSE.
    // This can be used as a clue by the application whether it should build
    // and maintain it's own spatial index for features in this layer.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should build and maintain it's own spatial index for features in this layer.
    }

    if (ogrLayer->TestCapability("FastFeatureCount"))
    // TRUE if this layer can return a feature count
    // (via OGRLayer::GetFeatureCount()) efficiently ... ie. without counting
    // the features. In some cases this will return TRUE until a spatial
    // filter is installed after which it will return FALSE.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to count features.
    }

    if (ogrLayer->TestCapability("FastGetExtent"))
    // TRUE if this layer can return its data extent 
    // (via OGRLayer::GetExtent()) efficiently ... ie. without scanning
    // all the features. In some cases this will return TRUE until a
    // spatial filter is installed after which it will return FALSE.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to calculate extent.
    }

    if (ogrLayer->TestCapability("FastSetNextByIndex"))
    // TRUE if this layer can perform the SetNextByIndex() call efficiently.
    {
      // No use required for this QGIS release.
    }

    if (1)
    {
      // Ideally this should test for Shapefile type and GDAL >= 1.2.6
      // In reality, createSpatialIndex() looks after itself.
      ability |= QgsVectorDataProvider::CreateSpatialIndex;
    }

    // OGR doesn't handle shapefiles without attributes, ie. missing DBFs well, fixes #803
    if( ogrDriverName.startsWith("ESRI") && mAttributeFields.size()==0 )
    {
      QgsDebugMsg("OGR doesn't handle shapefile without attributes well, ie. missing DBFs");
      ability &= ~(AddFeatures|DeleteFeatures|ChangeAttributeValues|AddAttributes|DeleteAttributes);
    }
  }

  return ability;

/*
    return (QgsVectorDataProvider::AddFeatures
	    | QgsVectorDataProvider::ChangeAttributeValues
	    | QgsVectorDataProvider::CreateSpatialIndex);
*/
}




QString  QgsOgrProvider::name() const
{
    return TEXT_PROVIDER_KEY;
} // QgsOgrProvider::name()



QString  QgsOgrProvider::description() const
{
    return TEXT_PROVIDER_DESCRIPTION;
} //  QgsOgrProvider::description()





/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

  @note

  Copied from qgisapp.cpp.  

  @todo XXX This should probably be generalized and moved to a standard
            utility type thingy.

*/
static QString createFileFilter_(QString const &longName, QString const &glob)
{
    return "[OGR] " + 
           longName + " (" + glob.lower() + " " + glob.upper() + ");;";
} // createFileFilter_





QGISEXTERN QString fileVectorFilters()
{
    static QString myFileFilters;

    // if we've already built the supported vector string, just return what
    // we've already built
    if ( ! ( myFileFilters.isEmpty() || myFileFilters.isNull() ) )
    {
        return myFileFilters;
    }
    
    // register ogr plugins
    OGRRegisterAll();

    // first get the GDAL driver manager

    OGRSFDriverRegistrar *driverRegistrar = OGRSFDriverRegistrar::GetRegistrar();

    if (!driverRegistrar)
    {
	QgsLogger::warning("OGR Driver Manager, unable to get OGRDriverManager");
        return "";              // XXX good place to throw exception if we
    }                           // XXX decide to do exceptions

    // then iterate through all of the supported drivers, adding the
    // corresponding file filter

    OGRSFDriver *driver;          // current driver

    QString driverName;           // current driver name

    // Grind through all the drivers and their respective metadata.
    // We'll add a file filter for those drivers that have a file
    // extension defined for them; the others, welll, even though
    // theoreticaly we can open those files because there exists a
    // driver for them, the user will have to use the "All Files" to
    // open datasets with no explicitly defined file name extension.
    QgsDebugMsg( QString("Driver count: %1").arg( driverRegistrar->GetDriverCount() ) );

    for (int i = 0; i < driverRegistrar->GetDriverCount(); ++i)
    {
        driver = driverRegistrar->GetDriver(i);

        Q_CHECK_PTR(driver);

        if (!driver)
        {
	    QgsLogger::warning("unable to get driver " + QString::number(i));
            continue;
        }

        driverName = driver->GetName();


        if (driverName.startsWith("ESRI"))
        {
            myFileFilters += createFileFilter_("ESRI Shapefiles", "*.shp");
        }
        else if (driverName.startsWith("UK"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("SDTS"))
        {
            myFileFilters += createFileFilter_( "Spatial Data Transfer Standard",
                                                "*catd.ddf" );
        }
        else if (driverName.startsWith("TIGER"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("S57"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("MapInfo"))
        {
            myFileFilters += createFileFilter_("MapInfo", "*.mif *.tab");
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("DGN"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("VRT"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("AVCBin"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("REC"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("Memory"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("Jis"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("GML"))
        {
            // XXX not yet supported; post 0.1 release task
            myFileFilters += createFileFilter_( "Geography Markup Language",
                                                "*.gml" );
        }
        else if (driverName.startsWith("CSV"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("PostgreSQL"))
        {
            // XXX needs file filter extension
        }
        else if (driverName.startsWith("GRASS")) 
        { 
          // XXX needs file filter extension 
        } 
        else if (driverName.startsWith("KML")) 
        { 
          // XXX needs file filter extension 
        } 
        else if (driverName.startsWith("Interlis 1")) 
        { 
          // XXX needs file filter extension 
        } 
        else if (driverName.startsWith("Interlis 2")) 
        { 
          // XXX needs file filter extension 
        } 
        else if (driverName.startsWith("SQLite")) 
        { 
          // XXX needs file filter extension 
        } 
        else if (driverName.startsWith("MySQL")) 
        { 
          // XXX needs file filter extension 
        } 
        else
        {
            // NOP, we don't know anything about the current driver
            // with regards to a proper file filter string
	    QgsLogger::debug("fileVectorFilters, unknown driver: " + driverName);
        }

    }                           // each loaded GDAL driver

    // can't forget the default case

    myFileFilters += "All files (*.*)";

    return myFileFilters;

} // fileVectorFilters() const



QString QgsOgrProvider::fileVectorFilters() const
{
    return fileVectorFilters();
} // QgsOgrProvider::fileVectorFilters() const






/**
 * Class factory to return a pointer to a newly created 
 * QgsOgrProvider object
 */
QGISEXTERN QgsOgrProvider * classFactory(const QString *uri)
{
  return new QgsOgrProvider(*uri);
}



/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}


/**
 * Required description function 
 */
QGISEXTERN QString description()
{
    return TEXT_PROVIDER_DESCRIPTION;
} 

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */

QGISEXTERN bool isProvider()
{
  return true;
}

/**Creates an empty data source
@param uri location to store the file(s)
@param format data format (e.g. "ESRI Shapefile"
@param vectortype point/line/polygon or multitypes
@param attributes a list of name/type pairs for the initial attributes
@return true in case of success*/
QGISEXTERN bool createEmptyDataSource(const QString& uri,
                                      const QString& format,
                                      const QString& encoding,
                                      QGis::WKBTYPE vectortype,
                                      const std::list<std::pair<QString, QString> >& attributes)
{
    OGRSFDriver* driver;
    OGRRegisterAll();
    driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(format);
    if(driver == NULL)
    {
	return false;
    }

    OGRDataSource* dataSource;
    dataSource = driver->CreateDataSource(QFile::encodeName(uri).constData(), NULL);
    if(dataSource == NULL)
    {
	return false;
    }

    //consider spatial reference system
    OGRSpatialReference* reference = NULL;
    QgsSpatialRefSys mySpatialRefSys;
    mySpatialRefSys.validate();
    QString myWKT = mySpatialRefSys.toWkt();
    
    if( !myWKT.isNull()  &&  myWKT.length() != 0 )
    {
	reference = new OGRSpatialReference(myWKT.toLocal8Bit().data());
    }

    // Map the qgis geometry type to the OGR geometry type
    OGRwkbGeometryType OGRvectortype = wkbUnknown;
    switch (vectortype)
    {
    case QGis::WKBPoint:
      OGRvectortype = wkbPoint;
      break;
    case QGis::WKBLineString:
      OGRvectortype = wkbLineString;
      break;
    case QGis::WKBPolygon:
      OGRvectortype = wkbPolygon;
      break;
    case QGis::WKBMultiPoint:
      OGRvectortype = wkbMultiPoint;
      break;
    case QGis::WKBMultiLineString:
      OGRvectortype = wkbMultiLineString;
      break;
    case QGis::WKBMultiPolygon:
      OGRvectortype = wkbMultiPolygon;
      break;
    default:
      {
        QgsLogger::debug("Unknown vector type of: ", (int)(vectortype), 1, 
                         __FILE__, __FUNCTION__, __LINE__);
	return false;
	break;
      }
    }

    OGRLayer* layer;	
    layer = dataSource->CreateLayer(QFile::encodeName(QFileInfo(uri).baseName()).constData(), reference, OGRvectortype, NULL);
    if(layer == NULL)
    {
	return false;
    }

    //create the attribute fields
    
    QTextCodec* codec=QTextCodec::codecForName(encoding.toLocal8Bit().data());
    
    for(std::list<std::pair<QString, QString> >::const_iterator it= attributes.begin(); it != attributes.end(); ++it)
    {
	if(it->second == "Real")
	{
	    OGRFieldDefn field(codec->fromUnicode(it->first).data(), OFTReal);
	    field.SetPrecision(3);
	    field.SetWidth(32);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
		QgsLogger::warning("creation of OFTReal field failed");
	    }
	}
	else if(it->second == "Integer")
	{
	    OGRFieldDefn field(codec->fromUnicode(it->first).data(), OFTInteger);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
		QgsLogger::warning("creation of OFTInteger field failed");
	    }
	}
	else if(it->second == "String")
	{
	    OGRFieldDefn field(codec->fromUnicode(it->first).data(), OFTString);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
	      QgsLogger::warning("creation of OFTString field failed");
	    }
	}
    }

    OGRDataSource::DestroyDataSource(dataSource);

    QgsDebugMsg( QString("GDAL Version number %1").arg( GDAL_VERSION_NUM ) );
#if GDAL_VERSION_NUM >= 1310
    if(reference)
    {
    reference->Release();
    }
#endif //GDAL_VERSION_NUM
    return true;
}



QgsSpatialRefSys QgsOgrProvider::getSRS()
{
  QgsDebugMsg("QgsOgrProvider::getSRS()");

  QgsSpatialRefSys srs;
  
  OGRSpatialReference * mySpatialRefSys = ogrLayer->GetSpatialRef();
  if (mySpatialRefSys == NULL)
  {
    QgsDebugMsg("no spatial reference found"); 
  }
  else
  {
    // if appropriate, morph the projection from ESRI form
    QString fileName = ogrDataSource->GetName();
    QgsDebugMsg("Data source file name is : " + fileName); 
    if(fileName.contains(".shp"))
    {
      QgsDebugMsg("Morphing " + fileName + " WKT from ESRI"); 
      // morph it
      mySpatialRefSys->morphFromESRI();
    }
    // get the proj4 text
    char * ppszProj4;
    mySpatialRefSys->exportToProj4   ( &ppszProj4 );
    QgsDebugMsg(ppszProj4); 
    char    *pszWKT = NULL;
    mySpatialRefSys->exportToWkt( &pszWKT );
    QString myWKTString = QString(pszWKT);
    OGRFree(pszWKT);  
    
    // create SRS from WKT
    srs.createFromWkt( myWKTString );
  }

  return srs;
}
