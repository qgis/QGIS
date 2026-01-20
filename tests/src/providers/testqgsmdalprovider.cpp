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

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>

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
      : QgsTest( u"MDAL Provider Tests"_s ) {}

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
  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( u"mdal"_s );

  // simple file uri
  QVariantMap parts = mdalMetadata->decodeUri( u"/home/data/test.nc"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/data/test.nc"_s );
  QCOMPARE( parts.value( u"driver"_s ).toString(), QString() );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), u"/home/data/test.nc"_s );

  // uri with driver and layer name
  parts = mdalMetadata->decodeUri( u"netcdf:\"/home/data/test.nc\":layer3"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/data/test.nc"_s );
  QCOMPARE( parts.value( u"driver"_s ).toString(), u"netcdf"_s );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), u"layer3"_s );
  QCOMPARE( mdalMetadata->encodeUri( parts ), u"netcdf:\"/home/data/test.nc\":layer3"_s );

  // uri with driver and layer name with space
  parts = mdalMetadata->decodeUri( u"netcdf:\"/home/data/test.nc\":layer 3"_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/data/test.nc"_s );
  QCOMPARE( parts.value( u"driver"_s ).toString(), u"netcdf"_s );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), u"layer 3"_s );
  QCOMPARE( mdalMetadata->encodeUri( parts ), u"netcdf:\"/home/data/test.nc\":layer 3"_s );

  // uri with driver
  parts = mdalMetadata->decodeUri( u"Ugrid:\"/home/data/test.nc\""_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/data/test.nc"_s );
  QCOMPARE( parts.value( u"driver"_s ).toString(), u"Ugrid"_s );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), u"Ugrid:\"/home/data/test.nc\""_s );

  parts = mdalMetadata->decodeUri( u"ESRI_TIN:\"/home/data/tdenv9.adf\""_s );
  QCOMPARE( parts.value( u"path"_s ).toString(), u"/home/data/tdenv9.adf"_s );
  QCOMPARE( parts.value( u"driver"_s ).toString(), u"ESRI_TIN"_s );
  QCOMPARE( parts.value( u"layerName"_s ).toString(), QString() );
  QCOMPARE( mdalMetadata->encodeUri( parts ), u"ESRI_TIN:\"/home/data/tdenv9.adf\""_s );
}

void TestQgsMdalProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *mdalMetadata = QgsProviderRegistry::instance()->providerMetadata( "mdal" );
  QVERIFY( mdalMetadata );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_flower.2dm"_s;
  QString relativeUri = u"./mesh/quad_flower.2dm"_s;
  QCOMPARE( mdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = u"2DM:\"%1/mesh/mesh_flower.2dm\""_s.arg( QStringLiteral( TEST_DATA_DIR ) );
  relativeUri = u"2DM:\"./mesh/mesh_flower.2dm\""_s;
  QCOMPARE( mdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsMdalProvider::load()
{
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_flower.2dm";
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
      u"mdal"_s,
      file,
      QgsDataProvider::ProviderOptions()
    );

    QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
    QVERIFY( mp );
    QVERIFY( mp->isValid() );
    delete provider;
  }
  {
    const QString file = QStringLiteral( TEST_DATA_DIR ) + u"/goodluckwiththisfilename.2dm"_s;
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
      u"mdal"_s,
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

  QString uri = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/small.mesh"_s;

  QDir dir( QDir::tempPath() + u"/mesh_metadata_test"_s );
  dir.mkpath( dir.path() );
  Q_ASSERT( dir.exists() );
  QFile meshFile( uri );
  const QString copiedFile = dir.filePath( u"small.mesh"_s );
  meshFile.copy( copiedFile );

  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    u"mdal"_s,
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
  const QString file = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_and_triangle.2dm"_s;
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    u"mdal"_s,
    file,
    QgsDataProvider::ProviderOptions()
  );

  QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );

  QgsDataProvider *provider1 = QgsProviderRegistry::instance()->createProvider(
    u"mdal"_s,
    file,
    QgsDataProvider::ProviderOptions()
  );
  QgsMeshDataProvider *mp1 = dynamic_cast<QgsMeshDataProvider *>( provider1 );

  // these three dataset files have the same name
  const QString fileDatasetGroup1 = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_and_triangle_vertex_vector.dat"_s;
  const QString fileDatasetGroup2 = QDir::tempPath() + u"/quad_and_triangle_vertex_vector_1.dat"_s;
  const QString fileDatasetGroup3 = QDir::tempPath() + u"/quad_and_triangle_vertex_vector_2.dat"_s;

  QFile::remove( fileDatasetGroup2 );
  QVERIFY( QFile::copy( fileDatasetGroup1, fileDatasetGroup2 ) );

  QFile::remove( fileDatasetGroup3 );
  QVERIFY( QFile::copy( fileDatasetGroup1, fileDatasetGroup3 ) );

  // test that if added to different provider they have same names
  QVERIFY( mp->addDataset( fileDatasetGroup1 ) );
  QCOMPARE( mp->datasetGroupCount(), 2 );

  QgsMeshDatasetGroupMetadata metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset"_s );

  QVERIFY( mp1->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp1->datasetGroupCount(), 2 );

  metadata = mp1->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset"_s );

  QCOMPARE( mp1->datasetGroupMetadata( 1 ).name(), mp->datasetGroupMetadata( 1 ).name() );

  //test that if both additional files are added to the same provider, the names will be unique
  QVERIFY( mp->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp->datasetGroupCount(), 3 );

  QVERIFY( mp->addDataset( fileDatasetGroup3 ) );
  QCOMPARE( mp->datasetGroupCount(), 4 );

  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset"_s );
  metadata = mp->datasetGroupMetadata( 2 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset_1"_s );
  metadata = mp->datasetGroupMetadata( 3 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset_2"_s );

  delete provider;
  delete provider1;
}

void TestQgsMdalProvider::addRemoveDatasetGroups()
{
  const QString file = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_and_triangle.2dm"_s;
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider(
    u"mdal"_s,
    file,
    QgsDataProvider::ProviderOptions()
  );

  QgsMeshDataProvider *mp = dynamic_cast<QgsMeshDataProvider *>( provider );
  QVERIFY( mp );
  QVERIFY( mp->isValid() );
  QCOMPARE( mp->datasetGroupCount(), 1 );

  const QString fileDatasetGroup1 = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_and_triangle_vertex_vector.dat"_s;
  QVERIFY( mp->addDataset( fileDatasetGroup1 ) );
  QCOMPARE( mp->datasetGroupCount(), 2 );

  // cannot add the same dataset twice
  QCOMPARE( mp->addDataset( fileDatasetGroup1 ), false );

  const QString fileDatasetGroup2 = QStringLiteral( TEST_DATA_DIR ) + u"/mesh/quad_and_triangle_vertex_scalar2.dat"_s;
  QVERIFY( mp->addDataset( fileDatasetGroup2 ) );
  QCOMPARE( mp->datasetGroupCount(), 3 );

  QgsMeshDatasetGroupMetadata metadata = mp->datasetGroupMetadata( 0 );
  QCOMPARE( metadata.name(), u"Bed Elevation"_s );

  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), u"VertexVectorDataset"_s );

  metadata = mp->datasetGroupMetadata( 2 );
  QCOMPARE( metadata.name(), u"VertexScalarDataset2"_s );

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
  QCOMPARE( metadata.name(), u"Bed Elevation"_s );
  metadata = mp->datasetGroupMetadata( 1 );
  QCOMPARE( metadata.name(), u"VertexScalarDataset2"_s );

  // remove the second - only the original remains
  QCOMPARE( mp->removeDatasetGroup( 1 ), true );

  // check the dataset group that are left
  QCOMPARE( mp->datasetGroupCount(), 1 );
  metadata = mp->datasetGroupMetadata( 0 );
  QCOMPARE( metadata.name(), u"Bed Elevation"_s );

  delete provider;
}
QGSTEST_MAIN( TestQgsMdalProvider )
#include "testqgsmdalprovider.moc"
