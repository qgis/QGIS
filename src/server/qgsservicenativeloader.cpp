/***************************************************************************
                          qgsservicerenativeloader.cpp

  Define Loader for native service modules
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QLibrary>
#include <QDir>
#include <QDebug>

#include "qgsservicenativeloader.h"
#include "qgsservicemodule.h"
#include "qgsmessagelog.h"
#include "qgis.h"


typedef void unloadHook_t( QgsServiceModule * );

/**
 * \ingroup server
 * \class QgsServiceNativeModuleEntry
 * \brief Native module (location, the module itself and the unload function).
 * \since QGIS 3.0
 */
class QgsServiceNativeModuleEntry
{
  public:

    /**
     * Constructor for QgsServiceNativeModuleEntry.
     * \param location Relative path of the module
     */
    QgsServiceNativeModuleEntry( const QString &location )
      : mLocation( location )
    {}

    QString mLocation;
    QgsServiceModule *mModule = nullptr;
    unloadHook_t *mUnloadHook = nullptr;
};

void QgsServiceNativeLoader::loadModules( const QString &modulePath, QgsServiceRegistry &registrar,
    QgsServerInterface *serverIface )
{
  QDir moduleDir( modulePath );
  moduleDir.setSorting( QDir::Name | QDir::IgnoreCase );
  moduleDir.setFilter( QDir::Files );

#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  moduleDir.setNameFilters( QStringList( "*.dll" ) );
#else
  moduleDir.setNameFilters( QStringList( "*.so" ) );
#endif

  // qDebug() << QString( "Checking %1 for native services modules" ).arg( moduleDir.path() );

  const auto constFiList( moduleDir.entryInfoList() );
  for ( const QFileInfo &fi : constFiList )
  {
    QgsServiceModule *module = loadNativeModule( fi.filePath() );
    if ( module )
    {
      // Register services
      module->registerSelf( registrar, serverIface );
    }
  }
}


typedef QgsServiceModule *serviceEntryPoint_t();

QgsServiceModule *QgsServiceNativeLoader::loadNativeModule( const QString &location )
{
  QgsServiceNativeModuleEntry *entry = findModuleEntry( location );
  if ( entry )
  {
    return entry->mModule;
  }

  QLibrary lib( location );
  //QgsDebugMsg( QStringLiteral( "Loading native module %1" ).arg( location ) );
  qDebug() << QString( "Loading native module %1" ).arg( location );
  if ( !lib.load() )
  {
    QgsMessageLog::logMessage( QString( "Failed to load library %1: %2" ).arg( lib.fileName(), lib.errorString() ) );
    return nullptr;
  }
  // Load entry point
  serviceEntryPoint_t *
  entryPointFunc = reinterpret_cast<serviceEntryPoint_t *>( cast_to_fptr( lib.resolve( "QGS_ServiceModule_Init" ) ) );

  if ( entryPointFunc )
  {
    QgsServiceModule *module = entryPointFunc();
    if ( module )
    {
      entry = new QgsServiceNativeModuleEntry( location );
      entry->mModule     = module;
      entry->mUnloadHook = reinterpret_cast<unloadHook_t *>( cast_to_fptr( lib.resolve( "QGS_ServiceModule_Exit" ) ) );

      // Add entry
      mModules.insert( location, ModuleTable::mapped_type( entry ) );
      return module;
    }
    else
    {
      QgsMessageLog::logMessage( QString( "No entry point for module %1" ).arg( lib.fileName() ) );
    }
  }
  else
  {
    QgsMessageLog::logMessage( QString( "Error: entry point returned null for %1" ).arg( lib.fileName() ) );
  }

  // No module found: release library
  lib.unload();
  return nullptr;
}

void QgsServiceNativeLoader::unloadModules()
{
  ModuleTable::iterator it  = mModules.begin();
  const ModuleTable::iterator end = mModules.end();

  while ( it != end )
  {
    unloadModuleEntry( it->get() );
    ++it;
  }

  mModules.clear();
}

QgsServiceNativeModuleEntry *QgsServiceNativeLoader::findModuleEntry( const QString &location )
{
  QgsServiceNativeModuleEntry *entry = nullptr;
  const ModuleTable::iterator item = mModules.find( location );
  if ( item != mModules.end() )
  {
    entry = item->get();
  }
  return entry;
}

void QgsServiceNativeLoader::unloadModuleEntry( QgsServiceNativeModuleEntry *entry )
{
  // Call cleanup function if it exists
  if ( entry->mUnloadHook )
  {
    entry->mUnloadHook( entry->mModule );
  }

  QLibrary lib( entry->mLocation );
  lib.unload();
}
