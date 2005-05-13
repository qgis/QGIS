/***************************************************************************
                          qgis.h - QGIS namespace
                             -------------------
    begin                : Sat Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGIS_H
#define QGIS_H
/*!  \mainpage Quantum GIS
*
*  \section about  About QGIS
* QGIS aims to be an easy to use desktop GIS tool. Initial focus is on viewing spatial
* and tabular data from common data stores, including rasters, Shapefiles and PostGIS.
*
* This API documentation provides information about all classes that make up QGIS.
*
*/
#include "qgsconfig.h"

#include <qevent.h>

namespace QGis
{ 
  // Version constants
  //
  // Version string 
  static const char *qgisVersion = VERSION;
  // Version number used for comparing versions using the "Check QGIS Version" function
  static const int qgisVersionInt =600;
  // Release name
  static const char *qgisReleaseName = "Seamus";

  // Enumerations
  //
  // Maptool enumeration
  enum MapTools
  {
    ZoomIn,
    ZoomOut,
    Pan,
    Distance,
    Identify,
    Table,
    Select,
    CapturePoint,
    CaptureLine,
    CapturePolygon,
    EmitPoint,
    Measure
  };
  //! Used for symbology operations
  // Featuure types
  enum WKBTYPE
  {
    WKBPoint = 1,
    WKBLineString,
    WKBPolygon,
    WKBMultiPoint,
    WKBMultiLineString,
    WKBMultiPolygon,
    WKBUnknown
  };
  enum VectorType
  {
    Point,
    Line,
    Polygon
  };
  static const char *qgisVectorGeometryType[] =
  {
    "Point",
    "Line",
    "Polygon"
  };
  //! description strings for feature types
  static const char *qgisFeatureTypes[] = 
  {
    "Null",
    "WKBPoint",
    "WKBLineString",
    "WKBPolygon",
    "WKBMultiPoint",
    "WKBMultiLineString",
    "WKBMultiPolygon" 
  };

  //! map units that qgis supports
  typedef enum 
  {
    METERS,
    FEET,
    DEGREES,
    UNKNOWN
  } units;

  //! User defined event types
  enum UserEvent
  {
    // These first two are useful for threads to alert their parent data providers

    //! The extents have been calculated by a provider of a layer
    ProviderExtentCalcEvent = (QEvent::User + 1),

    //! The row count has been calculated by a provider of a layer
    ProviderCountCalcEvent
  };

}
  /** WKT string that represents a geographic coord sys */
  const  QString GEOWKT =
      "GEOGCS[\"WGS 84\", "
      "  DATUM[\"WGS_1984\", "
      "    SPHEROID[\"WGS 84\",6378137,298.257223563, "
      "      AUTHORITY[\"EPSG\",7030]], "
      "    TOWGS84[0,0,0,0,0,0,0], "
      "    AUTHORITY[\"EPSG\",6326]], "
      "  PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]], "
      "  UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]], "
      "  AXIS[\"Lat\",NORTH], "
      "  AXIS[\"Long\",EAST], "
      "  AUTHORITY[\"EPSG\",4326]]";
  /** PROJ4 string that represents a geographic coord sys */
  const QString GEOPROJ4 = "+proj=longlat +ellps=WGS84 +no_defs";
  /** Magic number for a geographic coord sys in POSTGIS SRID */
  const long GEOSRID = 4326;
  /** Magic number for a geographic coord sys in QGIS srs.db tbl_srs.srs_id */
  const long GEOSRS_ID = 2585;
  /**  Magic number for a geographic coord sys in EPSG ID format */
  const long EPSGID = 4326;
  /** The length of teh string "+proj=" */
  const int PROJ_PREFIX_LEN = 5;
  /** The length of teh string "+ellps=" */
  const int ELLPS_PREFIX_LEN = 6;
  /** Magick number that determins whether a projection srsid is a system (srs.db)
   *  or user (~/.qgis.qgis.db) defined projection. */
  const int USER_PROJECTION_START_ID=100000;



  //! Structure for storing a spatial_ref_sys item
  /* THIS IS DEFUNCT NOW!
  typedef struct{
    QString srid; // spatial reference id (ala PostGIS)
    QString auth_name; // name of the author for this SRS
    QString auth_srid; // srid used by the author
    QString srtext; // WKT of the coordinate system
    QString proj4text; // Proj4 parameter string 
  } SPATIAL_REF_SYS; 
  */
#endif
