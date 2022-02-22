/***************************************************************************
     testqgsmaptooladdfeaturepointzm.cpp
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
class TestQgsMapToolAddFeaturePointZM : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapToolAddFeaturePointZM();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testPointZM();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsVectorLayer *mLayerPointZM = nullptr;
};

TestQgsMapToolAddFeaturePointZM::TestQgsMapToolAddFeaturePointZM() = default;


//runs before all tests
void TestQgsMapToolAddFeaturePointZM::initTestCase()
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

  // make testing ZM layer
  mLayerPointZM = new QgsVectorLayer( QStringLiteral( "PointZM?crs=EPSG:27700" ), QStringLiteral( "layer point ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayerPointZM->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayerPointZM );

  mLayerPointZM->startEditing();
  QgsFeature pointFZM;
  const QString pointWktZM = "PointZM(7 7 4 3)";
  pointFZM.setGeometry( QgsGeometry::fromWkt( pointWktZM ) );

  mLayerPointZM->addFeature( pointFZM );
  QCOMPARE( mLayerPointZM->featureCount(), ( long )1 );

  // create the tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CapturePoint );
  mCanvas->setMapTool( mCaptureTool );

  QCOMPARE( mCanvas->mapSettings().outputSize(), QSize( 512, 512 ) );
  QCOMPARE( mCanvas->mapSettings().visibleExtent(), QgsRectangle( 0, 0, 8, 8 ) );
}

//runs after all tests
void TestQgsMapToolAddFeaturePointZM::cleanupTestCase()
{
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsMapToolAddFeaturePointZM::testPointZM()
{
  TestQgsMapToolAdvancedDigitizingUtils utils( mCaptureTool );
  mCanvas->setCurrentLayer( mLayerPointZM );
  // test with default Z value = 123
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 123 );
  // test with default M value = 333
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 333 );

  QSet<QgsFeatureId> oldFids = utils.existingFeatureIds();

  utils.mouseClick( 4, 0, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  QgsFeatureId newFid = utils.newFeatureId( oldFids );

  QCOMPARE( mLayerPointZM->featureCount(), ( long )2 );

  QString wkt = "PointZM (4 0 123 333)";
  QCOMPARE( mLayerPointZM->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointZM->undoStack()->undo();

  // test with default Z value = 345
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 345 );
  // test with default M value = 123
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 123 );

  oldFids = utils.existingFeatureIds();
  utils.mouseClick( 6, 6, Qt::LeftButton, Qt::KeyboardModifiers(), true );
  newFid = utils.newFeatureId( oldFids );

  wkt = "PointZM (6 6 345 123)";
  QCOMPARE( mLayerPointZM->getFeature( newFid ).geometry().asWkt(), wkt );

  mLayerPointZM->undoStack()->undo();
}

QGSTEST_MAIN( TestQgsMapToolAddFeaturePointZM )
#include "testqgsmaptooladdfeaturepointzm.moc"
