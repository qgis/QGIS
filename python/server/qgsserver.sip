/***************************************************************************
                         qgsserver.sip
  QGIS Server main class.
                        -------------------
  begin                : 2015-05-21
  copyright            : (C) 2015 by Alessandro Pasotti
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


/** \ingroup server
 * The QgsServer class provides OGC web services.
 */
class QgsServer
{
%TypeHeaderCode
#include "qgsserver.h"
%End

  public:
    /** Creates the server instance
     * @param captureOutput set to false for stdout output (FCGI)
     */
    QgsServer();
    ~QgsServer();

    /** Set environment variable
     * @param var environment variable name
     * @param val value
     * @note added in 2.14
     */
    void putenv( const QString &var, const QString &val );

   /** Handles the request.
     * The query string is normally read from environment
     * but can be also passed in args and in this case overrides the environment
     * variable
     *
     * @param request a QgsServerRequest holding request parameters
     * @param response a QgsServerResponse for handling response I/O)
     */
    void handleRequest( QgsServerRequest &request, QgsServerResponse &response );


    /** Returns a pointer to the server interface */
    QgsServerInterface *serverInterface();

  private:
    QgsServer( const QgsServer& );
    QgsServer &operator=( const QgsServer& );
};

