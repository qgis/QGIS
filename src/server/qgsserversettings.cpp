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

#include <iostream>

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
                             "Override the default path for user configuration",
                             "",
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sOptPath.envVar ] = sOptPath;

  // parallel rendering
  const Setting sParRend = { QgsServerSettingsEnv::QGIS_SERVER_PARALLEL_RENDERING,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             "Activate/Deactivate parallel rendering for WMS getMap request",
                             "/qgis/parallel_rendering",
                             QVariant::Bool,
                             QVariant( false ),
                             QVariant()
                           };
  mSettings[ sParRend.envVar ] = sParRend;

  // max threads
  const Setting sMaxThreads = { QgsServerSettingsEnv::QGIS_SERVER_MAX_THREADS,
                                QgsServerSettingsEnv::DEFAULT_VALUE,
                                "Number of threads to use when parallel rendering is activated",
                                "/qgis/max_threads",
                                QVariant::Int,
                                QVariant( -1 ),
                                QVariant()
                              };
  mSettings[ sMaxThreads.envVar ] = sMaxThreads;

  // log level
  const Setting sLogLevel = { QgsServerSettingsEnv::QGIS_SERVER_LOG_LEVEL,
                              QgsServerSettingsEnv::DEFAULT_VALUE,
                              "Log level",
                              "",
                              QVariant::Int,
                              QVariant( Qgis::None ),
                              QVariant()
                            };
  mSettings[ sLogLevel.envVar ] = sLogLevel;

  // log file
  const Setting sLogFile = { QgsServerSettingsEnv::QGIS_SERVER_LOG_FILE,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             "Log file",
                             "",
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sLogFile.envVar ] = sLogFile;

  // log to stderr
  const Setting sLogStderr = { QgsServerSettingsEnv::QGIS_SERVER_LOG_STDERR,
                               QgsServerSettingsEnv::DEFAULT_VALUE,
                               "Activate/Deactivate logging to stderr",
                               "",
                               QVariant::Bool,
                               QVariant( false ),
                               QVariant()
                             };
  mSettings[ sLogStderr.envVar ] = sLogStderr;

  // project file
  const Setting sProject = { QgsServerSettingsEnv::QGIS_PROJECT_FILE,
                             QgsServerSettingsEnv::DEFAULT_VALUE,
                             "QGIS project file",
                             "",
                             QVariant::String,
                             QVariant( "" ),
                             QVariant()
                           };
  mSettings[ sProject.envVar ] = sProject;

  // max cache layers
  const Setting sMaxCacheLayers = { QgsServerSettingsEnv::MAX_CACHE_LAYERS,
                                    QgsServerSettingsEnv::DEFAULT_VALUE,
                                    "Specify the maximum number of cached layers",
                                    "",
                                    QVariant::Int,
                                    QVariant( 100 ),
                                    QVariant()
                                  };
  mSettings[ sMaxCacheLayers.envVar ] = sMaxCacheLayers;

  // cache directory
  const Setting sCacheDir = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_DIRECTORY,
                              QgsServerSettingsEnv::DEFAULT_VALUE,
                              "Specify the cache directory",
                              "/cache/directory",
                              QVariant::String,
                              QVariant( QgsApplication::qgisSettingsDirPath() + "cache" ),
                              QVariant()
                            };
  mSettings[ sCacheDir.envVar ] = sCacheDir;

  // cache size
  const Setting sCacheSize = { QgsServerSettingsEnv::QGIS_SERVER_CACHE_SIZE,
                               QgsServerSettingsEnv::DEFAULT_VALUE,
                               "Specify the cache size",
                               "/cache/size",
                               QVariant::LongLong,
                               QVariant( 50 * 1024 * 1024 ),
                               QVariant()
                             };
  mSettings[ sCacheSize.envVar ] = sCacheSize;
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
