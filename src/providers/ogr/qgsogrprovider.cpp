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
//Added by qt3to4:
#include <Q3CString>

#ifndef WIN32
#include <netinet/in.h>
#endif
#include <iostream>
#include <cfloat>
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
#include <QMessageBox>
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


#include "qgssearchtreenode.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgis.h"


#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

static const QString TEXT_PROVIDER_KEY = "ogr";
static const QString TEXT_PROVIDER_DESCRIPTION = 
               QString("OGR data provider")
             + " (compiled against GDAL/OGR library version "
             + GDAL_RELEASE_NAME
             + ", running against GDAL/OGR library version "
             + GDALVersionInfo("RELEASE_NAME")
             + ")";



QgsOgrProvider::QgsOgrProvider(QString const & uri)
    : QgsVectorDataProvider(uri), ogrDataSource(0), extent_(0), ogrLayer(0), ogrDriver(0), minmaxcachedirty(true)
{
  OGRRegisterAll();

  // set the selection rectangle pointer to 0
  mSelectionRectangle = 0;
  // make connection to the data source

  QgsDebugMsg("Data source uri is " + uri);

  // try to open for update
  ogrDataSource = OGRSFDriverRegistrar::Open(QFile::encodeName(uri).constData(), TRUE, &ogrDriver);
  if(ogrDataSource == NULL)
  {
    // try to open read-only
    ogrDataSource = OGRSFDriverRegistrar::Open(QFile::encodeName(uri).constData(), FALSE, &ogrDriver);

    //TODO Need to set a flag or something to indicate that the layer
    //TODO is in read-only mode, otherwise edit ops will fail
    // TODO: capabilities() should now reflect this; need to test.
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

  //resize the cache matrix
  minmaxcache=new double*[fieldCount()];
  for(int i=0;i<fieldCount();i++)
  {
    minmaxcache[i]=new double[2];
  }
  // create the geos objects
  geometryFactory = new geos::GeometryFactory();
  assert(geometryFactory!=0);
  // create the reader
  //    std::cerr << "Creating the wktReader\n";
  wktReader = new geos::WKTReader(geometryFactory);

  mNumericalTypes.push_back("Integer");
  mNumericalTypes.push_back("Real");
  mNonNumericalTypes.push_back("String");
}

QgsOgrProvider::~QgsOgrProvider()
{
  for(int i=0;i<fieldCount();i++)
  {
    delete[] minmaxcache[i];
  }
  delete[] minmaxcache;

  OGRDataSource::DestroyDataSource(ogrDataSource);
  ogrDataSource = 0;
  delete extent_;
  extent_ = 0;
  delete geometryFactory;
  delete wktReader;
}

void QgsOgrProvider::setEncoding(const QString& e)
{
    QgsVectorDataProvider::setEncoding(e);
    loadFields();
}

void QgsOgrProvider::loadFields()
{
    //the attribute fields need to be read again when the encoding changes
    attributeFields.clear();
    OGRFeatureDefn* fdef = ogrLayer->GetLayerDefn();
    if(fdef)
    {
      geomType = fdef->GetGeomType();
      for(int i=0;i<fdef->GetFieldCount();++i)
      {
        OGRFieldDefn *fldDef = fdef->GetFieldDefn(i);
        OGRFieldType type = type = fldDef->GetType();
        bool numeric = (type == OFTInteger || type == OFTReal);
        attributeFields.push_back(QgsField(
              mEncoding->toUnicode(fldDef->GetNameRef()), 
              mEncoding->toUnicode(fldDef->GetFieldTypeName(type)),
              fldDef->GetWidth(),
              fldDef->GetPrecision(),
              numeric));
      }
    }
}

QString QgsOgrProvider::getProjectionWKT()
{ 
  QgsDebugMsg("QgsOgrProvider::getProjectionWKT()");
  OGRSpatialReference * mySpatialRefSys = ogrLayer->GetSpatialRef();
  if (mySpatialRefSys == NULL)
  {
    QgsLogger::critical("QgsOgrProvider::getProjectionWKT() --- no wkt found..returning null"); 
    return NULL;
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
    return myWKTString;
  }
}


QString QgsOgrProvider::storageType()
{
  // Delegate to the driver loaded in by OGR
  return ogrDriverName;
}


/**
 * Get the first feature resutling from a select operation
 * @return QgsFeature
 */
QgsFeature * QgsOgrProvider::getFirstFeature(bool fetchAttributes)
{
  QgsFeature *f = 0;

  if(valid)
  {
    QgsDebugMsg("getting first feature");
    ogrLayer->ResetReading();

    OGRFeature * feat = ogrLayer->GetNextFeature();

    Q_CHECK_PTR( feat  );

    if(feat)
    {
      QgsDebugMsg("First feature is not null");
    }
    else
    {
      QgsLogger::warning("First feature is null");
      return 0x0;               // so return a null feature indicating that we got a null feature
    }

    // get the feature type name, if any
    OGRFeatureDefn * featureDefinition = feat->GetDefnRef();
    QString featureTypeName =   
      featureDefinition ? QString(featureDefinition->GetName()) : QString("");

    f = new QgsFeature(feat->GetFID(), featureTypeName );

    Q_CHECK_PTR( f );

    if ( ! f )                  // return null if we can't get a new QgsFeature
    {
      delete feat;

      return 0x0;
    }

    size_t geometry_size = feat->GetGeometryRef()->WkbSize();
    f->setGeometryAndOwnership(getGeometryPointer(feat), geometry_size);

    if(fetchAttributes)
    {
      getFeatureAttributes(feat, f);
    }

    delete feat;

  }

  return f;

} // QgsOgrProvider::getFirstFeature()




bool QgsOgrProvider::getNextFeature(QgsFeature &f, bool fetchAttributes)
{
  bool returnValue;
  if(valid){
    //std::cerr << "getting next feature\n";
    // skip features without geometry
    OGRFeature *fet;
    while ((fet = ogrLayer->GetNextFeature()) != NULL) {
      if (fet->GetGeometryRef())
        break;
    }
    if(fet){
      OGRGeometry *geom = fet->GetGeometryRef();

      // get the wkb representation
      unsigned char *feature = new unsigned char[geom->WkbSize()];
      geom->exportToWkb((OGRwkbByteOrder) endian(), feature);
      f.setFeatureId(fet->GetFID());
      f.setGeometryAndOwnership(feature, geom->WkbSize());

      OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
      QString featureTypeName =   
        featureDefinition ? QString(featureDefinition->GetName()) : QString("");
      f.typeName( featureTypeName );

      if(fetchAttributes){
        getFeatureAttributes(fet, &f);
      }
      /*   char *wkt = new char[2 * geom->WkbSize()];
           geom->exportToWkt(&wkt);
           f->setWellKnownText(wkt);
           delete[] wkt;  */
      delete fet;
      returnValue = true;
    }else{
#ifdef QGISDEBUG
      QgsLogger::warning("Feature is null");
      f.setValid(false);
      returnValue = false;
#endif
      // probably should reset reading here
      ogrLayer->ResetReading();
    }


  }else{
    QgsLogger::critical("Read attempt on an invalid shapefile data source");
  }
  return returnValue;
}

/**
 * Get the next feature resutling from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
QgsFeature *QgsOgrProvider::getNextFeature(bool fetchAttributes)
{
   if(!valid)
   {
       QgsLogger::critical("Read attempt on an invalid shapefile data source");
       return 0;
   }
   
   OGRFeature* fet;
   OGRGeometry* geom;
   QgsFeature *f = 0;
   while((fet = ogrLayer->GetNextFeature()) != NULL)
   {
     
     if (fet->GetGeometryRef())
       {
	   geom = fet->GetGeometryRef();
	   // get the wkb representation
	   unsigned char *feature = new unsigned char[geom->WkbSize()];
	   geom->exportToWkb((OGRwkbByteOrder) endian(), feature);
	   OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
	   QString featureTypeName =   featureDefinition ? QString(featureDefinition->GetName()) : QString("");

	   f = new QgsFeature(fet->GetFID(), featureTypeName);
	   f->setGeometryAndOwnership(feature, geom->WkbSize());
	   if(fetchAttributes  /*|| !mAttributeFilter.isEmpty()*/)
	   {
	       getFeatureAttributes(fet, f);
	   
        // filtering by attribute
        // TODO: improve, speed up!
/*        if (!mAttributeFilter.isEmpty() && !mAttributeFilter.tree()->checkAgainst(f->attributeMap()))
        {
          delete fet;
          continue;
        }    */
	   }
    
	   if(mUseIntersect)
	   {
	       geos::Geometry *geosGeom = 0;
	       geosGeom=f->geosGeometry();
	       assert(geosGeom != 0);
         
	       char *sWkt = new char[2 * mSelectionRectangle->WkbSize()];
	       mSelectionRectangle->exportToWkt(&sWkt);  
	       geos::Geometry *geosRect = wktReader->read(sWkt);
	       assert(geosRect != 0);
	       if(geosGeom->intersects(geosRect))
	       {
		   QgsDebugMsg("intersection found");
		   delete[] sWkt;
		   delete geosGeom;
		   break;
	       }
	       else
	       {
		   QgsDebugMsg("no intersection found");
		   delete[] sWkt;
		   delete geosGeom;
		   delete f;
		   f=0;
	       }
	   }
	   else
	   {
	       break;
	   }
       }
	   delete fet;
   }
   return f;
}

/**
 * Get the next feature resutling from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
/*QgsFeature *QgsOgrProvider::getNextFeature(bool fetchAttributes)
{
  QgsFeature *f = 0;
  if(valid){
    OGRFeature *fet;
    OGRGeometry *geom;
    while ((fet = ogrLayer->GetNextFeature()) != NULL) {
      if (fet->GetGeometryRef())
	{
        if(mUseIntersect)
        {
          geom  =  fet->GetGeometryRef();
          char *wkt = new char[2 * geom->WkbSize()];
          geom->exportToWkt(&wkt);
          geos::Geometry *geosGeom = wktReader->read(wkt);
          assert(geosGeom != 0);
          QgsDebugMsg("Geometry type of geos object is : " + geosGeom->getGeometryType()); 
          // get the selection rectangle and create a geos geometry from it
          char *sWkt = new char[2 * mSelectionRectangle->WkbSize()];
          mSelectionRectangle->exportToWkt(&sWkt);
          std::cerr << "Passing " << sWkt << " to goes\n";    
          geos::Geometry *geosRect = wktReader->read(sWkt);
          assert(geosRect != 0);
          std::cerr << "About to apply intersects function\n";
          // test the geometry
          if(geosGeom->intersects(geosRect))
          {
            std::cerr << "Intersection found\n";
            break;
          }
          //XXX For some reason deleting these on win32 causes segfault
          //XXX Someday I'll figure out why...
          //delete[] wkt;  
          //delete[] sWkt;  
        }
        else
        {
          break;
        }
      }
    }
    if(fet){
      geom = fet->GetGeometryRef();

      // get the wkb representation
      unsigned char *feature = new unsigned char[geom->WkbSize()];
      geom->exportToWkb((OGRwkbByteOrder) endian(), feature);

      OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
      QString featureTypeName =   
        featureDefinition ? QString(featureDefinition->GetName()) : QString("");

      f = new QgsFeature(fet->GetFID(), featureTypeName);
      f->setGeometryAndOwnership(feature, geom->WkbSize());

      if(fetchAttributes){
        getFeatureAttributes(fet, f);
      }
      delete fet;
    }else{
#ifdef QGISDEBUG
      std::cerr << "Feature is null\n";
#endif
      // probably should reset reading here
      ogrLayer->ResetReading();
    }


  }else{
#ifdef QGISDEBUG    
    std::cerr << "Read attempt on an invalid shapefile data source\n";
#endif
  }
  return f;
}*/


QgsFeature *QgsOgrProvider::getNextFeature(std::list<int> const& attlist, int featureQueueSize)
{
  QgsFeature *f = 0; 
  if(valid)
  {
    // skip features without geometry
    OGRFeature *fet;
    while ((fet = ogrLayer->GetNextFeature()) != NULL) {

      if (fet->GetGeometryRef())
      {
        if(mUseIntersect)
        {
          // test this geometry to see if it should be
          // returned 
	  //QgsDebugMsg("Testing geometry using intersect");
        }
        else
        {
	  //QgsDebugMsg("Testing geometry using mbr");
          break;
        }
      }
    }
    if(fet)
    {
      OGRGeometry *geom = fet->GetGeometryRef();
      // get the wkb representation
      unsigned char *feature = new unsigned char[geom->WkbSize()];
      geom->exportToWkb((OGRwkbByteOrder) endian(), feature);
      OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
      QString featureTypeName =   
        featureDefinition ? QString(featureDefinition->GetName()) : QString("");

      f = new QgsFeature(fet->GetFID(), featureTypeName);
      f->setGeometryAndOwnership(feature, geom->WkbSize());
      for(std::list<int>::const_iterator it=attlist.begin();it!=attlist.end();++it)
      {
        getFeatureAttribute(fet,f,*it);
      }
      delete fet;
      //delete [] feature;
    }
    else
    {
      QgsDebugMsg("Feature is null");  
      // probably should reset reading here
      ogrLayer->ResetReading();
    }
  }
  else
  {
    QgsLogger::warning("Read attempt on an invalid shapefile data source");
  }
  return f;
}

/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 * @param useIntersect If true, an intersect test will be used in selecting
 * features. In OGR, this is a two pass affair. The mUseIntersect value is
 * stored. If true, a secondary filter (using GEOS) is applied to each
 * feature in the getNextFeature function.
 */
void QgsOgrProvider::select(QgsRect *rect, bool useIntersect)
{
  mUseIntersect = useIntersect;
  // spatial query to select features
#ifdef QGISDEBUG
  QgsLogger::debug<QgsRect>("Selection rectangle is: ", *rect, __FILE__, __FUNCTION__, __LINE__, 1);
#endif
  OGRGeometry *filter = 0;
  filter = OGRGeometryFactory::createGeometry(wkbPolygon);
  QString wktExtent = QString("POLYGON ((%1))").arg(rect->asPolygon());
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
  wktExtent = QString("POLYGON ((%1))").arg(rect->asPolygon());
  wktText = (const char *)wktExtent;

  OGRErr result = ((OGRPolygon *) filter)->importFromWkt((char **)&wktText);
  //TODO - detect an error in setting the filter and figure out what to
  //TODO   about it. If setting the filter fails, all records will be returned
  if (result == OGRERR_NONE) 
  {
    QgsDebugMsg("Setting spatial filter using " + wktExtent);
    ogrLayer->SetSpatialFilter(filter);
    //ogrLayer->SetSpatialFilterRect(rect->xMin(), rect->yMin(), rect->xMax(), rect->yMax());
  }else{
#ifdef QGISDEBUG    
    QgsLogger::warning("Setting spatial filter failed!");
    assert(result==OGRERR_NONE);
#endif
  }
  OGRGeometryFactory::destroyGeometry(filter);  
} // QgsOgrProvider::select



/**
 * Identify features within the search radius specified by rect
 * @param rect Bounding rectangle of search radius
 * @return std::vector containing QgsFeature objects that intersect rect
 */
std::vector<QgsFeature>& QgsOgrProvider::identify(QgsRect * rect)
{
  // select the features
  select(rect);
#ifdef WIN32
  //TODO fix this later for win32
  std::vector<QgsFeature> feat;
  return feat;
#endif
}

unsigned char * QgsOgrProvider::getGeometryPointer(OGRFeature *fet){
  OGRGeometry *geom = fet->GetGeometryRef();
  unsigned char *gPtr=0;
  // get the wkb representation
  gPtr = new unsigned char[geom->WkbSize()];

  geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
  return gPtr;

}


QgsRect *QgsOgrProvider::extent()
{
  mExtentRect.set(extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
  return &mExtentRect;
}


size_t QgsOgrProvider::layerCount() const
{
    return ogrDataSource->GetLayerCount();
} // QgsOgrProvider::layerCount()


/** 
 * Return the feature type
 */
int QgsOgrProvider::geometryType() const
{
  return geomType;
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
int QgsOgrProvider::fieldCount() const
{
  return attributeFields.size();
}

void QgsOgrProvider::getFeatureAttribute(OGRFeature * ogrFet, QgsFeature * f, int attindex)
{
  OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(attindex);

  if ( ! fldDef )
  {
    QgsDebugMsg("ogrFet->GetFieldDefnRef(attindex) returns NULL");
    return;
  }

  QString fld = fldDef->GetNameRef();
  Q3CString cstr(ogrFet->GetFieldAsString(attindex));
  bool numeric = attributeFields[attindex].isNumeric();

  f->addAttribute(fld, mEncoding->toUnicode(cstr), numeric);
}

/**
 * Fetch attributes for a selected feature
 */
void QgsOgrProvider::getFeatureAttributes(OGRFeature *ogrFet, QgsFeature *f){
  for (int i = 0; i < ogrFet->GetFieldCount(); i++) {
    getFeatureAttribute(ogrFet,f,i);
    // add the feature attributes to the tree
    /*OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(i);
      QString fld = fldDef->GetNameRef();
    //    OGRFieldType fldType = fldDef->GetType();
    QString val;

    val = ogrFet->GetFieldAsString(i);
    f->addAttribute(fld, val);*/
  }
}

std::vector<QgsField> const & QgsOgrProvider::fields() const
{
  return attributeFields;
}

void QgsOgrProvider::reset()
{
  // TODO: check whether it supports normal SQL or only that "restricted_sql"
  if (mAttributeFilter.isEmpty())
    ogrLayer->SetAttributeFilter(NULL);
  else
    ogrLayer->SetAttributeFilter(mAttributeFilter.string());

  ogrLayer->SetSpatialFilter(0);
  ogrLayer->ResetReading();
  // Reset the use intersect flag on a provider reset, otherwise only the last
  // selected feature(s) will be displayed when the attribute table
  // is opened. 
  //XXX In a future release, this "feature" can be used to implement
  // the display of only selected records in the attribute table.
  mUseIntersect = false;
}

QString QgsOgrProvider::minValue(int position)
{
  if(position>=fieldCount())
  {
#ifdef QGISDEBUG
    QgsLogger::warning("Warning: access requested to invalid position in QgsOgrProvider::minValue(..)");
#endif
  }
  if(minmaxcachedirty)
  {
    fillMinMaxCash();
  }
  return QString::number(minmaxcache[position][0],'f',2);
}


QString QgsOgrProvider::maxValue(int position)
{
  if(position>=fieldCount())
  {
#ifdef QGISDEBUG
    QgsLogger::warning("Warning: access requested to invalid position in QgsOgrProvider::maxValue(..)");
#endif    
  }
  if(minmaxcachedirty)
  {
    fillMinMaxCash();
  }
  return QString::number(minmaxcache[position][1],'f',2);
}

void QgsOgrProvider::fillMinMaxCash()
{
  for(int i=0;i<fieldCount();i++)
  {
    minmaxcache[i][0]=DBL_MAX;
    minmaxcache[i][1]=-DBL_MAX;
  }

  QgsFeature* f=getFirstFeature(true);
  do
  {
    for(int i=0;i<fieldCount();i++)
    {
      double value=(f->attributeMap())[i].fieldValue().toDouble();
      if(value<minmaxcache[i][0])
      {
        minmaxcache[i][0]=value;  
      }  
      if(value>minmaxcache[i][1])
      {
        minmaxcache[i][1]=value;  
      }
    }
    delete f;

  }while(f=getNextFeature(true));

  minmaxcachedirty=false;
}

//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid()
{
  return valid;
}

bool QgsOgrProvider::addFeature(QgsFeature* f)
{ 
  bool returnValue = true;
  OGRFeatureDefn* fdef=ogrLayer->GetLayerDefn();
  OGRFeature* feature=new OGRFeature(fdef);
  QGis::WKBTYPE ftype;
  OGRErr err;
  memcpy(&ftype, (f->getGeometry()+1), sizeof(int));
  switch(ftype)
  {
    case QGis::WKBPoint:
      {
        OGRPoint* p=new OGRPoint();
        p->importFromWkb(f->getGeometry(),1+sizeof(int)+2*sizeof(double));
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
        memcpy(&length,f->getGeometry()+1+sizeof(int),sizeof(int));
        l->importFromWkb(f->getGeometry(),1+2*sizeof(int)+2*length*sizeof(double));
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
        unsigned char* ptr=f->getGeometry()+1+sizeof(int);
        memcpy(&numrings,ptr,sizeof(int));
        ptr+=sizeof(int);
        for(int i=0;i<numrings;++i)
        {
          memcpy(&numpoints,ptr,sizeof(int));
          ptr+=sizeof(int);
          totalnumpoints+=numpoints;
          ptr+=(2*sizeof(double));
        }
        pol->importFromWkb(f->getGeometry(),1+2*sizeof(int)+numrings*sizeof(int)+totalnumpoints*2*sizeof(double));
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
        memcpy(&count,f->getGeometry()+1+sizeof(int),sizeof(int));
        multip->importFromWkb(f->getGeometry(),1+2*sizeof(int)+count*2*sizeof(double));
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
        memcpy(&numlines,f->getGeometry()+1+sizeof(int),sizeof(int));
        int totalpoints=0;
        int numpoints;//number of point in one line
        unsigned char* ptr=f->getGeometry()+9;
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
        multil->importFromWkb(f->getGeometry(),size);
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
        memcpy(&numpolys,f->getGeometry()+1+sizeof(int),sizeof(int));
        int numrings;//number of rings in one polygon
        int totalrings=0;
        int totalpoints=0;
        int numpoints;//number of points in one ring
        unsigned char* ptr=f->getGeometry()+9;

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
        multipol->importFromWkb(f->getGeometry(),size);
        err = feature->SetGeometry(multipol);
	if(err != OGRERR_NONE)
	  {
	    delete multipol;
	    return false;
	  }
        break;
      }
  }

  //add possible attribute information
  for(int i=0;i<f->attributeMap().size();++i)
  {
    QString s=(f->attributeMap())[i].fieldValue();
    if(!s.isEmpty())
    {
      if(fdef->GetFieldDefn(i)->GetType()==OFTInteger)
      {
        feature->SetField(i,s.toInt());
      }
      else if(fdef->GetFieldDefn(i)->GetType()==OFTReal)
      {
        feature->SetField(i,s.toDouble());
      }
      else if(fdef->GetFieldDefn(i)->GetType()==OFTString)
      {
	  feature->SetField(i,s.ascii());
      }
      else
      {
        QgsLogger::warning("QgsOgrProvider::addFeature, no type found");
      }
    }
  }

  if(ogrLayer->CreateFeature(feature)!=OGRERR_NONE)
  {
    QgsLogger::warning("Writing of the feature failed");
    returnValue = false;
  }
  ++numberFeatures;
  delete feature;
  ogrLayer->SyncToDisk();
  return returnValue;
}

bool QgsOgrProvider::addFeatures(std::list<QgsFeature*> const flist)
{
  bool returnvalue=true;
  for(std::list<QgsFeature*>::const_iterator it=flist.begin();it!=flist.end();++it)
  {
    if(!addFeature(*it))
    {
      returnvalue=false;
    }
  }
  return returnvalue;
}

bool QgsOgrProvider::addAttributes(std::map<QString,QString> const & name)
{
    bool returnvalue=true;

    for(std::map<QString,QString>::const_iterator iter=name.begin();iter!=name.end();++iter)
    {
	if(iter->second=="OFTInteger")
	{
	    OGRFieldDefn fielddefn(iter->first,OFTInteger);
	    if(ogrLayer->CreateField(&fielddefn)!=OGRERR_NONE)
	    {
		QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTInteger field failed");	
		returnvalue=false;
	    }
	}
	else if(iter->second=="OFTReal")
	{
	    OGRFieldDefn fielddefn(iter->first,OFTReal);
	    if(ogrLayer->CreateField(&fielddefn)!=OGRERR_NONE)
	    {
		QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTReal field failed");
		returnvalue=false;
	    }
	}
	else if(iter->second=="OFTString")
	{
	    OGRFieldDefn fielddefn(iter->first,OFTString);
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

bool QgsOgrProvider::changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map)
{   
  std::map<int,std::map<QString,QString> > am = attr_map; // stupid, but I don't know other way to convince gcc to compile
  for( std::map<int,std::map<QString,QString> >::iterator it=am.begin();it!=am.end();++it)
  {
    long fid = (long) (*it).first;

    OGRFeature *of = ogrLayer->GetFeature ( fid );

    if ( !of ) {
      QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Cannot read feature, cannot change attributes");
      return false;
    }

    std::map<QString,QString> attr = (*it).second;

    for( std::map<QString,QString>::iterator it2 = attr.begin(); it2!=attr.end(); ++it2 )
    {
	QString name = (*it2).first;
	QString value = (*it2).second;
		
	int fc = of->GetFieldCount();
	for ( int f = 0; f < fc; f++ ) {
	    OGRFieldDefn *fd = of->GetFieldDefnRef ( f );
	    
	    if ( name.compare( fd->GetNameRef() ) == 0 ) {
		OGRFieldType type = fd->GetType();
		switch ( type ) {
		    case OFTInteger:
		        of->SetField ( f, value.toInt() );
			break;
		    case OFTReal:
		        of->SetField ( f, value.toDouble() );
			break;
		    case OFTString:
		        of->SetField ( f, value.ascii() );
			break;
		    default:
			QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Unknown field type,\
cannot change attribute");
			break;
		}

		continue;
	    }	
	}
	ogrLayer->SetFeature ( of );
    }
  }

  ogrLayer->SyncToDisk();

  return true;
}

bool QgsOgrProvider::createSpatialIndex()
{
    QString filename=getDataSourceUri().section('/',-1,-1);//find out the filename from the uri
    QString layername=filename.section('.',0,0);
    QString sql="CREATE SPATIAL INDEX ON "+layername;
    ogrDataSource->ExecuteSQL (sql.ascii(), ogrLayer->GetSpatialFilter(),"");
    //find out, if the .qix file is there
    QString indexname = getDataSourceUri();
    indexname.truncate(getDataSourceUri().length()-filename.length());
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

bool QgsOgrProvider::deleteFeatures(std::list<int> const & id)
{
  bool returnvalue=true;
  for(std::list<int>::const_iterator it=id.begin();it!=id.end();++it)
  {
    if(!deleteFeature(*it))
    {
      returnvalue=false;
    }
  }
  ogrLayer->SyncToDisk();
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
    // TRUE if the GetFeature() method works for this layer.
    {
      // TODO: Perhaps influence if QGIS caches into memory (vs read from disk every time) based on this setting.
    }

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

      // This provider can't change geometries yet anyway (cf. Postgres provider)
      // ability |= QgsVectorDataProvider::ChangeGeometries;
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
#ifdef QGISDEBUG
    QgsLogger::debug("Driver count: ", driverRegistrar->GetDriverCount(), 1, __FILE__, __FUNCTION__, __LINE__);
#endif

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

#ifdef QGISDEBUG
    qDebug() << myFileFilters;
#endif
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
QGISEXTERN bool createEmptyDataSource(const QString& uri,const QString& format, QGis::WKBTYPE vectortype, \
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
    dataSource = driver->CreateDataSource(uri, NULL);
    if(dataSource == NULL)
    {
	return false;
    }

    //consider spatial reference system
    OGRSpatialReference* reference = NULL;
    QgsSpatialRefSys mySpatialRefSys;
    mySpatialRefSys.validate();
    char* WKT;
    QString myWKT = NULL;
    if(mySpatialRefSys.toOgrSrs().exportToWkt(&WKT)==OGRERR_NONE)
    {
	myWKT=WKT;
    }
    else
    {
	QgsLogger::warning("createEmptyDataSource: export of srs to wkt failed");
    }
    if( !myWKT.isNull()  &&  myWKT.length() != 0 )
    {
	reference = new OGRSpatialReference(myWKT.toLocal8Bit().data());
    }

    OGRLayer* layer;	
    layer = dataSource->CreateLayer(QFileInfo(uri).baseName(), reference, (OGRwkbGeometryType)vectortype, NULL);
    if(layer == NULL)
    {
	return false;
    }

    //create the attribute fields
    for(std::list<std::pair<QString, QString> >::const_iterator it= attributes.begin(); it != attributes.end(); ++it)
    {
	if(it->second == "Real")
	{
	    OGRFieldDefn field(it->first, OFTReal);
	    field.SetPrecision(3);
	    field.SetWidth(32);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
		QgsLogger::warning("creation of OFTReal field failed");
	    }
	}
	else if(it->second == "Integer")
	{
	    OGRFieldDefn field(it->first, OFTInteger);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
		QgsLogger::warning("creation of OFTInteger field failed");
	    }
	}
	else if(it->second == "String")
	{
	    OGRFieldDefn field(it->first, OFTString);
	    if(layer->CreateField(&field) != OGRERR_NONE)
	    {
	      QgsLogger::warning("creation of OFTString field failed");
	    }
	}
    }

    OGRDataSource::DestroyDataSource(dataSource);

#ifdef QGISDEBUG
    QgsLogger::debug("GDAL Version number", GDAL_VERSION_NUM, 1, __FILE__, __FUNCTION__, __LINE__);
#endif
#if GDAL_VERSION_NUM >= 1310
    if(reference)
    {
    reference->Release();
    }
#endif //GDAL_VERSION_NUM
    return true;
}



