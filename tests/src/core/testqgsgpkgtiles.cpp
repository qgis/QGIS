/***************************************************************************
  testqgsgpkgtiles.cpp
  --------------------------------------
  Date                 : September 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall.dawson@gmail.com
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
#include <QString>

using namespace Qt::StringLiterals;

//qgis includes...
#include "qgsapplication.h"
#include "qgsgpkgtiles.h"
#include "qgsrasterlayer.h"
#include "qgstiles.h"
#include <sqlite3.h>
#include <QBuffer>

/**
 * \ingroup UnitTests
 * This is a unit test for QgsGeoPackageTiles tiles
 */
class TestQgsGeoPackageTiles : public QObject
{
    Q_OBJECT

  public:
    TestQgsGeoPackageTiles() = default;

  private:
    QString mDataDir;
    QTemporaryDir mTmpDir;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testCreateValidation();
    void testCreateAndWriteTiles();
    void testBatchWriteTiles();
    void testCustomCrsAndTileMatrix();
    void testGdalRasterLayerOpen();
    void testGdalRasterLayerOpenCustomCrs();
};


void TestQgsGeoPackageTiles::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
}

void TestQgsGeoPackageTiles::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGeoPackageTiles::testCreateValidation()
{
  // no filenname
  QgsGeoPackageTiles emptyTiles { QString() };

  const QgsRectangle extent( -100, -100, 100, 100 );

  QgsTileMatrix z0matrix = QgsTileMatrix::fromWebMercator( 0 );
  QVERIFY( !emptyTiles.create( z0matrix, extent, 0, 2 ) );

  // writing over an existing file should fail
  const QString existingFilePath = mTmpDir.filePath( u"existing.gpkg"_s );
  QFile file( existingFilePath );
  QVERIFY( file.open( QIODevice::WriteOnly ) );
  file.write( "dummy content" );
  file.close();

  QgsGeoPackageTiles existingTiles( existingFilePath );
  QVERIFY( !existingTiles.create( z0matrix, extent, 0, 2 ) );
}

void TestQgsGeoPackageTiles::testCreateAndWriteTiles()
{
  const QString filePath = mTmpDir.filePath( u"test_tiles.gpkg"_s );
  QgsGeoPackageTiles gpkgTiles( filePath );

  const QgsRectangle extent( -20037508.34, -20037508.34, 20037508.34, 20037508.34 );
  const QgsTileMatrix z0matrix = QgsTileMatrix::fromWebMercator( 0 );
  QVERIFY( gpkgTiles.create( z0matrix, extent, 0, 2, 256, 256 ) );

  QgsGeoPackageTiles::TileData tile1;
  tile1.z = 0;
  tile1.x = 0;
  tile1.y = 0;
  tile1.data = QByteArray( "PNG_DUMMY_DATA_1" );

  gpkgTiles.setTileData( { tile1 } );
  QVERIFY( gpkgTiles.finalize() );
  QVERIFY( gpkgTiles.close() );

  // inspect content
  sqlite3_database_unique_ptr db;
  int result = db.open_v2( filePath, SQLITE_OPEN_READONLY, nullptr );
  QCOMPARE( result, SQLITE_OK );

  int res = 0;
  QString sql = u"SELECT data_type FROM gpkg_contents WHERE table_name='tiles';"_s;
  sqlite3_statement_unique_ptr stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsText( 0 ), u"tiles"_s );

  sql = u"SELECT COUNT(*) FROM gpkg_tile_matrix WHERE table_name='tiles';"_s;
  stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsInt64( 0 ), 3 );

  sql = u"SELECT tile_data FROM tiles WHERE zoom_level=0 AND tile_column=0 AND tile_row=0;"_s;
  stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsBlob( 0 ), QByteArray( "PNG_DUMMY_DATA_1" ) );
}

void TestQgsGeoPackageTiles::testBatchWriteTiles()
{
  const QString filePath = mTmpDir.filePath( u"test_batch_tiles.gpkg"_s );
  QgsGeoPackageTiles gpkgTiles( filePath );

  const QgsRectangle extent( -20037508.34, -20037508.34, 20037508.34, 20037508.34 );
  const QgsTileMatrix z0matrix = QgsTileMatrix::fromWebMercator( 0 );
  QVERIFY( gpkgTiles.create( z0matrix, extent, 1, 2, 256, 256 ) );

  QList<QgsGeoPackageTiles::TileData> batch;
  for ( int x = 0; x < 5; ++x )
  {
    for ( int y = 0; y < 5; ++y )
    {
      batch.append( { 1, x, y, QString( "DATA_%1_%2" ).arg( x ).arg( y ).toUtf8() } );
    }
  }

  gpkgTiles.setTileData( batch );
  QVERIFY( gpkgTiles.finalize() );
  QVERIFY( gpkgTiles.close() );

  sqlite3_database_unique_ptr db;
  int result = db.open_v2( filePath, SQLITE_OPEN_READONLY, nullptr );
  QCOMPARE( result, SQLITE_OK );

  int res = 0;
  const QString sql = u"SELECT COUNT(*) FROM tiles;"_s;
  sqlite3_statement_unique_ptr stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsInt64( 0 ), 25 );
}

void TestQgsGeoPackageTiles::testCustomCrsAndTileMatrix()
{
  const QString filePath = mTmpDir.filePath( u"test_custom_crs_matrix.gpkg"_s );
  QgsGeoPackageTiles gpkgTiles( filePath );

  const QgsCoordinateReferenceSystem crs( u"EPSG:4326"_s );
  const QgsRectangle tmsExtent( -180.0, -90.0, 180.0, 90.0 );
  const QgsRectangle contentsExtent( -100.0, -40.0, 100.0, 40.0 );

  const QgsTileMatrix z0Matrix = QgsTileMatrix::fromCustomDef( 0, crs, QgsPointXY( -180, 90 ), 180, 2, 1 );

  QVERIFY( gpkgTiles.create( z0Matrix, contentsExtent, 0, 1, 256, 256 ) );
  QVERIFY( gpkgTiles.finalize() );
  QVERIFY( gpkgTiles.close() );

  sqlite3_database_unique_ptr db;
  int result = db.open_v2( filePath, SQLITE_OPEN_READONLY, nullptr );
  QCOMPARE( result, SQLITE_OK );

  int res = 0;
  QString sql = u"SELECT srs_id FROM gpkg_contents WHERE table_name='tiles';"_s;
  sqlite3_statement_unique_ptr stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsInt64( 0 ), 4326 );

  sql = u"SELECT matrix_width, matrix_height FROM gpkg_tile_matrix WHERE table_name='tiles' AND zoom_level=0;"_s;
  stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsInt64( 0 ), 2 );
  QCOMPARE( stmt.columnAsInt64( 1 ), 1 );

  sql = u"SELECT matrix_width, matrix_height FROM gpkg_tile_matrix WHERE table_name='tiles' AND zoom_level=1;"_s;
  stmt = db.prepare( sql, res );
  QCOMPARE( res, SQLITE_OK );
  QCOMPARE( stmt.step(), SQLITE_ROW );
  QCOMPARE( stmt.columnAsInt64( 0 ), 4 );
  QCOMPARE( stmt.columnAsInt64( 1 ), 2 );
}

void TestQgsGeoPackageTiles::testGdalRasterLayerOpen()
{
  const QString filePath = mTmpDir.filePath( u"test_gdal_open.gpkg"_s );
  QgsGeoPackageTiles gpkgTiles( filePath );

  const QgsRectangle extent( -20037508.342789244, -20037508.342789244, 20037508.342789244, 20037508.342789244 );
  const QgsTileMatrix z0matrix = QgsTileMatrix::fromWebMercator( 0 );
  QVERIFY( gpkgTiles.create( z0matrix, extent, 0, 0, 256, 256 ) );

  // a valid 256x256 PNG image tile
  QImage img( 256, 256, QImage::Format_ARGB32 );
  img.fill( Qt::blue );
  QByteArray pngData;
  QBuffer buffer( &pngData );
  buffer.open( QIODevice::WriteOnly );
  QVERIFY( img.save( &buffer, "PNG" ) );

  QgsGeoPackageTiles::TileData tile;
  tile.z = 0;
  tile.x = 0;
  tile.y = 0;
  tile.data = pngData;

  gpkgTiles.setTileData( { tile } );
  QVERIFY( gpkgTiles.finalize() );
  QVERIFY( gpkgTiles.close() );

  // open generated file via QgsRasterLayer using GDAL provider -- make sure GDAL is happy with this
  auto layer = std::make_unique<QgsRasterLayer>( filePath, u"gpkg_raster"_s, u"gdal"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->crs().authid(), u"EPSG:3857"_s );
  QCOMPARE( layer->width(), 256 );
  QCOMPARE( layer->height(), 256 );
  QGSCOMPARENEAR( layer->extent().xMinimum(), -20037508.3428, 1 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 20037508.342768, 1 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), -20037508.342768, 1 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 20037508.3428, 1 );
}

void TestQgsGeoPackageTiles::testGdalRasterLayerOpenCustomCrs()
{
  const QString filePath = mTmpDir.filePath( u"test_gdal_open_custom_crs.gpkg"_s );
  QgsGeoPackageTiles gpkgTiles( filePath );

  const QgsCoordinateReferenceSystem crs( u"EPSG:4326"_s );
  const QgsRectangle tmsExtent( -180.0, -90.0, 180.0, 90.0 );
  const QgsRectangle contentsExtent( -180.0, 0.0, 180.0, 90.0 );

  const QgsTileMatrix z0Matrix = QgsTileMatrix::fromCustomDef( 0, crs, QgsPointXY( -180, 90 ), 180, 2, 1 );

  QVERIFY( gpkgTiles.create( z0Matrix, contentsExtent, 0, 0, 256, 256 ) );

  // a valid 256x256 PNG image tile
  QImage img( 256, 256, QImage::Format_ARGB32 );
  img.fill( Qt::blue );
  QByteArray pngData;
  QBuffer buffer( &pngData );
  buffer.open( QIODevice::WriteOnly );
  QVERIFY( img.save( &buffer, "PNG" ) );

  // Tile (0,0,0) -> Northern Western Hemisphere [-180, 0, 0, 90]
  QgsGeoPackageTiles::TileData tile0 { 0, 0, 0, pngData };
  // Tile (0,1,0) -> Northern Eastern Hemisphere [0, 0, 180, 90]
  QgsGeoPackageTiles::TileData tile1 { 0, 1, 0, pngData };

  gpkgTiles.setTileData( { tile0, tile1 } );
  QVERIFY( gpkgTiles.finalize() );
  QVERIFY( gpkgTiles.close() );

  auto layer = std::make_unique<QgsRasterLayer>( filePath, u"gpkg_raster_4326"_s, u"gdal"_s );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->crs().authid(), u"EPSG:4326"_s );

  QCOMPARE( layer->width(), 512 );
  QCOMPARE( layer->height(), 128 );

  QGSCOMPARENEAR( layer->extent().xMinimum(), -180.0, 0.0001 );
  QGSCOMPARENEAR( layer->extent().xMaximum(), 180.0, 0.0001 );
  QGSCOMPARENEAR( layer->extent().yMinimum(), 0.0, 0.0001 );
  QGSCOMPARENEAR( layer->extent().yMaximum(), 90.0, 0.0001 );
}

QGSTEST_MAIN( TestQgsGeoPackageTiles )
#include "testqgsgpkgtiles.moc"
