/***************************************************************************
 *   Copyright (C) 2004 by Lars Luthman                                    *
 *   larsl@users.sourceforge.net                                           *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <exif-data.h>
#include <qdir.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>

#include "exif2gpx.h"


bool Exif2GPX::loadGPX(const QString& filename, 
		       bool useTracks, bool useWaypoints) {
  QDomDocument qdd;
  QFile file(filename);
  qdd.setContent(&file);
  QDomNode node, node2, node3, node4, node5;
  node = qdd.namedItem("gpx");
  if (node.isNull()) {
    std::cerr<<"Could not load "<<filename<<std::endl;
    return false;
  }
  
  node2 = node.firstChild();
  while (!node2.isNull()) {
    if (node2.nodeName() == "wpt")
      loadPoint(node2);
    else if (node2.nodeName() == "trk") {
      node3 = node2.firstChild();
      while (!node3.isNull()) {
	if (node3.nodeName() == "trkseg") {
	  node4 = node3.firstChild();
	  while (!node4.isNull()) {
	    if (node4.nodeName() == "trkpt")
	      loadPoint(node4);
	    node4 = node4.nextSibling();
	  }
	}
	node3 = node3.nextSibling();
      }
    }
    node2 = node2.nextSibling();
  }
  
  // did we get any points?
  if (points.size() == 0) {
    std::cerr<<"No timestamped points found in "<<filename<<std::endl;
    return false;
  }
  
  return true;
}


bool Exif2GPX::writeGPX(const QStringList& pictures, const QString& gpxOutput,
			bool interpolate, time_t offset,
			const QString& prefix) {
  // initialize GPX DOM
  QDomDocument qdd;
  QDomElement gpxElt = qdd.createElement("gpx");
  qdd.appendChild(gpxElt);
  gpxElt.setAttribute("version", "1.0");
  
  // add the waypoints
  QStringList::const_iterator iter;
  for (iter = pictures.begin(); iter != pictures.end(); ++iter) {
    time_t timestamp = getTimeFromEXIF(*iter);
    QFile file(*iter);
    QFileInfo fi(file);
    addWaypoint(gpxElt, computePosition(timestamp - offset, interpolate), 
		timestamp - offset, prefix, fi.fileName());
  }
  
  // write the file
  QFile gpxOut(gpxOutput);
  if (!gpxOut.open(IO_WriteOnly)) {
    std::cerr<<"Could not open "<<gpxOutput<<std::endl;
    return false;
  }
  QTextStream str(&gpxOut);
  str<<qdd.toString();
  
  return true;
}


time_t Exif2GPX::getTimeFromEXIF(const QString& filename) {
  
  // load the exif data
  ExifData* exifData;
  if (!(exifData = exif_data_new_from_file((const char*)filename))) {
    std::cerr<<"Could not load EXIF data from "<<filename<<std::endl;
    return time_t(0);
  }
  
  // find the timestamp
  ExifEntry* entry;
  entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_0], 
				 EXIF_TAG_DATE_TIME);
  if (!entry)
    entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_EXIF],
				   EXIF_TAG_DATE_TIME_ORIGINAL);
  if (!entry)
    entry = exif_content_get_entry(exifData->ifd[EXIF_IFD_EXIF],
				   EXIF_TAG_DATE_TIME_DIGITIZED);
  if (!entry) {
    std::cerr<<"Could not find a valid timestamp in "<<filename<<std::endl;
    return time_t(0);
  }
  
  // parse it
  tm timeStruct;
  if (std::sscanf((char*)entry->data, "%d:%d:%d %d:%d:%d", &timeStruct.tm_year,
		  &timeStruct.tm_mon, &timeStruct.tm_mday, &timeStruct.tm_hour,
		  &timeStruct.tm_min, &timeStruct.tm_sec) != 6) {
    std::cerr<<"Could not find a valid timestamp in "<<filename<<std::endl;
    return time_t(0);
  }
  timeStruct.tm_year -= 1900;
  timeStruct.tm_mon -= 1;
  timeStruct.tm_isdst = -1;
  time_t timestamp = std::mktime(&timeStruct);
  
  std::cerr<<filename<<": "<<std::ctime(&timestamp)<<endl;

  return timestamp;
}


std::pair<double, double> Exif2GPX::computePosition(time_t time, 
						    bool interpolate) {
  std::map<time_t, std::pair<double, double> >::const_iterator iter, iter2;
  iter = points.lower_bound(time);
  
  // all elements are smaller than time - return the last one
  if (iter == points.end())
    return points.rbegin()->second;

  // all elements are larger than time - return the first one
  if (iter == points.begin())
    return iter->second;
  
  // find the closest element
  iter2 = iter;
  --iter2;
  if ((iter->first - time) < (time - iter2->first))
    return iter->second;
  return iter2->second;
}


void Exif2GPX::addWaypoint(QDomElement& elt, std::pair<double,double> position,
			   time_t time, const QString& prefix, 
			   const QString& name) {
  QDomDocument qdd = elt.ownerDocument();
  QDomElement wptElt = qdd.createElement("wpt");
  wptElt.setAttribute("lat", QString("%1").arg(position.first, 0, 'f'));
  wptElt.setAttribute("lon", QString("%1").arg(position.second, 0, 'f'));
  QDomElement timeElt = qdd.createElement("time");
  char buffer[22];
  std::memset(buffer, 0, 22);
  std::strftime(buffer, 22, "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&time));
  timeElt.appendChild(qdd.createTextNode(buffer));
  wptElt.appendChild(timeElt);
  QDomElement urlElt = qdd.createElement("url");
  urlElt.appendChild(qdd.createTextNode(prefix + name));
  wptElt.appendChild(urlElt);
  QDomElement nameElt = qdd.createElement("name");
  nameElt.appendChild(qdd.createTextNode(name));
  wptElt.appendChild(nameElt);
  elt.appendChild(wptElt);
}


void Exif2GPX::loadPoint(const QDomNode& node) {
  QDomNode node2 = node.namedItem("time");
  if (!node2.isNull()) {
    std::tm timestruct;
    std::time_t t;
    double lat, lon;
    if (strptime((const char*)node2.firstChild().nodeValue(),
		 "%Y-%m-%dT%H:%M:%S", &timestruct)) {
      t = mktime(&timestruct);
      node2 = node.attributes().namedItem("lat");
      if (node2.isNull())
	return;
      lat = node2.nodeValue().toDouble();
      node2 = node.attributes().namedItem("lon");
      if (node2.isNull())
	return;
      lon = node2.nodeValue().toDouble();
      points[t] = std::pair<double, double>(lat, lon);
      std::cerr<<node.nodeName()<<std::endl;
    }
  }
}
