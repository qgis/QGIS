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
#include "qgsmslayercache.h"

/** Constructor */
QgsServerInterfaceImpl::QgsServerInterfaceImpl( QgsCapabilitiesCache* capCache )
    : mCapabilitiesCache( capCache )
{
  mRequestHandler = nullptr;
  mAccessControls = new QgsAccessControl();
}


QString QgsServerInterfaceImpl::getEnv( const QString& name ) const
{
  return getenv( name.toLocal8Bit() );
}


/** Destructor */
QgsServerInterfaceImpl::~QgsServerInterfaceImpl()
{
  delete mAccessControls;
}


void QgsServerInterfaceImpl::clearRequestHandler()
{
  mRequestHandler = nullptr;
}

void QgsServerInterfaceImpl::setRequestHandler( QgsRequestHandler * requestHandler )
{
  mRequestHandler = requestHandler;
}

void QgsServerInterfaceImpl::setConfigFilePath( const QString& configFilePath )
{
  mConfigFilePath = configFilePath;
}

void QgsServerInterfaceImpl::registerFilter( QgsServerFilter *filter, int priority )
{
  mFilters.insert( priority, filter );
}

void QgsServerInterfaceImpl::setFilters( QgsServerFiltersMap* filters )
{
  mFilters = *filters;
}

/** Register a new access control filter */
void QgsServerInterfaceImpl::registerAccessControl( QgsAccessControlFilter* accessControl, int priority )
{
  mAccessControls->registerAccessControl( accessControl, priority );
}


void QgsServerInterfaceImpl::removeConfigCacheEntry( const QString& path )
{
  if ( mCapabilitiesCache )
  {
    mCapabilitiesCache->removeCapabilitiesDocument( path );
  }
  QgsConfigCache::instance()->removeEntry( path );
}

void QgsServerInterfaceImpl::removeProjectLayers( const QString& path )
{
  QgsMSLayerCache::instance()->removeProjectLayers( path );
}



