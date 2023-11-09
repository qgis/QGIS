/***************************************************************************
     testqgsrange.cpp
     -------------------
    Date                 : March 2021
    Copyright            : (C) 2021 Nyall Dawson
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

#include "qgsrange.h"

class TestQgsRange: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testMergeRangesDate();
    void testMergeRangesDateTime();

  private:

};

void TestQgsRange::initTestCase()
{
}

void TestQgsRange::cleanupTestCase()
{
}

void TestQgsRange::init()
{

}

void TestQgsRange::cleanup()
{

}

void TestQgsRange::testMergeRangesDate()
{
  QList< QgsDateRange > res = QgsDateRange::mergeRanges( {} );
  QVERIFY( res.empty() );

  res = QgsDateRange::mergeRanges( {QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) )} );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 10 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 15 ) );

  res = QgsDateRange::mergeRanges( {QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) ),
                                    QgsDateRange( QDate( 2020, 1, 19 ), QDate( 2020, 1, 22 ) )} );
  QCOMPARE( res.size(), 2 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 10 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 15 ) );
  QCOMPARE( res.at( 1 ).begin(), QDate( 2020, 1, 19 ) );
  QCOMPARE( res.at( 1 ).end(), QDate( 2020, 1, 22 ) );

  res = QgsDateRange::mergeRanges( {QgsDateRange( QDate( 2020, 1, 19 ), QDate( 2020, 1, 22 ) ),
                                    QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) )} );
  QCOMPARE( res.size(), 2 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 10 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 15 ) );
  QCOMPARE( res.at( 1 ).begin(), QDate( 2020, 1, 19 ) );
  QCOMPARE( res.at( 1 ).end(), QDate( 2020, 1, 22 ) );

  res = QgsDateRange::mergeRanges( {QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) ),
                                    QgsDateRange( QDate( 2020, 1, 12 ), QDate( 2020, 1, 22 ) )} );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 10 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 22 ) );

  res = QgsDateRange::mergeRanges( {QgsDateRange( QDate( 2020, 1, 12 ), QDate( 2020, 1, 22 ) ),
                                    QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) )} );
  QCOMPARE( res.size(), 1 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 10 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 22 ) );

  const QList< QgsDateRange > ranges { QgsDateRange( QDate( 2020, 1, 10 ), QDate( 2020, 1, 15 ) ),
                                       QgsDateRange( QDate( 2020, 1, 20 ), QDate( 2020, 1, 25 ) ),
                                       QgsDateRange( QDate( 2020, 1, 9 ), QDate( 2020, 1, 11 ) ),
                                       QgsDateRange( QDate( 2020, 1, 19 ), QDate( 2020, 1, 27 ) ),
                                       QgsDateRange( QDate( 2020, 1, 1 ), QDate( 2020, 1, 3 ) ) };

  res = QgsDateRange::mergeRanges( ranges );
  QCOMPARE( res.size(), 3 );
  QCOMPARE( res.at( 0 ).begin(), QDate( 2020, 1, 1 ) );
  QCOMPARE( res.at( 0 ).end(), QDate( 2020, 1, 3 ) );
  QCOMPARE( res.at( 1 ).begin(), QDate( 2020, 1, 9 ) );
  QCOMPARE( res.at( 1 ).end(), QDate( 2020, 1, 15 ) );
  QCOMPARE( res.at( 2 ).begin(), QDate( 2020, 1, 19 ) );
  QCOMPARE( res.at( 2 ).end(), QDate( 2020, 1, 27 ) );
}

void TestQgsRange::testMergeRangesDateTime()
{
  const QList< QgsDateTimeRange > ranges { QgsDateTimeRange( QDateTime( QDate( 2020, 1, 10 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 1, 15 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange( QDateTime( QDate( 2020, 1, 20 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 1, 25 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange( QDateTime( QDate( 2020, 1, 9 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 1, 11 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange( QDateTime( QDate( 2020, 1, 19 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 1, 27 ), QTime( 0, 0, 0 ) ) ),
      QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) ) };

  const QList< QgsDateTimeRange > res = QgsDateTimeRange::mergeRanges( ranges );
  QCOMPARE( res.size(), 3 );
  QCOMPARE( res.at( 0 ).begin(), QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ) );
  QCOMPARE( res.at( 0 ).end(), QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ) ) );
  QCOMPARE( res.at( 1 ).begin(), QDateTime( QDate( 2020, 1, 9 ), QTime( 0, 0, 0 ) ) );
  QCOMPARE( res.at( 1 ).end(), QDateTime( QDate( 2020, 1, 15 ), QTime( 0, 0, 0 ) ) );
  QCOMPARE( res.at( 2 ).begin(), QDateTime( QDate( 2020, 1, 19 ), QTime( 0, 0, 0 ) ) );
  QCOMPARE( res.at( 2 ).end(), QDateTime( QDate( 2020, 1, 27 ), QTime( 0, 0, 0 ) ) );
}


QGSTEST_MAIN( TestQgsRange )
#include "testqgsrange.moc"
