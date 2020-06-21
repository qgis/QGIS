/***************************************************************************
     testqgsmaptooladdfeatureline.cpp
     ----------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
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
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvastracer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"

bool operator==( const QgsGeometry &g1, const QgsGeometry &g2 )
{
  if ( g1.isNull() && g2.isNull() )
    return true;
  else
    return g1.isGeosEqual( g2 );
}

namespace QTest
{
  // pretty printing of geometries in comparison tests
  template<> char *toString( const QgsGeometry &geom )
  {
    QByteArray ba = geom.asWkt().toLatin1();
    return qstrdup( ba.data() );
  }
}


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolAddFeatureLine : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeatureLine();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testNoTracing();
    void testTracing();
    void testTracingWithOffset();
    void testTracingWithConvertToCurves();
    void testZ();
    void testZMSnapping();
    void testTopologicalEditingZ();
    void testCloseLine();
    void testSelfSnapping();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapCanvasTracer *mTracer = nullptr;
    QAction *mEnableTracingAction = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerLineCurved = nullptr;
    QgsVectorLayer *mLayerLineZ = nullptr;
    QgsVectorLayer *mLayerPointZM = nullptr;
    QgsVectorLayer *mLayerTopoZ = nullptr;
    QgsVectorLayer *mLayerLine2D = nullptr;
    QgsVectorLayer *mLayerCloseLine = nullptr;
    QgsVectorLayer *mLayerSelfSnapLine = nullptr;
    QgsFeatureId mFidLineF1 = 0;
    QgsFeatureId mFidCurvedF1 = 0;
};

TestQgsMapToolAddFeatureLine::TestQgsMapToolAddFeatureLine() = default;


//runs before all tests
void TestQgsMapToolAddFeatureLine::initTestCase()
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

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayerLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLine );

  QgsPolylineXY line1;
  line1 << QgsPointXY( 1, 1 ) << QgsPointXY( 2, 1 ) << QgsPointXY( 3, 2 ) << QgsPointXY( 1, 2 ) << QgsPointXY( 1, 1 );
  QgsFeature lineF1;
  lineF1.setGeometry( QgsGeometry::fromPolylineXY( line1 ) );

  mLayerLine->startEditing();
  mLayerLine->addFeature( lineF1 );
  mFidLineF1 = lineF1.id();
  QCOMPARE( mLayerLine->featureCount(), ( long )1 );

  // just one added feature
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // make testing layers
  mLayerLineCurved = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "curved layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineCurved->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineCurved );

  QgsFeature curveF1;
  curveF1.setGeometry( QgsGeometry::fromWkt( "CIRCULARSTRING(6 1, 6.5 1.5, 7 1)" ) );

  mLayerLineCurved->startEditing();
  mLayerLineCurved->addFeature( curveF1 );
  mFidCurvedF1 = curveF1.id();
  QCOMPARE( mLayerLineCurved->featureCount(), ( long )1 );

  // just one added feature
  QCOMPARE( mLayerLineCurved->undoStack()->index(), 1 );

  // make testing layers
  mLayerLineZ = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineZ );

  QgsPolyline line2;
  line2 << QgsPoint( 1, 1, 0 ) << QgsPoint( 2, 1, 1 ) << QgsPoint( 3, 2, 2 ) << QgsPoint( 1, 2, 3 ) << QgsPoint( 1, 1, 0 );
  QgsFeature lineF2;
  lineF2.setGeometry( QgsGeometry::fromPolyline( line2 ) );

  mLayerLineZ->startEditing();
  mLayerLineZ->addFeature( lineF2 );
  QCOMPARE( mLayerLineZ->featureCount(), ( long )1 );

  mLayerCloseLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line Closed" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerCloseLine->isValid() );
  mLayerCloseLine->startEditing();
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerCloseLine );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // make layer for snapping
  mLayerPointZM = new QgsVectorLayer( QStringLiteral( "PointZM?crs=EPSG:27700" ), QStringLiteral( "layer point ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPointZM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPointZM );

  mLayerPointZM->startEditing();
  QgsFeature pointF;
  QString pointWktZM = "PointZM(6 6 3 4)";
  pointF.setGeometry( QgsGeometry::fromWkt( pointWktZM ) );

  mLayerPointZM->addFeature( pointF );
  QCOMPARE( mLayerPointZM->featureCount(), ( long )1 );

  // make layer for topologicalEditing with Z
  mLayerTopoZ = new QgsVectorLayer( QStringLiteral( "MultiLineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer topologicalEditing Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerTopoZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerTopoZ );

  mLayerTopoZ->startEditing();
  QgsFeature topoFeat;
  topoFeat.setGeometry( QgsGeometry::fromWkt( "MultiLineStringZ ((7.25 6 0, 7.25 7 0, 7.5 7 0, 7.5 6 0, 7.25 6 0),(6 6 0, 6 7 10, 7 7 0, 7 6 0, 6 6 0),(6.25 6.25 0, 6.75 6.25 0, 6.75 6.75 0, 6.25 6.75 0, 6.25 6.25 0)));" ) );

  mLayerTopoZ->addFeature( topoFeat );
  QCOMPARE( mLayerTopoZ->featureCount(), ( long ) 1 );

  // make 2D layer for snapping with a 3D layer
  mLayerLine2D = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine2D->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLine2D );

  mLayerLine2D->startEditing();
  QgsFeature lineString2DF;
  lineString2DF.setGeometry( QgsGeometry::fromWkt( "LineString ((8 8, 9 9))" ) );

  mLayerLine2D->addFeature( lineString2DF );
  QCOMPARE( mLayerLine2D->featureCount(), ( long )1 );

  // make testing layers
  mLayerSelfSnapLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerSelfSnapLine->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerSelfSnapLine );
  mLayerSelfSnapLine->startEditing();

  // add layers to canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerLineCurved << mLayerLineZ << mLayerPointZM << mLayerTopoZ << mLayerLine2D << mLayerSelfSnapLine );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, /*mAdvancedDigitizingDockWidget, */ QgsMapToolCapture::CaptureLine );

  mCanvas->setMapTool( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerLine );

  mEnableTracingAction = new QAction( nullptr );
  mEnableTracingAction->setCheckable( true );

  mTracer = new QgsMapCanvasTracer( mCanvas, nullptr );
  mTracer->setActionEnableTracing( mEnableTracingAction );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeatureLine::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeatureLine::testNoTracing()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  // tracing not enabled - will be straight line

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::RightButton );

  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 3 2)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
}

void TestQgsMapToolAddFeatureLine::testTracing()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  // tracing enabled - same clicks - now following line

  mEnableTracingAction->setChecked( true );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::RightButton );

  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 2 1, 3 2)" ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // tracing enabled - combined with first and last segments that are not traced

  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );

  QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid2 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(0 2, 1 1, 2 1, 3 2, 4 1)" ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  mEnableTracingAction->setChecked( false );
}

void TestQgsMapToolAddFeatureLine::testTracingWithOffset()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  // tracing enabled + offset enabled

  mEnableTracingAction->setChecked( true );
  mTracer->setOffset( 0.1 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 2, 1, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );

  QgsGeometry g = mLayerLine->getFeature( newFid ).geometry();
  QgsPolylineXY poly = g.asPolyline();
  QCOMPARE( poly.count(), 3 );
  QCOMPARE( poly[0], QgsPointXY( 2, 0.9 ) );
  QCOMPARE( poly[1], QgsPointXY( 0.9, 0.9 ) );
  QCOMPARE( poly[2], QgsPointXY( 0.9, 2 ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // use negative offset
  mTracer->setOffset( -0.1 );

  utils.mouseClick( 2, 1, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  QgsGeometry g2 = mLayerLine->getFeature( newFid2 ).geometry();
  QgsPolylineXY poly2 = g2.asPolyline();
  QCOMPARE( poly2.count(), 3 );
  QCOMPARE( poly2[0], QgsPointXY( 2, 1.1 ) );
  QCOMPARE( poly2[1], QgsPointXY( 1.1, 1.1 ) );
  QCOMPARE( poly2[2], QgsPointXY( 1.1, 2 ) );

  mLayerLine->undoStack()->undo();

  // tracing enabled + offset enabled - combined with first and last segments that are not traced

  utils.mouseClick( 3, 0, Qt::LeftButton );
  utils.mouseClick( 2, 1, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::LeftButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 0, 1, Qt::RightButton );

  QgsFeatureId newFid3 = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QgsGeometry g3 = mLayerLine->getFeature( newFid3 ).geometry();
  QgsPolylineXY poly3 = g3.asPolyline();
  QCOMPARE( poly3.count(), 5 );
  QCOMPARE( poly3[0], QgsPointXY( 3, 0 ) );
  QCOMPARE( poly3[1], QgsPointXY( 2, 1.1 ) );
  QCOMPARE( poly3[2], QgsPointXY( 1.1, 1.1 ) );
  QCOMPARE( poly3[3], QgsPointXY( 1.1, 2 ) );
  QCOMPARE( poly3[4], QgsPointXY( 0, 1 ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );


  mEnableTracingAction->setChecked( false );
  mTracer->setOffset( 0 );
}

void TestQgsMapToolAddFeatureLine::testTracingWithConvertToCurves()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineCurved );

  // enable snapping and tracing
  mEnableTracingAction->setChecked( true );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  // tracing enabled - without converting to curves
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), false );

  utils.mouseClick( 6, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::RightButton );

  QgsFeatureId newFid1 = utils.newFeatureId( oldFids );

  const QgsAbstractGeometry *g = mLayerLineCurved->getFeature( newFid1 ).geometry().constGet();
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 6, 1 ) );
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, g->vertexCount() - 1 ) ), QgsPoint( 7, 1 ) );
  QVERIFY( g->vertexCount() > 3 );  // a segmentized arc has (much) more than 3 points

  mLayerLineCurved->undoStack()->undo();

  // we redo the same with convert to curves enabled
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), true );

  // tracing enabled - without converting to curves
  utils.mouseClick( 6, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::RightButton );

  QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  g = mLayerLineCurved->getFeature( newFid2 ).geometry().constGet();
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 6, 1 ) );
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, g->vertexCount() - 1 ) ), QgsPoint( 7, 1 ) );
  QVERIFY( g->vertexCount() == 3 );  // a true arc is composed of 3 vertices

  mLayerLineCurved->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLineCurved->undoStack()->index(), 1 );

  mEnableTracingAction->setChecked( false );
}

void TestQgsMapToolAddFeatureLine::testZ()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineZ );

  // test with default Z value = 333
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineStringZ (4 0 333, 5 0 333, 5 1 333, 4 1 333)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  // test with default Z value = 222
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 222 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );
  newFid = utils.newFeatureId( oldFids );

  wkt = "LineStringZ (4 0 222, 5 0 222, 5 1 222, 4 1 222)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  mCanvas->setCurrentLayer( mLayerLine );
}

void TestQgsMapToolAddFeatureLine::testZMSnapping()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  // snap a on a layer with ZM support
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 5, 0, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->getFeature( newFid ).geometry().get()->is3D(), false );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry().get()->isMeasure(), false );

  mLayerLine->undoStack()->undo();

  mCanvas->setCurrentLayer( mLayerLineZ );
  oldFids = utils.existingFeatureIds();
  // test with default Z value = 222
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 222 );
  // snap a on a layer without ZM support
  utils.mouseClick( 9, 9, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 8, 7, Qt::LeftButton );
  utils.mouseClick( 5, 0, Qt::RightButton );

  newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->is3D(), true );

  QString wkt = "LineStringZ (9 9 222, 8 7 222)";
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );
  QCOMPARE( mLayerLineZ->getFeature( newFid ).geometry().get()->isMeasure(), false );

  mLayerLine->undoStack()->undo();

  cfg.setEnabled( false );

  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
}

void TestQgsMapToolAddFeatureLine::testTopologicalEditingZ()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerTopoZ );

  // test with default Z value = 333
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  bool topologicalEditing = cfg.project()->topologicalEditing();
  cfg.project()->setTopologicalEditing( true );

  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.5, 6.5, Qt::LeftButton );
  utils.mouseClick( 8, 6.5, Qt::RightButton );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineStringZ (6 6.5 5, 6.25 6.5 333, 6.75 6.5 333, 7.25 6.5 333, 7.5 6.5 333)";
  QCOMPARE( mLayerTopoZ->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );
  wkt = "MultiLineStringZ ((7.25 6 0, 7.25 6.5 333, 7.25 7 0, 7.5 7 0, 7.5 6.5 333, 7.5 6 0, 7.25 6 0),(6 6 0, 6 6.5 5, 6 7 10, 7 7 0, 7 6 0, 6 6 0),(6.25 6.25 0, 6.75 6.25 0, 6.75 6.5 333, 6.75 6.75 0, 6.25 6.75 0, 6.25 6.5 333, 6.25 6.25 0))";
  QCOMPARE( mLayerTopoZ->getFeature( qgis::setToList( oldFids ).last() ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();

  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
  cfg.project()->setTopologicalEditing( topologicalEditing );
}

void TestQgsMapToolAddFeatureLine::testCloseLine()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerCloseLine );
  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 5, 5, Qt::LeftButton );
  utils.mouseClick( 5, 5, Qt::RightButton, Qt::ShiftModifier );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QString wkt = "LineString (1 1, 5 1, 5 5, 1 1)";
  QCOMPARE( mLayerCloseLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerCloseLine->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testSelfSnapping()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerSelfSnapLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( true );
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setTypeFlag( QgsSnappingConfig::VertexFlag );
  cfg.setTolerance( 50 );
  cfg.setUnits( QgsTolerance::Pixels );
  mCanvas->snappingUtils()->setConfig( cfg );


  QString targetWkt = "LineString (2 5, 3 5, 3 6, 2 5)";

  // Without self snapping, endpoint won't snap to start point
  cfg.setSelfSnapping( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  utils.mouseClick( 2, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::RightButton );

  QgsFeatureId newFid1 = utils.newFeatureId( oldFids );
  QVERIFY( ! mLayerSelfSnapLine->getFeature( newFid1 ).geometry().equals( QgsGeometry::fromWkt( targetWkt ) ) );
  mLayerSelfSnapLine->undoStack()->undo();

  // With self snapping, endpoint will snap to start point
  cfg.setSelfSnapping( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  utils.mouseClick( 2, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::RightButton );

  QgsFeatureId newFid2 = utils.newFeatureId( oldFids );
  QCOMPARE( mLayerSelfSnapLine->getFeature( newFid2 ).geometry(), QgsGeometry::fromWkt( targetWkt ) );
  mLayerSelfSnapLine->undoStack()->undo();

}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLine )
#include "testqgsmaptooladdfeatureline.moc"
