/***************************************************************************
    testqgsvectorlayerjoinbuffer.cpp
     --------------------------------------
    Date                 : September 2014
    Copyright            : (C) 2014 Martin Dobias
    Email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QtTest/QtTest>
#include <QObject>

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgsmaplayerregistry.h>
#include <qgslayerdefinition.h>
#include <qgsproject.h>

/** @ingroup UnitTests
 * This is a unit test for the vector layer join buffer
 *
 * @see QgsVectorLayerJoinBuffer
 */
class TestVectorLayerJoinBuffer : public QObject
{
    Q_OBJECT

  public:
    TestVectorLayerJoinBuffer()
        : mLayers( QMap<QPair<QString, QString>, QgsVectorLayer*>() )
    {}

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testJoinBasic_data();
    void testJoinBasic();
    void testJoinTransitive_data();
    void testJoinTransitive();
    void testJoinDetectCycle_data();
    void testJoinDetectCycle();
    void testJoinSubset_data();
    void testJoinSubset();
    void testJoinTwoTimes_data();
    void testJoinTwoTimes();
    void testJoinLayerDefinitionFile();
    void testCacheUpdate_data();
    void testCacheUpdate();

  private:
    QList<QString> mProviders;
    // map of layers. First key is the name of the layer A, B or C and second key is the provider memory or PG.
    QMap<QPair<QString, QString>, QgsVectorLayer*> mLayers;
};

// runs before all tests
void TestVectorLayerJoinBuffer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  mProviders = QList<QString>() << "memory";

  // Create memory layers
  // LAYER A //
  QgsVectorLayer* vlA = new QgsVectorLayer( "Point?field=id_a:integer", "A", "memory" );
  QVERIFY( vlA->isValid() );
  QVERIFY( vlA->fields().count() == 1 );
  // LAYER B //
  QgsVectorLayer* vlB = new QgsVectorLayer( "Point?field=id_b:integer&field=value_b", "B", "memory" );
  QVERIFY( vlB->isValid() );
  QVERIFY( vlB->fields().count() == 2 );
  // LAYER C //
  QgsVectorLayer* vlC = new QgsVectorLayer( "Point?field=id_c:integer&field=value_c", "C", "memory" );
  QVERIFY( vlC->isValid() );
  QVERIFY( vlC->fields().count() == 2 );
  // LAYER X //
  QgsVectorLayer* vlX = new QgsVectorLayer( "Point?field=id_x:integer&field=value_x1:integer&field=value_x2", "X", "memory" );
  QVERIFY( vlX->isValid() );
  QVERIFY( vlX->fields().count() == 3 );

  mLayers = QMap<QPair<QString, QString>, QgsVectorLayer*>();
  mLayers.insert( QPair<QString, QString>( "A", "memory" ), vlA );
  mLayers.insert( QPair<QString, QString>( "B", "memory" ), vlB );
  mLayers.insert( QPair<QString, QString>( "C", "memory" ), vlC );
  mLayers.insert( QPair<QString, QString>( "X", "memory" ), vlX );

  // Add PG layers
#ifdef ENABLE_PGTEST
  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "dbname='qgis_test'";
  }
  QgsVectorLayer* vlA_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_a' table=\"qgis_test\".\"table_a\" sql=" ).arg( dbConn ), "A_PG", "postgres" );
  QgsVectorLayer* vlB_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_b' table=\"qgis_test\".\"table_b\" sql=" ).arg( dbConn ), "B_PG", "postgres" );
  QgsVectorLayer* vlC_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_c' table=\"qgis_test\".\"table_c\" sql=" ).arg( dbConn ), "C_PG", "postgres" );
  QgsVectorLayer* vlX_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_x' table=\"qgis_test\".\"table_x\" sql=" ).arg( dbConn ), "X_PG", "postgres" );
  QVERIFY( vlA_PG->isValid() );
  QVERIFY( vlB_PG->isValid() );
  QVERIFY( vlC_PG->isValid() );
  QVERIFY( vlX_PG->isValid() );
  QVERIFY( vlA_PG->fields().count() == 1 );
  QVERIFY( vlB_PG->fields().count() == 2 );
  QVERIFY( vlC_PG->fields().count() == 2 );
  QVERIFY( vlX_PG->fields().count() == 3 );
  mLayers.insert( QPair<QString, QString>( "A", "PG" ), vlA_PG );
  mLayers.insert( QPair<QString, QString>( "B", "PG" ), vlB_PG );
  mLayers.insert( QPair<QString, QString>( "C", "PG" ), vlC_PG );
  mLayers.insert( QPair<QString, QString>( "X", "PG" ), vlX_PG );
  mProviders << "PG";
#endif

  // Create features
  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( "id_a", 1 );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( "id_a", 2 );
  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( "id_b", 1 );
  fB1.setAttribute( "value_b", 11 );
  QgsFeature fB2( vlB->dataProvider()->fields(), 2 );
  fB2.setAttribute( "id_b", 2 );
  fB2.setAttribute( "value_b", 12 );
  QgsFeature fC1( vlC->dataProvider()->fields(), 1 );
  fC1.setAttribute( "id_c", 1 );
  fC1.setAttribute( "value_c", 101 );
  QgsFeature fX1( vlX->dataProvider()->fields(), 1 );
  fX1.setAttribute( "id_x", 1 );
  fX1.setAttribute( "value_x1", 111 );
  fX1.setAttribute( "value_x2", 222 );

  // Commit features and layers to qgis
  Q_FOREACH ( const QString provider, mProviders )
  {
    QgsVectorLayer* vl = mLayers.value( QPair<QString, QString>( "A", provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );
    QVERIFY( vl->featureCount() == 2 );
    QgsMapLayerRegistry::instance()->addMapLayer( vl );
  }

  Q_FOREACH ( const QString provider, mProviders )
  {
    QgsVectorLayer* vl = mLayers.value( QPair<QString, QString>( "B", provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fB1 << fB2 );
    QVERIFY( vl->featureCount() == 2 );
    QgsMapLayerRegistry::instance()->addMapLayer( vl );
  }

  Q_FOREACH ( const QString provider, mProviders )
  {
    QgsVectorLayer* vl = mLayers.value( QPair<QString, QString>( "C", provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fC1 );
    QVERIFY( vl->featureCount() == 1 );
    QgsMapLayerRegistry::instance()->addMapLayer( vl );
  }

  Q_FOREACH ( const QString provider, mProviders )
  {
    QgsVectorLayer* vl = mLayers.value( QPair<QString, QString>( "X", provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fX1 );
    QVERIFY( vl->featureCount() == 1 );
    QgsMapLayerRegistry::instance()->addMapLayer( vl );
  }

  QVERIFY( QgsMapLayerRegistry::instance()->mapLayers().count() == 4*mProviders.count() );
}

void TestVectorLayerJoinBuffer::init()
{
}

void TestVectorLayerJoinBuffer::cleanup()
{
}

void TestVectorLayerJoinBuffer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestVectorLayerJoinBuffer::testJoinBasic_data()
{
  QTest::addColumn<QString>( "provider" );
  QTest::addColumn<bool>( "memoryCache" );

  QTest::newRow( "memory with cache" ) << "memory" << true ;
  QTest::newRow( "memory without cache" ) << "memory" << false;

#ifdef ENABLE_PGTEST
  QTest::newRow( "postgresql with cache" ) << "PG" << true ;
  QTest::newRow( "postgresql without cache" ) << "PG" << false;
#endif
}

void TestVectorLayerJoinBuffer::testJoinBasic()
{
  QFETCH( bool, memoryCache );
  QFETCH( QString, provider );

  QgsVectorLayer* vlA = mLayers.value( QPair<QString, QString>( "A", provider ) );
  QgsVectorLayer* vlB = mLayers.value( QPair<QString, QString>( "B", provider ) );

  QVERIFY( vlA->fields().count() == 1 );

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = vlB->id();
  joinInfo.joinFieldName = "id_b";
  joinInfo.memoryCache = memoryCache;
  joinInfo.prefix = "B_";
  vlA->addJoin( joinInfo );

  QVERIFY( vlA->fields().count() == 2 );

  QgsFeatureIterator fi = vlA->getFeatures();
  QgsFeature fA1, fA2;
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 11 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 12 );

  vlA->removeJoin( vlB->id() );

  QVERIFY( vlA->fields().count() == 1 );
}

void TestVectorLayerJoinBuffer::testJoinTransitive_data()
{
  QTest::addColumn<QString>( "provider" );
  QTest::newRow( "memory" ) << "memory";
#ifdef ENABLE_PGTEST
  QTest::newRow( "postgresql" ) << "PG";
#endif
}

void TestVectorLayerJoinBuffer::testJoinTransitive()
{

  QFETCH( QString, provider );

  QgsVectorLayer* vlA = mLayers.value( QPair<QString, QString>( "A", provider ) );
  QgsVectorLayer* vlB = mLayers.value( QPair<QString, QString>( "B", provider ) );
  QgsVectorLayer* vlC = mLayers.value( QPair<QString, QString>( "C", provider ) );

  // test join A -> B -> C
  // first we join A -> B and after that B -> C
  // layer A should automatically update to include joined data from C

  QVERIFY( vlA->fields().count() == 1 ); // id_a

  // add join A -> B

  QgsVectorJoinInfo joinInfo1;
  joinInfo1.targetFieldName = "id_a";
  joinInfo1.joinLayerId = vlB->id();
  joinInfo1.joinFieldName = "id_b";
  joinInfo1.memoryCache = true;
  joinInfo1.prefix = "B_";
  vlA->addJoin( joinInfo1 );
  QVERIFY( vlA->fields().count() == 2 ); // id_a, B_value_b

  // add join B -> C

  QgsVectorJoinInfo joinInfo2;
  joinInfo2.targetFieldName = "id_b";
  joinInfo2.joinLayerId = vlC->id();
  joinInfo2.joinFieldName = "id_c";
  joinInfo2.memoryCache = true;
  joinInfo2.prefix = "C_";
  vlB->addJoin( joinInfo2 );
  QVERIFY( vlB->fields().count() == 3 ); // id_b, value_b, C_value_c

  // now layer A must include also data from layer C
  QVERIFY( vlA->fields().count() == 3 ); // id_a, B_value_b, B_C_value_c

  QgsFeatureIterator fi = vlA->getFeatures();
  QgsFeature fA1;
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 11 );
  QCOMPARE( fA1.attribute( "B_C_value_c" ).toInt(), 101 );

  // test that layer A gets updated when layer C changes its fields
  vlC->addExpressionField( "123", QgsField( "dummy", QVariant::Int ) );
  QVERIFY( vlA->fields().count() == 4 ); // id_a, B_value_b, B_C_value_c, B_C_dummy
  vlC->removeExpressionField( 0 );

  // cleanup
  vlA->removeJoin( vlB->id() );
  vlB->removeJoin( vlC->id() );
}

void TestVectorLayerJoinBuffer::testJoinDetectCycle_data()
{
  QTest::addColumn<QString>( "provider" );
  QTest::newRow( "memory" ) << "memory";
#ifdef ENABLE_PGTEST
  QTest::newRow( "postgresql" ) << "PG";
#endif
}

void TestVectorLayerJoinBuffer::testJoinDetectCycle()
{
  QFETCH( QString, provider );

  QgsVectorLayer* vlA = mLayers.value( QPair<QString, QString>( "A", provider ) );
  QgsVectorLayer* vlB = mLayers.value( QPair<QString, QString>( "B", provider ) );

  // if A joins B and B joins A, we may get to an infinite loop if the case is not handled properly

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = vlB->id();
  joinInfo.joinFieldName = "id_b";
  joinInfo.memoryCache = true;
  joinInfo.prefix = "B_";
  vlA->addJoin( joinInfo );

  QgsVectorJoinInfo joinInfo2;
  joinInfo2.targetFieldName = "id_b";
  joinInfo2.joinLayerId = vlA->id();
  joinInfo2.joinFieldName = "id_a";
  joinInfo2.memoryCache = true;
  joinInfo2.prefix = "A_";
  bool res = vlB->addJoin( joinInfo2 );

  QVERIFY( !res );

  // the join in layer B must be rejected
  QVERIFY( vlB->vectorJoins().isEmpty() );

  vlA->removeJoin( vlB->id() );
}


void TestVectorLayerJoinBuffer::testJoinSubset_data()
{
  QTest::addColumn<QString>( "provider" );
  QTest::addColumn<bool>( "memoryCache" );

  QTest::newRow( "memory with cache" ) << "memory" << true ;
  QTest::newRow( "memory without cache" ) << "memory" << false;

#ifdef ENABLE_PGTEST
  QTest::newRow( "postgresql with cache" ) << "PG" << true ;
  QTest::newRow( "postgresql without cache" ) << "PG" << false;
#endif
}


void TestVectorLayerJoinBuffer::testJoinSubset()
{
  QFETCH( bool, memoryCache );
  QFETCH( QString, provider );

  QVERIFY( QgsMapLayerRegistry::instance()->mapLayers().count() == 4*mProviders.count() );

  QgsVectorLayer* vlA = mLayers.value( QPair<QString, QString>( "A", provider ) );
  QgsVectorLayer* vlX = mLayers.value( QPair<QString, QString>( "X", provider ) );

  // case 1: join without subset

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = vlX->id();
  joinInfo.joinFieldName = "id_x";
  joinInfo.memoryCache = memoryCache;
  joinInfo.prefix = "X_";
  bool res = vlA->addJoin( joinInfo );
  QVERIFY( res );

  QCOMPARE( vlA->fields().count(), 3 ); // id_a, X_value_x1, X_value_x2
  QgsFeatureIterator fi = vlA->getFeatures();
  QgsFeature fAX;
  fi.nextFeature( fAX );
  QCOMPARE( fAX.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fAX.attribute( "X_value_x1" ).toInt(), 111 );
  QCOMPARE( fAX.attribute( "X_value_x2" ).toInt(), 222 );

  vlA->removeJoin( vlX->id() );

  // case 2: join with subset

  QStringList* subset = new QStringList;
  *subset << "value_x2";
  joinInfo.setJoinFieldNamesSubset( subset );
  vlA->addJoin( joinInfo );

  QCOMPARE( vlA->fields().count(), 2 ); // id_a, X_value_x2

  fi = vlA->getFeatures();
  fi.nextFeature( fAX );
  QCOMPARE( fAX.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fAX.attribute( "X_value_x2" ).toInt(), 222 );

  vlA->removeJoin( vlX->id() );
}

void TestVectorLayerJoinBuffer::testJoinTwoTimes_data()
{
  QTest::addColumn<QString>( "provider" );
  QTest::newRow( "memory" ) << "memory";
#ifdef ENABLE_PGTEST
  QTest::newRow( "postgresql" ) << "PG";
#endif
}

void TestVectorLayerJoinBuffer::testJoinTwoTimes()
{

  QFETCH( QString, provider );

  QgsVectorLayer* vlA = mLayers.value( QPair<QString, QString>( "A", provider ) );
  QgsVectorLayer* vlB = mLayers.value( QPair<QString, QString>( "B", provider ) );

  QVERIFY( vlA->fields().count() == 1 );

  QgsVectorJoinInfo joinInfo1;
  joinInfo1.targetFieldName = "id_a";
  joinInfo1.joinLayerId = vlB->id();
  joinInfo1.joinFieldName = "id_b";
  joinInfo1.memoryCache = true;
  joinInfo1.prefix = "j1_";
  vlA->addJoin( joinInfo1 );

  QgsVectorJoinInfo joinInfo2;
  joinInfo2.targetFieldName = "id_a";
  joinInfo2.joinLayerId = vlB->id();
  joinInfo2.joinFieldName = "id_b";
  joinInfo2.memoryCache = true;
  joinInfo2.prefix = "j2_";
  vlA->addJoin( joinInfo2 );

  QCOMPARE( vlA->vectorJoins().count(), 2 );

  QVERIFY( vlA->fields().count() == 3 );

  QgsFeatureIterator fi = vlA->getFeatures();
  QgsFeature fA1; //, fA2;
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "j1_value_b" ).toInt(), 11 );
  QCOMPARE( fA1.attribute( "j2_value_b" ).toInt(), 11 );

  vlA->removeJoin( vlB->id() );
  vlA->removeJoin( vlB->id() );

  QCOMPARE( vlA->vectorJoins().count(), 0 );
}

void TestVectorLayerJoinBuffer::testJoinLayerDefinitionFile()
{
  bool r;

  QgsMapLayerRegistry::instance()->removeAllMapLayers();

  // Create two layers
  QgsVectorLayer* layerA = new QgsVectorLayer( "Point?crs=epsg:4326&field=key:integer&field=value:double&index=yes", "layerA", "memory" );
  QVERIFY( layerA );
  QgsMapLayerRegistry::instance()->addMapLayer( layerA );

  QgsVectorLayer* layerB = new QgsVectorLayer( "Point?crs=epsg:4326&field=id:integer&index=yes", "layerB", "memory" );
  QVERIFY( layerB );
  QgsMapLayerRegistry::instance()->addMapLayer( layerB );

  // Create vector join
  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id";
  joinInfo.joinLayerId = layerA->id();
  joinInfo.joinFieldName = "key";
  joinInfo.memoryCache = true;
  joinInfo.prefix = "joined_";
  r = layerB->addJoin( joinInfo );
  QVERIFY( r );

  // Generate QLR
  QDomDocument qlrDoc( "qgis-layer-definition" );
  QString errorMessage;
  r = QgsLayerDefinition::exportLayerDefinition( qlrDoc, QgsProject::instance()->layerTreeRoot()->children(), errorMessage );
  QVERIFY2( r, errorMessage.toUtf8().constData() );

  // Clear
  QgsMapLayerRegistry::instance()->removeAllMapLayers();

  // Load QLR
  r = QgsLayerDefinition::loadLayerDefinition( qlrDoc, QgsProject::instance()->layerTreeRoot(), errorMessage );
  QVERIFY2( r, errorMessage.toUtf8().constData() );

  // Get layer
  QList<QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayersByName( "layerB" );
  QCOMPARE( mapLayers.count(), 1 );

  QgsVectorLayer* vLayer = dynamic_cast<QgsVectorLayer*>( mapLayers.value( 0 ) );
  QVERIFY( vLayer );

  // Check for vector join
  QCOMPARE( vLayer->vectorJoins().count(), 1 );

  // Check for joined field
  QVERIFY( vLayer->fieldNameIndex( joinInfo.prefix + "value" ) >= 0 );
}

void TestVectorLayerJoinBuffer::testCacheUpdate_data()
{
  QTest::addColumn<bool>( "useCache" );
  QTest::newRow( "cache" ) << true;
  QTest::newRow( "no cache" ) << false;
}

void TestVectorLayerJoinBuffer::testCacheUpdate()
{
  QFETCH( bool, useCache );

  QgsVectorLayer* vlA = new QgsVectorLayer( "Point?field=id_a:integer", "cacheA", "memory" );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer* vlB = new QgsVectorLayer( "Point?field=id_b:integer&field=value_b", "cacheB", "memory" );
  QVERIFY( vlB->isValid() );
  QgsMapLayerRegistry::instance()->addMapLayer( vlA );
  QgsMapLayerRegistry::instance()->addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( "id_a", 1 );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( "id_a", 2 );

  vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );

  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( "id_b", 1 );
  fB1.setAttribute( "value_b", 11 );
  QgsFeature fB2( vlB->dataProvider()->fields(), 2 );
  fB2.setAttribute( "id_b", 2 );
  fB2.setAttribute( "value_b", 12 );

  vlB->dataProvider()->addFeatures( QgsFeatureList() << fB1 << fB2 );

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = vlB->id();
  joinInfo.joinFieldName = "id_b";
  joinInfo.memoryCache = useCache;
  joinInfo.prefix = "B_";
  vlA->addJoin( joinInfo );

  QgsFeatureIterator fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 11 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 12 );

  // change value in join target layer
  vlB->startEditing();
  vlB->changeAttributeValue( 1, 1, 111 );
  vlB->changeAttributeValue( 2, 0, 3 );
  vlB->commitChanges();

  fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 111 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QVERIFY( fA2.attribute( "B_value_b" ).isNull() );

  // change value in joined layer
  vlA->startEditing();
  vlA->changeAttributeValue( 2, 0, 3 );
  vlA->commitChanges();

  fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 111 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 3 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 12 );

  QgsMapLayerRegistry::instance()->removeMapLayer( vlA );
  QgsMapLayerRegistry::instance()->removeMapLayer( vlB );
}


QTEST_MAIN( TestVectorLayerJoinBuffer )
#include "testqgsvectorlayerjoinbuffer.moc"


