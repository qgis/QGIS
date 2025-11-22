/***************************************************************************
     test_template.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:23 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsvectorlayerutils.h"
#include "qgsvectorlayer.h"
#include <QThread>

/**
 * \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsVectorLayerUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayerUtils() = default;

  private slots:

    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testGetFeatureSource();
    void testFilterValidFeatureIds();
};

void TestQgsVectorLayerUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsVectorLayerUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

class FeatureFetcher : public QThread
{
    Q_OBJECT

  public:
    FeatureFetcher( QPointer<QgsVectorLayer> layer )
      : mLayer( layer )
    {
    }

    void run() override
    {
      QgsFeature feat;
      auto fs = QgsVectorLayerUtils::getFeatureSource( mLayer );
      if ( fs )
        fs->getFeatures().nextFeature( feat );
      emit resultReady( feat.attribute( QStringLiteral( "col1" ) ) );
    }

  signals:
    void resultReady( const QVariant &attribute );

  private:
    QPointer<QgsVectorLayer> mLayer;
};


void TestQgsVectorLayerUtils::testGetFeatureSource()
{
  auto vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  vl->startEditing();
  QgsFeature f1( vl->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 10 );
  vl->addFeature( f1 );

  const QPointer<QgsVectorLayer> vlPtr( vl.get() );

  QgsFeature feat;
  QgsVectorLayerUtils::getFeatureSource( vlPtr )->getFeatures().nextFeature( feat );
  QCOMPARE( feat.attribute( QStringLiteral( "col1" ) ).toInt(), 10 );

  FeatureFetcher *thread = new FeatureFetcher( vlPtr );

  bool finished = false;
  QVariant result;

  auto onResultReady = [&finished, &result]( const QVariant &res ) {
    finished = true;
    result = res;
  };

  connect( thread, &FeatureFetcher::resultReady, this, onResultReady );
  connect( thread, &QThread::finished, thread, &QThread::deleteLater );

  thread->start();
  while ( !finished )
    QCoreApplication::processEvents();
  QCOMPARE( result.toInt(), 10 );
  thread->quit();

  FeatureFetcher *thread2 = new FeatureFetcher( vlPtr );

  finished = false;
  result = QVariant();
  connect( thread2, &FeatureFetcher::resultReady, this, onResultReady );
  connect( thread2, &QThread::finished, thread, &QThread::deleteLater );

  vl.reset();
  thread2->start();
  while ( !finished )
    QCoreApplication::processEvents();
  QVERIFY( result.isNull() );
  thread2->quit();
}

void TestQgsVectorLayerUtils::testFilterValidFeatureIds()
{
  std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=id:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  QgsFeature f1( layer->fields() );
  f1.setAttribute( 0, 1 );
  QgsFeature f2( layer->fields() );
  f2.setAttribute( 0, 2 );
  QgsFeature f3( layer->fields() );
  f3.setAttribute( 0, 3 );
  QgsFeature f4( layer->fields() );
  f4.setAttribute( 0, 4 );
  QgsFeature f5( layer->fields() );
  f5.setAttribute( 0, 5 );

  layer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 << f4 << f5 );
  QCOMPARE( layer->featureCount(), 5L );

  QgsFeatureIds testIds = { 1, 2, 3 };
  QgsFeatureIds result = QgsVectorLayerUtils::filterValidFeatureIds( nullptr, testIds );
  QVERIFY( result.isEmpty() );

  result = QgsVectorLayerUtils::filterValidFeatureIds( layer.get(), QgsFeatureIds() );
  QVERIFY( result.isEmpty() );

  testIds = { 1, 2, 3, 4, 5 };
  result = QgsVectorLayerUtils::filterValidFeatureIds( layer.get(), testIds );
  QCOMPARE( result.size(), 5 );
  QVERIFY( result.contains( 1 ) );
  QVERIFY( result.contains( 2 ) );
  QVERIFY( result.contains( 3 ) );
  QVERIFY( result.contains( 4 ) );
  QVERIFY( result.contains( 5 ) );

  testIds = { 1, 2, 99, 100, 3, 200 };
  result = QgsVectorLayerUtils::filterValidFeatureIds( layer.get(), testIds );
  QCOMPARE( result.size(), 3 );
  QVERIFY( result.contains( 1 ) );
  QVERIFY( result.contains( 2 ) );
  QVERIFY( result.contains( 3 ) );
  QVERIFY( !result.contains( 99 ) );
  QVERIFY( !result.contains( 100 ) );
  QVERIFY( !result.contains( 200 ) );

  testIds = { 99, 100, 200 };
  result = QgsVectorLayerUtils::filterValidFeatureIds( layer.get(), testIds );
  QVERIFY( result.isEmpty() );

  testIds = { -1, 1, 2, -5 };
  result = QgsVectorLayerUtils::filterValidFeatureIds( layer.get(), testIds );
  QCOMPARE( result.size(), 2 );
  QVERIFY( result.contains( 1 ) );
  QVERIFY( result.contains( 2 ) );
  QVERIFY( !result.contains( -1 ) );
  QVERIFY( !result.contains( -5 ) );
}

QGSTEST_MAIN( TestQgsVectorLayerUtils )
#include "testqgsvectorlayerutils.moc"
