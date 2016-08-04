/***************************************************************************
    testqgswmscapabilities.cpp
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
#include <qgswmscapabilities.h>
#include <qgsapplication.h>

/** \ingroup UnitTests
 * This is a unit test for the WMS capabilities parser.
 */
class TestQgsWmsCapabilities: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      // init QGIS's paths - true means that all path will be inited from prefix
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    //runs after all tests
    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }


    void read()
    {
      QgsWmsCapabilities capabilities;

      QFile file( QString( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll();
      QVERIFY( content.size() > 0 );
      const QgsWmsParserSettings config;

      QVERIFY( capabilities.parseResponse( content, config ) );
      QCOMPARE( capabilities.supportedLayers().size(), 5 );
      QCOMPARE( capabilities.supportedLayers()[0].name, QString( "agri_zones" ) );
      QCOMPARE( capabilities.supportedLayers()[1].name, QString( "buildings" ) );
      QCOMPARE( capabilities.supportedLayers()[2].name, QString( "land_surveing_parcels" ) );
      QCOMPARE( capabilities.supportedLayers()[3].name, QString( "cadastre" ) );
      QCOMPARE( capabilities.supportedLayers()[4].name, QString( "test" ) );

      // make sure the default style is not seen twice in the child layers
      QCOMPARE( capabilities.supportedLayers()[3].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[3].style[0].name, QString( "default" ) );
      QCOMPARE( capabilities.supportedLayers()[1].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[1].style[0].name, QString( "default" ) );
      QCOMPARE( capabilities.supportedLayers()[2].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[2].style[0].name, QString( "default" ) );

      // check it can read 2 styles for a layer and that the legend URL is OK
      QCOMPARE( capabilities.supportedLayers()[0].style.size(), 2 );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].name, QString( "yt_style" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].legendUrl.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].legendUrl[0].onlineResource.xlinkHref,
                QString( "http://www.example.com/yt.png" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].name, QString( "fb_style" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].legendUrl.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].legendUrl[0].onlineResource.xlinkHref,
                QString( "http://www.example.com/fb.png" ) );
    }

};

QTEST_MAIN( TestQgsWmsCapabilities )
#include "testqgswmscapabilities.moc"
