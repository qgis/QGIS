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
#include <QObject>
#include "qgstest.h"
#include <QNetworkReply>

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

};

void TestQgsNetworkAccessManager::initTestCase()
{
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
  } );
  connect( QgsNetworkAccessManager::instance(), qgis::overload< QgsNetworkReplyContent >::of( &QgsNetworkAccessManager::finished ), &context, [&]( const QgsNetworkReplyContent & reply )
  {
    QCOMPARE( reply.error(), QNetworkReply::NoError );
    QCOMPARE( reply.requestId(), requestId );
    QVERIFY( reply.rawHeaderList().contains( "Content-Length" ) );
    QCOMPARE( reply.request().url(), u );
    loaded = true;
  } );
  QgsNetworkAccessManager::instance()->get( QNetworkRequest( u ) );

  while ( !loaded )
  {
    qApp->processEvents();
  }

  QVERIFY( gotRequestAboutToBeCreatedSignal );
}

QGSTEST_MAIN( TestQgsNetworkAccessManager )
#include "testqgsnetworkaccessmanager.moc"
