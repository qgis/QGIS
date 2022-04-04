/***************************************************************************
     testqgsmaptoolcircularstring.cpp
     --------------------------------
    Date                 : January 2018
    Copyright            : (C) 2018 by Paul Blottiere
    Email                : paul.blottiere@oslandia.com
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
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmaptooladdfeature.h"

#include "testqgsmaptoolutils.h"
#include "qgsmaptoolshapecircularstringradius.h"


class TestQgsMapToolCircularString : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolCircularString();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void testAddCircularStringCurvePoint();
    void testAddCircularStringRadius();
    void testAddCircularStringRadiusWithDeletedVertex();
    void testAddCircularStringAfterClassicDigitizing();

  private:
    void resetMapTool( QgsMapToolShapeMetadata *metadata );

    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer = nullptr;
};

TestQgsMapToolCircularString::TestQgsMapToolCircularString() = default;


//runs before all tests
void TestQgsMapToolCircularString::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  mLayer = new QgsVectorLayer( QStringLiteral( "CompoundCurveZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer->isValid() );
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLayer );

  // set layers in canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer );
  mCanvas->setCurrentLayer( mLayer );

  mMapTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
//  mCanvas->setMapTool( mMapTool );
}

void TestQgsMapToolCircularString::cleanupTestCase()
{
  QgsApplication::exitQgis();
  delete mMapTool;
}

void TestQgsMapToolCircularString::cleanup()
{
  mMapTool->clean();
}

void TestQgsMapToolCircularString::resetMapTool( QgsMapToolShapeMetadata *metadata )
{
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mMapTool->setCurrentShapeMapTool( metadata ) ;
}

void TestQgsMapToolCircularString::testAddCircularStringCurvePoint()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::CircularString );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "CompoundCurveZ (CircularStringZ (0 0 333, 1 1 333, 0 2 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircularString::testAddCircularStringRadius()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeCircularStringRadiusMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "CompoundCurveZ (CircularStringZ (0 0 111, 0.17912878474779187 0.82087121525220819 111, 1 1 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircularString::testAddCircularStringRadiusWithDeletedVertex()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 111 );
  mLayer->startEditing();

  QgsMapToolShapeCircularStringRadiusMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 2, Qt::RightButton );
  const QgsFeatureId newFid = utils.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  const QString wkt = "CompoundCurveZ (CircularStringZ (0 0 111, 0.17912878474779187 0.82087121525220819 111, 1 1 111))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}

void TestQgsMapToolCircularString::testAddCircularStringAfterClassicDigitizing()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 333 );
  mLayer->startEditing();

  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );

  TestQgsMapToolAdvancedDigitizingUtils utilsClassic( mMapTool );
  utilsClassic.mouseClick( 2, 1, Qt::LeftButton );
  utilsClassic.mouseClick( 2, 0, Qt::LeftButton );
  utilsClassic.mouseClick( 0, 0, Qt::LeftButton );

  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::CircularString );

  TestQgsMapToolAdvancedDigitizingUtils utilsCircular( mMapTool );
  utilsCircular.mouseClick( 1, 1, Qt::LeftButton );
  utilsCircular.mouseClick( 0, 2, Qt::LeftButton );

  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::StraightSegments );
  utilsClassic.mouseClick( 2, 2, Qt::LeftButton );
  utilsClassic.mouseClick( 4, 2, Qt::LeftButton );

  utilsCircular.mouseClick( 4, 2, Qt::RightButton );
  const QgsFeatureId newFid = utilsCircular.newFeatureId();

  QCOMPARE( mLayer->featureCount(), ( long )1 );
  const QgsFeature f = mLayer->getFeature( newFid );

  qDebug() << f.geometry().asWkt();

  const QString wkt = "CompoundCurveZ ((2 1 333, 2 0 333, 0 0 333),CircularStringZ (0 0 333, 1 1 333, 0 2 333),(0 2 333, 2 2 333, 4 2 333))";
  QCOMPARE( f.geometry().asWkt(), wkt );

  mLayer->rollBack();
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
}
QGSTEST_MAIN( TestQgsMapToolCircularString )
#include "testqgsmaptoolcircularstring.moc"
