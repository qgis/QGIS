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
#include "qgspythonutils.h"
#include "qgslogger.h"

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
    : mPythonUtils( NULL )
{
// constructor does nothing
}

void QgsPluginRegistry::setPythonUtils( QgsPythonUtils* pythonUtils )
{
  mPythonUtils = pythonUtils;
}

bool QgsPluginRegistry::isLoaded( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  return ( it != mPlugins.end() );
}

QString QgsPluginRegistry::library( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it == mPlugins.end() )
    return QString();

  return it->library();
}

QgisPlugin *QgsPluginRegistry::plugin( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it == mPlugins.end() )
    return NULL;

  return it->plugin();
}

bool QgsPluginRegistry::isPythonPlugin( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it == mPlugins.end() )
    return false;
  return it->isPython();
}

void QgsPluginRegistry::addPlugin( QString key, QgsPluginMetadata metadata )
{
  mPlugins.insert( key, metadata );
}

void QgsPluginRegistry::dump()
{
  QgsDebugMsg( "PLUGINS IN REGISTRY: key -> (name, library, isPython)" );
  for ( QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    QgsDebugMsg( QString( "PLUGIN: %1 -> (%2, %3, %4)" )
                 .arg( it.key() )
                 .arg( it->name() )
                 .arg( it->library() )
                 .arg( it->isPython() ) );
  }
}


void QgsPluginRegistry::removePlugin( QString key )
{
  QgsDebugMsg( "removing plugin: " + key );
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it != mPlugins.end() )
  {
    mPlugins.erase( it );
  }
}

void QgsPluginRegistry::unloadAll()
{
  for ( QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( isPythonPlugin( it.key() ) )
    {
      if ( mPythonUtils )
        mPythonUtils->unloadPlugin( it->library() );
      else
        QgsDebugMsg( "warning: python utils is NULL" );
    }
    else
    {
      if ( it->plugin() )
        it->plugin()->unload();
      else
        QgsDebugMsg( "warning: plugin is NULL:" + it.key() );
    }
  }
}
