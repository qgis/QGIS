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

#include "qgisapp.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaptooladdfeature.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptoolshapeellipse4points.h"
#include "qgsmaptoolshapeellipsecenter2points.h"
#include "qgsmaptoolshapeellipsecenter3points.h"
#include "qgsmaptoolshapeellipsecenterpoint.h"
#include "qgsmaptoolshapeellipseextent.h"
#include "qgsmaptoolshapeellipsefoci.h"
#include "qgssettingsregistrycore.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "testqgsmaptoolutils.h"

#include <QSignalSpy>

class TestQgsMapToolEllipse : public QObject
{
    Q_OBJECT

  public:
    TestQgsMapToolEllipse() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testEllipse_data();
    void testEllipse();

    void testEllipseFromCenterAndPointNotEnoughPoints();
    void testEllipseFromCenterAnd2PointsNotEnoughPoints();
    void testEllipseFromCenterAnd3PointsNotEnoughPoints();
    void testEllipseFromExtentNotEnoughPoints();
    void testEllipseFromFociNotEnoughPoints();
    void testEllipseFrom4PointsNotEnoughPoints();

    void testTransientGeometrySignalCenterPoint();
    void testTransientGeometrySignalCenter2Points();
    void testTransientGeometrySignalExtent();
    void testTransientGeometrySignalFoci();

  private:
    QgisApp *mQgisApp = nullptr;
    QgsMapToolCapture *mMapTool = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    std::map<QString, std::unique_ptr<QgsVectorLayer>> mVectorLayerMap = {};

    const QList<QString> mCoordinateList = {
      "XY", "XYZ", "XYM", "XYZM"
    };
    const QList<QString> mDrawingEllipseMethods = {
      "CenterAndPoint",
      "CenterAndPointWithDeletedVertex",
      "CenterAnd2Points",
      "CenterAnd2PointsWithDeletedVertex",
      "CenterAnd3Points",
      "CenterAnd3PointsWithDeletedVertex",
      "FromExtent",
      "FromExtentWithDeletedVertex",
      "FromFoci",
      "FromFociWithDeletedVertex",
      "From4Points",
      "From4PointsWithDeletedVertex",
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
    QgsFeatureId drawEllipseFromCenterAnd3Points();
    QgsFeatureId drawEllipseFromCenterAnd3PointsWithDeletedVertex();
    QgsFeatureId drawEllipseFromExtent();
    QgsFeatureId drawEllipseFromExtentWithDeletedVertex();
    QgsFeatureId drawEllipseFromFoci();
    QgsFeatureId drawEllipseFromFociWithDeletedVertex();
    QgsFeatureId drawEllipseFrom4Points();
    QgsFeatureId drawEllipseFrom4PointsWithDeletedVertex();

    const double Z = 444.0;
    const double M = 222.0;
    const int WKT_PRECISION = 2;

    unsigned int segments() { return QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->value() * 12; }
};


//runs before all tests
void TestQgsMapToolEllipse::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mQgisApp = new QgisApp();
  mCanvas = new QgsMapCanvas();
  mCanvas->setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:27700"_s ) );

  // make testing layers
  QList<QgsMapLayer *> layerList;

  mVectorLayerMap["XY"] = std::make_unique<QgsVectorLayer>( u"LineString?crs=EPSG:27700"_s, u"layer line "_s, u"memory"_s );
  QVERIFY( mVectorLayerMap["XY"]->isValid() );
  layerList << mVectorLayerMap["XY"].get();

  mVectorLayerMap["XYZ"] = std::make_unique<QgsVectorLayer>( u"LineStringZ?crs=EPSG:27700"_s, u"layer line Z"_s, u"memory"_s );
  QVERIFY( mVectorLayerMap["XYZ"]->isValid() );
  layerList << mVectorLayerMap["XYZ"].get();

  mVectorLayerMap["XYM"] = std::make_unique<QgsVectorLayer>( u"LineStringM?crs=EPSG:27700"_s, u"layer line M"_s, u"memory"_s );
  QVERIFY( mVectorLayerMap["XYM"]->isValid() );
  layerList << mVectorLayerMap["XYM"].get();

  mVectorLayerMap["XYZM"] = std::make_unique<QgsVectorLayer>( u"LineStringZM?crs=EPSG:27700"_s, u"layer line ZM"_s, u"memory"_s );
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
  mDrawFunctionUserNames["CenterAnd3Points"] = "from center and 3 points";
  mDrawFunctionUserNames["CenterAnd3PointsWithDeletedVertex"] = "from center and 3 points with deleted vertex";
  mDrawFunctionUserNames["FromExtent"] = "from extent point";
  mDrawFunctionUserNames["FromExtentWithDeletedVertex"] = "from extent with deleted vertex";
  mDrawFunctionUserNames["FromFoci"] = "from foci point";
  mDrawFunctionUserNames["FromFociWithDeletedVertex"] = "from foci with deleted vertex";
  mDrawFunctionUserNames["From4Points"] = "from 4 points";
  mDrawFunctionUserNames["From4PointsWithDeletedVertex"] = "from 4 points with deleted vertex";

  mDrawFunctionPtrMap["CenterAndPoint"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAndPoint, this );
  mDrawFunctionPtrMap["CenterAndPointWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAndPointWithDeletedVertex, this );
  mDrawFunctionPtrMap["CenterAnd2Points"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd2Points, this );
  mDrawFunctionPtrMap["CenterAnd2PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd2PointsWithDeletedVertex, this );
  mDrawFunctionPtrMap["CenterAnd3Points"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd3Points, this );
  mDrawFunctionPtrMap["CenterAnd3PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromCenterAnd3PointsWithDeletedVertex, this );
  mDrawFunctionPtrMap["FromExtent"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromExtent, this );
  mDrawFunctionPtrMap["FromExtentWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromExtentWithDeletedVertex, this );
  mDrawFunctionPtrMap["FromFoci"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromFoci, this );
  mDrawFunctionPtrMap["FromFociWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFromFociWithDeletedVertex, this );
  mDrawFunctionPtrMap["From4Points"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFrom4Points, this );
  mDrawFunctionPtrMap["From4PointsWithDeletedVertex"] = std::bind( &TestQgsMapToolEllipse::drawEllipseFrom4PointsWithDeletedVertex, this );

  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAndPoint" )]
    = QgsEllipse::fromCenterPoint( QgsPoint( 0, 0 ), QgsPoint( 1, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAndPointWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAnd2Points" )]
    = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAnd2PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XY"
                                "FromExtent" )]
    = QgsEllipse::fromExtent( QgsPoint( 0, 0 ), QgsPoint( 2, 2 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "FromExtentWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XY"
                                "FromFoci" )]
    = QgsEllipse::fromFoci( QgsPoint( 0, 0 ), QgsPoint( 1, -1 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "FromFociWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "FromFoci" )];
  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAnd3Points" )]
    = QgsEllipse::fromCenter3Points( QgsPoint( 0, 0 ), QgsPoint( 0, 1 ), QgsPoint( 1, 0 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "CenterAnd3PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "CenterAnd3Points" )];
  mExpectedWkts[QStringLiteral( "XY"
                                "From4Points" )]
    = QgsEllipse::from4Points( QgsPoint( 2, 0 ), QgsPoint( -2, 0 ), QgsPoint( 0, 1 ), QgsPoint( 0, -1 ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XY"
                                "From4PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XY"
                                    "From4Points" )];

  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAndPoint" )]
    = QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAndPointWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAnd2Points" )]
    = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAnd2PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYZ"
                                "FromExtent" )]
    = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 2, 2, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "FromExtentWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYZ"
                                "FromFoci" )]
    = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "FromFociWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "FromFoci" )];
  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAnd3Points" )]
    = QgsEllipse::fromCenter3Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 1, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "CenterAnd3PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "CenterAnd3Points" )];
  mExpectedWkts[QStringLiteral( "XYZ"
                                "From4Points" )]
    = QgsEllipse::from4Points( QgsPoint( 2, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( -2, 0, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZ ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZ ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZ"
                                "From4PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZ"
                                    "From4Points" )];

  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAndPoint" )]
    = QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAndPointWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAnd2Points" )]
    = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAnd2PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYM"
                                "FromExtent" )]
    = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 2, 2, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "FromExtentWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYM"
                                "FromFoci" )]
    = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "FromFociWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "FromFoci" )];
  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAnd3Points" )]
    = QgsEllipse::fromCenter3Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointM ), QgsPoint( 1, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "CenterAnd3PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "CenterAnd3Points" )];
  mExpectedWkts[QStringLiteral( "XYM"
                                "From4Points" )]
    = QgsEllipse::from4Points( QgsPoint( 2, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( -2, 0, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYM"
                                "From4PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYM"
                                    "From4Points" )];

  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAndPoint" )]
    = QgsEllipse::fromCenterPoint( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAndPointWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "CenterAndPoint" )];
  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAnd2Points" )]
    = QgsEllipse::fromCenter2Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAnd2PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "CenterAnd2Points" )];
  mExpectedWkts[QStringLiteral( "XYZM"
                                "FromExtent" )]
    = QgsEllipse::fromExtent( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 2, 2, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "FromExtentWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "FromExtent" )];
  mExpectedWkts[QStringLiteral( "XYZM"
                                "FromFoci" )]
    = QgsEllipse::fromFoci( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 1, -1, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "FromFociWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "FromFoci" )];
  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAnd3Points" )]
    = QgsEllipse::fromCenter3Points( QgsPoint( 0, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 1, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "CenterAnd3PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "CenterAnd3Points" )];
  mExpectedWkts[QStringLiteral( "XYZM"
                                "From4Points" )]
    = QgsEllipse::from4Points( QgsPoint( 2, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( -2, 0, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, 1, Z, M, Qgis::WkbType::PointZM ), QgsPoint( 0, -1, Z, M, Qgis::WkbType::PointZM ) ).toLineString( segments() )->asWkt( WKT_PRECISION );
  mExpectedWkts[QStringLiteral( "XYZM"
                                "From4PointsWithDeletedVertex" )]
    = mExpectedWkts[QStringLiteral( "XYZM"
                                    "From4Points" )];
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
  mMapTool->setCurrentShapeMapTool( metadata );
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

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAnd3Points()
{
  resetMapTool( new QgsMapToolShapeEllipseCenter3PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::LeftButton ); // center
  utils.mouseClick( 0, 1, Qt::LeftButton ); // pt1
  utils.mouseClick( 1, 0, Qt::LeftButton ); // pt2
  utils.mouseMove( 0, -1 );                 // pt3
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFromCenterAnd3PointsWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipseCenter3PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 0, 0, Qt::LeftButton ); // center
  utils.mouseClick( 0, 1, Qt::LeftButton ); // pt1
  utils.mouseClick( 1, 0, Qt::LeftButton ); // pt2
  utils.mouseMove( 0, -1 );                 // pt3
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFrom4Points()
{
  resetMapTool( new QgsMapToolShapeEllipse4PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 2, 0, Qt::LeftButton );  // pt1
  utils.mouseClick( -2, 0, Qt::LeftButton ); // pt2
  utils.mouseClick( 0, 1, Qt::LeftButton );  // pt3
  utils.mouseMove( 0, -1 );                  // pt4
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}

QgsFeatureId TestQgsMapToolEllipse::drawEllipseFrom4PointsWithDeletedVertex()
{
  resetMapTool( new QgsMapToolShapeEllipse4PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 4, 1, Qt::LeftButton );
  utils.keyClick( Qt::Key_Backspace );
  utils.mouseClick( 2, 0, Qt::LeftButton );  // pt1
  utils.mouseClick( -2, 0, Qt::LeftButton ); // pt2
  utils.mouseClick( 0, 1, Qt::LeftButton );  // pt3
  utils.mouseMove( 0, -1 );                  // pt4
  utils.mouseClick( 0, -1, Qt::RightButton );

  return utils.newFeatureId();
}


void TestQgsMapToolEllipse::testEllipse_data()
{
  QTest::addColumn<QString>( "wktGeometry" );
  QTest::addColumn<QString>( "wktExpected" );
  QTest::addColumn<qlonglong>( "featureCount" );
  QTest::addColumn<long>( "featureCountExpected" );

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->setValue( Z );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->setValue( M );

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
      QTest::newRow( rowStringName.toStdString().c_str() ) << f.geometry().asWkt( WKT_PRECISION ) << wkt << mLayer->featureCount() << ( long ) 1;

      mLayer->rollBack();
    }
  }

  QgsSettingsRegistryCore::settingsDigitizingDefaultZValue->setValue( 0 );
  QgsSettingsRegistryCore::settingsDigitizingDefaultMValue->setValue( 0 );
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

void TestQgsMapToolEllipse::testEllipseFromCenterAndPointNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipseCenterPointMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testEllipseFromCenterAnd2PointsNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipseCenter2PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testEllipseFromCenterAnd3PointsNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipseCenter3PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testEllipseFromExtentNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipseExtentMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testEllipseFromFociNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipseFociMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 1, 1 );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testEllipseFrom4PointsNotEnoughPoints()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();
  const long long count = layer->featureCount();

  QgsMapToolShapeEllipse4PointsMetadata md;
  resetMapTool( &md );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 0, 0, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  utils.keyClick( Qt::Key_Escape );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 1, 1, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::LeftButton );
  utils.mouseClick( 2, 2, Qt::RightButton );
  QCOMPARE( layer->featureCount(), count );

  layer->rollBack();
}

void TestQgsMapToolEllipse::testTransientGeometrySignalCenterPoint()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();

  resetMapTool( new QgsMapToolShapeEllipseCenterPointMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  QSignalSpy spy( mMapTool, &QgsMapToolCapture::transientGeometryChanged );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 2, 1 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((2 0, 2 -0.1, 2 -0.1, 2 -0.2, 1.9 -0.3, 1.9 -0.3, 1.8 -0.4, 1.8 -0.4, 1.7 -0.5, 1.7 -0.6, 1.6 -0.6, 1.5 -0.7, 1.4 -0.7, 1.3 -0.8, 1.2"_s );

  utils.mouseMove( 2, 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((2 0, 2 -0.1, 2 -0.3, 2 -0.4, 1.9 -0.5, 1.9 -0.6, 1.8 -0.8, 1.8 -0.9, 1.7 -1, 1.7 -1.1, 1.6 -1.2, 1.5 -1.3, 1.4 -1.4, 1.3 -1.5, 1.2 -"_s );

  utils.mouseClick( 2, 1, Qt::RightButton );
  layer->rollBack();
}

void TestQgsMapToolEllipse::testTransientGeometrySignalCenter2Points()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();

  resetMapTool( new QgsMapToolShapeEllipseCenter2PointsMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  QSignalSpy spy( mMapTool, &QgsMapToolCapture::transientGeometryChanged );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 2, 0, Qt::LeftButton );
  utils.mouseMove( 3, 1 );

  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((2 0, 2 -0.1, 2 -0.2, 2 -0.3, 1.9 -0.4, 1.9 -0.5, 1.8 -0.5, 1.8 -0.6, 1.7 -0.7, 1.7 -0.8, 1.6 -0.9, 1.5 -0.9, 1.4 -1, 1.3 -1.1, 1.2 -"_s );

  utils.mouseMove( 3, 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((0 -2.2, -0.1 -2.2, -0.3 -2.2, -0.4 -2.2, -0.5 -2.2, -0.6 -2.1, -0.8 -2.1, -0.9 -2, -1 -1.9, -1.1 -1.9, -1.2 -1.8, -1.3 -1.7, -1.4 -1"_s );

  utils.mouseClick( 0, 1, Qt::RightButton );
  layer->rollBack();
}

void TestQgsMapToolEllipse::testTransientGeometrySignalExtent()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();

  resetMapTool( new QgsMapToolShapeEllipseExtentMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  QSignalSpy spy( mMapTool, &QgsMapToolCapture::transientGeometryChanged );

  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseMove( 4, 2 );

  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((4 1, 4 0.9, 4 0.9, 4 0.8, 3.9 0.7, 3.9 0.7, 3.8 0.6, 3.8 0.6, 3.7 0.5, 3.7 0.4, 3.6 0.4, 3.5 0.3, 3.4 0.3, 3.3 0.2, 3.2 0.2, 3.1 0.2"_s );

  utils.mouseMove( 3, 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((3 1, 3 0.9, 3 0.9, 3 0.8, 2.9 0.7, 2.9 0.7, 2.9 0.6, 2.8 0.6, 2.8 0.5, 2.7 0.4, 2.7 0.4, 2.6 0.3, 2.6 0.3, 2.5 0.2, 2.4 0.2, 2.3 0.2"_s );

  utils.mouseClick( 4, 2, Qt::RightButton );
  layer->rollBack();
}

void TestQgsMapToolEllipse::testTransientGeometrySignalFoci()
{
  QgsVectorLayer *layer = mVectorLayerMap["XY"].get();
  mCanvas->setCurrentLayer( layer );
  layer->startEditing();

  resetMapTool( new QgsMapToolShapeEllipseFociMetadata() );

  TestQgsMapToolAdvancedDigitizingUtils utils( mMapTool );
  QSignalSpy spy( mMapTool, &QgsMapToolCapture::transientGeometryChanged );
  utils.mouseClick( 0, 0, Qt::LeftButton );
  utils.mouseClick( 4, 0, Qt::LeftButton );
  utils.mouseMove( 2, 3 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.at( 0 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((5.6 0, 5.6 -0.5, 5.4 -1, 5.1 -1.5, 4.8 -1.9, 4.3 -2.3, 3.8 -2.6, 3.2 -2.8, 2.6 -3, 2 -3, 1.4 -3, 0.8 -2.8, 0.2 -2.6, -0.3 -2.3, -0.8"_s );

  utils.mouseMove( 3, 2 );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.at( 1 ).at( 0 ).value< QgsReferencedGeometry >().asWkt( 1 ).left( 142 ), u"Polygon ((4.9 0, 4.9 -0.4, 4.7 -0.7, 4.5 -1.1, 4.2 -1.4, 3.9 -1.6, 3.5 -1.8, 3 -2, 2.5 -2.1, 2 -2.1, 1.5 -2.1, 1 -2, 0.5 -1.8, 0.1 -1.6, -0.2 "_s );

  utils.mouseClick( 2, 3, Qt::RightButton );
  layer->rollBack();
}

QGSTEST_MAIN( TestQgsMapToolEllipse )
#include "testqgsmaptoolellipse.moc"
