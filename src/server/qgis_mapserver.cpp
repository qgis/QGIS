/***************************************************************************
                              qgs_mapserver.cpp
 A server application supporting WMS / WFS / WCS
                              -------------------
  begin                : Jan 17 2020
  copyright            : (C) 2020by Alessandro Pasotti
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

//for CMAKE_INSTALL_PREFIX
#include "qgsconfig.h"
#include "qgsserver.h"
#include "qgsbufferserverrequest.h"
#include "qgsbufferserverresponse.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "http-parser/http_parser.h"

#include <QFontDatabase>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QObject>


#include <string>

int main( int argc, char *argv[] )
{
  // Test if the environ variable DISPLAY is defined
  // if it's not, the server is running in offscreen mode
  // Qt supports using various QPA (Qt Platform Abstraction) back ends
  // for rendering. You can specify the back end to use with the environment
  // variable QT_QPA_PLATFORM when invoking a Qt-based application.
  // Available platform plugins are: directfbegl, directfb, eglfs, linuxfb,
  // minimal, minimalegl, offscreen, wayland-egl, wayland, xcb.
  // https://www.ics.com/blog/qt-tips-and-tricks-part-1
  // http://doc.qt.io/qt-5/qpa.html
  const char *display = getenv( "DISPLAY" );
  bool withDisplay = true;
  if ( !display )
  {
    withDisplay = false;
    qputenv( "QT_QPA_PLATFORM", "offscreen" );
    QgsMessageLog::logMessage( "DISPLAY not set, running in offscreen mode, all printing capabilities will not be available.", "Server", Qgis::Info );
  }
  // since version 3.0 QgsServer now needs a qApp so initialize QgsApplication
  QgsApplication app( argc, argv, withDisplay, QString(), QStringLiteral( "server" ) );
  QgsServer server;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  server.initPython();
#endif

#ifdef Q_OS_WIN
  // Initialize font database before fcgi_accept.
  // When using FCGI with IIS, environment variables (QT_QPA_FONTDIR in this case) are lost after fcgi_accept().
  QFontDatabase fontDB;
#endif

  QTcpServer tcpServer;

  if ( !tcpServer.listen( QHostAddress::Any, 8081 ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to start the server: %1." )
                               .arg( tcpServer.errorString() ) );
    tcpServer.close();
  }
  else
  {
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for ( int i = 0; i < ipAddressesList.size(); ++i )
    {
      if ( ipAddressesList.at( i ) != QHostAddress::LocalHost &&
           ipAddressesList.at( i ).toIPv4Address() )
      {
        ipAddress = ipAddressesList.at( i ).toString();
        break;
      }
    }
    // if we did not find one, use IPv4 localhost
    if ( ipAddress.isEmpty() )
      ipAddress = QHostAddress( QHostAddress::LocalHost ).toString();
    QgsMessageLog::logMessage( QObject::tr( "QGIS Server is listening on %1:%2\n" )
                               .arg( ipAddress ).arg( tcpServer.serverPort() ) );



    // Starts HTTP loop with a poor man's HTTP parser
    tcpServer.connect( &tcpServer, &QTcpServer::newConnection, [ & ]
    {
      QTcpSocket *clientConnection = tcpServer.nextPendingConnection();
      QgsMessageLog::logMessage( QObject::tr( "Incoming connection %1 from %2:%3" )
                                 .arg( clientConnection->peerName() )
                                 .arg( clientConnection->peerAddress().toString() )
                                 .arg( clientConnection->peerPort() ) );

      clientConnection->connect( clientConnection, &QAbstractSocket::disconnected,
                                 clientConnection, &QObject::deleteLater );

      clientConnection->connect( clientConnection, &QIODevice::readyRead, [ =, &server ] {
        const QString incomingData { clientConnection->readAll() };
        QgsMessageLog::logMessage( QObject::tr( "Incoming data: %1" ).arg( incomingData ) );

        // Parse protocol and URL GET /path HTTP/1.1
        int firstLinePos { incomingData.indexOf( "\r\n" ) };
        // TODO: err if -1
        const QString firstLine { incomingData.left( firstLinePos ) };
        const QStringList firstLinePieces { firstLine.split( ' ' ) };
        // TODO: check pieces are not 3
        const QString methodString { firstLinePieces.at( 0 ) };

        QgsServerRequest::Method method;
        if ( methodString == "GET" )
        {
          method = QgsServerRequest::Method::GetMethod;
        }
        else if ( methodString == "POST" )
        {
          method = QgsServerRequest::Method::PostMethod;
        }
        else if ( methodString == "HEAD" )
        {
          method = QgsServerRequest::Method::HeadMethod;
        }
        else if ( methodString == "PUT" )
        {
          method = QgsServerRequest::Method::PutMethod;
        }
        else if ( methodString == "PATCH" )
        {
          method = QgsServerRequest::Method::PatchMethod;
        }
        else if ( methodString == "DELETE" )
        {
          method = QgsServerRequest::Method::DeleteMethod;
        }
        else
        {
          // TODO: err if not known
        }

        const QString url { firstLinePieces.at( 1 )};
        const QString protocol { firstLinePieces.at( 2 )};
        // TODO: err if not HTTP/1.0

        // Headers
        QgsBufferServerRequest::Headers headers;
        int endHeadersPos { incomingData.indexOf( "\r\n\r\n" ) };
        // TODO: err if -1
        const QStringList httpHeaders { incomingData.mid( firstLinePos + 2, endHeadersPos - firstLinePos ).split( "\r\n" ) };

        for ( const auto &headerLine : httpHeaders )
        {
          const int headerColonPos { headerLine.indexOf( ':' ) };
          if ( headerColonPos > 0 )
          {
            headers.insert( headerLine.left( headerColonPos ), headerLine.mid( headerColonPos + 1 ) );
          }
        }

        // Inefficient copy :(
        QByteArray data { incomingData.mid( endHeadersPos + 2 ).toUtf8() };
        QgsBufferServerRequest request { url, method, headers, &data };
        QgsBufferServerResponse response;

        server.handleRequest( request, response );

        static const QMap<int, QString> knownStatuses {
          { 200, QStringLiteral( "OK" ) },
          { 201, QStringLiteral( "Created" ) },
          { 202, QStringLiteral( "Accepted" ) },
          { 204, QStringLiteral( "No Content" ) },
          { 301, QStringLiteral( "Moved Permanently" ) },
          { 302, QStringLiteral( "Moved Temporarily" ) },
          { 304, QStringLiteral( "Not Modified" ) },
          { 400, QStringLiteral( "Bad Request" ) },
          { 401, QStringLiteral( "Unauthorized" ) },
          { 403, QStringLiteral( "Forbidden" ) },
          { 404, QStringLiteral( "Not Found" ) },
          { 500, QStringLiteral( "Internal Server Error" ) },
          { 501, QStringLiteral( "Not Implemented" ) },
          { 502, QStringLiteral( "Bad Gateway" ) },
          { 503, QStringLiteral( "Service Unavailable" ) }
        };

        // Output stream
        clientConnection->write( QStringLiteral( "HTTP/1.0 %1 \r\n" ).arg( response.statusCode() ).arg( knownStatuses.value( response.statusCode() ) ).toUtf8() );
        clientConnection->write( QStringLiteral( "Server: QGIS\r\n" ).toUtf8() );
        const auto responseHeaders { response.headers() };
        for ( auto it = responseHeaders.constBegin(); it != responseHeaders.constEnd(); ++it )
        {
          clientConnection->write( QStringLiteral( "%1: %2\r\n" ).arg( it.key(), it.value() ).toUtf8() );
        }
        clientConnection->write( "\r\n" );
        clientConnection->write( response.body() );
        clientConnection->disconnectFromHost();
      } );

    } );

  }
  app.exec();
  app.exitQgis();
  return 0;
}

