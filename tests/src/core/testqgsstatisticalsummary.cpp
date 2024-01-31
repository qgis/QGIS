/***************************************************************************
     testqgsstatisticalsummary.cpp
     -----------------------------
    Date                 : May 2015
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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

#include "qgsstatisticalsummary.h"
#include "qgis.h"

class TestQgsStatisticSummary: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void stats();
    void individualStatCalculations_data();
    void individualStatCalculations();
    void maxMin();
    void countMissing();
    void noValues();
    void shortName();

  private:

};

void TestQgsStatisticSummary::initTestCase()
{

}

void TestQgsStatisticSummary::cleanupTestCase()
{

}

void TestQgsStatisticSummary::init()
{

}

void TestQgsStatisticSummary::cleanup()
{

}

void TestQgsStatisticSummary::stats()
{
  //note - we test everything twice, once using the statistics calculated by passing
  //a list of values and once using the statistics calculated by passing values
  //one-at-a-time
  QgsStatisticalSummary s( QgsStatisticalSummary::Statistic::All );
  QgsStatisticalSummary s2( QgsStatisticalSummary::Statistic::All );
  QList<double> values;
  values << 4 << 2 << 3 << 2 << 5 << 8;
  s.calculate( values );
  s2.addValue( 4 );
  s2.addValue( 2 );
  s2.addValue( 3 );
  s2.addValue( 2 );
  s2.addValue( 5 );
  s2.addValue( 8 );
  s2.finalize();

  QCOMPARE( s.count(), 6 );
  QCOMPARE( s2.count(), 6 );
  QCOMPARE( s.sum(), 24.0 );
  QCOMPARE( s2.sum(), 24.0 );
  QCOMPARE( s.mean(), 4.0 );
  QCOMPARE( s2.mean(), 4.0 );
  QCOMPARE( s.first(), 4.0 );
  QCOMPARE( s2.first(), 4.0 );
  QCOMPARE( s.last(), 8.0 );
  QCOMPARE( s2.last(), 8.0 );
  QGSCOMPARENEAR( s.stDev(), 2.0816, 0.0001 );
  QGSCOMPARENEAR( s2.stDev(), 2.0816, 0.0001 );
  QGSCOMPARENEAR( s.sampleStDev(), 2.2803, 0.0001 );
  QGSCOMPARENEAR( s2.sampleStDev(), 2.2803, 0.0001 );

  QCOMPARE( s.min(), 2.0 );
  QCOMPARE( s2.min(), 2.0 );
  QCOMPARE( s.max(), 8.0 );
  QCOMPARE( s2.max(), 8.0 );
  QCOMPARE( s.range(), 6.0 );
  QCOMPARE( s2.range(), 6.0 );

  QCOMPARE( s.median(), 3.5 );
  QCOMPARE( s2.median(), 3.5 );
  values << 9;
  s.calculate( values );
  s2.addValue( 9 );
  s2.finalize();
  QCOMPARE( s.median(), 4.0 );
  QCOMPARE( s2.median(), 4.0 );

  values << 4 << 5 << 8 << 12 << 12 << 12;
  s.calculate( values );
  s2.addValue( 4 );
  s2.addValue( 5 ) ;
  s2.addValue( 8 );
  s2.addValue( 12 );
  s2.addValue( 12 );
  s2.addValue( 12 );
  s2.finalize();
  QCOMPARE( s.variety(), 7 );
  QCOMPARE( s2.variety(), 7 );
  QCOMPARE( s.minority(), 3.0 );
  QCOMPARE( s2.minority(), 3.0 );
  QCOMPARE( s.majority(), 12.0 );
  QCOMPARE( s2.majority(), 12.0 );

  //test quartiles. lots of possibilities here, involving odd/even/divisible by 4 counts
  values.clear();
  values << 7 << 15 << 36 << 39 << 40 << 41;
  s.calculate( values );
  s2.reset();
  s2.addValue( 7 );
  s2.addValue( 15 );
  s2.addValue( 36 );
  s2.addValue( 39 );
  s2.addValue( 40 );
  s2.addValue( 41 );
  s2.finalize();
  QCOMPARE( s.median(), 37.5 );
  QCOMPARE( s2.median(), 37.5 );
  QCOMPARE( s.firstQuartile(), 15.0 );
  QCOMPARE( s2.firstQuartile(), 15.0 );
  QCOMPARE( s.thirdQuartile(), 40.0 );
  QCOMPARE( s2.thirdQuartile(), 40.0 );
  QCOMPARE( s.interQuartileRange(), 25.0 );
  QCOMPARE( s2.interQuartileRange(), 25.0 );

  values.clear();
  values << 7 << 15 << 36 << 39 << 40 << 41 << 43 << 49;
  s.calculate( values );
  s2.reset();
  s2.addValue( 7 );
  s2.addValue( 15 );
  s2.addValue( 36 );
  s2.addValue( 39 );
  s2.addValue( 40 );
  s2.addValue( 41 ) ;
  s2.addValue( 43 );
  s2.addValue( 49 );
  s2.finalize();
  QCOMPARE( s.median(), 39.5 );
  QCOMPARE( s2.median(), 39.5 );
  QCOMPARE( s.firstQuartile(), 25.5 );
  QCOMPARE( s2.firstQuartile(), 25.5 );
  QCOMPARE( s.thirdQuartile(), 42.0 );
  QCOMPARE( s2.thirdQuartile(), 42.0 );
  QCOMPARE( s.interQuartileRange(), 16.5 );
  QCOMPARE( s2.interQuartileRange(), 16.5 );

  values.clear();
  values << 6 << 7 << 15 << 36 << 39 << 40 << 41 << 42 << 43 << 47 << 49;
  s.calculate( values );
  s2.reset();
  s2.addValue( 6 );
  s2.addValue( 7 );
  s2.addValue( 15 );
  s2.addValue( 36 );
  s2.addValue( 39 );
  s2.addValue( 40 );
  s2.addValue( 41 );
  s2.addValue( 42 );
  s2.addValue( 43 );
  s2.addValue( 47 );
  s2.addValue( 49 );
  s2.finalize();
  QCOMPARE( s.median(), 40.0 );
  QCOMPARE( s2.median(), 40.0 );
  QCOMPARE( s.firstQuartile(), 25.5 );
  QCOMPARE( s2.firstQuartile(), 25.5 );
  QCOMPARE( s.thirdQuartile(), 42.5 );
  QCOMPARE( s2.thirdQuartile(), 42.5 );
  QCOMPARE( s.interQuartileRange(), 17.0 );
  QCOMPARE( s2.interQuartileRange(), 17.0 );

  values.clear();
  values << 6 << 7 << 15 << 36 << 39 << 40 << 41 << 42 << 43 << 47 << 49 << 50 << 58;
  s.calculate( values );
  s2.addValue( 50 );
  s2.addValue( 58 );
  s2.finalize();
  QCOMPARE( s.median(), 41.0 );
  QCOMPARE( s2.median(), 41.0 );
  QCOMPARE( s.firstQuartile(), 36.0 );
  QCOMPARE( s2.firstQuartile(), 36.0 );
  QCOMPARE( s.thirdQuartile(), 47.0 );
  QCOMPARE( s2.thirdQuartile(), 47.0 );
  QCOMPARE( s.interQuartileRange(), 11.0 );
  QCOMPARE( s2.interQuartileRange(), 11.0 );
}

void TestQgsStatisticSummary::individualStatCalculations_data()
{
  QTest::addColumn< int >( "statInt" );
  QTest::addColumn<double>( "expected" );

  QTest::newRow( "count" ) << ( int )QgsStatisticalSummary::Statistic::Count << 10.0;
  QTest::newRow( "sum" ) << ( int )QgsStatisticalSummary::Statistic::Sum << 45.0;
  QTest::newRow( "mean" ) << ( int )QgsStatisticalSummary::Statistic::Mean << 4.5;
  QTest::newRow( "median" ) << ( int )QgsStatisticalSummary::Statistic::Median << 4.0;
  QTest::newRow( "st_dev" ) << ( int )QgsStatisticalSummary::Statistic::StDev << 1.96214;
  QTest::newRow( "st_dev_sample" ) << ( int )QgsStatisticalSummary::Statistic::StDevSample << 2.06828;
  QTest::newRow( "min" ) << ( int )QgsStatisticalSummary::Statistic::Min << 2.0;
  QTest::newRow( "max" ) << ( int )QgsStatisticalSummary::Statistic::Max << 8.0;
  QTest::newRow( "range" ) << ( int )QgsStatisticalSummary::Statistic::Range << 6.0;
  QTest::newRow( "minority" ) << ( int )QgsStatisticalSummary::Statistic::Minority << 2.0;
  QTest::newRow( "majority" ) << ( int )QgsStatisticalSummary::Statistic::Majority << 3.0;
  QTest::newRow( "variety" ) << ( int )QgsStatisticalSummary::Statistic::Variety << 5.0;
  QTest::newRow( "first_quartile" ) << ( int )QgsStatisticalSummary::Statistic::FirstQuartile << 3.0;
  QTest::newRow( "third_quartile" ) << ( int )QgsStatisticalSummary::Statistic::ThirdQuartile << 5.0;
  QTest::newRow( "iqr" ) << ( int )QgsStatisticalSummary::Statistic::InterQuartileRange << 2.0;
  QTest::newRow( "missing" ) << ( int )QgsStatisticalSummary::Statistic::CountMissing << 0.0;
  QTest::newRow( "first" ) << static_cast< int >( QgsStatisticalSummary::Statistic::First ) << 4.0;
  QTest::newRow( "last" ) << static_cast< int >( QgsStatisticalSummary::Statistic::Last ) << 8.0;
}

void TestQgsStatisticSummary::individualStatCalculations()
{
  //tests calculation of statistics one at a time, to make sure statistic calculations are not
  //dependent on each other

  QList<double> values;
  values << 4 << 4 << 2 << 3 << 3 << 3 << 5 << 5 << 8 << 8;

  QFETCH( int, statInt );
  const QgsStatisticalSummary::Statistic stat = ( QgsStatisticalSummary::Statistic ) statInt;
  QFETCH( double, expected );

  //start with a summary which calculates NO statistics
  QgsStatisticalSummary s{ QgsStatisticalSummary::Statistics() };
  //set it to calculate just a single statistic
  s.setStatistics( stat );
  QCOMPARE( s.statistics(), stat );

  s.calculate( values );
  QGSCOMPARENEAR( s.statistic( stat ), expected, 0.00001 );

  //also test using values added one-at-a-time
  QgsStatisticalSummary s2{ QgsStatisticalSummary::Statistics() };
  s2.setStatistics( stat );
  s2.addValue( 4 );
  s2.addValue( 4 );
  s2.addValue( 2 );
  s2.addValue( 3 );
  s2.addValue( 3 );
  s2.addValue( 3 ) ;
  s2.addValue( 5 ) ;
  s2.addValue( 5 ) ;
  s2.addValue( 8 );
  s2.addValue( 8 );
  s2.finalize();
  QCOMPARE( s2.statistics(), stat );

  //make sure stat has a valid display name
  QVERIFY( !QgsStatisticalSummary::displayName( stat ).isEmpty() );
}

void TestQgsStatisticSummary::maxMin()
{
  QgsStatisticalSummary s( QgsStatisticalSummary::Statistic::All );

  //test max/min of negative value list
  QList<double> negativeVals;
  negativeVals << -5.0 << -10.0 << -15.0;
  s.calculate( negativeVals );

  QCOMPARE( s.min(), -15.0 );
  QCOMPARE( s.max(), -5.0 );
}

void TestQgsStatisticSummary::countMissing()
{
  QgsStatisticalSummary s( QgsStatisticalSummary::Statistic::All );
  s.addVariant( 5 );
  s.addVariant( 6 );
  s.addVariant( QVariant() );
  s.addVariant( 7 );
  s.addVariant( QVariant( QVariant::Double ) );
  s.addVariant( 9 );
  s.addVariant( "Asdasdsad" );
  s.finalize();

  QCOMPARE( s.countMissing(), 3 );
  QCOMPARE( s.statistic( QgsStatisticalSummary::Statistic::CountMissing ),  3.0 );
}

void TestQgsStatisticSummary::noValues()
{
  // test returned stats when no values present
  QgsStatisticalSummary s( QgsStatisticalSummary::Statistic::All );
  s.finalize();

  QCOMPARE( s.count(), 0 );
  QCOMPARE( s.statistic( QgsStatisticalSummary::Statistic::Count ), 0.0 );
  QCOMPARE( s.countMissing(), 0 );
  QCOMPARE( s.statistic( QgsStatisticalSummary::Statistic::CountMissing ), 0.0 );
  QCOMPARE( s.sum(), 0.0 );
  QCOMPARE( s.statistic( QgsStatisticalSummary::Statistic::Sum ), 0.0 );
  QVERIFY( std::isnan( s.first() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::First ) ) );
  QVERIFY( std::isnan( s.last() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Last ) ) );
  QVERIFY( std::isnan( s.mean() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Mean ) ) );
  QVERIFY( std::isnan( s.median() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Median ) ) );
  QVERIFY( std::isnan( s.stDev() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::StDev ) ) );
  QVERIFY( std::isnan( s.sampleStDev() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::StDevSample ) ) );
  QVERIFY( std::isnan( s.min() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Min ) ) );
  QVERIFY( std::isnan( s.max() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Max ) ) );
  QVERIFY( std::isnan( s.range() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Range ) ) );
  QVERIFY( std::isnan( s.minority() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Minority ) ) );
  QVERIFY( std::isnan( s.majority() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::Majority ) ) );
  QCOMPARE( s.variety(), 0 );
  QCOMPARE( s.statistic( QgsStatisticalSummary::Statistic::Variety ), 0.0 );
  QVERIFY( std::isnan( s.firstQuartile() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::FirstQuartile ) ) );
  QVERIFY( std::isnan( s.thirdQuartile() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::ThirdQuartile ) ) );
  QVERIFY( std::isnan( s.interQuartileRange() ) );
  QVERIFY( std::isnan( s.statistic( QgsStatisticalSummary::Statistic::InterQuartileRange ) ) );
}

void TestQgsStatisticSummary::shortName()
{
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Count ), QStringLiteral( "count" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::CountMissing ), QStringLiteral( "countmissing" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Sum ), QStringLiteral( "sum" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Mean ), QStringLiteral( "mean" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Median ), QStringLiteral( "median" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::StDev ), QStringLiteral( "stdev" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::StDevSample ), QStringLiteral( "stdevsample" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Min ), QStringLiteral( "min" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Max ), QStringLiteral( "max" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Range ), QStringLiteral( "range" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Minority ), QStringLiteral( "minority" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Majority ), QStringLiteral( "majority" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Variety ), QStringLiteral( "variety" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::FirstQuartile ), QStringLiteral( "q1" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::ThirdQuartile ), QStringLiteral( "q3" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::InterQuartileRange ), QStringLiteral( "iqr" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::First ), QStringLiteral( "first" ) );
  QCOMPARE( QgsStatisticalSummary::shortName( QgsStatisticalSummary::Statistic::Last ), QStringLiteral( "last" ) );
}

QGSTEST_MAIN( TestQgsStatisticSummary )
#include "testqgsstatisticalsummary.moc"
