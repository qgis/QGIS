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
#include "qgstest.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsfieldcalculator.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"

/**
 * \ingroup UnitTests
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
    QgisApp *mQgisApp = nullptr;
};

TestQgsFieldCalculator::TestQgsFieldCalculator() = default;

//runs before all tests
void TestQgsFieldCalculator::initTestCase()
{
  qDebug() << "TestQgsFieldCalculator::initTestCase()";
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
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolylineXY line3111;
  line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
  const QgsGeometry line3111G = QgsGeometry::fromPolylineXY( line3111 ) ;
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceMeters );

  // run length calculation
  tempLayer->startEditing();
  std::unique_ptr< QgsFieldCalculator > calc( new QgsFieldCalculator( tempLayer.get() ) );

  // this next part is fragile, and may need to be modified if the dialog changes:
  calc->mUpdateExistingGroupBox->setChecked( true );
  calc->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc->builder->setExpressionText( QStringLiteral( "$length" ) );
  calc->accept();

  tempLayer->commitChanges();

  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 26932.156;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceFeet );
  tempLayer->startEditing();
  std::unique_ptr< QgsFieldCalculator > calc2( new QgsFieldCalculator( tempLayer.get() ) );
  calc2->mUpdateExistingGroupBox->setChecked( true );
  calc2->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc2->builder->setExpressionText( QStringLiteral( "$length" ) );
  calc2->accept();
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 88360.0918635;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );

}

void TestQgsFieldCalculator::testAreaCalculations()
{
  //test area calculation respects ellipsoid and project area units

  //create a temporary layer
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );

  QgsPolylineXY polygonRing3111;
  polygonRing3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 ) << QgsPointXY( 2520109, 2397715 ) << QgsPointXY( 2520792, 2425494 ) << QgsPointXY( 2484588, 2425722 );
  QgsPolygonXY polygon3111;
  polygon3111 << polygonRing3111;
  const QgsGeometry polygon3111G = QgsGeometry::fromPolygonXY( polygon3111 ) ;
  f1.setGeometry( polygon3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMeters );

  // run area calculation
  tempLayer->startEditing();
  std::unique_ptr< QgsFieldCalculator > calc( new QgsFieldCalculator( tempLayer.get() ) );

  // this next part is fragile, and may need to be modified if the dialog changes:
  calc->mUpdateExistingGroupBox->setChecked( true );
  calc->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc->builder->setExpressionText( QStringLiteral( "$area" ) );
  calc->accept();

  tempLayer->commitChanges();

  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 1005755617.819130;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMiles );
  tempLayer->startEditing();
  std::unique_ptr< QgsFieldCalculator > calc2( new QgsFieldCalculator( tempLayer.get() ) );
  calc2->mUpdateExistingGroupBox->setChecked( true );
  calc2->mExistingFieldComboBox->setCurrentIndex( 1 );
  calc2->builder->setExpressionText( QStringLiteral( "$area" ) );
  calc2->accept();
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 388.324420;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );
}

QGSTEST_MAIN( TestQgsFieldCalculator )
#include "testqgsfieldcalculator.moc"
