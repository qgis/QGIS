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
#include "qgsmaprenderer.h"
#include "qgsconfigcache.h"
#include "qgscapabilitiescache.h"

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsserverplugins.h"
#include "qgsserverfilter.h"
#include "qgsserverinterfaceimpl.h"
#endif


/**
 * The QgsServer class provides OGC web services.
 */
class  SERVER_EXPORT QgsServer
{
  public:
    QgsServer();
    ~QgsServer();
    /** Server initialisation: intialise QGIS ang QT core application.
     * This method is automatically called by handleRequest if it wasn't
     * explicitly called before */
    static bool init( int & argc, char ** argv );
    //! The following is mainly for python bindings, that do not pass argc/argv
    static bool init();

    /** Handle the request. The output is normally printed trough FCGI printf
     * by the request handler or, in case the server has been invoked from python
     * bindings, a flag is set that capures all the output headers and body, instead
     * of printing it returns the output as a QByteArray.
     * When calling handleRequest() from python bindings an additional argument
     * specify if we only want the headers or the body back, this is mainly useful
     * for testing purposes.
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable
     *
     * @param queryString optional QString containing the query string
     * @return the response QByteArray if called from python bindings, empty otherwise
     */
    QByteArray handleRequest( const QString queryString = QString( ) );
    QByteArray handleRequest( const QString queryString,
                              const bool returnHeaders,
                              const bool returnBody );
    /**
     * Handles the request and returns only the body
     *
     * @param queryString optional QString containing the query string
     * @return the response body QByteArray if called from python bindings, empty otherwise
     */
    QByteArray handleRequestGetBody( const QString queryString = QString( ) );

    /**
     * Handles the request and returns only the headers
     *
     * @param queryString optional QString containing the query string
     * @return the response headers QByteArray if called from python bindings, empty otherwise
     */
    QByteArray handleRequestGetHeaders( const QString queryString = QString( ) );

    /** Returns a pointer to the server interface */
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerInterfaceImpl* serverInterface( ) { return mServerInterface; }
#endif

  private:
    // All functions that where previously in the main file are now
    // static methods of this class
    static QString configPath( const QString& defaultConfigPath,
                               const QMap<QString, QString>& parameters );
    // Mainly for debug
    static void dummyMessageHandler( QtMsgType type, const char *msg );
    // Mainly for debug
    static void printRequestInfos();
    // Mainly for debug
    static void printRequestParameters(
      const QMap< QString, QString>& parameterMap,
      int logLevel );
    static QFileInfo defaultProjectFile();
    static QFileInfo defaultAdminSLD();
    static void setupNetworkAccessManager();
    //! Create and return a request handler instance
    static QgsRequestHandler* createRequestHandler(
      const bool captureOutput = FALSE );

    // Server status
    static QString mConfigFilePath;
    static QgsCapabilitiesCache* mCapabilitiesCache;
    static QgsMapRenderer* mMapRenderer;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    static QgsServerInterfaceImpl* mServerInterface;
    static bool mInitPython;
#endif
    static bool mInitialised;
    static QString mServerName;
    static char* mArgv[1];
    static int mArgc;
    static QgsApplication* mQgsApplication;
    static bool mCaptureOutput;
};
#endif // QGSSERVER_H

