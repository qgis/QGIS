/***************************************************************************
                    qgsserversettings.cpp
                    ---------------------
  begin                : December 19, 2016
  copyright            : (C) 2016 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserversettings.h"
#include "qgsapplication.h"

#include <QSettings>
#include <QDir>

QgsServerSettings::QgsServerSettings()
{
  load();
}

void QgsServerSettings::initSettings()
{
  mSettings.clear();

  // options path
  const Setting sOptPath = { QgsServerSettingsEnv::QGIS_OPTIONS_PATH,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             QStringLiteral( "Override the default path for user configuration" ),
                             QString(),
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sOptPath.envVar ] = sOptPath;

  // parallel rendering
  const Setting sParRend = { QgsServerSettingsEnv::QGIS_SERVER_PARALLEL_RENDERING,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             QStringLiteral( "Activate/Deactivate parallel rendering for WMS getMap request" ),
                             QStringLiteral( "/qgis/parallel_rendering" ),
                             QVariant::Bool,
                             QVariant( false ),
                             QVariant()
                           };
  mSettings[ sParRend.envVar ] = sParRend;

  // max threads
  const Setting sMaxThreads = { QgsServerSettingsEnv::QGIS_SERVER_MAX_THREADS,
                                QgsServerSettingsEnv::DEFAULT_VALUE,
                                QStringLiteral( "Number of threads to use when parallel rendering is activated" ),
                                QStringLiteral( "/qgis/max_threads" ),
                                QVariant::Int,
                                QVariant( -1 ),
                                QVariant()
                              };
  mSettings[ sMaxThreads.envVar ] = sMaxThreads;

  // log level
  const Setting sLogLevel = { QgsServerSettingsEnv::QGIS_SERVER_LOG_LEVEL,
                              QgsServerSettingsEnv::DEFAULT_VALUE,
                              QStringLiteral( "Log level" ),
                              QString(),
                              QVariant::Int,
                              QVariant::fromValue( Qgis::MessageLevel::NoLevel ),
                              QVariant()
                            };
  mSettings[ sLogLevel.envVar ] = sLogLevel;

  // log file
  const Setting sLogFile = { QgsServerSettingsEnv::QGIS_SERVER_LOG_FILE,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             QStringLiteral( "Log file" ),
                             QString(),
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sLogFile.envVar ] = sLogFile;

  // log to stderr
  const Setting sLogStderr = { QgsServerSettingsEnv::QGIS_SERVER_LOG_STDERR,
                               QgsServerSettingsEnv::DEFAULT_VALUE,
                               QStringLiteral( "Activate/Deactivate logging to stderr" ),
                               QString(),
                               QVariant::Bool,
                               QVariant( false ),
                               QVariant()
                             };
  mSettings[ sLogStderr.envVar ] = sLogStderr;

  // project file
  const Setting sProject = { QgsServerSettingsEnv::QGIS_PROJECT_FILE,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             QStringLiteral( "QGIS project file" ),
                             QString(),
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sProject.envVar ] = sProject;

  // cache directory
  const Setting sCacheDir = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_DIRECTORY,
                              QgsServerSettingsEnv::DEFAULT_VALUE,
                              QStringLiteral( "Specify the cache directory" ),
                              QStringLiteral( "/cache/directory" ),
                              QVariant::String,
                              QVariant( QgsApplication::qgisSettingsDirPath() + "cache" ),
                              QVariant()
                            };
  mSettings[ sCacheDir.envVar ] = sCacheDir;

  // cache size
  const Setting sCacheSize = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_SIZE,
                               QgsServerSettingsEnv::DEFAULT_VALUE,
                               QStringLiteral( "Specify the cache size" ),
                               QStringLiteral( "/cache/size" ),
                               QVariant::LongLong,
                               QVariant( 256 * 1024 * 1024 ),
                               QVariant()
                             };
  mSettings[ sCacheSize.envVar ] = sCacheSize;

  // system locale override
  const Setting sOverrideSystemLocale = { QgsServerSettingsEnv::QGIS_SERVER_OVERRIDE_SYSTEM_LOCALE,
                                          QgsServerSettingsEnv::DEFAULT_VALUE,
                                          QStringLiteral( "Override system locale" ),
                                          QStringLiteral( "/locale/userLocale" ),
                                          QVariant::String,
                                          QVariant( "" ),
                                          QVariant()
                                        };
  mSettings[ sOverrideSystemLocale.envVar ] = sOverrideSystemLocale;

  // bad layers handling
  const Setting sIgnoreBadLayers = { QgsServerSettingsEnv::QGIS_SERVER_IGNORE_BAD_LAYERS,
                                     QgsServerSettingsEnv::DEFAULT_VALUE,
                                     QStringLiteral( "Ignore bad layers" ),
                                     QString(),
                                     QVariant::Bool,
                                     QVariant( false ),
                                     QVariant()
                                   };
  mSettings[ sIgnoreBadLayers.envVar ] = sIgnoreBadLayers;

  // trust layer metadata
  const Setting sTrustLayerMetadata = { QgsServerSettingsEnv::QGIS_SERVER_TRUST_LAYER_METADATA,
                                        QgsServerSettingsEnv::DEFAULT_VALUE,
                                        QStringLiteral( "Trust layer metadata" ),
                                        QString(),
                                        QVariant::Bool,
                                        QVariant( false ),
                                        QVariant()
                                      };
  mSettings[ sTrustLayerMetadata.envVar ] = sTrustLayerMetadata;

  // don't load layouts
  const Setting sDontLoadLayouts = { QgsServerSettingsEnv::QGIS_SERVER_DISABLE_GETPRINT,
                                     QgsServerSettingsEnv::DEFAULT_VALUE,
                                     QStringLiteral( "Don't load layouts" ),
                                     QString(),
                                     QVariant::Bool,
                                     QVariant( false ),
                                     QVariant()
                                   };
  mSettings[ sDontLoadLayouts.envVar ] = sDontLoadLayouts;

  // show group separator
  const Setting sShowGroupSeparator = { QgsServerSettingsEnv::QGIS_SERVER_SHOW_GROUP_SEPARATOR,
                                        QgsServerSettingsEnv::DEFAULT_VALUE,
                                        QStringLiteral( "Show group (thousands) separator" ),
                                        QStringLiteral( "/locale/showGroupSeparator" ),
                                        QVariant::String,
                                        QVariant( false ),
                                        QVariant()
                                      };
  mSettings[ sShowGroupSeparator.envVar ] = sShowGroupSeparator;

  // max height
  const Setting sMaxHeight = { QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_HEIGHT,
                               QgsServerSettingsEnv::DEFAULT_VALUE,
                               QStringLiteral( "Maximum height for a WMS request. The lower one of this and the project configuration is used." ),
                               QStringLiteral( "/qgis/max_wms_height" ),
                               QVariant::LongLong,
                               QVariant( -1 ),
                               QVariant()
                             };
  mSettings[ sMaxHeight.envVar ] = sMaxHeight;

  // max width
  const Setting sMaxWidth = { QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_WIDTH,
                              QgsServerSettingsEnv::DEFAULT_VALUE,
                              QStringLiteral( "Maximum width for a WMS request. The most conservative between this and the project one is used" ),
                              QStringLiteral( "/qgis/max_wms_width" ),
                              QVariant::LongLong,
                              QVariant( -1 ),
                              QVariant()
                            };
  mSettings[ sMaxWidth.envVar ] = sMaxWidth;

  // API templates and static override directory
  const Setting sApiResourcesDirectory = { QgsServerSettingsEnv::QGIS_SERVER_API_RESOURCES_DIRECTORY,
                                           QgsServerSettingsEnv::DEFAULT_VALUE,
                                           QStringLiteral( "Base directory where HTML templates and static assets (e.g. images, js and css files) are searched for" ),
                                           QStringLiteral( "/qgis/server_api_resources_directory" ),
                                           QVariant::String,
                                           QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/server/api" ) ),
                                           QString()
                                         };

  mSettings[ sApiResourcesDirectory.envVar ] = sApiResourcesDirectory;

  // API WFS3 max limit
  const Setting sApiWfs3MaxLimit = { QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_MAX_LIMIT,
                                     QgsServerSettingsEnv::DEFAULT_VALUE,
                                     QStringLiteral( "Maximum value for \"limit\" in a features request, defaults to 10000" ),
                                     QStringLiteral( "/qgis/server_api_wfs3_max_limit" ),
                                     QVariant::LongLong,
                                     QVariant( 10000 ),
                                     QVariant()
                                   };

  mSettings[ sApiWfs3MaxLimit.envVar ] = sApiWfs3MaxLimit;

  // projects directory for landing page service
  const Setting sProjectsDirectories = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES,
                                         QgsServerSettingsEnv::DEFAULT_VALUE,
                                         QStringLiteral( "Directories used by the landing page service to find .qgs and .qgz projects" ),
                                         QStringLiteral( "/qgis/server_projects_directories" ),
                                         QVariant::String,
                                         QVariant( "" ),
                                         QVariant()
                                       };

  mSettings[ sProjectsDirectories.envVar ] = sProjectsDirectories;

  // postgresql connection string for landing page service
  const Setting sProjectsPgConnections = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS,
                                           QgsServerSettingsEnv::DEFAULT_VALUE,
                                           QStringLiteral( "PostgreSQL connection strings used by the landing page service to find projects" ),
                                           QStringLiteral( "/qgis/server_projects_pg_connections" ),
                                           QVariant::String,
                                           QVariant( "" ),
                                           QVariant()
                                         };

  mSettings[ sProjectsPgConnections.envVar ] = sProjectsPgConnections;

  // landing page base URL prefix
  const Setting sLandingPageBaseUrlPrefix = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PREFIX,
                                              QgsServerSettingsEnv::DEFAULT_VALUE,
                                              QStringLiteral( "Landing page base URL path prefix" ),
                                              QStringLiteral( "/qgis/server_landing_page_base_url_prefix" ),
                                              QVariant::String,
                                              QVariant( "" ),
                                              QVariant()
                                            };

  mSettings[ sLandingPageBaseUrlPrefix.envVar ] = sLandingPageBaseUrlPrefix;

  // log profile
  const Setting sLogProfile = { QgsServerSettingsEnv::QGIS_SERVER_LOG_PROFILE,
                                QgsServerSettingsEnv::DEFAULT_VALUE,
                                QStringLiteral( "Add detailed profile information to the logs, only effective when QGIS_SERVER_LOG_LEVEL=0" ),
                                QStringLiteral( "/qgis/server_log_profile" ),
                                QVariant::Bool,
                                QVariant( false ),
                                QVariant()
                              };

  mSettings[ sLogProfile.envVar ] = sLogProfile;

  // the default service URL.
  const Setting sServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_SERVICE_URL,
                                QgsServerSettingsEnv::DEFAULT_VALUE,
                                QStringLiteral( "The default service URL" ),
                                QStringLiteral( "/qgis/server_service_url" ),
                                QVariant::String,
                                QVariant( "" ),
                                QVariant()
                              };
  mSettings[ sServiceUrl.envVar ] = sServiceUrl;

  // the default WMS service URL.
  const Setting sWmsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WMS_SERVICE_URL,
                                   QgsServerSettingsEnv::DEFAULT_VALUE,
                                   QStringLiteral( "The default WMS service URL" ),
                                   QStringLiteral( "/qgis/server_wms_service_url" ),
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
  mSettings[ sServiceUrl.envVar ] = sWmsServiceUrl;

  // the default WFS service URL.
  const Setting sWfsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WFS_SERVICE_URL,
                                   QgsServerSettingsEnv::DEFAULT_VALUE,
                                   QStringLiteral( "The default WFS service URL" ),
                                   QStringLiteral( "/qgis/server_wfs_service_url" ),
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
  mSettings[ sServiceUrl.envVar ] = sWfsServiceUrl;

  // the default WCS service URL.
  const Setting sWcsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WCS_SERVICE_URL,
                                   QgsServerSettingsEnv::DEFAULT_VALUE,
                                   QStringLiteral( "The default WcS service URL" ),
                                   QStringLiteral( "/qgis/server_wcs_service_url" ),
                                   QVariant::String,
                                   QVariant( "" ),
                                   QVariant()
                                 };
  mSettings[ sServiceUrl.envVar ] = sWfsServiceUrl;

  // the default WMTS service URL.
  const Setting sWmtsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WMTS_SERVICE_URL,
                                    QgsServerSettingsEnv::DEFAULT_VALUE,
                                    QStringLiteral( "The default WMTS service URL" ),
                                    QStringLiteral( "/qgis/server_wmts_service_url" ),
                                    QVariant::String,
                                    QVariant( "" ),
                                    QVariant()
                                  };
  mSettings[ sServiceUrl.envVar ] = sWmtsServiceUrl;

  // the default config cache check interval
  const Setting sConfigCacheCheckInterval = { QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_CHECK_INTERVAL,
                                              QgsServerSettingsEnv::DEFAULT_VALUE,
                                              QStringLiteral( "The default project cache check interval" ),
                                              QStringLiteral( "/qgis/server_project_cache_check_interval" ),
                                              QVariant::Int,
                                              QVariant( 0 ),
                                              QVariant()
                                            };
  mSettings[ sConfigCacheCheckInterval.envVar ] = sConfigCacheCheckInterval;

  // the default config cache strategy
  const Setting sProjectCacheStrategy = { QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_STRATEGY,
                                          QgsServerSettingsEnv::DEFAULT_VALUE,
                                          QStringLiteral( "Project's cache strategy. Possible values are 'off','filesystem' or 'periodic'" ),
                                          QStringLiteral( "/qgis/server_project_cache_strategy" ),
                                          QVariant::String,
                                          QVariant( "" ),
                                          QVariant()
                                        };
  mSettings[ sProjectCacheStrategy.envVar ] = sProjectCacheStrategy;

}

void QgsServerSettings::load()
{
  // init settings each time to take into account QgsApplication and
  // QCoreApplication configuration for some default values
  initSettings();

  // store environment variables
  QMap<QgsServerSettingsEnv::EnvVar, QString> env = getEnv();

  // load QSettings if QGIS_OPTIONS_PATH is defined
  loadQSettings( env[ QgsServerSettingsEnv::QGIS_OPTIONS_PATH ] );

  // prioritize values: 'env var' -> 'ini file' -> 'default value'
  prioritize( env );
}

bool QgsServerSettings::load( const QString &envVarName )
{
  bool rc( false );
  const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerSettingsEnv::EnvVar>() );
  const int value = metaEnum.keyToValue( envVarName.toStdString().c_str() );

  if ( value >= 0 )
  {
    const QString envValue( getenv( envVarName.toStdString().c_str() ) );
    prioritize( QMap<QgsServerSettingsEnv::EnvVar, QString> { {( QgsServerSettingsEnv::EnvVar ) value, envValue } } );
    rc = true;
  }

  return rc;
}

QMap<QgsServerSettingsEnv::EnvVar, QString> QgsServerSettings::getEnv() const
{
  QMap<QgsServerSettingsEnv::EnvVar, QString> env;

  const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerSettingsEnv::EnvVar>() );
  for ( int i = 0; i < metaEnum.keyCount(); i++ )
  {
    env[( QgsServerSettingsEnv::EnvVar ) metaEnum.value( i )] = getenv( metaEnum.key( i ) );
  }

  return env;
}

QVariant QgsServerSettings::value( QgsServerSettingsEnv::EnvVar envVar, bool actual ) const
{
  if ( actual )
  {
    const QString envValue( getenv( name( envVar ).toStdString().c_str() ) );

    if ( ! envValue.isEmpty() )
      return envValue;
  }

  if ( mSettings[ envVar ].src == QgsServerSettingsEnv::DEFAULT_VALUE )
  {
    return mSettings[ envVar ].defaultVal;
  }
  else
  {
    return mSettings[ envVar ].val;
  }
}

void QgsServerSettings::loadQSettings( const QString &envOptPath ) const
{
  if ( ! envOptPath.isEmpty() )
  {
    QSettings::setDefaultFormat( QSettings::IniFormat );
    QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, envOptPath );
  }
}

void QgsServerSettings::prioritize( const QMap<QgsServerSettingsEnv::EnvVar, QString> &env )
{
  const auto constKeys( env.keys() );
  for ( const QgsServerSettingsEnv::EnvVar &e : constKeys )
  {
    Setting s = mSettings[ e ];

    QVariant varValue;
    if ( ! env.value( e ).isEmpty() )
    {
      varValue.setValue( env.value( e ) );
    }

    if ( ! varValue.isNull() && varValue.canConvert( s.type ) )
    {
      s.val = varValue;
      s.src = QgsServerSettingsEnv::ENVIRONMENT_VARIABLE;
    }
    else if ( ! s.iniKey.isEmpty() && QSettings().contains( s.iniKey ) && QSettings().value( s.iniKey ).canConvert( s.type ) )
    {
      s.val = QSettings().value( s.iniKey );
      s.src = QgsServerSettingsEnv::INI_FILE;
    }
    else
    {
      s.val = QVariant();
      s.src  = QgsServerSettingsEnv::DEFAULT_VALUE;
    }

    // an empty string can be returned from QSettings. In this case, we want
    // to use the default value
    if ( s.type == QVariant::String && s.val.toString().isEmpty() )
    {
      s.val = QVariant();
      s.src  = QgsServerSettingsEnv::DEFAULT_VALUE;
    }

    mSettings[ e ] = s;
  }
}

QString QgsServerSettings::name( QgsServerSettingsEnv::EnvVar env )
{
  const QMetaEnum metaEnumEnv( QMetaEnum::fromType<QgsServerSettingsEnv::EnvVar>() );
  return metaEnumEnv.valueToKey( env );
}

void QgsServerSettings::logSummary() const
{
  const QMetaEnum metaEnumSrc( QMetaEnum::fromType<QgsServerSettingsEnv::Source>() );

  QgsMessageLog::logMessage( "QGIS Server Settings: ", "Server", Qgis::MessageLevel::Info );
  for ( const Setting &s : std::as_const( mSettings ) )
  {
    const QString src = metaEnumSrc.valueToKey( s.src );
    const QString var = name( s.envVar );

    const QString msg = "  - " + var + " / '" + s.iniKey + "' (" + s.descr + "): '" + value( s.envVar ).toString() + "' (read from " + src + ")";
    QgsMessageLog::logMessage( msg, "Server", Qgis::MessageLevel::Info );
  }

  if ( ! iniFile().isEmpty() )
  {
    const QString msg = "Ini file used to initialize settings: " + iniFile();
    QgsMessageLog::logMessage( msg, "Server", Qgis::MessageLevel::Info );
  }
}

// getter
QString QgsServerSettings::iniFile() const
{
  return QSettings().fileName();
}

bool QgsServerSettings::parallelRendering() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_PARALLEL_RENDERING ).toBool();
}

int QgsServerSettings::maxThreads() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_MAX_THREADS ).toInt();
}

QString QgsServerSettings::logFile() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LOG_FILE ).toString();
}

bool QgsServerSettings::logStderr() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LOG_STDERR ).toBool();
}

Qgis::MessageLevel QgsServerSettings::logLevel() const
{
  return static_cast<Qgis::MessageLevel>( value( QgsServerSettingsEnv::QGIS_SERVER_LOG_LEVEL ).toInt() );
}

QString QgsServerSettings::projectFile() const
{
  return value( QgsServerSettingsEnv::QGIS_PROJECT_FILE ).toString();
}

qint64 QgsServerSettings::cacheSize() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_CACHE_SIZE ).toLongLong();
}

QString QgsServerSettings::cacheDirectory() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_CACHE_DIRECTORY ).toString();
}

QString QgsServerSettings::overrideSystemLocale() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_OVERRIDE_SYSTEM_LOCALE ).toString();
}

bool QgsServerSettings::showGroupSeparator() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_SHOW_GROUP_SEPARATOR ).toBool();
}

int QgsServerSettings::wmsMaxHeight() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_HEIGHT ).toInt();
}

int QgsServerSettings::wmsMaxWidth() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_WIDTH ).toInt();
}

QString QgsServerSettings::landingPageProjectsDirectories() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES, true ).toString();
}

QString QgsServerSettings::landingPageProjectsPgConnections() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS, true ).toString();
}

QString QgsServerSettings::landingPageBaseUrlPrefix() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PREFIX, true ).toString();
}

QString QgsServerSettings::apiResourcesDirectory() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_API_RESOURCES_DIRECTORY ).toString();
}

qlonglong QgsServerSettings::apiWfs3MaxLimit() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_MAX_LIMIT ).toLongLong();
}

bool QgsServerSettings::ignoreBadLayers() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_IGNORE_BAD_LAYERS ).toBool();
}

bool QgsServerSettings::trustLayerMetadata() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_TRUST_LAYER_METADATA ).toBool();
}

bool QgsServerSettings::getPrintDisabled() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_DISABLE_GETPRINT ).toBool();
}

bool QgsServerSettings::logProfile() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_LOG_PROFILE, false ).toBool();
}

QString QgsServerSettings::serviceUrl( const QString &service ) const
{
  QString result;
  if ( service.compare( QLatin1String( "WMS" ) ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WMS_SERVICE_URL ).toString();
  }
  else if ( service.compare( QLatin1String( "WFS" ) ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WFS_SERVICE_URL ).toString();
  }
  else if ( service.compare( QLatin1String( "WCS" ) ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WCS_SERVICE_URL ).toString();
  }
  else if ( service.compare( QLatin1String( "WMTS" ) ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WMTS_SERVICE_URL ).toString();
  }

  if ( result.isEmpty() )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_SERVICE_URL ).toString();
  }

  return result;
}

int QgsServerSettings::projectCacheCheckInterval() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_CHECK_INTERVAL ).toInt();
}

QString QgsServerSettings::projectCacheStrategy() const
{
  QString result = value( QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_STRATEGY ).toString();
  if ( result.compare( QLatin1String( "filesystem" ) ) &&
       result.compare( QLatin1String( "periodic" ) ) &&
       result.compare( QLatin1String( "off" ) ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Invalid cache strategy, expecting 'filesystem', 'periodic' or 'off'. Using 'filesystem' as default." ), "Server", Qgis::MessageLevel::Warning );
    result = QStringLiteral( "filesystem" );
  }
  return result;
}

