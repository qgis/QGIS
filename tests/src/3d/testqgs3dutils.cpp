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

#include "qgsbox3d.h"
#include "qgsray3d.h"

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
    void testRayFromScreenPoint();
    void testQgsBox3DDistanceTo();
    void testQgsRay3D();
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

void TestQgs3DUtils::testRayFromScreenPoint()
{

  Qt3DRender::QCamera camera;
  {
    camera.setFieldOfView( 45.0f );
    camera.setNearPlane( 10.0f );
    camera.setFarPlane( 100.0f );
    camera.setAspectRatio( 2 );
    camera.setPosition( QVector3D( 9.0f, 15.0f, 30.0f ) );
    camera.setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
    camera.setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );

    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 50, 50 ), QSize( 100, 100 ), &camera );
      QgsRay3D ray2( QVector3D( 8.99999904632568, 14.9999980926514, 29.9999980926514 ), QVector3D( -0.25916051864624, -0.431934207677841, -0.863868415355682 ) );
      QVERIFY( ray1 == ray2 );
    }
    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 0, 0 ), QSize( 100, 100 ), &camera );
      QgsRay3D ray2( QVector3D( 8.99999904632568, 14.9999980926514, 29.9999980926514 ), QVector3D( -0.810001313686371, -0.0428109727799892, -0.584863305091858 ) );
      QVERIFY( ray1 == ray2 );
    }
    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 100, 100 ), QSize( 100, 100 ), &camera );
      QgsRay3D ray2( QVector3D( 8.99999904632568, 14.9999980926514, 29.9999980926514 ), QVector3D( 0.429731547832489, -0.590972006320953, -0.682702660560608 ) );
      QVERIFY( ray1 == ray2 );
    }
  }

  {
    camera.setFieldOfView( 60.0f );
    camera.setNearPlane( 1.0f );
    camera.setFarPlane( 1000.0f );
    camera.setAspectRatio( 2 );
    camera.setPosition( QVector3D( 0.0f, 0.0f, 0.0f ) );
    camera.setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
    camera.setViewCenter( QVector3D( 0.0f, 100.0f, -100.0f ) );

    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 500, 500 ), QSize( 1000, 1000 ), &camera );
      QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( 0, 0.70710676908493, -0.70710676908493 ) );
      QVERIFY( ray1 == ray2 );
    }
    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 0, 0 ), QSize( 1000, 1000 ), &camera );
      QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( -0.70710676908493, 0.683012664318085, -0.183012709021568 ) );
      QVERIFY( ray1 == ray2 );
    }
    {
      QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 500, 1000 ), QSize( 1000, 1000 ), &camera );
      QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( 0, 0.258819073438644, -0.965925812721252 ) );
      QVERIFY( ray1 == ray2 );
    }
  }
}

void TestQgs3DUtils::testQgsBox3DDistanceTo()
{
  {
    QgsBox3d box( -1, -1, -1, 1, 1, 1 );
    QVERIFY( box.distanceTo( QVector3D( 0, 0, 0 ) ) == 0.0 );
    QVERIFY( box.distanceTo( QVector3D( 2, 2, 2 ) ) == qSqrt( 3.0 ) );
  }
  {
    QgsBox3d box( 1, 2, 1, 4, 3, 3 );
    QVERIFY( box.distanceTo( QVector3D( 1, 2, 1 ) ) == 0.0 );
    QVERIFY( box.distanceTo( QVector3D( 0, 0, 0 ) ) == qSqrt( 6.0 ) );
  }
}

void TestQgs3DUtils::testQgsRay3D()
{
  QgsRay3D ray( QVector3D( 0, 0, 0 ), QVector3D( 1, 1, 1 ) );
  float t = 1.0f + ( float )( rand() % 1000 ) / 1000.0f;
  QVector3D p1 = ray.origin() + t * ray.direction();
  QVector3D p2 = ray.origin() - t * ray.direction();
  // point already on the ray
  QVERIFY( ray.projectedPoint( p1 ) == p1 );
  QVERIFY( ray.projectedPoint( p2 ) == p2 );

  // t >= 0 then the point is in front of the ray
  QVERIFY( ray.isInFront( p1 ) );
  // t < 0 then the point is in front of the ray
  QVERIFY( !ray.isInFront( p2 ) );

  for ( int i = 0; i < 8; ++i )
  {
    // random vector
    QVector3D n = QVector3D( 1.0f + ( float )( rand() % 1000 ) / 1000.0f, 1.0f + ( float )( rand() % 1000 ) / 1000.0f, 1.0f + ( float )( rand() % 1000 ) / 1000.0f ).normalized();
    // random point on the ray
    float t = 1.0f + ( float )( rand() % 1000 ) / 1000.0f;
    QVector3D p = ray.origin() + t * ray.direction();
    // a random point that projects to p
    QVector3D p2 = p + ( 1.0f + ( float )( rand() % 1000 ) / 1000.0f ) * QVector3D::crossProduct( ray.direction(), n );

    QVERIFY( qFuzzyCompare( ray.projectedPoint( p2 ), p ) );

    float angle = qRadiansToDegrees( std::atan2( ( p2 - ray.origin() ).length(), ( p - ray.origin() ).length() ) );

    QVERIFY( qFuzzyCompare( ( float )ray.angleToPoint( p2 ), angle ) );
  }

}

QGSTEST_MAIN( TestQgs3DUtils )
#include "testqgs3dutils.moc"
