/***************************************************************************
     testqgsmdalprovider.cpp
     --------------------------------------
    Date                 : Decemeber 2018
    Copyright            : (C) 2018 by Peter Petrik
    Email                : zilolv@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmeshdataprovider.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsMdalProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsMdalProvider()
      : QgsTest( QStringLiteral( "MDAL Provider Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void load();
    void filters();
    void encodeDecodeUri();
    void absoluteRelativeUri();
    void preserveMeshMetadata();
    void uniqueDatasetNames();
    void addRemoveDatasetGroups();

  private:
    QString mTestDataDir;
};

//runs before all tests
void TestQgsMdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
}

//runs after all tests
void TestQgsMdalProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMdalProvider::filters()
{
  const QString meshFilters = QgsProviderRegistry::instance()->fileMeshFilters();
  QVERIFY( meshFilters.contains( "*.2dm" ) );

  const QString datasetFilters = QgsProviderRegistry::instance()->fileMeshDatasetFilters();
  QVERIFY( datasetFilters.contains( "*.dat" ) );
}

void TestQgsMdalProvider::encodeDecodeUri()
{
  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "mdal" ) );

  // simple file uri
  QVariantMap parts = mdalMetadata->decodeUri( QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QString() );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "/home/data/test.nc" ) );

  // uri with driver and layer name
  parts = mdalMetadata->decodeUri( QStringLiteral( "netcdf:\"/home/data/test.nc\":layer3" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "netcdf" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QStringLiteral( "layer3" ) );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "netcdf:\"/home/data/test.nc\":layer3" ) );

  // uri with driver and layer name with space
  parts = mdalMetadata->decodeUri( QStringLiteral( "netcdf:\"/home/data/test.nc\":layer 3" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "netcdf" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QStringLiteral( "layer 3" ) );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "netcdf:\"/home/data/test.nc\":layer 3" ) );

  // uri with driver
  parts = mdalMetadata->decodeUri( QStringLiteral( "Ugrid:\"/home/data/test.nc\"" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/test.nc" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "Ugrid" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "Ugrid:\"/home/data/test.nc\"" ) );

  parts = mdalMetadata->decodeUri( QStringLiteral( "ESRI_TIN:\"/home/data/tdenv9.adf\"" ) );
  QCOMPARE( parts.value( QStringLiteral( "path" ) ).toString(), QStringLiteral( "/home/data/tdenv9.adf" ) );
  QCOMPARE( parts.value( QStringLiteral( "driver" ) ).toString(), QStringLiteral( "ESRI_TIN" ) );
  QCOMPARE( parts.value( QStringLiteral( "layerName" ) ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), QStringLiteral( "ESRI_TIN:\"/home/data/tdenv9.adf\"" ) );
}

void TestQgsMdalProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( "mdal" );
  QVERIFY( mdalMetadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_flower.2dm" );
  QString relativeUri = QStringLiteral( "./mesh/quad_flower.2dm" );
  QCOMPARE( mdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = QStringLiteral( "2DM:\"%1/mesh/mesh_flower.2dm\"" ).arg( QStringLiteral( TEST_DATA_DIR ) );
  relativeUri = QStringLiteral( "2DM:\"./mesh/mesh_flower.2dm\"" );
  QCOMPARE( mdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsMdalProvider::load()
{
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_flower.2dm";
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
      QStringLiteral( "mdal" ),
      file,
      QgsDataProvider::ProviderOptions()
    );

    QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
    QVERIFY( mp );
    QVERIFY( mp->isValid() );
    delete provider;
  }
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/goodluckwiththisfilename.2dm" );
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
      QStringLiteral( "mdal" ),
      file,
      QgsDataProvider::ProviderOptions()
    );

    QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
    QVERIFY( mp );
    QVERIFY( !mp->isValid() );
    delete provider;
  }
}

void TestQgsMdalProvider::preserveMeshMetadata()
{
  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( "mdal" );
  QVERIFY( mdalMetadata );

  QString uri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/small.mesh" );

  QDir dir( QDir::tempPath() + QStringLiteral( "/mesh_metadata_test" ) );
  dir.mkpath( dir.path() );
  Q_ASSERT( dir.exists() );
  QFile meshFile( uri );
  const QString copiedFile = dir.filePath( QStringLiteral( "small.mesh" ) );
  meshFile.copy( copiedFile );

  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    QStringLiteral( "mdal" ),
    copiedFile,
    QgsDataProvider::ProviderOptions()
  );

  QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
  QVERIFY( mp );
  QVERIFY( mp->isValid() );

  QgsMesh *mesh = new QgsMesh();
  mp->populateMesh( mesh );
  QVERIFY( mp->saveMeshFrame( *mesh ) );
  mp->reloadData();

  QVERIFY( mp->isValid() );

  dir.removeRecursively();

  delete provider;
  delete mesh;
}


void TestQgsMdalProvider::uniqueDatasetNames()
{
  const QString file = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle.2dm" );
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    QStringLiteral( "mdal" ),
    file,
    QgsDataProvider::ProviderOptions()
  );

  QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );

  QgsDataProvider *provider1 = QgsProviderRegistry::instance()->createProvider(
    QStringLiteral( "mdal" ),
    file,
    QgsDataProvider::ProviderOptions()
  );
  QgsMeshDataProvider *mp1 = dynamic_cast<QgsMeshDataProvider *>( provider1 );

  // these three dataset files have the same name
  const QString fileDatasetGroup1 = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle_vertex_vector.dat" );
  const QString fileDatasetGroup2 = QDir::tempPath() + QStringLiteral( "/quad_and_triangle_vertex_vector_1.dat" );
  const QString fileDatasetGroup3 = QDir::tempPath() + QStringLiteral( "/quad_and_triangle_vertex_vector_2.dat" );

  QFile::remove( fileDatasetGroup2 );
  QVERIFY( QFile::copy( fileDatasetGroup1, fileDatasetGroup2 ) );

  QFile::remove( fileDatasetGroup3 );
  QVERIFY( QFile::copy( fileDatasetGroup1, fileDatasetGroup3 ) );

  // test that if added to different provider they have same names
  QVERIFY( mp->addDataset( fileDatasetGroup1 ) );
  QCOMPARE( mp->datasetGroupCount(), 2 );

  QgsMeshDatasetGroupMetadata metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset" ) );

  QVERIFY( mp1->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp1->datasetGroupCount(), 2 );

  metadata = mp1->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset" ) );

  QCOMPARE( mp1->datasetGroupMetadata( 1 ).name(), mp->datasetGroupMetadata( 1 ).name() );

  //test that if both additional files are added to the same provider, the names will be unique
  QVERIFY( mp->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp->datasetGroupCount(), 3 );

  QVERIFY( mp->addDataset( fileDatasetGroup3 ) );
  QCOMPARE( mp->datasetGroupCount(), 4 );

  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset" ) );
  metadata = mp->datasetGroupMetadata( 2 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset_1" ) );
  metadata = mp->datasetGroupMetadata( 3 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset_2" ) );

  delete provider;
  delete provider1;
}

void TestQgsMdalProvider::addRemoveDatasetGroups()
{
  const QString file = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle.2dm" );
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    QStringLiteral( "mdal" ),
    file,
    QgsDataProvider::ProviderOptions()
  );

  QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
  QVERIFY( mp );
  QVERIFY( mp->isValid() );
  QCOMPARE( mp->datasetGroupCount(), 1 );

  const QString fileDatasetGroup1 = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle_vertex_vector.dat" );
  QVERIFY( mp->addDataset( fileDatasetGroup1 ) );
  QCOMPARE( mp->datasetGroupCount(), 2 );

  // cannot add the same dataset twice
  QCOMPARE( mp->addDataset( fileDatasetGroup1 ), false );

  const QString fileDatasetGroup2 = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/mesh/quad_and_triangle_vertex_scalar2.dat" );
  QVERIFY( mp->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp->datasetGroupCount(), 3 );

  QgsMeshDatasetGroupMetadata metadata = mp->datasetGroupMetadata( 0 );
  QCOMPARE( metadata.name(), QStringLiteral( "Bed Elevation" ) );

  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexVectorDataset" ) );

  metadata = mp->datasetGroupMetadata( 2 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexScalarDataset2" ) );

  // cannot remove dataset groups with index outside of range
  QCOMPARE( mp->removeDatasetGroup( -1 ), false );
  QCOMPARE( mp->removeDatasetGroup( 10 ), false );

  // cannot remove data associated with source file
  QCOMPARE( mp->removeDatasetGroup( 0 ), false );

  // can remove other datasets group
  QCOMPARE( mp->removeDatasetGroup( 1 ), true );

  // check the dataset group that are left
  QCOMPARE( mp->datasetGroupCount(), 2 );
  metadata = mp->datasetGroupMetadata( 0 );
  QCOMPARE( metadata.name(), QStringLiteral( "Bed Elevation" ) );
  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), QStringLiteral( "VertexScalarDataset2" ) );

  // remove the second - only the original remains
  QCOMPARE( mp->removeDatasetGroup( 1 ), true );

  // check the dataset group that are left
  QCOMPARE( mp->datasetGroupCount(), 1 );
  metadata = mp->datasetGroupMetadata( 0 );
  QCOMPARE( metadata.name(), QStringLiteral( "Bed Elevation" ) );

  delete provider;
}
QGSTEST_MAIN( TestQgsMdalProvider )
#include "testqgsmdalprovider.moc"
