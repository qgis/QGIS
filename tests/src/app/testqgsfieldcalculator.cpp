/***************************************************************************
     testqgsfieldcalculator.cpp
     --------------------------
    Date                 : 2016-02-14
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsfieldcalculator.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"

/** \ingroup UnitTests
 * This is a unit test for the field calculator
 */
class TestQgsFieldCalculator : public QObject
{
    Q_OBJECT
  public:
    TestQgsFieldCalculator();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testLengthCalculations();
    void testAreaCalculations();

  private:
    QgisApp * mQgisApp;
};

TestQgsFieldCalculator::TestQgsFieldCalculator()
    : mQgisApp( nullptr )
{

}

//runs before all tests
void TestQgsFieldCalculator::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgsFieldCalculator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFieldCalculator::testLengthCalculations()
{
  //test length calculation respects ellipsoid and project distance units

  //create a temporary layer
  QScopedPointer< QgsVectorLayer> tempLayer( new QgsVectorLayer( "LineString?crs=epsg:3111&field=pk:int&field=col1:double", "vl", "memory" ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( "pk", 1 );
  f1.setAttribute( "col1", 0.0 );
  QgsPolyline line3111;
  line3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 );
  f1.setGeometry( QgsGeometry::fromPolyline( line3111 ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Meters ) );

  // run length calculation
  tempLayer->startEditing();
  QScopedPointer< QgsFieldCalculator > calc( new QgsFieldCalculator( tempLayer.data() ) );

  // this next part is fragile, and may need to be modified if the dialog changes:
  calc->mUpdateExistingGroupBox->setChecked( true );
  calc->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc->builder->setExpressionText( "$length" );
  calc->accept();

  tempLayer->commitChanges();

  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 26932.156;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Feet ) );
  tempLayer->startEditing();
  QScopedPointer< QgsFieldCalculator > calc2( new QgsFieldCalculator( tempLayer.data() ) );
  calc2->mUpdateExistingGroupBox->setChecked( true );
  calc2->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc2->builder->setExpressionText( "$length" );
  calc2->accept();
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 88360.0918635;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );

}

void TestQgsFieldCalculator::testAreaCalculations()
{
  //test area calculation respects ellipsoid and project area units

  //create a temporary layer
  QScopedPointer< QgsVectorLayer> tempLayer( new QgsVectorLayer( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double", "vl", "memory" ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( "pk", 1 );
  f1.setAttribute( "col1", 0.0 );

  QgsPolyline polygonRing3111;
  polygonRing3111 << QgsPoint( 2484588, 2425722 ) << QgsPoint( 2482767, 2398853 ) << QgsPoint( 2520109, 2397715 ) << QgsPoint( 2520792, 2425494 ) << QgsPoint( 2484588, 2425722 );
  QgsPolygon polygon3111;
  polygon3111 << polygonRing3111;
  f1.setGeometry( QgsGeometry::fromPolygon( polygon3111 ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  QgisApp::instance()->mapCanvas()->setCrsTransformEnabled( true );
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMeters ) );

  // run area calculation
  tempLayer->startEditing();
  QScopedPointer< QgsFieldCalculator > calc( new QgsFieldCalculator( tempLayer.data() ) );

  // this next part is fragile, and may need to be modified if the dialog changes:
  calc->mUpdateExistingGroupBox->setChecked( true );
  calc->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc->builder->setExpressionText( "$area" );
  calc->accept();

  tempLayer->commitChanges();

  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 1009089817.0;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 1.0 ) );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMiles ) );
  tempLayer->startEditing();
  QScopedPointer< QgsFieldCalculator > calc2( new QgsFieldCalculator( tempLayer.data() ) );
  calc2->mUpdateExistingGroupBox->setChecked( true );
  calc2->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc2->builder->setExpressionText( "$area" );
  calc2->accept();
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 389.6117565069;
  QVERIFY( qgsDoubleNear( f.attribute( "col1" ).toDouble(), expected, 0.001 ) );
}

QTEST_MAIN( TestQgsFieldCalculator )
#include "testqgsfieldcalculator.moc"
