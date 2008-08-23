/***************************************************************************
      gpsdata.h  -  Data structures for GPS data
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

#ifndef GPSDATA_H
#define GPSDATA_H

#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include <expat.h>
#include <QString>
#include <QTextStream>

#include "qgsrect.h"

// workaround for MSVC compiler which already has defined macro max
// that interferes with calling std::numeric_limits<int>::max
#ifdef _MSC_VER
# ifdef max
#  undef max
# endif
#endif

typedef QSet<int> QgsFeatureIds;

/** This is the parent class for all GPS data classes (except tracksegment).
    It contains the variables that all GPS objects can have.
*/
class GPSObject {
 public:
  virtual ~GPSObject() {};
  QString xmlify(const QString& str);
  virtual void writeXML(QTextStream& stream);
  QString name, cmt, desc, src, url, urlname;
};


/** This is the parent class for all GPS point classes. It contains common data
    members and common initialization code for all point classes.
*/
class GPSPoint : public GPSObject {
 public:
  GPSPoint();
  virtual void writeXML(QTextStream& stream);
  double lat, lon, ele;
  QString sym;
};


/** This is the parent class for all GPS object types that can have a nonempty
    bounding box (Route, Track). It contains common data members for all 
    those classes. */
class GPSExtended : public GPSObject {
 public:
  GPSExtended();
  virtual void writeXML(QTextStream& stream);
  double xMin, xMax, yMin, yMax;
  int number;
};


// they both have the same data members in GPX, so...
typedef GPSPoint Routepoint;
typedef GPSPoint Trackpoint;


/** This is the waypoint class. It is a GPSPoint with an ID. */
class Waypoint : public GPSPoint {
 public:
  virtual void writeXML(QTextStream& stream);
  int id;
};


/** This class represents a GPS route.
 */
class Route : public GPSExtended {
 public:
  virtual void writeXML(QTextStream& stream);
  std::vector<Routepoint> points;
  int id;
};


/** This class represents a GPS track segment, which is a contiguous part of
    a track. See the GPX specification for a better explanation.
*/
class TrackSegment {
 public:
  std::vector<Trackpoint> points;
};


/** This class represents a GPS tracklog. It consists of 0 or more track
    segments.
*/
class Track : public GPSExtended {
 public:
  virtual void writeXML(QTextStream& stream);
  std::vector<TrackSegment> segments;
  int id;
};


/** This class represents a set of GPS data, for example a GPS layer in QGIS.
 */
class GPSData {
 public:
  
  /** This iterator type is used to iterate over waypoints. */
  typedef std::list<Waypoint>::iterator WaypointIterator;
  /** This iterator type is used to iterate over routes. */
  typedef std::list<Route>::iterator RouteIterator;
  /** This iterator type is used to iterate over tracks. */
  typedef std::list<Track>::iterator TrackIterator;
  
  
  /** This constructor initializes the extent to a nonsense value. Don't try
      to use a GPSData object in QGIS without parsing a datafile into it. */
  GPSData();
  
  /** This function returns a pointer to a dynamically allocated QgsRect
      which is the bounding box for this dataset. You'll have to deallocate it
      yourself. */
  QgsRect getExtent() const;

  /** Sets a default sensible extent. Only applies when there are no actual data. */
  void setNoDataExtent();

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfWaypoints() const;

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfRoutes() const;

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfTracks() const;
  
  /** This function returns an iterator that points to the first waypoint. */
  WaypointIterator waypointsBegin();
  
  /** This function returns an iterator that points to the first route. */
  RouteIterator routesBegin();
  
  /** This function returns an iterator that points to the first track. */
  TrackIterator tracksBegin();
  
  /** This function returns an iterator that points to the end of the 
      waypoint list. */
  WaypointIterator waypointsEnd();
  
  /** This function returns an iterator that points to the end of the
      route list. */
  RouteIterator routesEnd();
  
  /** This function returns an iterator that points to the end of the 
      track list. */
  TrackIterator tracksEnd();
  
  /** This function tries to add a new waypoint. An iterator to the new
      waypoint will be returned (it will be waypointsEnd() if the waypoint
      couldn't be added. */
  WaypointIterator addWaypoint(double lat, double lon, QString name = "", 
			       double ele = -std::numeric_limits<double>::max());

  WaypointIterator addWaypoint(const Waypoint& wpt);

  /** This function tries to add a new route. It returns an iterator to the
      new route. */
  RouteIterator addRoute(QString name = "");

  RouteIterator addRoute(const Route& rte);

  /** This function tries to add a new track. An iterator to the new track
      will be returned. */
  TrackIterator addTrack(QString name = "");
  
  TrackIterator addTrack(const Track& trk);
  
  /** This function removes the waypoints whose IDs are in the list. */
  void removeWaypoints(const QgsFeatureIds & ids);
  
  /** This function removes the routes whose IDs are in the list. */
  void removeRoutes(const QgsFeatureIds & ids);
  
  /** This function removes the tracks whose IDs are in the list. */
  void removeTracks(const QgsFeatureIds & ids);
  
  /** This function will write the contents of this GPSData object as XML to
      the given text stream. */
  void writeXML(QTextStream& stream);
  
  /** This function returns a pointer to the GPSData object associated with
      the file @c fileName. If the file does not exist or can't be parsed,
      NULL will be returned. If the file is already used by another layer,
      a pointer to the same GPSData object will be returned. And if the file
      is not used by any other layer, and can be parsed, a new GPSData object
      will be created and a pointer to it will be returned. If you use this
      function you should also call releaseData() with the same @c fileName
      when you're done with the GPSData pointer, otherwise the data will stay
      in memory forever and you will get an ugly memory leak. */
  static GPSData* getData(const QString& fileName);
  
  /** Call this function when you're done with a GPSData pointer that you
      got earlier using getData(). Do NOT call this function if you haven't
      called getData() earlier with the same @c fileName, that can cause data
      that is still in use to be deleted. */
  static void releaseData(const QString& fileName);
  
  
  /** operator<< is our friend. For debugging, not for file I/O. */
  //friend std::ostream& operator<<(std::ostream& os, const GPSData& d);
  
 protected:
  
  std::list<Waypoint> waypoints;
  std::list<Route> routes;
  std::list<Track> tracks;
  int nextWaypoint, nextRoute, nextTrack;

  double xMin, xMax, yMin, yMax;
  
  /** This is used internally to store GPS data objects (one per file). */
  typedef std::map<QString, std::pair<GPSData*, unsigned> > DataMap;
  
  /** This is the static container that maps fileNames to GPSData objects and
      does reference counting, so several providers can use the same GPSData 
      object. */
  static DataMap dataObjects;
  
};



class GPXHandler {
public:
  
  GPXHandler(GPSData& data) : mData(data) { }
  
  /** This function is called when expat encounters a new start element in 
      the XML stream. */
  bool startElement(const XML_Char* qName, const XML_Char** attr);
  
  /** This function is called when expat encounters character data in the
      XML stream. */
  void characters(const XML_Char* chars, int len);
  
  /** This function is called when expat encounters a new end element in
      the XML stream. */
  bool endElement(const std::string& qName);
  
  // static wrapper functions for the XML handler functions (expat is in C,
  // it does not know about member functions)
  static void start(void* data, const XML_Char* el, const XML_Char** attr) {
    static_cast<GPXHandler*>(data)->startElement(el, attr);
  }
  static void end(void* data, const XML_Char* el) {
    static_cast<GPXHandler*>(data)->endElement(el);
  }
  static void chars(void* data, const XML_Char* chars, int len) {
    static_cast<GPXHandler*>(data)->characters(chars, len);
  }
  
private:
  
  enum ParseMode {
    ParsingDocument,
    ParsingWaypoint,
    ParsingRoute,
    ParsingTrack,
    ParsingRoutepoint,
    ParsingTrackSegment,
    ParsingTrackpoint,
    ParsingDouble,
    ParsingInt,
    ParsingString,
    ParsingUnknown
  };
  
  /** This is used to keep track of what kind of data we are parsing. */
  std::stack<ParseMode> parseModes;
  
  GPSData& mData;
  Waypoint mWpt;
  Route mRte;
  Track mTrk;
  Routepoint mRtept;
  TrackSegment mTrkseg;
  Trackpoint mTrkpt;
  GPSObject* mObj;
  QString* mString;
  double* mDouble;
  int* mInt;
  QString mCharBuffer;
};



#endif
