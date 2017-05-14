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
#include "qgstestutils.h"
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
    virtual QVariantMap run( const QVariantMap &,
                             QgsProcessingContext &, QgsProcessingFeedback * ) const override { return QVariantMap(); }
    QString mName;

    void runParameterChecks()
    {
      QVERIFY( parameterDefinitions().isEmpty() );
      QVERIFY( addParameter( new QgsProcessingParameterBoolean( "p1" ) ) );
      QCOMPARE( parameterDefinitions().count(), 1 );
      QCOMPARE( parameterDefinitions().at( 0 )->name(), QString( "p1" ) );

      QVERIFY( !addParameter( nullptr ) );
      QCOMPARE( parameterDefinitions().count(), 1 );
      // duplicate name!
      QgsProcessingParameterBoolean *p2 = new QgsProcessingParameterBoolean( "p1" );
      QVERIFY( !addParameter( p2 ) );
      delete p2;
      QCOMPARE( parameterDefinitions().count(), 1 );

      QCOMPARE( parameterDefinition( "p1" ), parameterDefinitions().at( 0 ) );
      // parameterDefinition should be case insensitive
      QCOMPARE( parameterDefinition( "P1" ), parameterDefinitions().at( 0 ) );
      QVERIFY( !parameterDefinition( "invalid" ) );

      QCOMPARE( countVisibleParameters(), 1 );
      QgsProcessingParameterBoolean *p3 = new QgsProcessingParameterBoolean( "p3" );
      QVERIFY( addParameter( p3 ) );
      QCOMPARE( countVisibleParameters(), 2 );
      QgsProcessingParameterBoolean *p4 = new QgsProcessingParameterBoolean( "p4" );
      p4->setFlags( QgsProcessingParameterDefinition::FlagHidden );
      QVERIFY( addParameter( p4 ) );
      QCOMPARE( countVisibleParameters(), 2 );
    }

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
    void parameters();
    void algorithmParameters();
    void parameterGeneral();
    void parameterBoolean();
    void parameterCrs();
    void parameterLayer();
    void parameterExtent();
    void parameterPoint();
    void parameterFile();
    void parameterMatrix();
    void parameterLayerList();
    void parameterNumber();
    void parameterRange();
    void parameterRasterLayer();
    void parameterEnum();
    void parameterString();
    void parameterExpression();
    void parameterField();
    void parameterVectorLayer();

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

void TestQgsProcessing::parameters()
{
  // test parameter utilities

  QgsProcessingParameterDefinition *def = nullptr;;
  QVariantMap params;
  params.insert( QStringLiteral( "prop" ), QgsProperty::fromField( "a_field" ) );
  params.insert( QStringLiteral( "string" ), QStringLiteral( "a string" ) );
  params.insert( QStringLiteral( "double" ), 5.2 );
  params.insert( QStringLiteral( "int" ), 15 );
  params.insert( QStringLiteral( "bool" ), true );

  QgsProcessingContext context;

  // isDynamic
  QVERIFY( QgsProcessingParameters::isDynamic( params, QStringLiteral( "prop" ) ) );
  QVERIFY( !QgsProcessingParameters::isDynamic( params, QStringLiteral( "string" ) ) );
  QVERIFY( !QgsProcessingParameters::isDynamic( params, QStringLiteral( "bad" ) ) );

  // parameterAsString
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "string" ), context ), QStringLiteral( "a string" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "double" ), context ).left( 3 ), QStringLiteral( "5.2" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "int" ), context ), QStringLiteral( "15" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "bool" ), context ), QStringLiteral( "true" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "bad" ), context ), QString() );

  // string with dynamic property (feature not set)
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "prop" ), context ), QString() );

  // correctly setup feature
  QgsFields fields;
  fields.append( QgsField( "a_field", QVariant::String ) );
  QgsFeature f( fields );
  f.setAttribute( 0, QStringLiteral( "field value" ) );
  context.expressionContext().setFeature( f );
  context.expressionContext().setFields( fields );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "prop" ), context ), QStringLiteral( "field value" ) );

  // as double
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "double" ), context ), 5.2 );
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "int" ), context ), 15.0 );
  f.setAttribute( 0, QStringLiteral( "6.2" ) );
  context.expressionContext().setFeature( f );
  QCOMPARE( QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "prop" ), context ), 6.2 );

  // as int
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "double" ), context ), 5 );
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "int" ), context ), 15 );
  QCOMPARE( QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "prop" ), context ), 6 );

  // as bool
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "double" ), context ), true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "int" ), context ), true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "bool" ), context ), true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "prop" ), context ), true );
  f.setAttribute( 0, false );
  context.expressionContext().setFeature( f );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "prop" ), context ), false );

  // as layer
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "double" ), context ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def,  params, QStringLiteral( "int" ), context ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "bool" ), context ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "prop" ), context ) );

  QVERIFY( context.temporaryLayerStore()->mapLayers().isEmpty() );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  f.setAttribute( 0, testDataDir + "/raster/band1_float32_noct_epsg4326.tif" );
  context.expressionContext().setFeature( f );
  QVERIFY( QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "prop" ), context ) );
  // make sure layer was loaded
  QVERIFY( !context.temporaryLayerStore()->mapLayers().isEmpty() );
}

void TestQgsProcessing::algorithmParameters()
{
  DummyAlgorithm alg( "test" );
  alg.runParameterChecks();
}

void TestQgsProcessing::parameterGeneral()
{
  // test constructor
  QgsProcessingParameterBoolean param( "p1", "desc", true, true );
  QCOMPARE( param.name(), QString( "p1" ) );
  QCOMPARE( param.description(), QString( "desc" ) );
  QCOMPARE( param.defaultValue(), QVariant( true ) );
  QVERIFY( param.flags() & QgsProcessingParameterDefinition::FlagOptional );

  // test getters and setters
  param.setDescription( "p2" );
  QCOMPARE( param.description(), QString( "p2" ) );
  param.setDefaultValue( false );
  QCOMPARE( param.defaultValue(), QVariant( false ) );
  param.setFlags( QgsProcessingParameterDefinition::FlagHidden );
  QCOMPARE( param.flags(), QgsProcessingParameterDefinition::FlagHidden );
  param.setDefaultValue( true );
  QCOMPARE( param.defaultValue(), QVariant( true ) );
  param.setDefaultValue( QVariant() );
  QCOMPARE( param.defaultValue(), QVariant() );
}

void TestQgsProcessing::parameterBoolean()
{
  QgsProcessingContext context;

  // test no def
  QVariantMap params;
  params.insert( "no_def",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), false );
  params.insert( "no_def",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), true );
  params.insert( "no_def",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), true );
  params.insert( "no_def",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), false );
  params.insert( "no_def",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), false );
  params.remove( "no_def" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( nullptr, params, QStringLiteral( "no_def" ), context ), false );

  // with defs

  QgsProcessingParameterDefinition *def = new QgsProcessingParameterBoolean( "non_optional_default_false" );
  params.insert( "non_optional_default_false",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), false );
  params.insert( "non_optional_default_false",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), true );
  params.insert( "non_optional_default_false",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), true );
  params.insert( "non_optional_default_false",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), false );

  //non-optional - behavior is undefined, but internally default to false
  params.insert( "non_optional_default_false",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), false );
  params.remove( "non_optional_default_false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_false" ), context ), false );

  delete def;
  def = new QgsProcessingParameterBoolean( "optional_default_true", QString(), true, true );
  params.insert( "optional_default_true",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), false );
  params.insert( "optional_default_true",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), true );
  params.insert( "optional_default_true",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), true );
  params.insert( "optional_default_true",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), false );
  //optional - should be default
  params.insert( "optional_default_true",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), true );
  params.remove( "optional_default_true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_true" ), context ), true );

  delete def;
  def = new QgsProcessingParameterBoolean( "optional_default_false", QString(), false, true );
  params.insert( "optional_default_false",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), false );
  params.insert( "optional_default_false",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), true );
  params.insert( "optional_default_false",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), true );
  params.insert( "optional_default_false",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), false );
  //optional - should be default
  params.insert( "optional_default_false",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), false );
  params.remove( "optional_default_false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "optional_default_false" ), context ), false );

  delete def;
  def = new QgsProcessingParameterBoolean( "non_optional_default_true", QString(), true, false );
  params.insert( "non_optional_default_true",  false );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), false );
  params.insert( "non_optional_default_true",  true );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), true );
  params.insert( "non_optional_default_true",  "true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), true );
  params.insert( "non_optional_default_true",  "false" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), false );
  //non-optional - behavior is undefined, but internally fallback to default
  params.insert( "non_optional_default_true",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), true );
  params.remove( "non_optional_default_true" );
  QCOMPARE( QgsProcessingParameters::parameterAsBool( def, params, QStringLiteral( "non_optional_default_true" ), context ), true );
  delete def;
}

void TestQgsProcessing::parameterCrs()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterCrs *def = new QgsProcessingParameterCrs( "non_optional", QString(), QString( "EPSG:3113" ), false );

  // using map layer
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).authid(), QString( "EPSG:3111" ) );

  // special ProjectCrs string
  params.insert( "non_optional",  QStringLiteral( "ProjectCrs" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).authid(), QString( "EPSG:28353" ) );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).authid(), QString( "EPSG:4326" ) );
  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).authid(), QString( "EPSG:32633" ) );

  // string representation of a crs
  params.insert( "non_optional", QString( "EPSG:28355" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).authid(), QString( "EPSG:28355" ) );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "non_optional" ), context ).isValid() );

  // optional
  delete def;
  def = new QgsProcessingParameterCrs( "optional", QString(), QString( "EPSG:3113" ), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsCrs( def, params, QStringLiteral( "optional" ), context ).authid(), QString( "EPSG:3113" ) );
}

void TestQgsProcessing::parameterLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterMapLayer *def = new QgsProcessingParameterMapLayer( "non_optional", QString(), QString( "EPSG:3113" ), false );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "non_optional" ), context )->id(), v1->id() );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "non_optional" ), context )->id(), r1->id() );
  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "non_optional" ), context )->publicSource(), raster2 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "non_optional" ), context ) );

  // optional
  delete def;
  def = new QgsProcessingParameterMapLayer( "optional", QString(), v1->id(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayer( def, params, QStringLiteral( "optional" ), context )->id(), v1->id() );
}

void TestQgsProcessing::parameterExtent()
{
  // setup a context
  QgsProject p;
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  p.addMapLayers( QList<QgsMapLayer *>() << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterExtent *def = new QgsProcessingParameterExtent( "non_optional", QString(), QString( "1,2,3,4" ), false );

  // using map layer
  QVariantMap params;
  params.insert( "non_optional",  r1->id() );
  QgsRectangle ext = QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( ext, r1->extent() );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "non_optional" ), context ),  r1->extent() );

  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  ext = QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( ext.xMinimum(), 781662.375000, 10 );
  QGSCOMPARENEAR( ext.xMaximum(), 793062.375000, 10 );
  QGSCOMPARENEAR( ext.yMinimum(),  3339523.125000, 10 );
  QGSCOMPARENEAR( ext.yMaximum(), 3350923.125000, 10 );

  // string representation of an extent
  params.insert( "non_optional", QString( "1.1,2.2,3.3,4.4" ) );
  ext = QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( ext.xMinimum(), 1.1, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 2.2, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(),  3.3, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 4.4, 0.001 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  QVERIFY( QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "non_optional" ), context ).isNull() );

  // optional
  delete def;
  def = new QgsProcessingParameterExtent( "optional", QString(), QString( "5,6,7,8" ), true );
  params.insert( "optional",  QVariant() );
  ext = QgsProcessingParameters::parameterAsExtent( def, params, QStringLiteral( "optional" ), context );
  QGSCOMPARENEAR( ext.xMinimum(), 5.0, 0.001 );
  QGSCOMPARENEAR( ext.xMaximum(), 6.0, 0.001 );
  QGSCOMPARENEAR( ext.yMinimum(), 7.0, 0.001 );
  QGSCOMPARENEAR( ext.yMaximum(), 8.0, 0.001 );
}

void TestQgsProcessing::parameterPoint()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterPoint *def = new QgsProcessingParameterPoint( "non_optional", QString(), QString( "1,2" ), false );

  // string representing a point
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1,2.2" ) );
  QgsPoint point = QgsProcessingParameters::parameterAsPoint( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( point.x(), 1.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 2.2, 0.001 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a crs, and nothing you can do will make me one" ) );
  point = QgsProcessingParameters::parameterAsPoint( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( point.x(), 0.0 );
  QCOMPARE( point.y(), 0.0 );

  // optional
  delete def;
  def = new QgsProcessingParameterPoint( "optional", QString(), QString( "5.1,6.2" ), true );
  params.insert( "optional",  QVariant() );
  point = QgsProcessingParameters::parameterAsPoint( def, params, QStringLiteral( "optional" ), context );
  QGSCOMPARENEAR( point.x(), 5.1, 0.001 );
  QGSCOMPARENEAR( point.y(), 6.2, 0.001 );
}

void TestQgsProcessing::parameterFile()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterFile *def = new QgsProcessingParameterFile( "non_optional", QString(), QgsProcessingParameterFile::File, QString(), QString( "abc.bmp" ), false );

  // string representing a file
  QVariantMap params;
  params.insert( "non_optional", QString( "def.bmp" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsFile( def, params, QStringLiteral( "non_optional" ), context ), QString( "def.bmp" ) );

  // optional
  delete def;
  def = new QgsProcessingParameterFile( "optional", QString(), QgsProcessingParameterFile::File, QString(), QString( "gef.bmp" ),  true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsFile( def, params, QStringLiteral( "optional" ), context ), QString( "gef.bmp" ) );
}

void TestQgsProcessing::parameterMatrix()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterMatrix *def = new QgsProcessingParameterMatrix( "non_optional", QString(), 3, false, QStringList(), QString( ), false );

  // list
  QVariantMap params;
  params.insert( "non_optional", QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def, params, QStringLiteral( "non_optional" ), context ), QVariantList() << 1 << 2 << 3 );

  //string
  params.insert( "non_optional", QString( "4,5,6" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def, params, QStringLiteral( "non_optional" ), context ), QVariantList() << 4 << 5 << 6 );

  // optional
  delete def;
  def = new QgsProcessingParameterMatrix( "optional", QString(), 3, false, QStringList(), QVariantList() << 4 << 5 << 6,  true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def, params, QStringLiteral( "optional" ), context ), QVariantList() << 4 << 5 << 6 );
  delete def;
  def = new QgsProcessingParameterMatrix( "optional", QString(), 3, false, QStringList(), QString( "1,2,3" ),  true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsMatrix( def, params, QStringLiteral( "optional" ), context ), QVariantList() << 1 << 2 << 3 );
}

void TestQgsProcessing::parameterLayerList()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterMultipleLayers *def = new QgsProcessingParameterMultipleLayers( "non_optional", QString(), QgsProcessingParameterDefinition::TypeAny, QString(), false );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "non_optional" ), context ), QList< QgsMapLayer *>() << v1 );

  // using two existing map layer ID
  params.insert( "non_optional",  QVariantList() << v1->id() << r1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "non_optional" ), context ), QList< QgsMapLayer *>() << v1 << r1 );

  // mix of existing layers and non project layer string
  params.insert( "non_optional",  QVariantList() << v1->id() << raster2 );
  QList< QgsMapLayer *> layers = QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( layers.at( 0 ), v1 );
  QCOMPARE( layers.at( 1 )->publicSource(), raster2 );

  // empty string
  params.insert( "non_optional",  QString( "" ) );
  QVERIFY( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "non_optional" ), context ).isEmpty() );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "non_optional" ), context ).isEmpty() );

  // optional with one default layer
  delete def;
  def = new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessingParameterDefinition::TypeAny, v1->id(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "optional" ), context ), QList< QgsMapLayer *>() << v1 );

  // optional with two default layers
  delete def;
  def = new QgsProcessingParameterMultipleLayers( "optional", QString(), QgsProcessingParameterDefinition::TypeAny, QVariantList() << v1->id() << r1->publicSource(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsLayerList( def, params, QStringLiteral( "optional" ), context ), QList< QgsMapLayer *>() << v1 << r1 );
}

void TestQgsProcessing::parameterNumber()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterNumber *def = new QgsProcessingParameterNumber( "non_optional", QString(), QgsProcessingParameterNumber::Double, 5, false );

  // string representing a number
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1" ) );
  double number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );
  int iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 1 );

  // double
  params.insert( "non_optional", 1.1 );
  number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( number, 1.1, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 1 );

  // int
  params.insert( "non_optional", 1 );
  number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( number, 1, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a number, and nothing you can do will make me one" ) );
  number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( number, 5.0 );
  iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 5 );

  // optional
  delete def;
  def = new QgsProcessingParameterNumber( "optional", QString(), QgsProcessingParameterNumber::Double, 5.4, true );
  params.insert( "optional",  QVariant() );
  number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "optional" ), context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumber, 5 );
  // unconvertible string
  params.insert( "optional",  QVariant( "aaaa" ) );
  number = QgsProcessingParameters::parameterAsDouble( def, params, QStringLiteral( "optional" ), context );
  QGSCOMPARENEAR( number, 5.4, 0.001 );
  iNumber = QgsProcessingParameters::parameterAsInt( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumber, 5 );
}

void TestQgsProcessing::parameterRange()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterRange *def = new QgsProcessingParameterRange( "non_optional", QString(), QgsProcessingParameterNumber::Double, QString( "5,6" ), false );

  // string representing a range of numbers
  QVariantMap params;
  params.insert( "non_optional", QString( "1.1,1.2" ) );
  QList< double > range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // list
  params.insert( "non_optional", QVariantList() << 1.1 << 1.2 );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // too many elements:
  params.insert( "non_optional", QString( "1.1,1.2,1.3" ) );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );
  params.insert( "non_optional", QVariantList() << 1.1 << 1.2 << 1.3 );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );
  QGSCOMPARENEAR( range.at( 0 ), 1.1, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 1.2, 0.001 );

  // not enough elements - don't care about the result, just don't crash!
  params.insert( "non_optional", QString( "1.1" ) );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );
  params.insert( "non_optional", QVariantList() << 1.1 );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "non_optional" ), context );

  // optional
  delete def;
  def = new QgsProcessingParameterRange( "optional", QString(), QgsProcessingParameterNumber::Double, QString( "5.4,7.4" ), true );
  params.insert( "optional",  QVariant() );
  range = QgsProcessingParameters::parameterAsRange( def, params, QStringLiteral( "optional" ), context );
  QGSCOMPARENEAR( range.at( 0 ), 5.4, 0.001 );
  QGSCOMPARENEAR( range.at( 1 ), 7.4, 0.001 );
}

void TestQgsProcessing::parameterRasterLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString raster1 = testDataDir + "tenbytenraster.asc";
  QString raster2 = testDataDir + "landsat.tif";
  QFileInfo fi1( raster1 );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterRasterLayer *def = new QgsProcessingParameterRasterLayer( "non_optional", QString(), QString( "EPSG:3113" ), false );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  r1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "non_optional" ), context )->id(), r1->id() );

  // not raster layer
  params.insert( "non_optional",  v1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "non_optional" ), context ) );

  // string representing a project layer source
  params.insert( "non_optional", raster1 );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "non_optional" ), context )->id(), r1->id() );
  // string representing a non-project layer source
  params.insert( "non_optional", raster2 );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "non_optional" ), context )->publicSource(), raster2 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "non_optional" ), context ) );

  // optional
  delete def;
  def = new QgsProcessingParameterRasterLayer( "optional", QString(), r1->id(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsRasterLayer( def, params, QStringLiteral( "optional" ), context )->id(), r1->id() );
}

void TestQgsProcessing::parameterEnum()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterEnum *def = new QgsProcessingParameterEnum( "non_optional", QString(), QStringList() << "A" << "B" << "C", false, 2, false );

  // string representing a number
  QVariantMap params;
  params.insert( "non_optional", QString( "1" ) );
  int iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 1 );

  // double
  params.insert( "non_optional", 2.2 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 2 );

  // int
  params.insert( "non_optional", 1 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a number, and nothing you can do will make me one" ) );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 2 );

  // out of range
  params.insert( "non_optional", 4 );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumber, 2 );

  // multiple
  def = new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "A" << "B" << "C", true, 5, false );
  params.insert( "non_optional", QString( "1,2" ) );
  QList< int > iNumbers = QgsProcessingParameters::parameterAsEnums( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
  params.insert( "non_optional", QVariantList() << 0 << 2 );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( iNumbers, QList<int>() << 0 << 2 );

  // optional
  delete def;
  def = new QgsProcessingParameterEnum( "optional", QString(), QStringList(), false, 5, true );
  params.insert( "optional",  QVariant() );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumber, 5 );
  // unconvertible string
  params.insert( "optional",  QVariant( "aaaa" ) );
  iNumber = QgsProcessingParameters::parameterAsEnum( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumber, 5 );
  //optional with multiples
  delete def;
  def = new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "A" << "B" << "C", true, QVariantList() << 1 << 2, true );
  params.insert( "optional",  QVariant() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
  def = new QgsProcessingParameterEnum( "optional", QString(), QStringList() << "A" << "B" << "C", true, "1,2", true );
  params.insert( "optional",  QVariant() );
  iNumbers = QgsProcessingParameters::parameterAsEnums( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( iNumbers, QList<int>() << 1 << 2 );
}

void TestQgsProcessing::parameterString()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterString *def = new QgsProcessingParameterString( "non_optional", QString(), QString(), false, false );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "abcdef" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "non_optional" ), context ), QString( "abcdef" ) );

  // optional
  delete def;
  def = new QgsProcessingParameterString( "optional", QString(), QString( "default" ), false, true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsString( def, params, QStringLiteral( "optional" ), context ), QString( "default" ) );
}

void TestQgsProcessing::parameterExpression()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterExpression *def = new QgsProcessingParameterExpression( "non_optional", QString(), QString(), QString(), false );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "abcdef" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def, params, QStringLiteral( "non_optional" ), context ), QString( "abcdef" ) );

  // optional
  delete def;
  def = new QgsProcessingParameterExpression( "optional", QString(), QString( "default" ), QString(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def, params, QStringLiteral( "optional" ), context ), QString( "default" ) );
  // valid expression, should not fallback
  params.insert( "optional",  QVariant( "1+2" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def, params, QStringLiteral( "optional" ), context ), QString( "1+2" ) );
  // invalid expression, should fallback
  params.insert( "optional",  QVariant( "1+" ) );
  QCOMPARE( QgsProcessingParameters::parameterAsExpression( def, params, QStringLiteral( "optional" ), context ), QString( "default" ) );
}

void TestQgsProcessing::parameterField()
{
  QgsProcessingContext context;

  // not optional!
  QgsProcessingParameterTableField *def = new QgsProcessingParameterTableField( "non_optional", QString(), QString(), QString(), QgsProcessingParameterTableField::Any, false, false );

  // string
  QVariantMap params;
  params.insert( "non_optional", QString( "a" ) );
  QStringList fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( fields, QStringList() << "a" );

  // multiple
  def = new QgsProcessingParameterTableField( "non_optional", QString(), QString(), QString(), QgsProcessingParameterTableField::Any, true, false );
  params.insert( "non_optional", QString( "a;b" ) );
  fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( fields, QStringList() << "a" << "b" );
  params.insert( "non_optional", QVariantList() << "a" << "b" );
  fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "non_optional" ), context );
  QCOMPARE( fields, QStringList() << "a" << "b" );

  // optional
  delete def;
  def = new QgsProcessingParameterTableField( "non_optional", QString(), QString( "def" ), QString(), QgsProcessingParameterTableField::Any, false, true );
  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( fields, QStringList() << "def" );

  //optional with multiples
  delete def;
  def = new QgsProcessingParameterTableField( "non_optional", QString(), QString( "abc;def" ), QString(), QgsProcessingParameterTableField::Any, true, true );
  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( fields, QStringList() << "abc" << "def" );
  delete def;
  def = new QgsProcessingParameterTableField( "non_optional", QString(), QVariantList() << "abc" << "def", QString(), QgsProcessingParameterTableField::Any, true, true );
  params.insert( "optional",  QVariant() );
  fields = QgsProcessingParameters::parameterAsFields( def, params, QStringLiteral( "optional" ), context );
  QCOMPARE( fields, QStringList() << "abc" << "def" );
}

void TestQgsProcessing::parameterVectorLayer()
{
  // setup a context
  QgsProject p;
  p.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 28353 ) );
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  QString vector1 = testDataDir + "multipoint.shp";
  QString raster = testDataDir + "landsat.tif";
  QFileInfo fi1( raster );
  QgsRasterLayer *r1 = new QgsRasterLayer( fi1.filePath(), "R1" );
  QgsVectorLayer *v1 = new QgsVectorLayer( "Polygon?crs=EPSG:3111", "V4", "memory" );
  p.addMapLayers( QList<QgsMapLayer *>() << v1 << r1 );
  QgsProcessingContext context;
  context.setProject( &p );

  // not optional!
  QgsProcessingParameterVector *def = new QgsProcessingParameterVector( "non_optional", QString(), QgsProcessingParameterDefinition::TypeVectorAny, QString( "EPSG:3113" ), false );

  // using existing map layer ID
  QVariantMap params;
  params.insert( "non_optional",  v1->id() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def, params, QStringLiteral( "non_optional" ), context )->id(), v1->id() );

  // not vector layer
  params.insert( "non_optional",  r1->id() );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def, params, QStringLiteral( "non_optional" ), context ) );

  // string representing a layer source
  params.insert( "non_optional", vector1 );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def, params, QStringLiteral( "non_optional" ), context )->publicSource(), vector1 );

  // nonsense string
  params.insert( "non_optional", QString( "i'm not a layer, and nothing you can do will make me one" ) );
  QVERIFY( !QgsProcessingParameters::parameterAsVectorLayer( def, params, QStringLiteral( "non_optional" ), context ) );

  // optional
  delete def;
  def = new QgsProcessingParameterVector( "optional", QString(), QgsProcessingParameterDefinition::TypeVectorAny, v1->id(), true );
  params.insert( "optional",  QVariant() );
  QCOMPARE( QgsProcessingParameters::parameterAsVectorLayer( def, params, QStringLiteral( "optional" ), context )->id(), v1->id() );
}

QGSTEST_MAIN( TestQgsProcessing )
#include "testqgsprocessing.moc"
