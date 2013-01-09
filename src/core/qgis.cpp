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
#include <QDate>
#include <QTime>
#include <QDateTime>
#include "qgsconfig.h"
#include "qgslogger.h"

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
const CORE_EXPORT QString GEOPROJ4 = "+proj=longlat +datum=WGS84 +no_defs";
#else
const CORE_EXPORT QString GEOPROJ4 = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
#endif

const CORE_EXPORT QString GEOWKT =
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

const CORE_EXPORT QString PROJECT_SCALES =
  "1:1000000,1:500000,1:250000,1:100000,1:50000,1:25000,"
  "1:10000,1:5000,1:2500,1:1000,1:500";

const CORE_EXPORT QString GEO_EPSG_CRS_AUTHID = "EPSG:4326";

const CORE_EXPORT QString GEO_NONE = "NONE";

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
  for ( unsigned int i = 0; i < ( sizeof( qgisUnitTypes ) / sizeof( qgisUnitTypes[0] ) ); i++ )
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

void *QgsMalloc( size_t size )
{
  if ( size == 0 || long( size ) < 0 )
  {
    QgsDebugMsg( QString( "Negative or zero size %1." ).arg( size ) );
    return NULL;
  }
  void *p = malloc( size );
  if ( p == NULL )
  {
    QgsDebugMsg( QString( "Allocation of %1 bytes failed." ).arg( size ) );
  }
  return p;
}

void *QgsCalloc( size_t nmemb, size_t size )
{
  if ( nmemb == 0 || long( nmemb ) < 0 || size == 0 || long( size ) < 0 )
  {
    QgsDebugMsg( QString( "Negative or zero nmemb %1 or size %2." ).arg( nmemb ).arg( size ) );
    return NULL;
  }
  void *p = QgsMalloc( nmemb * size );
  if ( p != NULL )
  {
    memset( p, 0, nmemb * size );
  }
  return p;
}

void QgsFree( void *ptr )
{
  free( ptr );
}

bool qgsVariantLessThan( const QVariant& lhs, const QVariant& rhs )
{
  switch ( lhs.type() )
  {
    case QVariant::Int:
      return lhs.toInt() < rhs.toInt();
    case QVariant::UInt:
      return lhs.toUInt() < rhs.toUInt();
    case QVariant::LongLong:
      return lhs.toLongLong() < rhs.toLongLong();
    case QVariant::ULongLong:
      return lhs.toULongLong() < rhs.toULongLong();
    case QVariant::Double:
      return lhs.toDouble() < rhs.toDouble();
    case QVariant::Char:
      return lhs.toChar() < rhs.toChar();
    case QVariant::Date:
      return lhs.toDate() < rhs.toDate();
    case QVariant::Time:
      return lhs.toTime() < rhs.toTime();
    case QVariant::DateTime:
      return lhs.toDateTime() < rhs.toDateTime();
    default:
      return QString::localeAwareCompare( lhs.toString(), rhs.toString() ) < 0;
  }
}

bool qgsVariantGreaterThan( const QVariant& lhs, const QVariant& rhs )
{
  return ! qgsVariantLessThan( lhs, rhs );
}

