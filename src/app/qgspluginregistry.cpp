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

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QMessageBox>
#include <QSettings>

#include "qgis.h"
#include "qgisinterface.h"
#include "qgspluginregistry.h"
#include "qgspluginmetadata.h"
#include "qgisplugin.h"
#include "qgspythonutils.h"
#include "qgslogger.h"


/* typedefs for plugins */
typedef QgisPlugin *create_ui( QgisInterface * qI );
typedef QString name_t();
typedef QString description_t();
typedef int type_t();


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
    : mPythonUtils( NULL ), mQgisInterface( NULL )
{
// constructor does nothing
}

void QgsPluginRegistry::setQgisInterface( QgisInterface* iface )
{
  mQgisInterface = iface;
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



void QgsPluginRegistry::loadPythonPlugin( QString packageName )
{
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
  {
    QgsDebugMsg( "Python is not enabled in QGIS." );
    return;
  }

  // is loaded already?
  if ( ! isLoaded( packageName ) )
  {
    mPythonUtils->loadPlugin( packageName );
    mPythonUtils->startPlugin( packageName );

    // TODO: test success

    QString pluginName = mPythonUtils->getPluginMetadata( packageName, "name" );

    // add to plugin registry
    addPlugin( packageName, QgsPluginMetadata( packageName, pluginName, NULL, true ) );

    // add to settings
    QSettings settings;
    settings.setValue( "/PythonPlugins/" + packageName, true );
    std::cout << "Loaded : " << pluginName.toLocal8Bit().constData() << " (package: "
        << packageName.toLocal8Bit().constData() << ")" << std::endl; // OK

  }
}


void QgsPluginRegistry::loadCppPlugin( QString theFullPathName )
{
  QSettings settings;

  QString baseName = QFileInfo( theFullPathName ).baseName();

  // first check to see if its already loaded
  if ( isLoaded( baseName ) )
  {
    // plugin is loaded
    // QMessageBox::warning(this, "Already Loaded", description + " is already loaded");
    return;
  }
    
  QLibrary myLib( theFullPathName );

  QString myError; //we will only show detailed diagnostics if something went wrong
  myError += "Library name is " + myLib.fileName() + " " + QString( __LINE__ ) + " in " + QString( __FUNCTION__ ) + "\n";

  bool loaded = myLib.load();
  if ( ! loaded )
  {
    QgsDebugMsg( "Failed to load " + theFullPathName );
    return;
  }
    
  myError += "Attempting to resolve the classFactory function " +  QString( __LINE__ ) + " in " + QString( __FUNCTION__ ) + "\n";

  type_t *pType = ( type_t * ) cast_to_fptr( myLib.resolve( "type" ) );
  name_t *pName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );

  switch ( pType() )
  {
    case QgisPlugin::RENDERER:
    case QgisPlugin::UI:
    {
      // UI only -- doesn't use mapcanvas
      create_ui *cf = ( create_ui * ) cast_to_fptr( myLib.resolve( "classFactory" ) );
      if ( cf )
      {
        QgisPlugin *pl = cf( mQgisInterface );
        if ( pl )
        {
          pl->initGui();
          // add it to the plugin registry
          addPlugin( baseName, QgsPluginMetadata( myLib.fileName(), pName(), pl ) );
          //add it to the qsettings file [ts]
          settings.setValue( "/Plugins/" + baseName, true );
        }
        else
        {
          // something went wrong
          QMessageBox::warning( mQgisInterface->mainWindow(), QObject::tr( "Error Loading Plugin" ),
                                QObject::tr( "There was an error loading a plugin."
              "The following diagnostic information may help the QGIS developers resolve the issue:\n%1." ).arg
                  ( myError ) );
          //disable it to the qsettings file [ts]
          settings.setValue( "/Plugins/" + baseName, false );
        }
      }
      else
      {
        QgsDebugMsg( "Unable to find the class factory for " + theFullPathName );
      }

    }
    break;
    default:
      // type is unknown
      QgsDebugMsg( "Plugin " + theFullPathName + " did not return a valid type and cannot be loaded" );
      break;
  }

}


void QgsPluginRegistry::restoreSessionPlugins( QString thePluginDirString )
{
  QSettings mySettings;

#ifdef WIN32
  QString pluginExt = "*.dll";
#else
  QString pluginExt = "*.so*";
#endif

  // check all libs in the current plugin directory and get name and descriptions
  QDir myPluginDir( thePluginDirString, pluginExt, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

  for ( uint i = 0; i < myPluginDir.count(); i++ )
  {
    QString myFullPath = thePluginDirString + "/" + myPluginDir[i];
    if (checkCppPlugin( myFullPath ) )
    {
      // check if the plugin was active on last session

      QString baseName = QFileInfo( myFullPath ).baseName();
      if ( mySettings.value( "/Plugins/" + baseName ).toBool() )
      {
        loadCppPlugin( myFullPath );
      }
    }
  }
  
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // check for python plugins system-wide
    QStringList pluginList = mPythonUtils->pluginList();
    QgsDebugMsg("Loading python plugins");

    for ( int i = 0; i < pluginList.size(); i++ )
    {
      QString packageName = pluginList[i];
      
      if (checkPythonPlugin( packageName ) )
      {
        // check if the plugin was active on last session

        if ( mySettings.value( "/PythonPlugins/" + packageName ).toBool() )
        {
          loadPythonPlugin( packageName );
        }
      }
    }
  }

  QgsDebugMsg("Plugin loading completed");
}
  

bool QgsPluginRegistry::checkCppPlugin( QString pluginFullPath )
{
  QLibrary myLib( pluginFullPath );
  bool loaded = myLib.load();
  if ( ! loaded )
  {
    //QgsDebugMsg("Failed to load " + myLib.fileName());
    //QgsDebugMsg("Reason: " + myLib.errorString());
    return false;
  }

  name_t * myName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );
  description_t *  myDescription = ( description_t * )  cast_to_fptr( myLib.resolve( "description" ) );
  version_t *  myVersion = ( version_t * ) cast_to_fptr( myLib.resolve( "version" ) );
  
  if ( myName && myDescription  && myVersion )
    return true;
      
  QgsDebugMsg("Failed to get name, description, or type for " + myLib.fileName());
  return false;
}
    

bool QgsPluginRegistry::checkPythonPlugin( QString packageName )
{
  // import plugin's package
  if ( !mPythonUtils->loadPlugin( packageName ) )
    return false;

  QString pluginName, description, version;
  
  // get information from the plugin
  // if there are some problems, don't continue with metadata retreival
  pluginName = mPythonUtils->getPluginMetadata( packageName, "name" );
  if ( pluginName != "__error__" )
  {
    description = mPythonUtils->getPluginMetadata( packageName, "description" );
    if ( description != "__error__" )
      version = mPythonUtils->getPluginMetadata( packageName, "version" );
  }

  if ( pluginName == "__error__" || description == "__error__" || version == "__error__" )
  {
    QMessageBox::warning( mQgisInterface->mainWindow(), QObject::tr( "Python error" ),
                          QObject::tr( "Error when reading metadata of plugin " ) + packageName );
    return false;
  }
  
  return true;
}
