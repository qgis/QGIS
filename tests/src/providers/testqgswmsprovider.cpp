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
#include "qgstest.h"
#include <qgswmsprovider.h>
#include <qgsapplication.h>
#include <qgsmultirenderchecker.h>
#include <qgsrasterlayer.h>
#include <qgsproviderregistry.h>

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

      QVERIFY( imageCheck( "mbtiles_1", &layer, layer.extent() ) );
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

    bool imageCheck( const QString &testType, QgsMapLayer *layer, const QgsRectangle &extent )
    {
      //use the QgsRenderChecker test utility class to
      //ensure the rendered output matches our control image
      QgsMapSettings mapSettings;
      mapSettings.setLayers( QList<QgsMapLayer *>() << layer );
      mapSettings.setExtent( extent );
      mapSettings.setOutputSize( QSize( 400, 400 ) );
      mapSettings.setOutputDpi( 96 );
      QgsMultiRenderChecker myChecker;
      myChecker.setControlPathPrefix( QStringLiteral( "wmsprovider" ) );
      myChecker.setControlName( "expected_" + testType );
      myChecker.setMapSettings( mapSettings );
      myChecker.setColorTolerance( 20 );
      bool myResultFlag = myChecker.runTest( testType, 500 );
      mReport += myChecker.report();
      return myResultFlag;
    }

  private:
    QgsWmsCapabilities *mCapabilities = nullptr;

    QString mReport;
};

QGSTEST_MAIN( TestQgsWmsProvider )
#include "testqgswmsprovider.moc"
