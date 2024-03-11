/***************************************************************************
     testqgsmaptoolsplitfeatures.cpp
     ------------------------
    Date                 : August 2020
    Copyright            : (C) 2020 by Stefanos Natsis
    Email                : uclaros@gmail.com
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
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsgeometryutils.h"

#include "testqgsmaptoolutils.h"

class TestQgsMapToolSplitFeatures : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolSplitFeatures();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testNoFeaturesSplit();
    void testSplitPolygon();
    void testSplitPolygonTopologicalEditing();
    void testSplitSelectedLines();
    void testSplitSomeOfSelectedLines();
    // see https://github.com/qgis/QGIS/issues/29270
    void testSplitPolygonSnapToSegment();

  private:
    QPoint mapToPoint( double x, double y );
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    TestQgsMapToolUtils *mUtils = nullptr;
    QgsVectorLayer *mMultiLineStringLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;
    QgsVectorLayer *mMultiPolygonLayer = nullptr;
    QgsFeature lineF1, lineF2, polygonF1, polygonF2, multipolygonF1;
};

TestQgsMapToolSplitFeatures::TestQgsMapToolSplitFeatures() = default;


//runs before all tests
void TestQgsMapToolSplitFeatures::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );

  // make testing layers
  mMultiLineStringLayer = new QgsVectorLayer( QStringLiteral( "MultiLineString?crs=EPSG:3946" ), QStringLiteral( "layer multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( mMultiLineStringLayer->isValid() );
  mMultiLineStringLayer->startEditing();
  lineF1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((0 0, 10 0))" ) ) );
  lineF2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((0 5, 10 5),(10 5, 15 5))" ) ) );
  mMultiLineStringLayer->dataProvider()->addFeatures( QgsFeatureList() << lineF1 << lineF2 );

  mPolygonLayer = new QgsVectorLayer( QStringLiteral( "PolygonZ?crs=EPSG:3946" ), QStringLiteral( "layer polygon" ), QStringLiteral( "memory" ) );
  QVERIFY( mPolygonLayer->isValid() );
  mPolygonLayer->startEditing();
  polygonF1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PolygonZ ((0 5 10, 0 10 20, 10 10 30, 10 5 20, 0 5 10))" ) ) );
  polygonF2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "PolygonZ ((0 0 10, 0 5 20, 10 5 30, 0 0 10))" ) ) );
  mPolygonLayer->dataProvider()->addFeatures( QgsFeatureList() << polygonF1 << polygonF2 );

  mMultiPolygonLayer = new QgsVectorLayer( QStringLiteral( "MultiPolygon?crs=EPSG:3946" ), QStringLiteral( "layer multipolygon" ), QStringLiteral( "memory" ) );
  QVERIFY( mMultiPolygonLayer->isValid() );
  mMultiPolygonLayer->startEditing();
  multipolygonF1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiPolygon (((0 5, 0 10, 10 10, 10 5, 0 5)),((0 0, 0 4, 10 4, 10 0, 0 0)))" ) ) );
  mMultiPolygonLayer->dataProvider()->addFeature( multipolygonF1 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 50, 50 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer
                                        << mPolygonLayer
                                        << mMultiPolygonLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer
                      << mPolygonLayer
                      << mMultiPolygonLayer );

  QgsMapToolSplitFeatures *mapTool = new QgsMapToolSplitFeatures( mCanvas ) ;
  mCanvas->setMapTool( mapTool );
  mUtils = new TestQgsMapToolUtils( mapTool );
}

void TestQgsMapToolSplitFeatures::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

// runs after each test
void TestQgsMapToolSplitFeatures::cleanup()
{
  mMultiLineStringLayer->undoStack()->setIndex( 0 );
  mMultiLineStringLayer->removeSelection();
  mPolygonLayer->undoStack()->setIndex( 0 );
  mPolygonLayer->removeSelection();
  mMultiPolygonLayer->undoStack()->setIndex( 0 );
  mMultiPolygonLayer->removeSelection();
}

QPoint TestQgsMapToolSplitFeatures::mapToPoint( double x, double y )
{

  const QgsPointXY mapPoint = mCanvas->mapSettings().mapToPixel().transform( x, y );

  return QPoint( std::round( mapPoint.x() ), std::round( mapPoint.y() ) );
}

void TestQgsMapToolSplitFeatures::testNoFeaturesSplit()
{
  mCanvas->setCurrentLayer( mMultiLineStringLayer );

  mUtils->mouseClick( 4, 7, Qt::LeftButton );
  mUtils->mouseClick( 4, 8, Qt::LeftButton );
  mUtils->mouseClick( 4, 8, Qt::RightButton );

  QCOMPARE( mMultiLineStringLayer->featureCount(), 2 );
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 0 );
}

void TestQgsMapToolSplitFeatures::testSplitPolygon()
{
  mCanvas->setCurrentLayer( mPolygonLayer );

  mUtils->mouseClick( 4, 11, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::RightButton );

  QCOMPARE( mPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mPolygonLayer->featureCount(), 3 );
  QCOMPARE( mPolygonLayer->getFeature( 1 ).geometry().asWkt(), QStringLiteral( "PolygonZ ((4 10 24, 4 5 14, 0 5 10, 0 10 20, 4 10 24))" ) );
  QCOMPARE( mPolygonLayer->getFeature( 2 ).geometry().asWkt(), QStringLiteral( "PolygonZ ((0 0 10, 0 5 20, 10 5 30, 0 0 10))" ) );

  // no change to other layers
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 0 );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 0 );
}

void TestQgsMapToolSplitFeatures::testSplitPolygonTopologicalEditing()
{
  QgsProject::instance()->setTopologicalEditing( true );
  mCanvas->setCurrentLayer( mPolygonLayer );

  mUtils->mouseClick( 4, 11, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::RightButton );

  QCOMPARE( mPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mPolygonLayer->featureCount(), 3 );
  QCOMPARE( mPolygonLayer->getFeature( 1 ).geometry().asWkt(), QStringLiteral( "PolygonZ ((4 10 24, 4 5 14, 0 5 10, 0 10 20, 4 10 24))" ) );
  QCOMPARE( mPolygonLayer->getFeature( 2 ).geometry().asWkt(), QStringLiteral( "PolygonZ ((0 0 10, 0 5 20, 4 5 14, 10 5 30, 0 0 10))" ) );

  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 1 );
  QCOMPARE( mMultiLineStringLayer->getFeature( 2 ).geometry().asWkt(), QStringLiteral( "MultiLineString ((0 5, 4 5, 10 5),(10 5, 15 5))" ) );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mMultiPolygonLayer->getFeature( 1 ).geometry().asWkt(), QStringLiteral( "MultiPolygon (((0 5, 0 10, 4 10, 10 10, 10 5, 4 5, 0 5)),((0 0, 0 4, 10 4, 10 0, 0 0)))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsMapToolSplitFeatures::testSplitSelectedLines()
{
  mMultiLineStringLayer->select( 1 );
  mCanvas->setCurrentLayer( mMultiLineStringLayer );

  mUtils->mouseClick( 5, 6, Qt::LeftButton );
  mUtils->mouseClick( 5, -1, Qt::LeftButton );
  mUtils->mouseClick( 5, -1, Qt::RightButton );

  // only the selected feature should be split
  QCOMPARE( mMultiLineStringLayer->featureCount(), 3 );
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 1 );
}

void TestQgsMapToolSplitFeatures::testSplitSomeOfSelectedLines()
{
  mMultiLineStringLayer->selectAll();
  mCanvas->setCurrentLayer( mMultiLineStringLayer );

  mUtils->mouseClick( 5, 1, Qt::LeftButton );
  mUtils->mouseClick( 5, -1, Qt::LeftButton );
  mUtils->mouseClick( 5, -1, Qt::RightButton );

  // intersecting selected feature should be split
  QCOMPARE( mMultiLineStringLayer->featureCount(), 3 );
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 1 );
}

void TestQgsMapToolSplitFeatures::testSplitPolygonSnapToSegment()
{
  QgsProject::instance()->setTopologicalEditing( false );
  QgsSnappingConfig oldCfg = mCanvas->snappingUtils()->config();
  QgsSnappingConfig cfg( oldCfg );
  cfg.setEnabled( true );
  cfg.setTolerance( 20 );
  cfg.setUnits( Qgis::MapToolUnit::Pixels );
  cfg.setMode( Qgis::SnappingMode::ActiveLayer );
  cfg.setTypeFlag( Qgis::SnappingType::Segment );
  mCanvas->snappingUtils()->setConfig( cfg );
  mCanvas->snappingUtils()->locatorForLayer( mPolygonLayer )->init();

  mCanvas->setCurrentLayer( mPolygonLayer );

  mUtils->mouseClick( 1, 0.6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 1, 4.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 2, 1.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 2, 4.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 3, 1.6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 3, 4.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 4, 2.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 4, 4.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 5, 2.6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 5, 4.9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 6, 3.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mUtils->mouseClick( 6, 3.1, Qt::RightButton, Qt::KeyboardModifiers(), true );

  // Split line should split the triangle into 11 triangles
  QCOMPARE( mPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mPolygonLayer->featureCount(), 12 );

  // No change to the other feature in the layer
  QCOMPARE( mPolygonLayer->getFeature( 1 ).geometry().asWkt( 2 ), QStringLiteral( "PolygonZ ((0 5 10, 0 10 20, 10 10 30, 10 5 20, 0 5 10))" ) );

  // No change to other layers
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 0 );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 0 );

  mCanvas->snappingUtils()->setConfig( oldCfg );
}

QGSTEST_MAIN( TestQgsMapToolSplitFeatures )
#include "testqgsmaptoolsplitfeatures.moc"
