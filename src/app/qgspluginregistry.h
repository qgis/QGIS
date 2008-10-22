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
#include <map>
class QgsPluginMetadata;
class QgsPythonUtils;
class QgisPlugin;
class QString;
/**
* \class QgsPluginRegistry
* \brief This class tracks plugins that are currently loaded an provides
* a means to fetch a pointer to a plugin and unload it
*/
class QgsPluginRegistry
{
  public:
    //! Returns the instance pointer, creating the object on the first call
    static QgsPluginRegistry* instance();
    //! Return the full path to the plugins library using the plugin name as a key
    QString library( QString pluginKey );
    //! Retrieve the metadata for a plugin by name
    QgsPluginMetadata * pluginMetadata( QString name );
    //! Retrieve a pointer to a loaded plugin by name
    QgisPlugin * plugin( QString name );
    //! Return whether the plugin is pythonic
    bool isPythonPlugin( QString name );
    //! Add a plugin to the map of loaded plugins
    void addPlugin( QString _library, QString _name, QgisPlugin * _plugin );
    //! Add a plugin written in python
    void addPythonPlugin( QString packageName, QString pluginName );
    //! Remove a plugin from the list of loaded plugins
    void removePlugin( QString name );
    //! Unload plugins
    void unloadAll();
    //! Save pointer for python utils (needed for unloading python plugins)
    void setPythonUtils( QgsPythonUtils* pythonUtils );
  protected:
    //! protected constructor
    QgsPluginRegistry();
  private:
    static QgsPluginRegistry* _instance;
    std::map<QString, QgsPluginMetadata*> plugins;
    QgsPythonUtils* mPythonUtils;
};
#endif //QgsPluginRegistry_H
