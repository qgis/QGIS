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

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgssnappingutils.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
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
    void testLargestGeometryToOriginalFeaturePolygon();
    void testLargestGeometryToOriginalFeatureLine();

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
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );

  // make testing layers
  mMultiLineStringLayer = new QgsVectorLayer( u"MultiLineString?crs=EPSG:3946"_s, u"layer multiline"_s, u"memory"_s );
  QVERIFY( mMultiLineStringLayer->isValid() );
  mMultiLineStringLayer->startEditing();
  lineF1.setGeometry( QgsGeometry::fromWkt( u"MultiLineString ((0 0, 10 0))"_s ) );
  lineF2.setGeometry( QgsGeometry::fromWkt( u"MultiLineString ((0 5, 10 5),(10 5, 15 5))"_s ) );
  mMultiLineStringLayer->dataProvider()->addFeatures( QgsFeatureList() << lineF1 << lineF2 );

  mPolygonLayer = new QgsVectorLayer( u"PolygonZ?crs=EPSG:3946"_s, u"layer polygon"_s, u"memory"_s );
  QVERIFY( mPolygonLayer->isValid() );
  mPolygonLayer->startEditing();
  polygonF1.setGeometry( QgsGeometry::fromWkt( u"PolygonZ ((0 5 10, 0 10 20, 10 10 30, 10 5 20, 0 5 10))"_s ) );
  polygonF2.setGeometry( QgsGeometry::fromWkt( u"PolygonZ ((0 0 10, 0 5 20, 10 5 30, 0 0 10))"_s ) );
  mPolygonLayer->dataProvider()->addFeatures( QgsFeatureList() << polygonF1 << polygonF2 );

  mMultiPolygonLayer = new QgsVectorLayer( u"MultiPolygon?crs=EPSG:3946"_s, u"layer multipolygon"_s, u"memory"_s );
  QVERIFY( mMultiPolygonLayer->isValid() );
  mMultiPolygonLayer->startEditing();
  multipolygonF1.setGeometry( QgsGeometry::fromWkt( u"MultiPolygon (((0 5, 0 10, 10 10, 10 5, 0 5)),((0 0, 0 4, 10 4, 10 0, 0 0)))"_s ) );
  mMultiPolygonLayer->dataProvider()->addFeature( multipolygonF1 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 50, 50 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer << mPolygonLayer << mMultiPolygonLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer << mPolygonLayer << mMultiPolygonLayer );

  QgsMapToolSplitFeatures *mapTool = new QgsMapToolSplitFeatures( mCanvas );
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

  QSet<QgsFeatureId> oldFids = mUtils->existingFeatureIds();

  mUtils->mouseClick( 4, 11, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::RightButton );

  QgsFeatureId newFid = mUtils->newFeatureId( oldFids );

  QCOMPARE( mPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mPolygonLayer->featureCount(), 3 );
  QCOMPARE( mPolygonLayer->getFeature( newFid ).geometry().asWkt(), u"Polygon Z ((4 10 24, 4 5 14, 0 5 10, 0 10 20, 4 10 24))"_s );
  QCOMPARE( mPolygonLayer->getFeature( 2 ).geometry().asWkt(), u"Polygon Z ((0 0 10, 0 5 20, 10 5 30, 0 0 10))"_s );

  // no change to other layers
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 0 );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 0 );
}

void TestQgsMapToolSplitFeatures::testSplitPolygonTopologicalEditing()
{
  QgsProject::instance()->setTopologicalEditing( true );
  mCanvas->setCurrentLayer( mPolygonLayer );

  QSet<QgsFeatureId> oldFids = mUtils->existingFeatureIds();

  mUtils->mouseClick( 4, 11, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::LeftButton );
  mUtils->mouseClick( 4, 3, Qt::RightButton );

  QgsFeatureId newFid = mUtils->newFeatureId( oldFids );

  QCOMPARE( mPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mPolygonLayer->featureCount(), 3 );
  QCOMPARE( mPolygonLayer->getFeature( newFid ).geometry().asWkt(), u"Polygon Z ((4 10 24, 4 5 14, 0 5 10, 0 10 20, 4 10 24))"_s );
  QCOMPARE( mPolygonLayer->getFeature( 2 ).geometry().asWkt(), u"Polygon Z ((0 0 10, 0 5 20, 4 5 14, 10 5 30, 0 0 10))"_s );

  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 1 );
  QCOMPARE( mMultiLineStringLayer->getFeature( 2 ).geometry().asWkt(), u"MultiLineString ((0 5, 4 5, 10 5),(10 5, 15 5))"_s );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 1 );
  QCOMPARE( mMultiPolygonLayer->getFeature( 1 ).geometry().asWkt(), u"MultiPolygon (((0 5, 0 10, 4 10, 10 10, 10 5, 4 5, 0 5)),((0 0, 0 4, 10 4, 10 0, 0 0)))"_s );

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
  QCOMPARE( mPolygonLayer->getFeature( 1 ).geometry().asWkt( 2 ), u"Polygon Z ((0 5 10, 0 10 20, 10 10 30, 10 5 20, 0 5 10))"_s );

  // No change to other layers
  QCOMPARE( mMultiLineStringLayer->undoStack()->index(), 0 );
  QCOMPARE( mMultiPolygonLayer->undoStack()->index(), 0 );

  mCanvas->snappingUtils()->setConfig( oldCfg );
}

void TestQgsMapToolSplitFeatures::testLargestGeometryToOriginalFeaturePolygon()
{
  mCanvas->setCurrentLayer( mPolygonLayer );

  QSet<QgsFeatureId> oldFids = mUtils->existingFeatureIds();

  mUtils->mouseClick( 1, 11, Qt::LeftButton );
  mUtils->mouseClick( 1, 5, Qt::LeftButton );
  mUtils->mouseClick( 1, 5, Qt::RightButton );

  QgsFeatureId newFid = mUtils->newFeatureId( oldFids );

  QCOMPARE( mPolygonLayer->featureCount(), 3 );

  QgsFeature oldFeat = mPolygonLayer->getFeature( 1 );
  QgsFeature newFeat = mPolygonLayer->getFeature( newFid );

  //larger
  QCOMPARE( oldFeat.geometry().asWkt(), u"Polygon Z ((1 5 10, 1 10 21, 10 10 30, 10 5 20, 1 5 10))"_s );

  //smaller
  QCOMPARE( newFeat.geometry().asWkt(), u"Polygon Z ((1 10 21, 1 5 10, 0 5 10, 0 10 20, 1 10 21))"_s );
}

void TestQgsMapToolSplitFeatures::testLargestGeometryToOriginalFeatureLine()
{
  mCanvas->setCurrentLayer( mMultiLineStringLayer );

  QSet<QgsFeatureId> oldFids = mUtils->existingFeatureIds();

  mUtils->mouseClick( 4, 1, Qt::LeftButton );
  mUtils->mouseClick( 4, -1, Qt::LeftButton );
  mUtils->mouseClick( 4, -1, Qt::RightButton );

  QgsFeatureId newFid = mUtils->newFeatureId( oldFids );

  QgsFeature oldFeat = mMultiLineStringLayer->getFeature( 1 );
  QgsFeature newFeat = mMultiLineStringLayer->getFeature( newFid );

  //larger
  QCOMPARE( oldFeat.geometry().asWkt(), u"MultiLineString ((4 0, 10 0))"_s );

  //smaller
  QCOMPARE( newFeat.geometry().asWkt(), u"MultiLineString ((0 0, 4 0))"_s );
}

QGSTEST_MAIN( TestQgsMapToolSplitFeatures )
#include "testqgsmaptoolsplitfeatures.moc"
