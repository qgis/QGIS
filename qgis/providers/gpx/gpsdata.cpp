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


bool GPSObject::parseNode(const QDomNode& node) {
  
  QDomNode node2;
  
  // name is optional
  node2 = node.namedItem("name");
  if (!node2.isNull())
    name = (const char*)node2.firstChild().nodeValue();
  
  // cmt is optional
  node2 = node.namedItem("cmt");
  if (!node2.isNull())
    cmt = (const char*)node2.firstChild().nodeValue();
  
  // desc is optional
  node2 = node.namedItem("desc");
  if (!node2.isNull())
    desc = (const char*)node2.firstChild().nodeValue();
  
  // src is optional
  node2 = node.namedItem("src");
  if (!node2.isNull())
    src = (const char*)node2.firstChild().nodeValue();
  
  // url is optional
  node2 = node.namedItem("url");
  if (!node2.isNull())
    url = (const char*)node2.firstChild().nodeValue();
  
  // urlname is optional
  node2 = node.namedItem("urlname");
  if (!node2.isNull())
    urlname = (const char*)node2.firstChild().nodeValue();
  
  return true;
}


void GPSObject::fillElement(QDomElement& elt) {
  QDomDocument qdd = elt.ownerDocument();
  if (!name.isEmpty()) {
    QDomElement nameElt = qdd.createElement("name");
    nameElt.appendChild(qdd.createTextNode(name));
    elt.appendChild(nameElt);
  }
  if (!cmt.isEmpty()) {
    QDomElement cmtElt = qdd.createElement("cmt");
    cmtElt.appendChild(qdd.createTextNode(cmt));
    elt.appendChild(cmtElt);
  }
  if (!desc.isEmpty()) {
    QDomElement descElt = qdd.createElement("desc");
    descElt.appendChild(qdd.createTextNode(desc));
    elt.appendChild(descElt);
  }
  if (!src.isEmpty()) {
    QDomElement srcElt = qdd.createElement("src");
    srcElt.appendChild(qdd.createTextNode(src));
    elt.appendChild(srcElt);
  }
  if (!url.isEmpty()) {
    QDomElement urlElt = qdd.createElement("url");
    urlElt.appendChild(qdd.createTextNode(url));
    elt.appendChild(urlElt);
  }
  if (!urlname.isEmpty()) {
    QDomElement urlnameElt = qdd.createElement("urlname");
    urlnameElt.appendChild(qdd.createTextNode(urlname));
    elt.appendChild(urlnameElt);
  }
}


GPSPoint::GPSPoint() {
  ele = -std::numeric_limits<double>::max();
}


bool GPSPoint::parseNode(const QDomNode& node) {
  GPSObject::parseNode(node);

  QDomNode node2;
  
  // lat and lon are required
  node2 = node.attributes().namedItem("lat");
  if (node2.isNull())
    return false;
  lat = node2.nodeValue().toDouble();
  node2 = node.attributes().namedItem("lon");
  if (node2.isNull())
    return false;
  lon = node2.nodeValue().toDouble();
  
  // ele is optional
  node2 = node.namedItem("ele");
  if (!node2.isNull())
    ele = std::atof((const char*)node2.firstChild().nodeValue());
  else
    ele = -std::numeric_limits<double>::max();
  
  // sym is optional
  node2 = node.namedItem("sym");
  if (!node2.isNull())
    sym = (const char*)node2.firstChild().nodeValue();
  
  return true;
}


void GPSPoint::fillElement(QDomElement& elt) {
  GPSObject::fillElement(elt);
  QDomDocument qdd = elt.ownerDocument();
  elt.setAttribute("lat", QString("%1").arg(lat, 0, 'f'));
  elt.setAttribute("lon", QString("%1").arg(lon, 0, 'f'));
  if (ele != -std::numeric_limits<double>::max()) {
    QDomElement eleElt = qdd.createElement("ele");
    eleElt.appendChild(qdd.createTextNode(QString("%1").arg(ele, 0, 'f')));
    elt.appendChild(eleElt);
  }
  if (!sym.isEmpty()) {
    QDomElement symElt = qdd.createElement("sym");
    symElt.appendChild(qdd.createTextNode(sym));
    elt.appendChild(symElt);
  }
}


bool GPSExtended::parseNode(const QDomNode& node) {
  GPSObject::parseNode(node);

  // number is optional
  QDomNode node2 = node.namedItem("number");
  if (!node2.isNull())
    number = std::atoi((const char*)node2.firstChild().nodeValue());
  else
    number = std::numeric_limits<int>::max();
  return true;
}


void GPSExtended::fillElement(QDomElement& elt) {
  GPSObject::fillElement(elt);
  QDomDocument qdd = elt.ownerDocument();
  if (number != std::numeric_limits<int>::max()) {
    QDomElement numberElt = qdd.createElement("number");
    numberElt.appendChild(qdd.createTextNode(QString("%1").arg(number)));
    elt.appendChild(numberElt);
  }
}


bool Route::parseNode(const QDomNode& node) {
  GPSExtended::parseNode(node);
  
  QDomNode node2;
  
  // reset extent
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
  
  // routepoints are optional, empty routes are allowed
  node2 = node.namedItem("rtept");
  while (!node2.isNull()) {
    if (node2.nodeName() == "rtept") {
      Routepoint rtept;
      if (!rtept.parseNode(node2))
	return false;
      points.push_back(rtept);
      
      // update the route bounds
      xMin = (xMin < rtept.lon ? xMin : rtept.lon);
      xMax = (xMax > rtept.lon ? xMax : rtept.lon);
      yMin = (yMin < rtept.lat ? yMin : rtept.lat);
      yMax = (yMax > rtept.lat ? yMax : rtept.lat);
    }
    node2 = node2.nextSibling();
  }
  
  return true;
}


void Route::fillElement(QDomElement& elt) {
  GPSExtended::fillElement(elt);
  
  QDomDocument qdd = elt.ownerDocument();
  
  for (int i = 0; i < points.size(); ++i) {
    QDomElement ptElt = qdd.createElement("rtept");
    points[i].fillElement(ptElt);
    elt.appendChild(ptElt);
  }
}


bool Track::parseNode(const QDomNode& node) {
  GPSExtended::parseNode(node);
  
  QDomNode node2, node3;

  // reset track bounds
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
  
  // track segments are optional - empty tracks are allowed
  node2 = node.namedItem("trkseg");
  while (!node2.isNull()) {
    if (node2.nodeName() == "trkseg") {
      TrackSegment trkseg;
      node3 = node2.namedItem("trkpt");
      while (!node3.isNull()) {
	if (node3.nodeName() == "trkpt") {
	  Trackpoint trkpt;
	  if (!trkpt.parseNode(node3))
	    return false;
	  trkseg.points.push_back(trkpt);
	  
	  // update the track bounds
	  xMin = (xMin < trkpt.lon ? xMin : trkpt.lon);
	  xMax = (xMax > trkpt.lon ? xMax : trkpt.lon);
	  yMin = (yMin < trkpt.lat ? yMin : trkpt.lat);
	  yMax = (yMax > trkpt.lat ? yMax : trkpt.lat);
	}
	node3 = node3.nextSibling();
      }
      
      segments.push_back(trkseg);
    }
    node2 = node2.nextSibling();
  }
  
  return true;
}


void Track::fillElement(QDomElement& elt) {
  GPSExtended::fillElement(elt);
  QDomDocument qdd = elt.ownerDocument();
  
  for (int i = 0; i < segments.size(); ++i) {
    QDomElement sgmElt = qdd.createElement("trkseg");
    for (int j = 0; j < segments[i].points.size(); ++j) {
      QDomElement ptElt = qdd.createElement("trkpt");
      segments[i].points[j].fillElement(ptElt);
      sgmElt.appendChild(ptElt);
    }
    elt.appendChild(sgmElt);
  }
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


int GPSData::addWaypoint(double lat, double lon, QString name, 
			 double ele) {
  Waypoint wpt;
  wpt.lat = lat;
  wpt.lon = lon;
  wpt.name = name;
  wpt.ele = ele;
  xMax = xMax > wpt.lon ? xMax : wpt.lon;
  xMin = xMin < wpt.lon ? xMin : wpt.lon;
  yMax = yMax > wpt.lat ? yMax : wpt.lat;
  yMin = yMin < wpt.lat ? yMin : wpt.lat;
  waypoints.push_back(wpt);
  return waypoints.size() - 1;
}


int GPSData::addWaypoint(const Waypoint& wpt) {
  waypoints.push_back(wpt);
  return waypoints.size() - 1;
}


int GPSData::addRoute(QString name) {
  Route rte;
  rte.name = name;
  routes.push_back(rte);
  return routes.size() - 1;
}


int GPSData::addRoute(const Route& rte) {
  routes.push_back(rte);
  return routes.size() - 1;
}


int GPSData::addTrack(QString name) {
  Track trk;
  trk.name = name;
  tracks.push_back(trk);
  return tracks.size() - 1;
}
  

int GPSData::addTrack(const Track& trk) {
  tracks.push_back(trk);
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
  while (node.nodeName() != "gpx")
    node = node.nextSibling();
  
  // there must be a gpx element
  if (node.isNull())
    return false;
  
  return parseGPX(node);
}


void GPSData::fillDom(QDomDocument& qdd) {
  QDomElement gpxElt = qdd.createElement("gpx");
  qdd.appendChild(gpxElt);
  gpxElt.setAttribute("version", "1.0");
  
  // add waypoints
  for (int i = 0; i < waypoints.size(); ++i) {
    QDomElement wptElt = qdd.createElement("wpt");
    waypoints[i].fillElement(wptElt);
    gpxElt.appendChild(wptElt);
  }

  // add routes
  for (int i = 0; i < routes.size(); ++i) {
    QDomElement rteElt = qdd.createElement("rte");
    routes[i].fillElement(rteElt);
    gpxElt.appendChild(rteElt);
  }

  // add tracks
  for (int i = 0; i < tracks.size(); ++i) {
    QDomElement trkElt = qdd.createElement("trk");
    tracks[i].fillElement(trkElt);
    gpxElt.appendChild(trkElt);
  }
}


bool GPSData::parseGPX(QDomNode& node) {
  // start parsing child nodes
  node = node.firstChild();
  QDomNode node2, node3, node4;
  while (!node.isNull()) {

    // waypoint
    if (node.nodeName() == "wpt") {
      Waypoint wpt;
      if (!wpt.parseNode(node))
	return false;
      waypoints.push_back(wpt);
      
      // update the extent
      xMin = xMin < wpt.lon ? xMin : wpt.lon;
      xMax = xMax > wpt.lon ? xMax : wpt.lon;
      yMin = yMin < wpt.lat ? yMin : wpt.lat;
      yMax = yMax > wpt.lat ? yMax : wpt.lat;
    }
    
    // route
    else if (node.nodeName() == "rte") {
      Route rte;
      if (!rte.parseNode(node))
	return false;
      routes.push_back(rte);

      // update the extent
      xMin = xMin < rte.xMin ? xMin : rte.xMin;
      xMax = xMax > rte.xMax ? xMax : rte.xMax;
      yMin = yMin < rte.yMin ? yMin : rte.yMin;
      yMax = yMax > rte.yMax ? yMax : rte.yMax;
    }
    
    // track
    else if (node.nodeName() == "trk") {
      Track trk;
      if (!trk.parseNode(node))
	return false;
      tracks.push_back(trk);
      
      // update the extent
      xMin = xMin < trk.xMin ? xMin : trk.xMin;
      xMax = xMax > trk.xMax ? xMax : trk.xMax;
      yMin = yMin < trk.yMin ? yMin : trk.yMin;
      yMax = yMax > trk.yMax ? yMax : trk.yMax;
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
    GPSData* data = new GPSData;
    std::cerr<<"Loading file "<<filename<<std::endl;
    if (!(qdd.setContent(&file) && data->parseDom(qdd))) {
      std::cerr<<filename<<"is not valid GPX!"<<std::endl;
      return 0;
    }
    dataObjects[filename] = std::pair<GPSData*, unsigned>(data, 0);
  }
  else
    std::cerr<<filename<<" is already loaded"<<std::endl;
  
  // return a pointer and increase the reference count for that filename
  DataMap::iterator iter = dataObjects.find(filename);
  ++(iter->second.second);
  return (GPSData*)(iter->second.first);
}


void GPSData::releaseData(const QString& filename) {
  
  /* decrease the reference count for the filename (if it is used), and erase
     it if the reference count becomes 0 */
  DataMap::iterator iter = dataObjects.find(filename);
  if (iter != dataObjects.end()) {
    std::cerr<<"unrefing "<<filename<<std::endl;
    if (--(iter->second.second) == 0) {
      std::cerr<<"No one's using "<<filename<<", I'll erase it"<<std::endl;
      delete iter->second.first;
      dataObjects.erase(iter);
    }
  }
}


// we have to initialize the static member
GPSData::DataMap GPSData::dataObjects;
