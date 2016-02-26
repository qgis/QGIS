/***************************************************************************
     testqgsmaptoolidentifyaction.cpp
     --------------------------------
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
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"
#include "qgsmaptoolidentifyaction.h"

class TestQgsMapToolIdentifyAction : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolIdentifyAction()
        : canvas( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.
    void lengthCalculation(); //test calculation of derived length attributes
    void perimeterCalculation(); //test calculation of derived perimeter attribute
    void areaCalculation(); //test calculation of derived area attribute

  private:
    QgsMapCanvas* canvas;
};

void TestQgsMapToolIdentifyAction::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );
}

void TestQgsMapToolIdentifyAction::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolIdentifyAction::init()
{
  canvas = new QgsMapCanvas();
}

void TestQgsMapToolIdentifyAction::cleanup()
{
  delete canvas;
}

void TestQgsMapToolIdentifyAction::lengthCalculation()
{
  QSettings s;
  s.setValue( "/qgis/measure/keepbaseunit", true );

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
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  canvas->setCrsTransformEnabled( true );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry()->boundingBox() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Meters ) );

  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  QScopedPointer< QgsMapToolIdentifyAction > action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  QString derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length" )];
  double length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( length, 26932.2, 0.1 ) );

  //check that project units are respected
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Feet ) );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( length, 88360.1, 0.1 ) );

  //test unchecked "keep base units" setting
  s.setValue( "/qgis/measure/keepbaseunit", false );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( length, 16.735, 0.001 ) );
}

void TestQgsMapToolIdentifyAction::perimeterCalculation()
{
  QSettings s;
  s.setValue( "/qgis/measure/keepbaseunit", true );

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
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  canvas->setCrsTransformEnabled( true );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry()->boundingBox() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Meters ) );

  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  QScopedPointer< QgsMapToolIdentifyAction > action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  QString derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter" )];
  double perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QCOMPARE( perimeter, 128289.074 );

  //check that project units are respected
  QgsProject::instance()->writeEntry( "Measurement", "/DistanceUnits", QgsUnitTypes::encodeUnit( QGis::Feet ) );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( perimeter, 420896.0, 0.1 ) );

  //test unchecked "keep base units" setting
  s.setValue( "/qgis/measure/keepbaseunit", false );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( perimeter, 79.715, 0.001 ) );
}

void TestQgsMapToolIdentifyAction::areaCalculation()
{
  QSettings s;
  s.setValue( "/qgis/measure/keepbaseunit", true );

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
  QgsCoordinateReferenceSystem srs( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  canvas->setCrsTransformEnabled( true );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry()->boundingBox() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", srs.toProj4() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSID", ( int ) srs.srsid() );
  QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCrs", srs.authid() );
  QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "WGS84" ) );
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMeters ) );

  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  QScopedPointer< QgsMapToolIdentifyAction > action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  QString derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area" )];
  double area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( area, 1009089817.0, 1.0 ) );

  //check that project units are respected
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareMiles ) );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( area, 389.6117, 0.001 ) );

  //test unchecked "keep base units" setting
  s.setValue( "/qgis/measure/keepbaseunit", false );
  QgsProject::instance()->writeEntry( "Measurement", "/AreaUnits", QgsUnitTypes::encodeUnit( QgsUnitTypes::SquareFeet ) );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << tempLayer.data() );
  QCOMPARE( result.length(), 1 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QVERIFY( qgsDoubleNear( area, 389.6117, 0.001 ) );
}


QTEST_MAIN( TestQgsMapToolIdentifyAction )
#include "testqgsmaptoolidentifyaction.moc"




