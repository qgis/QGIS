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

#include "qgspluginregistry.h"

#include "qgis.h"
#include "qgisapp.h"
#include "qgisinterface.h"
#include "qgisplugin.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsmessagelog.h"
#include "qgspluginmetadata.h"
#include "qgsruntimeprofiler.h"
#include "qgssettings.h"

#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QMessageBox>

#ifdef WITH_BINDINGS
#include "qgspythonutils.h"
#endif

/* typedefs for plugins */
typedef QgisPlugin *create_ui( QgisInterface *qI );
typedef const QString *name_t();
typedef const QString *description_t();
typedef const QString *category_t();
typedef int type_t();


QgsPluginRegistry *QgsPluginRegistry::sInstance = nullptr;
QgsPluginRegistry *QgsPluginRegistry::instance()
{
  if ( !sInstance )
  {
    sInstance = new QgsPluginRegistry();
  }
  return sInstance;
}

void QgsPluginRegistry::setQgisInterface( QgisInterface *iface )
{
  mQgisInterface = iface;
}

void QgsPluginRegistry::setPythonUtils( QgsPythonUtils *pythonUtils )
{
  mPythonUtils = pythonUtils;
}

bool QgsPluginRegistry::isLoaded( const QString &key ) const
{
  const QMap<QString, QgsPluginMetadata>::const_iterator it = mPlugins.find( key );
  if ( it != mPlugins.end() ) // found a c++ plugin?
    return true;

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    return mPythonUtils->isPluginLoaded( key );
  }
#endif

  return false;
}

QString QgsPluginRegistry::library( const QString &key )
{
  const QMap<QString, QgsPluginMetadata>::const_iterator it = mPlugins.constFind( key );
  if ( it != mPlugins.constEnd() )
    return it->library();

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    if ( mPythonUtils->isPluginLoaded( key ) )
      return key;
  }
#endif

  return QString();
}

QgisPlugin *QgsPluginRegistry::plugin( const QString &key )
{
  const QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
  if ( it == mPlugins.end() )
    return nullptr;

  // note: not used by python plugins

  return it->plugin();
}

bool QgsPluginRegistry::isPythonPlugin( const QString &key ) const
{
#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    if ( mPythonUtils->isPluginLoaded( key ) )
      return true;
  }
#else
  Q_UNUSED( key )
#endif
  return false;
}

void QgsPluginRegistry::addPlugin( const QString &key, const QgsPluginMetadata &metadata )
{
  mPlugins.insert( key, metadata );
}

void QgsPluginRegistry::dump()
{
  QgsDebugMsgLevel( u"PLUGINS IN REGISTRY: key -> (name, library)"_s, 1 );
  for ( QMap<QString, QgsPluginMetadata>::const_iterator it = mPlugins.constBegin();
        it != mPlugins.constEnd();
        ++it )
  {
    QgsDebugMsgLevel( u"PLUGIN: %1 -> (%2, %3)"_s.arg( it.key(), it->name(), it->library() ), 1 );
  }

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    QgsDebugMsgLevel( u"PYTHON PLUGINS IN REGISTRY:"_s, 1 );
    const auto constListActivePlugins = mPythonUtils->listActivePlugins();
    for ( const QString &pluginName : constListActivePlugins )
    {
      Q_UNUSED( pluginName )
      QgsDebugMsgLevel( pluginName, 1 );
    }
  }
#endif
}


void QgsPluginRegistry::removePlugin( const QString &key )
{
  QgsDebugMsgLevel( "removing plugin: " + key, 2 );
  const QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.find( key );
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
        ++it )
  {
    if ( it->plugin() )
    {
      it->plugin()->unload();
    }
    else
    {
      QgsDebugError( "warning: plugin is NULL:" + it.key() );
    }
  }

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    const auto constListActivePlugins = mPythonUtils->listActivePlugins();
    for ( const QString &pluginName : constListActivePlugins )
    {
      mPythonUtils->unloadPlugin( pluginName );
    }
  }
#endif
}


bool QgsPluginRegistry::checkQgisVersion( const QString &minVersion, const QString &maxVersion ) const
{
  // Parse qgisMinVersion. Must be in form x.y.z or just x.y
  const QStringList minVersionParts = minVersion.split( '.' );
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

  // Parse qgisMaxVersion. Must be in form x.y.z or just x.y
  int maxVerMajor, maxVerMinor, maxVerBugfix = 99;
  if ( maxVersion.isEmpty() || maxVersion == "__error__"_L1 )
  {
    maxVerMajor = minVerMajor;
    maxVerMinor = 99;
  }
  else
  {
    const QStringList maxVersionParts = maxVersion.split( '.' );
    if ( maxVersionParts.count() != 2 && maxVersionParts.count() != 3 )
      return false;

    bool ok;
    maxVerMajor = maxVersionParts.at( 0 ).toInt( &ok );
    if ( !ok )
      return false;
    maxVerMinor = maxVersionParts.at( 1 ).toInt( &ok );
    if ( !ok )
      return false;
    if ( maxVersionParts.count() == 3 )
    {
      maxVerBugfix = maxVersionParts.at( 2 ).toInt( &ok );
      if ( !ok )
        return false;
    }
  }

  // our qgis version - cut release name after version number
  const QString qgisVersion = Qgis::version().section( '-', 0, 0 );

  const QStringList qgisVersionParts = qgisVersion.split( '.' );

  int qgisMajor = qgisVersionParts.at( 0 ).toInt();
  int qgisMinor = qgisVersionParts.at( 1 ).toInt();
  int qgisBugfix = qgisVersionParts.at( 2 ).toInt();

  if ( qgisMinor == 99 )
  {
    // we want the API version, so for x.99 bump it up to the next major release: e.g. 2.99 to 3.0.0
    qgisMajor++;
    qgisMinor = 0;
    qgisBugfix = 0;
  };

  // build XxYyZz strings with trailing zeroes if needed
  const QString minVer = u"%1%2%3"_s.arg( minVerMajor, 2, 10, QChar( '0' ) ).arg( minVerMinor, 2, 10, QChar( '0' ) ).arg( minVerBugfix, 2, 10, QChar( '0' ) );
  const QString maxVer = u"%1%2%3"_s.arg( maxVerMajor, 2, 10, QChar( '0' ) ).arg( maxVerMinor, 2, 10, QChar( '0' ) ).arg( maxVerBugfix, 2, 10, QChar( '0' ) );
  const QString curVer = u"%1%2%3"_s.arg( qgisMajor, 2, 10, QChar( '0' ) ).arg( qgisMinor, 2, 10, QChar( '0' ) ).arg( qgisBugfix, 2, 10, QChar( '0' ) );

  // compare
  return ( minVer <= curVer && maxVer >= curVer );
}


void QgsPluginRegistry::loadPythonPlugin( const QString &packageName )
{
#ifdef WITH_BINDINGS
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Python is not enabled in QGIS." ), QObject::tr( "Plugins" ) );
    return;
  }

  QgsSettings settings;

  // is loaded already?
  if ( !isLoaded( packageName ) )
  {
    // if plugin is not compatible, disable it
    if ( !isPythonPluginCompatible( packageName ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Plugin \"%1\" is not compatible with this version of QGIS.\nIt will be disabled." ).arg( packageName ), QObject::tr( "Plugins" ) );
      settings.setValue( "/PythonPlugins/" + packageName, false );
      return;
    }

    const QgsScopedRuntimeProfile profile( packageName );
    mPythonUtils->loadPlugin( packageName );
    mPythonUtils->startPlugin( packageName );

    // TODO: test success

    const QString pluginName = mPythonUtils->getPluginMetadata( packageName, u"name"_s );

    // add to settings
    settings.setValue( "/PythonPlugins/" + packageName, true );
    QgsMessageLog::logMessage( QObject::tr( "Loaded %1 (package: %2)" ).arg( pluginName, packageName ), QObject::tr( "Plugins" ), Qgis::MessageLevel::Info );

    settings.remove( "/PythonPlugins/watchDogTimestamp/" + packageName );
  }
#else
  Q_UNUSED( packageName )
#endif
}


void QgsPluginRegistry::loadCppPlugin( const QString &fullPathName )
{
  QgsSettings settings;

  QString baseName = QFileInfo( fullPathName ).baseName();

  // first check to see if it's already loaded
  if ( isLoaded( baseName ) )
  {
    // plugin is loaded
    // QMessageBox::warning(this, "Loading Plugins", description + " is already loaded");
    return;
  }

  const QgsScopedRuntimeProfile profile( baseName );

  QLibrary myLib( fullPathName );

  QString myError; //we will only show detailed diagnostics if something went wrong
  myError += QObject::tr( "Library name is %1\n" ).arg( myLib.fileName() );

  const bool loaded = myLib.load();
  if ( !loaded )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1 (Reason: %2)" ).arg( myLib.fileName(), myLib.errorString() ), QObject::tr( "Plugins" ) );
    return;
  }

  myError += QObject::tr( "Attempting to resolve the classFactory function\n" );

  type_t *pType = ( type_t * ) cast_to_fptr( myLib.resolve( "type" ) );
  name_t *pName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );

  switch ( pType() )
  {
    case QgisPlugin::Renderer:
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
          addPlugin( baseName, QgsPluginMetadata( myLib.fileName(), *pName(), pl ) );
          //add it to the qsettings file [ts]
          settings.setValue( "/Plugins/" + baseName, true );
          QgsMessageLog::logMessage( QObject::tr( "Loaded %1 (Path: %2)" ).arg( *pName(), myLib.fileName() ), QObject::tr( "Plugins" ), Qgis::MessageLevel::Info );

          QObject *o = dynamic_cast<QObject *>( pl );
          if ( o )
          {
            QgsDebugMsgLevel( u"plugin object name: %1"_s.arg( o->objectName() ), 2 );
            if ( o->objectName().isEmpty() )
            {
#ifndef Q_OS_WIN
              baseName = baseName.mid( 3 );
#endif
              QgsDebugMsgLevel( u"object name to %1"_s.arg( baseName ), 2 );
              o->setObjectName( u"qgis_plugin_%1"_s.arg( baseName ) );
              QgsDebugMsgLevel( u"plugin object name now: %1"_s.arg( o->objectName() ), 2 );
            }

            if ( !o->parent() )
            {
              QgsDebugMsgLevel( u"setting plugin parent"_s, 2 );
              o->setParent( QgisApp::instance() );
            }
            else
            {
              QgsDebugMsgLevel( u"plugin parent already set"_s, 2 );
            }
          }

          settings.remove( "/Plugins/watchDogTimestamp/" + baseName );
        }
        else
        {
          // something went wrong
          QMessageBox::warning( mQgisInterface->mainWindow(), QObject::tr( "Loading Plugins" ), QObject::tr( "There was an error loading a plugin. "
                                                                                                             "The following diagnostic information may help the QGIS developers resolve the issue:\n%1." )
                                                                                                  .arg( myError ) );
          //disable it to the qsettings file [ts]
          settings.setValue( "/Plugins/" + baseName, false );
        }
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "Unable to find the class factory for %1." ).arg( fullPathName ), QObject::tr( "Plugins" ) );
      }
    }
    break;
    default:
      // type is unknown
      QgsMessageLog::logMessage( QObject::tr( "Plugin %1 did not return a valid type and cannot be loaded" ).arg( fullPathName ), QObject::tr( "Plugins" ) );
      break;
  }
}


void QgsPluginRegistry::unloadPythonPlugin( const QString &packageName )
{
#ifdef WITH_BINDINGS
  if ( !mPythonUtils || !mPythonUtils->isEnabled() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Python is not enabled in QGIS." ), QObject::tr( "Plugins" ) );
    return;
  }

  if ( isLoaded( packageName ) )
  {
    mPythonUtils->unloadPlugin( packageName );
    QgsDebugMsgLevel( "Python plugin successfully unloaded: " + packageName, 2 );
  }

  // disable the plugin no matter if successfully loaded or not
  QgsSettings settings;
  settings.setValue( "/PythonPlugins/" + packageName, false );
#else
  Q_UNUSED( packageName )
#endif
}


void QgsPluginRegistry::unloadCppPlugin( const QString &fullPathName )
{
  QgsSettings settings;
  const QString baseName = QFileInfo( fullPathName ).baseName();
  settings.setValue( "/Plugins/" + baseName, false );
  if ( isLoaded( baseName ) )
  {
    QgisPlugin *pluginInstance = plugin( baseName );
    if ( pluginInstance )
    {
      pluginInstance->unload();
    }
    // remove the plugin from the registry
    removePlugin( baseName );
    QgsDebugMsgLevel( "Cpp plugin successfully unloaded: " + baseName, 2 );
  }
}


//overloaded version of the next method that will load from multiple directories not just one
void QgsPluginRegistry::restoreSessionPlugins( const QStringList &pluginDirList )
{
  QStringListIterator myIterator( pluginDirList );
  while ( myIterator.hasNext() )
  {
    restoreSessionPlugins( myIterator.next() );
  }
}

void QgsPluginRegistry::restoreSessionPlugins( const QString &pluginDirString )
{
  QgsSettings mySettings;

  const QgsScopedRuntimeProfile profile( QObject::tr( "Load plugins" ) );

#if defined( Q_OS_WIN ) || defined( __CYGWIN__ )
  QString pluginExt = "*.dll";
#elif ANDROID
  QString pluginExt = "*plugin.so";
#else
  const QString pluginExt = u"*.so"_s;
#endif

  // check all libs in the current plugin directory and get name and descriptions
  const QDir myPluginDir( pluginDirString, pluginExt, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks );

  for ( uint i = 0; i < myPluginDir.count(); i++ )
  {
    const QString myFullPath = pluginDirString + '/' + myPluginDir[i];
    if ( checkCppPlugin( myFullPath ) )
    {
      // check if there is a watchdog timestamp left after last session

      bool pluginCrashedPreviously = false;
      const QString baseName = QFileInfo( myFullPath ).baseName();
      const QVariant lastRun = mySettings.value( u"Plugins/watchDogTimestamp/%1"_s.arg( baseName ) );
      if ( lastRun.isValid() )
      {
        if ( QDateTime::currentDateTime().toSecsSinceEpoch() - lastRun.toLongLong() > 5 )
        {
          // The timestamp is left unremoved and is older than 5 seconds, so it's not coming
          // from a parallelly running instance.
          pluginCrashedPreviously = true;
        }
      }

      if ( pluginCrashedPreviously )
      {
        QToolButton *btnEnablePlugin = new QToolButton();
        btnEnablePlugin->setText( QObject::tr( "Enable Plugin" ) );
        btnEnablePlugin->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

        QToolButton *btnIgnore = new QToolButton();
        btnIgnore->setText( QObject::tr( "Ignore" ) );
        btnIgnore->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

        QgsMessageBarItem *watchdogMsg = new QgsMessageBarItem(
          QObject::tr( "Plugin %1" ).arg( baseName ),
          QObject::tr( "This plugin is disabled because it previously crashed QGIS." ),
          btnEnablePlugin,
          Qgis::MessageLevel::Warning,
          0,
          mQgisInterface->messageBar()
        );
        watchdogMsg->layout()->addWidget( btnIgnore );

        QObject::connect( btnEnablePlugin, &QToolButton::clicked, mQgisInterface->messageBar(), [this, baseName, myFullPath, watchdogMsg]() {
          QgsSettings settings;
          settings.setValue( "/Plugins/" + baseName, true );
          loadCppPlugin( myFullPath );
          settings.remove( u"/Plugins/watchDogTimestamp/%1"_s.arg( baseName ) );
          mQgisInterface->messageBar()->popWidget( watchdogMsg );
        } );
        QObject::connect( btnIgnore, &QToolButton::clicked, mQgisInterface->messageBar(), [this, baseName, watchdogMsg]() {
          QgsSettings settings;
          settings.setValue( "/Plugins/" + baseName, false );
          settings.remove( "/Plugins/watchDogTimestamp/" + baseName );
          mQgisInterface->messageBar()->popWidget( watchdogMsg );
        } );

        mQgisInterface->messageBar()->pushItem( watchdogMsg );
        mySettings.setValue( "/Plugins/" + baseName, false );
      }
      if ( mySettings.value( "/Plugins/" + baseName ).toBool() )
      {
        mySettings.setValue( u"Plugins/watchDogTimestamp/%1"_s.arg( baseName ), QDateTime::currentDateTime().toSecsSinceEpoch() );
        loadCppPlugin( myFullPath );
        mySettings.remove( u"/Plugins/watchDogTimestamp/%1"_s.arg( baseName ) );
      }
    }
  }

#ifdef WITH_BINDINGS
  if ( mPythonUtils && mPythonUtils->isEnabled() )
  {
    // check for python plugins system-wide
    const QStringList pluginList = mPythonUtils->pluginList();
    QgsDebugMsgLevel( u"Loading python plugins"_s, 2 );
    QgsDebugMsgLevel( u"Python plugins will be loaded in the following order: "_s + pluginList.join( "," ), 2 );

    QStringList corePlugins = QStringList();
    corePlugins << u"db_manager"_s;
    corePlugins << u"processing"_s;
    corePlugins << u"MetaSearch"_s;
    corePlugins << u"grassprovider"_s;

    // make the required core plugins enabled by default:
    const auto constCorePlugins = corePlugins;
    for ( const QString &corePlugin : constCorePlugins )
    {
      if ( !mySettings.contains( "/PythonPlugins/" + corePlugin ) )
      {
        mySettings.setValue( "/PythonPlugins/" + corePlugin, true );
      }
    }

    const auto constPluginList = pluginList;
    for ( const QString &packageName : constPluginList )
    {
      // TODO: apply better solution for #5879
      // start - temporary fix for issue #5879
      if ( QgsApplication::isRunningFromBuildDir() )
      {
        if ( corePlugins.contains( packageName ) )
        {
          QgsApplication::setPkgDataPath( QString() );
        }
        else
        {
          QgsApplication::setPkgDataPath( QgsApplication::buildSourcePath() );
        }
      }
      // end - temporary fix for issue #5879, more below

      bool pluginCrashedPreviously = false;
      const QVariant lastRun = mySettings.value( u"/PythonPlugins/watchDogTimestamp/%1"_s.arg( packageName ) );
      if ( lastRun.isValid() )
      {
        if ( QDateTime::currentDateTime().toSecsSinceEpoch() - lastRun.toLongLong() > 5 )
        {
          // The timestamp is left unremoved and is older than 5 seconds, so it's not coming
          // from a parallelly running instance.
          pluginCrashedPreviously = true;
        }
      }

      if ( pluginCrashedPreviously )
      {
        QToolButton *btnEnablePlugin = new QToolButton();
        btnEnablePlugin->setText( QObject::tr( "Enable Plugin" ) );
        btnEnablePlugin->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

        QToolButton *btnIgnore = new QToolButton();
        btnIgnore->setText( QObject::tr( "Ignore" ) );
        btnIgnore->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );

        QgsMessageBarItem *watchdogMsg = new QgsMessageBarItem(
          QObject::tr( "Plugin %1" ).arg( packageName ),
          QObject::tr( "This plugin is disabled because it previously crashed QGIS." ),
          btnEnablePlugin,
          Qgis::MessageLevel::Warning,
          0,
          mQgisInterface->messageBar()
        );
        watchdogMsg->layout()->addWidget( btnIgnore );

        QObject::connect( btnEnablePlugin, &QToolButton::clicked, mQgisInterface->messageBar(), [this, packageName, watchdogMsg]() {
          QgsSettings settings;
          settings.setValue( "/PythonPlugins/" + packageName, true );
          if ( checkPythonPlugin( packageName ) )
          {
            loadPythonPlugin( packageName );
          }
          settings.remove( "/PythonPlugins/watchDogTimestamp/" + packageName );

          mQgisInterface->messageBar()->popWidget( watchdogMsg );
        } );

        QObject::connect( btnIgnore, &QToolButton::clicked, mQgisInterface->messageBar(), [this, packageName, watchdogMsg]() {
          QgsSettings settings;
          settings.setValue( "/PythonPlugins/" + packageName, false );
          settings.remove( "/PythonPlugins/watchDogTimestamp/" + packageName );
          mQgisInterface->messageBar()->popWidget( watchdogMsg );
        } );

        mQgisInterface->messageBar()->pushItem( watchdogMsg );

        mySettings.setValue( "/PythonPlugins/" + packageName, false );
      }
      // check if the plugin was active on last session
      if ( mySettings.value( "/PythonPlugins/" + packageName ).toBool() )
      {
        mySettings.setValue( "/PythonPlugins/watchDogTimestamp/" + packageName, QDateTime::currentDateTime().toSecsSinceEpoch() );
        if ( checkPythonPlugin( packageName ) )
        {
          loadPythonPlugin( packageName );
        }
        mySettings.remove( "/PythonPlugins/watchDogTimestamp/" + packageName );
      }
    }
    // start - temporary fix for issue #5879, more above
    if ( QgsApplication::isRunningFromBuildDir() )
    {
      QgsApplication::setPkgDataPath( QgsApplication::buildOutputPath() + u"/data"_s );
    }
    // end - temporary fix for issue #5879
  }
#endif

  QgsDebugMsgLevel( u"Plugin loading completed"_s, 2 );
}


bool QgsPluginRegistry::checkCppPlugin( const QString &pluginFullPath )
{
  QLibrary myLib( pluginFullPath );
  const bool loaded = myLib.load();
  if ( !loaded )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1 (Reason: %2)" ).arg( myLib.fileName(), myLib.errorString() ), QObject::tr( "Plugins" ) );
    return false;
  }

  name_t *myName = ( name_t * ) cast_to_fptr( myLib.resolve( "name" ) );
  description_t *myDescription = ( description_t * ) cast_to_fptr( myLib.resolve( "description" ) );
  category_t *myCategory = ( category_t * ) cast_to_fptr( myLib.resolve( "category" ) );
  version_t *myVersion = ( version_t * ) cast_to_fptr( myLib.resolve( "version" ) );

  if ( myName && myDescription && myVersion && myCategory )
    return true;

  QgsDebugMsgLevel( "Failed to get name, description, category or type for " + myLib.fileName(), 2 );
  return false;
}


bool QgsPluginRegistry::checkPythonPlugin( const QString &packageName )
{
#ifdef WITH_BINDINGS
  QString pluginName, description, /*category,*/ version;

  // get information from the plugin
  // if there are some problems, don't continue with metadata retrieval
  pluginName = mPythonUtils->getPluginMetadata( packageName, u"name"_s );
  description = mPythonUtils->getPluginMetadata( packageName, u"description"_s );
  version = mPythonUtils->getPluginMetadata( packageName, u"version"_s );
  // for Python plugins category still optional, by default used "Plugins" category
  //category = mPythonUtils->getPluginMetadata( packageName, "category" );

  if ( pluginName == "__error__"_L1 || description == "__error__"_L1 || version == "__error__"_L1 )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error when reading metadata of plugin %1" ).arg( packageName ), QObject::tr( "Plugins" ) );
    return false;
  }

  return true;
#else
  Q_UNUSED( packageName )
  return false;
#endif
}

bool QgsPluginRegistry::isPythonPluginCompatible( const QString &packageName ) const
{
#ifdef WITH_BINDINGS
  bool supportsQgis4 = true;
  const QString supportsQt6 = mPythonUtils->getPluginMetadata( packageName, u"supportsQt6"_s ).trimmed();
  if ( supportsQt6.compare( "YES"_L1, Qt::CaseInsensitive ) != 0 && supportsQt6.compare( "TRUE"_L1, Qt::CaseInsensitive ) != 0 )
  {
    if ( !getenv( "QGIS_DISABLE_SUPPORTS_QT6_CHECK" ) )
    {
      return false;
    }
    supportsQgis4 = false;
  }
  const QString minVersion = mPythonUtils->getPluginMetadata( packageName, u"qgisMinimumVersion"_s );
  // try to read qgisMaximumVersion. Note checkQgisVersion can cope with "__error__" value.
  QString maxVersion = mPythonUtils->getPluginMetadata( packageName, u"qgisMaximumVersion"_s );
  if ( maxVersion == "__error__"_L1 && minVersion.startsWith( "3."_L1 ) && supportsQgis4 )
  {
    maxVersion = "4.99.0"_L1;
  }
  return minVersion != "__error__"_L1 && checkQgisVersion( minVersion, maxVersion );
#else
  Q_UNUSED( packageName )
  return false;
#endif
}

QList<QgsPluginMetadata *> QgsPluginRegistry::pluginData()
{
  QList<QgsPluginMetadata *> resultList;
  QMap<QString, QgsPluginMetadata>::iterator it = mPlugins.begin();
  for ( ; it != mPlugins.end(); ++it )
  {
    resultList.push_back( &( it.value() ) );
  }
  return resultList;
}
