/***************************************************************************
     testqgsmaptooldistributefeature.cpp
     --------------------------------
    Date                 : November 2025
    Copyright            : (C) 2025 by Jacky Volpes
    Email                : jacky dot volpes at oslandia dot com
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
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgsmaptooldistributefeature.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "qgsvectorlayerutils.h"
#include "testqgsmaptoolutils.h"
#include <algorithm>


/**
 * \ingroup UnitTests
 * This is a unit test for the distribute feature tool
 */
class TestQgsMapToolDistributeFeature : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolDistributeFeature();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testDistributeFeatureFeatureCount();
    void testDistributeFeatureFeatureSpacing();
    void testDistributeFeatureFeatureCountAndSpacing();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolDistributeFeature *mDistributeFeatureTool = nullptr;
    QgsVectorLayer *mLayerPoint = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerPolygon = nullptr;
    QgsVectorLayer *mLayerPolygon2154 = nullptr;
};

TestQgsMapToolDistributeFeature::TestQgsMapToolDistributeFeature() = default;


//runs before all tests
void TestQgsMapToolDistributeFeature::initTestCase()
{
  qDebug() << "TestMapToolDistributeFeature::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 28, 28 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerPoint = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:3946&field=my_text:string" ), QStringLiteral( "pointlayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPoint->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPoint );
  mLayerLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:3946&field=my_text:string" ), QStringLiteral( "linelayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLine );
  mLayerPolygon = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:3946&field=my_text:string" ), QStringLiteral( "polygonlayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPolygon->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPolygon );
  mLayerPolygon2154 = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:2154&field=my_text:string" ), QStringLiteral( "polygonlayer2154" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPolygon2154->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPolygon2154 );

  // Point layer
  mLayerPoint->startEditing();
  const QString wkt1 = QStringLiteral( "Point (14 10)" );
  QgsAttributeMap attributes1 = QgsAttributeMap();
  attributes1[0] = QStringLiteral( "why?" );
  QgsFeature f1 = QgsVectorLayerUtils::createFeature( mLayerPoint, QgsGeometry::fromWkt( wkt1 ), attributes1 );
  QVERIFY( mLayerPoint->dataProvider()->addFeature( f1 ) );
  QCOMPARE( mLayerPoint->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerPoint->getFeature( 1 ).geometry().asWkt(), wkt1 );

  // Line layer
  mLayerLine->startEditing();
  const QString wkt2 = QStringLiteral( "LineString (0 3, 3 6)" );
  QgsAttributeMap attributes2 = QgsAttributeMap();
  attributes2[0] = QStringLiteral( "maybe?" );
  QgsFeature f2 = QgsVectorLayerUtils::createFeature( mLayerLine, QgsGeometry::fromWkt( wkt2 ), attributes2 );
  QVERIFY( mLayerLine->dataProvider()->addFeature( f2 ) );
  QCOMPARE( mLayerLine->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerLine->getFeature( 1 ).geometry().asWkt(), wkt2 );

  // Polygon layer
  mLayerPolygon->startEditing();
  const QString wkt3 = QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
  QgsAttributeMap attributes3 = QgsAttributeMap();
  attributes3[0] = QStringLiteral( "because." );
  QgsFeature f3 = QgsVectorLayerUtils::createFeature( mLayerPolygon, QgsGeometry::fromWkt( wkt3 ), attributes3 );
  QVERIFY( mLayerPolygon->dataProvider()->addFeature( f3 ) );
  QCOMPARE( mLayerPolygon->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerPolygon->getFeature( 1 ).geometry().asWkt(), wkt3 );

  // Polygon 2154 layer
  mLayerPolygon2154->startEditing();
  QString wkt4 = QStringLiteral( "Polygon ((0 5, 0 6, 1 6, 1 5, 0 5))" );
  QgsAttributeMap attributes4 = QgsAttributeMap();
  attributes4[0] = QStringLiteral( "sure?" );
  QgsGeometry geom = QgsGeometry::fromWkt( wkt4 );
  QCOMPARE( geom.transform( QgsCoordinateTransform( QgsCoordinateReferenceSystem( "EPSG:3946" ), QgsCoordinateReferenceSystem( "EPSG:2154" ), QgsCoordinateTransformContext() ) ), Qgis::GeometryOperationResult::Success );
  wkt4 = geom.asWkt();
  QgsFeature f4 = QgsVectorLayerUtils::createFeature( mLayerPolygon2154, geom, attributes4 );
  QVERIFY( mLayerPolygon2154->dataProvider()->addFeature( f4 ) );
  QCOMPARE( mLayerPolygon2154->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerPolygon2154->getFeature( 1 ).geometry().asWkt(), wkt4 );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerPoint << mLayerLine << mLayerPolygon << mLayerPolygon2154 );

  // create the tool
  mDistributeFeatureTool = new QgsMapToolDistributeFeature( mCanvas );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 28, 28 ) );
}

//runs after all tests
void TestQgsMapToolDistributeFeature::cleanupTestCase()
{
  delete mDistributeFeatureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolDistributeFeature::testDistributeFeatureFeatureCount()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mDistributeFeatureTool );

  mDistributeFeatureTool->setMode( QgsMapToolDistributeFeature::DistributeMode::FeatureCount );
  mDistributeFeatureTool->setFeatureCount( 4 );         // will add 4 new features
  mDistributeFeatureTool->setFeatureSpacing( 10.6543 ); // will not be taken into account because the mode is FeatureCount
  QCOMPARE( mDistributeFeatureTool->featureCount(), 4 );
  QCOMPARE( mDistributeFeatureTool->featureSpacing(), 0 );

  // Point layer
  mCanvas->setCurrentLayer( mLayerPoint );
  utils.mouseClick( 14, 10, Qt::LeftButton );
  utils.mouseClick( 18, 18, Qt::LeftButton );
  QCOMPARE( mLayerPoint->featureCount(), 5 );
  QCOMPARE( mLayerPoint->getFeature( -9 ).geometry().asWkt( 1 ), "Point (15 12)" );
  QCOMPARE( mLayerPoint->getFeature( -10 ).geometry().asWkt( 1 ), "Point (16 14)" );
  QCOMPARE( mLayerPoint->getFeature( -11 ).geometry().asWkt( 1 ), "Point (17 16)" );
  QCOMPARE( mLayerPoint->getFeature( -12 ).geometry().asWkt( 1 ), "Point (18 18)" );
  QgsFeatureIds fids = mLayerPoint->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPoint->getFeature( fid ).attribute( 0 ), "why?" ); } );
  mLayerPoint->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Line layer
  mCanvas->setCurrentLayer( mLayerLine );
  utils.mouseClick( 1, 4, Qt::LeftButton );
  utils.mouseClick( 1, 12, Qt::LeftButton );
  QCOMPARE( mLayerLine->featureCount(), 5 );
  QCOMPARE( mLayerLine->getFeature( -13 ).geometry().asWkt( 1 ), "LineString (0 5, 3 8)" );
  QCOMPARE( mLayerLine->getFeature( -14 ).geometry().asWkt( 1 ), "LineString (0 7, 3 10)" );
  QCOMPARE( mLayerLine->getFeature( -15 ).geometry().asWkt( 1 ), "LineString (0 9, 3 12)" );
  QCOMPARE( mLayerLine->getFeature( -16 ).geometry().asWkt( 1 ), "LineString (0 11, 3 14)" );
  fids = mLayerLine->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerLine->getFeature( fid ).attribute( 0 ), "maybe?" ); } );
  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon layer
  mCanvas->setCurrentLayer( mLayerPolygon );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 20, 1, Qt::LeftButton );
  QCOMPARE( mLayerPolygon->featureCount(), 5 );
  QCOMPARE( mLayerPolygon->getFeature( -17 ).geometry().asWkt( 1 ), "Polygon ((5 0, 5 1, 6 1, 6 0, 5 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -18 ).geometry().asWkt( 1 ), "Polygon ((10 0, 10 1, 11 1, 11 0, 10 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -19 ).geometry().asWkt( 1 ), "Polygon ((15 0, 15 1, 16 1, 16 0, 15 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -20 ).geometry().asWkt( 1 ), "Polygon ((20 0, 20 1, 21 1, 21 0, 20 0))" );
  fids = mLayerPolygon->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon->getFeature( fid ).attribute( 0 ), "because." ); } );
  mLayerPolygon->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon 2154 layer
  mCanvas->setCurrentLayer( mLayerPolygon2154 );
  QgsCoordinateTransform tr2154to3946 = QgsCoordinateTransform(
    QgsCoordinateReferenceSystem( "EPSG:2154" ), QgsCoordinateReferenceSystem( "EPSG:3946" ), QgsCoordinateTransformContext()
  );
  utils.mouseClick( 0, 5, Qt::LeftButton );
  utils.mouseClick( 12, 9, Qt::LeftButton );
  QCOMPARE( mLayerPolygon2154->featureCount(), 5 );
  QgsGeometry geom = mLayerPolygon2154->getFeature( -21 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((3 6, 3 7, 4 7, 4 6, 3 6))" );
  geom = mLayerPolygon2154->getFeature( -22 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((6 7, 6 8, 7 8, 7 7, 6 7))" );
  geom = mLayerPolygon2154->getFeature( -23 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((9 8, 9 9, 10 9, 10 8, 9 8))" );
  geom = mLayerPolygon2154->getFeature( -24 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((12 9, 12 10, 13 10, 13 9, 12 9))" );
  fids = mLayerPolygon2154->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon2154->getFeature( fid ).attribute( 0 ), "sure?" ); } );
  mLayerPolygon2154->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Nothing happened
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 20, 1, Qt::RightButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key::Key_Escape );
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );
}

void TestQgsMapToolDistributeFeature::testDistributeFeatureFeatureSpacing()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mDistributeFeatureTool );

  mDistributeFeatureTool->setMode( QgsMapToolDistributeFeature::DistributeMode::FeatureSpacing );
  mDistributeFeatureTool->setFeatureCount( 7 );     // will not be taken into account because the mode is FeatureSpacing
  mDistributeFeatureTool->setFeatureSpacing( 1.6 ); // will add a feature every 1.6 map unit
  QCOMPARE( mDistributeFeatureTool->featureCount(), 0 );
  QCOMPARE( mDistributeFeatureTool->featureSpacing(), 1.6 );

  // Point layer
  mCanvas->setCurrentLayer( mLayerPoint );
  utils.mouseClick( 14, 10, Qt::LeftButton );
  utils.mouseClick( 20, 10, Qt::LeftButton );
  QCOMPARE( mLayerPoint->featureCount(), 4 );
  QCOMPARE( mLayerPoint->getFeature( -25 ).geometry().asWkt( 1 ), "Point (15.6 10)" );
  QCOMPARE( mLayerPoint->getFeature( -26 ).geometry().asWkt( 1 ), "Point (17.2 10)" );
  QCOMPARE( mLayerPoint->getFeature( -27 ).geometry().asWkt( 1 ), "Point (18.8 10)" );
  QgsFeatureIds fids = mLayerPoint->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPoint->getFeature( fid ).attribute( 0 ), "why?" ); } );
  mLayerPoint->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Line layer
  mCanvas->setCurrentLayer( mLayerLine );
  utils.mouseClick( 1, 4, Qt::LeftButton );
  utils.mouseClick( 1, 12, Qt::LeftButton );
  QCOMPARE( mLayerLine->featureCount(), 5 );
  QCOMPARE( mLayerLine->getFeature( -28 ).geometry().asWkt( 1 ), "LineString (0 4.6, 3 7.6)" );
  QCOMPARE( mLayerLine->getFeature( -29 ).geometry().asWkt( 1 ), "LineString (0 6.2, 3 9.2)" );
  QCOMPARE( mLayerLine->getFeature( -30 ).geometry().asWkt( 1 ), "LineString (0 7.8, 3 10.8)" );
  QCOMPARE( mLayerLine->getFeature( -31 ).geometry().asWkt( 1 ), "LineString (0 9.4, 3 12.4)" );
  fids = mLayerLine->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerLine->getFeature( fid ).attribute( 0 ), "maybe?" ); } );
  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon layer
  mCanvas->setCurrentLayer( mLayerPolygon );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 6.5, 1, Qt::LeftButton );
  QCOMPARE( mLayerPolygon->featureCount(), 5 );
  QCOMPARE( mLayerPolygon->getFeature( -32 ).geometry().asWkt( 1 ), "Polygon ((1.6 0, 1.6 1, 2.6 1, 2.6 0, 1.6 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -33 ).geometry().asWkt( 1 ), "Polygon ((3.2 0, 3.2 1, 4.2 1, 4.2 0, 3.2 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -34 ).geometry().asWkt( 1 ), "Polygon ((4.8 0, 4.8 1, 5.8 1, 5.8 0, 4.8 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -35 ).geometry().asWkt( 1 ), "Polygon ((6.4 0, 6.4 1, 7.4 1, 7.4 0, 6.4 0))" );
  fids = mLayerPolygon->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon->getFeature( fid ).attribute( 0 ), "because." ); } );
  mLayerPolygon->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon 2154 layer
  mCanvas->setCurrentLayer( mLayerPolygon2154 );
  QgsCoordinateTransform tr2154to3946 = QgsCoordinateTransform(
    QgsCoordinateReferenceSystem( "EPSG:2154" ), QgsCoordinateReferenceSystem( "EPSG:3946" ), QgsCoordinateTransformContext()
  );
  utils.mouseClick( 0, 5, Qt::LeftButton );
  utils.mouseClick( 0, 10.7, Qt::LeftButton );
  QCOMPARE( mLayerPolygon2154->featureCount(), 4 );
  QgsGeometry geom = mLayerPolygon2154->getFeature( -36 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 6.6, 0 7.6, 1 7.6, 1 6.6, 0 6.6))" );
  geom = mLayerPolygon2154->getFeature( -37 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 8.2, 0 9.2, 1 9.2, 1 8.2, 0 8.2))" );
  geom = mLayerPolygon2154->getFeature( -38 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 9.8, 0 10.8, 1 10.8, 1 9.8, 0 9.8))" );
  fids = mLayerPolygon2154->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon2154->getFeature( fid ).attribute( 0 ), "sure?" ); } );
  mLayerPolygon2154->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Nothing happened
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 20, 1, Qt::RightButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key::Key_Escape );
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );
}

void TestQgsMapToolDistributeFeature::testDistributeFeatureFeatureCountAndSpacing()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mDistributeFeatureTool );

  mDistributeFeatureTool->setMode( QgsMapToolDistributeFeature::DistributeMode::FeatureCountAndSpacing );
  mDistributeFeatureTool->setFeatureCount( 6 );     // will add 6 new features
  mDistributeFeatureTool->setFeatureSpacing( 2.4 ); // every 2.4 map unit
  QCOMPARE( mDistributeFeatureTool->featureCount(), 6 );
  QCOMPARE( mDistributeFeatureTool->featureSpacing(), 2.4 );

  // Point layer
  mCanvas->setCurrentLayer( mLayerPoint );
  utils.mouseClick( 14, 10, Qt::LeftButton );
  utils.mouseClick( 14, 11, Qt::LeftButton );
  QCOMPARE( mLayerPoint->featureCount(), 7 );
  QCOMPARE( mLayerPoint->getFeature( -39 ).geometry().asWkt( 1 ), "Point (14 12.4)" );
  QCOMPARE( mLayerPoint->getFeature( -40 ).geometry().asWkt( 1 ), "Point (14 14.8)" );
  QCOMPARE( mLayerPoint->getFeature( -41 ).geometry().asWkt( 1 ), "Point (14 17.2)" );
  QCOMPARE( mLayerPoint->getFeature( -42 ).geometry().asWkt( 1 ), "Point (14 19.6)" );
  QCOMPARE( mLayerPoint->getFeature( -43 ).geometry().asWkt( 1 ), "Point (14 22)" );
  QCOMPARE( mLayerPoint->getFeature( -44 ).geometry().asWkt( 1 ), "Point (14 24.4)" );
  QgsFeatureIds fids = mLayerPoint->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPoint->getFeature( fid ).attribute( 0 ), "why?" ); } );
  mLayerPoint->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Line layer
  mCanvas->setCurrentLayer( mLayerLine );
  utils.mouseClick( 1, 4, Qt::LeftButton );
  utils.mouseClick( 2.123, 4, Qt::LeftButton );
  QCOMPARE( mLayerLine->featureCount(), 7 );
  QCOMPARE( mLayerLine->getFeature( -45 ).geometry().asWkt( 1 ), "LineString (2.4 3, 5.4 6)" );
  QCOMPARE( mLayerLine->getFeature( -46 ).geometry().asWkt( 1 ), "LineString (4.8 3, 7.8 6)" );
  QCOMPARE( mLayerLine->getFeature( -47 ).geometry().asWkt( 1 ), "LineString (7.2 3, 10.2 6)" );
  QCOMPARE( mLayerLine->getFeature( -48 ).geometry().asWkt( 1 ), "LineString (9.6 3, 12.6 6)" );
  QCOMPARE( mLayerLine->getFeature( -49 ).geometry().asWkt( 1 ), "LineString (12 3, 15 6)" );
  QCOMPARE( mLayerLine->getFeature( -50 ).geometry().asWkt( 1 ), "LineString (14.4 3, 17.4 6)" );
  fids = mLayerLine->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerLine->getFeature( fid ).attribute( 0 ), "maybe?" ); } );
  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon layer
  mCanvas->setCurrentLayer( mLayerPolygon );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  QCOMPARE( mLayerPolygon->featureCount(), 7 );
  QCOMPARE( mLayerPolygon->getFeature( -51 ).geometry().asWkt( 1 ), "Polygon ((2.4 0, 2.4 1, 3.4 1, 3.4 0, 2.4 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -52 ).geometry().asWkt( 1 ), "Polygon ((4.8 0, 4.8 1, 5.8 1, 5.8 0, 4.8 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -53 ).geometry().asWkt( 1 ), "Polygon ((7.2 0, 7.2 1, 8.2 1, 8.2 0, 7.2 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -54 ).geometry().asWkt( 1 ), "Polygon ((9.6 0, 9.6 1, 10.6 1, 10.6 0, 9.6 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -55 ).geometry().asWkt( 1 ), "Polygon ((12 0, 12 1, 13 1, 13 0, 12 0))" );
  QCOMPARE( mLayerPolygon->getFeature( -56 ).geometry().asWkt( 1 ), "Polygon ((14.4 0, 14.4 1, 15.4 1, 15.4 0, 14.4 0))" );
  fids = mLayerPolygon->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon->getFeature( fid ).attribute( 0 ), "because." ); } );
  mLayerPolygon->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Polygon 2154 layer
  mCanvas->setCurrentLayer( mLayerPolygon2154 );
  QgsCoordinateTransform tr2154to3946 = QgsCoordinateTransform(
    QgsCoordinateReferenceSystem( "EPSG:2154" ), QgsCoordinateReferenceSystem( "EPSG:3946" ), QgsCoordinateTransformContext()
  );
  utils.mouseClick( 0, 5, Qt::LeftButton );
  utils.mouseClick( 0, 8.3, Qt::LeftButton );
  QCOMPARE( mLayerPolygon2154->featureCount(), 7 );
  QgsGeometry geom = mLayerPolygon2154->getFeature( -57 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 7.4, 0 8.4, 1 8.4, 1 7.4, 0 7.4))" );
  geom = mLayerPolygon2154->getFeature( -58 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 9.8, 0 10.8, 1 10.8, 1 9.8, 0 9.8))" );
  geom = mLayerPolygon2154->getFeature( -59 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 12.2, 0 13.2, 1 13.2, 1 12.2, 0 12.2))" );
  geom = mLayerPolygon2154->getFeature( -60 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 14.6, 0 15.6, 1 15.6, 1 14.6, 0 14.6))" );
  geom = mLayerPolygon2154->getFeature( -61 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 17, 0 18, 1 18, 1 17, 0 17))" );
  geom = mLayerPolygon2154->getFeature( -62 ).geometry();
  QCOMPARE( geom.transform( tr2154to3946 ), Qgis::GeometryOperationResult::Success );
  QCOMPARE( geom.asWkt( 1 ), "Polygon ((0 19.4, 0 20.4, 1 20.4, 1 19.4, 0 19.4))" );
  fids = mLayerPolygon2154->allFeatureIds();
  std::for_each( fids.constBegin(), fids.constEnd(), [this]( long fid ) { QCOMPARE( mLayerPolygon2154->getFeature( fid ).attribute( 0 ), "sure?" ); } );
  mLayerPolygon2154->undoStack()->undo();
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );

  // Nothing happened
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 20, 1, Qt::RightButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key::Key_Escape );
  QCOMPARE( mLayerPoint->featureCount(), 1 );
  QCOMPARE( mLayerLine->featureCount(), 1 );
  QCOMPARE( mLayerPolygon->featureCount(), 1 );
  QCOMPARE( mLayerPolygon2154->featureCount(), 1 );
}

QGSTEST_MAIN( TestQgsMapToolDistributeFeature )
#include "testqgsmaptooldistributefeature.moc"
