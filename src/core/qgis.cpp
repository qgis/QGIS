/***************************************************************************
                                qgis.cpp 
                       
                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
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
const int QGis::qgisVersionInt =910;
  
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

