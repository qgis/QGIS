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

#include <QtTest/QtTest>
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

/** \ingroup UnitTests
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

    void scaleDataType(); //test resultant data types for int raster with float scale (#11573)
    void warpedVrt(); //test loading raster which requires a warped vrt
    void noData();
    void invalidNoDataInSourceIgnored();
    void isRepresentableValue();

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

  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mReport = "<h1>GDAL Provider Tests</h1>\n";
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

void TestQgsGdalProvider::scaleDataType()
{
  QString rasterWithOffset = QString( TEST_DATA_DIR ) + "/int_raster_with_scale.tif";
  QgsDataProvider* provider = QgsProviderRegistry::instance()->provider( "gdal", rasterWithOffset );
  QgsRasterDataProvider* rp = dynamic_cast< QgsRasterDataProvider* >( provider );
  QVERIFY( rp );
  //raster is an integer data type, but has a scale < 1, so data type must be float
  QCOMPARE( rp->dataType( 1 ), QGis::Float32 );
  QCOMPARE( rp->srcDataType( 1 ), QGis::Float32 );
  delete provider;
}

void TestQgsGdalProvider::warpedVrt()
{
  QString raster = QString( TEST_DATA_DIR ) + "/requires_warped_vrt.tif";
  QgsDataProvider* provider = QgsProviderRegistry::instance()->provider( "gdal", raster );
  QgsRasterDataProvider* rp = dynamic_cast< QgsRasterDataProvider* >( provider );
  QVERIFY( rp );

  qDebug() << "x min: " << rp->extent().xMinimum();
  qDebug() << "x max: " << rp->extent().xMaximum();
  qDebug() << "y min: " << rp->extent().yMinimum();
  qDebug() << "y max: " << rp->extent().yMaximum();

  QVERIFY( qgsDoubleNear( rp->extent().xMinimum(), 2058589, 1 ) );
  QVERIFY( qgsDoubleNear( rp->extent().xMaximum(), 3118999, 1 ) );
  QVERIFY( qgsDoubleNear( rp->extent().yMinimum(), 2281355, 1 ) );
  QVERIFY( qgsDoubleNear( rp->extent().yMaximum(), 3129683, 1 ) );
  delete provider;
}

void TestQgsGdalProvider::noData()
{
  QString raster = QString( TEST_DATA_DIR ) + "/raster/band1_byte_ct_epsg4326.tif";
  QgsDataProvider* provider = QgsProviderRegistry::instance()->provider( "gdal", raster );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider* rp = dynamic_cast< QgsRasterDataProvider* >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->srcNoDataValue( 1 ), static_cast<double>( 255 ) );
  }
  delete provider;
}

void TestQgsGdalProvider::invalidNoDataInSourceIgnored()
{
  QString raster = QString( TEST_DATA_DIR ) + "/raster/byte_with_nan_nodata.tif";
  QgsDataProvider* provider = QgsProviderRegistry::instance()->provider( "gdal", raster );
  QVERIFY( provider->isValid() );
  QgsRasterDataProvider* rp = dynamic_cast< QgsRasterDataProvider* >( provider );
  QVERIFY( rp );
  if ( rp )
  {
    QCOMPARE( rp->srcHasNoDataValue( 1 ), false );
  }
  delete provider;
}

void TestQgsGdalProvider::isRepresentableValue()
{
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., QGis::Byte ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., QGis::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 255., QGis::Byte ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 256., QGis::Byte ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., QGis::UInt16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., QGis::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65535., QGis::UInt16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 65536., QGis::UInt16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32769., QGis::Int16 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -32768., QGis::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32767., QGis::Int16 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 32768., QGis::Int16 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -1., QGis::UInt32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 0., QGis::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967295., QGis::UInt32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., QGis::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483649., QGis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -2147483648., QGis::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483647., QGis::Int32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( 2147483648., QGis::Int32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( 4294967296., QGis::UInt32 ), false );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), QGis::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), QGis::Float32 ), false );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<float>::max(), QGis::Float32 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<float>::max(), QGis::Float32 ), true );

  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::infinity(), QGis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::infinity(), QGis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::quiet_NaN(), QGis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( -std::numeric_limits<double>::max(), QGis::Float64 ), true );
  QCOMPARE( QgsRaster::isRepresentableValue( std::numeric_limits<double>::max(), QGis::Float64 ), true );
}

QTEST_MAIN( TestQgsGdalProvider )
#include "testqgsgdalprovider.moc"
