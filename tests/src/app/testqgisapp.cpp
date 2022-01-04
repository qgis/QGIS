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
#include "qgstest.h"

#include <QApplication>
#include "gdal.h"

#include "qgisapp.h"
#include "qgsproject.h"
#include "qgsmaplayerstore.h"

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
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void addVectorLayerShp();
    void addVectorLayerGeopackageSingleLayer();
    void addVectorLayerGeopackageSingleLayerAlreadyLayername();
    void addVectorLayerInvalid();
    void addEmbeddedGroup();

  private:
    QgisApp *mQgisApp = nullptr;
    QString mTestDataDir;
};

TestQgisApp::TestQgisApp() = default;

//runs before all tests
void TestQgisApp::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

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
  const QString filePath = mTestDataDir + QStringLiteral( "points.shp" );
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );

  // No need for |layerName= for shapefiles
  QVERIFY( layer->source().endsWith( QLatin1String( "points.shp" ) ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() <<  layer->id() );
}

void TestQgisApp::addVectorLayerGeopackageSingleLayer()
{
  const QString filePath = QLatin1String( "/vsimem/test.gpkg" );
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );

  // Need for |layerName= for geopackage
  QVERIFY( layer->source().endsWith( QLatin1String( "/vsimem/test.gpkg|layername=my_layer" ) ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() <<  layer->id() );
}

void TestQgisApp::addVectorLayerGeopackageSingleLayerAlreadyLayername()
{
  const QString filePath = QLatin1String( "/vsimem/test.gpkg|layername=my_layer" );
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( filePath, "test", QStringLiteral( "ogr" ) );
  QVERIFY( layer->isValid() );

  // Need for |layerName= for geopackage
  QVERIFY( layer->source().endsWith( QLatin1String( "/vsimem/test.gpkg|layername=my_layer" ) ) );

  // cleanup
  QgsProject::instance()->layerStore()->removeMapLayers( QStringList() <<  layer->id() );
}

void TestQgisApp::addVectorLayerInvalid()
{
  QgsVectorLayer *layer = mQgisApp->addVectorLayer( "i_do_not_exist", "test", QStringLiteral( "ogr" ) );
  QVERIFY( !layer );

  layer = mQgisApp->addVectorLayer( "/vsimem/test.gpkg|layername=invalid_layer_name", "test", QStringLiteral( "ogr" ) );
  QVERIFY( !layer );
}

void TestQgisApp::addEmbeddedGroup()
{
  const QString projectPath = QString( TEST_DATA_DIR ) + QStringLiteral( "/embedded_groups/joins1.qgs" );

  QCOMPARE( QgsProject::instance()->layers<QgsVectorLayer *>().count(), 0 );

  mQgisApp->addEmbeddedItems( projectPath, QStringList() << QStringLiteral( "GROUP" ), QStringList() );

  QgsVectorLayer *vl = QgsProject::instance()->mapLayer<QgsVectorLayer *>( QStringLiteral( "polys_with_id_32002f94_eebe_40a5_a182_44198ba1bc5a" ) );
  QCOMPARE( vl->fields().count(), 5 );

  // cleanup
  QgsProject::instance()->clear();
}



QGSTEST_MAIN( TestQgisApp )
#include "testqgisapp.moc"
