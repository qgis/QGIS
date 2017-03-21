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
const QString Qgis::QGIS_VERSION( QStringLiteral( VERSION ) );

// development version
const char *Qgis::QGIS_DEV_VERSION = QGSVERSION;

// Version number used for comparing versions using the
// "Check QGIS Version" function
const int Qgis::QGIS_VERSION_INT = VERSION_INT;

// Release name
const QString Qgis::QGIS_RELEASE_NAME( QStringLiteral( RELEASE_NAME ) );
// Gdal Version string at build/compile time
QString Qgis::GDAL_BUILD_VERSION( QString::fromUtf8( GDAL_RELEASE_NAME ) );
const int Qgis::GDAL_BUILD_VERSION_MAJOR = GDAL_VERSION_MAJOR;
const int Qgis::GDAL_BUILD_VERSION_MINOR = GDAL_VERSION_MINOR;
const int Qgis::GDAL_BUILD_VERSION_REV = GDAL_VERSION_REV;
QString Qgis::GDAL_RUNTIME_VERSION = QString::null;
int Qgis::GDAL_RUNTIME_VERSION_MAJOR = -1;
int Qgis::GDAL_RUNTIME_VERSION_MINOR = -1;
int Qgis::GDAL_RUNTIME_VERSION_REV = -1;
int Qgis::GDAL_OGR_RUNTIME_SUPPORTED=0;
int Qgis::GDAL_RASTER_RUNTIME_SUPPORTED=0;
bool Qgis::ogrRuntimeSupport()
{ // Called in QgisApp and tested in QgsOgrProvider constructor
  if ( Qgis::GDAL_RUNTIME_VERSION.isNull() )
  { // do this only once
    Qgis::GDAL_RUNTIME_VERSION = QString( "%1" ).arg( GDALVersionInfo( "RELEASE_NAME" ) );
    // Remove non-numeric characters (with the excetion of '.') : '2.2.0dev' to '2.2.0'
    QString s_GdalVersionInfo = Qgis::GDAL_RUNTIME_VERSION;
    s_GdalVersionInfo.remove( QRegExp( QString::fromUtf8( "[-`~!@#$%^&*()_—+=|:;<>«»,?/{a-zA-Z}\'\"\\[\\]\\\\]" ) ) );
    QStringList sa_split = s_GdalVersionInfo.split( '.' );
    if ( sa_split.size() > 0 )
    { // setting gdal-runtime version
      Qgis::GDAL_RUNTIME_VERSION_MAJOR = sa_split[0].toInt();
      if ( sa_split.size() > 1 )
        Qgis::GDAL_RUNTIME_VERSION_MINOR = sa_split[1].toInt();
      if ( sa_split.size() > 2 )
        Qgis::GDAL_RUNTIME_VERSION_REV = sa_split[2].toInt();
    }
    // QgsOgrProvider constructor [with Layer is not valid] returns before QgsApplication::registerOgrDrivers is called when not 1
    // - tested in QgsApplication::registerOgrDrivers, QgisApp::about()
    //  --> QgisApp::about() will report version as 'Deprecated'
    // -> QgisApp::addVectorLayers should return 'is not a valid or recognized data source'
    if (Qgis::GDAL_RUNTIME_VERSION_MAJOR >= Qgis::GDAL_BUILD_VERSION_MAJOR)
    { // Deprecated gdal major-versions that are less than the major-version
       Qgis::GDAL_OGR_RUNTIME_SUPPORTED=1;
    }
    if (Qgis::GDAL_RUNTIME_VERSION_MAJOR >= Qgis::GDAL_BUILD_VERSION_MAJOR)
    { // // Deprecated gdal major-versions that are less than the major-version [GDALRasterIOEx]
       Qgis::GDAL_RASTER_RUNTIME_SUPPORTED=1;
    }
  }
  return (bool)Qgis::GDAL_OGR_RUNTIME_SUPPORTED;
}
bool Qgis::gdalRuntimeSupport()
{
  if ( Qgis::GDAL_RUNTIME_VERSION.isNull() )
  { // do this only once
    Qgis::ogrRuntimeSupport();
  }
  return (bool)Qgis::GDAL_RASTER_RUNTIME_SUPPORTED;
}

const QString GEOPROJ4 = QStringLiteral( "+proj=longlat +datum=WGS84 +no_defs" );

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

const QString GEO_EPSG_CRS_AUTHID = QStringLiteral( "EPSG:4326" );

const QString GEO_NONE = QStringLiteral( "NONE" );

const double Qgis::DEFAULT_SEARCH_RADIUS_MM = 2.;

//! Default threshold between map coordinates and device coordinates for map2pixel simplification
const float Qgis::DEFAULT_MAPTOPIXEL_THRESHOLD = 1.0f;

const QColor Qgis::DEFAULT_HIGHLIGHT_COLOR = QColor( 255, 0, 0, 128 );

const double Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM = 0.5;

const double Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM = 1.0;

const double Qgis::SCALE_PRECISION = 0.9999999999;

const double Qgis::DEFAULT_Z_COORDINATE = 0.0;

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

bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs )
{
  return ! qgsVariantLessThan( lhs, rhs );
}

QString qgsVsiPrefix( const QString &path )
{
  if ( path.startsWith( QLatin1String( "/vsizip/" ), Qt::CaseInsensitive ) ||
       path.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) )
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
    return QLatin1String( "" );
}
