/***************************************************************************
    testqgsnewsfeedparser.cpp
     --------------------------------------
    Date                 : July 2019
    Copyright            : (C) 2019 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnewsfeedmodel.h"
#include "qgsnewsfeedparser.h"
#include "qgstest.h"

#include <QObject>
#include <QSignalSpy>

class TestQgsNewsFeedParser : public QObject
{
    Q_OBJECT
  public:
    TestQgsNewsFeedParser() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testFetch();
    void testAutoExpiry();
    void testLang();
    void testGeoFencing();
    void testModel();
    void testProxyModel();
    void testUpdatedEntries();
};


void TestQgsNewsFeedParser::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );
}

void TestQgsNewsFeedParser::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsNewsFeedParser::init()
{
}


void TestQgsNewsFeedParser::cleanup()
{
}

void TestQgsNewsFeedParser::testFetch()
{
  QList<QgsNewsFeedParser::Entry> entries;

  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );

  const qint64 beforeTime = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();

  QgsNewsFeedParser parser( url );
  const QSignalSpy spy( &parser, &QgsNewsFeedParser::entryAdded );
  QVERIFY( parser.entries().isEmpty() );
  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop.quit();
  } );
  parser.fetch();
  loop.exec();

  // check result
  QCOMPARE( spy.count(), 5 );
  QCOMPARE( entries.count(), 5 );
  QCOMPARE( entries.at( 0 ).title, u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( entries.at( 1 ).title, u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( entries.at( 2 ).title, u"QGIS Italian Meeting"_s );
  QCOMPARE( entries.at( 3 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( entries.at( 4 ).title, u"Null Island QGIS Meeting"_s );

  QCOMPARE( parser.entries().count(), 5 );
  QCOMPARE( parser.entries().at( 0 ).title, u"Next Microsoft Windows code name revealed"_s );
  QVERIFY( parser.entries().at( 0 ).expiry.isValid() );
  QCOMPARE( parser.entries().at( 0 ).expiry.toUTC(), QDateTime( QDate( 2027, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
  QCOMPARE( parser.entries().at( 1 ).title, u"QGIS core will be rewritten in Rust"_s );
  QVERIFY( !parser.entries().at( 1 ).expiry.isValid() );
  QCOMPARE( parser.entries().at( 2 ).title, u"QGIS Italian Meeting"_s );
  QCOMPARE( parser.entries().at( 3 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( parser.entries().at( 4 ).title, u"Null Island QGIS Meeting"_s );

  entries.clear();

  // after a fetch, the current timestamp should be saved to avoid refetching these
  const uint after = QgsNewsFeedParser::settingsFeedLastFetchTime->value( feedKey );
  QVERIFY( after >= beforeTime );

  // reset to a standard known last time
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );
  QgsNewsFeedParser::settingsFeedLastFetchTime->setValue( 1457360008, feedKey );

  // refetch, only new items should be fetched
  QgsNewsFeedParser parser2( url );
  QVERIFY( parser2.entries().isEmpty() );
  QEventLoop loop2;
  connect( &parser2, &QgsNewsFeedParser::fetched, this, [=, &loop2, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop2.quit();
  } );

  parser2.fetch();
  loop2.exec();

  // check only new entries are present
  QCOMPARE( entries.count(), 4 );
  QCOMPARE( entries.at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QVERIFY( parser.entries().at( 0 ).expiry.isValid() );
  QCOMPARE( parser.entries().at( 0 ).expiry.toUTC(), QDateTime( QDate( 2027, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
  QCOMPARE( entries.at( 1 ).title, u"Next Microsoft Windows code name revealed"_s );
  QVERIFY( !parser.entries().at( 1 ).expiry.isValid() );
  QCOMPARE( entries.at( 2 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( entries.at( 3 ).title, u"QGIS Italian Meeting"_s );

  QCOMPARE( parser2.entries().count(), 4 );
  QCOMPARE( parser2.entries().at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( parser2.entries().at( 1 ).title, u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( parser2.entries().at( 2 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser2.entries().at( 3 ).title, u"QGIS Italian Meeting"_s );

  entries.clear();

  // make a new parser with existing stored entries
  QgsNewsFeedParser parser3( url );
  // previous entries should be automatically read
  QCOMPARE( parser3.entries().count(), 4 );
  QCOMPARE( parser3.entries().at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QVERIFY( parser.entries().at( 0 ).expiry.isValid() );
  QCOMPARE( parser.entries().at( 0 ).expiry.toUTC(), QDateTime( QDate( 2027, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
  QCOMPARE( parser3.entries().at( 1 ).title, u"Next Microsoft Windows code name revealed"_s );
  QVERIFY( !parser.entries().at( 1 ).expiry.isValid() );
  QCOMPARE( parser3.entries().at( 2 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser3.entries().at( 3 ).title, u"QGIS Italian Meeting"_s );

  // dismiss imaginary entry
  const QSignalSpy dismissSpy( &parser3, &QgsNewsFeedParser::entryDismissed );
  parser3.dismissEntry( -1 );
  QCOMPARE( dismissSpy.count(), 0 );
  QCOMPARE( parser3.entries().count(), 4 );

  // dismiss valid entry
  parser3.dismissEntry( 4 );
  QCOMPARE( dismissSpy.count(), 1 );
  QCOMPARE( parser3.entries().count(), 3 );
  QCOMPARE( parser3.entries().at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( parser3.entries().at( 1 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser3.entries().at( 2 ).title, u"QGIS Italian Meeting"_s );

  // craft a new parser, should not have dismissed entry
  QgsNewsFeedParser parser4( url );
  QCOMPARE( parser4.entries().count(), 3 );
  QCOMPARE( parser4.entries().at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( parser4.entries().at( 1 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser4.entries().at( 2 ).title, u"QGIS Italian Meeting"_s );
  // even if we re-fetch, the dismissed entry should not come back
  parser4.fetch();
  QCOMPARE( parser4.entries().count(), 3 );
  QCOMPARE( parser4.entries().at( 0 ).title, u"QGIS acquired by ESRI"_s );
  QCOMPARE( parser4.entries().at( 1 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser4.entries().at( 2 ).title, u"QGIS Italian Meeting"_s );

  // dismiss all
  parser4.dismissAll();
  QCOMPARE( parser4.entries().count(), 0 );

  const QgsNewsFeedParser parser5( url );
  QCOMPARE( parser5.entries().count(), 0 );
}

void TestQgsNewsFeedParser::testAutoExpiry()
{
  const QUrl url( u"xxx"_s );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );

  // ensure entries "auto expire" when past their use-by date
  QgsNewsFeedParser::Entry testEntry;
  testEntry.key = 1;
  testEntry.title = u"test entry"_s;
  QgsNewsFeedParser::Entry testEntry2;
  testEntry2.key = 2;
  testEntry2.title = u"test entry2"_s;
  testEntry2.expiry = QDateTime( QDate( 1997, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC );

  QgsNewsFeedParser parser( url );
  parser.storeEntryInSettings( testEntry );
  parser.storeEntryInSettings( testEntry2 );

  // on relaunch, expired entries should be auto-pruned
  const QgsNewsFeedParser parser2( url );
  QCOMPARE( parser2.entries().count(), 1 );
  QCOMPARE( parser2.entries().at( 0 ).title, u"test entry"_s );
  QVERIFY( !parser2.entries().at( 0 ).expiry.isValid() );
}

void TestQgsNewsFeedParser::testLang()
{
  QList<QgsNewsFeedParser::Entry> entries;

  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );
  // force to Spanish language
  QgsNewsFeedParser::settingsFeedLanguage->setValue( u"es"_s, feedKey );

  QgsNewsFeedParser parser( url );
  const QSignalSpy spy( &parser, &QgsNewsFeedParser::entryAdded );
  QVERIFY( parser.entries().isEmpty() );
  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop.quit();
  } );

  parser.fetch();
  loop.exec();

  QCOMPARE( entries.count(), 1 );
  QCOMPARE( entries.at( 0 ).title, u"Primer seminario SIG libre"_s );
}

void TestQgsNewsFeedParser::testGeoFencing()
{
  QList<QgsNewsFeedParser::Entry> entries;

  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );
  QgsNewsFeedParser::settingsFeedLatitude->setValue( 37.2343, feedKey );
  QgsNewsFeedParser::settingsFeedLongitude->setValue( -115.8067, feedKey );

  QgsNewsFeedParser parser( url );
  const QSignalSpy spy( &parser, &QgsNewsFeedParser::entryAdded );
  QVERIFY( parser.entries().isEmpty() );
  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop.quit();
  } );

  parser.fetch();
  loop.exec();

  // check only geofenced entries are present (i.e. that request has included lat/lon params)
  QCOMPARE( entries.count(), 1 );
  QCOMPARE( entries.at( 0 ).title, u"Secret docs leaked"_s );
}

void TestQgsNewsFeedParser::testModel()
{
  // test news feed model
  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );

  QgsNewsFeedParser parser( url );
  const QgsNewsFeedModel model( &parser );
  QCOMPARE( model.rowCount(), 0 );

  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop]( const QList<QgsNewsFeedParser::Entry> & ) {
    loop.quit();
  } );
  parser.fetch();
  loop.exec();

  QCOMPARE( model.rowCount(), 5 );
  QVERIFY( model.data( model.index( 0, 0, QModelIndex() ), Qt::DisplayRole ).toString().startsWith( "<p>Rumors from a whistleblower revealed the next Windows release code nam"_L1 ) );
  QVERIFY( model.data( model.index( 1, 0, QModelIndex() ), Qt::DisplayRole ).toString().startsWith( "<p>Tired with C++ intricacies, the core developers h"_L1 ) );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), Qt::DisplayRole ).toString(), u"<p>Ciao from Italy!</p>"_s );
  QVERIFY( model.data( model.index( 3, 0, QModelIndex() ), Qt::DisplayRole ).toString().startsWith( "<p>QGIS is finally part of the ESRI ecosystem, i"_L1 ) );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), Qt::DisplayRole ).toString(), u"<p>Let's dive in the ocean together!</p>"_s );
  QVERIFY( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Content ) ).toString().startsWith( "<p>Rumors from a whistleblower revealed the next Windows release code nam"_L1 ) );
  QVERIFY( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Content ) ).toString().startsWith( "<p>Tired with C++ intricacies, the core developers h"_L1 ) );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Content ) ).toString(), u"<p>Ciao from Italy!</p>"_s );
  QVERIFY( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Content ) ).toString().startsWith( "<p>QGIS is finally part of the ESRI ecosystem, i"_L1 ) );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Content ) ).toString(), u"<p>Let's dive in the ocean together!</p>"_s );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), u"QGIS Italian Meeting"_s );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), u"QGIS acquired by ESRI"_s );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), Qt::ToolTipRole ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS Italian Meeting"_s );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 4 );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 6 );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 11 );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 3 );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 5 );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::ImageUrl ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::ImageUrl ) ).toString(), u"http://0.0.0.0:8000/media/feedimages/rust.png"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::ImageUrl ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::ImageUrl ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::ImageUrl ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Link ) ).toString(), u"https://www.winux.microsoft.com"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Link ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Link ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Link ) ).toString(), u"https://www.qgis.com"_s );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Link ) ).toString(), QString() );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), true );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), true );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );

  // remove an entry
  parser.dismissEntry( 11 );
  QCOMPARE( model.rowCount(), 4 );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );

  // construct a new model/parser -- should initially have stored entries
  QgsNewsFeedParser parser2( url );
  const QgsNewsFeedModel model2( &parser2 );
  QCOMPARE( model2.rowCount(), 4 );
  QCOMPARE( model2.data( model2.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );
  QCOMPARE( model2.data( model2.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model2.data( model2.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model2.data( model2.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
}

void TestQgsNewsFeedParser::testProxyModel()
{
  // test news feed proxy model
  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );

  QgsNewsFeedParser parser( url );
  const QgsNewsFeedProxyModel model( &parser );
  QCOMPARE( model.rowCount(), 0 );

  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop]( const QList<QgsNewsFeedParser::Entry> & ) {
    loop.quit();
  } );
  parser.fetch();
  loop.exec();

  QCOMPARE( model.rowCount(), 5 );
  // stickies first, then sort by key descending (i.e. more recently published entries first)
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), true );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), true );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Sticky ) ).toBool(), false );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 6 );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 4 );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 11 );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 5 );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Key ) ).toInt(), 3 );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS Italian Meeting"_s );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model.data( model.index( 4, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );

  // remove an entry
  parser.dismissEntry( 11 );
  QCOMPARE( model.rowCount(), 4 );
  QCOMPARE( model.data( model.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model.data( model.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model.data( model.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model.data( model.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );

  // construct a new model/parser -- should initially have stored entries
  QgsNewsFeedParser parser2( url );
  const QgsNewsFeedProxyModel model2( &parser2 );
  QCOMPARE( model2.rowCount(), 4 );
  QCOMPARE( model2.data( model2.index( 0, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS core will be rewritten in Rust"_s );
  QCOMPARE( model2.data( model2.index( 1, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( model2.data( model2.index( 2, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"Null Island QGIS Meeting"_s );
  QCOMPARE( model2.data( model2.index( 3, 0, QModelIndex() ), static_cast<int>( QgsNewsFeedModel::CustomRole::Title ) ).toString(), u"QGIS acquired by ESRI"_s );
}

void TestQgsNewsFeedParser::testUpdatedEntries()
{
  QList<QgsNewsFeedParser::Entry> entries;

  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );

  // reset to a standard known last time
  QgsNewsFeedParser::sTreeNewsFeed->deleteItem( feedKey );
  QgsNewsFeedParser::settingsFeedLastFetchTime->setValue( 1457360008, feedKey );

  // refetch, only new items should be fetched
  QgsNewsFeedParser parser( url );
  QVERIFY( parser.entries().isEmpty() );
  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [=, &loop, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop.quit();
  } );

  parser.fetch();
  loop.exec();

  // check only new entries are present
  QCOMPARE( entries.count(), 4 );

  entries.clear();

  // Now request a new date and check that:
  // - entry 3 has a new expiry date in the past so it is removed
  // - entry 11 has been modified

  QgsNewsFeedParser::settingsFeedLastFetchTime->setValue( 1557079653, feedKey );

  QgsNewsFeedParser parser2( url );
  QVERIFY( !parser2.entries().isEmpty() );
  QEventLoop loop2;
  connect( &parser2, &QgsNewsFeedParser::fetched, this, [=, &loop2, &entries]( const QList<QgsNewsFeedParser::Entry> &e ) {
    entries = e;
    loop2.quit();
  } );

  const QSignalSpy spyUpdated( &parser2, &QgsNewsFeedParser::entryUpdated );
  const QSignalSpy spyDismissed( &parser2, &QgsNewsFeedParser::entryDismissed );

  parser2.fetch();
  loop2.exec();

  QCOMPARE( spyDismissed.count(), 1 );
  QCOMPARE( spyUpdated.count(), 1 );

  QCOMPARE( entries.count(), 2 );
  QCOMPARE( parser2.entries().count(), 3 );
  QCOMPARE( parser2.entries().at( 0 ).title, u"Next Microsoft Windows code name revealed"_s );
  QCOMPARE( parser2.entries().at( 1 ).title, u"Null Island QGIS Meeting"_s );
  QCOMPARE( parser2.entries().at( 2 ).title, u"QGIS Italian Meeting Revisited"_s );
  QCOMPARE( parser2.entries().at( 2 ).expiry.toSecsSinceEpoch(), 7868426853 );
}


QGSTEST_MAIN( TestQgsNewsFeedParser )
#include "testqgsnewsfeedparser.moc"
