/***************************************************************************
     testqgsgltf3dutils.cpp
     ----------------------
    Date                 : June 2023
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

#include <Qt3DCore/QEntity>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QGeometryRenderer>

#include "qgsgltf3dutils.h"
#include "qgsmetalroughmaterial.h"
#include "qgstexturematerial.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsGltf3DUtils : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsGltf3DUtils()
      : QgsTest( QStringLiteral( "GLTF 3D Utils" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testInvalid();
    void testBox();
    void testBoxTextured();
    void testTransforms();

  private:
};

//runs before all tests
void TestQgsGltf3DUtils::initTestCase()
{
}

//runs after all tests
void TestQgsGltf3DUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGltf3DUtils::testInvalid()
{
  QgsGltf3DUtils::EntityTransform transform;

  QStringList errors1;
  Qt3DCore::QEntity *entity1 = QgsGltf3DUtils::gltfToEntity( QByteArray(), transform, QString(), &errors1 );
  QVERIFY( !entity1 );
  QCOMPARE( errors1.count(), 1 );
  QVERIFY( errors1.first().contains( "GLTF load error: JSON string too short." ) );

  QStringList errors2;
  Qt3DCore::QEntity *entity2 = QgsGltf3DUtils::gltfToEntity( QByteArray( "hello" ), transform, QString(), &errors2 );
  QVERIFY( !entity2 );
  QCOMPARE( errors2.count(), 1 );
  QVERIFY( errors2.first().contains( "GLTF load error:" ) && errors2.first().contains( "error while parsing value" ) );
}

void TestQgsGltf3DUtils::testBox()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/Box.glb";

  QgsGltf3DUtils::EntityTransform transform;

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );

  Qt3DCore::QEntity *entity = QgsGltf3DUtils::gltfToEntity( f.readAll(), transform, QString(), nullptr );
  QVERIFY( entity );

  QCOMPARE( entity->children().count(), 1 ); // there's one primitive to render
  Qt3DCore::QEntity *child = qobject_cast<Qt3DCore::QEntity *>( entity->children()[0] );
  QVERIFY( child );

  QVector<Qt3DRender::QGeometryRenderer *> geomRenderers = child->componentsOfType<Qt3DRender::QGeometryRenderer>();
  QCOMPARE( geomRenderers.count(), 1 );
  Qt3DRender::QGeometryRenderer *geomRenderer = geomRenderers[0];
  QCOMPARE( geomRenderer->vertexCount(), 36 );
  QCOMPARE( geomRenderer->primitiveType(), Qt3DRender::QGeometryRenderer::Triangles );
  Qt3DQGeometry *geometry = geomRenderer->geometry();
  QVERIFY( geometry );
  QVector<Qt3DQAttribute *> attributes = geometry->attributes();
  QCOMPARE( attributes.count(), 3 );

  Qt3DQAttribute *positionAttr = attributes[0];
  QCOMPARE( positionAttr->name(), Qt3DQAttribute::defaultPositionAttributeName() );
  QCOMPARE( positionAttr->attributeType(), Qt3DQAttribute::VertexAttribute );
  QCOMPARE( positionAttr->count(), 24 );
  QCOMPARE( positionAttr->vertexBaseType(), Qt3DQAttribute::Float );
  QCOMPARE( positionAttr->vertexSize(), 3 );

  Qt3DQAttribute *normalAttr = attributes[1];
  QCOMPARE( normalAttr->name(), Qt3DQAttribute::defaultNormalAttributeName() );
  QCOMPARE( normalAttr->attributeType(), Qt3DQAttribute::VertexAttribute );
  QCOMPARE( normalAttr->count(), 24 );
  QCOMPARE( normalAttr->vertexBaseType(), Qt3DQAttribute::Float );
  QCOMPARE( normalAttr->vertexSize(), 3 );

  Qt3DQAttribute *indexAttr = attributes[2];
  QCOMPARE( indexAttr->attributeType(), Qt3DQAttribute::IndexAttribute );
  QCOMPARE( indexAttr->count(), 36 );
  QCOMPARE( indexAttr->vertexBaseType(), Qt3DQAttribute::UnsignedShort );
  QCOMPARE( indexAttr->vertexSize(), 1 );

  QVector<QgsMetalRoughMaterial *> pbrMaterials = child->componentsOfType<QgsMetalRoughMaterial>();
  QCOMPARE( pbrMaterials.count(), 1 );
  QgsMetalRoughMaterial *pbrMaterial = pbrMaterials[0];
  QCOMPARE( pbrMaterial->baseColor(), QColor::fromRgbF( 0.8, 0.0, 0.0, 1.0 ) );

  delete entity;
}

void TestQgsGltf3DUtils::testBoxTextured()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QgsGltf3DUtils::EntityTransform transform;

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );

  Qt3DCore::QEntity *entity = QgsGltf3DUtils::gltfToEntity( f.readAll(), transform, QString(), nullptr );
  QVERIFY( entity );

  QCOMPARE( entity->children().count(), 1 ); // there's one primitive to render
  Qt3DCore::QEntity *child = qobject_cast<Qt3DCore::QEntity *>( entity->children()[0] );
  QVERIFY( child );

  QVector<Qt3DRender::QGeometryRenderer *> geomRenderers = child->componentsOfType<Qt3DRender::QGeometryRenderer>();
  QCOMPARE( geomRenderers.count(), 1 );
  Qt3DRender::QGeometryRenderer *geomRenderer = geomRenderers[0];
  QCOMPARE( geomRenderer->vertexCount(), 36 );
  QCOMPARE( geomRenderer->primitiveType(), Qt3DRender::QGeometryRenderer::Triangles );
  Qt3DQGeometry *geometry = geomRenderer->geometry();
  QVERIFY( geometry );
  QVector<Qt3DQAttribute *> attributes = geometry->attributes();
  QCOMPARE( attributes.count(), 4 );

  Qt3DQAttribute *positionAttr = attributes[0];
  QCOMPARE( positionAttr->name(), Qt3DQAttribute::defaultPositionAttributeName() );
  QCOMPARE( positionAttr->attributeType(), Qt3DQAttribute::VertexAttribute );
  QCOMPARE( positionAttr->count(), 24 );
  QCOMPARE( positionAttr->vertexBaseType(), Qt3DQAttribute::Float );
  QCOMPARE( positionAttr->vertexSize(), 3 );

  Qt3DQAttribute *normalAttr = attributes[1];
  QCOMPARE( normalAttr->name(), Qt3DQAttribute::defaultNormalAttributeName() );
  QCOMPARE( normalAttr->attributeType(), Qt3DQAttribute::VertexAttribute );
  QCOMPARE( normalAttr->count(), 24 );
  QCOMPARE( normalAttr->vertexBaseType(), Qt3DQAttribute::Float );
  QCOMPARE( normalAttr->vertexSize(), 3 );

  Qt3DQAttribute *texAttr = attributes[2];
  QCOMPARE( texAttr->name(), Qt3DQAttribute::defaultTextureCoordinateAttributeName() );
  QCOMPARE( texAttr->attributeType(), Qt3DQAttribute::VertexAttribute );
  QCOMPARE( texAttr->count(), 24 );
  QCOMPARE( texAttr->vertexBaseType(), Qt3DQAttribute::Float );
  QCOMPARE( texAttr->vertexSize(), 2 );

  Qt3DQAttribute *indexAttr = attributes[3];
  QCOMPARE( indexAttr->attributeType(), Qt3DQAttribute::IndexAttribute );
  QCOMPARE( indexAttr->count(), 36 );
  QCOMPARE( indexAttr->vertexBaseType(), Qt3DQAttribute::UnsignedShort );
  QCOMPARE( indexAttr->vertexSize(), 1 );

  QVector<QgsTextureMaterial *> textureMaterials = child->componentsOfType<QgsTextureMaterial>();
  QCOMPARE( textureMaterials.count(), 1 );
  QgsTextureMaterial *textureMaterial = textureMaterials[0];
  QVERIFY( textureMaterial->texture() );

  delete entity;
}

static void extractTriangleCoordinates( Qt3DCore::QEntity *entity, QVector3D &v1, QVector3D &v2, QVector3D &v3 )
{
  Qt3DCore::QEntity *child = qobject_cast<Qt3DCore::QEntity *>( entity->children()[0] );
  QVector<Qt3DRender::QGeometryRenderer *> geomRenderers = child->componentsOfType<Qt3DRender::QGeometryRenderer>();
  Qt3DRender::QGeometryRenderer *geomRenderer = geomRenderers[0];
  Qt3DQAttribute *positionAttr = geomRenderer->geometry()->attributes()[0];
  QByteArray positionBufferData = positionAttr->buffer()->data();
  const float *f = ( float * ) positionBufferData.constData();
  v1 = QVector3D( f[0], f[1], f[2] );
  v2 = QVector3D( f[3], f[4], f[5] );
  v3 = QVector3D( f[6], f[7], f[8] );
}

void TestQgsGltf3DUtils::testTransforms()
{
  // this triangle has the following vertices:
  // [0,0,0]
  // [1,0,0]
  // [0,1,0]

  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/TriangleWithoutIndices.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  QByteArray gltfData = f.readAll();
  QVector3D v1, v2, v3;

  // with no transforms, coordinates are not modified
  QgsGltf3DUtils::EntityTransform transform1;
  Qt3DCore::QEntity *entity1 = QgsGltf3DUtils::gltfToEntity( gltfData, transform1, QString(), nullptr );
  extractTriangleCoordinates( entity1, v1, v2, v3 );
  QCOMPARE( v1, QVector3D( 0, 0, 0 ) );
  QCOMPARE( v2, QVector3D( 1, 0, 0 ) );
  QCOMPARE( v3, QVector3D( 0, 0, 1 ) );
  delete entity1;

  QgsGltf3DUtils::EntityTransform transform2;
  transform2.chunkOriginTargetCrs = QgsVector3D( -10, -20, 0 );
  Qt3DCore::QEntity *entity2 = QgsGltf3DUtils::gltfToEntity( gltfData, transform2, QString(), nullptr );
  extractTriangleCoordinates( entity2, v1, v2, v3 );
  QCOMPARE( v1, QVector3D( 10, 20, 0 ) );
  QCOMPARE( v2, QVector3D( 11, 20, 0 ) );
  QCOMPARE( v3, QVector3D( 10, 20, 1 ) );
  delete entity2;

  QgsGltf3DUtils::EntityTransform transform3;
  transform3.tileTransform = QgsMatrix4x4( 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1 );
  transform3.chunkOriginTargetCrs = QgsVector3D( -10, -20, 0 );
  Qt3DCore::QEntity *entity3 = QgsGltf3DUtils::gltfToEntity( gltfData, transform3, QString(), nullptr );
  extractTriangleCoordinates( entity3, v1, v2, v3 );
  QCOMPARE( v1, QVector3D( 10, 20, 0 ) );
  QCOMPARE( v2, QVector3D( 12, 20, 0 ) );
  QCOMPARE( v3, QVector3D( 10, 20, 2 ) );
  delete entity3;
}

QGSTEST_MAIN( TestQgsGltf3DUtils )
#include "testqgsgltf3dutils.moc"
