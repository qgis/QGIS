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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QSharedPointer>

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
  QgsStatisticalSummary s( QgsStatisticalSummary::All );
  QList<double> values;
  values << 4 << 2 << 3 << 2 << 5 << 8;
  s.calculate( values );

  QCOMPARE( s.count(), 6 );
  QCOMPARE( s.sum(), 24.0 );
  QCOMPARE( s.mean(), 4.0 );
  QVERIFY( qgsDoubleNear( s.stDev(), 2.0816, 0.0001 ) );
  QVERIFY( qgsDoubleNear( s.sampleStDev(), 2.2803, 0.0001 ) );

  QCOMPARE( s.min(), 2.0 );
  QCOMPARE( s.max(), 8.0 );
  QCOMPARE( s.range(), 6.0 );

  QCOMPARE( s.median(), 3.5 );
  values << 9;
  s.calculate( values );
  QCOMPARE( s.median(), 4.0 );

  values << 4 << 5 << 8 << 12 << 12 << 12;
  s.calculate( values );
  QCOMPARE( s.variety(), 7 );
  QCOMPARE( s.minority(), 3.0 );
  QCOMPARE( s.majority(), 12.0 );
}

QTEST_MAIN( TestQgsStatisticSummary )
#include "testqgsstatisticalsummary.moc"
