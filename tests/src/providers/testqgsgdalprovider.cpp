/***************************************************************************
     testqgsgdalprovider.cpp
     --------------------------------------
    Date                 : March 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include <qgsrectangle.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsGdalProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void decodeUri(); // test decode URI implementation
    void scaleDataType(); //test resultant data types for int raster with float scale (#11573)
    void warpedVrt(); //test loading raster which requires a warped vrt
    void noData();
    void noDataOutsideExtent();
    void invalidNoDataInSourceIgnored();
    void isRepresentableValue();
    void mask();
    void bandName(); // test band name based on `gtiff` tags (#7317)
    void bandNameNoDescription(); // test band name for when no description or tags available (#16047)
    void bandNameWithDescription(); // test band name for when description available (#16047)
    void interactionBetweenRasterChangeAndCache(); // test that updading a raster invalidates the GDAL dataset cache (#20104)

  private:
    QString mTestDataDir;
    QString mReport;
};

//runs before all tests
void TestQgsGdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = QStringLiteral( "<h1>GDAL Provider Tests</h1>\n" );
}

//runs after all tests
void TestQgsGdalProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsGdalProvider::decodeUri()
{
  QString uri = QStringLiteral( "/home/to/path/raster.tif" );
  QVariantMap components;

  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components[QStringLiteral( "path" )].toString(), uri );

  uri = QStringLiteral( "gpkg:/home/to/path/my_file.gpkg:layer_name" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components[QStringLiteral( "path" )].toString(), QStringLiteral( "/home/to/path/my_file.gpkg" ) );
  QCOMPARE( components[QStringLiteral( "layerName" )].toString(), QStringLiteral( "layer_name" ) );

  //test windows path
  uri = QStringLiteral( "gpkg:c:/home/to/path/my_file.gpkg:layer_name" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components[QStringLiteral( "path" )].toString(), QStringLiteral( "c:/home/to/path/my_file.gpkg" ) );
  QCOMPARE( components[QStringLiteral( "layerName" )].toString(), QStringLiteral( "layer_name" ) );
}

void TestQgsGdalProvider::scaleDataType()
{
  QString rasterWithOffset = QStringLiteral( TEST_DATA_DIR ) + "/int_raster_with_scale.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), rasterWithOffset, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  //raster is an integer data type, but has a scale < 1, so data type must be float
  QCOMPARE( rp->dataType( 1 ), Qgis::Float32 );
  QCOMPARE( rp->sourceDataType( 1 ), Qgis::Float32 );
  delete provider;
}

void TestQgsGdalProvider::warpedVrt()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/requires_warped_vrt.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );

  qDebug() << "x min: " << rp->extent().xMinimum();
  qDebug() << "x max: " << rp->extent().xMaximum();
  qDebug() << "y min: " << rp->extent().yMinimum();
  qDebug() << "y max: " << rp->extent().yMaximum();

  QGSCOMPARENEAR( rp->extent().xMinimum(), 2058589, 1 );
  QGSCOMPARENEAR( rp->extent().xMaximum(), 3118999, 1 );
  QGSCOMPARENEAR( rp->extent().yMinimum(), 2281355, 1 );
  QGSCOMPARENEAR( rp->extent().yMaximum(), 3129683, 1 );
  delete provider;
}

void TestQgsGdalProvider::noData()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->sourceNoDataValue( 1 ), static_cast<double>( 255 ) );
  }
  delete provider;
}

void TestQgsGdalProvider::noDataOutsideExtent()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    std::unique_ptr<QgsRasterBlock> block( rp->block( 1, QgsRectangle( 10, 10, 12, 12 ), 16, 16 ) );
    QVERIFY( block );
    QCOMPARE( block->width(), 16 );
    QCOMPARE( block->height(), 16 );
    for ( int y = 0; y < 16; ++y )
    {
      for ( int x = 0; x < 16; ++x )
      {
        QVERIFY( block->isNoData( y, x ) );
      }
    }
  }
  delete provider;
}

void TestQgsGdalProvider::invalidNoDataInSourceIgnored()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/byte_with_nan_nodata.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->sourceHasNoDataValue( 1 ), false );
  }
  delete provider;
}

void TestQgsGdalProvider::isRepresentableValue()
{
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 255., Qgis::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 256., Qgis::Byte ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65535., Qgis::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65536., Qgis::UInt16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32769., Qgis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32768., Qgis::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32767., Qgis::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32768., Qgis::Int16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967295., Qgis::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., Qgis::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483649., Qgis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483648., Qgis::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483647., Qgis::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483648., Qgis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., Qgis::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), Qgis::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), Qgis::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<float>::max(), Qgis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<float>::max(), Qgis::Float32 ), true );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), Qgis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), Qgis::Float64 ), true );
}

void TestQgsGdalProvider::mask()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/rgb_with_mask.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->bandCount(), 4 );
    QCOMPARE( rp->dataType( 4 ), Qgis::Byte );
    QCOMPARE( rp->sourceDataType( 4 ), Qgis::Byte );
    QCOMPARE( rp->colorInterpretation( 4 ), static_cast<int>( QgsRaster::AlphaBand ) );
    QCOMPARE( rp->bandScale( 4 ), 1.0 );
    QCOMPARE( rp->bandOffset( 4 ), 0.0 );
    QgsRectangle rect( 0, 0, 162, 150 );
    QgsRasterBlock *block = rp->block( 4, rect, 162, 150 );
    QVERIFY( block );
    delete block;
  }
  delete provider;
}

void TestQgsGdalProvider::bandName()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/gtiff_tags.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1: wvln=1.234 (um)" ) );
  delete provider;
}

void TestQgsGdalProvider::bandNameNoDescription()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1" ) );
  delete provider;
}

void TestQgsGdalProvider::bandNameWithDescription()
{
  QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/gtiff_desc.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1: 1.234 um" ) );
  delete provider;
}

void TestQgsGdalProvider::interactionBetweenRasterChangeAndCache()
{
  double geoTransform[6] = { 0, 2, 0, 0, 0, -2};
  QgsCoordinateReferenceSystem crs;
  QString filename = QStringLiteral( "/vsimem/temp.tif" );

  // Create a all-0 dataset
  auto provider = QgsRasterDataProvider::create(
                    QStringLiteral( "gdal" ), filename, "GTiff", 1, Qgis::Byte, 1, 1, geoTransform, crs );
  delete provider;

  // Open it
  provider = dynamic_cast< QgsRasterDataProvider * >(
               QgsProviderRegistry::instance()->createProvider(
                 QStringLiteral( "gdal" ), filename, QgsDataProvider::ProviderOptions() ) );
  QVERIFY( provider );
  auto rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );

  // Create a first clone, and destroys it
  auto rpClone = dynamic_cast< QgsRasterDataProvider *>( rp->clone() );
  QVERIFY( rpClone );
  QCOMPARE( rpClone->sample( QgsPointXY( 0.5, -0.5 ), 1 ), 0.0 );
  delete rpClone;
  // Now the main provider should have a cached GDAL dataset corresponding
  // to the one that was used by rpClone

  // Modify the raster
  rp->setEditable( true );
  auto rblock = new QgsRasterBlock( Qgis::Byte, 1, 1 );
  rblock->setValue( 0, 0, 255 );
  rp->writeBlock( rblock, 1, 0, 0 );
  delete rblock;
  rp->setEditable( false );

  // Creates a new clone, and check that we get an updated sample value
  rpClone = dynamic_cast< QgsRasterDataProvider *>( rp->clone() );
  QVERIFY( rpClone );
  QCOMPARE( rpClone->sample( QgsPointXY( 0.5, -0.5 ), 1 ), 255.0 );
  delete rpClone;

  provider->remove();
  delete provider;
}

QGSTEST_MAIN( TestQgsGdalProvider )
#include "testqgsgdalprovider.moc"
