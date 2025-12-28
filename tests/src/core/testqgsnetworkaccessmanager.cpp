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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"
#include "qgstest.h"

#include <QAuthenticator>
#include <QHttpMultiPart>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QObject>
#include <QThread>

class BackgroundRequest : public QThread
{
    Q_OBJECT

  public:
    BackgroundRequest( const QNetworkRequest &request, QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation, const QByteArray &data = QByteArray() )
      : mRequest( request )
    {
      moveToThread( this );
      connect( this, &QThread::started, this, [this, op, data] {
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
    BackgroundBlockingRequest( const QNetworkRequest &request, QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation, QNetworkReply::NetworkError expectedRes = QNetworkReply::NoError, const QByteArray &data = QByteArray(), const QByteArray &expectedData = QByteArray() )
      : mRequest( request )
      , mExpectedData( expectedData )
    {
      moveToThread( this );
      connect( this, &QThread::started, this, [this, op, expectedRes, data] {
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

class TestQgsNetworkAccessManager : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsNetworkAccessManager()
      : QgsTest( u"Network access manager tests"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testRequestPreprocessor();
    void fetchEmptyUrl();       //test fetching blank url
    void fetchBadUrl();         //test fetching bad url
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
    void testHandlers();

  private:
    QString mHttpBinHost;
};

void TestQgsNetworkAccessManager::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsNetworkAccessManager::settingsNetworkTimeout->setValue( 5000 );

  mHttpBinHost = u"httpbin.org"_s;
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
  const QNetworkProxy fallback( QNetworkProxy::HttpProxy, u"babies_first_proxy"_s );
  manager.setFallbackProxyAndExcludes( fallback, QStringList() << u"intranet"_s << u"something_else"_s, QStringList() << u"noProxyUrl"_s );
  QCOMPARE( manager.fallbackProxy().hostName(), u"babies_first_proxy"_s );
  QCOMPARE( manager.excludeList(), QStringList() << u"intranet"_s << u"something_else"_s );
  QCOMPARE( manager.noProxyList(), QStringList() << u"noProxyUrl"_s );

  QgsNetworkAccessManager manager2;
  manager2.setFallbackProxyAndExcludes( fallback, QStringList() << u"intranet"_s << "", QStringList() << u"noProxyUrl"_s << "" );
  // empty strings MUST be filtered from this list - otherwise they match all hosts!
  QCOMPARE( manager2.excludeList(), QStringList() << u"intranet"_s );
  QCOMPARE( manager2.noProxyList(), QStringList() << u"noProxyUrl"_s );

  // check that when we query an exclude URL, the returned proxy is no proxy
  QgsNetworkAccessManager::instance()->setFallbackProxyAndExcludes( fallback, QStringList() << u"intranet"_s << "", QStringList() << u"noProxy"_s );
  QList<QNetworkProxy> proxies = QgsNetworkAccessManager::instance()->proxyFactory()->queryProxy( QNetworkProxyQuery( QUrl( "intranet/mystuff" ) ) );
  QCOMPARE( proxies.count(), 1 );
  QCOMPARE( proxies.at( 0 ).type(), QNetworkProxy::DefaultProxy );
  proxies = QgsNetworkAccessManager::instance()->proxyFactory()->queryProxy( QNetworkProxyQuery( QUrl( "noProxy/mystuff" ) ) );
  QCOMPARE( proxies.count(), 1 );
  QCOMPARE( proxies.at( 0 ).type(), QNetworkProxy::NoProxy );
}


class DummySslErrorHandler : public QgsSslErrorHandler
{
  public:
};

class DummyNetworkAuthenticationHandler : public QgsNetworkAuthenticationHandler
{
  public:
};

void TestQgsNetworkAccessManager::testHandlers()
{
  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique<DummySslErrorHandler>() );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<DummyNetworkAuthenticationHandler>() );

  QVERIFY( dynamic_cast<DummySslErrorHandler *>( QgsNetworkAccessManager::instance()->mSslErrorHandler.get() ) );
  QVERIFY( dynamic_cast<DummyNetworkAuthenticationHandler *>( QgsNetworkAccessManager::instance()->mAuthHandler.get() ) );

  // handlers should NOT be overwritten
  QgsNetworkAccessManager::instance()->setupDefaultProxyAndCache();
  QVERIFY( dynamic_cast<DummySslErrorHandler *>( QgsNetworkAccessManager::instance()->mSslErrorHandler.get() ) );
  QVERIFY( dynamic_cast<DummyNetworkAuthenticationHandler *>( QgsNetworkAccessManager::instance()->mAuthHandler.get() ) );

  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique<QgsSslErrorHandler>() );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<QgsNetworkAuthenticationHandler>() );
}

void TestQgsNetworkAccessManager::fetchEmptyUrl()
{
  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl() );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.errorString(), u"Protocol \"\" is unknown"_s );
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
  QNetworkRequest req { QUrl() };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), u"Protocol \"\" is unknown"_s );
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
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl( u"http://x"_s ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.errorString(), u"Host x not found"_s );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), QUrl( u"http://x"_s ) );

    loaded = true;
  } );
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( QUrl( u"http://x"_s ) ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  QNetworkRequest req { QUrl( u"http://x"_s ) };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), u"Host x not found"_s );
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QCOMPARE( rep.requestId(), requestId );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  BackgroundRequest *thread = new BackgroundRequest( QNetworkRequest( QUrl( u"http://x"_s ) ) );

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

  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( QNetworkRequest( QUrl( u"http://x"_s ) ), QNetworkAccessManager::GetOperation, QNetworkReply::HostNotFoundError );
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
  QUrl u = QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + '/' + "encoded_html.html" );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.initiatorClassName(), u"myTestClass"_s );
    QCOMPARE( params.initiatorRequestId().toInt(), 55 );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );

    // newer qt versions force headers to lower case, older ones didn't
    QStringList lowerCaseRawHeaders;
    for ( const QByteArray &header : reply.rawHeaderList() )
    {
      lowerCaseRawHeaders.append( header.toLower() );
    }

    QVERIFY( lowerCaseRawHeaders.contains( "content-length" ) );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  QNetworkRequest r( u );
  r.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsNetworkRequestParameters::AttributeInitiatorClass ), u"myTestClass"_s );
  r.setAttribute( static_cast<QNetworkRequest::Attribute>( QgsNetworkRequestParameters::AttributeInitiatorRequestId ), 55 );
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

  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( r, QNetworkAccessManager::GetOperation, QNetworkReply::NoError, QByteArray(), "<title>test</title>" );
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
  QUrl u = QUrl( u"http://"_s + mHttpBinHost + u"/post"_s );
  QNetworkRequest req( u );
  req.setHeader( QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s );
  QString replyContent;

  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::PostOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.content(), QByteArray( "a=b&c=d" ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );

    // newer qt versions force headers to lower case, older ones didn't
    QStringList lowerCaseRawHeaders;
    for ( const QByteArray &header : reply.rawHeaderList() )
    {
      lowerCaseRawHeaders.append( header.toLower() );
    }

    QVERIFY( lowerCaseRawHeaders.contains( "content-type" ) );
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
  QVERIFY( replyContent.contains( u"\"a\": \"b\""_s ) );
  QVERIFY( replyContent.contains( u"\"c\": \"d\""_s ) );
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  // blocking request
  req.setHeader( QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s );
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

  req.setHeader( QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s );
  BackgroundRequest *thread = new BackgroundRequest( req, QNetworkAccessManager::PostOperation, QByteArray( "a=b&c=d" ) );

  thread->start();

  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  replyContent = thread->mReply->readAll();
  QVERIFY( replyContent.contains( u"\"a\": \"b\""_s ) );
  QVERIFY( replyContent.contains( u"\"c\": \"d\""_s ) );
  thread->exit();
  thread->wait();
  thread->deleteLater();

  // blocking request in worker thread
  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;

  req.setHeader( QNetworkRequest::ContentTypeHeader, u"application/x-www-form-urlencoded"_s );
  BackgroundBlockingRequest *blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::PostOperation, QNetworkReply::NoError, QByteArray( "a=b&c=d" ), "\"a\": \"b\"" );
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
  QHttpMultiPart::ContentType contentType = static_cast<QHttpMultiPart::ContentType>( iContentType );
  QHttpMultiPart *multipart = new QHttpMultiPart( contentType );
  QHttpPart part;
  part.setHeader( QNetworkRequest::ContentDispositionHeader, u"form-data; name=\"param\""_s );
  part.setBody( u"some data"_s.toUtf8() );
  multipart->append( part );
  QUrl u = QUrl( u"http://"_s + mHttpBinHost + u"/post"_s );
  QNetworkRequest req( u );

  // MultiPart
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->post( req, multipart );
  multipart->setParent( reply );

  QEventLoop el;
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::finished, &el, &QEventLoop::quit );
  el.exec();

  QCOMPARE( reply->error(), QNetworkReply::NoError );

  // newer qt versions force headers to lower case, older ones didn't
  QStringList lowerCaseRawHeaders;
  for ( const QByteArray &header : reply->rawHeaderList() )
  {
    lowerCaseRawHeaders.append( header.toLower() );
  }
  QVERIFY( lowerCaseRawHeaders.contains( "content-type" ) );

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
  if ( QgsTest::isCIRun() )
  {
    QSKIP( "badssl.com service is not reliable enough for use on CI" );
  }

  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotSslError = false;
  bool gotRequestEncounteredSslError = false;
  int requestId = -1;
  QUrl u = QUrl( u"https://expired.badssl.com"_s );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.error(), QNetworkReply::SslHandshakeFailedError );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, &context, [&]( QNetworkReply *, const QList<QSslError> &errors ) {
    QCOMPARE( errors.at( 0 ).error(), QSslError::CertificateExpired );
    gotSslError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestEncounteredSslErrors, &context, [&]( int errorRequestId, const QList<QSslError> &errors ) {
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
  QNetworkRequest req { u };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), u"SSL handshake failed: The certificate has expired"_s );
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
  if ( QgsTest::isCIRun() )
  {
    QSKIP( "badssl.com service is not reliable enough for use on CI" );
  }

  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique<TestSslErrorHandler>() );

  const QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotSslError = false;
  int requestId = -1;
  bool gotRequestEncounteredSslError = false;
  QUrl u = QUrl( u"https://self-signed.badssl.com/"_s );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.error(), QNetworkReply::NoError ); // because handler ignores error
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QNetworkAccessManager::sslErrors, &context, [&]( QNetworkReply *, const QList<QSslError> &errors ) {
    QCOMPARE( errors.at( 0 ).error(), QSslError::SelfSignedCertificate );
    gotSslError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestEncounteredSslErrors, &context, [&]( int errorRequestId, const QList<QSslError> &errors ) {
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
  QNetworkRequest req { u };
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

  QgsNetworkAccessManager::instance()->setSslErrorHandler( std::make_unique<QgsSslErrorHandler>() );
}

void TestQgsNetworkAccessManager::testAuthRequestHandler()
{
  // initially this request should fail -- we aren't providing the username and password required
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<TestAuthRequestHandler>( QString(), QString() ) );

  const QObject context;
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotAuthRequest = false;
  bool gotAuthDetailsAdded = false;
  QString hash = QUuid::createUuid().toString().mid( 1, 10 );
  QString expectedUser;
  QString expectedPassword;
  int requestId = -1;
  QUrl u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );
  QNetworkReply::NetworkError expectedError = QNetworkReply::NoError;

  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
    QCOMPARE( reply.error(), expectedError );
    QCOMPARE( reply.requestId(), requestId );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );

  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestRequiresAuth, &context, [&]( int authRequestId, const QString &realm ) {
    QCOMPARE( authRequestId, requestId );
    QCOMPARE( realm, u"Fake Realm"_s );
    gotAuthRequest = true;
  } );

  connect( QgsNetworkAccessManager::instance(), &QgsNetworkAccessManager::requestAuthDetailsAdded, &context, [&]( int authRequestId, const QString &realm, const QString &user, const QString &password ) {
    QCOMPARE( authRequestId, requestId );
    QCOMPARE( realm, u"Fake Realm"_s );
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
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;

  QNetworkRequest req { u };
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
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );

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
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );
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
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<TestAuthRequestHandler>( u"me"_s, hash ) );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = u"me"_s;
  expectedPassword = hash;

  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded || !gotAuthRequest || !gotRequestAboutToBeCreatedSignal || !gotAuthDetailsAdded )
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
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<TestAuthRequestHandler>( u"me"_s, hash ) );
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me/"_s + hash );
  req = QNetworkRequest { u };
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
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<TestAuthRequestHandler>( u"me2"_s, hash ) );
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me2/"_s + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = u"me2"_s;
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
  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<TestAuthRequestHandler>( u"me2"_s, hash ) );
  u = QUrl( u"http://"_s + mHttpBinHost + u"/basic-auth/me2/"_s + hash );
  loaded = false;
  gotAuthRequest = false;
  gotRequestAboutToBeCreatedSignal = false;
  gotAuthDetailsAdded = false;
  expectedError = QNetworkReply::NoError;
  expectedUser = u"me2"_s;
  expectedPassword = hash;
  req = QNetworkRequest { u };
  blockingThread = new BackgroundBlockingRequest( req, QNetworkAccessManager::GetOperation, QNetworkReply::NoError, QByteArray(), "\"user\": \"me2\"" );
  blockingThread->start();
  while ( !loaded )
  {
    qApp->processEvents();
  }
  QVERIFY( gotRequestAboutToBeCreatedSignal );
  blockingThread->exit();
  blockingThread->wait();
  blockingThread->deleteLater();

  QgsNetworkAccessManager::instance()->setAuthHandler( std::make_unique<QgsNetworkAuthenticationHandler>() );
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
  QUrl u = QUrl( u"http://"_s + mHttpBinHost + u"/delay/10"_s );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters &params ) {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkRequestParameters>( &QgsNetworkAccessManager::requestTimedOut ), &context, [&]( const QgsNetworkRequestParameters &request ) {
    QCOMPARE( request.requestId(), requestId );
    gotTimeoutError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), qOverload<QgsNetworkReplyContent>( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent &reply ) {
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
  QNetworkRequest req = QNetworkRequest { u };
  const QgsNetworkReplyContent rep = QgsNetworkAccessManager::blockingGet( req );
  QCOMPARE( rep.errorString(), u"Operation canceled"_s );
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
  req = QNetworkRequest { u };
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
    Q_OBJECT
  public:
    FunctionThread( const std::function<bool()> &f )
      : m_f( f ) {}
    bool getResult() const
    {
      return m_result;
    }

  private:
    std::function<bool()> m_f;
    bool m_result = false;

    void run() override
    {
      m_result = m_f();
    }
};

void TestQgsNetworkAccessManager::testCookieManagement()
{
  const QUrl url( "http://example.com" );
  // Set cookie in a thread and verify that it also set in main thread
  QEventLoop evLoop;
  FunctionThread thread1( [url] {
    QgsNetworkAccessManager::instance()->cookieJar()->setCookiesFromUrl(
      QList<QNetworkCookie>() << QNetworkCookie( "foo=bar" ), url
    );
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

  FunctionThread thread2( [url] {
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
  const QString processorId = QgsNetworkAccessManager::setRequestPreprocessor( []( QNetworkRequest *request ) { request->setHeader( QNetworkRequest::UserAgentHeader, u"QGIS"_s ); } );
  QNetworkRequest request;
  QgsNetworkAccessManager::instance()->preprocessRequest( &request );
  const QString userAgent = request.header( QNetworkRequest::UserAgentHeader ).toString();
  QCOMPARE( userAgent, "QGIS" );
  QgsNetworkAccessManager::removeRequestPreprocessor( processorId );
};

QGSTEST_MAIN( TestQgsNetworkAccessManager )
#include "testqgsnetworkaccessmanager.moc"
