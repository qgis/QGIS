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

#include <cstdlib>
#include <cstring>
#include <iostream>

#ifdef WIN32
#include <libexif\exif-data.h>
#else
#include <exif-data.h>
#endif

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
    if (node2.nodeName() == "wpt" && useWaypoints)
      loadPoint(node2);
    else if (node2.nodeName() == "trk" && useTracks) {
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
			bool interpolate, unsigned offset,
			const QString& prefix) {
  // initialize GPX DOM
  QDomDocument qdd;
  QDomElement gpxElt = qdd.createElement("gpx");
  qdd.appendChild(gpxElt);
  gpxElt.setAttribute("version", "1.0");
  
  // add the waypoints
  QStringList::const_iterator iter;
  for (iter = pictures.begin(); iter != pictures.end(); ++iter) {
    QDateTime timestamp = getTimeFromEXIF(*iter);
    QFile file(*iter);
    QFileInfo fi(file);
    addWaypoint(gpxElt, computePosition(timestamp.addSecs(-offset),
					interpolate), 
		timestamp.addSecs(-offset), prefix, fi.fileName());
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


QDateTime Exif2GPX::getTimeFromEXIF(const QString& filename) {
  
  // load the exif data
  ExifData* exifData;
  if (!(exifData = exif_data_new_from_file((const char*)filename))) {
    std::cerr<<"Could not load EXIF data from "<<filename<<std::endl;
    return QDateTime();
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
    return QDateTime();
  }
  
  // parse it
  int year, month, day, hour, minute, second;
  if (std::sscanf((char*)entry->data, "%d:%d:%d %d:%d:%d", &year, &month, &day,
		  &hour, &minute, &second) != 6) {
    std::cerr<<"Could not find a valid timestamp in "<<filename<<std::endl;
    return QDateTime();
  }
  QDateTime timestamp(QDate(year, month, day), QTime(hour, minute, second));
  
  std::cerr<<filename<<": "<<timestamp.toString()<<std::endl;

  return timestamp;
}


std::pair<double, double> Exif2GPX::computePosition(QDateTime time, 
						    bool interpolate) {
  std::map<QDateTime, std::pair<double, double> >::const_iterator iter, iter2;
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
  if ((time.secsTo(iter->first)) < (iter2->first.secsTo(time)))
    return iter->second;
  return iter2->second;
}


void Exif2GPX::addWaypoint(QDomElement& elt, std::pair<double,double> position,
			   QDateTime time, const QString& prefix, 
			   const QString& name) {
  QDomDocument qdd = elt.ownerDocument();
  QDomElement wptElt = qdd.createElement("wpt");
  wptElt.setAttribute("lat", QString("%1").arg(position.first, 0, 'f'));
  wptElt.setAttribute("lon", QString("%1").arg(position.second, 0, 'f'));
  QDomElement timeElt = qdd.createElement("time");
  char buffer[22];
  std::memset(buffer, 0, 22);
  QDate d = time.date();
  QTime t = time.time();
  std::sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
	       d.year(), d.month(), d.day(), t.hour(), t.minute(), t.second());
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
    double lat, lon;
    QDateTime t = QDateTime::fromString(node2.firstChild().nodeValue(),
					Qt::ISODate);
    if (t.isValid()) {
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
