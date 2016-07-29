/***************************************************************************
  testopenstreetmap.cpp
  --------------------------------------
  Date                 : January 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <qgsapplication.h>
//#include <qgsproviderregistry.h>

#include "openstreetmap/qgsosmdatabase.h"
#include "openstreetmap/qgsosmdownload.h"
#include "openstreetmap/qgsosmimport.h"

class TestOpenStreetMap : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() ;// will be called before each testfunction is executed.
    void cleanup() ;// will be called after every testfunction.
    /** Our tests proper begin here */
    void download();
    void importAndQueries();
  private:

};

void  TestOpenStreetMap::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  //QgsApplication::initQgis();
  //QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  //create a map layer that will be used in all tests...
  //QString myBaseFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
}
void  TestOpenStreetMap::cleanupTestCase()
{

}
void  TestOpenStreetMap::init()
{

}
void  TestOpenStreetMap::cleanup()
{
}


void TestOpenStreetMap::download()
{
  QgsRectangle rect( 7.148, 51.249, 7.152, 51.251 );

  // start download
  QgsOSMDownload download;
  download.setQuery( QgsOSMDownload::queryFromRect( rect ) );
  download.setOutputFileName( QDir::tempPath() + "/dl-test.osm" );
  bool res = download.start();
  QVERIFY( res );

  // wait for finished() signal
  int timeout = 15000; // in miliseconds - max waiting time
  int waitTime = 500; // in miliseconds - unit waiting time
  QSignalSpy spy( &download, SIGNAL( finished() ) );
  while ( timeout > 0 && spy.count() == 0 )
  {
    QTest::qWait( waitTime );
    timeout -= waitTime;
  }

  QVERIFY( spy.count() != 0 );

  if ( download.hasError() )
    qDebug( "ERROR: %s", download.errorString().toAscii().data() );
}


void TestOpenStreetMap::importAndQueries()
{
  QString dbFilename =  QDir::tempPath() + "/testdata.db";
  QString xmlFilename = TEST_DATA_DIR "/openstreetmap/testdata.xml";

  QgsOSMXmlImport import( xmlFilename, dbFilename );
  bool res = import.import();
  if ( import.hasError() )
    qDebug( "XML ERR: %s", import.errorString().toAscii().data() );
  QCOMPARE( res, true );
  QCOMPARE( import.hasError(), false );

  qDebug( "import finished" );

  QgsOSMDatabase db( dbFilename );
  bool dbopenRes = db.open();
  if ( !db.errorString().isEmpty() )
    qDebug( "DB ERR: %s", db.errorString().toAscii().data() );
  QCOMPARE( dbopenRes, true );

  // query node

  QgsOSMNode n = db.node( 11111 );
  QCOMPARE( n.isValid(), true );
  QCOMPARE( n.point().x(), 14.4277148 );
  QCOMPARE( n.point().y(), 50.0651387 );

  QgsOSMNode nNot = db.node( 22222 );
  QCOMPARE( nNot.isValid(), false );

  // query node tags

  QgsOSMTags tags = db.tags( false, 11111 );
  QCOMPARE( tags.count(), 7 );
  QCOMPARE( tags.value( "addr:postcode" ), QString( "12800" ) );

  QgsOSMTags tags2 = db.tags( false, 360769661 );
  QCOMPARE( tags2.count(), 0 );
  QCOMPARE( tags2.value( "addr:postcode" ), QString() );

  QgsOSMTags tagsNot = db.tags( false, 22222 );
  QCOMPARE( tagsNot.count(), 0 );

  // list nodes

  QgsOSMNodeIterator nodes = db.listNodes();
  QCOMPARE( nodes.next().id(), ( qint64 )11111 );
  QCOMPARE( nodes.next().id(), ( qint64 )360769661 );
  nodes.close();

  // query way

  QgsOSMWay w = db.way( 32137532 );
  QCOMPARE( w.isValid(), true );
  QCOMPARE( w.nodes().count(), 5 );
  QCOMPARE( w.nodes().at( 0 ), ( qint64 )360769661 );
  QCOMPARE( w.nodes().at( 1 ), ( qint64 )360769664 );

  QgsOSMWay wNot = db.way( 1234567 );
  QCOMPARE( wNot.isValid(), false );

  // query way tags

  QgsOSMTags tagsW = db.tags( true, 32137532 );
  QCOMPARE( tagsW.count(), 3 );
  QCOMPARE( tagsW.value( "building" ), QString( "yes" ) );

  QgsOSMTags tagsWNot = db.tags( true, 1234567 );
  QCOMPARE( tagsWNot.count(), 0 );

  // list ways

  QgsOSMWayIterator ways = db.listWays();
  QCOMPARE( ways.next().id(), ( qint64 )32137532 );
  QCOMPARE( ways.next().isValid(), false );
  ways.close();

  bool exportRes1 = db.exportSpatiaLite( QgsOSMDatabase::Point, "sl_points", QStringList( "addr:postcode" ) );
  //bool exportRes = db.exportSpatiaLite( QStringList("amenity") << "name" << "highway" );
  if ( !db.errorString().isEmpty() )
    qDebug( "EXPORT-1 ERR: %s", db.errorString().toAscii().data() );
  QCOMPARE( exportRes1, true );


  bool exportRes2 = db.exportSpatiaLite( QgsOSMDatabase::Polyline, "sl_lines", QStringList( "building" ) );
  //bool exportRes2 = db.exportSpatiaLite( QStringList("amenity") << "name" << "highway" );
  if ( !db.errorString().isEmpty() )
    qDebug( "EXPORT-2 ERR: %s", db.errorString().toAscii().data() );
  QCOMPARE( exportRes2, true );


  // TODO: test exported data
}


QTEST_MAIN( TestOpenStreetMap )

#include "testopenstreetmap.moc"
