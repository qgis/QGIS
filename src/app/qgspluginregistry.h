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

#ifndef QGSPLUGINREGISTRY_H
#define QGSPLUGINREGISTRY_H

#include <QMap>
#include "qgspluginmetadata.h"

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
* - Python plugins: module name (directory) of plugin, e.g. db_manager
*/
class APP_EXPORT QgsPluginRegistry
{
  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsPluginRegistry* instance();

    //! set pointer to qgis interface passed to plugins (used by QgisApp)
    void setQgisInterface( QgisInterface* iface );

    //! Check whether this module is loaded
    bool isLoaded( const QString& key ) const;

    //! Retrieve library of the plugin
    QString library( const QString& key );

    //! Retrieve a pointer to a loaded plugin
    QgisPlugin * plugin( const QString& key );

    //! Return whether the plugin is pythonic
    bool isPythonPlugin( const QString& key ) const;

    //! Add a plugin to the map of loaded plugins
    void addPlugin( const QString& key, const QgsPluginMetadata& metadata );

    //! Remove a plugin from the list of loaded plugins
    void removePlugin( const QString& key );

    //! Unload plugins
    void unloadAll();

    //! Save pointer for python utils (needed for unloading python plugins)
    void setPythonUtils( QgsPythonUtils* pythonUtils );

    //! Dump list of plugins
    void dump();

    //! C++ plugin loader
    void loadCppPlugin( const QString& mFullPath );
    //! Python plugin loader
    void loadPythonPlugin( const QString& packageName );

    //! C++ plugin unloader
    void unloadCppPlugin( const QString& theFullPathName );
    //! Python plugin unloader
    void unloadPythonPlugin( const QString& packageName );

    //! Overloaded version of the next method that will load from multiple directories not just one
    void restoreSessionPlugins( const QStringList& thePluginDirList );
    //! Load any plugins used in the last qgis session
    void restoreSessionPlugins( const QString& thePluginDirString );

    //! Check whether plugin is compatible with current version of QGIS
    bool isPythonPluginCompatible( const QString& packageName ) const;

    //! Returns metadata of all loaded plugins
    QList<QgsPluginMetadata*> pluginData();

  protected:
    //! protected constructor
    QgsPluginRegistry();

    //! Try to load and get metadata from c++ plugin, return true on success
    bool checkCppPlugin( const QString& pluginFullPath );
    //! Try to load and get metadata from Python plugin, return true on success
    bool checkPythonPlugin( const QString& packageName );

    //! Check current QGIS version against requested minimal and optionally maximal QGIS version
    //! if maxVersion not specified, the default value is assumed: floor(minVersion) + 0.99.99
    bool checkQgisVersion( const QString& minVersion, const QString& maxVersion = "" ) const;

  private:
    static QgsPluginRegistry* _instance;
    QMap<QString, QgsPluginMetadata> mPlugins;
    QgsPythonUtils* mPythonUtils;
    QgisInterface* mQgisInterface;
};
#endif //QgsPluginRegistry_H
