/***************************************************************************
     testqgsmaptooladdfeaturepoint.cpp
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
#include "qgssnappingconfig.h"
#include "qgssnappingutils.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvastracer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolAddFeaturePoint : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeaturePoint();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testPointZ();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerPointZSnap = nullptr;
    QgsVectorLayer *mLayerPointZ = nullptr;
};

TestQgsMapToolAddFeaturePoint::TestQgsMapToolAddFeaturePoint() = default;


//runs before all tests
void TestQgsMapToolAddFeaturePoint::initTestCase()
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

  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();

  // make testing layers
  mLayerPointZ = new QgsVectorLayer( QStringLiteral( "PointZ?crs=EPSG:27700" ), QStringLiteral( "layer point Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPointZ->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPointZ );

  mLayerPointZ->startEditing();
  QgsFeature pointFZ;
  QString pointWktZ = "PointZ(7 7 4)";
  pointFZ.setGeometry( QgsGeometry::fromWkt( pointWktZ ) );

  mLayerPointZ->addFeature( pointFZ );
  QCOMPARE( mLayerPointZ->featureCount(), ( long )1 );

  // make layer for snapping
  mLayerPointZSnap = new QgsVectorLayer( QStringLiteral( "PointZ?crs=EPSG:27700" ), QStringLiteral( "Snap point" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPointZSnap->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPointZSnap );

  mLayerPointZSnap->startEditing();
  QgsFeature pointF;
  QString pointWktZSnap = "PointZ(6 6 3)";
  pointF.setGeometry( QgsGeometry::fromWkt( pointWktZSnap ) );

  mLayerPointZSnap->addFeature( pointF );
  QCOMPARE( mLayerPointZSnap->featureCount(), ( long )1 );

  QgsSnappingConfig cfg = mCanvas->snappingUtils()->config();
  cfg.setMode( QgsSnappingConfig::AllLayers );
  cfg.setTolerance( 100 );
  cfg.setType( QgsSnappingConfig::VertexAndSegment );
  cfg.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( cfg );

  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayerPointZ << mLayerPointZSnap );
  mCanvas->setCurrentLayer( mLayerPointZ );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, /*mAdvancedDigitizingDockWidget, */ QgsMapToolCapture::CapturePoint );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeaturePoint::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeaturePoint::testPointZ()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );

  // test with default Z value = 333
  QgsSettings().setValue( QStringLiteral( "/qgis/digitizing/default_z_value" ), 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 4, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerPointZ->featureCount(), ( long )2 );

  QString wkt = "PointZ (4 0 333)";
  QCOMPARE( mLayerPointZ->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointZ->undoStack()->undo();

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  newFid = utils.newFeatureId( oldFids );

  wkt = "PointZ (6 6 3)";
  QCOMPARE( mLayerPointZ->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointZ->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolAddFeaturePoint )
#include "testqgsmaptooladdfeaturepoint.moc"
