/***************************************************************************
     testqgsmaptooladdfeature.cpp
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
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvastracer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"


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

static QSet<QgsFeatureId> _existingFeatureIds( QgsVectorLayer *layer )
{
  QSet<QgsFeatureId> fids;
  QgsFeature f;
  QgsFeatureIterator it = layer->getFeatures();
  while ( it.nextFeature( f ) )
    fids << f.id();
  return fids;
}

static QgsFeatureId _newFeatureId( QgsVectorLayer *layer, QSet<QgsFeatureId> oldFids )
{
  QSet<QgsFeatureId> newFids = _existingFeatureIds( layer );
  QSet<QgsFeatureId> diffFids = newFids.subtract( oldFids );
  Q_ASSERT( diffFids.count() == 1 );
  return *diffFids.constBegin();
}



/**
 * \ingroup UnitTests
 * This is a unit test for the node tool
 */
class TestQgsMapToolAddFeature : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeature();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testNoTracing();
    void testTracing();
    void testTracingWithOffset();

  private:
    QPoint mapToScreen( double mapX, double mapY )
    {
      QgsPointXY pt = mCanvas->mapSettings().mapToPixel().transform( mapX, mapY );
      return QPoint( std::round( pt.x() ), std::round( pt.y() ) );
    }

    void mouseMove( double mapX, double mapY )
    {
      QgsMapMouseEvent e( mCanvas, QEvent::MouseMove, mapToScreen( mapX, mapY ) );
      mCaptureTool->cadCanvasMoveEvent( &e );
    }

    void mousePress( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers() )
    {
      QgsMapMouseEvent e1( mCanvas, QEvent::MouseButtonPress, mapToScreen( mapX, mapY ), button, button, stateKey );
      mCaptureTool->cadCanvasPressEvent( &e1 );
    }

    void mouseRelease( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers() )
    {
      QgsMapMouseEvent e2( mCanvas, QEvent::MouseButtonRelease, mapToScreen( mapX, mapY ), button, Qt::MouseButton(), stateKey );
      mCaptureTool->cadCanvasReleaseEvent( &e2 );
    }

    void mouseClick( double mapX, double mapY, Qt::MouseButton button, Qt::KeyboardModifiers stateKey = Qt::KeyboardModifiers() )
    {
      mousePress( mapX, mapY, button, stateKey );
      mouseRelease( mapX, mapY, button, stateKey );
    }

    void keyClick( int key )
    {
      QKeyEvent e1( QEvent::KeyPress, key, Qt::KeyboardModifiers() );
      mCaptureTool->keyPressEvent( &e1 );

      QKeyEvent e2( QEvent::KeyRelease, key, Qt::KeyboardModifiers() );
      mCaptureTool->keyReleaseEvent( &e2 );
    }

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapCanvasTracer *mTracer = nullptr;
    QAction *mEnableTracingAction = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerLine = nullptr;
    QgsFeatureId mFidLineF1 = 0;
};

TestQgsMapToolAddFeature::TestQgsMapToolAddFeature() = default;


//runs before all tests
void TestQgsMapToolAddFeature::initTestCase()
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

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerLine ); //<< mLayerPolygon << mLayerPoint );

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
void TestQgsMapToolAddFeature::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeature::testNoTracing()
{
  // tracing not enabled - will be straight line

  QSet<QgsFeatureId> oldFids = _existingFeatureIds( mLayerLine );

  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 3, 2, Qt::LeftButton );
  mouseClick( 3, 2, Qt::RightButton );

  QgsFeatureId newFid = _newFeatureId( mLayerLine, oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 3 2)" ) );

  mLayerLine->undoStack()->undo();
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );
}

void TestQgsMapToolAddFeature::testTracing()
{
  // tracing enabled - same clicks - now following line

  mEnableTracingAction->setChecked( true );

  QSet<QgsFeatureId> oldFids = _existingFeatureIds( mLayerLine );

  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 3, 2, Qt::LeftButton );
  mouseClick( 3, 2, Qt::RightButton );

  QgsFeatureId newFid = _newFeatureId( mLayerLine, oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid ).geometry(), QgsGeometry::fromWkt( "LINESTRING(1 1, 2 1, 3 2)" ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  // tracing enabled - combined with first and last segments that are not traced

  mouseClick( 0, 2, Qt::LeftButton );
  mouseClick( 1, 1, Qt::LeftButton );
  mouseClick( 3, 2, Qt::LeftButton );
  mouseClick( 4, 1, Qt::LeftButton );
  mouseClick( 4, 1, Qt::RightButton );

  QgsFeatureId newFid2 = _newFeatureId( mLayerLine, oldFids );

  QCOMPARE( mLayerLine->undoStack()->index(), 2 );
  QCOMPARE( mLayerLine->getFeature( newFid2 ).geometry(), QgsGeometry::fromWkt( "LINESTRING(0 2, 1 1, 2 1, 3 2, 4 1)" ) );

  mLayerLine->undoStack()->undo();

  // no other unexpected changes happened
  QCOMPARE( mLayerLine->undoStack()->index(), 1 );

  mEnableTracingAction->setChecked( false );
}

void TestQgsMapToolAddFeature::testTracingWithOffset()
{
  // tracing enabled + offset enabled

  mEnableTracingAction->setChecked( true );
  mTracer->setOffset( 0.1 );

  QSet<QgsFeatureId> oldFids = _existingFeatureIds( mLayerLine );

  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 1, 2, Qt::LeftButton );
  mouseClick( 1, 2, Qt::RightButton );

  QgsFeatureId newFid = _newFeatureId( mLayerLine, oldFids );

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

  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 1, 2, Qt::LeftButton );
  mouseClick( 1, 2, Qt::RightButton );

  QgsFeatureId newFid2 = _newFeatureId( mLayerLine, oldFids );

  QgsGeometry g2 = mLayerLine->getFeature( newFid2 ).geometry();
  QgsPolylineXY poly2 = g2.asPolyline();
  QCOMPARE( poly2.count(), 3 );
  QCOMPARE( poly2[0], QgsPointXY( 2, 1.1 ) );
  QCOMPARE( poly2[1], QgsPointXY( 1.1, 1.1 ) );
  QCOMPARE( poly2[2], QgsPointXY( 1.1, 2 ) );

  mLayerLine->undoStack()->undo();

  // tracing enabled + offset enabled - combined with first and last segments that are not traced

  mouseClick( 3, 0, Qt::LeftButton );
  mouseClick( 2, 1, Qt::LeftButton );
  mouseClick( 1, 2, Qt::LeftButton );
  mouseClick( 0, 1, Qt::LeftButton );
  mouseClick( 0, 1, Qt::RightButton );

  QgsFeatureId newFid3 = _newFeatureId( mLayerLine, oldFids );

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
}


QGSTEST_MAIN( TestQgsMapToolAddFeature )
#include "testqgsmaptooladdfeature.moc"
