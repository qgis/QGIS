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
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsproviderutils.h"
#include "qgsvectortileloader.h"
#include "qgsvectortilemvtdecoder.h"

/**
 * \ingroup UnitTests
 * This is a unit test for a vector tile layer
 */
class TestQgsVectorTileLayer : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsVectorTileLayer()
      : QgsTest( u"Vector Tile Layer Tests"_s, u"vector_tile"_s ) {}

  private:
    QString mDataDir;
    QgsVectorTileLayer *mLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void test_basic();
    void test_render();
    void test_render_withClip();
    void test_labeling();
    void test_labeling_clone();

    void testMbtilesProviderMetadata();
    void test_relativePathsMbTiles();
    void test_absoluteRelativeUriMbTiles();
    void test_mbtilesZoom16();

    void test_relativePathsXyz();
    void test_absoluteRelativeUriXyz();

    void testVtpkProviderMetadata();
    void test_relativePathsVtpk();
    void test_absoluteRelativeUriVtpk();

    void test_polygonWithLineStyle();
    void test_polygonWithMarker();

    void test_styleMinZoomBeyondTileMaxZoom();
    void test_filterRuleAllLayers();

    void test_longIntAttributes();
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
  mMapSettings->setOutputDpi( 96 );

  // let's have some standard style config for the layer
  QColor polygonFillColor = Qt::blue;
  const QColor polygonStrokeColor = polygonFillColor;
  polygonFillColor.setAlpha( 100 );
  const double polygonStrokeWidth = Qgis::DEFAULT_LINE_WIDTH * 2;
  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = Qgis::DEFAULT_LINE_WIDTH * 2;
  QColor pointFillColor = Qt::red;
  const QColor pointStrokeColor = pointFillColor;
  pointFillColor.setAlpha( 100 );
  const double pointSize = Qgis::DEFAULT_POINT_SIZE;

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QgsVectorTileBasicRenderer::simpleStyle(
    polygonFillColor, polygonStrokeColor, polygonStrokeWidth,
    lineStrokeColor, lineStrokeWidth,
    pointFillColor, pointStrokeColor, pointSize
  ) );
  mLayer->setRenderer( rend ); // takes ownership
}

void TestQgsVectorTileLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorTileLayer::test_basic()
{
  // tile fetch test
  const QgsVectorTileRawData tile0rawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 0 ) );
  QCOMPARE( tile0rawData.data.first().length(), 64822 );

  const QgsVectorTileRawData invalidTileRawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 99 ) );
  QCOMPARE( invalidTileRawData.data.first().length(), 0 );

  // an xyz vector tile layer should be considered as a basemap layer
  QCOMPARE( mLayer->properties(), Qgis::MapLayerProperties( Qgis::MapLayerProperty::IsBasemapLayer ) );
}

void TestQgsVectorTileLayer::test_render()
{
  mMapSettings->setExtent( mLayer->extent() );
  mMapSettings->setDestinationCrs( mLayer->crs() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"render_test_basic"_s, u"render_test_basic"_s, *mMapSettings, 0, 15 );
}

void TestQgsVectorTileLayer::test_render_withClip()
{
  QgsMapClippingRegion region( QgsGeometry::fromWkt( "Polygon ((-3584104.41462873760610819 9642431.51156153343617916, -3521836.1401221314445138 -3643384.67029104987159371, -346154.14028519613202661 -10787760.6154897827655077, 11515952.15322335436940193 -10530608.51481428928673267, 11982964.21202290244400501 11308099.1972544826567173, -3584104.41462873760610819 9642431.51156153343617916))" ) );
  region.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipPainterOnly );
  QgsMapClippingRegion region2( QgsGeometry::fromWkt( "Polygon ((836943.07534032803960145 12108307.34630974195897579, 1179418.58512666448950768 -8011790.66139839310199022, 17306901.68233776465058327 -8130936.37545258551836014, 17680511.32937740534543991 14072993.65374799631536007, 836943.07534032803960145 12108307.34630974195897579))" ) );
  region2.setFeatureClip( QgsMapClippingRegion::FeatureClippingType::ClipToIntersection );
  mMapSettings->addClippingRegion( region );
  mMapSettings->addClippingRegion( region2 );

  mMapSettings->setExtent( mLayer->extent() );
  mMapSettings->setDestinationCrs( mLayer->crs() );
  const bool res = QGSRENDERMAPSETTINGSCHECK( u"render_painterclip"_s, u"render_painterclip"_s, *mMapSettings, 0, 15 );
  mMapSettings->setClippingRegions( QList<QgsMapClippingRegion>() );
  QVERIFY( res );
}

void TestQgsVectorTileLayer::test_labeling()
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ).family() );
  format.setSize( 12 );
  format.setNamedStyle( u"Bold"_s );
  format.setColor( QColor( 200, 0, 200 ) );

  QgsPalLayerSettings labelSettings;
  labelSettings.drawLabels = true;
  labelSettings.fieldName = "name:en";
  labelSettings.placement = Qgis::LabelPlacement::OverPoint;
  labelSettings.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Show, QgsProperty::fromExpression( "\"name:en\" IN ('Canada', 'Mexico', 'Argentina', 'Russia', 'Italy', 'China', 'Australia')" ) );
  labelSettings.setFormat( format );

  QgsVectorTileBasicLabelingStyle st;
  st.setStyleName( "st1" );
  st.setLayerName( "place" );
  st.setFilterExpression( "rank = 1 AND class = 'country'" );
  st.setGeometryType( Qgis::GeometryType::Point );
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
    Qt::transparent, Qt::white, Qgis::DEFAULT_LINE_WIDTH * 2,
    Qt::transparent, 0,
    Qt::transparent, Qt::transparent, 0
  ) );
  mLayer->setRenderer( rend ); // takes ownership

  mMapSettings->setExtent( mLayer->extent() );
  mMapSettings->setDestinationCrs( mLayer->crs() );
  const bool res1 = QGSRENDERMAPSETTINGSCHECK( u"render_test_labeling"_s, u"render_test_labeling"_s, *mMapSettings, 0, 15 );

  // disable label rendering
  mLayer->setLabelsEnabled( false );
  const bool res2 = QGSRENDERMAPSETTINGSCHECK( u"render_test_labeling_disabled"_s, u"render_test_labeling_disabled"_s, *mMapSettings, 0, 15 );

  mLayer->setRenderer( oldRenderer );

  QVERIFY( res1 );
  QVERIFY( res2 );
}

void TestQgsVectorTileLayer::test_labeling_clone()
{
  QgsVectorTileBasicLabeling *labeling = new QgsVectorTileBasicLabeling;
  mLayer->setLabeling( labeling );

  std::unique_ptr<QgsVectorTileLayer> vtlClone;
  vtlClone.reset( mLayer->clone() );
  QVERIFY( vtlClone->isValid() );
  QVERIFY( vtlClone->labeling() != nullptr );
}

void TestQgsVectorTileLayer::testMbtilesProviderMetadata()
{
  QgsProviderMetadata *vectorTileMetadata = QgsProviderRegistry::instance()->providerMetadata( "mbtilesvectortiles" );
  QVERIFY( vectorTileMetadata );

  // not mbtile uris
  QCOMPARE( vectorTileMetadata->priorityForUri( QString() ), 0 );
  QCOMPARE( vectorTileMetadata->validLayerTypesForUri( QString() ), {} );

  QCOMPARE( vectorTileMetadata->priorityForUri( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ), 0 );
  QVERIFY( vectorTileMetadata->validLayerTypesForUri( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ).isEmpty() );
  QVERIFY( vectorTileMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ).isEmpty() );

  QCOMPARE( vectorTileMetadata->priorityForUri( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ), 0 );
  QVERIFY( vectorTileMetadata->validLayerTypesForUri( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ).isEmpty() );
  QVERIFY( vectorTileMetadata->querySublayers( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ).isEmpty() );

  // mbtile uris
  QCOMPARE( vectorTileMetadata->priorityForUri( u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) ), 100 );
  QCOMPARE( vectorTileMetadata->validLayerTypesForUri( u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) ), { Qgis::LayerType::VectorTile } );

  QCOMPARE( vectorTileMetadata->priorityForUri( u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) ), 100 );
  QCOMPARE( vectorTileMetadata->validLayerTypesForUri( u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) ), { Qgis::LayerType::VectorTile } );

  // query sublayers
  QString localMbtilesPath = u"%1%2"_s.arg( QUrl::toPercentEncoding( TEST_DATA_DIR ), QUrl::toPercentEncoding( u"/vector_tile/mbtiles_vt.mbtiles"_s ) );
  QList<QgsProviderSublayerDetails> sublayers = vectorTileMetadata->querySublayers( u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), u"mbtilesvectortiles"_s );
  QCOMPARE( sublayers.at( 0 ).name(), u"mbtiles_vt"_s );
  QCOMPARE( sublayers.at( 0 ).uri(), u"type=mbtiles&url=%1"_s.arg( localMbtilesPath ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );
  QVERIFY( !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers ) );

  sublayers = vectorTileMetadata->querySublayers( u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), u"mbtilesvectortiles"_s );
  QCOMPARE( sublayers.at( 0 ).name(), u"mbtiles_vt"_s );
  QCOMPARE( sublayers.at( 0 ).uri(), u"type=mbtiles&url=%1"_s.arg( localMbtilesPath ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );

  // fast scan flag
  sublayers = vectorTileMetadata->querySublayers( u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), u"mbtilesvectortiles"_s );
  QCOMPARE( sublayers.at( 0 ).name(), u"mbtiles_vt"_s );
  QCOMPARE( sublayers.at( 0 ).uri(), u"type=mbtiles&url=%1"_s.arg( localMbtilesPath ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );
  QVERIFY( QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers ) );

  sublayers = vectorTileMetadata->querySublayers( u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), u"mbtilesvectortiles"_s );
  QCOMPARE( sublayers.at( 0 ).name(), u"mbtiles_vt"_s );
  QCOMPARE( sublayers.at( 0 ).uri(), u"type=mbtiles&url=%1"_s.arg( localMbtilesPath ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );

  // fast scan mode means that any mbtile file will be reported, including those with only raster tiles
  // (we are skipping a potentially expensive db open and format check)
  QString localIsleOfManPath = u"%1%2"_s.arg( QUrl::toPercentEncoding( TEST_DATA_DIR ), QUrl::toPercentEncoding( u"/isle_of_man.mbtiles"_s ) );

  sublayers = vectorTileMetadata->querySublayers( u"%1/isle_of_man.mbtiles"_s.arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), u"mbtilesvectortiles"_s );
  QCOMPARE( sublayers.at( 0 ).name(), u"isle_of_man"_s );
  QCOMPARE( sublayers.at( 0 ).uri(), u"type=mbtiles&url=%1"_s.arg( localIsleOfManPath ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );

  // test that mbtilesvectortiles provider is the preferred provider for vector tile mbtiles files
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"type=mbtiles&url=%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
  // wms provider also reports handling this url
  int vtProviderIndex = candidates.at( 0 ).metadata()->key() == "mbtilesvectortiles"_L1 ? 0 : 1;
  QCOMPARE( candidates.size(), 2 );
  QCOMPARE( candidates.at( vtProviderIndex ).metadata()->key(), u"mbtilesvectortiles"_s );
  QCOMPARE( candidates.at( vtProviderIndex ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::VectorTile );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"%1/vector_tile/mbtiles_vt.mbtiles"_s.arg( TEST_DATA_DIR ) );
  // wms provider also reports handling this url
  QCOMPARE( candidates.size(), 2 );
  vtProviderIndex = candidates.at( 0 ).metadata()->key() == "mbtilesvectortiles"_L1 ? 0 : 1;
  QCOMPARE( candidates.at( vtProviderIndex ).metadata()->key(), u"mbtilesvectortiles"_s );
  QCOMPARE( candidates.at( vtProviderIndex ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::VectorTile );

  QCOMPARE( vectorTileMetadata->filters( Qgis::FileFilterType::VectorTile ), u"Mbtiles Vector Tiles (*.mbtiles *.MBTILES)"_s );
  QCOMPARE( vectorTileMetadata->filters( Qgis::FileFilterType::PointCloud ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->fileVectorTileFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.mbtiles *.MBTILES)" ) );
}

void TestQgsVectorTileLayer::test_relativePathsMbTiles()
{
  QgsReadWriteContext contextRel;
  contextRel.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );
  const QgsReadWriteContext contextAbs;
  QString localMbtilesPath = u"%1%2"_s.arg( QUrl::toPercentEncoding( TEST_DATA_DIR ), QUrl::toPercentEncoding( u"/vector_tile/mbtiles_vt.mbtiles"_s ) );

  const QString srcMbtiles = u"type=mbtiles&url=%1"_s.arg( localMbtilesPath );

  auto layer = std::make_unique<QgsVectorTileLayer>( srcMbtiles );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->providerType(), u"mbtilesvectortiles"_s );

  // encode source: converting absolute paths to relative
  const QString srcMbtilesRel = layer->encodedSource( srcMbtiles, contextRel );
  QCOMPARE( srcMbtilesRel, u"type=mbtiles&url=.%2Fvector_tile%2Fmbtiles_vt.mbtiles"_s );

  // encode source: keeping absolute paths
  QCOMPARE( layer->encodedSource( srcMbtiles, contextAbs ), srcMbtiles );

  // decode source: converting relative paths to absolute
  QCOMPARE( layer->decodedSource( srcMbtilesRel, u"mbtilesvectortiles"_s, contextRel ), srcMbtiles );

  // decode source: keeping absolute paths
  QCOMPARE( layer->decodedSource( srcMbtiles, u"mbtilesvectortiles"_s, contextAbs ), srcMbtiles );
}

void TestQgsVectorTileLayer::test_absoluteRelativeUriMbTiles()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *vectorTileMetadata = QgsProviderRegistry::instance()->providerMetadata( "mbtilesvectortiles" );
  QVERIFY( vectorTileMetadata );

  QgsDataSourceUri dsAbs;
  dsAbs.setParam( "type", "mbtiles" );
  dsAbs.setParam( "url", QString( "%1/vector_tile/mbtiles_vt.mbtiles" ).arg( TEST_DATA_DIR ) );

  QgsDataSourceUri dsRel;
  dsRel.setParam( "type", "mbtiles" );
  dsRel.setParam( "url", "./vector_tile/mbtiles_vt.mbtiles" );

  QString absoluteUri = dsAbs.encodedUri();
  QString relativeUri = dsRel.encodedUri();
  QCOMPARE( vectorTileMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( vectorTileMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsVectorTileLayer::test_mbtilesZoom16()
{
  const QString srcMbtiles = u"type=mbtiles&url=%1/vector_tile/z16.mbtiles"_s.arg( TEST_DATA_DIR );

  auto layer = std::make_unique<QgsVectorTileLayer>( srcMbtiles );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->providerType(), u"mbtilesvectortiles"_s );
  QCOMPARE( layer->sourceMinZoom(), 16 );
  QCOMPARE( layer->sourceMaxZoom(), 16 );
}

void TestQgsVectorTileLayer::test_relativePathsXyz()
{
  QgsReadWriteContext contextRel;
  contextRel.setPathResolver( QgsPathResolver( "/home/qgis/project.qgs" ) );
  const QgsReadWriteContext contextAbs;

  const QString srcXyzLocal = "type=xyz&url=file%3A%2F%2F%2Fhome%2Fqgis%2F%7Bz%7D%2F%7Bx%7D%2F%7By%7D.pbf";
  const QString srcXyzRemote = "type=xyz&url=http%3A%2F%2Fwww.example.com%2F%7Bz%7D%2F%7Bx%7D%2F%7By%7D.pbf";

  auto layer = std::make_unique<QgsVectorTileLayer>( srcXyzLocal );
  QCOMPARE( layer->providerType(), u"xyzvectortiles"_s );

  // encode source: converting absolute paths to relative
  const QString srcXyzLocalRel = layer->encodedSource( srcXyzLocal, contextRel );
  QCOMPARE( srcXyzLocalRel, u"type=xyz&url=file%3A.%2F%7Bz%7D%2F%7Bx%7D%2F%7By%7D.pbf"_s );
  QCOMPARE( layer->encodedSource( srcXyzRemote, contextRel ), srcXyzRemote );

  // encode source: keeping absolute paths
  QCOMPARE( layer->encodedSource( srcXyzLocal, contextAbs ), srcXyzLocal );
  QCOMPARE( layer->encodedSource( srcXyzRemote, contextAbs ), srcXyzRemote );

  // decode source: converting relative paths to absolute
  QCOMPARE( layer->decodedSource( srcXyzLocalRel, u"xyzvectortiles"_s, contextRel ), srcXyzLocal );
  QCOMPARE( layer->decodedSource( srcXyzRemote, u"xyzvectortilesx"_s, contextRel ), srcXyzRemote );

  // decode source: keeping absolute paths
  QCOMPARE( layer->decodedSource( srcXyzLocal, u"xyzvectortiles"_s, contextAbs ), srcXyzLocal );
  QCOMPARE( layer->decodedSource( srcXyzRemote, u"xyzvectortiles"_s, contextAbs ), srcXyzRemote );
}

void TestQgsVectorTileLayer::test_absoluteRelativeUriXyz()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *vectorTileMetadata = QgsProviderRegistry::instance()->providerMetadata( "xyzvectortiles" );
  QVERIFY( vectorTileMetadata );

  QgsDataSourceUri dsAbs;
  dsAbs.setParam( "type", "xyz" );
  dsAbs.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  dsAbs.setParam( "zmax", "1" );

  QgsDataSourceUri dsRel;
  dsRel.setParam( "type", "xyz" );
  dsRel.setParam( "url", "file:./vector_tile/{z}-{x}-{y}.pbf" );
  dsRel.setParam( "zmax", "1" );

  QString absoluteUri = dsAbs.encodedUri();
  QString relativeUri = dsRel.encodedUri();
  QString absToRelUri = vectorTileMetadata->absoluteToRelativeUri( absoluteUri, context );
  QCOMPARE( absToRelUri, relativeUri );
  QCOMPARE( vectorTileMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsVectorTileLayer::testVtpkProviderMetadata()
{
  QgsProviderMetadata *vectorTileMetadata = QgsProviderRegistry::instance()->providerMetadata( "vtpkvectortiles" );
  QVERIFY( vectorTileMetadata );

  // not vtpk uris
  QCOMPARE( vectorTileMetadata->priorityForUri( QString() ), 0 );
  QCOMPARE( vectorTileMetadata->validLayerTypesForUri( QString() ), {} );

  QCOMPARE( vectorTileMetadata->priorityForUri( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ), 0 );
  QVERIFY( vectorTileMetadata->validLayerTypesForUri( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ).isEmpty() );
  QVERIFY( vectorTileMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s ).isEmpty() );

  QCOMPARE( vectorTileMetadata->priorityForUri( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ), 0 );
  QVERIFY( vectorTileMetadata->validLayerTypesForUri( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ).isEmpty() );
  QVERIFY( vectorTileMetadata->querySublayers( u"type=vtpk&url=%1/points.shp"_s.arg( TEST_DATA_DIR ) ).isEmpty() );

  // vtpk uris
  QString localVtpkPath = u"%1%2"_s.arg( QUrl::toPercentEncoding( TEST_DATA_DIR ), QUrl::toPercentEncoding( u"/testvtpk.vtpk"_s ) );

  for ( auto uriStr : {
          u"%1/%2"_s.arg( TEST_DATA_DIR ).arg( "testvtpk.vtpk" ), //
          u"type=vtpk&url=%1"_s.arg( localVtpkPath ),             //
          u"type=vtpk&url=%1"_s.arg( QStringLiteral( TEST_DATA_DIR ) + u"/testvtpk.vtpk"_s )
        } )
  {
    QCOMPARE( vectorTileMetadata->priorityForUri( uriStr ), 100 );
    QCOMPARE( vectorTileMetadata->validLayerTypesForUri( uriStr ), { Qgis::LayerType::VectorTile } );
    QList<QgsProviderSublayerDetails> sublayers = vectorTileMetadata->querySublayers( uriStr );
    QCOMPARE( sublayers.size(), 1 );
    QCOMPARE( sublayers.at( 0 ).providerKey(), u"vtpkvectortiles"_s );
    QCOMPARE( sublayers.at( 0 ).name(), u"testvtpk"_s );
    QCOMPARE( sublayers.at( 0 ).uri(), u"type=vtpk&url=%1"_s.arg( localVtpkPath ) );
    QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::VectorTile );
  }

  // test that vtpk provider is the preferred provider for vtpk files
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( u"type=vtpk&url=%1/testvtpk.vtpk"_s.arg( TEST_DATA_DIR ) );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"vtpkvectortiles"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::VectorTile );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( TEST_DATA_DIR ) + u"/testvtpk.vtpk"_s );
  QCOMPARE( candidates.size(), 1 );
  QCOMPARE( candidates.at( 0 ).metadata()->key(), u"vtpkvectortiles"_s );
  QCOMPARE( candidates.at( 0 ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::VectorTile );

  QCOMPARE( vectorTileMetadata->filters( Qgis::FileFilterType::VectorTile ), u"VTPK Vector Tiles (*.vtpk *.VTPK)"_s );
  QCOMPARE( vectorTileMetadata->filters( Qgis::FileFilterType::PointCloud ), QString() );

  const QString registryPointCloudFilters = QgsProviderRegistry::instance()->fileVectorTileFilters();
  QVERIFY( registryPointCloudFilters.contains( "(*.vtpk *.VTPK)" ) );
}

void TestQgsVectorTileLayer::test_relativePathsVtpk()
{
  QgsReadWriteContext contextRel;
  contextRel.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );
  const QgsReadWriteContext contextAbs;

  QString localVtpkPath = u"%1%2"_s.arg( QUrl::toPercentEncoding( TEST_DATA_DIR ), QUrl::toPercentEncoding( u"/testvtpk.vtpk"_s ) );

  const QString srcVtpk = u"type=vtpk&url=%1"_s.arg( localVtpkPath );

  auto layer = std::make_unique<QgsVectorTileLayer>( srcVtpk );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->providerType(), u"vtpkvectortiles"_s );

  // encode source: converting absolute paths to relative
  const QString srcVtpkRel = layer->encodedSource( srcVtpk, contextRel );
  QCOMPARE( srcVtpkRel, u"type=vtpk&url=.%2Ftestvtpk.vtpk"_s );

  // encode source: keeping absolute paths
  QCOMPARE( layer->encodedSource( srcVtpk, contextAbs ), srcVtpk );

  // decode source: converting relative paths to absolute
  QCOMPARE( layer->decodedSource( srcVtpkRel, u"vtpkvectortiles"_s, contextRel ), srcVtpk );

  // decode source: keeping absolute paths
  QCOMPARE( layer->decodedSource( srcVtpk, u"vtpkvectortiles"_s, contextAbs ), srcVtpk );
}

void TestQgsVectorTileLayer::test_absoluteRelativeUriVtpk()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + u"/project.qgs"_s ) );

  QgsProviderMetadata *vectorTileMetadata = QgsProviderRegistry::instance()->providerMetadata( "vtpkvectortiles" );
  QVERIFY( vectorTileMetadata );

  QgsDataSourceUri dsAbs;
  dsAbs.setParam( "type", "vtpk" );
  dsAbs.setParam( "url", QString( "%1/testvtpk.vtpk" ).arg( TEST_DATA_DIR ) );

  QgsDataSourceUri dsRel;
  dsRel.setParam( "type", "vtpk" );
  dsRel.setParam( "url", "./testvtpk.vtpk" );

  QString absoluteUri = dsAbs.encodedUri();
  QString relativeUri = dsRel.encodedUri();
  QCOMPARE( vectorTileMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( vectorTileMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}


void TestQgsVectorTileLayer::test_polygonWithLineStyle()
{
  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = Qgis::DEFAULT_LINE_WIDTH * 2;

  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer->setColor( lineStrokeColor );
  lineSymbolLayer->setWidth( lineStrokeWidth );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );

  QgsVectorTileBasicRendererStyle st( u"Polygons"_s, QString(), Qgis::GeometryType::Line );
  st.setSymbol( lineSymbol );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( u"background"_s, u"background"_s, Qgis::GeometryType::Polygon );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend ); // takes ownership

  mMapSettings->setExtent( layer->extent() );
  mMapSettings->setDestinationCrs( layer->crs() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"render_test_polygon_with_line_style"_s, u"render_test_polygon_with_line_style"_s, *mMapSettings, 0, 15 );
}

void TestQgsVectorTileLayer::test_polygonWithMarker()
{
  // a polygon in a vector tile layer which is matched by a marker rule should result in a point-inside-polygon placement
  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor markerColor = Qt::blue;

  QgsSimpleMarkerSymbolLayer *markerSymbolLayer = new QgsSimpleMarkerSymbolLayer;
  markerSymbolLayer->setColor( markerColor );
  QgsMarkerSymbol *markerSymbol = new QgsMarkerSymbol( QgsSymbolLayerList() << markerSymbolLayer );

  QgsVectorTileBasicRendererStyle st( u"Polygons"_s, QString(), Qgis::GeometryType::Point );
  st.setSymbol( markerSymbol );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( u"background"_s, u"background"_s, Qgis::GeometryType::Polygon );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend ); // takes ownership

  mMapSettings->setExtent( layer->extent() );
  mMapSettings->setDestinationCrs( layer->crs() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"render_test_polygon_with_marker"_s, u"render_test_polygon_with_marker"_s, *mMapSettings, 0, 15 );
}

void TestQgsVectorTileLayer::test_styleMinZoomBeyondTileMaxZoom()
{
  // a polygon in a vector tile layer which is matched by a marker rule should result in a point-inside-polygon placement
  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = Qgis::DEFAULT_LINE_WIDTH * 2;

  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer->setColor( lineStrokeColor );
  lineSymbolLayer->setWidth( lineStrokeWidth );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );

  QgsVectorTileBasicRendererStyle st( u"Polygons"_s, QString(), Qgis::GeometryType::Line );
  st.setSymbol( lineSymbol );
  st.setMinZoomLevel( 2 );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( u"background"_s, u"background"_s, Qgis::GeometryType::Polygon );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend ); // takes ownership

  mMapSettings->setExtent( QgsRectangle( -1180017, 4261973, 155871, 5474783 ) );
  mMapSettings->setDestinationCrs( layer->crs() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"render_test_style_min_zoom"_s, u"render_test_style_min_zoom"_s, *mMapSettings, 0, 15 );
}

void TestQgsVectorTileLayer::test_filterRuleAllLayers()
{
  // test using a filter with field access for an "all layers" rule
  QgsDataSourceUri ds;
  ds.setParam( "type", "mbtiles" );
  ds.setParam( "url", QString( "/%1/mbtiles_vt.mbtiles" ).arg( mDataDir ) );
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  mMapSettings->setLayers( QList<QgsMapLayer *>() << layer.get() );

  const QColor lineStrokeColor = Qt::blue;
  const double lineStrokeWidth = Qgis::DEFAULT_LINE_WIDTH * 4;

  QgsSimpleLineSymbolLayer *lineSymbolLayer = new QgsSimpleLineSymbolLayer;
  lineSymbolLayer->setColor( lineStrokeColor );
  lineSymbolLayer->setWidth( lineStrokeWidth );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol( QgsSymbolLayerList() << lineSymbolLayer );

  QgsVectorTileBasicRendererStyle st( u"Lines"_s, QString(), Qgis::GeometryType::Line );
  st.setFilterExpression( u"\"Name\"='Highway'"_s );
  st.setSymbol( lineSymbol );

  QgsSimpleFillSymbolLayer *fillSymbolLayer = new QgsSimpleFillSymbolLayer;
  fillSymbolLayer->setColor( Qt::white );
  fillSymbolLayer->setStrokeStyle( Qt::NoPen );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol( QgsSymbolLayerList() << fillSymbolLayer );

  QgsVectorTileBasicRendererStyle bgst( u"background"_s, u"background"_s, Qgis::GeometryType::Polygon );
  bgst.setSymbol( fillSymbol );

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QList<QgsVectorTileBasicRendererStyle>() << bgst << st );
  layer->setRenderer( rend ); // takes ownership

  mMapSettings->setExtent( layer->extent() );
  mMapSettings->setDestinationCrs( layer->crs() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( u"render_test_filter_all_layers"_s, u"render_test_filter_all_layers"_s, *mMapSettings, 0, 15 );
}

void TestQgsVectorTileLayer::test_longIntAttributes()
{
  // test using a filter with field access for an "all layers" rule
  QgsDataSourceUri ds;
  ds.setParam( "type", "mbtiles" );
  ds.setParam( "url", QString( "/%1/longint.mbtiles" ).arg( mDataDir ) );
  auto layer = std::make_unique<QgsVectorTileLayer>( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( layer->isValid() );

  const QgsVectorTileRawData tile0 = layer->getRawTile( QgsTileXYZ( 0, 0, 0 ) );
  QgsVectorTileMVTDecoder decoder( QgsVectorTileMatrixSet::fromWebMercator() );
  const bool resDecode0 = decoder.decode( tile0 );
  QVERIFY( resDecode0 );
  const QStringList fieldNamesLines = decoder.layerFieldNames( "lines" );
  QCOMPARE( fieldNamesLines, QStringList() << "Name" << "Value" );

  QgsFields fields;
  fields.append( QgsField( "Value", QMetaType::Type::QString ) );
  QMap<QString, QgsFields> perLayerFields;
  perLayerFields["lines"] = fields;

  QgsVectorTileFeatures features = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
  QCOMPARE( features["lines"].count(), 1 );

  QgsAttributes attrs = features["lines"][0].attributes();
  QCOMPARE( attrs.count(), 1 );
  QCOMPARE( attrs[0].toString(), "103097205262536" );
}


QGSTEST_MAIN( TestQgsVectorTileLayer )
#include "testqgsvectortilelayer.moc"
