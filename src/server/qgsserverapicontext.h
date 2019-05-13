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

class QgsServerResponse;
class QgsServerRequest;
class QgsServerInterface;
class QgsProject;

/**
 * The QgsServerApiContext class encapsulates the resources for a particular client request:
 * the request and response objects, the project (might be null) and the server interface.
 *
 * QgsServerApiContext is lightweight copyable object meant to be passed along the request handlers chain.
 *
 */
class SERVER_EXPORT QgsServerApiContext
{
  public:

    /**
    * QgsServerApiContext constructor
    * \param request the incoming request
    * \param response the response
    * \param project the project (might be null)
    * \param serverInterface the server interface
    */
    QgsServerApiContext( const QgsServerRequest *request, QgsServerResponse *response, const QgsProject *project, QgsServerInterface *serverInterface );

    const QgsServerRequest *request() const;

    QgsServerResponse *response() const;

    const QgsProject *project() const;
    void setProject( const QgsProject *project );

    QgsServerInterface *serverInterface() const;

  private:

    const QgsServerRequest *mRequest = nullptr;
    QgsServerResponse *mResponse = nullptr;
    const QgsProject *mProject = nullptr;
    QgsServerInterface *mServerInterface = nullptr;
};

#endif // QGSSERVERAPICONTEXT_H
