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
#include "qgsvariantutils.h"

#include <QDir>
#include <QSettings>

#include "moc_qgsserversettings.cpp"

QgsServerSettings::QgsServerSettings()
{
  load();
}

void QgsServerSettings::initSettings()
{
  mSettings.clear();

  // options path
  const Setting sOptPath = { QgsServerSettingsEnv::QGIS_OPTIONS_PATH, QgsServerSettingsEnv::DEFAULT_VALUE, u"Override the default path for user configuration"_s, QString(), QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sOptPath.envVar] = sOptPath;

  // parallel rendering
  const Setting sParRend = { QgsServerSettingsEnv::QGIS_SERVER_PARALLEL_RENDERING, QgsServerSettingsEnv::DEFAULT_VALUE, u"Activate/Deactivate parallel rendering for WMS getMap request"_s, u"/qgis/parallel_rendering"_s, QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sParRend.envVar] = sParRend;

  // max threads
  const Setting sMaxThreads = { QgsServerSettingsEnv::QGIS_SERVER_MAX_THREADS, QgsServerSettingsEnv::DEFAULT_VALUE, u"Number of threads to use when parallel rendering is activated"_s, u"/qgis/max_threads"_s, QMetaType::Type::Int, QVariant( -1 ), QVariant() };
  mSettings[sMaxThreads.envVar] = sMaxThreads;

  // log level
  const Setting sLogLevel = { QgsServerSettingsEnv::QGIS_SERVER_LOG_LEVEL, QgsServerSettingsEnv::DEFAULT_VALUE, u"Log level"_s, QString(), QMetaType::Type::Int, QVariant::fromValue( Qgis::MessageLevel::NoLevel ), QVariant() };
  mSettings[sLogLevel.envVar] = sLogLevel;

  // log file
  const Setting sLogFile = { QgsServerSettingsEnv::QGIS_SERVER_LOG_FILE, QgsServerSettingsEnv::DEFAULT_VALUE, u"Log file"_s, QString(), QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sLogFile.envVar] = sLogFile;

  // log to stderr
  const Setting sLogStderr = { QgsServerSettingsEnv::QGIS_SERVER_LOG_STDERR, QgsServerSettingsEnv::DEFAULT_VALUE, u"Activate/Deactivate logging to stderr"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sLogStderr.envVar] = sLogStderr;

  // project file
  const Setting sProject = { QgsServerSettingsEnv::QGIS_PROJECT_FILE, QgsServerSettingsEnv::DEFAULT_VALUE, u"QGIS project file"_s, QString(), QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sProject.envVar] = sProject;

  // cache directory
  const Setting sCacheDir = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_DIRECTORY, QgsServerSettingsEnv::DEFAULT_VALUE, u"Specify the cache directory"_s, u"/cache/directory"_s, QMetaType::Type::QString, QVariant( QgsApplication::qgisSettingsDirPath() + "cache" ), QVariant() };
  mSettings[sCacheDir.envVar] = sCacheDir;

  // cache size
  const Setting sCacheSize = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_SIZE, QgsServerSettingsEnv::DEFAULT_VALUE, u"Specify the cache size (0 = automatic size)"_s, u"/cache/size-bytes"_s, QMetaType::Type::LongLong, 0, QVariant() };
  mSettings[sCacheSize.envVar] = sCacheSize;

  // system locale override
  const Setting sOverrideSystemLocale = { QgsServerSettingsEnv::QGIS_SERVER_OVERRIDE_SYSTEM_LOCALE, QgsServerSettingsEnv::DEFAULT_VALUE, u"Override system locale"_s, u"/locale/userLocale"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sOverrideSystemLocale.envVar] = sOverrideSystemLocale;

  // bad layers handling
  const Setting sIgnoreBadLayers = { QgsServerSettingsEnv::QGIS_SERVER_IGNORE_BAD_LAYERS, QgsServerSettingsEnv::DEFAULT_VALUE, u"Ignore bad layers"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sIgnoreBadLayers.envVar] = sIgnoreBadLayers;

  const Setting sIgnoreRenderingErrors = { QgsServerSettingsEnv::QGIS_SERVER_IGNORE_RENDERING_ERRORS, QgsServerSettingsEnv::DEFAULT_VALUE, u"Ignore rendering errors"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sIgnoreRenderingErrors.envVar] = sIgnoreRenderingErrors;

  // retry bad layers
  const Setting sRetryBadLayers = { QgsServerSettingsEnv::QGIS_SERVER_RETRY_BAD_LAYERS, QgsServerSettingsEnv::DEFAULT_VALUE, u"Retry bad layers"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sRetryBadLayers.envVar] = sRetryBadLayers;

  // trust layer metadata
  const Setting sTrustLayerMetadata = { QgsServerSettingsEnv::QGIS_SERVER_TRUST_LAYER_METADATA, QgsServerSettingsEnv::DEFAULT_VALUE, u"Trust layer metadata"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sTrustLayerMetadata.envVar] = sTrustLayerMetadata;


  // force to open layers in read-only mode
  const Setting sForceReadOnlyLayers = { QgsServerSettingsEnv::QGIS_SERVER_FORCE_READONLY_LAYERS, QgsServerSettingsEnv::DEFAULT_VALUE, u"Force to open layers in read-only mode"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sForceReadOnlyLayers.envVar] = sForceReadOnlyLayers;

  // don't load layouts
  const Setting sDontLoadLayouts = { QgsServerSettingsEnv::QGIS_SERVER_DISABLE_GETPRINT, QgsServerSettingsEnv::DEFAULT_VALUE, u"Don't load layouts"_s, QString(), QMetaType::Type::Bool, QVariant( false ), QVariant() };
  mSettings[sDontLoadLayouts.envVar] = sDontLoadLayouts;

  // show group separator
  const Setting sShowGroupSeparator = { QgsServerSettingsEnv::QGIS_SERVER_SHOW_GROUP_SEPARATOR, QgsServerSettingsEnv::DEFAULT_VALUE, u"Show group (thousands) separator"_s, u"/locale/showGroupSeparator"_s, QMetaType::Type::QString, QVariant( false ), QVariant() };
  mSettings[sShowGroupSeparator.envVar] = sShowGroupSeparator;

  // max height
  const Setting sMaxHeight = { QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_HEIGHT, QgsServerSettingsEnv::DEFAULT_VALUE, u"Maximum height for a WMS request. The lower one of this and the project configuration is used."_s, u"/qgis/max_wms_height"_s, QMetaType::Type::LongLong, QVariant( -1 ), QVariant() };
  mSettings[sMaxHeight.envVar] = sMaxHeight;

  // max width
  const Setting sMaxWidth = { QgsServerSettingsEnv::QGIS_SERVER_WMS_MAX_WIDTH, QgsServerSettingsEnv::DEFAULT_VALUE, u"Maximum width for a WMS request. The most conservative between this and the project one is used"_s, u"/qgis/max_wms_width"_s, QMetaType::Type::LongLong, QVariant( -1 ), QVariant() };
  mSettings[sMaxWidth.envVar] = sMaxWidth;

  // API templates and static override directory
  const Setting sApiResourcesDirectory = { QgsServerSettingsEnv::QGIS_SERVER_API_RESOURCES_DIRECTORY, QgsServerSettingsEnv::DEFAULT_VALUE, u"Base directory where HTML templates and static assets (e.g. images, js and css files) are searched for"_s, u"/qgis/server_api_resources_directory"_s, QMetaType::Type::QString, QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( u"resources/server/api"_s ), QString() };

  mSettings[sApiResourcesDirectory.envVar] = sApiResourcesDirectory;

  // API WFS3 max limit
  const Setting sApiWfs3MaxLimit = { QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_MAX_LIMIT, QgsServerSettingsEnv::DEFAULT_VALUE, u"Maximum value for \"limit\" in a features request, defaults to 10000"_s, u"/qgis/server_api_wfs3_max_limit"_s, QMetaType::Type::LongLong, QVariant( 10000 ), QVariant() };

  mSettings[sApiWfs3MaxLimit.envVar] = sApiWfs3MaxLimit;

  // API WFS3 root path
  // TODO: remove when QGIS 4 is released
#if _QGIS_VERSION_INT > 40000
  const Setting sApiWfs3RootPath = { QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_ROOT_PATH, QgsServerSettingsEnv::DEFAULT_VALUE, u"Root path for the OAPIF (WFS3) API"_s, u"/qgis/server_api_wfs3_root_path"_s, QMetaType::Type::QString, QVariant( "/ogcapi" ), QVariant() };
#else
  const Setting sApiWfs3RootPath = { QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_ROOT_PATH, QgsServerSettingsEnv::DEFAULT_VALUE, u"Root path for the OAPIF (WFS3) API"_s, u"/qgis/server_api_wfs3_root_path"_s, QMetaType::Type::QString, QVariant( "/wfs3" ), QVariant() };
#endif

  mSettings[sApiWfs3RootPath.envVar] = sApiWfs3RootPath;

  // projects directory for landing page service
  const Setting sProjectsDirectories = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES, QgsServerSettingsEnv::DEFAULT_VALUE, u"Directories used by the landing page service to find .qgs and .qgz projects"_s, u"/qgis/server_projects_directories"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };

  mSettings[sProjectsDirectories.envVar] = sProjectsDirectories;

  // postgresql connection string for landing page service
  const Setting sProjectsPgConnections = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS, QgsServerSettingsEnv::DEFAULT_VALUE, u"PostgreSQL connection strings used by the landing page service to find projects"_s, u"/qgis/server_projects_pg_connections"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };

  mSettings[sProjectsPgConnections.envVar] = sProjectsPgConnections;

  // landing page base URL prefix
  const Setting sLandingPageBaseUrlPrefix = { QgsServerSettingsEnv::QGIS_SERVER_LANDING_PAGE_PREFIX, QgsServerSettingsEnv::DEFAULT_VALUE, u"Landing page base URL path prefix"_s, u"/qgis/server_landing_page_base_url_prefix"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };

  mSettings[sLandingPageBaseUrlPrefix.envVar] = sLandingPageBaseUrlPrefix;

  // log profile
  const Setting sLogProfile = { QgsServerSettingsEnv::QGIS_SERVER_LOG_PROFILE, QgsServerSettingsEnv::DEFAULT_VALUE, u"Add detailed profile information to the logs, only effective when QGIS_SERVER_LOG_LEVEL=0"_s, u"/qgis/server_log_profile"_s, QMetaType::Type::Bool, QVariant( false ), QVariant() };

  mSettings[sLogProfile.envVar] = sLogProfile;

  // the default service URL.
  const Setting sServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_SERVICE_URL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default service URL"_s, u"/qgis/server_service_url"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sServiceUrl.envVar] = sServiceUrl;

  // the default WMS service URL.
  const Setting sWmsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WMS_SERVICE_URL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default WMS service URL"_s, u"/qgis/server_wms_service_url"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sServiceUrl.envVar] = sWmsServiceUrl;

  // the default WFS service URL.
  const Setting sWfsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WFS_SERVICE_URL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default WFS service URL"_s, u"/qgis/server_wfs_service_url"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sServiceUrl.envVar] = sWfsServiceUrl;

  // the default WCS service URL.
  const Setting sWcsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WCS_SERVICE_URL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default WcS service URL"_s, u"/qgis/server_wcs_service_url"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sServiceUrl.envVar] = sWfsServiceUrl;

  // the default WMTS service URL.
  const Setting sWmtsServiceUrl = { QgsServerSettingsEnv::QGIS_SERVER_WMTS_SERVICE_URL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default WMTS service URL"_s, u"/qgis/server_wmts_service_url"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sServiceUrl.envVar] = sWmtsServiceUrl;

  // the default config cache check interval
  const Setting sConfigCacheCheckInterval = { QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_CHECK_INTERVAL, QgsServerSettingsEnv::DEFAULT_VALUE, u"The default project cache check interval (in ms)"_s, u"/qgis/server_project_cache_check_interval"_s, QMetaType::Type::Int, QVariant( 0 ), QVariant() };
  mSettings[sConfigCacheCheckInterval.envVar] = sConfigCacheCheckInterval;

  // the default config cache strategy
  const Setting sProjectCacheStrategy = { QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_STRATEGY, QgsServerSettingsEnv::DEFAULT_VALUE, u"Project's cache strategy. Possible values are 'off','filesystem' or 'periodic'"_s, u"/qgis/server_project_cache_strategy"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sProjectCacheStrategy.envVar] = sProjectCacheStrategy;

  // the default config cache size
  const Setting sProjectCacheSize = { QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_SIZE, QgsServerSettingsEnv::DEFAULT_VALUE, u"Project's cache size, in number of projects."_s, u"/qgis/server_project_cache_size"_s, QMetaType::Type::QString, QVariant( 100 ), QVariant() };
  mSettings[sProjectCacheSize.envVar] = sProjectCacheSize;

  const Setting sAllowedExtraSqlTokens = { QgsServerSettingsEnv::QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS, QgsServerSettingsEnv::DEFAULT_VALUE, u"List of comma separated SQL tokens to be added to the list of allowed tokens that the services accepts when filtering features"_s, u"/qgis/server_allowed_extra_sql_tokens"_s, QMetaType::Type::QString, QVariant( "" ), QVariant() };
  mSettings[sAllowedExtraSqlTokens.envVar] = sAllowedExtraSqlTokens;

  const Setting sApplicationName = { QgsServerSettingsEnv::QGIS_SERVER_APPLICATION_NAME, QgsServerSettingsEnv::DEFAULT_VALUE, u"The QGIS Server application name"_s, u"/qgis/application_full_name"_s, QMetaType::Type::QString, QVariant( QgsApplication::applicationFullName() ), QVariant() };
  mSettings[sApplicationName.envVar] = sApplicationName;

  const Setting sCapabilitiesCacheSize = { QgsServerSettingsEnv::QGIS_SERVER_CAPABILITIES_CACHE_SIZE, QgsServerSettingsEnv::DEFAULT_VALUE, u"The QGIS Server capabilities cache size"_s, u"/qgis/capabilities_cache_size"_s, QMetaType::Type::Int, QVariant( 40 ), QVariant() };
  mSettings[sCapabilitiesCacheSize.envVar] = sCapabilitiesCacheSize;
}

void QgsServerSettings::load()
{
  // init settings each time to take into account QgsApplication and
  // QCoreApplication configuration for some default values
  initSettings();

  // store environment variables
  QMap<QgsServerSettingsEnv::EnvVar, QString> env = getEnv();

  // load QSettings if QGIS_OPTIONS_PATH is defined
  loadQSettings( env[QgsServerSettingsEnv::QGIS_OPTIONS_PATH] );

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
    prioritize( QMap<QgsServerSettingsEnv::EnvVar, QString> { { ( QgsServerSettingsEnv::EnvVar ) value, envValue } } );
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

    if ( !envValue.isEmpty() )
      return envValue;
  }

  if ( mSettings[envVar].src == QgsServerSettingsEnv::DEFAULT_VALUE )
  {
    return mSettings[envVar].defaultVal;
  }
  else
  {
    return mSettings[envVar].val;
  }
}

void QgsServerSettings::loadQSettings( const QString &envOptPath ) const
{
  if ( !envOptPath.isEmpty() )
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
    Setting s = mSettings[e];

    QVariant varValue;
    if ( !env.value( e ).isEmpty() )
    {
      varValue.setValue( env.value( e ) );
    }

    if ( !QgsVariantUtils::isNull( varValue ) && varValue.canConvert( s.type ) )
    {
      s.val = varValue;
      s.src = QgsServerSettingsEnv::ENVIRONMENT_VARIABLE;
    }
    else if ( !s.iniKey.isEmpty() && QSettings().contains( s.iniKey ) && QSettings().value( s.iniKey ).canConvert( s.type ) )
    {
      s.val = QSettings().value( s.iniKey );
      s.src = QgsServerSettingsEnv::INI_FILE;
    }
    else
    {
      s.val = QVariant();
      s.src = QgsServerSettingsEnv::DEFAULT_VALUE;
    }

    // an empty string can be returned from QSettings. In this case, we want
    // to use the default value
    if ( s.type == QMetaType::Type::QString && s.val.toString().isEmpty() )
    {
      s.val = QVariant();
      s.src = QgsServerSettingsEnv::DEFAULT_VALUE;
    }

    mSettings[e] = s;
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

  if ( !iniFile().isEmpty() )
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

QString QgsServerSettings::apiWfs3RootPath() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_ROOT_PATH ).toString();
}

bool QgsServerSettings::ignoreBadLayers() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_IGNORE_BAD_LAYERS ).toBool();
}

bool QgsServerSettings::ignoreRenderingErrors() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_IGNORE_RENDERING_ERRORS ).toBool();
}

bool QgsServerSettings::retryBadLayers() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_RETRY_BAD_LAYERS ).toBool();
}

bool QgsServerSettings::trustLayerMetadata() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_TRUST_LAYER_METADATA ).toBool();
}

bool QgsServerSettings::forceReadOnlyLayers() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_FORCE_READONLY_LAYERS ).toBool();
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
  if ( service.compare( "WMS"_L1 ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WMS_SERVICE_URL ).toString();
  }
  else if ( service.compare( "WFS"_L1 ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WFS_SERVICE_URL ).toString();
  }
  else if ( service.compare( "WCS"_L1 ) )
  {
    result = value( QgsServerSettingsEnv::QGIS_SERVER_WCS_SERVICE_URL ).toString();
  }
  else if ( service.compare( "WMTS"_L1 ) )
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
  if ( result.compare( "filesystem"_L1 ) && result.compare( "periodic"_L1 ) && result.compare( "off"_L1 ) )
  {
    if ( !result.isEmpty() )
    {
      QgsMessageLog::logMessage( u"Invalid cache strategy '%1', expecting 'filesystem', 'periodic' or 'off'. Using 'filesystem' as default."_s.arg( result ), "Server", Qgis::MessageLevel::Warning );
    }
    else
    {
      QgsMessageLog::logMessage( u"No cache strategy was specified. Using 'filesystem' as default."_s, "Server", Qgis::MessageLevel::Info );
    }
    result = u"filesystem"_s;
  }
  return result;
}

int QgsServerSettings::projectCacheSize() const
{
  bool ok;
  int size = value( QgsServerSettingsEnv::QGIS_SERVER_PROJECT_CACHE_SIZE ).toInt( &ok );
  if ( ok && size >= 1 )
    return size;

  QgsMessageLog::logMessage( u"Invalid project cache size, expecting integer - defaulting to 100"_s, "Server", Qgis::MessageLevel::Warning );
  return 100;
}

QStringList QgsServerSettings::allowedExtraSqlTokens() const
{
  const QString strVal { value( QgsServerSettingsEnv::QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS ).toString().trimmed() };
  if ( strVal.isEmpty() )
  {
    return QStringList();
  }
  return strVal.split( ',' );
}

QString QgsServerSettings::applicationName() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_APPLICATION_NAME ).toString().trimmed();
}

int QgsServerSettings::capabilitiesCacheSize() const
{
  bool ok;
  int size = value( QgsServerSettingsEnv::QGIS_SERVER_CAPABILITIES_CACHE_SIZE ).toInt( &ok );
  if ( ok && size >= 1 )
    return size;

  QgsMessageLog::logMessage( u"Invalid capabilities cache size, expecting integer - defaulting to 40"_s, "Server", Qgis::MessageLevel::Warning );
  return 40;
}
