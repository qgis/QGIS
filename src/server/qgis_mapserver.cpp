/***************************************************************************
                              qgs_mapserver.cpp

A QGIS development HTTP server for testing/development purposes.
The server listens to localhost:8000, the address and port can be changed with the
environment variable QGIS_SERVER_ADDRESS and QGIS_SERVER_PORT or passing <address>:<port>
on the command line.

All requests and application messages are printed to the standard output,
while QGIS server internal logging is printed to stderr.

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
#include <QCommandLineParser>
#include <QObject>


#ifndef Q_OS_WIN
#include <csignal>
#endif

#include <string>
#include <chrono>

///@cond PRIVATE

/**
 * The HttpException class represents an HTTP parsing exception.
 */
class HttpException: public std::exception
{

  public:

    /**
     * Constructs an HttpException with the given \a message
     */
    HttpException( const QString &message )
      : mMessage( message )
    {
    }

    /**
     * Returns the exception message.
     */
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
  const QString display { qgetenv( "DISPLAY" ) };
  bool withDisplay = true;
  if ( display.isEmpty() )
  {
    withDisplay = false;
    qputenv( "QT_QPA_PLATFORM", "offscreen" );
  }

  // since version 3.0 QgsServer now needs a qApp so initialize QgsApplication
  QgsApplication app( argc, argv, withDisplay, QString(), QStringLiteral( "QGIS Development Server" ) );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( "QGIS Development Server" );
  QCoreApplication::setApplicationVersion( "1.0" );

  if ( ! withDisplay )
  {
    QgsMessageLog::logMessage( "DISPLAY environment variable is not set, running in offscreen mode, all printing capabilities will not be available.\n"
                               "Consider installing an X server like 'xvfb' and export DISPLAY to the actual display value.", "Server", Qgis::Warning );
  }

#ifdef Q_OS_WIN
  // Initialize font database before fcgi_accept.
  // When using FCGI with IIS, environment variables (QT_QPA_FONTDIR in this case) are lost after fcgi_accept().
  QFontDatabase fontDB;
#endif

  // The port to listen
  QString serverPort { qgetenv( "QGIS_SERVER_PORT" ) };
  // The address to listen
  QString ipAddress { qgetenv( "QGIS_SERVER_ADDRESS" ) };

  if ( serverPort.isEmpty() )
  {
    serverPort = QStringLiteral( "8000" );
  }

  if ( ipAddress.isEmpty() )
  {
    ipAddress = QStringLiteral( "localhost" );
  }

  QCommandLineParser parser;
  parser.setApplicationDescription( QObject::tr( "QGIS Development Server" ) );
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument( QStringLiteral( "addressAndPort" ),
                                QObject::tr( "Address and port (default: \"localhost:8000\")\n"
                                    "address and port can also be specified with the environment\n"
                                    "variables QGIS_SERVER_ADDRESS and QGIS_SERVER_PORT." ), QStringLiteral( "[address:port]" ) );
  QCommandLineOption logLevelOption( "l", QObject::tr( "Log level (default: 0)\n"
                                     "0: INFO\n"
                                     "1: WARNING\n"
                                     "2: CRITICAL" ), "logLevel", "0" );
  parser.addOption( logLevelOption );

  QCommandLineOption projectOption( "p", QObject::tr( "Path to a QGIS project file (*.qgs or *.qgz),\n"
                                    "if specified it will override the query string MAP argument\n"
                                    "and the QGIS_PROJECT_FILE environment variable." ), "projectPath", "" );
  parser.addOption( projectOption );

  parser.process( app );
  const QStringList args = parser.positionalArguments();

  if ( args.size() == 1 )
  {
    QStringList addressAndPort { args.at( 0 ).split( ':' ) };
    if ( addressAndPort.size() == 2 )
    {
      ipAddress = addressAndPort.at( 0 );
      serverPort = addressAndPort.at( 1 );
    }
  }

  QString logLevel = parser.value( logLevelOption );
  qunsetenv( "QGIS_SERVER_LOG_FILE" );
  qputenv( "QGIS_SERVER_LOG_LEVEL", logLevel.toUtf8() );
  qputenv( "QGIS_SERVER_LOG_STDERR", "1" );

  if ( ! parser.value( projectOption ).isEmpty( ) )
  {
    // Check it!
    const QString projectFilePath { parser.value( projectOption ) };
    if ( ! QFile::exists( projectFilePath ) )
    {
      std::cout << QObject::tr( "Project file not found, the option will be ignored." ).toStdString() << std::endl;
    }
    else
    {
      qputenv( "QGIS_PROJECT_FILE", projectFilePath.toUtf8() );
    }
  }

  // Disable parallel rendering because if its internal loop
  //qputenv( "QGIS_SERVER_PARALLEL_RENDERING", "0" );

  // Create server
  QTcpServer tcpServer;

  QHostAddress address { QHostAddress::AnyIPv4 };
  address.setAddress( ipAddress );

  if ( ! tcpServer.listen( address, serverPort.toInt( ) ) )
  {
    std::cerr << QObject::tr( "Unable to start the server: %1." )
              .arg( tcpServer.errorString() ).toStdString() << std::endl;
    tcpServer.close();
    app.exitQgis();
    return 1;
  }
  else
  {
    const int port { tcpServer.serverPort() };
    std::cout << QObject::tr( "QGIS Development Server listening on http://%1:%2" )
              .arg( ipAddress ).arg( port ).toStdString() << std::endl;

#ifndef Q_OS_WIN
    std::cout << QObject::tr( "CTRL+C to exit" ).toStdString() << std::endl;
#endif

    QAtomicInt connCounter { 0 };

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

    QgsServer server;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    server.initPython();
#endif

    // Starts HTTP loop with a poor man's HTTP parser
    tcpServer.connect( &tcpServer, &QTcpServer::newConnection, [ & ]
    {
      QTcpSocket *clientConnection = tcpServer.nextPendingConnection();

      connCounter++;

      //qDebug() << "Active connections: " << connCounter;

      // Lambda disconnect context
      QObject *context { new QObject };

      // Deletes the connection later
      auto connectionDeleter = [ =, &connCounter ]()
      {
        clientConnection->deleteLater();
        connCounter--;
      };

      // This will delete the connection when disconnected before ready read is called
      clientConnection->connect( clientConnection, &QAbstractSocket::disconnected, context, connectionDeleter );

      // Incoming connection parser
      clientConnection->connect( clientConnection, &QIODevice::readyRead, context, [ =, &server, &connCounter ] {

        // Disconnect the lambdas
        delete context;

        // Read all incoming data
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
              headers.insert( headerLine.left( headerColonPos ), headerLine.mid( headerColonPos + 2 ) );
            }
          }

          // Build URL from env ...
          QString url { qgetenv( "REQUEST_URI" ) };
          // ... or from server ip/port and request path
          if ( url.isEmpty() )
          {
            const QString path { firstLinePieces.at( 1 )};
            // Take Host header if defined
            if ( headers.contains( QStringLiteral( "Host" ) ) )
            {
              url = QStringLiteral( "http://%1%2" ).arg( headers.value( QStringLiteral( "Host" ) ) ).arg( path );
            }
            else
            {
              url = QStringLiteral( "http://%1:%2%3" ).arg( ipAddress ).arg( port ).arg( path );
            }
          }

          // Inefficient copy :(
          QByteArray data { incomingData.mid( endHeadersPos + 4 ).toUtf8() };

          auto start = std::chrono::steady_clock::now();

          QgsBufferServerRequest request { url, method, headers, &data };
          QgsBufferServerResponse response;

          server.handleRequest( request, response );

          // The QGIS server machinery calls processEvents and has internal loop events
          // that might change the connection state
          if ( clientConnection->state() == QAbstractSocket::SocketState::ConnectedState )
          {
            clientConnection->connect( clientConnection, &QAbstractSocket::disconnected,
                                       clientConnection, connectionDeleter );
          }
          else
          {
            connCounter --;
            clientConnection->deleteLater();
            return;
          }

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
                    .arg( clientConnection->peerAddress().toString(),
                          QDateTime::currentDateTime().toString(),
                          QString::number( body.size() ),
                          QString::number( std::chrono::duration_cast<std::chrono::milliseconds>( elapsedTime ).count() ),
                          firstLinePieces.join( ' ' ),
                          QString::number( response.statusCode() ) )
                    .toStdString()
                    << std::endl;

          clientConnection->disconnectFromHost();
        }
        catch ( HttpException &ex )
        {

          if ( clientConnection->state() == QAbstractSocket::SocketState::ConnectedState )
          {
            clientConnection->connect( clientConnection, &QAbstractSocket::disconnected,
                                       clientConnection, connectionDeleter );
          }
          else
          {
            connCounter --;
            clientConnection->deleteLater();
            return;
          }

          // Output stream: send error
          clientConnection->write( QStringLiteral( "HTTP/1.0 %1 %2\r\n" ).arg( 500 ).arg( knownStatuses.value( 500 ) ).toUtf8() );
          clientConnection->write( QStringLiteral( "Server: QGIS\r\n" ).toUtf8() );
          clientConnection->write( "\r\n" );
          clientConnection->write( ex.message().toUtf8() );

          std::cout << QStringLiteral( "%1 [%2] \"%3\" - - 500" )
                    .arg( clientConnection->peerAddress().toString() )
                    .arg( QDateTime::currentDateTime().toString() )
                    .arg( ex.message() ).toStdString() << std::endl;

          clientConnection->disconnectFromHost();
        }

      } );

    } );

  }

  // Exit handlers
#ifndef Q_OS_WIN

  auto exitHandler = [ ]( int signal )
  {
    std::cout << QStringLiteral( "Signal %1 received: quitting" ).arg( signal ).toStdString() << std::endl;
    qApp->quit();
  };

  signal( SIGTERM, exitHandler );
  signal( SIGABRT, exitHandler );
  signal( SIGINT, exitHandler );
  signal( SIGPIPE, [ ]( int )
  {
    std::cerr << QStringLiteral( "Signal SIGPIPE received: ignoring" ).toStdString() << std::endl;
  } );

#endif

  app.exec();
  app.exitQgis();
  return 0;
}

/// @endcond
