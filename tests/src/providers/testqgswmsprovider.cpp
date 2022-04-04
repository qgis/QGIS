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

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS provider.
 */
class TestQgsWmsProvider: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
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

      mReport += QLatin1String( "<h1>WMS Provider Tests</h1>\n" );
    }

    //runs after all tests
    void cleanupTestCase()
    {
      QString myReportFile = QDir::tempPath() + "/qgistest.html";
      QFile myFile( myReportFile );
      if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
      {
        QTextStream myQTextStream( &myFile );
        myQTextStream << mReport;
        myFile.close();
      }

      delete mCapabilities;
      QgsApplication::exitQgis();
    }

    void legendGraphicsWithStyle()
    {
      QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/fb.png?" ) );
    }

    void legendGraphicsWithSecondStyle()
    {
      QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=yt_style&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/yt.png?" ) );
    }

    void legendGraphicsWithoutStyleWithDefault()
    {
      QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
      //only one style, can guess default => use it
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/buildings.png?" ) );
    }

    void legendGraphicsWithoutStyleWithoutDefault()
    {
      QgsWmsProvider provider( QStringLiteral( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg" ), QgsDataProvider::ProviderOptions(), mCapabilities );
      //two style, cannot guess default => use the WMS GetLegendGraphics
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://localhost:8380/mapserv?" ) );
    }

    // regression #20271 - WMS is not displayed in QGIS 3.4.0
    void queryItemsWithNullValue()
    {
      QString failingAddress( "http://localhost:8380/mapserv" );
      QgsWmsProvider provider( failingAddress, QgsDataProvider::ProviderOptions(), mCapabilities );
      QUrl url( provider.createRequestUrlWMS( QgsRectangle( 0, 0, 90, 90 ), 100, 100 ) );
      QCOMPARE( url.toString(), QString( "http://localhost:8380/mapserv?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap"
                                         "&BBOX=0,0,90,90&CRS=CRS:84&WIDTH=100&HEIGHT=100&LAYERS=&"
                                         "STYLES=&FORMAT=&TRANSPARENT=TRUE" ) );
    }

    // regression #41116
    void queryItemsWithPlusSign()
    {
      const QString failingAddress( "layers=plus+sign&styles=&url=http://localhost:8380/mapserv" );
      const QgsWmsParserSettings config;
      QgsWmsCapabilities cap;
      QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll().replace( "<Name>test</Name>",  "<Name>plus+sign</Name>" );
      QVERIFY( cap.parseResponse( content, config ) );
      QgsWmsProvider provider( failingAddress, QgsDataProvider::ProviderOptions(), &cap );
      QUrl url( provider.createRequestUrlWMS( QgsRectangle( 0, 0, 90, 90 ), 100, 100 ) );
      QCOMPARE( url.toString(), QString( "http://localhost:8380/mapserv?SERVICE=WMS&VERSION=1.3.0&REQUEST=GetMap&BBOX=0,0,90,90&"
                                         "CRS=EPSG:2056&WIDTH=100&HEIGHT=100&"
                                         "LAYERS=plus%2Bsign&STYLES=&FORMAT=&TRANSPARENT=TRUE" ) );
    }


    void noCrsSpecified()
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

    void testMBTiles()
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
    }

    void testDpiDependentData()
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

    void providerUriUpdates()
    {
      QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( "wms" );
      QString uriString = QStringLiteral( "crs=EPSG:4326&dpiMode=7&"
                                          "layers=testlayer&styles&"
                                          "url=http://localhost:8380/mapserv&"
                                          "testParam=true" );
      QVariantMap parts = metadata->decodeUri( uriString );
      QVariantMap expectedParts { { QString( "crs" ), QVariant( "EPSG:4326" ) },  { QString( "dpiMode" ), QVariant( "7" ) },
        { QString( "testParam" ), QVariant( "true" ) },  { QString( "layers" ), QVariant( "testlayer" ) },
        { QString( "styles" ), QString() },  { QString( "url" ), QVariant( "http://localhost:8380/mapserv" ) } };
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

    void providerUriLocalFile()
    {
      QString uriString = QStringLiteral( "url=file:///my/local/tiles.mbtiles&type=mbtiles" );
      QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "wms" ), uriString );
      QVariantMap expectedParts { { QString( "type" ), QVariant( "mbtiles" ) },
        { QString( "path" ), QVariant( "/my/local/tiles.mbtiles" ) } };
      QCOMPARE( parts, expectedParts );

      QString encodedUri = QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "wms" ), parts );
      QCOMPARE( encodedUri, uriString );
    }

    void testXyzIsBasemap()
    {
      // test that xyz tile layers are considered basemap layers
      QgsRasterLayer layer( QStringLiteral( "type=xyz&url=file://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0" ), QString(), QStringLiteral( "wms" ) );
      QCOMPARE( layer.properties(), Qgis::MapLayerProperties( Qgis::MapLayerProperty::IsBasemapLayer ) );
    }

    void testOsmMetadata()
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

    void testConvertToValue()
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

    void testTerrariumInterpretation()
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

    void testResampling()
    {
      QString dataDir( TEST_DATA_DIR );

      QgsXyzConnection xyzConn;
      xyzConn.url = QUrl::fromLocalFile( dataDir + QStringLiteral( "/maptiler_terrain_rgb.png" ) ).toString();
      xyzConn.interpretation = QStringLiteral( "maptilerterrain" );
      QgsRasterLayer layer( xyzConn.encodedUri(), "terrain", "wms" );
      QVERIFY( layer.isValid() );
      QVERIFY( layer.dataProvider()->dataType( 1 ) == Qgis::DataType::Float32 );

      QVERIFY( layer.dataProvider()->enableProviderResampling( true ) );
      QVERIFY( layer.dataProvider()->setZoomedInResamplingMethod( QgsRasterDataProvider::ResamplingMethod::Cubic ) );
      QVERIFY( layer.dataProvider()->setZoomedOutResamplingMethod( QgsRasterDataProvider::ResamplingMethod::Cubic ) );
      layer.setResamplingStage( Qgis::RasterResamplingStage::Provider );
      std::unique_ptr<QgsHillshadeRenderer> hillshade = std::make_unique<QgsHillshadeRenderer>( layer.dataProvider(), 1, 315, 45 );
      hillshade->setZFactor( 0.0005 );
      layer.setRenderer( hillshade.release() );

      QgsMapSettings mapSettings;
      mapSettings.setLayers( QList<QgsMapLayer *>() << &layer );
      QgsRectangle layerExtent = layer.extent();
      mapSettings.setExtent( QgsRectangle( layerExtent.xMinimum() + 1000,
                                           layerExtent.yMinimum() + 1000,
                                           layerExtent.xMinimum() + 1000 + layerExtent.width() / 3000000,
                                           layerExtent.yMinimum() + 1000 + layerExtent.height() / 3000000 ) );
      mapSettings.setOutputSize( QSize( 400, 400 ) );
      mapSettings.setOutputDpi( 96 );
      mapSettings.setDpiTarget( 48 );
      QVERIFY( imageCheck( "cubic_resampling", mapSettings ) );
    }

    bool imageCheck( const QString &testType, QgsMapSettings &mapSettings )
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

    void testParseWmstUriWithoutTemporalExtent()
    {
      // test fix for https://github.com/qgis/QGIS/issues/43158
      // we just check we don't crash
      QgsWmsProvider provider( QStringLiteral( "allowTemporalUpdates=true&temporalSource=provider&type=wmst&layers=foostyles=bar&crs=EPSG:3857&format=image/png&url=file:///dummy" ), QgsDataProvider::ProviderOptions(), mCapabilities );
    }

  private:
    QgsWmsCapabilities *mCapabilities = nullptr;

    QString mReport;
};

QGSTEST_MAIN( TestQgsWmsProvider )
#include "testqgswmsprovider.moc"
