/***************************************************************************
    qgspythonutils.h - abstract interface for Python routines
    ---------------------
    begin                : October 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPYTHONUTILS_H
#define QGSPYTHONUTILS_H

// Needed for CMake variables defines
#include "qgsconfig.h"

#include "qgis_python.h"

#include <QString>
#include <QStringList>


class QgisInterface;
#ifdef  HAVE_SERVER_PYTHON_PLUGINS
class QgsServerInterface;
#endif


/**
 * All calls to Python functions in QGIS come here.
 * This class is a singleton.
 *
 * Default path for Python plugins is:
 *
 * - QgsApplication::qgisSettingsDirPath() + "/python/plugins"
 * - QgsApplication::pkgDataPath() + "/python/plugins"
 *
 */
class PYTHON_EXPORT QgsPythonUtils
{
  public:

    virtual ~QgsPythonUtils() = default;

    /**
     * Returns TRUE if Python support is ready to use.
     *
     * Python support must be initialized first, via a call to initPython().
     */
    virtual bool isEnabled() = 0;

    /**
     * Initializes Python and imports the PyQGIS bindings.
     *
     * The \a iface argument should be set to an instance of the QGIS interface, or
     * NULLPTR if no interface is available.
     *
     * If \a installErrorHook is true then the custom QGIS GUI error hook will be used.
     *
     * Since QGIS 3.24, the \a faultHandlerLogPath argument can be used to specify a file path
     * for Python's faulthandler to dump tracebacks in if Python code causes QGIS to crash.
     */
    virtual void initPython( QgisInterface *iface, bool installErrorHook, const QString &faultHandlerLogPath = QString() ) = 0;

#ifdef HAVE_SERVER_PYTHON_PLUGINS

    /**
     * Initializes Python and imports server bindings.
     */
    virtual void initServerPython( QgsServerInterface *iface ) = 0;

    /**
     * Starts a server plugin.
     *
     * Calls the plugin's classServerFactory(serverInterface) and adds the matching plugin to the
     * active plugins list.
     */
    virtual bool startServerPlugin( QString packageName ) = 0;
#endif

    /**
     * Gracefully closes the Python interpreter and cleans up Python library handles.
     */
    virtual void exitPython() = 0;

    /**
     * Runs a Python \a command, showing an error message if one occurred.
     * \returns TRUE if no error occurred
     */
    virtual bool runString( const QString &command, QString msgOnError = QString(), bool single = true ) = 0;

    /**
     * Runs a Python \a command.
     * \returns empty QString if no error occurred, or Python traceback as a QString on error.
     */
    virtual QString runStringUnsafe( const QString &command, bool single = true ) = 0;

    /**
     * Evaluates a Python \a command and stores the result in a the \a result string.
     */
    virtual bool evalString( const QString &command, QString &result ) = 0;

    /**
     * Gets information about error to the supplied arguments
     * \returns FALSE if there was no Python error
     */
    virtual bool getError( QString &errorClassName, QString &errorText ) = 0;

    /* plugins */

    /**
     * Returns a list of all available Python plugins.
     *
     * \see listActivePlugins()
     */
    virtual QStringList pluginList() = 0;

    /**
     * Returns TRUE if the plugin with matching name is loaded (active).
     *
     * \see isPluginEnabled()
     * \see listActivePlugins()
     * \see loadPlugin()
     */
    virtual bool isPluginLoaded( const QString &packageName ) = 0;

    /**
     * Returns TRUE if the plugin is user enabled (i.e. installed and checked in the user's plugin configuration)
     *
     * \see isPluginLoaded()
     */
    virtual bool isPluginEnabled( const QString &packageName ) const = 0;

    /**
     * Returns a list of active (loaded) plugins.
     *
     * \see isPluginLoaded()
     * \see loadPlugin()
     */
    virtual QStringList listActivePlugins() = 0;

    /**
     * Loads a Python plugin (via import) and returns TRUE if the plugin was successfully loaded.
     *
     * \see isPluginLoaded()
     * \see startPlugin()
     */
    virtual bool loadPlugin( const QString &packageName ) = 0;

    /**
     * Starts the plugin with matching \a packageName. The plugin must have already been loaded
     * via a call to loadPlugin().
     *
     * Calling this adds the plugin to the active plugins list and calls its initGui() implementation.
     *
     * Returns TRUE if the plugin was successfully started.
     *
     * \see loadPlugin()
     */
    virtual bool startPlugin( const QString &packageName ) = 0;

    /**
     * Start a Processing plugin
     *
     * This command adds a plugin to active plugins and calls initProcessing(),
     * initializing only Processing related components of that plugin.
     *
     * \see pluginHasProcessingProvider()
     * \since QGIS 3.8
     */
    virtual bool startProcessingPlugin( const QString &packageName ) = 0;

    /**
     * Helper function to return some information about a plugin.
     *
     * \param function metadata component to return. Must match one of the strings: name, type, version, description, hasProcessingProvider.
     */
    virtual QString getPluginMetadata( const QString &pluginName, const QString &function ) = 0;

    /**
     * Returns TRUE if a plugin implements a Processing provider.
     *
     * This is determined by checking the plugin metadata for the "hasProcessingProvider=yes"
     * or "hasProcessingProvider=true" line.
     *
     * \see startProcessingPlugin()
     * \since QGIS 3.8
     */
    virtual bool pluginHasProcessingProvider( const QString &pluginName ) = 0;

    /**
     * Confirms that the plugin can be uninstalled.
     */
    virtual bool canUninstallPlugin( const QString &packageName ) = 0;

    /**
     * Unloads a plugin.
     *
     * Triggers the plugin's unload() implementation and removes it from the list of loaded plugins.
     *
     * Returns TRUE if the plugin was successfully unloaded.
     */
    virtual bool unloadPlugin( const QString &packageName ) = 0;

};

#endif
