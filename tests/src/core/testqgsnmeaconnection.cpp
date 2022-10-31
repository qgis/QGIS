/***************************************************************************
  testqgsnmeaconnection.cpp

 ---------------------
 begin                : October 2022
 copyright            : (C) 2022 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include "qgis.h"
#include "qgsnmeaconnection.h"

#include <QObject>
#include <QBuffer>
#include <QSignalSpy>

class ReplayNmeaConnection : public QgsNmeaConnection
{
    Q_OBJECT
  public:

    ReplayNmeaConnection()
      : QgsNmeaConnection( nullptr )
    {
      mBuffer = new QBuffer();
      setSource( mBuffer );
      Q_ASSERT( connect() );
    }

    QgsGpsInformation push( const QString &message )
    {
      const QString messageTerminated = message + QStringLiteral( "\r\n" );

      QSignalSpy spy( this, &QgsNmeaConnection::stateChanged );
      const qint64 pos = mBuffer->pos();
      mBuffer->write( messageTerminated.toLocal8Bit().constData() );
      mBuffer->seek( pos );

      spy.wait();
      return spy.constLast().at( 0 ).value< QgsGpsInformation >();
    }

  private:

    QBuffer *mBuffer = nullptr;
};


class TestQgsNmeaConnection : public QgsTest
{
    Q_OBJECT

  public:

    TestQgsNmeaConnection() : QgsTest( QStringLiteral( "NMEA Connection Tests" ) ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testBasic();
    void testFixStatus();

};

void TestQgsNmeaConnection::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsNmeaConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsNmeaConnection::testBasic()
{
  ReplayNmeaConnection connection;

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPGSV,3,1,12,07,41,181,26,05,34,292,30,16,34,060,36,26,24,031,24*7B" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info = connection.push( QStringLiteral( "$GPGSV,3,2,12,02,61,115,,21,53,099,,03,51,111,,19,32,276,*72" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info =  connection.push( QStringLiteral( "$GPGSV,3,3,12,17,31,279,,28,27,320,,23,23,026,,14,22,060,*7B" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info = connection.push( QStringLiteral( "$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 0 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGGA,084112.185,6938.6532,N,01856.8526,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGSA,A,3,07,05,16,26,,,,,,,,,3.4,1.4,3.1*33" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPRMC,084112.185,A,6938.6532,N,01856.8526,E,0.08,2.00,220120,,,A*60" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGGA,084113.185,6938.6532,N,01856.8526,E,1,04,1.4,34.0,M,29.4,M,,0000*63" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGSA,A,3,07,05,16,26,,,,,,,,,3.4,1.4,3.1*33" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPRMC,084113.185,A,6938.6532,N,01856.8526,E,0.05,2.00,220120,,,A*6C" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );
}

void TestQgsNmeaConnection::testFixStatus()
{
  ReplayNmeaConnection connection;

  QSignalSpy statusSpy( &connection, &QgsGpsConnection::fixStatusChanged );

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPRMC,220516,V,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( !info.isValid() );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoFix );

  // no fix status change
  connection.push( QStringLiteral( "$GPRMC,220516,V,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QCOMPARE( statusSpy.count(), 1 );

  // simulate 3d fix
  connection.push( QStringLiteral( "$GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix3D );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 2 );

  // simulate 2d fix
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( statusSpy.count(), 3 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix2D );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 3 );

  // simulate fix not available
  connection.push( QStringLiteral( "$GPGSA,A,1,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( !info.isValid() );
  QCOMPARE( info.fixStatus(), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( statusSpy.count(), 4 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoFix );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,1,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 4 );

  // invalid fix due to bad lat / long values
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  // latitude 99 degrees => out of range
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,9933.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QGSCOMPARENEAR( info.latitude, 99.563666, 0.00001 );
  QGSCOMPARENEAR( info.longitude, -0.70400000, 0.00001 );
  QVERIFY( !info.isValid() );

  QCOMPARE( statusSpy.count(), 5 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix2D );

  info = connection.push( QStringLiteral( "$GPRMC,220516,A,9933.82,S,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QGSCOMPARENEAR( info.latitude, -99.563666, 0.00001 );
  QGSCOMPARENEAR( info.longitude, -0.70400000, 0.00001 );
  QVERIFY( !info.isValid() );

  // longitude 192 degrees => out of range
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,1933.82,N,19192.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QGSCOMPARENEAR( info.latitude, 19.5636666667, 0.00001 );
  QGSCOMPARENEAR( info.longitude, -192.5373333333, 0.00001 );
  QVERIFY( !info.isValid() );

  info = connection.push( QStringLiteral( "$GPRMC,220516,A,1933.82,N,19192.24,E,173.8,231.8,130694,004.2,W*70" ) );
  QGSCOMPARENEAR( info.latitude, 19.5636666667, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 192.5373333333, 0.00001 );
  QVERIFY( !info.isValid() );

  QCOMPARE( statusSpy.count(), 5 );

  connection.close();
  QCOMPARE( statusSpy.count(), 6 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoData );

}

QGSTEST_MAIN( TestQgsNmeaConnection )
#include "testqgsnmeaconnection.moc"
