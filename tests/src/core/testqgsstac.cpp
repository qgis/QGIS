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

#include "qgstest.h"
#include <QObject>
#include <QString>

//qgis includes...
#include "qgsstaccontroller.h"
#include "qgsstaccatalog.h"
#include "qgsstaccollection.h"
#include "qgsstacitem.h"
#include "qgsstacitemcollection.h"
#include "qgsstaccollections.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgis.h"
#include "qsignalspy.h"

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
  mDataDir = QStringLiteral( "%1/stac/" ).arg( TEST_DATA_DIR );
}

void TestQgsStac::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsStac::testParseLocalCatalog()
{
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "catalog.json" ) ) );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( url.toString() );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Catalog );
  QgsStacCatalog *cat = dynamic_cast<QgsStacCatalog *>( obj );

  QVERIFY( cat );
  QCOMPARE( cat->id(), QLatin1String( "examples" ) );
  QCOMPARE( cat->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( cat->title(), QLatin1String( "Example Catalog" ) );
  QCOMPARE( cat->description(), QLatin1String( "This catalog is a simple demonstration of an example catalog that is used to organize a hierarchy of collections and their items." ) );
  QVERIFY( cat->stacExtensions().isEmpty() );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = cat->links();
  QCOMPARE( links.size(), 6 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), QStringLiteral( "%1catalog.json" ).arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), QStringLiteral( "%1extensions-collection/collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), QStringLiteral( "%1collection-only/collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), QStringLiteral( "%1collection-only/collection-with-schemas.json" ).arg( basePath ) );
  QCOMPARE( links.at( 4 ).href(), QStringLiteral( "%1collectionless-item.json" ).arg( basePath ) );
  QCOMPARE( links.at( 5 ).href(), QStringLiteral( "https://raw.githubusercontent.com/radiantearth/stac-spec/v1.0.0/examples/catalog.json" ) );

  delete cat;
}

void TestQgsStac::testParseLocalCollection()
{
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "collection.json" ) ) );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( url.toString() );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Collection );
  QgsStacCollection *col = dynamic_cast<QgsStacCollection *>( obj );

  QVERIFY( col );
  QCOMPARE( col->id(), QLatin1String( "simple-collection" ) );
  QCOMPARE( col->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( col->title(), QLatin1String( "Simple Example Collection" ) );
  QCOMPARE( col->description(), QLatin1String( "A simple collection demonstrating core catalog fields with links to a couple of items" ) );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = col->links();
  QCOMPARE( links.size(), 5 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), QStringLiteral( "%1collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), QStringLiteral( "%1simple-item.json" ).arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), QStringLiteral( "%1core-item.json" ).arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), QStringLiteral( "%1extended-item.json" ).arg( basePath ) );
  QCOMPARE( links.at( 4 ).href(), QStringLiteral( "https://raw.githubusercontent.com/radiantearth/stac-spec/v1.0.0/examples/collection.json" ) );

  QCOMPARE( col->providers().size(), 1 );
  QCOMPARE( col->stacExtensions().size(), 3 );
  QCOMPARE( col->license(), QLatin1String( "CC-BY-4.0" ) );
  QVERIFY( col->assets().isEmpty() );

  // extent
  QgsStacExtent ext( col->extent() );
  QVERIFY( !ext.hasDetailedSpatialExtents() );
  QCOMPARE( ext.spatialExtent().xMinimum(), 172.91173669923782 );
  QCOMPARE( ext.spatialExtent().yMinimum(), 1.3438851951615003 );
  QCOMPARE( ext.spatialExtent().xMaximum(), 172.95469614953714 );
  QCOMPARE( ext.spatialExtent().yMaximum(), 1.3690476620161975 );

  QVERIFY( !ext.hasDetailedTemporalExtents() );
  QCOMPARE( ext.temporalExtent().begin(), QDateTime::fromString( QStringLiteral( "2020-12-11T22:38:32.125Z" ), Qt::ISODateWithMs ) );
  QCOMPARE( ext.temporalExtent().end(), QDateTime::fromString( QStringLiteral( "2020-12-14T18:02:31.437Z" ), Qt::ISODateWithMs ) );

  // summaries
  QVariantMap sum( col->summaries() );
  QCOMPARE( sum.size(), 9 );
  QCOMPARE( sum.value( QStringLiteral( "platform" ) ).toStringList(), QStringList() << QLatin1String( "cool_sat1" ) << QLatin1String( "cool_sat2" ) );

  delete col;
}

void TestQgsStac::testParseLocalItem()
{
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "core-item.json" ) ) );
  QgsStacController c;
  QgsStacObject *obj = c.fetchStacObject( url.toString() );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Item );
  QgsStacItem *item = dynamic_cast<QgsStacItem *>( obj );

  QVERIFY( item );
  QCOMPARE( item->id(), QLatin1String( "20201211_223832_CS2" ) );
  QCOMPARE( item->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( item->stacExtensions().size(), 0 );
  QCOMPARE( item->title(), QStringLiteral( "Core Item" ) );
  QCOMPARE( item->description(), QStringLiteral( "A sample STAC Item that includes examples of all common metadata" ) );

  const QgsMimeDataUtils::UriList uris = item->uris();
  QCOMPARE( uris.size(), 2 );
  QCOMPARE( uris.first().uri, QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "20201211_223832_CS2_analytic.tif" ) ) );
  QCOMPARE( uris.first().name, QStringLiteral( "4-Band Analytic" ) );
  QCOMPARE( uris.last().uri, QStringLiteral( "/vsicurl/https://storage.googleapis.com/open-cogs/stac-examples/20201211_223832_CS2.tif" ) );
  QCOMPARE( uris.last().name, QStringLiteral( "3-Band Visual" ) );

  // check that relative links are correctly resolved into absolute links
  const QVector<QgsStacLink> links = item->links();
  QCOMPARE( links.size(), 4 );
  const QString basePath = url.adjusted( QUrl::RemoveFilename ).toString();
  QCOMPARE( links.at( 0 ).href(), QStringLiteral( "%1collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 1 ).href(), QStringLiteral( "%1collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 2 ).href(), QStringLiteral( "%1collection.json" ).arg( basePath ) );
  QCOMPARE( links.at( 3 ).href(), QStringLiteral( "http://remotedata.io/catalog/20201211_223832_CS2/index.html" ) );

  QCOMPARE( item->assets().size(), 6 );
  QgsStacAsset asset = item->assets().value( QStringLiteral( "analytic" ), QgsStacAsset( {}, {}, {}, {}, {} ) );
  QCOMPARE( asset.href(), basePath + QStringLiteral( "20201211_223832_CS2_analytic.tif" ) );
  QVERIFY( asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), QStringLiteral( "COG" ) );

  QgsMimeDataUtils::Uri uri = asset.uri();
  QCOMPARE( uri.uri, basePath + QStringLiteral( "20201211_223832_CS2_analytic.tif" ) );
  QCOMPARE( uri.name, QStringLiteral( "4-Band Analytic" ) );
  QCOMPARE( uri.layerType, QStringLiteral( "raster" ) );

  asset = item->assets().value( QStringLiteral( "thumbnail" ), QgsStacAsset( {}, {}, {}, {}, {} ) );
  QCOMPARE( asset.href(), QStringLiteral( "https://storage.googleapis.com/open-cogs/stac-examples/20201211_223832_CS2.jpg" ) );
  QVERIFY( !asset.isCloudOptimized() );
  uri = asset.uri();
  QVERIFY( !uri.isValid() );
  QVERIFY( uri.uri.isEmpty() );
  QVERIFY( uri.name.isEmpty() );

  // normal geotiff is not cloud optimized
  asset = item->assets().value( QStringLiteral( "udm" ), QgsStacAsset( {}, {}, {}, {}, {} ) );
  QVERIFY( !asset.isCloudOptimized() );
  QCOMPARE( asset.formatName(), QString() );
  uri = asset.uri();
  QVERIFY( !uri.isValid() );
  QVERIFY( uri.uri.isEmpty() );
  QVERIFY( uri.name.isEmpty() );

  delete item;
}

void TestQgsStac::testParseLocalItemCollection()
{
  const QString fileName = mDataDir + QStringLiteral( "itemcollection-sample-full.json" );
  QgsStacController c;
  QgsStacItemCollection *ic = c.fetchItemCollection( QStringLiteral( "file://%1" ).arg( fileName ) );
  QVERIFY( ic );
  QCOMPARE( ic->numberReturned(), 1 );
  QCOMPARE( ic->numberMatched(), 10 );
  QCOMPARE( ic->rootUrl().toString(), QLatin1String( "http://stac.example.com/" ) );

  QVector<QgsStacItem *> items = ic->items();
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first()->id(), QLatin1String( "cs3-20160503_132131_05" ) );
  QCOMPARE( items.first()->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( items.first()->links().size(), 3 );
  QCOMPARE( items.first()->stacExtensions().size(), 0 );
  QCOMPARE( items.first()->assets().size(), 2 );

  delete ic;
}

void TestQgsStac::testParseLocalCollections()
{
  const QString fileName = mDataDir + QStringLiteral( "collectioncollection-sample-full.json" );
  QgsStacController c;
  QgsStacCollections *cols = c.fetchCollections( QStringLiteral( "file://%1" ).arg( fileName ) );
  QVERIFY( cols );
  QCOMPARE( cols->numberReturned(), 1 );
  QCOMPARE( cols->numberMatched(), 11 );
  QCOMPARE( cols->rootUrl().toString(), QLatin1String( "http://stac.example.com/" ) );
  QCOMPARE( cols->url().toString(), QLatin1String( "http://stac.example.com/collections?page=2" ) );
  QCOMPARE( cols->nextUrl().toString(), QLatin1String( "http://stac.example.com/collections?page=3" ) );
  QCOMPARE( cols->prevUrl().toString(), QLatin1String( "http://stac.example.com/collections?page=1" ) );

  QCOMPARE( cols->collections().size(), 1 );

  QgsStacCollection *col = cols->collections().first();
  QCOMPARE( col->id(), QStringLiteral( "simple-collection" ) );
  QCOMPARE( col->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( col->links().size(), 3 );
  QCOMPARE( col->stacExtensions().size(), 0 );

  delete cols;
}

void TestQgsStac::testFetchStacObjectAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // Catalog
  QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "catalog.json" ) ) );
  int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QgsStacObject *obj = c.takeStacObject( id );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Catalog );
  delete obj;

  // cannot take same id twice
  obj = c.takeStacObject( id );
  QVERIFY( obj == nullptr );


  // Collection
  spy.clear();
  url.setUrl( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "collection.json" ) ) );
  id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  obj = c.takeStacObject( id );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Collection );
  delete obj;

  // cannot take same id twice
  obj = c.takeStacObject( id );
  QVERIFY( obj == nullptr );


  // Item
  spy.clear();
  url.setUrl( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "core-item.json" ) ) );
  id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  obj = c.takeStacObject( id );
  QVERIFY( obj );
  QCOMPARE( obj->type(), QgsStacObject::Type::Item );
  delete obj;

  // cannot take same id twice
  obj = c.takeStacObject( id );
  QVERIFY( obj == nullptr );
}

void TestQgsStac::testFetchItemCollectionAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // Catalog
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "itemcollection-sample-full.json" ) ) );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QgsStacItemCollection *ic = c.takeItemCollection( id );
  QVERIFY( ic );
  QCOMPARE( ic->numberReturned(), 1 );
  QCOMPARE( ic->numberMatched(), 10 );
  QCOMPARE( ic->rootUrl().toString(), QLatin1String( "http://stac.example.com/" ) );

  QVector<QgsStacItem *> items = ic->items();
  QCOMPARE( items.size(), 1 );
  QCOMPARE( items.first()->id(), QLatin1String( "cs3-20160503_132131_05" ) );
  QCOMPARE( items.first()->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( items.first()->links().size(), 3 );
  QCOMPARE( items.first()->stacExtensions().size(), 0 );
  QCOMPARE( items.first()->assets().size(), 2 );
  delete ic;

  // cannot take same id twice
  ic = c.takeItemCollection( id );
  QVERIFY( ic == nullptr );
}

void TestQgsStac::testFetchCollectionsAsync()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // Catalog
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "collectioncollection-sample-full.json" ) ) );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id and an empty error string
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  QVERIFY( spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QgsStacCollections *cols = c.takeCollections( id );
  QVERIFY( cols );
  QCOMPARE( cols->numberReturned(), 1 );
  QCOMPARE( cols->numberMatched(), 11 );
  QCOMPARE( cols->rootUrl().toString(), QLatin1String( "http://stac.example.com/" ) );
  QCOMPARE( cols->url().toString(), QLatin1String( "http://stac.example.com/collections?page=2" ) );
  QCOMPARE( cols->nextUrl().toString(), QLatin1String( "http://stac.example.com/collections?page=3" ) );
  QCOMPARE( cols->prevUrl().toString(), QLatin1String( "http://stac.example.com/collections?page=1" ) );

  QCOMPARE( cols->collections().size(), 1 );

  QgsStacCollection *col = cols->collections().first();
  QCOMPARE( col->id(), QStringLiteral( "simple-collection" ) );
  QCOMPARE( col->stacVersion(), QLatin1String( "1.0.0" ) );
  QCOMPARE( col->links().size(), 3 );
  QCOMPARE( col->stacExtensions().size(), 0 );

  delete cols;
}

void TestQgsStac::testFetchStacObjectAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // this is not a stac object (catalog, collection or item)
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "collectioncollection-sample-full.json" ) ) );
  const int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeStacObject( id ) == nullptr );
}

void TestQgsStac::testFetchItemCollectionAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // This is not an item collection
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "collectioncollection-sample-full.json" ) ) );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeItemCollection( id ) == nullptr );
}

void TestQgsStac::testFetchCollectionsAsyncInvalid()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // This is not a collections collection
  const QUrl url( QStringLiteral( "file://%1%2" ).arg( mDataDir, QStringLiteral( "itemcollection-sample-full.json" ) ) );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeCollections( id ) == nullptr );
}

void TestQgsStac::testFetchStacObjectAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedStacObjectRequest );

  // Unreachable link
  const QUrl url( QStringLiteral( "https://localhost/42.json" ) );
  const int id = c.fetchStacObjectAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeStacObject( id ) == nullptr );
}

void TestQgsStac::testFetchItemCollectionAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedItemCollectionRequest );

  // Unreachable link
  const QUrl url( QStringLiteral( "https://localhost/42.json" ) );
  const int id = c.fetchItemCollectionAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeItemCollection( id ) == nullptr );
}

void TestQgsStac::testFetchCollectionsAsyncUnavailable()
{
  QgsStacController c;
  QSignalSpy spy( &c, &QgsStacController::finishedCollectionsRequest );

  // Unreachable link
  const QUrl url( QStringLiteral( "https://localhost/42.json" ) );
  const int id = c.fetchCollectionsAsync( url.toString() );
  QVERIFY( id > -1 );
  spy.wait( 100 );
  QCOMPARE( spy.count(), 1 );

  // signal contains id
  QCOMPARE( spy.at( 0 ).at( 0 ).toInt(), id );
  // Error should not be empty on failure
  QVERIFY( !spy.at( 0 ).at( 1 ).toString().isEmpty() );

  QVERIFY( c.takeCollections( id ) == nullptr );
}

QGSTEST_MAIN( TestQgsStac )
#include "testqgsstac.moc"
