/***************************************************************************
     testqgsrasterblock.cpp
     --------------------------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder.sk at gmail.com
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
#include <QTemporaryFile>
#include <QLocale>

#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsRasterBlock class.
 */
class TestQgsRasterBlock : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsRasterBlock()
      : QgsTest( QStringLiteral( "Raster block" ) )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testBasic();
    void testWrite();
    void testPrintValueFloat_data();
    void testPrintValueFloat();
    void testPrintValueDouble_data();
    void testPrintValueDouble();
    void testMinimumMaximum();

  private:
    QString mTestDataDir;
    QgsRasterLayer *mpRasterLayer = nullptr;
};


//runs before all tests
void TestQgsRasterBlock::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString band1byteRaster = mTestDataDir + "/raster/band1_byte_ct_epsg4326.tif";

  mpRasterLayer = new QgsRasterLayer( band1byteRaster, QStringLiteral( "band1_byte" ) );

  QVERIFY( mpRasterLayer && mpRasterLayer->isValid() );
}

//runs after all tests
void TestQgsRasterBlock::cleanupTestCase()
{
  delete mpRasterLayer;

  QgsApplication::exitQgis();
}

void TestQgsRasterBlock::testBasic()
{
  QgsRasterDataProvider *provider = mpRasterLayer->dataProvider();
  QVERIFY( provider );

  const QgsRectangle fullExtent = mpRasterLayer->extent();
  const int width = mpRasterLayer->width();
  const int height = mpRasterLayer->height();

  QgsRasterBlock *block = provider->block( 1, fullExtent, width, height );

  bool isNoData = false;

  QCOMPARE( block->width(), 10 );
  QCOMPARE( block->height(), 10 );

  QVERIFY( block->isValid() );
  QVERIFY( !block->isEmpty() );
  QCOMPARE( block->dataType(), Qgis::DataType::Byte );
  QCOMPARE( block->dataTypeSize(), 1 );
  QVERIFY( block->hasNoDataValue() );
  QVERIFY( block->hasNoData() );
  QCOMPARE( block->noDataValue(), 255. );

  // value() with row, col
  QCOMPARE( block->value( 0, 0 ), 2. );
  QCOMPARE( block->value( 0, 1 ), 5. );
  QCOMPARE( block->value( 1, 0 ), 27. );
  QCOMPARE( block->valueAndNoData( 0, 0, isNoData ), 2. );
  QVERIFY( !isNoData );
  QCOMPARE( block->valueAndNoData( 0, 1, isNoData ), 5. );
  QVERIFY( !isNoData );
  QCOMPARE( block->valueAndNoData( 1, 0, isNoData ), 27. );
  QVERIFY( !isNoData );
  QVERIFY( std::isnan( block->valueAndNoData( mpRasterLayer->width() + 1, 0, isNoData ) ) );
  QVERIFY( isNoData );

  // value() with index
  QCOMPARE( block->value( 0 ), 2. );
  QCOMPARE( block->value( 1 ), 5. );
  QCOMPARE( block->value( 10 ), 27. );
  QCOMPARE( block->valueAndNoData( 0, isNoData ), 2. );
  QVERIFY( !isNoData );
  QCOMPARE( block->valueAndNoData( 1, isNoData ), 5. );
  QVERIFY( !isNoData );
  QCOMPARE( block->valueAndNoData( 10, isNoData ), 27. );
  QVERIFY( !isNoData );
  QVERIFY( std::isnan( block->valueAndNoData( mpRasterLayer->width() * mpRasterLayer->height(), isNoData ) ) );
  QVERIFY( isNoData );

  // isNoData()
  QCOMPARE( block->isNoData( 0, 1 ), false );
  QCOMPARE( block->isNoData( 0, 2 ), true );
  QCOMPARE( block->isNoData( 1 ), false );
  QCOMPARE( block->isNoData( 2 ), true );
  QCOMPARE( block->valueAndNoData( 0, 1, isNoData ), 5. );
  QVERIFY( !isNoData );
  block->valueAndNoData( 0, 2, isNoData );
  QVERIFY( isNoData );
  QCOMPARE( block->valueAndNoData( 1, isNoData ), 5. );
  QVERIFY( !isNoData );
  block->valueAndNoData( 2, isNoData );
  QVERIFY( isNoData );

  // data()
  const QByteArray data = block->data();
  QCOMPARE( data.count(), 100 ); // 10x10 raster with 1 byte/pixel
  QCOMPARE( data.at( 0 ), ( char ) 2 );
  QCOMPARE( data.at( 1 ), ( char ) 5 );
  QCOMPARE( data.at( 10 ), ( char ) 27 );

  // setData()
  const QByteArray newData( "\xaa\xbb\xcc\xdd" );
  block->setData( newData, 1 );
  const QByteArray data2 = block->data();
  QCOMPARE( data2.at( 0 ), ( char ) 2 );
  QCOMPARE( data2.at( 1 ), '\xaa' );
  QCOMPARE( data2.at( 2 ), '\xbb' );
  QCOMPARE( data2.at( 10 ), ( char ) 27 );

  delete block;
}

void TestQgsRasterBlock::testWrite()
{
  const QgsRectangle extent = mpRasterLayer->extent();
  int nCols = mpRasterLayer->width(), nRows = mpRasterLayer->height();
  QVERIFY( nCols > 0 );
  QVERIFY( nRows > 0 );
  double tform[] = {
    extent.xMinimum(), extent.width() / nCols, 0.0,
    extent.yMaximum(), 0.0, -extent.height() / nRows
  };

  // generate unique filename (need to open the file first to generate it)
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();

  // create a GeoTIFF - this will create data provider in editable mode
  const QString filename = tmpFile.fileName();
  QgsRasterDataProvider *dp = QgsRasterDataProvider::create( QStringLiteral( "gdal" ), filename, QStringLiteral( "GTiff" ), 1, Qgis::DataType::Byte, 10, 10, tform, mpRasterLayer->crs() );

  QgsRasterBlock *block = mpRasterLayer->dataProvider()->block( 1, mpRasterLayer->extent(), mpRasterLayer->width(), mpRasterLayer->height() );

  QByteArray origData = block->data();
  origData.detach(); // make sure we have private copy independent from independent block content
  QCOMPARE( origData.at( 0 ), ( char ) 2 );
  QCOMPARE( origData.at( 1 ), ( char ) 5 );

  // change first two pixels
  block->setData( QByteArray( "\xa0\xa1" ) );
  bool res = dp->writeBlock( block, 1 );
  QVERIFY( res );

  QgsRasterBlock *block2 = dp->block( 1, mpRasterLayer->extent(), mpRasterLayer->width(), mpRasterLayer->height() );
  const QByteArray newData2 = block2->data();
  QCOMPARE( newData2.at( 0 ), '\xa0' );
  QCOMPARE( newData2.at( 1 ), '\xa1' );

  delete block2;
  delete dp;

  // newly open raster and verify the write was permanent
  QgsRasterLayer *rlayer = new QgsRasterLayer( filename, QStringLiteral( "tmp" ), QStringLiteral( "gdal" ) );
  QVERIFY( rlayer->isValid() );
  QgsRasterBlock *block3 = rlayer->dataProvider()->block( 1, rlayer->extent(), rlayer->width(), rlayer->height() );
  const QByteArray newData3 = block3->data();
  QCOMPARE( newData3.at( 0 ), '\xa0' );
  QCOMPARE( newData3.at( 1 ), '\xa1' );

  QgsRasterBlock *block4 = new QgsRasterBlock( Qgis::DataType::Byte, 1, 2 );
  block4->setData( QByteArray( "\xb0\xb1" ) );

  // cannot write when provider is not editable
  res = rlayer->dataProvider()->writeBlock( block4, 1 );
  QVERIFY( !res );

  // some sanity checks
  QVERIFY( !rlayer->dataProvider()->isEditable() );
  res = rlayer->dataProvider()->setEditable( false );
  QVERIFY( !res );

  // make the provider editable
  res = rlayer->dataProvider()->setEditable( true );
  QVERIFY( res );
  QVERIFY( rlayer->dataProvider()->isEditable() );

  res = rlayer->dataProvider()->writeBlock( block4, 1 );
  QVERIFY( res );

  // finish the editing session
  res = rlayer->dataProvider()->setEditable( false );
  QVERIFY( res );
  QVERIFY( !rlayer->dataProvider()->isEditable() );

  // verify the change is there
  QgsRasterBlock *block5 = rlayer->dataProvider()->block( 1, rlayer->extent(), rlayer->width(), rlayer->height() );
  const QByteArray newData5 = block5->data();
  QCOMPARE( newData5.at( 0 ), '\xb0' );
  QCOMPARE( newData5.at( 1 ), '\xa1' ); // original data
  QCOMPARE( newData5.at( 10 ), '\xb1' );

  delete block3;
  delete block4;
  delete block5;
  delete rlayer;

  delete block;
}

void TestQgsRasterBlock::testPrintValueDouble_data()
{
  QTest::addColumn<double>( "value" );
  QTest::addColumn<bool>( "localized" );
  QTest::addColumn<QLocale::Language>( "language" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "English double" ) << 123456.789 << true << QLocale::Language::English << QStringLiteral( "123,456.789" );
  QTest::newRow( "English int" ) << 123456.0 << true << QLocale::Language::English << QStringLiteral( "123,456" );
  QTest::newRow( "English int no locale" ) << 123456.0 << false << QLocale::Language::English << QStringLiteral( "123456" );
  QTest::newRow( "English double no locale" ) << 123456.789 << false << QLocale::Language::English << QStringLiteral( "123456.789" );
  QTest::newRow( "English negative double" ) << -123456.789 << true << QLocale::Language::English << QStringLiteral( "-123,456.789" );

  QTest::newRow( "Italian double" ) << 123456.789 << true << QLocale::Language::Italian << QStringLiteral( "123.456,789" );
  QTest::newRow( "Italian int" ) << 123456.0 << true << QLocale::Language::Italian << QStringLiteral( "123.456" );
  QTest::newRow( "Italian int no locale" ) << 123456.0 << false << QLocale::Language::Italian << QStringLiteral( "123456" );
  QTest::newRow( "Italian double no locale" ) << 123456.789 << false << QLocale::Language::Italian << QStringLiteral( "123456.789" );
  QTest::newRow( "Italian negative double" ) << -123456.789 << true << QLocale::Language::Italian << QStringLiteral( "-123.456,789" );
}


void TestQgsRasterBlock::testPrintValueDouble()
{
  QFETCH( double, value );
  QFETCH( bool, localized );
  QFETCH( QLocale::Language, language );
  QFETCH( QString, expected );

  QLocale::setDefault( language );
  QString actual = QgsRasterBlock::printValue( value, localized );
  QCOMPARE( actual, expected );
  QLocale::setDefault( QLocale::Language::English );
}

void TestQgsRasterBlock::testMinimumMaximum()
{
  QgsRasterLayer rl( copyTestData( QStringLiteral( "raster/dem.tif" ) ), QStringLiteral( "dem" ) );
  QVERIFY( rl.isValid() );

  QgsRasterDataProvider *provider = rl.dataProvider();
  provider->setUseSourceNoDataValue( 1, false );
  const QgsRectangle fullExtent = rl.extent();
  const int width = rl.width();
  const int height = rl.height();
  std::unique_ptr<QgsRasterBlock> block( provider->block( 1, fullExtent, width, height ) );
  QVERIFY( block );
  QVERIFY( !block->hasNoData() );

  double value = 0;
  int row = 0;
  int col = 0;
  QVERIFY( block->minimum( value, row, col ) );
  QCOMPARE( value, 85 );
  QCOMPARE( row, 89 );
  QCOMPARE( col, 123 );

  QVERIFY( block->maximum( value, row, col ) );
  QCOMPARE( value, 243 );
  QCOMPARE( row, 152 );
  QCOMPARE( col, 301 );

  double value2 = 0;
  int row2 = 0;
  int col2 = 0;

  QVERIFY( block->minimumMaximum( value, row, col, value2, row2, col2 ) );
  QCOMPARE( value, 85 );
  QCOMPARE( row, 89 );
  QCOMPARE( col, 123 );
  QCOMPARE( value2, 243 );
  QCOMPARE( row2, 152 );
  QCOMPARE( col2, 301 );

  // with no data value corresponding to min
  provider->setNoDataValue( 1, 85 );
  block.reset( provider->block( 1, fullExtent, width, height ) );
  QVERIFY( block );
  QVERIFY( block->hasNoData() );
  QCOMPARE( block->noDataValue(), 85 );

  QVERIFY( block->minimum( value, row, col ) );
  QCOMPARE( value, 85.5 );
  QCOMPARE( row, 50 );
  QCOMPARE( col, 183 );
  QVERIFY( block->maximum( value, row, col ) );
  QCOMPARE( value, 243 );
  QCOMPARE( row, 152 );
  QCOMPARE( col, 301 );
  QVERIFY( block->minimumMaximum( value, row, col, value2, row2, col2 ) );
  QCOMPARE( value, 85.5 );
  QCOMPARE( row, 50 );
  QCOMPARE( col, 183 );
  QCOMPARE( value2, 243 );
  QCOMPARE( row2, 152 );
  QCOMPARE( col2, 301 );

  // with no data value corresponding to max
  provider->setNoDataValue( 1, 243 );
  block.reset( provider->block( 1, fullExtent, width, height ) );
  QVERIFY( block );
  QVERIFY( block->hasNoData() );
  QCOMPARE( block->noDataValue(), 243 );

  QVERIFY( block->minimum( value, row, col ) );
  QCOMPARE( value, 85 );
  QCOMPARE( row, 89 );
  QCOMPARE( col, 123 );
  QVERIFY( block->maximum( value, row, col ) );
  QGSCOMPARENEAR( value, 241.800003052, 0.0000001 );
  QCOMPARE( row, 144 );
  QCOMPARE( col, 293 );
  QVERIFY( block->minimumMaximum( value, row, col, value2, row2, col2 ) );
  QCOMPARE( value, 85 );
  QCOMPARE( row, 89 );
  QCOMPARE( col, 123 );
  QGSCOMPARENEAR( value2, 241.800003052, 0.0000001 );
  QCOMPARE( row2, 144 );
  QCOMPARE( col2, 293 );
}

void TestQgsRasterBlock::testPrintValueFloat_data()
{
  QTest::addColumn<float>( "value" );
  QTest::addColumn<bool>( "localized" );
  QTest::addColumn<QLocale::Language>( "language" );
  QTest::addColumn<QString>( "expected" );

  QTest::newRow( "English float" ) << 123456.789f << true << QLocale::Language::English << QStringLiteral( "123,456.79" );
  QTest::newRow( "English int" ) << 123456.f << true << QLocale::Language::English << QStringLiteral( "123,456" );
  QTest::newRow( "English int no locale" ) << 123456.f << false << QLocale::Language::English << QStringLiteral( "123456" );
  QTest::newRow( "English float no locale" ) << 123456.789f << false << QLocale::Language::English << QStringLiteral( "123456.79" );
  QTest::newRow( "English negative float" ) << -123456.789f << true << QLocale::Language::English << QStringLiteral( "-123,456.79" );

  QTest::newRow( "Italian float" ) << 123456.789f << true << QLocale::Language::Italian << QStringLiteral( "123.456,79" );
  QTest::newRow( "Italian int" ) << 123456.f << true << QLocale::Language::Italian << QStringLiteral( "123.456" );
  QTest::newRow( "Italian int no locale" ) << 123456.f << false << QLocale::Language::Italian << QStringLiteral( "123456" );
  QTest::newRow( "Italian float no locale" ) << 123456.789f << false << QLocale::Language::Italian << QStringLiteral( "123456.79" );
  QTest::newRow( "Italian negative float" ) << -123456.789f << true << QLocale::Language::Italian << QStringLiteral( "-123.456,79" );
}

void TestQgsRasterBlock::testPrintValueFloat()
{
  QFETCH( float, value );
  QFETCH( bool, localized );
  QFETCH( QLocale::Language, language );
  QFETCH( QString, expected );

  QLocale::setDefault( language );
  QString actual = QgsRasterBlock::printValue( value, localized );
  QCOMPARE( actual, expected );
  QLocale::setDefault( QLocale::Language::English );
}

QGSTEST_MAIN( TestQgsRasterBlock )

#include "testqgsrasterblock.moc"
