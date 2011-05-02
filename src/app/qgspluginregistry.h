/***************************************************************************
                          qgspluginregistry.h
           Singleton class for keeping track of installed plugins.
                             -------------------
    begin                : Mon Jan 26 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSPLUGINREGISTRY_H
#define QGSPLUGINREGISTRY_H

#include <QMap>

class QgsPluginMetadata;
class QgsPythonUtils;
class QgisPlugin;
class QgisInterface;
class QString;

/**
* \class QgsPluginRegistry
* \brief This class tracks plugins that are currently loaded and provides
* a means to fetch a pointer to a plugin and unload it
*
* plugin key is:
* - C++ plugins: base name of plugin library, e.g. libgrassplugin
* - Python plugins: module name (directory) of plugin, e.g. plugin_installer
*/
class QgsPluginRegistry
{
  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsPluginRegistry* instance();

    //! set pointer to qgis interface passed to plugins (used by QgisApp)
    void setQgisInterface( QgisInterface* iface );

    //! Check whether this module is loaded
    bool isLoaded( QString key );

    //! Retrieve library of the plugin
    QString library( QString key );

    //! Retrieve a pointer to a loaded plugin
    QgisPlugin * plugin( QString key );

    //! Return whether the plugin is pythonic
    bool isPythonPlugin( QString key );

    //! Add a plugin to the map of loaded plugins
    void addPlugin( QString key, QgsPluginMetadata metadata );

    //! Remove a plugin from the list of loaded plugins
    void removePlugin( QString key );

    //! Unload plugins
    void unloadAll();

    //! Save pointer for python utils (needed for unloading python plugins)
    void setPythonUtils( QgsPythonUtils* pythonUtils );

    //! Dump list of plugins
    void dump();

    //! C++ plugin loader
    void loadCppPlugin( QString mFullPath );
    //! Python plugin loader
    void loadPythonPlugin( QString packageName );

    //! Overloaded version of the next method that will load from multiple directories not just one
    void restoreSessionPlugins( QStringList thePluginDirList );
    //! Load any plugins used in the last qgis session
    void restoreSessionPlugins( QString thePluginDirString );

    //! Check whether plugin is compatible with current version of QGIS
    bool isPythonPluginCompatible( QString packageName );

    //! Returns metadata of all loaded plugins
    QList<QgsPluginMetadata*> pluginData();

  protected:
    //! protected constructor
    QgsPluginRegistry();

    //! Try to load and get metadata from c++ plugin, return true on success
    bool checkCppPlugin( QString pluginFullPath );
    //! Try to load and get metadata from Python plugin, return true on success
    bool checkPythonPlugin( QString packageName );

    //! Check current QGIS version against plugin's minimal requested QGIS version
    bool checkQgisVersion( QString minVersion );

  private:
    static QgsPluginRegistry* _instance;
    QMap<QString, QgsPluginMetadata> mPlugins;
    QgsPythonUtils* mPythonUtils;
    QgisInterface* mQgisInterface;
};
#endif //QgsPluginRegistry_H
