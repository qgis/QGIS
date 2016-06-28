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
#include "qgsrasterlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"
#include "qgsmaptoolidentifyaction.h"

#include "cpl_conv.h"

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
    void identifyRasterFloat32(); // test pixel identification and decimal precision
    void identifyRasterFloat64(); // test pixel identification and decimal precision
    void identifyInvalidPolygons(); // test selecting invalid polygons

  private:
    QgsMapCanvas* canvas;

    QString testIdentifyRaster( QgsRasterLayer* layer, double xGeoref, double yGeoref );
    QList<QgsMapToolIdentify::IdentifyResult> testIdentifyVector( QgsVectorLayer* layer, double xGeoref, double yGeoref );

    // Release return with delete []
    unsigned char *
    hex2bytes( const char *hex, int *size )
    {
      QByteArray ba = QByteArray::fromHex( hex );
      unsigned char *out = new unsigned char[ba.size()];
      memcpy( out, ba.data(), ba.size() );
      *size = ba.size();
      return out;
    }

    // TODO: make this a QgsGeometry member...
    QgsGeometry geomFromHexWKB( const char *hexwkb )
    {
      int wkbsize;
      unsigned char *wkb = hex2bytes( hexwkb, &wkbsize );
      QgsGeometry geom;
      // NOTE: QgsGeometry takes ownership of wkb
      geom.fromWkb( wkb, wkbsize );
      return geom;
    }

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

// private
QString TestQgsMapToolIdentifyAction::testIdentifyRaster( QgsRasterLayer* layer, double xGeoref, double yGeoref )
{
  QScopedPointer< QgsMapToolIdentifyAction > action( new QgsMapToolIdentifyAction( canvas ) );
  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << layer );
  if ( result.length() != 1 )
    return "";
  return result[0].mAttributes["Band 1"];
}

// private
QList<QgsMapToolIdentify::IdentifyResult>
TestQgsMapToolIdentifyAction::testIdentifyVector( QgsVectorLayer* layer, double xGeoref, double yGeoref )
{
  QScopedPointer< QgsMapToolIdentifyAction > action( new QgsMapToolIdentifyAction( canvas ) );
  QgsPoint mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer*>() << layer );
  return result;
}

void TestQgsMapToolIdentifyAction::identifyRasterFloat32()
{
  //create a temporary layer
  QString raster = QString( TEST_DATA_DIR ) + "/raster/test.asc";

  // By default the QgsRasterLayer forces AAIGRID_DATATYPE=Float64
  CPLSetConfigOption( "AAIGRID_DATATYPE", "Float32" );
  QScopedPointer< QgsRasterLayer> tempLayer( new QgsRasterLayer( raster ) );
  CPLSetConfigOption( "AAIGRID_DATATYPE", nullptr );

  QVERIFY( tempLayer->isValid() );

  canvas->setExtent( QgsRectangle( 0, 0, 7, 1 ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 0.5, 0.5 ), QString( "-999.9" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 1.5, 0.5 ), QString( "-999.987" ) );

  // More than 6 significant digits for corresponding value in .asc:
  // precision loss in Float32
  QCOMPARE( testIdentifyRaster( tempLayer.data(), 2.5, 0.5 ), QString( "1.234568" ) ); // in .asc file : 1.2345678

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 3.5, 0.5 ), QString( "123456" ) );

  // More than 6 significant digits: no precision loss here for that particular value
  QCOMPARE( testIdentifyRaster( tempLayer.data(), 4.5, 0.5 ), QString( "1234567" ) );

  // More than 6 significant digits: no precision loss here for that particular value
  QCOMPARE( testIdentifyRaster( tempLayer.data(), 5.5, 0.5 ), QString( "-999.9876" ) );

  // More than 6 significant digits for corresponding value in .asc:
  // precision loss in Float32
  QCOMPARE( testIdentifyRaster( tempLayer.data(), 6.5, 0.5 ), QString( "1.234568" ) ); // in .asc file : 1.2345678901234
}

void TestQgsMapToolIdentifyAction::identifyRasterFloat64()
{
  //create a temporary layer
  QString raster = QString( TEST_DATA_DIR ) + "/raster/test.asc";
  QScopedPointer< QgsRasterLayer> tempLayer( new QgsRasterLayer( raster ) );
  QVERIFY( tempLayer->isValid() );

  canvas->setExtent( QgsRectangle( 0, 0, 7, 1 ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 0.5, 0.5 ), QString( "-999.9" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 1.5, 0.5 ), QString( "-999.987" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 2.5, 0.5 ), QString( "1.2345678" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 3.5, 0.5 ), QString( "123456" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 4.5, 0.5 ), QString( "1234567" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 5.5, 0.5 ), QString( "-999.9876" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.data(), 6.5, 0.5 ), QString( "1.2345678901234" ) );
}

void TestQgsMapToolIdentifyAction::identifyInvalidPolygons()
{
  //create a temporary layer
  QScopedPointer< QgsVectorLayer > memoryLayer( new QgsVectorLayer( "Polygon?field=pk:int", "vl", "memory" ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature f1( memoryLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( "pk", 1 );
  // This geometry is an invalid polygon (3 distinct vertices).
  // GEOS reported invalidity: Points of LinearRing do not form a closed linestring
  f1.setGeometry( geomFromHexWKB(
                    "010300000001000000030000000000000000000000000000000000000000000000000024400000000000000000000000000000244000000000000024400000000000000000"
                  ) );
  // TODO: check why we need the ->dataProvider() part, since
  //       there's a QgsVectorLayer::addFeatures method too
  //memoryLayer->addFeatures( QgsFeatureList() << f1 );
  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  canvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  QList<QgsMapToolIdentify::IdentifyResult> identified;
  identified = testIdentifyVector( memoryLayer.data(), 4, 6 );
  QCOMPARE( identified.length(), 0 );
  identified = testIdentifyVector( memoryLayer.data(), 6, 4 );
  QCOMPARE( identified.length(), 1 );
  QCOMPARE( identified[0].mFeature.attribute( "pk" ), QVariant( 1 ) );

}


QTEST_MAIN( TestQgsMapToolIdentifyAction )
#include "testqgsmaptoolidentifyaction.moc"




