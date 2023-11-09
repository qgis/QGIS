/***************************************************************************
     testqgsmaptoolsplitparts.cpp
     ------------------------
    Date                 : February 2020
    Copyright            : (C) 2020 by Loïc Bartoletti
    Email                : loic.bartoletti@oslandia.com
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
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmaptoolsplitparts.h"
#include "qgsgeometryutils.h"

#include "testqgsmaptoolutils.h"

class TestQgsMapToolSplitParts : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolSplitParts();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testSplitMultiLineString();

  private:
    QPoint mapToPoint( double x, double y );
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mMultiLineStringLayer = nullptr;
    QgsFeature lineF1, lineF2;
};

TestQgsMapToolSplitParts::TestQgsMapToolSplitParts() = default;


//runs before all tests
void TestQgsMapToolSplitParts::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );

  // make testing layers
  mMultiLineStringLayer = new QgsVectorLayer( QStringLiteral( "MultiLineString?crs=EPSG:3946" ), QStringLiteral( "layer multiline" ), QStringLiteral( "memory" ) );
  QVERIFY( mMultiLineStringLayer->isValid() );
  mMultiLineStringLayer->startEditing();
  lineF1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((0 0, 10 0))" ) ) );
  lineF2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "MultiLineString ((0 5, 10 5),(10 5, 15 5))" ) ) );
  mMultiLineStringLayer->addFeature( lineF1 );
  mMultiLineStringLayer->addFeature( lineF2 );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 50, 50 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 10, 10 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
  // Disable flaky tests on windows...
  // QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 50, 50 ) );
  // QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 10, 10 ) );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mMultiLineStringLayer );
}

void TestQgsMapToolSplitParts::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QPoint TestQgsMapToolSplitParts::mapToPoint( double x, double y )
{

  const QgsPointXY mapPoint = mCanvas->mapSettings().mapToPixel().transform( x, y );

  return QPoint( std::round( mapPoint.x() ), std::round( mapPoint.y() ) );
}

void TestQgsMapToolSplitParts::testSplitMultiLineString()
{

  mCanvas->setCurrentLayer( mMultiLineStringLayer );
  QgsMapToolSplitParts *mapTool = new QgsMapToolSplitParts( mCanvas ) ;
  mCanvas->setMapTool( mapTool );

  std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
        mCanvas,
        QEvent::MouseButtonRelease,
        mapToPoint( 4, 7 ),
        Qt::LeftButton
      ) );
  mapTool->cadCanvasReleaseEvent( event.get() );
  event.reset( new QgsMapMouseEvent(
                 mCanvas,
                 QEvent::MouseButtonRelease,
                 mapToPoint( 4, -1 ),
                 Qt::LeftButton
               ) );
  mapTool->cadCanvasReleaseEvent( event.get() );

  event.reset( new QgsMapMouseEvent(
                 mCanvas,
                 QEvent::MouseButtonRelease,
                 mapToPoint( 4, -1 ),
                 Qt::RightButton
               ) );
  mapTool->cadCanvasReleaseEvent( event.get() );


  QCOMPARE( mMultiLineStringLayer->featureCount(), ( long )2 );
  QCOMPARE( mMultiLineStringLayer->getFeature( lineF1.id() ).geometry().asWkt(), QStringLiteral( "MultiLineString ((0 0, 4 0),(4 0, 10 0))" ) );
  QCOMPARE( mMultiLineStringLayer->getFeature( lineF2.id() ).geometry().asWkt(), QStringLiteral( "MultiLineString ((0 5, 4 5),(4 5, 10 5),(10 5, 15 5))" ) );

  mMultiLineStringLayer->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolSplitParts )
#include "testqgsmaptoolsplitparts.moc"
