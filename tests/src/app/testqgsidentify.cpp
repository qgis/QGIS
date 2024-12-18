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

#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectortilelayer.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsmeshlayer.h"
#include "qgsmaptoolidentifyaction.h"
#include "qgssettings.h"
#include "qgsidentifymenu.h"
#include "qgisapp.h"
#include "qgsaction.h"
#include "qgsactionmanager.h"
#include "qgsactionmenu.h"
#include "qgsidentifyresultsdialog.h"
#include "qgsmapmouseevent.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsconfig.h"
#include "qgspointcloudlayer.h"

#include <QTimer>

#include "cpl_conv.h"

class TestQgsIdentify : public QObject
{
    Q_OBJECT
  public:
    TestQgsIdentify() = default;

  private slots:
    void initTestCase();          // will be called before the first testfunction is executed.
    void cleanupTestCase();       // will be called after the last testfunction was executed.
    void init();                  // will be called before each testfunction is executed.
    void cleanup();               // will be called after every testfunction.
    void lengthCalculation();     //test calculation of derived length attributes
    void perimeterCalculation();  //test calculation of derived perimeter attribute
    void areaCalculation();       //test calculation of derived area attribute
    void identifyRasterFloat32(); // test pixel identification and decimal precision
    void identifyRasterFloat64(); // test pixel identification and decimal precision
    void identifyRasterTemporal();
    void identifyRasterDerivedAttributes(); // test derived pixel attributes
    void identifyMesh();                    // test identification for mesh layer
    void identifyVectorTile();              // test identification for vector tile layer
    void identifyInvalidPolygons();         // test selecting invalid polygons
    void clickxy();                         // test if click_x and click_y variables are propagated
    void closestPoint();
    void testRelations();
    void testPointZ();
    void testLineStringZ();
    void testPolygonZ();
    void identifyPointCloud();
    void identifyVirtualPointCloud();

  private:
    void doAction();

    QgsMapCanvas *canvas = nullptr;
    QgsMapToolIdentifyAction *mIdentifyAction = nullptr;
    QgisApp *mQgisApp = nullptr;

    QList<QgsMapToolIdentify::IdentifyResult> testIdentifyRaster( QgsRasterLayer *layer, double xGeoref, double yGeoref, bool roundToCanvasPixels = true );
    QList<QgsMapToolIdentify::IdentifyResult> testIdentifyVector( QgsVectorLayer *layer, double xGeoref, double yGeoref );
    QList<QgsMapToolIdentify::IdentifyResult> testIdentifyMesh( QgsMeshLayer *layer, double xGeoref, double yGeoref );
    QList<QgsMapToolIdentify::IdentifyResult> testIdentifyVectorTile( QgsVectorTileLayer *layer, double xGeoref, double yGeoref );

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

void TestQgsIdentify::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::showSettings();

  // enforce C locale because the tests expect it
  // (decimal separators / thousand separators)
  QLocale::setDefault( QLocale::c() );

  mQgisApp = new QgisApp();
}

void TestQgsIdentify::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsIdentify::init()
{
  canvas = new QgsMapCanvas();
}

void TestQgsIdentify::cleanup()
{
  delete canvas;
}

void TestQgsIdentify::doAction()
{
  bool ok = false;
  const int clickxOk = 2484588;
  const int clickyOk = 2425722;

  // test QActionMenu
  QList<QAction *> actions = mIdentifyAction->identifyMenu()->actions();
  bool testDone = false;

  for ( int i = 0; i < actions.count(); i++ )
  {
    if ( actions[i]->text().compare( "MyAction" ) == 0 )
    {
      const QgsActionMenu::ActionData data = actions[i]->data().value<QgsActionMenu::ActionData>();
      const QgsAction act = data.actionData.value<QgsAction>();

      const int clickx = act.expressionContextScope().variable( "click_x" ).toString().toInt( &ok, 10 );
      QCOMPARE( clickx, clickxOk );

      const int clicky = act.expressionContextScope().variable( "click_y" ).toString().toInt( &ok, 10 );
      QCOMPARE( clicky, clickyOk );

      testDone = true;
    }
  }

  QCOMPARE( testDone, true );

  // test QgsIdentifyResultsDialog expression context scope
  QgsIdentifyResultsDialog *dlg = mIdentifyAction->resultsDialog();
  const int clickx = dlg->expressionContextScope().variable( "click_x" ).toString().toInt( &ok, 10 );
  QCOMPARE( clickx, clickxOk );

  const int clicky = dlg->expressionContextScope().variable( "click_y" ).toString().toInt( &ok, 10 );
  QCOMPARE( clicky, clickyOk );

  // close
  mIdentifyAction->identifyMenu()->close();
}

void TestQgsIdentify::clickxy()
{
  // create temp layer
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3111" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  // add feature
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  const QgsPointXY wordPoint( 2484588, 2425722 );
  const QgsGeometry geom = QgsGeometry::fromPointXY( wordPoint );
  f1.setGeometry( geom );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // prepare canvas
  QList<QgsMapLayer *> layers;
  layers.append( tempLayer.get() );

  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  canvas->setDestinationCrs( srs );
  canvas->setLayers( layers );
  canvas->setCurrentLayer( tempLayer.get() );

  // create/add action
  QgsAction act( Qgis::AttributeActionType::GenericPython, "MyAction", "", true );

  QSet<QString> scopes;
  scopes << "Feature";
  act.setActionScopes( scopes );
  tempLayer->actions()->addAction( act );

  // init map tool identify action
  mIdentifyAction = new QgsMapToolIdentifyAction( canvas );

  // simulate a click on the canvas
  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );
  const QPoint point = QPoint( mapPoint.x(), mapPoint.y() );
  QMouseEvent releases( QEvent::MouseButtonRelease, point, Qt::RightButton, Qt::LeftButton, Qt::NoModifier );
  QgsMapMouseEvent mapReleases( nullptr, &releases );

  // simulate a click on the corresponding action
  QTimer::singleShot( 2000, this, &TestQgsIdentify::doAction );
  mIdentifyAction->canvasReleaseEvent( &mapReleases );
}

void TestQgsIdentify::closestPoint()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=epsg:3111&field=pk:int&field=col1:double&field=url:string" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 3.3 );
  f1.setAttribute( QStringLiteral( "url" ), QStringLiteral( "home: http://qgis.org" ) );
  QgsPolylineXY line3111;
  line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
  const QgsGeometry line3111G = QgsGeometry::fromWkt( QStringLiteral( "LineStringZM( 2484588 2425722 11 31, 2484588 2398853 15 37)" ) );
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry().boundingBox() );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 2484587, 2399800 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QgsIdentifyResultsDialog *dlg = action->resultsDialog();

  //check that closest point attributes are present
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest X" )], QStringLiteral( "2484588.000" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest Y" )], QStringLiteral( "2399800.000" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Interpolated M" )].left( 4 ), QStringLiteral( "36.7" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Interpolated Z" )].left( 4 ), QStringLiteral( "14.8" ) );
  dlg->addFeature( result.at( 0 ) );

  QTreeWidgetItem *layerItem = dlg->layerItem( tempLayer.get() );
  QVERIFY( layerItem );
  QTreeWidgetItem *featureItem = layerItem->child( 0 );
  QTreeWidgetItem *derivedItem = featureItem->child( 0 );
  QTreeWidgetItem *closestXItem = nullptr;
  for ( int row = 0; row < derivedItem->childCount(); ++row )
  {
    if ( derivedItem->child( row )->text( 0 ) == tr( "Closest X" ) )
    {
      closestXItem = derivedItem->child( row );
      break;
    }
  }
  QVERIFY( closestXItem );
  QCOMPARE( closestXItem->text( 1 ), QStringLiteral( "2484588.000" ) );
  QCOMPARE( dlg->retrieveAttribute( closestXItem ).toString(), QStringLiteral( "2484588.000" ) );

  QTreeWidgetItem *col1Item = nullptr;
  QTreeWidgetItem *urlAttributeItem = nullptr;
  for ( int row = 0; row < featureItem->childCount(); ++row )
  {
    if ( featureItem->child( row )->text( 0 ) == tr( "col1" ) )
    {
      col1Item = featureItem->child( row );
    }
    else if ( featureItem->child( row )->text( 0 ) == tr( "url" ) )
    {
      urlAttributeItem = featureItem->child( row );
    }
  }
  QVERIFY( col1Item );
  QCOMPARE( col1Item->text( 1 ), QStringLiteral( "3.30000" ) );
  QCOMPARE( dlg->retrieveAttribute( col1Item ).toString(), QStringLiteral( "3.30000" ) );
  QVERIFY( urlAttributeItem );
  // urlAttributeItem has a delegate widget, but we should still be able to retrieve the raw field value from it
  QCOMPARE( dlg->retrieveAttribute( urlAttributeItem ).toString(), QStringLiteral( "home: http://qgis.org" ) );

  // polygons
  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> tempLayer2( new QgsVectorLayer( QStringLiteral( "PolygonZM?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer2->isValid() );
  f1 = QgsFeature( tempLayer2->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );

  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PolygonZM((2484588 2425722 1 11, 2484588 2398853 2 12, 2520109 2397715 3 13, 2520792 2425494 4 14, 2484588 2425722 1 11))" ) ) );
  QVERIFY( f1.hasGeometry() );
  tempLayer2->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  mapPoint = canvas->getCoordinateTransform()->transform( 2484589, 2399800 );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer2.get() );
  QCOMPARE( result.length(), 1 );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest X" )], QStringLiteral( "2484588.000" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest Y" )], QStringLiteral( "2399800.000" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Interpolated M" )].left( 4 ), QStringLiteral( "11.9" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Interpolated Z" )].left( 4 ), QStringLiteral( "1.96" ) );

  QgsProject::instance()->displaySettings()->setCoordinateAxisOrder( Qgis::CoordinateOrder::YX );
  result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << tempLayer2.get() );
  QCOMPARE( result.length(), 1 );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest X" )], QStringLiteral( "2484588.000" ) );
  QCOMPARE( result.at( 0 ).mDerivedAttributes[tr( "Closest Y" )], QStringLiteral( "2399800.000" ) );
}

void TestQgsIdentify::lengthCalculation()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolylineXY line3111;
  line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
  const QgsGeometry line3111G = QgsGeometry::fromPolylineXY( line3111 );
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry().boundingBox() );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  QString derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  double length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 26932.2, 0.1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 26930.6, 0.1 );

  //check that project units are respected
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Feet );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 88360.1, 0.1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 88355.1, 0.1 );

  //test unchecked "keep base units" setting
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), false );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 16.735, 0.001 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 16.734000, 0.001 );

  // no conversion of Cartesian lengths between unit types
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Degrees );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 0.242000, 0.001 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 26930.6, 0.1 );

  // LineString with Z
  tempLayer = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringZ?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineStringZ(2484588 2425722 10, 2482767 2398853 1000)" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 0.242000, 0.001 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian — 2D)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 26930.6, 0.1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian — 3D)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 26948.827000, 0.1 );

  // CircularString with Z (no length 3d for now, not supported by circular string API)
  tempLayer = std::make_unique<QgsVectorLayer>( QStringLiteral( "CircularStringZ?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "CircularStringZ(2484588 2425722 10, 2483588 2429722 10, 2482767 2398853 1000)" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 2.5880, 0.001 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 288140.206, 0.1 );

  // MultiLineString with Z
  tempLayer = std::make_unique<QgsVectorLayer>( QStringLiteral( "MultiLineStringZ?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );
  QVERIFY( tempLayer->isValid() );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineStringZ((2484588 2425722 10, 2482767 2398853 1000), (2494588 2435722 10, 2422767 2318853 1000))" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Ellipsoidal — WGS84)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 1.4740, 0.001 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian — 2D)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 164104.319, 0.1 );
  derivedLength = result.at( 0 ).mDerivedAttributes[tr( "Length (Cartesian — 3D)" )];
  length = derivedLength.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( length, 164126.083, 0.1 );
}

void TestQgsIdentify::perimeterCalculation()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolylineXY polygonRing3111;
  polygonRing3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 ) << QgsPointXY( 2520109, 2397715 ) << QgsPointXY( 2520792, 2425494 ) << QgsPointXY( 2484588, 2425722 );
  QgsPolygonXY polygon3111;
  polygon3111 << polygonRing3111;
  const QgsGeometry polygon3111G = QgsGeometry::fromPolygonXY( polygon3111 );
  f1.setGeometry( polygon3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry().boundingBox() );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  QString derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Ellipsoidal — WGS84)" )];
  double perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QCOMPARE( perimeter, 128289.074 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Cartesian)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QCOMPARE( perimeter, 128282.086 );

  //check that project units are respected
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Feet );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Ellipsoidal — WGS84)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( perimeter, 420896.0, 0.1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Cartesian)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( perimeter, 420873.0, 0.1 );

  //test unchecked "keep base units" setting
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), false );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Ellipsoidal — WGS84)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( perimeter, 79.715, 0.001 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Cartesian)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QCOMPARE( perimeter, 79.711 );

  // no conversion of Cartesian lengths between unit types
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Degrees );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Ellipsoidal — WGS84)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( perimeter, 1.152000, 0.001 );
  derivedPerimeter = result.at( 0 ).mDerivedAttributes[tr( "Perimeter (Cartesian)" )];
  perimeter = derivedPerimeter.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( perimeter, 128282, 0.1 );
}

void TestQgsIdentify::areaCalculation()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );

  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );

  QgsPolylineXY polygonRing3111;
  polygonRing3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 ) << QgsPointXY( 2520109, 2397715 ) << QgsPointXY( 2520792, 2425494 ) << QgsPointXY( 2484588, 2425722 );
  QgsPolygonXY polygon3111;
  polygon3111 << polygonRing3111;
  const QgsGeometry polygon3111G = QgsGeometry::fromPolygonXY( polygon3111 );
  f1.setGeometry( polygon3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( f1.geometry().boundingBox() );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMeters );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 2484588, 2425722 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  QString derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Ellipsoidal — WGS84)" )];
  double area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 1005755617.819000, 1.0 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Cartesian)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 1005640568.0, 1.0 );

  //check that project units are respected
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMiles );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Ellipsoidal — WGS84)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 388.324000, 0.001 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Cartesian)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 388.280000, 0.001 );

  //test unchecked "keep base units" setting
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), false );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareFeet );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Ellipsoidal — WGS84)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 388.324000, 0.001 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Cartesian)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 388.280000, 0.001 );

  // no conversion of Cartesian lengths between unit types
  s.setValue( QStringLiteral( "/qgis/measure/keepbaseunit" ), true );
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareDegrees );
  result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Ellipsoidal — WGS84)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 0.081000, 0.001 );
  derivedArea = result.at( 0 ).mDerivedAttributes[tr( "Area (Cartesian)" )];
  area = derivedArea.remove( ',' ).split( ' ' ).at( 0 ).toDouble();
  QGSCOMPARENEAR( area, 1005640568.6, 1 );
}

// private
QList<QgsMapToolIdentify::IdentifyResult> TestQgsIdentify::testIdentifyRaster( QgsRasterLayer *layer, double xGeoref, double yGeoref, bool roundToCanvasPixels )
{
  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );

  QgsIdentifyContext identifyContext;
  if ( canvas->mapSettings().isTemporal() )
    identifyContext.setTemporalRange( canvas->temporalRange() );

  if ( roundToCanvasPixels )
    return action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << layer, QgsMapToolIdentify::DefaultQgsSetting, identifyContext );
  else
    return action->identify( QgsGeometry::fromPointXY( QgsPointXY( xGeoref, yGeoref ) ), QgsMapToolIdentify::DefaultQgsSetting, { layer }, QgsMapToolIdentify::AllLayers, identifyContext );
}

// private
QList<QgsMapToolIdentify::IdentifyResult> TestQgsIdentify::testIdentifyMesh( QgsMeshLayer *layer, double xGeoref, double yGeoref )
{
  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );
  //check that closest point attributes are present
  QgsIdentifyContext identifyContext;
  if ( canvas->mapSettings().isTemporal() )
    identifyContext.setTemporalRange( canvas->temporalRange() );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << layer, QgsMapToolIdentify::DefaultQgsSetting, identifyContext );
  return result;
}

// private
QList<QgsMapToolIdentify::IdentifyResult>
  TestQgsIdentify::testIdentifyVector( QgsVectorLayer *layer, double xGeoref, double yGeoref )
{
  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );
  QgsIdentifyContext identifyContext;
  if ( canvas->mapSettings().isTemporal() )
    identifyContext.setTemporalRange( canvas->temporalRange() );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << layer, QgsMapToolIdentify::DefaultQgsSetting, identifyContext );
  return result;
}

// private
QList<QgsMapToolIdentify::IdentifyResult>
  TestQgsIdentify::testIdentifyVectorTile( QgsVectorTileLayer *layer, double xGeoref, double yGeoref )
{
  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( xGeoref, yGeoref );
  QgsIdentifyContext identifyContext;
  if ( canvas->mapSettings().isTemporal() )
    identifyContext.setTemporalRange( canvas->temporalRange() );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( mapPoint.x(), mapPoint.y(), QList<QgsMapLayer *>() << layer, QgsMapToolIdentify::DefaultQgsSetting, identifyContext );
  return result;
}

void TestQgsIdentify::identifyRasterTemporal()
{
  //create a temporary layer
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/test.asc";
  std::unique_ptr<QgsRasterLayer> tempLayer = std::make_unique<QgsRasterLayer>( raster );
  QVERIFY( tempLayer->isValid() );

  // activate temporal properties
  tempLayer->temporalProperties()->setIsActive( true );

  const QgsDateTimeRange range = QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime(), Qt::UTC ), QDateTime( QDate( 2020, 3, 31 ), QTime(), Qt::UTC ) );
  qobject_cast<QgsRasterLayerTemporalProperties *>( tempLayer->temporalProperties() )->setFixedTemporalRange( range );

  canvas->setExtent( QgsRectangle( 0, 0, 7, 1 ) );

  // invalid temporal range on canvas
  canvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 1950, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), QDateTime( QDate( 1950, 01, 01 ), QTime( 1, 0, 0 ), Qt::UTC ) ) );
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 0.5, 0.5 ).length(), 0 );

  // valid temporal range on canvas
  canvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 1950, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), QDateTime( QDate( 2050, 01, 01 ), QTime( 1, 0, 0 ), Qt::UTC ) ) );
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 0.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.9" ) );
}

void TestQgsIdentify::identifyRasterFloat32()
{
  //create a temporary layer
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/test.asc";

  // By default the QgsRasterLayer forces AAIGRID_DATATYPE=Float64
  CPLSetConfigOption( "AAIGRID_DATATYPE", "Float32" );
  std::unique_ptr<QgsRasterLayer> tempLayer( new QgsRasterLayer( raster ) );
  CPLSetConfigOption( "AAIGRID_DATATYPE", nullptr );

  QVERIFY( tempLayer->isValid() );

  canvas->setExtent( QgsRectangle( 0, 0, 7, 1 ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 0.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.9" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 1.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.987" ) );

  // More than 6 significant digits for corresponding value in .asc:
  // precision loss in Float32
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 2.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1.234568" ) ); // in .asc file : 1.2345678

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 3.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "123456" ) );

  // More than 6 significant digits: no precision loss here for that particular value
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 4.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1234567" ) );

  // More than 6 significant digits: no precision loss here for that particular value
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 5.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.9876" ) );

  // More than 6 significant digits for corresponding value in .asc:
  // precision loss in Float32
  QCOMPARE( testIdentifyRaster( tempLayer.get(), 6.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1.234568" ) ); // in .asc file : 1.2345678901234
}

void TestQgsIdentify::identifyRasterFloat64()
{
  //create a temporary layer
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/test.asc";
  std::unique_ptr<QgsRasterLayer> tempLayer( new QgsRasterLayer( raster ) );
  QVERIFY( tempLayer->isValid() );

  canvas->setExtent( QgsRectangle( 0, 0, 7, 1 ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 0.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.9" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 1.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.987" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 2.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1.2345678" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 3.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "123456" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 4.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1234567" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 5.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "-999.9876" ) );

  QCOMPARE( testIdentifyRaster( tempLayer.get(), 6.5, 0.5 ).at( 0 ).mAttributes[QStringLiteral( "Band 1" )], QString( "1.2345678901234" ) );
}

void TestQgsIdentify::identifyRasterDerivedAttributes()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/dem.tif";
  std::unique_ptr<QgsRasterLayer> tempLayer( new QgsRasterLayer( raster ) );
  QVERIFY( tempLayer->isValid() );

  const QgsRectangle layerExtent = tempLayer->extent();
  const double halfColumn = tempLayer->rasterUnitsPerPixelX() * 0.5;
  const double halfRow = tempLayer->rasterUnitsPerPixelY() * 0.5;

  canvas->resize( tempLayer->width(), tempLayer->height() ); // make canvas fit raster 1:1
  canvas->setDestinationCrs( tempLayer->crs() );
  canvas->setExtent( layerExtent );

  // checking the four corners of the raster plus one somewhere in the center
  QList<QgsMapToolIdentify::IdentifyResult> results;

  // right at corner of raster
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMinimum() + 0.0000000001, layerExtent.yMaximum() - 0.0000000001, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString( "0" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString( "0" ) );

  // offset by half a pixel
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMinimum() + halfColumn, layerExtent.yMaximum() - halfRow, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString( "0" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString( "0" ) );

  // right at corner of raster
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMaximum() - 0.0000000001, layerExtent.yMaximum() - 0.0000000001, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString::number( tempLayer->width() - 1 ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString( "0" ) );

  // offset by half a pixel
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMaximum() - halfColumn, layerExtent.yMaximum() - halfRow, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString::number( tempLayer->width() - 1 ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString( "0" ) );

  // right at corner of raster
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMinimum() + 0.0000000001, layerExtent.yMinimum() + 0.0000000001, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString( "0" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString::number( tempLayer->height() - 1 ) );

  // offset by half a pixel
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMinimum() + halfColumn, layerExtent.yMinimum() + halfRow, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString( "0" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString::number( tempLayer->height() - 1 ) );

  // right at corner of raster
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMaximum() - 0.0000000001, layerExtent.yMinimum() + 0.0000000001, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString::number( tempLayer->width() - 1 ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString::number( tempLayer->height() - 1 ) );

  // offset by half a pixel
  results = testIdentifyRaster( tempLayer.get(), layerExtent.xMaximum() - halfColumn, layerExtent.yMinimum() + halfRow, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString::number( tempLayer->width() - 1 ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString::number( tempLayer->height() - 1 ) );

  const double xSomewhereCenter = layerExtent.xMinimum() + halfColumn * 2 * 201;
  const double ySomewhereCenter = layerExtent.yMaximum() - halfRow * 2 * 141;
  results = testIdentifyRaster( tempLayer.get(), xSomewhereCenter, ySomewhereCenter, false );
  QCOMPARE( results.length(), 1 ); // just to ensure that we did get a result back
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Column (0-based)" )], QString( "201" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Row (0-based)" )], QString( "141" ) );
}

void TestQgsIdentify::identifyMesh()
{
  //create a temporary layer
  const QString mesh = QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle.2dm";
  QgsMeshLayer *tempLayer = new QgsMeshLayer( mesh, "testlayer", "mdal" );
  QVERIFY( tempLayer->isValid() );
  const QString vectorDs = QStringLiteral( TEST_DATA_DIR ) + "/mesh/quad_and_triangle_vertex_vector.dat";
  tempLayer->dataProvider()->addDataset( vectorDs );
  static_cast<QgsMeshLayerTemporalProperties *>(
    tempLayer->temporalProperties()
  )
    ->setReferenceTime(
      QDateTime( QDate( 1950, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), tempLayer->dataProvider()->temporalCapabilities()
    );

  // we need to setup renderer otherwise triangular mesh
  // will not be populated and identify will not work
  QgsMapSettings mapSettings;
  mapSettings.setExtent( tempLayer->extent() );
  mapSettings.setDestinationCrs( tempLayer->crs() );
  mapSettings.setOutputDpi( 96 );

  // here we check that datasets automatically get our default color ramp applied ("Plasma")
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  tempLayer->createMapRenderer( context );

  // only scalar dataset
  tempLayer->temporalProperties()->setIsActive( false );
  tempLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 0, 0 ) );
  QList<QgsMapToolIdentify::IdentifyResult> results;

  results = testIdentifyMesh( tempLayer, 500, 500 );
  QCOMPARE( results.size(), 2 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Scalar Value" )], QStringLiteral( "no data" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Source" )], mesh );
  QCOMPARE( results[1].mLabel, QStringLiteral( "Geometry" ) );
  results = testIdentifyMesh( tempLayer, 2400, 2400 );
  QCOMPARE( results.size(), 2 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Scalar Value" )], QStringLiteral( "42" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Source" )], mesh );
  QCOMPARE( results[1].mLabel, QStringLiteral( "Geometry" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Face Centroid X" )], QStringLiteral( "2333.33" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Face Centroid Y" )], QStringLiteral( "2333.33" ) );
  results = testIdentifyMesh( tempLayer, 1999, 2999 );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Snapped Vertex Position X" )], QStringLiteral( "2000" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Snapped Vertex Position Y" )], QStringLiteral( "3000" ) );

  canvas->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 1950, 01, 01 ), QTime( 0, 0, 0 ), Qt::UTC ), QDateTime( QDate( 1950, 01, 01 ), QTime( 1, 0, 0 ), Qt::UTC ) ) );

  tempLayer->temporalProperties()->setIsActive( true );
  results = testIdentifyMesh( tempLayer, 2400, 2400 );
  QCOMPARE( results.size(), 3 );
  QCOMPARE( results[0].mLabel, QStringLiteral( "Bed Elevation (active)" ) );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Scalar Value" )], QStringLiteral( "42" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Source" )], mesh );

  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Time Step" )], QStringLiteral( "1950-01-01 00:00:00" ) );

  QCOMPARE( results[1].mLabel, QStringLiteral( "VertexVectorDataset" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Source" )], vectorDs );
  QCOMPARE( results[1].mAttributes[QStringLiteral( "Vector Magnitude" )], QStringLiteral( "3" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Vector x-component" )], QStringLiteral( "1.8" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Vector y-component" )], QStringLiteral( "2.4" ) );

  QCOMPARE( results[2].mLabel, QStringLiteral( "Geometry" ) );
  QCOMPARE( results[2].mDerivedAttributes[QStringLiteral( "Face Centroid X" )], QStringLiteral( "2333.33" ) );
  QCOMPARE( results[2].mDerivedAttributes[QStringLiteral( "Face Centroid Y" )], QStringLiteral( "2333.33" ) );
  results = testIdentifyMesh( tempLayer, 1999, 2999 );
  QCOMPARE( results[2].mDerivedAttributes[QStringLiteral( "Snapped Vertex Position X" )], QStringLiteral( "2000" ) );
  QCOMPARE( results[2].mDerivedAttributes[QStringLiteral( "Snapped Vertex Position Y" )], QStringLiteral( "3000" ) );

  tempLayer->temporalProperties()->setIsActive( false );

  // scalar + vector same
  tempLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  tempLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  results = testIdentifyMesh( tempLayer, 500, 500 );
  QCOMPARE( results.size(), 2 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Vector Value" )], QStringLiteral( "no data" ) );
  results = testIdentifyMesh( tempLayer, 2400, 2400 );
  QCOMPARE( results.size(), 2 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Vector Magnitude" )], QStringLiteral( "3" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Vector x-component" )], QStringLiteral( "1.8" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Vector y-component" )], QStringLiteral( "2.4" ) );

  // scalar + vector different
  tempLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex( 0, 0 ) );
  tempLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  results = testIdentifyMesh( tempLayer, 2400, 2400 );
  QCOMPARE( results.size(), 3 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Scalar Value" )], QStringLiteral( "42" ) );
  QCOMPARE( results[1].mAttributes[QStringLiteral( "Vector Magnitude" )], QStringLiteral( "3" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Vector x-component" )], QStringLiteral( "1.8" ) );
  QCOMPARE( results[1].mDerivedAttributes[QStringLiteral( "Vector y-component" )], QStringLiteral( "2.4" ) );

  // only vector
  tempLayer->setStaticScalarDatasetIndex( QgsMeshDatasetIndex() );
  tempLayer->setStaticVectorDatasetIndex( QgsMeshDatasetIndex( 1, 0 ) );
  results = testIdentifyMesh( tempLayer, 2400, 2400 );
  QCOMPARE( results.size(), 2 );
  QCOMPARE( results[0].mAttributes[QStringLiteral( "Vector Magnitude" )], QStringLiteral( "3" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Vector x-component" )], QStringLiteral( "1.8" ) );
  QCOMPARE( results[0].mDerivedAttributes[QStringLiteral( "Vector y-component" )], QStringLiteral( "2.4" ) );
}

void TestQgsIdentify::identifyVectorTile()
{
  //create a temporary layer
  const QString vtPath = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/vector_tile/{z}-{x}-{y}.pbf" );
  QgsDataSourceUri dsUri;
  dsUri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
  dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( vtPath ).toString() );
  QgsVectorTileLayer *tempLayer = new QgsVectorTileLayer( dsUri.encodedUri(), QStringLiteral( "testlayer" ) );
  QVERIFY( tempLayer->isValid() );

  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3857" ) );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( tempLayer->extent() );
  canvas->resize( 512, 512 );
  canvas->setLayers( QList<QgsMapLayer *>() << tempLayer );
  canvas->setCurrentLayer( tempLayer );

  QList<QgsMapToolIdentify::IdentifyResult> results;
  results = testIdentifyVectorTile( tempLayer, 15186127, -2974969 );
  QCOMPARE( results.size(), 1 );
  QCOMPARE( results[0].mLayer, tempLayer );
  QCOMPARE( results[0].mLabel, QStringLiteral( "place" ) );
  QCOMPARE( results[0].mFeature.geometry().wkbType(), Qgis::WkbType::Point );
  QCOMPARE( results[0].mFeature.attribute( QStringLiteral( "class" ) ).toString(), QStringLiteral( "country" ) );
  QCOMPARE( results[0].mFeature.attribute( QStringLiteral( "name" ) ).toString(), QStringLiteral( "Australia" ) );

  delete tempLayer;
}

void TestQgsIdentify::identifyInvalidPolygons()
{
  //create a temporary layer
  std::unique_ptr<QgsVectorLayer> memoryLayer( new QgsVectorLayer( QStringLiteral( "Polygon?field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature f1( memoryLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
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
  identified = testIdentifyVector( memoryLayer.get(), 4, 6 );
  QCOMPARE( identified.length(), 0 );
  identified = testIdentifyVector( memoryLayer.get(), 6, 4 );
  QCOMPARE( identified.length(), 1 );
  QCOMPARE( identified[0].mFeature.attribute( "pk" ), QVariant( 1 ) );
}

void TestQgsIdentify::testRelations()
{
  QgsVectorLayer *layerA = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk_id:integer" ), QStringLiteral( "layerA" ), QStringLiteral( "memory" ) );
  QVERIFY( layerA->isValid() );
  QgsFeature featureA( layerA->dataProvider()->fields() );
  constexpr int PK_ID_A = 1;
  constexpr int PK_ID_C = 2;
  featureA.setAttribute( 0, PK_ID_A );
  layerA->dataProvider()->addFeature( featureA );

  QgsVectorLayer *layerB = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=fk_id_to_A:integer&field=fk_id_to_C:integer&field=other_field:integer" ), QStringLiteral( "layerB" ), QStringLiteral( "memory" ) );
  QVERIFY( layerB->isValid() );
  constexpr int IDX_OTHER_FIELD = 2;
  constexpr int OTHER_FIELD = 100;
  {
    QgsFeature featureB( layerB->dataProvider()->fields() );
    featureB.setAttribute( 0, PK_ID_A );
    featureB.setAttribute( 1, PK_ID_C );
    featureB.setAttribute( IDX_OTHER_FIELD, OTHER_FIELD );
    layerB->dataProvider()->addFeature( featureB );
  }
  {
    QgsFeature featureB( layerB->dataProvider()->fields() );
    featureB.setAttribute( 0, PK_ID_A );
    featureB.setAttribute( 1, PK_ID_C + 1 );
    featureB.setAttribute( IDX_OTHER_FIELD, OTHER_FIELD + 1 );
    layerB->dataProvider()->addFeature( featureB );
  }

  QgsVectorLayer *layerC = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=pk_id:integer" ), QStringLiteral( "layerC" ), QStringLiteral( "memory" ) );
  QVERIFY( layerC->isValid() );
  {
    QgsFeature featureC( layerC->dataProvider()->fields() );
    featureC.setAttribute( 0, PK_ID_C );
    layerC->dataProvider()->addFeature( featureC );
  }
  {
    QgsFeature featureC( layerC->dataProvider()->fields() );
    featureC.setAttribute( 0, PK_ID_C + 1 );
    layerC->dataProvider()->addFeature( featureC );
  }

  QgsProject::instance()->layerStore()->addMapLayer( layerA, true );
  QgsProject::instance()->layerStore()->addMapLayer( layerB, true );
  QgsProject::instance()->layerStore()->addMapLayer( layerC, true );

  QgsRelationManager *relationManager = QgsProject::instance()->relationManager();
  {
    QgsRelation relation;
    relation.setId( "B-A-id" );
    relation.setName( "B-A" );
    relation.setReferencingLayer( layerB->id() );
    relation.setReferencedLayer( layerA->id() );
    relation.addFieldPair( QStringLiteral( "fk_id_to_A" ), QStringLiteral( "pk_id" ) );

    relationManager->addRelation( relation );
  }
  {
    QgsRelation relation;
    relation.setId( "A-B-id" );
    relation.setName( "A-B" );
    relation.setReferencingLayer( layerA->id() );
    relation.setReferencedLayer( layerB->id() );
    relation.addFieldPair( QStringLiteral( "pk_id" ), QStringLiteral( "fk_id_to_A" ) );

    relationManager->addRelation( relation );
  }
  {
    QgsRelation relation;
    relation.setId( "B-C-id" );
    relation.setName( "B-C" );
    relation.setReferencingLayer( layerB->id() );
    relation.setReferencedLayer( layerC->id() );
    relation.addFieldPair( QStringLiteral( "fk_id_to_C" ), QStringLiteral( "pk_id" ) );

    relationManager->addRelation( relation );
  }

  std::unique_ptr<QgsIdentifyResultsDialog> dialog = std::make_unique<QgsIdentifyResultsDialog>( canvas );
  dialog->addFeature( layerA, featureA, QMap<QString, QString>() );

  QCOMPARE( dialog->lstResults->topLevelItemCount(), 1 );
  QTreeWidgetItem *topLevelItem = dialog->lstResults->topLevelItem( 0 );
  QCOMPARE( topLevelItem->childCount(), 1 );
  QgsIdentifyResultsFeatureItem *featureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( topLevelItem->child( 0 ) );
  QVERIFY( featureItem );
  std::vector<QgsIdentifyResultsRelationItem *> relationItems;
  for ( int i = 0; i < featureItem->childCount(); ++i )
  {
    QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( featureItem->child( i ) );
    if ( relationItem )
      relationItems.push_back( relationItem );
  }
  QCOMPARE( relationItems.size(), 2 );

  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A […]" ) );
  QCOMPARE( relationItems[0]->childCount(), 0 );
  QCOMPARE( relationItems[0]->childIndicatorPolicy(), QTreeWidgetItem::ShowIndicator );
  QCOMPARE( relationItems[0]->isExpanded(), false );

  QCOMPARE( relationItems[1]->text( 0 ), QStringLiteral( "layerB through A-B [1]" ) );
  QCOMPARE( relationItems[1]->childCount(), 0 );
  QCOMPARE( relationItems[1]->childIndicatorPolicy(), QTreeWidgetItem::ShowIndicator );
  QCOMPARE( relationItems[1]->isExpanded(), false );

  // Check referenced relation

  // Check that expandAll() doesn't result in automatic resolution of relations
  dialog->expandAll();
  QCOMPARE( relationItems[0]->childCount(), 0 );

  relationItems[0]->setExpanded( true );
  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A [2]" ) );
  QCOMPARE( relationItems[0]->childCount(), 2 );

  // Check that folding/unfolding after initial expansion works
  relationItems[0]->setExpanded( false );
  relationItems[0]->setExpanded( true );
  QCOMPARE( relationItems[0]->text( 0 ), QStringLiteral( "layerB through B-A [2]" ) );
  QCOMPARE( relationItems[0]->childCount(), 2 );

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[0]->child( 0 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value<QgsFeature>();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD );

    {
      std::vector<QgsIdentifyResultsRelationItem *> childRelationItems;
      for ( int i = 0; i < relatedFeatureItem->childCount(); ++i )
      {
        QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( relatedFeatureItem->child( i ) );
        if ( relationItem )
          childRelationItems.push_back( relationItem );
      }
      QCOMPARE( childRelationItems.size(), 1 );

      QCOMPARE( childRelationItems[0]->childCount(), 0 );
      QCOMPARE( childRelationItems[0]->text( 0 ), QStringLiteral( "layerC through B-C [1]" ) );

      childRelationItems[0]->setExpanded( true );
      QCOMPARE( childRelationItems[0]->childCount(), 1 );
      QCOMPARE( childRelationItems[0]->text( 0 ), QStringLiteral( "layerC through B-C [1]" ) );

      {
        QgsIdentifyResultsFeatureItem *childRelatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( childRelationItems[0]->child( 0 ) );
        QVERIFY( childRelatedFeatureItem );
        QVERIFY( childRelatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
        const QgsFeature relatedFeature = childRelatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value<QgsFeature>();
        QCOMPARE( relatedFeature.attribute( 0 ), PK_ID_C );

        // Check that this child doesn't link back to parent feature A
        std::vector<QgsIdentifyResultsRelationItem *> childChildRelationItems;
        for ( int i = 0; i < childRelatedFeatureItem->childCount(); ++i )
        {
          QgsIdentifyResultsRelationItem *relationItem = dynamic_cast<QgsIdentifyResultsRelationItem *>( childRelatedFeatureItem->child( i ) );
          if ( relationItem )
            childChildRelationItems.push_back( relationItem );
        }
        QCOMPARE( childChildRelationItems.size(), 0 );
      }
    }
  }

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[0]->child( 1 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value<QgsFeature>();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD + 1 );
  }

  // Check referencing relation
  relationItems[1]->setExpanded( true );
  QCOMPARE( relationItems[1]->text( 0 ), QStringLiteral( "layerB through A-B [1]" ) );
  QCOMPARE( relationItems[1]->childCount(), 1 );

  {
    QgsIdentifyResultsFeatureItem *relatedFeatureItem = dynamic_cast<QgsIdentifyResultsFeatureItem *>( relationItems[1]->child( 0 ) );
    QVERIFY( relatedFeatureItem );
    QVERIFY( relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).isValid() );
    const QgsFeature relatedFeature = relatedFeatureItem->data( 0, QgsIdentifyResultsDialog::FeatureRole ).value<QgsFeature>();
    QCOMPARE( relatedFeature.attribute( IDX_OTHER_FIELD ), OTHER_FIELD );
  }
}

void TestQgsIdentify::testPointZ()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "PointZ?crs=epsg:4979" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QCOMPARE( tempLayer->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4979" ) );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PointZ(134.445567853 -23.445567853 5543.325)" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:4985" ) );
  QgsProject::instance()->setCrs( srs );
  canvas->setDestinationCrs( srs );
  QCOMPARE( QgsProject::instance()->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4985" ) );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 134.445567853, -23.445567853 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  double z4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Z (EPSG:4979 - WGS 84)" )].toDouble();
  double z4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Z (EPSG:4985 - WGS 72)" )].toDouble();
  QGSCOMPARENEAR( z4979, 5543.325, 0.001 );
  QGSCOMPARENEAR( z4985, 5545.6857, 0.01 );
}

void TestQgsIdentify::testLineStringZ()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=epsg:4979" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QCOMPARE( tempLayer->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4979" ) );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineStringZ(134.445567853 -23.445567853 5543.325, 140.485567853 -23.445567853 5563.325)" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:4985" ) );
  QgsProject::instance()->setCrs( srs );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( tempLayer->extent() );
  QCOMPARE( QgsProject::instance()->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4985" ) );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 136.46, -23.445567853 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  double interpolatedZ4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Interpolated Z (EPSG:4979 - WGS 84)" )].toDouble();
  double interpolatedZ4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Interpolated Z (EPSG:4985 - WGS 72)" )].toDouble();
  double closestZ4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Closest vertex Z (EPSG:4979 - WGS 84)" )].toDouble();
  double closestZ4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Closest vertex Z (EPSG:4985 - WGS 72)" )].toDouble();
  QGSCOMPARENEAR( interpolatedZ4979, 5548.472636, 0.001 );
  QGSCOMPARENEAR( interpolatedZ4985, 5550.8333350, 0.01 );
  QGSCOMPARENEAR( closestZ4979, 5543.325, 0.001 );
  QGSCOMPARENEAR( closestZ4985, 5545.6857, 0.01 );
}

void TestQgsIdentify::testPolygonZ()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "PolygonZ?crs=epsg:4979" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QCOMPARE( tempLayer->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4979" ) );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PolygonZ((134.445567853 -23.445567853 5543.325, 140.485567853 -23.445567853 5563.325, 140.485567853 -20.445567853 5523.325, 134.445567853 -23.445567853 5543.325))" ) ) );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:4985" ) );
  QgsProject::instance()->setCrs( srs );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( tempLayer->extent() );
  QCOMPARE( QgsProject::instance()->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4985" ) );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 3 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 136.46, -23.445567853 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << tempLayer.get() );
  QCOMPARE( result.length(), 1 );
  double interpolatedZ4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Interpolated Z (EPSG:4979 - WGS 84)" )].toDouble();
  double interpolatedZ4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Interpolated Z (EPSG:4985 - WGS 72)" )].toDouble();
  double closestZ4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Closest vertex Z (EPSG:4979 - WGS 84)" )].toDouble();
  double closestZ4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Closest vertex Z (EPSG:4985 - WGS 72)" )].toDouble();
  QGSCOMPARENEAR( interpolatedZ4979, 5549.9817600000, 0.02 );
  QGSCOMPARENEAR( interpolatedZ4985, 5552.3424580000, 0.02 );
  QGSCOMPARENEAR( closestZ4979, 5543.325, 0.001 );
  QGSCOMPARENEAR( closestZ4985, 5545.6857, 0.01 );
}

void TestQgsIdentify::identifyPointCloud()
{
#ifdef HAVE_EPT
  std::unique_ptr<QgsPointCloudLayer> pointCloud = std::make_unique<QgsPointCloudLayer>( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/ept/rgb16/ept.json", QStringLiteral( "pointcloud" ), QStringLiteral( "ept" ) );
  QVERIFY( pointCloud->isValid() );
  pointCloud->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4979" ) ) );
  QCOMPARE( pointCloud->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4979" ) );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:4985" ) );
  QgsProject::instance()->setCrs( srs );
  canvas->setDestinationCrs( srs );
  canvas->setExtent( QgsRectangle::fromCenterAndSize( QgsPointXY( 7.42006, 2.74911 ), 0.1, 0.1 ) );
  QCOMPARE( QgsProject::instance()->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:4985" ) );

  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ), false );
  QgsProject::instance()->writeEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ), 4 );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 7.42006, 2.74911 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << pointCloud.get() );
  QCOMPARE( result.length(), 1 );
  double z4979 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Z (EPSG:4979 - WGS 84)" )].toDouble();
  double z4985 = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Z (EPSG:4985 - WGS 72)" )].toDouble();
  QGSCOMPARENEAR( z4979, -5.79000, 0.001 );
  QGSCOMPARENEAR( z4985, -5.40314874, 0.001 );
#endif
}

void TestQgsIdentify::identifyVirtualPointCloud()
{
#ifdef HAVE_COPC
  std::unique_ptr<QgsPointCloudLayer> pointCloud = std::make_unique<QgsPointCloudLayer>( QStringLiteral( TEST_DATA_DIR ) + "/point_clouds/virtual/sunshine-coast/combined-with-overview.vpc", QStringLiteral( "pointcloud" ), QStringLiteral( "vpc" ) );
  QVERIFY( pointCloud->isValid() );
  pointCloud->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:28356" ) ) );
  QCOMPARE( pointCloud->crs3D().horizontalCrs().authid(), QStringLiteral( "EPSG:28356" ) );

  for ( int i = 0; i < pointCloud->dataProvider()->subIndexes().size(); i++ )
    pointCloud->dataProvider()->loadSubIndex( i );

  // set project CRS and ellipsoid
  // Note that using a different CRS here (a world-wide WGS84-based one) caused
  // problems on some machines due to insufficient precision in reprojection.
  QgsProject::instance()->setCrs( pointCloud->crs() );
  canvas->setDestinationCrs( pointCloud->crs() );
  canvas->setExtent( QgsRectangle::fromCenterAndSize( QgsPointXY( 498065.5, 7050992.5 ), 1, 1 ) );

  const QgsPointXY mapPoint = canvas->getCoordinateTransform()->transform( 498065.23, 7050992.90 );

  std::unique_ptr<QgsMapToolIdentifyAction> action( new QgsMapToolIdentifyAction( canvas ) );
  QList<QgsMapToolIdentify::IdentifyResult> result = action->identify( static_cast<int>( mapPoint.x() ), static_cast<int>( mapPoint.y() ), QList<QgsMapLayer *>() << pointCloud.get() );
  QCOMPARE( result.length(), 1 );
  double z = result.at( 0 ).mDerivedAttributes[QStringLiteral( "Z" )].toDouble();
  QGSCOMPARENEAR( z, 74.91, 0.001 );
#endif
}

QGSTEST_MAIN( TestQgsIdentify )
#include "testqgsidentify.moc"
