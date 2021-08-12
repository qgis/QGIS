/***************************************************************************
                         testqgspagesizeregistry.cpp
                         ----------------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgstest.h"
#include "qgspagesizeregistry.h"
#include "qgis.h"
#include "qgsapplication.h"
#include <QObject>

class TestQgsPageSizeRegistry : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void pageSizeEquality(); //test equality of QgsPageSize
    void pageCopyConstructor(); //test copy constructor of QgsPageSize
    void createInstance(); // create global instance of QgsPageSizeRegistry
    void instanceHasDefaultSizes(); // check that global instance is populated with default page sizes
    void addSize(); // check adding a size to the registry
    void findSize(); //find a size in the registry
    void findBySize(); //find a matching size in the registry
    void decodePageSize(); //test decoding a page size string

  private:

};

void TestQgsPageSizeRegistry::initTestCase()
{

}

void TestQgsPageSizeRegistry::cleanupTestCase()
{

}

void TestQgsPageSizeRegistry::init()
{

}

void TestQgsPageSizeRegistry::cleanup()
{

}

void TestQgsPageSizeRegistry::pageSizeEquality()
{
  const QgsPageSize size1( QStringLiteral( "test" ), QgsLayoutSize( 1, 2 ) );
  const QgsPageSize size2( QStringLiteral( "test" ), QgsLayoutSize( 1, 2 ) );
  QCOMPARE( size1, size2 );
  QVERIFY( !( size1 != size2 ) );
  const QgsPageSize size3( QStringLiteral( "no match" ), QgsLayoutSize( 1, 2 ) );
  const QgsPageSize size4( QStringLiteral( "test" ), QgsLayoutSize( 3, 4 ) );
  QVERIFY( !( size1 == size3 ) );
  QVERIFY( size1 != size3 );
  QVERIFY( !( size1 == size4 ) );
  QVERIFY( size1 != size4 );
  QVERIFY( !( size3 == size4 ) );
  QVERIFY( size3 != size4 );
}

void TestQgsPageSizeRegistry::pageCopyConstructor()
{
  const QgsPageSize size1( QStringLiteral( "test" ), QgsLayoutSize( 1, 2 ) );
  const QgsPageSize size2( size1 );
  QCOMPARE( size1.name, size2.name );
  QCOMPARE( size1.size, size2.size );
}

void TestQgsPageSizeRegistry::createInstance()
{
  QVERIFY( QgsApplication::pageSizeRegistry() );
}

void TestQgsPageSizeRegistry::instanceHasDefaultSizes()
{
  //check that registry instance is initially populated with some known page sizes
  QgsPageSizeRegistry *registry = QgsApplication::pageSizeRegistry();
  QVERIFY( registry->entries().length() > 0 );
}

void TestQgsPageSizeRegistry::addSize()
{
  QgsPageSizeRegistry *registry = QgsApplication::pageSizeRegistry();
  const int oldSize = registry->entries().length();
  const QgsPageSize newSize( QStringLiteral( "new" ), QgsLayoutSize( 1, 2 ) );
  registry->add( newSize );
  QCOMPARE( registry->entries().length(), oldSize + 1 );
  QVERIFY( registry->entries().indexOf( newSize ) >= 0 );
}

void TestQgsPageSizeRegistry::findSize()
{
  QgsPageSizeRegistry *registry = QgsApplication::pageSizeRegistry();
  const QgsPageSize newSize( QStringLiteral( "test size" ), QgsLayoutSize( 1, 2 ) );
  registry->add( newSize );
  const QList< QgsPageSize > results = registry->find( QStringLiteral( "test size" ) );
  QVERIFY( results.length() > 0 );
  QCOMPARE( results.at( 0 ), newSize );
  //check that match is case insensitive
  const QList< QgsPageSize > results2 = registry->find( QStringLiteral( "tEsT Size" ) );
  QVERIFY( results2.length() > 0 );
  QCOMPARE( results2.at( 0 ), newSize );
}

void TestQgsPageSizeRegistry::findBySize()
{
  QgsPageSizeRegistry *registry = QgsApplication::pageSizeRegistry();
  QVERIFY( registry->find( QgsLayoutSize( 1, 1 ) ).isEmpty() );
  QCOMPARE( registry->find( QgsLayoutSize( 210, 297 ) ), QStringLiteral( "A4" ) );
  QCOMPARE( registry->find( QgsLayoutSize( 297, 210 ) ), QStringLiteral( "A4" ) );
  QCOMPARE( registry->find( QgsLayoutSize( 125, 176 ) ), QStringLiteral( "B6" ) );
  QCOMPARE( registry->find( QgsLayoutSize( 21, 29.7, QgsUnitTypes::LayoutCentimeters ) ), QStringLiteral( "A4" ) );
  // must have allowance of 0.01 units - because we round to this precision in all page size widgets
  QCOMPARE( registry->find( QgsLayoutSize( 125.009, 175.991 ) ), QStringLiteral( "B6" ) );
}

void TestQgsPageSizeRegistry::decodePageSize()
{
  QgsPageSizeRegistry *registry = QgsApplication::pageSizeRegistry();

  //test with good string
  QgsPageSize result;
  QVERIFY( registry->decodePageSize( QString( " a3 " ), result ) );
  QCOMPARE( result.name, QString( "A3" ) );
  QCOMPARE( result.size, QgsLayoutSize( 297.0, 420.0 ) );

  //good strings
  QVERIFY( registry->decodePageSize( QStringLiteral( "a4" ), result ) );
  QCOMPARE( result.size.width(), 210.0 );
  QCOMPARE( result.size.height(), 297.0 );
  QVERIFY( registry->decodePageSize( QStringLiteral( "B0" ),  result ) );
  QCOMPARE( result.size.width(), 1000.0 );
  QCOMPARE( result.size.height(), 1414.0 );
  QVERIFY( registry->decodePageSize( QStringLiteral( "letter" ),  result ) );
  QCOMPARE( result.size.width(), 215.9 );
  QCOMPARE( result.size.height(), 279.4 );
  QVERIFY( registry->decodePageSize( QStringLiteral( "LEGAL" ),  result ) );
  QCOMPARE( result.size.width(), 215.9 );
  QCOMPARE( result.size.height(), 355.6 );

  //test with bad string
  QgsPageSize result2( QStringLiteral( "nomatch" ), QgsLayoutSize( 10.0, 20.0 ) );
  const QgsPageSize expected( result2 ); //for a bad match, expect page size to be unchanged
  QVERIFY( !registry->decodePageSize( QString( "bad" ), result2 ) );
  QCOMPARE( result2, expected );
}

QTEST_MAIN( TestQgsPageSizeRegistry )
#include "testqgspagesizeregistry.moc"
