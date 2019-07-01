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


#include "qgstest.h"
#include <QObject>

#include "qgsnewsfeedparser.h"
#include "qgssettings.h"

class TestQgsNewsFeedParser: public QObject
{
    Q_OBJECT
  public:
    TestQgsNewsFeedParser() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testFetch();

};


void TestQgsNewsFeedParser::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
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
  QList< QgsNewsFeedParser::Entry > entries;

  const QUrl url( QUrl::fromLocalFile( QStringLiteral( TEST_DATA_DIR ) + "/newsfeed/feed" ) );
  const QString feedKey = QgsNewsFeedParser::keyForFeed( url.toString() );
  QgsSettings().remove( feedKey, QgsSettings::Core );

  uint beforeTime = QDateTime::currentDateTimeUtc().toTime_t();

  QgsNewsFeedParser parser( url );
  QVERIFY( parser.entries().isEmpty() );
  QEventLoop loop;
  connect( &parser, &QgsNewsFeedParser::fetched, this, [ =, &loop, &entries ]( const  QList< QgsNewsFeedParser::Entry > &e )
  {
    entries = e;
    loop.quit();
  } );
  parser.fetch();
  loop.exec();

  // check result
  QCOMPARE( entries.count(), 5 );
  QCOMPARE( entries.at( 0 ).title, QStringLiteral( "Next Microsoft Windows code name revealed" ) );
  QCOMPARE( entries.at( 1 ).title, QStringLiteral( "QGIS core will be rewritten in Rust" ) );
  QCOMPARE( entries.at( 2 ).title, QStringLiteral( "QGIS Italian Meeting" ) );
  QCOMPARE( entries.at( 3 ).title, QStringLiteral( "QGIS acquired by ESRI" ) );
  QCOMPARE( entries.at( 4 ).title, QStringLiteral( "Null Island QGIS Meeting" ) );

  QCOMPARE( parser.entries().count(), 5 );
  QCOMPARE( parser.entries().at( 0 ).title, QStringLiteral( "Next Microsoft Windows code name revealed" ) );
  QCOMPARE( parser.entries().at( 1 ).title, QStringLiteral( "QGIS core will be rewritten in Rust" ) );
  QCOMPARE( parser.entries().at( 2 ).title, QStringLiteral( "QGIS Italian Meeting" ) );
  QCOMPARE( parser.entries().at( 3 ).title, QStringLiteral( "QGIS acquired by ESRI" ) );
  QCOMPARE( parser.entries().at( 4 ).title, QStringLiteral( "Null Island QGIS Meeting" ) );

  entries.clear();

  // after a fetch, the current timestamp should be saved to avoid refetching these
  uint after = QgsSettings().value( feedKey + "/lastFetchTime", 0, QgsSettings::Core ).toUInt();
  QVERIFY( after >= beforeTime );

  // reset to a standard known last time
  QgsSettings().remove( feedKey, QgsSettings::Core );
  QgsSettings().setValue( feedKey + "/lastFetchTime", 1457360008, QgsSettings::Core );

  // refetch, only new items should be fetched
  QgsNewsFeedParser parser2( url );
  QVERIFY( parser2.entries().isEmpty() );
  QEventLoop loop2;
  connect( &parser2, &QgsNewsFeedParser::fetched, this, [ =, &loop2, &entries ]( const  QList< QgsNewsFeedParser::Entry > &e )
  {
    entries = e;
    loop2.quit();
  } );

  parser2.fetch();
  loop2.exec();

  // check only new entries are present
  QCOMPARE( entries.count(), 4 );
  QCOMPARE( entries.at( 0 ).title, QStringLiteral( "QGIS acquired by ESRI" ) );
  QCOMPARE( entries.at( 1 ).title, QStringLiteral( "Next Microsoft Windows code name revealed" ) );
  QCOMPARE( entries.at( 2 ).title, QStringLiteral( "Null Island QGIS Meeting" ) );
  QCOMPARE( entries.at( 3 ).title, QStringLiteral( "QGIS Italian Meeting" ) );

  QCOMPARE( parser2.entries().count(), 4 );
  QCOMPARE( parser2.entries().at( 0 ).title, QStringLiteral( "QGIS acquired by ESRI" ) );
  QCOMPARE( parser2.entries().at( 1 ).title, QStringLiteral( "Next Microsoft Windows code name revealed" ) );
  QCOMPARE( parser2.entries().at( 2 ).title, QStringLiteral( "Null Island QGIS Meeting" ) );
  QCOMPARE( parser2.entries().at( 3 ).title, QStringLiteral( "QGIS Italian Meeting" ) );

  entries.clear();

  // make a new parser with existing stored entries
  QgsNewsFeedParser parser3( url );
  // previous entries should be automatically read
  QCOMPARE( parser3.entries().count(), 4 );
  QCOMPARE( parser3.entries().at( 0 ).title, QStringLiteral( "QGIS acquired by ESRI" ) );
  QCOMPARE( parser3.entries().at( 1 ).title, QStringLiteral( "Next Microsoft Windows code name revealed" ) );
  QCOMPARE( parser3.entries().at( 2 ).title, QStringLiteral( "Null Island QGIS Meeting" ) );
  QCOMPARE( parser3.entries().at( 3 ).title, QStringLiteral( "QGIS Italian Meeting" ) );

}


QGSTEST_MAIN( TestQgsNewsFeedParser )
#include "testqgsnewsfeedparser.moc"


