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

#include "qgs3dexportobject.h"

#include <QSize>
#include <QtMath>

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgs3DUtils : public QgsTest
{
    Q_OBJECT
  public:
    TestQgs3DUtils() : QgsTest( QStringLiteral( "3D Utils" ), QStringLiteral( "3d" ) ) { }

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testTransforms();
    void testRayFromScreenPoint();
    void testQgsBox3DDistanceTo();
    void testQgsRay3D();
    void testExportToObj();
  private:
};

//runs before all tests
void TestQgs3DUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgs3DUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DUtils::testTransforms()
{
  const QgsVector3D map123( 1, 2, 3 );

  const QgsVector3D world123 = Qgs3DUtils::mapToWorldCoordinates( map123, QgsVector3D() );
  QCOMPARE( world123, QgsVector3D( 1, 3, -2 ) );

  const QgsVector3D world123map = Qgs3DUtils::worldToMapCoordinates( world123, QgsVector3D() );
  QCOMPARE( world123map, map123 );

  // now with non-zero origin

  const QgsVector3D origin( -10, -20, -30 );

  const QgsVector3D world123x = Qgs3DUtils::mapToWorldCoordinates( map123, origin );
  QCOMPARE( world123x, QgsVector3D( 11, 33, -22 ) );

  const QgsVector3D world123xmap = Qgs3DUtils::worldToMapCoordinates( world123x, origin );
  QCOMPARE( world123xmap, map123 );

  //
  // transform world point from one system to another
  //

  const QgsVector3D worldPoint1( 5, 7, -6 );
  const QgsVector3D origin1( 10, 20, 30 );
  const QgsVector3D origin2( 1, 2, 3 );
  const QgsVector3D worldPoint2 = Qgs3DUtils::transformWorldCoordinates( worldPoint1, origin1, QgsCoordinateReferenceSystem(), origin2, QgsCoordinateReferenceSystem(), QgsCoordinateTransformContext() );
  QCOMPARE( worldPoint2, QgsVector3D( 14, 34, -24 ) );
  // verify that both are the same map point
  const QgsVector3D mapPoint1 = Qgs3DUtils::worldToMapCoordinates( worldPoint1, origin1 );
  const QgsVector3D mapPoint2 = Qgs3DUtils::worldToMapCoordinates( worldPoint2, origin2 );
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
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 50, 50 ), QSize( 100, 100 ), &camera );
      const QgsRay3D ray2( QVector3D( 8.99999904632568f, 14.9999980926514f, 29.9999980926514f ), QVector3D( -0.25916051864624f, -0.431934207677841f, -0.863868415355682f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
    }
    {
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 0, 0 ), QSize( 100, 100 ), &camera );
      const QgsRay3D ray2( QVector3D( 8.99999904632568f, 14.9999980926514f, 29.9999980926514f ), QVector3D( -0.810001313686371f, -0.0428109727799892f, -0.584863305091858f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
    }
    {
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 100, 100 ), QSize( 100, 100 ), &camera );
      const QgsRay3D ray2( QVector3D( 8.99999904632568f, 14.9999980926514f, 29.9999980926514f ), QVector3D( 0.429731547832489f, -0.590972006320953f, -0.682702660560608f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
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
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 500, 500 ), QSize( 1000, 1000 ), &camera );
      const QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( 0, 0.70710676908493f, -0.70710676908493f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
    }
    {
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 0, 0 ), QSize( 1000, 1000 ), &camera );
      const QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( -0.70710676908493f, 0.683012664318085f, -0.183012709021568f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
    }
    {
      const QgsRay3D ray1 = Qgs3DUtils::rayFromScreenPoint( QPoint( 500, 1000 ), QSize( 1000, 1000 ), &camera );
      const QgsRay3D ray2( QVector3D( 0, 0, 0 ), QVector3D( 0, 0.258819073438644f, -0.965925812721252f ) );
      QGSCOMPARENEARVECTOR3D( ray1.origin(), ray2.origin(), 0.00001 );
      QGSCOMPARENEARVECTOR3D( ray1.direction(), ray2.direction(), 0.00001 );
    }
  }
}

void TestQgs3DUtils::testQgsBox3DDistanceTo()
{
  {
    const QgsBox3D box( -1, -1, -1, 1, 1, 1 );
    QCOMPARE( box.distanceTo( QVector3D( 0, 0, 0 ) ), 0.0 );
    QCOMPARE( box.distanceTo( QVector3D( 2, 2, 2 ) ), qSqrt( 3.0 ) );
  }
  {
    const QgsBox3D box( 1, 2, 1, 4, 3, 3 );
    QCOMPARE( box.distanceTo( QVector3D( 1, 2, 1 ) ), 0.0 );
    QCOMPARE( box.distanceTo( QVector3D( 0, 0, 0 ) ), qSqrt( 6.0 ) );
  }
}

void TestQgs3DUtils::testQgsRay3D()
{
  {
    const QgsRay3D ray( QVector3D( 0, 0, 0 ), QVector3D( 1, 1, 1 ) );
    const QVector3D p1( 0.5f, 0.5f, 0.5f );
    const QVector3D p2( -0.5f, -0.5f, -0.5f );
    // points are already on the ray
    const QVector3D projP1 = ray.projectedPoint( p1 );
    const QVector3D projP2 = ray.projectedPoint( p2 );

    QCOMPARE( p1.x(), projP1.x() );
    QCOMPARE( p1.y(), projP1.y() );
    QCOMPARE( p1.z(), projP1.z() );

    QCOMPARE( p2.x(), projP2.x() );
    QCOMPARE( p2.y(), projP2.y() );
    QCOMPARE( p2.z(), projP2.z() );

    QVERIFY( qFuzzyIsNull( ( float )ray.angleToPoint( p1 ) ) );
    QVERIFY( qFuzzyIsNull( ( float )ray.angleToPoint( p2 ) ) );

    QVERIFY( ray.isInFront( p1 ) );
    QVERIFY( !ray.isInFront( p2 ) );
  }

  {
    const QgsRay3D ray( QVector3D( 0, 0, 0 ), QVector3D( 1, 1, 1 ) );
    const QVector3D p1( 0, 1, 1 );
    const QVector3D p2( 0, -1, -1 );
    const QVector3D expectedProjP1( 0.666667f, 0.666667f, 0.666667f );
    const QVector3D expectedProjP2( -0.666667f, -0.666667f, -0.666667f );

    const QVector3D projP1 = ray.projectedPoint( p1 );
    QCOMPARE( projP1.x(), expectedProjP1.x() );
    QCOMPARE( projP1.y(), expectedProjP1.y() );
    QCOMPARE( projP1.z(), expectedProjP1.z() );

    const QVector3D projP2 = ray.projectedPoint( p2 );
    QCOMPARE( projP2.x(), expectedProjP2.x() );
    QCOMPARE( projP2.y(), expectedProjP2.y() );
    QCOMPARE( projP2.z(), expectedProjP2.z() );

    QVERIFY( ray.isInFront( p1 ) );
    QVERIFY( !ray.isInFront( p2 ) );
  }
}

void TestQgs3DUtils::testExportToObj()
{
  // all vertice positions
  QVector<float> positionData =
  {
    -0.456616, 0.00187836, -0.413774,
      -0.4718, 0.00187836, -0.0764642,
      -0.25705, 0.00187836, -0.230477,
      -0.25705, 0.00187836, -0.230477,
      -0.4718, 0.00187836, -0.0764642,
      0.0184382, 0.00187836, 0.177332,
      -0.25705, 0.00187836, -0.230477,
      0.0184382, 0.00187836, 0.177332,
      -0.25705, -0.00187836, -0.230477,
      -0.25705, -0.00187836, -0.230477,
      0.0184382, 0.00187836, 0.177332,
      0.0184382, -0.00187836, 0.177332,
      0.0184382, 0.00187836, 0.177332,
      -0.4718, 0.00187836, -0.0764642,
      0.0184382, -0.00187836, 0.177332,
      0.0184382, -0.00187836, 0.177332,
      -0.4718, 0.00187836, -0.0764642,
      -0.4718, -0.00187836, -0.0764642,
      -0.4718, 0.00187836, -0.0764642,
      -0.456616, 0.00187836, -0.413774,
      -0.4718, -0.00187836, -0.0764642,
      -0.4718, -0.00187836, -0.0764642,
      -0.456616, 0.00187836, -0.413774,
      -0.456616, -0.00187836, -0.413774,
      -0.456616, 0.00187836, -0.413774,
      -0.25705, 0.00187836, -0.230477,
      -0.456616, -0.00187836, -0.413774,
      -0.456616, -0.00187836, -0.413774,
      -0.25705, 0.00187836, -0.230477,
      -0.25705, -0.00187836, -0.230477
    };

  // all vertice normals
  QVector<float> normalsData =
  {
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0, 1, 0,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    0.828644, 0, -0.559776,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.459744, 0, 0.888052,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    -0.998988, 0, -0.0449705,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489,
    0.676449, 0, -0.736489
  };
  float scale = 1.0f;
  QVector3D translation( 0.0f, 0.0f, 0.0f );

  const QString myTmpDir = QDir::tempPath() + '/';

  // case where all vertices are used
  {
    Qgs3DExportObject object( "all_faces" );
    object.setupPositionCoordinates( positionData, scale, translation );
    QCOMPARE( object.vertexPosition().size(), positionData.size() );

    // exported vertice indexes
    QVector<uint> indexData =
    {
      0, 1, 2,
      3, 4, 5,
      6, 7, 8,
      9, 10, 11,
      12, 13, 14,
      15, 16, 17,
      18, 19, 20,
      21, 22, 23,
      24, 25, 26,
      27, 28, 29,
    };

    object.setupFaces( indexData );
    QCOMPARE( object.indexes().size(), indexData.size() );

    object.setupNormalCoordinates( normalsData );
    QCOMPARE( object.normals().size(), normalsData.size() );


    QFile file( myTmpDir + "all_faces.obj" );
    file.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate );
    QTextStream out( &file );

    out << "o " << object.name() << "\n";
    object.saveTo( out, scale, translation, 3 );

    out.flush();
    out.seek( 0 );

    QString actual = out.readAll();
    QGSCOMPARELONGSTR( "export_obj", "all_faces.obj", actual.toUtf8() );
  }

  // case where only a subset of vertices are used
  {
    // exported vertice indexes
    QVector<uint> indexData =
    {
      // 0, 1, 2,
      // 3, 4, 5,
      6, 7, 8,
      9, 10, 11,
      12, 13, 14,
      15, 16, 17,
      // 18, 19, 20,
      21, 22, 23,
      // 24, 25, 26,
      // 27, 28, 29,
    };

    Qgs3DExportObject object( "sparse_faces" );
    object.setupPositionCoordinates( positionData, scale, translation );
    QCOMPARE( object.vertexPosition().size(), positionData.size() );

    object.setupFaces( indexData );
    QCOMPARE( object.indexes().size(), indexData.size() );

    object.setupNormalCoordinates( normalsData );
    QCOMPARE( object.normals().size(), normalsData.size() );

    QFile file( myTmpDir + "sparse_faces.obj" );
    file.open( QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate );
    QTextStream out( &file );
    out << "o " << object.name() << "\n";
    object.saveTo( out, scale, translation, 3 );

    out.flush();
    out.seek( 0 );

    QString actual = out.readAll();
    QGSCOMPARELONGSTR( "export_obj", "sparse_faces.obj", actual.toUtf8() );
  }
}


QGSTEST_MAIN( TestQgs3DUtils )
#include "testqgs3dutils.moc"
