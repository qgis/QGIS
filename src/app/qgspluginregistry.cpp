/***************************************************************************
                    QgsPluginRegistry.cpp  -  Singleton class for
                    tracking registering plugins.
                             -------------------
    begin                : Fri Feb 7 2004
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

#include "qgspluginregistry.h"
#include "qgspluginmetadata.h"
#include "qgisplugin.h"

QgsPluginRegistry *QgsPluginRegistry::_instance = 0;
QgsPluginRegistry *QgsPluginRegistry::instance()
{
  if ( _instance == 0 )
  {
    _instance = new QgsPluginRegistry();
  }
  return _instance;
}

QgsPluginRegistry::QgsPluginRegistry()
{
// constructor does nothing
}
QString QgsPluginRegistry::library( QString pluginKey )
{
  QgsPluginMetadata *pmd = plugins[pluginKey];
  QString retval;
  if ( pmd )
  {
    retval = pmd->library();
  }
  return retval;
}

QgsPluginMetadata *QgsPluginRegistry::pluginMetadata( QString name )
{
  return plugins[name];
}

QgisPlugin *QgsPluginRegistry::plugin( QString name )
{
  QgsPluginMetadata *pmd = plugins[name];
  QgisPlugin *retval = 0;
  if ( pmd )
  {
    retval = pmd->plugin();
  }
  return retval;
}

bool QgsPluginRegistry::isPythonPlugin( QString name )
{
  QgsPluginMetadata* pmd = plugins[name];
  if ( pmd )
    return pmd->isPython();
  else
    return false;
}

void QgsPluginRegistry::addPlugin( QString library, QString name, QgisPlugin * plugin )
{
  plugins[name] = new QgsPluginMetadata( library, name, plugin );
}


void QgsPluginRegistry::addPythonPlugin( QString packageName, QString pluginName )
{
  plugins[pluginName] = new QgsPluginMetadata( packageName, pluginName, NULL, true ); // true == python plugin
}

void QgsPluginRegistry::removePlugin( QString name )
{
  plugins.erase( name );
}

void QgsPluginRegistry::unloadAll()
{
  for(std::map<QString, QgsPluginMetadata*>::iterator it=plugins.begin(); 
      it!=plugins.end();
      it++)
    if( it->second->plugin() )
      it->second->plugin()->unload();
}
