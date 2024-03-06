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
      bool ok = connect();
      Q_ASSERT( ok );
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

    void pushString( const QString &string )
    {
      const qint64 pos = mBuffer->pos();
      mBuffer->write( string.toLocal8Bit().constData() );
      mBuffer->seek( pos );
      parseData();
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
    void testVtg();
    void testFixStatus();
    void testFixStatusAcrossConstellations();
    void testConstellation();
    void testPosition();
    void testComponent();
    void testIncompleteMessage();

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
  Qgis::GnssConstellation constellation = Qgis::GnssConstellation::Unknown;
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( constellation, Qgis::GnssConstellation::Unknown );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info = connection.push( QStringLiteral( "$GPGSV,3,2,12,02,61,115,,21,53,099,,03,51,111,,19,32,276,*72" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info =  connection.push( QStringLiteral( "$GPGSV,3,3,12,17,31,279,,28,27,320,,23,23,026,,14,22,060,*7B" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Unknown (-1)" ) );
  QCOMPARE( info.latitude, 0 );
  QCOMPARE( info.longitude, 0 );
  QCOMPARE( info.elevation, 0 );
  QVERIFY( std::isnan( info.direction ) );

  info = connection.push( QStringLiteral( "$GPGGA,084112.185,6938.6532,N,01856.8526,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  // At this stage, we have no direction information, waiting for an GPRMC sentence
  QCOMPARE( info.direction, std::numeric_limits<double>::quiet_NaN() );
  // At this stage, having only received an GPGGA sentence, the date remains unknown
  QCOMPARE( info.utcDateTime, QDateTime() );
  QCOMPARE( info.utcTime, QTime( 8, 41, 12, 185 ) );

  info = connection.push( QStringLiteral( "$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );
  // The (optional) GPRMC sentence came in, we now have a date and a time value.
  QDateTime dateTime;
  dateTime.setTimeSpec( Qt::UTC );
  dateTime.setDate( QDate( 2020, 1, 22 ) );
  dateTime.setTime( QTime( 8, 41, 11, 185 ) );
  QCOMPARE( info.utcDateTime, dateTime );
  QCOMPARE( info.utcTime, dateTime.time() );

  info = connection.push( QStringLiteral( "$GPGSA,A,3,07,05,16,26,,,,,,,,,3.4,1.4,3.1*33" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( constellation, Qgis::GnssConstellation::Gps );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPRMC,084112.185,A,6938.6532,N,01856.8526,E,0.08,2.00,220120,,,A*60" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 35 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGGA,084113.185,6938.6532,N,01856.8526,E,1,04,1.4,34.0,M,29.4,M,,0000*63" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPGSA,A,3,07,05,16,26,,,,,,,,,3.4,1.4,3.1*33" ) );
  QVERIFY( info.isValid() );
  QVERIFY( !info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );

  info = connection.push( QStringLiteral( "$GPRMC,084113.185,A,6938.6532,N,01856.8526,E,0.05,2.00,220120,,,A*6C" ) );
  QVERIFY( info.isValid() );
  QVERIFY( info.satInfoComplete );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( info.qualityDescription(), QStringLiteral( "Autonomous" ) );
  QGSCOMPARENEAR( info.latitude, 69.6442183333, 0.00001 );
  QGSCOMPARENEAR( info.longitude, 18.947545, 0.00001 );
  QCOMPARE( info.elevation, 34 );
  QGSCOMPARENEAR( info.direction, 2.0000000000, 0.0001 );
}

void TestQgsNmeaConnection::testVtg()
{
  ReplayNmeaConnection connection;

  QSignalSpy statusSpy( &connection, &QgsGpsConnection::fixStatusChanged );

  // try initially with no direction
  QgsGpsInformation info = connection.push( QStringLiteral( "$GPVTG,,T,,M,0.003,N,0.005,K,D*20" ) );
  QVERIFY( info.isValid() );
  QVERIFY( std::isnan( info.direction ) );
  QCOMPARE( info.speed, 0.005 );
  info = connection.push( QStringLiteral( "$GPVTG,224.592,T,224.492,M,0.003,N,0.006,K,D*20" ) );
  QVERIFY( info.isValid() );
  // must be direction to true north, not magnetic north
  QCOMPARE( info.direction, 224.592 );
  QCOMPARE( info.speed, 0.006 );
  info = connection.push( QStringLiteral( "$GNVTG,139.969,T,139.969,M,0.007,N,0.013,K,D*3D" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.direction, 139.969 );
  QCOMPARE( info.speed, 0.013 );
  // direction should not be overwritten with nan
  info = connection.push( QStringLiteral( "$GPVTG,,T,,M,0.003,N,0.005,K,D*20" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.direction, 139.969 );
  QCOMPARE( info.speed, 0.005 );
}

void TestQgsNmeaConnection::testFixStatus()
{
  ReplayNmeaConnection connection;

  QSignalSpy statusSpy( &connection, &QgsGpsConnection::fixStatusChanged );

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPRMC,220516,V,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( !info.isValid() );
  Qgis::GnssConstellation constellation = Qgis::GnssConstellation::Unknown;
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoData );
  QCOMPARE( constellation, Qgis::GnssConstellation::Unknown );
  QCOMPARE( statusSpy.count(), 0 );

  // no fix status change
  connection.push( QStringLiteral( "$GPRMC,220516,V,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QCOMPARE( statusSpy.count(), 0 );

  // simulate 3d fix
  connection.push( QStringLiteral( "$GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix3D );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,3,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 1 );

  // simulate 2d fix
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( info.isValid() );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix2D );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 2 );

  // simulate fix not available
  connection.push( QStringLiteral( "$GPGSA,A,1,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QVERIFY( !info.isValid() );
  QCOMPARE( info.bestFixStatus( constellation ), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( statusSpy.count(), 3 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoFix );

  // no fix status change
  connection.push( QStringLiteral( "$GPGSA,A,1,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  QCOMPARE( statusSpy.count(), 3 );

  // invalid fix due to bad lat / long values
  connection.push( QStringLiteral( "$GPGSA,A,2,,,,,,16,18,,22,24,,,3.6,2.1,2.2*3C" ) );
  // latitude 99 degrees => out of range
  info = connection.push( QStringLiteral( "$GPRMC,220516,A,9933.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70" ) );
  QGSCOMPARENEAR( info.latitude, 99.563666, 0.00001 );
  QGSCOMPARENEAR( info.longitude, -0.70400000, 0.00001 );
  QVERIFY( !info.isValid() );

  QCOMPARE( statusSpy.count(), 4 );
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

  QCOMPARE( statusSpy.count(), 4 );

  connection.close();
  QCOMPARE( statusSpy.count(), 5 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoData );

}

void TestQgsNmeaConnection::testFixStatusAcrossConstellations()
{
  ReplayNmeaConnection connection;

  Qgis::GnssConstellation bestConstellation = Qgis::GnssConstellation::Unknown;

  QSignalSpy statusSpy( &connection, &QgsGpsConnection::fixStatusChanged );

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPGSA,A,1,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Gps );

  info = connection.push( QStringLiteral( "$GLGSA,A,1,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( statusSpy.count(), 1 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::NoFix );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Gps );

  info = connection.push( QStringLiteral( "$GLGSA,A,2,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07\r\n$GPGSA,A,1,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( statusSpy.count(), 2 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Glonass );

  info = connection.push( QStringLiteral( "$GNGSA,A,3,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( statusSpy.count(), 3 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Gps );

  // GPS fix lost, best fix is 2D glonass one
  info = connection.push( QStringLiteral( "$GNGSA,A,1,7,9,16,27,30,9,7,1,6,5,,,4.2,3.4,2.4*07" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( statusSpy.count(), 4 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix2D );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Glonass );

  // another test where a number of GNGSA sentences are received at once
  info = connection.push( QStringLiteral( "$GNGSA,A,3,6,11,12,19,20,25,29,10,20,19,7,10,7.5,5.5,5.1*24\r\n$GNGSA,A,3,11,12,24,25,31,,,,,,,,7.5,5.5,5.1*2A\r\n$GNGSA,A,1,194,195,,,,,,,,,,,7.5,5.5,5.1*29" ) );
  QCOMPARE( info.bestFixStatus( bestConstellation ), Qgis::GpsFixStatus::Fix3D );
  QVERIFY( info.isValid() ) ;
  QCOMPARE( statusSpy.count(), 5 );
  QCOMPARE( statusSpy.constLast().at( 0 ).value< Qgis::GpsFixStatus >(), Qgis::GpsFixStatus::Fix3D );
  QCOMPARE( bestConstellation, Qgis::GnssConstellation::Gps );
}

void TestQgsNmeaConnection::testConstellation()
{
  ReplayNmeaConnection connection;
  // GSV sentences

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPGSV,8,5,30,06,33,224,00,08,19,298,00,09,21,234,00,13,18,286,00*7E" ) );
  QCOMPARE( info.satellitesInView.count(), 4 );
  QCOMPARE( info.satellitesInView.at( 0 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 1 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 2 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 3 ).constellation(), Qgis::GnssConstellation::Gps );

  info = connection.push( QStringLiteral( "$GLGSV,8,5,30,06,33,224,00,08,19,298,00,09,21,234,00,13,18,286,00*7E" ) );
  QCOMPARE( info.satellitesInView.count(), 5 );
  QCOMPARE( info.satellitesInView.at( 0 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 1 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 2 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 3 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 4 ).constellation(), Qgis::GnssConstellation::Glonass );

  info = connection.push( QStringLiteral( "$GAGSV,8,5,30,31,33,224,00,08,19,298,00,09,21,234,00,13,18,286,00*7E" ) );
  QCOMPARE( info.satellitesInView.count(), 6 );
  QCOMPARE( info.satellitesInView.at( 0 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 1 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 2 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 3 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 4 ).constellation(), Qgis::GnssConstellation::Glonass );
  QCOMPARE( info.satellitesInView.at( 5 ).constellation(), Qgis::GnssConstellation::Galileo );

  info = connection.push( QStringLiteral( "$GQGSV,8,5,30,7,33,224,00,08,19,298,00,09,21,234,00,13,18,286,00*7E" ) );
  QCOMPARE( info.satellitesInView.count(), 7 );
  QCOMPARE( info.satellitesInView.at( 0 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 1 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 2 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 3 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 4 ).constellation(), Qgis::GnssConstellation::Glonass );
  QCOMPARE( info.satellitesInView.at( 5 ).constellation(), Qgis::GnssConstellation::Galileo );
  QCOMPARE( info.satellitesInView.at( 6 ).constellation(), Qgis::GnssConstellation::Qzss );

  info = connection.push( QStringLiteral( "$GBGSV,8,5,30,64,33,224,00,08,19,298,00,09,21,234,00,13,18,286,00*7E" ) );
  QCOMPARE( info.satellitesInView.count(), 8 );
  QCOMPARE( info.satellitesInView.at( 0 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 1 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 2 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 3 ).constellation(), Qgis::GnssConstellation::Gps );
  QCOMPARE( info.satellitesInView.at( 4 ).constellation(), Qgis::GnssConstellation::Glonass );
  QCOMPARE( info.satellitesInView.at( 5 ).constellation(), Qgis::GnssConstellation::Galileo );
  QCOMPARE( info.satellitesInView.at( 6 ).constellation(), Qgis::GnssConstellation::Qzss );
  QCOMPARE( info.satellitesInView.at( 7 ).constellation(), Qgis::GnssConstellation::BeiDou );
}

void TestQgsNmeaConnection::testPosition()
{
  ReplayNmeaConnection connection;

  QSignalSpy spy( &connection, &QgsGpsConnection::positionChanged );

  connection.push( QStringLiteral( "$GPGGA,084112.185,6900.0,N,01800.0,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( spy.constLast().at( 0 ).value< QgsPoint>(), QgsPoint( 18, 69, 35 ) );
  QCOMPARE( connection.lastValidLocation(), QgsPoint( 18, 69, 35 ) );
  // push same location, should be no new signal
  connection.push( QStringLiteral( "$GPGGA,084112.185,6900.0,N,01800.0,E,1,04,1.4,35.0,M,29.6,M,,0000*63" ) );
  QCOMPARE( spy.count(), 1 );

  // new location
  connection.push( QStringLiteral( "$GPGGA,084112.185,6900.0,N,01900.0,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );
  QCOMPARE( spy.count(), 2 );
  QCOMPARE( spy.constLast().at( 0 ).value< QgsPoint>(), QgsPoint( 19, 69, 35 ) );
  QCOMPARE( connection.lastValidLocation(), QgsPoint( 19, 69, 35 ) );

  // invalid location (latitude > 90 degrees)
  connection.push( QStringLiteral( "$GPGGA,084112.185,9900.0,N,01900.0,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );
  // signal will NOT be emitted
  QCOMPARE( spy.count(), 2 );
  // last valid location remains unchanged
  QCOMPARE( connection.lastValidLocation(), QgsPoint( 19, 69, 35 ) );
}

void TestQgsNmeaConnection::testComponent()
{
  ReplayNmeaConnection connection;

  QgsGpsInformation info = connection.push( QStringLiteral( "$GPGGA,084112.185,6900.0,N,01800.0,E,1,04,1.4,35.0,M,29.4,M,,0000*63" ) );

  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Location ).value< QgsPointXY >(), QgsPointXY( 18, 69 ) );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Altitude ).toDouble(), 35 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::GroundSpeed ).toDouble(), 0 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Bearing ).toDouble(), 0 );

  info = connection.push( QStringLiteral( "$GPRMC,084111.185,A,6938.6531,N,01856.8527,E,0.16,2.00,220120,,,A*6E" ) );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Location ).value< QgsPointXY >(), QgsPointXY( 18.94754499999999808, 69.644218333333341779 ) );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Altitude ).toDouble(), 35 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::GroundSpeed ).toDouble(),  0.29632 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Bearing ).toDouble(), 2 );
}

void TestQgsNmeaConnection::testIncompleteMessage()
{
  ReplayNmeaConnection connection;
  QSignalSpy stateChangedSpy( &connection, &QgsNmeaConnection::stateChanged );

  QCOMPARE( connection.status(), QgsGpsConnection::Status::Connected );

  // start with an incomplete message
  connection.pushString( QStringLiteral( "$GPGGA," ) );
  // status should be "data received", we don't have the full sentence yet
  QCOMPARE( connection.status(), QgsGpsConnection::Status::DataReceived );
  // should be no stateChanged signal yet, we are still waiting on more data
  QCOMPARE( stateChangedSpy.size(), 0 );

  connection.pushString( QStringLiteral( "084112.185,6900.0,N,01800.0,E,1,04,1.4,35.0,M,29.4,M,,0000*63\r\n" ) );
  // got a full sentence now, status should be "data received"
  QCOMPARE( connection.status(), QgsGpsConnection::Status::GPSDataReceived );
  QCOMPARE( stateChangedSpy.size(), 1 );
  const QgsGpsInformation info = stateChangedSpy.at( 0 ).at( 0 ).value< QgsGpsInformation >();

  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Location ).value< QgsPointXY >(), QgsPointXY( 18, 69 ) );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Altitude ).toDouble(), 35 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::GroundSpeed ).toDouble(), 0 );
  QCOMPARE( info.componentValue( Qgis::GpsInformationComponent::Bearing ).toDouble(), 0 );
}

QGSTEST_MAIN( TestQgsNmeaConnection )
#include "testqgsnmeaconnection.moc"
