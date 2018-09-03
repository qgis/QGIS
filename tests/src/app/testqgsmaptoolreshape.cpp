/***************************************************************************
  testqgsmaptoolreshape.cpp
  --------------------------------
Date                 : 2017-12-14
Copyright            : (C) 2017 by Paul Blottiere
Email                : paul.blottiere@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtTest/QtTest>
#ifndef Q_MOC_RUN
#include "qgisapp.h"
#endif
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgslinestringv2.h"
#include "qgsmaptoolreshape.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"

class TestQgsMapToolReshape : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolReshape() : mQgisApp( nullptr ) {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void reshapeWithBindingLine();

  private:
    QgisApp *mQgisApp;
};

void TestQgsMapToolReshape::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );

  mQgisApp = new QgisApp();
}

void TestQgsMapToolReshape::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolReshape::init()
{
}

void TestQgsMapToolReshape::cleanup()
{
}

void TestQgsMapToolReshape::reshapeWithBindingLine()
{
  // prepare vector layer
  QgsVectorLayer *vl = new QgsVectorLayer( "LineString?crs=epsg:4326&field=name:string(20)", "vl", "memory" );

  QgsGeometry *g0 = QgsGeometry::fromWkt( "LineString (0 0, 1 1, 1 2)" );
  QgsFeature f0;
  f0.setGeometry( g0 );
  f0.setAttribute( 0, "polyline0" );

  QgsGeometry *g1 = QgsGeometry::fromWkt( "LineString (2 1, 3 2, 3 3, 2 2)" );
  QgsFeature f1;
  f1.setGeometry( g1 );
  f1.setAttribute( 0, "polyline1" );

  vl->dataProvider()->addFeatures( QgsFeatureList() << f0 << f1 );

  std::cout << "Feature count: " << vl->featureCount() << std::endl;

  // prepare canvas
  QList<QgsMapLayer *> layers;
  layers.append( vl );

  QgsCoordinateReferenceSystem srs( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsMapLayerRegistry::instance()->addMapLayer( vl );
  mQgisApp->mapCanvas()->setDestinationCrs( srs );
  mQgisApp->mapCanvas()->setCurrentLayer( vl );

  // reshape to add line to polyline0
  QgsLineStringV2 cl0;
  cl0.setPoints( QgsPointSequenceV2() << QgsPointV2( 1, 2 ) << QgsPointV2( 2, 1 ) );

  QgsAbstractGeometryV2 *curveGgeom0 = cl0.toCurveType();
  QgsCompoundCurveV2 curve0;
  curve0.fromWkt( curveGgeom0->asWkt() );

  QgsMapToolReshape tool0( mQgisApp->mapCanvas() );
  tool0.mCaptureCurve = curve0;

  vl->startEditing();
  tool0.reshape( vl );

  vl->getFeatures( QgsFeatureRequest().setFilterFid( 1 ) ).nextFeature( f0 );
  QCOMPARE( f0.geometry()->exportToWkt(), QString( "LineString (0 0, 1 1, 1 2, 2 1)" ) );

  vl->getFeatures( QgsFeatureRequest().setFilterFid( 2 ) ).nextFeature( f1 );
  QCOMPARE( f1.geometry()->exportToWkt(), QString( "LineString (2 1, 3 2, 3 3, 2 2)" ) );

  vl->rollBack();

  // reshape to add line to polyline1
  QgsLineStringV2 cl1;
  cl1.setPoints( QgsPointSequenceV2() << QgsPointV2( 2, 1 ) << QgsPointV2( 1, 2 ) );

  QgsAbstractGeometryV2 *curveGeom1 = cl1.toCurveType();
  QgsCompoundCurveV2 curve1;
  curve1.fromWkt( curveGeom1->asWkt() );

  QgsMapToolReshape tool1( mQgisApp->mapCanvas() );
  tool1.mCaptureCurve = curve1;

  vl->startEditing();
  tool1.reshape( vl );

  vl->getFeatures( QgsFeatureRequest().setFilterFid( 1 ) ).nextFeature( f0 );
  QCOMPARE( f0.geometry()->exportToWkt(), QString( "LineString (0 0, 1 1, 1 2)" ) );

  vl->getFeatures( QgsFeatureRequest().setFilterFid( 2 ) ).nextFeature( f1 );
  QCOMPARE( f1.geometry()->exportToWkt(), QString( "LineString (1 2, 2 1, 3 2, 3 3, 2 2)" ) );

  vl->rollBack();
}

QTEST_MAIN( TestQgsMapToolReshape )
#include "testqgsmaptoolreshape.moc"
