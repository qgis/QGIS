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
    }

    //runs after all tests
    void cleanupTestCase()
    {
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

  private:
    QgsWmsCapabilities *mCapabilities = nullptr;
};

QGSTEST_MAIN( TestQgsWmsProvider )
#include "testqgswmsprovider.moc"
