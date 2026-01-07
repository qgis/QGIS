/***************************************************************************
     testqgsvectorlayerutils.cpp
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
#include <memory>

#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"

#include <QMetaType>
#include <QThread>
#include <QtGlobal>

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
    void testGetValues();
    void testUniqueValues();

  private:
    QString mTestDataDir;
};

void TestQgsVectorLayerUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
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
      emit resultReady( feat.attribute( u"col1"_s ) );
    }

  signals:
    void resultReady( const QVariant &attribute );

  private:
    QPointer<QgsVectorLayer> mLayer;
};


void TestQgsVectorLayerUtils::testGetFeatureSource()
{
  auto vl = std::make_unique<QgsVectorLayer>( u"Point?field=col1:integer"_s, u"vl"_s, u"memory"_s );
  vl->startEditing();
  QgsFeature f1( vl->fields(), 1 );
  f1.setAttribute( u"col1"_s, 10 );
  vl->addFeature( f1 );

  const QPointer<QgsVectorLayer> vlPtr( vl.get() );

  QgsFeature feat;
  QgsVectorLayerUtils::getFeatureSource( vlPtr )->getFeatures().nextFeature( feat );
  QCOMPARE( feat.attribute( u"col1"_s ).toInt(), 10 );

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

void TestQgsVectorLayerUtils::testGetValues()
{
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );

  auto pointsLayer = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );
  QVERIFY( pointsLayer->isValid() );

  // from an attribute
  bool retrievedValues = false;
  QList<QVariant> valuesAttr = QgsVectorLayerUtils::getValues( pointsLayer.get(), u"Class"_s, retrievedValues );
  QVERIFY( retrievedValues );
  QCOMPARE( valuesAttr.size(), 17 );
  QCOMPARE( valuesAttr[0].typeId(), QMetaType::QString );

  int nrJet = 0;
  int nrBiPlane = 0;
  int nrB52 = 0;
  for ( const QVariant &value : valuesAttr )
  {
    if ( value.toString() == "Jet" )
    {
      nrJet += 1;
    }
    else if ( value.toString() == "Biplane" )
    {
      nrBiPlane += 1;
    }
    else if ( value.toString() == "B52" )
    {
      nrB52 += 1;
    }
  }

  QCOMPARE( nrJet, 8 );
  QCOMPARE( nrBiPlane, 5 );
  QCOMPARE( nrB52, 4 );

  // from an expression
  retrievedValues = false;
  QList<QVariant> valuesExp = QgsVectorLayerUtils::getValues( pointsLayer.get(), u"Pilots+4"_s, retrievedValues );
  QVERIFY( retrievedValues );
  QCOMPARE( valuesExp.size(), 17 );
  QCOMPARE( valuesExp[0].typeId(), QMetaType::LongLong );

  double minVal = std::numeric_limits<int>::max();
  double maxVal = -std::numeric_limits<int>::max();
  for ( const QVariant &value : valuesExp )
  {
    const double valI = value.toInt();
    if ( valI > maxVal )
    {
      maxVal = valI;
    }
    if ( valI < minVal )
    {
      minVal = valI;
    }
  }

  QCOMPARE( minVal, 5 );
  QCOMPARE( maxVal, 7 );
}

void TestQgsVectorLayerUtils::testUniqueValues()
{
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );

  auto pointsLayer = std::make_unique<QgsVectorLayer>( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), u"ogr"_s );

  // from an attribute
  {
    bool retrievedValues = false;
    QList<QVariant> valuesAttr = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Class"_s, retrievedValues );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesAttr.size(), 3 );
    QCOMPARE( valuesAttr[0].typeId(), QMetaType::QString );

    std::sort( valuesAttr.begin(), valuesAttr.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toString() < b.toString();
    } );

    QList<QVariant> expectedAttr;
    expectedAttr << u"B52"_s << u"Biplane"_s << u"Jet"_s;
    QCOMPARE( valuesAttr, expectedAttr );
  }

  // from an attribute - limit to 2 values
  {
    bool retrievedValues = false;
    QList<QVariant> valuesAttr = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Class"_s, retrievedValues, false, 2 );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesAttr.size(), 2 );
    QCOMPARE( valuesAttr[0].typeId(), QMetaType::QString );

    std::sort( valuesAttr.begin(), valuesAttr.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toString() < b.toString();
    } );

    QList<QVariant> expectedAttr;
    expectedAttr << u"B52"_s << u"Biplane"_s;
    QCOMPARE( valuesAttr, expectedAttr );
  }

  // from an attribute - selected only
  {
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
    QgsFeatureIds selectedFeatures = { 1, 4, 7 };
    pointsLayer->selectByIds( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 3 );

    bool retrievedValues = false;
    QList<QVariant> valuesAttr = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Class"_s, retrievedValues, true );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesAttr.size(), 2 );
    QCOMPARE( valuesAttr[0].typeId(), QMetaType::QString );

    std::sort( valuesAttr.begin(), valuesAttr.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toString() < b.toString();
    } );

    QList<QVariant> expectedAttr;
    expectedAttr << u"Biplane"_s << u"Jet"_s;
    QCOMPARE( valuesAttr, expectedAttr );

    pointsLayer->deselect( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
  }

  // from an attribute - selected only - limit to 1
  {
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
    QgsFeatureIds selectedFeatures = { 1, 4, 7 };
    pointsLayer->selectByIds( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 3 );

    bool retrievedValues = false;
    const QList<QVariant> valuesAttr = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Class"_s, retrievedValues, true, 1 );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesAttr.size(), 1 );
    QCOMPARE( valuesAttr[0].typeId(), QMetaType::QString );

    QList<QVariant> expectedAttr = { u"Biplane"_s };
    QCOMPARE( valuesAttr, expectedAttr );

    pointsLayer->deselect( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
  }

  // from an attribute - layer with a null value
  {
    auto uniqueLayer = std::make_unique<QgsVectorLayer>( "Point?crs=EPSG:4326", "points_test", "memory" );
    QVERIFY( uniqueLayer->isValid() );
    uniqueLayer->dataProvider()->addAttributes( { QgsField( "name", QMetaType::Type::QString ) } );
    uniqueLayer->updateFields();

    QList<QgsFeature> features;
    for ( int i = 0; i < 5; i++ )
    {
      QgsFeature feature( uniqueLayer->fields() );
      feature.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( i, i ) ) );
      if ( i != 2 )
      {
        feature.setAttribute( "name", QString( "Point_%1" ).arg( i ) );
      }
      features.append( feature );
    }
    uniqueLayer->dataProvider()->addFeatures( features );

    bool retrievedValues = false;
    QList<QVariant> valuesAttr = QgsVectorLayerUtils::uniqueValues( uniqueLayer.get(), u"name"_s, retrievedValues );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesAttr.size(), 4 );
    std::sort( valuesAttr.begin(), valuesAttr.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toString() < b.toString();
    } );

    QList<QVariant> expectedAttr;
    expectedAttr << u"Point_0"_s << u"Point_1"_s << u"Point_3"_s << u"Point_4"_s;
    QCOMPARE( valuesAttr, expectedAttr );
  }

  // from an expression
  {
    bool retrievedValues = false;
    QList<QVariant> valuesExp = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Pilots+4"_s, retrievedValues );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesExp.size(), 3 );
    QCOMPARE( valuesExp[0].typeId(), QMetaType::LongLong );
    std::sort( valuesExp.begin(), valuesExp.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toInt() < b.toInt();
    } );
    QList<QVariant> expectedExp;
    expectedExp << 5 << 6 << 7;
    QCOMPARE( valuesExp, expectedExp );
  }

  // from an expression - limit to 2 values
  {
    bool retrievedValues = false;
    QList<QVariant> valuesExp = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Pilots+4"_s, retrievedValues, false, 2 );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesExp.size(), 2 );
    QCOMPARE( valuesExp[0].typeId(), QMetaType::LongLong );
    std::sort( valuesExp.begin(), valuesExp.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toInt() < b.toInt();
    } );
    QList<QVariant> expectedExp;
    expectedExp << 6 << 7;
    QCOMPARE( valuesExp, expectedExp );
  }

  // from an expression - selected only
  {
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
    QgsFeatureIds selectedFeatures = { 1, 8, 9 };
    pointsLayer->selectByIds( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 3 );

    bool retrievedValues = false;
    QList<QVariant> valuesExp = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Pilots+4"_s, retrievedValues, true );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesExp.size(), 2 );
    QCOMPARE( valuesExp[0].typeId(), QMetaType::LongLong );
    std::sort( valuesExp.begin(), valuesExp.end(), []( const QVariant &a, const QVariant &b ) {
      return a.toInt() < b.toInt();
    } );
    QList<QVariant> expectedExp;
    expectedExp << 6 << 7;
    QCOMPARE( valuesExp, expectedExp );

    pointsLayer->deselect( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
  }

  // from an expression - selected only - limit to 1
  {
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
    QgsFeatureIds selectedFeatures = { 1, 8, 9 };
    pointsLayer->selectByIds( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 3 );

    bool retrievedValues = false;
    QList<QVariant> valuesExp = QgsVectorLayerUtils::uniqueValues( pointsLayer.get(), u"Pilots+4"_s, retrievedValues, true, 1 );
    QVERIFY( retrievedValues );
    QCOMPARE( valuesExp.size(), 1 );
    QCOMPARE( valuesExp[0].typeId(), QMetaType::LongLong );
    QList<QVariant> expectedExp = { 7 };
    QCOMPARE( valuesExp, expectedExp );

    pointsLayer->deselect( selectedFeatures );
    QCOMPARE( pointsLayer->selectedFeatureCount(), 0 );
  }
}

QGSTEST_MAIN( TestQgsVectorLayerUtils )
#include "testqgsvectorlayerutils.moc"
