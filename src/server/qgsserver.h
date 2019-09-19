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
#include "qgsconfigcache.h"
#include "qgscapabilitiescache.h"
#include "qgsserviceregistry.h"
#include "qgsserversettings.h"
#include "qgsserverplugins.h"
#include "qgsserverinterfaceimpl.h"
#include "qgis_server.h"
#include "qgsserverrequest.h"

class QgsServerResponse;
class QgsProject;

/**
 * \ingroup server
 * The QgsServer class provides OGC web services.
 */
class SERVER_EXPORT QgsServer
{
  public:

    /**
     * Creates the server instance
     */
    QgsServer();

    /**
     * Set environment variable
     * \param var environment variable name
     * \param val value
     * \since QGIS 2.14
     */
    void putenv( const QString &var, const QString &val );

    /**
     * Handles the request.
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable
     *
     * \param request a QgsServerRequest holding request parameters
     * \param response a QgsServerResponse for handling response I/O)
     * \param project a QgsProject or NULLPTR, if it is NULLPTR the project
     *        is created from the MAP param specified in request or from
     *        the QGIS_PROJECT_FILE setting
     */
    void handleRequest( QgsServerRequest &request, QgsServerResponse &response, const QgsProject *project = nullptr );


    //! Returns a pointer to the server interface
    QgsServerInterfaceImpl SIP_PYALTERNATIVETYPE( QgsServerInterface ) *serverInterface() { return sServerInterface; }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

    /**
     * Initialize Python
     * \note not available in Python bindings
     */
    void initPython();
#endif

  private:
#ifdef SIP_RUN
    QgsServer( const QgsServer & );
    QgsServer &operator=( const QgsServer & );
#endif

    //! Server initialization
    static bool init();

    /**
     * Returns the configuration file path.
     */
    static QString configPath( const QString &defaultConfigPath,
                               const QString &configPath );

    /**
     * \brief QgsServer::printRequestParameters prints the request parameters
     * \param parameterMap
     * \param logLevel
     */
    static void printRequestParameters(
      const QMap< QString, QString> &parameterMap,
      Qgis::MessageLevel logLevel );

    /**
     * Returns the default project file.
     */
    static QFileInfo defaultProjectFile();
    static QFileInfo defaultAdminSLD();

    /**
     * \brief QgsServer::setupNetworkAccessManager
     */
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
    static QgsServiceRegistry *sServiceRegistry;

    static QgsServerSettings sSettings;

    //! cache
    QgsConfigCache *mConfigCache = nullptr;

    //! Initialize locale
    static void initLocale();
};
#endif // QGSSERVER_H
