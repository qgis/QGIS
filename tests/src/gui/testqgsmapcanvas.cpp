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
#include <QSignalSpy>
#include <QtMath>

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

class QgsMapToolTest : public QgsMapTool // clazy:exclude=missing-qobject-macro
{
  public:
    QgsMapToolTest( QgsMapCanvas *canvas ) : QgsMapTool( canvas ) {}

    bool canvasToolTipEvent( QHelpEvent *e ) override
    {
      Q_UNUSED( e );
      mGotTooltipEvent = true;
      return true;
    }
    bool gotTooltipEvent() const
    {
      return mGotTooltipEvent;
    }

  private:
    bool mGotTooltipEvent = false;

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
    void testZoomResolutions();
    void testTooltipEvent();
    void testMapLayers();
    void testExtentHistory();

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
  const QList<Qt::Key> keys = QList<Qt::Key>() << Qt::Key_Left << Qt::Key_Down << Qt::Key_Right << Qt::Key_Up;

  // The canvas rotations to test
  const QList<double> rotations = QList<double>() << 0.0 << 30.0;

  const QgsRectangle initialExtent( 100, 100, 110, 110 );

  for ( const double rotation : rotations )
  {
    // Set rotation and initial extent
    mCanvas->setRotation( rotation );
    mCanvas->setExtent( initialExtent );

    // Save actual extent, simulate panning by keyboard and verify the extent is unchanged
    const QgsRectangle originalExtent = mCanvas->extent();
    for ( const Qt::Key key : keys )
    {
      const QgsRectangle tempExtent = mCanvas->extent();
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
  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
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
  const QString tmpName = tmpFile.fileName();
  tmpFile.close();

  // build vector layer
  const QString myPointsFileName = testDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
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
  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  const QString myPointsFileName = testDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
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
  const QgsFeature f1( layer->dataProvider()->fields(), 1 );
  const QgsFeature f2( layer->dataProvider()->fields(), 2 );
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
  const double scale = mCanvas->scale();
  mCanvas->zoomScale( 6.052017 * 10e7 );

  mCanvas->setMagnificationFactor( 4.0 );
  mCanvas->setMagnificationFactor( 1.0 );

  mCanvas->zoomScale( scale );
  compareExtent( mCanvas->extent(), initialExtent );
}

void TestQgsMapCanvas::testMagnificationScale()
{
  mCanvas->setMagnificationFactor( 1.0 );
  const double initialScale = mCanvas->scale();

  mCanvas->setMagnificationFactor( 4.0 );
  QCOMPARE( initialScale, mCanvas->scale() );

  mCanvas->setMagnificationFactor( 7.5 );
  QCOMPARE( initialScale, mCanvas->scale() );

  mCanvas->setMagnificationFactor( 1.0 );
  QCOMPARE( initialScale, mCanvas->scale() );
}

void TestQgsMapCanvas::testScaleLockCanvasResize()
{
  const QSize prevSize = mCanvas->size();

  mCanvas->resize( 600, 400 );
  QgsApplication::sendPostedEvents( mCanvas );
  mCanvas->resizeEvent( nullptr );
  QCOMPARE( mCanvas->width(), 600 );
  QCOMPARE( mCanvas->height(), 400 );

  mCanvas->setMagnificationFactor( 2.0 );
  const double initialScale = mCanvas->scale();
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
  const QgsRectangle initialExtent = mCanvas->extent();
  const double originalWidth = initialExtent.width();
  const double originalHeight = initialExtent.height();

  mCanvas->setWheelFactor( 2 );

  //test zoom out
  std::unique_ptr< QWheelEvent > e = std::make_unique< QWheelEvent >( QPoint( 0, 0 ), QPointF(), QPoint( 0, -QWheelEvent::DefaultDeltasPerStep ), QPoint( 0, -QWheelEvent::DefaultDeltasPerStep ), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false );
  mCanvas->wheelEvent( e.get() );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth * 2.0, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight * 2.0, 0.1 );

  //test zoom in
  e = std::make_unique< QWheelEvent >( QPoint( 0, 0 ), QPointF(), QPoint( 0, QWheelEvent::DefaultDeltasPerStep ), QPoint( 0, QWheelEvent::DefaultDeltasPerStep ), Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false );
  mCanvas->wheelEvent( e.get() );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.1 );

  // test zoom out with ctrl
  e = std::make_unique< QWheelEvent >( QPoint( 0, 0 ), QPointF(), QPoint( 0, -QWheelEvent::DefaultDeltasPerStep ), QPoint( 0, -QWheelEvent::DefaultDeltasPerStep ), Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false );
  mCanvas->wheelEvent( e.get() );
  QGSCOMPARENEAR( mCanvas->extent().width(), 1.05 * originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), 1.05 * originalHeight, 0.1 );

  //test zoom in with ctrl
  e = std::make_unique< QWheelEvent >( QPoint( 0, 0 ), QPointF(), QPoint( 0, QWheelEvent::DefaultDeltasPerStep ), QPoint( 0, QWheelEvent::DefaultDeltasPerStep ), Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false );
  mCanvas->wheelEvent( e.get() );
  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.1 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.1 );
}

void TestQgsMapCanvas::testShiftZoom()
{
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  const QgsRectangle initialExtent = mCanvas->extent();
  const double originalWidth = initialExtent.width();
  const double originalHeight = initialExtent.height();

  const QPoint startPos = QPoint( mCanvas->width() / 4, mCanvas->height() / 4 );
  const QPoint endPos = QPoint( mCanvas->width() * 3 / 4.0, mCanvas->height() * 3 / 4.0 );

  QgsMapToolPan panTool( mCanvas );

  // start by testing a tool with shift-zoom enabled
  mCanvas->setMapTool( &panTool );

  std::unique_ptr< QMouseEvent > e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonPress, startPos,
                                     Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseMove, endPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonRelease, endPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( e.get() );

  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth / 2.0, 0.2 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight / 2.0, 0.2 );

  //reset
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );

  //test that a shift-click (no movement) will not zoom
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonPress, startPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseMove, startPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonRelease, startPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( e.get() );

  QGSCOMPARENEAR( mCanvas->extent().width(), originalWidth, 0.0001 );
  QGSCOMPARENEAR( mCanvas->extent().height(), originalHeight, 0.0001 );

  //reset
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );

  //test with map tool which does not have shift-zoom enabled
  QgsMapToolTest mapTool( mCanvas );
  mCanvas->setMapTool( &mapTool );

  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonPress, startPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mousePressEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseMove, endPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseMoveEvent( e.get() );
  e = std::make_unique< QMouseEvent >( QMouseEvent::MouseButtonRelease, endPos,
                                       Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier );
  mCanvas->mouseReleaseEvent( e.get() );

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
  std::unique_ptr< QMimeData > data = std::make_unique< QMimeData >();
  std::unique_ptr< QDragEnterEvent > event = std::make_unique< QDragEnterEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
  mCanvas->dragEnterEvent( event.get() );
  QVERIFY( !event->isAccepted() );

  // with mime data
  QgsMimeDataUtils::UriList list;
  QgsMimeDataUtils::Uri uri;
  uri.name = QStringLiteral( "name" );
  uri.providerKey = QStringLiteral( "test" );
  list << uri;
  data.reset( QgsMimeDataUtils::encodeUriList( list ) );
  event = std::make_unique< QDragEnterEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
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
  std::unique_ptr< QDropEvent > dropEvent = std::make_unique< QDropEvent >( QPoint( 10, 10 ), Qt::CopyAction, data.get(), Qt::LeftButton, Qt::NoModifier );
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

void TestQgsMapCanvas::testZoomResolutions()
{
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  const double resolution = mCanvas->mapSettings().mapUnitsPerPixel();
  const double wheelFactor = 2.0;
  mCanvas->setWheelFactor( wheelFactor );

  const double nextResolution = qCeil( resolution ) + 1;
  QList<double> resolutions = QList<double>() << nextResolution << ( 2.5 * nextResolution ) << ( 3.6 * nextResolution ) << ( 4.7 * nextResolution );
  mCanvas->setZoomResolutions( resolutions );

  // From first to last resolution in list
  // Ensure we are at first resolution
  while ( mCanvas->mapSettings().mapUnitsPerPixel() < resolutions[0] )
  {
    mCanvas->zoomOut();
  }
  int nResolutions = resolutions.size();
  for ( int i = 1; i < nResolutions; ++i )
  {
    mCanvas->zoomOut();
    QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions[i], 0.0001 );
  }

  // beyond last resolution
  mCanvas->zoomOut();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), wheelFactor * resolutions.last(), 0.0001 );

  mCanvas->zoomOut();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), wheelFactor * wheelFactor * resolutions.last(), 0.0001 );

  mCanvas->zoomIn();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), wheelFactor * resolutions.last(), 0.0001 );

  // From last to first resolution in list
  for ( int i = nResolutions - 1; i >= 0; --i )
  {
    mCanvas->zoomIn();
    QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions[i], 0.0001 );
  }

  // before first resolution
  mCanvas->zoomIn();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions.first() / wheelFactor, 0.0001 );

  mCanvas->zoomIn();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions.first() / ( wheelFactor * wheelFactor ), 0.0001 );

  mCanvas->zoomOut();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions.first() / wheelFactor, 0.0001 );

  mCanvas->zoomOut();
  QGSCOMPARENEAR( mCanvas->mapSettings().mapUnitsPerPixel(), resolutions.first(), 0.0001 );

  QCOMPARE( mCanvas->zoomResolutions(), resolutions );
}

void TestQgsMapCanvas::testTooltipEvent()
{
  QgsMapToolTest mapTool( mCanvas );
  mCanvas->setMapTool( &mapTool );

  QHelpEvent helpEvent( QEvent::ToolTip, QPoint( 10, 10 ), QPoint( 10, 10 ) );

  QApplication::sendEvent( mCanvas->viewport(), &helpEvent );

  QVERIFY( mapTool.gotTooltipEvent() );
}

void TestQgsMapCanvas::testMapLayers()
{
  QgsProject::instance()->clear();
  //set up canvas with a mix of project and non-project layers
  QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:3946&field=halig:string&field=valig:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QVERIFY( vl1->isValid() );
  QgsProject::instance()->addMapLayer( vl1 );

  std::unique_ptr< QgsVectorLayer > vl2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "Point?crs=epsg:3946&field=halig:string&field=valig:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QVERIFY( vl2->isValid() );

  std::unique_ptr< QgsMapCanvas > canvas = std::make_unique< QgsMapCanvas >();
  canvas->setLayers( { vl1, vl2.get() } );

  QCOMPARE( canvas->layers(), QList< QgsMapLayer * >( { vl1, vl2.get() } ) );
  // retrieving layer by id should work for both layers from the project AND for freestanding layers
  QCOMPARE( canvas->layer( vl1->id() ), vl1 );
  QCOMPARE( canvas->layer( vl2->id() ), vl2.get() );
  QCOMPARE( canvas->layer( QStringLiteral( "xxx" ) ), nullptr );
}

void TestQgsMapCanvas::testExtentHistory()
{
  QgsRectangle initialExtent;
  const QList<double> rotations = QList<double>() << 0.0 << 30.0;
  for ( double rotation : rotations )
  {
    mCanvas->setRotation( rotation );
    mCanvas->clearExtentHistory();
    mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
    initialExtent = mCanvas->extent();
    mCanvas->setExtent( QgsRectangle( 100, 100, 110, 110 ) );
    mCanvas->zoomToPreviousExtent();
    QCOMPARE( mCanvas->extent(), initialExtent );
  }
}

QGSTEST_MAIN( TestQgsMapCanvas )
#include "testqgsmapcanvas.moc"
