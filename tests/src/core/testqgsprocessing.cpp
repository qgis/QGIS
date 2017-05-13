/***************************************************************************
                         testqgsprocessing.cpp
                         ---------------------
    begin                : January 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingregistry.h"
#include "qgsprocessingprovider.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include <QObject>
#include <QtTest/QSignalSpy>
#include "qgis.h"
#include "qgstest.h"
#include "qgsrasterlayer.h"
#include "qgsproject.h"
#include "qgspointv2.h"
#include "qgsgeometry.h"
#include "qgsvectorfilewriter.h"

class DummyAlgorithm : public QgsProcessingAlgorithm
{
  public:

    DummyAlgorithm( const QString &name ) : mName( name ) {}

    QString name() const override { return mName; }
    QString displayName() const override { return mName; }

    QString mName;
};

//dummy provider for testing
class DummyProvider : public QgsProcessingProvider
{
  public:

    DummyProvider( const QString &id ) : mId( id ) {}

    virtual QString id() const override { return mId; }

    virtual QString name() const override { return "dummy"; }

    void unload() override { if ( unloaded ) { *unloaded = true; } }

    bool *unloaded = nullptr;

  protected:

    virtual void loadAlgorithms() override
    {
      QVERIFY( addAlgorithm( new DummyAlgorithm( "alg1" ) ) );
      QVERIFY( addAlgorithm( new DummyAlgorithm( "alg2" ) ) );

      //dupe name
      QgsProcessingAlgorithm *a = new DummyAlgorithm( "alg1" );
      QVERIFY( !addAlgorithm( a ) );
      delete a;

      QVERIFY( !addAlgorithm( nullptr ) );
    }

    QString mId;


};

class DummyProviderNoLoad : public DummyProvider
{
  public:

    DummyProviderNoLoad( const QString &id ) : DummyProvider( id ) {}

    bool load() override
    {
      return false;
    }

};

class TestQgsProcessing: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void instance();
    void addProvider();
    void providerById();
    void removeProvider();
    void compatibleLayers();
    void normalizeLayerSource();
    void context();
    void mapLayers();
    void mapLayerFromStore();
    void mapLayerFromString();
    void algorithm();
    void features();
    void uniqueValues();
    void createIndex();
    void createFeatureSink();

  private:

};

void TestQgsProcessing::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsProcessing::cleanupTestCase()
{
  QFile::remove( QDir::tempPath() + "/create_feature_sink.tab" );
  QgsVectorFileWriter::deleteShapeFile( QDir::tempPath() + "/create_feature_sink2.shp" );

  QgsApplication::exitQgis();
}

void TestQgsProcessing::instance()
{
  // test that application has a registry instance
  QVERIFY( QgsApplication::processingRegistry() );
}

void TestQgsProcessing::addProvider()
{
  QgsProcessingRegistry r;
  QSignalSpy spyProviderAdded( &r, &QgsProcessingRegistry::providerAdded );

  QVERIFY( r.providers().isEmpty() );

  QVERIFY( !r.addProvider( nullptr ) );

  // add a provider
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( r.addProvider( p ) );
  QCOMPARE( r.providers(), QList< QgsProcessingProvider * >() << p );
  QCOMPARE( spyProviderAdded.count(), 1 );
  QCOMPARE( spyProviderAdded.last().at( 0 ).toString(), QString( "p1" ) );

  //try adding another provider
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );
  QCOMPARE( spyProviderAdded.last().at( 0 ).toString(), QString( "p2" ) );

  //try adding a provider with duplicate id
  DummyProvider *p3 = new DummyProvider( "p2" );
  QVERIFY( !r.addProvider( p3 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );
  delete p3;

  // test that adding a provider which does not load means it is not added to registry
  DummyProviderNoLoad *p4 = new DummyProviderNoLoad( "p4" );
  QVERIFY( !r.addProvider( p4 ) );
  QCOMPARE( r.providers().toSet(), QSet< QgsProcessingProvider * >() << p << p2 );
  QCOMPARE( spyProviderAdded.count(), 2 );
  delete p4;
}

void TestQgsProcessing::providerById()
{
  QgsProcessingRegistry r;

  // no providers
  QVERIFY( !r.providerById( "p1" ) );

  // add a provider
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( r.addProvider( p ) );
  QCOMPARE( r.providerById( "p1" ), p );
  QVERIFY( !r.providerById( "p2" ) );

  //try adding another provider
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );
  QCOMPARE( r.providerById( "p1" ), p );
  QCOMPARE( r.providerById( "p2" ), p2 );
  QVERIFY( !r.providerById( "p3" ) );
}

void TestQgsProcessing::removeProvider()
{
  QgsProcessingRegistry r;
  QSignalSpy spyProviderRemoved( &r, &QgsProcessingRegistry::providerRemoved );

  QVERIFY( !r.removeProvider( nullptr ) );
  QVERIFY( !r.removeProvider( "p1" ) );
  // provider not in registry
  DummyProvider *p = new DummyProvider( "p1" );
  QVERIFY( !r.removeProvider( p ) );
  QCOMPARE( spyProviderRemoved.count(), 0 );

  // add some providers
  QVERIFY( r.addProvider( p ) );
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( r.addProvider( p2 ) );

  // remove one by pointer
  bool unloaded = false;
  p->unloaded = &unloaded;
  QVERIFY( r.removeProvider( p ) );
  QCOMPARE( spyProviderRemoved.count(), 1 );
  QCOMPARE( spyProviderRemoved.last().at( 0 ).toString(), QString( "p1" ) );
  QCOMPARE( r.providers(), QList< QgsProcessingProvider * >() << p2 );

  //test that provider was unloaded
  QVERIFY( unloaded );

  // should fail, already removed
  QVERIFY( !r.removeProvider( "p1" ) );

  // remove one by id
  QVERIFY( r.removeProvider( "p2" ) );
  QCOMPARE( spyProviderRemoved.count(), 2 );
  QCOMPARE( spyProviderRemoved.last().at( 0 ).toString(), QString( "p2" ) );
  QVERIFY( r.providers().isEmpty() );
}

void TestQgsProcessing::compatibleLayers()
{
  QgsProject p;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QString raster3 = testDataDir + "/raster/band1_float32_noct_epsg4326.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );
  QFileInfo fi3( raster3 );
  QgsRasterLayer *r3 = new QgsRasterLayer( fi3.filePath(), "zz" );
  QVERIFY( r3->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  QgsVectorLayer *v3 = new QgsVectorLayer( "LineString", "v3", "memory" );
  QgsVectorLayer *v4 = new QgsVectorLayer( "none", "vvvv4", "memory" );

  p.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << r3 << v1 << v2 << v3 << v4 );

  // compatibleRasterLayers
  QVERIFY( QgsProcessingUtils::compatibleRasterLayers( nullptr ).isEmpty() );

  // sorted
  QStringList lIds;
  Q_FOREACH ( QgsRasterLayer *rl, QgsProcessingUtils::compatibleRasterLayers( &p ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "ar2" << "R1" << "zz" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsRasterLayer *rl, QgsProcessingUtils::compatibleRasterLayers( &p, false ) )
    lIds << rl->name();
  QCOMPARE( lIds, QStringList() << "R1" << "ar2" << "zz" );


  // compatibleVectorLayers
  QVERIFY( QgsProcessingUtils::compatibleVectorLayers( nullptr ).isEmpty() );

  // sorted
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" << "V4" << "vvvv4" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<QgsWkbTypes::GeometryType>(), false ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "V4" << "v1" << "v3" << "vvvv4" );

  // point only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<QgsWkbTypes::GeometryType>() << QgsWkbTypes::PointGeometry ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" );

  // polygon only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<QgsWkbTypes::GeometryType>() << QgsWkbTypes::PolygonGeometry ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "V4" );

  // line only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<QgsWkbTypes::GeometryType>() << QgsWkbTypes::LineGeometry ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v3" );

  // point and line only
  lIds.clear();
  Q_FOREACH ( QgsVectorLayer *vl, QgsProcessingUtils::compatibleVectorLayers( &p, QList<QgsWkbTypes::GeometryType>() << QgsWkbTypes::PointGeometry << QgsWkbTypes::LineGeometry ) )
    lIds << vl->name();
  QCOMPARE( lIds, QStringList() << "v1" << "v3" );


  // all layers
  QVERIFY( QgsProcessingUtils::compatibleLayers( nullptr ).isEmpty() );

  // sorted
  lIds.clear();
  Q_FOREACH ( QgsMapLayer *l, QgsProcessingUtils::compatibleLayers( &p ) )
    lIds << l->name();
  QCOMPARE( lIds, QStringList() << "ar2" << "R1" << "v1" << "v3" << "V4" << "vvvv4" <<  "zz" );

  // unsorted
  lIds.clear();
  Q_FOREACH ( QgsMapLayer *l, QgsProcessingUtils::compatibleLayers( &p, false ) )
    lIds << l->name();
  QCOMPARE( lIds, QStringList() << "R1" << "ar2" << "zz"  << "V4" << "v1" << "v3" << "vvvv4" );
}

void TestQgsProcessing::normalizeLayerSource()
{
  QCOMPARE( QgsProcessingUtils::normalizeLayerSource( "data\\layers\\test.shp" ), QString( "data/layers/test.shp" ) );
  QCOMPARE( QgsProcessingUtils::normalizeLayerSource( "data\\layers \"new\"\\test.shp" ), QString( "data/layers 'new'/test.shp" ) );
}

void TestQgsProcessing::context()
{
  QgsProcessingContext context;

  // simple tests for getters/setters
  context.setDefaultEncoding( "my_enc" );
  QCOMPARE( context.defaultEncoding(), QStringLiteral( "my_enc" ) );

  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  QCOMPARE( context.flags(), QgsProcessingContext::UseSelectionIfPresent );
  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  QCOMPARE( context.flags(), QgsProcessingContext::Flags( 0 ) );

  QgsProject p;
  context.setProject( &p );
  QCOMPARE( context.project(), &p );

  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometrySkipInvalid );
  QCOMPARE( context.invalidGeometryCheck(), QgsFeatureRequest::GeometrySkipInvalid );
}

void TestQgsProcessing::mapLayers()
{
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster = testDataDir + "landsat.tif";
  QString vector = testDataDir + "points.shp";

  // test loadMapLayerFromString with raster
  QgsMapLayer *l = QgsProcessingUtils::loadMapLayerFromString( raster );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::RasterLayer );
  delete l;

  //test with vector
  l = QgsProcessingUtils::loadMapLayerFromString( vector );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::VectorLayer );
  delete l;

  l = QgsProcessingUtils::loadMapLayerFromString( QString() );
  QVERIFY( !l );
  l = QgsProcessingUtils::loadMapLayerFromString( QStringLiteral( "so much room for activities!" ) );
  QVERIFY( !l );
  l = QgsProcessingUtils::loadMapLayerFromString( testDataDir + "multipoint.shp" );
  QVERIFY( l->isValid() );
  QCOMPARE( l->type(), QgsMapLayer::VectorLayer );
  delete l;
}

void TestQgsProcessing::mapLayerFromStore()
{
  // test mapLayerFromStore

  QgsMapLayerStore store;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  store.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << v1 << v2 );

  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QString(), nullptr ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QStringLiteral( "v1" ), nullptr ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromStore( QString(), &store ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( raster1, &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( raster2, &store ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "R1", &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "ar2", &store ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "V4", &store ), v1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( "v1", &store ), v2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( r1->id(), &store ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromStore( v1->id(), &store ), v1 );
}

void TestQgsProcessing::mapLayerFromString()
{
  // test mapLayerFromString

  QgsProcessingContext c;
  QgsProject p;

  // add a bunch of layers to a project
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QVERIFY( r1->isValid() );
  QFileInfo fi2( raster2 );
  QgsRasterLayer *r2 = new QgsRasterLayer( fi2.filePath(), "ar2" );
  QVERIFY( r2->isValid() );

  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon", "V4", "memory" );
  QgsVectorLayer *v2 = new QgsVectorLayer( "Point", "v1", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << r1 << r2 << v1 << v2 );

  // no project set yet
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QString(), c ) );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QStringLiteral( "v1" ), c ) );

  c.setProject( &p );

  // layers from current project
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( QString(), c ) );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( raster1, c ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( raster2, c ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "R1", c ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "ar2", c ), r2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V4", c ), v1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "v1", c ), v2 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( r1->id(), c ), r1 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v1->id(), c ), v1 );

  // check that layers in context temporary store are used
  QgsVectorLayer *v5 = new QgsVectorLayer( "Polygon", "V5", "memory" );
  QgsVectorLayer *v6 = new QgsVectorLayer( "Point", "v6", "memory" );
  c.temporaryLayerStore()->addMapLayers( QList<QgsMapLayer *>() << v5 << v6 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "V5", c ), v5 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( "v6", c ), v6 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v5->id(), c ), v5 );
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( v6->id(), c ), v6 );
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( "aaaaa", c ) );

  // if specified, check that layers can be loaded
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( "aaaaa", c ) );
  QString newRaster = testDataDir + "requires_warped_vrt.tif";
  // don't allow loading
  QVERIFY( ! QgsProcessingUtils::mapLayerFromString( newRaster, c, false ) );
  // allow loading
  QgsMapLayer *loadedLayer = QgsProcessingUtils::mapLayerFromString( newRaster, c, true );
  QVERIFY( loadedLayer->isValid() );
  QCOMPARE( loadedLayer->type(), QgsMapLayer::RasterLayer );
  // should now be in temporary store
  QCOMPARE( c.temporaryLayerStore()->mapLayer( loadedLayer->id() ), loadedLayer );

  // since it's now in temporary store, should be accessible even if we deny loading new layers
  QCOMPARE( QgsProcessingUtils::mapLayerFromString( newRaster, c, false ), loadedLayer );
}

void TestQgsProcessing::algorithm()
{
  DummyAlgorithm alg( "test" );
  DummyProvider *p = new DummyProvider( "p1" );
  QCOMPARE( alg.id(), QString( "test" ) );
  alg.setProvider( p );
  QCOMPARE( alg.provider(), p );
  QCOMPARE( alg.id(), QString( "p1:test" ) );

  QVERIFY( p->algorithms().isEmpty() );

  QSignalSpy providerRefreshed( p, &DummyProvider::algorithmsLoaded );
  p->refreshAlgorithms();
  QCOMPARE( providerRefreshed.count(), 1 );

  for ( int i = 0; i < 2; ++i )
  {
    QCOMPARE( p->algorithms().size(), 2 );
    QCOMPARE( p->algorithm( "alg1" )->name(), QStringLiteral( "alg1" ) );
    QCOMPARE( p->algorithm( "alg1" )->provider(), p );
    QCOMPARE( p->algorithm( "alg2" )->provider(), p );
    QCOMPARE( p->algorithm( "alg2" )->name(), QStringLiteral( "alg2" ) );
    QVERIFY( !p->algorithm( "aaaa" ) );
    QVERIFY( p->algorithms().contains( p->algorithm( "alg1" ) ) );
    QVERIFY( p->algorithms().contains( p->algorithm( "alg2" ) ) );

    // reload, then retest on next loop
    // must be safe for providers to reload their algorithms
    p->refreshAlgorithms();
    QCOMPARE( providerRefreshed.count(), 2 + i );
  }

  QgsProcessingRegistry r;
  r.addProvider( p );
  QCOMPARE( r.algorithms().size(), 2 );
  QVERIFY( r.algorithms().contains( p->algorithm( "alg1" ) ) );
  QVERIFY( r.algorithms().contains( p->algorithm( "alg2" ) ) );

  // algorithmById
  QCOMPARE( r.algorithmById( "p1:alg1" ), p->algorithm( "alg1" ) );
  QCOMPARE( r.algorithmById( "p1:alg2" ), p->algorithm( "alg2" ) );
  QVERIFY( !r.algorithmById( "p1:alg3" ) );
  QVERIFY( !r.algorithmById( "px:alg1" ) );

  //test that loading a provider triggers an algorithm refresh
  DummyProvider *p2 = new DummyProvider( "p2" );
  QVERIFY( p2->algorithms().isEmpty() );
  p2->load();
  QCOMPARE( p2->algorithms().size(), 2 );

  // test that adding a provider to the registry automatically refreshes algorithms (via load)
  DummyProvider *p3 = new DummyProvider( "p3" );
  QVERIFY( p3->algorithms().isEmpty() );
  r.addProvider( p3 );
  QCOMPARE( p3->algorithms().size(), 2 );
}

void TestQgsProcessing::features()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point", "v1", "memory" );
  for ( int i = 1; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setGeometry( QgsGeometry( new QgsPointV2( 1, 2 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProcessingContext context;
  // disable check for geometry validity
  context.setFlags( QgsProcessingContext::Flags( 0 ) );

  std::function< QgsFeatureIds( QgsFeatureIterator it ) > getIds = []( QgsFeatureIterator it )
  {
    QgsFeature f;
    QgsFeatureIds ids;
    while ( it.nextFeature( f ) )
    {
      ids << f.id();
    }
    return ids;
  };

  // test with all features
  QgsFeatureIds ids = getIds( QgsProcessingUtils::getFeatures( layer, context ) );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( QgsProcessingUtils::featureCount( layer, context ), 5L );

  // test with selected features
  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  ids = getIds( QgsProcessingUtils::getFeatures( layer, context ) );
  QCOMPARE( ids, QgsFeatureIds() << 2 << 4 );
  QCOMPARE( QgsProcessingUtils::featureCount( layer, context ), 2L );

  // selection, but not using selected features
  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  ids = getIds( QgsProcessingUtils::getFeatures( layer, context ) );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( QgsProcessingUtils::featureCount( layer, context ), 5L );

  // using selected features, but no selection
  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  layer->removeSelection();
  ids = getIds( QgsProcessingUtils::getFeatures( layer, context ) );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( QgsProcessingUtils::featureCount( layer, context ), 5L );


  // test that feature request is honored
  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  ids = getIds( QgsProcessingUtils::getFeatures( layer, context, QgsFeatureRequest().setFilterFids( QgsFeatureIds() << 1 << 3 << 5 ) ) );
  QCOMPARE( ids, QgsFeatureIds() << 1 << 3 << 5 );

  // count is only rough - but we expect (for now) to see full layer count
  QCOMPARE( QgsProcessingUtils::featureCount( layer, context ), 5L );


  //test that feature request is honored when using selections
  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  layer->selectByIds( QgsFeatureIds() << 2 << 4 );
  ids = getIds( QgsProcessingUtils::getFeatures( layer, context, QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) ) );
  QCOMPARE( ids, QgsFeatureIds() << 2 << 4 );

  // test callback is hit when filtering invalid geoms
  bool encountered = false;
  std::function< void( const QgsFeature & ) > callback = [ &encountered ]( const QgsFeature & )
  {
    encountered = true;
  };

  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometryAbortOnInvalid );
  context.setInvalidGeometryCallback( callback );
  QgsVectorLayer *polyLayer = new QgsVectorLayer( "Polygon", "v2", "memory" );
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 1 0, 0 1, 1 1, 0 0))" ) ) );
  polyLayer->dataProvider()->addFeatures( QgsFeatureList() << f );

  ids = getIds( QgsProcessingUtils::getFeatures( polyLayer, context ) );
  QVERIFY( encountered );

  encountered = false;
  context.setInvalidGeometryCheck( QgsFeatureRequest::GeometryNoCheck );
  ids = getIds( QgsProcessingUtils::getFeatures( polyLayer, context ) );
  QVERIFY( !encountered );

  delete layer;
  delete polyLayer;
}

void TestQgsProcessing::uniqueValues()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point?field=a:integer&field=b:string", "v1", "memory" );
  for ( int i = 0; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setAttributes( QgsAttributes() << i % 3 + 1 << QString( QChar( ( i % 3 ) + 65 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProcessingContext context;
  context.setFlags( QgsProcessingContext::Flags( 0 ) );

  // some bad checks
  QVERIFY( QgsProcessingUtils::uniqueValues( nullptr, 0, context ).isEmpty() );
  QVERIFY( QgsProcessingUtils::uniqueValues( nullptr, -1, context ).isEmpty() );
  QVERIFY( QgsProcessingUtils::uniqueValues( nullptr, 10001, context ).isEmpty() );
  QVERIFY( QgsProcessingUtils::uniqueValues( layer, -1, context ).isEmpty() );
  QVERIFY( QgsProcessingUtils::uniqueValues( layer, 10001, context ).isEmpty() );

  // good checks
  QList< QVariant > vals = QgsProcessingUtils::uniqueValues( layer, 0, context );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  QVERIFY( vals.contains( 3 ) );
  vals = QgsProcessingUtils::uniqueValues( layer, 1, context );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );
  QVERIFY( vals.contains( QString( "C" ) ) );

  //using only selected features
  layer->selectByIds( QgsFeatureIds() << 1 << 2 << 4 );
  // but not using selection yet...
  vals = QgsProcessingUtils::uniqueValues( layer, 0, context );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  QVERIFY( vals.contains( 3 ) );
  vals = QgsProcessingUtils::uniqueValues( layer, 1, context );
  QCOMPARE( vals.count(), 3 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );
  QVERIFY( vals.contains( QString( "C" ) ) );

  // selection and using selection
  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  QVERIFY( QgsProcessingUtils::uniqueValues( layer, -1, context ).isEmpty() );
  QVERIFY( QgsProcessingUtils::uniqueValues( layer, 10001, context ).isEmpty() );
  vals = QgsProcessingUtils::uniqueValues( layer, 0, context );
  QCOMPARE( vals.count(), 2 );
  QVERIFY( vals.contains( 1 ) );
  QVERIFY( vals.contains( 2 ) );
  vals = QgsProcessingUtils::uniqueValues( layer, 1, context );
  QCOMPARE( vals.count(), 2 );
  QVERIFY( vals.contains( QString( "A" ) ) );
  QVERIFY( vals.contains( QString( "B" ) ) );

  delete layer;
}

void TestQgsProcessing::createIndex()
{
  QgsVectorLayer *layer = new QgsVectorLayer( "Point", "v1", "memory" );
  for ( int i = 1; i < 6; ++i )
  {
    QgsFeature f( i );
    f.setGeometry( QgsGeometry( new QgsPointV2( i, 2 ) ) );
    layer->dataProvider()->addFeatures( QgsFeatureList() << f );
  }

  QgsProcessingContext context;
  // disable selected features check
  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  QgsSpatialIndex index = QgsProcessingUtils::createSpatialIndex( layer, context );
  QList<QgsFeatureId> ids = index.nearestNeighbor( QgsPoint( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 2 );

  // selected features check, but none selected
  context.setFlags( QgsProcessingContext::UseSelectionIfPresent );
  index = QgsProcessingUtils::createSpatialIndex( layer, context );
  ids = index.nearestNeighbor( QgsPoint( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 2 );

  // create selection
  layer->selectByIds( QgsFeatureIds() << 4 << 5 );
  index = QgsProcessingUtils::createSpatialIndex( layer, context );
  ids = index.nearestNeighbor( QgsPoint( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 4 );

  // selection but not using selection mode
  context.setFlags( QgsProcessingContext::Flags( 0 ) );
  index = QgsProcessingUtils::createSpatialIndex( layer, context );
  ids = index.nearestNeighbor( QgsPoint( 2.1, 2 ), 1 );
  QCOMPARE( ids, QList<QgsFeatureId>() << 2 );

}

void TestQgsProcessing::createFeatureSink()
{
  QgsProcessingContext context;

  // empty destination
  QString destination;
  destination = QString();
  QgsVectorLayer *layer = nullptr;

  // should create a memory layer
  QgsFeatureSink *sink = QgsProcessingUtils::createFeatureSink( destination, QString(), QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem(), context );
  QVERIFY( sink );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QgsFeature f;
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;
  delete sink;

  // specific memory layer output
  destination = QStringLiteral( "memory:mylayer" );
  sink = QgsProcessingUtils::createFeatureSink( destination, QString(), QgsFields(), QgsWkbTypes::Point, QgsCoordinateReferenceSystem(), context );
  QVERIFY( sink );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( layer->name(), QStringLiteral( "memory:mylayer" ) );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;
  delete sink;

  // memory layer parameters
  destination = QStringLiteral( "memory:mylayer" );
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "my_field" ), QVariant::String, QString(), 100 ) );
  sink = QgsProcessingUtils::createFeatureSink( destination, QString(), fields, QgsWkbTypes::PointZM, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ), context );
  QVERIFY( sink );
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, false ) );
  QVERIFY( layer );
  QCOMPARE( static_cast< QgsProxyFeatureSink *>( sink )->destinationSink(), layer->dataProvider() );
  QCOMPARE( layer->dataProvider()->name(), QStringLiteral( "memory" ) );
  QCOMPARE( layer->name(), QStringLiteral( "memory:mylayer" ) );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::PointZM );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 0 ).type(), QVariant::String );
  QCOMPARE( destination, layer->id() );
  QCOMPARE( context.temporaryLayerStore()->mapLayer( layer->id() ), layer ); // layer should be in store
  QCOMPARE( layer->featureCount(), 0L );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( layer->featureCount(), 1L );
  context.temporaryLayerStore()->removeAllMapLayers();
  layer = nullptr;
  delete sink;

  // non memory layer output
  destination = QDir::tempPath() + "/create_feature_sink.tab";
  QString prevDest = destination;
  sink = QgsProcessingUtils::createFeatureSink( destination, QString(), fields, QgsWkbTypes::Polygon, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ), context );
  QVERIFY( sink );
  f = QgsFeature( fields );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "Polygon((0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  f.setAttributes( QgsAttributes() << "val" );
  QVERIFY( sink->addFeature( f ) );
  QCOMPARE( destination, prevDest );
  delete sink;
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 0 ).type(), QVariant::String );
  QCOMPARE( layer->featureCount(), 1L );
  delete layer;
  layer = nullptr;

  // no extension, should default to shp
  destination = QDir::tempPath() + "/create_feature_sink2";
  prevDest = QDir::tempPath() + "/create_feature_sink2.shp";
  sink = QgsProcessingUtils::createFeatureSink( destination, QString(), fields, QgsWkbTypes::Point25D, QgsCoordinateReferenceSystem::fromEpsgId( 3111 ), context );
  QVERIFY( sink );
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PointZ(1 2 3)" ) ) );
  QVERIFY( sink->addFeature( f ) );
  QVERIFY( !layer );
  QCOMPARE( destination, prevDest );
  delete sink;
  layer = qobject_cast< QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( destination, context, true ) );
  QCOMPARE( layer->wkbType(), QgsWkbTypes::Point25D );
  QCOMPARE( layer->crs().authid(), QStringLiteral( "EPSG:3111" ) );
  QCOMPARE( layer->fields().size(), 1 );
  QCOMPARE( layer->fields().at( 0 ).name(), QStringLiteral( "my_field" ) );
  QCOMPARE( layer->fields().at( 0 ).type(), QVariant::String );
  QCOMPARE( layer->featureCount(), 1L );
  delete layer;
  layer = nullptr;
}

QGSTEST_MAIN( TestQgsProcessing )
#include "testqgsprocessing.moc"
