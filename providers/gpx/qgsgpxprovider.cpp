/***************************************************************************
      qgsgpxprovider.cpp  -  Data provider for GPS eXchange files
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

    Partly based on qgsdelimitedtextprovider.cpp, (C) 2004 Gary E. Sherman
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cfloat>
#include <iostream>
#include <limits>
#include <math.h>

#include <qapp.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <qdom.h>
#include <qrect.h>

#include "../../src/qgis.h"
#include "../../src/qgsdataprovider.h"
#include "../../src/qgsfeature.h"
#include "../../src/qgsfield.h"
#include "../../src/qgsrect.h"
#include "qgsgpxprovider.h"
#include "gpsdata.h"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif


QgsGPXProvider::QgsGPXProvider(QString uri) : mDataSourceUri(uri),
					      mMinMaxCacheDirty(true),
					      mEditable(false) {
  
  // assume that it won't work
  mValid = false;
  
  // get the filename and the type parameter from the URI
  int fileNameEnd = uri.find('?');
  if (fileNameEnd == -1 || uri.mid(fileNameEnd + 1, 5) != "type=") {
    std::cerr<<"Bad URI - you need to specify the feature type"<<std::endl;
    return;
  }
  QString typeStr = uri.mid(fileNameEnd + 6);
  mFeatureType = (typeStr == "waypoint" ? WaypointType :
		  (typeStr == "route" ? RouteType : TrackType));
  
  // set up the attributes and the geometry type depending on the feature type
  attributeFields.push_back(QgsField("name", "text"));
  if (mFeatureType == WaypointType) {
    mGeomType = 1;
    attributeFields.push_back(QgsField("ele", "text"));
    attributeFields.push_back(QgsField("sym", "text"));
  }
  else if (mFeatureType == RouteType || mFeatureType == TrackType) {
    mGeomType = 2;
    attributeFields.push_back(QgsField("number", "text"));
  }
  else {
    std::cerr<<"Unknown feature type: "<<mFeatureType<<std::endl;
    return;
  }
  attributeFields.push_back(QgsField("cmt", "text"));
  attributeFields.push_back(QgsField("desc", "text"));
  attributeFields.push_back(QgsField("src", "text"));
  attributeFields.push_back(QgsField("url", "text"));
  attributeFields.push_back(QgsField("urlname", "text"));
  mFileName = uri.left(fileNameEnd);

  // set the selection rectangle to null
  mSelectionRectangle = 0;
  
  // parse the file
  data = GPSData::getData(mFileName);
  if (data == 0)
    return;
  mValid = true;
  
  // resize the cache matrix
  mMinMaxCache=new double*[attributeFields.size()];
  for(int i=0;i<attributeFields.size();i++) {
    mMinMaxCache[i]=new double[2];
  }
}


QgsGPXProvider::~QgsGPXProvider() {
  for(int i=0;i<fieldCount();i++) {
    delete mMinMaxCache[i];
  }
  delete[] mMinMaxCache;
  GPSData::releaseData(mFileName);
}


/**
 * Get the first feature resulting from a select operation
 * @return QgsFeature
 */
QgsFeature *QgsGPXProvider::getFirstFeature(bool fetchAttributes) {
  mFid = 0;
  return getNextFeature(fetchAttributes);
}


/**
 * Get the next feature resulting from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
bool QgsGPXProvider::getNextFeature(QgsFeature &feature, bool fetchAttributes){
  return false;
}


/**
 * Get the next feature resulting from a select operation
 * Return 0 if there are no features in the selection set
 * @return QgsFeature
 */
QgsFeature *QgsGPXProvider::getNextFeature(bool fetchAttributes) {
  QgsFeature* result = 0;
  
  if (mFeatureType == WaypointType) {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    for (; mFid < data->getNumberOfWaypoints()+mAddedFeatures.size(); ++mFid) {
      const Waypoint* wpt;
      if (mFid < data->getNumberOfWaypoints())
	wpt = &(data->getWaypoint(mFid));
      else
	wpt = dynamic_cast<Waypoint*>(mAddedFeatures[mFid - data->getNumberOfWaypoints()]);
      
      if (boundsCheck(wpt->lon, wpt->lat)) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	char* geo = new char[21];
	std::memset(geo, 0, 21);
	geo[0] = endian();
	geo[1] = 1;
	std::memcpy(geo+5, &wpt->lon, sizeof(double));
	std::memcpy(geo+13, &wpt->lat, sizeof(double));
	result->setGeometry((unsigned char *)geo, sizeof(wkbPoint));
	result->setValid(true);

	// add attributes if they are wanted
	if (fetchAttributes) {
	  result->addAttribute("name", wpt->name);
	  if (wpt->ele == -std::numeric_limits<double>::max())
	    result->addAttribute("ele", "");
	  else
	    result->addAttribute("ele", QString("%1").arg(wpt->ele));
	  result->addAttribute("sym", wpt->sym);
	  result->addAttribute("cmt", wpt->cmt);
	  result->addAttribute("desc", wpt->desc);
	  result->addAttribute("src", wpt->src);
	  result->addAttribute("url", wpt->url);
	  result->addAttribute("urlname", wpt->urlname);
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == RouteType) {
    // go through the routes and return the first one that is in the bounds
    // rectangle
    for (; mFid < data->getNumberOfRoutes(); ++mFid) {
      const Route& rte(data->getRoute(mFid));
      if (rte.points.size() == 0)
	continue;
      const Routepoint& rtept(rte.points[0]);
      const QgsRect& b(*mSelectionRectangle);
      if ((rte.xMax >= b.xMin()) && (rte.xMin <= b.xMax()) &&
	  (rte.yMax >= b.yMin()) && (rte.yMin <= b.yMax())) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	int nPoints = rte.points.size();
	char* geo = new char[9 + 16 * nPoints];
	std::memset(geo, 0, 9 + 16 * nPoints);
	geo[0] = endian();
	geo[1] = 2;
	std::memcpy(geo + 5, &nPoints, 4);
	for (int i = 0; i < rte.points.size(); ++i) {
	  std::memcpy(geo + 9 + 16 * i, &rte.points[i].lon, sizeof(double));
	  std::memcpy(geo + 9 + 16 * i + 8, &rte.points[i].lat, sizeof(double));
	}
	result->setGeometry((unsigned char *)geo, 9 + 16 * nPoints);
	result->setValid(true);
	
	// add attributes if they are wanted
	if (fetchAttributes) {
	  result->addAttribute("name", rte.name);
	  if (rte.number == std::numeric_limits<int>::max())
	    result->addAttribute("number", "");
	  else
	    result->addAttribute("number", QString("%1").arg(rte.number));
	  result->addAttribute("cmt", rte.cmt);
	  result->addAttribute("desc", rte.desc);
	  result->addAttribute("src", rte.src);
	  result->addAttribute("url", rte.url);
	  result->addAttribute("urlname", rte.urlname);
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == TrackType) {
    // go through the tracks and return the first one that is in the bounds
    // rectangle
    for (; mFid < data->getNumberOfTracks(); ++mFid) {
      const Track& trk(data->getTrack(mFid));
      if (trk.segments.size() == 0)
	continue;
      if (trk.segments[0].points.size() == 0)
	continue;
      const Trackpoint& trkpt(trk.segments[0].points[0]);
      const QgsRect& b(*mSelectionRectangle);
      if ((trk.xMax >= b.xMin()) && (trk.xMin <= b.xMax()) &&
	  (trk.yMax >= b.yMin()) && (trk.yMin <= b.yMax())) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	int nPoints = trk.segments[0].points.size();
	char* geo = new char[9 + 16 * nPoints];
	std::memset(geo, 0, 9 + 16 * nPoints);
	geo[0] = endian();
	geo[1] = 2;
	std::memcpy(geo + 5, &nPoints, 4);
	for (int i = 0; i < nPoints; ++i) {
	  std::memcpy(geo + 9 + 16 * i, &trk.segments[0].points[i].lon, sizeof(double));
	  std::memcpy(geo + 9 + 16 * i + 8, &trk.segments[0].points[i].lat, sizeof(double));
	}
	result->setGeometry((unsigned char *)geo, 9 + 16 * nPoints);
	result->setValid(true);
	
	// add attributes if they are wanted
	if (fetchAttributes) {
	  result->addAttribute("name", trk.name);
	  if (trk.number == std::numeric_limits<int>::max())
	    result->addAttribute("number", "");
	  else
	    result->addAttribute("number", QString("%1").arg(trk.number));
	  result->addAttribute("cmt", trk.cmt);
	  result->addAttribute("desc", trk.desc);
	  result->addAttribute("src", trk.src);
	  result->addAttribute("url", trk.url);
	  result->addAttribute("urlname", trk.urlname);
	}
	
	++mFid;
	break;
      }
    }
  }
  
  return result;
}


QgsFeature * QgsGPXProvider::getNextFeature(std::list<int>& attlist, bool getnotcommited) {
  QgsFeature* result = 0;
  std::list<int>::const_iterator iter;
  
  if (mFeatureType == WaypointType) {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    int maxFid = data->getNumberOfWaypoints() +
      (getnotcommited ? mAddedFeatures.size() : 0);
    
    for (; mFid < maxFid; ++mFid) {
      const Waypoint* wpt;
      if (mFid < data->getNumberOfWaypoints())
	wpt = &(data->getWaypoint(mFid));
      else {
	wpt = dynamic_cast<Waypoint*>
	  (mAddedFeatures[mFid - data->getNumberOfWaypoints()]);
      }
      if (boundsCheck(wpt->lon, wpt->lat)) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	char* geo = new char[21];
	std::memset(geo, 0, 21);
	geo[0] = endian();
	geo[1] = 1;
	std::memcpy(geo+5, &wpt->lon, sizeof(double));
	std::memcpy(geo+13, &wpt->lat, sizeof(double));
	result->setGeometry((unsigned char *)geo, sizeof(wkbPoint));
	result->setValid(true);
	
	// add attributes if they are wanted
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  switch (*iter) {
	  case 0:
	    result->addAttribute("name", wpt->name);
	    break;
	  case 1:
	    if (wpt->ele == -std::numeric_limits<double>::max())
	      result->addAttribute("ele", "");
	    else
	      result->addAttribute("ele", QString("%1").arg(wpt->ele));
	    break;
	  case 2:
	    result->addAttribute("sym", wpt->sym);
	    break;
	  case 3:
	    result->addAttribute("cmt", wpt->cmt);
	    break;
	  case 4:
	    result->addAttribute("desc", wpt->desc);
	    break;
	  case 5:
	    result->addAttribute("src", wpt->src);
	    break;
	  case 6:
	    result->addAttribute("url", wpt->url);
	    break;
	  case 7:
	    result->addAttribute("urlname", wpt->urlname);
	    break;
	  }
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == RouteType) {
    // go through the routes and return the first one that is in the bounds
    // rectangle
    int maxFid = data->getNumberOfRoutes() +
      (getnotcommited ? mAddedFeatures.size() : 0);
    for (; mFid < maxFid; ++mFid) {
      const Route* rte;
      if (mFid < data->getNumberOfRoutes())
	rte = &(data->getRoute(mFid));
      else {
	rte = dynamic_cast<Route*>
	  (mAddedFeatures[mFid - data->getNumberOfRoutes()]);
      }

      if (rte->points.size() == 0)
	continue;
      const Routepoint& rtept(rte->points[0]);
      const QgsRect& b(*mSelectionRectangle);
      if ((rte->xMax >= b.xMin()) && (rte->xMin <= b.xMax()) &&
	  (rte->yMax >= b.yMin()) && (rte->yMin <= b.yMax())) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	int nPoints = rte->points.size();
	char* geo = new char[9 + 16 * nPoints];
	std::memset(geo, 0, 9 + 16 * nPoints);
	geo[0] = endian();
	geo[1] = 2;
	std::memcpy(geo + 5, &nPoints, 4);
	for (int i = 0; i < rte->points.size(); ++i) {
	  std::memcpy(geo + 9 + 16 * i, &rte->points[i].lon, sizeof(double));
	  std::memcpy(geo + 9 + 16 * i + 8, &rte->points[i].lat, sizeof(double));
	}
	result->setGeometry((unsigned char *)geo, 9 + 16 * nPoints);
	result->setValid(true);
	
	// add attributes if they are wanted
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  if (*iter == 0)
	    result->addAttribute("name", rte->name);
	  else if (*iter == 1) {
	    if (rte->number == std::numeric_limits<int>::max())
	      result->addAttribute("number", "");
	    else
	      result->addAttribute("number", QString("%1").arg(rte->number));
	  }
	  else if (*iter == 2)
	    result->addAttribute("cmt", rte->url);
	  else if (*iter == 3)
	    result->addAttribute("desc", rte->url);
	  else if (*iter == 4)
	    result->addAttribute("src", rte->url);
	  else if (*iter == 5)
	    result->addAttribute("url", rte->url);
	  else if (*iter == 6)
	    result->addAttribute("urlname", rte->url);
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == TrackType) {
    // go through the tracks and return the first one that is in the bounds
    // rectangle
    int maxFid = data->getNumberOfTracks() +
      (getnotcommited ? mAddedFeatures.size() : 0);
    for (; mFid < maxFid; ++mFid) {
      const Track* trk;
      if (mFid < data->getNumberOfTracks())
	trk = &(data->getTrack(mFid));
      else {
	trk = dynamic_cast<Track*>
	  (mAddedFeatures[mFid - data->getNumberOfTracks()]);
      }
      
      if (trk->segments.size() == 0)
	continue;
      if (trk->segments[0].points.size() == 0)
	continue;
      const Trackpoint& trkpt(trk->segments[0].points[0]);
      const QgsRect& b(*mSelectionRectangle);
      if ((trk->xMax >= b.xMin()) && (trk->xMin <= b.xMax()) &&
	  (trk->yMax >= b.yMin()) && (trk->yMin <= b.yMax())) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	int nPoints = trk->segments[0].points.size();
	char* geo = new char[9 + 16 * nPoints];
	std::memset(geo, 0, 9 + 16 * nPoints);
	geo[0] = endian();
	geo[1] = 2;
	std::memcpy(geo + 5, &nPoints, 4);
	for (int i = 0; i < nPoints; ++i) {
	  std::memcpy(geo + 9 + 16 * i, &trk->segments[0].points[i].lon, sizeof(double));
	  std::memcpy(geo + 9 + 16 * i + 8, &trk->segments[0].points[i].lat, sizeof(double));
	}
	result->setGeometry((unsigned char *)geo, 9 + 16 * nPoints);
	result->setValid(true);
	
	// add attributes if they are wanted
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  if (*iter == 0)
	    result->addAttribute("name", trk->name);
	  else if (*iter == 1) {
	    if (trk->number == std::numeric_limits<int>::max())
	      result->addAttribute("number", "");
	    else
	      result->addAttribute("number", QString("%1").arg(trk->number));
	  }
	  else if (*iter == 2)
	    result->addAttribute("cmt", trk->url);
	  else if (*iter == 3)
	    result->addAttribute("desc", trk->url);
	  else if (*iter == 4)
	    result->addAttribute("src", trk->url);
	  else if (*iter == 5)
	    result->addAttribute("url", trk->url);
	  else if (*iter == 6)
	    result->addAttribute("urlname", trk->url);
	}
	
	++mFid;
	break;
      }
    }
  }
  
  return result;
}


/**
 * Select features based on a bounding rectangle. Features can be retrieved
 * with calls to getFirstFeature and getNextFeature.
 * @param mbr QgsRect containing the extent to use in selecting features
 */
void QgsGPXProvider::select(QgsRect *rect, bool useIntersect) {
  
  // Setting a spatial filter doesn't make much sense since we have to
  // compare each point against the rectangle.
  // We store the rect and use it in getNextFeature to determine if the
  // feature falls in the selection area
  mSelectionRectangle = new QgsRect(*rect);
  // Select implies an upcoming feature read so we reset the data source
  reset();
  // Reset the feature id to 0
  mFid = 0;
}


/**
 * Identify features within the search radius specified by rect
 * @param rect Bounding rectangle of search radius
 * @return std::vector containing QgsFeature objects that intersect rect
 */
std::vector<QgsFeature>& QgsGPXProvider::identify(QgsRect * rect) {
  // reset the data source since we need to be able to read through
  // all features
  reset();
  std::cerr<<"Attempting to identify features falling within "
	    <<rect->stringRep()<<std::endl; 
  // select the features
  select(rect);
  // temporary fix to get this to compile under windows
  std::vector<QgsFeature> features;
  return features;
}


/*
   unsigned char * QgsGPXProvider::getGeometryPointer(OGRFeature *fet){
   unsigned char *gPtr=0;
// get the wkb representation

//geom->exportToWkb((OGRwkbByteOrder) endian(), gPtr);
return gPtr;

}
*/
int QgsGPXProvider::endian() {
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


// Return the extent of the layer
QgsRect *QgsGPXProvider::extent() {
  return data->getExtent();
}


/** 
 * Return the feature type
 */
int QgsGPXProvider::geometryType() {
  return mGeomType;
}


/** 
 * Return the feature type
 */
long QgsGPXProvider::featureCount() {
  if (mFeatureType == WaypointType)
    return data->getNumberOfWaypoints();
  if (mFeatureType == RouteType)
    return data->getNumberOfRoutes();
  if (mFeatureType == TrackType)
    return data->getNumberOfTracks();
  return 0;
}


/**
 * Return the number of fields
 */
int QgsGPXProvider::fieldCount() {
  return attributeFields.size();
}


std::vector<QgsField>& QgsGPXProvider::fields(){
  return attributeFields;
}


void QgsGPXProvider::reset() {
  // Reset feature id to 0
  mFid = 0;
}


QString QgsGPXProvider::minValue(int position) {
  if (position >= fieldCount()) {
    std::cerr<<"Warning: access requested to invalid position "
	     <<"in QgsGPXProvider::minValue(..)"<<std::endl;
  }
  if (mMinMaxCacheDirty) {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][0],'f',2);
}


QString QgsGPXProvider::maxValue(int position) {
  if (position >= fieldCount()) {
    std::cerr<<"Warning: access requested to invalid position "
	     <<"in QgsGPXProvider::maxValue(..)"<<std::endl;
  }
  if (mMinMaxCacheDirty) {
    fillMinMaxCash();
  }
  return QString::number(mMinMaxCache[position][1],'f',2);
}


void QgsGPXProvider::fillMinMaxCash() {
  for(int i=0;i<fieldCount();i++) {
    mMinMaxCache[i][0]=DBL_MAX;
    mMinMaxCache[i][1]=-DBL_MAX;
  }

  QgsFeature f;
  reset();

  getNextFeature(f, true);
  do {
    for(int i=0;i<fieldCount();i++) {
      double value=(f.attributeMap())[i].fieldValue().toDouble();
      if(value<mMinMaxCache[i][0]) {
        mMinMaxCache[i][0]=value;  
      }  
      if(value>mMinMaxCache[i][1]) {
        mMinMaxCache[i][1]=value;  
      }
    }
  } while(getNextFeature(f, true));

  mMinMaxCacheDirty=false;
}


void QgsGPXProvider::setDataSourceUri(QString uri) {
  mDataSourceUri = uri;
}
  

QString QgsGPXProvider::getDataSourceUri() {
  return mDataSourceUri;
}


bool QgsGPXProvider::isValid(){
  return mValid;
}

bool QgsGPXProvider::startEditing() {
  mEditable = true;
  return true;
}


void QgsGPXProvider::stopEditing() {
  mEditable = false;
}


bool QgsGPXProvider::commitChanges() {
  for (int i = 0; i < mAddedFeatures.size(); ++i) {
    if (mFeatureType == WaypointType)
      data->addWaypoint(*dynamic_cast<Waypoint*>(mAddedFeatures[i]));
    else if (mFeatureType == RouteType)
      data->addRoute(*dynamic_cast<Route*>(mAddedFeatures[i]));
    else if (mFeatureType == TrackType)
      data->addTrack(*dynamic_cast<Track*>(mAddedFeatures[i]));
    
    delete mAddedFeatures[i];
  }
  mAddedFeatures.clear();
  
  // write back to file
  QDomDocument qdd;
  data->fillDom(qdd);
  QFile file(mFileName);
  if (file.open(IO_WriteOnly)) {
    QTextStream stream(&file);
    stream<<qdd.toString();
  }
  else {
    std::cerr<<"Could not write \""<<mFileName<<"\""<<std::endl;
    return false;
  }

  return true;
}


bool QgsGPXProvider::rollBack() {
  for (int i = 0; i < mAddedFeatures.size(); ++i)
    delete mAddedFeatures[i];
  mAddedFeatures.clear();
  return true;
}


bool QgsGPXProvider::addFeature(QgsFeature* f) {
  if (!mEditable)
    return false;
  
  GPSObject* obj = NULL;
  
  unsigned char* geo = f->getGeometry();
  int id;
  QGis::WKBTYPE ftype;
  memcpy(&ftype, (geo + 1), sizeof(int));
  
  /* Do different things for different geometry types:
     WKBPoint      -> add waypoint if this is a waypoint layer
     WKBLineString -> add track if this is a track layer, route if this is a
                      route layer
     WKBPolygon and the WKBMulti types are not supported because they don't
     make sense for GPX files. */
  switch(ftype) {
    
    // the user is trying to add a point feature
  case QGis::WKBPoint: {
    if (mFeatureType != WaypointType) {
      std::cerr<<"Tried to write point feature to non-point layer!"<<std::endl;
      return false;
    }
    mAddedFeatures.push_back(new Waypoint);
    Waypoint* wpt(dynamic_cast<Waypoint*>(mAddedFeatures[mAddedFeatures.size() - 1]));
    wpt->lat = *((double*)(geo + 5 + sizeof(double)));
    wpt->lon = *((double*)(geo + 5));
    
    // parse waypoint-specific attributes
    std::vector<QgsFeatureAttribute>::const_iterator iter;
    for (iter = f->attributeMap().begin(); 
	 iter != f->attributeMap().end(); ++iter) {
      if (iter->fieldName() == "ele") {
	bool b;
	double d = iter->fieldValue().toDouble(&b);
	if (b)
	  wpt->ele = d;
	else
	  wpt->ele = -std::numeric_limits<double>::max();
      }
      else if (iter->fieldName() == "sym")
	wpt->sym = iter->fieldValue();
    }
    
    obj = wpt;
    
    break;
  }
    
    // the user is trying to add a line feature
  case QGis::WKBLineString: {
    if (mFeatureType == WaypointType) {
      std::cerr<<"Tried to write line feature to point layer!"<<std::endl;
      return false;
    }
    
    // get the number of points
    int length;
    memcpy(&length,f->getGeometry()+1+sizeof(int),sizeof(int));
#ifdef QGISDEBUG
    qWarning("length: "+QString::number(length));
#endif
    
    GPSExtended* ext = NULL;
    
    // add route
    if (mFeatureType == RouteType) {
      mAddedFeatures.push_back(new Route);
      Route* rte(dynamic_cast<Route*>(mAddedFeatures[mAddedFeatures.size() - 1]));
      for (int i = 0; i < length; ++i) {
	Routepoint rpt;
	std::memcpy(&rpt.lon, geo + 9 + 16 * i, sizeof(double));
	std::memcpy(&rpt.lat, geo + 9 + 16 * i + 8, sizeof(double));

	// update the route bounds
	rte->xMin = (rte->xMin < rpt.lon ? rte->xMin : rpt.lon);
	rte->xMax = (rte->xMax > rpt.lon ? rte->xMax : rpt.lon);
	rte->yMin = (rte->yMin < rpt.lat ? rte->yMin : rpt.lat);
	rte->yMax = (rte->yMax > rpt.lat ? rte->yMax : rpt.lat);
	
	rte->points.push_back(rpt);
      }
      
      ext = rte;
    }
    
    // add track
    else if (mFeatureType == TrackType) {
      mAddedFeatures.push_back(new Track);
      Track* trk(dynamic_cast<Track*>(mAddedFeatures[mAddedFeatures.size() - 1]));
      TrackSegment trkSeg;
      for (int i = 0; i < length; ++i) {
	Trackpoint tpt;
	std::memcpy(&tpt.lon, geo + 9 + 16 * i, sizeof(double));
	std::memcpy(&tpt.lat, geo + 9 + 16 * i + 8, sizeof(double));
	
	// update the track bounds
	trk->xMin = (trk->xMin < tpt.lon ? trk->xMin : tpt.lon);
	trk->xMax = (trk->xMax > tpt.lon ? trk->xMax : tpt.lon);
	trk->yMin = (trk->yMin < tpt.lat ? trk->yMin : tpt.lat);
	trk->yMax = (trk->yMax > tpt.lat ? trk->yMax : tpt.lat);

	trkSeg.points.push_back(tpt);
      }
      trk->segments.push_back(trkSeg);
      
      ext = trk;
    }
    
    // parse GPSExtended-specific attributes
    std::vector<QgsFeatureAttribute>::const_iterator iter;
    for (iter = f->attributeMap().begin(); 
	 iter != f->attributeMap().end(); ++iter) {
      if (iter->fieldName() == "number") {
	bool b;
	int n = iter->fieldValue().toInt(&b);
	if (b)
	  ext->number = n;
	else
	  ext->number = std::numeric_limits<int>::max();
      }
    }
    
    obj = ext;
    
    break;
  }
    
    // unsupported geometry - something's wrong
  default:
    return false;
    
  }
  
  // parse common attributes
  std::vector<QgsFeatureAttribute>::const_iterator iter;
  for (iter = f->attributeMap().begin(); 
       iter != f->attributeMap().end(); ++iter) {
    if (iter->fieldName() == "name")
      obj->name = iter->fieldValue();
    else if (iter->fieldName() == "cmt")
      obj->cmt = iter->fieldValue();
    else if (iter->fieldName() == "desc")
      obj->desc = iter->fieldValue();
    else if (iter->fieldName() == "src")
      obj->src = iter->fieldValue();
    else if (iter->fieldName() == "url")
      obj->url = iter->fieldValue();
    else if (iter->fieldName() == "urlname")
      obj->urlname = iter->fieldValue();
  }
  
  return true;
}


bool QgsGPXProvider::deleteFeature(int id) {
  return false;
}


/** 
 * Check to see if the point is within the selection rectangle
 */
bool QgsGPXProvider::boundsCheck(double x, double y)
{
  bool inBounds = (((x < mSelectionRectangle->xMax()) &&
        (x > mSelectionRectangle->xMin())) &&
      ((y < mSelectionRectangle->yMax()) &&
       (y > mSelectionRectangle->yMin())));
  QString hit = inBounds?"true":"false";
  return inBounds;
}


/**
 * Class factory to return a pointer to a newly created 
 * QgsGPXProvider object
 */
QGISEXTERN QgsGPXProvider * classFactory(const char *uri) {
  return new QgsGPXProvider(uri);
}


/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey(){
  return QString("gpx");
}


/**
 * Required description function 
 */
QGISEXTERN QString description(){
  return QString("GPS eXchange format provider");
} 


/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider(){
  return true;
}


