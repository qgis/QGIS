/***************************************************************************
                          qgsserver.h
  QGIS Server main class.
                        -------------------
  begin                : June 05, 2015
  copyright            : (C) 2015 by Alessandro Pasotti
  email                : a dot pasotti at itopen dot it

  Based on previous work from:

  begin                : July 04, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch

  ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSERVER_H
#define QGSSERVER_H

#include <QFileInfo>
#include "qgsrequesthandler.h"
#include "qgsapplication.h"
#include "qgsconfigcache.h"
#include "qgscapabilitiescache.h"
#include "qgsmapsettings.h"
#include "qgsmessagelog.h"
#include "qgsserviceregistry.h"
#include "qgsserversettings.h"
#include "qgsserverplugins.h"
#include "qgsserverfilter.h"
#include "qgsserverinterfaceimpl.h"
#include "qgis_server.h"
#include "qgsserverrequest.h"

class QgsServerResponse;
class QgsProject;

/** \ingroup server
 * The QgsServer class provides OGC web services.
 */
class SERVER_EXPORT QgsServer
{
  public:

    /** Creates the server instance
     */
    QgsServer();

    /** Set environment variable
     * \param var environment variable name
     * \param val value
     * \since QGIS 2.14
     */
    void putenv( const QString &var, const QString &val );

    /** Handles the request.
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable
     *
     * \param request a QgsServerRequest holding request parameters
     * \param response a QgsServerResponse for handling response I/O)
     */
    void handleRequest( QgsServerRequest &request, QgsServerResponse &response );

    /** Handles the request from query string
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable.
     *
     * \param urlstr QString containing the request url (simple quely string must be preceded by '?')
     * \param requestMethod QgsServerRequest::Method that indicates the method. Only "GET" or "POST" are supported.
     * \param data array of bytes containing post data
     * \param map of request headers
     * \returns the response headers and body QPair of QByteArray
     */
    QPair<QByteArray, QByteArray> handleRequest( const QString &urlstr, const QgsServerRequest::Method requestMethod = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers(), const char *data = nullptr );

    //! Returns a pointer to the server interface
    QgsServerInterfaceImpl *serverInterface() { return sServerInterface; }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    //! Initialize Python
    //! Note: not in Python bindings
    void initPython( );
#endif

  private:

    //! Server initialization
    static bool init();

    // All functions that where previously in the main file are now
    // static methods of this class
    static QString configPath( const QString &defaultConfigPath,
                               const QMap<QString, QString> &parameters );

    /**
     * \brief QgsServer::printRequestParameters prints the request parameters
     * \param parameterMap
     * \param logLevel
     */
    static void printRequestParameters(
      const QMap< QString, QString> &parameterMap,
      QgsMessageLog::MessageLevel logLevel );

    static QFileInfo defaultProjectFile();
    static QFileInfo defaultAdminSLD();
    static void setupNetworkAccessManager();
    //! Create and return a request handler instance
    static QgsRequestHandler *createRequestHandler( const QgsServerRequest &request, QgsServerResponse &response );

    // Return the server name
    static QString &serverName();

    // Status
    static QString *sConfigFilePath;
    static QgsCapabilitiesCache *sCapabilitiesCache;
    static QgsServerInterfaceImpl *sServerInterface;
    //! Initialization must run once for all servers
    static bool sInitialized;

    //! service registry
    static QgsServiceRegistry sServiceRegistry;

    static QgsServerSettings sSettings;

    // map of QgsProject
    QMap<QString, const QgsProject *> mProjectRegistry;
};
#endif // QGSSERVER_H

