/***************************************************************************
                         testqgsreclassifyutils.cpp
                         ---------------------
    begin                : June 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
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
#include "qgsreclassifyutils.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterfilewriter.h"
#include <QTemporaryFile>

class TestQgsReclassifyUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void reclassifyValue();
    void testReclassify_data();
    void testReclassify();

  private:

    QVector<double> reclassifyBlock( const QVector<double> &input, int nRows, int nCols,
                                     const QVector< QgsReclassifyUtils::RasterClass > &classes,
                                     double destNoDataValue, bool useNoDataForMissing );

};

void TestQgsReclassifyUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsReclassifyUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsReclassifyUtils::reclassifyValue()
{
  // no classes
  bool ok = false;
  QVector< QgsReclassifyUtils::RasterClass > classes;
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 5.9, ok ), 5.9 );
  QVERIFY( !ok );

  // one class
  classes << QgsReclassifyUtils::RasterClass( 5, 11, QgsRasterRange::IncludeMin, -99.5 );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 5.9, ok ), -99.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 15.9, ok ), 15.9 );
  QVERIFY( !ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, -15.9, ok ), -15.9 );
  QVERIFY( !ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 5, ok ), -99.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 11, ok ), 11.0 );
  QVERIFY( !ok );

  // second class
  classes << QgsReclassifyUtils::RasterClass( 11, 15, QgsRasterRange::IncludeMin, -59.5 );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 5.9, ok ), -99.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 11.9, ok ), -59.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 15.9, ok ), 15.9 );
  QVERIFY( !ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, -15.9, ok ), -15.9 );
  QVERIFY( !ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 5, ok ), -99.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 11, ok ), -59.5 );
  QVERIFY( ok );
  QCOMPARE( QgsReclassifyUtils::reclassifyValue( classes, 15, ok ), 15.0 );
  QVERIFY( !ok );
}

Q_DECLARE_METATYPE( QgsReclassifyUtils::RasterClass );

void TestQgsReclassifyUtils::testReclassify_data()
{
  QTest::addColumn<QVector< double >>( "input" );
  QTest::addColumn<int>( "nRows" );
  QTest::addColumn<int>( "nCols" );
  QTest::addColumn<QVector< QgsReclassifyUtils::RasterClass  >>( "classes" );
  QTest::addColumn<double>( "destNoDataValue" );
  QTest::addColumn<bool>( "useNoDataForMissing" );
  QTest::addColumn<int>( "dataType" );
  QTest::addColumn<QVector< double >>( "expected" );

  QTest::newRow( "no change" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                               << 3 << 2
                               << QVector< QgsReclassifyUtils::RasterClass  >()
                               << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                               << QVector< double > { 1, 2, 3, 4, 5, 6 };

  QTest::newRow( "one class" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                               << 3 << 2
                               << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                    << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMax, 8 ) )
                               << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                               << QVector< double > { 1, 2, 3, 8, 8, 6 };

  QTest::newRow( "byte type" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                               << 3 << 2
                               << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                    << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMax, 8 ) )
                               << -9999.0 << false << static_cast< int >( Qgis::DataType::Byte )
                               << QVector< double > { 1, 2, 3, 8, 8, 6 };

  QTest::newRow( "two class" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                               << 3 << 2
                               << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                    << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMax, 8 )
                                    << QgsReclassifyUtils::RasterClass( 1, 3, QgsRasterRange::IncludeMin, -7 ) )
                               << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                               << QVector< double > { -7, -7, 3, 8, 8, 6 };

  QTest::newRow( "infinite range" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                                    << 3 << 2
                                    << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                         << QgsReclassifyUtils::RasterClass( 3, std::numeric_limits<double>::quiet_NaN(), QgsRasterRange::IncludeMax, 8 )
                                         << QgsReclassifyUtils::RasterClass( 1, 3, QgsRasterRange::IncludeMin, -7 ) )
                                    << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                                    << QVector< double > { -7, -7, 3, 8, 8, 8 };

  QTest::newRow( "infinite range 2" ) << QVector< double > { 1, 2, 3, 4, 5, 6 }
                                      << 3 << 2
                                      << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                          << QgsReclassifyUtils::RasterClass( 3, 4, QgsRasterRange::IncludeMax, 8 )
                                          << QgsReclassifyUtils::RasterClass( std::numeric_limits<double>::quiet_NaN(), 3, QgsRasterRange::IncludeMin, -7 ) )
                                      << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                                      << QVector< double > { -7, -7, 3, 8, 5, 6 };

  QTest::newRow( "with source no data" ) << QVector< double > { 1, 2, -9999, 4, 5, 6 }
                                         << 3 << 2
                                         << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                             << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMinAndMax, 8 ) )
                                         << -9999.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                                         << QVector< double > { 1, 2, -9999, 8, 8, 6 };

  QTest::newRow( "with dest no data" ) << QVector< double > { 1, 2, -9999, 4, 5, 6 }
                                       << 3 << 2
                                       << ( QVector< QgsReclassifyUtils::RasterClass  >()
                                           << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMinAndMax, 8 ) )
                                       << -99.0 << false << static_cast< int >( Qgis::DataType::Float32 )
                                       << QVector< double > { 1, 2, -99, 8, 8, 6 };

  QTest::newRow( "use no data for missing" ) << QVector< double > { 1, 2, -9999, 4, 5, 6 }
      << 3 << 2
      << ( QVector< QgsReclassifyUtils::RasterClass  >()
           << QgsReclassifyUtils::RasterClass( 3, 5, QgsRasterRange::IncludeMinAndMax, 8 ) )
      << -9999.0 << true << static_cast< int >( Qgis::DataType::Float32 )
      << QVector< double > { -9999, -9999, -9999, 8, 8, -9999 };
}

void TestQgsReclassifyUtils::testReclassify()
{
  QFETCH( QVector< double >, input );
  QFETCH( int, nRows );
  QFETCH( int, nCols );
  QFETCH( QVector< QgsReclassifyUtils::RasterClass  >, classes );
  QFETCH( double, destNoDataValue );
  QFETCH( bool, useNoDataForMissing );
  QFETCH( int, dataType );
  QFETCH( QVector< double >, expected );

  const QgsRectangle extent = QgsRectangle( 0, 0, nRows, nCols );
  const QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:3857" ) );
  double tform[] =
  {
    extent.xMinimum(), extent.width() / nCols, 0.0,
    extent.yMaximum(), 0.0, -extent.height() / nRows
  };

  // generate unique filename (need to open the file first to generate it)
  QTemporaryFile tmpFile;
  tmpFile.open();
  tmpFile.close();

  // create a GeoTIFF - this will create data provider in editable mode
  QString filename = tmpFile.fileName();

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( filename );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( QStringLiteral( "GTiff" ) );
  std::unique_ptr<QgsRasterDataProvider > dp( writer->createOneBandRaster( Qgis::DataType::Float32, nCols, nRows, extent, crs ) );
  QVERIFY( dp->isValid() );
  dp->setNoDataValue( 1, -9999 );
  std::unique_ptr< QgsRasterBlock > block( dp->block( 1, extent, nCols, nRows ) );
  if ( !dp->isEditable() )
  {
    QVERIFY( dp->setEditable( true ) );
  }
  int i = 0;
  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      block->setValue( row, col, input[i++] );
    }
  }
  QVERIFY( dp->writeBlock( block.get(), 1 ) );
  QVERIFY( dp->setEditable( false ) );

  // make destination raster
  QTemporaryFile tmpFile2;
  tmpFile2.open();
  tmpFile2.close();

  // create a GeoTIFF - this will create data provider in editable mode
  filename = tmpFile2.fileName();
  std::unique_ptr< QgsRasterDataProvider > dp2( QgsRasterDataProvider::create( QStringLiteral( "gdal" ), filename, QStringLiteral( "GTiff" ), 1, static_cast< Qgis::DataType >( dataType ), 10, 10, tform, crs ) );
  QVERIFY( dp2->isValid() );

  // reclassify
  QgsReclassifyUtils::reclassify( classes, dp.get(), 1, extent, nCols, nRows, dp2.get(), destNoDataValue, useNoDataForMissing );

  // read back in values
  block.reset( dp2->block( 1, extent, nCols, nRows ) );
  QVector< double > res( nCols * nRows );
  i = 0;
  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      res[i++] = block->value( row, col );
    }
  }

  for ( int row = 0; row < nRows; row++ )
  {
    for ( int col = 0; col < nCols; col++ )
    {
      QCOMPARE( res[row * nCols + col], expected[row * nCols + col] );
    }
  }
}


QGSTEST_MAIN( TestQgsReclassifyUtils )
#include "testqgsreclassifyutils.moc"
