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
#include "moc_qgis.cpp"
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
#include "qgsgdalutils.h"
#include "qgswkbtypes.h"

#include <gdal.h>
#include <geos_c.h>
#include <ogr_api.h>

#define qgis_xstr(x) qgis_str(x)
#define qgis_str(x) #x

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

const Qgis::MapToolUnit Qgis::DEFAULT_SNAP_UNITS = Qgis::MapToolUnit::Pixels;

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
    QgsDebugError( QStringLiteral( "Zero size requested" ) );
    return nullptr;
  }

  if ( ( size >> ( 8 * sizeof( size ) - 1 ) ) != 0 )
  {
    QgsDebugError( QStringLiteral( "qgsMalloc - bad size requested: %1" ).arg( size ) );
    return nullptr;
  }

  void *p = malloc( size );
  if ( !p )
  {
    QgsDebugError( QStringLiteral( "Allocation of %1 bytes failed." ).arg( size ) );
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

  switch ( lhs.userType() )
  {
    case QMetaType::Type::Int:
      return lhs.toInt() < rhs.toInt();
    case QMetaType::Type::UInt:
      return lhs.toUInt() < rhs.toUInt();
    case QMetaType::Type::LongLong:
      return lhs.toLongLong() < rhs.toLongLong();
    case QMetaType::Type::ULongLong:
      return lhs.toULongLong() < rhs.toULongLong();
    case QMetaType::Type::Double:
      return lhs.toDouble() < rhs.toDouble();
    case QMetaType::Type::QChar:
      return lhs.toChar() < rhs.toChar();
    case QMetaType::Type::QDate:
      return lhs.toDate() < rhs.toDate();
    case QMetaType::Type::QTime:
      return lhs.toTime() < rhs.toTime();
    case QMetaType::Type::QDateTime:
      return lhs.toDateTime() < rhs.toDateTime();
    case QMetaType::Type::Bool:
      return lhs.toBool() < rhs.toBool();

    case QMetaType::Type::QVariantList:
    {
      const QList<QVariant> &lhsl = lhs.toList();
      const QList<QVariant> &rhsl = rhs.toList();

      int i, n = std::min( lhsl.size(), rhsl.size() );
      for ( i = 0; i < n && lhsl[i].userType() == rhsl[i].userType() && qgsVariantEqual( lhsl[i], rhsl[i] ); i++ )
        ;

      if ( i == n )
        return lhsl.size() < rhsl.size();
      else
        return qgsVariantLessThan( lhsl[i], rhsl[i] );
    }

    case QMetaType::Type::QStringList:
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
  return QgsGdalUtils::vsiPrefixForPath( path );
}

uint qHash( const QVariant &variant )
{
  if ( !variant.isValid() || variant.isNull() )
    return std::numeric_limits<uint>::max();

  switch ( variant.userType() )
  {
    case QMetaType::Type::Int:
      return qHash( variant.toInt() );
    case QMetaType::Type::UInt:
      return qHash( variant.toUInt() );
    case QMetaType::Type::Bool:
      return qHash( variant.toBool() );
    case QMetaType::Type::Double:
      return qHash( variant.toDouble() );
    case QMetaType::Type::LongLong:
      return qHash( variant.toLongLong() );
    case QMetaType::Type::ULongLong:
      return qHash( variant.toULongLong() );
    case QMetaType::Type::QString:
      return qHash( variant.toString() );
    case QMetaType::Type::QChar:
      return qHash( variant.toChar() );
    case QMetaType::Type::QVariantList:
      return qHash( variant.toList() );
    case QMetaType::Type::QStringList:
      return qHash( variant.toStringList() );
    case QMetaType::Type::QByteArray:
      return qHash( variant.toByteArray() );
    case QMetaType::Type::QDate:
      return qHash( variant.toDate() );
    case QMetaType::Type::QTime:
      return qHash( variant.toTime() );
    case QMetaType::Type::QDateTime:
      return qHash( variant.toDateTime() );
    case QMetaType::Type::QUrl:
    case QMetaType::Type::QLocale:
    case QMetaType::Type::QRegularExpression:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    case QMetaType::Type::QRegExp:
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
  static const int version = atoi( qgis_xstr( GEOS_VERSION_PATCH ) );
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

