/***************************************************************************
     testqgsvertextool.cpp
     ----------------------
    Date                 : 2017-03-01
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

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "vertextool/qgsvertextool.h"
#include "qgslinestring.h"
#include "qgssnappingconfig.h"
#include "qgssettings.h"
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
    QByteArray ba = geom.asWkt().toAscii();
    return qstrdup( ba.data() );
  }
}

/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsVertexTool : public QObject
{
    Q_OBJECT
  public:
    TestQgsVertexTool();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testTopologicalEditingMoveVertexZ();
    void testTopologicalEditingMoveVertexOnSegmentZ();
    void testMoveVertex();
    void testMoveEdge();
    void testAddVertex();
    void testAddVertexAtEndpoint();
    void testAddVertexDoubleClick();
    void testAddVertexDoubleClickWithShift();
    void testDeleteVertex();
    void testMoveMultipleVertices();
    void testMoveMultipleVertices2();
    void testMoveVertexTopo();
    void testDeleteVertexTopo();
    void testAddVertexTopo();
    void testMoveEdgeTopo();
    void testAddVertexTopoFirstSegment();
    void testActiveLayerPriority();
    void testSelectedFeaturesPriority();

  private:
    QPoint mapToScreen( double mapX, double mapY )
    {
      QgsPointXY pt = mCanvas->mapSettings().mapToPixel().transform( mapX, mapY );
      return QPoint( std::round( pt.x() ), std::round( pt.y() ) );
    }

    void mouseMove( double mapX, double mapY )
    {
      QgsMapMouseEvent e( mCanvas, QEvent::MouseMove, mapToScreen( mapX, mapY ) );
      mVertexTool->cadCanvasMoveEvent( &e );
    }

    void mousePress( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      QgsMapMouseEvent e1( mCanvas, QEvent::MouseButtonPress, mapToScreen( mapX, mapY ), button, button, stateKey );
      if ( snap )
        e1.snapPoint();
      mVertexTool->cadCanvasPressEvent( &e1 );
    }

    void mouseRelease( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      QgsMapMouseEvent e2( mCanvas, QEvent::MouseButtonRelease, mapToScreen( mapX, mapY ), button, Qt::MouseButton(), stateKey );
      if ( snap )
        e2.snapPoint();
      mVertexTool->cadCanvasReleaseEvent( &e2 );
    }

    void mouseClick( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      mousePress( mapX, mapY, button, stateKey, snap );
      mouseRelease( mapX, mapY, button, stateKey, snap );
    }

    void mouseDoubleClick( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers(), bool snap = false )
    {
      // this is how Qt passes the events: 1. mouse press, 2. mouse release, 3. mouse double-click, 4. mouse release

      mouseClick( mapX, mapY, button, stateKey, snap );

      QgsMapMouseEvent e( mCanvas, QEvent::MouseButtonDblClick, mapToScreen( mapX, mapY ), button, button, stateKey );
      if ( snap )
        e.snapPoint();
      mVertexTool->canvasDoubleClickEvent( &e );

      mouseRelease( mapX, mapY, button, stateKey );
    }

    void keyClick( int key )
    {
      QKeyEvent e1( QEvent::KeyPress, key, Qt::KeyboardModifiers() );
      mVertexTool->keyPressEvent( &e1 );

      QKeyEvent e2( QEvent::KeyRelease, key, Qt::KeyboardModifiers() );
      mVertexTool->keyReleaseEvent( &e2 );
    }

  private:
    QgsMapCanvas *mCanvas = nullptr;
    QgisApp *mQgisApp = nullptr;
    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;
    QgsVertexTool *mVertexTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsVectorLayer *mLayerPolygon = nullptr;
    QgsVectorLayer *mLayerPoint = nullptr;
    QgsVectorLayer *mLayerLineZ = nullptr;
    QgsFeatureId mFidLineZF1 = 0;
    QgsFeatureId mFidLineZF2 = 0;
    QgsFeatureId mFidLineF1 = 0;
    QgsFeatureId mFidPolygonF1 = 0;
    QgsFeatureId mFidPointF1 = 0;
};

TestQgsVertexTool::TestQgsVertexTool() = default;


//runs before all tests
void TestQgsVertexTool::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  mAdvancedDigitizingDockWidget = new QgsAdvancedDigitizingDockWidget( mCanvas );

  // make testing layers
  mLayerLine = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLine->isValid() );
  mLayerPolygon = new QgsVectorLayer( QStringLiteral( "Polygon?crs=EPSG:27700" ), QStringLiteral( "layer polygon" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPolygon->isValid() );
  mLayerPoint = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:27700" ), QStringLiteral( "layer point" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPoint->isValid() );
  mLayerLineZ = new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerLineZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerPolygon << mLayerPoint << mLayerLineZ );

  QgsPolylineXY line1;
  line1 << QgsPointXY( 2, 1 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 1, 3 );
  QgsFeature lineF1;
  lineF1.setGeometry( QgsGeometry::fromPolylineXY( line1 ) );

  QgsPolygonXY polygon1;
  QgsPolylineXY polygon1exterior;
  polygon1exterior << QgsPointXY( 4, 1 ) << QgsPointXY( 7, 1 ) << QgsPointXY( 7, 4 ) << QgsPointXY( 4, 4 ) << QgsPointXY( 4, 1 );
  polygon1 << polygon1exterior;
  QgsFeature polygonF1;
  polygonF1.setGeometry( QgsGeometry::fromPolygonXY( polygon1 ) );

  QgsFeature pointF1;
  pointF1.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( 2, 3 ) ) );

  QgsFeature linez1, linez2;
  QVector<QgsPoint> linez1pts, linez2pts;
  linez1pts << QgsPoint( 5, 5, 1 ) << QgsPoint( 6, 6, 1 ) << QgsPoint( 7, 5, 1 );
  linez2pts << QgsPoint( 5, 7, 5 ) << QgsPoint( 7, 7, 10 );
  QgsLineString linez1geom( linez1pts );
  QgsLineString linez2geom( linez2pts );
  linez1.setGeometry( std::unique_ptr< QgsAbstractGeometry >( linez1geom.clone() ) );
  linez2.setGeometry( std::unique_ptr< QgsAbstractGeometry >( linez2geom.clone() ) );

  mLayerLine->startEditing();
  mLayerLine->addFeature( lineF1 );
  mFidLineF1 = lineF1.id();
  QCOMPARE( mLayerLine->featureCount(), ( long )1 );

  mLayerPolygon->startEditing();
  mLayerPolygon->addFeature( polygonF1 );
  mFidPolygonF1 = polygonF1.id();
  QCOMPARE( mLayerPolygon->featureCount(), ( long )1 );

  mLayerPoint->startEditing();
  mLayerPoint->addFeature( pointF1 );
  mFidPointF1 = pointF1.id();
  QCOMPARE( mLayerPoint->featureCount(), ( long )1 );

  mLayerLineZ->startEditing();
  mLayerLineZ->addFeature( linez1 );
  mLayerLineZ->addFeature( linez2 );
  mFidLineZF1 = linez1.id();
  mFidLineZF2 = linez2.id();
  QCOMPARE( mLayerLineZ->featureCount(), ( long ) 2 );

  // just one added feature in each undo stack
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );
  // except for layerLineZ
  QCOMPARE( mLayerLineZ->undoStack()->index(), 2 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerPolygon << mLayerPoint << mLayerLineZ );

  QgsMapCanvasSnappingUtils *snappingUtils = new QgsMapCanvasSnappingUtils( mCanvas, this );
  mCanvas->setSnappingUtils( snappingUtils );

  snappingUtils->locatorForLayer( mLayerLine )->init();
  snappingUtils->locatorForLayer( mLayerPolygon )->init();
  snappingUtils->locatorForLayer( mLayerPoint )->init();
  snappingUtils->locatorForLayer( mLayerLineZ )->init();

  // create vertex tool
  mVertexTool = new QgsVertexTool( mCanvas, mAdvancedDigitizingDockWidget );

  mCanvas->setMapTool( mVertexTool );
}

//runs after all tests
void TestQgsVertexTool::cleanupTestCase()
{
  delete mVertexTool;
  delete mAdvancedDigitizingDockWidget;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsVertexTool::testTopologicalEditingMoveVertexZ()
{
  bool topologicalEditing = QgsProject::instance()->topologicalEditing();
  QgsProject::instance()->setTopologicalEditing( true );
  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setTolerance( 10 );
  cfg.setType( QgsSnappingConfig::VertexAndSegment );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mouseClick( 5, 7, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerLineZ->getFeature( mFidLineZF1 ).geometry().asWkt(), QString( "LineStringZ (5 5 1, 5 7 5, 7 5 1)" ) );
  QCOMPARE( mLayerLineZ->getFeature( mFidLineZF2 ).geometry().asWkt(), QString( "LineStringZ (5 7 5, 7 7 10)" ) );

  QgsProject::instance()->setTopologicalEditing( topologicalEditing );
  mLayerLineZ->undoStack()->undo();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
}

void TestQgsVertexTool::testTopologicalEditingMoveVertexOnSegmentZ()
{
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );

  bool topologicalEditing = QgsProject::instance()->topologicalEditing();
  QgsProject::instance()->setTopologicalEditing( true );
  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setTolerance( 10 );
  cfg.setType( QgsSnappingConfig::VertexAndSegment );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  mouseClick( 6, 7, Qt::LeftButton, Qt::KeyboardModifiers(), true );

  QCOMPARE( mLayerLineZ->getFeature( mFidLineZF1 ).geometry().asWkt(), QString( "LineStringZ (5 5 1, 6 7 1, 7 5 1)" ) );
  QCOMPARE( mLayerLineZ->getFeature( mFidLineZF2 ).geometry().asWkt(), QString( "LineStringZ (5 7 5, 6 7 333, 7 7 10)" ) );

  QgsProject::instance()->setTopologicalEditing( topologicalEditing );
  mLayerLineZ->undoStack()->undo();
  cfg.setEnabled( false );
  mCanvas->snappingUtils()->setConfig( cfg );
}

void TestQgsVertexTool::testMoveVertex()
{
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  // move vertex of linestring

  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 2, 2, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 2, 1 1, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 0.5, 0.5, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 0.5 0.5, 1 3)" ) );

  mLayerLine->undoStack()->undo();

  // move point

  mouseClick( 2, 3, Qt::LeftButton );
  mouseClick( 1, 4, Qt::LeftButton );

  QCOMPARE( mLayerPoint->undoStack()->index(), 2 );
  QCOMPARE( mLayerPoint->getFeature( mFidPointF1 ).geometry(), QgsGeometry::fromWkt( "POINT(1 4)" ) );

  mLayerPoint->undoStack()->undo();

  QCOMPARE( mLayerPoint->getFeature( mFidPointF1 ).geometry(), QgsGeometry::fromWkt( "POINT(2 3)" ) );

  // move vertex of polygon

  mouseClick( 4, 1, Qt::LeftButton );
  mouseClick( 5, 2, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((5 2, 7 1, 7 4, 4 4, 5 2))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  mouseClick( 4, 4, Qt::LeftButton );
  mouseClick( 5, 5, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 5 5, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // cancel moving of a linestring with right mouse button
  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // clicks somewhere away from features - should do nothing
  mouseClick( 2, 2, Qt::LeftButton );
  mouseClick( 2, 4, Qt::LeftButton );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );
}

void TestQgsVertexTool::testMoveEdge()
{
  // move edge of linestring

  mouseClick( 1.2, 1, Qt::LeftButton );
  mouseClick( 1.2, 2, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 2, 1 2, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // move edge of polygon

  mouseClick( 5, 1, Qt::LeftButton );
  mouseClick( 6, 1, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((5 1, 8 1, 7 4, 4 4, 5 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );
}


void TestQgsVertexTool::testAddVertex()
{
  // add vertex in linestring

  mouseClick( 1.5, 1, Qt::LeftButton );
  mouseClick( 1.5, 2, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1.5 2, 1 1, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // add vertex in polygon

  mouseClick( 4, 2.5, Qt::LeftButton );
  mouseClick( 3, 2.5, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 3 2.5, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );
}


void TestQgsVertexTool::testAddVertexAtEndpoint()
{
  // offset of the endpoint marker - currently set as 15px away from the last vertex in direction of the line
  double offsetInMapUnits = 15 * mCanvas->mapSettings().mapUnitsPerPixel();

  // add vertex at the end

  mouseMove( 1, 3 ); // first we need to move to the vertex
  mouseClick( 1, 3 + offsetInMapUnits, Qt::LeftButton );
  mouseClick( 2, 3, Qt::LeftButton );
  mouseClick( 2, 3, Qt::RightButton ); // we need a right click to stop adding new nodes

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3, 2 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // add vertex at the start

  mouseMove( 2, 1 ); // first we need to move to the vertex
  mouseClick( 2 + offsetInMapUnits, 1, Qt::LeftButton );
  mouseClick( 2, 2, Qt::LeftButton );
  mouseClick( 2, 2, Qt::RightButton ); // we need a right click to stop adding new nodes

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 2, 2 1, 1 1, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // add three vertices at once

  mouseMove( 2, 1 ); // first we need to move to the vertex
  mouseClick( 2 + offsetInMapUnits, 1, Qt::LeftButton );
  mouseClick( 2, 2, Qt::LeftButton );
  mouseClick( 2, 3, Qt::LeftButton );
  mouseClick( 2, 4, Qt::LeftButton );
  mouseClick( 2, 2, Qt::RightButton ); // we need a right click to stop adding new nodes

  QCOMPARE( mLayerLine->undoStack()->index(), 4 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 4, 2 3, 2 2, 2 1, 1 1, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  mLayerLine->undoStack()->undo();
  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
}

void TestQgsVertexTool::testAddVertexDoubleClick()
{
  // add vertex in linestring with double-click and then place the point to the new location

  mouseDoubleClick( 1, 1.5, Qt::LeftButton );
  mouseClick( 2, 2, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 2 2, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // add vertex in polygon
  mouseDoubleClick( 4, 2, Qt::LeftButton );
  mouseClick( 3, 2.5, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 3 2.5, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );

}

void TestQgsVertexTool::testAddVertexDoubleClickWithShift()
{
  // add vertex in linestring with shift + double-click to immediately place the new vertex

  mouseDoubleClick( 1, 1.5, Qt::LeftButton, Qt::ShiftModifier );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 1.5, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // add vertex in polygon
  mouseDoubleClick( 4, 2, Qt::LeftButton, Qt::ShiftModifier );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 2, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );

}


void TestQgsVertexTool::testDeleteVertex()
{
  // delete vertex in linestring

  mouseClick( 1, 1, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // delete vertex in polygon

  mouseClick( 7, 4, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 4 4, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  // delete vertex in point - deleting its geometry

  mouseClick( 2, 3, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  QCOMPARE( mLayerPoint->undoStack()->index(), 2 );
  QCOMPARE( mLayerPoint->getFeature( mFidPointF1 ).geometry(), QgsGeometry() );

  mLayerPoint->undoStack()->undo();

  QCOMPARE( mLayerPoint->getFeature( mFidPointF1 ).geometry(), QgsGeometry::fromWkt( "POINT(2 3)" ) );

  // delete a vertex by dragging a selection rect

  mousePress( 0.5, 2.5, Qt::LeftButton );
  mouseMove( 1.5, 3.5 );
  mouseRelease( 1.5, 3.5, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 1 );
  QCOMPARE( mLayerPoint->undoStack()->index(), 1 );
}

void TestQgsVertexTool::testMoveMultipleVertices()
{
  // select two vertices
  mousePress( 0.5, 0.5, Qt::LeftButton );
  mouseMove( 1.5, 3.5 );
  mouseRelease( 1.5, 3.5, Qt::LeftButton );

  // move them by -1,-1
  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 0, 0, Qt::LeftButton );

  // extra click away from everything to clear the selection
  mouseClick( 8, 8, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 0 0, 0 2)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
}

void TestQgsVertexTool::testMoveMultipleVertices2()
{
  // this time select two vertices with shift
  mouseClick( 1, 1, Qt::LeftButton, Qt::ShiftModifier );
  mouseClick( 2, 1, Qt::LeftButton, Qt::ShiftModifier );

  // move them by +1, +1
  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 2, 2, Qt::LeftButton );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(3 2, 2 2, 1 3)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
}

void TestQgsVertexTool::testMoveVertexTopo()
{
  // test moving of vertices of two features at once

  QgsProject::instance()->setTopologicalEditing( true );

  // connect linestring with polygon at point (2, 1)
  mouseClick( 4, 1, Qt::LeftButton );
  mouseClick( 2, 1, Qt::LeftButton );

  // move shared vertex of linestring and polygon
  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 3, 3, Qt::LeftButton );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(3 3, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((3 3, 7 1, 7 4, 4 4, 3 3))" ) );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 3 );  // one more move of vertex from earlier
  mLayerLine->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsVertexTool::testDeleteVertexTopo()
{
  // test deletion of vertices with topological editing enabled

  QgsProject::instance()->setTopologicalEditing( true );

  // connect linestring with polygon at point (2, 1)
  mouseClick( 4, 1, Qt::LeftButton );
  mouseClick( 2, 1, Qt::LeftButton );

  // move shared vertex of linestring and polygon
  mouseClick( 2, 1, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((7 1, 7 4, 4 4, 7 1))" ) );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerPolygon->undoStack()->index(), 3 );  // one more move of vertex from earlier
  mLayerLine->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsVertexTool::testAddVertexTopo()
{
  // test addition of a vertex on a segment shared with another geometry

  // add a temporary polygon
  QgsFeature fTmp;
  fTmp.setGeometry( QgsGeometry::fromWkt( "POLYGON((4 4, 7 4, 7 6, 4 6, 4 4))" ) );
  bool resAdd = mLayerPolygon->addFeature( fTmp );
  QVERIFY( resAdd );
  QgsFeatureId fTmpId = fTmp.id();

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );

  QgsProject::instance()->setTopologicalEditing( true );

  mouseClick( 5.5, 4, Qt::LeftButton );
  mouseClick( 5, 5, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 3 );

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 5 5, 4 4, 4 1))" ) );
  QCOMPARE( mLayerPolygon->getFeature( fTmpId ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 4, 5 5, 7 4, 7 6, 4 6, 4 4))" ) );

  mLayerPolygon->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsVertexTool::testMoveEdgeTopo()
{
  // test move of an edge shared with another feature

  // add a temporary polygon
  QgsFeature fTmp;
  fTmp.setGeometry( QgsGeometry::fromWkt( "POLYGON((4 4, 7 4, 7 6, 4 6, 4 4))" ) );
  bool resAdd = mLayerPolygon->addFeature( fTmp );
  QVERIFY( resAdd );
  QgsFeatureId fTmpId = fTmp.id();

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );

  QgsProject::instance()->setTopologicalEditing( true );

  // move shared segment
  mouseClick( 6, 4, Qt::LeftButton );
  mouseClick( 6, 5, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 3 );

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 5, 4 5, 4 1))" ) );
  QCOMPARE( mLayerPolygon->getFeature( fTmpId ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 5, 7 5, 7 6, 4 6, 4 5))" ) );

  mLayerPolygon->undoStack()->undo();

  // another test to move a shared segment - but this time we just pick two individual points of a feature
  // and do vertex move

  QgsProject::instance()->setTopologicalEditing( false );

  // this time select two vertices with shift
  mouseClick( 4, 4, Qt::LeftButton, Qt::ShiftModifier );
  mouseClick( 7, 4, Qt::LeftButton, Qt::ShiftModifier );

  QgsProject::instance()->setTopologicalEditing( true );

  // now move the shared segment
  mouseClick( 4, 4, Qt::LeftButton );
  mouseClick( 4, 3, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 3 );

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 3, 4 3, 4 1))" ) );
  QCOMPARE( mLayerPolygon->getFeature( fTmpId ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 3, 7 3, 7 6, 4 6, 4 3))" ) );

  mLayerPolygon->undoStack()->undo();

  //

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsVertexTool::testAddVertexTopoFirstSegment()
{
  // check that when adding a vertex to the first segment of a polygon's ring with topo editing
  // enabled, the geometry does not get corrupted (#20774)

  QgsProject::instance()->setTopologicalEditing( true );

  mouseClick( 5.5, 1, Qt::LeftButton );
  mouseClick( 5, 2, Qt::LeftButton );

  QCOMPARE( mLayerPolygon->undoStack()->index(), 2 );

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 5 2, 7 1, 7 4, 4 4, 4 1))" ) );

  mLayerPolygon->undoStack()->undo();

  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((4 1, 7 1, 7 4, 4 4, 4 1))" ) );

  QgsProject::instance()->setTopologicalEditing( false );
}

void TestQgsVertexTool::testActiveLayerPriority()
{
  // check that features from current layer get priority when picking points

  // create a temporary line layer that has a common vertex with existing line layer at (1, 1)
  QgsVectorLayer *layerLine2 = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line 2" ), QStringLiteral( "memory" ) );
  QVERIFY( layerLine2->isValid() );
  QgsPolylineXY line1;
  line1 << QgsPointXY( 0, 1 ) << QgsPointXY( 1, 1 ) << QgsPointXY( 1, 0 );
  QgsFeature lineF1;
  lineF1.setGeometry( QgsGeometry::fromPolylineXY( line1 ) );
  layerLine2->startEditing();
  layerLine2->addFeature( lineF1 );
  QgsFeatureId fidLineF1 = lineF1.id();
  QCOMPARE( layerLine2->featureCount(), ( long )1 );
  QgsProject::instance()->addMapLayer( layerLine2 );
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerPolygon << mLayerPoint << layerLine2 );

  // make one layer active and check its vertex is used

  mCanvas->snappingUtils()->locatorForLayer( layerLine2 )->init();

  mCanvas->setCurrentLayer( mLayerLine );

  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 0, 0, Qt::LeftButton );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 0 0, 1 3)" ) );
  QCOMPARE( layerLine2->getFeature( fidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(0 1, 1 1, 1 0)" ) );
  mLayerLine->undoStack()->undo();

  // make the other layer active and check its vertex is used

  mCanvas->setCurrentLayer( layerLine2 );

  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 0, 0, Qt::LeftButton );

  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
  QCOMPARE( layerLine2->getFeature( fidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(0 1, 0 0, 1 0)" ) );
  layerLine2->undoStack()->undo();

  mCanvas->setCurrentLayer( nullptr );

  // get rid of the temporary layer
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine << mLayerPolygon << mLayerPoint );
  QgsProject::instance()->removeMapLayer( layerLine2 );
}

void TestQgsVertexTool::testSelectedFeaturesPriority()
{
  // preparation: make the polygon feature touch line feature
  mouseClick( 4, 1, Qt::LeftButton );
  mouseClick( 2, 1, Qt::LeftButton );

  //
  // test that clicking a location with selected and non-selected feature will always pick the selected feature
  //

  mLayerLine->selectByIds( QgsFeatureIds() << mFidLineF1 );
  mLayerPolygon->selectByIds( QgsFeatureIds() );

  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 3, 1, Qt::LeftButton );

  // check that move of (2,1) to (3,1) affects only line layer
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(3 1, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((2 1, 7 1, 7 4, 4 4, 2 1))" ) );
  mLayerLine->undoStack()->undo();

  mLayerLine->selectByIds( QgsFeatureIds() );
  mLayerPolygon->selectByIds( QgsFeatureIds() << mFidPolygonF1 );

  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 3, 1, Qt::LeftButton );

  // check that move of (2,1) to (3,1) affects only polygon layer
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((3 1, 7 1, 7 4, 4 4, 3 1))" ) );
  mLayerPolygon->undoStack()->undo();

  //
  // test that dragging rectangle to pick vertices in location with selected and non-selected feature
  // will always pick vertices only from the selected feature
  //

  mLayerLine->selectByIds( QgsFeatureIds() );
  mLayerPolygon->selectByIds( QgsFeatureIds() );

  mousePress( 1.5, 0.5, Qt::LeftButton );
  mouseMove( 2.5, 1.5 );
  mouseRelease( 2.5, 1.5, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  // check we have deleted vertex at (2,1) from both line and polygon features
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((7 1, 7 4, 4 4, 7 1))" ) );
  mLayerLine->undoStack()->undo();
  mLayerPolygon->undoStack()->undo();

  mLayerLine->selectByIds( QgsFeatureIds() << mFidLineF1 );
  mLayerPolygon->selectByIds( QgsFeatureIds() );

  mousePress( 1.5, 0.5, Qt::LeftButton );
  mouseMove( 2.5, 1.5 );
  mouseRelease( 2.5, 1.5, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  // check we have deleted vertex at (2,1) just from line feature
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((2 1, 7 1, 7 4, 4 4, 2 1))" ) );
  mLayerLine->undoStack()->undo();

  mLayerLine->selectByIds( QgsFeatureIds() );
  mLayerPolygon->selectByIds( QgsFeatureIds() << mFidPolygonF1 );

  mousePress( 1.5, 0.5, Qt::LeftButton );
  mouseMove( 2.5, 1.5 );
  mouseRelease( 2.5, 1.5, Qt::LeftButton );
  keyClick( Qt::Key_Delete );

  // check we have deleted vertex at (2,1) just from polygon feature
  QCOMPARE( mLayerLine->getFeature( mFidLineF1 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(2 1, 1 1, 1 3)" ) );
  QCOMPARE( mLayerPolygon->getFeature( mFidPolygonF1 ).geometry(), QgsGeometry::fromWkt( "POLYGON((7 1, 7 4, 4 4, 7 1))" ) );
  mLayerPolygon->undoStack()->undo();

  mLayerPolygon->undoStack()->undo();  // undo the initial change
}

QGSTEST_MAIN( TestQgsVertexTool )
#include "testqgsvertextool.moc"
