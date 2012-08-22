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
#ifndef QGSVERSION
#include "qgsversion.h"
#endif
#include <QCoreApplication>
#include "qgsconfig.h"

#include <ogr_api.h>

// Version constants
//

// Version string
const char* QGis::QGIS_VERSION = VERSION;

// development version
const char* QGis::QGIS_DEV_VERSION = QGSVERSION;

// Version number used for comparing versions using the
// "Check QGIS Version" function
const int QGis::QGIS_VERSION_INT = VERSION_INT;

// Release name
const char* QGis::QGIS_RELEASE_NAME = RELEASE_NAME;

#if GDAL_VERSION_NUM >= 1800
const QString GEOPROJ4 = "+proj=longlat +datum=WGS84 +no_defs";
#else
const QString GEOPROJ4 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
#endif

const char* QGis::qgisVectorGeometryType[] =
{
  "Point",
  "Line",
  "Polygon",
  "Unknown geometry",
  "No geometry",
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


const double QGis::DEFAULT_IDENTIFY_RADIUS = 0.5;

// description strings for units
// Order must match enum indices
const char* QGis::qgisUnitTypes[] =
{
  QT_TRANSLATE_NOOP( "QGis::UnitType", "meters" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "feet" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "<unknown>" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" )
};

QGis::UnitType QGis::fromLiteral( QString literal, QGis::UnitType defaultType )
{
  for ( unsigned int i = 0; i < sizeof( qgisUnitTypes ); i++ )
  {
    if ( literal == qgisUnitTypes[ i ] )
    {
      return static_cast<UnitType>( i );
    }
  }
  return defaultType;
}

QString QGis::toLiteral( QGis::UnitType unit )
{
  return QString( qgisUnitTypes[ static_cast<int>( unit )] );
}

QString QGis::tr( QGis::UnitType unit )
{
  return QCoreApplication::translate( "QGis::UnitType", qPrintable( toLiteral( unit ) ) );
}
