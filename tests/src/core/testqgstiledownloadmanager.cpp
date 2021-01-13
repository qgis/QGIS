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
#include <QtTest>
#include <iostream>

#include "qgsapplication.h"
#include "qgstiledownloadmanager.h"

const QString url_1 = "https://www.qwant.com/maps/tiles/ozbasemap/0/0/0.pbf";
const QString url_2 = "https://www.qwant.com/maps/tiles/ozbasemap/1/0/0.pbf";
const QString url_bad = "http://www.example.com/download-manager-fail";


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

};


void TestQgsTileDownloadManager::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsTileDownloadManager::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTileDownloadManager::testOneRequest()
{
  // the simplest test: make a single request and check that data
  // will be returned at some point later

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

  QSignalSpy spy( r.get(), &QgsTileDownloadManagerReply::finished );
  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !r->data().isEmpty() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
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

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

  QThread::usleep( 1000 );  // sleep 1ms - enough time to start request but not enough to finish it

  r.reset();

  // we are not really expecting the worker thread to abort
  // the work, hoping that if we download this item, it may
  // be needed sometime soon

  QVERIFY( QgsTileDownloadManager::waitForPendingRequests() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
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

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

  QSignalSpy spy1( r1.get(), &QgsTileDownloadManagerReply::finished );
  QSignalSpy spy2( r2.get(), &QgsTileDownloadManagerReply::finished );
  spy1.wait();
  QCOMPARE( spy1.count(), 1 );
  QCOMPARE( spy2.count(), 1 );
  QVERIFY( !r1->data().isEmpty() );
  QVERIFY( !r2->data().isEmpty() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
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

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

  QThread::usleep( 1000 );  // sleep 1ms - enough time to start request but not enough to finish it

  r1.reset();

  QSignalSpy spy( r2.get(), &QgsTileDownloadManagerReply::finished );
  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( !r2->data().isEmpty() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
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

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r( QgsTileDownloadManager::get( QNetworkRequest( url_bad ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

  QSignalSpy spy( r.get(), &QgsTileDownloadManagerReply::finished );
  spy.wait();
  QCOMPARE( spy.count(), 1 );
  QVERIFY( r->data().isEmpty() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
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

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );
  QgsTileDownloadManager::resetStatistics();

  std::unique_ptr<QgsTileDownloadManagerReply> r1( QgsTileDownloadManager::get( QNetworkRequest( url_1 ) ) );
  std::unique_ptr<QgsTileDownloadManagerReply> r2( QgsTileDownloadManager::get( QNetworkRequest( url_2 ) ) );

  QVERIFY( QgsTileDownloadManager::hasPendingRequests() );

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
  QVERIFY( !r2->data().isEmpty() );

  QVERIFY( !QgsTileDownloadManager::hasPendingRequests() );

  QgsTileDownloadManager::Stats stats = QgsTileDownloadManager::statistics();
  QCOMPARE( stats.requestsTotal, 2 );
  QCOMPARE( stats.requestsMerged, 0 );
  QCOMPARE( stats.requestsEarlyDeleted, 0 );
  QCOMPARE( stats.networkRequestsStarted, 2 );
  QCOMPARE( stats.networkRequestsOk, 2 );
  QCOMPARE( stats.networkRequestsFailed, 0 );
}

QTEST_MAIN( TestQgsTileDownloadManager )
#include "testqgstiledownloadmanager.moc"
