/***************************************************************************
                          qgsseerversinterface.h
 Interface class for exposing functions in Qgis Server for use by plugins
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

#ifndef QGSSERVERINTERFACE_H
#define QGSSERVERINTERFACE_H

#include "qgscapabilitiescache.h"
#include "qgsrequesthandler.h"
#include "qgsserverfilter.h"

/**
 * QgsServerInterface
 * Class defining interfaces exposed by Qgis Mapserver and
 * made available to plugins.
 *
 */

class SERVER_EXPORT QgsServerInterface
{

  public:

    /** Constructor */
    QgsServerInterface( );

    /** Destructor */
    virtual ~QgsServerInterface() = 0;

    virtual void setRequestHandler( QgsRequestHandler* requestHandler ) = 0;
    virtual QgsCapabilitiesCache* capabiblitiesCache() = 0;
    virtual QgsRequestHandler* requestHandler( ) = 0;
    virtual void registerFilter( QgsServerFilter* filter, int priority = 0 ) = 0;
    virtual QgsServerFiltersMap filters( ) = 0;
    /*Pass  environment variables to python*/
    virtual QString getEnv( const QString& name ) const = 0;

};

#endif // QGSSERVERINTERFACE_H
