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
* Quantum GIS (QGIS) is a user friendly Open Source Geographic Information
* System (GIS) that runs on Linux, Unix, Mac OSX, and Windows. QGIS supports
* vector, raster, and database formats. QGIS is licensed under the GNU Public
* License.
*
* This API documentation provides information about all classes that make up QGIS.
*/
#include "qgsconfig.h"

#include <qevent.h>

class QGis
{ 
public:
  // Version constants
  //
  // Version string 
  static const char* qgisVersion;
  // Version number used for comparing versions using the "Check QGIS Version" function
  static const int qgisVersionInt;
  // Release name
  static const char* qgisReleaseName;

  // Enumerations
  //

  //! Used for symbology operations
  // Feature types
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
  static const char *qgisVectorGeometryType[];
  
 //! description strings for feature types
  static const char *qgisFeatureTypes[];

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
  
  static const int DEFAULT_IDENTIFY_RADIUS;
};
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
  const int PROJ_PREFIX_LEN = 6;
  /** The length of teh string "+ellps=" */
  const int ELLPS_PREFIX_LEN = 7;
  /** Magick number that determins whether a projection srsid is a system (srs.db)
   *  or user (~/.qgis.qgis.db) defined projection. */
  const int USER_PROJECTION_START_ID=100000;



/** debugging convenience function

  Wrapper round qDebug() that's only embedded if QGISDEBUG set, thus
  elminating large blocks of #ifdef/#endif text.  Also uses GNU g++
  __FUNCTION__ extension if that compiler used.

*/
#ifdef QGISDEBUG
#ifdef __GNUG__
#define QgsDebug(str) qDebug("%s:%d %s, %s", __FILE__, __LINE__, __FUNCTION__, str)
#else
#define QgsDebug(str) qDebug("%s:%d %s", __FILE__, __LINE__, str)
#endif
#else
#define QgsDebug(str)
#endif

#endif
