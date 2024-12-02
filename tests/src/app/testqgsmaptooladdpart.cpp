/***************************************************************************
     testqgsmaptooladdpart.cpp
     --------------------------------
    Date                 : 2023-09-25
    Copyright            : (C) 2023 by Mathieu Pellerin
    Email                : mathieu@opengis.ch
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
#include "qgsmaptooladdpart.h"
#include "qgsproject.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the add part map tool
 */
class TestQgsMapToolAddPart : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddPart();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testAddPart();
    void testAddPartClockWise();
    void testAddPartToSingleGeometryLess();

  private:
    QPoint mapToPoint( double x, double y );

    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddPart *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerMultiPolygon = nullptr;
};

TestQgsMapToolAddPart::TestQgsMapToolAddPart() = default;


//runs before all tests
void TestQgsMapToolAddPart::initTestCase()
{
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
  mLayerMultiPolygon = new QgsVectorLayer( QStringLiteral( "MultiPolygon?crs=EPSG:3946" ), QStringLiteral( "multipolygon" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerMultiPolygon->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerMultiPolygon );

  mLayerMultiPolygon->startEditing();
  QgsFeature f;
  const QString wkt( "MultiPolygon (((2 2, 4 2, 4 4, 2 4)))" );
  f.setGeometry( QgsGeometry::fromWkt( wkt ) );
  mLayerMultiPolygon->dataProvider()->addFeatures( QgsFeatureList() << f );
  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );

  mCanvas->setCurrentLayer( mLayerMultiPolygon );

  // create the tool
  mCaptureTool = new QgsMapToolAddPart( mCanvas );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddPart::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

QPoint TestQgsMapToolAddPart::mapToPoint( double x, double y )
{
  const QgsPointXY mapPoint = mCanvas->mapSettings().mapToPixel().transform( x, y );

  return QPoint( static_cast<int>( std::round( mapPoint.x() ) ), static_cast<int>( std::round( mapPoint.y() ) ) );
}

void TestQgsMapToolAddPart::testAddPart()
{
  mLayerMultiPolygon->select( 1 );

  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 5, 5 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 5, 5 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 6, 5 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 6, 6 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 5, 6 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 5, 5 ),
    Qt::RightButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  const QString wkt = "MultiPolygon (((2 2, 4 2, 4 4, 2 4)),((5 5, 5 5, 6 5, 6 6, 5 6, 5 5)))";
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );
}

void TestQgsMapToolAddPart::testAddPartClockWise()
{
  mLayerMultiPolygon->select( 1 );

  // Draw in clockwise
  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 15, 15 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 15, 16 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 16, 16 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 16, 15 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 15, 15 ),
    Qt::RightButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  const QString wkt = "MultiPolygon (((2 2, 4 2, 4 4, 2 4)),((5 5, 5 5, 6 5, 6 6, 5 6, 5 5)),((15 15, 16 15, 16 16, 15 16, 15 15)))";
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );
}

void TestQgsMapToolAddPart::testAddPartToSingleGeometryLess()
{
  QMap<QStringList, QList<QgsPoint>> geomTypes;
  geomTypes.insert( { "Point" }, { QgsPoint( 0, 0 ) } );
  geomTypes.insert( { "LineString", "CompoundCurve" }, { QgsPoint( 0, 0 ), QgsPoint( 1, 1 ) } );
  geomTypes.insert( { "Polygon", "CurvePolygon" }, { QgsPoint( 0, 0 ), QgsPoint( 1, 0 ), QgsPoint( 1, 1 ), QgsPoint( 0, 1 ) } );


  for ( auto it = geomTypes.constBegin(); it != geomTypes.constEnd(); it++ )
  {
    for ( const QString &geomType : it.key() )
    {
      QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "%1?crs=EPSG:3946" ).arg( geomType ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) );
      QVERIFY( vl->isValid() );
      QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << vl );

      vl->startEditing();
      QgsFeature f;
      vl->dataProvider()->addFeatures( QgsFeatureList() << f );
      QCOMPARE( vl->featureCount(), ( long ) 1 );
      QVERIFY( vl->getFeature( 1 ).geometry().isNull() );

      mCanvas->setCurrentLayer( vl );
      vl->select( 1 );

      QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
      QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );

      std::unique_ptr<QgsMapMouseEvent> event;
      for ( const QgsPoint &point : it.value() )
      {
        event.reset( new QgsMapMouseEvent(
          mCanvas,
          QEvent::MouseButtonRelease,
          mapToPoint( point.x(), point.y() ),
          Qt::LeftButton
        ) );
        mCaptureTool->cadCanvasReleaseEvent( event.get() );
      }
      event.reset( new QgsMapMouseEvent(
        mCanvas,
        QEvent::MouseButtonRelease,
        mapToPoint( 0, 0 ),
        Qt::RightButton
      ) );
      mCaptureTool->cadCanvasReleaseEvent( event.get() );

      QVERIFY2( !vl->getFeature( 1 ).geometry().isNull(), QString( "failed for %1" ).arg( geomType ).toLocal8Bit().data() );
    }
  }
}

QGSTEST_MAIN( TestQgsMapToolAddPart )
#include "testqgsmaptooladdpart.moc"
