/***************************************************************************
     testqgisapp.cpp
     --------------------------------------
    Date                 : 6 october 2018
    Copyright            : (C) 2018 by Even Rouault
    Email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <gdal.h>

#include "qgisapp.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerstore.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgstest.h"

#include <QApplication>

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp
 */
class TestQgisApp : public QObject
{
    Q_OBJECT

  public:
    TestQgisApp();

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    //! Test for issue GH #63346
    void pasteRasterStyleCategory();

  public slots:
    void addVectorLayerShp();
    void addVectorLayerGeopackageSingleLayer();
    void addVectorLayerGeopackageSingleLayerAlreadyLayername();
    void addVectorLayerInvalid();
    void addEmbeddedGroup();
    void pasteFeature();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mTestDataDir;
};

TestQgisApp::TestQgisApp() = default;

//runs before all tests
void TestQgisApp::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  qDebug() << "TestQgisApp::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
  mQgisApp = new QgisApp();
}

//runs after all tests
void TestQgisApp::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgisApp::init()
{
  GDALDriverH hDriver = GDALGetDriverByName( "GPKG" );
  QVERIFY( hDriver );
  GDALDatasetH hDS = GDALCreate( hDriver, "/vsimem/test.gpkg", 0, 0, 0, GDT_Unknown, nullptr );
  QVERIFY( hDS );
  GDALDatasetCreateLayer( hDS, "my_layer", nullptr, wkbPoint, nullptr );
  GDALClose( hDS );
}

void TestQgisApp::cleanup()
{
  GDALDriverH hDriver = GDALGetDriverByName( "GPKG" );
  QVERIFY( hDriver );
  GDALDeleteDataset( hDriver, "/vsimem/test.gpkg" );
}

void TestQgisApp::addVectorLayerShp()
{
  const QString filePath = mTestDataDir + u"points.shp"_s;
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", u"ogr"_s );
  QVERIFY( layer->isValid() );

  // No need for |layerName= for shapefiles
  QVERIFY( layer->source().endsWith( "points.shp"_L1 ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() << layer->id() );
}

void TestQgisApp::addVectorLayerGeopackageSingleLayer()
{
  const QString filePath = "/vsimem/test.gpkg"_L1;
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", u"ogr"_s );
  QVERIFY( layer->isValid() );

  // Need for |layerName= for geopackage
  QVERIFY( layer->source().endsWith( "/vsimem/test.gpkg|layername=my_layer"_L1 ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() << layer->id() );
}

void TestQgisApp::addVectorLayerGeopackageSingleLayerAlreadyLayername()
{
  const QString filePath = "/vsimem/test.gpkg|layername=my_layer"_L1;
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", u"ogr"_s );
  QVERIFY( layer->isValid() );

  // Need for |layerName= for geopackage
  QVERIFY( layer->source().endsWith( "/vsimem/test.gpkg|layername=my_layer"_L1 ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() << layer->id() );
}

void TestQgisApp::addVectorLayerInvalid()
{
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( "i_do_not_exist", "test", u"ogr"_s );
  QVERIFY( !layer );

  layer = mQgisApp->addVectorLayer( "/vsimem/test.gpkg|layername=invalid_layer_name", "test", u"ogr"_s );
  QVERIFY( !layer );
}

void TestQgisApp::addEmbeddedGroup()
{
  const QString projectPath = QString( TEST_DATA_DIR ) + u"/embedded_groups/joins1.qgs"_s;

  QCOMPARE( QgsProject::instance()->layers<QgsVectorLayer *>().count(), 0 );

  mQgisApp->addEmbeddedItems( projectPath, QStringList() << u"GROUP"_s, QStringList() );

  QgsVectorLayer *vl = QgsProject::instance()->mapLayer<QgsVectorLayer *>( u"polys_with_id_32002f94_eebe_40a5_a182_44198ba1bc5a"_s );
  QCOMPARE( vl->fields().count(), 5 );

  // cleanup
  QgsProject::instance()->clear();
}

void TestQgisApp::pasteFeature()
{
  QgsVectorLayer *vl = new QgsVectorLayer( u"Polygon?crs=EPSG:4326"_s, u"polygons"_s, u"memory"_s );

  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( u"POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))"_s ) );
  vl->startEditing();
  vl->addFeature( f );
  vl->commitChanges();

  QgsProject::instance()->addMapLayer( vl );
  QgsProject::instance()->setAvoidIntersectionsMode( Qgis::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer );

  vl->selectByIds( QgsFeatureIds() << 1 );

  // Copy feature with the initial polygon
  mQgisApp->copySelectionToClipboard( vl );

  vl->startEditing();
  QgsGeometry geom = QgsGeometry::fromWkt( u"POLYGON((5 0, 10 0, 10 10, 5 10, 5 0))"_s );
  vl->changeGeometry( 1, geom );
  vl->commitChanges();

  vl->startEditing();
  mQgisApp->pasteFromClipboard( vl );
  vl->commitChanges();

  f = vl->getFeature( 2 );
  QCOMPARE( f.geometry().asWkt(), u"Polygon ((0 0, 0 10, 5 10, 5 0, 0 0))"_s );
}

void TestQgisApp::pasteRasterStyleCategory()
{
  // Create a 2x2 int rasters using gdal
  QTemporaryDir tempDir;
  const QString tempDirPath = tempDir.path();
  const QString rasterPath1 = tempDirPath + u"/raster1.tif"_s;
  const QString rasterPath2 = tempDirPath + u"/raster2.tif"_s;

  GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
  QVERIFY( hDriver );
  GDALDatasetH hDS = GDALCreate( hDriver, rasterPath1.toUtf8().constData(), 2, 2, 1, GDT_Int32, nullptr );
  QVERIFY( hDS );
  GDALClose( hDS );
  hDS = GDALCreate( hDriver, rasterPath2.toUtf8().constData(), 2, 2, 1, GDT_Int32, nullptr );
  QVERIFY( hDS );
  GDALClose( hDS );
  QgsRasterLayer rl1( rasterPath1, u"raster1"_s );
  QgsRasterLayer rl2( rasterPath2, u"raster2"_s );
  QVERIFY( rl1.isValid() );
  QVERIFY( rl2.isValid() );

  // Set legend flag for raster 1
  rl1.legend()->setFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault );

  QVERIFY( rl1.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );
  QVERIFY( !rl2.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );

  // Copy style from raster 1
  mQgisApp->copyStyle( &rl1, QgsMapLayer::StyleCategory::Legend );
  // Paste style to raster 2
  mQgisApp->pasteStyle( &rl2, QgsMapLayer::StyleCategory::Legend );

  QVERIFY( rl2.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );

  // Check the other way round
  rl1.legend()->setFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault, false );
  rl2.legend()->setFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault, true );
  QVERIFY( !rl1.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );
  QVERIFY( rl2.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );

  // Copy style from raster 1
  mQgisApp->copyStyle( &rl1, QgsMapLayer::StyleCategory::Legend );
  // Paste style to raster 2
  mQgisApp->pasteStyle( &rl2, QgsMapLayer::StyleCategory::Legend );
  QVERIFY( !rl2.legend()->flags().testFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault ) );
}


QGSTEST_MAIN( TestQgisApp )
#include "testqgisapp.moc"
