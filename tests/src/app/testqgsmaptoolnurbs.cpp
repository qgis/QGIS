/***************************************************************************
    testqgsmaptoolnurbs.cpp
    -----------------------
    Date                 : December 2025
    Copyright            : (C) 2025 by Loïc Bartoletti
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
#include "qgsmaptooladdfeature.h"
#include "qgsmaptoolcapture.h"
#include "qgsnurbscurve.h"
#include "qgssettingsentryenumflag.h"
#include "qgssettingsregistrycore.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

class TestQgsMapToolNurbs : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolNurbs() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testNurbsControlPointsMode();
    void testNurbsPolyBezierMode();
    void testNurbsControlPointsNotEnoughPoints();
    void testNurbsPolyBezierNotEnoughPoints();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    std::unique_ptr<QgsVectorLayer> mLineLayer;

    void resetMapTool( Qgis::NurbsMode mode );
};

void TestQgsMapToolNurbs::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  mLineLayer = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line" ), QStringLiteral( "memory" ) );
  QVERIFY( mLineLayer->isValid() );

  QgsProject::instance()->addMapLayers( { mLineLayer.get() } );
  mCanvas->setLayers( { mLineLayer.get() } );
  mCanvas->setCurrentLayer( mLineLayer.get() );

  mMapTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mCanvas->setMapTool( mMapTool );
}

void TestQgsMapToolNurbs::cleanupTestCase()
{
  delete mMapTool;
  mLineLayer.reset();

  QgsApplication::exitQgis();
}

void TestQgsMapToolNurbs::resetMapTool( Qgis::NurbsMode mode )
{
  mMapTool->clean();
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::NurbsCurve );
  QgsSettingsRegistryCore::settingsDigitizingNurbsMode->setValue( mode );
  QgsSettingsRegistryCore::settingsDigitizingNurbsDegree->setValue( 3 );
}

void TestQgsMapToolNurbs::testNurbsControlPointsMode()
{
  mLineLayer->startEditing();
  mLineLayer->dataProvider()->truncate();

  resetMapTool( Qgis::NurbsMode::ControlPoints );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 5, 10, Qt::LeftButton );
  utils.mouseClick( 10, 5, Qt::LeftButton );
  utils.mouseClick( 15, 10, Qt::LeftButton );
  utils.mouseMove( 20, 0 );
  utils.mouseClick( 20, 0, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId();
  const QgsFeature f = mLineLayer->getFeature( newFid );

  QVERIFY( f.isValid() );
  QVERIFY( !f.geometry().isNull() );

  const QgsAbstractGeometry *geom = f.geometry().constGet();
  QVERIFY( geom != nullptr );

  QCOMPARE( mLineLayer->featureCount(), 1LL );

  mLineLayer->rollBack();
}

void TestQgsMapToolNurbs::testNurbsPolyBezierMode()
{
  mLineLayer->startEditing();
  mLineLayer->dataProvider()->truncate();

  resetMapTool( Qgis::NurbsMode::PolyBezier );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 5, 5 );
  utils.mouseClick( 10, 0, Qt::LeftButton );
  utils.mouseMove( 15, -5 );
  utils.mouseClick( 20, 0, Qt::RightButton );

  const QgsFeatureId newFid = utils.newFeatureId();
  const QgsFeature f = mLineLayer->getFeature( newFid );

  QVERIFY( f.isValid() );
  QVERIFY( !f.geometry().isNull() );

  const QgsAbstractGeometry *geom = f.geometry().constGet();
  QVERIFY( geom != nullptr );

  QCOMPARE( mLineLayer->featureCount(), 1LL );

  mLineLayer->rollBack();
}

void TestQgsMapToolNurbs::testNurbsControlPointsNotEnoughPoints()
{
  mLineLayer->startEditing();
  mLineLayer->dataProvider()->truncate();
  const long long count = mLineLayer->featureCount();

  resetMapTool( Qgis::NurbsMode::ControlPoints );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );

  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( mLineLayer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( mLineLayer->featureCount(), count );

  mLineLayer->rollBack();
}

void TestQgsMapToolNurbs::testNurbsPolyBezierNotEnoughPoints()
{
  mLineLayer->startEditing();
  mLineLayer->dataProvider()->truncate();
  const long long count = mLineLayer->featureCount();

  resetMapTool( Qgis::NurbsMode::PolyBezier );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );

  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( mLineLayer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( mLineLayer->featureCount(), count );

  mLineLayer->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolNurbs )
#include "testqgsmaptoolnurbs.moc"
