
#include "qgis.h"
#ifndef QGSSVNVERSION
#include "qgssvnversion.h"
#endif

#include "qgsconfig.h"

// Version constants
//

// Version string
const char* QGis::qgisVersion = VERSION;

// SVN version
const char* QGis::qgisSvnVersion = QGSSVNVERSION;
  
// Version number used for comparing versions using the "Check QGIS Version" function
const int QGis::qgisVersionInt =900;
  
// Release name
const char* QGis::qgisReleaseName = "Ganymede";

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

const double QGis::DEFAULT_IDENTIFY_RADIUS=0.5;

