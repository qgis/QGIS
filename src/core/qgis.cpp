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
QString Qgis::QGIS_VERSION( QString::fromUtf8( VERSION ) );

// development version
const char* Qgis::QGIS_DEV_VERSION = QGSVERSION;

// Version number used for comparing versions using the
// "Check QGIS Version" function
const int Qgis::QGIS_VERSION_INT = VERSION_INT;

// Release name
QString Qgis::QGIS_RELEASE_NAME( QString::fromUtf8( RELEASE_NAME ) );

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

const double Qgis::DEFAULT_SEARCH_RADIUS_MM = 2.;

//! Default threshold between map coordinates and device coordinates for map2pixel simplification
const float Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD = 1.0f;

const QColor Qgis::DEFAULT_HIGHLIGHT_COLOR = QColor( 255, 0, 0, 128 );

double Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM = 0.5;

double Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM = 1.0;

double Qgis::SCALE_PRECISION = 0.9999999999;

QgsWKBTypes::Type Qgis::fromOldWkbType( Qgis::WkbType type )
{
  switch ( type )
  {
    case Qgis::WKBPoint:
      return QgsWKBTypes::Point;
    case Qgis::WKBLineString:
      return QgsWKBTypes::LineString;
    case Qgis::WKBPolygon:
      return QgsWKBTypes::Polygon;
    case Qgis::WKBMultiPoint:
      return QgsWKBTypes::MultiPoint;
    case Qgis::WKBMultiLineString:
      return QgsWKBTypes::MultiLineString;
    case Qgis::WKBMultiPolygon:
      return QgsWKBTypes::MultiPolygon;
    case Qgis::WKBNoGeometry:
      return QgsWKBTypes::NoGeometry;
    case Qgis::WKBPoint25D:
      return QgsWKBTypes::PointZ;
    case Qgis::WKBLineString25D:
      return QgsWKBTypes::LineStringZ;
    case Qgis::WKBPolygon25D:
      return QgsWKBTypes::PolygonZ;
    case Qgis::WKBMultiPoint25D:
      return QgsWKBTypes::MultiPointZ;
    case Qgis::WKBMultiLineString25D:
      return QgsWKBTypes::MultiLineStringZ;
    case Qgis::WKBMultiPolygon25D:
      return QgsWKBTypes::MultiPolygonZ;
    case Qgis::WKBUnknown:
      return QgsWKBTypes::Unknown;
    default:
      break;
  }

  QgsDebugMsg( QString( "unexpected old wkbType=%1" ).arg( type ) );
  return static_cast< QgsWKBTypes::Type >( type );
}

Qgis::WkbType Qgis::fromNewWkbType( QgsWKBTypes::Type type )
{
  switch ( type )
  {
    case QgsWKBTypes::Point:
      return Qgis::WKBPoint;
    case QgsWKBTypes::LineString:
      return Qgis::WKBLineString;
    case QgsWKBTypes::Polygon:
      return Qgis::WKBPolygon;
    case QgsWKBTypes::MultiPoint:
      return Qgis::WKBMultiPoint;
    case QgsWKBTypes::MultiLineString:
      return Qgis::WKBMultiLineString;
    case QgsWKBTypes::MultiPolygon:
      return Qgis::WKBMultiPolygon;
    case QgsWKBTypes::NoGeometry:
      return Qgis::WKBNoGeometry;
    case QgsWKBTypes::PointZ:
    case QgsWKBTypes::Point25D:
      return Qgis::WKBPoint25D;
    case QgsWKBTypes::LineStringZ:
    case QgsWKBTypes::LineString25D:
      return Qgis::WKBLineString25D;
    case QgsWKBTypes::PolygonZ:
    case QgsWKBTypes::Polygon25D:
      return Qgis::WKBPolygon25D;
    case QgsWKBTypes::MultiPointZ:
    case QgsWKBTypes::MultiPoint25D:
      return Qgis::WKBMultiPoint25D;
    case QgsWKBTypes::MultiLineStringZ:
    case QgsWKBTypes::MultiLineString25D:
      return Qgis::WKBMultiLineString25D;
    case QgsWKBTypes::MultiPolygonZ:
    case QgsWKBTypes::MultiPolygon25D:
      return Qgis::WKBMultiPolygon25D;
    default:
      break;
  }

  QgsDebugMsg( QString( "unexpected new wkbType=%1" ).arg( type ) );
  return static_cast< Qgis::WkbType >( type );
}

double qgsPermissiveToDouble( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale::system().groupSeparator() );
  return QLocale::system().toDouble( string, &ok );
}

int qgsPermissiveToInt( QString string, bool &ok )
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

Qgis::WkbType Qgis::singleType( Qgis::WkbType type )
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

Qgis::WkbType Qgis::multiType( Qgis::WkbType type )
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

Qgis::WkbType Qgis::flatType( Qgis::WkbType type )
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

bool Qgis::isSingleType( Qgis::WkbType type )
{
  return QgsWKBTypes::isSingleType( fromOldWkbType( type ) );
}

bool Qgis::isMultiType( Qgis::WkbType type )
{
  return QgsWKBTypes::isMultiType( fromOldWkbType( type ) );
}

int Qgis::wkbDimensions( Qgis::WkbType type )
{
  if ( type == WKBUnknown || type == WKBNoGeometry )
    return 0;

  QgsWKBTypes::Type wkbType = fromOldWkbType( type );
  return 2 + ( QgsWKBTypes::hasZ( wkbType ) ? 1 : 0 ) + ( QgsWKBTypes::hasM( wkbType ) ? 1 : 0 );
}

const char *Qgis::vectorGeometryType( Qgis::GeometryType type )
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


const char *Qgis::featureType( Qgis::WkbType type )
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
