/***************************************************************************
                          qgsserviceregistry.cpp

  Class defining the service manager for QGIS server services.
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

#include "qgsserviceregistry.h"
#include "qgsservice.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

namespace {

    // Build a key entry from name and version
    QString make_service_key( const QString& name, const QString& version  )
    {
        return QString( "%1_%2" ).arg(name,version);
    }

    // Compare two version strings:
    // The strings are splitted into dot separated segment
    // Each segment are compared up to the shortest number of segment of the 
    // lists. Remaining segments are dropped.
    // If both segments can be intepreted as numbers the are compared as numbers, otherwise 
    // They are compared lexicographically.
    // Return true if v1 is greater than v2
    bool is_version_greater( const QString& v1, const QString& v2 )
    {
        QStringList l1 = v1.split('.');
        QStringList l2 = v2.split('.');
        QStringList::iterator it1 = l1.begin();
        QStringList::iterator it2 = l2.begin();
        bool isint;
        while( it1 != l1.end() && it2 != l2.end() ) 
        {
            if ( *it1 != *it2 )
            {
                // Compare as numbers
                int i1 = it1->toInt(&isint);
                if(isint) 
                {
                    int i2 = it2->toInt(&isint);
                    if( isint && i1 != i2 )
                    {
                        return i1 > i2;
                    }
                }
                // Compare lexicographically
                if ( !isint )
                {
                    return *it1 > *it2;
                }
            }
            ++it1;
            ++it2;
        }
        // We reach the end of one of the list 
        return false;
    }
    
} // namespace

class QgsServiceEntry
{
    public:
       ~QgsServiceEntry() 
        {
            // We have the ownership by design
            // XXX Take care of /Transfer/ decorator for registerService in sip 
            QgsDebugMsg( QString("Deleting service %1 %2").arg(mName, mVersion) );
            delete mService;
        }

       QgsServiceEntry( const QString& name, QgsService* service, const QString& version )
       : mName(name)
       , mService(service)
       , mVersion(version)
       {}

       QString     mName;
       QgsService* mService;
       QString     mVersion;    
};

QgsServiceRegistry::QgsServiceRegistry()
{
    //TODO
}

QgsServiceRegistry::~QgsServiceRegistry()
{
    cleanUp();
}

QgsService* QgsServiceRegistry::getService( const QString& name, const QString& version )
{
    QgsService* service = nullptr;
    QString key;

    // Check that we have a service of that name
    VersionTable::iterator v = mVersions.find(name);
    if( v != mVersions.end() )
    { 
        key = version.isEmpty() ? v->second : make_service_key(name, version );
        ServiceTable::iterator it = mServices.find(key);
        if( it != mServices.end() )
        {
            service = (*it)->mService;
        }
        else
        {
            QgsMessageLog::logMessage( QString("Service %1 %2 not found").arg(name, version) );
        }
    }
    else
    {
        QgsMessageLog::logMessage( QString("Service %1 is not registered").arg(name) );
    }
    return service;  
}

void QgsServiceRegistry::registerService( const QString& name,  QgsService* service, const QString& version )
{
    // Test if service is already registered
    QString key = make_service_key( name, version );
    if( mServices.find(key) != mServices.end() )
    {
        QgsMessageLog::logMessage( QString("Error Service %1 %2 is already registered").arg(name,version) );
        return;    
    }

    QgsMessageLog::logMessage( QString( "Adding service %1 %2").arg(name,version) );
    mServices.insert( key, std::make_shared<QgsServiceEntry>( name, service, version ) ); 

    // Check the default version
    // and replace with te new one if it has a higher version
    VersionTable::iterator v = mVersions.find( name );
    if( v != mVersions.end() )
    {
        if( is_version_greater( version, v->first ) )
        {
            // Replace the default version key
            mVersions.insert( name, VersionTable::mapped_type( version, key ) );
        } 
    }
    else 
    {
        // Insert the service as the default one
        mVersions.insert( name, VersionTable::mapped_type( version, key ) );
    }
}

void QgsServiceRegistry::init( const QString& nativeModulePath, const QString& pythonModulePath )
{
    mNativeLoader.loadModules(nativeModulePath, *this );
    #ifdef HAVE_SERVER_PYTHON_SERVICES
    mPythonLoader.loadModules(pythonModulePath, *this );        
    #endif
}

void QgsServiceRegistry::cleanUp()
{
    // Release all services
    mServices.clear();     

    mNativeLoader.unloadModules();
    #ifdef HAVE_SERVER_PYTHON_SERVICES
    mPythonLoader.unloadModules();        
    #endif
}


