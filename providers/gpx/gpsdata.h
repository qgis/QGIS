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
#include <string>
#include <map>
#include <vector>

#include <qdom.h>
#include <qstring.h>

#include "../../src/qgsrect.h"


/** This is the parent class for all GPS data classes (except tracksegment).
    It contains the variables that all GPS objects can have.
*/
class GPSObject {
 public:
  virtual bool parseNode(const QDomNode& node);
  virtual void fillElement(QDomElement& elt);
  QString name, cmt, desc, src, url, urlname;
};


/** This is the parent class for all GPS point classes. It contains common data
    members and common initialization code for all point classes.
*/
class GPSPoint : public GPSObject {
 public:
  GPSPoint();
  virtual bool parseNode(const QDomNode& node);
  virtual void fillElement(QDomElement& elt);
  double lat, lon, ele;
  QString sym;
};


/** This is the parent class for all GPS object types that can have a nonempty
    bounding box (Route, Track). It contains common data members for all 
    those classes. */
class GPSExtended : public GPSObject {
 public:
  virtual bool parseNode(const QDomNode& node);
  virtual void fillElement(QDomElement& elt);
  int number;
  double xMin, xMax, yMin, yMax;
};


// they both have the same data members in GPX, so...
typedef GPSPoint Routepoint;
typedef GPSPoint Trackpoint;


/** This is the waypoint class. It is a GPSPoint with an ID. */
class Waypoint : public GPSPoint {
 public:
  int id;
};


/** This class represents a GPS route.
 */
class Route : public GPSExtended {
 public:
  bool parseNode(const QDomNode& node);
  void fillElement(QDomElement& elt);
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
  bool parseNode(const QDomNode& node);
  void fillElement(QDomElement& elt);
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
  QgsRect* getExtent() const;
  

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfWaypoints() const;

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfRoutes() const;

  /** Returns the number of waypoints in this dataset. */
  int getNumberOfTracks() const;
  
  WaypointIterator waypointsBegin();
  RouteIterator routesBegin();
  TrackIterator tracksBegin();
  WaypointIterator waypointsEnd();
  RouteIterator routesEnd();
  TrackIterator tracksEnd();
  
  /** This function returns the waypoint with ID @c id. If there is no
      waypoint with that ID an exception will be thrown.
  */
  //Waypoint& getWaypoint(int id);

  /** This function returns the route with index @c index. If there is no
      route with that index an exception will be thrown.
  */
  //Route& getRoute(int index);

  /** This function returns the track with index @c index. If there is no
      track with that index an exception will be thrown.
  */
  //Track& getTrack(int index);

  
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
  void removeWaypoints(std::list<int> const & ids);
  
  /** This function removes the routes whose IDs are in the list. */
  void removeRoutes(std::list<int> const & ids);
  
  /** This function removes the tracks whose IDs are in the list. */
  void removeTracks(std::list<int> const & ids);
  
  /** This function tries to parse a QDomDocument as a GPX tree. If it
      succeeds it will fill this GPSData object with the data from the
      QDomDocument and return true, otherwise it will return false. */
  bool parseDom(QDomDocument& qdd);
  
  /** This function will fill the given QDomDocument with child nodes that
      represent the data in this GPSData object as a GPX tree. */
  void fillDom(QDomDocument& qdd);
  
  /** This function returns a pointer to the GPSData object associated with
      the file @c filename. If the file does not exist or can't be parsed,
      NULL will be returned. If the file is already used by another layer,
      a pointer to the same GPSData object will be returned. And if the file
      is not used by any other layer, and can be parsed, a new GPSData object
      will be created and a pointer to it will be returned. If you use this
      function you should also call releaseData() with the same @c filename
      when you're done with the GPSData pointer, otherwise the data will stay
      in memory forever and you will get an ugly memory leak. */
  static GPSData* getData(const QString& filename);
  
  /** Call this function when you're done with a GPSData pointer that you
      got earlier using getData(). Do NOT call this function if you haven't
      called getData() earlier with the same @c filename, that can cause data
      that is still in use to be deleted. */
  static void releaseData(const QString& filename);
  
  
  /** operator<< is our friend. For debugging, not for file I/O. */
  //friend std::ostream& operator<<(std::ostream& os, const GPSData& d);
  
 protected:
  
  /** This function parses a GPX file and places the data it finds in this 
      object. If any errors are found in the XML tree the function will return
      false, but it doesn't do a full validity check so some errors could get
      through. If it succeeds it will return true.
      @param node The <gpx> node from a QDomDocument
  */
  bool parseGPX(QDomNode& node);
  
  std::list<Waypoint> waypoints;
  std::list<Route> routes;
  std::list<Track> tracks;
  int nextWaypoint, nextRoute, nextTrack;

  double xMin, xMax, yMin, yMax;
  
  /** This is used internally to store GPS data objects (one per file). */
  typedef std::map<QString, std::pair<GPSData*, unsigned> > DataMap;
  
  /** This is the static container that maps filenames to GPSData objects and
      does reference counting, so several providers can use the same GPSData 
      object. */
  static DataMap dataObjects;
  
};

#endif
