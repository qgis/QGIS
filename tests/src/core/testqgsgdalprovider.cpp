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

// GDAL includes
#include <gdal.h>
#include <cpl_conv.h>

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
#include "qgspoint.h"
#include "qgsprovidermetadata.h"
#include "qgsprovidersublayerdetails.h"
#include "qgsrasterlayer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsGdalProvider : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsGdalProvider() : QgsTest( QStringLiteral( "GDAL Provider Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void decodeUri(); // test decode URI implementation
    void encodeUri(); // test encode URI implementation
    void scaleDataType(); //test resultant data types for int raster with float scale (#11573)
    void warpedVrt(); //test loading raster which requires a warped vrt
    void testVrtAlphaBandRequired();
    void testVrtAlphaBandNotRequired();
    void noData();
    void noDataOutsideExtent();
    void invalidNoDataInSourceIgnored();
    void isRepresentableValue();
    void mask();
    void bandName(); // test band name based on `gtiff` tags (#7317)
    void bandNameNoDescription(); // test band name for when no description or tags available (#16047)
    void bandNameWithDescription(); // test band name for when description available (#16047)
    void colorTable();
    void interactionBetweenRasterChangeAndCache(); // test that updading a raster invalidates the GDAL dataset cache (#20104)
    void scale0(); //test when data has scale 0 (#20493)
    void transformCoordinates();
    void testGdalProviderQuerySublayers();
    void testGdalProviderQuerySublayers_NetCDF();
    void testGdalProviderQuerySublayersFastScan();
    void testGdalProviderQuerySublayersFastScan_NetCDF();
    void testGdalProviderAbsoluteRelativeUri();
    void testVsiCredentialOptions();
    void testVsiCredentialOptionsQuerySublayers();

  private:
    QString mTestDataDir;
    bool mSupportsNetCDF;
    QgsProviderMetadata *mGdalMetadata;

};

//runs before all tests
void TestQgsGdalProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  mGdalMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "gdal" ) );

  mSupportsNetCDF = static_cast< bool >( GDALGetDriverByName( "netcdf" ) );

  // Disable creation of .aux.xml (stats) files during test run,
  // to avoid modifying .zip files.
  // See https://github.com/qgis/QGIS/issues/48846
  CPLSetConfigOption( "GDAL_PAM_ENABLED", "NO" );

}

//runs after all tests
void TestQgsGdalProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
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

  uri = QStringLiteral( "/vsizip//home/to/path/file.zip/my.tif" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components[QStringLiteral( "path" )].toString(), QStringLiteral( "/home/to/path/file.zip" ) );
  QCOMPARE( components[QStringLiteral( "vsiPrefix" )].toString(), QStringLiteral( "/vsizip/" ) );
  QCOMPARE( components[QStringLiteral( "vsiSuffix" )].toString(), QStringLiteral( "/my.tif" ) );

  //test windows path
  uri = QStringLiteral( "gpkg:c:/home/to/path/my_file.gpkg:layer_name" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components[QStringLiteral( "path" )].toString(), QStringLiteral( "c:/home/to/path/my_file.gpkg" ) );
  QCOMPARE( components[QStringLiteral( "layerName" )].toString(), QStringLiteral( "layer_name" ) );

  // test authcfg with vsicurl URI
  uri = QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.tif authcfg='1234567'" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components.value( QStringLiteral( "path" ) ).toString(), QString( "https://www.qgis.org/dataset.tif" ) );
  QCOMPARE( components.value( QStringLiteral( "vsiPrefix" ) ).toString(), QString( "/vsicurl/" ) );
  QCOMPARE( components.value( QStringLiteral( "authcfg" ) ).toString(), QString( "1234567" ) );

  // vsis3
  uri = QStringLiteral( "/vsis3/nz-elevation/auckland/auckland-north_2016-2018/dem_1m/2193/AY30_10000_0405.tiff" );
  components = QgsProviderRegistry::instance()->decodeUri( QStringLiteral( "gdal" ), uri );
  QCOMPARE( components.value( QStringLiteral( "path" ) ).toString(), QString( "nz-elevation/auckland/auckland-north_2016-2018/dem_1m/2193/AY30_10000_0405.tiff" ) );
  QCOMPARE( components.value( QStringLiteral( "vsiPrefix" ) ).toString(), QString( "/vsis3/" ) );
}

void TestQgsGdalProvider::encodeUri()
{
  QVariantMap parts;
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/user/test.gpkg" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "/home/user/test.gpkg" ) );

  parts.insert( QStringLiteral( "layerName" ), QStringLiteral( "layername" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "GPKG:/home/user/test.gpkg:layername" ) );

  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/home/user/test.zip" ) );
  parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsizip/" ) );
  parts.insert( QStringLiteral( "vsiSuffix" ), QStringLiteral( "/my.tif" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "/vsizip//home/user/test.zip/my.tif" ) );

  // test authcfg with vsicurl
  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.tif" ) );
  parts.insert( QStringLiteral( "authcfg" ), QStringLiteral( "1234567" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.tif authcfg='1234567'" ) );
  parts.clear();
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "https://www.qgis.org/dataset.tif" ) );
  parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsicurl/" ) );
  parts.insert( QStringLiteral( "authcfg" ), QStringLiteral( "1234567" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "/vsicurl/https://www.qgis.org/dataset.tif authcfg='1234567'" ) );

  // vsis3
  parts.clear();
  parts.insert( QStringLiteral( "vsiPrefix" ), QStringLiteral( "/vsis3/" ) );
  parts.insert( QStringLiteral( "path" ), QStringLiteral( "nz-elevation/auckland/auckland-north_2016-2018/dem_1m/2193/AY30_10000_0405.tiff" ) );
  QCOMPARE( QgsProviderRegistry::instance()->encodeUri( QStringLiteral( "gdal" ), parts ), QStringLiteral( "/vsis3/nz-elevation/auckland/auckland-north_2016-2018/dem_1m/2193/AY30_10000_0405.tiff" ) );
}

void TestQgsGdalProvider::scaleDataType()
{
  const QString rasterWithOffset = QStringLiteral( TEST_DATA_DIR ) + "/int_raster_with_scale.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), rasterWithOffset, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  //raster is an integer data type, but has a scale < 1, so data type must be float
  QCOMPARE( rp->dataType( 1 ), Qgis::DataType::Float32 );
  QCOMPARE( rp->sourceDataType( 1 ), Qgis::DataType::Float32 );
  delete provider;
}

void TestQgsGdalProvider::warpedVrt()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/requires_warped_vrt.tif";
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

void TestQgsGdalProvider::testVrtAlphaBandRequired()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/rotated_rgb.png";
  std::unique_ptr< QgsDataProvider > provider( QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() ) );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider.get() );
  QVERIFY( rp );

  QCOMPARE( rp->bandCount(), 4 );
  QCOMPARE( rp->colorInterpretation( 1 ), Qgis::RasterColorInterpretation::RedBand );
  QCOMPARE( rp->colorInterpretation( 2 ), Qgis::RasterColorInterpretation::GreenBand );
  QCOMPARE( rp->colorInterpretation( 3 ), Qgis::RasterColorInterpretation::BlueBand );
  QCOMPARE( rp->colorInterpretation( 4 ), Qgis::RasterColorInterpretation::AlphaBand );
}

void TestQgsGdalProvider::testVrtAlphaBandNotRequired()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/72_528t50dgm.txt";
  std::unique_ptr< QgsDataProvider > provider( QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() ) );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider.get() );
  QVERIFY( rp );

  QGSCOMPARENEAR( rp->extent().xMinimum(), 719975, 0.0001 );
  QGSCOMPARENEAR( rp->extent().xMaximum(), 720075, 0.0001 );
  QGSCOMPARENEAR( rp->extent().yMinimum(), 5279975, 0.0001 );
  QGSCOMPARENEAR( rp->extent().yMaximum(), 5280075, 0.0001 );
  QCOMPARE( rp->bandCount(), 1 );
  QCOMPARE( rp->colorInterpretation( 1 ), Qgis::RasterColorInterpretation::Undefined );
}

void TestQgsGdalProvider::noData()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
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
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
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
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/byte_with_nan_nodata.tif";
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
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::DataType::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::DataType::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 255., Qgis::DataType::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 256., Qgis::DataType::Byte ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::DataType::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::DataType::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65535., Qgis::DataType::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65536., Qgis::DataType::UInt16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32769., Qgis::DataType::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32768., Qgis::DataType::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32767., Qgis::DataType::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32768., Qgis::DataType::Int16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., Qgis::DataType::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., Qgis::DataType::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967295., Qgis::DataType::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., Qgis::DataType::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483649., Qgis::DataType::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483648., Qgis::DataType::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483647., Qgis::DataType::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483648., Qgis::DataType::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., Qgis::DataType::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), Qgis::DataType::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), Qgis::DataType::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<float>::max(), Qgis::DataType::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<float>::max(), Qgis::DataType::Float32 ), true );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), Qgis::DataType::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), Qgis::DataType::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), Qgis::DataType::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), Qgis::DataType::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), Qgis::DataType::Float64 ), true );
}

void TestQgsGdalProvider::mask()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/rgb_with_mask.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->bandCount(), 4 );
    QCOMPARE( rp->dataType( 4 ), Qgis::DataType::Byte );
    QCOMPARE( rp->sourceDataType( 4 ), Qgis::DataType::Byte );
    QCOMPARE( rp->colorInterpretation( 4 ), Qgis::RasterColorInterpretation::AlphaBand );
    QCOMPARE( rp->bandScale( 4 ), 1.0 );
    QCOMPARE( rp->bandOffset( 4 ), 0.0 );
    const QgsRectangle rect( 0, 0, 162, 150 );
    QgsRasterBlock *block = rp->block( 4, rect, 162, 150 );
    QVERIFY( block );
    delete block;
  }
  delete provider;
}

void TestQgsGdalProvider::bandName()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/gtiff_tags.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1: wvln=1.234 (um)" ) );
  delete provider;
}

void TestQgsGdalProvider::bandNameNoDescription()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1" ) );
  delete provider;
}

void TestQgsGdalProvider::bandNameWithDescription()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/gtiff_desc.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->generateBandName( 1 ), QStringLiteral( "Band 1: 1.234 um" ) );
  delete provider;
}

void TestQgsGdalProvider::colorTable()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->colorTable( 1 ).size(), 256 );
  // invalid band
  QVERIFY( rp->colorTable( 2 ).isEmpty() );
  delete provider;
}

void TestQgsGdalProvider::interactionBetweenRasterChangeAndCache()
{
  double geoTransform[6] = { 0, 2, 0, 0, 0, -2};
  const QgsCoordinateReferenceSystem crs;
  const QString filename = QStringLiteral( "/vsimem/temp.tif" );

  // Create a all-0 dataset
  auto provider = QgsRasterDataProvider::create(
                    QStringLiteral( "gdal" ), filename, "GTiff", 1, Qgis::DataType::Byte, 1, 1, geoTransform, crs );
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
  auto rblock = new QgsRasterBlock( Qgis::DataType::Byte, 1, 1 );
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

void TestQgsGdalProvider::scale0()
{
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/raster/scale0ingdal23.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QCOMPARE( rp->bandScale( 1 ), 1.0 );
  QCOMPARE( rp->bandOffset( 1 ), 0.0 );
  delete provider;
}

void TestQgsGdalProvider::transformCoordinates()
{
  // Test implementation of QgsRasterDataProvider::transformCoordinates()
  const QString raster = QStringLiteral( TEST_DATA_DIR ) + "/float1-16.tif";
  QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "gdal" ), raster, QgsDataProvider::ProviderOptions() );
  QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
  QVERIFY( rp );
  QVERIFY( rp->isValid() );

  // forward transform - image coordinates to georeferenced coordinates
  const QgsPoint pt1Layer = rp->transformCoordinates( QgsPoint( 0, 0 ), QgsRasterDataProvider::TransformImageToLayer ); // bottom-left corner
  const QgsPoint pt2Layer = rp->transformCoordinates( QgsPoint( 4, 0 ), QgsRasterDataProvider::TransformImageToLayer );
  const QgsPoint pt3Layer = rp->transformCoordinates( QgsPoint( 4, 4 ), QgsRasterDataProvider::TransformImageToLayer ); // top-right corner

  QCOMPARE( pt1Layer, QgsPoint( 106.0, -7.0, 0 ) );
  QCOMPARE( pt2Layer, QgsPoint( 106.8, -7.0, 0 ) );
  QCOMPARE( pt3Layer, QgsPoint( 106.8, -6.2, 0 ) );

  // inverse transform - georeferenced coordinates to image coordinates
  const QgsPoint pt1Image = rp->transformCoordinates( QgsPoint( 106.0, -7.0 ), QgsRasterDataProvider::TransformLayerToImage ); // bottom-left corner
  const QgsPoint pt2Image = rp->transformCoordinates( QgsPoint( 106.8, -7.0 ), QgsRasterDataProvider::TransformLayerToImage );
  const QgsPoint pt3Image = rp->transformCoordinates( QgsPoint( 106.8, -6.2 ), QgsRasterDataProvider::TransformLayerToImage ); // top-right corner

  QCOMPARE( pt1Image, QgsPoint( 0, 0, 0 ) );
  QCOMPARE( pt2Image, QgsPoint( 4, 0, 0 ) );
  QCOMPARE( pt3Image, QgsPoint( 4, 4, 0 ) );

}

void TestQgsGdalProvider::testGdalProviderQuerySublayers()
{
  // invalid uri
  QList< QgsProviderSublayerDetails >res = mGdalMetadata->querySublayers( QString() );
  QVERIFY( res.empty() );

  // not a raster
  res = mGdalMetadata->querySublayers( QString( TEST_DATA_DIR ) + "/lines.shp" );
  QVERIFY( res.empty() );

  // single layer raster
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GTiff" ) );

  // make sure result is valid to load layer from
  const QgsProviderSublayerDetails::LayerOptions options{ QgsCoordinateTransformContext() };
  std::unique_ptr< QgsRasterLayer > rl( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // geopackage with two raster layers
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/mixed_layers.gpkg" );
  QCOMPARE( res.count(), 2 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "band1" ) );
  QCOMPARE( res.at( 0 ).description(), QStringLiteral( "band1" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "GPKG:%1/mixed_layers.gpkg:band1" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GPKG" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 1 ).layerNumber(), 2 );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "band2" ) );
  QCOMPARE( res.at( 1 ).description(), QStringLiteral( "band2" ) );
  QCOMPARE( res.at( 1 ).uri(), QStringLiteral( "GPKG:%1/mixed_layers.gpkg:band2" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 1 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 1 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 1 ).driverName(), QStringLiteral( "GPKG" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 1 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  // geopackage with one raster layer with an identifier
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/qgis_server/test_project_wms_grouped_layers.gpkg" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "osm" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "%1/qgis_server/test_project_wms_grouped_layers.gpkg" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GPKG" ) );

  // aigrid file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/aigrid" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "aigrid" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "%1/aigrid" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "AIG" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // aigrid, pointing to .adf file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/aigrid/hdr.adf" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "aigrid" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "%1/aigrid/hdr.adf" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "AIG" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // zip archive, only 1 file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/zip/landsat_b1.zip" );
  QCOMPARE( res.count(), 1 );
  const QgsProviderSublayerDetails &sl = res.at( 0 );
  QCOMPARE( sl.layerNumber(), 1 );
  QCOMPARE( sl.name(), QStringLiteral( "landsat_b1.tif" ) );
  QCOMPARE( sl.description(), QString() );
  QCOMPARE( sl.uri(), QStringLiteral( "/vsizip/%1/zip/landsat_b1.zip/landsat_b1.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( sl.providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( sl.type(), Qgis::LayerType::Raster );
  QCOMPARE( sl.driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( sl.toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // multi-layer archive
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz" );
  QCOMPARE( res.count(), 3 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "folder/folder2/landsat_b2.tif" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/folder/folder2/landsat_b2.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 1 ).layerNumber(), 1 );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "landsat_b1.tif" ) );
  QCOMPARE( res.at( 1 ).description(), QString() );
  QCOMPARE( res.at( 1 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/landsat_b1.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 1 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 1 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 1 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 1 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 2 ).layerNumber(), 1 );
  QCOMPARE( res.at( 2 ).name(), QStringLiteral( "landsat_b1.vrt" ) );
  QCOMPARE( res.at( 2 ).description(), QString() );
  QCOMPARE( res.at( 2 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/landsat_b1.vrt" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 2 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 2 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 2 ).driverName(), QStringLiteral( "VRT" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 2 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // multi-layer archive, but with specific suffix specified
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/folder/folder2/landsat_b2.tif" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "folder/folder2/landsat_b2.tif" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/folder/folder2/landsat_b2.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/landsat_b1.tif" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat_b1.tif" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/landsat_b1.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsizip/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testzip.zip/landsat_b1.vrt" );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat_b1.vrt" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsizip/%1/zip/testzip.zip/landsat_b1.vrt" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "VRT" ) );

  // multi-layer archive, format not supported by gdal
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/points.qml" );
  QCOMPARE( res.count(), 0 );

  // metadata.xml file next to tdenv?.adf file -- this is a subcomponent of an ESRI tin layer, should not be exposed
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/esri_tin/metadata.xml" );
  QVERIFY( res.empty() );

  // SAFE format zip file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/zip/S2A_MSIL2A_0000.zip" );
  QCOMPARE( res.count(), 4 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "SENTINEL2_L2A:/vsizip/%1/zip/S2A_MSIL2A_0000.zip/S2A_MSIL2A_0000.SAFE/MTD_MSIL2A.xml:10m:EPSG_32634" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).description(), QString( "Bands B2, B3, B4, B8 with 10m resolution, UTM 34N" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "SENTINEL2_L2A:/vsizip/%1/zip/S2A_MSIL2A_0000.zip/S2A_MSIL2A_0000.SAFE/MTD_MSIL2A.xml:10m:EPSG_32634" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "SENTINEL2" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // tiff with two raster layers and TIFF Tags describing sublayers
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/raster/gtiff_subdataset_tags.tif" );
  QCOMPARE( res.count(), 2 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "Test Document Name 1" ) );
  QCOMPARE( res.at( 0 ).description(), QStringLiteral( "Test Image Description 1" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "GTIFF_DIR:1:%1/raster/gtiff_subdataset_tags.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 1 ).layerNumber(), 2 );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "Test Document Name 2" ) );
  QCOMPARE( res.at( 1 ).description(), QStringLiteral( "Test Image Description 2" ) );
  QCOMPARE( res.at( 1 ).uri(), QStringLiteral( "GTIFF_DIR:2:%1/raster/gtiff_subdataset_tags.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 1 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 1 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 1 ).driverName(), QStringLiteral( "GTiff" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 1 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
}

void TestQgsGdalProvider::testGdalProviderQuerySublayers_NetCDF()
{
  if ( ! mSupportsNetCDF )
  {
    QSKIP( "NetCDF based tests require the netcdf GDAL driver" );
  }

  QList< QgsProviderSublayerDetails > res;
  std::unique_ptr< QgsRasterLayer > rl;
  const QgsProviderSublayerDetails::LayerOptions options{ QgsCoordinateTransformContext() };

  // netcdf file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc" );
  QCOMPARE( res.count(), 8 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "cell_node" ) );
  QCOMPARE( res.at( 0 ).description(), QStringLiteral( "[320x4] cell_node (32-bit integer)" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "NETCDF:\"%1/mesh/trap_steady_05_3D.nc\":cell_node" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "netCDF" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 1 ).layerNumber(), 2 );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "layerface_Z" ) );
  QCOMPARE( res.at( 1 ).description(), QStringLiteral( "[37x3520] layerface_Z (32-bit floating-point)" ) );
  QCOMPARE( res.at( 1 ).uri(), QStringLiteral( "NETCDF:\"%1/mesh/trap_steady_05_3D.nc\":layerface_Z" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 1 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 1 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 1 ).driverName(), QStringLiteral( "netCDF" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 1 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );

  // netcdf with open options
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc|option:HONOUR_VALID_RANGE=YES" );
  QCOMPARE( res.count(), 8 );
  QCOMPARE( res.at( 0 ).layerNumber(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "cell_node" ) );
  QCOMPARE( res.at( 0 ).description(), QStringLiteral( "[320x4] cell_node (32-bit integer)" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "NETCDF:\"%1/mesh/trap_steady_05_3D.nc\":cell_node|option:HONOUR_VALID_RANGE=YES" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 0 ).driverName(), QStringLiteral( "netCDF" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 0 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
  QCOMPARE( res.at( 1 ).layerNumber(), 2 );
  QCOMPARE( res.at( 1 ).name(), QStringLiteral( "layerface_Z" ) );
  QCOMPARE( res.at( 1 ).description(), QStringLiteral( "[37x3520] layerface_Z (32-bit floating-point)" ) );
  QCOMPARE( res.at( 1 ).uri(), QStringLiteral( "NETCDF:\"%1/mesh/trap_steady_05_3D.nc\":layerface_Z|option:HONOUR_VALID_RANGE=YES" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 1 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 1 ).type(), Qgis::LayerType::Raster );
  QCOMPARE( res.at( 1 ).driverName(), QStringLiteral( "netCDF" ) );
  rl.reset( qgis::down_cast< QgsRasterLayer * >( res.at( 1 ).toLayer( options ) ) );
  QVERIFY( rl->isValid() );
}

void TestQgsGdalProvider::testGdalProviderQuerySublayersFastScan()
{
  // invalid uri
  QList< QgsProviderSublayerDetails >res = mGdalMetadata->querySublayers( QString(), Qgis::SublayerQueryFlag::FastScan );
  QVERIFY( res.empty() );

  // not a raster
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/lines.shp", Qgis::SublayerQueryFlag::FastScan );
  QVERIFY( res.empty() );

  // single layer raster
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !res.at( 0 ).skippedContainerScan() );

  // geopackage with two raster layers
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/mixed_layers.gpkg", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "mixed_layers" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/mixed_layers.gpkg" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( res.at( 0 ).skippedContainerScan() );

  // aigrid, pointing to .adf file
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/aigrid/hdr.adf", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "aigrid" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "%1/aigrid/hdr.adf" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( !res.at( 0 ).skippedContainerScan() );

  // vector vrt
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/vector_vrt.vrt", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 0 );

  // raster vrt
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/raster/hub13263.vrt", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "hub13263" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/raster/hub13263.vrt" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( res.at( 0 ).skippedContainerScan() );

  // metadata.xml file next to tdenv?.adf file -- this is a subcomponent of an ESRI tin layer, should not be exposed
  res = mGdalMetadata->querySublayers( QStringLiteral( TEST_DATA_DIR ) + "/esri_tin/metadata.xml", Qgis::SublayerQueryFlag::FastScan );
  QVERIFY( res.empty() );

  // multi-layer archive, but with specific suffix specified
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/folder/folder2/landsat_b2.tif", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat_b2.tif" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/folder/folder2/landsat_b2.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/landsat_b1.tif", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat_b1.tif" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsitar/%1/zip/testtar.tgz/landsat_b1.tif" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );

  // multi-layer archive, format not supported by gdal
  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsitar/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testtar.tgz/points.qml", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 0 );

  res = mGdalMetadata->querySublayers( QStringLiteral( "/vsizip/" ) + QStringLiteral( TEST_DATA_DIR ) + "/zip/testzip.zip/landsat_b1.vrt", Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).layerNumber(), 0 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "landsat_b1.vrt" ) );
  QCOMPARE( res.at( 0 ).description(), QString() );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( "/vsizip/%1/zip/testzip.zip/landsat_b1.vrt" ).arg( QStringLiteral( TEST_DATA_DIR ) ) );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
}

void TestQgsGdalProvider::testGdalProviderQuerySublayersFastScan_NetCDF()
{
  if ( ! mSupportsNetCDF )
  {
    QSKIP( "NetCDF based tests require the netcdf GDAL driver" );
  }

  QList< QgsProviderSublayerDetails > res;
  std::unique_ptr< QgsRasterLayer > rl;

  // netcdf file
  res = mGdalMetadata->querySublayers(
          QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc",
          Qgis::SublayerQueryFlag::FastScan
        );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "trap_steady_05_3D" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( res.at( 0 ).skippedContainerScan() );

  // netcdf with open options
  res = mGdalMetadata->querySublayers(
          QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc|option:HONOUR_VALID_RANGE=YES",
          Qgis::SublayerQueryFlag::FastScan
        );
  QCOMPARE( res.count(), 1 );
  QCOMPARE( res.at( 0 ).name(), QStringLiteral( "trap_steady_05_3D" ) );
  QCOMPARE( res.at( 0 ).uri(), QStringLiteral( TEST_DATA_DIR ) + "/mesh/trap_steady_05_3D.nc|option:HONOUR_VALID_RANGE=YES" );
  QCOMPARE( res.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( res.at( 0 ).type(), Qgis::LayerType::Raster );
  QVERIFY( res.at( 0 ).skippedContainerScan() );

}

void TestQgsGdalProvider::testGdalProviderAbsoluteRelativeUri()
{
  QgsReadWriteContext context;
  context.setPathResolver( QgsPathResolver( QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/project.qgs" ) ) );

  QString absoluteUri = QStringLiteral( TEST_DATA_DIR ) + QStringLiteral( "/landsat.tif" );
  QString relativeUri = QStringLiteral( "./landsat.tif" );
  QCOMPARE( mGdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mGdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = QStringLiteral( "GPKG:%1/mixed_layers.gpkg:band1" ).arg( TEST_DATA_DIR );
  relativeUri = QStringLiteral( "GPKG:./mixed_layers.gpkg:band1" );
  QCOMPARE( mGdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mGdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );

  absoluteUri = QStringLiteral( "NETCDF:\"%1/landsat.nc\":Band1" ).arg( TEST_DATA_DIR );
  relativeUri = QStringLiteral( "NETCDF:\"./landsat.nc\":Band1" );
  QCOMPARE( mGdalMetadata->absoluteToRelativeUri( absoluteUri, context ), relativeUri );
  QCOMPARE( mGdalMetadata->relativeToAbsoluteUri( relativeUri, context ), absoluteUri );
}

void TestQgsGdalProvider::testVsiCredentialOptions()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 6, 0)
  // test that credential options are correctly set when layer URI specifies them

  // if actual aws dataset proves flaky, use this instead:
  // std::unique_ptr< QgsRasterLayer > rl = std::make_unique< QgsRasterLayer >( QStringLiteral( "/vsis3/testbucket/test|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-1|credential:AWS_S3_ENDPOINT=localhost" ), QStringLiteral( "test" ), QStringLiteral( "gdal" ) );
  std::unique_ptr< QgsRasterLayer > rl = std::make_unique< QgsRasterLayer >( QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ), QStringLiteral( "test" ), QStringLiteral( "gdal" ) );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );
  QString region( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( rl->dataProvider()->dataSourceUri(), QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ) );

  // different bucket
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  // credentials should be bucket specific
  std::unique_ptr< QgsRasterLayer > rl2 = std::make_unique< QgsRasterLayer >( QStringLiteral( "/vsis3/another/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost" ), QStringLiteral( "test" ), QStringLiteral( "gdal" ) );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );
  region = QString( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/another/subfolder/subfolder2", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "NO" ) );
  region = QString( VSIGetPathSpecificOption( "/vsis3/another/subfolder/subfolder2", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QStringLiteral( "eu-central-2" ) );
  noSign = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QString() );
  region = QString( VSIGetPathSpecificOption( "/vsis3/another", "AWS_REGION", nullptr ) );
  QCOMPARE( region, QString() );

  QCOMPARE( rl2->dataProvider()->dataSourceUri(), QStringLiteral( "/vsis3/another/subfolder/subfolder2/test|credential:AWS_NO_SIGN_REQUEST=NO|credential:AWS_REGION=eu-central-2|credential:AWS_S3_ENDPOINT=localhost" ) );

  // cleanup
  VSIClearPathSpecificOptions( "/vsis3/cdn.proj.org" );
  VSIClearPathSpecificOptions( "/vsis3/another/subfolder/subfolder2" );
#endif
}

void TestQgsGdalProvider::testVsiCredentialOptionsQuerySublayers()
{
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 6, 0)
  QgsProviderMetadata *gdalMetadata = QgsProviderRegistry::instance()->providerMetadata( "gdal" );
  QVERIFY( gdalMetadata );

  // test that credential options are correctly handled when querying sublayers

  // if actual aws dataset proves flaky, use this instead:
  //QList< QgsProviderSublayerDetails> subLayers = gdalMetadata->querySublayers( QStringLiteral( "/vsis3/gdalsublayerstestbucket/test.tif|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost" ) );
  QList< QgsProviderSublayerDetails> subLayers = gdalMetadata->querySublayers( QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ) );
  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), QStringLiteral( "us_nga_egm96_15" ) );
  QCOMPARE( subLayers.at( 0 ).uri(), QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ) );
  QCOMPARE( subLayers.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Raster );

  // confirm that GDAL VSI configuration options are set
  QString noSign( VSIGetPathSpecificOption( "/vsis3/cdn.proj.org", "AWS_NO_SIGN_REQUEST", nullptr ) );
  QCOMPARE( noSign, QStringLiteral( "YES" ) );

  // subLayers = gdalMetadata->querySublayers( QStringLiteral( "/vsis3/gdalsublayerstestbucket/test.tif|credential:AWS_NO_SIGN_REQUEST=YES|credential:AWS_REGION=eu-central-3|credential:AWS_S3_ENDPOINT=localhost" ), Qgis::SublayerQueryFlag::FastScan );
  subLayers = gdalMetadata->querySublayers( QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ), Qgis::SublayerQueryFlag::FastScan );
  QCOMPARE( subLayers.size(), 1 );
  QCOMPARE( subLayers.at( 0 ).name(), QStringLiteral( "us_nga_egm96_15" ) );
  QCOMPARE( subLayers.at( 0 ).uri(), QStringLiteral( "/vsis3/cdn.proj.org/us_nga_egm96_15.tif|credential:AWS_NO_SIGN_REQUEST=YES" ) );
  QCOMPARE( subLayers.at( 0 ).providerKey(), QStringLiteral( "gdal" ) );
  QCOMPARE( subLayers.at( 0 ).type(), Qgis::LayerType::Raster );

  // cleanup
  VSIClearPathSpecificOptions( "/vsis3/cdn.proj.org" );
#endif
}

QGSTEST_MAIN( TestQgsGdalProvider )
#include "testqgsgdalprovider.moc"
