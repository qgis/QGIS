/***************************************************************************
                         testqgsstac.cpp
                         -------------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstaccollectionlist.h"
#include "qgsstaccontroller.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"
#include "qgstest.h"

#include <QObject>
#include <QSignalSpy>
#include <QString>

/**
 * \ingroup UnitTests
 * This is a unit test for STAC
 */
class TestQgsStac : public QObject
{
    Q_OBJECT

  public:
    TestQgsStac() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testParseLocalCatalog();
    void testParseLocalCollection();
    void testParseLocalItem();
    void testParseLocalItemCollection();
    void testParseLocalCollections();

    void testFetchStacObjectAsync();
    void testFetchItemCollectionAsync();
    void testFetchCollectionsAsync();
    void testFetchStacObjectAsyncInvalid();
    void testFetchItemCollectionAsyncInvalid();
    void testFetchCollectionsAsyncInvalid();
    void testFetchStacObjectAsyncUnavailable();
    void testFetchItemCollectionAsyncUnavailable();
    void testFetchCollectionsAsyncUnavailable();

  private:
    QString mDataDir;
};

void TestQgsStac::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = u"%1/stac/"_s.arg( TEST_DATA_DIR );
}

void TestQgsStac::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsStac::testParseLocalCatalog()
{
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"catalog.json"_s ) );
  QgsStacController c;
  std::unique_ptr< QgsStacCatalog > cat = c.fetchStacObject< QgsStacCatalog >( url.toString() );
  QVERIFY( cat );
  QCOMPARE( cat->type(), Qgis::StacObjectType::Catalog );

  QVERIFY( cat );
  QCOMPARE( cat->id(), "examples"_L1 );
  QCOMPARE( cat->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( cat->title(), "Example Catalog"_L1 );
  QCOMPARE( cat->description(), "This catalog is a simple demonstration of an example catalog that is used to organize a hierarchy of collections and their items."_L1 );
  QVERIFY( cat->stacExtensions().isEmpty() );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = cat->links();
  QCOMPARE( links.size(), 6 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), u"%1catalog.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), u"%1extensions-collection/collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), u"%1collection-only/collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), u"%1collection-only/collection-with-schemas.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 4 ).href(), u"%1collectionless-item.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 5 ).href(), u"https://raw.githubusercontent.com/radiantearth/stac-spec/v1.0.0/examples/catalog.json"_s );
}

void TestQgsStac::testParseLocalCollection()
{
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"collection.json"_s ) );
  QgsStacController c;
  std::unique_ptr< QgsStacCollection > col = c.fetchStacObject< QgsStacCollection >( url.toString() );
  QVERIFY( col );
  QCOMPARE( col->type(), Qgis::StacObjectType::Collection );

  QVERIFY( col );
  QCOMPARE( col->id(), "simple-collection"_L1 );
  QCOMPARE( col->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( col->title(), "Simple Example Collection"_L1 );
  QCOMPARE( col->description(), "A simple collection demonstrating core catalog fields with links to a couple of items"_L1 );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = col->links();
  QCOMPARE( links.size(), 5 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), u"%1collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), u"%1simple-item.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), u"%1core-item.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), u"%1extended-item.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 4 ).href(), u"https://raw.githubusercontent.com/radiantearth/stac-spec/v1.0.0/examples/collection.json"_s );

  QCOMPARE( col->providers().size(), 1 );
  QCOMPARE( col->stacExtensions().size(), 3 );
  QCOMPARE( col->license(), "CC-BY-4.0"_L1 );

  // extent
  QgsStacExtent ext( col->extent() );
  QVERIFY( !ext.hasDetailedSpatialExtents() );
  QCOMPARE( ext.spatialExtent().xMinimum(), 172.91173669923782 );
  QCOMPARE( ext.spatialExtent().yMinimum(), 1.3438851951615003 );
  QCOMPARE( ext.spatialExtent().xMaximum(), 172.95469614953714 );
  QCOMPARE( ext.spatialExtent().yMaximum(), 1.3690476620161975 );

  QVERIFY( !ext.hasDetailedTemporalExtents() );
  QCOMPARE( ext.temporalExtent().begin(), QDateTime::fromString( u"2020-12-11T22:38:32.125Z"_s, Qt::ISODateWithMs ) );
  QCOMPARE( ext.temporalExtent().end(), QDateTime::fromString( u"2020-12-14T18:02:31.437Z"_s, Qt::ISODateWithMs ) );

  // summaries
  QVariantMap sum( col->summaries() );
  QCOMPARE( sum.size(), 9 );
  QCOMPARE( sum.value( u"platform"_s ).toStringList(), QStringList() << "cool_sat1"_L1 << "cool_sat2"_L1 );

  QCOMPARE( col->assets().size(), 1 );

  QgsStacAsset asset = col->assets().value( u"geoparquet-file"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QCOMPARE( asset.formatName(), u"Parquet"_s );
  QCOMPARE( asset.uri().layerType, u"vector"_s );
  QVERIFY( asset.isDownloadable() );
  QVERIFY( asset.isCloudOptimized() );
}

void TestQgsStac::testParseLocalItem()
{
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"core-item.json"_s ) );
  QgsStacController c;
  std::unique_ptr< QgsStacItem > item = c.fetchStacObject< QgsStacItem >( url.toString() );
  QVERIFY( item );
  QCOMPARE( item->type(), Qgis::StacObjectType::Item );

  QVERIFY( item );
  QCOMPARE( item->id(), "20201211_223832_CS2"_L1 );
  QCOMPARE( item->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( item->stacExtensions().size(), 0 );
  QCOMPARE( item->title(), u"Core Item"_s );
  QCOMPARE( item->description(), u"A sample STAC Item that includes examples of all common metadata"_s );

  const QgsMimeDataUtils::UriList uris = item->uris();
  QCOMPARE( uris.size(), 4 );
  QCOMPARE( uris.at( 0 ).uri, u"file://%1%2"_s.arg( mDataDir, u"20201211_223832_CS2_analytic.tif"_s ) );
  QCOMPARE( uris.at( 0 ).name, u"4-Band Analytic"_s );
  QCOMPARE( uris.at( 1 ).uri, u"/vsicurl/https://github.com/opengeospatial/geoparquet/raw/refs/heads/main/examples/example.parquet"_s );
  QCOMPARE( uris.at( 1 ).name, u"GeoParquet File"_s );
  QCOMPARE( uris.at( 2 ).uri, u"/vsicurl/https://storage.googleapis.com/open-cogs/stac-examples/20201211_223832_CS2.tif"_s );
  QCOMPARE( uris.at( 2 ).name, u"3-Band Visual"_s );
  QCOMPARE( uris.at( 3 ).uri, u"ZARR:\"/vsicurl/https://objectstore.eodc.eu:2222/e05ab01a9d56408d82ac32d69a5aae2a:202505-s02msil2a/22/products/cpm_v256/S2B_MSIL2A_20250522T125039_N0511_R095_T26TML_20250522T133252.zarr\""_s );
  QCOMPARE( uris.at( 3 ).name, u"Example Zarr Store"_s );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = item->links();
  QCOMPARE( links.size(), 4 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), u"%1collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), u"%1collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), u"%1collection.json"_s.arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), u"http://remotedata.io/catalog/20201211_223832_CS2/index.html"_s );

  QCOMPARE( item->assets().size(), 8 );
  QgsStacAsset asset = item->assets().value( u"analytic"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QCOMPARE( asset.href(), basePath + u"20201211_223832_CS2_analytic.tif"_s );
  QVERIFY( asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), u"COG"_s );
  QVERIFY( asset.isDownloadable() );

  QgsMimeDataUtils::Uri uri = asset.uri();
  QCOMPARE( uri.uri, basePath + u"20201211_223832_CS2_analytic.tif"_s );
  QCOMPARE( uri.name, u"4-Band Analytic"_s );
  QCOMPARE( uri.layerType, u"raster"_s );

  asset = item->assets().value( u"thumbnail"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QCOMPARE( asset.href(), u"https://storage.googleapis.com/open-cogs/stac-examples/20201211_223832_CS2.jpg"_s );
  QVERIFY( !asset.isCloudOptimized() );
  uri = asset.uri();
  QVERIFY( !uri.isValid() );
  QVERIFY( uri.uri.isEmpty() );
  QVERIFY( uri.name.isEmpty() );
  QVERIFY( asset.isDownloadable() );

  // normal geotiff is not cloud optimized
  asset = item->assets().value( u"udm"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QVERIFY( !asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), QString() );
  uri = asset.uri();
  QVERIFY( !uri.isValid() );
  QVERIFY( uri.uri.isEmpty() );
  QVERIFY( uri.name.isEmpty() );
  QVERIFY( asset.isDownloadable() );

  // Zarr recognised as cloud optimized
  asset = item->assets().value( u"zarr-store"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QVERIFY( asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), u"Zarr"_s );
  QCOMPARE( asset.uri().layerType, u"raster"_s );
  QVERIFY( !asset.isDownloadable() );

  // GeoParquet recognised as cloud optimized
  asset = item->assets().value( u"geoparquet-file"_s, QgsStacAsset( {}, {}, {}, {}, {} ) );
  QVERIFY( asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), u"Parquet"_s );
  QCOMPARE( asset.uri().layerType, u"vector"_s );
  QVERIFY( asset.isDownloadable() );
}

void TestQgsStac::testParseLocalItemCollection()
{
  const QString fileName = mDataDir + u"itemcollection-sample-full.json"_s;
  QgsStacController c;
  std::unique_ptr< QgsStacItemCollection > ic = c.fetchItemCollection( u"file://%1"_s.arg( fileName ) );
  QVERIFY( ic );
  QCOMPARE( ic->numberReturned(), 1 );
  QCOMPARE( ic->numberMatched(), 10 );
  QCOMPARE( ic->rootUrl().toString(), "http://stac.example.com/"_L1 );

  QVector<QgsStacItem *> items = ic->items();
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first()->id(), "cs3-20160503_132131_05"_L1 );
  QCOMPARE( items.first()->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( items.first()->links().size(), 3 );
  QCOMPARE( items.first()->stacExtensions().size(), 0 );
  QCOMPARE( items.first()->assets().size(), 2 );
}

void TestQgsStac::testParseLocalCollections()
{
  const QString fileName = mDataDir + u"collectioncollection-sample-full.json"_s;
  QgsStacController c;
  std::unique_ptr< QgsStacCollectionList > cols = c.fetchCollections( u"file://%1"_s.arg( fileName ) );
  QVERIFY( cols );
  QCOMPARE( cols->numberReturned(), 1 );
  QCOMPARE( cols->numberMatched(), 11 );
  QCOMPARE( cols->rootUrl().toString(), "http://stac.example.com/"_L1 );
  QCOMPARE( cols->url().toString(), "http://stac.example.com/collections?page=2"_L1 );
  QCOMPARE( cols->nextUrl().toString(), "http://stac.example.com/collections?page=3"_L1 );
  QCOMPARE( cols->prevUrl().toString(), "http://stac.example.com/collections?page=1"_L1 );

  QCOMPARE( cols->collections().size(), 1 );

  QgsStacCollection *col = cols->collections().first();
  QCOMPARE( col->id(), u"simple-collection"_s );
  QCOMPARE( col->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( col->links().size(), 3 );
  QCOMPARE( col->stacExtensions().size(), 0 );
}

void TestQgsStac::testFetchStacObjectAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // Catalog
  QUrl url( u"file://%1%2"_s.arg( mDataDir, u"catalog.json"_s ) );
  int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  std::unique_ptr< QgsStacCatalog > obj = c.takeStacObject< QgsStacCatalog >( id );
  QVERIFY( obj );
  QCOMPARE( obj->type(), Qgis::StacObjectType::Catalog );

  // cannot take same id twice
  obj = c.takeStacObject< QgsStacCatalog >( id );
  QVERIFY( !obj );


  // Collection
  spy.clear();
  url.setUrl( u"file://%1%2"_s.arg( mDataDir, u"collection.json"_s ) );
  id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  obj = c.takeStacObject< QgsStacCatalog >( id );
  QVERIFY( obj );
  QCOMPARE( obj->type(), Qgis::StacObjectType::Collection );

  // cannot take same id twice
  obj = c.takeStacObject< QgsStacCatalog >( id );
  QVERIFY( obj == nullptr );


  // Item
  spy.clear();
  url.setUrl( u"file://%1%2"_s.arg( mDataDir, u"core-item.json"_s ) );
  id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  std::unique_ptr< QgsStacItem > item = c.takeStacObject< QgsStacItem >( id );
  QVERIFY( item );
  QCOMPARE( item->type(), Qgis::StacObjectType::Item );

  // cannot take same id twice
  item = c.takeStacObject< QgsStacItem >( id );
  QVERIFY( !item );
}

void TestQgsStac::testFetchItemCollectionAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // Catalog
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"itemcollection-sample-full.json"_s ) );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  std::unique_ptr< QgsStacItemCollection > ic = c.takeItemCollection( id );
  QVERIFY( ic );
  QCOMPARE( ic->numberReturned(), 1 );
  QCOMPARE( ic->numberMatched(), 10 );
  QCOMPARE( ic->rootUrl().toString(), "http://stac.example.com/"_L1 );

  QVector<QgsStacItem *> items = ic->items();
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first()->id(), "cs3-20160503_132131_05"_L1 );
  QCOMPARE( items.first()->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( items.first()->links().size(), 3 );
  QCOMPARE( items.first()->stacExtensions().size(), 0 );
  QCOMPARE( items.first()->assets().size(), 2 );

  // cannot take same id twice
  ic = c.takeItemCollection( id );
  QVERIFY( !ic );
}

void TestQgsStac::testFetchCollectionsAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // Catalog
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"collectioncollection-sample-full.json"_s ) );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  std::unique_ptr< QgsStacCollectionList > cols = c.takeCollections( id );
  QVERIFY( cols );
  QCOMPARE( cols->numberReturned(), 1 );
  QCOMPARE( cols->numberMatched(), 11 );
  QCOMPARE( cols->rootUrl().toString(), "http://stac.example.com/"_L1 );
  QCOMPARE( cols->url().toString(), "http://stac.example.com/collections?page=2"_L1 );
  QCOMPARE( cols->nextUrl().toString(), "http://stac.example.com/collections?page=3"_L1 );
  QCOMPARE( cols->prevUrl().toString(), "http://stac.example.com/collections?page=1"_L1 );

  QCOMPARE( cols->collections().size(), 1 );

  QgsStacCollection *col = cols->collections().first();
  QCOMPARE( col->id(), u"simple-collection"_s );
  QCOMPARE( col->stacVersion(), "1.0.0"_L1 );
  QCOMPARE( col->links().size(), 3 );
  QCOMPARE( col->stacExtensions().size(), 0 );
}

void TestQgsStac::testFetchStacObjectAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // this is not a stac object (catalog, collection or item)
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"collectioncollection-sample-full.json"_s ) );
  const int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeStacObject< QgsStacObject >( id ) );
}

void TestQgsStac::testFetchItemCollectionAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // This is not an item collection
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"collectioncollection-sample-full.json"_s ) );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeItemCollection( id ) );
}

void TestQgsStac::testFetchCollectionsAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // This is not a collections collection
  const QUrl url( u"file://%1%2"_s.arg( mDataDir, u"itemcollection-sample-full.json"_s ) );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeCollections( id ) );
}

void TestQgsStac::testFetchStacObjectAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // Unreachable link
  const QUrl url( u"https://localhost/42.json"_s );
  const int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeStacObject< QgsStacObject >( id ) );
}

void TestQgsStac::testFetchItemCollectionAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // Unreachable link
  const QUrl url( u"https://localhost/42.json"_s );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeItemCollection( id ) );
}

void TestQgsStac::testFetchCollectionsAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // Unreachable link
  const QUrl url( u"https://localhost/42.json"_s );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( !c.takeCollections( id ) );
}

QGSTEST_MAIN( TestQgsStac )
#include "testqgsstac.moc"
