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


#include "qgsmimedatautils.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>

const QString TEST_ENCODED_DATA( "raster:wms:A Fancy WMS From Ciriè City:crs=EPSG\\:2036&dpiMode=7&format=image/png&layers=lidar&styles=default"
                                 "&url=https\\://geoegl.msp.gouv.qc.:EPSG\\\\:2036\\:EPSG\\\\:3857:image/tiff\\:image/jpeg:::PointZ:/home/me/my data.jpg" );

class TestQgsMimeDataUtils : public QObject
{
    Q_OBJECT
  public:
    TestQgsMimeDataUtils() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

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
  uri.layerType = u"raster"_s;
  uri.name = u"A Fancy WMS From Ciriè City"_s;
  uri.providerKey = u"wms"_s;
  uri.supportedCrs << u"EPSG:2036"_s << u"EPSG:3857"_s;
  uri.supportedFormats << u"image/tiff"_s << u"image/jpeg"_s;
  uri.uri = u"crs=EPSG:2036&dpiMode=7&format=image/png&layers=lidar&styles=default&url=https://geoegl.msp.gouv.qc."_s;
  uri.wkbType = Qgis::WkbType::PointZ;
  uri.filePath = u"/home/me/my data.jpg"_s;

  QVERIFY( !uri.mapLayer() );

  QgsMimeDataUtils::UriList uriList;
  uriList << uri;

  QMimeData *mimeData = QgsMimeDataUtils::encodeUriList( uriList );

  const QgsMimeDataUtils::Uri uriDecoded( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );

  QCOMPARE( uriDecoded.name, uri.name );
  QCOMPARE( uriDecoded.providerKey, uri.providerKey );
  QCOMPARE( uriDecoded.supportedFormats, uri.supportedFormats );
  QCOMPARE( uriDecoded.uri, uri.uri );
  QCOMPARE( uriDecoded.supportedCrs, uri.supportedCrs );
  QCOMPARE( uriDecoded.wkbType, Qgis::WkbType::PointZ );
  QCOMPARE( uriDecoded.filePath, u"/home/me/my data.jpg"_s );

  QgsMimeDataUtils::decodeUriList( mimeData );

  // Encode representation:
  const QString data( uri.data() );

  QCOMPARE( data, TEST_ENCODED_DATA );

  QStringList fragments( QgsMimeDataUtils::decode( data ) );
  QCOMPARE( fragments.size(), 10 );

  QCOMPARE( fragments[0], u"raster"_s );
  QCOMPARE( fragments[1], u"wms"_s );
  QCOMPARE( fragments[2], u"A Fancy WMS From Ciriè City"_s );
  QCOMPARE( fragments[3], u"crs=EPSG:2036&dpiMode=7&format=image/png&layers=lidar&styles=default&url=https://geoegl.msp.gouv.qc."_s );
  QCOMPARE( fragments[4], u"EPSG\\:2036:EPSG\\:3857"_s );
  QCOMPARE( fragments[5], u"image/tiff:image/jpeg"_s );
  QCOMPARE( fragments[6], QString() );
  QCOMPARE( fragments[7], QString() );
  QCOMPARE( fragments[8], u"PointZ"_s );
  QCOMPARE( fragments[9], u"/home/me/my data.jpg"_s );
}

void TestQgsMimeDataUtils::testLayerFromProject()
{
  QgsVectorLayer *vl1 = new QgsVectorLayer( u"LineString?crs=epsg:3111&field=id:int"_s, u"vl1"_s, u"memory"_s );
  QVERIFY( vl1->isValid() );
  QgsProject::instance()->addMapLayer( vl1 );

  QgsVectorLayer *vl2 = new QgsVectorLayer( u"LineString?crs=epsg:3111&field=id:int"_s, u"vl1"_s, u"memory"_s );
  QVERIFY( vl2->isValid() );
  QgsProject::instance()->addMapLayer( vl2 );

  QMimeData *mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << QgsMimeDataUtils::Uri( vl1 ) << QgsMimeDataUtils::Uri( vl2 ) );

  const QgsMimeDataUtils::Uri uriDecoded( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QCOMPARE( uriDecoded.mapLayer(), vl1 );
  QCOMPARE( uriDecoded.wkbType, Qgis::WkbType::LineString );
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
  uri.pId = u"1"_s;
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri );
  const QgsMimeDataUtils::Uri uriDecoded3( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded3.mapLayer() );
  QVERIFY( !uriDecoded3.vectorLayer( owner, error ) );
  QVERIFY( !owner );
  QVERIFY( !error.isEmpty() );

  // bad layerId
  QgsMimeDataUtils::Uri uri2( vl1 );
  uri2.layerId = u"xcxxcv"_s;
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri2 );
  const QgsMimeDataUtils::Uri uriDecoded4( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded4.mapLayer() );
  QVERIFY( !uriDecoded4.vectorLayer( owner, error ) );
  QVERIFY( !owner );
  QVERIFY( !error.isEmpty() );


  const QString testDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
  const QString pointsFileName = testDataDir + "points.shp";
  QgsVectorLayer *points = new QgsVectorLayer( pointsFileName, u"points"_s, u"ogr"_s );
  QgsProject::instance()->addMapLayer( points );

  // bad layerId, but valid data source (i.e. not a memory layer)
  QgsMimeDataUtils::Uri uri3( points );
  uri3.layerId = u"xcxxcv"_s;
  mimeData = QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri3 );
  const QgsMimeDataUtils::Uri uriDecoded5( QgsMimeDataUtils::decodeUriList( mimeData ).at( 0 ) );
  QVERIFY( !uriDecoded5.mapLayer() );
  QCOMPARE( uriDecoded5.wkbType, Qgis::WkbType::Point );
  QgsVectorLayer *res = uriDecoded5.vectorLayer( owner, error );
  QVERIFY( res );
  QVERIFY( res->isValid() );
  QVERIFY( owner );
  QVERIFY( error.isEmpty() );
  delete res;
}


QGSTEST_MAIN( TestQgsMimeDataUtils )
#include "testqgsmimedatautils.moc"
