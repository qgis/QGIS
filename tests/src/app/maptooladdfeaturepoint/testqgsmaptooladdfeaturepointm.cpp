/***************************************************************************
     testqgsmaptooladdfeaturepointm.cpp
     ----------------------
    Date                 : April 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
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
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"
#include "testqgsmaptoolutils.h"


/**
 * \ingroup UnitTests
 * This is a unit test for the vertex tool
 */
class TestQgsMapToolAddFeaturePointM : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeaturePointM();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testPointM();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerPointM = nullptr;
};

TestQgsMapToolAddFeaturePointM::TestQgsMapToolAddFeaturePointM() = default;


//runs before all tests
void TestQgsMapToolAddFeaturePointM::initTestCase()
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

  // make testing M layer
  mLayerPointM = new QgsVectorLayer( QStringLiteral( "PointM?crs=EPSG:27700" ), QStringLiteral( "layer point M" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPointM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPointM );

  mLayerPointM->startEditing();
  QgsFeature pointFM;
  const QString pointWktM = "PointM(7 7 4)";
  pointFM.setGeometry( QgsGeometry::fromWkt( pointWktM ) );

  mLayerPointM->addFeature( pointFM );
  QCOMPARE( mLayerPointM->featureCount(), ( long )1 );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CapturePoint );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeaturePointM::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeaturePointM::testPointM()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerPointM );
  // test with default M value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 4, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerPointM->featureCount(), ( long )2 );

  QString wkt = "PointM (4 0 333)";
  QCOMPARE( mLayerPointM->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointM->undoStack()->undo();

  // test with default M value = 123
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 123 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  newFid = utils.newFeatureId( oldFids );

  wkt = "PointM (6 6 123)";
  QCOMPARE( mLayerPointM->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointM->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolAddFeaturePointM )
#include "testqgsmaptooladdfeaturepointm.moc"
