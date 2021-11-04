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
    void testSmallPolygons();
    void testShortName();

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

  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";
  mTempPath = QDir::tempPath() + '/';

  // copy test data to temp directory
  const QDir testDir( myTestDataPath );
  const QStringList files = testDir.entryList( QDir::Files | QDir::NoDotAndDotDot );
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
  QgsZonalStatistics zs( mVectorLayer, mRasterLayer, QString(), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterFid( 0 );
  bool fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.13888888888889 );

  // same with long prefix to ensure that field name truncation handled correctly
  QgsZonalStatistics zsl( mVectorLayer, mRasterLayer, QStringLiteral( "myqgis2_" ), 1, QgsZonalStatistics::All );
  zsl.calculateStatistics( nullptr );

  request.setFilterFid( 0 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_co" ) ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_su" ) ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_me" ) ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__1" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_st" ) ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_mi" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ma" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ra" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__2" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__3" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_va" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__4" ) ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_co" ) ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_su" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_me" ) ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__1" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_st" ) ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_mi" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ma" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ra" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__2" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__3" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_va" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__4" ) ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_co" ) ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_su" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_me" ) ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__1" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_st" ) ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_mi" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ma" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_ra" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__2" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__3" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2_va" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "myqgis2__4" ) ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testReprojection()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // create a reprojected version of the layer
  std::unique_ptr< QgsVectorLayer > vectorLayer( new QgsVectorLayer( myTestDataPath + "polys.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) ) );
  std::unique_ptr< QgsVectorLayer > reprojected( vectorLayer->materialize( QgsFeatureRequest().setDestinationCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3785" ) ), QgsProject::instance()->transformContext() ) ) );

  QCOMPARE( reprojected->featureCount(), vectorLayer->featureCount() );
  QgsZonalStatistics zs( reprojected.get(), mRasterLayer, QString(), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = reprojected->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.222222222222222 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.24691358024691 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "count" ) ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( QStringLiteral( "sum" ) ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( QStringLiteral( "mean" ) ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( QStringLiteral( "median" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "stdev" ) ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( QStringLiteral( "min" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "max" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "range" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "minority" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "majority" ) ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variety" ) ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( QStringLiteral( "variance" ) ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testNoData()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // test that zonal stats respects no data and user set no data values
  std::unique_ptr< QgsRasterLayer > rasterLayer = std::make_unique< QgsRasterLayer >( myTestDataPath + "raster.tif", QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  std::unique_ptr< QgsVectorLayer > vectorLayer = std::make_unique< QgsVectorLayer >( mTempPath + "polys2.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) );

  QgsZonalStatistics zs( vectorLayer.get(), rasterLayer.get(), QStringLiteral( "n" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = vectorLayer->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 16.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 13428.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 43868.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 0.0 );

  // with user no data
  rasterLayer->dataProvider()->setUserNoDataValue( 1, QgsRasterRangeList() << QgsRasterRange( 842, 852 )
      << QgsRasterRange( 877, 891 ) );

  zs = QgsZonalStatistics( vectorLayer.get(), rasterLayer.get(), QStringLiteral( "un" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  it = vectorLayer->getFeatures( request );
  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "uncount" ) ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( QStringLiteral( "unsum" ) ).toDouble(), 6652.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "uncount" ) ).toDouble(), 25.0 );
  QCOMPARE( f.attribute( QStringLiteral( "unsum" ) ).toDouble(), 21750.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( QStringLiteral( "uncount" ) ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( QStringLiteral( "unsum" ) ).toDouble(), 0.0 );
}

void TestQgsZonalStatistics::testSmallPolygons()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // test that zonal stats works ok with polygons much smaller than pixel size
  const std::unique_ptr< QgsRasterLayer > rasterLayer = std::make_unique< QgsRasterLayer >( myTestDataPath + "raster.tif", QStringLiteral( "raster" ), QStringLiteral( "gdal" ) );
  std::unique_ptr< QgsVectorLayer > vectorLayer = std::make_unique< QgsVectorLayer >( mTempPath + "small_polys.shp", QStringLiteral( "poly" ), QStringLiteral( "ogr" ) );

  QgsZonalStatistics zs( vectorLayer.get(), rasterLayer.get(), QStringLiteral( "n" ), 1, QgsZonalStatistics::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = vectorLayer->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 0.698248, 0.001 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 588.711, 0.01 );
  QCOMPARE( f.attribute( QStringLiteral( "nmin" ) ).toDouble(), 826.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nmax" ) ).toDouble(), 851.0 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nmean" ) ).toDouble(), 843.125292, 0.01 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 0.240808, 0.001 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 208.030921, 0.01 );
  QCOMPARE( f.attribute( QStringLiteral( "nmin" ) ).toDouble(), 859.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nmax" ) ).toDouble(), 868.0 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nmean" ) ).toDouble(), 863.887500, 0.01 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "ncount" ) ).toDouble(), 0.259522, 0.001 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nsum" ) ).toDouble(), 224.300747, 0.01 );
  QCOMPARE( f.attribute( QStringLiteral( "nmin" ) ).toDouble(), 851.0 );
  QCOMPARE( f.attribute( QStringLiteral( "nmax" ) ).toDouble(), 872.0 );
  QGSCOMPARENEAR( f.attribute( QStringLiteral( "nmean" ) ).toDouble(), 864.285638, 0.01 );
}

void TestQgsZonalStatistics::testShortName()
{
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Count ), QStringLiteral( "count" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Sum ), QStringLiteral( "sum" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Mean ), QStringLiteral( "mean" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Median ), QStringLiteral( "median" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::StDev ), QStringLiteral( "stdev" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Min ), QStringLiteral( "min" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Max ), QStringLiteral( "max" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Range ), QStringLiteral( "range" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Minority ), QStringLiteral( "minority" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Majority ), QStringLiteral( "majority" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Variety ), QStringLiteral( "variety" ) );
  QCOMPARE( QgsZonalStatistics::shortName( QgsZonalStatistics::Variance ), QStringLiteral( "variance" ) );
}

QGSTEST_MAIN( TestQgsZonalStatistics )
#include "testqgszonalstatistics.moc"
