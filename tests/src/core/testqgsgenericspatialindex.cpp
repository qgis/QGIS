/***************************************************************************
     testqgsgenericspatialindex.cpp
     ------------------------------
    Date                 : December 2019
    Copyright            : (C) 2019 Nyall Dawson
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
#include <QString>
#include <QStringList>
#include <QLocale>

#include <memory>

#include "qgsgenericspatialindex.h"
#include "qgstest.h"
#include "qgslabelposition.h"

class TestQgsGenericSpatialIndex: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testIndex();

  private:
};

void TestQgsGenericSpatialIndex::initTestCase()
{
}

void TestQgsGenericSpatialIndex::cleanupTestCase()
{

}

void TestQgsGenericSpatialIndex::init()
{
}

void TestQgsGenericSpatialIndex::cleanup()
{
}

void TestQgsGenericSpatialIndex::testIndex()
{
  QgsGenericSpatialIndex< QgsLabelPosition > index;

  QList< QgsLabelPosition * > found;
  index.intersects( QgsRectangle( std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::max(),
                                  std::numeric_limits< double >::max() ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );

  QVERIFY( found.isEmpty() );

  // remove feature not in index
  QgsLabelPosition p1;
  QVERIFY( ! index.remove( &p1, QgsRectangle() ) );

  // add data
  QVERIFY( index.insert( &p1, QgsRectangle( 1, 1, 5, 5 ) ) );
  QgsLabelPosition p2;
  QVERIFY( index.insert( &p2, QgsRectangle( 1, 1, 4, 4 ) ) );
  QgsLabelPosition p3;
  QVERIFY( index.insert( &p3, QgsRectangle( 11, 11, 14, 14 ) ) );

  index.intersects( QgsRectangle( std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::max(),
                                  std::numeric_limits< double >::max() ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 3 );
  QVERIFY( found.contains( &p1 ) );
  QVERIFY( found.contains( &p2 ) );
  QVERIFY( found.contains( &p3 ) );
  found.clear();

  index.intersects( QgsRectangle( 0, 0, 3, 3 ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 2 );
  QVERIFY( found.contains( &p1 ) );
  QVERIFY( found.contains( &p2 ) );
  found.clear();

  index.intersects( QgsRectangle( 4.5, 4.5, 5, 5 ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 1 );
  QVERIFY( found.contains( &p1 ) );
  found.clear();

  index.intersects( QgsRectangle( 10, 10, 13, 13 ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 1 );
  QVERIFY( found.contains( &p3 ) );
  found.clear();

  QVERIFY( index.remove( &p1, QgsRectangle( 1, 1, 5, 5 ) ) );

  index.intersects( QgsRectangle( 0, 0, 3, 3 ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 1 );
  QVERIFY( found.contains( &p2 ) );
  found.clear();

  QVERIFY( !index.remove( &p1, QgsRectangle( 1, 1, 5, 5 ) ) );

  index.intersects( QgsRectangle( std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::max(),
                                  std::numeric_limits< double >::max() ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 2 );
  QVERIFY( found.contains( &p2 ) );
  QVERIFY( found.contains( &p3 ) );
  found.clear();

  QVERIFY( index.remove( &p2, QgsRectangle( 1, 1, 4, 4 ) ) );

  index.intersects( QgsRectangle( std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::max(),
                                  std::numeric_limits< double >::max() ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QCOMPARE( found.count(), 1 );
  QVERIFY( found.contains( &p3 ) );
  found.clear();

  QVERIFY( index.remove( &p3, QgsRectangle( 11, 11, 14, 14 ) ) );

  index.intersects( QgsRectangle( std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::lowest(),
                                  std::numeric_limits< double >::max(),
                                  std::numeric_limits< double >::max() ),
                    [&found]( QgsLabelPosition * p )-> bool
  {
    found.append( p );
    return true;
  } );
  QVERIFY( found.empty() );
}


QGSTEST_MAIN( TestQgsGenericSpatialIndex )
#include "testqgsgenericspatialindex.moc"
