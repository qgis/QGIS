/***************************************************************************
     testqgsvector.cpp
     --------------------------------------
    Date                 : 18 Apr 2018
    Copyright            : (C) 2018 by Loïc Bartoletti
    Email                : loic dot bartoletti @ oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsvector3d.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the different geometry operations on vector features.
 */
class TestQgsVector : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    // vector3d
    void vector3d();
};

void TestQgsVector::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsVector::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVector::vector3d()
{
  //string
  QCOMPARE( QgsVector3D().toString(), QString( "Vector3D (0, 0, 0)" ) );
  QCOMPARE( QgsVector3D( 0, 1, 2 ).toString(), QString( "Vector3D (0, 1, 2)" ) );
  QCOMPARE( QgsVector3D( 0.12, 1.234, 2.3456789 ).toString( 1 ), QString( "Vector3D (0.1, 1.2, 2.3)" ) );

  QgsVector3D p0( 0.0, 0.0, 0.0 );
  const QgsVector3D p1( 1.0, 2.0, 3.0 );
  const QgsVector3D p2( 4.0, 5.0, 6.0 );
  // cross product
  QCOMPARE( QgsVector3D::crossProduct( p1, p2 ), QgsVector3D( -3.0, 6.0, -3.0 ) );

  // dot product
  QCOMPARE( QgsVector3D::dotProduct( p1, p2 ), 32.0 );

  // normalize
  QgsVector3D p3( 0.0, -6.0, 0.0 );
  QgsVector3D p4( 1.0, 2.0, -2.0 );
  p0.normalize();
  p3.normalize();
  p4.normalize();
  QCOMPARE( p0, QgsVector3D() );
  QCOMPARE( p3, QgsVector3D( 0.0, -1.0, 0.0 ) );
  QCOMPARE( p4, QgsVector3D( 1.0 / 3.0, 2.0 / 3.0, -2.0 / 3.0 ) );

  // distance
  QCOMPARE( p0.distance( p0 ), 0.0 );
  QCOMPARE( p0.distance( QgsVector3D( 5.0, 0.0, 0.0 ) ), 5.0 );
  QCOMPARE( p0.distance( QgsVector3D( 0.0, 5.0, 0.0 ) ), 5.0 );
  QCOMPARE( p0.distance( QgsVector3D( 0.0, 0.0, 5.0 ) ), 5.0 );
  QCOMPARE( p0.distance( QgsVector3D( 1.0, 1.0, 0.0 ) ), sqrt( 2 ) );
  QCOMPARE( p0.distance( QgsVector3D( 1.0, 1.0, 1.0 ) ), sqrt( 3 ) );

  // perpendicular point
  QCOMPARE( QgsVector3D::perpendicularPoint( QgsVector3D( 0.0, 0.0, 0.0 ), QgsVector3D( 0.0, 5.0, 0.0 ), QgsVector3D( 1.0, 4.0, 0.0 ) ), QgsVector3D( 0.0, 4.0, 0.0 ) );
  QCOMPARE( QgsVector3D::perpendicularPoint( QgsVector3D( 0.0, 0.0, 5.0 ), QgsVector3D( 0.0, 0.0, 10.0 ), QgsVector3D( 2.0, 4.0, 7 ) ), QgsVector3D( 0.0, 0.0, 7.0 ) );
  QCOMPARE( QgsVector3D::perpendicularPoint( QgsVector3D( 0.0, 0.0, 5.0 ), QgsVector3D( 0.0, 5.0, 10.0 ), QgsVector3D( 1.0, 4.0, 5.0 ) ).toString( 2 ), QgsVector3D( 0.0, 2.0, 7.0 ).toString( 2 ) );
}

QGSTEST_MAIN( TestQgsVector )
#include "testqgsvector.moc"
