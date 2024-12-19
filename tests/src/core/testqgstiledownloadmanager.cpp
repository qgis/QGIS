/***************************************************************************
                         testqgstiledownloadmanager.cpp
                         ----------------------
    begin                : January 2021
    copyright            : (C) 2021 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCoreApplication>
#include <QTimer>
#include <QTest>
#include <QAbstractNetworkCache>
#include <iostream>
#include <memory>
#include <QSignalSpy>

#include "qgsapplication.h"
#include "qgstiledownloadmanager.h"
#include "qgsnetworkaccessmanager.h"

const QString url_1 = "https://www.qwant.com/maps/tiles/ozbasemap/0/0/0.pbf";
const QString url_2 = "https://www.qwant.com/maps/tiles/ozbasemap/1/0/0.pbf";
const QString url_bad = "https://www.qwant.com/maps/tiles/ozbasemap/1/0/90913.pbf";


class TestQgsTileDownloadManager : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testOneRequest();
    void testOneRequestEarlyDelete();
    void testOneRequestTwice();
    void testOneRequestTwiceAndEarlyDelete();
    void testOneRequestFailure();
    void testTwoRequests();
    void testShutdownWithPendingRequest();
    void testIdleThread();

};


void TestQgsTileDownloadManager::initTestCase()
{
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsNetworkAccessManager::instance()->cache()->clear();
}

void TestQgsTileDownloadManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTileDownloadManager::testOneRequest()
{
  // the simplest test: make a single request and check that data
  // will be returned at some point later

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( manager.get( QNetworkRequest( url_1 ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QSignalSpy spy( r.get(), &QgsTileDownloadManagerReply::finished );
  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !r->data().isEmpty() );
  QVERIFY( r->error() == QNetworkReply::NoError );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 1 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 1 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}

void TestQgsTileDownloadManager::testOneRequestEarlyDelete()
{
  // make a single request and then delete it before the data
  // are actually returned - nothing should break

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( manager.get( QNetworkRequest( url_1 ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QThread::usleep( 1000 );  // sleep 1ms - enough time to start request but not enough to finish it

  r.reset();

  // we are not really expecting the worker thread to abort
  // the work, hoping that if we download this item, it may
  // be needed sometime soon

  QVERIFY( manager.waitForPendingRequests() );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 1 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 1 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 1 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}


void TestQgsTileDownloadManager::testOneRequestTwice()
{
  // start two requests for a single URL, and make sure that
  // both replies will receive the data

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( manager.get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( manager.get( QNetworkRequest( url_1 ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QSignalSpy spy1( r1.get(), &QgsTileDownloadManagerReply::finished );
  QSignalSpy spy2( r2.get(), &QgsTileDownloadManagerReply::finished );
  spy1.wait();
  spy2.wait();
  QCoreApplication::processEvents();
  QCOMPARE( spy1.count(), 1 );
  QCOMPARE( spy2.count(), 1 );
  QVERIFY( !r1->data().isEmpty() );
  QVERIFY( r1->error() == QNetworkReply::NoError );
  QVERIFY( !r2->data().isEmpty() );
  QVERIFY( r2->error() == QNetworkReply::NoError );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 2 );
  QCOMPARE( stats.requestsMerged, 1 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 1 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}

void TestQgsTileDownloadManager::testOneRequestTwiceAndEarlyDelete()
{
  // start two requests for a single URL, then delete one request
  // before it is actually finished: the other request should be
  // finalized correctly with nothing broken

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( manager.get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( manager.get( QNetworkRequest( url_1 ) ) );

  QVERIFY( manager.hasPendingRequests() );
  QSignalSpy spy( r2.get(), &QgsTileDownloadManagerReply::finished );

  QThread::usleep( 1000 );  // sleep 1ms - enough time to start request but not enough to finish it

  r1.reset();

  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !r2->data().isEmpty() );
  QVERIFY( r2->error() == QNetworkReply::NoError );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 2 );
  QCOMPARE( stats.requestsMerged, 1 );
  QCOMPARE( stats.requestsEarlyDeleted, 1 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 1 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}

void TestQgsTileDownloadManager::testOneRequestFailure()
{
  // start a request that will try to fetch URL that does not exist

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( manager.get( QNetworkRequest( url_bad ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QSignalSpy spy( r.get(), &QgsTileDownloadManagerReply::finished );
  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( r->error() != QNetworkReply::NoError );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 1 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 0 );
  QCOMPARE( stats.networkRequestsFailed, 1 );
}

void TestQgsTileDownloadManager::testTwoRequests()
{
  // test multiple parallel requests to different URLs

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( manager.get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( manager.get( QNetworkRequest( url_2 ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QSignalSpy spy1( r1.get(), &QgsTileDownloadManagerReply::finished );
  QSignalSpy spy2( r2.get(), &QgsTileDownloadManagerReply::finished );

  QVERIFY( spy1.wait() );
  if ( spy2.isEmpty() )
  {
    QVERIFY( spy2.wait() );  // r1 it may have finished earlier...
  }
  QCOMPARE( spy1.count(), 1 );
  QCOMPARE( spy2.count(), 1 );
  QVERIFY( !r1->data().isEmpty() );
  QVERIFY( r1->error() == QNetworkReply::NoError );
  QVERIFY( !r2->data().isEmpty() );
  QVERIFY( r2->error() == QNetworkReply::NoError );

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 2 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 2 );
  QCOMPARE( stats.networkRequestsOk, 2 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}

void TestQgsTileDownloadManager::testShutdownWithPendingRequest()
{
  // test that if we shut down download manager while there is a pending request,
  // nothing breaks and the request fails as expected

  QgsNetworkAccessManager::instance()->cache()->clear();

  QgsTileDownloadManager manager;

  QVERIFY( !manager.hasPendingRequests() );
  manager.resetStatistics();

  const std::unique_ptr<QgsTileDownloadManagerReply> r( manager.get( QNetworkRequest( url_1 ) ) );

  QVERIFY( manager.hasPendingRequests() );

  QThread::usleep( 1000 );  // sleep 1ms - enough time to start request but not enough to finish it

  manager.shutdown();

  QVERIFY( !manager.hasPendingRequests() );

  const QgsTileDownloadManager::Stats stats = manager.statistics();
  QCOMPARE( stats.requestsTotal, 1 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 1 );
  QCOMPARE( stats.networkRequestsOk, 0 );
  QCOMPARE( stats.networkRequestsFailed, 1 );
}

void TestQgsTileDownloadManager::testIdleThread()
{
  // check that the worker thread gets killed after some time when it is idle

  QgsTileDownloadManager manager;
  manager.setIdleThreadTimeout( 1000 );  // shorter timeout so that we don't need to wait for too long

  QVERIFY( !manager.hasWorkerThreadRunning() );

  const std::unique_ptr<QgsTileDownloadManagerReply> r1( manager.get( QNetworkRequest( url_1 ) ) );

  QSignalSpy spy1( r1.get(), &QgsTileDownloadManagerReply::finished );
  spy1.wait();
  QCOMPARE( spy1.count(), 1 );

  QVERIFY( manager.hasWorkerThreadRunning() );

  QThread::usleep( 1500000 );  // sleep 1.5s - enough time to get the thread killed due to being idle

  QVERIFY( !manager.hasWorkerThreadRunning() );

  // check that the thread can be restarted again

  const std::unique_ptr<QgsTileDownloadManagerReply> r2( manager.get( QNetworkRequest( url_2 ) ) );

  QSignalSpy spy2( r2.get(), &QgsTileDownloadManagerReply::finished );
  spy2.wait();
  QCOMPARE( spy2.count(), 1 );
}

QTEST_MAIN( TestQgsTileDownloadManager )
#include "testqgstiledownloadmanager.moc"
