/***************************************************************************
                         testqgsmeshlayer.cpp
                         --------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
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

//qgis includes...
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"

#include "qgsmeshdataprovidertemporalcapabilities.h"

/**
 * \ingroup UnitTests
 * This is a unit test for a mesh layer
 */
class TestQgsMeshLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsMeshLayer() = default;

  private:
    QString mDataDir;
    QgsMeshLayer *mMemoryLayer = nullptr;
    QgsMeshLayer *mMdalLayer = nullptr;
    QgsMeshLayer *mMemory1DLayer = nullptr;
    QgsMeshLayer *mMdal1DLayer = nullptr;
    QgsMeshLayer *mMdal3DLayer = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    QString readFile( const QString &fname ) const;

    void test_write_read_project();

    void test_read_1d_edge_vector_dataset();
    void test_read_1d_edge_scalar_dataset();
    void test_read_1d_vertex_vector_dataset();
    void test_read_1d_vertex_scalar_dataset();
    void test_read_1d_bed_elevation_dataset();
    void test_read_1d_mesh();

    void test_read_mesh();
    void test_read_flower_mesh();
    void test_read_bed_elevation_dataset();
    void test_read_vertex_scalar_dataset();
    void test_read_vertex_vector_dataset();
    void test_read_face_scalar_dataset();
    void test_read_face_vector_dataset();
    void test_read_vertex_scalar_dataset_with_inactive_face();
    void test_extent();

    void test_temporal();

    void test_reload();
    void test_reload_extra_dataset();

    void test_mesh_simplification();

    void test_snap_on_mesh();
    void test_dataset_value_from_layer();

    void test_dataset_group_item_tree_item();

    void test_memory_dataset_group();
    void test_memory_dataset_group_1d();
};

QString TestQgsMeshLayer::readFile( const QString &fname ) const
{
  QString uri;
  QFile f( mDataDir + fname );
  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
    uri = f.readAll();
  return uri;
}


void TestQgsMeshLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/mesh";

  // Memory layer
  mMemoryLayer = new QgsMeshLayer( readFile( "/quad_and_triangle.txt" ), "Triangle and Quad Memory", "mesh_memory" );
  QVERIFY( mMemoryLayer->isValid() );
  QCOMPARE( mMemoryLayer->dataProvider()->extraDatasets().count(), 0 );
  QVERIFY( !mMemoryLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( !mMemoryLayer->temporalProperties()->isActive() );
  QCOMPARE( mMemoryLayer->datasetGroupTreeRootItem()->childCount(), 0 );
  mMemoryLayer->addDatasets( readFile( "/quad_and_triangle_bed_elevation.txt" ) );
  mMemoryLayer->addDatasets( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->addDatasets( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->addDatasets( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->addDatasets( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QCOMPARE( mMemoryLayer->dataProvider()->extraDatasets().count(), 5 );
  QCOMPARE( mMemoryLayer->datasetGroupTreeRootItem()->childCount(), 5 );
  QVERIFY( mMemoryLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMemoryLayer->temporalProperties()->isActive() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );

  // MDAL Layer
  QString uri( mDataDir + "/quad_and_triangle.2dm" );
  mMdalLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  QCOMPARE( mMdalLayer->dataProvider()->datasetGroupCount(), 1 ); //bed elevation is already in the 2dm
  QCOMPARE( mMdalLayer->datasetGroupTreeRootItem()->childCount(), 1 );
  QVERIFY( !mMdalLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_vector.dat" );
  QCOMPARE( mMdalLayer->dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( mMdalLayer->datasetGroupTreeRootItem()->childCount(), 3 );
  QVERIFY( mMdalLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );

  //The face dataset is recognized by "_els_" in the filename for this format
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_vector.dat" );
  QCOMPARE( mMdalLayer->datasetGroupTreeRootItem()->childCount(), 5 );

  QVERIFY( mMdalLayer->isValid() );
  QVERIFY( mMemoryLayer->temporalProperties()->isActive() );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMdalLayer );

  // Memory 1D layer
  mMemory1DLayer = new QgsMeshLayer( readFile( "/lines.txt" ), "Lines Memory", "mesh_memory" );
  QVERIFY( mMemory1DLayer->isValid() );
  QCOMPARE( mMemory1DLayer->dataProvider()->extraDatasets().count(), 0 );
  QVERIFY( !mMemory1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( !mMemory1DLayer->temporalProperties()->isActive() );
  QCOMPARE( mMemory1DLayer->datasetGroupTreeRootItem()->childCount(), 0 );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_bed_elevation.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_vector.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_vector.txt" ) );
  QCOMPARE( mMemory1DLayer->dataProvider()->extraDatasets().count(), 5 );
  QCOMPARE( mMemory1DLayer->datasetGroupTreeRootItem()->childCount(), 5 );
  QVERIFY( mMemory1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMemory1DLayer->temporalProperties()->isActive() );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemory1DLayer );

  // MDAL 1D Layer
  uri = QString( mDataDir + "/lines.2dm" );
  mMdal1DLayer = new QgsMeshLayer( uri, "Lines MDAL", "mdal" );
  QCOMPARE( mMdal1DLayer->dataProvider()->datasetGroupCount(), 1 ); //bed elevation is already in the 2dm
  QCOMPARE( mMdal1DLayer->datasetGroupTreeRootItem()->childCount(), 1 );
  QVERIFY( !mMdal1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( !mMdal1DLayer->temporalProperties()->isActive() );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_vertex_scalar.dat" );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_vertex_vector.dat" );
  QCOMPARE( mMdal1DLayer->dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( mMdal1DLayer->datasetGroupTreeRootItem()->childCount(), 3 );
  QVERIFY( mMdal1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMdal1DLayer->temporalProperties()->isActive() );


  //The face dataset is recognized by "_els_" in the filename for this format
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_els_scalar.dat" );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_els_vector.dat" );
  QCOMPARE( mMdal1DLayer->datasetGroupTreeRootItem()->childCount(), 5 );

  QVERIFY( mMdal1DLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMdal1DLayer );


  // MDAL 3D Layer
  uri = QString( mDataDir + "/trap_steady_05_3D.nc" );
  mMdal3DLayer = new QgsMeshLayer( uri, "Lines MDAL", "mdal" );

  QVERIFY( mMdal3DLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMdal3DLayer );
}

void TestQgsMeshLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMeshLayer::test_read_flower_mesh()
{
  QString uri( mDataDir + "/quad_flower.2dm" );
  QgsMeshLayer layer( uri, "Quad Flower MDAL", "mdal" );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 ); //bed elevation is already in the 2dm
  QVERIFY( layer.dataProvider() != nullptr );
  QVERIFY( layer.dataProvider()->isValid() );
  QCOMPARE( 8, layer.dataProvider()->vertexCount() );
  QCOMPARE( 5, layer.dataProvider()->faceCount() );
}

void TestQgsMeshLayer::test_read_1d_mesh()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QgsMesh mesh;
    dp->populateMesh( &mesh );

    QCOMPARE( 4, dp->vertexCount() );
    const QVector<QgsMeshVertex> vertices = mesh.vertices;
    QCOMPARE( 1000.0, vertices.at( 0 ).x() );
    QCOMPARE( 2000.0, vertices.at( 1 ).x() );
    QCOMPARE( 3000.0, vertices.at( 2 ).x() );
    QCOMPARE( 2000.0, vertices.at( 3 ).x() );
    QCOMPARE( 2000.0, vertices.at( 0 ).y() );
    QCOMPARE( 2000.0, vertices.at( 1 ).y() );
    QCOMPARE( 2000.0, vertices.at( 2 ).y() );
    QCOMPARE( 3000.0, vertices.at( 3 ).y() );

    QCOMPARE( 3, dp->edgeCount() );
    const QVector<QgsMeshEdge> edges = mesh.edges;
    QgsMeshEdge f1( 0, 1 );
    QCOMPARE( f1, edges.at( 0 ) );

    QgsMeshEdge f2( 1, 2 );
    QCOMPARE( f2, edges.at( 1 ) );
  }
}

void TestQgsMeshLayer::test_read_1d_bed_elevation_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemory1DLayer );
  layers.append( mMdal1DLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );


    QgsMeshDatasetIndex ds( 0, 0 );

    QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    meta = ly->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );


    QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    dmeta = ly->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    // We have 4 values, since dp->vertexCount() = 4
    QCOMPARE( QgsMeshDatasetValue( 20 ), dp->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), dp->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), dp->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), dp->datasetValue( ds, 3 ) );

    QCOMPARE( QgsMeshDatasetValue( 20 ), ly->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), ly->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), ly->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), ly->datasetValue( ds, 3 ) );
  }
}

void TestQgsMeshLayer::test_read_1d_vertex_scalar_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemory1DLayer );
  layers.append( mMdal1DLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 1, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      meta = ly->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 4 values, since dp->vertexCount() = 4
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 4.0 + i ), dp->datasetValue( ds, 3 ) );

      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), ly->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 4.0 + i ), ly->datasetValue( ds, 3 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_1d_vertex_vector_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemory1DLayer );
  layers.append( mMdal1DLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 2, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      meta = ly->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 4 values, since dp->vertexCount() = 4
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 3 ) );

      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), ly->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), ly->datasetValue( ds, 3 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_1d_edge_scalar_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemory1DLayer );
  layers.append( mMdal1DLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 3, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 3 values, since dp->edgeCount() = 3
      QCOMPARE( QgsMeshDatasetValue( 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i ), dp->datasetValue( ds, 2 ) );

      QCOMPARE( QgsMeshDatasetValue( 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i ), ly->datasetValue( ds, 2 ) );
    }
  }
}


void TestQgsMeshLayer::test_read_1d_edge_vector_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemory1DLayer );
  layers.append( mMdal1DLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 4, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 3 values, since dp->edgeCount() = 3
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 3 + i ), dp->datasetValue( ds, 2 ) );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 3 values, since dp->edgeCount() = 3
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 3 + i ), ly->datasetValue( ds, 2 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_mesh()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QgsMesh mesh;
    dp->populateMesh( &mesh );

    QCOMPARE( 5, dp->vertexCount() );
    const QVector<QgsMeshVertex> vertices = mesh.vertices;
    QCOMPARE( 1000.0, vertices.at( 0 ).x() );
    QCOMPARE( 2000.0, vertices.at( 1 ).x() );
    QCOMPARE( 3000.0, vertices.at( 2 ).x() );
    QCOMPARE( 2000.0, vertices.at( 3 ).x() );
    QCOMPARE( 1000.0, vertices.at( 4 ).x() );
    QCOMPARE( 2000.0, vertices.at( 0 ).y() );
    QCOMPARE( 2000.0, vertices.at( 1 ).y() );
    QCOMPARE( 2000.0, vertices.at( 2 ).y() );
    QCOMPARE( 3000.0, vertices.at( 3 ).y() );
    QCOMPARE( 3000.0, vertices.at( 4 ).y() );

    QCOMPARE( 2, dp->faceCount() );
    const QVector<QgsMeshFace> faces = mesh.faces;
    QgsMeshFace f1;
    f1 << 0 << 1 << 3 << 4;
    QCOMPARE( f1, faces.at( 0 ) );

    QgsMeshFace f2;
    f2 << 1 << 2 << 3;
    QCOMPARE( f2, faces.at( 1 ) );
  }
}

void TestQgsMeshLayer::test_read_bed_elevation_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemoryLayer );
  layers.append( mMdalLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    QgsMeshDatasetIndex ds( 0, 0 );

    QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    // We have 5 values, since dp->vertexCount() = 5
    QCOMPARE( QgsMeshDatasetValue( 20 ), dp->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), dp->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), dp->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), dp->datasetValue( ds, 3 ) );
    QCOMPARE( QgsMeshDatasetValue( 10 ), dp->datasetValue( ds, 4 ) );

    QVERIFY( dp->isFaceActive( ds, 0 ) );

    meta = ly->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    dmeta = ly->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    // We have 5 values, since dp->vertexCount() = 5
    QCOMPARE( QgsMeshDatasetValue( 20 ), ly->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), ly->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), ly->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), ly->datasetValue( ds, 3 ) );
    QCOMPARE( QgsMeshDatasetValue( 10 ), ly->datasetValue( ds, 4 ) );

    QVERIFY( ly->isFaceActive( ds, 0 ) );
  }
}

void TestQgsMeshLayer::test_read_vertex_scalar_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemoryLayer );
  layers.append( mMdalLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 1, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
      {
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Scalar Dataset" ) );
        QCOMPARE( meta.extraOptions()["meta2"], QString( "best dataset" ) );
      }
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 4 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
      {
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Scalar Dataset" ) );
        QCOMPARE( meta.extraOptions()["meta2"], QString( "best dataset" ) );
      }
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), ly->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), ly->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), ly->datasetValue( ds, 4 ) );

      QVERIFY( ly->isFaceActive( ds, 0 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_vertex_vector_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemoryLayer );
  layers.append( mMdalLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 2, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1 + i, -2 + i ), dp->datasetValue( ds, 4 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), ly->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), ly->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), ly->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1 + i, -2 + i ), ly->datasetValue( ds, 4 ) );

      QVERIFY( ly->isFaceActive( ds, 0 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_face_scalar_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemoryLayer );
  layers.append( mMdalLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 3, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), dp->datasetValue( ds, 1 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), ly->datasetValue( ds, 1 ) );

      QVERIFY( ly->isFaceActive( ds, 0 ) );
    }
  }
}


void TestQgsMeshLayer::test_read_face_vector_dataset()
{
  QList<const QgsMeshLayer *> layers;
  layers.append( mMemoryLayer );
  layers.append( mMdalLayer );

  foreach ( auto ly, layers )
  {
    const QgsMeshDataProvider *dp = ly->dataProvider();
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );
    QCOMPARE( 5, ly->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 4, i );

      QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 1 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );

      meta = ly->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      dmeta = ly->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), ly->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), ly->datasetValue( ds, 1 ) );

      QVERIFY( ly->isFaceActive( ds, 0 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_vertex_scalar_dataset_with_inactive_face()
{
  QString uri( mDataDir + "/quad_and_triangle.2dm" );
  QgsMeshLayer layer( uri, "Triangle and Quad MDAL", "mdal" );
  layer.dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_scalar_with_inactive_face.dat" );
  QgsMeshDataProvider *dp = layer.dataProvider();
  QVERIFY( dp != nullptr );
  QVERIFY( dp->isValid() );
  QCOMPARE( 2, dp->datasetGroupCount() );
  QCOMPARE( 2, layer.datasetGroupCount() );

  for ( int i = 0; i < 2 ; ++i )
  {
    QgsMeshDatasetIndex ds( 1, i );

    QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QCOMPARE( meta.name(), QString( "VertexScalarDatasetWithInactiveFace1" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
    QVERIFY( dmeta.isValid() );

    // We have 5 values, since dp->vertexCount() = 5
    QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), dp->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 3 ) );
    QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 4 ) );

    // We have 2 faces
    QVERIFY( !dp->isFaceActive( ds, 0 ) );
    QVERIFY( dp->isFaceActive( ds, 1 ) );

    meta = layer.datasetGroupMetadata( ds );
    QCOMPARE( meta.name(), QString( "VertexScalarDatasetWithInactiveFace1" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    dmeta = layer.datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
    QVERIFY( dmeta.isValid() );

    // We have 5 values, since dp->vertexCount() = 5
    QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), layer.datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), layer.datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), layer.datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), layer.datasetValue( ds, 3 ) );
    QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), layer.datasetValue( ds, 4 ) );

    // We have 2 faces
    QVERIFY( !layer.isFaceActive( ds, 0 ) );
    QVERIFY( layer.isFaceActive( ds, 1 ) );
  }
}

void TestQgsMeshLayer::test_extent()
{
  QgsRectangle expectedExtent( 1000.0, 2000.0, 3000.0, 3000.0 );

  QCOMPARE( expectedExtent, mMemoryLayer->extent() );
  QCOMPARE( mMemoryLayer->dataProvider()->extent(), mMemoryLayer->extent() );

  QCOMPARE( expectedExtent, mMdalLayer->extent() );
  QCOMPARE( mMdalLayer->dataProvider()->extent(), mMdalLayer->extent() );
}

void TestQgsMeshLayer::test_write_read_project()
{
  QgsProject prj;
  prj.addMapLayer( mMemoryLayer->clone() );
  prj.addMapLayer( mMdalLayer->clone() );

  QTemporaryFile f;
  QVERIFY( f.open() );
  f.close();
  prj.setFileName( f.fileName() );
  prj.write();

  QgsProject prj2;
  prj2.setFileName( f.fileName() );
  QVERIFY( prj2.read() );
  QVector<QgsMapLayer *> layers = prj2.layers<QgsMapLayer *>();
  QVERIFY( layers.size() == 2 );
}

void TestQgsMeshLayer::test_reload()
{
  //init file for the test
  QString uri1( mDataDir + "/quad_and_triangle.2dm" );
  QFile fileSource1( uri1 );

  QString uri2( mDataDir + "/quad_flower.2dm" );
  QFile fileSource2( uri2 );

  QTemporaryFile testFile;

  auto copyToTemporaryFile = []( QFile & fileTocopy, QTemporaryFile & tempFile )
  {
    QDataStream streamToCopy( &fileTocopy );
    QDataStream streamTemporaryFile( &tempFile );
    tempFile.open();
    fileTocopy.open( QIODevice::ReadOnly );

    while ( !streamToCopy.atEnd() )
    {
      char *rd = new char[1];
      int len = streamToCopy.readRawData( rd, 1 );
      streamTemporaryFile.writeRawData( rd, len );
    }
    fileTocopy.close();
    tempFile.close();
  };

  //copy the quad_and_triangle.2dm to the temporary testFile
  copyToTemporaryFile( fileSource1, testFile );

  //create layer with temporary file
  QgsMeshLayer layer( testFile.fileName(), "Test", "mdal" );
  layer.updateTriangularMesh(); //to active the lazy loading of mesh data

  //Test if the layer matches with quad and triangle
  QCOMPARE( layer.datasetGroupCount(), 1 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 1 );
  QCOMPARE( 5, layer.nativeMesh()->vertexCount() );
  QCOMPARE( 2, layer.nativeMesh()->faceCount() );

  //Test dataSet in quad and triangle
  QgsMeshDatasetIndex ds( 0, 0 );
  QCOMPARE( QgsMeshDatasetValue( 20 ), layer.datasetValue( ds, 0 ) );
  QCOMPARE( QgsMeshDatasetValue( 30 ), layer.datasetValue( ds, 1 ) );
  QCOMPARE( QgsMeshDatasetValue( 40 ), layer.datasetValue( ds, 2 ) );
  QCOMPARE( QgsMeshDatasetValue( 50 ), layer.datasetValue( ds, 3 ) );
  QCOMPARE( QgsMeshDatasetValue( 10 ), layer.datasetValue( ds, 4 ) );

  //copy the quad_flower.2dm to the temporary testFile
  copyToTemporaryFile( fileSource2, testFile );

  //reload the layer
  layer.reload();

  //Test if the layer matches with quad flower
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 1 );
  QCOMPARE( 8, layer.nativeMesh()->vertexCount() );
  QCOMPARE( 5, layer.nativeMesh()->faceCount() );

  //Test dataSet in quad flower
  QCOMPARE( QgsMeshDatasetValue( 200 ), layer.dataProvider()->datasetValue( ds, 0 ) );
  QCOMPARE( QgsMeshDatasetValue( 200 ), layer.dataProvider()->datasetValue( ds, 1 ) );
  QCOMPARE( QgsMeshDatasetValue( 800 ), layer.dataProvider()->datasetValue( ds, 2 ) );
  QCOMPARE( QgsMeshDatasetValue( 200 ), layer.dataProvider()->datasetValue( ds, 3 ) );
  QCOMPARE( QgsMeshDatasetValue( 200 ), layer.dataProvider()->datasetValue( ds, 4 ) );
  QCOMPARE( QgsMeshDatasetValue( 800 ), layer.dataProvider()->datasetValue( ds, 5 ) );
  QCOMPARE( QgsMeshDatasetValue( 800 ), layer.dataProvider()->datasetValue( ds, 6 ) );
  QCOMPARE( QgsMeshDatasetValue( 800 ), layer.dataProvider()->datasetValue( ds, 7 ) );
}

void TestQgsMeshLayer::test_reload_extra_dataset()
{
  //init files for the test
  QgsMeshLayer layer( mDataDir + "/quad_and_triangle.2dm", "MDAL layer", "mdal" );

  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 0 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 1 );

  QString datasetUri_1( mDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  QFile dataSetFile_1( datasetUri_1 );


  QString datasetUri_2( mDataDir + "/quad_and_triangle_vertex_scalar_incompatible_mesh.dat" );
  QFile dataSetFile_2( datasetUri_2 );

  QString datasetUri_3( mDataDir + "/quad_and_triangle_vertex_vector.dat" );
  QFile dataSetFile_3( datasetUri_3 );

  QTemporaryFile testFileDataSet;

  auto copyToTemporaryFile = []( QFile & fileTocopy, QTemporaryFile & tempFile )
  {
    QDataStream streamToCopy( &fileTocopy );
    QDataStream streamTemporaryFile( &tempFile );
    tempFile.open();
    fileTocopy.open( QIODevice::ReadOnly );

    while ( !streamToCopy.atEnd() )
    {
      char *rd = new char[1];
      int len = streamToCopy.readRawData( rd, 1 );
      streamTemporaryFile.writeRawData( rd, len );
    }
    fileTocopy.close();
    tempFile.close();
  };

  //copy the qad_and_triangle_vertex_scalar.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_1, testFileDataSet );

  //add the data set from temporary file and test
  QVERIFY( layer.addDatasets( testFileDataSet.fileName() ) );
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 2 );

  //copy the qad_and_triangle_vertex_scalar_incompatible_mesh.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_2, testFileDataSet );

  layer.reload();

  //test if dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 ); //uri still present
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 ); //dataset not loaded in the provider
  QCOMPARE( layer.datasetGroupCount(), 2 ); //dataset group still registered in the layer
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 2 ); //dataset group tree item still have all dataset group

  //copy again the qad_and_triangle_vertex_scalar.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_1, testFileDataSet );

  layer.reload();

  //add the data set from temporary tesFile and test
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 2 );

  //copy a invalid file to the temporary testFile
  QVERIFY( testFileDataSet.open() );
  QDataStream streamTestFile( &testFileDataSet );
  streamTestFile.writeBytes( "x", 1 );
  testFileDataSet.close();

  layer.reload();

  //test dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );
  QCOMPARE( layer.datasetGroupCount(), 2 ); //dataset group still registered in the layer
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 2 ); //dataset group tree item still have all dataset groups

  //copy again the qad_and_triangle_vertex_scalar.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_1, testFileDataSet );

  layer.reload();

  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupCount(), 2 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 2 );

  // Add twice the same file
  QVERIFY( layer.addDatasets( testFileDataSet.fileName() ) ); //dataset added
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 ); //uri not dupplicated
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 3 ); //dataset added
  QCOMPARE( layer.datasetGroupCount(), 3 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 3 );

  //test dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 3 );
  QCOMPARE( layer.datasetGroupCount(), 3 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 3 );

  //add another dataset
  QTemporaryFile testFileDataSet_3;
  copyToTemporaryFile( dataSetFile_3, testFileDataSet_3 );

  QVERIFY( layer.addDatasets( testFileDataSet_3.fileName() ) );
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 4 );
  QCOMPARE( layer.datasetGroupCount(), 4 );
  QCOMPARE( layer.datasetGroupTreeRootItem()->childCount(), 4 );

  //Test dataSet
  QgsMeshDatasetIndex ds( 3, 0 );
  QCOMPARE( QgsMeshDatasetValue( 1, 1 ), layer.datasetValue( ds, 0 ) );
  QCOMPARE( QgsMeshDatasetValue( 2, 1 ), layer.datasetValue( ds, 1 ) );
  QCOMPARE( QgsMeshDatasetValue( 3, 2 ), layer.datasetValue( ds, 2 ) );
  QCOMPARE( QgsMeshDatasetValue( 2, 2 ), layer.datasetValue( ds, 3 ) );
  QCOMPARE( QgsMeshDatasetValue( 1, -2 ), layer.datasetValue( ds, 4 ) );
}

void TestQgsMeshLayer::test_mesh_simplification()
{
  QgsCoordinateTransform invalidTransform;
  mMdal3DLayer->updateTriangularMesh( invalidTransform );
  QgsTriangularMesh *baseMesh = mMdal3DLayer->triangularMesh();

  QCOMPARE( baseMesh->triangles().count(), 640 );

  // Simplify with reduction factor lesser than 1
  QVector<QgsTriangularMesh *> simplifiedMeshes = baseMesh->simplifyMesh( 0.5, 1 );
  QCOMPARE( simplifiedMeshes.count(), 0 );

  // Simplify with reduction factor equal to 2
  simplifiedMeshes = baseMesh->simplifyMesh( 2, 1 );
  QCOMPARE( simplifiedMeshes.count(), 5 );
  QCOMPARE( simplifiedMeshes.at( 0 )->triangles().count(), 320 );
  QCOMPARE( simplifiedMeshes.at( 1 )->triangles().count(), 112 );
  QCOMPARE( simplifiedMeshes.at( 2 )->triangles().count(), 80 );
  QCOMPARE( simplifiedMeshes.at( 3 )->triangles().count(), 32 );
  QCOMPARE( simplifiedMeshes.at( 4 )->triangles().count(), 5 );

  // Delete simplified meshes
  for ( QgsTriangularMesh *m : simplifiedMeshes )
    delete m;
}

void TestQgsMeshLayer::test_snap_on_mesh()
{
  //1D mesh
  mMdal1DLayer->updateTriangularMesh();
  double searchRadius = 10;

  QgsPointXY snappedPoint;

  //1D mesh
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY(), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY() );
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY( 1002, 2005 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1000, 2000 ) );
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Edge, QgsPointXY( 1002, 2005 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1002, 2000 ) );
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Edge, QgsPointXY( 998, 2005 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1000, 2000 ) );
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Edge, QgsPointXY( 990, 2010 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY() );
  snappedPoint = mMdal1DLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY( 2002, 2998 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 2000, 3000 ) );


  //2D mesh
  mMdalLayer->updateTriangularMesh();
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY(), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY() );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY(), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY() );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY( 1002, 2005 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1000, 2000 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Vertex, QgsPointXY( 2002, 2998 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 2000, 3000 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Face, QgsPointXY( 998, 1998 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1500, 2500 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Face, QgsPointXY( 1002, 2001 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1500, 2500 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Face, QgsPointXY( 1998, 2998 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 1500, 2500 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Face, QgsPointXY( 2002, 1998 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY( 2333.33333333, 2333.333333333 ) );
  snappedPoint = mMdalLayer->snapOnElement( QgsMesh::Face, QgsPointXY( 500, 500 ), searchRadius );
  QCOMPARE( snappedPoint, QgsPointXY() );
}

void TestQgsMeshLayer::test_dataset_value_from_layer()
{
  QgsMeshDatasetValue value;

  //1D mesh
  mMdal1DLayer->updateTriangularMesh();
  value = mMdal1DLayer->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 1500, 2009 ), 10 );
  QCOMPARE( QgsMeshDatasetValue( 25 ), value );
  value = mMdal1DLayer->datasetValue( QgsMeshDatasetIndex( 1, 0 ), QgsPointXY( 2500, 1991 ), 10 );
  QCOMPARE( QgsMeshDatasetValue( 2.5 ), value );
  value = mMdal1DLayer->datasetValue( QgsMeshDatasetIndex( 2, 0 ), QgsPointXY( 2500, 2500 ), 10 );
  QCOMPARE( QgsMeshDatasetValue( 2.5, 2 ), value );
  value = mMdal1DLayer->datasetValue( QgsMeshDatasetIndex( 3, 1 ), QgsPointXY( 2495, 2000 ), 10 );
  QCOMPARE( QgsMeshDatasetValue( 3 ), value );
  value = mMdal1DLayer->datasetValue( QgsMeshDatasetIndex( 4, 1 ), QgsPointXY( 2500, 2495 ), 10 );
  QCOMPARE( QgsMeshDatasetValue( 4, 4 ), value );

  //2D mesh
  mMdalLayer->updateTriangularMesh();
  value = mMdalLayer->datasetValue( QgsMeshDatasetIndex( 0, 0 ), QgsPointXY( 1750, 2250 ) );
  QCOMPARE( QgsMeshDatasetValue( 32.5 ), value );
  value = mMdalLayer->datasetValue( QgsMeshDatasetIndex( 1, 0 ), QgsPointXY( 1750, 2250 ) );
  QCOMPARE( QgsMeshDatasetValue( 1.75 ), value );
  value = mMdalLayer->datasetValue( QgsMeshDatasetIndex( 2, 0 ), QgsPointXY( 1750, 2250 ) );
  QCOMPARE( QgsMeshDatasetValue( 1.75, 1.25 ), value );
  value = mMdalLayer->datasetValue( QgsMeshDatasetIndex( 3, 1 ), QgsPointXY( 1750, 2250 ) );
  QCOMPARE( QgsMeshDatasetValue( 2 ), value );
  value = mMdalLayer->datasetValue( QgsMeshDatasetIndex( 4, 1 ), QgsPointXY( 1750, 2250 ) );
  QCOMPARE( QgsMeshDatasetValue( 2, 2 ), value );

}

void TestQgsMeshLayer::test_dataset_group_item_tree_item()
{
  QgsMeshDatasetGroupTreeItem *rootItem = mMdal3DLayer->datasetGroupTreeRootItem();

  QCOMPARE( rootItem->childCount(), 5 );
  QCOMPARE( rootItem->totalChildCount(), 21 );
  for ( int i = 0; i < rootItem->totalChildCount(); ++i )
    QVERIFY( rootItem->childFromDatasetGroupIndex( i )->isEnabled() );

  QStringList names;
  names << "Bed Elevation" <<
        "temperature" << "Maximums" << "Minimums" << "Time at Maximums" << "Time at Minimums" <<
        "velocity" << "Maximums" << "Minimums" << "Time at Maximums" << "Time at Minimums" <<
        "water depth" << "Maximums" << "Minimums" << "Time at Maximums" << "Time at Minimums" <<
        "water surface elevation" << "Maximums" << "Minimums" << "Time at Maximums" << "Time at Minimums";

  for ( int i = 0; i < rootItem->totalChildCount(); ++i )
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->name(), names.at( i ) );

  QCOMPARE( rootItem->child( 0 )->child( 0 ), nullptr );

  rootItem->child( 0 )->appendChild( new QgsMeshDatasetGroupTreeItem( "added item", QString(), true, 21 ) );
  names << "added item";
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 21 ), rootItem->child( 0 )->child( 0 ) );


  QCOMPARE( rootItem->childFromDatasetGroupIndex( 1 ), rootItem->child( 1 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 2 ), rootItem->child( 1 )->child( 0 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 3 ), rootItem->child( 1 )->child( 1 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 4 ), rootItem->child( 1 )->child( 2 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 5 ), rootItem->child( 1 )->child( 3 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 6 ), rootItem->child( 2 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 7 ), rootItem->child( 2 )->child( 0 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 8 ), rootItem->child( 2 )->child( 1 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 9 ), rootItem->child( 2 )->child( 2 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 10 ), rootItem->child( 2 )->child( 3 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 11 ), rootItem->child( 3 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 12 ), rootItem->child( 3 )->child( 0 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 13 ), rootItem->child( 3 )->child( 1 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 14 ), rootItem->child( 3 )->child( 2 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 15 ), rootItem->child( 3 )->child( 3 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 16 ), rootItem->child( 4 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 17 ), rootItem->child( 4 )->child( 0 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 18 ), rootItem->child( 4 )->child( 1 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 19 ), rootItem->child( 4 )->child( 2 ) );
  QCOMPARE( rootItem->childFromDatasetGroupIndex( 20 ), rootItem->child( 4 )->child( 3 ) );

  //Rename some items
  names.replace( 7, "Other name 1" );
  names.replace( 12, "Other name 2" );
  names.replace( 18, "Other name 3" );
  rootItem->childFromDatasetGroupIndex( 7 )->setName( "Other name 1" );
  rootItem->childFromDatasetGroupIndex( 12 )->setName( "Other name 2" );
  rootItem->childFromDatasetGroupIndex( 18 )->setName( "Other name 3" );
  for ( int i = 0; i < rootItem->totalChildCount(); ++i )
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->name(), names.at( i ) );

  // Disable some items
  rootItem->childFromDatasetGroupIndex( 7 )->setIsEnabled( false );
  rootItem->childFromDatasetGroupIndex( 10 )->setIsEnabled( false );
  rootItem->childFromDatasetGroupIndex( 15 )->setIsEnabled( false );

  QDomDocument doc;
  QgsReadWriteContext context;
  QDomElement rootElement = rootItem->writeXml( doc, context );

  std::unique_ptr<QgsMeshDatasetGroupTreeItem> otherRoot( new QgsMeshDatasetGroupTreeItem( rootElement, context ) );

  for ( int i = 0; i < rootItem->totalChildCount(); ++i )
    QCOMPARE( otherRoot->childFromDatasetGroupIndex( i )->name(), names.at( i ) );

  for ( int i = 0; i < 21; ++i )
  {
    QVERIFY( otherRoot->childFromDatasetGroupIndex( i ) );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->row(), otherRoot->childFromDatasetGroupIndex( i )->row() );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->name(), otherRoot->childFromDatasetGroupIndex( i )->name() );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->isVector(), otherRoot->childFromDatasetGroupIndex( i )->isVector() );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->isEnabled(), otherRoot->childFromDatasetGroupIndex( i )->isEnabled() );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->childCount(), otherRoot->childFromDatasetGroupIndex( i )->childCount() );
    QCOMPARE( rootItem->childFromDatasetGroupIndex( i )->totalChildCount(), otherRoot->childFromDatasetGroupIndex( i )->totalChildCount() );
  }

  QVERIFY( !otherRoot->childFromDatasetGroupIndex( 7 )->isEnabled() );
  QVERIFY( !otherRoot->childFromDatasetGroupIndex( 10 )->isEnabled() );
  QVERIFY( !otherRoot->childFromDatasetGroupIndex( 15 )->isEnabled() );

  QCOMPARE( otherRoot->child( 0 )->child( 0 ), otherRoot->child( 0 )->child( 0 ) );

  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 1 ), otherRoot->child( 1 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 2 ), otherRoot->child( 1 )->child( 0 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 3 ), otherRoot->child( 1 )->child( 1 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 4 ), otherRoot->child( 1 )->child( 2 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 5 ), otherRoot->child( 1 )->child( 3 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 6 ), otherRoot->child( 2 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 7 ), otherRoot->child( 2 )->child( 0 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 8 ), otherRoot->child( 2 )->child( 1 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 9 ), otherRoot->child( 2 )->child( 2 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 10 ), otherRoot->child( 2 )->child( 3 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 11 ), otherRoot->child( 3 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 12 ), otherRoot->child( 3 )->child( 0 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 13 ), otherRoot->child( 3 )->child( 1 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 14 ), otherRoot->child( 3 )->child( 2 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 15 ), otherRoot->child( 3 )->child( 3 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 16 ), otherRoot->child( 4 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 17 ), otherRoot->child( 4 )->child( 0 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 18 ), otherRoot->child( 4 )->child( 1 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 19 ), otherRoot->child( 4 )->child( 2 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 20 ), otherRoot->child( 4 )->child( 3 ) );
  QCOMPARE( otherRoot->childFromDatasetGroupIndex( 21 ), otherRoot->child( 0 )->child( 0 ) );
}

void TestQgsMeshLayer::test_memory_dataset_group()
{
  int vertexCount = mMdalLayer->dataProvider()->vertexCount();
  int faceCount = mMdalLayer->dataProvider()->faceCount();

  QCOMPARE( mMdalLayer->datasetGroupCount(), 5 );

  std::unique_ptr<QgsMeshMemoryDatasetGroup> goodDatasetGroupVertices( new QgsMeshMemoryDatasetGroup );
  std::unique_ptr<QgsMeshMemoryDatasetGroup>  badDatasetGroupVertices( new QgsMeshMemoryDatasetGroup );
  goodDatasetGroupVertices->setName( QStringLiteral( "good vertices datasets" ) );
  goodDatasetGroupVertices->setDataType( QgsMeshDatasetGroupMetadata::DataOnVertices );
  badDatasetGroupVertices->setDataType( QgsMeshDatasetGroupMetadata::DataOnVertices );
  for ( int i = 1; i < 10; i++ )
  {
    std::shared_ptr<QgsMeshMemoryDataset> gds = std::make_shared<QgsMeshMemoryDataset>();
    std::shared_ptr<QgsMeshMemoryDataset> bds = std::make_shared<QgsMeshMemoryDataset>();
    for ( int v = 0; v < vertexCount; ++v )
      gds->values.append( QgsMeshDatasetValue( v / 2.0 ) );
    bds->values.append( QgsMeshDatasetValue( 2.0 ) );
    gds->valid = true;
    bds->valid = true;
    goodDatasetGroupVertices->addDataset( gds );
    badDatasetGroupVertices->addDataset( bds );
  }

  QVERIFY( mMdalLayer->addDatasets( goodDatasetGroupVertices.release() ) );
  QVERIFY( !mMdalLayer->addDatasets( badDatasetGroupVertices.release() ) );

  QCOMPARE( mMdalLayer->datasetGroupCount(), 6 );
  QCOMPARE( mMdalLayer->extraDatasetGroupCount(), 1 );

  std::unique_ptr<QgsMeshMemoryDatasetGroup> goodDatasetGroupFaces(
    new QgsMeshMemoryDatasetGroup( QStringLiteral( "good faces datasets" ), QgsMeshDatasetGroupMetadata::DataOnFaces ) );
  std::unique_ptr<QgsMeshMemoryDatasetGroup> badDatasetGroupFaces( new QgsMeshMemoryDatasetGroup );
  badDatasetGroupFaces->setDataType( QgsMeshDatasetGroupMetadata::DataOnFaces );
  for ( int i = 1; i < 10; i++ )
  {
    std::shared_ptr<QgsMeshMemoryDataset> gds = std::make_shared<QgsMeshMemoryDataset>();
    std::shared_ptr<QgsMeshMemoryDataset> bds = std::make_shared<QgsMeshMemoryDataset>();
    for ( int v = 0; v < faceCount; ++v )
      gds->values.append( QgsMeshDatasetValue( v / 2.0 ) );
    bds->values.append( QgsMeshDatasetValue( 2.0 ) );
    gds->valid = true;
    bds->valid = true;
    goodDatasetGroupFaces->addDataset( gds );
    badDatasetGroupFaces->addDataset( bds );
  }

  QVERIFY( mMdalLayer->addDatasets( goodDatasetGroupFaces.release() ) );
  QVERIFY( !mMdalLayer->addDatasets( badDatasetGroupFaces.release() ) );

  QCOMPARE( mMdalLayer->datasetGroupCount(), 7 );
  QCOMPARE( mMdalLayer->extraDatasetGroupCount(), 2 );

  QCOMPARE( mMdalLayer->datasetGroupMetadata( 5 ).name(), QStringLiteral( "good vertices datasets" ) );
  QCOMPARE( mMdalLayer->datasetGroupMetadata( 6 ).name(), QStringLiteral( "good faces datasets" ) );

  QTemporaryFile file;
  QVERIFY( file.open() );
  file.close();

  QVERIFY( !mMdalLayer->saveDataset( file.fileName(), 6, "ASCII_DAT" ) );
  QCOMPARE( mMdalLayer->datasetGroupCount(), 7 );
  QCOMPARE( mMdalLayer->extraDatasetGroupCount(), 1 );

  QCOMPARE( mMdalLayer->datasetGroupMetadata( 5 ).name(), QStringLiteral( "good vertices datasets" ) );
  QCOMPARE( mMdalLayer->datasetGroupMetadata( 6 ).name(), QStringLiteral( "good faces datasets" ) );

}

void TestQgsMeshLayer::test_memory_dataset_group_1d()
{
  int vertexCount = mMdal1DLayer->dataProvider()->vertexCount();
  int edgeCount = mMdal1DLayer->dataProvider()->edgeCount();

  QCOMPARE( mMdal1DLayer->datasetGroupCount(), 5 );

  std::unique_ptr<QgsMeshMemoryDatasetGroup> goodDatasetGroupVertices( new QgsMeshMemoryDatasetGroup );
  std::unique_ptr<QgsMeshMemoryDatasetGroup>  badDatasetGroupVertices( new QgsMeshMemoryDatasetGroup );
  goodDatasetGroupVertices->setDataType( QgsMeshDatasetGroupMetadata::DataOnVertices );
  badDatasetGroupVertices->setDataType( QgsMeshDatasetGroupMetadata::DataOnVertices );
  for ( int i = 1; i < 10; i++ )
  {
    std::shared_ptr<QgsMeshMemoryDataset> gds = std::make_shared<QgsMeshMemoryDataset>();
    std::shared_ptr<QgsMeshMemoryDataset> bds = std::make_shared<QgsMeshMemoryDataset>();
    for ( int v = 0; v < vertexCount; ++v )
      gds->values.append( QgsMeshDatasetValue( v / 2.0 ) );
    bds->values.append( QgsMeshDatasetValue( 2.0 ) );
    goodDatasetGroupVertices->addDataset( gds );
    badDatasetGroupVertices->addDataset( bds );
  }

  QVERIFY( mMdal1DLayer->addDatasets( goodDatasetGroupVertices.release() ) );
  QVERIFY( !mMdal1DLayer->addDatasets( badDatasetGroupVertices.release() ) );

  QCOMPARE( mMdal1DLayer->datasetGroupCount(), 6 );

  std::unique_ptr<QgsMeshMemoryDatasetGroup> goodDatasetGroupEdges( new QgsMeshMemoryDatasetGroup );
  std::unique_ptr<QgsMeshMemoryDatasetGroup>  badDatasetGroupEdges( new QgsMeshMemoryDatasetGroup );
  goodDatasetGroupEdges->setDataType( QgsMeshDatasetGroupMetadata::DataOnEdges );
  badDatasetGroupEdges->setDataType( QgsMeshDatasetGroupMetadata::DataOnEdges );
  for ( int i = 1; i < 10; i++ )
  {
    std::shared_ptr<QgsMeshMemoryDataset> gds = std::make_shared<QgsMeshMemoryDataset>();
    std::shared_ptr<QgsMeshMemoryDataset> bds = std::make_shared<QgsMeshMemoryDataset>();
    for ( int v = 0; v < edgeCount; ++v )
      gds->values.append( QgsMeshDatasetValue( v / 2.0 ) );
    bds->values.append( QgsMeshDatasetValue( 2.0 ) );
    goodDatasetGroupEdges->addDataset( gds );
    badDatasetGroupEdges->addDataset( bds );
  }

  QVERIFY( mMdal1DLayer->addDatasets( goodDatasetGroupEdges.release() ) );
  QVERIFY( !mMdal1DLayer->addDatasets( badDatasetGroupEdges.release() ) );

  QCOMPARE( mMdal1DLayer->datasetGroupCount(), 7 );
  QCOMPARE( mMdal1DLayer->extraDatasetGroupCount(), 2 );
}

void TestQgsMeshLayer::test_temporal()
{
  qint64 relativeTime_0 = -1000;
  qint64 relativeTime_1 = 0;
  qint64 relativeTime_2 = 1000 * 60 * 60 * 0.6;
  qint64 relativeTime_3 = 1000 * 60 * 60 * 1;
  qint64 relativeTime_4 = 1000 * 60 * 60 * 2;
  // Mesh memory provider
  QgsMeshDataProviderTemporalCapabilities *tempCap = mMemoryLayer->dataProvider()->temporalCapabilities();
  // Static dataset
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_0 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_3 ).dataset(), 0 );
  // Temporal dataset
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_0 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_3 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_4 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_0 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_2 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_3 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_4 ).dataset(), -1 );


  // Mesh MDAL provider with internal dataset
  tempCap = mMdalLayer->dataProvider()->temporalCapabilities();
  // Static dataset
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_0 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_2 ).dataset(), 0 );
  // Temporal dataset
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_0 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_3 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 1, relativeTime_4 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_0 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_2 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_3 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexClosestFromRelativeTime( 1, relativeTime_4 ).dataset(), -1 );

  //Mesh MDAL provider with reference time
  tempCap = mMdal3DLayer->dataProvider()->temporalCapabilities();
  QCOMPARE( tempCap->datasetIndexClosestBeforeRelativeTime( 0, relativeTime_0 ).dataset(), 0 );
  QDateTime begin = QDateTime( QDate( 1990, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC );
  QDateTime end = QDateTime( QDate( 1990, 1, 1 ), QTime( 6, 0, 1, 938 ), Qt::UTC );
  QCOMPARE( tempCap->timeExtent(), QgsDateTimeRange( begin, end ) );

  QDateTime time_1 = QDateTime( QDate( 1990, 1, 1 ), QTime( 3, 0, 0 ), Qt::UTC );
  QDateTime time_2 = time_1.addSecs( 300 );
  QgsMeshRendererSettings settings = mMdal3DLayer->rendererSettings();
  // Static dataset (bed elevation)
  settings.setActiveScalarDatasetGroup( 0 ); //static dataset (bed elevation)
  mMdal3DLayer->setRendererSettings( settings );
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1, time_2 ) ).dataset(), 0 );
  // Attempt to next dataset
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1.addSecs( 400 ), time_2.addSecs( 400 ) ) ).dataset(), 0 );

  // Temporal dataset
  settings.setActiveScalarDatasetGroup( 1 );
  mMdal3DLayer->setRendererSettings( settings );
  mMdal3DLayer->setTemporalMatchingMethod( QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime );
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1, time_2 ) ).dataset(), 17 );
  mMdal3DLayer->setTemporalMatchingMethod( QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetFromStartRangeTime );
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1, time_2 ) ).dataset(), 18 );
  // Next dataset
  mMdal3DLayer->setTemporalMatchingMethod( QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetBeforeStartRangeTime );
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1.addSecs( 400 ), time_2.addSecs( 400 ) ) ).dataset(), 18 );
  mMdal3DLayer->setTemporalMatchingMethod( QgsMeshDataProviderTemporalCapabilities::FindClosestDatasetFromStartRangeTime );
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1.addSecs( 400 ), time_2.addSecs( 400 ) ) ).dataset(), 19 );
  mMdal3DLayer->temporalProperties();
}

QGSTEST_MAIN( TestQgsMeshLayer )
#include "testqgsmeshlayer.moc"
