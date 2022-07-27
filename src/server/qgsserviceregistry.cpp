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
#include "qgsserverapi.h"
#include "qgsmessagelog.h"

#include <algorithm>
#include <functional>


namespace
{

// Build a key entry from name and version
  QString makeServiceKey( const QString &name, const QString &version )
  {
    return QString( "%1_%2" ).arg( name, version );
  }

// Compare two version strings:
// The strings are split into dot separated segment
// Each segment are compared up to the shortest number of segment of the
// lists. Remaining segments are dropped.
// If both segments can be interpreted as numbers the are compared as numbers, otherwise
// They are compared lexicographically.
// Return true if v1 is greater than v2
  bool isVersionGreater( const QString &v1, const QString &v2 )
  {
    QStringList l1 = v1.split( '.' );
    QStringList l2 = v2.split( '.' );
    QStringList::iterator it1 = l1.begin();
    QStringList::iterator it2 = l2.begin();
    bool isint;
    while ( it1 != l1.end() && it2 != l2.end() )
    {
      if ( *it1 != *it2 )
      {
        // Compare as numbers
        const int i1 = it1->toInt( &isint );
        if ( isint )
        {
          const int i2 = it2->toInt( &isint );
          if ( isint && i1 != i2 )
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

// Check that two versions are c


} // namespace

QgsServiceRegistry::~QgsServiceRegistry()
{
  cleanUp();
}

QgsService *QgsServiceRegistry::getService( const QString &name, const QString &version )
{
  QgsService *service = nullptr;
  QString key;

  // Check that we have a service of that name
  const VersionTable::const_iterator v = mServiceVersions.constFind( name );
  if ( v != mServiceVersions.constEnd() )
  {
    key = version.isEmpty() ? v->second : makeServiceKey( name, version );
    const ServiceTable::const_iterator it = mServices.constFind( key );
    if ( it != mServices.constEnd() )
    {
      service = it->get();
    }
    else
    {
      // Return the default version
      QgsMessageLog::logMessage( QString( "Service %1 %2 not found, returning default" ).arg( name, version ),
                                 QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
      service = mServices[v->second].get();
    }
  }
  else
  {
    QgsMessageLog::logMessage( QString( "Service %1 is not registered" ).arg( name ),
                               QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
  }
  return service;
}

void QgsServiceRegistry::registerService( QgsService *service )
{
  const QString name    = service->name();
  const QString version = service->version();

  // Test if service is already registered
  const QString key = makeServiceKey( name, version );
  if ( mServices.constFind( key ) != mServices.constEnd() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Error Service %1 %2 is already registered" ).arg( name, version ),
                               QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    return;
  }

  QgsMessageLog::logMessage( QStringLiteral( "Adding service %1 %2" ).arg( name, version ),
                             QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  mServices.insert( key, std::shared_ptr<QgsService>( service ) );

  // Check the default version
  // The first inserted service of a given name
  // is the default one.
  // this will ensure that native services are always
  // the defaults.
  const VersionTable::const_iterator v = mServiceVersions.constFind( name );
  if ( v == mServiceVersions.constEnd() )
  {
    // Insert the service as the default one
    mServiceVersions.insert( name, VersionTable::mapped_type( version, key ) );
  }
  /*
  if ( v != mVersions.constEnd() )
  {
    if ( isVersionGreater( version, v->first ) )
    {
      // Replace the default version key
      mVersions.insert( name, VersionTable::mapped_type( version, key ) );
    }
  }
  else
  {
    // Insert the service as the default one
    mVersions.insert( name, VersionTable::mapped_type( version, key ) );
  }*/

}

int QgsServiceRegistry::unregisterApi( const QString &name, const QString &version )
{

  // Check that we have an API of that name
  int removed = 0;
  const VersionTable::const_iterator v = mApiVersions.constFind( name );
  if ( v != mApiVersions.constEnd() )
  {
    if ( version.isEmpty() )
    {
      // No version specified, remove all versions
      ApiTable::iterator it = mApis.begin();
      while ( it != mApis.end() )
      {
        if ( ( *it )->name() == name )
        {
          QgsMessageLog::logMessage( QString( "Unregistering API %1 %2" ).arg( name, ( *it )->version() ),
                                     QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
          it = mApis.erase( it );
          ++removed;
        }
        else
        {
          ++it;
        }
      }
      // Remove from version table
      mApiVersions.remove( name );
    }
    else
    {
      const QString key = makeServiceKey( name, version );
      const ApiTable::iterator found = mApis.find( key );
      if ( found != mApis.end() )
      {
        QgsMessageLog::logMessage( QString( "Unregistering API %1 %2" ).arg( name, version ),
                                   QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
        mApis.erase( found );
        removed = 1;

        // Find if we have other services of that name
        // but with different version
        //
        QString maxVer;
        const std::function < void ( const ApiTable::mapped_type & ) >
        findGreaterVersion = [name, &maxVer]( const ApiTable::mapped_type & api )
        {
          if ( api->name() == name &&
               ( maxVer.isEmpty() || isVersionGreater( api->version(), maxVer ) ) )
            maxVer = api->version();
        };

        mApiVersions.remove( name );

        std::for_each( mApis.constBegin(), mApis.constEnd(), findGreaterVersion );
        if ( !maxVer.isEmpty() )
        {
          // Set the new default service
          const QString key = makeServiceKey( name, maxVer );
          mApiVersions.insert( name, VersionTable::mapped_type( version, key ) );
        }
      }
    }
  }
  return removed;
}

QgsServerApi *QgsServiceRegistry::apiForRequest( const QgsServerRequest &request ) const
{
  for ( const auto &api : mApis )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Trying URL path: '%1' for '%2'" ).arg( request.url().path(), api->rootPath() ),
                               QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    if ( api->accept( request.url() ) )
    {
      Q_ASSERT( !api->name().isEmpty() );
      QgsMessageLog::logMessage( QStringLiteral( "API %1 accepts the URL path '%2' " ).arg( api->name(), request.url().path() ),
                                 QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      return api.get();
    }
  }
  return nullptr;
}

QgsServerApi *QgsServiceRegistry::getApi( const QString &name, const QString &version )
{
  QgsServerApi *api = nullptr;
  QString key;

  // Check that we have an API with that name
  const VersionTable::const_iterator v = mApiVersions.constFind( name );
  if ( v != mApiVersions.constEnd() )
  {
    key = version.isEmpty() ? v->second : makeServiceKey( name, version );
    const ApiTable::const_iterator it = mApis.constFind( key );
    if ( it != mApis.constEnd() )
    {
      api = it->get();
    }
    else
    {
      // Return the default version
      QgsMessageLog::logMessage( QString( "API %1 %2 not found, returning default" ).arg( name, version ),
                                 QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
      api = mApis[v->second].get();
    }
  }
  else
  {
    QgsMessageLog::logMessage( QString( "API %1 is not registered" ).arg( name ),
                               QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
  }
  return api;
}

int QgsServiceRegistry::unregisterService( const QString &name, const QString &version )
{
  // Check that we have a service of that name
  int removed = 0;
  const VersionTable::const_iterator v = mServiceVersions.constFind( name );
  if ( v != mServiceVersions.constEnd() )
  {
    if ( version.isEmpty() )
    {
      // No version specified, remove all versions
      ServiceTable::iterator it = mServices.begin();
      while ( it != mServices.end() )
      {
        if ( ( *it )->name() == name )
        {
          QgsMessageLog::logMessage( QString( "Unregistering service %1 %2" ).arg( name, ( *it )->version() ),
                                     QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
          it = mServices.erase( it );
          ++removed;
        }
        else
        {
          ++it;
        }
      }
      // Remove from version table
      mServiceVersions.remove( name );
    }
    else
    {
      const QString key = makeServiceKey( name, version );
      const ServiceTable::iterator found = mServices.find( key );
      if ( found != mServices.end() )
      {
        QgsMessageLog::logMessage( QString( "Unregistering service %1 %2" ).arg( name, version ),
                                   QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
        mServices.erase( found );
        removed = 1;

        // Find if we have other services of that name
        // but with different version
        //
        QString maxVer;
        const std::function < void ( const ServiceTable::mapped_type & ) >
        findGreaterVersion = [name, &maxVer]( const ServiceTable::mapped_type & service )
        {
          if ( service->name() == name &&
               ( maxVer.isEmpty() || isVersionGreater( service->version(), maxVer ) ) )
            maxVer = service->version();
        };

        mServiceVersions.remove( name );

        std::for_each( mServices.constBegin(), mServices.constEnd(), findGreaterVersion );
        if ( !maxVer.isEmpty() )
        {
          // Set the new default service
          const QString key = makeServiceKey( name, maxVer );
          mServiceVersions.insert( name, VersionTable::mapped_type( version, key ) );
        }
      }
    }
  }
  return removed;
}

void QgsServiceRegistry::init( const QString &nativeModulePath, QgsServerInterface *serverIface )
{
  mNativeLoader.loadModules( nativeModulePath, *this, serverIface );
}

void QgsServiceRegistry::cleanUp()
{
  // Release all services
  mServiceVersions.clear();
  mServices.clear();
  mApis.clear();
  mNativeLoader.unloadModules();
}

bool QgsServiceRegistry::registerApi( QgsServerApi *api )
{

  const QString name = api->name();
  const QString version = api->version();

  // Test if service is already registered
  const QString key = makeServiceKey( name, version );
  if ( mApis.constFind( key ) != mApis.constEnd() )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Error API %1 %2 is already registered" ).arg( name, version ),
                               QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    return false;
  }

  QgsMessageLog::logMessage( QStringLiteral( "Adding API %1 %2" ).arg( name, version ), QString(), Qgis::MessageLevel::Info );
  mApis.insert( key, std::shared_ptr<QgsServerApi>( api ) );

  // Check the default version
  // The first inserted service of a given name
  // is the default one.
  // this will ensure that native services are always
  // the defaults.
  const VersionTable::const_iterator v = mApiVersions.constFind( name );
  if ( v == mApiVersions.constEnd() )
  {
    // Insert the service as the default one
    mApiVersions.insert( name, VersionTable::mapped_type( version, key ) );
  }
  return true;
}
