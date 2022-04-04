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
#include "qgswkbtypes.h"

#include <gdal.h>
#include <geos_c.h>
#include <ogr_api.h>

#define xstr(x) str(x)
#define str(x) #x

// Version constants
//

// development version
const char *Qgis::QGIS_DEV_VERSION = QGSVERSION;

const double Qgis::DEFAULT_SEARCH_RADIUS_MM = 2.;

const float Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD = 1.0f;

const QColor Qgis::DEFAULT_HIGHLIGHT_COLOR = QColor( 255, 0, 0, 128 );

const double Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM = 0.5;

const double Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM = 1.0;

const double Qgis::SCALE_PRECISION = 0.9999999999;

const double Qgis::DEFAULT_Z_COORDINATE = 0.0;

const double Qgis::DEFAULT_M_COORDINATE = 0.0;

const double Qgis::DEFAULT_SNAP_TOLERANCE = 12.0;

const QgsTolerance::UnitType Qgis::DEFAULT_SNAP_UNITS = QgsTolerance::Pixels;

#ifdef Q_OS_WIN
const double Qgis::UI_SCALE_FACTOR = 1.5;
#else
const double Qgis::UI_SCALE_FACTOR = 1;
#endif

double qgsPermissiveToDouble( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale().groupSeparator() );
  return QLocale().toDouble( string, &ok );
}

int qgsPermissiveToInt( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale().groupSeparator() );
  return QLocale().toInt( string, &ok );
}

qlonglong qgsPermissiveToLongLong( QString string, bool &ok )
{
  //remove any thousands separators
  string.remove( QLocale().groupSeparator() );
  return QLocale().toLongLong( string, &ok );
}

void *qgsMalloc( size_t size )
{
  if ( size == 0 )
  {
    QgsDebugMsg( QStringLiteral( "Zero size requested" ) );
    return nullptr;
  }

  if ( ( size >> ( 8 * sizeof( size ) - 1 ) ) != 0 )
  {
    QgsDebugMsg( QStringLiteral( "qgsMalloc - bad size requested: %1" ).arg( size ) );
    return nullptr;
  }

  void *p = malloc( size );
  if ( !p )
  {
    QgsDebugMsg( QStringLiteral( "Allocation of %1 bytes failed." ).arg( size ) );
  }
  return p;
}

void qgsFree( void *ptr )
{
  free( ptr );
}

bool qgsVariantLessThan( const QVariant &lhs, const QVariant &rhs )
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

      int i, n = std::min( lhsl.size(), rhsl.size() );
      for ( i = 0; i < n && lhsl[i].type() == rhsl[i].type() && qgsVariantEqual( lhsl[i], rhsl[i] ); i++ )
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

      int i, n = std::min( lhsl.size(), rhsl.size() );
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

bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs )
{
  return ! qgsVariantLessThan( lhs, rhs );
}

QString qgsVsiPrefix( const QString &path )
{
  if ( path.startsWith( QLatin1String( "/vsizip/" ), Qt::CaseInsensitive ) )
    return QStringLiteral( "/vsizip/" );
  else if ( path.endsWith( QLatin1String( ".shp.zip" ), Qt::CaseInsensitive ) )
  {
    // GDAL 3.1 Shapefile driver directly handles .shp.zip files
    if ( GDALIdentifyDriverEx( path.toUtf8().constData(), GDAL_OF_VECTOR, nullptr, nullptr ) )
      return QString();
    return QStringLiteral( "/vsizip/" );
  }
  else if ( path.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) )
    return QStringLiteral( "/vsizip/" );
  else if ( path.startsWith( QLatin1String( "/vsitar/" ), Qt::CaseInsensitive ) ||
            path.endsWith( QLatin1String( ".tar" ), Qt::CaseInsensitive ) ||
            path.endsWith( QLatin1String( ".tar.gz" ), Qt::CaseInsensitive ) ||
            path.endsWith( QLatin1String( ".tgz" ), Qt::CaseInsensitive ) )
    return QStringLiteral( "/vsitar/" );
  else if ( path.startsWith( QLatin1String( "/vsigzip/" ), Qt::CaseInsensitive ) ||
            path.endsWith( QLatin1String( ".gz" ), Qt::CaseInsensitive ) )
    return QStringLiteral( "/vsigzip/" );
  else
    return QString();
}

uint qHash( const QVariant &variant )
{
  if ( !variant.isValid() || variant.isNull() )
    return std::numeric_limits<uint>::max();

  switch ( variant.type() )
  {
    case QVariant::Int:
      return qHash( variant.toInt() );
    case QVariant::UInt:
      return qHash( variant.toUInt() );
    case QVariant::Bool:
      return qHash( variant.toBool() );
    case QVariant::Double:
      return qHash( variant.toDouble() );
    case QVariant::LongLong:
      return qHash( variant.toLongLong() );
    case QVariant::ULongLong:
      return qHash( variant.toULongLong() );
    case QVariant::String:
      return qHash( variant.toString() );
    case QVariant::Char:
      return qHash( variant.toChar() );
    case QVariant::List:
      return qHash( variant.toList() );
    case QVariant::StringList:
      return qHash( variant.toStringList() );
    case QVariant::ByteArray:
      return qHash( variant.toByteArray() );
    case QVariant::Date:
      return qHash( variant.toDate() );
    case QVariant::Time:
      return qHash( variant.toTime() );
    case QVariant::DateTime:
      return qHash( variant.toDateTime() );
    case QVariant::Url:
    case QVariant::Locale:
    case QVariant::RegularExpression:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QVariant::RegExp:
#endif
      return qHash( variant.toString() );
    default:
      break;
  }

  return std::numeric_limits<uint>::max();
}

bool qgsVariantEqual( const QVariant &lhs, const QVariant &rhs )
{
  return ( lhs.isNull() == rhs.isNull() && lhs == rhs ) || ( lhs.isNull() && rhs.isNull() && lhs.isValid() && rhs.isValid() );
}

QString Qgis::defaultProjectScales()
{
  return QStringLiteral( "1:1000000,1:500000,1:250000,1:100000,1:50000,1:25000,"
                         "1:10000,1:5000,1:2500,1:1000,1:500" );
}

QString Qgis::version()
{
  return QString::fromUtf8( VERSION );
}

int Qgis::versionInt()
{
  // Version number used for comparing versions using the
  // "Check QGIS Version" function
  return VERSION_INT;
}

QString Qgis::releaseName()
{
  return QString::fromUtf8( RELEASE_NAME );
}

QString Qgis::devVersion()
{
  return QString::fromUtf8( QGIS_DEV_VERSION );
}

QString Qgis::geosVersion()
{
  return GEOSversion();
}

int Qgis::geosVersionInt()
{
  static const int version = QStringLiteral( "%1%2%3" )
                             .arg( GEOS_VERSION_MAJOR, 2, 10, QChar( '0' ) )
                             .arg( GEOS_VERSION_MINOR, 2, 10, QChar( '0' ) )
                             .arg( geosVersionPatch(), 2, 10, QChar( '0' ) ).toInt()
                             ;
  return version;
}

int Qgis::geosVersionMajor()
{
  return GEOS_VERSION_MAJOR;
}

int Qgis::geosVersionMinor()
{
  return GEOS_VERSION_MINOR;
}

int Qgis::geosVersionPatch()
{
  static const int version = atoi( xstr( GEOS_VERSION_PATCH ) );
  return version;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template<>
bool qMapLessThanKey<QVariantList>( const QVariantList &key1, const QVariantList &key2 )
{
  // qt's built in qMapLessThanKey for QVariantList is broken and does a case-insensitive operation.
  // this breaks QMap< QVariantList, ... >, where key matching incorrectly becomes case-insensitive..!!?!
  return qgsVariantGreaterThan( key1, key2 ) && key1 != key2;
}
#endif

