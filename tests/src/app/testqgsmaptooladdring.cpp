/***************************************************************************
     testqgsmaptooladdring.cpp
     --------------------------------
    Date                 : 2023-11-22
    Copyright            : (C) 2023 by Loïc Bartoletti
    Email                : loic dot bartoletti at oslandia dot com
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
#include "qgsmaptooladdring.h"
#include "qgsproject.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the add ring map tool
 */
class TestQgsMapToolAddRing : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddRing();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testAddRing();
    void testAddRingClockWise();

  private:
    QPoint mapToPoint( double x, double y );

    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddRing *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerMultiPolygon = nullptr;
};

TestQgsMapToolAddRing::TestQgsMapToolAddRing() = default;


//runs before all tests
void TestQgsMapToolAddRing::initTestCase()
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
  const QString wkt( "MultiPolygon (((0 0, 5 0, 5 5, 0 5, 0 0)))" );
  f.setGeometry( QgsGeometry::fromWkt( wkt ) );
  mLayerMultiPolygon->dataProvider()->addFeatures( QgsFeatureList() << f );
  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 1 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );

  mCanvas->setCurrentLayer( mLayerMultiPolygon );

  // create the tool
  mCaptureTool = new QgsMapToolAddRing( mCanvas );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddRing::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

QPoint TestQgsMapToolAddRing::mapToPoint( double x, double y )
{
  const QgsPointXY mapPoint = mCanvas->mapSettings().mapToPixel().transform( x, y );

  return QPoint( static_cast<int>( std::round( mapPoint.x() ) ), static_cast<int>( std::round( mapPoint.y() ) ) );
}

void TestQgsMapToolAddRing::testAddRing()
{
  mLayerMultiPolygon->select( 1 );

  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 1, 1 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 1, 2 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 2, 2 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 2, 1 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 1, 1 ),
    Qt::RightButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  const QString wkt = "MultiPolygon (((0 0, 5 0, 5 5, 0 5, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1)))";
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );
}

void TestQgsMapToolAddRing::testAddRingClockWise()
{
  mLayerMultiPolygon->select( 1 );

  // Draw in clockwise
  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 3, 3 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 4, 3 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 4, 4 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 3, 4 ),
    Qt::LeftButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( 3, 3 ),
    Qt::RightButton
  ) );
  mCaptureTool->cadCanvasReleaseEvent( event.get() );

  const QString wkt = "MultiPolygon (((0 0, 5 0, 5 5, 0 5, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(3 3, 3 4, 4 4, 4 3, 3 3)))";
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), wkt );
}
QGSTEST_MAIN( TestQgsMapToolAddRing )
#include "testqgsmaptooladdring.moc"
