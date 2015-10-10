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
#include <QColor>
#include <QDate>
#include <QTime>
#include <QLocale>
#include <QDateTime>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "geometry/qgswkbtypes.h"

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

const QString GEOWKT =
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

const QString PROJECT_SCALES =
  "1:1000000,1:500000,1:250000,1:100000,1:50000,1:25000,"
  "1:10000,1:5000,1:2500,1:1000,1:500";

const QString GEO_EPSG_CRS_AUTHID = "EPSG:4326";

const QString GEO_NONE = "NONE";

const double QGis::DEFAULT_IDENTIFY_RADIUS = 0.5;
const double QGis::DEFAULT_SEARCH_RADIUS_MM = 2.;

//! Default threshold between map coordinates and device coordinates for map2pixel simplification
const float QGis::DEFAULT_MAPTOPIXEL_THRESHOLD = 1.0f;

const QColor QGis::DEFAULT_HIGHLIGHT_COLOR = QColor( 255, 0, 0, 128 );

double QGis::DEFAULT_HIGHLIGHT_BUFFER_MM = 0.5;

double QGis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM = 1.0;

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
  QT_TRANSLATE_NOOP( "QGis::UnitType", "degrees" ),
  QT_TRANSLATE_NOOP( "QGis::UnitType", "nautical miles" )
};

QgsWKBTypes::Type QGis::fromOldWkbType( QGis::WkbType type )
{
  switch ( type )
  {
    case QGis::WKBPoint: return QgsWKBTypes::Point;
    case QGis::WKBLineString: return QgsWKBTypes::LineString;
    case QGis::WKBPolygon: return QgsWKBTypes::Polygon;
    case QGis::WKBMultiPoint: return QgsWKBTypes::MultiPoint;
    case QGis::WKBMultiLineString: return QgsWKBTypes::MultiLineString;
    case QGis::WKBMultiPolygon: return QgsWKBTypes::MultiPolygon;
    case QGis::WKBNoGeometry: return QgsWKBTypes::NoGeometry;
    case QGis::WKBPoint25D: return QgsWKBTypes::PointZ;
    case QGis::WKBLineString25D: return QgsWKBTypes::LineStringZ;
    case QGis::WKBPolygon25D: return QgsWKBTypes::PolygonZ;
    case QGis::WKBMultiPoint25D: return QgsWKBTypes::MultiPointZ;
    case QGis::WKBMultiLineString25D: return QgsWKBTypes::MultiLineStringZ;
    case QGis::WKBMultiPolygon25D: return QgsWKBTypes::MultiPolygonZ;
    case QGis::WKBUnknown: return QgsWKBTypes::Unknown;
    default: break;
  }

  QgsDebugMsg( QString( "unexpected old wkbType=%1" ).arg( type ) );
  return ( QgsWKBTypes::Type ) type;
}

QGis::WkbType QGis::fromNewWkbType( QgsWKBTypes::Type type )
{
  switch ( type )
  {
    case QgsWKBTypes::Point: return QGis::WKBPoint;
    case QgsWKBTypes::LineString: return QGis::WKBLineString;
    case QgsWKBTypes::Polygon: return QGis::WKBPolygon;
    case QgsWKBTypes::MultiPoint: return QGis::WKBMultiPoint;
    case QgsWKBTypes::MultiLineString: return QGis::WKBMultiLineString;
    case QgsWKBTypes::MultiPolygon: return QGis::WKBMultiPolygon;
    case QgsWKBTypes::NoGeometry: return QGis::WKBNoGeometry;
    case QgsWKBTypes::PointZ: return QGis::WKBPoint25D;
    case QgsWKBTypes::LineStringZ: return QGis::WKBLineString25D;
    case QgsWKBTypes::PolygonZ: return QGis::WKBPolygon25D;
    case QgsWKBTypes::MultiPointZ: return QGis::WKBMultiPoint25D;
    case QgsWKBTypes::MultiLineStringZ: return QGis::WKBMultiLineString25D;
    case QgsWKBTypes::MultiPolygonZ: return QGis::WKBMultiPolygon25D;
    default: break;
  }

  QgsDebugMsg( QString( "unexpected new wkbType=%1" ).arg( type ) );
  return ( QGis::WkbType ) type;
}


QGis::UnitType QGis::fromLiteral( const QString& literal, QGis::UnitType defaultType )
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

QGis::UnitType QGis::fromTr( const QString& literal, QGis::UnitType defaultType )
{
  for ( unsigned int i = 0; i < ( sizeof( qgisUnitTypes ) / sizeof( qgisUnitTypes[0] ) ); i++ )
  {
    if ( literal == QGis::tr( static_cast<UnitType>( i ) ) )
    {
      return static_cast<UnitType>( i );
    }
  }
  return defaultType;
}

double QGis::fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit )
{
#define DEGREE_TO_METER 111319.49079327358
#define FEET_TO_METER 0.3048
#define NMILE_TO_METER 1852.0

  // Unify degree units
  if ( fromUnit == QGis::DecimalDegrees || fromUnit == QGis::DegreesMinutesSeconds || fromUnit == QGis::DegreesDecimalMinutes )
    fromUnit = QGis::Degrees;
  if ( toUnit == QGis::DecimalDegrees || toUnit == QGis::DegreesMinutesSeconds || toUnit == QGis::DegreesDecimalMinutes )
    toUnit = QGis::Degrees;

  // Calculate the conversion factor between the specified units
  if ( fromUnit != toUnit && fromUnit != QGis::UnknownUnit && toUnit != QGis::UnknownUnit )
  {
    switch ( fromUnit )
    {
      case QGis::Meters:
      {
        if ( toUnit == QGis::Feet ) return 1.0 / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return 1.0 / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return 1.0 / NMILE_TO_METER;
        break;
      }
      case QGis::Feet:
      {
        if ( toUnit == QGis::Meters ) return FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return FEET_TO_METER / DEGREE_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return FEET_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::Degrees:
      {
        if ( toUnit == QGis::Meters ) return DEGREE_TO_METER;
        if ( toUnit == QGis::Feet ) return DEGREE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::NauticalMiles ) return DEGREE_TO_METER / NMILE_TO_METER;
        break;
      }
      case QGis::NauticalMiles:
      {
        if ( toUnit == QGis::Meters ) return NMILE_TO_METER;
        if ( toUnit == QGis::Feet ) return NMILE_TO_METER / FEET_TO_METER;
        if ( toUnit == QGis::Degrees ) return NMILE_TO_METER / DEGREE_TO_METER;
        break;
      }
      case QGis::UnknownUnit:
        break;
    }
  }
  return 1.0;
}

double QGis::permissiveToDouble( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale::system().groupSeparator() );
  return QLocale::system().toDouble( string, &ok );
}

int QGis::permissiveToInt( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale::system().groupSeparator() );
  return QLocale::system().toInt( string, &ok );
}

void *qgsMalloc( size_t size )
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

void *qgsCalloc( size_t nmemb, size_t size )
{
  if ( nmemb == 0 || long( nmemb ) < 0 || size == 0 || long( size ) < 0 )
  {
    QgsDebugMsg( QString( "Negative or zero nmemb %1 or size %2." ).arg( nmemb ).arg( size ) );
    return NULL;
  }
  void *p = qgsMalloc( nmemb * size );
  if ( p != NULL )
  {
    memset( p, 0, nmemb * size );
  }
  return p;
}

void qgsFree( void *ptr )
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

QString qgsVsiPrefix( const QString& path )
{
  if ( path.startsWith( "/vsizip/", Qt::CaseInsensitive ) ||
       path.endsWith( ".zip", Qt::CaseInsensitive ) )
    return "/vsizip/";
  else if ( path.startsWith( "/vsitar/", Qt::CaseInsensitive ) ||
            path.endsWith( ".tar", Qt::CaseInsensitive ) ||
            path.endsWith( ".tar.gz", Qt::CaseInsensitive ) ||
            path.endsWith( ".tgz", Qt::CaseInsensitive ) )
    return "/vsitar/";
  else if ( path.startsWith( "/vsigzip/", Qt::CaseInsensitive ) ||
            path.endsWith( ".gz", Qt::CaseInsensitive ) )
    return "/vsigzip/";
  else
    return "";
}

QGis::WkbType QGis::singleType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBMultiPoint:         return WKBPoint;
    case WKBMultiLineString:    return WKBLineString;
    case WKBMultiPolygon:       return WKBPolygon;
    case WKBMultiPoint25D:      return WKBPoint25D;
    case WKBMultiLineString25D: return WKBLineString25D;
    case WKBMultiPolygon25D:    return WKBPolygon25D;
    default:                    return fromNewWkbType( QgsWKBTypes::singleType( fromOldWkbType( type ) ) );
  }
}

QGis::WkbType QGis::multiType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBPoint:         return WKBMultiPoint;
    case WKBLineString:    return WKBMultiLineString;
    case WKBPolygon:       return WKBMultiPolygon;
    case WKBPoint25D:      return WKBMultiPoint25D;
    case WKBLineString25D: return WKBMultiLineString25D;
    case WKBPolygon25D:    return WKBMultiPolygon25D;
    default:               return fromNewWkbType( QgsWKBTypes::multiType( fromOldWkbType( type ) ) );
  }
}

QGis::WkbType QGis::flatType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBPoint25D:           return WKBPoint;
    case WKBLineString25D:      return WKBLineString;
    case WKBPolygon25D:         return WKBPolygon;
    case WKBMultiPoint25D:      return WKBMultiPoint;
    case WKBMultiLineString25D: return WKBMultiLineString;
    case WKBMultiPolygon25D:    return WKBMultiPolygon;
    default:                    return fromNewWkbType( QgsWKBTypes::flatType( fromOldWkbType( type ) ) );
  }
}

bool QGis::isSingleType( QGis::WkbType type )
{
  return QgsWKBTypes::isSingleType( fromOldWkbType( type ) );
}

bool QGis::isMultiType( QGis::WkbType type )
{
  return QgsWKBTypes::isMultiType( fromOldWkbType( type ) );
}

int QGis::wkbDimensions( QGis::WkbType type )
{
  if ( type == WKBUnknown || type == WKBNoGeometry )
    return 0;

  QgsWKBTypes::Type wkbType = fromOldWkbType( type );
  return 2 + ( QgsWKBTypes::hasZ( wkbType ) ? 1 : 0 ) + ( QgsWKBTypes::hasM( wkbType ) ? 1 : 0 );
}

const char *QGis::vectorGeometryType( QGis::GeometryType type )
{
  switch ( type )
  {
    case Point:           return "Point";
    case Line:            return "Line";
    case Polygon:         return "Polygon";
    case UnknownGeometry: return "Unknown geometry";
    case NoGeometry:      return "No geometry";
    default:              return "Invalid type";
  }
}


const char *QGis::featureType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBUnknown:            return "WKBUnknown";
    case WKBPoint:              return "WKBPoint";
    case WKBLineString:         return "WKBLineString";
    case WKBPolygon:            return "WKBPolygon";
    case WKBMultiPoint:         return "WKBMultiPoint";
    case WKBMultiLineString:    return "WKBMultiLineString";
    case WKBMultiPolygon:       return "WKBMultiPolygon";
    case WKBNoGeometry:         return "WKBNoGeometry";
    case WKBPoint25D:           return "WKBPoint25D";
    case WKBLineString25D:      return "WKBLineString25D";
    case WKBPolygon25D:         return "WKBPolygon25D";
    case WKBMultiPoint25D:      return "WKBMultiPoint25D";
    case WKBMultiLineString25D: return "WKBMultiLineString25D";
    case WKBMultiPolygon25D:    return "WKBMultiPolygon25D";
    default:                    return "invalid wkbtype";

  }
}
