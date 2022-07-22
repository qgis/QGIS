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
  copyright            : (C) 2020 by Alessandro Pasotti
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

#include <thread>
#include <string>
#include <chrono>
#include <condition_variable>

//for CMAKE_INSTALL_PREFIX
#include "qgscommandlineutils.h"
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
#include <QQueue>
#include <QThread>
#include <QPointer>

#ifndef Q_OS_WIN
#include <csignal>
#endif

///@cond PRIVATE

// For the signal exit handler
QAtomicInt IS_RUNNING = 1;

QString ipAddress;
QString serverPort;

std::condition_variable REQUEST_WAIT_CONDITION;
std::mutex REQUEST_QUEUE_MUTEX;
std::mutex SERVER_MUTEX;

struct RequestContext
{
  QPointer<QTcpSocket> clientConnection;
  QString httpHeader;
  std::chrono::steady_clock::time_point startTime;
  QgsBufferServerRequest request;
  QgsBufferServerResponse response;
};


QQueue<RequestContext *> REQUEST_QUEUE;

const QMap<int, QString> knownStatuses
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


class TcpServerWorker: public QObject
{
    Q_OBJECT

  public:

    TcpServerWorker( const QString &ipAddress, int port )
    {
      QHostAddress address { QHostAddress::AnyIPv4 };
      address.setAddress( ipAddress );

      if ( ! mTcpServer.listen( address, port ) )
      {
        std::cerr << tr( "Unable to start the server: %1." )
                  .arg( mTcpServer.errorString() ).toStdString() << std::endl;
      }
      else
      {
        const int port { mTcpServer.serverPort() };

        std::cout << tr( "QGIS Development Server listening on http://%1:%2" ).arg( ipAddress ).arg( port ).toStdString() << std::endl;
#ifndef Q_OS_WIN
        std::cout << tr( "CTRL+C to exit" ).toStdString() << std::endl;
#endif

        mIsListening = true;

        // Incoming connection handler
        QTcpServer::connect( &mTcpServer, &QTcpServer::newConnection, this, [ = ]
        {
          QTcpSocket *clientConnection = mTcpServer.nextPendingConnection();

          mConnectionCounter++;

          //qDebug() << "Active connections: " << mConnectionCounter;

          QString *incomingData = new QString();

          // Lambda disconnect context
          QObject *context { new QObject };

          // Deletes the connection later
          auto connectionDeleter = [ = ]()
          {
            clientConnection->deleteLater();
            mConnectionCounter--;
            delete incomingData;
          };

          // This will delete the connection
          QTcpSocket::connect( clientConnection, &QAbstractSocket::disconnected, clientConnection, connectionDeleter, Qt::QueuedConnection );

#if 0     // Debugging output
          clientConnection->connect( clientConnection, &QAbstractSocket::errorOccurred, clientConnection, [ = ]( QAbstractSocket::SocketError socketError )
          {
            qDebug() << "Socket error #" << socketError;
          }, Qt::QueuedConnection );
#endif

          // Incoming connection parser
          QTcpSocket::connect( clientConnection, &QIODevice::readyRead, context, [ = ] {

            // Read all incoming data
            while ( clientConnection->bytesAvailable() > 0 )
            {
              incomingData->append( clientConnection->readAll() );
            }

            try
            {
              // Parse protocol and URL GET /path HTTP/1.1
              const int firstLinePos { incomingData->indexOf( "\r\n" ) };
              if ( firstLinePos == -1 )
              {
                throw HttpException( QStringLiteral( "HTTP error finding protocol header" ) );
              }

              const QString firstLine { incomingData->left( firstLinePos ) };
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
              if ( protocol != QLatin1String( "HTTP/1.0" ) && protocol != QLatin1String( "HTTP/1.1" ) )
              {
                throw HttpException( QStringLiteral( "HTTP error unsupported protocol: %1" ).arg( protocol ) );
              }

              // Headers
              QgsBufferServerRequest::Headers headers;
              const int endHeadersPos { incomingData->indexOf( "\r\n\r\n" ) };

              if ( endHeadersPos == -1 )
              {
                throw HttpException( QStringLiteral( "HTTP error finding headers" ) );
              }

              const QStringList httpHeaders { incomingData->mid( firstLinePos + 2, endHeadersPos - firstLinePos ).split( "\r\n" ) };

              for ( const auto &headerLine : httpHeaders )
              {
                const int headerColonPos { headerLine.indexOf( ':' ) };
                if ( headerColonPos > 0 )
                {
                  headers.insert( headerLine.left( headerColonPos ), headerLine.mid( headerColonPos + 2 ) );
                }
              }

              const int headersSize { endHeadersPos + 4 };

              // Check for content length and if we have got all data
              if ( headers.contains( QStringLiteral( "Content-Length" ) ) )
              {
                bool ok;
                const int contentLength { headers.value( QStringLiteral( "Content-Length" ) ).toInt( &ok ) };
                if ( ok && contentLength > incomingData->length() - headersSize )
                {
                  return;
                }
              }

              // At this point we should have read all data:
              // disconnect the lambdas
              delete context;

              // Build URL from env ...
              QString url { qgetenv( "REQUEST_URI" ) };
              // ... or from server ip/port and request path
              if ( url.isEmpty() )
              {
                const QString path { firstLinePieces.at( 1 )};
                // Take Host header if defined
                if ( headers.contains( QStringLiteral( "Host" ) ) )
                {
                  url = QStringLiteral( "http://%1%2" ).arg( headers.value( QStringLiteral( "Host" ) ), path );
                }
                else
                {
                  url = QStringLiteral( "http://%1:%2%3" ).arg( ipAddress ).arg( port ).arg( path );
                }
              }

              // Inefficient copy :(
              QByteArray data { incomingData->mid( headersSize ).toUtf8() };

              if ( !incomingData->isEmpty() && clientConnection->state() == QAbstractSocket::SocketState::ConnectedState )
              {
                auto requestContext = new RequestContext
                {
                  clientConnection,
                  firstLinePieces.join( ' ' ),
                  std::chrono::steady_clock::now(),
                  { url, method, headers, &data },
                  {},
                } ;
                REQUEST_QUEUE_MUTEX.lock();
                REQUEST_QUEUE.enqueue( requestContext );
                REQUEST_QUEUE_MUTEX.unlock();
                REQUEST_WAIT_CONDITION.notify_one();
              }
            }
            catch ( HttpException &ex )
            {
              if ( clientConnection->state() == QAbstractSocket::SocketState::ConnectedState )
              {
                // Output stream: send error
                clientConnection->write( QStringLiteral( "HTTP/1.0 %1 %2\r\n" ).arg( 500 ).arg( knownStatuses.value( 500 ) ).toUtf8() );
                clientConnection->write( QStringLiteral( "Server: QGIS\r\n" ).toUtf8() );
                clientConnection->write( "\r\n" );
                clientConnection->write( ex.message().toUtf8() );

                std::cout << QStringLiteral( "\033[1;31m%1 [%2] \"%3\" - - 500\033[0m" )
                          .arg( clientConnection->peerAddress().toString() )
                          .arg( QDateTime::currentDateTime().toString() )
                          .arg( ex.message() ).toStdString() << std::endl;

                clientConnection->disconnectFromHost();
              }
            }
          } );
        } );
      }
    }

    ~TcpServerWorker()
    {
      mTcpServer.close();
    }

    bool isListening() const
    {
      return mIsListening;
    }

  public slots:

    // Outgoing connection handler
    void responseReady( RequestContext *requestContext )  //#spellok
    {
      std::unique_ptr<RequestContext> request { requestContext };
      const auto elapsedTime { std::chrono::steady_clock::now() - request->startTime };

      const auto &response { request->response };
      const auto &clientConnection { request->clientConnection };

      if ( ! clientConnection ||
           clientConnection->state() != QAbstractSocket::SocketState::ConnectedState )
      {
        std::cout << "Connection reset by peer" << std::endl;
        return;
      }

      // Output stream
      if ( -1 == clientConnection->write( QStringLiteral( "HTTP/1.0 %1 %2\r\n" ).arg( response.statusCode() ).arg( knownStatuses.value( response.statusCode(), QStringLiteral( "Unknown response code" ) ) ).toUtf8() ) )
      {
        std::cout << "Cannot write to output socket" << std::endl;
        clientConnection->disconnectFromHost();
        return;
      }

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
      std::cout << QStringLiteral( "\033[1;92m%1 [%2] %3 %4ms \"%5\" %6\033[0m" )
                .arg( clientConnection->peerAddress().toString(),
                      QDateTime::currentDateTime().toString(),
                      QString::number( body.size() ),
                      QString::number( std::chrono::duration_cast<std::chrono::milliseconds>( elapsedTime ).count() ),
                      request->httpHeader,
                      QString::number( response.statusCode() ) )
                .toStdString()
                << std::endl;

      // This will trigger delete later on the socket object
      clientConnection->disconnectFromHost();
    }

  private:

    QTcpServer mTcpServer;
    qlonglong mConnectionCounter = 0;
    bool mIsListening = false;

};


class TcpServerThread: public QThread
{
    Q_OBJECT

  public:

    TcpServerThread( const QString &ipAddress, const int port )
      : mIpAddress( ipAddress )
      , mPort( port )
    {
    }

    void emitResponseReady( RequestContext *requestContext )  //#spellok
    {
      if ( requestContext->clientConnection )
        emit responseReady( requestContext );  //#spellok
    }

    void run( )
    {
      const TcpServerWorker worker( mIpAddress, mPort );
      if ( ! worker.isListening() )
      {
        emit serverError();
      }
      else
      {
        // Forward signal to worker
        connect( this, &TcpServerThread::responseReady, &worker, &TcpServerWorker::responseReady );  //#spellok
        QThread::run();
      }
    }

  signals:

    void responseReady( RequestContext *requestContext );  //#spellok
    void serverError( );

  private:

    QString mIpAddress;
    int mPort;
};


class QueueMonitorThread: public QThread
{

    Q_OBJECT

  public:
    void run( )
    {
      while ( mIsRunning )
      {
        std::unique_lock<std::mutex> requestLocker( REQUEST_QUEUE_MUTEX );
        REQUEST_WAIT_CONDITION.wait( requestLocker, [ = ] { return ! mIsRunning || ! REQUEST_QUEUE.isEmpty(); } );
        if ( mIsRunning )
        {
          // Lock if server is running
          SERVER_MUTEX.lock();
          emit requestReady( REQUEST_QUEUE.dequeue() );
        }
      }
    }

  signals:

    void requestReady( RequestContext *requestContext );

  public slots:

    void stop()
    {
      mIsRunning = false;
    }

  private:

    bool mIsRunning = true;

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
  const QgsApplication app( argc, argv, withDisplay, QString(), QStringLiteral( "QGIS Development Server" ) );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( "QGIS Development Server" );
  QCoreApplication::setApplicationVersion( VERSION );

  if ( ! withDisplay )
  {
    QgsMessageLog::logMessage( "DISPLAY environment variable is not set, running in offscreen mode, all printing capabilities will not be available.\n"
                               "Consider installing an X server like 'xvfb' and export DISPLAY to the actual display value.", "Server", Qgis::MessageLevel::Warning );
  }

#ifdef Q_OS_WIN
  // Initialize font database before fcgi_accept.
  // When using FCGI with IIS, environment variables (QT_QPA_FONTDIR in this case) are lost after fcgi_accept().
  QFontDatabase fontDB;
#endif

  // The port to listen
  serverPort = qgetenv( "QGIS_SERVER_PORT" );
  // The address to listen
  ipAddress = qgetenv( "QGIS_SERVER_ADDRESS" );

  if ( serverPort.isEmpty() )
  {
    serverPort = QStringLiteral( "8000" );
  }

  if ( ipAddress.isEmpty() )
  {
    ipAddress = QStringLiteral( "localhost" );
  }

  QCommandLineParser parser;
  parser.setApplicationDescription( QObject::tr( "QGIS Development Server %1" ).arg( VERSION ) );
  parser.addHelpOption();

  const QCommandLineOption versionOption( QStringList() << "v" << "version", QObject::tr( "Version of QGIS and libraries" ) );
  parser.addOption( versionOption );

  parser.addPositionalArgument( QStringLiteral( "addressAndPort" ),
                                QObject::tr( "Address and port (default: \"localhost:8000\")\n"
                                    "address and port can also be specified with the environment\n"
                                    "variables QGIS_SERVER_ADDRESS and QGIS_SERVER_PORT." ), QStringLiteral( "[address:port]" ) );
  const QCommandLineOption logLevelOption( "l", QObject::tr( "Log level (default: 0)\n"
      "0: INFO\n"
      "1: WARNING\n"
      "2: CRITICAL" ), "logLevel", "0" );
  parser.addOption( logLevelOption );

  const QCommandLineOption projectOption( "p", QObject::tr( "Path to a QGIS project file (*.qgs or *.qgz),\n"
                                          "if specified it will override the query string MAP argument\n"
                                          "and the QGIS_PROJECT_FILE environment variable." ), "projectPath", "" );
  parser.addOption( projectOption );

  parser.process( app );

  if ( parser.isSet( versionOption ) )
  {
    std::cout << QgsCommandLineUtils::allVersions().toStdString();
    return 0;
  }

  const QStringList args = parser.positionalArguments();

  if ( args.size() == 1 )
  {
    const QStringList addressAndPort { args.at( 0 ).split( ':' ) };
    if ( addressAndPort.size() == 2 )
    {
      ipAddress = addressAndPort.at( 0 );
      serverPort = addressAndPort.at( 1 );
    }
  }

  const QString logLevel = parser.value( logLevelOption );
  qunsetenv( "QGIS_SERVER_LOG_FILE" );
  qputenv( "QGIS_SERVER_LOG_LEVEL", logLevel.toUtf8() );
  qputenv( "QGIS_SERVER_LOG_STDERR", "1" );

  QgsServer server;

  if ( ! parser.value( projectOption ).isEmpty( ) )
  {
    // Check it!
    const QString projectFilePath { parser.value( projectOption ) };
    if ( ! QgsProject::instance()->read( projectFilePath,
                                         Qgis::ProjectReadFlag::DontResolveLayers
                                         | Qgis::ProjectReadFlag::DontLoadLayouts
                                         | Qgis::ProjectReadFlag::DontStoreOriginalStyles
                                         | Qgis::ProjectReadFlag::DontLoad3DViews ) )
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


#ifdef HAVE_SERVER_PYTHON_PLUGINS
  server.initPython();
#endif

  // TCP thread
  TcpServerThread tcpServerThread{ ipAddress, serverPort.toInt() };

  bool isTcpError = false;
  TcpServerThread::connect( &tcpServerThread, &TcpServerThread::serverError, qApp, [ & ]
  {
    isTcpError = true;
    qApp->quit();
  }, Qt::QueuedConnection );

  // Monitoring thread
  QueueMonitorThread queueMonitorThread;
  QueueMonitorThread::connect( &queueMonitorThread, &QueueMonitorThread::requestReady, qApp, [ & ]( RequestContext * requestContext )
  {
    if ( requestContext->clientConnection && requestContext->clientConnection->isValid() )
    {
      server.handleRequest( requestContext->request, requestContext->response );
      SERVER_MUTEX.unlock();
    }
    else
    {
      delete requestContext;
      SERVER_MUTEX.unlock();
      return;
    }
    if ( requestContext->clientConnection && requestContext->clientConnection->isValid() )
      tcpServerThread.emitResponseReady( requestContext );  //#spellok
    else
      delete requestContext;
  } );

  // Exit handlers
#ifndef Q_OS_WIN

  auto exitHandler = [ ]( int signal )
  {
    std::cout << QStringLiteral( "Signal %1 received: quitting" ).arg( signal ).toStdString() << std::endl;
    IS_RUNNING = 0;
    qApp->quit( );
  };

  signal( SIGTERM, exitHandler );
  signal( SIGABRT, exitHandler );
  signal( SIGINT, exitHandler );
  signal( SIGPIPE, [ ]( int )
  {
    std::cerr << QStringLiteral( "Signal SIGPIPE received: ignoring" ).toStdString() << std::endl;
  } );

#endif

  tcpServerThread.start();
  queueMonitorThread.start();

  QgsApplication::exec();
  // Wait for threads
  tcpServerThread.exit();
  tcpServerThread.wait();
  queueMonitorThread.stop();
  REQUEST_WAIT_CONDITION.notify_all();
  queueMonitorThread.wait();
  QgsApplication::exitQgis();

  return isTcpError ? 1 : 0;
}

#include "qgis_mapserver.moc"

/// @endcond


