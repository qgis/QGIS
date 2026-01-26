/***************************************************************************
     testqgsmaptooldeletepart.cpp
     --------------------------------
    Date                 : 2024-02-19
    Copyright            : (C) 2024 by Stefanos Natsis
    Email                : uclaros at gmail dot com
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
#include "qgsmaptooldeletepart.h"
#include "qgsproject.h"
#include "qgssettingsregistrycore.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the delete part map tool
 */
class TestQgsMapToolDeletePart : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolDeletePart();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void cleanup();         // will be called after every testfunction.

    void testDeletePart();
    void testDeleteLastPart();
    void testDeletePartSelected();

  private:
    void click( double x, double y );
    QPoint mapToPoint( double x, double y );

    QString mWkt1, mWkt2, mWkt3, mWkt4, mWkt5, mWkt6;

    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolDeletePart *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerMultiPolygon = nullptr;
    QgsVectorLayer *mLayerPolygon = nullptr;
};

TestQgsMapToolDeletePart::TestQgsMapToolDeletePart() = default;


//runs before all tests
void TestQgsMapToolDeletePart::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();

  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3946"_s ) );

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerMultiPolygon = new QgsVectorLayer( u"MultiPolygon?crs=EPSG:3946"_s, u"multipolygon"_s, u"memory"_s );
  QVERIFY( mLayerMultiPolygon->isValid() );
  mLayerPolygon = new QgsVectorLayer( u"Polygon?crs=EPSG:3946"_s, u"polygon"_s, u"memory"_s );
  QVERIFY( mLayerMultiPolygon->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerMultiPolygon << mLayerPolygon );

  mLayerMultiPolygon->startEditing();
  QgsFeature f1, f2, f3;
  mWkt1 = u"MultiPolygon (((0 0, 4 0, 4 7, 0 7, 0 0),(1 1, 1 6, 3 6, 3 1, 1 1)))"_s;
  mWkt2 = u"MultiPolygon (((0 0, 0 3, 7 3, 7 0, 0 0),(1 2, 1 1, 2 1, 2 2, 1 2),(5 1, 6 1, 6 2, 5 2, 5 1)),((0 4, 0 7, 7 7, 7 4, 0 4),(1 6, 1 5, 2 5, 2 6, 1 6),(5 6, 5 5, 6 5, 6 6, 5 6)))"_s;
  mWkt3 = u"MultiPolygon (((6 4, 7 4, 7 3, 6 3, 6 4)))"_s;
  f1.setGeometry( QgsGeometry::fromWkt( mWkt1 ) );
  f2.setGeometry( QgsGeometry::fromWkt( mWkt2 ) );
  f3.setGeometry( QgsGeometry::fromWkt( mWkt3 ) );
  mLayerMultiPolygon->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 );
  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), mWkt1 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 2 ).geometry().asWkt(), mWkt2 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 3 ).geometry().asWkt(), mWkt3 );

  mLayerPolygon->startEditing();
  mWkt4 = u"Polygon ((0 0, 4 0, 4 7, 0 7, 0 0),(1 1, 1 6, 3 6, 3 1, 1 1))"_s;
  mWkt5 = u"MultiPolygon (((0 0, 0 3, 7 3, 7 0, 0 0),(1 2, 1 1, 2 1, 2 2, 1 2),(5 1, 6 1, 6 2, 5 2, 5 1)),((0 4, 0 7, 7 7, 7 4, 0 4),(1 6, 1 5, 2 5, 2 6, 1 6),(5 6, 5 5, 6 5, 6 6, 5 6)))"_s;
  mWkt6 = u"Polygon ((6 4, 7 4, 7 3, 6 3, 6 4))"_s;
  f1.setGeometry( QgsGeometry::fromWkt( mWkt4 ) );
  f2.setGeometry( QgsGeometry::fromWkt( mWkt5 ) );
  f3.setGeometry( QgsGeometry::fromWkt( mWkt6 ) );
  mLayerPolygon->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 );
  QCOMPARE( mLayerPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerPolygon->getFeature( 1 ).geometry().asWkt(), mWkt4 );
  QCOMPARE( mLayerPolygon->getFeature( 2 ).geometry().asWkt(), mWkt5 );
  QCOMPARE( mLayerPolygon->getFeature( 3 ).geometry().asWkt(), mWkt6 );

  // create the tool
  mCaptureTool = new QgsMapToolDeletePart( mCanvas );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolDeletePart::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolDeletePart::cleanup()
{
  mLayerMultiPolygon->undoStack()->setIndex( 0 );
  mLayerPolygon->undoStack()->setIndex( 0 );
}

void TestQgsMapToolDeletePart::click( double x, double y )
{
  std::unique_ptr<QgsMapMouseEvent> event( new QgsMapMouseEvent(
    mCanvas,
    QEvent::MouseButtonPress,
    mapToPoint( x, y ),
    Qt::LeftButton
  ) );
  mCaptureTool->canvasPressEvent( event.get() );
  event = std::make_unique<QgsMapMouseEvent>(
    mCanvas,
    QEvent::MouseButtonRelease,
    mapToPoint( x, y ),
    Qt::LeftButton
  );
  mCaptureTool->canvasReleaseEvent( event.get() );
}

QPoint TestQgsMapToolDeletePart::mapToPoint( double x, double y )
{
  const QgsPointXY mapPoint = mCanvas->mapSettings().mapToPixel().transform( x, y );

  return QPoint( static_cast<int>( std::round( mapPoint.x() ) ), static_cast<int>( std::round( mapPoint.y() ) ) );
}

void TestQgsMapToolDeletePart::testDeletePart()
{
  mCanvas->setCurrentLayer( mLayerMultiPolygon );
  click( 2.5, 2.5 );

  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 2 ).geometry().asWkt(), u"MultiPolygon (((0 4, 0 7, 7 7, 7 4, 0 4),(1 6, 1 5, 2 5, 2 6, 1 6),(5 6, 5 5, 6 5, 6 6, 5 6)))"_s );

  // delete a part from a multi polygon residing in a singlepart layer
  mCanvas->setCurrentLayer( mLayerPolygon );
  click( 2.5, 2.5 );

  QCOMPARE( mLayerPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerPolygon->getFeature( 2 ).geometry().asWkt(), u"MultiPolygon (((0 4, 0 7, 7 7, 7 4, 0 4),(1 6, 1 5, 2 5, 2 6, 1 6),(5 6, 5 5, 6 5, 6 6, 5 6)))"_s );
}

void TestQgsMapToolDeletePart::testDeleteLastPart()
{
  mCanvas->setCurrentLayer( mLayerMultiPolygon );
  click( 0.5, 3.5 );

  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QVERIFY2( mLayerMultiPolygon->getFeature( 1 ).geometry().isEmpty(), mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt().toLocal8Bit().constData() );

  // also for singlepart layer
  mCanvas->setCurrentLayer( mLayerPolygon );
  click( 0.5, 3.5 );

  QCOMPARE( mLayerPolygon->featureCount(), ( long ) 3 );
  QVERIFY2( mLayerPolygon->getFeature( 1 ).geometry().isEmpty(), mLayerPolygon->getFeature( 1 ).geometry().asWkt().toLocal8Bit().constData() );
}

void TestQgsMapToolDeletePart::testDeletePartSelected()
{
  mCanvas->setCurrentLayer( mLayerMultiPolygon );
  // test that only the selected feature is affected.
  mLayerMultiPolygon->select( 1 );
  click( 0.5, 0.5 );

  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QVERIFY2( mLayerMultiPolygon->getFeature( 1 ).geometry().isEmpty(), mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt().toLocal8Bit().constData() );
  QCOMPARE( mLayerMultiPolygon->getFeature( 2 ).geometry().asWkt(), mWkt2 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 3 ).geometry().asWkt(), mWkt3 );
  mLayerMultiPolygon->undoStack()->undo();
  mLayerMultiPolygon->removeSelection();

  mLayerMultiPolygon->select( 2 );
  click( 0.5, 0.5 );

  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), mWkt1 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 2 ).geometry().asWkt(), u"MultiPolygon (((0 4, 0 7, 7 7, 7 4, 0 4),(1 6, 1 5, 2 5, 2 6, 1 6),(5 6, 5 5, 6 5, 6 6, 5 6)))"_s );
  QCOMPARE( mLayerMultiPolygon->getFeature( 3 ).geometry().asWkt(), mWkt3 );
  mLayerMultiPolygon->undoStack()->undo();
  mLayerMultiPolygon->removeSelection();

  // no parts are deleted if a disjoint feature is selected.
  mLayerMultiPolygon->select( 3 );
  click( 0.5, 0.5 );

  QCOMPARE( mLayerMultiPolygon->featureCount(), ( long ) 3 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 1 ).geometry().asWkt(), mWkt1 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 2 ).geometry().asWkt(), mWkt2 );
  QCOMPARE( mLayerMultiPolygon->getFeature( 3 ).geometry().asWkt(), mWkt3 );
  mLayerMultiPolygon->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolDeletePart )
#include "testqgsmaptooldeletepart.moc"
