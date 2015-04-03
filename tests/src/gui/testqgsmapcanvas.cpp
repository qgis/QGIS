
#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsmaprenderer.h>

namespace QTest
{
  template<>
  char* toString( const QgsRectangle& r )
  {
    QByteArray ba = r.toString().toLocal8Bit();
    return qstrdup( ba.data() );
  }
}

class TestQgsMapCanvas : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapCanvas()
        : mCanvas( 0 )
    {}

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testMapRendererInteraction();

  private:
    QgsMapCanvas* mCanvas;
};



void TestQgsMapCanvas::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup

  mCanvas = new QgsMapCanvas();
}

void TestQgsMapCanvas::cleanupTestCase()
{
}

void TestQgsMapCanvas::testMapRendererInteraction()
{
  Q_NOWARN_DEPRECATED_PUSH
  QgsMapRenderer* mr = mCanvas->mapRenderer();
  Q_NOWARN_DEPRECATED_POP

  // CRS transforms

  QSignalSpy spy0( mCanvas, SIGNAL( hasCrsTransformEnabledChanged( bool ) ) );
  mr->setProjectionsEnabled( true );
  QCOMPARE( mr->hasCrsTransformEnabled(), true );
  QCOMPARE( mCanvas->hasCrsTransformEnabled(), true );
  QCOMPARE( spy0.count(), 1 );

  QSignalSpy spy1( mr, SIGNAL( hasCrsTransformEnabled( bool ) ) );
  mCanvas->setCrsTransformEnabled( false );
  QCOMPARE( mr->hasCrsTransformEnabled(), false );
  QCOMPARE( mCanvas->hasCrsTransformEnabled(), false );
  QCOMPARE( spy1.count(), 1 );

  // Extent

  QSignalSpy spy2( mCanvas, SIGNAL( extentsChanged() ) );
  QgsRectangle r1( 10, 10, 20, 20 );
  mr->setExtent( r1 );
  QgsRectangle r2 = mr->extent();
  QCOMPARE( mCanvas->extent(), r2 );
  QCOMPARE( spy2.count(), 1 );

  QgsRectangle r3( 100, 100, 200, 200 );
  QSignalSpy spy3( mr, SIGNAL( extentsChanged() ) );
  mCanvas->setExtent( r3 );
  QgsRectangle r4 = mCanvas->extent();
  QCOMPARE( mr->extent(), r4 );
  QCOMPARE( spy3.count(), 1 );

  // Destination CRS

  QgsCoordinateReferenceSystem crs1( "EPSG:27700" );
  QCOMPARE( crs1.isValid(), true );
  QSignalSpy spy4( mCanvas, SIGNAL( destinationCrsChanged() ) );
  mr->setDestinationCrs( crs1 );
  qDebug( " crs %s vs %s", mCanvas->mapSettings().destinationCrs().authid().toAscii().data(), crs1.authid().toAscii().data() );
  QCOMPARE( mCanvas->mapSettings().destinationCrs(), crs1 );
  QCOMPARE( mr->destinationCrs(), crs1 );
  QCOMPARE( spy4.count(), 1 );

  QgsCoordinateReferenceSystem crs2( "EPSG:4326" );
  QCOMPARE( crs2.isValid(), true );
  QSignalSpy spy5( mr, SIGNAL( destinationSrsChanged() ) );
  mCanvas->setDestinationCrs( crs2 );
  QCOMPARE( mCanvas->mapSettings().destinationCrs(), crs2 );
  QCOMPARE( mr->destinationCrs(), crs2 );
  QCOMPARE( spy5.count(), 1 );

  // TODO: set map units
}


QTEST_MAIN( TestQgsMapCanvas )
#include "testqgsmapcanvas.moc"
