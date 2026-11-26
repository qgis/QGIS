/***************************************************************************
                         testqgsprocessing3dalgs.cpp
                         ---------------------
    begin                : November 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <memory>

#include "qgs3dalgorithms.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsprocessing.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingregistry.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

class TestQgsProcessing3DAlgs : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsProcessing3DAlgs()
      : QgsTest( u"Processing 3D Algorithms Test"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void tessellate_data();
    void tessellate();
};

void TestQgsProcessing3DAlgs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::processingRegistry()->addProvider( new Qgs3DAlgorithms( QgsApplication::processingRegistry() ) );
}

void TestQgsProcessing3DAlgs::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsProcessing3DAlgs::init()
{}

void TestQgsProcessing3DAlgs::cleanup()
{}

void TestQgsProcessing3DAlgs::tessellate_data()
{
  QTest::addColumn<QString>( "layerType" );
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<int>( "expectedNrTriangles" );
  QTest::addColumn<double>( "expectedArea3D" );

  // two square faces: a horizontal one and a vertical one
  QTest::newRow( "polyhedral surface" ) << u"PolyhedralSurfaceZ"_s << u"PolyhedralSurface Z (((0 0 0, 1 0 0, 1 1 0, 0 1 0, 0 0 0)), ((0 0 0, 0 0 1, 1 0 1, 1 0 0, 0 0 0)))"_s << 4 << 2.0;

  QTest::newRow( "multipolygon" ) << u"MultiPolygonZ"_s << u"MultiPolygon Z (((0 0 0, 1 0 0, 1 1 0, 0 1 0, 0 0 0)), ((2 0 0, 3 0 0, 3 1 0, 2 1 0, 2 0 0)))"_s << 4 << 2.0;
}

void TestQgsProcessing3DAlgs::tessellate()
{
  QFETCH( QString, layerType );
  QFETCH( QString, wkt );
  QFETCH( int, expectedNrTriangles );
  QFETCH( double, expectedArea3D );

  QgsVectorLayer *inputLayer = new QgsVectorLayer( u"%1?crs=EPSG:2154"_s.arg( layerType ), u"input"_s, u"memory"_s );
  QVERIFY( inputLayer->isValid() );

  QgsFeature feature;
  feature.setGeometry( QgsGeometry::fromWkt( wkt ) );
  QVERIFY( inputLayer->dataProvider()->addFeature( feature ) );

  QgsProject project;
  project.addMapLayer( inputLayer );

  QgsProcessingContext context;
  context.setProject( &project );
  QgsProcessingFeedback feedback;

  std::unique_ptr<QgsProcessingAlgorithm> alg( QgsApplication::processingRegistry()->createAlgorithmById( u"3d:tessellate"_s ) );
  QVERIFY( alg != nullptr );

  QVariantMap parameters;
  parameters.insert( u"INPUT"_s, inputLayer->id() );
  parameters.insert( u"OUTPUT"_s, QgsProcessing::TEMPORARY_OUTPUT );

  bool ok = false;
  const QVariantMap results = alg->run( parameters, context, &feedback, &ok );
  QVERIFY( ok );

  QgsVectorLayer *outputLayer = qobject_cast<QgsVectorLayer *>( context.getMapLayer( results.value( u"OUTPUT"_s ).toString() ) );
  QVERIFY( outputLayer );
  QVERIFY( outputLayer->isValid() );
  QCOMPARE( outputLayer->featureCount(), 1 );

  QgsFeature outputFeature;
  QVERIFY( outputLayer->getFeatures().nextFeature( outputFeature ) );

  const QgsGeometry outputGeometry = outputFeature.geometry();
  QCOMPARE( outputGeometry.wkbType(), Qgis::WkbType::MultiPolygonZ );
  QCOMPARE( outputGeometry.constGet()->partCount(), expectedNrTriangles );

  QCOMPARE( outputGeometry.area3D(), expectedArea3D );
}

QGSTEST_MAIN( TestQgsProcessing3DAlgs )
#include "testqgsprocessing3dalgs.moc"
