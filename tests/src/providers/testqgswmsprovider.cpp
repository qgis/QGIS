/***************************************************************************
    testqgswmsprovider.cpp
    ---------------------
    begin                : May 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QFile>
#include <QObject>
#include <QUrlQuery>

#include "qgstest.h"
#include <qgswmsprovider.h>
#include <qgsapplication.h>
#include <qgsmultirenderchecker.h>
#include <qgsrasterlayer.h>
#include <qgsproviderregistry.h>
#include <qgsxyzconnection.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgsrastershader.h>
#include <qgsstyle.h>
#include "qgssinglebandgrayrenderer.h"
#include "qgsrasterlayer.h"
#include "qgshillshaderenderer.h"
#include "qgsproviderutils.h"
#include "qgsprovidersublayerdetails.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS provider.
 */
class TestQgsWmsProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsWmsProvider()
      : QgsTest( QStringLiteral( "WMS Provider Tests" ) ) {}

  private slots:

    void initTestCase();

    //runs after all tests
    void cleanupTestCase();

    void legendGraphicsWithStyle();

    void legendGraphicsWithSecondStyle();

    void legendGraphicsWithoutStyleWithDefault();

    void legendGraphicsWithoutStyleWithoutDefault();

    // regression #20271 - WMS is not displayed in QGIS 3.4.0
    void queryItemsWithNullValue();

    // regression #41116
    void queryItemsWithPlusSign();

    void noCrsSpecified();

    void testWmtsConstruction();

    void testMBTiles();

    void testMBTilesSample();

    void testMbtilesProviderMetadata();

    void testDpiDependentData();

    void providerUriUpdates();

    void providerUriLocalFile();

    void absoluteRelativeUri();

    void testXyzIsBasemap();

    void testOsmMetadata();

    void testConvertToValue();

    void testTerrariumInterpretation();

    void testResampling();

    bool imageCheck( const QString &testType, QgsMapSettings &mapSettings );

    void testParseWmstUriWithoutTemporalExtent();

    void testMaxTileSize();

  private:
    QgsWmsCapabilities *mCapabilities = nullptr;
};


void TestQgsWmsProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QByteArray content = file.readAll();
  QVERIFY( content.size() > 0 );
  const QgsWmsParserSettings config;

  mCapabilities = new QgsWmsCapabilities();
  QVERIFY( mCapabilities->parseResponse( content, config ) );
}

void TestQgsWmsProvider::cleanupTestCase()
{
  delete mCapabilities;
  QgsApplication::exitQgis();
}

void TestQgsWmsProvider::legendGraphicsWithStyle()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/fb.png?" ) );
}

void TestQgsWmsProvider::legendGraphicsWithSecondStyle()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=yt_style&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/yt.png?" ) );
}

void TestQgsWmsProvider::legendGraphicsWithoutStyleWithDefault()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  //only one style, can guess default => use it
  QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/buildings.png?" ) );
}

void TestQgsWmsProvider::legendGraphicsWithoutStyleWithoutDefault()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  //two style, cannot guess default => use the WMS GetLegendGraphics
  QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://localhost:8380/mapserv?" ) );
}

void TestQgsWmsProvider::queryItemsWithNullValue()
{
  QString failingAddress( "http://localhost:8380/mapserv" );
  QgsWmsProvider provider( failingAddress, QgsDataProvider::ProviderOptions(), mCapabilities );
  QUrl url( provider.createRequestUrlWMS( QgsRectangle( 0, 0, 90, 90 ), 100, 100 ) );
  QCOMPARE( url.toString(), QString( "http://localhost:8380/mapserv?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap"
                                     "&BBOX=0%2C0%2C90%2C90&CRS=CRS%3A84&WIDTH=100&HEIGHT=100&LAYERS=&"
                                     "STYLES=&FORMAT=&TRANSPARENT=TRUE" ) );
}

void TestQgsWmsProvider::queryItemsWithPlusSign()
{
  const QString failingAddress( "layers=plus+sign&styles=&url=http://localhost:8380/mapserv" );
  const QgsWmsParserSettings config;
  QgsWmsCapabilities cap;
  QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QByteArray content = file.readAll().replace( "<Name>test</Name>", "<Name>plus+sign</Name>" );
  QVERIFY( cap.parseResponse( content, config ) );
  QgsWmsProvider provider( failingAddress, QgsDataProvider::ProviderOptions(), &cap );
  QUrl url( provider.createRequestUrlWMS( QgsRectangle( 0, 0, 90, 90 ), 100, 100 ) );
  QCOMPARE( url.toString(), QString( "http://localhost:8380/mapserv?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&BBOX=0%2C0%2C90%2C90&"
                                     "CRS=EPSG%3A2056&WIDTH=100&HEIGHT=100&"
                                     "LAYERS=plus%2Bsign&STYLES=&FORMAT=&TRANSPARENT=TRUE" ) );
}

void TestQgsWmsProvider::noCrsSpecified()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  QCOMPARE( provider.crs().authid(), QStringLiteral( "EPSG:2056" ) );
  QgsWmsProvider provider2( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg&crs=EPSG:4326" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  QCOMPARE( provider2.crs().authid(), QStringLiteral( "EPSG:4326" ) );

  QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities2.xml" );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QByteArray content = file.readAll();
  QVERIFY( content.size() > 0 );
  const QgsWmsParserSettings config;

  QgsWmsCapabilities capabilities;
  QVERIFY( capabilities.parseResponse( content, config ) );
  QgsWmsProvider provider3( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg&crs=EPSG:4326" ), QgsDataProvider::ProviderOptions(), &capabilities );
  QCOMPARE( provider3.crs().authid(), QStringLiteral( "EPSG:4326" ) );
  QgsWmsProvider provider4( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), &capabilities );
  QCOMPARE( provider4.crs().authid(), QStringLiteral( "EPSG:3857" ) );
}

void TestQgsWmsProvider::testWmtsConstruction()
{
  const QgsWmsParserSettings config;
  QgsWmsCapabilities cap;
  QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilitiesWmts.xml" );
  QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
  const QByteArray content = file.readAll();
  QVERIFY( cap.parseResponse( content, config ) );

  // explicitly state crs and format
  {
    QgsWmsProvider provider( "contextualWMSLegend=0&crs=EPSG:4326&dpiMode=7&featureCount=10&format=image/jpg&layers=CountryGroup&styles=default&tileMatrixSet=standard&tilePixelRatio=0&url=http://localhost:8380/mapserv?xxx", QgsDataProvider::ProviderOptions(), &cap );
    QVERIFY( provider.mSettings.mTiled );
    QCOMPARE( provider.mSettings.mTileMatrixSetId, QStringLiteral( "standard" ) );
    QCOMPARE( provider.crs().authid(), QStringLiteral( "EPSG:4326" ) );
    QCOMPARE( provider.mSettings.mImageMimeType, QStringLiteral( "image/jpg" ) );
  }
  // no crs or format specified, should use tile matrix crs
  {
    QgsWmsProvider provider( "contextualWMSLegend=0&dpiMode=7&featureCount=10&layers=CountryGroup&styles=default&tileMatrixSet=standard&tilePixelRatio=0&url=http://localhost:8380/mapserv?xxx", QgsDataProvider::ProviderOptions(), &cap );
    QVERIFY( provider.mSettings.mTiled );
    QCOMPARE( provider.mSettings.mTileMatrixSetId, QStringLiteral( "standard" ) );
    QCOMPARE( provider.crs().authid(), QStringLiteral( "EPSG:3857" ) );
    QCOMPARE( provider.mSettings.mImageMimeType, QStringLiteral( "image/png" ) );
  }
  // no tileMatrixSet specified, should use first listed
  {
    QgsWmsProvider provider( "layers=CountryGroup&styles=default&tileDimensions=&url=http://localhost:8380/mapserv?xxx", QgsDataProvider::ProviderOptions(), &cap );
    QVERIFY( provider.mSettings.mTiled );
    QCOMPARE( provider.mSettings.mTileMatrixSetId, QStringLiteral( "standard" ) );
    QCOMPARE( provider.crs().authid(), QStringLiteral( "EPSG:3857" ) );
    QCOMPARE( provider.mSettings.mImageMimeType, QStringLiteral( "image/png" ) );
  }
  // no tileMatrixSet specified, using type=wmts to request wmts
  {
    QgsWmsProvider provider( "layers=CountryGroup&styles=default&type=wmts&url=http://localhost:8380/mapserv?xxx", QgsDataProvider::ProviderOptions(), &cap );
    QVERIFY( provider.mSettings.mTiled );
    QCOMPARE( provider.mSettings.mTileMatrixSetId, QStringLiteral( "standard" ) );
    QCOMPARE( provider.crs().authid(), QStringLiteral( "EPSG:3857" ) );
    QCOMPARE( provider.mSettings.mImageMimeType, QStringLiteral( "image/png" ) );
  }
}

void TestQgsWmsProvider::testMBTiles()
{
  QString dataDir( TEST_DATA_DIR );
  QUrlQuery uq;
  uq.addQueryItem( "type", "mbtiles" );
  uq.addQueryItem( "url", QUrl::fromLocalFile( dataDir + "/isle_of_man_xxx_invalid.mbtiles" ).toString() );

  // check first that we do not accept invalid mbtiles paths
  QgsRasterLayer layerInvalid( uq.toString(), "invalid", "wms" );
  //QgsWmsProvider providerInvalid( uq.toString(), QgsDataProvider::ProviderOptions() );
  QVERIFY( !layerInvalid.isValid() );

  uq.addQueryItem( "url", QUrl::fromLocalFile( dataDir + "/isle_of_man.mbtiles" ).toString() );
  QgsRasterLayer layer( uq.toString(), "isle_of_man", "wms" );
  QVERIFY( layer.isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << &layer );
  mapSettings.setExtent( layer.extent() );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  QVERIFY( imageCheck( "mbtiles_1", mapSettings ) );

  // since no terrain interpretation set, we don't know for sure that the layer contains elevation
  QVERIFY( !layer.dataProvider()->elevationProperties()->containsElevationData() );
}

void TestQgsWmsProvider::testMBTilesSample()
{
  QString dataDir( TEST_DATA_DIR );
  QUrlQuery uq;
  uq.addQueryItem( "type", "mbtiles" );
  uq.addQueryItem( "interpretation", "maptilerterrain" );
  uq.addQueryItem( "url", QUrl::fromLocalFile( dataDir + "/isle_of_man.mbtiles" ).toString() );

  QgsRasterLayer layer( uq.toString(), "isle_of_man", "wms" );
  QVERIFY( layer.isValid() );

  bool ok = false;
  const double value = layer.dataProvider()->sample( QgsPointXY( -496419, 7213350 ), 1, &ok );
  QVERIFY( ok );
  QCOMPARE( value, 1167617.375 );

  // ensure that terrain interpretation correctly indicates that the layer contains elevation
  QVERIFY( layer.dataProvider()->elevationProperties()->containsElevationData() );
}

void TestQgsWmsProvider::testMbtilesProviderMetadata()
{
  QgsProviderMetadata *wmsMetadata = QgsProviderRegistry::instance()->providerMetadata( "wms" );
  QVERIFY( wmsMetadata );

  // not mbtile uris
  QCOMPARE( wmsMetadata->priorityForUri( QString() ), 0 );
  QCOMPARE( wmsMetadata->validLayerTypesForUri( QString() ), {} );
  QVERIFY( wmsMetadata->querySublayers( QString() ).isEmpty() );
  QVERIFY( wmsMetadata->querySublayers( QStringLiteral( "xxx" ) ).isEmpty() );

  QCOMPARE( wmsMetadata->priorityForUri( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points.shp" ) ), 0 );
  QVERIFY( wmsMetadata->validLayerTypesForUri( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points.shp" ) ).isEmpty() );
  QVERIFY( wmsMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/points.shp" ) ).isEmpty() );
  QVERIFY( wmsMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) ).isEmpty() );

  QCOMPARE( wmsMetadata->priorityForUri( QStringLiteral( "type=mbtiles&url=%1/points.shp" ).arg( TEST_DATA_DIR ) ), 0 );
  QVERIFY( wmsMetadata->validLayerTypesForUri( QStringLiteral( "type=mbtiles&url=%1/points.shp" ).arg( TEST_DATA_DIR ) ).isEmpty() );
  QVERIFY( wmsMetadata->querySublayers( QStringLiteral( "type=mbtiles&url=%1/points.shp" ).arg( TEST_DATA_DIR ) ).isEmpty() );

  // mbtile uris
  QCOMPARE( wmsMetadata->priorityForUri( QStringLiteral( "%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) ), 100 );
  QCOMPARE( wmsMetadata->validLayerTypesForUri( QStringLiteral( "%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) ), { Qgis::LayerType::Raster } );

  QCOMPARE( wmsMetadata->priorityForUri( QStringLiteral( "type=mbtiles&url=%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) ), 100 );
  QCOMPARE( wmsMetadata->validLayerTypesForUri( QStringLiteral( "type=mbtiles&url=%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) ), { Qgis::LayerType::Raster } );

  // query sublayers
  QList<QgsProviderSublayerDetails> sublayers = wmsMetadata->querySublayers( QStringLiteral( "%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QStringLiteral( "isle_of_man" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=file://%1/isle_of_man.mbtiles&type=mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );
  QVERIFY( !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers ) );

  sublayers = wmsMetadata->querySublayers( QStringLiteral( "type=mbtiles&url=file://%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QStringLiteral( "isle_of_man" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=file://%1/isle_of_man.mbtiles&type=mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );

  // WMS source
  sublayers = wmsMetadata->querySublayers( QStringLiteral( "url=http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );

  // fast scan flag
  sublayers = wmsMetadata->querySublayers( QStringLiteral( "%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QStringLiteral( "isle_of_man" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=file://%1/isle_of_man.mbtiles&type=mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );
  QVERIFY( QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers ) );

  sublayers = wmsMetadata->querySublayers( QStringLiteral( "type=mbtiles&url=%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QStringLiteral( "isle_of_man" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=file://%1/isle_of_man.mbtiles&type=mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );

  // WMS source
  sublayers = wmsMetadata->querySublayers( QStringLiteral( "url=http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );

  // fast scan mode means that any mbtile file will be reported, including those with only vector tiles
  // (we are skipping a potentially expensive db open and format check)
  sublayers = wmsMetadata->querySublayers( QStringLiteral( "%1/vector_tile/mbtiles_vt.mbtiles" ).arg( TEST_DATA_DIR ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QStringLiteral( "mbtiles_vt" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "url=file://%1/vector_tile/mbtiles_vt.mbtiles&type=mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( sublayers.at( 0 ).skippedContainerScan() );

  // test that wms provider is the preferred provider for raster mbtiles files
  QList<QgsProviderRegistry::ProviderCandidateDetails> candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "type=mbtiles&url=%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) );
  QCOMPARE( candidates.size(), 2 );

  int candidateIndex = candidates.at( 0 ).metadata()->key() == QLatin1String( "wms" ) ? 0 : 1;
  QCOMPARE( candidates.at( candidateIndex ).metadata()->key(), QStringLiteral( "wms" ) );
  QCOMPARE( candidates.at( candidateIndex ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::Raster );

  candidates = QgsProviderRegistry::instance()->preferredProvidersForUri( QStringLiteral( "%1/isle_of_man.mbtiles" ).arg( TEST_DATA_DIR ) );
  // mbtiles vector tile provider also reports handling this url
  QCOMPARE( candidates.size(), 2 );
  candidateIndex = candidates.at( 0 ).metadata()->key() == QLatin1String( "wms" ) ? 0 : 1;
  QCOMPARE( candidates.at( candidateIndex ).metadata()->key(), QStringLiteral( "wms" ) );
  QCOMPARE( candidates.at( candidateIndex ).layerTypes(), QList<Qgis::LayerType>() << Qgis::LayerType::Raster );
}

void TestQgsWmsProvider::testDpiDependentData()
{
  QString dataDir( TEST_DATA_DIR );
  QUrlQuery uq;
  uq.addQueryItem( "type", "mbtiles" );
  uq.addQueryItem( "url", QUrl::fromLocalFile( dataDir + "/isle_of_man_xxx_invalid.mbtiles" ).toString() );

  // check first that we do not accept invalid mbtiles paths
  QgsRasterLayer layerInvalid( uq.toString(), "invalid", "wms" );
  //QgsWmsProvider providerInvalid( uq.toString(), QgsDataProvider::ProviderOptions() );
  QVERIFY( !layerInvalid.isValid() );

  uq.addQueryItem( "url", QUrl::fromLocalFile( dataDir + "/isle_of_man.mbtiles" ).toString() );
  QgsRasterLayer layer( uq.toString(), "isle_of_man", "wms" );
  QVERIFY( layer.isValid() );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << &layer );
  mapSettings.setExtent( layer.extent() );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDpiTarget( 48 );
  QVERIFY( imageCheck( "mbtiles_dpidependentdata", mapSettings ) );
}

void TestQgsWmsProvider::providerUriUpdates()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( "wms" );
  QString uriString = QStringLiteral( "crs=EPSG:4326&dpiMode=7&"
                                      "layers=testlayer&styles&"
                                      "url=http://localhost:8380/mapserv&"
                                      "testParam=true" );
  QVariantMap parts = metadata->decodeUri( uriString );
  QVariantMap expectedParts { { QString( "crs" ), QVariant( "EPSG:4326" ) }, { QString( "dpiMode" ), QVariant( "7" ) }, { QString( "testParam" ), QVariant( "true" ) }, { QString( "layers" ), QVariant( "testlayer" ) }, { QString( "styles" ), QString() }, { QString( "url" ), QVariant( "http://localhost:8380/mapserv" ) } };
  QCOMPARE( parts, expectedParts );

  parts["testParam"] = QVariant( "false" );

  QCOMPARE( parts["testParam"], QVariant( "false" ) );

  QString updatedUri = metadata->encodeUri( parts );
  QString expectedUri = QStringLiteral( "crs=EPSG:4326&dpiMode=7&"
                                        "layers=testlayer&styles&"
                                        "testParam=false&"
                                        "url=http://localhost:8380/mapserv" );
  QCOMPARE( updatedUri, expectedUri );
}

void TestQgsWmsProvider::providerUriLocalFile()
{
  QString uriString = QStringLiteral( "url=file:///my/local/tiles.mbtiles&type=mbtiles" );
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "wms" ), uriString );
  QVariantMap expectedParts { { QString( "type" ), QVariant( "mbtiles" ) }, { QString( "path" ), QVariant( "/my/local/tiles.mbtiles" ) }, { QString( "url" ), QVariant( "file:///my/local/tiles.mbtiles" ) } };
  QCOMPARE( parts, expectedParts );

  QString encodedUri = QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "wms" ), parts );
  QCOMPARE( encodedUri, uriString );

  QgsProviderMetadata *wmsMetadata = QgsProviderRegistry::instance()->providerMetadata( "wms" );
  QVERIFY( wmsMetadata );

  // query sublayers
  QList<QgsProviderSublayerDetails> sublayers;
  sublayers = wmsMetadata->querySublayers( QStringLiteral( "type=xyz&url=file:///my/xyz/directory/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0" ) );
  QCOMPARE( sublayers.size(), 1 );
  QCOMPARE( sublayers.at( 0 ).providerKey(), QStringLiteral( "wms" ) );
  QCOMPARE( sublayers.at( 0 ).name(), QLatin1String( "" ) );
  QCOMPARE( sublayers.at( 0 ).uri(), QStringLiteral( "type=xyz&url=file:///my/xyz/directory/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0" ) );
  QCOMPARE( sublayers.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !sublayers.at( 0 ).skippedContainerScan() );
  QVERIFY( !QgsProviderUtils::sublayerDetailsAreIncomplete( sublayers ) );
}

void TestQgsWmsProvider::absoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QgsProviderMetadata *wmsMetadata = QgsProviderRegistry::instance()->providerMetadata( "wms" );
  QVERIFY( wmsMetadata );

  QString absoluteUri = "type=mbtiles&url=file://" + QStringLiteral( TEST_DATA_DIR ) + "/isle_of_man.mbtiles";
  QString relativeUri = "type=mbtiles&url=file:./isle_of_man.mbtiles";
  QCOMPARE( wmsMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( wmsMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsWmsProvider::testXyzIsBasemap()
{
  // test that xyz tile layers are considered basemap layers
  QgsRasterLayer layer( QStringLiteral( "type=xyz&url=file://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0" ), QString(), QStringLiteral( "wms" ) );
  QCOMPARE( layer.properties(), Qgis::MapLayerProperties( Qgis::MapLayerProperty::IsBasemapLayer ) );
}

void TestQgsWmsProvider::testOsmMetadata()
{
  // test that we auto-populate openstreetmap tile metadata

  // don't actually hit the osm server -- the url below uses "file" instead of "http"!
  QgsWmsProvider provider( QStringLiteral( "type=xyz&url=file://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  QCOMPARE( provider.layerMetadata().identifier(), QStringLiteral( "OpenStreetMap tiles" ) );
  QCOMPARE( provider.layerMetadata().title(), QStringLiteral( "OpenStreetMap tiles" ) );
  QVERIFY( !provider.layerMetadata().abstract().isEmpty() );
  QCOMPARE( provider.layerMetadata().licenses().at( 0 ), QStringLiteral( "Open Data Commons Open Database License (ODbL)" ) );
  QCOMPARE( provider.layerMetadata().licenses().at( 1 ), QStringLiteral( "Creative Commons Attribution-ShareAlike (CC-BY-SA)" ) );
  QVERIFY( provider.layerMetadata().rights().at( 0 ).startsWith( "Base map and data from OpenStreetMap and OpenStreetMap Foundation" ) );
}

void TestQgsWmsProvider::testConvertToValue()
{
  QString dataDir( TEST_DATA_DIR );

  QgsXyzConnection xyzConn;
  xyzConn.url = QUrl::fromLocalFile( dataDir + QStringLiteral( "/maptiler_terrain_rgb.png" ) ).toString();
  QgsRasterLayer layer( xyzConn.encodedUri(), "terrain", "wms" );
  QVERIFY( layer.isValid() );
  QVERIFY( layer.dataProvider()->dataType( 1 ) == Qgis::DataType::ARGB32 );

  xyzConn.interpretation = QStringLiteral( "maptilerterrain" );
  QgsRasterLayer layer2( xyzConn.encodedUri(), "terrain", "wms" );
  QVERIFY( layer2.isValid() );
  QVERIFY( layer2.dataProvider()->dataType( 1 ) == Qgis::DataType::Float32 );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << &layer2 );
  mapSettings.setExtent( layer2.extent() );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDpiTarget( 48 );
  QVERIFY( imageCheck( "convert_value", mapSettings ) );
}

void TestQgsWmsProvider::testTerrariumInterpretation()
{
  QString dataDir( TEST_DATA_DIR );

  QgsXyzConnection xyzConn;
  xyzConn.interpretation = QgsWmsInterpretationConverterTerrariumRGB::interpretationKey();
  xyzConn.url = QUrl::fromLocalFile( dataDir + QStringLiteral( "/terrarium_terrain_rgb.png" ) ).toString();
  QgsRasterLayer layer( xyzConn.encodedUri(), "terrain", "wms" );
  QVERIFY( layer.isValid() );
  QVERIFY( layer.dataProvider()->dataType( 1 ) == Qgis::DataType::Float32 );

  QgsSingleBandGrayRenderer *renderer = new QgsSingleBandGrayRenderer( layer.dataProvider(), 1 );
  QgsContrastEnhancement *e = new QgsContrastEnhancement( Qgis::DataType::Float32 );
  e->setMinimumValue( -50 );
  e->setMaximumValue( 50 );
  e->setContrastEnhancementAlgorithm( QgsContrastEnhancement::StretchToMinimumMaximum );
  renderer->setContrastEnhancement( e );
  layer.setRenderer( renderer );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << &layer );
  mapSettings.setExtent( layer.extent() );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDpiTarget( 48 );
  QVERIFY( imageCheck( "terrarium_terrain", mapSettings ) );
}

void TestQgsWmsProvider::testResampling()
{
  QString dataDir( TEST_DATA_DIR );

  QgsXyzConnection xyzConn;
  xyzConn.url = QUrl::fromLocalFile( dataDir + QStringLiteral( "/maptiler_terrain_rgb.png" ) ).toString();
  xyzConn.interpretation = QStringLiteral( "maptilerterrain" );
  QgsRasterLayer layer( xyzConn.encodedUri(), "terrain", "wms" );
  QVERIFY( layer.isValid() );
  QVERIFY( layer.dataProvider()->dataType( 1 ) == Qgis::DataType::Float32 );

  QVERIFY( layer.dataProvider()->enableProviderResampling( true ) );
  QVERIFY( layer.dataProvider()->setZoomedInResamplingMethod( Qgis::RasterResamplingMethod::Cubic ) );
  QVERIFY( layer.dataProvider()->setZoomedOutResamplingMethod( Qgis::RasterResamplingMethod::Cubic ) );
  layer.setResamplingStage( Qgis::RasterResamplingStage::Provider );
  std::unique_ptr<QgsHillshadeRenderer> hillshade = std::make_unique<QgsHillshadeRenderer>( layer.dataProvider(), 1, 315, 45 );
  hillshade->setZFactor( 0.0005 );
  layer.setRenderer( hillshade.release() );

  QgsMapSettings mapSettings;
  mapSettings.setLayers( QList<QgsMapLayer *>() << &layer );
  QgsRectangle layerExtent = layer.extent();
  mapSettings.setExtent( QgsRectangle( layerExtent.xMinimum() + 1000, layerExtent.yMinimum() + 1000, layerExtent.xMinimum() + 1000 + layerExtent.width() / 3000000, layerExtent.yMinimum() + 1000 + layerExtent.height() / 3000000 ) );
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDpiTarget( 48 );
  QVERIFY( imageCheck( "cubic_resampling", mapSettings ) );
}

bool TestQgsWmsProvider::imageCheck( const QString &testType, QgsMapSettings &mapSettings )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "wmsprovider" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsWmsProvider::testParseWmstUriWithoutTemporalExtent()
{
  // test fix for https://github.com/qgis/QGIS/issues/43158
  // we just check we don't crash
  QgsWmsProvider provider( QStringLiteral( "allowTemporalUpdates=true&temporalSource=provider&type=wmst&layers=foostyles=bar&crs=EPSG:3857&format=image/png&url=file:///dummy" ), QgsDataProvider::ProviderOptions(), mCapabilities );
}

void TestQgsWmsProvider::testMaxTileSize()
{
  QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  const QSize maxTileSize = provider.maximumTileSize();
  QCOMPARE( maxTileSize.width(), 5000 );
  QCOMPARE( maxTileSize.height(), 5000 );

  // test that we can override the max tile size if the server advertises a larger size
  QgsWmsProvider provider2( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg&maxHeight=3000&maxWidth=3000" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  const QSize maxTileSize2 = provider2.maximumTileSize();
  QCOMPARE( maxTileSize2.width(), 3000 );
  QCOMPARE( maxTileSize2.height(), 3000 );

  // test that we cannot override the maximum advertised size
  QgsWmsProvider provider3( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg&maxHeight=6000&maxWidth=6000" ), QgsDataProvider::ProviderOptions(), mCapabilities );
  const QSize maxTileSize3 = provider3.maximumTileSize();
  QCOMPARE( maxTileSize3.width(), 5000 );
  QCOMPARE( maxTileSize3.height(), 5000 );

  // Remove the max tile size from the capabilities to check that the default value is used
  QgsWmsCapabilities capabilities { *mCapabilities };
  capabilities.mCapabilities.service.maxHeight = 0;
  capabilities.mCapabilities.service.maxWidth = 0;
  QgsWmsProvider provider4( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), &capabilities );
  const QSize maxTileSize4 = provider4.maximumTileSize();
  QCOMPARE( maxTileSize4.width(), QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH );
  QCOMPARE( maxTileSize4.height(), QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT );

  // test that we can override the default maximum tile size
  QgsWmsProvider provider5( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg&maxHeight=3000&maxWidth=3000" ), QgsDataProvider::ProviderOptions(), &capabilities );
  const QSize maxTileSize5 = provider5.maximumTileSize();
  QCOMPARE( maxTileSize5.width(), 3000 );
  QCOMPARE( maxTileSize5.height(), 3000 );
}

QGSTEST_MAIN( TestQgsWmsProvider )
#include "testqgswmsprovider.moc"
