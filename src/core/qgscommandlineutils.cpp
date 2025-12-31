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
  QString versionString = u"QGIS %1 '%2' (%3)\n"_s.arg( VERSION, RELEASE_NAME, QGSVERSION );

  // QGIS code revision
  if ( QString( Qgis::devVersion() ) == "exported"_L1 )
  {
    versionString += "QGIS code branch"_L1;
    if ( Qgis::version().endsWith( "Master"_L1 ) )
    {
      versionString += "master\n"_L1;
    }
    else
    {
      versionString += u"Release %1.%2\n"_s.arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 );
    }
  }
  else
  {
    versionString += u"QGIS code revision %1\n"_s.arg( Qgis::devVersion() );
  }

  // Qt version
  const QString qtVersionCompiled{ QT_VERSION_STR };
  const QString qtVersionRunning{ qVersion() };
  if ( qtVersionCompiled != qtVersionRunning )
  {
    versionString += u"Compiled against Qt %1\n"_s.arg( qtVersionCompiled );
    versionString += u"Running against Qt %1\n"_s.arg( qtVersionRunning );
  }
  else
  {
    versionString += u"Qt version %1\n"_s.arg( qtVersionCompiled );
  }

  // Python version
  versionString += u"Python version %1\n"_s.arg( PYTHON_VERSION );

  // GDAL version
  const QString gdalVersionCompiled { GDAL_RELEASE_NAME };
  const QString gdalVersionRunning { GDALVersionInfo( "RELEASE_NAME" ) };
  if ( gdalVersionCompiled != gdalVersionRunning )
  {
    versionString += u"Compiled against GDAL/OGR %1\n"_s.arg( gdalVersionCompiled );
    versionString += u"Running against GDAL/OGR %1\n"_s.arg( gdalVersionRunning );
  }
  else
  {
    versionString += u"GDAL/OGR version %1\n"_s.arg( gdalVersionCompiled );
  }

  // proj
  const PJ_INFO info = proj_info();
  const QString projVersionCompiled { u"%1.%2.%3"_s.arg( PROJ_VERSION_MAJOR ).arg( PROJ_VERSION_MINOR ).arg( PROJ_VERSION_PATCH ) };
  const QString projVersionRunning { info.version };
  if ( projVersionCompiled != projVersionRunning )
  {
    versionString += u"Compiled against PROJ %1\n"_s.arg( projVersionCompiled );
    versionString += u"Running against PROJ %2\n"_s.arg( projVersionRunning );
  }
  else
  {
    versionString += u"PROJ version %1\n"_s.arg( projVersionCompiled );
  }

  // CRS database versions
  versionString += u"EPSG Registry database version %1 (%2)\n"_s.arg( QgsProjUtils::epsgRegistryVersion(), QgsProjUtils::epsgRegistryDate().toString( Qt::ISODate ) );

  // GEOS version
  const QString geosVersionCompiled { GEOS_CAPI_VERSION };
  const QString geosVersionRunning { GEOSversion() };
  if ( geosVersionCompiled != geosVersionRunning )
  {
    versionString += u"Compiled against GEOS %1\n"_s.arg( geosVersionCompiled );
    versionString += u"Running against GEOS %1\n"_s.arg( geosVersionRunning );
  }
  else
  {
    versionString += u"GEOS version %1\n"_s.arg( geosVersionCompiled );
  }

  // SFCGAL version
#ifdef WITH_SFCGAL
  const QString sfcgalVersionCompiled { SFCGAL_VERSION };
  const QString sfcgalVersionRunning { sfcgal_version() };
  if ( sfcgalVersionCompiled != sfcgalVersionRunning )
  {
    versionString += u"Compiled against SFCGAL %1\n"_s.arg( sfcgalVersionCompiled );
    versionString += u"Running against SFCGAL %1\n"_s.arg( sfcgalVersionRunning );
  }
  else
  {
    versionString += u"SFCGAL version %1\n"_s.arg( sfcgalVersionCompiled );
  }
#else
  versionString += "No support for SFCGAL\n"_L1;
#endif

  // GeographicLib version
#ifdef WITH_GEOGRAPHICLIB
  const QString geographicLibVersionRunning = u"%1.%2.%3"_s.arg( GEOGRAPHICLIB_VERSION_MAJOR ).arg( GEOGRAPHICLIB_VERSION_MINOR ).arg( GEOGRAPHICLIB_VERSION_PATCH );
  versionString += u"GeographicLib version %1\n"_s.arg( geographicLibVersionRunning );
#else
  versionString += "No support for GeographicLib\n"_L1;
#endif

  // SQLite version
  const QString sqliteVersionCompiled { SQLITE_VERSION };
  const QString sqliteVersionRunning { sqlite3_libversion() };
  if ( sqliteVersionCompiled != sqliteVersionRunning )
  {
    versionString += u"Compiled against SQLite %1\n"_s.arg( sqliteVersionCompiled );
    versionString += u"Running against SQLite %1\n"_s.arg( sqliteVersionRunning );
  }
  else
  {
    versionString += u"SQLite version %1\n"_s.arg( sqliteVersionCompiled );
  }

  // Operating system
  versionString += u"OS %1\n"_s.arg( QSysInfo::prettyProductName() );

#ifdef QGISDEBUG
  versionString += "This copy of QGIS writes debugging output.\n"_L1;
#endif

  return versionString;
}
