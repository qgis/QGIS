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
#include <QtTest/QtTest>
#include <qgswmsprovider.h>
#include <qgsapplication.h>

/** \ingroup UnitTests
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

      QFile file( QString( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
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
      QgsWmsProvider provider( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=fb_style&format=image/jpg", mCapabilities );
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/fb.png?" ) );
    }

    void legendGraphicsWithSecondStyle()
    {
      QgsWmsProvider provider( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=yt_style&format=image/jpg", mCapabilities );
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/yt.png?" ) );
    }

    void legendGraphicsWithoutStyleWithDefault()
    {
      QgsWmsProvider provider( "http://localhost:8380/mapserv?xxx&layers=buildings&styles=&format=image/jpg", mCapabilities );
      //only one style, can guess default => use it
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://www.example.com/buildings.png?" ) );
    }

    void legendGraphicsWithoutStyleWithoutDefault()
    {
      QgsWmsProvider provider( "http://localhost:8380/mapserv?xxx&layers=agri_zones&styles=&format=image/jpg", mCapabilities );
      //two style, cannot guess default => use the WMS GetLegendGraphics
      QCOMPARE( provider.getLegendGraphicUrl(), QString( "http://localhost:8380/mapserv?" ) );
    }

  private:
    QgsWmsCapabilities* mCapabilities;
};

QTEST_MAIN( TestQgsWmsProvider )
#include "testqgswmsprovider.moc"
