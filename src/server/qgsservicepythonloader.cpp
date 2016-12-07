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

#include "qgsservicepythonloader.h"
#include "qgsserviceregistry.h"
#include "qgsmessagelog.h"
#include <QLibrary> 
#include <QDir>

typedef void unloadHook_t( QgsServiceModule* );

class QgsServicePythonModuleEntry
{
    public:
        QgsServicePythonModuleEntry( const QString& location )
        : mLocation(location)
        , mModule(nullptr)
        {}

        QString           mLocation;
        QgsServiceModule* mModule;
        unloadHook_t*     mUnloadHook;
};


//! Constructor
QgsServicePythonLoader::QgsServicePythonLoader()
{
}

//! Destructor
QgsServicePythonLoader::~QgsServicePythonLoader()
{

}

void QgsServicePythonLoader::loadModules( const QString& modulePath, QgsServiceRegistry& registrar )
{
    //TODO
}


void QgsServicePythonLoader::unloadModules()
{
    ModuleTable::iterator it  = mModules.begin();
    ModuleTable::iterator end = mModules.end();

    while( it!=end ) 
    {
        unloadModuleEntry( it->get() );
        ++it;
    }

    mModules.clear();
}


void QgsServicePythonLoader::unloadModuleEntry( QgsServicePythonModuleEntry* entry )
{
   // Call cleanup function if it exists
   if( entry->mUnloadHook ) {
       entry->mUnloadHook( entry->mModule );
   }
}


