/***************************************************************************
                          qgsseerversinterface.h
 Interface class for exposing functions in QGIS Server for use by plugins
                             -------------------
  begin                : 2014-09-10
  copyright            : (C) 2014 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsserverinterfaceimpl.h"
#include "qgsconfigcache.h"

//! Constructor
QgsServerInterfaceImpl::QgsServerInterfaceImpl( QgsCapabilitiesCache *capCache, QgsServiceRegistry *srvRegistry, QgsServerSettings *settings )
  : mCapabilitiesCache( capCache )
  , mServiceRegistry( srvRegistry )
  , mServerSettings( settings )
{
  mRequestHandler = nullptr;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mAccessControls = new QgsAccessControl();
  mCacheManager = new QgsServerCacheManager( *settings );
#endif
}

QString QgsServerInterfaceImpl::getEnv( const QString &name ) const
{
  return getenv( name.toLocal8Bit() );
}


QgsServerInterfaceImpl::~QgsServerInterfaceImpl()
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  delete mAccessControls;
  delete mCacheManager;
#endif
}


void QgsServerInterfaceImpl::clearRequestHandler()
{
  mRequestHandler = nullptr;
}

void QgsServerInterfaceImpl::setRequestHandler( QgsRequestHandler *requestHandler )
{
  mRequestHandler = requestHandler;
}

void QgsServerInterfaceImpl::setConfigFilePath( const QString &configFilePath )
{
  mConfigFilePath = configFilePath;
}

void QgsServerInterfaceImpl::registerFilter( QgsServerFilter *filter, int priority )
{
  mFilters.insert( priority, filter );
}

void QgsServerInterfaceImpl::setFilters( QgsServerFiltersMap *filters )
{
  mFilters = *filters;
}

//! Register a new access control filter
void QgsServerInterfaceImpl::registerAccessControl( QgsAccessControlFilter *accessControl, int priority )
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mAccessControls->registerAccessControl( accessControl, priority );
#else
  Q_UNUSED( accessControl )
  Q_UNUSED( priority )
#endif
}

//! Register a new access control filter
void QgsServerInterfaceImpl::registerServerCache( QgsServerCacheFilter *serverCache, int priority )
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mCacheManager->registerServerCache( serverCache, priority );
#else
  Q_UNUSED( serverCache )
  Q_UNUSED( priority )
#endif
}

QgsServerCacheManager *QgsServerInterfaceImpl::cacheManager() const
{
  return mCacheManager;
}

void QgsServerInterfaceImpl::removeConfigCacheEntry( const QString &path )
{
  if ( mCapabilitiesCache )
  {
    mCapabilitiesCache->removeCapabilitiesDocument( path );
  }
  QgsConfigCache::instance()->removeEntry( path );
}

QgsServiceRegistry *QgsServerInterfaceImpl::serviceRegistry()
{
  return mServiceRegistry;
}

QgsServerSettings *QgsServerInterfaceImpl::serverSettings()
{
  return mServerSettings;
}
