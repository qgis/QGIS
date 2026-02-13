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
    void testFromPolyBezierControlPoints();
    void testFromPolyBezierControlPointsWithZM();
    void testFromPolyBezierControlPointsQgsPointXY();
    void testFromPolyBezierControlPointsInvalid();
    void testCalculateSymmetricHandles();
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

  QgsPointSequence points = data.interpolateLine();
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

  QgsBezierData data2;
  data2.addAnchor( QgsPoint( 0, 0 ) );
  data2.addAnchor( QgsPoint( 10, 10 ) );
  data2.moveHandle( 1, QgsPoint( 5, 5 ) ); // right handle of anchor 0
  data2.moveHandle( 2, QgsPoint( 5, 5 ) ); // left handle of anchor 1

  // Test degree 2 (quadratic)
  std::unique_ptr<QgsNurbsCurve> nurbs2( data2.asNurbsCurve( 2 ) );
  QVERIFY( nurbs2 != nullptr );
  QCOMPARE( nurbs2->degree(), 2 );
  QCOMPARE( nurbs2->controlPoints().size(), 3 );
  QCOMPARE( nurbs2->controlPoints()[1], QgsPoint( 5, 5 ) );

  // Test degree 1 (linear)
  std::unique_ptr<QgsNurbsCurve> nurbs1( data2.asNurbsCurve( 1 ) );
  QVERIFY( nurbs1 != nullptr );
  QCOMPARE( nurbs1->degree(), 1 );
  QCOMPARE( nurbs1->controlPoints().size(), 2 );
}

void TestQgsBezierData::testFromPolyBezierControlPoints()
{
  QVector<QgsPoint> controlPoints;
  controlPoints << QgsPoint( 0, 0 )
                << QgsPoint( 3, 3 )
                << QgsPoint( 7, 7 )
                << QgsPoint( 10, 10 );

  QgsBezierData data = QgsBezierData::fromPolyBezierControlPoints( controlPoints );

  QCOMPARE( data.anchorCount(), 2 );
  QCOMPARE( data.handleCount(), 4 );

  QCOMPARE( data.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 10, 10 ) );

  QCOMPARE( data.handle( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.handle( 1 ), QgsPoint( 3, 3 ) );
  QCOMPARE( data.handle( 2 ), QgsPoint( 7, 7 ) );
  QCOMPARE( data.handle( 3 ), QgsPoint( 10, 10 ) );

  // Test degree 2 (quadratic): anchor, control, anchor
  QVector<QgsPoint> controlPoints2;
  controlPoints2 << QgsPoint( 0, 0 ) << QgsPoint( 5, 5 ) << QgsPoint( 10, 0 );
  QgsBezierData data2 = QgsBezierData::fromPolyBezierControlPoints( controlPoints2, 2 );
  QCOMPARE( data2.anchorCount(), 2 );
  QCOMPARE( data2.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data2.anchor( 1 ), QgsPoint( 10, 0 ) );
  QCOMPARE( data2.handle( 1 ), QgsPoint( 5, 5 ) ); // right handle of anchor 0
  QCOMPARE( data2.handle( 2 ), QgsPoint( 5, 5 ) ); // left handle of anchor 1

  // Test degree 1 (linear): anchor, anchor
  QVector<QgsPoint> controlPoints1;
  controlPoints1 << QgsPoint( 0, 0 ) << QgsPoint( 10, 10 );
  QgsBezierData data1 = QgsBezierData::fromPolyBezierControlPoints( controlPoints1, 1 );
  QCOMPARE( data1.anchorCount(), 2 );
  QCOMPARE( data1.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data1.anchor( 1 ), QgsPoint( 10, 10 ) );
}

void TestQgsBezierData::testFromPolyBezierControlPointsWithZM()
{
  QVector<QgsPoint> controlPoints;
  controlPoints << QgsPoint( 0, 0, 1.0, 10.0 )
                << QgsPoint( 3, 3, 2.0, 20.0 )
                << QgsPoint( 7, 7, 3.0, 30.0 )
                << QgsPoint( 10, 10, 4.0, 40.0 )
                << QgsPoint( 13, 7, 5.0, 50.0 )
                << QgsPoint( 17, 3, 6.0, 60.0 )
                << QgsPoint( 20, 0, 7.0, 70.0 );

  QgsBezierData data = QgsBezierData::fromPolyBezierControlPoints( controlPoints );

  QCOMPARE( data.anchorCount(), 3 );

  QgsPoint anchor0 = data.anchor( 0 );
  QCOMPARE( anchor0.x(), 0.0 );
  QCOMPARE( anchor0.y(), 0.0 );
  QCOMPARE( anchor0.z(), 1.0 );
  QCOMPARE( anchor0.m(), 10.0 );

  QgsPoint anchor1 = data.anchor( 1 );
  QCOMPARE( anchor1.x(), 10.0 );
  QCOMPARE( anchor1.y(), 10.0 );
  QCOMPARE( anchor1.z(), 4.0 );
  QCOMPARE( anchor1.m(), 40.0 );

  QgsPoint handle1 = data.handle( 1 );
  QCOMPARE( handle1.z(), 2.0 );
  QCOMPARE( handle1.m(), 20.0 );
}

void TestQgsBezierData::testFromPolyBezierControlPointsQgsPointXY()
{
  QVector<QgsPointXY> controlPoints;
  controlPoints << QgsPointXY( 0, 0 )
                << QgsPointXY( 3, 3 )
                << QgsPointXY( 7, 7 )
                << QgsPointXY( 10, 10 );

  QgsBezierData data = QgsBezierData::fromPolyBezierControlPoints( controlPoints );

  QCOMPARE( data.anchorCount(), 2 );
  QCOMPARE( data.anchor( 0 ), QgsPoint( 0, 0 ) );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 10, 10 ) );
}

void TestQgsBezierData::testFromPolyBezierControlPointsInvalid()
{
  QVector<QgsPoint> empty;
  QgsBezierData data1 = QgsBezierData::fromPolyBezierControlPoints( empty );
  QVERIFY( data1.isEmpty() );

  QVector<QgsPoint> tooFew;
  tooFew << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 2, 2 );
  QgsBezierData data2 = QgsBezierData::fromPolyBezierControlPoints( tooFew );
  QVERIFY( data2.isEmpty() );

  QVector<QgsPoint> wrongCount;
  wrongCount << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 ) << QgsPoint( 2, 2 ) << QgsPoint( 3, 3 ) << QgsPoint( 4, 4 );
  QgsBezierData data3 = QgsBezierData::fromPolyBezierControlPoints( wrongCount );
  QVERIFY( data3.isEmpty() );
}

void TestQgsBezierData::testCalculateSymmetricHandles()
{
  // 1. Test static method with individual points
  QgsPoint anchor( 5, 5 );
  QgsPoint mouse( 7, 7 );
  QgsPoint handleFollow, handleOpposite;
  QgsBezierData::calculateSymmetricHandles( anchor, mouse, &handleFollow, &handleOpposite );
  QCOMPARE( handleFollow, QgsPoint( 7, 7 ) );
  QCOMPARE( handleOpposite, QgsPoint( 3, 3 ) );

  // 2. Test static method with QVector
  QVector<QgsPoint> controlPoints;
  controlPoints << QgsPoint( 0, 0 ) << QgsPoint( 1, 1 )                      // Anchor 0, handle right
                << QgsPoint( 4, 4 ) << QgsPoint( 5, 5 ) << QgsPoint( 6, 6 ); // Handle Left, Anchor 1, Handle Right
  // Index 3 is anchor. Index 4 should follow, Index 2 should be opposite.
  QgsBezierData::calculateSymmetricHandles( controlPoints, 3, QgsPoint( 7, 7 ) );
  QCOMPARE( controlPoints[3], QgsPoint( 5, 5 ) );
  QCOMPARE( controlPoints[4], QgsPoint( 7, 7 ) );
  QCOMPARE( controlPoints[2], QgsPoint( 3, 3 ) );

  // 3. Test method
  QgsBezierData data;
  data.addAnchor( QgsPoint( 0, 0 ) );
  data.addAnchor( QgsPoint( 5, 5 ) );
  data.addAnchor( QgsPoint( 10, 10 ) );

  // Anchor 1 is (5, 5).
  data.calculateSymmetricHandles( 1, QgsPoint( 7, 7 ) );
  QCOMPARE( data.anchor( 1 ), QgsPoint( 5, 5 ) );
  QCOMPARE( data.handle( 3 ), QgsPoint( 7, 7 ) ); // right handle follows
  QCOMPARE( data.handle( 2 ), QgsPoint( 3, 3 ) ); // left handle opposite
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
