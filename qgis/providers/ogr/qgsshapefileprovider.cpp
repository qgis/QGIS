
/* QGIS data provider for ESRI Shapefile format */
/* $Id$ */

#include "qgsshapefileprovider.h"

#ifndef WIN32
#include <netinet/in.h>
#endif
#include <iostream>
#include <cfloat>

#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <cpl_error.h>
#include <qmessagebox.h>

#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"
#include "../../src/qgis.h"
#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif
QgsShapeFileProvider::QgsShapeFileProvider(QString uri):dataSourceUri(uri), minmaxcachedirty(true)
{
  OGRRegisterAll();

  // make connection to the data source
#ifdef QGISDEBUG
  std::cerr << "Data source uri is " << uri << std::endl;
#endif
  // try to open for update
  ogrDataSource = OGRSFDriverRegistrar::Open((const char *) uri,TRUE);
  if(ogrDataSource == NULL)
  {
    // try to open read-only
    ogrDataSource = OGRSFDriverRegistrar::Open((const char *) uri,FALSE);
  //TODO Need to set a flag or something to indicate that the layer
  //TODO is in read-only mode, otherwise edit ops will fail
  }
  if (ogrDataSource != NULL) {
#ifdef QGISDEBUG
    std::cerr << "Data source is valid" << std::endl;
#endif
    valid = true;
    ogrLayer = ogrDataSource->GetLayer(0);
    // get the extent_ (envelope) of the layer
#ifdef QGISDEBUG
    std::cerr << "Starting get extent\n";
#endif
    extent_ = new OGREnvelope();
    ogrLayer->GetExtent(extent_);
#ifdef QGISDEBUG
    std::cerr << "Finished get extent\n";
#endif
    // getting the total number of features in the layer
    numberFeatures = ogrLayer->GetFeatureCount();
    // check the validity of the layer
#ifdef QGISDEBUG
    std::cerr << "checking validity\n";
#endif
    OGRFeature *feat = ogrLayer->GetNextFeature();
    if (feat) {
      OGRGeometry *geom = feat->GetGeometryRef();
      if (geom) {
        geomType = geom->getGeometryType();

      } else {
        valid = false;
      }
      // Populate the field vector for this layer. The field vector contains
      // field name, type, length, and precision (if numeric)
      for (int i = 0; i < feat->GetFieldCount(); i++) {
        OGRFieldDefn *fldDef = feat->GetFieldDefnRef(i);
        attributeFields.push_back(QgsField(
              fldDef->GetNameRef(), 
              fldDef->GetFieldTypeName(fldDef->GetType()),
              fldDef->GetWidth(),
              fldDef->GetPrecision()));
      }

      delete feat;
    } else {
      valid = false;
    }

    ogrLayer->ResetReading();
#ifdef QGISDEBUG
    std::cerr << "Done checking validity\n";
#endif
  } else {
    std::cerr << "Data source is invalid" << std::endl;
    const char *er = CPLGetLastErrorMsg();
#ifdef QGISDEBUG
    std::cerr << er << std::endl;
#endif
    valid = false;
  }

  //resize the cache matrix
  minmaxcache=new double*[fieldCount()];
  for(int i=0;i<fieldCount();i++)
  {
    minmaxcache[i]=new double[2];
  }
}

QgsShapeFileProvider::~QgsShapeFileProvider()
{
  for(int i=0;i<fieldCount();i++)
  {
    delete[] minmaxcache[i];
  }
  delete[] minmaxcache;
}

/**
 * Get the first feature resutling from a select operation
 * @return QgsFeature
 */
QgsFeature * QgsShapeFileProvider::getFirstFeature(bool fetchAttributes)
{
  QgsFeature *f = 0;

  if(valid)
  {
#ifdef QGISDEBUG
    std::cerr << "getting first feature\n";
#endif
    ogrLayer->ResetReading();

    OGRFeature * feat = ogrLayer->GetNextFeature();

    Q_CHECK_PTR( feat  );

    if(feat)
    {
#ifdef QGISDEBUG
      std::cerr << "First feature is not null\n";
#endif
    }
    else
    {
#ifdef QGISDEBUG
      std::cerr << "First feature is null\n";

#endif
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
    f->setGeometry(getGeometryPointer(feat), geometry_size);

    if(fetchAttributes)
    {
      getFeatureAttributes(feat, f);
    }

    delete feat;

  }

  return f;

} // QgsShapeFileProvider::getFirstFeature()




bool QgsShapeFileProvider::getNextFeature(QgsFeature &f, bool fetchAttributes)
{
  bool returnValue;
  if(valid){
    //std::cerr << "getting next feature\n";
    OGRFeature *fet = ogrLayer->GetNextFeature();
    if(fet){
      OGRGeometry *geom = fet->GetGeometryRef();

      // get the wkb representation
      unsigned char *feature = new unsigned char[geom->WkbSize()];
      geom->exportToWkb((OGRwkbByteOrder) endian(), feature);
      f.setFeatureId(fet->GetFID());
      f.setGeometry(feature, geom->WkbSize());

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
      std::cerr << "Feature is null\n";
      f.setValid(false);
      returnValue = false;
#endif
      // probably should reset reading here
      ogrLayer->ResetReading();
    }


  }else{
#ifdef QGISDEBUG    
    std::cerr << "Read attempt on an invalid shapefile data source\n";
#endif
  }
  return returnValue;
}

/**
 * Get the next feature resutling from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
QgsFeature *QgsShapeFileProvider::getNextFeature(bool fetchAttributes)
{

  QgsFeature *f = 0;
  if(valid){
    //std::cerr << "getting next feature\n";
    OGRFeature *fet = ogrLayer->GetNextFeature();
    if(fet){
      OGRGeometry *geom = fet->GetGeometryRef();

      // get the wkb representation
      unsigned char *feature = new unsigned char[geom->WkbSize()];
      geom->exportToWkb((OGRwkbByteOrder) endian(), feature);

      OGRFeatureDefn * featureDefinition = fet->GetDefnRef();
      QString featureTypeName =   
          featureDefinition ? QString(featureDefinition->GetName()) : QString("");

      f = new QgsFeature(fet->GetFID(), featureTypeName);
      f->setGeometry(feature, geom->WkbSize());

      if(fetchAttributes){
        getFeatureAttributes(fet, f);
      }
      /*   char *wkt = new char[2 * geom->WkbSize()];
           geom->exportToWkt(&wkt);
           f->setWellKnownText(wkt);
           delete[] wkt;  */
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
}

QgsFeature *QgsShapeFileProvider::getNextFeature(std::list<int>& attlist)
{
   QgsFeature *f = 0; 
   if(valid)
   {
       OGRFeature *fet = ogrLayer->GetNextFeature();
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
	 f->setGeometry(feature, geom->WkbSize());
	 for(std::list<int>::iterator it=attlist.begin();it!=attlist.end();++it)
	 {
	     getFeatureAttribute(fet,f,*it);
	 }
	 delete fet;
       }
       else
       {
#ifdef QGISDEBUG
	   std::cerr << "Feature is null\n";
#endif  
           // probably should reset reading here
	   ogrLayer->ResetReading();
       }
   }
   else
   {
#ifdef QGISDEBUG    
    std::cerr << "Read attempt on an invalid shapefile data source\n";
#endif    
   }
   return f;
}

/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsShapeFileProvider::select(QgsRect *rect, bool useIntersect)
{
  // spatial query to select features
  //  std::cerr << "Selection rectangle is " << *rect << std::endl;
  OGRGeometry *filter = 0;
  filter = new OGRPolygon();
  QString wktExtent = QString("POLYGON ((%1))").arg(rect->stringRep());
  const char *wktText = (const char *)wktExtent;

  OGRErr result = ((OGRPolygon *) filter)->importFromWkt((char **)&wktText);
  //TODO - detect an error in setting the filter and figure out what to
  //TODO   about it. If setting the filter fails, all records will be returned
  if (result == OGRERR_NONE) {
    //      std::cerr << "Setting spatial filter using " << wktExtent    << std::endl;
    ogrLayer->SetSpatialFilter(filter);
    //      std::cerr << "Feature count: " << ogrLayer->GetFeatureCount() << std::endl;
    /*  int featureCount = 0;
        while (OGRFeature * fet = ogrLayer->GetNextFeature()) {
        if (fet) {
        select(fet->GetFID());
        if (tabledisplay) {
        tabledisplay->table()->selectRowWithId(fet->GetFID());
        (*selected)[fet->GetFID()] = true;
        }
        } 
        }
        ogrLayer->ResetReading();*/
  }else{
#ifdef QGISDEBUG    
    std::cerr << "Setting spatial filter failed!" << std::endl;
#endif
  }
}
/**
 * Set the data source specification. This may be a path or database
 * connection string
 * @uri data source specification
 */
void QgsShapeFileProvider::setDataSourceUri(QString uri)
{
  dataSourceUri = uri;
}

/**
 * Get the data source specification. This may be a path or database
 * connection string
 * @return data source specification
 */
QString QgsShapeFileProvider::getDataSourceUri()
{
  return dataSourceUri;
}

/**
 * Identify features within the search radius specified by rect
 * @param rect Bounding rectangle of search radius
 * @return std::vector containing QgsFeature objects that intersect rect
 */
std::vector<QgsFeature>& QgsShapeFileProvider::identify(QgsRect * rect)
{
  // select the features
  select(rect);
#ifdef WIN32
  //TODO fix this later for win32
  std::vector<QgsFeature> feat;
  return feat;
#endif
}

unsigned char * QgsShapeFileProvider::getGeometryPointer(OGRFeature *fet){
  OGRGeometry *geom = fet->GetGeometryRef();
  unsigned char *gPtr=0;
  // get the wkb representation
  gPtr = new unsigned char[geom->WkbSize()];

  geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
  return gPtr;

}


int QgsShapeFileProvider::endian()
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


// TODO - make this function return the real extent_
QgsRect *QgsShapeFileProvider::extent()
{
  return new QgsRect(extent_->MinX, extent_->MinY, extent_->MaxX, extent_->MaxY);
}

/** 
 * Return the feature type
 */
int QgsShapeFileProvider::geometryType(){
  return geomType;
}

/** 
 * Return the feature type
 */
long QgsShapeFileProvider::featureCount(){
  return numberFeatures;
}

/**
 * Return the number of fields
 */
int QgsShapeFileProvider::fieldCount(){
  return attributeFields.size();
}

void QgsShapeFileProvider::getFeatureAttribute(OGRFeature * ogrFet, QgsFeature * f, int attindex)
{
    OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(attindex);
    QString fld = fldDef->GetNameRef();
    QString val;
    val = ogrFet->GetFieldAsString(attindex);
    f->addAttribute(fld, val);
}

/**
 * Fetch attributes for a selected feature
 */
void QgsShapeFileProvider::getFeatureAttributes(OGRFeature *ogrFet, QgsFeature *f){
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

std::vector < QgsField > &QgsShapeFileProvider::fields()
{
  return attributeFields;
}

void QgsShapeFileProvider::reset()
{
  ogrLayer->SetSpatialFilter(0);
  ogrLayer->ResetReading();
}

QString QgsShapeFileProvider::minValue(int position)
{
  if(position>=fieldCount())
  {
    std::cerr << "Warning: access requested to invalid position in QgsShapeFileProvider::minValue(..)" << std::endl;
  }
  if(minmaxcachedirty)
  {
    fillMinMaxCash();
  }
  return QString::number(minmaxcache[position][0],'f',2);
}


QString QgsShapeFileProvider::maxValue(int position)
{
  if(position>=fieldCount())
  {
    std::cerr << "Warning: access requested to invalid position in QgsShapeFileProvider::maxValue(..)" << std::endl;
  }
  if(minmaxcachedirty)
  {
    fillMinMaxCash();
  }
  return QString::number(minmaxcache[position][1],'f',2);
}

void QgsShapeFileProvider::fillMinMaxCash()
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
bool QgsShapeFileProvider::isValid()
{
  return valid;
}

bool QgsShapeFileProvider::addFeature(QgsFeature* f)
{
  bool returnValue = true;
  OGRFeatureDefn* fdef=ogrLayer->GetLayerDefn();
  OGRFeature* feature=new OGRFeature(fdef);
  QGis::WKBTYPE ftype;
  memcpy(&ftype, (f->getGeometry()+1), sizeof(int));
  switch(ftype)
  {
      case QGis::WKBPoint:
      {
	  OGRPoint* p=new OGRPoint();
	  p->importFromWkb(f->getGeometry(),1+sizeof(int)+2*sizeof(double));
	  feature->SetGeometry(p);
	  break;
      }
      case QGis::WKBLineString:
      {
	  OGRLineString* l=new OGRLineString();
	  int length;
	  memcpy(&length,f->getGeometry()+5,sizeof(int));
#ifdef QGISDEBUG
	  qWarning("length: "+QString::number(length));
#endif
	  l->importFromWkb(f->getGeometry(),1+2*sizeof(int)+2*length*sizeof(double));
	  feature->SetGeometry(l);
	  break;
      }
  }

  if(ogrLayer->CreateFeature(feature)!=OGRERR_NONE)
  {
      //writing failed
      QMessageBox::warning (0, "Warning", "Writing of the feature failed", QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
      returnValue = false;
  }
  ogrLayer->SyncToDisk();
  return returnValue;
}

bool QgsShapeFileProvider::deleteFeature(int id)
{
/*#ifdef QGISDEBUG
    int test=ogrLayer->TestCapability("OLCDeleteFeature");
    if(!test)
    {
	    qWarning("no support for deletion of features");	
    }
#endif
    OGRErr message=ogrLayer->DeleteFeature(id);
    switch(message)
    {
	case OGRERR_UNSUPPORTED_OPERATION:
#ifdef QGISDEBUG
	    qWarning("driver does not support deletion");
#endif
	    return false;
	case OGRERR_NONE:
#ifdef QGISDEBUG
	    qWarning("deletion successfull");
#endif
	    break;
    }
    return true;*/
    return false;
}

/**
 * Class factory to return a pointer to a newly created 
 * QgsShapeFileProvider object
 */
QGISEXTERN QgsShapeFileProvider * classFactory(const char *uri)
{
  return new QgsShapeFileProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return QString("ogr");
}
/**
 * Required description function 
 */
QGISEXTERN QString description()
{
  return QString("OGR data provider (shapefile and other formats)");
} 
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */

QGISEXTERN bool isProvider()
{
  return true;
}
