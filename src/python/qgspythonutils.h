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

#include <QString>
#include <QStringList>


class QgisInterface;

/**
 All calls to Python functions in QGIS come here.
 This class is a singleton.

 Default path for python plugins is:
 - QgsApplication::qgisSettingsDirPath() + "/python/plugins"
 - QgsApplication::pkgDataPath() + "/python/plugins"

 */

class PYTHON_EXPORT QgsPythonUtils
{
  public:

    virtual ~QgsPythonUtils() {}

    //! returns true if python support is ready to use (must be inited first)
    virtual bool isEnabled() = 0;

    //! initialize python and import bindings
    virtual void initPython( QgisInterface* interface ) = 0;

    //! close python interpreter
    virtual void exitPython() = 0;

    /* console */

    //! run a statement, show an error message on error
    //! @return true if no error occured
    virtual bool runString( const QString& command, QString msgOnError = QString() ) = 0;

    //! run a statement, error reporting is not done
    //! @return true if no error occured
    virtual bool runStringUnsafe( const QString& command, bool single = true ) = 0;

    virtual bool evalString( const QString& command, QString& result ) = 0;

    //! get information about error to the supplied arguments
    //! @return false if there was no python error
    virtual bool getError( QString& errorClassName, QString& errorText ) = 0;

    /* plugins */

    //! return list of all available python plugins
    virtual QStringList pluginList() = 0;

    //! return whether the plugin is loaded (active)
    virtual bool isPluginLoaded( QString packageName ) = 0;

    //! return a list of active plugins
    virtual QStringList listActivePlugins() = 0;

    //! load python plugin (import)
    virtual bool loadPlugin( QString packageName ) = 0;

    //! start plugin: add to active plugins and call initGui()
    virtual bool startPlugin( QString packageName ) = 0;

    //! helper function to get some information about plugin
    //! @param function one of these strings: name, tpye, version, description
    virtual QString getPluginMetadata( QString pluginName, QString function ) = 0;

    //! confirm that the plugin can be uninstalled
    virtual bool canUninstallPlugin( QString packageName ) = 0;

    //! unload plugin
    virtual bool unloadPlugin( QString packageName ) = 0;
};

#endif
