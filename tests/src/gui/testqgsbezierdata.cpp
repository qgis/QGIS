/***************************************************************************
    testqgsbezierdata.cpp
     --------------------------------------
    Date                 : December 2025
    Copyright            : (C) 2025 Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgsapplication.h"
#include "qgsbezierdata.h"
#include "qgsnurbscurve.h"
#include "qgstest.h"

#include <QTest>

class TestQgsBezierData : public QObject
{
    Q_OBJECT

  public:
    TestQgsBezierData() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testConstructor();
    void testAddAnchor();
    void testMoveAnchor();
    void testMoveHandle();
    void testInsertAnchor();
    void testDeleteAnchor();
    void testRetractHandle();
    void testExtendHandle();
    void testFindClosestAnchor();
    void testFindClosestHandle();
    void testInterpolate();
    void testAsNurbsCurve();
    void testClear();
};

void TestQgsBezierData::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsBezierData::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsBezierData::init()
{
}

void TestQgsBezierData::cleanup()
{
}

void TestQgsBezierData::testConstructor()
{
  QgsBezierData data;
  QVERIFY( data.isEmpty() );
  QCOMPARE( data.anchorCount(), 0 );
  QCOMPARE( data.handleCount(), 0 );
}

void TestQgsBezierData::testAddAnchor()
{
  QgsBezierData data;

  data.addAnchor( QgsPoint( 0, 0 ) );
  QCOMPARE( data.anchorCount(), 1 );
  QCOMPARE( data.handleCount(), 2 );
  QCOMPARE( data.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.handle( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 0, 0 ) );

  data.addAnchor( QgsPoint( 10, 0 ) );
  QCOMPARE( data.anchorCount(), 2 );
  QCOMPARE( data.handleCount(), 4 );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 10, 0 ) );
}

void TestQgsBezierData::testMoveAnchor()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.moveHandle( 1, QgsPoint( 3, 3 ) );

  data.moveAnchor( 0, QgsPoint( 5, 5 ) );
  QCOMPARE( data.anchor( 0 ), QgsPoint( 5, 5 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 8, 8 ) );
}

void TestQgsBezierData::testMoveHandle()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 10, 0 ) );

  data.moveHandle( 1, QgsPoint( 3, 3 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 3, 3 ) );
  QCOMPARE( data.handle( 0 ), QgsPoint( 0, 0 ) );
}

void TestQgsBezierData::testInsertAnchor()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 20, 0 ) );

  data.insertAnchor( 1, QgsPoint( 10, 5 ) );
  QCOMPARE( data.anchorCount(), 3 );
  QCOMPARE( data.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 10, 5 ) );
  QCOMPARE( data.anchor( 2 ), QgsPoint( 20, 0 ) );
}

void TestQgsBezierData::testDeleteAnchor()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 10, 0 ) );
  data.addAnchor( QgsPoint( 20, 0 ) );

  data.deleteAnchor( 1 );
  QCOMPARE( data.anchorCount(), 2 );
  QCOMPARE( data.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 20, 0 ) );
}

void TestQgsBezierData::testRetractHandle()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.moveHandle( 1, QgsPoint( 5, 5 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 5, 5 ) );

  data.retractHandle( 1 );
  QCOMPARE( data.handle( 1 ), QgsPoint( 0, 0 ) );
}

void TestQgsBezierData::testExtendHandle()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 0, 0 ) );

  data.extendHandle( 1, QgsPoint( 5, 5 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 5, 5 ) );
}

void TestQgsBezierData::testFindClosestAnchor()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 10, 0 ) );
  data.addAnchor( QgsPoint( 20, 0 ) );

  QCOMPARE( data.findClosestAnchor( QgsPoint( 0.5, 0 ), 1.0 ), 0 );
  QCOMPARE( data.findClosestAnchor( QgsPoint( 9.5, 0 ), 1.0 ), 1 );
  QCOMPARE( data.findClosestAnchor( QgsPoint( 100, 100 ), 1.0 ), -1 );
}

void TestQgsBezierData::testFindClosestHandle()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.moveHandle( 1, QgsPoint( 5, 5 ) );

  QCOMPARE( data.findClosestHandle( QgsPoint( 4.5, 4.5 ), 1.0 ), 1 );
  QCOMPARE( data.findClosestHandle( QgsPoint( 100, 100 ), 1.0 ), -1 );
}

void TestQgsBezierData::testInterpolate()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 10, 10 ) );

  QgsPointSequence points = data.interpolate();
  QVERIFY( !points.isEmpty() );
  QCOMPARE( points.first(), QgsPoint( 0, 0 ) );
  QCOMPARE( points.last(), QgsPoint( 10, 10 ) );
}

void TestQgsBezierData::testAsNurbsCurve()
{
  QgsBezierData data;
  QVERIFY( data.asNurbsCurve() == nullptr );

  data.addAnchor( QgsPoint( 0, 0 ) );
  QVERIFY( data.asNurbsCurve() == nullptr );

  data.addAnchor( QgsPoint( 10, 10 ) );
  data.moveHandle( 1, QgsPoint( 3, 3 ) );
  data.moveHandle( 2, QgsPoint( 7, 7 ) );

  std::unique_ptr<QgsNurbsCurve> nurbs( data.asNurbsCurve() );
  QVERIFY( nurbs != nullptr );
  QCOMPARE( nurbs->degree(), 3 );
  QCOMPARE( nurbs->controlPoints().size(), 4 );
  QCOMPARE( nurbs->startPoint(), QgsPoint( 0, 0 ) );
  QCOMPARE( nurbs->endPoint(), QgsPoint( 10, 10 ) );
}

void TestQgsBezierData::testClear()
{
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 10, 0 ) );
  QVERIFY( !data.isEmpty() );

  data.clear();
  QVERIFY( data.isEmpty() );
  QCOMPARE( data.anchorCount(), 0 );
  QCOMPARE( data.handleCount(), 0 );
}

QGSTEST_MAIN( TestQgsBezierData )
#include "testqgsbezierdata.moc"
