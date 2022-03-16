/***************************************************************************
     testqgsmaptoolcircle.cpp
     ------------------------
    Date                 : January 2018
    Copyright            : (C) 2018 by Paul Blottiere
                           (C) 2021 by Loïc Bartoletti
                           (C) 2021 by Antoine Facchini
    Email                : paul.blottiere@oslandia.com
                           loic dot bartoletti @oslandia dot com
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
#include "qgsmaptoolshapecircle2points.h"
#include "qgsmaptoolshapecircle3points.h"
#include "qgsmaptoolshapecirclecenterpoint.h"


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
    void resetMapTool( QgsMapToolShapeMetadata *metadata );

    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
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

    const double Z = 444;
    const double M = 222;
    const double WKT_PRECISION = 2;
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

  mVectorLayerMap["XY"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "CompoundCurve?crs=EPSG:27700" ), QStringLiteral( "layer line " ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XY"]->isValid() );
  layerList << mVectorLayerMap["XY"].get();

  mVectorLayerMap["XYZ"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "CompoundCurveZ?crs=EPSG:27700" ), QStringLiteral( "layer line Z" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYZ"]->isValid() );
  layerList << mVectorLayerMap["XYZ"].get();

  mVectorLayerMap["XYM"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "CompoundCurveM?crs=EPSG:27700" ), QStringLiteral( "layer line M" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYM"]->isValid() );
  layerList << mVectorLayerMap["XYM"].get();

  mVectorLayerMap["XYZM"] = std::make_unique<QgsVectorLayer>( QStringLiteral( "CompoundCurveZM?crs=EPSG:27700" ), QStringLiteral( "layer line ZM" ), QStringLiteral( "memory" ) );
  QVERIFY( mVectorLayerMap["XYZM"]->isValid() );
  layerList << mVectorLayerMap["XYZM"].get();

  // add and set layers in canvas
  QgsProject::instance()->addMapLayers( layerList );
  mCanvas->setLayers( layerList );

  mMapTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mCanvas->setMapTool( mMapTool );

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

  mExpectedWkts[QStringLiteral( "XY" "2Points" )] = QgsCircle::from2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::Point ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::Point ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "2Points" )];
  mExpectedWkts[QStringLiteral( "XY" "3Points" )] = QgsCircle::from3Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::Point ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::Point ), QgsPoint( 1, 1, Z, M, QgsWkbTypes::Point ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "3PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "3Points" )];
  mExpectedWkts[QStringLiteral( "XY" "centerPoint" )] = QgsCircle::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::Point ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::Point ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "centerPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "centerPoint" )] ;

  mExpectedWkts[QStringLiteral( "XYZ" "2Points" )] = QgsCircle::from2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZ ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "2Points" )];
  mExpectedWkts[QStringLiteral( "XYZ" "3Points" )] = QgsCircle::from3Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 1, 1, Z, M, QgsWkbTypes::PointZ ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "3PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "3Points" )];
  mExpectedWkts[QStringLiteral( "XYZ" "centerPoint" )] = QgsCircle::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZ ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "centerPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "centerPoint" )] ;

  mExpectedWkts[QStringLiteral( "XYM" "2Points" )] = QgsCircle::from2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "2Points" )];
  mExpectedWkts[QStringLiteral( "XYM" "3Points" )] = QgsCircle::from3Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointM ), QgsPoint( 1, 1, Z, M, QgsWkbTypes::PointM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "3PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "3Points" )];
  mExpectedWkts[QStringLiteral( "XYM" "centerPoint" )] = QgsCircle::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "centerPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "centerPoint" )] ;

  mExpectedWkts[QStringLiteral( "XYZM" "2Points" )] = QgsCircle::from2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "2Points" )];
  mExpectedWkts[QStringLiteral( "XYZM" "3Points" )] = QgsCircle::from3Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 1, 1, Z, M, QgsWkbTypes::PointZM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "3PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "3Points" )];
  mExpectedWkts[QStringLiteral( "XYZM" "centerPoint" )] = QgsCircle::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, 2, Z, M, QgsWkbTypes::PointZM ) ).toCircularString( true )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "centerPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "centerPoint" )] ;
}

void TestQgsMapToolCircle::cleanupTestCase()
{
  for ( const QString &coordinate : std::as_const( mCoordinateList ) )
  {
    mVectorLayerMap[coordinate].reset();
  }
  delete mMapTool;
  QgsApplication::exitQgis();
}

void TestQgsMapToolCircle::resetMapTool( QgsMapToolShapeMetadata *metadata )
{
  mMapTool->clean();
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mMapTool->setCurrentShapeMapTool( metadata ) ;
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom2Points()
{
  QgsMapToolShapeCircle2PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom2PointsWithDeletedVertex()
{
  QgsMapToolShapeCircle2PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom3Points()
{
  QgsMapToolShapeCircle3PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 2, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFrom3PointsWithDeletedVertex()
{
  QgsMapToolShapeCircle3PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
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
  QgsMapToolShapeCircleCenterPointMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 0, 2 );
  utils.mouseClick( 0, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolCircle::drawCircleFromCenterPointWithDeletedVertex()
{
  QgsMapToolShapeCircleCenterPointMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
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

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( Z );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( M );

  QgsFeatureId newFid;
  QgsFeature f;
  QString wkt;
  QgsVectorLayer *mLayer;

  QString rowStringName;

  for ( const QString &coordinate : std::as_const( mCoordinateList ) )
  {
    mLayer = mVectorLayerMap[coordinate].get();
    mCanvas->setCurrentLayer( mLayer );

    for ( const QString &drawMethod : std::as_const( mDrawingCircleMethods ) )
    {
      mLayer->startEditing();
      newFid = mDrawFunctionPtrMap[drawMethod]();
      f = mLayer->getFeature( newFid );

      wkt = mExpectedWkts[coordinate + drawMethod];
      rowStringName = coordinate + " " + mDrawFunctionUserNames[drawMethod];
      const QgsAbstractGeometry *ageom = f.geometry().constGet();
      Q_ASSERT( ageom != nullptr );
      const QgsCompoundCurve *compoundCurveGeom = QgsCompoundCurve::cast( ageom );
      Q_ASSERT( compoundCurveGeom != nullptr );
      const QgsCurve *curveGeom = compoundCurveGeom->curveAt( 0 );
      Q_ASSERT( curveGeom != nullptr );
      QTest::newRow( rowStringName.toStdString().c_str() ) << curveGeom->asWkt( WKT_PRECISION ) << wkt << mLayer->featureCount() << ( long )1;

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
