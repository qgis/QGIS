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

#ifdef WITH_GEOGRAPHICLIB
#include <GeographicLib/Constants.hpp>
#endif

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

const int Qgis::USER_CRS_START_ID = 100000;
const double Qgis::DEFAULT_POINT_SIZE = 2.0;
const double Qgis::DEFAULT_LINE_WIDTH = 0.26;
const double Qgis::DEFAULT_SEGMENT_EPSILON = 1e-8;

const int Qgis::PREVIEW_JOB_DELAY_MS = 250;
const int Qgis::MAXIMUM_LAYER_PREVIEW_TIME_MS = 250;

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
    QgsDebugError( u"Zero size requested"_s );
    return nullptr;
  }

  if ( ( size >> ( 8 * sizeof( size ) - 1 ) ) != 0 )
  {
    QgsDebugError( u"qgsMalloc - bad size requested: %1"_s.arg( size ) );
    return nullptr;
  }

  void *p = malloc( size );
  if ( !p )
  {
    QgsDebugError( u"Allocation of %1 bytes failed."_s.arg( size ) );
  }
  return p;
}

void qgsFree( void *ptr )
{
  free( ptr );
}

int qgsVariantCompare( const QVariant &lhs, const QVariant &rhs, bool strictTypeCheck )
{
  // invalid < NULL < any value
  if ( !lhs.isValid() )
  {
    return rhs.isValid() ? -1 : 0;
  }
  else if ( lhs.isNull() )
  {
    if ( !rhs.isValid() )
      return 1;
    if ( rhs.isNull() )
      return 0;
    return -1;
  }
  else if ( !rhs.isValid() || rhs.isNull() )
  {
    return 1;
  }

  if ( strictTypeCheck && lhs.userType() != rhs.userType() )
  {
    return lhs.userType() < rhs.userType() ? -1 : 1;
  }

  // both valid
  switch ( lhs.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::Char:
    case QMetaType::Type::Short:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::Char:
        case QMetaType::Type::Short:
        {
          const int lhsInt = lhs.toInt();
          const int rhsInt = rhs.toInt();
          return lhsInt < rhsInt ? -1 : ( lhsInt == rhsInt ? 0 : 1 );
        }

        case QMetaType::Type::Long:
        case QMetaType::Type::LongLong:
        {
          const long long lhsInt = lhs.toLongLong();
          const long long rhsInt = rhs.toLongLong();
          return lhsInt < rhsInt ? -1 : ( lhsInt == rhsInt ? 0 : 1 );
        }

        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        {
          const double lhsDouble = static_cast< double >( lhs.toInt() );
          const double rhsDouble = rhs.toDouble();

          // consider NaN < any non-NaN
          const bool lhsIsNan = std::isnan( lhsDouble );
          const bool rhsIsNan = std::isnan( rhsDouble );
          if ( lhsIsNan )
          {
            return rhsIsNan ? 0 : -1;
          }
          else if ( rhsIsNan )
          {
            return 1;
          }

          return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
        }

        case QMetaType::Type::QString:
        {
          bool ok = false;
          const double rhsDouble = rhs.toDouble( &ok );
          if ( ok )
          {
            const double lhsDouble = static_cast< double >( lhs.toInt() );
            return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
          }
          break;
        }

        default:
          break;
      }
      break;
    }

    case QMetaType::Type::UInt:
    case QMetaType::Type::UChar:
    case QMetaType::Type::UShort:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::UInt:
        case QMetaType::Type::UChar:
        case QMetaType::Type::UShort:
        {
          const uint lhsUInt = lhs.toUInt();
          const uint rhsUInt = rhs.toUInt();
          return lhsUInt < rhsUInt ? -1 : ( lhsUInt == rhsUInt ? 0 : 1 );
        }

        case QMetaType::Type::ULong:
        case QMetaType::Type::ULongLong:
        {
          const qulonglong lhsInt = lhs.toULongLong();
          const qulonglong rhsInt = rhs.toULongLong();
          return lhsInt < rhsInt ? -1 : ( lhsInt == rhsInt ? 0 : 1 );
        }

        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        {
          const double lhsDouble = static_cast< double >( lhs.toUInt() );
          const double rhsDouble = rhs.toDouble();

          // consider NaN < any non-NaN
          const bool lhsIsNan = std::isnan( lhsDouble );
          const bool rhsIsNan = std::isnan( rhsDouble );
          if ( lhsIsNan )
          {
            return rhsIsNan ? 0 : -1;
          }
          else if ( rhsIsNan )
          {
            return 1;
          }

          return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
        }

        case QMetaType::Type::QString:
        {
          bool ok = false;
          const double rhsDouble = rhs.toDouble( &ok );
          if ( ok )
          {
            const double lhsDouble = static_cast< double >( lhs.toUInt() );
            return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
          }
          break;
        }

        default:
          break;
      }
      break;
    }

    case QMetaType::Type::LongLong:
    case QMetaType::Type::Long:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::Char:
        case QMetaType::Type::Short:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Long:
        {
          const qlonglong lhsLongLong = lhs.toLongLong();
          const qlonglong rhsLongLong = rhs.toLongLong();
          return lhsLongLong < rhsLongLong ? -1 : ( lhsLongLong == rhsLongLong ? 0 : 1 );
        }

        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        {
          const double lhsDouble = static_cast< double >( lhs.toLongLong() );
          const double rhsDouble = rhs.toDouble();

          // consider NaN < any non-NaN
          const bool lhsIsNan = std::isnan( lhsDouble );
          const bool rhsIsNan = std::isnan( rhsDouble );
          if ( lhsIsNan )
          {
            return rhsIsNan ? 0 : -1;
          }
          else if ( rhsIsNan )
          {
            return 1;
          }

          return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
        }

        case QMetaType::Type::QString:
        {
          bool ok = false;
          const double rhsDouble = rhs.toDouble( &ok );
          if ( ok )
          {
            const double lhsDouble = static_cast< double >( lhs.toLongLong() );
            return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
          }
          break;
        }

        default:
          break;
      }
      break;
    }

    case QMetaType::Type::ULongLong:
    case QMetaType::Type::ULong:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::UInt:
        case QMetaType::Type::UChar:
        case QMetaType::Type::UShort:
        case QMetaType::Type::ULongLong:
        case QMetaType::Type::ULong:
        {
          const qulonglong lhsULongLong = lhs.toULongLong();
          const qulonglong rhsULongLong = rhs.toULongLong();
          return lhsULongLong < rhsULongLong ? -1 : ( lhsULongLong == rhsULongLong ? 0 : 1 );
        }

        default:
          break;
      }
      break;
    }

    case QMetaType::Type::Double:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::Char:
        case QMetaType::Type::Short:
        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Long:
        case QMetaType::Type::ULongLong:
        case QMetaType::Type::ULong:
        case QMetaType::Type::QString:
        {
          const double lhsDouble = lhs.toDouble();
          bool rhsIsDoubleCompatible = false;
          const double rhsDouble = rhs.toDouble( &rhsIsDoubleCompatible );
          if ( rhsIsDoubleCompatible )
          {
            // consider NaN < any non-NaN
            const bool lhsIsNan = std::isnan( lhsDouble );
            const bool rhsIsNan = std::isnan( rhsDouble );
            if ( lhsIsNan )
            {
              return rhsIsNan ? 0 : -1;
            }
            else if ( rhsIsNan )
            {
              return 1;
            }

            return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
          }
          break;
        }

        default:
          break;
      }
      break;
    }

    case QMetaType::Type::Float:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::Char:
        case QMetaType::Type::Short:
        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Long:
        case QMetaType::Type::ULongLong:
        case QMetaType::Type::ULong:
        case QMetaType::Type::QString:
        {
          const float lhsFloat = lhs.toFloat();
          bool rhsIsFloatCompatible = false;
          const float rhsFloat = rhs.toFloat( &rhsIsFloatCompatible );
          if ( rhsIsFloatCompatible )
          {
            // consider NaN < any non-NaN
            const bool lhsIsNan = std::isnan( lhsFloat );
            const bool rhsIsNan = std::isnan( rhsFloat );
            if ( lhsIsNan )
            {
              return rhsIsNan ? 0 : -1;
            }
            else if ( rhsIsNan )
            {
              return 1;
            }

            return lhsFloat < rhsFloat ? -1 : ( lhsFloat == rhsFloat ? 0 : 1 );
          }
          break;
        }
        default:
          break;
      }
      break;
    }

    case QMetaType::Type::QChar:
    {
      if ( rhs.userType() == QMetaType::Type::QChar )
      {
        const QChar lhsChar = lhs.toChar();
        const QChar rhsChar = rhs.toChar();
        return lhsChar < rhsChar ? -1 : ( lhsChar == rhsChar ? 0 : 1 );
      }
      break;
    }

    case QMetaType::Type::QDate:
    {
      if ( rhs.userType() == QMetaType::Type::QDate )
      {
        const QDate lhsDate = lhs.toDate();
        const QDate rhsDate = rhs.toDate();
        return lhsDate < rhsDate ? -1 : ( lhsDate == rhsDate ? 0 : 1 );
      }
      break;
    }

    case QMetaType::Type::QTime:
    {
      if ( rhs.userType() == QMetaType::Type::QTime )
      {
        const QTime lhsTime = lhs.toTime();
        const QTime rhsTime = rhs.toTime();
        return lhsTime < rhsTime ? -1 : ( lhsTime == rhsTime ? 0 : 1 );
      }
      break;
    }

    case QMetaType::Type::QDateTime:
    {
      if ( rhs.userType() == QMetaType::Type::QDateTime )
      {
        const QDateTime lhsTime = lhs.toDateTime();
        const QDateTime rhsTime = rhs.toDateTime();
        return lhsTime < rhsTime ? -1 : ( lhsTime == rhsTime ? 0 : 1 );
      }
      break;
    }

    case QMetaType::Type::Bool:
    {
      if ( rhs.userType() == QMetaType::Type::Bool )
      {
        const bool lhsBool = lhs.toBool();
        const bool rhsBool = rhs.toBool();
        return lhsBool == rhsBool ? 0 : ( lhsBool ? 1 : -1 );
      }
      break;
    }

    case QMetaType::Type::QVariantList:
    {
      if ( rhs.userType() == QMetaType::Type::QVariantList )
      {
        const QList<QVariant> &lhsl = lhs.toList();
        const QList<QVariant> &rhsl = rhs.toList();

        int i, n = std::min( lhsl.size(), rhsl.size() );
        for ( i = 0; i < n && lhsl[i].userType() == rhsl[i].userType() && qgsVariantCompare( lhsl[i], rhsl[i] ) == 0; i++ )
          ;

        if ( i == n )
          return lhsl.size() < rhsl.size() ? -1 : ( lhsl.size() > rhsl.size() ? 1 : 0 );
        else
          return qgsVariantCompare( lhsl[i], rhsl[i] );
      }
      break;
    }

    case QMetaType::Type::QStringList:
    {
      if ( rhs.userType() == QMetaType::Type::QStringList )
      {
        const QStringList &lhsl = lhs.toStringList();
        const QStringList &rhsl = rhs.toStringList();

        int i, n = std::min( lhsl.size(), rhsl.size() );
        for ( i = 0; i < n && lhsl[i] == rhsl[i]; i++ )
          ;

        if ( i == n )
          return lhsl.size() < rhsl.size() ? -1 : ( lhsl.size() > rhsl.size() ? 1 : 0 );
        else
          return lhsl[i] < rhsl[i] ? -1 : ( lhsl[i] == rhsl[i] ? 0 : 1 );
      }
      break;
    }

    case QMetaType::Type::QString:
    {
      switch ( rhs.userType() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::Char:
        case QMetaType::Type::Short:
        case QMetaType::Type::Double:
        case QMetaType::Type::Float:
        case QMetaType::Type::LongLong:
        case QMetaType::Type::Long:
        case QMetaType::Type::ULongLong:
        case QMetaType::Type::ULong:
        {
          // try string to numeric conversion
          bool lhsIsDoubleCompatible = false;
          const double lhsDouble = lhs.toDouble( &lhsIsDoubleCompatible );
          if ( lhsIsDoubleCompatible )
          {
            const double rhsDouble = rhs.toDouble();
            // consider NaN < any non-NaN
            const bool lhsIsNan = std::isnan( lhsDouble );
            const bool rhsIsNan = std::isnan( rhsDouble );
            if ( lhsIsNan )
            {
              return rhsIsNan ? 0 : -1;
            }
            else if ( rhsIsNan )
            {
              return 1;
            }

            return lhsDouble < rhsDouble ? -1 : ( lhsDouble == rhsDouble ? 0 : 1 );
          }
          break;
        }

        default:
          break;
      }

      break;
    }

    default:
      break;
  }
  return std::clamp( QString::localeAwareCompare( lhs.toString(), rhs.toString() ), -1, 1 );
}

bool qgsVariantLessThan( const QVariant &lhs, const QVariant &rhs )
{
  return qgsVariantCompare( lhs, rhs ) < 0;
}

bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs )
{
  return qgsVariantCompare( lhs, rhs ) > 0;
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
      return qHash( variant.toString() );
    default:
      break;
  }

  return std::numeric_limits<uint>::max();
}

bool qgsVariantEqual( const QVariant &lhs, const QVariant &rhs )
{
  if ( lhs.isNull() == rhs.isNull() )
  {
    if ( lhs.type() == QVariant::String && rhs.type() == QVariant::String )
    {
      const QString lhsString = lhs.toString();
      const QString rhsString = rhs.toString();
      return lhsString.isNull() == rhsString.isNull()
             && lhsString.isEmpty() == rhsString.isEmpty()
             && lhsString == rhsString;
    }
    if ( lhs == rhs )
      return true;
  }

  return ( lhs.isNull() && rhs.isNull() && lhs.isValid() && rhs.isValid() );
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

bool Qgis::hasSfcgal()
{
#ifdef WITH_SFCGAL
  return true;
#else
  return false;
#endif
}

int Qgis::sfcgalVersionInt()
{
#ifdef WITH_SFCGAL
  return SFCGAL_VERSION_MAJOR_INT * 10000 + SFCGAL_VERSION_MINOR_INT * 100 + SFCGAL_VERSION_PATCH_INT;
#else
  throw QgsNotSupportedException( QObject::tr( "This operation requires a QGIS build based SFCGAL." ) );
#endif
}

bool Qgis::hasGeographicLib()
{
#ifdef WITH_GEOGRAPHICLIB
  return true;
#else
  return false;
#endif
}

int Qgis::geographicLibVersion()
{
#ifdef WITH_GEOGRAPHICLIB
  return GEOGRAPHICLIB_VERSION_MAJOR * 10000 + GEOGRAPHICLIB_VERSION_MINOR * 100 + GEOGRAPHICLIB_VERSION_PATCH;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool Qgis::hasQtWebkit()
{
  return false;
}

int Qgis::geosVersionInt()
{
  static const int version = u"%1%2%3"_s
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
