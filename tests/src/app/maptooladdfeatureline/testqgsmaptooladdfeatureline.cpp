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
    return g1.equals( g2 );
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
    void testTracingWithConvertToCurvesCustomTolerance();
    void testCloseLine();
    void testSelfSnapping();
    void testLineString();
    void testCompoundCurve();
    void testStream();
    void testUndo();
    void testStreamTolerance();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapCanvasTracer *mTracer = nullptr;
    QAction *mEnableTracingAction = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerLineCurved = nullptr;
    QgsVectorLayer *mLayerLineCurvedOffset = nullptr;
    QgsVectorLayer *mLayerLineZ = nullptr;
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
  QgsSettings settings;
  settings.clear();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayerLine = new QgsVectorLayer( QStringLiteral( "CompoundCurve?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
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

  // make testing layers for tracing curves
  mLayerLineCurved = new QgsVectorLayer( QStringLiteral( "CompoundCurve?crs=EPSG:27700" ), QStringLiteral( "curved layer line" ), QStringLiteral( "memory" ) );
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

  // make testing layers for tracing curves with offset
  mLayerLineCurvedOffset = new QgsVectorLayer( QStringLiteral( "CompoundCurve?crs=EPSG:27700" ), QStringLiteral( "curved layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineCurvedOffset->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLineCurvedOffset );

  QgsFeature curveF2;
  curveF2.setGeometry( QgsGeometry::fromWkt( "CIRCULARSTRING(100000006 100000001, 100000006.5 100000001.5, 100000007 100000001)" ) );

  mLayerLineCurvedOffset->startEditing();
  mLayerLineCurvedOffset->addFeature( curveF2 );
  mFidCurvedF1 = curveF2.id();
  QCOMPARE( mLayerLineCurvedOffset->featureCount(), ( long )1 );

  // just one added feature
  QCOMPARE( mLayerLineCurvedOffset->undoStack()->index(), 1 );

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
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerLineCurved << mLayerLineCurvedOffset << mLayerLineZ << mLayerLine2D << mLayerSelfSnapLine );
  mCanvas->setSnappingUtils( new QgsMapCanvasSnappingUtils( mCanvas, this ) );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );

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

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::RightButton );

  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LineString ((1 1, 3 2))" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::CircularString );

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::RightButton );

  // Circular string need 3 points, so no feature created
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( utils.existingFeatureIds().count(), 1 );

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseMove( 5, 5 );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseMove( 5, 5 );
  utils.mouseClick( 4, 2, Qt::LeftButton );

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );

  utils.mouseMove( 5, 5 );
  utils.mouseClick( 4, 3, Qt::LeftButton );

  utils.mouseMove( 5, 5 );
  utils.mouseClick( 4, 4, Qt::RightButton );

  newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  // as here, QCOMPARE with QgsGeometry uses isGeosEqual() and geos does not support curve (need to convert curve to line string), test with wkt string
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry().asWkt(), QStringLiteral( "CompoundCurve (CircularString (1 1, 3 2, 4 2),(4 2, 4 3))" ) ) ;

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
}

void TestQgsMapToolAddFeatureLine::testTracing()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  // tracing enabled - same clicks - now following line

  mEnableTracingAction->setChecked( true );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );

  // be sure it doesn't create an extra curve
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((1 1, 2 1, 3 2))" ) );

  utils.mouseClick( 3, 2, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LineString(1 1, 2 1, 3 2)" ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // tracing enabled - combined with first and last segments that are not traced

  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::RightButton );

  const QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid2 ).geometry(), QgsGeometry::fromWkt( "LineString(0 2, 1 1, 2 1, 3 2, 4 1)" ) );

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

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 2, 1, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );

  const QgsGeometry g = mLayerLine->getFeature( newFid ).geometry();
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

  const QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  const QgsGeometry g2 = mLayerLine->getFeature( newFid2 ).geometry();
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

  const QgsFeatureId newFid3 = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  const QgsGeometry g3 = mLayerLine->getFeature( newFid3 ).geometry();
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

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  // tracing enabled - without converting to curves
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), false );

  utils.mouseClick( 6, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::LeftButton );
  utils.mouseClick( 7, 1, Qt::RightButton );

  const QgsFeatureId newFid1 = utils.newFeatureId( oldFids );

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

  const QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  g = mLayerLineCurved->getFeature( newFid2 ).geometry().constGet();
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( 6, 1 ) );
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, g->vertexCount() - 1 ) ), QgsPoint( 7, 1 ) );
  QVERIFY( g->vertexCount() == 3 );  // a true arc is composed of 3 vertices

  mLayerLineCurved->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLineCurved->undoStack()->index(), 1 );

  mEnableTracingAction->setChecked( false );
}


void TestQgsMapToolAddFeatureLine::testTracingWithConvertToCurvesCustomTolerance()
{
  // Exactly the same as testTracingWithConvertToCurves but far from the origin
  // At this distance, the arcs aren't correctly detected with the default tolerance
  const double offset = 100000000; // remember to change the feature geometry accordingly in initTestCase (sic)

  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve_angle_tolerance" ), 1e-5 );
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve_distance_tolerance" ), 1e-5 );

  mCanvas->setExtent( QgsRectangle( offset + 0, offset + 0, offset + 8, offset + 8 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( offset + 0, offset + 0, offset + 8, offset + 8 ) );

  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineCurvedOffset );

  // enable snapping and tracing
  mEnableTracingAction->setChecked( true );

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  // tracing enabled - without converting to curves
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), false );

  utils.mouseClick( offset + 6, offset + 1, Qt::LeftButton );
  utils.mouseClick( offset + 7, offset + 1, Qt::LeftButton );
  utils.mouseClick( offset + 7, offset + 1, Qt::RightButton );

  const QgsFeatureId newFid1 = utils.newFeatureId( oldFids );

  const QgsAbstractGeometry *g = mLayerLineCurvedOffset->getFeature( newFid1 ).geometry().constGet();
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( offset + 6, offset + 1 ) );
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, g->vertexCount() - 1 ) ), QgsPoint( offset + 7, offset + 1 ) );
  QVERIFY( g->vertexCount() > 3 );  // a segmentized arc has (much) more than 3 points

  mLayerLineCurvedOffset->undoStack()->undo();

  // we redo the same with convert to curves enabled
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/convert_to_curve" ), true );

  // tracing enabled - without converting to curves
  utils.mouseClick( offset + 6, offset + 1, Qt::LeftButton );
  utils.mouseClick( offset + 7, offset + 1, Qt::LeftButton );
  utils.mouseClick( offset + 7, offset + 1, Qt::RightButton );

  const QgsFeatureId newFid2 = utils.newFeatureId( oldFids );

  g = mLayerLineCurvedOffset->getFeature( newFid2 ).geometry().constGet();
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, 0 ) ), QgsPoint( offset + 6, offset + 1 ) );
  QCOMPARE( g->vertexAt( QgsVertexId( 0, 0, g->vertexCount() - 1 ) ), QgsPoint( offset + 7, offset + 1 ) );
  QVERIFY( g->vertexCount() == 3 );  // a true arc is composed of 3 vertices

  mLayerLineCurvedOffset->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLineCurvedOffset->undoStack()->index(), 1 );

  mEnableTracingAction->setChecked( false );

  // restore the extent
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

}

void TestQgsMapToolAddFeatureLine::testCloseLine()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerCloseLine );
  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 5, 1, Qt::LeftButton );
  utils.mouseClick( 5, 5, Qt::LeftButton );
  utils.mouseClick( 5, 5, Qt::RightButton, Qt::ShiftModifier );
  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  const QString wkt = "LineString (1 1, 5 1, 5 5, 1 1)";
  QCOMPARE( mLayerCloseLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerCloseLine->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testSelfSnapping()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerSelfSnapLine );

  const QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( true );
  cfg.setMode( Qgis::SnappingMode::AllLayers );
  cfg.setTypeFlag( Qgis::SnappingType::Vertex );
  cfg.setTolerance( 50 );
  cfg.setUnits( QgsTolerance::Pixels );
  mCanvas->snappingUtils()->setConfig( cfg );


  const QString targetWkt = "LineString (2 5, 3 5, 3 6, 2 5)";

  // Without self snapping, endpoint won't snap to start point
  cfg.setSelfSnapping( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  utils.mouseClick( 2, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 5, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 3, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  utils.mouseClick( 2, 5.1, Qt::RightButton );

  const QgsFeatureId newFid1 = utils.newFeatureId( oldFids );
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

  const QgsFeatureId newFid2 = utils.newFeatureId( oldFids );
  QCOMPARE( mLayerSelfSnapLine->getFeature( newFid2 ).geometry(), QgsGeometry::fromWkt( targetWkt ) );
  mLayerSelfSnapLine->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testLineString()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 5, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.5, 6.5, Qt::LeftButton );

  // check capture curve initially
  QCOMPARE( mCaptureTool->captureCurve()->asWkt(), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5, 7.25 6.5, 7.5 6.5))" ) );

  utils.mouseClick( 8, 6.5, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  const QString wkt = "LineString(5 6.5, 6.25 6.5, 6.75 6.5, 7.25 6.5, 7.5 6.5)";
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testCompoundCurve()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLineCurved );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 5, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::CircularString );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.25, 6.5, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utils.mouseClick( 7.5, 6.5, Qt::LeftButton );
  utils.mouseClick( 7.5, 6.0, Qt::LeftButton );
  utils.mouseClick( 7.7, 6.0, Qt::LeftButton );

  // check capture curve initially
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5),CircularString (6.25 6.5, 6.75 6.5, 7.25 6.5),(7.25 6.5, 7.5 6.5, 7.5 6, 7.7 6))" ) );

  utils.mouseClick( 8, 6.5, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  const QString wkt = "CompoundCurve ((5 6.5, 6.25 6.5),CircularString (6.25 6.5, 6.75 6.5, 7.25 6.5),(7.25 6.5, 7.5 6.5, 7.5 6, 7.703125 6))";
  QCOMPARE( mLayerLineCurved->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLineCurved->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testStream()
{
  // test streaming mode digitizing
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utils.mouseClick( 5, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.0, 6.6 );
  utils.mouseMove( 7.1, 6.7 );
  utils.mouseMove( 7.2, 6.6 );
  utils.mouseMove( 7.3, 6.5 );
  utils.mouseMove( 7.5, 6.9 );
  utils.mouseMove( 7.6, 6.3 );
  utils.mouseClick( 7.75, 6.5, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utils.mouseClick( 7.5, 5.0, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.4, 5.0 );
  utils.mouseMove( 7.3, 5.1 );
  utils.mouseMove( 7.2, 5.0 );
  utils.mouseMove( 7.1, 4.9 );

  // check capture curve initially -- the streamed sections MUST become their own curve in the geometry, and not be compined with the straight line segments!
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.41 5, 7.3 5.09, 7.2 5, 7.09 4.91))" ) );
  utils.mouseClick( 7.0, 5.0, Qt::RightButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  const QString wkt = "LineString (5 6.5, 6.25 6.5, 6.75 6.5, 7 6.59375, 7.09375 6.703125, 7.203125 6.59375, 7.296875 6.5, 7.5 6.90625, 7.59375 6.296875, 7.5 5, 7.40625 5, 7.296875 5.09375, 7.203125 5, 7.09375 4.90625)";
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();
}

void TestQgsMapToolAddFeatureLine::testUndo()
{
  // test undo logic
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 5, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.0, 6.6 );
  utils.mouseMove( 7.1, 6.7 );
  utils.mouseMove( 7.2, 6.6 );
  utils.mouseMove( 7.3, 6.5 );
  utils.mouseMove( 7.5, 6.9 );
  utils.mouseMove( 7.6, 6.3 );
  utils.mouseClick( 7.75, 6.5, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utils.mouseClick( 7.5, 5.0, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.4, 5.0 );
  utils.mouseMove( 7.3, 5.1 );
  utils.mouseMove( 7.2, 5.0 );
  utils.mouseMove( 7.1, 4.9 );

  // check capture curve initially -- the streamed sections MUST become their own curve in the geometry, and not be compined with the straight line segments!
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.41 5, 7.3 5.09, 7.2 5, 7.09 4.91))" ) );

  // now lets try undoing...
  utils.keyClick( Qt::Key_Backspace );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.41 5, 7.3 5.09, 7.2 5))" ) );
  // simulate auto repeating undo from a held-down backspace key
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.41 5, 7.3 5.09))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.41 5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  // we've now finished undoing the streamed digitizing section, so undo should pause until the user releases the backspace key and then re-presses it
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), false );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91, 7.59 6.3))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), false );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5, 7.5 6.91))" ) );
  // simulate holding down another key
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59, 7.3 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7, 7.2 6.59))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.09 6.7))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  // should get "stuck" here again
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5))" ) );
  // release and repress
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), false );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5))" ) );
  utils.keyClick( Qt::Key_Backspace, Qt::KeyboardModifiers(), true );

  utils.mouseClick( 7.0, 5.0, Qt::RightButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
}

void TestQgsMapToolAddFeatureLine::testStreamTolerance()
{
  // test streaming mode digitizing with tolerance
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/qgis/digitizing/stream_tolerance" ), 10 );

  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  mCanvas->setCurrentLayer( mLayerLine );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 5, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.25, 6.5, Qt::LeftButton );
  utils.mouseClick( 6.75, 6.5, Qt::LeftButton );

  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.0, 6.6 );
  utils.mouseMove( 7.1, 6.7 );
  utils.mouseMove( 7.2, 6.6 );
  utils.mouseMove( 7.3, 6.5 );
  utils.mouseMove( 7.5, 6.9 );
  utils.mouseMove( 7.6, 6.3 );
  utils.mouseClick( 7.75, 6.5, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utils.mouseClick( 7.5, 5.0, Qt::LeftButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Streaming );
  utils.mouseMove( 7.4, 5.0 );
  utils.mouseMove( 7.3, 5.1 );
  utils.mouseMove( 7.2, 5.0 );
  utils.mouseMove( 7.1, 4.9 );

  // check capture curve initially -- the streamed sections MUST become their own curve in the geometry, and not be compined with the straight line segments!
  QCOMPARE( mCaptureTool->captureCurve()->asWkt( 2 ), QStringLiteral( "CompoundCurve ((5 6.5, 6.25 6.5, 6.75 6.5),(6.75 6.5, 7 6.59, 7.2 6.59, 7.5 6.91, 7.59 6.3),(7.59 6.3, 7.5 5),(7.5 5, 7.3 5.09, 7.09 4.91))" ) );
  utils.mouseClick( 7.0, 5.0, Qt::RightButton );
  mCaptureTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );

  const QgsFeatureId newFid = utils.newFeatureId( oldFids );

  const QString wkt = "LineString(5 6.5, 6.25 6.5, 6.75 6.5, 7 6.59375, 7.203125 6.59375, 7.5 6.90625, 7.59375 6.296875, 7.5 5, 7.296875 5.09375, 7.09375 4.90625)";
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( wkt ) );

  mLayerLine->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolAddFeatureLine )
#include "testqgsmaptooladdfeatureline.moc"
