/***************************************************************************
  testqgsvectortilelayer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
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
#include <QObject>
#include <QString>

//qgis includes...
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrenderchecker.h"
#include "qgstiles.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgsfontutils.h"
#include "qgslinesymbollayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for a vector tile layer
 */
class TestQgsVectorTileLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileLayer() = default;

  private:
    QString mDataDir;
    QgsVectorTileLayer *mLayer = nullptr;
    QString mReport;
    QgsMapSettings *mMapSettings = nullptr;

    bool imageCheck( const QString &testType, QgsVectorTileLayer *layer, QgsRectangle extent );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_basic();
    void test_render();
    void test_render_withClip();
    void test_labeling();
    void test_relativePaths();
    void test_polygonWithLineStyle();
    void test_polygonWithMarker();
};


void TestQgsVectorTileLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/vector_tile";

  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  mLayer = new QgsVectorTileLayer( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( mLayer->isValid() );

  QgsProject::instance()->addMapLayer( mLayer );

  mMapSettings = new QgsMapSettings();
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLayer );

  // let's have some standard style config for the layer
  QColor polygonFillColor = Qt::blue;
  const QColor polygonStrokeColor = polygonFillColor;
  polygonFillColor.setAlpha( 100 );
  const double polygonStrokeWidth = DEFAULT_LINE_WIDTH * 2;
  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = DEFAULT_LINE_WIDTH * 2;
  QColor pointFillColor = Qt::red;
  const QColor pointStrokeColor = pointFillColor;
  pointFillColor.setAlpha( 100 );
  const double pointSize = DEFAULT_POINT_SIZE;

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QgsVectorTileBasicRenderer::simpleStyle(
                     polygonFillColor, polygonStrokeColor, polygonStrokeWidth,
                     lineStrokeColor, lineStrokeWidth,
                     pointFillColor, pointStrokeColor, pointSize ) );
  mLayer->setRenderer( rend );  // takes ownership

  mReport += QLatin1String( "<h1>Vector Tile Layer Tests</h1>\n" );
}

void TestQgsVectorTileLayer::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsVectorTileLayer::test_basic()
{
  // tile fetch test
  const QByteArray tile0rawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 0 ) );
  QCOMPARE( tile0rawData.length(), 64822 );

  const QByteArray invalidTileRawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 99 ) );
  QCOMPARE( invalidTileRawData.length(), 0 );

  // an xyz vector tile layer should be considered as a basemap layer
  QCOMPARE( mLayer->properties(), Qgis::MapLayerProperties( Qgis::MapLayerProperty::IsBasemapLayer ) );
}


bool TestQgsVectorTileLayer::imageCheck( const QString &testType, QgsVectorTileLayer *layer, QgsRectangle extent )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setExtent( extent );
  mMapSettings->setDestinationCrs( layer->crs() );
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "vector_tile" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  const bool myResultFlag = myChecker.runTest( testType, 0 );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsVectorTileLayer::test_render()
{
  QVERIFY( imageCheck( "render_test_basic", mLayer, mLayer->extent() ) );
}

void TestQgsVectorTileLayer::test_render_withClip()
{
  QgsMapClippingRegion region( QgsGeometry::fromWkt( "Polygon ((-3584104.41462873760610819 9642431.51156153343617916, -3521836.1401221314445138 -3643384.67029104987159371, -346154.14028519613202661 -10787760.6154897827655077, 11515952.15322335436940193 -10530608.51481428928673267, 11982964.21202290244400501 11308099.1972544826567173, -3584104.41462873760610819 9642431.51156153343617916))" ) );
  region.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly );
  QgsMapClippingRegion region2( QgsGeometry::fromWkt( "Polygon ((836943.07534032803960145 12108307.34630974195897579, 1179418.58512666448950768 -8011790.66139839310199022, 17306901.68233776465058327 -8130936.37545258551836014, 17680511.32937740534543991 14072993.65374799631536007, 836943.07534032803960145 12108307.34630974195897579))" ) );
  region2.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection );
  mMapSettings->addClippingRegion( region );
  mMapSettings->addClippingRegion( region2 );
  const bool res = imageCheck( "render_painterclip", mLayer, mLayer->extent() );
  mMapSettings->setClippingRegions( QList< QgsMapClippingRegion >() );
  QVERIFY( res );
}

void TestQgsVectorTileLayer::test_labeling()
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );

  QgsPalLayerSettings labelSettings;
  labelSettings.drawLabels = true;
  labelSettings.fieldName = "name:en";
  labelSettings.placement = QgsPalLayerSettings::OverPoint;
  labelSettings.setFormat( format );

  QgsVectorTileBasicLabelingStyle st;
  st.setStyleName( "st1" );
  st.setLayerName( "place" );
  st.setFilterExpression( "rank = 1 AND class = 'country'" );
  st.setGeometryType( QgsWkbTypes::PointGeometry );
  st.setLabelSettings( labelSettings );

  QgsVectorTileBasicLabeling *labeling = new QgsVectorTileBasicLabeling;
  QList<QgsVectorTileBasicLabelingStyle> lst;
  lst << st;
  labeling->setStyles( lst );

  mLayer->setLabeling( labeling );

  QgsVectorTileRenderer *oldRenderer = mLayer->renderer()->clone();

  // use a different renderer to make the labels stand out more
  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QgsVectorTileBasicRenderer::simpleStyle(
                     Qt::transparent, Qt::white, DEFAULT_LINE_WIDTH * 2,
                     Qt::transparent, 0,
                     Qt::transparent, Qt::transparent, 0 ) );
  mLayer->setRenderer( rend );  // takes ownership

  const bool res = imageCheck( "render_test_labeling", mLayer, mLayer->extent() );

  mLayer->setRenderer( oldRenderer );

  QVERIFY( res );
}

void TestQgsVectorTileLayer::test_relativePaths()
{
  QgsReadWriteContext contextRel;
  contextRel.setPathResolver( QgsPathResolver( "/home/qgis/project.qgs" ) );
  const QgsReadWriteContext contextAbs;

  const QString srcXyzLocal = "type=xyz&url=file:///home/qgis/%7Bz%7D/%7Bx%7D/%7By%7D.pbf";
  const QString srcXyzRemote = "type=xyz&url=http://www.example.com/%7Bz%7D/%7Bx%7D/%7By%7D.pbf";
  const QString srcMbtiles = "type=mbtiles&url=/home/qgis/test/map.mbtiles";

  const QgsVectorTileLayer layer;

  // encode source: converting absolute paths to relative
  const QString srcXyzLocalRel = layer.encodedSource( srcXyzLocal, contextRel );
  QCOMPARE( srcXyzLocalRel, QStringLiteral( "type=xyz&url=file:./%7Bz%7D/%7Bx%7D/%7By%7D.pbf" ) );
  const QString srcMbtilesRel = layer.encodedSource( srcMbtiles, contextRel );
  QCOMPARE( srcMbtilesRel, QStringLiteral( "type=mbtiles&url=./test/map.mbtiles" ) );
  QCOMPARE( layer.encodedSource( srcXyzRemote, contextRel ), srcXyzRemote );

  // encode source: keeping absolute paths
  QCOMPARE( layer.encodedSource( srcXyzLocal, contextAbs ), srcXyzLocal );
  QCOMPARE( layer.encodedSource( srcXyzRemote, contextAbs ), srcXyzRemote );
  QCOMPARE( layer.encodedSource( srcMbtiles, contextAbs ), srcMbtiles );

  // decode source: converting relative paths to absolute
  QCOMPARE( layer.decodedSource( srcXyzLocalRel, QString(), contextRel ), srcXyzLocal );
  QCOMPARE( layer.decodedSource( srcMbtilesRel, QString(), contextRel ), srcMbtiles );
  QCOMPARE( layer.decodedSource( srcXyzRemote, QString(), contextRel ), srcXyzRemote );

  // decode source: keeping absolute paths
  QCOMPARE( layer.decodedSource( srcXyzLocal, QString(), contextAbs ), srcXyzLocal );
  QCOMPARE( layer.decodedSource( srcXyzRemote, QString(), contextAbs ), srcXyzRemote );
  QCOMPARE( layer.decodedSource( srcMbtiles, QString(), contextAbs ), srcMbtiles );
}

void TestQgsVectorTileLayer::test_polygonWithLineStyle()
{
  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  std::unique_ptr< QgsVectorTileLayer > layer = std::make_unique< QgsVectorTileLayer >( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = DEFAULT_LINE_WIDTH * 2;

  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer->setColor( lineStrokeColor );
  lineSymbolLayer->setWidth( lineStrokeWidth );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );

  QgsVectorTileBasicRendererStyle st( QStringLiteral( "Polygons" ), QString(), QgsWkbTypes::LineGeometry );
  st.setSymbol( lineSymbol );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( QStringLiteral( "background" ), QStringLiteral( "background" ), QgsWkbTypes::PolygonGeometry );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend );  // takes ownership

  QVERIFY( imageCheck( "render_test_polygon_with_line_style", layer.get(), layer->extent() ) );
}

void TestQgsVectorTileLayer::test_polygonWithMarker()
{
  // a polygon in a vector tile layer which is matched by a marker rule should result in a point-inside-polygon placement
  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  std::unique_ptr< QgsVectorTileLayer > layer = std::make_unique< QgsVectorTileLayer >( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor markerColor = Qt::blue;

  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer;
  markerSymbolLayer->setColor( markerColor );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );

  QgsVectorTileBasicRendererStyle st( QStringLiteral( "Polygons" ), QString(), QgsWkbTypes::PointGeometry );
  st.setSymbol( markerSymbol );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( QStringLiteral( "background" ), QStringLiteral( "background" ), QgsWkbTypes::PolygonGeometry );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend );  // takes ownership

  QVERIFY( imageCheck( "render_test_polygon_with_marker", layer.get(), layer->extent() ) );
}


QGSTEST_MAIN( TestQgsVectorTileLayer )
#include "testqgsvectortilelayer.moc"
