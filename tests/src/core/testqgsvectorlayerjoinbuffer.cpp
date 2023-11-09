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


#include "qgstest.h"
#include <QObject>

//qgis includes...
#include <qgsvectorlayer.h>
#include "qgsfeatureiterator.h"
#include "qgslayertreegroup.h"
#include "qgsreadwritecontext.h"
#include <qgsvectordataprovider.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgslayerdefinition.h>
#include <qgsproject.h>
#include "qgslayertree.h"
#include <QSignalSpy>

/**
 * @ingroup UnitTests
 * This is a unit test for the vector layer join buffer
 *
 * \see QgsVectorLayerJoinBuffer
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
    void testRemoveJoinOnLayerDelete();
    void testResolveReferences();
    void testSignals();
    void testChangeAttributeValues();
    void testCollidingNameColumn();
    void testCollidingNameColumnCached();

  private:
    QgsProject mProject;
    QList<QString> mProviders;
    // map of layers. First key is the name of the layer A, B or C and second key is the provider memory or PG.
    QMap<QPair<QString, QString>, QgsVectorLayer *> mLayers;
};

// runs before all tests
void TestVectorLayerJoinBuffer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mProviders = QList<QString>() << QStringLiteral( "memory" );

  // Create memory layers
  // LAYER A //
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QVERIFY( vlA->fields().count() == 1 );
  // LAYER B //
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  QVERIFY( vlB->fields().count() == 2 );
  // LAYER C //
  QgsVectorLayer *vlC = new QgsVectorLayer( QStringLiteral( "Point?field=id_c:integer&field=value_c" ), QStringLiteral( "C" ), QStringLiteral( "memory" ) );
  QVERIFY( vlC->isValid() );
  QVERIFY( vlC->fields().count() == 2 );
  // LAYER X //
  QgsVectorLayer *vlX = new QgsVectorLayer( QStringLiteral( "Point?field=id_x:integer&field=value_x1:integer&field=value_x2" ), QStringLiteral( "X" ), QStringLiteral( "memory" ) );
  QVERIFY( vlX->isValid() );
  QVERIFY( vlX->fields().count() == 3 );

  mLayers = QMap<QPair<QString, QString>, QgsVectorLayer *>();
  mLayers.insert( QPair<QString, QString>( QStringLiteral( "A" ), QStringLiteral( "memory" ) ), vlA );
  mLayers.insert( QPair<QString, QString>( QStringLiteral( "B" ), QStringLiteral( "memory" ) ), vlB );
  mLayers.insert( QPair<QString, QString>( QStringLiteral( "C" ), QStringLiteral( "memory" ) ), vlC );
  mLayers.insert( QPair<QString, QString>( QStringLiteral( "X" ), QStringLiteral( "memory" ) ), vlX );

  // Add PG layers
#ifdef ENABLE_PGTEST
  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=qgis_test";
  }
  QgsVectorLayer *vlA_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_a' table=\"qgis_test\".\"table_a\" sql=" ).arg( dbConn ), "A_PG", "postgres" );
  QgsVectorLayer *vlB_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_b' table=\"qgis_test\".\"table_b\" sql=" ).arg( dbConn ), "B_PG", "postgres" );
  QgsVectorLayer *vlC_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_c' table=\"qgis_test\".\"table_c\" sql=" ).arg( dbConn ), "C_PG", "postgres" );
  QgsVectorLayer *vlX_PG = new QgsVectorLayer( QString( "%1 sslmode=disable key='id_x' table=\"qgis_test\".\"table_x\" sql=" ).arg( dbConn ), "X_PG", "postgres" );
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
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( QStringLiteral( "id_a" ), 2 );
  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( QStringLiteral( "id_b" ), 1 );
  fB1.setAttribute( QStringLiteral( "value_b" ), 11 );
  QgsFeature fB2( vlB->dataProvider()->fields(), 2 );
  fB2.setAttribute( QStringLiteral( "id_b" ), 2 );
  fB2.setAttribute( QStringLiteral( "value_b" ), 12 );
  QgsFeature fC1( vlC->dataProvider()->fields(), 1 );
  fC1.setAttribute( QStringLiteral( "id_c" ), 1 );
  fC1.setAttribute( QStringLiteral( "value_c" ), 101 );
  QgsFeature fX1( vlX->dataProvider()->fields(), 1 );
  fX1.setAttribute( QStringLiteral( "id_x" ), 1 );
  fX1.setAttribute( QStringLiteral( "value_x1" ), 111 );
  fX1.setAttribute( QStringLiteral( "value_x2" ), 222 );

  // Commit features and layers to qgis
  for ( const QString &provider : mProviders )
  {
    QgsVectorLayer *vl = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );
    QVERIFY( vl->featureCount() == 2 );
    mProject.addMapLayer( vl );
  }

  for ( const QString &provider : mProviders )
  {
    QgsVectorLayer *vl = mLayers.value( QPair<QString, QString>( QStringLiteral( "B" ), provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fB1 << fB2 );
    QVERIFY( vl->featureCount() == 2 );
    mProject.addMapLayer( vl );
  }

  for ( const QString &provider : mProviders )
  {
    QgsVectorLayer *vl = mLayers.value( QPair<QString, QString>( QStringLiteral( "C" ), provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fC1 );
    QVERIFY( vl->featureCount() == 1 );
    mProject.addMapLayer( vl );
  }

  for ( const QString &provider : mProviders )
  {
    QgsVectorLayer *vl = mLayers.value( QPair<QString, QString>( QStringLiteral( "X" ), provider ) );
    vl->dataProvider()->addFeatures( QgsFeatureList() << fX1 );
    QVERIFY( vl->featureCount() == 1 );
    mProject.addMapLayer( vl );
  }

  QVERIFY( mProject.mapLayers().count() == 4 * mProviders.count() );
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

  QgsVectorLayer *vlA = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
  QgsVectorLayer *vlB = mLayers.value( QPair<QString, QString>( QStringLiteral( "B" ), provider ) );

  QVERIFY( vlA->fields().count() == 1 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( memoryCache );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
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

  QgsVectorLayer *vlA = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
  QgsVectorLayer *vlB = mLayers.value( QPair<QString, QString>( QStringLiteral( "B" ), provider ) );
  QgsVectorLayer *vlC = mLayers.value( QPair<QString, QString>( QStringLiteral( "C" ), provider ) );

  // test join A -> B -> C
  // first we join A -> B and after that B -> C
  // layer A should automatically update to include joined data from C

  QVERIFY( vlA->fields().count() == 1 ); // id_a

  // add join A -> B

  QgsVectorLayerJoinInfo joinInfo1;
  joinInfo1.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo1.setJoinLayer( vlB );
  joinInfo1.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo1.setUsingMemoryCache( true );
  joinInfo1.setPrefix( QStringLiteral( "B_" ) );
  vlA->addJoin( joinInfo1 );
  QVERIFY( vlA->fields().count() == 2 ); // id_a, B_value_b

  // add join B -> C

  QgsVectorLayerJoinInfo joinInfo2;
  joinInfo2.setTargetFieldName( QStringLiteral( "id_b" ) );
  joinInfo2.setJoinLayer( vlC );
  joinInfo2.setJoinFieldName( QStringLiteral( "id_c" ) );
  joinInfo2.setUsingMemoryCache( true );
  joinInfo2.setPrefix( QStringLiteral( "C_" ) );
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
  vlC->addExpressionField( QStringLiteral( "123" ), QgsField( QStringLiteral( "dummy" ), QVariant::Int ) );
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

  QgsVectorLayer *vlA = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
  QgsVectorLayer *vlB = mLayers.value( QPair<QString, QString>( QStringLiteral( "B" ), provider ) );

  // if A joins B and B joins A, we may get to an infinite loop if the case is not handled properly

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( true );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  vlA->addJoin( joinInfo );

  QgsVectorLayerJoinInfo joinInfo2;
  joinInfo2.setTargetFieldName( QStringLiteral( "id_b" ) );
  joinInfo2.setJoinLayer( vlA );
  joinInfo2.setJoinFieldName( QStringLiteral( "id_a" ) );
  joinInfo2.setUsingMemoryCache( true );
  joinInfo2.setPrefix( QStringLiteral( "A_" ) );
  const bool res = vlB->addJoin( joinInfo2 );

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

  QVERIFY( mProject.mapLayers().count() == 4 * mProviders.count() );

  QgsVectorLayer *vlA = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
  QgsVectorLayer *vlX = mLayers.value( QPair<QString, QString>( QStringLiteral( "X" ), provider ) );

  // case 1: join without subset

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlX );
  joinInfo.setJoinFieldName( QStringLiteral( "id_x" ) );
  joinInfo.setUsingMemoryCache( memoryCache );
  joinInfo.setPrefix( QStringLiteral( "X_" ) );
  const bool res = vlA->addJoin( joinInfo );
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

  QStringList *subset = new QStringList;
  *subset << QStringLiteral( "value_x2" );
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

  QgsVectorLayer *vlA = mLayers.value( QPair<QString, QString>( QStringLiteral( "A" ), provider ) );
  QgsVectorLayer *vlB = mLayers.value( QPair<QString, QString>( QStringLiteral( "B" ), provider ) );

  QVERIFY( vlA->fields().count() == 1 );

  QgsVectorLayerJoinInfo joinInfo1;
  joinInfo1.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo1.setJoinLayer( vlB );
  joinInfo1.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo1.setUsingMemoryCache( true );
  joinInfo1.setPrefix( QStringLiteral( "j1_" ) );
  vlA->addJoin( joinInfo1 );

  QgsVectorLayerJoinInfo joinInfo2;
  joinInfo2.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo2.setJoinLayer( vlB );
  joinInfo2.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo2.setUsingMemoryCache( true );
  joinInfo2.setPrefix( QStringLiteral( "j2_" ) );
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

  mProject.removeAllMapLayers();

  // Create two layers
  QgsVectorLayer *layerA = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=key:integer&field=value:double&index=yes" ), QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );
  QVERIFY( layerA );
  mProject.addMapLayer( layerA );

  QgsVectorLayer *layerB = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer&index=yes" ), QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );
  QVERIFY( layerB );
  mProject.addMapLayer( layerB );

  // Create vector join
  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id" ) );
  joinInfo.setJoinLayer( layerA );
  joinInfo.setJoinFieldName( QStringLiteral( "key" ) );
  joinInfo.setUsingMemoryCache( true );
  joinInfo.setPrefix( QStringLiteral( "joined_" ) );
  r = layerB->addJoin( joinInfo );
  QVERIFY( r );

  // Generate QLR
  const QDomDocument qlrDoc( QStringLiteral( "qgis-layer-definition" ) );
  QString errorMessage;
  r = QgsLayerDefinition::exportLayerDefinition( qlrDoc, mProject.layerTreeRoot()->children(), errorMessage, QgsReadWriteContext() );
  QVERIFY2( r, errorMessage.toUtf8().constData() );

  // Clear
  mProject.removeAllMapLayers();

  // Load QLR
  QgsReadWriteContext context = QgsReadWriteContext();
  r = QgsLayerDefinition::loadLayerDefinition( qlrDoc, &mProject, mProject.layerTreeRoot(), errorMessage, context );
  QVERIFY2( r, errorMessage.toUtf8().constData() );

  // Get layer
  const QList<QgsMapLayer *> mapLayers = mProject.mapLayersByName( QStringLiteral( "layerB" ) );
  QCOMPARE( mapLayers.count(), 1 );

  QgsVectorLayer *vLayer = dynamic_cast<QgsVectorLayer *>( mapLayers.value( 0 ) );
  QVERIFY( vLayer );

  // Check for vector join
  QCOMPARE( vLayer->vectorJoins().count(), 1 );

  // Check for joined field
  QVERIFY( vLayer->fields().lookupField( joinInfo.prefix() + "value" ) >= 0 );
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

  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  mProject.addMapLayer( vlA );
  mProject.addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( QStringLiteral( "id_a" ), 2 );

  vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );

  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( QStringLiteral( "id_b" ), 1 );
  fB1.setAttribute( QStringLiteral( "value_b" ), 11 );
  QgsFeature fB2( vlB->dataProvider()->fields(), 2 );
  fB2.setAttribute( QStringLiteral( "id_b" ), 2 );
  fB2.setAttribute( QStringLiteral( "value_b" ), 12 );

  vlB->dataProvider()->addFeatures( QgsFeatureList() << fB1 << fB2 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( useCache );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
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
}

void TestVectorLayerJoinBuffer::testRemoveJoinOnLayerDelete()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( true );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  vlA->addJoin( joinInfo );

  QCOMPARE( vlA->vectorJoins().count(), 1 );
  QCOMPARE( vlA->vectorJoins()[0].joinLayer(), vlB );
  QCOMPARE( vlA->vectorJoins()[0].joinLayerId(), vlB->id() );
  QCOMPARE( vlA->fields().count(), 2 );

  delete vlB;

  QCOMPARE( vlA->vectorJoins().count(), 0 );
  QCOMPARE( vlA->fields().count(), 1 );

  delete vlA;
}

void TestVectorLayerJoinBuffer::testResolveReferences()
{
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayerId( vlB->id() );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setUsingMemoryCache( true );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  vlA->addJoin( joinInfo );

  QCOMPARE( vlA->fields().count(), 1 );
  QCOMPARE( vlA->vectorJoins()[0].joinLayer(), ( QgsVectorLayer * ) nullptr );
  QCOMPARE( vlA->vectorJoins()[0].joinLayerId(), vlB->id() );

  QgsProject project;
  project.addMapLayer( vlB );

  vlA->resolveReferences( &project );

  QCOMPARE( vlA->fields().count(), 2 );
  QCOMPARE( vlA->vectorJoins()[0].joinLayer(), vlB );
  QCOMPARE( vlA->vectorJoins()[0].joinLayerId(), vlB->id() );

  delete vlA;
}

void TestVectorLayerJoinBuffer::testSignals()
{
  mProject.clear();
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  mProject.addMapLayer( vlA );
  mProject.addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( QStringLiteral( "id_a" ), 2 );

  vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  joinInfo.setEditable( true );
  joinInfo.setUpsertOnEdit( true );
  vlA->addJoin( joinInfo );

  QgsFeatureIterator fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QVERIFY( !fA1.attribute( "B_value_b" ).isValid() );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QVERIFY( !fA2.attribute( "B_value_b" ).isValid() );

  // change value in join target layer, check for signals
  const QSignalSpy spy( vlA, &QgsVectorLayer::attributeValueChanged );
  vlA->startEditing();
  vlB->startEditing();
  // adds new feature to second layer
  QVERIFY( vlA->changeAttributeValue( 1, 1, 111 ) );
  fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 111 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QVERIFY( !fA2.attribute( "B_value_b" ).isValid() );
  QCOMPARE( spy.count(), 1 );
  QVERIFY( vlA->changeAttributeValue( 2, 1, 222 ) );
  fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 111 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 222 );
  QCOMPARE( spy.count(), 2 );
  // changes existing feature in second layer
  QVERIFY( vlA->changeAttributeValue( 1, 1, 112 ) );
  fi = vlA->getFeatures();
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 112 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 222 );
  QCOMPARE( spy.count(), 3 );
}

void TestVectorLayerJoinBuffer::testChangeAttributeValues()
{
  // change attribute values in a vector layer which includes joins
  mProject.clear();
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer&field=value_a1:string&field=value_a2:string" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=value_b1:string&field=value_b2:string" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  mProject.addMapLayer( vlA );
  mProject.addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  fA1.setAttribute( QStringLiteral( "value_a1" ), QStringLiteral( "a_1_1" ) );
  fA1.setAttribute( QStringLiteral( "value_a2" ), QStringLiteral( "a_1_2" ) );
  QgsFeature fA2( vlA->dataProvider()->fields(), 2 );
  fA2.setAttribute( QStringLiteral( "id_a" ), 2 );
  fA2.setAttribute( QStringLiteral( "value_a1" ), QStringLiteral( "a_2_1" ) );
  fA2.setAttribute( QStringLiteral( "value_a2" ), QStringLiteral( "a_2_2" ) );

  QVERIFY( vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 ) );

  QCOMPARE( vlA->getFeature( 1 ).attributes().size(), 3 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "a_1_1" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "a_1_2" ) );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setPrefix( QStringLiteral( "B_" ) );
  joinInfo.setEditable( true );
  joinInfo.setUpsertOnEdit( true );
  vlA->addJoin( joinInfo );

  QVERIFY( vlA->startEditing() );
  QVERIFY( vlB->startEditing() );

  QCOMPARE( vlA->getFeature( 1 ).attributes().size(), 5 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "a_1_1" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "a_1_2" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 3 ).toString(), QString() );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 4 ).toString(), QString() );

  // change a provider field
  QVERIFY( vlA->changeAttributeValue( 1, 1, QStringLiteral( "new_a_1_1" ) ) );
  // change a join field
  QVERIFY( vlA->changeAttributeValue( 1, 3, QStringLiteral( "new_b_1_1" ) ) );

  QCOMPARE( vlA->getFeature( 1 ).attributes().size(), 5 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 1 ).toString(), QStringLiteral( "new_a_1_1" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 2 ).toString(), QStringLiteral( "a_1_2" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 3 ).toString(), QStringLiteral( "new_b_1_1" ) );
  QCOMPARE( vlA->getFeature( 1 ).attributes().at( 4 ).toString(), QString() );

  QgsFeature joinFeature;
  vlB->getFeatures().nextFeature( joinFeature );
  QVERIFY( joinFeature.isValid() );
  QCOMPARE( joinFeature.attributes().size(), 3 );
  QCOMPARE( joinFeature.attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( joinFeature.attributes().at( 1 ).toString(), QStringLiteral( "new_b_1_1" ) );
  QCOMPARE( joinFeature.attributes().at( 2 ).toString(), QString() );

  // change a combination of provider and joined fields at once
  QVERIFY( vlA->changeAttributeValues( 2, QgsAttributeMap{ { 1, QStringLiteral( "new_a_2_1" ) },
    { 2, QStringLiteral( "new_a_2_2" ) },
    { 3, QStringLiteral( "new_b_2_1" ) },
    { 4, QStringLiteral( "new_b_2_2" ) }} ) );

  QCOMPARE( vlA->getFeature( 2 ).attributes().size(), 5 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 0 ).toInt(), 2 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 1 ).toString(), QStringLiteral( "new_a_2_1" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 2 ).toString(), QStringLiteral( "new_a_2_2" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 3 ).toString(), QStringLiteral( "new_b_2_1" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 4 ).toString(), QStringLiteral( "new_b_2_2" ) );

  // change only provider fields
  QVERIFY( vlA->changeAttributeValues( 2, QgsAttributeMap{ { 1, QStringLiteral( "new_a_2_1b" ) },
    { 2, QStringLiteral( "new_a_2_2b" ) }} ) );

  QCOMPARE( vlA->getFeature( 2 ).attributes().size(), 5 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 0 ).toInt(), 2 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 1 ).toString(), QStringLiteral( "new_a_2_1b" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 2 ).toString(), QStringLiteral( "new_a_2_2b" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 3 ).toString(), QStringLiteral( "new_b_2_1" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 4 ).toString(), QStringLiteral( "new_b_2_2" ) );

  // change only joined fields
  QVERIFY( vlA->changeAttributeValues( 2, QgsAttributeMap{ { 3, QStringLiteral( "new_b_2_1b" ) },
    { 4, QStringLiteral( "new_b_2_2b" ) }} ) );

  QCOMPARE( vlA->getFeature( 2 ).attributes().size(), 5 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 0 ).toInt(), 2 );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 1 ).toString(), QStringLiteral( "new_a_2_1b" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 2 ).toString(), QStringLiteral( "new_a_2_2b" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 3 ).toString(), QStringLiteral( "new_b_2_1b" ) );
  QCOMPARE( vlA->getFeature( 2 ).attributes().at( 4 ).toString(), QStringLiteral( "new_b_2_2b" ) );

}

// Check https://github.com/qgis/QGIS/issues/26652
void TestVectorLayerJoinBuffer::testCollidingNameColumn()
{
  mProject.clear();
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer&field=name" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=name&field=value_b&field=value_c" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  mProject.addMapLayer( vlA );
  mProject.addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  fA1.setAttribute( QStringLiteral( "name" ), QStringLiteral( "name_a" ) );

  vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setPrefix( QLatin1String( "" ) );
  joinInfo.setEditable( true );
  joinInfo.setUpsertOnEdit( true );
  vlA->addJoin( joinInfo );

  QgsFeatureIterator fi1 = vlA->getFeatures();
  fi1.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QVERIFY( !fA1.attribute( "value_b" ).isValid() );
  QVERIFY( !fA1.attribute( "value_c" ).isValid() );

  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( QStringLiteral( "id_b" ), 1 );
  fB1.setAttribute( QStringLiteral( "name" ), QStringLiteral( "name_b" ) );
  fB1.setAttribute( QStringLiteral( "value_b" ), QStringLiteral( "value_b" ) );
  fB1.setAttribute( QStringLiteral( "value_c" ), QStringLiteral( "value_c" ) );

  vlB->dataProvider()->addFeatures( QgsFeatureList() << fB1 );

  QgsFeatureIterator fi2 = vlA->getFeatures();
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );

  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );
  QVERIFY( !fA1.attribute( "value_c" ).isValid() );

  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 3} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QVERIFY( !fA1.attribute( "value_b" ).isValid() );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( new QStringList( {"name"} ) );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( new QStringList( {"value_b"} ) );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( new QStringList( {"value_c"} ) );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( new QStringList( {"name", "value_c"} ) );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2, 3} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( new QStringList( {"value_b", "value_c"} ) );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2, 3} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );

  vlA->removeJoin( vlB->id() );
  joinInfo.setJoinFieldNamesSubset( nullptr );
  vlA->addJoin( joinInfo );
  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 2} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );
  QVERIFY( !fA1.attribute( "value_c" ).isValid() );

  fi2 = vlA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList( {0, 1, 3} ) ) );
  fi2.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QVERIFY( !fA1.attribute( "value_b" ).isValid() );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );
}

void TestVectorLayerJoinBuffer::testCollidingNameColumnCached()
{
  mProject.clear();
  QgsVectorLayer *vlA = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer&field=name" ), QStringLiteral( "cacheA" ), QStringLiteral( "memory" ) );
  QVERIFY( vlA->isValid() );
  QgsVectorLayer *vlB = new QgsVectorLayer( QStringLiteral( "Point?field=id_b:integer&field=name&field=value_b&field=value_c" ), QStringLiteral( "cacheB" ), QStringLiteral( "memory" ) );
  QVERIFY( vlB->isValid() );
  mProject.addMapLayer( vlA );
  mProject.addMapLayer( vlB );

  QgsFeature fA1( vlA->dataProvider()->fields(), 1 );
  fA1.setAttribute( QStringLiteral( "id_a" ), 1 );
  fA1.setAttribute( QStringLiteral( "name" ), QStringLiteral( "name_a" ) );

  vlA->dataProvider()->addFeatures( QgsFeatureList() << fA1 );

  QgsFeature fB1( vlB->dataProvider()->fields(), 1 );
  fB1.setAttribute( QStringLiteral( "id_b" ), 1 );
  fB1.setAttribute( QStringLiteral( "name" ), QStringLiteral( "name_b" ) );
  fB1.setAttribute( QStringLiteral( "value_b" ), QStringLiteral( "value_b" ) );
  fB1.setAttribute( QStringLiteral( "value_c" ), QStringLiteral( "value_c" ) );

  vlB->dataProvider()->addFeatures( QgsFeatureList() << fB1 );

  QgsVectorLayerJoinInfo joinInfo;
  joinInfo.setTargetFieldName( QStringLiteral( "id_a" ) );
  joinInfo.setJoinLayer( vlB );
  joinInfo.setJoinFieldName( QStringLiteral( "id_b" ) );
  joinInfo.setPrefix( QLatin1String( "" ) );
  joinInfo.setEditable( true );
  joinInfo.setUpsertOnEdit( false );
  joinInfo.setUsingMemoryCache( true );
  vlA->addJoin( joinInfo );

  QgsFeatureIterator fi1 = vlA->getFeatures();
  fi1.nextFeature( fA1 );
  QCOMPARE( fA1.fields().names(), QStringList( {"id_a", "name", "value_b", "value_c"} ) );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "name" ).toString(), QStringLiteral( "name_a" ) );
  QCOMPARE( fA1.attribute( "value_b" ).toString(), QStringLiteral( "value_b" ) );
  QCOMPARE( fA1.attribute( "value_c" ).toString(), QStringLiteral( "value_c" ) );
}

QGSTEST_MAIN( TestVectorLayerJoinBuffer )
#include "testqgsvectorlayerjoinbuffer.moc"
