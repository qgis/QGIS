/***************************************************************************
     testqgsmaptoolscalefeature.cpp
     --------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by roya0045
    Contact              : ping me on github
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
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgssnappingutils.h"
#include "qgsmaptoolscalefeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolScaleFeature: public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolScaleFeature();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testScaleFeature();
    void testScaleFeatureWithAnchor();
    void testCancelManualAnchor();
    void testScaleFeatureWithAnchorSetAfterStart();
    void testScaleSelectedFeatures();
    void testScaleFeatureManualAnchorSnapping();
    void testScaleFeatureDifferentCrs();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolScaleFeature *mScaleTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolScaleFeature::TestQgsMapToolScaleFeature() = default;


//runs before all tests
void TestQgsMapToolScaleFeature::initTestCase()
{
  qDebug() << "TestMapToolCapture::initTestCase()";
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
  mCanvas->setExtent( QgsRectangle( -4, -4, 4, 4 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerBase = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:3946" ), QStringLiteral( "baselayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerBase->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerBase );

  mLayerBase->startEditing();
  const QString wkt1 = QStringLiteral( "Polygon ((-2 -2, -2 -1, -1 -1, -1 -2, -2 -2))" );
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  const QString wkt2 = QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" );
  QgsFeature f2;
  f2.setGeometry( QgsGeometry::fromWkt( wkt2 ) );

  QgsFeatureList flist;
  flist << f1 << f2;
  mLayerBase->dataProvider()->addFeatures( flist );
  QCOMPARE( mLayerBase->featureCount(), 2L );
  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt(), wkt1 );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 1 ), wkt2 );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setTolerance( 1 );
  cfg.setTypeFlag( static_cast<Qgis::SnappingTypes>( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment ) );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerBase );
  mCanvas->setCurrentLayer( mLayerBase );
  mCanvas->snappingUtils()->locatorForLayer( mLayerBase )->init();

  // create the tool
  mScaleTool = new QgsMapToolScaleFeature( mCanvas );
  mCanvas->setMapTool( mScaleTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( -4, -4, 4, 4 ) );
}

//runs after all tests
void TestQgsMapToolScaleFeature::cleanupTestCase()
{
  delete mScaleTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolScaleFeature::testScaleFeature()
{
  TestQgsMapToolUtils utils( mScaleTool );

  //scale up
  utils.mouseClick( -2, -1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -2.5, -0.5 );
  utils.mouseClick( -2.5, -0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.5 -2.5, -2.5 -0.5, -0.5 -0.5, -0.5 -2.5, -2.5 -2.5))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  //scale down
  utils.mouseClick( 1.1, 0.8, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 1.35, 1.85 );
  utils.mouseClick( 1.35, 1.85, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.5 -2.5, -2.5 -0.5, -0.5 -0.5, -0.5 -2.5, -2.5 -2.5))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.35 1.84, 1.35 3.96, 1.85 3.96, 1.85 1.84, 1.35 1.84))" ) );

  mLayerBase->undoStack()->undo();
  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolScaleFeature::testScaleFeatureWithAnchor()
{
  TestQgsMapToolUtils utils( mScaleTool );

  //set manual anchor point
  utils.mouseClick( 1, -1, Qt::LeftButton, Qt::ControlModifier, true );

  // resize
  utils.mouseClick( -2, -1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -2.5, -0.5 );
  utils.mouseClick( -2.5, -0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.54 -2.18, -2.54 -1, -1.36 -1, -1.36 -2.18, -2.54 -2.18))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolScaleFeature::testCancelManualAnchor()
{
  TestQgsMapToolUtils utils( mScaleTool );

  //set manual anchor point
  utils.mouseClick( 1, -1, Qt::LeftButton, Qt::ControlModifier, true );

  // remove manual anchor point via right click
  utils.mouseClick( 10, 25, Qt::RightButton, Qt::KeyboardModifiers(), true );

  // resize
  utils.mouseClick( -2, -1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -2.5, -0.5 );
  utils.mouseClick( -2.5, -0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.5 -2.5, -2.5 -0.5, -0.5 -0.5, -0.5 -2.5, -2.5 -2.5))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolScaleFeature::testScaleFeatureWithAnchorSetAfterStart()
{
  TestQgsMapToolUtils utils( mScaleTool );

  // resize
  utils.mouseClick( -2, -1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  //set manual anchor point
  utils.mouseClick( 1, -1, Qt::LeftButton, Qt::ControlModifier, true );

  utils.mouseMove( -2.5, -0.5 );
  utils.mouseClick( -2.5, -0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-14 -6, -14 -1, -9 -1, -9 -6, -14 -6))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolScaleFeature::testScaleSelectedFeatures()
{
  TestQgsMapToolUtils utils( mScaleTool );
  mLayerBase->selectAll();

  // resize
  utils.mouseClick( -2, -1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -2.5, -0.5 );
  utils.mouseClick( -2.5, -0.5, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.54 -2.18, -2.54 -1, -1.36 -1, -1.36 -2.18, -2.54 -2.18))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.12 1.12, 1.12 6.07, 2.3 6.07, 2.3 1.12, 1.12 1.12))" ) );

  mLayerBase->removeSelection();
  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolScaleFeature::testScaleFeatureManualAnchorSnapping()
{
  TestQgsMapToolUtils utils( mScaleTool );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  const double tolerance = cfg.tolerance();
  const QgsTolerance::UnitType units = cfg.units();
  cfg.setTolerance( 0.5 );
  cfg.setUnits( QgsTolerance::LayerUnits );
  mCanvas->snappingUtils()->setConfig( cfg );

  //set manual anchor point, should snap to (-2, -2)
  utils.mouseClick( -1.9, -1.9, Qt::LeftButton, Qt::ControlModifier, true );

  // resize, source point should snap to (-1, -1)
  utils.mouseMove( -0.9, -0.9 );
  utils.mouseClick( -0.9, -0.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // target point should snap to (1.1, 0.8)
  utils.mouseMove( 1.2, 0.9 );
  utils.mouseClick( 1.2, 0.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2 -2, -2 0.95, 0.95 0.95, 0.95 -2, -2 -2))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  mLayerBase->undoStack()->undo();

  // restore tolerance setting
  cfg.setTolerance( tolerance );
  cfg.setUnits( units );
  mCanvas->snappingUtils()->setConfig( cfg );

  // remove manual anchor point via right click
  utils.mouseClick( 10, 25, Qt::RightButton, Qt::KeyboardModifiers(), true );
}

void TestQgsMapToolScaleFeature::testScaleFeatureDifferentCrs()
{
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  TestQgsMapToolUtils utils( mScaleTool );

  //scale up
  utils.mouseClick( -8.82187821744550682, 2.0909475540607434, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -8.82188215592444536, 2.09095048559432861 );
  utils.mouseClick( -8.82188215592444536, 2.09095048559432861, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.5 -2.5, -2.5 -0.5, -0.5 -0.5, -0.5 -2.5, -2.5 -2.5))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0.8, 1.1 5, 2.1 5, 2.1 0.8, 1.1 0.8))" ) );

  //scale down
  utils.mouseClick( -8.82185881943214234, 2.09096315856551129, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( -8.82185818217576667, 2.09097065484482636 );
  utils.mouseClick( -8.82185818217576667, 2.09097065484482636, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-2.5 -2.5, -2.5 -0.5, -0.5 -0.5, -0.5 -2.5, -2.5 -2.5))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.35 1.84, 1.35 3.96, 1.85 3.96, 1.85 1.84, 1.35 1.84))" ) );

  mLayerBase->undoStack()->undo();
  mLayerBase->undoStack()->undo();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
}

QGSTEST_MAIN( TestQgsMapToolScaleFeature )
#include "testqgsmaptoolscalefeature.moc"
