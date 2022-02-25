/***************************************************************************
     testqgsmaptoolrotatefeature.cpp
     --------------------------------
    Date                 : November 2019
    Copyright            : (C) 2019 by Nyall Dawson
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
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssnappingconfig.h"
#include "qgssnappingutils.h"
#include "qgsmaptoolrotatefeature.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolRotateFeature: public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolRotateFeature();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testRotateFeature();
    void testRotateFeatureManualAnchor();
    void testCancelManualAnchor();
    void testRotateFeatureManualAnchorAfterStartRotate();
    void testRotateFeatureManualAnchorSnapping();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolRotateFeature *mRotateTool = nullptr;
    QgsVectorLayer *mLayerBase = nullptr;
};

TestQgsMapToolRotateFeature::TestQgsMapToolRotateFeature() = default;


//runs before all tests
void TestQgsMapToolRotateFeature::initTestCase()
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
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerBase = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:3946" ), QStringLiteral( "baselayer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerBase->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerBase );

  mLayerBase->startEditing();
  const QString wkt1 = QStringLiteral( "Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))" );
  QgsFeature f1;
  f1.setGeometry( QgsGeometry::fromWkt( wkt1 ) );
  const QString wkt2 = QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" );
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
  mRotateTool = new QgsMapToolRotateFeature( mCanvas );
  mCanvas->setMapTool( mRotateTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolRotateFeature::cleanupTestCase()
{
  delete mRotateTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolRotateFeature::testRotateFeature()
{
  TestQgsMapToolUtils utils( mRotateTool );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-0.17 0.28, 0.28 1.17, 1.17 0.72, 0.72 -0.17, -0.17 0.28))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolRotateFeature::testRotateFeatureManualAnchor()
{
  // test rotating around a fixed anchor point
  TestQgsMapToolUtils utils( mRotateTool );

  // set anchor point
  utils.mouseClick( 0, 5, Qt::LeftButton, Qt::ControlModifier, true );

  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.08 0.12, 0.87 1.1, 1.84 1.31, 2.06 0.34, 1.08 0.12))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolRotateFeature::testCancelManualAnchor()
{
  // test canceling rotation around a fixed anchor point
  TestQgsMapToolUtils utils( mRotateTool );

  // set anchor point
  utils.mouseClick( 0, 5, Qt::LeftButton, Qt::ControlModifier, true );

  // right click = remove anchor point
  utils.mouseClick( 10, 15, Qt::RightButton, Qt::KeyboardModifiers(), true );

  // now rotate -- should be around feature center, not anchor point
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-0.17 0.28, 0.28 1.17, 1.17 0.72, 0.72 -0.17, -0.17 0.28))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolRotateFeature::testRotateFeatureManualAnchorAfterStartRotate()
{
  // test rotating around a fixed anchor point, where the fixed anchor point is placed after rotation begins
  TestQgsMapToolUtils utils( mRotateTool );

  // start rotation
  utils.mouseClick( 1, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  // set anchor point
  utils.mouseMove( 0, 5 );
  utils.mouseClick( 0, 5, Qt::LeftButton, Qt::ControlModifier, true );

  // complete rotation
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((-4.74 6.58, -3.79 6.26, -4.11 5.32, -5.06 5.63, -4.74 6.58))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" ) );

  mLayerBase->undoStack()->undo();
}

void TestQgsMapToolRotateFeature::testRotateFeatureManualAnchorSnapping()
{
  // test rotating around a fixed anchor point
  TestQgsMapToolUtils utils( mRotateTool );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  const double tolerance = cfg.tolerance();
  const QgsTolerance::UnitType units = cfg.units();
  cfg.setTolerance( 0.5 );
  cfg.setUnits( QgsTolerance::LayerUnits );
  mCanvas->snappingUtils()->setConfig( cfg );

  // set anchor point, should snap to (1.1, 5)
  utils.mouseMove( 1, 5.1 );
  utils.mouseClick( 1, 5.1, Qt::LeftButton, Qt::ControlModifier, true );

  // source point should snap to (1, 1)
  utils.mouseMove( 0.9, 0.9 );
  utils.mouseClick( 0.9, 0.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  // target point should snap to (2.1, 1)
  utils.mouseMove( 2, 1 );
  utils.mouseClick( 2, 1, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerBase->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.37 -0.11, 1.11 0.85, 2.07 1.12, 2.34 0.15, 1.37 -0.11))" ) );
  QCOMPARE( mLayerBase->getFeature( 2 ).geometry().asWkt( 2 ), QStringLiteral( "Polygon ((1.1 0, 1.1 5, 2.1 5, 2.1 0, 1.1 0))" ) );

  mLayerBase->undoStack()->undo();

  // restore tolerance setting
  cfg.setTolerance( tolerance );
  cfg.setUnits( units );
  mCanvas->snappingUtils()->setConfig( cfg );
}


QGSTEST_MAIN( TestQgsMapToolRotateFeature )
#include "testqgsmaptoolrotatefeature.moc"
