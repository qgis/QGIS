/***************************************************************************
                              qgswmsgetserversettings.cpp
                              ---------------------------
  begin                : Nov 7, 2019
  copyright            : (C) 2019 by Jorge Gustavo Rocha
  email                : jgr at geomaster dot pt
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmsutils.h"
#include "qgswmsgetprint.h"
#include "qgswmsrenderer.h"
#include "qgswmsserviceexception.h"
#include "qgsserverplugins.h"
#include "ogr_core.h"
#include "sqlite3.h"
#include "geos_c.h"
#include "pg_config.h"
#include "spatialite.h"
#include "qwt/qwt_global.h"
#if PROJ_VERSION_MAJOR>=6
#include <proj.h>
#else
#include <proj_api.h>
#endif
#include "qgis.h"
#include <QThread>
#include <QFontDatabase>
#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

namespace QgsWms
{

  QJsonArray availableFonts()
  {
    QJsonArray availableFamilies = {};
    QFontDatabase database;
    const QStringList fontFamilies = database.families();
    for ( const QString &family : fontFamilies )
    {
      QJsonObject thisFamily = {};
      thisFamily["family"] = family;
      QJsonArray familyStyles = {};
      const QStringList fontStyles = database.styles( family );
      for ( const QString &style : fontStyles )
      {
        QJsonObject thisStyle = {};
        thisStyle["style"] = style;
        // uncomment to provide all font sizes available
        // QJsonArray styleSizes = {};
        // QString sizes;
        // const QList<int> smoothSizes = database.smoothSizes( family, style );
        // for ( int points : smoothSizes )
        // {
        //   styleSizes.append( points );
        // }
        // thisStyle["sizes"] = styleSizes;
        familyStyles.append( thisStyle );
      }
      thisFamily["styles"] = familyStyles;
      availableFamilies.append( thisFamily );
    }
    return availableFamilies;
  }

  QJsonObject about()
  {
    QJsonObject aboutServer = {};
    QString caption = QStringLiteral( "QGIS Server - %1 ('%2')" ).arg( Qgis::QGIS_VERSION, Qgis::QGIS_RELEASE_NAME );
    aboutServer["Version"] = caption;
    if ( QString( Qgis::QGIS_DEV_VERSION ) == QLatin1String( "exported" ) )
    {
      caption = QStringLiteral( "https://github.com/qgis/QGIS/tree/release-%1_%2" )
                .arg( Qgis::QGIS_VERSION_INT / 10000 ).arg( Qgis::QGIS_VERSION_INT / 100 % 100 );
      aboutServer["QGIS code branch"] = caption;
    }
    else
    {
      caption = QStringLiteral( "https://github.com/qgis/QGIS/commit/%2" ).arg( Qgis::QGIS_DEV_VERSION );
      aboutServer["QGIS code revision"] = caption;
    }
    aboutServer["Compiled against Qt"] = QT_VERSION_STR;
    aboutServer["Running against Qt"] = qVersion();

    aboutServer["Compiled against GDAL/OGR"] = GDAL_RELEASE_NAME;
    aboutServer["Running against GDAL/OGR"] = GDALVersionInfo( "RELEASE_NAME" );

    aboutServer["Compiled against GEOS"] = GEOS_CAPI_VERSION;
    aboutServer["Running against GEOS"] = GEOSversion();

    aboutServer["Compiled against GEOS"] = SQLITE_VERSION;
    aboutServer["Running against GEOS"] = sqlite3_libversion();

#ifdef HAVE_POSTGRESQL
    aboutServer["PostgreSQL Client Version"] = QStringLiteral( PG_VERSION );
#else
    aboutServer["PostgreSQL Client Version"] = QStringLiteral( "No support" );
#endif

    aboutServer["SpatiaLite Version"] = spatialite_version();
    aboutServer["QWT Version"] = QWT_VERSION_STR;
    aboutServer["QScintilla2 Version"] = QSCINTILLA_VERSION_STR;

#if PROJ_VERSION_MAJOR > 4
    PJ_INFO info = proj_info();
    aboutServer["Compiled against PROJ"] = QStringLiteral( "%1.%2.%3" ).arg( PROJ_VERSION_MAJOR ).arg( PROJ_VERSION_MINOR ).arg( PROJ_VERSION_PATCH );
    aboutServer["Running against PROJ"] = info.release;
#else
    aboutServer["PROJ.4 Version"] = QString::number( PJ_VERSION );
#endif

    aboutServer["OS Version"] = QSysInfo::prettyProductName();
    aboutServer["Kernel version"] = QSysInfo::kernelVersion();
    aboutServer["Hostname"] = QSysInfo::machineHostName();
    aboutServer["CPU Cores"] = QThread::idealThreadCount();

#ifdef Q_OS_LINUX
    long pages = sysconf( _SC_PHYS_PAGES );
    long pagesAvailable = sysconf( _SC_AVPHYS_PAGES );
    long page_size = sysconf( _SC_PAGE_SIZE );
    aboutServer["System Memory (Mb)"] = QStringLiteral( "%1" ).arg( pages * page_size / 1024 / 1024 );
    aboutServer["Available memory (Mb)"] = QStringLiteral( "%1" ).arg( pagesAvailable * page_size / 1024 / 1024 );
#endif

    return aboutServer;
  }

  void writeServerSettings( QgsServerInterface *serverIface, QgsServerResponse &response )
  {
    QJsonObject serverSettings;
    serverSettings["about"] = about();
    serverSettings["settings"] = serverIface->serverSettings()->logSummaryJson();
    serverSettings["fonts"] = availableFonts();

    QJsonObject serverPlugins;
    QJsonArray availablePlugins = {};
    QStringList plugins = QgsServerPlugins::serverPlugins();
    for ( int i = 0; i < plugins.size(); ++i )
      availablePlugins.append( plugins.at( i ) );
    serverPlugins["enabled"] = availablePlugins;
    serverSettings["plugins"] = serverPlugins;

    QJsonDocument document( serverSettings );

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "application/json" ) );
    response.write( document.toJson() );
  }
} // namespace QgsWms
