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

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QMessageBox>
#include <QSettings>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgisinterface.h"
#include "qgspluginregistry.h"
#include "qgspluginmetadata.h"
#include "qgisplugin.h"
#include "qgspythonutils.h"
#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

/* typedefs for plugins */
typedef QgisPlugin *create_ui( QgisInterface * qI );
typedef QString name_t();
typedef QString description_t();
typedef QString category_t();
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
  if ( it != mPlugins.end() ) // found a c++ plugin?
    return true;

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    return mPythonUtils->isPluginLoaded( key );
  }

  return false;
}

QString QgsPluginRegistry::library( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it != mPlugins.end() )
    return it->library();

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    if ( mPythonUtils->isPluginLoaded( key ) )
      return key;
  }

  return QString();
}

QgisPlugin *QgsPluginRegistry::plugin( QString key )
{
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it == mPlugins.end() )
    return 0;

  // note: not used by python plugins

  return it->plugin();
}

bool QgsPluginRegistry::isPythonPlugin( QString key )
{
  return mPythonUtils && mPythonUtils->isEnabled() && mPythonUtils->isPluginLoaded( key );
}

void QgsPluginRegistry::addPlugin( QString key, QgsPluginMetadata metadata )
{
  mPlugins.insert( key, metadata );
}

void QgsPluginRegistry::dump()
{
  QgsDebugMsg( "PLUGINS IN REGISTRY: key -> (name, library)" );
  for ( QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    QgsDebugMsg( QString( "PLUGIN: %1 -> (%2, %3)" )
                 .arg( it.key() )
                 .arg( it->name() )
                 .arg( it->library() ) );
  }

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsDebugMsg( "PYTHON PLUGINS IN REGISTRY:" );
    foreach ( QString pluginName, mPythonUtils->listActivePlugins() )
    {
      QgsDebugMsg( pluginName );
    }
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

  // python plugins are removed when unloaded
}

void QgsPluginRegistry::unloadAll()
{
  for ( QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
        it != mPlugins.end();
        it++ )
  {
    if ( it->plugin() )
    {
      it->plugin()->unload();
    }
    else
    {
      QgsDebugMsg( "warning: plugin is NULL:" + it.key() );
    }
  }

  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    foreach ( QString pluginName, mPythonUtils->listActivePlugins() )
    {
      mPythonUtils->unloadPlugin( pluginName );
    }
  }
}


bool QgsPluginRegistry::checkQgisVersion( QString minVersion )
{
  QStringList minVersionParts = minVersion.split( '.' );
  // qgis version must be in form x.y.z or just x.y
  if ( minVersionParts.count() != 2 && minVersionParts.count() != 3 )
    return false;

  int minVerMajor, minVerMinor, minVerBugfix = 0;
  bool ok;
  minVerMajor = minVersionParts.at( 0 ).toInt( &ok );
  if ( !ok )
    return false;
  minVerMinor = minVersionParts.at( 1 ).toInt( &ok );
  if ( !ok )
    return false;
  if ( minVersionParts.count() == 3 )
  {
    minVerBugfix = minVersionParts.at( 2 ).toInt( &ok );
    if ( !ok )
      return false;
  }

  // our qgis version - cut release name after version number
  QString qgisVersion = QString( QGis::QGIS_VERSION ).section( '-', 0, 0 );
  QStringList qgisVersionParts = qgisVersion.split( "." );

  int qgisMajor = qgisVersionParts.at( 0 ).toInt();
  int qgisMinor = qgisVersionParts.at( 1 ).toInt();
  int qgisBugfix = qgisVersionParts.at( 2 ).toInt();

  // first check major version
  if ( minVerMajor > qgisMajor )
    return false;
  if ( minVerMajor < qgisMajor )
    return true;
  // if same, check minor version
  if ( minVerMinor > qgisMinor )
    return false;
  if ( minVerMinor < qgisMinor )
    return true;

  // if still same, check bugfix version
  if ( minVerBugfix > qgisBugfix )
    return false;

  // looks like min version is the same as our version - that's fine
  return true;
}


void QgsPluginRegistry::loadPythonPlugin( QString packageName )
{
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Python is not enabled in QGIS." ), QObject::tr( "Plugins" ) );
    return;
  }

  QSettings settings;

  // is loaded already?
  if ( ! isLoaded( packageName ) )
  {
    // if plugin is not compatible, disable it
    if ( ! isPythonPluginCompatible( packageName ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Plugin \"%1\" is not compatible with this version of QGIS.\nIt will be disabled." ).arg( packageName ),
                                 QObject::tr( "Plugins" ) );
      settings.setValue( "/PythonPlugins/" + packageName, false );
      return;
    }

    mPythonUtils->loadPlugin( packageName );
    mPythonUtils->startPlugin( packageName );

    // TODO: test success

    QString pluginName = mPythonUtils->getPluginMetadata( packageName, "name" );

    // add to settings
    settings.setValue( "/PythonPlugins/" + packageName, true );
    QgsMessageLog::logMessage( QObject::tr( "Loaded %1 (package: %2)" ).arg( pluginName ).arg( packageName ), QObject::tr( "Plugins" ), QgsMessageLog::INFO );
  }
}


void QgsPluginRegistry::loadCppPlugin( QString theFullPathName )
{
  // first check to see if its already loaded
  QString baseName = QFileInfo( theFullPathName ).baseName();
  if ( isLoaded( baseName ) )
    return;

  QLibrary myLib( theFullPathName );
  if ( !myLib.load() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1 (Reason: %2)" ).arg( myLib.fileName() ).arg( myLib.errorString() ), QObject::tr( "Plugins" ) );
    return;
  }

  QString myError( QObject::tr( "Library name is %1\n" ).arg( myLib.fileName() ) );

  myError += QObject::tr( "Attempting to resolve the classFactory function\n" );

  type_t *pType = ( type_t * ) cast_to_fptr( myLib.resolve( "type" ) );
  name_t *pName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );

  switch ( pType() )
  {
    case QgisPlugin::RENDERER:
    case QgisPlugin::UI:
    case QgisPlugin::VECTOR_OVERLAY:
    {
      // UI only -- doesn't use mapcanvas
      create_ui *cf = ( create_ui * ) cast_to_fptr( myLib.resolve( "classFactory" ) );
      if ( !cf )
      {
        QgsMessageLog::logMessage( QObject::tr( "Unable to find the class factory for %1." ).arg( theFullPathName ), QObject::tr( "Plugins" ) );
        break;
      }

      QSettings settings;
      QgisPlugin *pl = cf( mQgisInterface );
      if ( !pl )
      {
        // something went wrong
        QMessageBox::warning( mQgisInterface->mainWindow(), QObject::tr( "Error Loading Plugin" ),
                              QObject::tr( "There was an error loading a plugin."
                                           "The following diagnostic information may help the QGIS developers resolve the issue:\n%1." )
                              .arg( myError ) );
        //disable it to the qsettings file [ts]
        settings.setValue( "/Plugins/" + baseName, false );
        break;
      }

      pl->initGui();
      // add it to the plugin registry
      addPlugin( baseName, QgsPluginMetadata( myLib.fileName(), pName(), pl ) );
      //add it to the qsettings file [ts]
      settings.setValue( "/Plugins/" + baseName, true );
      QgsMessageLog::logMessage( QObject::tr( "Loaded %1 (Path: %2)" ).arg( pName() ).arg( myLib.fileName() ), QObject::tr( "Plugins" ), QgsMessageLog::INFO );

      QObject *o = dynamic_cast<QObject *>( pl );
      if ( !o )
        break;

      QgsDebugMsg( QString( "plugin object name: %1" ).arg( o->objectName() ) );
      if ( o->objectName().isEmpty() )
      {
#ifndef WIN32
        baseName = baseName.mid( 3 );
#endif
        QgsDebugMsg( QString( "object name to %1" ).arg( baseName ) );
        o->setObjectName( QString( "qgis_plugin_%1" ).arg( baseName ) );
        QgsDebugMsg( QString( "plugin object name now: %1" ).arg( o->objectName() ) );
      }

      if ( !o->parent() )
      {
        QgsDebugMsg( QString( "setting plugin parent" ) );
        o->setParent( QgisApp::instance() );
      }
      else
      {
        QgsDebugMsg( QString( "plugin parent already set" ) );
      }
    }
    break;
    default:
      // type is unknown
      QgsMessageLog::logMessage( QObject::tr( "Plugin %1 did not return a valid type and cannot be loaded" ).arg( theFullPathName ), QObject::tr( "Plugins" ) );
      break;
  }
}


void QgsPluginRegistry::unloadPythonPlugin( QString packageName )
{
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Python is not enabled in QGIS." ), QObject::tr( "Plugins" ) );
    return;
  }

  if ( isLoaded( packageName ) )
  {
    mPythonUtils->unloadPlugin( packageName );
    // add to settings
    QSettings settings;
    settings.setValue( "/PythonPlugins/" + packageName, false );
    QgsDebugMsg( "Python plugin successfully unloaded: " + packageName );
  }
}


void QgsPluginRegistry::unloadCppPlugin( QString theFullPathName )
{
  QString baseName = QFileInfo( theFullPathName ).baseName();
  // first check to see if it's loaded
  if ( isLoaded( baseName ) )
  {
    QgisPlugin * pluginInstance = plugin( baseName );
    if ( pluginInstance )
    {
      pluginInstance->unload();
    }
    QSettings settings;
    settings.setValue( "/Plugins/" + baseName, false );
    // remove the plugin from the registry
    removePlugin( baseName );
    QgsDebugMsg( "Cpp plugin successfully unloaded: " + baseName);
  }
}


//overloaded version of the next method that will load from multiple directories not just one
void QgsPluginRegistry::restoreSessionPlugins( QStringList thePluginDirList )
{
  QStringListIterator myIterator( thePluginDirList );
  while ( myIterator.hasNext() )
  {
    restoreSessionPlugins( myIterator.next() );
  }
}

void QgsPluginRegistry::restoreSessionPlugins( QString thePluginDirString )
{
  QSettings mySettings;

#if defined(WIN32) || defined(__CYGWIN__)
  QString pluginExt = "*.dll";
#elif ANDROID
  QString pluginExt = "*plugin.so";
#else
  QString pluginExt = "*.so*";
#endif

  // check all libs in the current plugin directory and get name and descriptions
  QDir myPluginDir( thePluginDirString, pluginExt, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

  for ( uint i = 0; i < myPluginDir.count(); i++ )
  {
    QString myFullPath = thePluginDirString + "/" + myPluginDir[i];
    if ( checkCppPlugin( myFullPath ) )
    {
      // check if the plugin was active on last session

      QString baseName = QFileInfo( myFullPath ).baseName();
      if ( mySettings.value( "/Plugins/" + baseName ).toBool() )
      {
        loadCppPlugin( myFullPath );
      }
    }
  }

  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
    return;

  // check for python plugins system-wide
  QStringList pluginList = mPythonUtils->pluginList();
  QgsDebugMsg( "Loading python plugins" );

  QStringList corePlugins = QStringList();
  corePlugins << "plugin_installer";
  corePlugins << "fTools";
  corePlugins << "GdalTools";
  corePlugins << "db_manager";

  // make the required core plugins enabled by default:
  for ( int i = 0; i < corePlugins.size(); i++ )
  {
    if ( !mySettings.contains( "/PythonPlugins/" + corePlugins[i] ) )
    {
      mySettings.setValue( "/PythonPlugins/" + corePlugins[i], true );
    }
  }

  for ( int i = 0; i < pluginList.size(); i++ )
  {
    QString packageName = pluginList[i];

    // TODO: apply better solution for #5879
    // start - temporary fix for issue #5879
    if ( QgsApplication::isRunningFromBuildDir() )
    {
      if ( corePlugins.contains( packageName ) )
      {
        QgsApplication::setPkgDataPath( QString( "" ) );
      }
      else
      {
        QgsApplication::setPkgDataPath( QgsApplication::buildSourcePath() );
      }
    }
    // end - temporary fix for issue #5879, more below

    if ( checkPythonPlugin( packageName ) )
    {
      // check if the plugin was active on last session

      if ( mySettings.value( "/PythonPlugins/" + packageName ).toBool() )
      {
        loadPythonPlugin( packageName );
      }
    }
  }

  // start - temporary fix for issue #5879, more above
  if ( QgsApplication::isRunningFromBuildDir() )
  {
    QgsApplication::setPkgDataPath( QgsApplication::buildSourcePath() );
  }
  // end - temporary fix for issue #5879

  QgsDebugMsg( "Plugin loading completed" );
}


bool QgsPluginRegistry::checkCppPlugin( QString pluginFullPath )
{
  QLibrary myLib( pluginFullPath );
  if ( !myLib.load() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1 (Reason: %2)" ).arg( myLib.fileName() ).arg( myLib.errorString() ), QObject::tr( "Plugins" ) );
    return false;
  }

  name_t * myName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );
  description_t *  myDescription = ( description_t * )  cast_to_fptr( myLib.resolve( "description" ) );
  category_t *  myCategory = ( category_t * )  cast_to_fptr( myLib.resolve( "category" ) );
  version_t *  myVersion = ( version_t * ) cast_to_fptr( myLib.resolve( "version" ) );

  if ( myName && myDescription && myVersion && myCategory )
    return true;

  QgsDebugMsg( "Failed to get name, description, category or type for " + myLib.fileName() );
  return false;
}


bool QgsPluginRegistry::checkPythonPlugin( QString packageName )
{
  QString pluginName, description, /*category,*/ version;

  // get information from the plugin
  // if there are some problems, don't continue with metadata retreival
  pluginName  = mPythonUtils->getPluginMetadata( packageName, "name" );
  description = mPythonUtils->getPluginMetadata( packageName, "description" );
  version     = mPythonUtils->getPluginMetadata( packageName, "version" );
  // for Python plugins category still optional, by default used "Plugins" category
  //category = mPythonUtils->getPluginMetadata( packageName, "category" );

  if ( pluginName == "__error__" || description == "__error__" || version == "__error__" )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error when reading metadata of plugin %1" ).arg( packageName ),
                               QObject::tr( "Plugins" ) );
    return false;
  }

  return true;
}

bool QgsPluginRegistry::isPythonPluginCompatible( QString packageName )
{
  QString minVersion = mPythonUtils->getPluginMetadata( packageName, "qgisMinimumVersion" );
  return minVersion != "__error__" && checkQgisVersion( minVersion );
}

QList<QgsPluginMetadata*> QgsPluginRegistry::pluginData()
{
  QList<QgsPluginMetadata*> resultList;
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
  for ( ; it != mPlugins.end(); ++it )
  {
    resultList.push_back( &( it.value() ) );
  }
  return resultList;
}
