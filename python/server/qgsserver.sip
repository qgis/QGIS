/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserver.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsServer
{
%Docstring
 The QgsServer class provides OGC web services.
%End

%TypeHeaderCode
#include "qgsserver.h"
%End
  public:

    QgsServer();
%Docstring
 Creates the server instance
%End

    void putenv( const QString &var, const QString &val );
%Docstring
 Set environment variable
 \param var environment variable name
 \param val value
.. versionadded:: 2.14
%End

    void handleRequest( QgsServerRequest &request, QgsServerResponse &response );
%Docstring
 Handles the request.
 The query string is normally read from environment
 but can be also passed in args and in this case overrides the environment
 variable

 \param request a QgsServerRequest holding request parameters
 \param response a QgsServerResponse for handling response I/O)
%End


    QgsServerInterface  *serverInterface();
%Docstring
Returns a pointer to the server interface
 :rtype: QgsServerInterfaceImpl
%End

    void initPython();
%Docstring
Note: not in Python bindings
%End

  private:
    QgsServer( const QgsServer & );
    QgsServer &operator=( const QgsServer & );
%Docstring
 :rtype: QgsServer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserver.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
