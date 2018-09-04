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

/**
 * \ingroup UnitTests
 * This is a unit test for the vector layer class.
 */
class TestQgsVectorLayerUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsVectorLayerUtils() = default;

  private:
    bool mTestHasError =  false ;
    QgsMapLayer *mpPointsLayer = nullptr;
    QgsMapLayer *mpLinesLayer = nullptr;
    QgsMapLayer *mpPolysLayer = nullptr;
    QgsVectorLayer *mpNonSpatialLayer = nullptr;
    QString mTestDataDir;
    QString mReport;

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
      QgsVectorLayerUtils::getFeatureSource( mLayer ).get()->getFeatures().nextFeature( feat );
      emit resultReady( feat.attribute( QStringLiteral( "col1" ) ) );
    }

  signals:
    void resultReady( const QVariant &attribute );

  private:
    QPoinst<QgsVectorLayer> mLayer;
};


void TestQgsVectorLayerUtils::testGetFeatureSource()
{
  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "Point?field=col1:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  vl->startEditing();
  QgsFeature f1( vl->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 10 );
  vl->addFeature( f1 );

  QPointer<QgsVectorLayer> vlPtr( vl );

  QgsFeature feat;
  QgsVectorLayerUtils::getFeatureSource( vlPtr ).get()->getFeatures().nextFeature( feat );
  QCOMPARE( feat.attribute( QStringLiteral( "col1" ) ).toInt(), 10 );

  FeatureFetcher *thread = new FeatureFetcher();

  bool finished = false;
  QVariant result;
  connect( thread, &FeatureFetcher::resultReady, this, [finished, result]( const QVariant & res )
  {
    finished = true;
    result = res;
  } );
  connect( thread, &QThread::finished, thread, &QThread::deleteLater );

  thread->start();
  while ( !finished )
    QCoreApplication::processEvents();
  QCOMPARE( result.toInt(), 10 );

  thread->quit();
}

QGSTEST_MAIN( TestQgsVectorLayerUtils )
#include "testqgsvectorlayerutils.moc"
