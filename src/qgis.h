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
/* $Id$ */
#include "qgsconfig.h"

namespace QGis
{ 
  // Version constants
  //
  // Version string 
  static const char *qgisVersion = VERSION;
  // Version number used for comparing versions using the "Check QGIS Version" function
  static const int qgisVersionInt =600;
  // Release name
  static const char *qgisReleaseName = "Simon";
  
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
    CapturePolygon
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
    WKBMultiPolygon
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
}
#endif
