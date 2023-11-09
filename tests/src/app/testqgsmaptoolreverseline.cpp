/***************************************************************************
     TestQgsMapToolReverseLine.cpp
     --------------------------------
    Date                 : May 2018
    Copyright            : (C) 2018 by Loïc Bartoletti
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
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmaptooladdfeature.h"

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolreverseline.h"
#include "qgsmapmouseevent.h"
#include "qgssnappingutils.h"

class TestQgsMapToolReverseLine : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolReverseLine();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testReverseCurve();
    void testReverseLineString();
    void testReverseMultiLineString();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
};

TestQgsMapToolReverseLine::TestQgsMapToolReverseLine() = default;


//runs before all tests
void TestQgsMapToolReverseLine::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3946" ) ) );
}

void TestQgsMapToolReverseLine::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolReverseLine::testReverseCurve()
{
  //create a temporary layer
  std::unique_ptr< QgsVectorLayer > memoryLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature curve( memoryLayer->dataProvider()->fields(), 1 );

  curve.setAttribute( QStringLiteral( "pk" ), 1 );
  curve.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
                       "CircularString(10 10, 5 5)" ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << curve );

  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );

  std::unique_ptr< QgsMapToolReverseLine > tool( new QgsMapToolReverseLine( mCanvas ) );

  memoryLayer->startEditing();
  const QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 5, 5 );
  const std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
        mCanvas,
        QEvent::MouseButtonRelease,
        QPoint( mapPoint.x(), mapPoint.y() )
      ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );
  const QgsFeature f = memoryLayer->getFeature( 1 );

  const QString wkt = "CircularString (5 5, 10 10)";
  QCOMPARE( f.geometry().asWkt(), wkt );
  memoryLayer->rollBack();

}

void TestQgsMapToolReverseLine::testReverseLineString()
{
  //create a temporary layer
  std::unique_ptr< QgsVectorLayer > memoryLayer( new QgsVectorLayer( QStringLiteral( "LineStringZ?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature line( memoryLayer->dataProvider()->fields(), 1 );

  line.setAttribute( QStringLiteral( "pk" ), 1 );
  line.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
      "LineStringZ(0 0 0, 10 10 10, 5 5 5)" ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << line );
  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );


  std::unique_ptr< QgsMapToolReverseLine > tool( new QgsMapToolReverseLine( mCanvas ) );
  memoryLayer->startEditing();
  const QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 6, 6 );
  const std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
        mCanvas,
        QEvent::MouseButtonRelease,
        QPoint( mapPoint.x(), mapPoint.y() )
      ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );

  const QgsFeature f = memoryLayer->getFeature( 1 );

  const QString wkt = "LineStringZ (5 5 5, 10 10 10, 0 0 0)";
  QCOMPARE( f.geometry().asWkt(), wkt );

  memoryLayer->rollBack();
}

void TestQgsMapToolReverseLine::testReverseMultiLineString()
{
  //create a temporary layer
  std::unique_ptr< QgsVectorLayer > memoryLayer( new QgsVectorLayer( QStringLiteral( "MultiLineStringZ?crs=EPSG:3946&field=pk:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature multi( memoryLayer->dataProvider()->fields(), 1 );

  multi.setAttribute( QStringLiteral( "pk" ), 1 );
  multi.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
                       "MultiLineStringZ((0 0 0, 10 10 10, 5 5 5), (100 100 100, 120 120 120))" ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << multi );
  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );

  std::unique_ptr< QgsMapToolReverseLine > tool( new QgsMapToolReverseLine( mCanvas ) );

  memoryLayer->startEditing();
  QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 6, 6 );
  std::unique_ptr< QgsMapMouseEvent > event( new QgsMapMouseEvent(
        mCanvas,
        QEvent::MouseButtonRelease,
        QPoint( mapPoint.x(), mapPoint.y() )
      ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );
  QgsFeature f = memoryLayer->getFeature( 1 );

  QString wkt = "MultiLineStringZ ((5 5 5, 10 10 10, 0 0 0),(100 100 100, 120 120 120))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mapPoint = mCanvas->getCoordinateTransform()->transform( 110, 110 );
  event.reset( new QgsMapMouseEvent(
                 mCanvas,
                 QEvent::MouseButtonRelease,
                 QPoint( mapPoint.x(), mapPoint.y() )
               ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );
  f = memoryLayer->getFeature( 1 );

  wkt = "MultiLineStringZ ((5 5 5, 10 10 10, 0 0 0),(120 120 120, 100 100 100))";
  QCOMPARE( f.geometry().asWkt(), wkt );
  memoryLayer->rollBack();
}
QGSTEST_MAIN( TestQgsMapToolReverseLine )
#include "testqgsmaptoolreverseline.moc"
