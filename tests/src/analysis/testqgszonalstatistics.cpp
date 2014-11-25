/***************************************************************************
     testqgszonalstatistics.cpp
     --------------------------------------
    Date                 : 15 Jul 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
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
#include "qgszonalstatistics.h"

/** \ingroup UnitTests
 * This is a unit test for the zonal statistics class
 */
class TestQgsZonalStatistics: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {};
    void cleanup() {};

    void testStatistics();

  private:
    QgsVectorLayer* mVectorLayer;
    QString mRasterPath;
};

void TestQgsZonalStatistics::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataPath = myDataPath + QDir::separator() + "zonalstatistics" + QDir::separator();
  QString myTempPath = QDir::tempPath() + QDir::separator();

  // copy test data to temp directory
  QDir testDir( myTestDataPath );
  QStringList files = testDir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0; i < files.size(); ++i )
  {
    QFile::remove( myTempPath + files.at( i ) );
    QVERIFY( QFile::copy( myTestDataPath + files.at( i ), myTempPath + files.at( i ) ) );
  }

  mVectorLayer = new QgsVectorLayer( myTempPath + "polys.shp", "poly", "ogr" );
  mRasterPath = myTempPath + "edge_problem.asc";
}

void TestQgsZonalStatistics::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsZonalStatistics::testStatistics()
{
  QgsZonalStatistics zs( mVectorLayer, mRasterPath, "", 1 );
  zs.calculateStatistics( NULL );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterFid( 0 );
  bool fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.666666666666667 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.555555555555556 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.833333333333333 );

  // same with long prefix to ensure that field name truncation handled correctly
  QgsZonalStatistics zsl( mVectorLayer, mRasterPath, "myqgis2_", 1 );
  zsl.calculateStatistics( NULL );

  request.setFilterFid( 0 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.666666666666667 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.555555555555556 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.833333333333333 );
}

QTEST_MAIN( TestQgsZonalStatistics )
#include "testqgszonalstatistics.moc"
