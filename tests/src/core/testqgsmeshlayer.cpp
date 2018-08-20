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

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    QString readFile( const QString &fname ) const;

    void test_write_read_project();
    void test_read_mesh();
    void test_read_vertex_scalar_dataset();
    void test_read_vertex_vector_dataset();
    void test_read_face_scalar_dataset();
    void test_read_face_vector_dataset();
    void test_read_vertex_scalar_dataset_with_inactive_face();
    void test_extent();
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
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_vertex_vector.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_scalar.txt" ) );
  mMemoryLayer->dataProvider()->addDataset( readFile( "/quad_and_triangle_face_vector.txt" ) );
  QCOMPARE( mMemoryLayer->dataProvider()->extraDatasets().count(), 4 );

  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMemoryLayer );

  // MDAL Layer
  QString uri( mDataDir + "/quad_and_triangle.2dm" );
  mMdalLayer = new QgsMeshLayer( uri, "Triangle and Quad MDAL", "mdal" );
  QCOMPARE( mMdalLayer->dataProvider()->extraDatasets().count(), 0 );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_vertex_vector.dat" );
  QCOMPARE( mMdalLayer->dataProvider()->extraDatasets().count(), 2 );

  //The face dataset is recognized by "_els_" in the filename for this format
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_scalar.dat" );
  mMdalLayer->dataProvider()->addDataset( mDataDir + "/quad_and_triangle_els_face_vector.dat" );

  QVERIFY( mMdalLayer->isValid() );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mMdalLayer );
}

void TestQgsMeshLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
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

    QCOMPARE( 5, dp->vertexCount() );
    QCOMPARE( QgsMeshVertex( 1000.0, 2000.0 ), dp->vertex( 0 ) );
    QCOMPARE( QgsMeshVertex( 2000.0, 2000.0 ), dp->vertex( 1 ) );
    QCOMPARE( QgsMeshVertex( 3000.0, 2000.0 ), dp->vertex( 2 ) );
    QCOMPARE( QgsMeshVertex( 2000.0, 3000.0 ), dp->vertex( 3 ) );
    QCOMPARE( QgsMeshVertex( 1000.0, 3000.0 ), dp->vertex( 4 ) );

    QCOMPARE( 2, dp->faceCount() );
    QgsMeshFace f1;
    f1 << 0 << 1 << 3 << 4;
    QCOMPARE( f1, dp->face( 0 ) );

    QgsMeshFace f2;
    f2 << 1 << 2 << 3;
    QCOMPARE( f2, dp->face( 1 ) );
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

    QCOMPARE( 4, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 0, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( meta.extraOptions().count() == 2 )
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

    QCOMPARE( 4, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 1, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( meta.extraOptions().count() == 2 )
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

    QCOMPARE( 4, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 2, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( meta.extraOptions().count() == 2 )
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

    QCOMPARE( 4, dp->datasetGroupCount() );

    for ( int i = 0; i < 2 ; ++i )
    {
      QgsMeshDatasetIndex ds( 3, i );

      const QgsMeshDatasetGroupMetadata meta = dp->datasetGroupMetadata( ds );
      if ( meta.extraOptions().count() == 2 )
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
  QCOMPARE( 1, dp->datasetGroupCount() );

  for ( int i = 0; i < 2 ; ++i )
  {
    QgsMeshDatasetIndex ds( 0, i );

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

QGSTEST_MAIN( TestQgsMeshLayer )
#include "testqgsmeshlayer.moc"
