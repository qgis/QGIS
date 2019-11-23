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
                              QVariant( Qgis::None ),
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

  // max cache layers
  const Setting sMaxCacheLayers = { QgsServerSettingsEnv::MAX_CACHE_LAYERS,
                                    QgsServerSettingsEnv::DEFAULT_VALUE,
                                    QStringLiteral( "Specify the maximum number of cached layers" ),
                                    QString(),
                                    QVariant::Int,
                                    QVariant( 100 ),
                                    QVariant()
                                  };
  mSettings[ sMaxCacheLayers.envVar ] = sMaxCacheLayers;

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
                               QVariant( 50 * 1024 * 1024 ),
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

QVariant QgsServerSettings::value( QgsServerSettingsEnv::EnvVar envVar ) const
{
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
  for ( QgsServerSettingsEnv::EnvVar e : env.keys() )
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

void QgsServerSettings::logSummary() const
{
  const QMetaEnum metaEnumSrc( QMetaEnum::fromType<QgsServerSettingsEnv::Source>() );
  const QMetaEnum metaEnumEnv( QMetaEnum::fromType<QgsServerSettingsEnv::EnvVar>() );

  QgsMessageLog::logMessage( "Qgis Server Settings: ", "Server", Qgis::Info );
  for ( Setting s : mSettings )
  {
    const QString src = metaEnumSrc.valueToKey( s.src );
    const QString var = metaEnumEnv.valueToKey( s.envVar );

    const QString msg = "  - " + var + " / '" + s.iniKey + "' (" + s.descr + "): '" + value( s.envVar ).toString() + "' (read from " + src + ")";
    QgsMessageLog::logMessage( msg, "Server", Qgis::Info );
  }

  if ( ! iniFile().isEmpty() )
  {
    const QString msg = "Ini file used to initialize settings: " + iniFile();
    QgsMessageLog::logMessage( msg, "Server", Qgis::Info );
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

int QgsServerSettings::maxCacheLayers() const
{
  return value( QgsServerSettingsEnv::MAX_CACHE_LAYERS ).toInt();
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

QString QgsServerSettings::apiResourcesDirectory() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_API_RESOURCES_DIRECTORY ).toString();
}

qlonglong QgsServerSettings::apiWfs3MaxLimit() const
{
  return value( QgsServerSettingsEnv::QGIS_SERVER_API_WFS3_MAX_LIMIT ).toLongLong();
}
