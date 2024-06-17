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

    ~QgsPythonUtilsImpl() override;

    /* general purpose functions */

    void initPython( QgisInterface *interface, bool installErrorHook, const QString &faultHandlerLogPath = QString() ) final;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    void initServerPython( QgsServerInterface *interface ) final;
    bool startServerPlugin( QString packageName ) final;
#endif
    void exitPython() final;
    bool isEnabled() final;
    bool runString( const QString &command, QString msgOnError = QString(), bool single = true ) final;
    QString runStringUnsafe( const QString &command, bool single = true ) final; // returns error traceback on failure, empty QString on success
    bool evalString( const QString &command, QString &result ) final;
    bool getError( QString &errorClassName, QString &errorText ) final;

    /**
     * Returns the path where QGIS Python related files are located.
     */
    QString pythonPath() const;

    /**
     * Returns an object's type name as a string
     */
    QString getTypeAsString( PyObject *obj );

    /* plugins related functions */

    /**
     * Returns the current path for Python plugins
     */
    QString pluginsPath() const;

    /**
     * Returns the current path for Python in home directory.
     */
    QString homePythonPath() const;

    /**
     * Returns the current path for home directory Python plugins.
     */
    QString homePluginsPath() const;

    /**
     * Returns a list of extra plugins paths passed with QGIS_PLUGINPATH environment variable.
     */
    QStringList extraPluginsPaths() const;

    QStringList pluginList() final;
    bool isPluginLoaded( const QString &packageName ) final;
    QStringList listActivePlugins() final;
    bool loadPlugin( const QString &packageName ) final;
    bool startPlugin( const QString &packageName ) final;
    bool startProcessingPlugin( const QString &packageName ) final;
    bool finalizeProcessingStartup() final;
    QString getPluginMetadata( const QString &pluginName, const QString &function ) final;
    bool pluginHasProcessingProvider( const QString &pluginName ) final;
    bool canUninstallPlugin( const QString &packageName ) final;
    bool unloadPlugin( const QString &packageName ) final;
    bool isPluginEnabled( const QString &packageName ) const final;
    void initGDAL() final;

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
    bool mPythonEnabled = false;

  private:

    bool mErrorHookInstalled = false;
    QString mFaultHandlerLogPath;
};

#endif
