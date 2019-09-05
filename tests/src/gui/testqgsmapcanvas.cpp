/***************************************************************************
    testqgsmapcanvas.cpp
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrenderchecker.h"
#include "qgsvectordataprovider.h"
#include "qgsmaptoolpan.h"
#include "qgscustomdrophandler.h"
#include "qgsreferencedgeometry.h"

namespace QTest
{
  template<>
  char *toString( const QgsRectangle &r )
  {
    QByteArray ba = r.toString().toLocal8Bit();
    return qstrdup( ba.data() );
  }
}

class QgsMapToolTest : public QgsMapTool
{
  public:
    QgsMapToolTest( QgsMapCanvas *canvas ) : QgsMapTool( canvas ) {}

};

class TestQgsMapCanvas : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapCanvas() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testPanByKeyboard();
    void testSetExtent();
    void testMagnification();
    void testMagnificationExtent();
    void testMagnificationScale();
    void testScaleLockCanvasResize();
    void testZoomByWheel();
    void testShiftZoom();
    void testDragDrop();

  private:
    QgsMapCanvas *mCanvas = nullptr;
};



void TestQgsMapCanvas::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();

  mCanvas = new QgsMapCanvas();
}

void TestQgsMapCanvas::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsMapCanvas::testPanByKeyboard()
{
  // The keys to simulate
  QList<Qt::Key> keys = QList<Qt::Key>() << Qt::Key_Left << Qt::Key_Down << Qt::Key_Right << Qt::Key_Up;

  // The canvas rotations to test
  QList<double> rotations = QList<double>() << 0.0 << 30.0;

  QgsRectangle initialExtent( 100, 100, 110, 110 );

  Q_FOREACH ( double rotation, rotations )
  {
    // Set rotation and initial extent
    mCanvas->setRotation( rotation );
    mCanvas->setExtent( initialExtent );

    // Save actual extent, simulate panning by keyboard and verify the extent is unchanged
    QgsRectangle originalExtent = mCanvas->extent();
    Q_FOREACH ( Qt::Key key, keys )
    {
      QgsRectangle tempExtent = mCanvas->extent();
      QKeyEvent keyEvent( QEvent::KeyPress, key, Qt::NoModifier );
      QApplication::sendEvent( mCanvas, &keyEvent );
      QVERIFY( mCanvas->extent() != tempExtent );
    }
    QVERIFY( mCanvas->extent() == originalExtent );
  }
}

void TestQgsMapCanvas::testSetExtent()
{
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  QVERIFY( mCanvas->setReferencedExtent( QgsReferencedRectangle( QgsRectangle( 0, 0, 10, 10 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) ) );
  QCOMPARE( mCanvas->extent().toString( 0 ), QStringLiteral( "-3,-3 : 13,13" ) );
  QVERIFY( mCanvas->setReferencedExtent( QgsReferencedRectangle( QgsRectangle( 16259461, -2477192, 16391255, -2372535 ), QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) ) ) );
  QCOMPARE( mCanvas->extent().toString( 0 ), QStringLiteral( "146,-22 : 147,-21" ) );
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( ) );
}

void TestQgsMapCanvas::testMagnification()
{
  // test directory
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  QString controlImageDir = testDataDir + "control_images/expected_map_magnification/";

  // prepare spy and unit testing stuff
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "mapcanvas" ) );
  checker.setColorTolerance( 5 );

  QSignalSpy spy( mCanvas, SIGNAL( mapCanvasRefreshed() ) );

  QEventLoop loop;
  QObject::connect( mCanvas, SIGNAL( mapCanvasRefreshed() ), &loop, SLOT( quit() ) );

  QTimer timer;
  QObject::connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );

  QTemporaryFile tmpFile;
  tmpFile.setAutoRemove( false );
  tmpFile.open(); // fileName is not available until open
  QString tmpName = tmpFile.fileName();
  tmpFile.close();

  // build vector layer
  QString myPointsFileName = testDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  QgsVectorLayer *layer = new QgsVectorLayer( myPointFileInfo.filePath(),
      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // prepare map canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << layer );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer );

  mCanvas->setExtent( layer->extent() );

  // refresh and wait for rendering
  mCanvas->refresh();
  timer.start( 3000 );
  loop.exec();
  QCOMPARE( spy.count(), 1 );
  spy.clear();

  // control image with magnification factor 1.0
  mCanvas->saveAsImage( tmpName );

  checker.setControlName( QStringLiteral( "expected_map_magnification" ) );
  checker.setRenderedImage( tmpName );
  checker.setSizeTolerance( 10, 10 );
  QCOMPARE( checker.compareImages( "map_magnification", 100 ), true );

  // set magnification factor (auto refresh)
  mCanvas->setMagnificationFactor( 6.5 );
  QCOMPARE( mCanvas->magnificationFactor(), 6.5 );

  // wait for rendering
  timer.start( 3000 );
  loop.exec();
  QCOMPARE( spy.count(), 1 );
  spy.clear();

  // control image with magnification factor 6.5
  mCanvas->saveAsImage( tmpName );

  checker.setRenderedImage( tmpName );
  checker.setControlName( QStringLiteral( "expected_map_magnification_6_5" ) );
  controlImageDir = testDataDir + "control_images/";
  checker.setSizeTolerance( 10, 10 );
  QCOMPARE( checker.compareImages( "map_magnification_6_5", 100 ), true );

  // set magnification factor (auto refresh)
  mCanvas->setMagnificationFactor( 1.0 );
  QCOMPARE( mCanvas->magnificationFactor(), 1.0 );

  // wait for rendering
  timer.start( 3000 );
  loop.exec();
  QCOMPARE( spy.count(), 1 );
  spy.clear();

  // control image with magnification factor 1.0
  mCanvas->saveAsImage( tmpName );

  checker.setControlName( QStringLiteral( "expected_map_magnification" ) );
  checker.setRenderedImage( tmpName );
  checker.setSizeTolerance( 10, 10 );
  QCOMPARE( checker.compareImages( QStringLiteral( "map_magnification" ), 100 ), true );
}

void compareExtent( const QgsRectangle &initialExtent,
                    const QgsRectangle &extent )
{
  QGSCOMPARENEAR( initialExtent.xMinimum(), extent.xMinimum(), 0.00000000001 );
  QGSCOMPARENEAR( initialExtent.xMaximum(), extent.xMaximum(), 0.00000000001 );
  QGSCOMPARENEAR( initialExtent.yMinimum(), extent.yMinimum(), 0.00000000001 );
  QGSCOMPARENEAR( initialExtent.yMaximum(), extent.yMaximum(), 0.00000000001 );
}

void TestQgsMapCanvas::testMagnificationExtent()
{
  // build vector layer
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  QString myPointsFileName = testDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  QgsVectorLayer *layer = new QgsVectorLayer( myPointFileInfo.filePath(),
      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // prepare map canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << layer );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << layer );

  // zoomToFullExtent
  mCanvas->zoomToFullExtent();
  QgsRectangle initialExtent = mCanvas->extent();

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  compareExtent( mCanvas->extent(), initialExtent );

  // setExtent with layer extent
  mCanvas->setExtent( layer->extent() );
  initialExtent = mCanvas->extent();

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  compareExtent( mCanvas->extent(), initialExtent );

  // zoomToSelected
  QgsFeature f1( layer->dataProvider()->fields(), 1 );
  QgsFeature f2( layer->dataProvider()->fields(), 2 );
  QgsFeatureIds ids;
  ids << f1.id() << f2.id();
  layer->selectByIds( ids );

  mCanvas->zoomToSelected( layer );
  initialExtent = mCanvas->extent();

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  compareExtent( mCanvas->extent(), initialExtent );

  // zoomToFeatureIds
  mCanvas->zoomToFeatureIds( layer, ids );
  initialExtent = mCanvas->extent();

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  compareExtent( mCanvas->extent(), initialExtent );

  // zoomIn / zoomOut
  initialExtent = mCanvas->extent();
  mCanvas->zoomIn();

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->zoomIn();
  mCanvas->zoomOut();
  mCanvas->setMagnificationFactor( 1.0 );

  mCanvas->zoomOut();

  compareExtent( mCanvas->extent(), initialExtent );

  // zoomScale
  initialExtent = mCanvas->extent();
  double scale = mCanvas->scale();
  mCanvas->zoomScale( 6.052017 * 10e7 );

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  mCanvas->zoomScale( scale );
  compareExtent( mCanvas->extent(), initialExtent );
}

void TestQgsMapCanvas::testMagnificationScale()
{
  mCanvas->setMagnificationFactor( 1.0 );
  double initialScale = mCanvas->scale();

  mCanvas->setMagnificationFactor( 4.0 );
  QCOMPARE( initialScale, mCanvas->scale() );

  mCanvas->setMagnificationFactor( 7.5 );
  QCOMPARE( initialScale, mCanvas->scale() );

  mCanvas->setMagnificationFactor( 1.0 );
  QCOMPARE( initialScale, mCanvas->scale() );
}

void TestQgsMapCanvas::testScaleLockCanvasResize()
{
  QSize prevSize = mCanvas->size();

  mCanvas->resize( 600, 400 );
  QgsApplication::sendPostedEvents( mCanvas );
  mCanvas->resizeEvent( nullptr );
  QCOMPARE( mCanvas->width(), 600 );
  QCOMPARE( mCanvas->height(), 400 );

  mCanvas->setMagnificationFactor( 2.0 );
  double initialScale = mCanvas->scale();
  mCanvas->setScaleLocked( true );

  mCanvas->resize( 300, 200 );
  QgsApplication::sendPostedEvents( mCanvas );
  mCanvas->resizeEvent( nullptr );
  QCOMPARE( mCanvas->width(), 300 );
  QCOMPARE( mCanvas->height(), 200 );

  QCOMPARE( mCanvas->magnificationFactor(), 2.0 );
  QCOMPARE( mCanvas->scale(), initialScale );

  mCanvas->setScaleLocked( false );
  mCanvas->setMagnificationFactor( 1.0 );
  mCanvas->resize( prevSize );
  QgsApplication::sendPostedEvents( mCanvas );
  mCanvas->resizeEvent( nullptr );
}

void TestQgsMapCanvas::testZoomByWheel()
{
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  QgsRectangle initialExtent = mCanvas->extent();
  double originalWidth = initialExtent.width();
  double originalHeight = initialExtent.height();

  mCanvas->setWheelFactor( 2 );

  //test zoom out
  QWheelEvent e( QPoint( 0, 0 ), -QWheelEvent::DefaultDeltasPerStep, Qt::NoButton, Qt::NoModifier );
  mCanvas->wheelEvent( &e );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth * 2.0, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight * 2.0, 0.1 );

  //test zoom in
  e = QWheelEvent( QPoint( 0, 0 ), QWheelEvent::DefaultDeltasPerStep, Qt::NoButton, Qt::NoModifier );
  mCanvas->wheelEvent( &e );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.1 );

  // test zoom out with ctrl
  e = QWheelEvent( QPoint( 0, 0 ), -QWheelEvent::DefaultDeltasPerStep, Qt::NoButton, Qt::ControlModifier );
  mCanvas->wheelEvent( &e );
  QGSCOMPARENEAR( mCanvas->extent().width(), 1.05 * originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), 1.05 * originalHeight, 0.1 );

  //test zoom in with ctrl
  e = QWheelEvent( QPoint( 0, 0 ), QWheelEvent::DefaultDeltasPerStep, Qt::NoButton, Qt::ControlModifier );
  mCanvas->wheelEvent( &e );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.1 );
}

void TestQgsMapCanvas::testShiftZoom()
{
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  QgsRectangle initialExtent = mCanvas->extent();
  double originalWidth = initialExtent.width();
  double originalHeight = initialExtent.height();

  QPoint startPos = QPoint( mCanvas->width() / 4, mCanvas->height() / 4 );
  QPoint endPos = QPoint( mCanvas->width() * 3 / 4.0, mCanvas->height() * 3 / 4.0 );

  QgsMapToolPan panTool( mCanvas );

  // start by testing a tool with shift-zoom enabled
  mCanvas->setMapTool( &panTool );

  QMouseEvent e( QMouseEvent::MouseButtonPress, startPos,
                 Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseMove, endPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseButtonRelease, endPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( &e );

  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth / 2.0, 0.2 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight / 2.0, 0.2 );

  //reset
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );

  //test that a shift-click (no movement) will not zoom
  e = QMouseEvent( QMouseEvent::MouseButtonPress, startPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseMove, startPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseButtonRelease, startPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( &e );

  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.0001 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.0001 );

  //reset
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );

  //test with map tool which does not have shift-zoom enabled
  QgsMapToolTest mapTool( mCanvas );
  mCanvas->setMapTool( &mapTool );

  e = QMouseEvent( QMouseEvent::MouseButtonPress, startPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseMove, endPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( &e );
  e = QMouseEvent( QMouseEvent::MouseButtonRelease, endPos,
                   Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( &e );

  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.00001 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.00001 );
}

class TestNoDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override { return QStringLiteral( "test" ); }
    bool canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * ) override { return false; }
    bool handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * ) const override { return false; }
};

class TestYesDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override { return QStringLiteral( "test" ); }
    bool canHandleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * ) override { return true; }
    bool handleCustomUriCanvasDrop( const QgsMimeDataUtils::Uri &, QgsMapCanvas * ) const override { return true; }
};

void TestQgsMapCanvas::testDragDrop()
{
  // default drag, should not be accepted
  std::unique_ptr< QMimeData > data = qgis::make_unique< QMimeData >();
  std::unique_ptr< QDragEnterEvent > event = qgis::make_unique< QDragEnterEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
  mCanvas->dragEnterEvent( event.get() );
  QVERIFY( !event->isAccepted() );

  // with mime data
  QgsMimeDataUtils::UriList list;
  QgsMimeDataUtils::Uri uri;
  uri.name = QStringLiteral( "name" );
  uri.providerKey = QStringLiteral( "test" );
  list << uri;
  data.reset( QgsMimeDataUtils::encodeUriList( list ) );
  event = qgis::make_unique< QDragEnterEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
  mCanvas->dragEnterEvent( event.get() );
  // still not accepted by default
  QVERIFY( !event->isAccepted() );

  // add a custom drop handler to the canvas
  TestNoDropHandler handler;
  mCanvas->setCustomDropHandlers( QVector< QPointer< QgsCustomDropHandler > >() << &handler );
  mCanvas->dragEnterEvent( event.get() );
  // not accepted by handler
  QVERIFY( !event->isAccepted() );

  TestYesDropHandler handler2;
  mCanvas->setCustomDropHandlers( QVector< QPointer< QgsCustomDropHandler > >() << &handler << &handler2 );
  mCanvas->dragEnterEvent( event.get() );
  // IS accepted by handler
  QVERIFY( event->isAccepted() );

  // check drop event logic
  mCanvas->setCustomDropHandlers( QVector< QPointer< QgsCustomDropHandler > >() );
  std::unique_ptr< QDropEvent > dropEvent = qgis::make_unique< QDropEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
  mCanvas->dropEvent( dropEvent.get() );
  QVERIFY( !dropEvent->isAccepted() );
  mCanvas->setCustomDropHandlers( QVector< QPointer< QgsCustomDropHandler > >() << &handler );
  mCanvas->dropEvent( dropEvent.get() );
  QVERIFY( !dropEvent->isAccepted() );
  mCanvas->setCustomDropHandlers( QVector< QPointer< QgsCustomDropHandler > >() << &handler << &handler2 );
  mCanvas->dropEvent( dropEvent.get() );
  // is accepted!
  QVERIFY( dropEvent->isAccepted() );
}

QGSTEST_MAIN( TestQgsMapCanvas )
#include "testqgsmapcanvas.moc"
