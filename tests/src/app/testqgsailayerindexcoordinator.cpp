/***************************************************************************
  testqgsailayerindexcoordinator.cpp
  --------------------------------
  begin                : May 2026
***************************************************************************/

#include <memory>

#include "ai/index/qgsaiembeddingprovider.h"
#include "ai/index/qgsailayerchunker.h"
#include "ai/index/qgsailayerindexcoordinator.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "ai/qgsaifilecontextprovider.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QFile>
#include <QMetaObject>
#include <QSignalSpy>
#include <QString>
#include <QTemporaryDir>
#include <QVector>

using namespace Qt::StringLiterals;

class TestQgsAiLayerIndexCoordinator : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void enablingSchedulesExistingLayers();
    void layerChangeSignalsAreDebounced();
    void layerWillBeRemovedDropsChunksImmediately();
    void disabledCoordinatorIgnoresProjectSignals();
    void unavailableEmbeddingProviderPreventsScheduling();

  private:
    /**
     * Copies the test points.shp (+ sidecars) into \a destDir so editingStopped
     * exercises a writable copy and never mutates the in-tree fixture.
     */
    static QString copyPointsShapefile( const QString &destDir );
};

class CountingWorkspaceIndex : public QgsAiWorkspaceIndex
{
  public:
    explicit CountingWorkspaceIndex( QgsAiFileContextProvider *contextProvider )
      : QgsAiWorkspaceIndex( contextProvider, nullptr )
    {}

    bool embeddingProviderAvailable() const override { return true; }

    bool reindexLayer( const QString &layerId, QString *errorMessage = nullptr ) override
    {
      Q_UNUSED( errorMessage )
      reindexedLayerIds.append( layerId );
      return true;
    }

    QStringList reindexedLayerIds;
};

QString TestQgsAiLayerIndexCoordinator::copyPointsShapefile( const QString &destDir )
{
  const QString src = QStringLiteral( TEST_DATA_DIR ) + u"/points"_s;
  const QString dst = destDir + u"/points"_s;
  for ( const QString &ext : { u".shp"_s, u".shx"_s, u".dbf"_s, u".prj"_s } )
  {
    if ( QFile::exists( src + ext ) )
      QFile::copy( src + ext, dst + ext );
  }
  return dst + u".shp"_s;
}

void TestQgsAiLayerIndexCoordinator::initTestCase()
{
  QgsApplication::initQgis();
}

void TestQgsAiLayerIndexCoordinator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAiLayerIndexCoordinator::cleanup()
{
  QgsProject::instance()->clear();
}

void TestQgsAiLayerIndexCoordinator::enablingSchedulesExistingLayers()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );

  CountingWorkspaceIndex index( &contextProvider );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 50 );

  // Add a layer to the project BEFORE enabling the coordinator: the coordinator
  // must wire signals on existing layers too via connectProjectSignals().
  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer.get(), false );

  QSignalSpy startSpy( &coord, &QgsAiLayerIndexCoordinator::reindexStarted );
  QSignalSpy doneSpy( &coord, &QgsAiLayerIndexCoordinator::reindexFinished );

  coord.setEnabled( true );

  QVERIFY( doneSpy.wait( 5000 ) );
  QCOMPARE( startSpy.count(), 1 );
  QCOMPARE( startSpy.first().at( 0 ).toString(), layer->id() );
  QCOMPARE( doneSpy.count(), 1 );
  QCOMPARE( doneSpy.first().at( 0 ).toString(), layer->id() );
  QCOMPARE( doneSpy.first().at( 1 ).toBool(), true );
  QCOMPARE( index.reindexedLayerIds, QStringList { layer->id() } );

  QgsProject::instance()->removeMapLayer( layer.release() );
}

void TestQgsAiLayerIndexCoordinator::layerChangeSignalsAreDebounced()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );

  CountingWorkspaceIndex index( &contextProvider );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 200 );

  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer.get(), false );

  QSignalSpy doneSpy( &coord, &QgsAiLayerIndexCoordinator::reindexFinished );
  coord.setEnabled( true );
  QVERIFY( doneSpy.wait( 5000 ) );
  doneSpy.clear();
  index.reindexedLayerIds.clear();

  QVERIFY( QMetaObject::invokeMethod( layer.get(), "layerModified", Qt::DirectConnection ) );
  QVERIFY( QMetaObject::invokeMethod( layer.get(), "dataChanged", Qt::DirectConnection ) );

  QVERIFY( layer->startEditing() );
  QgsFeature feature( layer->fields() );
  feature.setGeometry( QgsGeometry::fromWkt( u"Point(0 0)"_s ) );
  QVERIFY( layer->addFeature( feature ) );
  QVERIFY( layer->commitChanges() );

  QVERIFY( doneSpy.wait( 5000 ) );
  QCOMPARE( doneSpy.count(), 1 );
  QCOMPARE( doneSpy.first().at( 0 ).toString(), layer->id() );
  QCOMPARE( doneSpy.first().at( 1 ).toBool(), true );
  QCOMPARE( index.reindexedLayerIds, QStringList { layer->id() } );

  QgsProject::instance()->removeMapLayer( layer.release() );
}

void TestQgsAiLayerIndexCoordinator::layerWillBeRemovedDropsChunksImmediately()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );

  QgsAiWorkspaceIndex index( &contextProvider, nullptr );

  // Pre-populate the index with chunker output for a real shapefile,
  // bypassing the embedding step with dummy vectors.
  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );

  const auto chunks = QgsAiLayerChunker::chunkVector( layer.get() );
  QVERIFY( !chunks.isEmpty() );
  QList<QVector<float>> embeddings;
  embeddings.reserve( chunks.size() );
  for ( int i = 0; i < chunks.size(); ++i )
  {
    QVector<float> v( 4 );
    v[0] = 0.1f * i;
    embeddings.append( v );
  }
  QString err;
  QVERIFY( index.persistChunks( chunks, embeddings, QgsAiWorkspaceIndex::ReplaceScope::All, QString(), &err ) );
  QVERIFY( index.status().layerChunkCount > 0 );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 50 );

  QgsProject::instance()->addMapLayer( layer.get(), false );
  coord.setEnabled( true );

  // Removing the layer must purge its chunks synchronously (no debounce).
  const QString layerId = layer->id();
  QgsProject::instance()->removeMapLayer( layer.release() );
  QCOMPARE( index.status().layerChunkCount, 0 );
  QCOMPARE( index.chunks( QgsAiWorkspaceIndex::ReplaceScope::SingleLayer, layerId ).size(), 0 );
}

void TestQgsAiLayerIndexCoordinator::disabledCoordinatorIgnoresProjectSignals()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );

  CountingWorkspaceIndex index( &contextProvider );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 50 );
  // intentionally not enabling the coordinator

  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer.get(), false );

  QSignalSpy doneSpy( &coord, &QgsAiLayerIndexCoordinator::reindexFinished );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->commitChanges() );

  // Wait twice the debounce window — nothing should have fired.
  QTest::qWait( 200 );
  QCOMPARE( doneSpy.count(), 0 );

  QgsProject::instance()->removeMapLayer( layer.release() );
}

void TestQgsAiLayerIndexCoordinator::unavailableEmbeddingProviderPreventsScheduling()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );
  QgsAiUnavailableLocalEmbeddingProvider provider;
  QgsAiWorkspaceIndex index( &contextProvider, &provider );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 50 );

  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer.get(), false );

  QSignalSpy doneSpy( &coord, &QgsAiLayerIndexCoordinator::reindexFinished );
  coord.setEnabled( true );

  QTest::qWait( 200 );
  QCOMPARE( doneSpy.count(), 0 );

  QgsProject::instance()->removeMapLayer( layer.release() );
}

QGSTEST_MAIN( TestQgsAiLayerIndexCoordinator )
#include "testqgsailayerindexcoordinator.moc"
