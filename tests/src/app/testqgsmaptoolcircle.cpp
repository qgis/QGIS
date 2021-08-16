/***************************************************************************
     testqgsmaptoolcircle.cpp
     ------------------------
    Date                 : July 2021
    Copyright            : (C) 2021 by Loïc Bartoletti
                           (C) 2021 by Antoine Facchini
    Email                : loic dot bartoletti @oslandia dot com
                           antoine dot facchini @oslandia dot com
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
#include "qgsmaptoolcircle2points.h"
#include "qgsmaptoolcircle3points.h"
#include "qgsmaptoolcirclecenterpoint.h"


class TestQgsMapToolCircle : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolCircle();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCircle_data();
    void testCircle();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mParentTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    std::map<QString, std::unique_ptr<QgsVectorLayer>> mVectorLayerMap = {};

    const QList<QString> mCoordinateList =
    {
      "XY", "XYZ", "XYM", "XYZM"
    };
    const QList<QString> mDrawingCircleMethods =
    {
      "2Points", "2PointsWithDeletedVertex",
      "3Points", "3PointsWithDeletedVertex",
      "centerPoint", "centerPointWithDeletedVertex",
    };
    QMap<QString, QString> mDrawFunctionUserNames = {};
    QMap<QString, std::function<QgsFeatureId( void )>> mDrawFunctionPtrMap = {};
    QMap<QString, QString> mExpectedWkts = {};

    void initAttributs();

    QgsFeatureId drawCircleFrom2Points();
    QgsFeatureId drawCircleFrom2PointsWithDeletedVertex();
    QgsFeatureId drawCircleFrom3Points();
    QgsFeatureId drawCircleFrom3PointsWithDeletedVertex();
    QgsFeatureId drawCircleFromCenterPoint();
    QgsFeatureId drawCircleFromCenterPointWithDeletedVertex();
};

TestQgsMapToolCircle::TestQgsMapToolCircle() = default;


//runs before all tests
void TestQgsMapToolCircle::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:27700" ) ) );

  // make testing layers
  QList<QgsMapLayer *> layerList;

  mVectorLayerMap["XY"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineString?crs=EPSG:27700" ), QStringLiteral( "layer line " ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XY"]->isValid() );
  layerList << mVectorLayerMap["XY"].get();

  mVectorLayerMap["XYZ"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYZ"]->isValid() );
  layerList << mVectorLayerMap["XYZ"].get();

  mVectorLayerMap["XYM"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringM?crs=EPSG:27700" ), QStringLiteral( "layer line M" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYM"]->isValid() );
  layerList << mVectorLayerMap["XYM"].get();

  mVectorLayerMap["XYZM"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "LineStringZM?crs=EPSG:27700" ), QStringLiteral( "layer line ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYZM"]->isValid() );
  layerList << mVectorLayerMap["XYZM"].get();

  // add and set layers in canvas
  QgsProject::instance()->addMapLayers( layerList );
  mCanvas->setLayers( layerList );

  mParentTool = new QgsMapToolAddFeature( mCanvas, QgsMapToolCapture::CaptureLine );

  initAttributs();
}

void TestQgsMapToolCircle::initAttributs()
{
  mDrawFunctionUserNames["2Points"] = "from 2 points";
  mDrawFunctionUserNames["2PointsWithDeletedVertex"] = "from 2 points with deleted vertex";
  mDrawFunctionUserNames["3Points"] = "from 3 points";
  mDrawFunctionUserNames["3PointsWithDeletedVertex"] = "from 3 points with deleted vertex";
  mDrawFunctionUserNames["centerPoint"] = "from center point";
  mDrawFunctionUserNames["centerPointWithDeletedVertex"] = "from center point with deleted vertex";

  mDrawFunctionPtrMap["2Points"] = std::bind( &TestQgsMapToolCircle::drawCircleFrom2Points, this );
  mDrawFunctionPtrMap["2PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolCircle::drawCircleFrom2PointsWithDeletedVertex, this );
  mDrawFunctionPtrMap["3Points"] = std::bind( &TestQgsMapToolCircle::drawCircleFrom3Points, this );
  mDrawFunctionPtrMap["3PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolCircle::drawCircleFrom3PointsWithDeletedVertex, this );
  mDrawFunctionPtrMap["centerPoint"] = std::bind( &TestQgsMapToolCircle::drawCircleFromCenterPoint, this );
  mDrawFunctionPtrMap["centerPointWithDeletedVertex"] = std::bind( &TestQgsMapToolCircle::drawCircleFromCenterPointWithDeletedVertex, this );

  mExpectedWkts[QStringLiteral( "XY" "2Points" )] = "CompoundCurve (CircularString (0 2, 1 1, 0 0, -1 1, 0 2))";
  mExpectedWkts[QStringLiteral( "XY" "2PointsWithDeletedVertex" )] = "CompoundCurve (CircularString (0 2, 1 1, 0 0, -1 1, 0 2))";
  mExpectedWkts[QStringLiteral( "XY" "3Points" )] = "CompoundCurve (CircularString (0 2, 1 1, 0 0, -1 1, 0 2))";
  mExpectedWkts[QStringLiteral( "XY" "3PointsWithDeletedVertex" )] = "CompoundCurve (CircularString (0 2, 1 1, 0 0, -1 1, 0 2))";
  mExpectedWkts[QStringLiteral( "XY" "centerPoint" )] = "CompoundCurve (CircularString (0 2, 2 0, 0 -2, -2 0, 0 2))";
  mExpectedWkts[QStringLiteral( "XY" "centerPointWithDeletedVertex" )] = "CompoundCurve (CircularString (0 2, 2 0, 0 -2, -2 0, 0 2))";

  mExpectedWkts[QStringLiteral( "XYZ" "2Points" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 1 1 444, 0 0 444, -1 1 444, 0 2 444))";
  mExpectedWkts[QStringLiteral( "XYZ" "2PointsWithDeletedVertex" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 1 1 444, 0 0 444, -1 1 444, 0 2 444))";
  mExpectedWkts[QStringLiteral( "XYZ" "3Points" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 1 1 444, 0 0 444, -1 1 444, 0 2 444))";
  mExpectedWkts[QStringLiteral( "XYZ" "3PointsWithDeletedVertex" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 1 1 444, 0 0 444, -1 1 444, 0 2 444))";
  mExpectedWkts[QStringLiteral( "XYZ" "centerPoint" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 2 0 444, 0 -2 444, -2 0 444, 0 2 444))";
  mExpectedWkts[QStringLiteral( "XYZ" "centerPointWithDeletedVertex" )] = "CompoundCurveZ (CircularStringZ (0 2 444, 2 0 444, 0 -2 444, -2 0 444, 0 2 444))";

  mExpectedWkts[QStringLiteral( "XYM" "2Points" )] = "CompoundCurveM (CircularStringM (0 2 222, 1 1 222, 0 0 222, -1 1 222, 0 2 222))";
  mExpectedWkts[QStringLiteral( "XYM" "2PointsWithDeletedVertex" )] = "CompoundCurveM (CircularStringM (0 2 222, 1 1 222, 0 0 222, -1 1 222, 0 2 222))";
  mExpectedWkts[QStringLiteral( "XYM" "3Points" )] = "CompoundCurveM (CircularStringM (0 2 222, 1 1 222, 0 0 222, -1 1 222, 0 2 222))";
  mExpectedWkts[QStringLiteral( "XYM" "3PointsWithDeletedVertex" )] = "CompoundCurveM (CircularStringM (0 2 222, 1 1 222, 0 0 222, -1 1 222, 0 2 222))";
  mExpectedWkts[QStringLiteral( "XYM" "centerPoint" )] = "CompoundCurveM (CircularStringM (0 2 222, 2 0 222, 0 -2 222, -2 0 222, 0 2 222))";
  mExpectedWkts[QStringLiteral( "XYM" "centerPointWithDeletedVertex" )] = "CompoundCurveM (CircularStringM (0 2 222, 2 0 222, 0 -2 222, -2 0 222, 0 2 222))";

  mExpectedWkts[QStringLiteral( "XYZM" "2Points" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 1 1 444 222, 0 0 444 222, -1 1 444 222, 0 2 444 222))";
  mExpectedWkts[QStringLiteral( "XYZM" "2PointsWithDeletedVertex" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 1 1 444 222, 0 0 444 222, -1 1 444 222, 0 2 444 222))";
  mExpectedWkts[QStringLiteral( "XYZM" "3Points" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 1 1 444 222, 0 0 444 222, -1 1 444 222, 0 2 444 222))";
  mExpectedWkts[QStringLiteral( "XYZM" "3PointsWithDeletedVertex" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 1 1 444 222, 0 0 444 222, -1 1 444 222, 0 2 444 222))";
  mExpectedWkts[QStringLiteral( "XYZM" "centerPoint" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 2 0 444 222, 0 -2 444 222, -2 0 444 222, 0 2 444 222))";
  mExpectedWkts[QStringLiteral( "XYZM" "centerPointWithDeletedVertex" )] = "CompoundCurveZM (CircularStringZM (0 2 444 222, 2 0 444 222, 0 -2 444 222, -2 0 444 222, 0 2 444 222))";
}

void TestQgsMapToolCircle::cleanupTestCase()
{

  for ( QString coordinate : mCoordinateList )
  {
    mVectorLayerMap[coordinate].reset();
  }
  QgsApplication::exitQgis();
}


QgsFeatureId TestQgsMapToolCircle::drawCircleFrom2Points()
{
  QgsMapToolCircle2Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom2PointsWithDeletedVertex()
{
  QgsMapToolCircle2Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom3Points()
{
  QgsMapToolCircle3Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom3PointsWithDeletedVertex()
{
  QgsMapToolCircle3Points mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFromCenterPoint()
{
  QgsMapToolCircleCenterPoint mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFromCenterPointWithDeletedVertex()
{
  QgsMapToolCircleCenterPoint mapTool( mParentTool, mCanvas );
  mCanvas->setMapTool( &mapTool );

  TestQgsMapToolAdvancedDigitizingUtils utils( &mapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}


void TestQgsMapToolCircle::testCircle_data()
{
  QTest::addColumn<QString>( "wktGeometry" );
  QTest::addColumn<QString>( "wktExpected" );
  QTest::addColumn<qlonglong>( "featureCount" );
  QTest::addColumn<long>( "featureCountExpected" );

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 444 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 222 );

  QgsFeatureId newFid;
  QgsFeature f;
  QString wkt;
  QgsVectorLayer *mLayer;

  QString rowStringName;

  for ( QString coordinate : mCoordinateList )
  {
    mLayer = mVectorLayerMap[coordinate].get();
    mCanvas->setCurrentLayer( mLayer );

    for ( QString drawMethod : mDrawingCircleMethods )
    {
      mLayer->startEditing();
      newFid = mDrawFunctionPtrMap[drawMethod]();
      f = mLayer->getFeature( newFid );

      wkt = mExpectedWkts[coordinate + drawMethod];
      rowStringName = coordinate + " " + mDrawFunctionUserNames[drawMethod];
      QTest::newRow( rowStringName.toStdString().c_str() ) << f.geometry().asWkt() << wkt << mLayer->featureCount() << ( long )1;

      mLayer->rollBack();
    }
  }

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolCircle::testCircle()
{
  QFETCH( qlonglong, featureCount );
  QFETCH( long, featureCountExpected );
  QCOMPARE( featureCount, featureCountExpected );

  QFETCH( QString, wktGeometry );
  QFETCH( QString, wktExpected );
  QCOMPARE( wktGeometry, wktExpected );
}


QGSTEST_MAIN( TestQgsMapToolCircle )
#include "testqgsmaptoolcircle.moc"
