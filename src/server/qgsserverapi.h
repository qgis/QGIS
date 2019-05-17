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
class QgsServerApiContext;
class QgsServerInterface;

/**
 * \ingroup server
 * Server API endpoint abstract base class.
 *
 * An API must have a name and a (possibly empty) version and define a
 * (possibly empty) root path (e.g. "/wfs3").
 *
 * After the API has been registered to the server API registry:
 *
 * \code{.py}
 *   class API(QgsServerApi):
 *
 *     def name(self):
 *       return "Test API"
 *
 *     def rootPath(self):
 *       return "/testapi"
 *
 *     def executeRequest(self, request_context):
 *       request_context.response().write(b"\"Test API\"")
 *
 *   server = QgsServer()
 *   api = API(server.serverInterface())
 *   server.serverInterface().serviceRegistry().registerApi(api)
 * \endcode
 *
 * the incoming calls with an URL path starting with the API root path
 * will be routed to the first matching API and executeRequest() method
 * of the API will be invoked.
 *
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApi
{

  public:

    /**
     * Creates a QgsServerApi object
     */
    QgsServerApi( QgsServerInterface *serverIface );

    virtual ~QgsServerApi() = default;

    /**
     * Returns the API name
     */
    virtual const QString name() const = 0;

    /**
     * Returns the API description
     */
    virtual const QString description() const = 0;

    /**
     * Returns the version of the service
     * \note the default implementation returns an empty string
     */
    virtual const QString version() const { return QString(); }

    /**
     * Returns the root path for the API
     */
    virtual const QString rootPath() const = 0;

    /**
     * Returns TRUE if the given method is supported by the API, default implementation supports all methods.
     */
    virtual bool allowMethod( QgsServerRequest::Method ) const { return true; }

    /**
     * Executes a request by passing the given \a context to the handlers.
     */
    virtual void executeRequest( QgsServerApiContext *context ) const = 0;

    /**
     * Returns the server interface
     */
    QgsServerInterface *serverIface() const;

  private:

    QgsServerInterface *mServerIface = nullptr;
};


#endif // QGSSERVERAPI_H


