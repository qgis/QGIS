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

#include "qgsmimedatautils.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"

const QString TEST_ENCODED_DATA( "raster:wms:A Fancy WMS From Ciriè City:crs=EPSG\\:2036&dpiMode=7&format=image/png&layers=lidar&styles=default"
                                 "&url=https\\://geoegl.msp.gouv.qc.:EPSG\\\\:2036\\:EPSG\\\\:3857:image/tiff\\:image/jpeg:::PointZ:/home/me/my data.jpg" );

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
    void testLayerFromProject();

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
  uri.wkbType = QgsWkbTypes::PointZ;
  uri.filePath = QStringLiteral( "/home/me/my data.jpg" );

  QVERIFY( !uri.mapLayer() );

  QgsMimeDataUtils::UriList uriList;
  uriList << uri;

  QMimeData *mimeData =  QgsMimeDataUtils::encodeUriList( uriList );

  const QgsMimeDataUtils::Uri uriDecoded( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );

  QCOMPARE( uriDecoded.name, uri.name );
  QCOMPARE( uriDecoded.providerKey, uri.providerKey );
  QCOMPARE( uriDecoded.supportedFormats, uri.supportedFormats );
  QCOMPARE( uriDecoded.uri, uri.uri );
  QCOMPARE( uriDecoded.supportedCrs, uri.supportedCrs );
  QCOMPARE( uriDecoded.wkbType, QgsWkbTypes::PointZ );
  QCOMPARE( uriDecoded.filePath, QStringLiteral( "/home/me/my data.jpg" ) );

  QgsMimeDataUtils::decodeUriList( mimeData );

  // Encode representation:
  const QString data( uri.data() );

  QCOMPARE( data, TEST_ENCODED_DATA );

  QStringList fragments( QgsMimeDataUtils::decode( data ) );
  QCOMPARE( fragments.size(), 10 );

  QCOMPARE( fragments[0], QStringLiteral( "raster" ) );
  QCOMPARE( fragments[1], QStringLiteral( "wms" ) );
  QCOMPARE( fragments[2], QStringLiteral( "A Fancy WMS From Ciriè City" ) );
  QCOMPARE( fragments[3], QStringLiteral( "crs=EPSG:2036&dpiMode=7&format=image/png&layers=lidar&styles=default&url=https://geoegl.msp.gouv.qc." ) );
  QCOMPARE( fragments[4], QStringLiteral( "EPSG\\:2036:EPSG\\:3857" ) );
  QCOMPARE( fragments[5], QStringLiteral( "image/tiff:image/jpeg" ) );
  QCOMPARE( fragments[6], QString() );
  QCOMPARE( fragments[7], QString() );
  QCOMPARE( fragments[8], QStringLiteral( "PointZ" ) );
  QCOMPARE( fragments[9], QStringLiteral( "/home/me/my data.jpg" ) );
}

void TestQgsMimeDataUtils::testLayerFromProject()
{
  QgsVectorLayer *vl1 = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=id:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QVERIFY( vl1->isValid() );
  QgsProject::instance()->addMapLayer( vl1 );

  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=id:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QVERIFY( vl2->isValid() );
  QgsProject::instance()->addMapLayer( vl2 );

  QMimeData *mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << QgsMimeDataUtils::Uri( vl1 ) << QgsMimeDataUtils::Uri( vl2 ) );

  const QgsMimeDataUtils::Uri uriDecoded( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QCOMPARE( uriDecoded.mapLayer(), vl1 );
  QCOMPARE( uriDecoded.wkbType, QgsWkbTypes::LineString );
  bool owner = false;
  QString error;
  QCOMPARE( uriDecoded.vectorLayer( owner, error ), vl1 );
  QVERIFY( !owner );
  QVERIFY( error.isEmpty() );
  const QgsMimeDataUtils::Uri uriDecoded2( QgsMimeDataUtils::decodeUriList( mimeData ).at( 1 ) );
  QCOMPARE( uriDecoded2.mapLayer(), vl2 );
  QCOMPARE( uriDecoded2.vectorLayer( owner, error ), vl2 );
  QVERIFY( !owner );
  QVERIFY( error.isEmpty() );
  delete mimeData;

  // fake a uri with same layerId, but from a different application instance
  QgsMimeDataUtils::Uri uri( vl1 );
  uri.pId = QStringLiteral( "1" );
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri );
  const QgsMimeDataUtils::Uri uriDecoded3( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded3.mapLayer() );
  QVERIFY( !uriDecoded3.vectorLayer( owner, error ) );
  QVERIFY( !owner );
  QVERIFY( !error.isEmpty() );

  // bad layerId
  QgsMimeDataUtils::Uri uri2( vl1 );
  uri2.layerId = QStringLiteral( "xcxxcv" );
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri2 );
  const QgsMimeDataUtils::Uri uriDecoded4( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded4.mapLayer() );
  QVERIFY( !uriDecoded4.vectorLayer( owner, error ) );
  QVERIFY( !owner );
  QVERIFY( !error.isEmpty() );


  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  const QString pointsFileName = testDataDir + "points.shp";
  QgsVectorLayer *points = new QgsVectorLayer( pointsFileName, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QgsProject::instance()->addMapLayer( points );

  // bad layerId, but valid data source (i.e. not a memory layer)
  QgsMimeDataUtils::Uri uri3( points );
  uri3.layerId = QStringLiteral( "xcxxcv" );
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri3 );
  const QgsMimeDataUtils::Uri uriDecoded5( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded5.mapLayer() );
  QCOMPARE( uriDecoded5.wkbType, QgsWkbTypes::Point );
  QgsVectorLayer *res = uriDecoded5.vectorLayer( owner, error );
  QVERIFY( res );
  QVERIFY( res->isValid() );
  QVERIFY( owner );
  QVERIFY( error.isEmpty() );
  delete res;
}


QGSTEST_MAIN( TestQgsMimeDataUtils )
#include "testqgsmimedatautils.moc"


