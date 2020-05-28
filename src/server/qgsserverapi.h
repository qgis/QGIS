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
 * Server generic API endpoint abstract base class.
 *
 * \see QgsServerOgcApi for an OGC API (aka WFS3) implementation.
 *
 * An API must have a name and a (possibly empty) version and define a
 * (possibly empty) root path (e.g. "/wfs3").
 *
 * The server routing logic will check incoming request URLs by passing them
 * to the API's accept(url) method, the default implementation performs a simple
 * check for the presence of the API's root path string in the URL.
 * This simple logic implies that APIs must be registered in reverse order from the
 * most specific to the most generic: given two APIs with root paths '/wfs' and '/wfs3',
 * '/wfs3' must be registered first or it will be shadowed by '/wfs'.
 * APIs developers are encouraged to implement a more robust accept(url) logic by
 * making sure that their APIs accept only URLs they can actually handle, if they do,
 * the APIs registration order becomes irrelevant.
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
     * Returns TRUE if the given \a url is handled by the API, default implementation checks for the presence of rootPath inside the \a url path.
     */
    virtual bool accept( const QUrl &url ) const;

    /**
     * Executes a request by passing the given \a context to the API handlers.
     */
    virtual void executeRequest( const QgsServerApiContext &context ) const = 0;

    /**
     * Returns the server interface
     */
    QgsServerInterface *serverIface() const;

  private:

    QgsServerInterface *mServerIface = nullptr;
};


#endif // QGSSERVERAPI_H


