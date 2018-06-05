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
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgszonalstatistics.h"
#include "qgsproject.h"
#include "qgsvectorlayerutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the zonal statistics class
 */
class TestQgsZonalStatistics : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init() {}
    void cleanup() {}

    void testStatistics();
    void testReprojection();
    void testNoData();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsRasterLayer *mRasterLayer = nullptr;
    QString mTempPath;
};

void TestQgsZonalStatistics::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataPath = myDataPath + "/zonalstatistics/";
  mTempPath = QDir::tempPath() + '/';

  // copy test data to temp directory
  QDir testDir( myTestDataPath );
  QStringList files = testDir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  for ( int i = 0; i < files.size(); ++i )
  {
    QFile::remove( mTempPath + files.at( i ) );
    QVERIFY( QFile::copy( myTestDataPath + files.at( i ), mTempPath + files.at( i ) ) );
  }

  mVectorLayer = new QgsVectorLayer( mTempPath + "polys.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) );
  mRasterLayer = new QgsRasterLayer( mTempPath + "edge_problem.asc", QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mVectorLayer << mRasterLayer );
}

void TestQgsZonalStatistics::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsZonalStatistics::testStatistics()
{
  QgsZonalStatistics zs( mVectorLayer, mRasterLayer, QLatin1String( "" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterFid( 0 );
  bool fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.13888888888889 );

  // same with long prefix to ensure that field name truncation handled correctly
  QgsZonalStatistics zsl( mVectorLayer, mRasterLayer, QStringLiteral( "myqgis2_" ), 1, QgsZonalStatistics::All );
  zsl.calculateStatistics( nullptr );

  request.setFilterFid( 0 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( "myqgis2__1" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_st" ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( "myqgis2_mi" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2_ma" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_ra" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2__2" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2__3" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_va" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "myqgis2__4" ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( "myqgis2__1" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_st" ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( "myqgis2_mi" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2_ma" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_ra" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2__2" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2__3" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_va" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "myqgis2__4" ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "myqgis2_co" ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( "myqgis2_su" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "myqgis2_me" ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( "myqgis2__1" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_st" ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( "myqgis2_mi" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2_ma" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_ra" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2__2" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "myqgis2__3" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "myqgis2_va" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "myqgis2__4" ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testReprojection()
{
  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // create a reprojected version of the layer
  std::unique_ptr< QgsVectorLayer > vectorLayer( new QgsVectorLayer( myTestDataPath + "polys.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) ) );
  std::unique_ptr< QgsVectorLayer > reprojected( vectorLayer->materialize( QgsFeatureRequest().setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3785" ) ), QgsProject::instance()->transformContext() ) ) );

  QCOMPARE( reprojected->featureCount(), vectorLayer->featureCount() );
  QgsZonalStatistics zs( reprojected.get(), mRasterLayer, QString(), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  QgsFeatureRequest request;
  QgsFeatureIterator it = reprojected->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.222222222222222 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.24691358024691 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "count" ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( "sum" ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( "mean" ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( "median" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "stdev" ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( "min" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "max" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "range" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "minority" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "majority" ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( "variety" ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( "variance" ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testNoData()
{
  QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // test that zonal stats respects no data and user set no data values
  std::unique_ptr< QgsRasterLayer > rasterLayer = qgis::make_unique< QgsRasterLayer >( myTestDataPath + "raster.tif", QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  std::unique_ptr< QgsVectorLayer > vectorLayer = qgis::make_unique< QgsVectorLayer >( mTempPath + "polys2.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) );

  QgsZonalStatistics zs( vectorLayer.get(), rasterLayer.get(), QStringLiteral( "n" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  QgsFeatureRequest request;
  QgsFeatureIterator it = vectorLayer->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "ncount" ).toDouble(), 16.0 );
  QCOMPARE( f.attribute( "nsum" ).toDouble(), 13428.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "ncount" ).toDouble(), 103.0 );
  QCOMPARE( f.attribute( "nsum" ).toDouble(), 90536.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "ncount" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "nsum" ).toDouble(), 0.0 );

  // with user no data
  rasterLayer->dataProvider()->setUserNoDataValue( 1, QgsRasterRangeList() << QgsRasterRange( 842, 852 )
      << QgsRasterRange( 877, 891 ) );

  zs = QgsZonalStatistics( vectorLayer.get(), rasterLayer.get(), QStringLiteral( "un" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  it = vectorLayer->getFeatures( request );
  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "uncount" ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( "unsum" ).toDouble(), 6652.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "uncount" ).toDouble(), 52.0 );
  QCOMPARE( f.attribute( "unsum" ).toDouble(), 45374.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( "uncount" ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( "unsum" ).toDouble(), 0.0 );
}

QGSTEST_MAIN( TestQgsZonalStatistics )
#include "testqgszonalstatistics.moc"
