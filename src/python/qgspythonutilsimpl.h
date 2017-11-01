/***************************************************************************
    qgspythonutilsimpl.h - routines for interfacing Python
    ---------------------
    begin                : May 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPYTHONUTILSIMPL_H
#define QGSPYTHONUTILSIMPL_H

#include "qgspythonutils.h"

// forward declaration for PyObject
#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif


class QgsPythonUtilsImpl : public QgsPythonUtils
{
  public:

    QgsPythonUtilsImpl();

    virtual ~QgsPythonUtilsImpl();

    /* general purpose functions */

    //! initialize Python and import bindings
    void initPython( QgisInterface *interface ) override;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    //! initialize Python for server and import bindings
    void initServerPython( QgsServerInterface *interface ) override;
    bool startServerPlugin( QString packageName ) override;
#endif

    //! close Python interpreter
    void exitPython() override;

    //! returns true if Python support is ready to use (must be inited first)
    bool isEnabled() override;

    //! returns path where QGIS Python stuff is located
    QString pythonPath();

    /**
     * run a statement (wrapper for PyRun_String)
     * this command is more advanced as enables error checking etc.
     * when an exception is raised, it shows dialog with exception details
     * \returns true if no error occurred
     */
    bool runString( const QString &command, QString msgOnError = QString(), bool single = true ) override;

    /**
     * run a statement, error reporting is not done
     * \returns true if no error occurred
     */
    bool runStringUnsafe( const QString &command, bool single = true ) override;

    bool evalString( const QString &command, QString &result ) override;

    //! \returns object's type name as a string
    QString getTypeAsString( PyObject *obj );

    /**
     * get information about error to the supplied arguments
     * \returns false if there was no Python error
     */
    bool getError( QString &errorClassName, QString &errorText ) override;

    /* plugins related functions */

    //! return current path for Python plugins
    QString pluginsPath();

    //! return current path for Python in home directory
    QString homePythonPath();

    //! return current path for home directory Python plugins
    QString homePluginsPath();

    //! return a list of extra plugins paths passed with QGIS_PLUGINPATH environment variable
    QStringList extraPluginsPaths();

    //! return list of all available Python plugins
    QStringList pluginList() override;

    //! return whether the plugin is loaded (active)
    virtual bool isPluginLoaded( const QString &packageName ) override;

    //! return a list of active plugins
    virtual QStringList listActivePlugins() override;

    //! load Python plugin (import)
    bool loadPlugin( const QString &packageName ) override;

    //! start plugin: add to active plugins and call initGui()
    bool startPlugin( const QString &packageName ) override;

    /**
     * helper function to get some information about plugin
     * \param function one of these strings: name, tpye, version, description
     */
    QString getPluginMetadata( const QString &pluginName, const QString &function ) override;

    //! confirm it is safe to uninstall the plugin
    bool canUninstallPlugin( const QString &packageName ) override;

    //! unload plugin
    bool unloadPlugin( const QString &packageName ) override;

  protected:

    /* functions that do the initialization work */

    //! initialize Python context
    void init();

    //! check qgis imports and plugins
    //\returns true if all imports worked
    bool checkSystemImports();

    //\returns true if qgis.user could be imported
    bool checkQgisUser();

    //! import custom user and global Python code (startup scripts)
    void doCustomImports();

    //! cleanup Python context
    void finish();

    void installErrorHook();

    void uninstallErrorHook();

    QString getTraceback();

    //! convert Python object to QString. If the object isn't unicode/str, it will be converted
    QString PyObjectToQString( PyObject *obj );

    //! reference to module __main__
    PyObject *mMainModule = nullptr;

    //! dictionary of module __main__
    PyObject *mMainDict = nullptr;

    //! flag determining that Python support is enabled
    bool mPythonEnabled;
};

#endif
