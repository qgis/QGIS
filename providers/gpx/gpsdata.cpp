/***************************************************************************
      gpsdata.cpp  -  Data structures for GPS data
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <stdexcept>

#include <qfile.h>

#include "gpsdata.h"


GPSPoint::GPSPoint() {
  ele = -std::numeric_limits<double>::max();
}


GPSData::GPSData() {
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
}


QgsRect* GPSData::getExtent() const {
  return new QgsRect(xMin, yMin, xMax, yMax);
}


int GPSData::getNumberOfWaypoints() const {
  return waypoints.size();
}


int GPSData::getNumberOfRoutes() const {
  return routes.size();
}


int GPSData::getNumberOfTracks() const {
  return tracks.size();
}


Waypoint& GPSData::getWaypoint(int index) {
  if (index < 0 || index >= waypoints.size())
    throw std::out_of_range("Waypoint index is out of range");
  return waypoints[index];
}


Route& GPSData::getRoute(int index) {
  if (index < 0 || index >= routes.size())
    throw std::out_of_range("Route index is out of range");
  return routes[index];
}


Track& GPSData::getTrack(int index) {
  if (index < 0 || index >= tracks.size())
    throw std::out_of_range("Track index is out of range");
  return tracks[index];
}


int GPSData::addWaypoint(double lat, double lon, std::string name, 
			 double ele) {
  Waypoint wpt;
  wpt.lat = lat;
  wpt.lon = lon;
  wpt.name = name;
  wpt.ele = ele;
  waypoints.push_back(wpt);
  return waypoints.size() - 1;
}


int GPSData::addRoute(std::string name) {
  Route rte;
  rte.name = name;
  routes.push_back(rte);
  return routes.size() - 1;
}


int GPSData::addTrack(std::string name) {
  Track trk;
  trk.name = name;
  return tracks.size() - 1;
}
  

bool GPSData::removeWaypoint(int index, bool checkRoutes) {
  if (checkRoutes)
    throw std::logic_error("Not implemented");
  if (index < 0 || index >= waypoints.size())
    throw std::out_of_range("Waypoint index is out of range");
  waypoints.erase(waypoints.begin() + index);
  return true;
}


void GPSData::removeRoute(int index) {
  if (index < 0 || index >= routes.size())
    throw std::out_of_range("Route index is out of range");
  routes.erase(routes.begin() + index);
}


void GPSData::removeTrack(int index) {
  if (index < 0 || index >= tracks.size())
    throw std::out_of_range("Track index is out of range");
  tracks.erase(tracks.begin() + index);
}
  

std::ostream& operator<<(std::ostream& os, const GPSData& d) {
  os<<"  Waypoints:"<<std::endl;
  for (int i = 0; i < d.waypoints.size(); ++i)
    os<<"    "<<d.waypoints[i].name<<": "
      <<d.waypoints[i].lat<<", "<<d.waypoints[i].lon<<std::endl;

  os<<"  Routes:"<<std::endl;
  for (int i = 0; i < d.routes.size(); ++i)
    os<<"    "<<d.routes[i].name<<std::endl;

  os<<"  Tracks:"<<std::endl;
  for (int i = 0; i < d.tracks.size(); ++i)
    os<<"    "<<d.tracks[i].name<<std::endl;

  return os;
}


bool GPSData::parseDom(QDomDocument& qdd) {
  
  // reset the extent
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
  
  // reset the data
  waypoints.clear();
  routes.clear();
  tracks.clear();
  
  // ignore the <?xml... tags
  QDomNode node, node2, node3, node4;
  node = qdd.firstChild();
  while ((node.nodeName() != "gpx") && (node.nodeName() != "loc"))
    node = node.nextSibling();
  
  // there must be a gpx or loc element
  if (node.isNull())
    return false;
  
  // what format?
  if (node.nodeName() == "gpx")
    return parseGPX(node);
  else
    return parseLOC(node);
}


bool GPSData::parseGPX(QDomNode& node) {
  // start parsing child nodes
  node = node.firstChild();
  QDomNode node2, node3, node4;
  while (!node.isNull()) {

    // waypoint
    if (node.nodeName() == "wpt") {
      Waypoint wpt;

      // lat and lon are required
      node2 = node.attributes().namedItem("lat");
      if (node2.isNull())
	return false;
      wpt.lat = node2.nodeValue().toDouble();
      node2 = node.attributes().namedItem("lon");
      if (node2.isNull())
	return false;
      wpt.lon = node2.nodeValue().toDouble();
      
      // name is optional
      node2 = node.namedItem("name");
      if (!node2.isNull())
	wpt.name = (const char*)node2.firstChild().nodeValue();
      
      // url is optional
      node2 = node.namedItem("url");
      if (!node2.isNull())
	wpt.url = (const char*)node2.firstChild().nodeValue();
      
      // ele is optional
      node2 = node.namedItem("ele");
      if (!node2.isNull())
	wpt.ele = std::atof((const char*)node2.firstChild().nodeValue());
      else
	wpt.ele = -std::numeric_limits<double>::max();
      
      // update the extent
      xMin = xMin < wpt.lon ? xMin : wpt.lon;
      xMax = xMax > wpt.lon ? xMax : wpt.lon;
      yMin = yMin < wpt.lat ? yMin : wpt.lat;
      yMax = yMax > wpt.lat ? yMax : wpt.lat;
      
      waypoints.push_back(wpt);
    }
    
    // route
    else if (node.nodeName() == "rte") {
      Route rte;
      rte.xMin = std::numeric_limits<double>::max();
      rte.xMax = -std::numeric_limits<double>::max();
      rte.yMin = std::numeric_limits<double>::max();
      rte.yMax = -std::numeric_limits<double>::max();
      
      // routepoints are optional, empty routes are allowed
      node2 = node.namedItem("rtept");
      while (!node2.isNull()) {
	if (node2.nodeName() == "rtept") {
	  Routepoint rtept;
	  
	  // lat and lon are required for each routepoint
	  node3 = node2.attributes().namedItem("lat");
	  if (node3.isNull())
	    return false;
	  rtept.lat = node3.nodeValue().toDouble();
	  node3 = node2.attributes().namedItem("lon");
	  if (node3.isNull())
	    return false;
	  rtept.lon = node3.nodeValue().toDouble();
	  
	  // name is optional for routepoints too
	  node3 = node2.namedItem("name");
	  if (!node3.isNull())
	    rtept.name = (const char*)node3.firstChild().nodeValue();
	  
	  // ele is optional
	  node3 = node2.namedItem("ele");
	  if (!node3.isNull()) 
	    rtept.ele = std::atof((const char*)node3.firstChild().nodeValue());
	  else
	    rtept.ele = -std::numeric_limits<double>::max();
	  
	  // update the route bounds
	  rte.xMin = (rte.xMin < rtept.lon ? rte.xMin : rtept.lon);
	  rte.xMax = (rte.xMax > rtept.lon ? rte.xMax : rtept.lon);
	  rte.yMin = (rte.yMin < rtept.lat ? rte.yMin : rtept.lat);
	  rte.yMax = (rte.yMax > rtept.lat ? rte.yMax : rtept.lat);
	  
	  // update the extent
	  xMin = xMin < rte.xMin ? xMin : rte.xMin;
	  xMax = xMax > rte.xMax ? xMax : rte.xMax;
	  yMin = yMin < rte.yMin ? yMin : rte.yMin;
	  yMax = yMax > rte.yMax ? yMax : rte.yMax;
	  
	  rte.points.push_back(rtept);
	}
	node2 = node2.nextSibling();
      }
      
      // name is optional
      node2 = node.namedItem("name");
      if (!node2.isNull())
	rte.name = (const char*)node2.firstChild().nodeValue();
      
      // url is optional
      node2 = node.namedItem("url");
      if (!node2.isNull())
	rte.url = (const char*)node2.firstChild().nodeValue();
      
      routes.push_back(rte);
    }
    
    // track
    else if (node.nodeName() == "trk") {
      Track trk;
      
      // reset track bounds
      trk.xMin = std::numeric_limits<double>::max();
      trk.xMax = -std::numeric_limits<double>::max();
      trk.yMin = std::numeric_limits<double>::max();
      trk.yMax = -std::numeric_limits<double>::max();
      
      // track segments are optional - empty tracks are allowed
      node2 = node.namedItem("trkseg");
      while (!node2.isNull()) {
	if (node2.nodeName() == "trkseg") {
	  TrackSegment trkseg;
	  node3 = node2.namedItem("trkpt");
	  while (!node3.isNull()) {
	    if (node3.nodeName() == "trkpt") {
	      Trackpoint trkpt;

	      // lat and lon are required for each trackpoint
	      node4 = node3.attributes().namedItem("lat");
	      if (node4.isNull())
		return false;
	      trkpt.lat = node4.nodeValue().toDouble();
	      node4 = node3.attributes().namedItem("lon");
	      if (node4.isNull())
		return false;
	      trkpt.lon = node4.nodeValue().toDouble();
	  
	      // name is optional for trackpoints too
	      node4 = node3.namedItem("name");
	      if (!node4.isNull())
		trkpt.name = (const char*)node4.firstChild().nodeValue();
	      
	      // ele is optional
	      node4 = node.namedItem("ele");
	      if (!node4.isNull()) 
		trkpt.ele = 
		  std::atof((const char*)node4.firstChild().nodeValue());
	      else
		trkpt.ele = -std::numeric_limits<double>::max();
	      
	      // update the track bounds
	      trk.xMin = (trk.xMin < trkpt.lon ? trk.xMin : trkpt.lon);
	      trk.xMax = (trk.xMax > trkpt.lon ? trk.xMax : trkpt.lon);
	      trk.yMin = (trk.yMin < trkpt.lat ? trk.yMin : trkpt.lat);
	      trk.yMax = (trk.yMax > trkpt.lat ? trk.yMax : trkpt.lat);
	      
	      // update the extent
	      xMin = xMin < trk.xMin ? xMin : trk.xMin;
	      xMax = xMax > trk.xMax ? xMax : trk.xMax;
	      yMin = yMin < trk.yMin ? yMin : trk.yMin;
	      yMax = yMax > trk.yMax ? yMax : trk.yMax;
	      
	      trkseg.points.push_back(trkpt);
	    }
	    node3 = node3.nextSibling();
	  }
	  
	  trk.segments.push_back(trkseg);
	}
	node2 = node2.nextSibling();
      }
      
      // name is optional
      node2 = node.namedItem("name");
      if (!node2.isNull())
	trk.name = (const char*)node2.firstChild().nodeValue();
      
      // url is optional
      node2 = node.namedItem("url");
      if (!node2.isNull())
	trk.url = (const char*)node2.firstChild().nodeValue();

      tracks.push_back(trk);
    }
      
    node = node.nextSibling();
  } 
  
  return true;
}


bool GPSData::parseLOC(QDomNode& node) {
  // start parsing waypoint tags
  node = node.firstChild();
  QDomNode node2, node3;
  while (!node.isNull()) {
    if (node.nodeName() == "waypoint") {
      Waypoint wpt;
      
      // name is optional
      if (!(node2 = node.namedItem("name")).isNull())
	wpt.name = (const char*)node2.firstChild().nodeValue();
      
      // link is optional
      if (!(node2 = node.namedItem("link")).isNull())
	wpt.url = (const char*)node2.firstChild().nodeValue();
      
      // coord with lat and lon is required
      if ((node2 = node.namedItem("coord")).isNull())
	return false;
      if ((node3 = node2.attributes().namedItem("lat")).isNull())
	return false;
      wpt.lat = node3.nodeValue().toDouble();
      if ((node3 = node2.attributes().namedItem("lon")).isNull())
	return false;
      wpt.lon = node3.nodeValue().toDouble();
      
      // update the extent
      xMin = xMin < wpt.lon ? xMin : wpt.lon;
      xMax = xMax > wpt.lon ? xMax : wpt.lon;
      yMin = yMin < wpt.lat ? yMin : wpt.lat;
      yMax = yMax > wpt.lat ? yMax : wpt.lat;
      
      waypoints.push_back(wpt);
    }
    node = node.nextSibling();
  }
  return true;
}


GPSData* GPSData::getData(const QString& filename) {
  
  // if the data isn't there already, try to load it
  if (dataObjects.find(filename) == dataObjects.end()) {
    QDomDocument qdd;
    QFile file(filename);
    GPSData data;
    std::cerr<<"Loading file "<<filename<<std::endl;
    if (!(qdd.setContent(&file) && data.parseDom(qdd))) {
      std::cerr<<filename<<"is not valid GPX!"<<std::endl;
      return 0;
    }
    dataObjects[filename] = std::pair<GPSData, unsigned>(data, 0);
  }
  else
    std::cerr<<filename<<" is already loaded"<<std::endl;
  
  // return a pointer and increase the reference count for that filename
  DataMap::iterator iter = dataObjects.find(filename);
  ++(iter->second.second);
  return (GPSData*)&(iter->second.first);
}


void GPSData::releaseData(const QString& filename) {
  
  /* decrease the reference count for the filename (if it is used), and erase
     it if the reference count becomes 0 */
  DataMap::iterator iter = dataObjects.find(filename);
  if (iter != dataObjects.end()) {
    std::cerr<<"unrefing "<<filename<<std::endl;
    if (--(iter->second.second) == 0) {
      std::cerr<<"No one's using "<<filename<<", I'll erase it"<<std::endl;
      dataObjects.erase(iter);
    }
  }
}


// we have to initialize the static member
GPSData::DataMap GPSData::dataObjects;
