/***************************************************************************
    testqgsmimedatautils.cpp
     --------------------------------------
    Date                 : 18.06.2018
    Copyright            : (C) 2018 Alessandro Pasotti
    Email                : elpaso at itopen dot it
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

#include <qgsmimedatautils.h>

const QString TEST_ENCODED_DATA( "raster:wms:A Fancy WMS From Ciriè City:crs=EPSG\\:2036&dpiMode=7&format=image/png&layers=lidar&styles=default"
                                 "&url=https\\://geoegl.msp.gouv.qc.:EPSG\\\\:2036\\:EPSG\\\\:3857:image/tiff\\:image/jpeg" );

class TestQgsMimeDataUtils: public QObject
{
    Q_OBJECT
  public:
    TestQgsMimeDataUtils() = default;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testEncodeDecode();

};


void TestQgsMimeDataUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsMimeDataUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMimeDataUtils::init()
{
}


void TestQgsMimeDataUtils::cleanup()
{
}

void TestQgsMimeDataUtils::testEncodeDecode()
{

  QgsMimeDataUtils::Uri uri;
  uri.layerType = QStringLiteral( "raster" );
  uri.name = QStringLiteral( "A Fancy WMS From Ciriè City" );
  uri.providerKey = QStringLiteral( "wms" );
  uri.supportedCrs << QStringLiteral( "EPSG:2036" ) <<  QStringLiteral( "EPSG:3857" ) ;
  uri.supportedFormats << QStringLiteral( "image/tiff" ) << QStringLiteral( "image/jpeg" );
  uri.uri = QStringLiteral( "crs=EPSG:2036&dpiMode=7&format=image/png&layers=lidar&styles=default&url=https://geoegl.msp.gouv.qc." );

  QgsMimeDataUtils::UriList uriList;
  uriList << uri;

  QMimeData *mimeData =  QgsMimeDataUtils::encodeUriList( uriList );

  QgsMimeDataUtils::Uri uriDecoded( QgsMimeDataUtils::decodeUriList( mimeData )[0] );

  QCOMPARE( uriDecoded.name, uri.name );
  QCOMPARE( uriDecoded.providerKey, uri.providerKey );
  QCOMPARE( uriDecoded.supportedFormats, uri.supportedFormats );
  QCOMPARE( uriDecoded.uri, uri.uri );
  QCOMPARE( uriDecoded.supportedCrs, uri.supportedCrs );

  QgsMimeDataUtils::decodeUriList( mimeData );

  // Encode representation:
  QString data( uri.data() );

  QCOMPARE( data, TEST_ENCODED_DATA );

  QStringList fragments( QgsMimeDataUtils::decode( data ) );

  QCOMPARE( fragments[0], QString( "raster" ) );
  QCOMPARE( fragments[1], QString( "wms" ) );
  QCOMPARE( fragments[2], QString( "A Fancy WMS From Ciriè City" ) );
  QCOMPARE( fragments[3], QString( "crs=EPSG:2036&dpiMode=7&format=image/png&layers=lidar&styles=default&url=https://geoegl.msp.gouv.qc." ) );
  QCOMPARE( fragments[4], QString( "EPSG\\:2036:EPSG\\:3857" ) );

}


QGSTEST_MAIN( TestQgsMimeDataUtils )
#include "testqgsmimedatautils.moc"


