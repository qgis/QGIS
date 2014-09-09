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


#include <QtTest>
#include <QObject>

//qgis includes...
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsapplication.h>
#include <qgsvectorlayerjoinbuffer.h>
#include <qgsmaplayerregistry.h>

/** @ingroup UnitTests
 * This is a unit test for the vector layer join buffer
 *
 * @see QgsVectorLayerJoinBuffer
 */
class TestVectorLayerJoinBuffer: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();      // will be called before the first testfunction is executed.
    void cleanupTestCase();   // will be called after the last testfunction was executed.
    void init();              // will be called before each testfunction is executed.
    void cleanup();           // will be called after every testfunction.

    void testCacheBasic();
    void testCacheTransitive();

  private:
    QgsVectorLayer* mLayerA;
    QgsVectorLayer* mLayerB;
    QgsVectorLayer* mLayerC;
};

// runs before all tests
void TestVectorLayerJoinBuffer::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // LAYER A //

  mLayerA = new QgsVectorLayer( "Point?field=id_a:integer", "A", "memory" );
  QVERIFY( mLayerA->isValid() );
  QVERIFY( mLayerA->pendingFields().count() == 1 );

  QgsFeature fA1( mLayerA->dataProvider()->fields(), 1 );
  fA1.setAttribute( "id_a", 1 );
  QgsFeature fA2( mLayerA->dataProvider()->fields(), 2 );
  fA2.setAttribute( "id_a", 2 );
  mLayerA->dataProvider()->addFeatures( QgsFeatureList() << fA1 << fA2 );
  QVERIFY( mLayerA->pendingFeatureCount() == 2 );

  // LAYER B //

  mLayerB = new QgsVectorLayer( "Point?field=id_b:integer&field=value_b", "B", "memory" );
  QVERIFY( mLayerB->isValid() );
  QVERIFY( mLayerB->pendingFields().count() == 2 );

  QgsFeature fB1( mLayerB->dataProvider()->fields(), 1 );
  fB1.setAttribute( "id_b", 1 );
  fB1.setAttribute( "value_b", 11 );
  QgsFeature fB2( mLayerB->dataProvider()->fields(), 2 );
  fB2.setAttribute( "id_b", 2 );
  fB2.setAttribute( "value_b", 12 );
  mLayerB->dataProvider()->addFeatures( QgsFeatureList() << fB1 << fB2 );
  QVERIFY( mLayerB->pendingFeatureCount() == 2 );

  // LAYER C //

  mLayerC = new QgsVectorLayer( "Point?field=id_c:integer&field=value_c", "C", "memory" );
  QVERIFY( mLayerC->isValid() );
  QVERIFY( mLayerC->pendingFields().count() == 2 );

  QgsFeature fC1( mLayerC->dataProvider()->fields(), 1 );
  fC1.setAttribute( "id_c", 1 );
  fC1.setAttribute( "value_c", 101 );
  mLayerC->dataProvider()->addFeatures( QgsFeatureList() << fC1 );
  QVERIFY( mLayerC->pendingFeatureCount() == 1 );

  QgsMapLayerRegistry::instance()->addMapLayer( mLayerA );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayerB );
  QgsMapLayerRegistry::instance()->addMapLayer( mLayerC );
}

void TestVectorLayerJoinBuffer::init()
{
}

void TestVectorLayerJoinBuffer::cleanup()
{
}

void TestVectorLayerJoinBuffer::cleanupTestCase()
{
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
}

void TestVectorLayerJoinBuffer::testCacheBasic()
{
  QVERIFY( mLayerA->pendingFields().count() == 1 );

  QgsVectorJoinInfo joinInfo;
  joinInfo.targetFieldName = "id_a";
  joinInfo.joinLayerId = mLayerB->id();
  joinInfo.joinFieldName = "id_b";
  // memory provider does not implement setSubsetString() so direct join does not work!
  joinInfo.memoryCache = true;
  mLayerA->addJoin( joinInfo );

  QVERIFY( mLayerA->pendingFields().count() == 2 );

  QgsFeatureIterator fi = mLayerA->getFeatures();
  QgsFeature fA1, fA2;
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 11 );
  fi.nextFeature( fA2 );
  QCOMPARE( fA2.attribute( "id_a" ).toInt(), 2 );
  QCOMPARE( fA2.attribute( "B_value_b" ).toInt(), 12 );

  mLayerA->removeJoin( mLayerB->id() );

  QVERIFY( mLayerA->pendingFields().count() == 1 );
}

void TestVectorLayerJoinBuffer::testCacheTransitive()
{
  // test join A -> B -> C
  // first we join A -> B and after that B -> C
  // layer A should automatically update to include joined data from C

  QVERIFY( mLayerA->pendingFields().count() == 1 ); // id_a

  // add join A -> B

  QgsVectorJoinInfo joinInfo1;
  joinInfo1.targetFieldName = "id_a";
  joinInfo1.joinLayerId = mLayerB->id();
  joinInfo1.joinFieldName = "id_b";
  // memory provider does not implement setSubsetString() so direct join does not work!
  joinInfo1.memoryCache = true;
  mLayerA->addJoin( joinInfo1 );
  QVERIFY( mLayerA->pendingFields().count() == 2 ); // id_a, B_value_b

  // add join B -> C

  QgsVectorJoinInfo joinInfo2;
  joinInfo2.targetFieldName = "id_b";
  joinInfo2.joinLayerId = mLayerC->id();
  joinInfo2.joinFieldName = "id_c";
  // memory provider does not implement setSubsetString() so direct join does not work!
  joinInfo2.memoryCache = true;
  mLayerB->addJoin( joinInfo2 );
  QVERIFY( mLayerB->pendingFields().count() == 3 ); // id_b, value_b, C_value_c

  // now layer A must include also data from layer C
  QVERIFY( mLayerA->pendingFields().count() == 3 ); // id_a, B_value_b, B_C_value_c

  QgsFeatureIterator fi = mLayerA->getFeatures();
  QgsFeature fA1;
  fi.nextFeature( fA1 );
  QCOMPARE( fA1.attribute( "id_a" ).toInt(), 1 );
  QCOMPARE( fA1.attribute( "B_value_b" ).toInt(), 11 );
  QCOMPARE( fA1.attribute( "B_C_value_c" ).toInt(), 101 );

  // test that layer A gets updated when layer C changes its fields
  mLayerC->addExpressionField( "123", QgsField( "dummy", QVariant::Int ) );
  QVERIFY( mLayerA->pendingFields().count() == 4 ); // id_a, B_value_b, B_C_value_c, B_C_dummy
  mLayerC->removeExpressionField( 0 );

  // cleanup
  mLayerA->removeJoin( mLayerB->id() );
  mLayerB->removeJoin( mLayerC->id() );
}


QTEST_MAIN( TestVectorLayerJoinBuffer )
#include "moc_testqgsvectorlayerjoinbuffer.cxx"


