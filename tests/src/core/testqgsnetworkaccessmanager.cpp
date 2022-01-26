/***************************************************************************
                         testqgsnetworkaccessmanager.cpp
                         -----------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkcontentfetcher.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsapplication.h"
#include "qgis.h"
#include <QObject>
#include "qgstest.h"
#include "qgssettings.h"
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QThread>
#include <QHttpMultiPart>

class BackgroundRequest : public QThread
{
    Q_OBJECT

  public:
    BackgroundRequest( const QNetworkRequest &request, QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation, const QByteArray &data = QByteArray() )
      : mRequest( request )
    {
      moveToThread( this );
      connect( this, &QThread::started, this, [ = ]
      {
        QVERIFY( QThread::currentThread() != QCoreApplication::instance()->thread() );
        switch ( op )
        {
          case QNetworkAccessManager::GetOperation:
            mReply = QgsNetworkAccessManager::instance()->get( mRequest );
            break;

          case QNetworkAccessManager::PostOperation:
            mReply = QgsNetworkAccessManager::instance()->post( mRequest, data );
            break;

          default:
            break;
        }

      } );
    }

    QNetworkReply *mReply = nullptr;
  private:
    QNetworkRequest mRequest;
};


class BackgroundBlockingRequest : public QThread
{
    Q_OBJECT

  public:
    BackgroundBlockingRequest( const QNetworkRequest &request, QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation,
                               QNetworkReply::NetworkError expectedRes = QNetworkReply::NoError, const QByteArray &data = QByteArray(), const QByteArray &expectedData = QByteArray() )
      : mRequest( request )
      , mExpectedData( expectedData )
    {
      moveToThread( this );
      connect( this, &QThread::started, this, [ = ]
      {
        QVERIFY( QThread::currentThread() != QCoreApplication::instance()->thread() );
        switch ( op )
        {
          case QNetworkAccessManager::GetOperation:
            mReply = QgsNetworkAccessManager::blockingGet( mRequest );
            break;

          case QNetworkAccessManager::PostOperation:
            mReply = QgsNetworkAccessManager::blockingPost( mRequest, data );
            break;

          default:
            break;
        }

        QCOMPARE( mReply.error(), expectedRes );
        if ( !mExpectedData.isEmpty() )
          QVERIFY( mReply.content().contains( mExpectedData ) );

      } );
    }

    QgsNetworkReplyContent mReply;
  private:
    QNetworkRequest mRequest;
    QByteArray mExpectedData;
};

class TestSslErrorHandler : public QgsSslErrorHandler
{
  public:

    void handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors ) override
    {
      Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
      QCOMPARE( errors.at( 0 ).error(), QSslError::SelfSignedCertificate );
      reply->ignoreSslErrors();
    }

};

class TestAuthRequestHandler : public QgsNetworkAuthenticationHandler
{
  public:

    TestAuthRequestHandler( const QString &user, const QString &password )
      : mUser( user )
      , mPassword( password )
    {}

    void handleAuthRequest( QNetworkReply *, QAuthenticator *auth ) override
    {
      Q_ASSERT( QThread::currentThread() == QApplication::instance()->thread() );
      if ( !mUser.isEmpty() )
        auth->setUser( mUser );
      if ( !mPassword.isEmpty() )
        auth->setPassword( mPassword );
    }

    QString mUser;
    QString mPassword;

};

class TestQgsNetworkAccessManager : public QObject
{
    Q_OBJECT
  public:
    TestQgsNetworkAccessManager() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testRequestPreprocessor();
    void fetchEmptyUrl(); //test fetching blank url
    void fetchBadUrl(); //test fetching bad url
    void fetchEncodedContent(); //test fetching url content encoded as utf-8
    void fetchPost();
    void fetchPostMultiPart();
    void fetchPostMultiPart_data();
    void fetchBadSsl();
    void testSslErrorHandler();
    void testAuthRequestHandler();
    void fetchTimeout();
    void testCookieManagement();
    void testProxyExcludeList();

  private:

    QString mHttpBinHost;

};

void TestQgsNetworkAccessManager::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsNetworkAccessManager::settingsNetworkTimeout.setValue( 5000 );

  mHttpBinHost = QStringLiteral( "httpbin.org" );
  const QString overrideHost = qgetenv( "QGIS_HTTPBIN_HOST" );
  if ( !overrideHost.isEmpty() )
    mHttpBinHost = overrideHost;
}

void TestQgsNetworkAccessManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsNetworkAccessManager::init()
{

}

void TestQgsNetworkAccessManager::cleanup()
{
}

void TestQgsNetworkAccessManager::testProxyExcludeList()
{
  QgsNetworkAccessManager manager;
  const QNetworkProxy fallback( QNetworkProxy::HttpProxy, QStringLiteral( "babies_first_proxy" ) );
  manager.setFallbackProxyAndExcludes( fallback, QStringList() << QStringLiteral( "intranet" ) << QStringLiteral( "something_else" ), QStringList() << QStringLiteral( "noProxyUrl" ) );
  QCOMPARE( manager.fallbackProxy().hostName(), QStringLiteral( "babies_first_proxy" ) );
  QCOMPARE( manager.excludeList(), QStringList() << QStringLiteral( "intranet" ) << QStringLiteral( "something_else" ) );
  QCOMPARE( manager.noProxyList(), QStringList() << QStringLiteral( "noProxyUrl" ) );

  QgsNetworkAccessManager manager2;
  manager2.setFallbackProxyAndExcludes( fallback, QStringList() << QStringLiteral( "intranet" ) << "", QStringList() << QStringLiteral( "noProxyUrl" ) << "" );
  // empty strings MUST be filtered from this list - otherwise they match all hosts!
  QCOMPARE( manager2.excludeList(), QStringList() << QStringLiteral( "intranet" ) );
  QCOMPARE( manager2.noProxyList(), QStringList() << QStringLiteral( "noProxyUrl" ) );

  // check that when we query an exclude URL, the returned proxy is no proxy
  QgsNetworkAccessManager::instance()->setFallbackProxyAndExcludes( fallback, QStringList() << QStringLiteral( "intranet" ) << "", QStringList() << QStringLiteral( "noProxy" ) );
  QList<QNetworkProxy> proxies = QgsNetworkAccessManager::instance()->proxyFactory()->queryProxy( QNetworkProxyQuery( QUrl( "intranet/mystuff" ) ) );
  QCOMPARE( proxies.count(), 1 );
  QCOMPARE( proxies.at( 0 ).type(),  QNetworkProxy::DefaultProxy );
  proxies = QgsNetworkAccessManager::instance()->proxyFactory()->queryProxy( QNetworkProxyQuery( QUrl( "noProxy/mystuff" ) ) );
  QCOMPARE( proxies.count(), 1 );
  QCOMPARE( proxies.at( 0 ).type(),  QNetworkProxy::NoProxy );
}

void TestQgsNetworkAccessManager::fetchEmptyUrl()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl() );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.errorString(), QStringLiteral( "Protocol \"\" is unknown" ) );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), QUrl() );
    loaded = true;
  } );
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl() ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  QNetworkRequest req{ QUrl() };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), QStringLiteral( "Protocol \"\" is unknown" ) );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( QUrl() ) );

  thread->start();

  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();


  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( QNetworkRequest( QUrl() ), QNetworkAccessManager::GetOperation, QNetworkReply::ProtocolUnknownError );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();
}

void TestQgsNetworkAccessManager::fetchBadUrl()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl( QStringLiteral( "http://x" ) ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.errorString(), QStringLiteral( "Host x not found" ) );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), QUrl( QStringLiteral( "http://x" ) ) );

    loaded = true;
  } );
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl( QStringLiteral( "http://x" ) ) ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  QNetworkRequest req{ QUrl( QStringLiteral( "http://x" ) ) };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), QStringLiteral( "Host x not found" ) );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( QUrl( QStringLiteral( "http://x" ) ) ) );

  thread->start();

  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( QNetworkRequest( QUrl( QStringLiteral( "http://x" ) ) ), QNetworkAccessManager::GetOperation, QNetworkReply::HostNotFoundError );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();
}


void TestQgsNetworkAccessManager::fetchEncodedContent()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  QUrl u =  QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + '/' +  "encoded_html.html" );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.initiatorClassName(), QStringLiteral( "myTestClass" ) );
    QCOMPARE( params.initiatorRequestId().toInt(), 55 );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );
    QVERIFY( reply.rawHeaderList().contains( "Content-Length" ) );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  QNetworkRequest r( u );
  r.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorClass ), QStringLiteral( "myTestClass" ) );
  r.setAttribute( static_cast< QNetworkRequest::Attribute >( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), 55 );
  QgsNetworkAccessManager::instance()->get( r );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( r );
  QVERIFY( rep.content().contains( "<title>test</title>" ) );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundRequest *thread = new BackgroundRequest( r );

  thread->start();

  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( r, QNetworkAccessManager::GetOperation, QNetworkReply::NoError, QByteArray(),  "<title>test</title>" );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();
}

void TestQgsNetworkAccessManager::fetchPost()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/post" ) );
  QNetworkRequest req( u );
  req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/x-www-form-urlencoded" ) );
  QString replyContent;

  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::PostOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.content(), QByteArray( "a=b&c=d" ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );
    QVERIFY( reply.rawHeaderList().contains( "Content-Type" ) );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );

  QNetworkReply *reply = QgsNetworkAccessManager::instance()->post( req, QByteArray( "a=b&c=d" ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  replyContent = reply->readAll();
  QVERIFY( replyContent.contains( QStringLiteral( "\"a\": \"b\"" ) ) );
  QVERIFY( replyContent.contains( QStringLiteral( "\"c\": \"d\"" ) ) );
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/x-www-form-urlencoded" ) );
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingPost( req, QByteArray( "a=b&c=d" ) );
  QVERIFY( rep.content().contains( "\"a\": \"b\"" ) );
  QVERIFY( rep.content().contains( "\"c\": \"d\"" ) );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/x-www-form-urlencoded" ) );
  BackgroundRequest *thread = new BackgroundRequest( req, QNetworkAccessManager::PostOperation, QByteArray( "a=b&c=d" ) );

  thread->start();

  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  replyContent = thread->mReply->readAll();
  QVERIFY( replyContent.contains( QStringLiteral( "\"a\": \"b\"" ) ) );
  QVERIFY( replyContent.contains( QStringLiteral( "\"c\": \"d\"" ) ) );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/x-www-form-urlencoded" ) );
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::PostOperation, QNetworkReply::NoError, QByteArray( "a=b&c=d" ),  "\"a\": \"b\"" );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();
}

void TestQgsNetworkAccessManager::fetchPostMultiPart()
{
  QFETCH( int, iContentType );
  QHttpMultiPart::ContentType contentType = static_cast< QHttpMultiPart::ContentType>( iContentType );
  QHttpMultiPart *multipart = new QHttpMultiPart( contentType );
  QHttpPart part;
  part.setHeader( QNetworkRequest::ContentDispositionHeader,
                  QStringLiteral( "form-data; name=\"param\"" ) );
  part.setBody( QStringLiteral( "some data" ) .toUtf8() );
  multipart->append( part );
  QUrl u = QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/post" ) );
  QNetworkRequest req( u );

  // MultiPart
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->post( req, multipart );
  multipart->setParent( reply );

  QEventLoop el;
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::finished, &el, &QEventLoop::quit );
  el.exec();

  QCOMPARE( reply->error(), QNetworkReply::NoError );
  QVERIFY( reply->rawHeaderList().contains( "Content-Type" ) );
  QCOMPARE( reply->request().url(), u );
}

void TestQgsNetworkAccessManager::fetchPostMultiPart_data()
{
  QTest::addColumn<int>( "iContentType" );

  QTest::newRow( "MixedType" ) << static_cast<int>( QHttpMultiPart::MixedType );
  QTest::newRow( "FormDataType" ) << static_cast<int>( QHttpMultiPart::FormDataType );
  QTest::newRow( "RelatedType" ) << static_cast<int>( QHttpMultiPart::FormDataType );
  QTest::newRow( "AlternativeType" ) << static_cast<int>( QHttpMultiPart::FormDataType );
}

void TestQgsNetworkAccessManager::fetchBadSsl()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotSslError = false;
  bool gotRequestEncounteredSslError = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "https://expired.badssl.com" ) );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::SslHandshakeFailedError );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, &context, [&]( QNetworkReply *, const QList<QSslError> &errors )
  {
    QCOMPARE( errors.at( 0 ).error(), QSslError::CertificateExpired );
    gotSslError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestEncounteredSslErrors, &context, [&]( int errorRequestId, const QList<QSslError> &errors )
  {
    QCOMPARE( errors.at( 0 ).error(), QSslError::CertificateExpired );
    QCOMPARE( errorRequestId, requestId );
    gotRequestEncounteredSslError = true;
  } );

  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  // blocking request
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  QNetworkRequest req{ u };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), QStringLiteral( "SSL handshake failed" ) );
  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( u ) );

  thread->start();

  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();


  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, QNetworkReply::SslHandshakeFailedError );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();

}

void TestQgsNetworkAccessManager::testSslErrorHandler()
{
  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique< TestSslErrorHandler >() );

  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotSslError = false;
  int requestId = -1;
  bool gotRequestEncounteredSslError = false;
  QUrl u =  QUrl( QStringLiteral( "https://self-signed.badssl.com/" ) );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::NoError ); // because handler ignores error
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, &context, [&]( QNetworkReply *, const QList<QSslError> &errors )
  {
    QCOMPARE( errors.at( 0 ).error(), QSslError::SelfSignedCertificate );
    gotSslError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestEncounteredSslErrors, &context, [&]( int errorRequestId, const QList<QSslError> &errors )
  {
    QCOMPARE( errors.at( 0 ).error(), QSslError::SelfSignedCertificate );
    QCOMPARE( errorRequestId, requestId );
    gotRequestEncounteredSslError = true;
  } );

  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  // blocking request
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  QNetworkRequest req{ u };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.error(), QNetworkReply::NoError );
  QVERIFY( rep.content().contains( "<!DOCTYPE html>" ) );
  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );


  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( u ) );

  thread->start();

  while ( !loaded || !gotSslError || !gotRequestAboutToBeCreatedSignal || !gotRequestEncounteredSslError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  gotSslError = false;
  gotRequestEncounteredSslError = false;
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, QNetworkReply::NoError, QByteArray(), "<!DOCTYPE html>" );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();

  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique< QgsSslErrorHandler >() );
}

void TestQgsNetworkAccessManager::testAuthRequestHandler()
{
  // initially this request should fail -- we aren't providing the username and password required
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< TestAuthRequestHandler >( QString(), QString() ) );

  const QObject context;
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotAuthRequest = false;
  bool gotAuthDetailsAdded = false;
  QString hash = QUuid::createUuid().toString().mid( 1, 10 );
  QString expectedUser;
  QString expectedPassword;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );
  QNetworkReply::NetworkError expectedError = QNetworkReply::NoError;

  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), expectedError );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );

  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestRequiresAuth, &context, [&]( int authRequestId, const QString & realm )
  {
    QCOMPARE( authRequestId, requestId );
    QCOMPARE( realm, QStringLiteral( "Fake Realm" ) );
    gotAuthRequest = true;
  } );

  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestAuthDetailsAdded, &context, [&]( int authRequestId, const QString & realm, const QString & user, const QString & password )
  {
    QCOMPARE( authRequestId, requestId );
    QCOMPARE( realm, QStringLiteral( "Fake Realm" ) );
    QCOMPARE( user, expectedUser );
    QCOMPARE( password, expectedPassword );
    gotAuthDetailsAdded = true;
  } );

  expectedError = QNetworkReply::AuthenticationRequiredError;
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded || !gotAuthRequest || !gotRequestAboutToBeCreatedSignal || !gotAuthDetailsAdded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  // blocking request
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;

  QNetworkRequest req{ u };
  QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QVERIFY( rep.content().isEmpty() );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  // now try in a thread
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );

  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( u ) );

  thread->start();

  while ( !loaded || !gotAuthRequest || !gotRequestAboutToBeCreatedSignal || !gotAuthDetailsAdded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in a thread

  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );
  req = QNetworkRequest( u );
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, expectedError );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();

  // try with username and password specified
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< TestAuthRequestHandler >( QStringLiteral( "me" ), hash ) );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = QStringLiteral( "me" );
  expectedPassword = hash;

  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded || !gotAuthRequest || !gotRequestAboutToBeCreatedSignal  || !gotAuthDetailsAdded )
  {
    qApp->processEvents();
  }

  // blocking request
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  expectedPassword = hash;
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< TestAuthRequestHandler >( QStringLiteral( "me" ), hash ) );
  u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me/" ) + hash );
  req = QNetworkRequest{ u };
  rep = QgsNetworkAccessManager::blockingGet( req );
  QVERIFY( rep.content().contains( "\"user\": \"me\"" ) );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  // correct username and password, in a thread
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< TestAuthRequestHandler >( QStringLiteral( "me2" ), hash ) );
  u = QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me2/" ) + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = QStringLiteral( "me2" );
  expectedPassword = hash;

  thread = new BackgroundRequest( QNetworkRequest( u ) );
  thread->start();
  while ( !loaded || !gotAuthRequest || !gotRequestAboutToBeCreatedSignal || !gotAuthDetailsAdded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();


  // blocking request in worker thread
  hash = QUuid::createUuid().toString().mid( 1, 10 );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< TestAuthRequestHandler >( QStringLiteral( "me2" ), hash ) );
  u = QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/basic-auth/me2/" ) + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = QStringLiteral( "me2" );
  expectedPassword = hash;
  req = QNetworkRequest{ u };
  blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, QNetworkReply::NoError, QByteArray(),  "\"user\": \"me2\"" );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();

  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique< QgsNetworkAuthenticationHandler >() );
}

void TestQgsNetworkAccessManager::fetchTimeout()
{
  QgsNetworkAccessManager::setTimeout( 2000 );
  QCOMPARE( QgsNetworkAccessManager::timeout(), 2000 );
  QgsNetworkAccessManager::setTimeout( 1000 );

  const QObject context;
  //test fetching from a blank url
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotTimeoutError = false;
  bool finished = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "http://" ) + mHttpBinHost + QStringLiteral( "/delay/10" ) );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkRequestParameters >( &QgsNetworkAccessManager::requestTimedOut ), &context, [&]( const QgsNetworkRequestParameters & request )
  {
    QCOMPARE( request.requestId(), requestId );
    gotTimeoutError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload< QgsNetworkReplyContent >( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    finished = reply.error() != QNetworkReply::OperationCanceledError; // should not happen!
  } );

  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !gotTimeoutError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotTimeoutError );
  QVERIFY( !finished );

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  // blocking request
  gotRequestAboutToBeCreatedSignal = false;
  gotTimeoutError = false;
  finished = false;
  QNetworkRequest req = QNetworkRequest{ u };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), QStringLiteral( "Operation canceled" ) );
  while ( !gotTimeoutError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  // in a background thread

  gotRequestAboutToBeCreatedSignal = false;
  gotTimeoutError = false;
  finished = false;
  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( u ) );

  thread->start();

  while ( !gotTimeoutError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotTimeoutError );
  QVERIFY( !finished );

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  gotTimeoutError = false;
  finished = false;
  req = QNetworkRequest{ u };
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, QNetworkReply::OperationCanceledError );
  blockingThread->start();
  while ( !gotTimeoutError )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );

  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();
}

class FunctionThread : public QThread
{
  public:
    FunctionThread( const std::function<bool()> &f ) : m_f( f ), m_result( false ) {}
    bool getResult() const
    {
      return m_result;
    }
  private:
    std::function<bool()> m_f;
    bool m_result;

    void run()
    {
      m_result = m_f();
    }
};

void TestQgsNetworkAccessManager::testCookieManagement()
{
  const QUrl url( "http://example.com" );
  // Set cookie in a thread and verify that it also set in main thread
  QEventLoop evLoop;
  FunctionThread thread1( [ = ]
  {
    QgsNetworkAccessManager::instance()->cookieJar()->setCookiesFromUrl(
      QList<QNetworkCookie>() << QNetworkCookie( "foo=bar" ), url );
    return true;
  } );
  QObject::connect( &thread1, &QThread::finished, &evLoop, &QEventLoop::quit );
  thread1.start();
  evLoop.exec();

  QList<QNetworkCookie> cookies = QgsNetworkAccessManager::instance()->cookieJar()->cookiesForUrl( url );

  QCOMPARE( cookies.size(), 1 );
  QCOMPARE( cookies[0].toRawForm(), "foo=bar=; domain=example.com; path=/" );

  QVERIFY( QgsNetworkAccessManager::instance()->cookieJar()->deleteCookie( cookies[0] ) );

  // Set cookie in main thread and verify that it is also set in other thread
  QCOMPARE( QgsNetworkAccessManager::instance()->cookieJar()->cookiesForUrl( url ).size(), 0 );
  QgsNetworkAccessManager::instance()->cookieJar()->setCookiesFromUrl(
    QList<QNetworkCookie>() << QNetworkCookie( "baz=yadda" ), url
  );

  FunctionThread thread2( [ = ]
  {
    QList<QNetworkCookie> cookies = QgsNetworkAccessManager::instance()->cookieJar()->cookiesForUrl( url );
    return cookies.size() == 1 && cookies[0].toRawForm() == "baz=yadda=; domain=example.com; path=/";
  } );
  QObject::connect( &thread2, &QThread::finished, &evLoop, &QEventLoop::quit );
  thread2.start();
  evLoop.exec();
  QVERIFY( thread2.getResult() );
};

void TestQgsNetworkAccessManager::testRequestPreprocessor()
{
  const QString processorId = QgsNetworkAccessManager::setRequestPreprocessor( []( QNetworkRequest * request ) { request->setHeader( QNetworkRequest::UserAgentHeader, QStringLiteral( "QGIS" ) );} );
  QNetworkRequest request;
  QgsNetworkAccessManager::instance()->preprocessRequest( &request );
  const QString userAgent = request.header( QNetworkRequest::UserAgentHeader ).toString();
  QCOMPARE( userAgent, "QGIS" );
  QgsNetworkAccessManager::removeRequestPreprocessor( processorId );
};

QGSTEST_MAIN( TestQgsNetworkAccessManager )
#include "testqgsnetworkaccessmanager.moc"
