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


QgsServerInterfaceImpl::QgsServerInterfaceImpl( QgsCapabilitiesCache* capCache ) :
    mCapabilitiesCache( capCache )
{
  mRequestHandler = NULL;
}


QString QgsServerInterfaceImpl::getEnv( const QString& name ) const
{
  return getenv( name.toLocal8Bit() );
}


/** Destructor */
QgsServerInterfaceImpl::~QgsServerInterfaceImpl()
{
}


void QgsServerInterfaceImpl::clearRequestHandler( )
{
  mRequestHandler = NULL;
}

void QgsServerInterfaceImpl::setRequestHandler( QgsRequestHandler * requestHandler )
{
  mRequestHandler = requestHandler;
}

void QgsServerInterfaceImpl::setConfigFilePath( QString configFilePath )
{
  mConfigFilePath = configFilePath;
}

void QgsServerInterfaceImpl::registerFilter( QgsServerFilter *filter, int priority )
{
  mFilters.insert( priority, filter );
}
