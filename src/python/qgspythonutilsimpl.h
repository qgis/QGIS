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

    //! initialize python and import bindings
    void initPython( QgisInterface* interface ) override;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    //! initialize python for server and import bindings
    void initServerPython( QgsServerInterface* interface ) override;
    bool startServerPlugin( QString packageName ) override;
#endif

    //! close python interpreter
    void exitPython() override;

    //! returns true if python support is ready to use (must be inited first)
    bool isEnabled() override;

    //! returns path where QGIS python stuff is located
    QString pythonPath();

    //! run a statement (wrapper for PyRun_String)
    //! this command is more advanced as enables error checking etc.
    //! when an exception is raised, it shows dialog with exception details
    //! @return true if no error occured
    bool runString( const QString& command, QString msgOnError = QString(), bool single = true ) override;

    //! run a statement, error reporting is not done
    //! @return true if no error occured
    bool runStringUnsafe( const QString& command, bool single = true ) override;

    bool evalString( const QString& command, QString& result ) override;

    //! @return object's type name as a string
    QString getTypeAsString( PyObject* obj );

    //! get information about error to the supplied arguments
    //! @return false if there was no python error
    bool getError( QString& errorClassName, QString& errorText ) override;

    /* plugins related functions */

    //! return current path for python plugins
    QString pluginsPath();

    //! return current path for python in home directory
    QString homePythonPath();

    //! return current path for home directory python plugins
    QString homePluginsPath();

    //! return a list of extra plugins paths passed with QGIS_PLUGINPATH environment variable
    QStringList extraPluginsPaths();

    //! return list of all available python plugins
    QStringList pluginList() override;

    //! return whether the plugin is loaded (active)
    virtual bool isPluginLoaded( QString packageName ) override;

    //! return a list of active plugins
    virtual QStringList listActivePlugins() override;

    //! load python plugin (import)
    bool loadPlugin( QString packageName ) override;

    //! start plugin: add to active plugins and call initGui()
    bool startPlugin( QString packageName ) override;

    //! helper function to get some information about plugin
    //! @param function one of these strings: name, tpye, version, description
    QString getPluginMetadata( QString pluginName, QString function ) override;

    //! confirm it is safe to uninstall the plugin
    bool canUninstallPlugin( QString packageName ) override;

    //! unload plugin
    bool unloadPlugin( QString packageName ) override;

  protected:

    /* functions that do the initialization work */

    //! initialize Python context
    void init();

    //! check qgis imports and plugins
    //@return true if all imports worked
    bool checkSystemImports();

    //@return true if qgis.user could be imported
    bool checkQgisUser();

    //! import user defined Python code
    void doUserImports();

    //! cleanup Python context
    void finish();


    void installErrorHook();

    void uninstallErrorHook();

    QString getTraceback();

    //! convert python object to QString. If the object isn't unicode/str, it will be converted
    QString PyObjectToQString( PyObject* obj );

    //! reference to module __main__
    PyObject* mMainModule;

    //! dictionary of module __main__
    PyObject* mMainDict;

    //! flag determining that python support is enabled
    bool mPythonEnabled;
};


#endif
