/***************************************************************************
     testqgsgltutils.cpp
     ----------------------
    Date                 : August 2023
    Copyright            : (C) 2023 by Nyall Dawson
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

#include "qgsgltfutils.h"
#include "tiny_gltf.h"

#include "qgsmatrix4x4.h"

class TestQgsGltfUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsGltfUtils() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testImageResourceType();
    void testExtractEmbeddedImage();
    void testExtractTextureCoordinates();

    void testI3SDracoModel3DObject();
    void testI3SDracoModelIntegratedMesh();

  private:
};

//runs before all tests
void TestQgsGltfUtils::initTestCase()
{
}

//runs after all tests
void TestQgsGltfUtils::cleanupTestCase()
{
}

void TestQgsGltfUtils::testImageResourceType()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QCOMPARE( QgsGltfUtils::imageResourceType( model, 0 ), QgsGltfUtils::ResourceType::Embedded );
}

void TestQgsGltfUtils::testExtractEmbeddedImage()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QImage image = QgsGltfUtils::extractEmbeddedImage( model, 0 );
  QVERIFY( !image.isNull() );
  QCOMPARE( image.width(), 256 );
  QCOMPARE( image.height(), 256 );
  QCOMPARE( image.format(), QImage::Format_ARGB32 );
}

void TestQgsGltfUtils::testExtractTextureCoordinates()
{
  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/gltf/BoxTextured.glb";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadGltfModel( data, model, nullptr, nullptr ) );
  QVector<float> x;
  QVector<float> y;
  QVERIFY( QgsGltfUtils::extractTextureCoordinates( model, 3, x, y ) );
  QCOMPARE( x.size(), 24 );
  QCOMPARE( y.size(), 24 );
  QCOMPARE( x.at( 0 ), 6.0 );
  QCOMPARE( y.at( 0 ), 0.0 );
  QCOMPARE( x.at( 23 ), 1.0 );
  QGSCOMPARENEAR( y.at( 23 ), 1.0, 0.01 );
}

void TestQgsGltfUtils::testI3SDracoModel3DObject()
{
  // using sample data from Integrated Mesh I3S dataset linked from I3S spec
  // https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Buildings_NewYork_v18/SceneServer
  //
  // we are using node index 1
  //
  // the geometry's source URL is:
  // https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Buildings_NewYork_v18/SceneServer/layers/0/nodes/201/geometries/1

  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/i3s/newyork-node-1-resource-201.drc";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  // node's OBB's center (lon [deg], lat [deg], altitude [m])
  QgsVector3D nodeCenter( -73.95783168511981, 40.62920809646687, 37.10296226851642 );

  QgsGltfUtils::I3SNodeContext context;
  context.isGlobalMode = true;
  context.datasetToSceneTransform = QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4979" ), QgsCoordinateReferenceSystem( "EPSG:4978" ), QgsCoordinateTransformContext() );
  context.nodeCenterEcef = context.datasetToSceneTransform.transform( nodeCenter );
  context.materialInfo["pbrBaseColorFactor"] = QVariantList { 1.0, 1.0, 1.0, 1.0 };

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadDracoModel( data, context, model, nullptr ) );

  // check the model data
  QCOMPARE( model.scenes.size(), 1 );
  const tinygltf::Scene &scene = model.scenes[0];
  QCOMPARE( scene.nodes.size(), 1 );
  QCOMPARE( scene.nodes[0], 0 );
  const tinygltf::Node &node = model.nodes[0];
  QCOMPARE( node.mesh, 0 );
  const tinygltf::Mesh &mesh = model.meshes[0];
  QCOMPARE( mesh.primitives.size(), 1 );
  const tinygltf::Primitive &primitive = mesh.primitives[0];
  QCOMPARE( primitive.mode, TINYGLTF_MODE_TRIANGLES );
  QCOMPARE( primitive.indices, 2 );
  QCOMPARE( primitive.attributes.size(), 2 );
  QVERIFY( primitive.attributes.find( "POSITION" ) != primitive.attributes.end() );
  QCOMPARE( primitive.attributes.at( "POSITION" ), 0 );

  QCOMPARE( model.accessors.size(), 3 );

  // check positions
  const tinygltf::Accessor &posAccessor = model.accessors[0];
  QCOMPARE( posAccessor.bufferView, 0 );
  QCOMPARE( posAccessor.count, 29992 );
  QCOMPARE( posAccessor.type, TINYGLTF_TYPE_VEC3 );
  QCOMPARE( posAccessor.componentType, TINYGLTF_COMPONENT_TYPE_FLOAT );

  QVector<double> vx, vy, vz;
  QgsGltfUtils::accessorToMapCoordinates( model, 0, QgsMatrix4x4(), nullptr, context.nodeCenterEcef, nullptr, Qgis::Axis::Z, vx, vy, vz );
  QCOMPARE( vx.size(), 29992 );
  QCOMPARE( vx[0], 1339937.494089 );
  QCOMPARE( vy[0], -4657825.592067 );
  QCOMPARE( vz[0], 4132174.028885 );

  // check indices
  const tinygltf::Accessor &indexAccessor = model.accessors[2];
  QCOMPARE( indexAccessor.bufferView, 2 );
  QCOMPARE( indexAccessor.count, 74493 );
  QCOMPARE( indexAccessor.type, TINYGLTF_TYPE_SCALAR );
  QCOMPARE( indexAccessor.componentType, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT );

  const tinygltf::Buffer &indexBuffer = model.buffers[2];
  const uint32_t *indices = reinterpret_cast<const uint32_t *>( indexBuffer.data.data() );
  QCOMPARE( indices[0], 0 );
  QCOMPARE( indices[1], 1 );
  QCOMPARE( indices[2], 2 );
  QCOMPARE( indices[3], 1 );
  QCOMPARE( indices[4], 3 );
  QCOMPARE( indices[5], 2 );
}

void TestQgsGltfUtils::testI3SDracoModelIntegratedMesh()
{
  // using sample data from Integrated Mesh I3S dataset linked from I3S spec
  // https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Rancho_Mesh_v18/SceneServer
  //
  // we are using node index 1
  //
  // the geometry's source URL is:
  // https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Rancho_Mesh_v18/SceneServer/layers/0/nodes/16/geometries/1

  const QString dataDir( TEST_DATA_DIR );
  QString dataFile = dataDir + "/i3s/rancho-node-1-resource-16.drc";

  QFile f( dataFile );
  QVERIFY( f.open( QIODevice::ReadOnly ) );
  const QByteArray data = f.readAll();

  // node's OBB's center (lon [deg], lat [deg], altitude [m])
  QgsVector3D nodeCenter( -117.53793932189886, 34.123819641151094, 410.4816717179492 );

  QgsGltfUtils::I3SNodeContext context;
  context.isGlobalMode = true;
  context.datasetToSceneTransform = QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:4979" ), QgsCoordinateReferenceSystem( "EPSG:4978" ), QgsCoordinateTransformContext() );
  context.nodeCenterEcef = context.datasetToSceneTransform.transform( nodeCenter );
  context.materialInfo["pbrBaseColorFactor"] = QVariantList { 1.0, 1.0, 1.0, 1.0 };
  context.materialInfo["pbrBaseColorTexture"] = "https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Rancho_Mesh_v18/SceneServer/layers/0/nodes/16/textures/0";

  tinygltf::Model model;
  QVERIFY( QgsGltfUtils::loadDracoModel( data, context, model, nullptr ) );

  // check the model data
  QCOMPARE( model.scenes.size(), 1 );
  const tinygltf::Scene &scene = model.scenes[0];
  QCOMPARE( scene.nodes.size(), 1 );
  QCOMPARE( scene.nodes[0], 0 );
  const tinygltf::Node &node = model.nodes[0];
  QCOMPARE( node.mesh, 0 );
  const tinygltf::Mesh &mesh = model.meshes[0];
  QCOMPARE( mesh.primitives.size(), 1 );
  const tinygltf::Primitive &primitive = mesh.primitives[0];
  QCOMPARE( primitive.mode, TINYGLTF_MODE_TRIANGLES );
  QCOMPARE( primitive.indices, 2 );
  QCOMPARE( primitive.attributes.size(), 2 );
  QVERIFY( primitive.attributes.find( "POSITION" ) != primitive.attributes.end() );
  QCOMPARE( primitive.attributes.at( "POSITION" ), 0 );
  QVERIFY( primitive.attributes.find( "TEXCOORD_0" ) != primitive.attributes.end() );
  QCOMPARE( primitive.attributes.at( "TEXCOORD_0" ), 1 );

  QCOMPARE( model.accessors.size(), 3 );

  const tinygltf::Accessor &posAccessor = model.accessors[0];
  QCOMPARE( posAccessor.bufferView, 0 );
  QCOMPARE( posAccessor.count, 12162 );
  QCOMPARE( posAccessor.type, TINYGLTF_TYPE_VEC3 );
  QCOMPARE( posAccessor.componentType, TINYGLTF_COMPONENT_TYPE_FLOAT );

  // check positions
  QVector<double> vx, vy, vz;
  QgsGltfUtils::accessorToMapCoordinates( model, 0, QgsMatrix4x4(), nullptr, context.nodeCenterEcef, nullptr, Qgis::Axis::Z, vx, vy, vz );
  QCOMPARE( vx.size(), 12162 );
  QCOMPARE( vx[0], -2443892.712516 );
  QCOMPARE( vy[0], -4687029.053951 );
  QCOMPARE( vz[0], 3558046.710601 );

  // check texture coordinates
  const tinygltf::Accessor &texAccessor = model.accessors[1];
  QCOMPARE( texAccessor.bufferView, 1 );
  QCOMPARE( texAccessor.count, 12162 );
  QCOMPARE( texAccessor.type, TINYGLTF_TYPE_VEC2 );
  QCOMPARE( texAccessor.componentType, TINYGLTF_COMPONENT_TYPE_FLOAT );

  QVector<float> tx, ty;
  QVERIFY( QgsGltfUtils::extractTextureCoordinates( model, 1, tx, ty ) );
  QCOMPARE( tx[0], 0.597376167774 );
  QCOMPARE( ty[0], 0.083257958293 );

  // check indices
  const tinygltf::Accessor &indexAccessor = model.accessors[2];
  QCOMPARE( indexAccessor.bufferView, 2 );
  QCOMPARE( indexAccessor.count, 60000 );
  QCOMPARE( indexAccessor.type, TINYGLTF_TYPE_SCALAR );
  QCOMPARE( indexAccessor.componentType, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT );

  const tinygltf::Buffer &indexBuffer = model.buffers[2];
  const uint32_t *indices = reinterpret_cast<const uint32_t *>( indexBuffer.data.data() );
  QCOMPARE( indices[0], 0 );
  QCOMPARE( indices[1], 1 );
  QCOMPARE( indices[2], 2 );
  QCOMPARE( indices[3], 3 );
  QCOMPARE( indices[4], 4 );
  QCOMPARE( indices[5], 5 );
}

QGSTEST_MAIN( TestQgsGltfUtils )
#include "testqgsgltfutils.moc"
