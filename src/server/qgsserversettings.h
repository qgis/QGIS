/***************************************************************************
                    qgsserversettings.h
                    -------------------
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


#ifndef QGSSERVERSETTINGS_H
#define QGSSERVERSETTINGS_H

#include <QObject>
#include <QMetaEnum>

#include "qgsmessagelog.h"
#include "qgis_server.h"
#include "qgis_sip.h"

/**
 * \ingroup server
 * \class QgsServerSettingsEnv
 * \brief Provides some enum describing the environment currently supported for configuration.
 * \since QGIS 3.0
 */
#ifndef SIP_RUN
class SERVER_EXPORT QgsServerSettingsEnv : public QObject
{
    Q_OBJECT

  public:
    //! Source of the parameter used in the configuration
    enum Source
    {
      DEFAULT_VALUE,
      ENVIRONMENT_VARIABLE,
      INI_FILE
    };
    Q_ENUM( Source )

    //! Environment variables to configure the server
    enum EnvVar
    {
      QGIS_OPTIONS_PATH,
      QGIS_SERVER_PARALLEL_RENDERING,
      QGIS_SERVER_MAX_THREADS,
      QGIS_SERVER_LOG_LEVEL,
      QGIS_SERVER_LOG_FILE,
      QGIS_SERVER_LOG_STDERR,
      QGIS_PROJECT_FILE,
      MAX_CACHE_LAYERS,
      QGIS_SERVER_CACHE_DIRECTORY,
      QGIS_SERVER_CACHE_SIZE
    };
    Q_ENUM( EnvVar )
};
#endif

/**
 * \ingroup server
 * \class QgsServerSettings
 * \brief Provides a way to retrieve settings by prioritizing according to environment variables, ini file and default values.
 * \since QGIS 3.0
 */
class SERVER_EXPORT QgsServerSettings
{
  public:
    struct Setting SIP_SKIP
    {
      QgsServerSettingsEnv::EnvVar envVar;
      QgsServerSettingsEnv::Source src;
      QString descr;
      QString iniKey;
      QVariant::Type type;
      QVariant defaultVal;
      QVariant val;
    };

    /**
     * Constructor.
      */
    QgsServerSettings();

    /**
     * Load settings according to current environment variables.
      */
    void load();

    /**
     * Load setting for a specific environment variable name.
      * \returns true if loading is successful, false in case of an invalid name.
      */
    bool load( const QString &envVarName );

    /**
     * Log a summary of settings currently loaded.
      */
    void logSummary() const;

    /**
     * Returns the ini file loaded by QSetting.
      * \returns the path of the ini file or an empty string if none is loaded.
      */
    QString iniFile() const;

    /**
     * Returns parallel rendering setting.
      * \returns true if parallel rendering is activated, false otherwise.
      */
    bool parallelRendering() const;

    /**
     * Returns the maximum number of threads to use.
      * \returns the number of threads.
      */
    int maxThreads() const;

    /**
      * Returns the maximum number of cached layers.
      * \returns the number of cached layers.
      */
    int maxCacheLayers() const;

    /**
     * Returns the log level.
      * \returns the log level.
      */
    Qgis::MessageLevel logLevel() const;

    /**
     * Returns the QGS project file to use.
      * \returns the path of the QGS project or an empty string if none is defined.
      */
    QString projectFile() const;

    /**
     * Returns the log file.
      * \returns the path of the log file or an empty string if none is defined.
      */
    QString logFile() const;

    /**
     * Returns whether logging to stderr is activated.
     * \returns true if logging to stderr is activated, false otherwise.
     * \since QGIS 3.4
     */
    bool logStderr() const;

    /**
     * Returns the cache size.
      * \returns the cache size.
      */
    qint64 cacheSize() const;

    /**
     * Returns the cache directory.
      * \returns the directory.
      */
    QString cacheDirectory() const;

  private:
    void initSettings();
    QVariant value( QgsServerSettingsEnv::EnvVar envVar ) const;
    QMap<QgsServerSettingsEnv::EnvVar, QString> getEnv() const;
    void loadQSettings( const QString &envOptPath ) const;
    void prioritize( const QMap<QgsServerSettingsEnv::EnvVar, QString> &env );

    QMap< QgsServerSettingsEnv::EnvVar, Setting > mSettings;
};

#endif
