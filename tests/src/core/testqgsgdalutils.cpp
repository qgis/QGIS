/***************************************************************************
     testqgsgdalutils.cpp
     --------------------
    begin                : September 2018
    copyright            : (C) 2018 Even Rouault
    email                : even.rouault at spatialys.com
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
#include <QStringList>
#include <QSettings>

#include <gdal.h>

#include "qgsgdalutils.h"
#include "qgsapplication.h"
#include "qgsrasterlayer.h"


class TestQgsGdalUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void supportsRasterCreate();
    void testCreateSingleBandMemoryDataset();
    void testCreateMultiBandMemoryDataset();
    void testCreateSingleBandTiffDataset();
    void testResampleSingleBandRaster();
    void testImageToDataset();
    void testResampleImageToImage();
    void testPathIsCheapToOpen();
    void testVrtMatchesLayerType();
    void testMultilayerExtensions();

  private:

    double identify( GDALDatasetH dataset, int band, int px, int py );
};

void TestQgsGdalUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  GDALAllRegister();
}

void TestQgsGdalUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGdalUtils::init()
{

}

void TestQgsGdalUtils::cleanup()
{

}

void TestQgsGdalUtils::supportsRasterCreate()
{
  QVERIFY( QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "GTiff" ) ) );
  QVERIFY( QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "GPKG" ) ) );

  // special case
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "SQLite" ) ) );

  // create-only
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "DTED" ) ) );

  // vector-only
  QVERIFY( !QgsGdalUtils::supportsRasterCreate( GDALGetDriverByName( "ESRI Shapefile" ) ) );
}

#define EPSG_4326_WKT \
  "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]]," \
  "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433," \
  "AUTHORITY[\"EPSG\",\"9122\"]],AXIS[\"Latitude\",NORTH],AXIS[\"Longitude\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]"

void TestQgsGdalUtils::testCreateSingleBandMemoryDataset()
{
  const gdal::dataset_unique_ptr ds1 = QgsGdalUtils::createSingleBandMemoryDataset( GDT_Float32, QgsRectangle( 1, 1, 21, 11 ), 40, 20, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QVERIFY( ds1 );

  QCOMPARE( GDALGetRasterCount( ds1.get() ), 1 );
  QCOMPARE( GDALGetRasterXSize( ds1.get() ), 40 );
  QCOMPARE( GDALGetRasterYSize( ds1.get() ), 20 );

  QCOMPARE( GDALGetProjectionRef( ds1.get() ),  R"""(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]])""" );
  double geoTransform[6];
  double geoTransformExpected[] = { 1, 0.5, 0, 11, 0, -0.5 };
  QCOMPARE( GDALGetGeoTransform( ds1.get(), geoTransform ), CE_None );
  QVERIFY( memcmp( geoTransform, geoTransformExpected, sizeof( double ) * 6 ) == 0 );

  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 1 ) ), GDT_Float32 );
}

void TestQgsGdalUtils::testCreateMultiBandMemoryDataset()
{
  const gdal::dataset_unique_ptr ds1 = QgsGdalUtils::createMultiBandMemoryDataset( GDT_Float32, 4, QgsRectangle( 1, 1, 21, 11 ), 40, 20, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QVERIFY( ds1 );

  QCOMPARE( GDALGetRasterCount( ds1.get() ), 4 );
  QCOMPARE( GDALGetRasterXSize( ds1.get() ), 40 );
  QCOMPARE( GDALGetRasterYSize( ds1.get() ), 20 );

  QCOMPARE( GDALGetProjectionRef( ds1.get() ),  R"""(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]])""" );
  double geoTransform[6];
  double geoTransformExpected[] = { 1, 0.5, 0, 11, 0, -0.5 };
  QCOMPARE( GDALGetGeoTransform( ds1.get(), geoTransform ), CE_None );
  QVERIFY( memcmp( geoTransform, geoTransformExpected, sizeof( double ) * 6 ) == 0 );

  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 1 ) ), GDT_Float32 );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 2 ) ), GDT_Float32 );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 3 ) ), GDT_Float32 );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 4 ) ), GDT_Float32 );
}

void TestQgsGdalUtils::testCreateSingleBandTiffDataset()
{
  const QString filename = QDir::tempPath() + "/qgis_test_single_band_raster.tif";
  QFile::remove( filename );
  QVERIFY( !QFile::exists( filename ) );

  gdal::dataset_unique_ptr ds1 = QgsGdalUtils::createSingleBandTiffDataset( filename, GDT_Float32, QgsRectangle( 1, 1, 21, 11 ), 40, 20, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QVERIFY( ds1 );

  QCOMPARE( GDALGetRasterCount( ds1.get() ), 1 );
  QCOMPARE( GDALGetRasterXSize( ds1.get() ), 40 );
  QCOMPARE( GDALGetRasterYSize( ds1.get() ), 20 );

  QCOMPARE( GDALGetProjectionRef( ds1.get() ), R"""(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AXIS["Latitude",NORTH],AXIS["Longitude",EAST],AUTHORITY["EPSG","4326"]])""" );

  double geoTransform[6];
  double geoTransformExpected[] = { 1, 0.5, 0, 11, 0, -0.5 };
  QCOMPARE( GDALGetGeoTransform( ds1.get(), geoTransform ), CE_None );
  QVERIFY( memcmp( geoTransform, geoTransformExpected, sizeof( double ) * 6 ) == 0 );

  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( ds1.get(), 1 ) ), GDT_Float32 );

  ds1.reset();  // makes sure the file is fully written

  QVERIFY( QFile::exists( filename ) );

  std::unique_ptr<QgsRasterLayer> layer( new QgsRasterLayer( filename, "test", "gdal" ) );
  QVERIFY( layer->isValid() );
  QCOMPARE( layer->extent(), QgsRectangle( 1, 1, 21, 11 ) );
  QCOMPARE( layer->width(), 40 );
  QCOMPARE( layer->height(), 20 );

  layer.reset();  // let's clean up before removing the file
  QFile::remove( filename );
}

void TestQgsGdalUtils::testResampleSingleBandRaster()
{
  const QString inputFilename = QString( TEST_DATA_DIR ) + "/float1-16.tif";
  const gdal::dataset_unique_ptr srcDS( GDALOpen( inputFilename.toUtf8().constData(), GA_ReadOnly ) );
  QVERIFY( srcDS );

  const QString outputFilename = QDir::tempPath() + "/qgis_test_float1-16_resampled.tif";
  const QgsRectangle outputExtent( 106.25, -6.75, 106.55, -6.45 );
  gdal::dataset_unique_ptr dstDS = QgsGdalUtils::createSingleBandTiffDataset( outputFilename, GDT_Float32, outputExtent, 2, 2, QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  QVERIFY( dstDS );

  QgsGdalUtils::resampleSingleBandRaster( srcDS.get(), dstDS.get(), GRA_NearestNeighbour, nullptr );
  dstDS.reset();

  std::unique_ptr<QgsRasterLayer> layer( new QgsRasterLayer( outputFilename, "test", "gdal" ) );
  QVERIFY( layer );
  std::unique_ptr<QgsRasterBlock> block( layer->dataProvider()->block( 1, outputExtent, 2, 2 ) );
  QVERIFY( block );
  QCOMPARE( block->value( 0, 0 ), 6. );
  QCOMPARE( block->value( 1, 1 ), 11. );

  layer.reset();
  QFile::remove( outputFilename );
}

void TestQgsGdalUtils::testImageToDataset()
{
  const QString inputFilename = QString( TEST_DATA_DIR ) + "/rgb256x256.png";
  QImage src = QImage( inputFilename );
  src = src.convertToFormat( QImage::Format_ARGB32 );
  QVERIFY( !src.isNull() );

  gdal::dataset_unique_ptr dstDS = QgsGdalUtils::imageToMemoryDataset( QImage() );
  QVERIFY( !dstDS );

  dstDS = QgsGdalUtils::imageToMemoryDataset( src );
  QVERIFY( dstDS );

  QCOMPARE( GDALGetRasterCount( dstDS.get() ), 4 );
  QCOMPARE( GDALGetRasterXSize( dstDS.get() ), 256 );
  QCOMPARE( GDALGetRasterYSize( dstDS.get() ), 256 );

  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( dstDS.get(), 1 ) ), GDT_Byte );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( dstDS.get(), 2 ) ), GDT_Byte );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( dstDS.get(), 3 ) ), GDT_Byte );
  QCOMPARE( GDALGetRasterDataType( GDALGetRasterBand( dstDS.get(), 4 ) ), GDT_Byte );

  QCOMPARE( identify( dstDS.get(), 1, 50, 50 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 1, 200, 50 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 1, 50, 200 ), 0.0 );
  QCOMPARE( identify( dstDS.get(), 1, 200, 200 ), 0.0 );

  QCOMPARE( identify( dstDS.get(), 2, 50, 50 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 2, 200, 50 ), 0.0 );
  QCOMPARE( identify( dstDS.get(), 2, 50, 200 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 2, 200, 200 ), 0.0 );

  QCOMPARE( identify( dstDS.get(), 3, 50, 50 ), 0.0 );
  QCOMPARE( identify( dstDS.get(), 3, 200, 50 ), 0.0 );
  QCOMPARE( identify( dstDS.get(), 3, 50, 200 ), 0.0 );
  QCOMPARE( identify( dstDS.get(), 3, 200, 200 ), 255.0 );

  QCOMPARE( identify( dstDS.get(), 4, 50, 50 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 4, 200, 50 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 4, 50, 200 ), 255.0 );
  QCOMPARE( identify( dstDS.get(), 4, 200, 200 ), 255.0 );
}

void TestQgsGdalUtils::testResampleImageToImage()
{
  const QString inputFilename = QString( TEST_DATA_DIR ) + "/rgb256x256.png";
  QImage src = QImage( inputFilename );
  src = src.convertToFormat( QImage::Format_ARGB32 );
  QVERIFY( !src.isNull() );

  QImage res = QgsGdalUtils::resampleImage( QImage(), QSize( 50, 50 ), GRIORA_NearestNeighbour );
  QVERIFY( res.isNull() );

  res = QgsGdalUtils::resampleImage( src, QSize( 50, 50 ), GRIORA_NearestNeighbour );
  QVERIFY( !res.isNull() );
  QCOMPARE( res.width(), 50 );
  QCOMPARE( res.height(), 50 );

  QCOMPARE( qRed( res.pixel( 10, 10 ) ), 255 );
  QCOMPARE( qGreen( res.pixel( 10, 10 ) ), 255 );
  QCOMPARE( qBlue( res.pixel( 10, 10 ) ), 0 );
  QCOMPARE( qAlpha( res.pixel( 10, 10 ) ), 255 );

  QCOMPARE( qRed( res.pixel( 40, 10 ) ), 255 );
  QCOMPARE( qGreen( res.pixel( 40, 10 ) ), 0 );
  QCOMPARE( qBlue( res.pixel( 40, 10 ) ), 0 );
  QCOMPARE( qAlpha( res.pixel( 40, 10 ) ), 255 );

  QCOMPARE( qRed( res.pixel( 10, 40 ) ), 0 );
  QCOMPARE( qGreen( res.pixel( 10, 40 ) ), 255 );
  QCOMPARE( qBlue( res.pixel( 10, 40 ) ), 0 );
  QCOMPARE( qAlpha( res.pixel( 10, 40 ) ), 255 );

  QCOMPARE( qRed( res.pixel( 40, 40 ) ), 0 );
  QCOMPARE( qGreen( res.pixel( 40, 40 ) ), 0 );
  QCOMPARE( qBlue( res.pixel( 40, 40 ) ), 255 );
  QCOMPARE( qAlpha( res.pixel( 40, 40 ) ), 255 );
}

void TestQgsGdalUtils::testPathIsCheapToOpen()
{
  // should be safe and report false paths which don't exist
  QVERIFY( !QgsGdalUtils::pathIsCheapToOpen( "/not/a/file" ) );

  // for now, tiff aren't marked as cheap to open
  QVERIFY( !QgsGdalUtils::pathIsCheapToOpen( QStringLiteral( TEST_DATA_DIR ) + "/big_raster.tif" ) );

  // a csv is considered cheap to open
  QVERIFY( QgsGdalUtils::pathIsCheapToOpen( QStringLiteral( TEST_DATA_DIR ) + "/delimitedtext/test.csv" ) );

  // ... unless it's larger than the specified file size limit (500 bytes)
  QVERIFY( !QgsGdalUtils::pathIsCheapToOpen( QStringLiteral( TEST_DATA_DIR ) + "/delimitedtext/testdms.csv", 500 ) );
}

void TestQgsGdalUtils::testVrtMatchesLayerType()
{
  QVERIFY( QgsGdalUtils::vrtMatchesLayerType( QStringLiteral( TEST_DATA_DIR ) + "/raster/hub13263.vrt", QgsMapLayerType::RasterLayer ) );
  QVERIFY( !QgsGdalUtils::vrtMatchesLayerType( QStringLiteral( TEST_DATA_DIR ) + "/raster/hub13263.vrt", QgsMapLayerType::VectorLayer ) );

  QVERIFY( !QgsGdalUtils::vrtMatchesLayerType( QStringLiteral( TEST_DATA_DIR ) + "/vector_vrt.vrt", QgsMapLayerType::RasterLayer ) );
  QVERIFY( QgsGdalUtils::vrtMatchesLayerType( QStringLiteral( TEST_DATA_DIR ) + "/vector_vrt.vrt", QgsMapLayerType::VectorLayer ) );
}

void TestQgsGdalUtils::testMultilayerExtensions()
{
  const QStringList extensions = QgsGdalUtils::multiLayerFileExtensions();
  QVERIFY( extensions.contains( QStringLiteral( "gpkg" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "sqlite" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "db" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "kml" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "ods" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "osm" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "mdb" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "xls" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "xlsx" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "gpx" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "pdf" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "nc" ) ) );
  QVERIFY( extensions.contains( QStringLiteral( "gdb" ) ) );
}

double TestQgsGdalUtils::identify( GDALDatasetH dataset, int band, int px, int py )
{
  GDALRasterBandH hBand = GDALGetRasterBand( dataset, band );

  float *pafScanline = ( float * ) CPLMalloc( sizeof( float ) );
  const CPLErr err = GDALRasterIO( hBand, GF_Read, px, py, 1, 1,
                                   pafScanline, 1, 1, GDT_Float32, 0, 0 );
  const double value = err == CE_None ? pafScanline[0] : std::numeric_limits<double>::quiet_NaN();
  CPLFree( pafScanline );

  return value;
}

QGSTEST_MAIN( TestQgsGdalUtils )
#include "testqgsgdalutils.moc"
