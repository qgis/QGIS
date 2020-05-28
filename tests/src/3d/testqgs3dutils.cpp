/***************************************************************************
     testqgs3dutils.cpp
     ----------------------
    Date                 : November 2017
    Copyright            : (C) 2017 by Martin Dobias
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

#include "qgs3dutils.h"

#include <QSize>

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgs3DUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgs3DUtils() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testTransforms();
  private:
};

//runs before all tests
void TestQgs3DUtils::initTestCase()
{
}

//runs after all tests
void TestQgs3DUtils::cleanupTestCase()
{
}

void TestQgs3DUtils::testTransforms()
{
  QgsVector3D map123( 1, 2, 3 );

  QgsVector3D world123 = Qgs3DUtils::mapToWorldCoordinates( map123, QgsVector3D() );
  QCOMPARE( world123, QgsVector3D( 1, 3, -2 ) );

  QgsVector3D world123map = Qgs3DUtils::worldToMapCoordinates( world123, QgsVector3D() );
  QCOMPARE( world123map, map123 );

  // now with non-zero origin

  QgsVector3D origin( -10, -20, -30 );

  QgsVector3D world123x = Qgs3DUtils::mapToWorldCoordinates( map123, origin );
  QCOMPARE( world123x, QgsVector3D( 11, 33, -22 ) );

  QgsVector3D world123xmap = Qgs3DUtils::worldToMapCoordinates( world123x, origin );
  QCOMPARE( world123xmap, map123 );

  //
  // transform world point from one system to another
  //

  QgsVector3D worldPoint1( 5, 7, -6 );
  QgsVector3D origin1( 10, 20, 30 );
  QgsVector3D origin2( 1, 2, 3 );
  QgsVector3D worldPoint2 = Qgs3DUtils::transformWorldCoordinates( worldPoint1, origin1, QgsCoordinateReferenceSystem(), origin2, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext() );
  QCOMPARE( worldPoint2, QgsVector3D( 14, 34, -24 ) );
  // verify that both are the same map point
  QgsVector3D mapPoint1 = Qgs3DUtils::worldToMapCoordinates( worldPoint1, origin1 );
  QgsVector3D mapPoint2 = Qgs3DUtils::worldToMapCoordinates( worldPoint2, origin2 );
  QCOMPARE( mapPoint1, mapPoint2 );
}

QGSTEST_MAIN( TestQgs3DUtils )
#include "testqgs3dutils.moc"
