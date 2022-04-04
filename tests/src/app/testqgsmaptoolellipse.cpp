/***************************************************************************
     testqgsmaptoolellipse.cpp
     ------------------------
    Date                 : January 2018
    Copyright            : (C) 2018 by Paul Blottiere
                           (C) 2021 by Loïc Bartoletti
    Email                : paul.blottiere@oslandia.com
                           loic dot bartoletti @oslandia dot com
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
#include "qgsmaptoolcapture.h"

#include "qgsmaptoolshapeellipsecenterpoint.h"
#include "qgsmaptoolshapeellipsecenter2points.h"
#include "qgsmaptoolshapeellipseextent.h"
#include "qgsmaptoolshapeellipsefoci.h"

class TestQgsMapToolEllipse : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolEllipse() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testEllipse_data();
    void testEllipse();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    std::map<QString, std::unique_ptr<QgsVectorLayer>> mVectorLayerMap = {};

    const QList<QString> mCoordinateList =
    {
      "XY", "XYZ", "XYM", "XYZM"
    };
    const QList<QString> mDrawingEllipseMethods =
    {
      "CenterAndPoint", "CenterAndPointWithDeletedVertex",
      "CenterAnd2Points", "CenterAnd2PointsWithDeletedVertex",
      "FromExtent", "FromExtentWithDeletedVertex",
      "FromFoci", "FromFociWithDeletedVertex",
    };
    QMap<QString, QString> mDrawFunctionUserNames = {};
    QMap<QString, std::function<QgsFeatureId( void )>> mDrawFunctionPtrMap = {};
    QMap<QString, QString> mExpectedWkts = {};

    void initAttributs();

    void resetMapTool( QgsMapToolShapeMetadata *metadata );

    QgsFeatureId drawEllipseFromCenterAndPoint();
    QgsFeatureId drawEllipseFromCenterAndPointWithDeletedVertex();
    QgsFeatureId drawEllipseFromCenterAnd2Points();
    QgsFeatureId drawEllipseFromCenterAnd2PointsWithDeletedVertex();
    QgsFeatureId drawEllipseFromExtent();
    QgsFeatureId drawEllipseFromExtentWithDeletedVertex();
    QgsFeatureId drawEllipseFromFoci();
    QgsFeatureId drawEllipseFromFociWithDeletedVertex();

    const double Z = 444.0;
    const double M = 222.0;
    const int WKT_PRECISION = 2;

    unsigned int segments( ) { return QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg.value() * 12; }
};


//runs before all tests
void TestQgsMapToolEllipse::initTestCase()
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

  mMapTool = new QgsMapToolAddFeature( mCanvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine );
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mCanvas->setMapTool( mMapTool );

  initAttributs();
}

void TestQgsMapToolEllipse::initAttributs()
{
  mDrawFunctionUserNames["CenterAndPoint"] = "from center and a point";
  mDrawFunctionUserNames["CenterAndPointWithDeletedVertex"] = "from center and a point with deleted vertex";
  mDrawFunctionUserNames["CenterAnd2Points"] = "from center and 2 points";
  mDrawFunctionUserNames["CenterAnd2PointsWithDeletedVertex"] = "from center and 2 points with deleted vertex";
  mDrawFunctionUserNames["FromExtent"] = "from extent point";
  mDrawFunctionUserNames["FromExtentWithDeletedVertex"] = "from extent with deleted vertex";
  mDrawFunctionUserNames["FromFoci"] = "from foci point";
  mDrawFunctionUserNames["FromFociWithDeletedVertex"] = "from foci with deleted vertex";

  mDrawFunctionPtrMap["CenterAndPoint"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAndPoint, this );
  mDrawFunctionPtrMap["CenterAndPointWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAndPointWithDeletedVertex, this );
  mDrawFunctionPtrMap["CenterAnd2Points"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd2Points, this );
  mDrawFunctionPtrMap["CenterAnd2PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd2PointsWithDeletedVertex, this );
  mDrawFunctionPtrMap["FromExtent"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromExtent, this );
  mDrawFunctionPtrMap["FromExtentWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromExtentWithDeletedVertex, this );
  mDrawFunctionPtrMap["FromFoci"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromFoci, this );
  mDrawFunctionPtrMap["FromFociWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromFociWithDeletedVertex, this );

  mExpectedWkts[QStringLiteral( "XY" "CenterAndPoint" )] =  QgsEllipse::fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 1, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "CenterAndPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XY" "CenterAnd2Points" )] = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "CenterAnd2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XY" "FromExtent" )] = QgsEllipse::fromExtent( QgsPoint( 0, 0 ), QgsPoint( 2, 2 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "FromExtentWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XY" "FromFoci" )] = QgsEllipse::fromFoci( QgsPoint( 0, 0 ), QgsPoint( 1, -1 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY" "FromFociWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XY" "FromFoci" )];

  mExpectedWkts[QStringLiteral( "XYZ" "CenterAndPoint" )] =  QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "CenterAndPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYZ" "CenterAnd2Points" )] = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, 1, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "CenterAnd2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYZ" "FromExtent" )] = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 2, 2, Z, M, QgsWkbTypes::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "FromExtentWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYZ" "FromFoci" )] = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointZ ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ" "FromFociWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZ" "FromFoci" )];

  mExpectedWkts[QStringLiteral( "XYM" "CenterAndPoint" )] =  QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "CenterAndPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYM" "CenterAnd2Points" )] = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, 1, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "CenterAnd2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYM" "FromExtent" )] = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 2, 2, Z, M, QgsWkbTypes::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "FromExtentWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYM" "FromFoci" )] = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointM ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointM ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM" "FromFociWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYM" "FromFoci" )];

  mExpectedWkts[QStringLiteral( "XYZM" "CenterAndPoint" )] =  QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "CenterAndPointWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYZM" "CenterAnd2Points" )] = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, 1, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "CenterAnd2PointsWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYZM" "FromExtent" )] = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 2, 2, Z, M, QgsWkbTypes::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "FromExtentWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYZM" "FromFoci" )] = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 1, -1, Z, M, QgsWkbTypes::PointZM ), QgsPoint( 0, -1, Z, M, QgsWkbTypes::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM" "FromFociWithDeletedVertex" )] = mExpectedWkts[QStringLiteral( "XYZM" "FromFoci" )];
}

void TestQgsMapToolEllipse::cleanupTestCase()
{
  for ( const QString &coordinate : std::as_const( mCoordinateList ) )
  {
    mVectorLayerMap[coordinate].reset();
  }

  delete mMapTool;

  QgsApplication::exitQgis();
}

void TestQgsMapToolEllipse::resetMapTool( QgsMapToolShapeMetadata *metadata )
{
  mMapTool->clean();
  mMapTool->setCurrentCaptureTechnique( Qgis::CaptureTechnique::Shape );
  mMapTool->setCurrentShapeMapTool( metadata ) ;
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAndPoint()
{
  resetMapTool( new QgsMapToolShapeEllipseCenterPointMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, -1 );
  utils.mouseClick( 1, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAndPointWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipseCenterPointMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, -1 );
  utils.mouseClick( 1, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAnd2Points()
{
  resetMapTool( new QgsMapToolShapeEllipseCenter2PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseMove( 0, -1 );
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAnd2PointsWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipseCenter2PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 1, Qt::LeftButton );
  utils.mouseMove( 0, -1 );
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromExtent()
{
  resetMapTool( new QgsMapToolShapeEllipseExtentMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 2 );
  utils.mouseClick( 2, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromExtentWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipseExtentMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 2 );
  utils.mouseClick( 2, 2, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromFoci()
{
  resetMapTool( new QgsMapToolShapeEllipseFociMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, -1 );
  utils.mouseClick( 1, -1, Qt::LeftButton );
  utils.mouseMove( 0, -1 );
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromFociWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipseFociMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, -1 );
  utils.mouseClick( 1, -1, Qt::LeftButton );
  utils.mouseMove( 0, -1 );
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}


void TestQgsMapToolEllipse::testEllipse_data()
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

    for ( const QString &drawMethod : std::as_const( mDrawingEllipseMethods ) )
    {
      mLayer->startEditing();
      mLayer->dataProvider()->truncate();

      newFid = mDrawFunctionPtrMap[drawMethod]();
      f = mLayer->getFeature( newFid );

      wkt = mExpectedWkts[coordinate + drawMethod];
      rowStringName = coordinate + " " + mDrawFunctionUserNames[drawMethod];
      QTest::newRow( rowStringName.toStdString().c_str() ) << f.geometry().asWkt( WKT_PRECISION ) << wkt << mLayer->featureCount() << ( long )1;

      mLayer->rollBack();
    }
  }

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.setValue( 0 );
}

void TestQgsMapToolEllipse::testEllipse()
{
  QFETCH( qlonglong, featureCount );
  QFETCH( long, featureCountExpected );
  QCOMPARE( featureCount, featureCountExpected );

  QFETCH( QString, wktGeometry );
  QFETCH( QString, wktExpected );
  QCOMPARE( wktGeometry, wktExpected );
}


QGSTEST_MAIN( TestQgsMapToolEllipse )
#include "testqgsmaptoolellipse.moc"
