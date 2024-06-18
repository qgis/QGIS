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
 */
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
      QGIS_SERVER_IGNORE_BAD_LAYERS, //!< Do not consider the whole project unavailable if it contains bad layers
      QGIS_SERVER_CACHE_DIRECTORY,
      QGIS_SERVER_CACHE_SIZE,
      QGIS_SERVER_SHOW_GROUP_SEPARATOR,  //!< Show group (thousands) separator when formatting numeric values, defaults to FALSE (since QGIS 3.8)
      QGIS_SERVER_OVERRIDE_SYSTEM_LOCALE,  //!< Override system locale (since QGIS 3.8)
      QGIS_SERVER_WMS_MAX_HEIGHT, //!< Maximum height for a WMS request. The most conservative between this and the project one is used (since QGIS 3.6.2)
      QGIS_SERVER_WMS_MAX_WIDTH, //!< Maximum width for a WMS request. The most conservative between this and the project one is used (since QGIS 3.6.2)
      QGIS_SERVER_API_RESOURCES_DIRECTORY, //!< Base directory where HTML templates and static assets (e.g. images, js and css files) are searched for (since QGIS 3.10).
      QGIS_SERVER_API_WFS3_MAX_LIMIT, //!< Maximum value for "limit" in a features request, defaults to 10000 (since QGIS 3.10).
      QGIS_SERVER_TRUST_LAYER_METADATA, //!< Trust layer metadata. Improves project read time. (since QGIS 3.16).
      QGIS_SERVER_FORCE_READONLY_LAYERS, //!< Force to open layers in read-only mode. (since QGIS 3.28).
      QGIS_SERVER_DISABLE_GETPRINT, //!< Disabled WMS GetPrint request and don't load layouts. Improves project read time. (since QGIS 3.16).
      QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES, //!< Directories used by the landing page service to find .qgs and .qgz projects (since QGIS 3.16)
      QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS, //!< PostgreSQL connection strings used by the landing page service to find projects (since QGIS 3.16)
      QGIS_SERVER_LOG_PROFILE, //!< When QGIS_SERVER_LOG_LEVEL is 0 this flag adds to the logs detailed information about the time taken by the different processing steps inside the QGIS Server request (since QGIS 3.16)
      QGIS_SERVER_SERVICE_URL, //!< To set the service URL if it's not present in the project. (since QGIS 3.20).
      QGIS_SERVER_WMS_SERVICE_URL, //!< To set the WMS service URL if it's not present in the project. (since QGIS 3.20).
      QGIS_SERVER_WFS_SERVICE_URL, //!< To set the WFS service URL if it's not present in the project. (since QGIS 3.20).
      QGIS_SERVER_WCS_SERVICE_URL, //!< To set the WCS service URL if it's not present in the project. (since QGIS 3.20).
      QGIS_SERVER_WMTS_SERVICE_URL, //!< To set the WMTS service URL if it's not present in the project. (since QGIS 3.20).
      QGIS_SERVER_LANDING_PAGE_PREFIX, //! Prefix of the path component of the landing page base URL, default is empty (since QGIS 3.20).
      QGIS_SERVER_PROJECT_CACHE_CHECK_INTERVAL, //! Set the interval in milliseconds for cache invalidation strategy 'interval', default to 0 which select the legacy File system watcher  (since QGIS 3.26).
      QGIS_SERVER_PROJECT_CACHE_STRATEGY, //! Set the project cache strategy. Possible values are 'filesystem', 'periodic' or 'off' (since QGIS 3.26).
      QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS, //! Adds these tokens to the list of allowed tokens that the services accept when filtering features (since QGIS 3.28).
      QGIS_SERVER_APPLICATION_NAME, //! Define the QGIS Server application name (since QGIS 3.30).
      QGIS_SERVER_CAPABILITIES_CACHE_SIZE, //! Define the QGIS Server capabilities cache size (since QGIS 3.31)
    };
    Q_ENUM( EnvVar )
};

/**
 * \ingroup server
 * \class QgsServerSettings
 * \brief Provides a way to retrieve settings by prioritizing according to environment variables, ini file and default values.
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
      QMetaType::Type type;
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
     * \returns TRUE if loading is successful, FALSE in case of an invalid name.
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
     * \returns TRUE if parallel rendering is activated, FALSE otherwise.
     */
    bool parallelRendering() const;

    /**
     * Returns the maximum number of threads to use.
     * \returns the number of threads.
     */
    int maxThreads() const;

    /**
     * Returns the log level.
     * \returns the log level.
     */
    Qgis::MessageLevel logLevel() const;

    /**
     * Returns TRUE if profile information has to be added to the logs, default value is FALSE.
     *
     * \note this flag is only effective when logLevel() returns Qgis::MessageLevel::Info (0)
     * \see logLevel()
     * \since QGIS 3.18
     */
    bool logProfile() const;

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
     * \returns TRUE if logging to stderr is activated, FALSE otherwise.
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

    /**
     * Overrides system locale
     * \returns the optional override for system locale.
     * \since QGIS 3.8
     */
    QString overrideSystemLocale() const;

    /**
     * Show group (thousand) separator
     * \returns if group separator must be shown, default to FALSE.
     * \since QGIS 3.8
     */
    bool showGroupSeparator() const;

    /**
     * Returns the server-wide max height of a WMS GetMap request. The lower one of this and the project configuration is used.
     * \returns the max height of a WMS GetMap request.
     * \since QGIS 3.6.2
     */
    int wmsMaxHeight() const;

    /**
     * Returns the server-wide max width of a WMS GetMap request. The lower one of this and the project configuration is used.
     * \returns the max width of a WMS GetMap request.
     * \since QGIS 3.6.2
     */
    int wmsMaxWidth() const;

    /**
     * Returns the directories used by the landing page service to find .qgs
     * and .qgz projects. Multiple directories can be specified by separating
     * them with '||'.
     * \since QGIS 3.16
     */
    QString landingPageProjectsDirectories() const;

    /**
     * Returns the PostgreSQL connection strings used by the landing page
     * service to find projects. Multiple connections can be specified by
     * separating them with '||'.
     * \since QGIS 3.16
     */
    QString landingPageProjectsPgConnections() const;

    /**
     * Returns the landing page base URL regular expression, defaults to `/`.
     * \since QGIS 3.20
     */
    QString landingPageBaseUrlPrefix() const;

    /**
     * Returns the server-wide base directory where HTML templates and static assets (e.g. images, js and css files) are searched for.
     *
     * The default path is calculated by joining QgsApplication::pkgDataPath() with "resources/server/api", this path
     * can be changed by setting the environment variable QGIS_SERVER_API_RESOURCES_DIRECTORY.
     *
     * \since QGIS 3.10
     */
    QString apiResourcesDirectory() const;

    /**
     * Returns the server-wide maximum allowed value for \"limit\" in a features request.
     *
     * The default value is 10000, this value can be changed by setting the environment
     * variable QGIS_SERVER_API_WFS3_MAX_LIMIT.
     *
     * \since QGIS 3.10
     */
    qlonglong apiWfs3MaxLimit() const;

    /**
     * Returns TRUE if the bad layers are ignored and FALSE when the presence of a
     * bad layers invalidates the whole project making it unavailable.
     *
     * The default value is TRUE, this value can be changed by setting the environment
     * variable QGIS_SERVER_IGNORE_BAD_LAYERS.
     *
     * \since QGIS 3.10.5
     */
    bool ignoreBadLayers() const;

    /**
     * Returns TRUE if the reading flag trust layer metadata is activated.
     *
     * The default value is FALSE, this value can be changed by setting the environment
     * variable QGIS_SERVER_TRUST_LAYER_METADATA.
     *
     * \since QGIS 3.16
     */
    bool trustLayerMetadata() const;

    /**
     * Returns TRUE if the reading flag force layer read only is activated.
     *
     * The default value is FALSE, this value can be changed by setting the environment
     * variable QGIS_SERVER_FORCE_READONLY_LAYERS.
     *
     * \since QGIS 3.28
     */
    bool forceReadOnlyLayers() const;

    /**
     * Returns TRUE if WMS GetPrint request is disabled and the project's
     * reading flag QgsProject::ReadFlag::FlagDontLoadLayouts is activated.
     *
     * The default value is FALSE, this value can be changed by setting the environment
     * variable QGIS_SERVER_DISABLE_GETPRINT.
     *
     * \since QGIS 3.16
     */
    bool getPrintDisabled() const;

    /**
     * Returns the service URL from the setting.
     * \since QGIS 3.20
     */
    QString serviceUrl( const QString &service ) const;

    /**
     * Returns the config cache check interval (in ms) for the 'periodic' strategy.
     * \since QGIS 3.26
     */
    int projectCacheCheckInterval() const;

    /**
     * Returns the project's cache strategy
     * The default value is 'filesystem', the value can be changed by setting the environment
     * variable QGIS_SERVER_PROJECT_CACHE_STRATEGY.
     * Possible values are:
     *
     * - 'filesystem': Use file system watcher for notifying projects change. Note that it works
     *   only with projects stored in files and not across mounted NFS volumes on Linux.
     * - 'periodic': Timer based periodic check for project's changes. Works with all storage backend.
     * - 'off': Disable completely internal project's cache handling
     *
     * \since QGIS 3.26
     */
    QString projectCacheStrategy() const;

    /**
     * Returns the list of strings that represent the allowed extra SQL tokens
     * accepted as components of a feature filter.
     * The default value is an empty string, the value can be changed by setting the environment
     * variable QGIS_SERVER_ALLOWED_EXTRA_SQL_TOKENS.
     *
     * \since QGIS 3.28
     */
    QStringList allowedExtraSqlTokens() const;

    /**
     * Returns the QGIS Server application name.
     * The default value is the concatenation of QgsApplication::applicationName()
     * and QgsApplication::platform() separated by a space, the value can be changed
     * by setting the environment variable QGIS_SERVER_APPLICATION_NAME.
     *
     * \since QGIS 3.30
     */
    QString applicationName() const;

    /**
     * Returns the string representation of a setting.
     * \since QGIS 3.16
     */
    static QString name( QgsServerSettingsEnv::EnvVar env );

    /**
     * Returns the maximum number of project capabilities to cache.
     * The default value is 40 and the value can be changed
     * by setting the environment variable QGIS_SERVER_CAPABILITIES_CACHE_SIZE.
     *
     * \since QGIS 3.31
     */
    int capabilitiesCacheSize() const;

  private:
    void initSettings();
    QVariant value( QgsServerSettingsEnv::EnvVar envVar, bool actual = false ) const;
    QMap<QgsServerSettingsEnv::EnvVar, QString> getEnv() const;
    void loadQSettings( const QString &envOptPath ) const;
    void prioritize( const QMap<QgsServerSettingsEnv::EnvVar, QString> &env );

    QMap< QgsServerSettingsEnv::EnvVar, Setting > mSettings;
};

#endif
