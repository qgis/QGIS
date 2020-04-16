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
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_bed_elevation.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QCOMPARE( mMemoryLayer->dataProvider()->extraDatasets().count(), 5 );
  QVERIFY( mMemoryLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMemoryLayer->temporalProperties()->isActive() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );

  // MDAL Layer
  QString uri( mDataDir + "/quad_and_triangle.2dm" );
  mMdalLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  QCOMPARE( mMdalLayer->dataProvider()->datasetGroupCount(), 1 ); //bed elevation is already in the 2dm
  QVERIFY( !mMdalLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_vector.dat" );
  QCOMPARE( mMdalLayer->dataProvider()->extraDatasets().count(), 2 );
  QVERIFY( mMdalLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );

  //The face dataset is recognized by "_els_" in the filename for this format
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_vector.dat" );

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
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_bed_elevation.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_vertex_vector.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_scalar.txt" ) );
  mMemory1DLayer->dataProvider()->addDataset( readFile( "/lines_els_vector.txt" ) );
  QCOMPARE( mMemory1DLayer->dataProvider()->extraDatasets().count(), 5 );
  QVERIFY( mMemory1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMemory1DLayer->temporalProperties()->isActive() );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemory1DLayer );

  // MDAL 1D Layer
  uri = QString( mDataDir + "/lines.2dm" );
  mMdal1DLayer = new QgsMeshLayer( uri, "Lines MDAL", "mdal" );
  QCOMPARE( mMdal1DLayer->dataProvider()->datasetGroupCount(), 1 ); //bed elevation is already in the 2dm
  QVERIFY( !mMdal1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( !mMdal1DLayer->temporalProperties()->isActive() );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_vertex_scalar.dat" );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_vertex_vector.dat" );
  QCOMPARE( mMdal1DLayer->dataProvider()->extraDatasets().count(), 2 );
  QVERIFY( mMdal1DLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() );
  QVERIFY( mMdal1DLayer->temporalProperties()->isActive() );


  //The face dataset is recognized by "_els_" in the filename for this format
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_els_scalar.dat" );
  mMdal1DLayer->dataProvider()->addDataset( mDataDir + "/lines_els_vector.dat" );

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
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    QgsMeshDatasetIndex ds( 0, 0 );

    const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    // We have 4 values, since dp->vertexCount() = 4
    QCOMPARE( QgsMeshDatasetValue( 20 ), dp->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), dp->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), dp->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), dp->datasetValue( ds, 3 ) );
  }
}

void TestQgsMeshLayer::test_read_1d_vertex_scalar_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 1, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 4 values, since dp->vertexCount() = 4
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 4.0 + i ), dp->datasetValue( ds, 3 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_1d_vertex_vector_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 2, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 4 values, since dp->vertexCount() = 4
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 3 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_1d_edge_scalar_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 3, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 3 values, since dp->edgeCount() = 3
      QCOMPARE( QgsMeshDatasetValue( 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i ), dp->datasetValue( ds, 2 ) );
    }
  }
}


void TestQgsMeshLayer::test_read_1d_edge_vector_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemory1DLayer->dataProvider() );
  dataProviders.append( mMdal1DLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 4, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Edge Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "EdgeVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnEdges );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 3 values, since dp->edgeCount() = 3
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 3 + i ), dp->datasetValue( ds, 2 ) );
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
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    QgsMeshDatasetIndex ds( 0, 0 );

    const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QVERIFY( meta.name().contains( "Elevation" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
    QVERIFY( qgsDoubleNear( dmeta.time(), 0 ) );
    QVERIFY( dmeta.isValid() );

    // We have 5 values, since dp->vertexCount() = 5
    QCOMPARE( QgsMeshDatasetValue( 20 ), dp->datasetValue( ds, 0 ) );
    QCOMPARE( QgsMeshDatasetValue( 30 ), dp->datasetValue( ds, 1 ) );
    QCOMPARE( QgsMeshDatasetValue( 40 ), dp->datasetValue( ds, 2 ) );
    QCOMPARE( QgsMeshDatasetValue( 50 ), dp->datasetValue( ds, 3 ) );
    QCOMPARE( QgsMeshDatasetValue( 10 ), dp->datasetValue( ds, 4 ) );

    QVERIFY( dp->isFaceActive( ds, 0 ) );
  }
}

void TestQgsMeshLayer::test_read_vertex_scalar_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 1, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
      {
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Scalar Dataset" ) );
        QCOMPARE( meta.extraOptions()["meta2"], QString( "best dataset" ) );
      }
      QCOMPARE( meta.name(), QString( "VertexScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3.0 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2.0 + i ), dp->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1.0 + i ), dp->datasetValue( ds, 4 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_vertex_vector_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 2, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Vertex Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "VertexVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 5 values, since dp->vertexCount() = 5
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 1 + i ), dp->datasetValue( ds, 1 ) );
      QCOMPARE( QgsMeshDatasetValue( 3 + i, 2 + i ), dp->datasetValue( ds, 2 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 3 ) );
      QCOMPARE( QgsMeshDatasetValue( 1 + i, -2 + i ), dp->datasetValue( ds, 4 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );
    }
  }
}

void TestQgsMeshLayer::test_read_face_scalar_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 3, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Scalar Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceScalarDataset" ) );
      QVERIFY( meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i ), dp->datasetValue( ds, 1 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );
    }
  }
}


void TestQgsMeshLayer::test_read_face_vector_dataset()
{
  QList<const QgsMeshDataProvider *> dataProviders;
  dataProviders.append( mMemoryLayer->dataProvider() );
  dataProviders.append( mMdalLayer->dataProvider() );

  foreach ( auto dp, dataProviders )
  {
    QVERIFY( dp != nullptr );
    QVERIFY( dp->isValid() );

    QCOMPARE( 5, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 4, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( dp->name() == QStringLiteral( "mesh_memory" ) )
        QCOMPARE( meta.extraOptions()["description"], QString( "Face Vector Dataset" ) );
      QCOMPARE( meta.name(), QString( "FaceVectorDataset" ) );
      QVERIFY( !meta.isScalar() );
      QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnFaces );

      const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
      QVERIFY( qgsDoubleNear( dmeta.time(), i ) );
      QVERIFY( dmeta.isValid() );

      // We have 2 values, since dp->faceCount() = 2
      QCOMPARE( QgsMeshDatasetValue( 1 + i, 1 + i ), dp->datasetValue( ds, 0 ) );
      QCOMPARE( QgsMeshDatasetValue( 2 + i, 2 + i ), dp->datasetValue( ds, 1 ) );

      QVERIFY( dp->isFaceActive( ds, 0 ) );
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

  for ( int i = 0; i < 2 ; ++i )
  {
    QgsMeshDatasetIndex ds( 1, i );

    const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
    QCOMPARE( meta.name(), QString( "VertexScalarDatasetWithInactiveFace1" ) );
    QVERIFY( meta.isScalar() );
    QVERIFY( meta.dataType() == QgsMeshDatasetGroupMetadata::DataOnVertices );

    const QgsMeshDatasetMetadata dmeta = dp->datasetMetadata( ds );
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
  QgsRenderContext rendererContext;
  layer.createMapRenderer( rendererContext ); //to active the lazy loading of mesh data

  //Test if the layer matches with quad and triangle
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );
  QCOMPARE( 5, layer.nativeMesh()->vertexCount() );
  QCOMPARE( 2, layer.nativeMesh()->faceCount() );

  //Test dataSet in quad and triangle
  QgsMeshDatasetIndex ds( 0, 0 );
  QCOMPARE( QgsMeshDatasetValue( 20 ), layer.dataProvider()->datasetValue( ds, 0 ) );
  QCOMPARE( QgsMeshDatasetValue( 30 ), layer.dataProvider()->datasetValue( ds, 1 ) );
  QCOMPARE( QgsMeshDatasetValue( 40 ), layer.dataProvider()->datasetValue( ds, 2 ) );
  QCOMPARE( QgsMeshDatasetValue( 50 ), layer.dataProvider()->datasetValue( ds, 3 ) );
  QCOMPARE( QgsMeshDatasetValue( 10 ), layer.dataProvider()->datasetValue( ds, 4 ) );

  //copy the quad_flower.2dm to the temporary testFile
  copyToTemporaryFile( fileSource2, testFile );

  //reload the layer
  layer.reload();

  //Test if the layer matches with quad flower
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );
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
  QVERIFY( layer.dataProvider()->addDataset( testFileDataSet.fileName() ) );
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 2 );

  //copy the qad_and_triangle_vertex_scalar_incompatible_mesh.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_2, testFileDataSet );

  layer.reload();

  //test if dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 1 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );

  //copy again the qad_and_triangle_vertex_scalar.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_1, testFileDataSet );

  layer.reload();

  //add the data set from temporary tesFile and test
  QVERIFY( layer.dataProvider()->addDataset( testFileDataSet.fileName() ) );
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 3 );

  //copy a invalid file to the temporary testFile
  QVERIFY( testFileDataSet.open() );
  QDataStream streamTestFile( &testFileDataSet );
  streamTestFile.writeBytes( "x", 1 );
  testFileDataSet.close();

  layer.reload();

  //test dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 1 );

  //copy again the qad_and_triangle_vertex_scalar.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_1, testFileDataSet );

  layer.reload();

  //test dataset presence
  QCOMPARE( layer.dataProvider()->extraDatasets().count(), 2 );
  QCOMPARE( layer.dataProvider()->datasetGroupCount(), 3 );

  //copy the qad_and_triangle_vertex_vector.dat to the temporary testFile
  copyToTemporaryFile( dataSetFile_3, testFileDataSet );

  layer.reload();

  //Test dataSet
  QgsMeshDatasetIndex ds( 1, 0 );
  QCOMPARE( QgsMeshDatasetValue( 1, 1 ), layer.dataProvider()->datasetValue( ds, 0 ) );
  QCOMPARE( QgsMeshDatasetValue( 2, 1 ), layer.dataProvider()->datasetValue( ds, 1 ) );
  QCOMPARE( QgsMeshDatasetValue( 3, 2 ), layer.dataProvider()->datasetValue( ds, 2 ) );
  QCOMPARE( QgsMeshDatasetValue( 2, 2 ), layer.dataProvider()->datasetValue( ds, 3 ) );
  QCOMPARE( QgsMeshDatasetValue( 1, -2 ), layer.dataProvider()->datasetValue( ds, 4 ) );
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

void TestQgsMeshLayer::test_temporal()
{
  qint64 relativeTime_0 = -1000;
  qint64 relativeTime_1 = 0;
  qint64 relativeTime_2 = 1000 * 60 * 60 * 0.3;
  qint64 relativeTime_3 = 1000 * 60 * 60 * 0.5;
  qint64 relativeTime_4 = 1000 * 60 * 60 * 1.5;
  qint64 relativeTime_5 = 1000 * 60 * 60 * 2;
  // Mesh memory provider
  QgsMeshDataProviderTemporalCapabilities *tempCap = mMemoryLayer->dataProvider()->temporalCapabilities();
  // Static dataset
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_0, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_2, relativeTime_3 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_3, relativeTime_4 ).dataset(), 0 );
  // Temporal dataset
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_0, relativeTime_1 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_2, relativeTime_3 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_3, relativeTime_4 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_4, relativeTime_5 ).dataset(), -1 );

  // Mesh MDAL provider with internal dataset
  tempCap = mMdalLayer->dataProvider()->temporalCapabilities();
  // Static dataset
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_0, relativeTime_1 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_2, relativeTime_3 ).dataset(), 0 );
  // Temporal dataset
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_0, relativeTime_1 ).dataset(), -1 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_1, relativeTime_2 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_2, relativeTime_3 ).dataset(), 0 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_3, relativeTime_4 ).dataset(), 1 );
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 1, relativeTime_4, relativeTime_5 ).dataset(), -1 );

  //Mesh MDAL provider with reference time
  tempCap = mMdal3DLayer->dataProvider()->temporalCapabilities();
  QCOMPARE( tempCap->datasetIndexFromRelativeTimeRange( 0, relativeTime_0, relativeTime_1 ).dataset(), 0 );
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
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1, time_2 ) ).dataset(), 18 );
  // Next dataset
  QCOMPARE( mMdal3DLayer->activeScalarDatasetAtTime( QgsDateTimeRange( time_1.addSecs( 400 ), time_2.addSecs( 400 ) ) ).dataset(), 19 );

  mMdal3DLayer->temporalProperties();
}

QGSTEST_MAIN( TestQgsMeshLayer )
#include "testqgsmeshlayer.moc"
