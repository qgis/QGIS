/***************************************************************************
                          qgsserverapi.h

  Class defining the service interface for QGIS server APIs.
  -------------------
  begin                : 2019-04-16
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef QGSSERVERAPI_H
#define QGSSERVERAPI_H

#include "qgis_server.h"
#include <QRegularExpression>
#include "qgsserverexception.h"
#include "qgsserverrequest.h"

class QgsServerResponse;
class QgsProject;

/**
 * Server API endpoint abstract base class
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApi
{

  public:

    QgsServerApi( ) = default;
    virtual ~QgsServerApi() = default;

    /**
     * \returns the name of the API
     */
    virtual const QString name() const = 0;

    /**
     * \returns the version of the service
     */
    virtual const QString version() const = 0;

    /**
     * \returns the root path for the API
     */
    virtual const QString rootPath() const = 0;

    /**
     * Returns TRUE if the given method is supported by the API, default implementation supports all methods.
     */
    virtual bool allowMethod( QgsServerRequest::Method ) const { return true; }

    /**
     * Execute the requests and set result in QgsServerResponse
     */
    virtual void executeRequest( const QgsServerRequest &request,
                                 QgsServerResponse &response, const QgsProject *project ) const = 0;


};


#endif // QGSSERVERAPI_H
