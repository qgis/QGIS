/***************************************************************************
                              qgs_mapserver.cpp

A QGIS development HTTP server for testing/development purposes.
The server listens to localhost:8000, the address and port can be changed with the
environment variable QGIS_SERVER_ADDRESS and QGIS_SERVER_PORT or passing <address>:<port>
on the command line.

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

#include <QFontDatabase>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QObject>

#ifndef Q_OS_WIN
#include <csignal>
#endif

#include <string>
#include <chrono>

class HttpException: public std::exception
{

  public:

    HttpException( const QString &message )
      : mMessage( message )
    {
    }

    QString message( )
    {
      return mMessage;
    }

  private:

    QString mMessage;

};

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
  QgsApplication app( argc, argv, withDisplay, QString(), QStringLiteral( "QGIS Development Server" ) );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( "QGIS Development Server" );


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

  // The port to listen
  QString serverPort { getenv( "QGIS_SERVER_PORT" ) };
  // The address to listen
  QString ipAddress { getenv( "QGIS_SERVER_ADDRESS" ) };

  if ( serverPort.isEmpty() )
  {
    serverPort = QStringLiteral( "8000" );
  }

  if ( ipAddress.isEmpty() )
  {
    ipAddress = QStringLiteral( "localhost" );
  }

  // Override from command line
  if ( argc == 2 )
  {
    const QString arg{ argv[1] };
    if ( arg == QStringLiteral( "-h" ) )
    {
      std::cout << QObject::tr( "Usage: %1 [-h] [ADDRESS:PORT]\n"
                                "Example: %1 localhost:8000\n\n"
                                "Default: localhost:8000\n\n"
                                "The following environment variables can be used:\n"
                                "QGIS_SERVER_PORT: server port\n"
                                "QGIS_SERVER_ADDRESS: server address\n" ).arg( basename( argv[0] ) ).toStdString() << std::endl;
      exit( 0 );
    }
    else
    {
      QStringList addressAndPort { arg.split( ':' ) };
      if ( addressAndPort.size() == 2 )
      {
        ipAddress = addressAndPort.at( 0 );
        serverPort = addressAndPort.at( 1 );
      }
    }
  }

  QHostAddress address { QHostAddress::AnyIPv4 };
  address.setAddress( ipAddress );

  if ( ! tcpServer.listen( address, serverPort.toInt( ) ) )
  {
    std::cerr << QObject::tr( "Unable to start the server: %1." )
              .arg( tcpServer.errorString() ).toStdString();
    tcpServer.close();
  }
  else
  {

    const int port { tcpServer.serverPort() };
    std::cout << QObject::tr( "QGIS Development Server listening on http://%1:%2" )
              .arg( ipAddress ).arg( port ).toStdString() << std::endl;

#ifndef Q_OS_WIN
    std::cout << QObject::tr( "CTRL+C to exit" ).toStdString() << std::endl;
#endif

    static const QMap<int, QString> knownStatuses
    {
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

    // Starts HTTP loop with a poor man's HTTP parser
    tcpServer.connect( &tcpServer, &QTcpServer::newConnection, [ & ]
    {
      QTcpSocket *clientConnection = tcpServer.nextPendingConnection();

      clientConnection->connect( clientConnection, &QAbstractSocket::disconnected,
                                 clientConnection, &QObject::deleteLater );

      // Incoming connection parser
      clientConnection->connect( clientConnection, &QIODevice::readyRead, [ =, &server ] {

        QString incomingData;
        while ( clientConnection->bytesAvailable() > 0 )
        {
          incomingData += clientConnection->readAll();
        }

        try
        {

          // Parse protocol and URL GET /path HTTP/1.1
          int firstLinePos { incomingData.indexOf( "\r\n" ) };
          if ( firstLinePos == -1 )
          {
            throw HttpException( QStringLiteral( "HTTP error finding protocol header" ) );
          }

          const QString firstLine { incomingData.left( firstLinePos ) };
          const QStringList firstLinePieces { firstLine.split( ' ' ) };
          if ( firstLinePieces.size() != 3 )
          {
            throw HttpException( QStringLiteral( "HTTP error splitting protocol header" ) );
          }

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
            throw HttpException( QStringLiteral( "HTTP error unsupported method: %1" ).arg( methodString ) );
          }

          // Build URL from env ...
          QString url { getenv( "REQUEST_URI" ) };
          // ... or from server ip/port and request path
          if ( url.isEmpty() )
          {
            const QString path { firstLinePieces.at( 1 )};
            url = QStringLiteral( "http://%1:%2%3" ).arg( ipAddress ).arg( port ).arg( path );
          }

          const QString protocol { firstLinePieces.at( 2 )};
          if ( protocol != QStringLiteral( "HTTP/1.0" ) && protocol != QStringLiteral( "HTTP/1.1" ) )
          {
            throw HttpException( QStringLiteral( "HTTP error unsupported protocol: %1" ).arg( protocol ) );
          }

          // Headers
          QgsBufferServerRequest::Headers headers;
          int endHeadersPos { incomingData.indexOf( "\r\n\r\n" ) };

          if ( endHeadersPos == -1 )
          {
            throw HttpException( QStringLiteral( "HTTP error finding headers" ) );
          }

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
          QByteArray data { incomingData.mid( endHeadersPos + 4 ).toUtf8() };

          auto start = std::chrono::steady_clock::now();

          QgsBufferServerRequest request { url, method, headers, &data };
          QgsBufferServerResponse response;

          server.handleRequest( request, response );

          auto elapsedTime { std::chrono::steady_clock::now() - start };

          if ( ! knownStatuses.contains( response.statusCode() ) )
          {
            throw HttpException( QStringLiteral( "HTTP error unsupported status code: %1" ).arg( response.statusCode() ) );
          }

          // Output stream
          clientConnection->write( QStringLiteral( "HTTP/1.0 %1 %2\r\n" ).arg( response.statusCode() ).arg( knownStatuses.value( response.statusCode() ) ).toUtf8() );
          clientConnection->write( QStringLiteral( "Server: QGIS\r\n" ).toUtf8() );
          const auto responseHeaders { response.headers() };
          for ( auto it = responseHeaders.constBegin(); it != responseHeaders.constEnd(); ++it )
          {
            clientConnection->write( QStringLiteral( "%1: %2\r\n" ).arg( it.key(), it.value() ).toUtf8() );
          }
          clientConnection->write( "\r\n" );
          const QByteArray body { response.body() };
          clientConnection->write( body );

          // 10.185.248.71 [09/Jan/2015:19:12:06 +0000] 808840 <time> "GET / HTTP/1.1" 500"
          std::cout << QStringLiteral( "%1 [%2] %3 %4ms \"%5\" %6" )
                    .arg( clientConnection->peerAddress().toString() )
                    .arg( QDateTime::currentDateTime().toString() )
                    .arg( body.size() )
                    .arg( std::chrono::duration_cast<std::chrono::milliseconds>( elapsedTime ).count() )
                    .arg( firstLinePieces.join( ' ' ) )
                    .arg( response.statusCode() ).toStdString() << std::endl;

        }
        catch ( HttpException &ex )
        {
          // Output stream: send error
          clientConnection->write( QStringLiteral( "HTTP/1.0 %1 %2\r\n" ).arg( 500 ).arg( knownStatuses.value( 500 ) ).toUtf8() );
          clientConnection->write( QStringLiteral( "Server: QGIS\r\n" ).toUtf8() );
          clientConnection->write( "\r\n" );
          clientConnection->write( ex.message().toUtf8() );

          std::cout << QStringLiteral( "%1 [%2] \"%3\" - - 500" )
                    .arg( clientConnection->peerAddress().toString() )
                    .arg( QDateTime::currentDateTime().toString() )
                    .arg( ex.message() ).toStdString() << std::endl;

        }
        clientConnection->disconnectFromHost();
      } );

    } );

  }

  // Exit handlers
#ifndef Q_OS_WIN
  signal( SIGTERM, []( int ) { qApp->quit(); } );
  signal( SIGABRT, []( int ) { qApp->quit(); } );
  signal( SIGINT, []( int ) { qApp->quit(); } );
  signal( SIGKILL, []( int ) { qApp->quit(); } );
#endif

  app.exec();
  app.exitQgis();
  return 0;
}

