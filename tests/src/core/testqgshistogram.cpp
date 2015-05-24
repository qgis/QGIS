/***************************************************************************
     testqgshistogram.cpp
     --------------------
    Date                 : May 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDir>
#include <QtTest/QtTest>

#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgshistogram.h"

/** \ingroup UnitTests
 * This is a unit test for QgsHistogram
 */
class TestQgsHistogram : public QObject
{
    Q_OBJECT

  public:
    TestQgsHistogram();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {}
    void cleanup() {}
    void optimalBinWidth();
    void optimalBinCount();
    void binEdges();
    void counts();
    void fromLayer();

  private:

};

TestQgsHistogram::TestQgsHistogram()
{

}

void TestQgsHistogram::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsHistogram::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsHistogram::optimalBinWidth()
{
  QList<double> vals;
  vals << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;

  QgsHistogram h;
  h.setValues( vals );
  QVERIFY( qgsDoubleNear( h.optimalBinWidth(), 4.641, 0.001 ) );
}

void TestQgsHistogram::optimalBinCount()
{
  QList<double> vals;
  vals << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;

  QgsHistogram h;
  h.setValues( vals );
  QCOMPARE( h.optimalNumberBins(), 2 );
}

void TestQgsHistogram::binEdges()
{
  QList<double> vals;
  vals << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;

  QgsHistogram h;
  h.setValues( vals );
  QList<double> edges = h.binEdges( 3 );
  QCOMPARE( edges.count(), 4 );
  QCOMPARE( edges.at( 0 ), 1.0 );
  QCOMPARE( edges.at( 1 ), 4.0 );
  QCOMPARE( edges.at( 2 ), 7.0 );
  QCOMPARE( edges.at( 3 ), 10.0 );
}

void TestQgsHistogram::counts()
{
  QList<double> vals;
  vals << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10;

  QgsHistogram h;
  h.setValues( vals );
  QList<int> counts = h.counts( 1 );
  QList<int> expected;
  expected << 10;
  QCOMPARE( counts, expected );

  counts = h.counts( 2 );
  expected.clear();
  expected << 5 << 5;
  QCOMPARE( counts, expected );

  counts = h.counts( 5 );
  expected.clear();
  expected << 2 << 2 << 2 << 2 << 2;
  QCOMPARE( counts, expected );

  counts = h.counts( 20 );
  expected.clear();
  expected << 1 << 0 << 1 << 0 << 1 << 0 << 1 << 0 << 1 << 0 << 0 << 1 << 0 << 1 << 0 << 1 << 0 << 1 << 0 << 1;
  QCOMPARE( counts, expected );
}

void TestQgsHistogram::fromLayer()
{
  QgsHistogram h;

  QVERIFY( !h.setValues( 0, QString() ) );

  QgsVectorLayer* layer = new QgsVectorLayer( "Point?field=col1:real", "layer", "memory" );
  QVERIFY( layer->isValid() );
  QgsFeatureList features;
  for ( int i = 1; i <= 10; ++i )
  {
    QgsFeature f( layer->dataProvider()->fields(), i );
    f.setAttribute( "col1", i );
    features << f;
  }
  layer->dataProvider()->addFeatures( features );

  QVERIFY( !h.setValues( layer, QString() ) );
  QVERIFY( h.setValues( layer, QString( "col1" ) ) );
  QList<int>counts = h.counts( 5 );
  QList<int> expected;
  expected << 2 << 2 << 2 << 2 << 2;
  QCOMPARE( counts, expected );

  delete layer;
}

QTEST_MAIN( TestQgsHistogram )
#include "testqgshistogram.moc"
