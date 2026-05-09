/***************************************************************************
  testqgsailayerindexcoordinator.cpp
  --------------------------------
  begin                : May 2026
***************************************************************************/

#include <memory>

#include "ai/index/qgsaiembeddingclient.h"
#include "ai/index/qgsailayerchunker.h"
#include "ai/index/qgsailayerindexcoordinator.h"
#include "ai/index/qgsaiworkspaceindex.h"
#include "ai/qgsaifilecontextprovider.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QFile>
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

    void debouncesEditingStoppedAndCallsReindexLayer();
    void layerWillBeRemovedDropsChunksImmediately();
    void disabledCoordinatorIgnoresProjectSignals();

  private:
    /**
     * Copies the test points.shp (+ sidecars) into \a destDir so editingStopped
     * exercises a writable copy and never mutates the in-tree fixture.
     */
    static QString copyPointsShapefile( const QString &destDir );
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

void TestQgsAiLayerIndexCoordinator::debouncesEditingStoppedAndCallsReindexLayer()
{
  QTemporaryDir tempDir;
  QVERIFY( tempDir.isValid() );
  QgsAiFileContextProvider contextProvider( tempDir.path() );

  // Embedding client without API key — reindexLayer() will fail with a clear
  // "API key" error, which is what we want to assert (proves the coordinator
  // wired the call through). No network traffic happens.
  QgsAiEmbeddingClient client;
  QgsAiWorkspaceIndex index( &contextProvider, &client );

  QgsAiLayerIndexCoordinator coord( &index );
  coord.setDebounceMs( 50 );

  // Add a layer to the project BEFORE enabling the coordinator: the coordinator
  // must wire signals on existing layers too via connectProjectSignals().
  const QString shpPath = copyPointsShapefile( tempDir.path() );
  auto layer = std::make_unique<QgsVectorLayer>( shpPath, u"points"_s, u"ogr"_s );
  QVERIFY( layer->isValid() );
  QgsProject::instance()->addMapLayer( layer.get(), false );

  coord.setEnabled( true );

  QSignalSpy startSpy( &coord, &QgsAiLayerIndexCoordinator::reindexStarted );
  QSignalSpy doneSpy( &coord, &QgsAiLayerIndexCoordinator::reindexFinished );

  // Trigger an editingStopped pulse: startEditing then commitChanges.
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->commitChanges() );

  QVERIFY( doneSpy.wait( 5000 ) );
  QCOMPARE( startSpy.count(), 1 );
  QCOMPARE( startSpy.first().at( 0 ).toString(), layer->id() );
  QCOMPARE( doneSpy.count(), 1 );
  QCOMPARE( doneSpy.first().at( 0 ).toString(), layer->id() );
  // success must be false because no API key, and the message must mention API key.
  QCOMPARE( doneSpy.first().at( 1 ).toBool(), false );
  QVERIFY2( doneSpy.first().at( 2 ).toString().contains( u"API key"_s, Qt::CaseInsensitive ), doneSpy.first().at( 2 ).toString().toUtf8().constData() );

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

  QgsAiEmbeddingClient client;
  QgsAiWorkspaceIndex index( &contextProvider, &client );

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

QGSTEST_MAIN( TestQgsAiLayerIndexCoordinator )
#include "testqgsailayerindexcoordinator.moc"
