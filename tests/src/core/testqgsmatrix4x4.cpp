/***************************************************************************
  qgsmatrix4x4.cpp
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include "qgsmatrix4x4.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsMatrix4x4 class
 */
class TestQgsMatrix4x4 : public QObject
{
    Q_OBJECT
  public:
    TestQgsMatrix4x4() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testData();
    void testIdentity();
    void testVectorMultiply();
    void testMatrixMultiply();

  private:
};

//runs before all tests
void TestQgsMatrix4x4::initTestCase()
{
}

//runs after all tests
void TestQgsMatrix4x4::cleanupTestCase()
{
}

void TestQgsMatrix4x4::testData()
{
  // Initialization - row-major order
  QgsMatrix4x4 m1( 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 110., 120., 130., 140., 150., 160. );

  // Access through data() / constData() - column-major order
  const double *m1data = m1.constData();
  QCOMPARE( m1data[0], 10. );
  QCOMPARE( m1data[1], 50. );
  QCOMPARE( m1data[2], 90. );
  QCOMPARE( m1data[3], 130. );
  QCOMPARE( m1data[4], 20. );
  QCOMPARE( m1data[5], 60. );
  QCOMPARE( m1data[6], 100. );
  QCOMPARE( m1data[7], 140. );
  QCOMPARE( m1data[8], 30. );
  QCOMPARE( m1data[9], 70. );
  QCOMPARE( m1data[10], 110. );
  QCOMPARE( m1data[11], 150. );
  QCOMPARE( m1data[12], 40. );
  QCOMPARE( m1data[13], 80. );
  QCOMPARE( m1data[14], 120. );
  QCOMPARE( m1data[15], 160. );

  QVERIFY( !m1.isIdentity() );

  // verify we are using double precision:
  // 123456789 can't be represented as a float (the closest float is 123456792)
  m1.data()[0] = 123456789.;
  QCOMPARE( m1.data()[0], 123456789. );
}

void TestQgsMatrix4x4::testIdentity()
{
  QgsMatrix4x4 m;
  const double *mdata = m.constData();
  QCOMPARE( mdata[0], 1. );
  QCOMPARE( mdata[1], 0. );
  QCOMPARE( mdata[2], 0. );
  QCOMPARE( mdata[3], 0. );
  QCOMPARE( mdata[4], 0. );
  QCOMPARE( mdata[5], 1. );
  QCOMPARE( mdata[6], 0. );
  QCOMPARE( mdata[7], 0. );
  QCOMPARE( mdata[8], 0. );
  QCOMPARE( mdata[9], 0. );
  QCOMPARE( mdata[10], 1. );
  QCOMPARE( mdata[11], 0. );
  QCOMPARE( mdata[12], 0. );
  QCOMPARE( mdata[13], 0. );
  QCOMPARE( mdata[14], 0. );
  QCOMPARE( mdata[15], 1. );

  QVERIFY( m.isIdentity() );
}

void TestQgsMatrix4x4::testVectorMultiply()
{
  double sa = 0.5, ca = sqrt( 3. ) / 2.;                                             // sin(30 deg) and cos(30 deg)
  QgsMatrix4x4 m( ca, -sa, 0., 0., sa, ca, 0., 0., 0., 0., 1., 0., 0., 0., 0., 1. ); // CCW rotation around Z axis

  QgsVector3D v1( 5., 0., 1. );
  QgsVector3D v1rot = m.map( v1 );
  QCOMPARE( v1rot.x(), 5. * ca );
  QCOMPARE( v1rot.y(), 5. * sa );
  QCOMPARE( v1rot.z(), 1. );

  QgsVector3D v2( 0., 2., 1. );
  QgsVector3D v2rot = m.map( v2 );
  QCOMPARE( v2rot.x(), -2. * sa );
  QCOMPARE( v2rot.y(), 2. * ca );
  QCOMPARE( v2rot.z(), 1. );

  // translation by a large vector
  QgsMatrix4x4 mTr( 1., 0., 0., 123456789., 0., 1., 0., 234567890., 0., 0., 1., 345678901., 0., 0., 0., 1. );

  QgsVector3D v1tr = mTr.map( v1 );
  QCOMPARE( v1tr.x(), 123456794. );
  QCOMPARE( v1tr.y(), 234567890. );
  QCOMPARE( v1tr.z(), 345678902. );
}

void TestQgsMatrix4x4::testMatrixMultiply()
{
  QgsMatrix4x4 mTr( 1., 0., 0., 123456789., 0., 1., 0., 234567890., 0., 0., 1., 345678901., 0., 0., 0., 1. );

  QgsMatrix4x4 mTr2( 1., 0., 0., -123456790., 0., 1., 0., -234567892., 0., 0., 1., -345678904., 0., 0., 0., 1. );

  QgsMatrix4x4 m = mTr * mTr2;
  const double *mdata = m.constData();
  QCOMPARE( mdata[0], 1. );
  QCOMPARE( mdata[1], 0. );
  QCOMPARE( mdata[2], 0. );
  QCOMPARE( mdata[3], 0. );
  QCOMPARE( mdata[4], 0. );
  QCOMPARE( mdata[5], 1. );
  QCOMPARE( mdata[6], 0. );
  QCOMPARE( mdata[7], 0. );
  QCOMPARE( mdata[8], 0. );
  QCOMPARE( mdata[9], 0. );
  QCOMPARE( mdata[10], 1. );
  QCOMPARE( mdata[11], 0. );
  QCOMPARE( mdata[12], -1. );
  QCOMPARE( mdata[13], -2. );
  QCOMPARE( mdata[14], -3. );
  QCOMPARE( mdata[15], 1. );
}


QGSTEST_MAIN( TestQgsMatrix4x4 )
#include "testqgsmatrix4x4.moc"
