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
#include "qgsunittypes.h"

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

double QGis::SCALE_PRECISION = 0.9999999999;

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
    case QGis::WKBPoint:
      return QgsWKBTypes::Point;
    case QGis::WKBLineString:
      return QgsWKBTypes::LineString;
    case QGis::WKBPolygon:
      return QgsWKBTypes::Polygon;
    case QGis::WKBMultiPoint:
      return QgsWKBTypes::MultiPoint;
    case QGis::WKBMultiLineString:
      return QgsWKBTypes::MultiLineString;
    case QGis::WKBMultiPolygon:
      return QgsWKBTypes::MultiPolygon;
    case QGis::WKBNoGeometry:
      return QgsWKBTypes::NoGeometry;
    case QGis::WKBPoint25D:
      return QgsWKBTypes::PointZ;
    case QGis::WKBLineString25D:
      return QgsWKBTypes::LineStringZ;
    case QGis::WKBPolygon25D:
      return QgsWKBTypes::PolygonZ;
    case QGis::WKBMultiPoint25D:
      return QgsWKBTypes::MultiPointZ;
    case QGis::WKBMultiLineString25D:
      return QgsWKBTypes::MultiLineStringZ;
    case QGis::WKBMultiPolygon25D:
      return QgsWKBTypes::MultiPolygonZ;
    case QGis::WKBUnknown:
      return QgsWKBTypes::Unknown;
    default:
      break;
  }

  QgsDebugMsg( QString( "unexpected old wkbType=%1" ).arg( type ) );
  return static_cast< QgsWKBTypes::Type >( type );
}

QGis::WkbType QGis::fromNewWkbType( QgsWKBTypes::Type type )
{
  switch ( type )
  {
    case QgsWKBTypes::Point:
      return QGis::WKBPoint;
    case QgsWKBTypes::LineString:
      return QGis::WKBLineString;
    case QgsWKBTypes::Polygon:
      return QGis::WKBPolygon;
    case QgsWKBTypes::MultiPoint:
      return QGis::WKBMultiPoint;
    case QgsWKBTypes::MultiLineString:
      return QGis::WKBMultiLineString;
    case QgsWKBTypes::MultiPolygon:
      return QGis::WKBMultiPolygon;
    case QgsWKBTypes::NoGeometry:
      return QGis::WKBNoGeometry;
    case QgsWKBTypes::PointZ:
    case QgsWKBTypes::Point25D:
      return QGis::WKBPoint25D;
    case QgsWKBTypes::LineStringZ:
    case QgsWKBTypes::LineString25D:
      return QGis::WKBLineString25D;
    case QgsWKBTypes::PolygonZ:
    case QgsWKBTypes::Polygon25D:
      return QGis::WKBPolygon25D;
    case QgsWKBTypes::MultiPointZ:
    case QgsWKBTypes::MultiPoint25D:
      return QGis::WKBMultiPoint25D;
    case QgsWKBTypes::MultiLineStringZ:
    case QgsWKBTypes::MultiLineString25D:
      return QGis::WKBMultiLineString25D;
    case QgsWKBTypes::MultiPolygonZ:
    case QgsWKBTypes::MultiPolygon25D:
      return QGis::WKBMultiPolygon25D;
    default:
      break;
  }

  QgsDebugMsg( QString( "unexpected new wkbType=%1" ).arg( type ) );
  return static_cast< QGis::WkbType >( type );
}


QGis::UnitType QGis::fromLiteral( const QString& literal, QGis::UnitType defaultType )
{
  bool ok = false;
  QGis::UnitType unit = QgsUnitTypes::decodeDistanceUnit( literal, &ok );
  return ok ? unit : defaultType;
}

QString QGis::toLiteral( QGis::UnitType unit )
{
  return QgsUnitTypes::encodeUnit( unit );
}

QString QGis::tr( QGis::UnitType unit )
{
  return QgsUnitTypes::toString( unit );
}

QGis::UnitType QGis::fromTr( const QString& literal, QGis::UnitType defaultType )
{
  bool ok = false;
  QGis::UnitType unit = QgsUnitTypes::stringToDistanceUnit( literal, &ok );
  return ok ? unit : defaultType;
}

double QGis::fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit )
{
  return QgsUnitTypes::fromUnitToUnitFactor( fromUnit, toUnit );
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
    return nullptr;
  }
  void *p = malloc( size );
  if ( !p )
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
    return nullptr;
  }
  void *p = qgsMalloc( nmemb * size );
  if ( p )
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
  // invalid < NULL < any value
  if ( !lhs.isValid() )
    return rhs.isValid();
  else if ( lhs.isNull() )
    return rhs.isValid() && !rhs.isNull();
  else if ( !rhs.isValid() || rhs.isNull() )
    return false;

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
    case QVariant::Bool:
      return lhs.toBool() < rhs.toBool();

    case QVariant::List:
    {
      const QList<QVariant> &lhsl = lhs.toList();
      const QList<QVariant> &rhsl = rhs.toList();

      int i, n = qMin( lhsl.size(), rhsl.size() );
      for ( i = 0; i < n && lhsl[i].type() == rhsl[i].type() && lhsl[i].isNull() == rhsl[i].isNull() && lhsl[i] == rhsl[i]; i++ )
        ;

      if ( i == n )
        return lhsl.size() < rhsl.size();
      else
        return qgsVariantLessThan( lhsl[i], rhsl[i] );
    }

    case QVariant::StringList:
    {
      const QStringList &lhsl = lhs.toStringList();
      const QStringList &rhsl = rhs.toStringList();

      int i, n = qMin( lhsl.size(), rhsl.size() );
      for ( i = 0; i < n && lhsl[i] == rhsl[i]; i++ )
        ;

      if ( i == n )
        return lhsl.size() < rhsl.size();
      else
        return lhsl[i] < rhsl[i];
    }

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
    case WKBMultiPoint:
      return WKBPoint;
    case WKBMultiLineString:
      return WKBLineString;
    case WKBMultiPolygon:
      return WKBPolygon;
    case WKBMultiPoint25D:
      return WKBPoint25D;
    case WKBMultiLineString25D:
      return WKBLineString25D;
    case WKBMultiPolygon25D:
      return WKBPolygon25D;
    default:
      return fromNewWkbType( QgsWKBTypes::singleType( fromOldWkbType( type ) ) );
  }
}

QGis::WkbType QGis::multiType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBPoint:
      return WKBMultiPoint;
    case WKBLineString:
      return WKBMultiLineString;
    case WKBPolygon:
      return WKBMultiPolygon;
    case WKBPoint25D:
      return WKBMultiPoint25D;
    case WKBLineString25D:
      return WKBMultiLineString25D;
    case WKBPolygon25D:
      return WKBMultiPolygon25D;
    default:
      return fromNewWkbType( QgsWKBTypes::multiType( fromOldWkbType( type ) ) );
  }
}

QGis::WkbType QGis::flatType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBPoint25D:
      return WKBPoint;
    case WKBLineString25D:
      return WKBLineString;
    case WKBPolygon25D:
      return WKBPolygon;
    case WKBMultiPoint25D:
      return WKBMultiPoint;
    case WKBMultiLineString25D:
      return WKBMultiLineString;
    case WKBMultiPolygon25D:
      return WKBMultiPolygon;
    default:
      return fromNewWkbType( QgsWKBTypes::flatType( fromOldWkbType( type ) ) );
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
    case Point:
      return "Point";
    case Line:
      return "Line";
    case Polygon:
      return "Polygon";
    case UnknownGeometry:
      return "Unknown geometry";
    case NoGeometry:
      return "No geometry";
    default:
      return "Invalid type";
  }
}


const char *QGis::featureType( QGis::WkbType type )
{
  switch ( type )
  {
    case WKBUnknown:
      return "WKBUnknown";
    case WKBPoint:
      return "WKBPoint";
    case WKBLineString:
      return "WKBLineString";
    case WKBPolygon:
      return "WKBPolygon";
    case WKBMultiPoint:
      return "WKBMultiPoint";
    case WKBMultiLineString:
      return "WKBMultiLineString";
    case WKBMultiPolygon:
      return "WKBMultiPolygon";
    case WKBNoGeometry:
      return "WKBNoGeometry";
    case WKBPoint25D:
      return "WKBPoint25D";
    case WKBLineString25D:
      return "WKBLineString25D";
    case WKBPolygon25D:
      return "WKBPolygon25D";
    case WKBMultiPoint25D:
      return "WKBMultiPoint25D";
    case WKBMultiLineString25D:
      return "WKBMultiLineString25D";
    case WKBMultiPolygon25D:
      return "WKBMultiPolygon25D";
    default:
      return "invalid wkbtype";

  }
}
