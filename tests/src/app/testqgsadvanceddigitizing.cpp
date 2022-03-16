/***************************************************************************
    testqgsadvanceddigitizing.cpp
     ----------------------
    Date                 : December 2021
    Copyright            : (C) 2021 Antoine Facchini
    Email                : antoine dot facchini at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsguiutils.h"
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmapcanvassnappingutils.h"
#include "qgssettingsregistrycore.h"

#include "testqgsmaptoolutils.h"


class TestQgsAdvancedDigitizing: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    TestQgsMapToolAdvancedDigitizingUtils getMapToolDigitizingUtils( QgsVectorLayer *layer );
    QString getWktFromLastAddedFeature( TestQgsMapToolAdvancedDigitizingUtils utils, QSet<QgsFeatureId> oldFeatures );
    void setCanvasCrs( QString crsString );

    void distanceConstraint();
    void distanceConstraintDiffCrs();
    void distanceConstraintWhenSnapping();

    void angleConstraint();
    void angleConstraintWithGeographicCrs();
    void distanceConstraintWithAngleConstraint();

    void coordinateConstraint();
    void coordinateConstraintWithZM();
    void coordinateConstraintWhenSnapping();

    void perpendicularConstraint();

    void cadPointList();
    void currentPointWhenSanpping();
    void currentPointWhenSanppingWithDiffCanvasCRS();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolAddFeature *mCaptureTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsVectorLayer *mLayer3950 = nullptr;
    QgsVectorLayer *mLayer3950ZM = nullptr;
    QgsVectorLayer *mLayer4326 = nullptr;
    QgsVectorLayer *mLayer4326ZM = nullptr;
    QgsAdvancedDigitizingDockWidget *mAdvancedDigitizingDockWidget = nullptr;

    const double WKT_PRECISION = 2;
};

void TestQgsAdvancedDigitizing::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();

  mCanvas = new QgsMapCanvas();
  setCanvasCrs( QStringLiteral( "EPSG:3950" ) );

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 33 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 66 );

  // make test layers
  QList<QgsMapLayer *> layers;

  mLayer3950 = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:3950" ),
                                   QStringLiteral( "line layer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer3950->isValid() );
  layers << mLayer3950;

  mLayer3950ZM = new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=EPSG:3950" ),
                                     QStringLiteral( "ZM line layer" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer3950ZM->isValid() );
  layers << mLayer3950ZM;

  mLayer4326 = new QgsVectorLayer( QStringLiteral( "LineString?crs=EPSG:4326" ),
                                   QStringLiteral( "line layer diff crs" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer4326->isValid() );
  layers << mLayer4326;

  mLayer4326ZM = new QgsVectorLayer( QStringLiteral( "LineStringZM?crs=EPSG:4326" ),
                                     QStringLiteral( "ZM line layer diff crs" ), QStringLiteral( "memory" ) );
  QVERIFY( mLayer4326ZM->isValid() );
  layers << mLayer4326ZM;

  // set layers in canvas
  QgsProject::instance()->addMapLayers( layers );
  mCanvas->setLayers( layers );
  mCanvas->setCurrentLayer( mLayer3950 );

  // create advanced digitizing dock widget
  mAdvancedDigitizingDockWidget =  new QgsAdvancedDigitizingDockWidget( mCanvas );

  // create snapping config
  QgsSnappingConfig snapConfig;
  snapConfig.setEnabled( false );
  snapConfig.setIntersectionSnapping( true );
  snapConfig.setSelfSnapping( true );
  snapConfig.setMode( Qgis::SnappingMode::AllLayers );
  snapConfig.setTypeFlag( Qgis::SnappingType::Vertex );
  snapConfig.setTolerance( 1.0 );

  QgsMapSettings mapSettings;
  mapSettings.setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mapSettings.setOutputSize( QSize( 512, 512 ) );
  mapSettings.setLayers( layers );

  QgsSnappingUtils *snappingUtils = new QgsMapCanvasSnappingUtils( mCanvas );
  snappingUtils->setConfig( snapConfig );
  snappingUtils->setMapSettings( mapSettings );

  snappingUtils->locatorForLayer( mLayer3950 )->init();
  snappingUtils->locatorForLayer( mLayer3950ZM )->init();
  snappingUtils->locatorForLayer( mLayer4326 )->init();
  snappingUtils->locatorForLayer( mLayer4326ZM )->init();

  mCanvas->setSnappingUtils( snappingUtils );

  // create base map tool
  mCaptureTool = new QgsMapToolAddFeature( mCanvas, mAdvancedDigitizingDockWidget, QgsMapToolCapture::CaptureLine );
  mCanvas->setMapTool( mCaptureTool );
}

void TestQgsAdvancedDigitizing::cleanupTestCase()
{
  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );

  delete mAdvancedDigitizingDockWidget;
  delete mCaptureTool;
  delete mCanvas;
  QgsApplication::exitQgis();
}

void TestQgsAdvancedDigitizing::init()
{
}

void TestQgsAdvancedDigitizing::cleanup()
{
  // reset the advanced digitizing dock
  mAdvancedDigitizingDockWidget->releaseLocks();
  mAdvancedDigitizingDockWidget->enableAction()->trigger();
  QVERIFY( !mAdvancedDigitizingDockWidget->cadEnabled() );

  // disable the snapping
  QgsSnappingUtils *snappingUtils = mCanvas->snappingUtils();
  QgsSnappingConfig snapConfig = snappingUtils->config();
  snapConfig.setEnabled( false );
  snapConfig.setIntersectionSnapping( true );
  snapConfig.setSelfSnapping( true );
  snapConfig.setMode( Qgis::SnappingMode::AllLayers );
  snapConfig.setTypeFlag( Qgis::SnappingType::Vertex );
  snappingUtils->setConfig( snapConfig );

  // reset all layers
  mLayer3950->rollBack();
  mLayer3950ZM->rollBack();
  mLayer4326->rollBack();
  mLayer4326ZM->rollBack();
}

TestQgsMapToolAdvancedDigitizingUtils TestQgsAdvancedDigitizing::getMapToolDigitizingUtils( QgsVectorLayer *layer )
{
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  mCaptureTool->setLayer( layer );

  // enable the advanced digitizing dock
  mAdvancedDigitizingDockWidget->enableAction()->trigger();

  return TestQgsMapToolAdvancedDigitizingUtils( mCaptureTool );
}

QString TestQgsAdvancedDigitizing::getWktFromLastAddedFeature( TestQgsMapToolAdvancedDigitizingUtils utils, QSet<QgsFeatureId> oldFeatures )
{
  auto layer = qobject_cast<const QgsVectorLayer *>( mCanvas->currentLayer() );

  const QgsFeatureId newFid = utils.newFeatureId( oldFeatures );
  QgsGeometry geom = layer->getFeature( newFid ).geometry();

  // transform the coordinates when canvas CRS and layer CRS are different
  const QgsCoordinateTransform transform( layer->sourceCrs(), mCanvas->mapSettings().destinationCrs(),
                                          QgsProject::instance() );
  geom.transform( transform, Qgis::TransformDirection::Forward );

  return geom.asWkt( WKT_PRECISION );
}

void TestQgsAdvancedDigitizing::setCanvasCrs( QString crsString )
{
  const QgsCoordinateReferenceSystem crs( crsString );
  QVERIFY( crs.isValid() );

  mCanvas->setDestinationCrs( crs );
  mCanvas->setFrameStyle( QFrame::NoFrame );
  mCanvas->resize( 512, 512 );
  mCanvas->setExtent( QgsRectangle( 0, 0, 8, 8 ) );
  mCanvas->show(); // to make the canvas resize
  mCanvas->hide();
}

void TestQgsAdvancedDigitizing::distanceConstraint()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  // with no digitized vertex
  auto capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );

  QVERIFY( !mAdvancedDigitizingDockWidget->mAngleLineEdit->isEnabled() );
  QVERIFY( !mAdvancedDigitizingDockWidget->mDistanceLineEdit->isEnabled() );

  // activate constraint on the second point (one digitized vertex)
  utils.mouseClick( 1, 1, Qt::LeftButton );

  capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "10" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 8.07 8.07)" ) );

  // activate constraint on the third point
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "5" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 2 2, 7 2)" ) );

  capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );
}

void TestQgsAdvancedDigitizing::distanceConstraintDiffCrs()
{
  auto utils = getMapToolDigitizingUtils( mLayer4326 );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  utils.mouseClick( 1, 1, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "10" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 8.07 8.07)" ) );

  // activate constraint on third point
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "5" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 2 2, 7 2)" ) );
}

void TestQgsAdvancedDigitizing::distanceConstraintWhenSnapping()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  // line for snapping
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 2 2)" ) );
  oldFeatures = utils.existingFeatureIds();

  // with no digitized vertex
  auto capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );

  QVERIFY( !mAdvancedDigitizingDockWidget->mAngleLineEdit->isEnabled() );
  QVERIFY( !mAdvancedDigitizingDockWidget->mDistanceLineEdit->isEnabled() );

  // activate constraint on the second point (one digitized vertex)
  utils.mouseClick( 1, 1, Qt::LeftButton );

  capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "10" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 8.07 8.07)" ) );

  // activate constraint on the third point
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "5" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 3, 2, Qt::LeftButton );
  utils.mouseClick( 1, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 2 2, 7 2)" ) );

  capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::Distance ) );
}

void TestQgsAdvancedDigitizing::angleConstraint()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  // with no digitized vertex
  auto capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::AbsoluteAngle ) );

  // try angle hard lock in a side
  utils.mouseClick( 1, 1, Qt::LeftButton );

  capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( capacities.testFlag( QgsAdvancedDigitizingDockWidget::AbsoluteAngle ) );

  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "90" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 1 2)" ) );

  // and in the other side
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 1, 1, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "90" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 2, -2, Qt::LeftButton );
  utils.mouseClick( 2, -2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 1 -2)" ) );

  // try with an angle of 45°
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "45" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );
  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, 1 1)" ) );
}

void TestQgsAdvancedDigitizing::angleConstraintWithGeographicCrs()
{
  setCanvasCrs( QStringLiteral( "EPSG:4326" ) );

  QVERIFY( mCanvas->mapSettings().destinationCrs().isGeographic() );

  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  utils.mouseClick( 1, 1, Qt::LeftButton );

  auto capacities = mAdvancedDigitizingDockWidget->capacities();
  QVERIFY( !capacities.testFlag( QgsAdvancedDigitizingDockWidget::AbsoluteAngle ) );

  QVERIFY( !mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() );

  utils.mouseClick( 0, 2, Qt::RightButton );

  utils.mouseClick( 1, 1, Qt::LeftButton );

  // constraint angle can be forced even with geographical
  // should be normal ?
  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "90" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );
  QVERIFY( mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() );

  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 1 2)" ) );

  setCanvasCrs( QStringLiteral( "EPSG:3950" ) );
}

void TestQgsAdvancedDigitizing::distanceConstraintWithAngleConstraint()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  // with these 2 next locks, there are only 2 possibilities
  // first
  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "10" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "45" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, 7.07 7.07)" ) );

  // second
  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setDistance( QStringLiteral( "10" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setAngle( QStringLiteral( "45" ),
      QgsAdvancedDigitizingDockWidget::ReturnPressed );

  utils.mouseClick( -1000, 59, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, -7.07 -7.07)" ) );
}


void TestQgsAdvancedDigitizing::coordinateConstraint()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setY( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 3, 9, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "6" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setY( QStringLiteral( "7" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 0, Qt::LeftButton );

  utils.mouseClick( 0, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (5 0, 3 5, 6 7)" ) );

  // set Z/M constraints should have no effect
  oldFeatures = utils.existingFeatureIds();
  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setZ( QStringLiteral( "3" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setM( QStringLiteral( "3" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 2, Qt::LeftButton );

  utils.mouseClick( 0, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, 0 2)" ) );
}


void TestQgsAdvancedDigitizing::coordinateConstraintWithZM()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950ZM );
  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 0, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setY( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 3, 9, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setZ( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 4, 4, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setM( QStringLiteral( "5" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 6, 6, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "9" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setY( QStringLiteral( "9" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setZ( QStringLiteral( "9" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  mAdvancedDigitizingDockWidget->setM( QStringLiteral( "9" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 0, Qt::LeftButton );

  utils.mouseClick( 0, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineStringZM (5 0 33 66, 3 5 33 66, 4 4 5 66, 6 6 33 5, 9 9 9 9)" ) );
}

void TestQgsAdvancedDigitizing::coordinateConstraintWhenSnapping()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  // line for snapping
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (1 1, 2 2)" ) );

  oldFeatures = utils.existingFeatureIds();

  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( snapConfig );

  // simple snap test
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 2.02, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 2, 2 2)" ) );

  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 0, -2, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "0" ),
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 2.02, 2, Qt::LeftButton ); // shouldn't snap to (2 2)
  utils.mouseClick( -2, -2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 -2, 0 2)" ) );

  oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 0, -2, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->setX( QStringLiteral( "2.02" ), // shouldn't snap to (2 2)
                                       QgsAdvancedDigitizingDockWidget::ReturnPressed );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( -2, -2, Qt::RightButton );

  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 -2, 2.02 2)" ) );
}

void TestQgsAdvancedDigitizing::perpendicularConstraint()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  // line for the perpendicular test
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 10, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, 0 10)" ) );

  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  snapConfig.setTypeFlag( Qgis::SnappingType::Vertex | Qgis::SnappingType::Segment );
  mCanvas->snappingUtils()->setConfig( snapConfig );

  // test snapping on segment
  utils.mouseMove( 0.1, 4 );
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 0, 4 ) );

  QCOMPARE( mAdvancedDigitizingDockWidget->additionalConstraint(),
            QgsAdvancedDigitizingDockWidget::AdditionalConstraint::NoConstraint );

  // digitizing a first vertex
  utils.mouseClick( 5, 5, Qt::LeftButton );

  mAdvancedDigitizingDockWidget->lockAdditionalConstraint( QgsAdvancedDigitizingDockWidget::AdditionalConstraint::Perpendicular );
  QCOMPARE( mAdvancedDigitizingDockWidget->additionalConstraint(),
            QgsAdvancedDigitizingDockWidget::AdditionalConstraint::Perpendicular );

  // select the previous digitized line
  utils.mouseClick( 0.1, 4, Qt::LeftButton );

  // test the perpendicular constraint
  utils.mouseMove( 3, 2 );
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 3, 5 ) );

  utils.mouseClick( 0, 0, Qt::RightButton );
}

void TestQgsAdvancedDigitizing::cadPointList()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  // start a digitized line
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseClick( 0, 3, Qt::LeftButton );
  utils.mouseClick( 0, 4, Qt::LeftButton );
  utils.mouseMove( 0, 5 );

  bool exist;

  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2( &exist ), QgsPoint( 0, 5 ) );
  QVERIFY( exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->previousPointV2( &exist ), QgsPoint( 0, 4 ) );
  QVERIFY( exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->penultimatePointV2( &exist ), QgsPoint( 0, 3 ) );
  QVERIFY( exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->pointsCount(), 6 );

  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( getWktFromLastAddedFeature( utils, oldFeatures ),
            QStringLiteral( "LineString (0 0, 0 1, 0 2, 0 3, 0 4)" ) );

  utils.mouseMove( 1, 1 );

  // with no digitized points
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2( &exist ), QgsPoint( 1, 1 ) );
  QVERIFY( exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->previousPointV2( &exist ), QgsPoint( ) );
  QVERIFY( !exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->penultimatePointV2( &exist ), QgsPoint( ) );
  QVERIFY( !exist );

  QCOMPARE( mAdvancedDigitizingDockWidget->pointsCount(), 1 );
}

void TestQgsAdvancedDigitizing::currentPointWhenSanpping()
{
  auto utils = getMapToolDigitizingUtils( mLayer3950 );

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  utils.mouseClick( 0, 10, Qt::LeftButton );
  utils.mouseClick( 0, -10, Qt::LeftButton );
  utils.mouseClick( 0, -10, Qt::RightButton );

  utils.mouseClick( 10, 0, Qt::LeftButton );
  utils.mouseClick( -10, 0, Qt::LeftButton );
  utils.mouseClick( -10, 0, Qt::RightButton );

  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( snapConfig );

  QCOMPARE( mCanvas->snappingUtils()->currentLayer(), mLayer3950 );

  utils.mouseClick( 25, 0, Qt::LeftButton );
  utils.mouseClick( 30, 0, Qt::LeftButton );

  // on an existing point
  utils.mouseMove( 0.1, 10 );
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 0, 10 ) );

  // on an intersection (see issue #46128)
  utils.mouseMove( 0.1, 0 );
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 0, 0 ) );

  // on a self point
  utils.mouseMove( 25, 0.1 );
  QCOMPARE( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 25, 0 ) );

  utils.mouseClick( 30, 0, Qt::RightButton );
}

void TestQgsAdvancedDigitizing::currentPointWhenSanppingWithDiffCanvasCRS()
{
  auto utils = getMapToolDigitizingUtils( mLayer4326 );

  QVERIFY( mAdvancedDigitizingDockWidget->cadEnabled() );

  QSet<QgsFeatureId> oldFeatures = utils.existingFeatureIds();

  utils.mouseClick( 0, 10, Qt::LeftButton );
  utils.mouseClick( 0, -10, Qt::LeftButton );
  utils.mouseClick( 0, -10, Qt::RightButton );

  utils.mouseClick( 10, 0, Qt::LeftButton );
  utils.mouseClick( -10, 0, Qt::LeftButton );
  utils.mouseClick( -10, 0, Qt::RightButton );

  oldFeatures = utils.existingFeatureIds();

  QgsSnappingConfig snapConfig = mCanvas->snappingUtils()->config();
  snapConfig.setEnabled( true );
  mCanvas->snappingUtils()->setConfig( snapConfig );

  QCOMPARE( mCanvas->snappingUtils()->currentLayer(), mLayer4326 );

  utils.mouseClick( 25, 0, Qt::LeftButton );
  utils.mouseClick( 0.1, 10, Qt::LeftButton );
  utils.mouseClick( 0.1, 10, Qt::RightButton );

  // on an existing point (see issue #46352)
  utils.mouseMove( 0.1, 10 );
  QGSCOMPARENEARPOINT( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 0, 10 ), 0.000001 );

  // on an intersection
  utils.mouseMove( 0.1, 0 );
  QGSCOMPARENEARPOINT( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 0, 0 ), 0.000001 );

  // on a self point
  utils.mouseMove( 25, 0.1 );
  QGSCOMPARENEARPOINT( mAdvancedDigitizingDockWidget->currentPointV2(), QgsPoint( 25, 0 ), 0.000001 );
}

QGSTEST_MAIN( TestQgsAdvancedDigitizing )
#include "testqgsadvanceddigitizing.moc"
