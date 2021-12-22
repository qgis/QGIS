/***************************************************************************
  qgsserverapicontext.h - QgsServerApiContext

 ---------------------
 begin                : 13.5.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVERAPICONTEXT_H
#define QGSSERVERAPICONTEXT_H

#include "qgis_server.h"
#include <QString>

class QgsServerResponse;
class QgsServerRequest;
class QgsServerInterface;
class QgsProject;

/**
 * \ingroup server
 * \brief The QgsServerApiContext class encapsulates the resources for a particular client
 * request: the request and response objects, the project (might be NULL) and
 * the server interface, the API root path that matched the request is also added.
 *
 * QgsServerApiContext is lightweight copyable object meant to be passed along the
 * request handlers chain.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiContext
{
  public:

    /**
    * QgsServerApiContext constructor
    *
    * \param apiRootPath is the API root path, this information is used by the
    *        handlers to build the href links to the resources and to the HTML templates.
    * \param request the incoming request
    * \param response the response
    * \param project the project (might be NULL)
    * \param serverInterface the server interface
    */
    QgsServerApiContext( const QString &apiRootPath, const QgsServerRequest *request, QgsServerResponse *response,
                         const QgsProject *project, QgsServerInterface *serverInterface );

    /**
     * Returns the server request object
     */
    const QgsServerRequest *request() const;

    /**
     * Returns the server response object
     */
    QgsServerResponse *response() const;

    /**
     * Returns the (possibly NULL) project
     * \see setProject()
     */
    const QgsProject *project() const;

    /**
     * Sets the project to \a project
     * \see project()
     */
    void setProject( const QgsProject *project );

    /**
     * Returns the server interface
     */
    QgsServerInterface *serverInterface() const;

    /**
     * Returns the initial part of the incoming request URL path that matches the
     * API root path.
     * If there is no match returns an empty string (it should never happen).
     *
     * I.e. for an API with root path "/wfs3" and an incoming request
     * "https://www.qgis.org/services/wfs3/collections"
     * this method will return "/resources/wfs3"
     *
     */
    const QString matchedPath( ) const;

    /**
     * Returns the API root path
     */
    QString apiRootPath() const;

    /**
     * Sets context request to \a request
     */
    void setRequest( const QgsServerRequest *request );

    /**
     * Returns the handler component of the URL path, i.e. the part of the path that comes
     * after the API path.
     *
     * \since QGIS 3.22
     */
    QString handlerPath( ) const;

  private:

    QString mApiRootPath;
    const QgsServerRequest *mRequest = nullptr;
    QgsServerResponse *mResponse = nullptr;
    const QgsProject *mProject = nullptr;
    QgsServerInterface *mServerInterface = nullptr;
};

#endif // QGSSERVERAPICONTEXT_H
