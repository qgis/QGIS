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

    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testGetFeatureSource();
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
  std::unique_ptr<QgsVectorLayer> vl = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
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

  auto onResultReady = [&finished, &result]( const QVariant & res )
  {
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

QGSTEST_MAIN( TestQgsVectorLayerUtils )
#include "testqgsvectorlayerutils.moc"
