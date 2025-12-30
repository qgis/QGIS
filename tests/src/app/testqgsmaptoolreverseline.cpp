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

#include <memory>

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmaptoolreverseline.h"
#include "qgssettings.h"
#include "qgssnappingutils.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

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
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );
}

void TestQgsMapToolReverseLine::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMapToolReverseLine::testReverseCurve()
{
  //create a temporary layer
  auto memoryLayer = std::make_unique<QgsVectorLayer>( u"LineString?crs=EPSG:3946&field=pk:int"_s, u"vl"_s, u"memory"_s );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature curve( memoryLayer->dataProvider()->fields(), 1 );

  curve.setAttribute( u"pk"_s, 1 );
  curve.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
    "CircularString(10 10, 5 5)"
  ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << curve );

  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );

  auto tool = std::make_unique<QgsMapToolReverseLine>( mCanvas );

  memoryLayer->startEditing();
  const QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 5, 5 );
  const std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
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
  auto memoryLayer = std::make_unique<QgsVectorLayer>( u"LineStringZ?crs=EPSG:3946&field=pk:int"_s, u"vl"_s, u"memory"_s );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature line( memoryLayer->dataProvider()->fields(), 1 );

  line.setAttribute( u"pk"_s, 1 );
  line.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
    "LineStringZ(0 0 0, 10 10 10, 5 5 5)"
  ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << line );
  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );


  auto tool = std::make_unique<QgsMapToolReverseLine>( mCanvas );
  memoryLayer->startEditing();
  const QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 6, 6 );
  const std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    QPoint( mapPoint.x(), mapPoint.y() )
  ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );

  const QgsFeature f = memoryLayer->getFeature( 1 );

  const QString wkt = "LineString Z (5 5 5, 10 10 10, 0 0 0)";
  QCOMPARE( f.geometry().asWkt(), wkt );

  memoryLayer->rollBack();
}

void TestQgsMapToolReverseLine::testReverseMultiLineString()
{
  //create a temporary layer
  auto memoryLayer = std::make_unique<QgsVectorLayer>( u"MultiLineStringZ?crs=EPSG:3946&field=pk:int"_s, u"vl"_s, u"memory"_s );
  QVERIFY( memoryLayer->isValid() );
  QgsFeature multi( memoryLayer->dataProvider()->fields(), 1 );

  multi.setAttribute( u"pk"_s, 1 );
  multi.setGeometry( QgsGeometry::fromWkt( QStringLiteral(
    "MultiLineString Z((0 0 0, 10 10 10, 5 5 5), (100 100 100, 120 120 120))"
  ) ) );

  memoryLayer->dataProvider()->addFeatures( QgsFeatureList() << multi );
  mCanvas->setLayers( QList<QgsMapLayer *>() << memoryLayer.get() );
  mCanvas->setCurrentLayer( memoryLayer.get() );

  auto tool = std::make_unique<QgsMapToolReverseLine>( mCanvas );

  memoryLayer->startEditing();
  QgsPointXY mapPoint = mCanvas->getCoordinateTransform()->transform( 6, 6 );
  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonRelease,
    QPoint( mapPoint.x(), mapPoint.y() )
  ) );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );
  QgsFeature f = memoryLayer->getFeature( 1 );

  QString wkt = "MultiLineString Z ((5 5 5, 10 10 10, 0 0 0),(100 100 100, 120 120 120))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mapPoint = mCanvas->getCoordinateTransform()->transform( 110, 110 );
  event = std::make_unique<QgsMapMouseEvent>(
    mCanvas,
    QEvent::MouseButtonRelease,
    QPoint( mapPoint.x(), mapPoint.y() )
  );
  // trigger mouseRelease handler
  tool->canvasPressEvent( event.get() );
  tool->canvasReleaseEvent( event.get() );
  f = memoryLayer->getFeature( 1 );

  wkt = "MultiLineString Z ((5 5 5, 10 10 10, 0 0 0),(120 120 120, 100 100 100))";
  QCOMPARE( f.geometry().asWkt(), wkt );
  memoryLayer->rollBack();
}
QGSTEST_MAIN( TestQgsMapToolReverseLine )
#include "testqgsmaptoolreverseline.moc"
