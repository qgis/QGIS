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
					      mMinMaxCacheDirty(true) {
  
  // assume that it won't work
  mValid = false;
  
  // get the filename and the type parameter from the URI
  int fileNameEnd = uri.find('?');
  if (fileNameEnd == -1 || uri.mid(fileNameEnd + 1, 5) != "type=") {
    std::cerr<<"Bad URI - you need to specify the feature type"<<std::endl;
    return;
  }
  mFeatureType = uri.mid(fileNameEnd + 6);
  
  // set up the attributes and the geometry type depending on the feature type
  attributeFields.push_back(QgsField("name", "text"));
  if (mFeatureType == "waypoint") {
    mGeomType = 1;
    attributeFields.push_back(QgsField("lat", "text"));
    attributeFields.push_back(QgsField("lon", "text"));
    attributeFields.push_back(QgsField("ele", "text"));
  }
  else if (mFeatureType == "route" || mFeatureType == "track") {
    mGeomType = 2;
  }
  else {
    std::cerr<<"Unknown feature type: "<<mFeatureType<<std::endl;
    return;
  }
  attributeFields.push_back(QgsField("url", "text"));
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
  
  if (mFeatureType == "waypoint") {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    for (; mFid < data->getNumberOfWaypoints(); ++mFid) {
      const Waypoint& wpt(data->getWaypoint(mFid));
      if (boundsCheck(wpt.lon, wpt.lat)) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	char* geo = new char[21];
	std::memset(geo, 0, 21);
	geo[0] = endian();
	geo[1] = 1;
	std::memcpy(geo+5, &wpt.lon, sizeof(double));
	std::memcpy(geo+13, &wpt.lat, sizeof(double));
	result->setGeometry((unsigned char *)geo, sizeof(wkbPoint));
	result->setValid(true);
	
	// add attributes if they are wanted
	if (fetchAttributes) {
	  result->addAttribute("name", wpt.name.c_str());
	  result->addAttribute("lat", QString("%1").arg(wpt.lat));
	  result->addAttribute("lon", QString("%1").arg(wpt.lon));
	  if (wpt.ele == -std::numeric_limits<double>::max())
	    result->addAttribute("ele", "");
	  else
	    result->addAttribute("ele", QString("%1").arg(wpt.ele));
	  result->addAttribute("url", wpt.url.c_str());
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == "route") {
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
	  result->addAttribute("name", rte.name.c_str());
	  result->addAttribute("url", rte.url.c_str());
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == "track") {
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
	  result->addAttribute("name", trk.name.c_str());
	  result->addAttribute("url", trk.url.c_str());
	}
	
	++mFid;
	break;
      }
    }
  }
  
  return result;
}


QgsFeature * QgsGPXProvider::getNextFeature(std::list<int>& attlist) {
  QgsFeature* result = 0;
  std::list<int>::const_iterator iter;
  
  if (mFeatureType == "waypoint") {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    for (; mFid < data->getNumberOfWaypoints(); ++mFid) {
      const Waypoint& wpt(data->getWaypoint(mFid));
      if (boundsCheck(wpt.lon, wpt.lat)) {
	result = new QgsFeature(mFid);
	
	// some wkb voodoo
	char* geo = new char[21];
	std::memset(geo, 0, 21);
	geo[0] = endian();
	geo[1] = 1;
	std::memcpy(geo+5, &wpt.lon, sizeof(double));
	std::memcpy(geo+13, &wpt.lat, sizeof(double));
	result->setGeometry((unsigned char *)geo, sizeof(wkbPoint));
	result->setValid(true);
	
	// add attributes if they are wanted
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  switch (*iter) {
	  case 0:
	    result->addAttribute("name", wpt.name.c_str());
	    break;
	  case 1:
	    result->addAttribute("lat", QString("%1").arg(wpt.lat));
	    break;
	  case 2:
	    result->addAttribute("lon", QString("%1").arg(wpt.lon));
	    break;
	  case 3:
	    if (wpt.ele == -std::numeric_limits<double>::max())
	      result->addAttribute("ele", "");
	    else
	      result->addAttribute("ele", QString("%1").arg(wpt.ele));
	    break;
	  case 4:
	    result->addAttribute("url", wpt.url.c_str());
	    break;
	  }
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == "route") {
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
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  if (*iter == 0)
	    result->addAttribute("name", rte.name.c_str());
	  else if (*iter == 1)
	    result->addAttribute("url", rte.url.c_str());
	}
	
	++mFid;
	break;
      }
    }
  }
  
  else if (mFeatureType == "track") {
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
	for (iter = attlist.begin(); iter != attlist.end(); ++iter) {
	  if (*iter == 0)
	    result->addAttribute("name", trk.name.c_str());
	  else if (*iter == 1)
	    result->addAttribute("url", trk.url.c_str());
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
  if (mFeatureType == "waypoint")
    return data->getNumberOfWaypoints();
  if (mFeatureType == "route")
    return data->getNumberOfRoutes();
  if (mFeatureType == "track")
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

bool QgsGPXProvider::addFeature(QgsFeature* f)
{
    return false;
}

bool QgsGPXProvider::deleteFeature(int id)
{
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
  return QString("GPS eXchange format and LOC provider");
} 


/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider(){
  return true;
}


