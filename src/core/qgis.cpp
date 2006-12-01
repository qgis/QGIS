
#include "qgis.h"
#ifndef QGSSVNVERSION
#include "qgssvnversion.h"
#endif
// Version constants
//

// Version string
const char* QGis::qgisVersion = VERSION;

// SVN version
const char* QGis::qgisSvnVersion = QGSSVNVERSION;
  
// Version number used for comparing versions using the "Check QGIS Version" function
const int QGis::qgisVersionInt =800;
  
// Release name
const char* QGis::qgisReleaseName = "Titan - Preview 2";

const char* QGis::qgisVectorGeometryType[] =
{
  "Point",
  "Line",
  "Polygon"
};

// description strings for feature types
const char* QGis::qgisFeatureTypes[] =
{
  "Null",
  "WKBPoint",
  "WKBLineString",
  "WKBPolygon",
  "WKBMultiPoint",
  "WKBMultiLineString",
  "WKBMultiPolygon"
};

const int QGis::DEFAULT_IDENTIFY_RADIUS=0.5;

