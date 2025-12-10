/***************************************************************************
                             qgscommandlineutils.cpp
                             ---------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscommandlineutils.h"

#include <gdal_version.h>
#include <ogr_api.h>
#include <proj.h>
#include <sqlite3.h>

#include "qgsapplication.h"
#include "qgsgeos.h"
#include "qgsprojutils.h"
#include "qgsversion.h"

#include <QSysInfo>

#ifdef WITH_SFCGAL
#include <SFCGAL/capi/sfcgal_c.h>
#include <SFCGAL/version.h>
#endif

#ifdef WITH_GEOGRAPHICLIB
#include <GeographicLib/Constants.hpp>
#endif

QString QgsCommandLineUtils::allVersions( )
{

  // QGIS main version
  QString versionString = QStringLiteral( "QGIS %1 '%2' (%3)\n" ).arg( VERSION, RELEASE_NAME, QGSVERSION );

  // QGIS code revision
  if ( QString( Qgis::devVersion() ) == QLatin1String( "exported" ) )
  {
    versionString += QLatin1String( "QGIS code branch" );
    if ( Qgis::version().endsWith( QLatin1String( "Master" ) ) )
    {
      versionString += QLatin1String( "master\n" );
    }
    else
    {
      versionString += QStringLiteral( "Release %1.%2\n" ).arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 );
    }
  }
  else
  {
    versionString += QStringLiteral( "QGIS code revision %1\n" ).arg( Qgis::devVersion() );
  }

  // Qt version
  const QString qtVersionCompiled{ QT_VERSION_STR };
  const QString qtVersionRunning{ qVersion() };
  if ( qtVersionCompiled != qtVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against Qt %1\n" ).arg( qtVersionCompiled );
    versionString += QStringLiteral( "Running against Qt %1\n" ).arg( qtVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "Qt version %1\n" ).arg( qtVersionCompiled );
  }

  // Python version
  versionString += QStringLiteral( "Python version %1\n" ).arg( PYTHON_VERSION );

  // GDAL version
  const QString gdalVersionCompiled { GDAL_RELEASE_NAME };
  const QString gdalVersionRunning { GDALVersionInfo( "RELEASE_NAME" ) };
  if ( gdalVersionCompiled != gdalVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against GDAL/OGR %1\n" ).arg( gdalVersionCompiled );
    versionString += QStringLiteral( "Running against GDAL/OGR %1\n" ).arg( gdalVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "GDAL/OGR version %1\n" ).arg( gdalVersionCompiled );
  }

  // proj
  const PJ_INFO info = proj_info();
  const QString projVersionCompiled { QStringLiteral( "%1.%2.%3" ).arg( PROJ_VERSION_MAJOR ).arg( PROJ_VERSION_MINOR ).arg( PROJ_VERSION_PATCH ) };
  const QString projVersionRunning { info.version };
  if ( projVersionCompiled != projVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against PROJ %1\n" ).arg( projVersionCompiled );
    versionString += QStringLiteral( "Running against PROJ %2\n" ).arg( projVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "PROJ version %1\n" ).arg( projVersionCompiled );
  }

  // CRS database versions
  versionString += QStringLiteral( "EPSG Registry database version %1 (%2)\n" ).arg( QgsProjUtils::epsgRegistryVersion(), QgsProjUtils::epsgRegistryDate().toString( Qt::ISODate ) );

  // GEOS version
  const QString geosVersionCompiled { GEOS_CAPI_VERSION };
  const QString geosVersionRunning { GEOSversion() };
  if ( geosVersionCompiled != geosVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against GEOS %1\n" ).arg( geosVersionCompiled );
    versionString += QStringLiteral( "Running against GEOS %1\n" ).arg( geosVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "GEOS version %1\n" ).arg( geosVersionCompiled );
  }

  // SFCGAL version
#ifdef WITH_SFCGAL
  const QString sfcgalVersionCompiled { SFCGAL_VERSION };
  const QString sfcgalVersionRunning { sfcgal_version() };
  if ( sfcgalVersionCompiled != sfcgalVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against SFCGAL %1\n" ).arg( sfcgalVersionCompiled );
    versionString += QStringLiteral( "Running against SFCGAL %1\n" ).arg( sfcgalVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "SFCGAL version %1\n" ).arg( sfcgalVersionCompiled );
  }
#else
  versionString += QLatin1String( "No support for SFCGAL\n" );
#endif

  // GeographicLib version
#ifdef WITH_GEOGRAPHICLIB
  const QString geographicLibVersionRunning = QStringLiteral( "%1.%2.%3" ).arg( GEOGRAPHICLIB_VERSION_MAJOR ).arg( GEOGRAPHICLIB_VERSION_MINOR ).arg( GEOGRAPHICLIB_VERSION_PATCH );
  versionString += QStringLiteral( "GeographicLib version %1\n" ).arg( geographicLibVersionRunning );
#else
  versionString += QLatin1String( "No support for GeographicLib\n" );
#endif

  // SQLite version
  const QString sqliteVersionCompiled { SQLITE_VERSION };
  const QString sqliteVersionRunning { sqlite3_libversion() };
  if ( sqliteVersionCompiled != sqliteVersionRunning )
  {
    versionString += QStringLiteral( "Compiled against SQLite %1\n" ).arg( sqliteVersionCompiled );
    versionString += QStringLiteral( "Running against SQLite %1\n" ).arg( sqliteVersionRunning );
  }
  else
  {
    versionString += QStringLiteral( "SQLite version %1\n" ).arg( sqliteVersionCompiled );
  }

  // Operating system
  versionString += QStringLiteral( "OS %1\n" ).arg( QSysInfo::prettyProductName() );

#ifdef QGISDEBUG
  versionString += QLatin1String( "This copy of QGIS writes debugging output.\n" );
#endif

  return versionString;
}
