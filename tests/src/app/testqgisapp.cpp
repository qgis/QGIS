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
#include "qgsclipboard.h"
#include "qgsfillsymbol.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerstore.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmesheditor.h"
#include "qgsmeshlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsstyle.h"
#include "qgstest.h"

#include <QApplication>
#include <QMessageBox>
#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for the QgisApp
 */
class TestQgisApp : public QgsTest
{
    Q_OBJECT

  public:
    TestQgisApp()
      : QgsTest( u"QgisApp tests"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    //! Test for issue GH #63346
    void pasteRasterStyleCategory();
    void copyPasteMultipleStyles();
    void toggleEditingVectorLayer();
    void toggleEditingMeshLayer();
    void toggleEditingPointCloudsLayer();
    void removeVectorLayerFromProject();
    void removePointCloudLayerFromProject();
    void removeMeshFromProject();

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

void static clickSave()
{
  QMessageBox *box = qobject_cast<QMessageBox *>( QApplication::activeModalWidget() );
  box->button( QMessageBox::Save )->click();
}

void static clickDiscard()
{
  QMessageBox *box = qobject_cast<QMessageBox *>( QApplication::activeModalWidget() );
  box->button( QMessageBox::Discard )->click();
}

void static clickCancel()
{
  QMessageBox *box = qobject_cast<QMessageBox *>( QApplication::activeModalWidget() );
  box->button( QMessageBox::Cancel )->click();
}

void static clickOK()
{
  QMessageBox *box = qobject_cast<QMessageBox *>( QApplication::activeModalWidget() );
  box->button( QMessageBox::Ok )->click();
}

void static clickEditCurrentMesh()
{
  QMessageBox *box = qobject_cast<QMessageBox *>( QApplication::activeModalWidget() );
  for ( QAbstractButton *b : box->buttons() )
  {
    if ( b->text() == "Edit Current Mesh" )
    {
      b->click();
      return;
    }
  }
}

//runs before all tests
void TestQgisApp::initTestCase()
{
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

void TestQgisApp::copyPasteMultipleStyles()
{
  auto sourceLayer = std::make_unique< QgsVectorLayer >( u"Polygon?crs=EPSG:4326"_s, u"polygons"_s, u"memory"_s );
  auto destLayer = std::make_unique< QgsVectorLayer >( u"Polygon?crs=EPSG:4326"_s, u"polygons"_s, u"memory"_s );

  sourceLayer->setRenderer( new QgsSingleSymbolRenderer( QgsFillSymbol::createSimple( { { "color", "#110011" } } ).release() ) );
  sourceLayer->styleManager()->addStyleFromLayer( u"second style"_s );
  sourceLayer->styleManager()->setCurrentStyle( u"second style"_s );
  sourceLayer->setRenderer( new QgsSingleSymbolRenderer( QgsFillSymbol::createSimple( { { "color", "#220022" } } ).release() ) );
  sourceLayer->styleManager()->addStyleFromLayer( u"third style"_s );
  sourceLayer->styleManager()->setCurrentStyle( u"third style"_s );
  sourceLayer->setRenderer( new QgsSingleSymbolRenderer( QgsFillSymbol::createSimple( { { "color", "#330033" } } ).release() ) );
  QCOMPARE( sourceLayer->styleManager()->currentStyle(), u"third style"_s );

  destLayer->setRenderer( new QgsSingleSymbolRenderer( QgsFillSymbol::createSimple( { { "color", "#ff0000" } } ).release() ) );

  // copy all styles
  mQgisApp->copyAllStyles( sourceLayer.get() );
  QVERIFY( mQgisApp->clipboard()->hasFormat( QStringLiteral( QGSCLIPBOARD_STYLES_MIME ) ) );

  mQgisApp->pasteAllStyles( destLayer.get() );
  QCOMPARE( destLayer->styleManager()->styles().count(), 3 );
  QVERIFY( destLayer->styleManager()->styles().contains( u"default" ) );
  QVERIFY( destLayer->styleManager()->styles().contains( u"second style" ) );
  QVERIFY( destLayer->styleManager()->styles().contains( u"third style" ) );

  QCOMPARE( destLayer->styleManager()->currentStyle(), u"third style"_s );
  QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer *>( destLayer->renderer() )->symbol()->symbolLayer( 0 )->color().name(), u"#330033"_s );

  destLayer->styleManager()->setCurrentStyle( u"second style"_s );
  QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer *>( destLayer->renderer() )->symbol()->symbolLayer( 0 )->color().name(), u"#220022"_s );

  destLayer->styleManager()->setCurrentStyle( u"default"_s );
  QCOMPARE( dynamic_cast< QgsSingleSymbolRenderer *>( destLayer->renderer() )->symbol()->symbolLayer( 0 )->color().name(), u"#110011"_s );

  // make sure pasting styles onto layer with multiple existing styles clears those
  // copy two styles from dest layer back to source layer, the missing one should be removed
  destLayer->styleManager()->removeStyle( u"second style"_s );
  mQgisApp->copyAllStyles( destLayer.get() );
  mQgisApp->pasteAllStyles( sourceLayer.get() );
  QCOMPARE( sourceLayer->styleManager()->styles().count(), 2 );
  QVERIFY( sourceLayer->styleManager()->styles().contains( u"default" ) );
  QVERIFY( sourceLayer->styleManager()->styles().contains( u"third style" ) );
}

void TestQgisApp::toggleEditingVectorLayer()
{
  const QString path = copyTestData( u"points_gpkg.gpkg"_s );
  auto vectorLayer = std::make_unique<QgsVectorLayer>( path + u"|layername=points_gpkg"_s, u"points"_s, u"ogr"_s );
  QVERIFY( vectorLayer->isValid() );

  // start editing
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  // close editing, no changes were made, no dialogs should popup
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  // restart editing
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  const int featureCountStart = vectorLayer->featureCount(); // to compare against feature count later
  QgsFeature f( vectorLayer->fields() );
  f.setGeometry( QgsGeometry::fromWkt( u"Point(15 45)"_s ) );
  QVERIFY( vectorLayer->addFeature( f ) );
  QVERIFY( vectorLayer->isModified() );

  // cancel stop editing
  QTimer::singleShot( 0, clickCancel );
  QVERIFY( !mQgisApp->toggleEditing( vectorLayer.get() ) );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  // restart editing
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  // modify layer again
  QVERIFY( vectorLayer->addFeature( f ) );
  QVERIFY( vectorLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  QVERIFY( mQgisApp->toggleEditing( vectorLayer.get() ) );

  // test layer was changed, reload from disk
  vectorLayer.reset();
  auto vectorLayerNew = std::make_unique<QgsVectorLayer>( path, u"pc"_s, u"ogr"_s );
  QVERIFY( vectorLayerNew->featureCount() == featureCountStart + 1 );
}

void TestQgisApp::toggleEditingMeshLayer()
{
  const QString path = copyTestData( u"/mesh/quad_and_triangle.2dm"_s );
  auto meshLayer = std::make_unique<QgsMeshLayer>( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayer->isValid() );

  // start editing
  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  // close editing, no changes were made, no dialogs should popup
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  // restart editing
  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  const int verticesCountStart = meshLayer->meshVertexCount();
  QCOMPARE( meshLayer->meshEditor()->addVertices( { QgsMeshVertex( 2500, 3500, 0 ) }, 10 ), 1 );
  QVERIFY( meshLayer->isModified() );

  // cancel stop editing
  QTimer::singleShot( 0, clickCancel );
  QVERIFY( !mQgisApp->toggleEditing( meshLayer.get() ) );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  // restart editing
  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  // modify layer again
  QCOMPARE( meshLayer->meshEditor()->addVertices( { QgsMeshVertex( 2500, 3500, 0 ) }, 10 ), 1 );
  QVERIFY( meshLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  QVERIFY( mQgisApp->toggleEditing( meshLayer.get() ) );

  // test layer was changed, reload from disk
  meshLayer.reset();
  auto meshLayerNew = std::make_unique<QgsMeshLayer>( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayerNew->meshVertexCount() == verticesCountStart + 1 );
}

void TestQgisApp::toggleEditingPointCloudsLayer()
{
  const QString pcPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );
  auto pointCloudLayer = std::make_unique<QgsPointCloudLayer>( pcPath, u"pc"_s, u"copc"_s );
  QVERIFY( pointCloudLayer->isValid() );

  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // close editing, no changes were made, no dialogs should popup
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // restart editing
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QgsPointCloudRequest request;
  request.setAttributes( QgsPointCloudAttributeCollection( QVector<QgsPointCloudAttribute>() << at ) );

  // check values before any changes
  std::unique_ptr<QgsPointCloudBlock> block0 = pointCloudLayer->index().nodeData( n, request );
  const char *block0Data = block0->data();
  QCOMPARE( block0Data[0], 2 );
  QCOMPARE( block0Data[1], 2 );
  QCOMPARE( block0Data[2], 2 );

  QVERIFY( pointCloudLayer->changeAttributeValue( n, { 0, 1, 2 }, at, 6 ) );
  QVERIFY( pointCloudLayer->isModified() );

  // cancel stop editing

  QTimer::singleShot( 0, clickCancel );
  QVERIFY( !mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // restart editing
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // modify layer again
  QVERIFY( pointCloudLayer->changeAttributeValue( n, { 0, 1, 2 }, at, 6 ) );
  QVERIFY( pointCloudLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer.get() ) );

  // test layer was changed, reload from disk
  pointCloudLayer.reset();
  auto pointCloudLayerNew = std::make_unique<QgsPointCloudLayer>( pcPath, u"pc"_s, u"copc"_s );
  std::unique_ptr<QgsPointCloudBlock> block1 = pointCloudLayerNew->index().nodeData( n, request );
  const char *block1Data = block1->data();
  QCOMPARE( block1Data[0], 6 );
  QCOMPARE( block1Data[1], 6 );
  QCOMPARE( block1Data[2], 6 );
}

void TestQgisApp::removeVectorLayerFromProject()
{
  const QString path = copyTestData( u"points_gpkg.gpkg"_s );
  QgsVectorLayer *vectorLayer = mQgisApp->addVectorLayer( path + u"|layername=points_gpkg"_s, u"points"_s, u"ogr"_s );
  QVERIFY( vectorLayer->isValid() );

  // select layer
  QVERIFY( mQgisApp->setActiveLayer( vectorLayer ) );

  // cancel layer removal, no edits made
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( vectorLayer );

  // no edits made, remove layer
  QTimer::singleShot( 0, clickOK );
  mQgisApp->removeLayer();
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !vectorLayer );

  // do not ask for confirmation to remove the layer from now on
  QgsSettings settings;
  settings.setValue( u"qgis/askToDeleteLayers"_s, false );

  // reload layer
  vectorLayer = mQgisApp->addVectorLayer( path + u"|layername=points_gpkg"_s, u"points"_s, u"ogr"_s );
  QVERIFY( vectorLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( vectorLayer ) );

  // start editing
  QVERIFY( mQgisApp->toggleEditing( vectorLayer ) );

  // stop editing, no changes made, no dialog should popup
  mQgisApp->removeLayer();
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !vectorLayer );

  // reload layer
  vectorLayer = mQgisApp->addVectorLayer( path + u"|layername=points_gpkg"_s, u"points"_s, u"ogr"_s );
  QVERIFY( vectorLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( vectorLayer ) );
  QVERIFY( mQgisApp->toggleEditing( vectorLayer ) );

  QgsFeature f( vectorLayer->fields() );
  f.setGeometry( QgsGeometry::fromWkt( u"Point(15 45)"_s ) );
  QVERIFY( vectorLayer->addFeature( f ) );
  QVERIFY( vectorLayer->isModified() );

  // cancel editing
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  QVERIFY( vectorLayer->isModified() );
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( vectorLayer );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  mQgisApp->removeLayer();
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !vectorLayer );

  // reload layer
  vectorLayer = mQgisApp->addVectorLayer( path + u"|layername=points_gpkg"_s, u"points"_s, u"ogr"_s );
  QVERIFY( vectorLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( vectorLayer ) );

  // start editing
  QVERIFY( mQgisApp->toggleEditing( vectorLayer ) );
  QVERIFY( vectorLayer->addFeature( f ) );
  QVERIFY( vectorLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  mQgisApp->removeLayer();
  vectorLayer = qobject_cast<QgsVectorLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !vectorLayer );

  settings.setValue( u"qgis/askToDeleteLayers"_s, true );
}

void TestQgisApp::removePointCloudLayerFromProject()
{
  const QString path = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );
  QgsPointCloudLayer *pointCloudLayer = mQgisApp->addLayer<QgsPointCloudLayer>( path, u"pc"_s, u"copc"_s );
  QVERIFY( pointCloudLayer->isValid() );

  // select layer
  QVERIFY( mQgisApp->setActiveLayer( pointCloudLayer ) );

  // cancel layer removal, no edits made
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( pointCloudLayer );

  // no edits made, remove layer
  QTimer::singleShot( 0, clickOK );
  mQgisApp->removeLayer();
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !pointCloudLayer );

  // do not ask for confirmation to remove the layer from now on
  QgsSettings settings;
  settings.setValue( u"qgis/askToDeleteLayers"_s, false );

  // reload layer
  pointCloudLayer = mQgisApp->addLayer<QgsPointCloudLayer>( path, u"pc"_s, u"copc"_s );
  QVERIFY( pointCloudLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( pointCloudLayer ) );

  // start editing
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer ) );

  // stop editing, no changes made, no dialog should popup
  mQgisApp->removeLayer();
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !pointCloudLayer );

  // reload layer
  pointCloudLayer = mQgisApp->addLayer<QgsPointCloudLayer>( path, u"pc"_s, u"copc"_s );
  QVERIFY( pointCloudLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( pointCloudLayer ) );
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer ) );

  // modify layer
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QgsPointCloudRequest request;
  request.setAttributes( QgsPointCloudAttributeCollection( QVector<QgsPointCloudAttribute>() << at ) );

  // check values before any changes
  std::unique_ptr<QgsPointCloudBlock> block0 = pointCloudLayer->index().nodeData( n, request );
  QVERIFY( pointCloudLayer->changeAttributeValue( n, { 0, 1, 2 }, at, 6 ) );
  QVERIFY( pointCloudLayer->isModified() );

  // cancel editing
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  QVERIFY( pointCloudLayer->isModified() );
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( pointCloudLayer );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  mQgisApp->removeLayer();
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !pointCloudLayer );

  // reload layer
  pointCloudLayer = mQgisApp->addLayer<QgsPointCloudLayer>( path, u"pc"_s, u"copc"_s );
  QVERIFY( pointCloudLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( pointCloudLayer ) );

  // start editing
  QVERIFY( mQgisApp->toggleEditing( pointCloudLayer ) );
  QVERIFY( pointCloudLayer->changeAttributeValue( n, { 0, 1, 2 }, at, 6 ) );
  QVERIFY( pointCloudLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  mQgisApp->removeLayer();
  pointCloudLayer = qobject_cast<QgsPointCloudLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !pointCloudLayer );

  settings.setValue( u"qgis/askToDeleteLayers"_s, true );
}

void TestQgisApp::removeMeshFromProject()
{
  const QString path = copyTestData( u"/mesh/quad_and_triangle.2dm"_s );
  QgsMeshLayer *meshLayer = mQgisApp->addMeshLayer( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayer->isValid() );

  // select layer
  QVERIFY( mQgisApp->setActiveLayer( meshLayer ) );

  // cancel layer removal, no edits made
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( meshLayer );

  // no edits made, remove layer
  QTimer::singleShot( 0, clickOK );
  mQgisApp->removeLayer();
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !meshLayer );

  // do not ask for confirmation to remove the layer from now on
  QgsSettings settings;
  settings.setValue( u"qgis/askToDeleteLayers"_s, false );

  // reload layer
  meshLayer = mQgisApp->addMeshLayer( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( meshLayer ) );

  // start editing
  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer ) );

  // stop editing, no changes made, no dialog should popup
  mQgisApp->removeLayer();
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !meshLayer );

  // reload layer
  meshLayer = mQgisApp->addMeshLayer( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( meshLayer ) );

  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer ) );

  // modify layer
  QCOMPARE( meshLayer->meshEditor()->addVertices( { QgsMeshVertex( 2500, 3500, 0 ) }, 10 ), 1 );
  QVERIFY( meshLayer->isModified() );

  // cancel editing
  QTimer::singleShot( 0, clickCancel );
  mQgisApp->removeLayer();
  QVERIFY( meshLayer->isModified() );
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( meshLayer );

  // discard changes
  QTimer::singleShot( 0, clickDiscard );
  mQgisApp->removeLayer();
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !meshLayer );

  // reload layer
  meshLayer = mQgisApp->addMeshLayer( path, u"mesh"_s, u"mdal"_s );
  QVERIFY( meshLayer->isValid() );
  QVERIFY( mQgisApp->setActiveLayer( meshLayer ) );

  // start editing
  QTimer::singleShot( 0, clickEditCurrentMesh );
  QVERIFY( mQgisApp->toggleEditing( meshLayer ) );
  QCOMPARE( meshLayer->meshEditor()->addVertices( { QgsMeshVertex( 2500, 3500, 0 ) }, 10 ), 1 );
  QVERIFY( meshLayer->isModified() );

  // save changes
  QTimer::singleShot( 0, clickSave );
  mQgisApp->removeLayer();
  meshLayer = qobject_cast<QgsMeshLayer *>( mQgisApp->activeLayer() );
  QVERIFY( !meshLayer );

  settings.setValue( u"qgis/askToDeleteLayers"_s, true );
}

QGSTEST_MAIN( TestQgisApp )
#include "testqgisapp.moc"
