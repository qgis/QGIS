
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

//TODO Following ifndef can be removed once WIN32 GEOS support
//    is fixed
#ifndef NOWIN32GEOS
//XXX GEOS support on windows is broken until we can get VC++ to
//    tolerate geos.h without throwing a bunch of type errors. It
//    appears that the windows version of GEOS may be compiled with 
//    MINGW rather than VC++.
#include <geos.h>
#endif 
#include "ogr_api.h"//only for a test

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
QgsShapeFileProvider::QgsShapeFileProvider(QString uri): QgsVectorDataProvider(), dataSourceUri(uri), minmaxcachedirty(true)
{
  OGRRegisterAll();

  // set the selection rectangle pointer to 0
  mSelectionRectangle = 0;
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
    
    OGRFeatureDefn* fdef = ogrLayer->GetLayerDefn();
    if(fdef)
    {
	geomType = fdef->GetGeomType();
	for(int i=0;i<fdef->GetFieldCount();++i)
	{
	    OGRFieldDefn *fldDef = fdef->GetFieldDefn(i);
	    attributeFields.push_back(QgsField(
	    fldDef->GetNameRef(), 
	    fldDef->GetFieldTypeName(fldDef->GetType()),
	    fldDef->GetWidth(),
	    fldDef->GetPrecision()));
	}
    }

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

  //delete not commited features
  for(std::list<QgsFeature*>::iterator it=mAddedFeatures.begin();it!=mAddedFeatures.end();++it)
  {
      delete *it;
  }
  mAddedFeatures.clear();

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
    // skip features without geometry
    OGRFeature *fet;
    //TODO Following ifndef can be removed once WIN32 GEOS support
    //    is fixed
#ifndef NOWIN32GEOS
    // create the geos geometry factory
    geos::GeometryFactory *gf = new geos::GeometryFactory();
    // create the reader
    geos::WKTReader *wktReader = new geos::WKTReader(gf);
#endif 
    OGRGeometry *geom;
    while ((fet = ogrLayer->GetNextFeature()) != NULL) {
      if (fet->GetGeometryRef())
      {
        if(mUseIntersect)
        {
    //TODO Following ifndef can be removed once WIN32 GEOS support
    //    is fixed
#ifndef NOWIN32GEOS
          // Test this geometry to see if it should be
          // returned. This dies big time using the GDAL GEOS
          // functionality so we implement our own logic using
          // the geos library. The select functions has already
          // narrowed the selection to those features with the MBR
          // of the selection rectangle.
          // 
          // get the feature geometry and create a geos geometry from it
          geom  =  fet->GetGeometryRef();
          char *wkt = new char[2 * geom->WkbSize()];
          geom->exportToWkt(&wkt);
          //std::cerr << "Passing " << wkt << " to goes\n";
          geos::Geometry *geosGeom = wktReader->read(wkt);
          //std::cerr << "Geometry type of geos object is : " << geosGeom->getGeometryType() << std::endl; 
          // get the selection rectangle and create a geos geometry from it
          char *sWkt = new char[2 * mSelectionRectangle->WkbSize()];
          mSelectionRectangle->exportToWkt(&sWkt);
          //std::cerr << "Passing " << sWkt << " to goes\n";
          geos::Geometry *geosRect = wktReader->read(sWkt);
          //std::cerr << "About to apply contains function\n";

          // test the geometry
          if(geosGeom->intersects(geosRect))
          {
            break;
          }
          delete[] wkt;  
          delete[] sWkt;  
          delete geosGeom;
          delete geosRect;
#endif
        }
        else
        {
          break;
        }
      }
    }
    //TODO Following ifndef can be removed once WIN32 GEOS support
    //    is fixed
#ifndef NOWIN32GEOS
    delete gf;
    delete wktReader;
#endif 
    if(fet){
      geom = fet->GetGeometryRef();

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

QgsFeature *QgsShapeFileProvider::getNextFeature(std::list<int>& attlist, bool getnotcommited)
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
#ifdef QGISDEBUG 
             std::cerr << "Testing geometry using intersect" << std::endl; 
#endif 
           }
           else
           {
#ifdef QGISDEBUG 
             std::cerr << "Testing geometry using mbr" << std::endl; 
#endif 
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
         f->setGeometry(feature, geom->WkbSize());
         for(std::list<int>::iterator it=attlist.begin();it!=attlist.end();++it)
         {
           getFeatureAttribute(fet,f,*it);
         }
         delete fet;
       }
       else
       {
	   if(getnotcommited&&mAddedFeatures.size()>0&&mAddedFeaturesIt!=mAddedFeatures.end())
	   {
#ifdef QGISDEBUG
         qWarning("accessing feature in the cache");
#endif //QGISDEBUG
         QgsFeature* addedfeature=*mAddedFeaturesIt;
         ++mAddedFeaturesIt;
         //copy the feature because it will be deleted in QgsVectorLayer::draw()
         QgsFeature* returnf=new QgsFeature(*addedfeature);
         return returnf;
     }
#ifdef QGISDEBUG
     std::cerr << "Feature is null\n";
#endif  
           // probably should reset reading here
     ogrLayer->ResetReading();
     if(!mAddedFeatures.empty())
     {
         mAddedFeaturesIt=mAddedFeatures.begin();
     }
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
 * @param useIntersect If true, an intersect test will be used in selecting
 * features. In OGR, this is a two pass affair. The mUseIntersect value is
 * stored. If true, a secondary filter (using GEOS) is applied to each
 * feature in the getNextFeature function.
 */
void QgsShapeFileProvider::select(QgsRect *rect, bool useIntersect)
{
  mUseIntersect = useIntersect;
  // spatial query to select features
  //  std::cerr << "Selection rectangle is " << *rect << std::endl;
  OGRGeometry *filter = 0;
  filter = new OGRPolygon();
  QString wktExtent = QString("POLYGON ((%1))").arg(rect->asPolygon());
  const char *wktText = (const char *)wktExtent;

  if(useIntersect)
  {
    // store the selection rectangle for use in filtering features during
    // an identify and display attributes
    //    delete mSelectionRectangle;
    mSelectionRectangle = new OGRPolygon();
    mSelectionRectangle->importFromWkt((char **)&wktText);
  }
  // reset the extent for the ogr filter
  //
  wktExtent = QString("POLYGON ((%1))").arg(rect->stringRep());
  wktText = (const char *)wktExtent;

  OGRErr result = ((OGRPolygon *) filter)->importFromWkt((char **)&wktText);
  //TODO - detect an error in setting the filter and figure out what to
  //TODO   about it. If setting the filter fails, all records will be returned
  if (result == OGRERR_NONE) {
    //      std::cerr << "Setting spatial filter using " << wktExtent    << std::endl;
    ogrLayer->SetSpatialFilter(filter);
    //      std::cerr << "Feature count: " << ogrLayer->GetFeatureCount() << std::endl;
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
  if(!mAddedFeatures.empty())
  {
      mAddedFeaturesIt=mAddedFeatures.begin();
  }
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

bool QgsShapeFileProvider::startEditing()
{
    mEditable=true;
    return true;
}

bool QgsShapeFileProvider::commitFeature(QgsFeature* f)
{
  qWarning("try to commit a feature");
  if(mEditable)
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
          memcpy(&length,f->getGeometry()+1+sizeof(int),sizeof(int));
#ifdef QGISDEBUG
          qWarning("length: "+QString::number(length));
#endif
          l->importFromWkb(f->getGeometry(),1+2*sizeof(int)+2*length*sizeof(double));
          feature->SetGeometry(l);
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
          feature->SetGeometry(pol);
          break;
        }
      case QGis::WKBMultiPoint:
        {
          OGRMultiPoint* multip= new OGRMultiPoint();
          int count;
          //determine how many points
          memcpy(&count,f->getGeometry()+1+sizeof(int),sizeof(int));
          multip->importFromWkb(f->getGeometry(),1+2*sizeof(int)+count*2*sizeof(double));
          feature->SetGeometry(multip);
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
          feature->SetGeometry(multil);
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
          feature->SetGeometry(multipol);
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
	    }
	}
    }

    if(ogrLayer->CreateFeature(feature)!=OGRERR_NONE)
    {
      //writing failed
      QMessageBox::warning (0, "Warning", "Writing of the feature failed",
          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton );
      returnValue = false;
    }
    ++numberFeatures;
    delete feature;
    return returnValue;
  }
  else//layer not editable
  {
    return false;
  }
}

/*bool QgsShapeFileProvider::deleteFeature(int id)
{
#ifdef QGISDEBUG
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
    return true;
    return false;
}*/

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

QGISEXTERN bool createEmptyDataSource(const QString& uri,const QString& format, QGis::WKBTYPE vectortype)
{
    //hard coded for the moment
    OGRwkbGeometryType geomtype=(OGRwkbGeometryType)((int)vectortype);
    QString mOutputFormat = "ESRI Shapefile";
    QString mOutputFileName = uri;
#ifdef WIN32 
    QString outname=mOutputFileName.mid(mOutputFileName.findRev("\\")+1,mOutputFileName.length());
#else
    QString outname=mOutputFileName.mid(mOutputFileName.findRev("/")+1,mOutputFileName.length());
#endif
    OGRSFDriverRegistrar* driverregist = OGRSFDriverRegistrar::GetRegistrar();
    
    if(driverregist==0)
    {
	return false;
    }
    OGRSFDriver* driver = driverregist->GetDriverByName(mOutputFormat);
    if(driver==0)
    {
	return false;
    }
    OGRDataSource* datasource = driver->CreateDataSource(mOutputFileName,NULL);
    if(datasource==0)
    {
	return false;
    }

    OGRSpatialReference reference;
    OGRLayer* layer = datasource->CreateLayer(outname.latin1(),&reference,geomtype,NULL);
    if(layer==0)
    {
	return false;
    }

    //create a dummy field
    OGRFieldDefn fielddef("dummy",OFTReal);
    fielddef.SetWidth(1);
    fielddef.SetPrecision(1);
    if(layer->CreateField(&fielddef,FALSE)!=OGRERR_NONE)
    {
	return false;
    }

    int count=layer->GetLayerDefn()->GetFieldCount();
#ifdef QGISDEBUG
    qWarning("Field count is: "+QString::number(count));
#endif
    //just for a test: create a dummy featureO
    /*OGRFeatureDefn* fdef=layer->GetLayerDefn();
    OGRFeature* feature=new OGRFeature(fdef);
    OGRPoint* p=new OGRPoint();
    p->setX(700000);
    p->setY(300000);
    feature->SetGeometry(p);
    if(layer->CreateFeature(feature)!=OGRERR_NONE)
    {
	qWarning("errrrrrrrrrror!");
	}*/

    if(layer->SyncToDisk()!=OGRERR_NONE)
    {
	return false;
    }
    
    return true;

    /*OGRLayerH mLayerHandle;
    OGRRegisterAll();
    OGRSFDriverH myDriverHandle = OGRGetDriverByName( mOutputFormat );

    if( myDriverHandle == NULL )
    {
	std::cout << "Unable to find format driver named " << mOutputFormat << std::endl;
	return false;
    }

    OGRDataSourceH mDataSourceHandle = OGR_Dr_CreateDataSource( myDriverHandle, mOutputFileName, NULL );
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
#ifdef QGISDEBUG
    qWarning("mOutputFileName: "+mOutputFileName);
#endif //QGISDEBUG


#ifdef QGISDEBUG
  qWarning("outname: "+outname);
#endif //QGISDEBUG

  mLayerHandle = OGR_DS_CreateLayer( mDataSourceHandle, outname, 
  mySpatialReferenceSystemHandle, geomtype, NULL );
  
  if( mLayerHandle == NULL )
  {
    std::cout << "Error layer handle is null!" << std::endl;
    return false;
  }
  else
  {
    std::cout << "File handle created!" << std::endl;
  }

  OGRFieldDefnH myFieldDefinitionHandle;
  myFieldDefinitionHandle = OGR_Fld_Create( "dummy",OFTReal);
  OGR_Fld_SetWidth( myFieldDefinitionHandle,1);
  OGR_Fld_SetPrecision( myFieldDefinitionHandle,1);
  OGR_L_CreateField( mLayerHandle, myFieldDefinitionHandle, FALSE );
  OGR_Fld_Destroy( myFieldDefinitionHandle );
  
  return true;*/
}
