/***************************************************************************
                         testqgstemporalutils.cpp
                         ---------------
    begin                : November 2026
    copyright            : (C) 2026 by Jan Caha
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstemporalutils.h"
#include "qgstest.h"

#include <QObject>
#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for the QgsNetCdfGribTimeParser class.
 */
class TestQgsTemporalUtils : public QObject
{
    Q_OBJECT

  private slots:
    void parseCfTime_data();
    void parseCfTime();
    void parseCfTimeInvalid_data();
    void parseCfTimeInvalid();
    void parseGribTime_data();
    void parseGribTime();
    void timeStepFixed();
    void timeStepUnsortedWithDuplicates();
    void timeStepIrregular();
    void timeStepMonthly();
    void timeStepYearly();
    void timeStepNotEnoughValues();
};

static QDateTime utc( int year, int month, int day, int hour = 0, int minute = 0 )
{
  return QDateTime( QDate( year, month, day ), QTime( hour, minute ), Qt::UTC );
}

void TestQgsTemporalUtils::parseCfTime_data()
{
  QTest::addColumn<double>( "value" );
  QTest::addColumn<QString>( "units" );
  QTest::addColumn<QString>( "calendar" );
  QTest::addColumn<QDateTime>( "expected" );

  QTest::newRow( "days" ) << 43464.0 << u"days since 1900-01-01 00:00:00"_s << u"standard"_s << utc( 2019, 1, 1 );
  QTest::newRow( "fractional days" ) << 43465.5 << u"days since 1900-01-01 00:00:00"_s << u"standard"_s << utc( 2019, 1, 2, 12 );
  QTest::newRow( "reference without time" ) << 10.0 << u"days since 2024-01-01"_s << u"gregorian"_s << utc( 2024, 1, 11 );
  QTest::newRow( "no calendar" ) << 10.0 << u"days since 2024-01-01"_s << QString() << utc( 2024, 1, 11 );
  QTest::newRow( "hours" ) << 36.0 << u"hours since 2023-01-01 00:00:00"_s << QString() << utc( 2023, 1, 2, 12 );
  QTest::newRow( "minutes" ) << 90.0 << u"minutes since 2023-01-01 06:00:00"_s << QString() << utc( 2023, 1, 1, 7, 30 );
  QTest::newRow( "seconds" ) << 3600.0 << u"seconds since 1970-01-01 00:00:00"_s << QString() << utc( 1970, 1, 1, 1 );
  QTest::newRow( "fractional seconds rounded down" ) << 1.6 << u"seconds since 2020-01-01 00:00:00"_s << QString() << QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 1 ), Qt::UTC );
  QTest::newRow( "weeks" ) << 2.0 << u"weeks since 2024-01-01"_s << QString() << utc( 2024, 1, 15 );
  QTest::newRow( "months" ) << 1.0 << u"months since 2020-01-01"_s << QString() << QDateTime( QDate( 2020, 1, 31 ), QTime( 10, 29, 3 ), Qt::UTC );
  QTest::newRow( "unknown unit defaults to hours" ) << 2.0 << u"foo since 2023-01-01 00:00:00"_s << QString() << utc( 2023, 1, 1, 2 );
  QTest::newRow( "reference with time" ) << 1.0 << u"days since 2023-01-01 12:30:00"_s << QString() << utc( 2023, 1, 2, 12, 30 );
  QTest::newRow( "negative value" ) << -1.0 << u"days since 2023-01-01"_s << QString() << utc( 2022, 12, 31 );
  QTest::newRow( "proleptic gregorian" ) << 1.0 << u"days since 2024-02-28"_s << u"proleptic_gregorian"_s << utc( 2024, 2, 29 );
  QTest::newRow( "julian" ) << 0.0 << u"days since 2000-01-01"_s << u"julian"_s << utc( 2000, 1, 14 );
  QTest::newRow( "gregorian switch" ) << 1.0 << u"days since 1582-10-04"_s << u"gregorian"_s << utc( 1582, 10, 15 );
}

void TestQgsTemporalUtils::parseCfTime()
{
  QFETCH( double, value );
  QFETCH( QString, units );
  QFETCH( QString, calendar );
  QFETCH( QDateTime, expected );

  const QDateTime result = QgsNetCdfGribTimeParser::parseCfTime( value, units, calendar );
  QVERIFY( result.isValid() );
  QCOMPARE( result, expected );
  QCOMPARE( result.timeSpec(), Qt::UTC );
}

void TestQgsTemporalUtils::parseCfTimeInvalid_data()
{
  QTest::addColumn<double>( "value" );
  QTest::addColumn<QString>( "units" );
  QTest::addColumn<QString>( "calendar" );

  QTest::newRow( "completely empty units" ) << 1.0 << QString() << QString();
  QTest::newRow( "no reference date" ) << 1.0 << u"days"_s << QString();
  QTest::newRow( "missing since" ) << 1.0 << u"days after 2024-01-01"_s << QString();
  QTest::newRow( "wrong date format" ) << 1.0 << u"days since 2024/01/01"_s << QString();
  QTest::newRow( "calendar not supported" ) << 1.0 << u"days since 2050-01-01"_s << u"360_day"_s;
  QTest::newRow( "non positive year" ) << -400.0 << u"days since 0001-01-01"_s << QString();
}

void TestQgsTemporalUtils::parseCfTimeInvalid()
{
  QFETCH( double, value );
  QFETCH( QString, units );
  QFETCH( QString, calendar );

  QVERIFY( !QgsNetCdfGribTimeParser::parseCfTime( value, units, calendar ).isValid() );
}

void TestQgsTemporalUtils::parseGribTime_data()
{
  QTest::addColumn<QString>( "metadataValue" );
  QTest::addColumn<QDateTime>( "expected" );

  // GDAL < 3.12 format
  QTest::newRow( "sec UTC" ) << u"1546300800 sec UTC"_s << utc( 2019, 1, 1 );
  // GDAL >= 3.12 format
  QTest::newRow( "plain number" ) << u"1672531200"_s << utc( 2023, 1, 1 );
  QTest::newRow( "hours" ) << u"1672552800 sec UTC"_s << utc( 2023, 1, 1, 6 );
  QTest::newRow( "whitespace are skipped" ) << u"  1672531200   sec UTC  "_s << utc( 2023, 1, 1 );
  QTest::newRow( "epoch" ) << u"0 sec UTC"_s << utc( 1970, 1, 1 );
  QTest::newRow( "empty" ) << QString() << QDateTime();
  QTest::newRow( "whitespace" ) << u"   "_s << QDateTime();
}

void TestQgsTemporalUtils::parseGribTime()
{
  QFETCH( QString, metadataValue );
  QFETCH( QDateTime, expected );

  const QDateTime result = QgsNetCdfGribTimeParser::parseGribTime( metadataValue );
  QCOMPARE( result.isValid(), expected.isValid() );
  if ( expected.isValid() )
  {
    QCOMPARE( result, expected );
    QCOMPARE( result.timeSpec(), Qt::UTC );
  }
}

void TestQgsTemporalUtils::timeStepFixed()
{
  QList<QDateTime> dateTimes;
  for ( int i = 0; i < 5; ++i )
    dateTimes << QgsNetCdfGribTimeParser::parseCfTime( i * 6, u"hours since 2026-11-13 00:00:00"_s );

  QCOMPARE( QgsNetCdfGribTimeParser::calculateRegularTimeStep( dateTimes ).toString(), u"PT6H"_s );

  const QgsTimeDuration step = QgsNetCdfGribTimeParser::calculateRegularTimeStep( { utc( 2019, 1, 1 ), utc( 2019, 1, 2, 12 ), utc( 2019, 1, 4 ) } );
  QCOMPARE( step.days, 1 );
  QCOMPARE( step.hours, 12 );
  QCOMPARE( step.minutes, 0 );
  QCOMPARE( step.seconds, 0.0 );
}

void TestQgsTemporalUtils::timeStepUnsortedWithDuplicates()
{
  const QDateTime t0 = utc( 2024, 1, 1 );
  const QDateTime t1 = utc( 2024, 1, 2 );
  const QDateTime t2 = utc( 2024, 1, 3 );

  QList<QDateTime> unsortedWithDuplicates;
  unsortedWithDuplicates << t2 << t0 << t0 << t1 << QDateTime() << t1 << t2;

  QCOMPARE( QgsNetCdfGribTimeParser::calculateRegularTimeStep( unsortedWithDuplicates ).toString(), u"P1D"_s );
}

void TestQgsTemporalUtils::timeStepIrregular()
{
  // day frequence but missed date
  QList<QDateTime> irregularDates;
  irregularDates << utc( 2024, 1, 1 ) << utc( 2024, 1, 2 ) << utc( 2024, 1, 4 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( irregularDates ).isNull() );

  // monthly frequency but irregular dates
  QList<QDateTime> monthlyDates;
  monthlyDates << utc( 2023, 1, 16, 12 ) << utc( 2023, 2, 15, 12 ) << utc( 2023, 3, 16, 12 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( monthlyDates ).isNull() );

  // same day of month, but a month is skipped
  QList<QDateTime> skippedMonthDates;
  skippedMonthDates << utc( 2023, 1, 15 ) << utc( 2023, 2, 15 ) << utc( 2023, 4, 15 ) << utc( 2023, 5, 15 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( skippedMonthDates ).isNull() );

  // same day of month, but different time of day
  QList<QDateTime> differentTimeOfDayDates;
  differentTimeOfDayDates << utc( 2023, 1, 1 ) << utc( 2023, 2, 1, 12 ) << utc( 2023, 3, 1, 6 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( differentTimeOfDayDates ).isNull() );

  // 90 days apart, not detected
  QList<QDateTime> quarterlyDates;
  quarterlyDates << utc( 2024, 1, 1 ) << utc( 2024, 4, 1 ) << utc( 2024, 7, 1 ) << utc( 2024, 10, 1 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( quarterlyDates ).isNull() );
}

void TestQgsTemporalUtils::timeStepMonthly()
{
  QList<QDateTime> dateTimes;
  for ( const double days : { 0.0, 31.0, 60.0, 91.0, 121.0 } )
    dateTimes << QgsNetCdfGribTimeParser::parseCfTime( days, u"days since 2024-01-01"_s, u"gregorian"_s );

  QCOMPARE( QgsNetCdfGribTimeParser::calculateRegularTimeStep( dateTimes ).toString(), u"P1M"_s );

  // middle of the month, with time, across a year boundary
  QList<QDateTime> middleOfMonthDates;
  middleOfMonthDates << utc( 2023, 11, 15, 12 ) << utc( 2023, 12, 15, 12 ) << utc( 2024, 1, 15, 12 ) << utc( 2024, 2, 15, 12 );
  QCOMPARE( QgsNetCdfGribTimeParser::calculateRegularTimeStep( middleOfMonthDates ).toString(), u"P1M"_s );
}

void TestQgsTemporalUtils::timeStepYearly()
{
  // 2024 is a leap year, so the steps in days differ (366, 365)
  QList<QDateTime> yearlyDates;
  yearlyDates << utc( 2024, 1, 1 ) << utc( 2025, 1, 1 ) << utc( 2026, 1, 1 );
  const QgsTimeDuration step = QgsNetCdfGribTimeParser::calculateRegularTimeStep( yearlyDates );
  QCOMPARE( step.years, 1 );
  QCOMPARE( step.months, 0 );
  QCOMPARE( step.toString(), u"P1Y"_s );

  // make irregular step
  yearlyDates << utc( 2028, 1, 1 );
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( yearlyDates ).isNull() );
}

void TestQgsTemporalUtils::timeStepNotEnoughValues()
{
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( {} ).isNull() );

  const QDateTime t0 = utc( 2024, 1, 1 );
  QList<QDateTime> dates;

  // one value, not enough to determine a step
  dates << t0;
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( dates ).isNull() );

  // two values, but only one unique value
  dates << t0;
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( dates ).isNull() );

  // adding an invalid date time should not affect the result
  dates << QDateTime();
  QVERIFY( QgsNetCdfGribTimeParser::calculateRegularTimeStep( dates ).isNull() );
}

QGSTEST_MAIN( TestQgsTemporalUtils )
#include "testqgstemporalutils.moc"
