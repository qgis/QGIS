/* QGIS data provider for ESRI Shapefile format */
/* $Id$ */
#include <iostream>
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"
#include <ogrsf_frmts.h>
#include <ogr_geometry.h>
#include <cpl_error.h>
#include "qgsshapefileprovider.h"
#include <cfloat>

QgsShapeFileProvider::QgsShapeFileProvider(QString uri):dataSourceUri(uri), minmaxcachedirty(true)
{
	OGRRegisterAll();
		
	// make connection to the data source
	#ifdef QGISDEBUG
  std::cerr << "Data source uri is " << uri << std::endl;
  #endif
	ogrDataSource = OGRSFDriverRegistrar::Open((const char *) uri);
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
	delete minmaxcache[i];
    }
    delete[] minmaxcache;
}

/**
	* Get the first feature resutling from a select operation
	* @return QgsFeature
	*/
QgsFeature *QgsShapeFileProvider::getFirstFeature(bool fetchAttributes)
{
	QgsFeature *f = 0;
	if(valid){
		#ifdef QGISDEBUG
    std::cerr << "getting first feature\n";
    #endif
		ogrLayer->ResetReading();
		OGRFeature *feat = ogrLayer->GetNextFeature();
		if(feat){
			#ifdef QGISDEBUG
      std::cerr << "First feature is not null\n";
      #endif
		}else{
			#ifdef QGISDEBUG
      std::cerr << "First feature is null\n";
      #endif
		}
		f = new QgsFeature(feat->GetFID());
		f->setGeometry(getGeometryPointer(feat));
     if(fetchAttributes){
       getFeatureAttributes(feat, f);
      }
	}
	return f;
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
            f = new QgsFeature(fet->GetFID());
            f->setGeometry(feature);
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
	/* 	int featureCount = 0;
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
	char *chkEndian = new char[4];
	memset(chkEndian, '\0', 4);
	chkEndian[0] = 0xE8;

	int *ce = (int *) chkEndian;
	int retVal;
	if (232 == *ce)
		retVal = NDR;
	else
		retVal = XDR;
	delete[]chkEndian;
	return retVal;
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
/**
* Fetch attributes for a selected feature
*/
void QgsShapeFileProvider::getFeatureAttributes(OGRFeature *ogrFet, QgsFeature *f){
  for (int i = 0; i < ogrFet->GetFieldCount(); i++) {

					// add the feature attributes to the tree
					OGRFieldDefn *fldDef = ogrFet->GetFieldDefnRef(i);
					QString fld = fldDef->GetNameRef();
			//		OGRFieldType fldType = fldDef->GetType();
					QString val;

					val = ogrFet->GetFieldAsString(i);
          f->addAttribute(fld, val);
}
}

std::vector<QgsField>& QgsShapeFileProvider::fields(){
  return attributeFields;
}

void QgsShapeFileProvider::reset(){
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
    }while(f=getNextFeature(true));

    minmaxcachedirty=false;
}
//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsShapeFileProvider::isValid(){
  return true;
}
/**
* Class factory to return a pointer to a newly created 
* QgsShapeFileProvider object
*/
extern "C" QgsShapeFileProvider * classFactory(const char *uri)
{
	return new QgsShapeFileProvider(uri);
}
/** Required key function (used to map the plugin to a data store type)
*/
extern "C" QString providerKey(){
	return QString("ogr");
}
/**
* Required description function 
*/
extern "C" QString description(){
	return QString("OGR data provider (shapefile and other formats)");
} 
/**
* Required isProvider function. Used to determine if this shared library
* is a data provider plugin
*/
extern "C" bool isProvider(){
  return true;
}

