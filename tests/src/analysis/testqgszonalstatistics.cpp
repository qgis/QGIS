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

#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgszonalstatistics.h"

#include <QDir>

/**
 * \ingroup UnitTests
 * This is a unit test for the zonal statistics class
 */
class TestQgsZonalStatistics : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsZonalStatistics()
      : QgsTest( u"Zonal Statistics Test"_s )
    {}


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

  mVectorLayer = new QgsVectorLayer( mTempPath + "polys.shp", u"poly"_s, u"ogr"_s );
  mRasterLayer = new QgsRasterLayer( mTempPath + "edge_problem.asc", u"raster"_s, u"gdal"_s );
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mVectorLayer << mRasterLayer
  );
}

void TestQgsZonalStatistics::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsZonalStatistics::testStatistics()
{
  QgsZonalStatistics zs( mVectorLayer, mRasterLayer, QString(), 1, Qgis::ZonalStatistic::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterFid( 0 );
  bool fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.13888888888889 );

  // same with long prefix to ensure that field name truncation handled correctly
  QgsZonalStatistics zsl( mVectorLayer, mRasterLayer, u"myqgis2_"_s, 1, Qgis::ZonalStatistic::All );
  zsl.calculateStatistics( nullptr );

  request.setFilterFid( 0 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"myqgis2_co"_s ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( u"myqgis2_su"_s ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( u"myqgis2_me"_s ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( u"myqgis2__1"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_st"_s ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( u"myqgis2_mi"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2_ma"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_ra"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2__2"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2__3"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_va"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"myqgis2__4"_s ).toDouble(), 0.222222222222222 );

  request.setFilterFid( 1 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"myqgis2_co"_s ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( u"myqgis2_su"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"myqgis2_me"_s ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( u"myqgis2__1"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_st"_s ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( u"myqgis2_mi"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2_ma"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_ra"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2__2"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2__3"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_va"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"myqgis2__4"_s ).toDouble(), 0.24691358024691 );

  request.setFilterFid( 2 );
  fetched = mVectorLayer->getFeatures( request ).nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"myqgis2_co"_s ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( u"myqgis2_su"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"myqgis2_me"_s ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( u"myqgis2__1"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_st"_s ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( u"myqgis2_mi"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2_ma"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_ra"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2__2"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"myqgis2__3"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"myqgis2_va"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"myqgis2__4"_s ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testReprojection()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // create a reprojected version of the layer
  auto vectorLayer = std::make_unique<QgsVectorLayer>( myTestDataPath + "polys.shp", u"poly"_s, u"ogr"_s );
  std::unique_ptr<QgsVectorLayer> reprojected( vectorLayer->materialize( QgsFeatureRequest().setDestinationCrs( QgsCoordinateReferenceSystem( u"EPSG:3785"_s ), QgsProject::instance()->transformContext() ) ) );

  QCOMPARE( reprojected->featureCount(), vectorLayer->featureCount() );
  QgsZonalStatistics zs( reprojected.get(), mRasterLayer, QString(), 1, Qgis::ZonalStatistic::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = reprojected->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 12.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.666666666666667 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.47140452079103201 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.222222222222222 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 9.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.555555555555556 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.49690399499995302 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.24691358024691 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"count"_s ).toDouble(), 6.0 );
  QCOMPARE( f.attribute( u"sum"_s ).toDouble(), 5.0 );
  QCOMPARE( f.attribute( u"mean"_s ).toDouble(), 0.833333333333333 );
  QCOMPARE( f.attribute( u"median"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"stdev"_s ).toDouble(), 0.372677996249965 );
  QCOMPARE( f.attribute( u"min"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"max"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"range"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"minority"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"majority"_s ).toDouble(), 1.0 );
  QCOMPARE( f.attribute( u"variety"_s ).toDouble(), 2.0 );
  QCOMPARE( f.attribute( u"variance"_s ).toDouble(), 0.13888888888889 );
}

void TestQgsZonalStatistics::testNoData()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // test that zonal stats respects no data and user set no data values
  auto rasterLayer = std::make_unique<QgsRasterLayer>( myTestDataPath + "raster.tif", u"raster"_s, u"gdal"_s );
  auto vectorLayer = std::make_unique<QgsVectorLayer>( mTempPath + "polys2.shp", u"poly"_s, u"ogr"_s );

  QgsZonalStatistics zs( vectorLayer.get(), rasterLayer.get(), u"n"_s, 1, Qgis::ZonalStatistic::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = vectorLayer->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"ncount"_s ).toDouble(), 16.0 );
  QCOMPARE( f.attribute( u"nsum"_s ).toDouble(), 13428.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"ncount"_s ).toDouble(), 50.0 );
  QCOMPARE( f.attribute( u"nsum"_s ).toDouble(), 43868.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"ncount"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"nsum"_s ).toDouble(), 0.0 );

  // with user no data
  rasterLayer->dataProvider()->setUserNoDataValue( 1, QgsRasterRangeList() << QgsRasterRange( 842, 852 ) << QgsRasterRange( 877, 891 ) );

  zs = QgsZonalStatistics( vectorLayer.get(), rasterLayer.get(), u"un"_s, 1, Qgis::ZonalStatistic::All );
  zs.calculateStatistics( nullptr );

  it = vectorLayer->getFeatures( request );
  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"uncount"_s ).toDouble(), 8.0 );
  QCOMPARE( f.attribute( u"unsum"_s ).toDouble(), 6652.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"uncount"_s ).toDouble(), 25.0 );
  QCOMPARE( f.attribute( u"unsum"_s ).toDouble(), 21750.0 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QCOMPARE( f.attribute( u"uncount"_s ).toDouble(), 0.0 );
  QCOMPARE( f.attribute( u"unsum"_s ).toDouble(), 0.0 );
}

void TestQgsZonalStatistics::testSmallPolygons()
{
  const QString myDataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTestDataPath = myDataPath + "/zonalstatistics/";

  // test that zonal stats works ok with polygons much smaller than pixel size
  const std::unique_ptr<QgsRasterLayer> rasterLayer = std::make_unique<QgsRasterLayer>( myTestDataPath + "raster.tif", u"raster"_s, u"gdal"_s );
  auto vectorLayer = std::make_unique<QgsVectorLayer>( mTempPath + "small_polys.shp", u"poly"_s, u"ogr"_s );

  QgsZonalStatistics zs( vectorLayer.get(), rasterLayer.get(), u"n"_s, 1, Qgis::ZonalStatistic::All );
  zs.calculateStatistics( nullptr );

  QgsFeature f;
  const QgsFeatureRequest request;
  QgsFeatureIterator it = vectorLayer->getFeatures( request );
  bool fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( u"ncount"_s ).toDouble(), 0.698248, 0.001 );
  QGSCOMPARENEAR( f.attribute( u"nsum"_s ).toDouble(), 588.711, 0.01 );
  QCOMPARE( f.attribute( u"nmin"_s ).toDouble(), 826.0 );
  QCOMPARE( f.attribute( u"nmax"_s ).toDouble(), 851.0 );
  QGSCOMPARENEAR( f.attribute( u"nmean"_s ).toDouble(), 843.125292, 0.01 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( u"ncount"_s ).toDouble(), 0.240808, 0.001 );
  QGSCOMPARENEAR( f.attribute( u"nsum"_s ).toDouble(), 208.030921, 0.01 );
  QCOMPARE( f.attribute( u"nmin"_s ).toDouble(), 859.0 );
  QCOMPARE( f.attribute( u"nmax"_s ).toDouble(), 868.0 );
  QGSCOMPARENEAR( f.attribute( u"nmean"_s ).toDouble(), 863.887500, 0.01 );

  fetched = it.nextFeature( f );
  QVERIFY( fetched );
  QGSCOMPARENEAR( f.attribute( u"ncount"_s ).toDouble(), 0.259522, 0.001 );
  QGSCOMPARENEAR( f.attribute( u"nsum"_s ).toDouble(), 224.300747, 0.01 );
  QCOMPARE( f.attribute( u"nmin"_s ).toDouble(), 851.0 );
  QCOMPARE( f.attribute( u"nmax"_s ).toDouble(), 872.0 );
  QGSCOMPARENEAR( f.attribute( u"nmean"_s ).toDouble(), 864.285638, 0.01 );
}

void TestQgsZonalStatistics::testShortName()
{
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Count ), u"count"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Sum ), u"sum"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Mean ), u"mean"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Median ), u"median"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::StDev ), u"stdev"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Min ), u"min"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Max ), u"max"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Range ), u"range"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Minority ), u"minority"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Majority ), u"majority"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Variety ), u"variety"_s );
  QCOMPARE( QgsZonalStatistics::shortName( Qgis::ZonalStatistic::Variance ), u"variance"_s );
}

QGSTEST_MAIN( TestQgsZonalStatistics )
#include "testqgszonalstatistics.moc"
