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
#include <QNetworkReply>


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
    void fetchEmptyUrl(); //test fetching blank url
    void fetchBadUrl(); //test fetching bad url
    void fetchEncodedContent(); //test fetching url content encoded as utf-8
    void fetchPost();
    void fetchBadSsl();
    void fetchTimeout();

};

void TestQgsNetworkAccessManager::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsSettings().setValue( QStringLiteral( "/qgis/networkAndProxy/networkTimeout" ), 1000 );
  QgsApplication::init();
  QgsApplication::initQgis();
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

void TestQgsNetworkAccessManager::fetchEmptyUrl()
{
  QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl() );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
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
}

void TestQgsNetworkAccessManager::fetchBadUrl()
{
  QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), QUrl( QStringLiteral( "http://x" ) ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
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
}


void TestQgsNetworkAccessManager::fetchEncodedContent()
{
  QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  QUrl u =  QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + '/' +  "encoded_html.html" );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.initiatorClassName(), QStringLiteral( "myTestClass" ) );
    QCOMPARE( params.initiatorRequestId().toInt(), 55 );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
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
}

void TestQgsNetworkAccessManager::fetchPost()
{
  if ( QgsTest::isTravis() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "http://httpbin.org/post" ) );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::PostOperation );
    QCOMPARE( params.request().url(), u );
    QCOMPARE( params.content(), QByteArray( "a=b&c=d" ) );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );
    QVERIFY( reply.rawHeaderList().contains( "Content-Length" ) );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  QNetworkRequest req( u );
  req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral( "application/x-www-form-urlencoded" ) );
  QNetworkReply *reply = QgsNetworkAccessManager::instance()->post( req, QByteArray( "a=b&c=d" ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
  QString replyContent = reply->readAll();
  QVERIFY( replyContent.contains( QStringLiteral( "\"a\": \"b\"" ) ) );
  QVERIFY( replyContent.contains( QStringLiteral( "\"c\": \"d\"" ) ) );

  gotRequestAboutToBeCreatedSignal = false;
  loaded = false;
  req = QNetworkRequest( u );
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
}

void TestQgsNetworkAccessManager::fetchBadSsl()
{
  if ( QgsTest::isTravis() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QObject context;
  //test fetching from a blank url
  bool loaded = false;
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotSslError = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "https://expired.badssl.com" ) );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
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
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded && !gotSslError )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );

  // we don't test for background thread ssl error yet -- that signal isn't thread safe
}

void TestQgsNetworkAccessManager::fetchTimeout()
{
  if ( QgsTest::isTravis() )
    QSKIP( "This test is disabled on Travis CI environment" );

  QObject context;
  //test fetching from a blank url
  bool gotRequestAboutToBeCreatedSignal = false;
  bool gotTimeoutError = false;
  bool finished = false;
  int requestId = -1;
  QUrl u =  QUrl( QStringLiteral( "http://httpbin.org/delay/10" ) );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestAboutToBeCreated ), &context, [&]( const QgsNetworkRequestParameters & params )
  {
    gotRequestAboutToBeCreatedSignal = true;
    requestId = params.requestId();
    QVERIFY( requestId > 0 );
    QCOMPARE( params.operation(), QNetworkAccessManager::GetOperation );
    QCOMPARE( params.request().url(), u );
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkRequestParameters >::of( &QgsNetworkAccessManager::requestTimedOut ), &context, [&]( const QgsNetworkRequestParameters & request )
  {
    QCOMPARE( request.requestId(), requestId );
    gotTimeoutError = true;
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
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
}

QGSTEST_MAIN( TestQgsNetworkAccessManager )
#include "testqgsnetworkaccessmanager.moc"
