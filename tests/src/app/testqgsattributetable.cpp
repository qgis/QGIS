/***************************************************************************
     testqgsattributetable.cpp
     -------------------------
    Date                 : 2016-02-14
    Copyright            : (C) 2016 by Nyall Dawson
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
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayertemporalproperties.h"
#include "qgsattributetabledialog.h"
#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsunittypes.h"
#include "qgssettings.h"
#include "qgsvectorfilewriter.h"
#include "qgsfeaturelistmodel.h"
#include "qgsclipboard.h"
#include "qgsvectorlayercache.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the attribute table dialog
 */
class TestQgsAttributeTable : public QObject
{
    Q_OBJECT
  public:
    TestQgsAttributeTable();

  private slots:

    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();
    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void testRegression15974();
    void testFieldCalculation();
    void testFieldCalculationArea();
    void testNoGeom();
    void testSelected();
    void testEdited();
    void testSelectedOnTop();
    void testSortByDisplayExpression();
    void testOrderColumn();
    void testFilteredFeatures();
    void testVisibleTemporal();
    void testCopySelectedRows();
    void testSortNumbers();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsAttributeTable::TestQgsAttributeTable() = default;

//runs before all tests
void TestQgsAttributeTable::initTestCase()
{
  qDebug() << "TestQgisAppClipboard::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  mQgisApp = new QgisApp();

  // setup the test QSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );
}

//runs after all tests
void TestQgsAttributeTable::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsAttributeTable::init()
{
  QLocale::setDefault( QLocale::c() );
}

void TestQgsAttributeTable::testFieldCalculation()
{
  //test field calculation

  //create a temporary layer
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolylineXY line3111;
  line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
  const QgsGeometry line3111G = QgsGeometry::fromPolylineXY( line3111 ) ;
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceMeters );

  // run length calculation
  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );
  tempLayer->startEditing();
  dlg->runFieldCalculation( tempLayer.get(), QStringLiteral( "col1" ), QStringLiteral( "$length" ) );
  tempLayer->commitChanges();
  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 26932.156;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );

  // change project length unit, check calculation respects unit
  QgsProject::instance()->setDistanceUnits( QgsUnitTypes::DistanceFeet );
  std::unique_ptr< QgsAttributeTableDialog > dlg2( new QgsAttributeTableDialog( tempLayer.get() ) );
  tempLayer->startEditing();
  dlg2->runFieldCalculation( tempLayer.get(), QStringLiteral( "col1" ), QStringLiteral( "$length" ) );
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 88360.0918635;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );
}

void TestQgsAttributeTable::testFieldCalculationArea()
{
  //test $area field calculation

  //create a temporary layer
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );

  QgsPolylineXY polygonRing3111;
  polygonRing3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 ) << QgsPointXY( 2520109, 2397715 ) << QgsPointXY( 2520792, 2425494 ) << QgsPointXY( 2484588, 2425722 );
  QgsPolygonXY polygon3111;
  polygon3111 << polygonRing3111;
  const QgsGeometry polygon3111G = QgsGeometry::fromPolygonXY( polygon3111 );
  f1.setGeometry( polygon3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMeters );

  // run area calculation
  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );
  tempLayer->startEditing();
  dlg->runFieldCalculation( tempLayer.get(), QStringLiteral( "col1" ), QStringLiteral( "$area" ) );
  tempLayer->commitChanges();
  // check result
  QgsFeatureIterator fit = tempLayer->dataProvider()->getFeatures();
  QgsFeature f;
  QVERIFY( fit.nextFeature( f ) );
  double expected = 1005755617.819130;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 1.0 );

  // change project area unit, check calculation respects unit
  QgsProject::instance()->setAreaUnits( QgsUnitTypes::AreaSquareMiles );
  std::unique_ptr< QgsAttributeTableDialog > dlg2( new QgsAttributeTableDialog( tempLayer.get() ) );
  tempLayer->startEditing();
  dlg2->runFieldCalculation( tempLayer.get(), QStringLiteral( "col1" ), QStringLiteral( "$area" ) );
  tempLayer->commitChanges();
  // check result
  fit = tempLayer->dataProvider()->getFeatures();
  QVERIFY( fit.nextFeature( f ) );
  expected = 388.324420;
  QGSCOMPARENEAR( f.attribute( "col1" ).toDouble(), expected, 0.001 );
}

void TestQgsAttributeTable::testNoGeom()
{
  const QgsSettings s;

  //test that by default the attribute table DOESN'T fetch geometries (because performance)
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( dlg->mMainView->masterModel()->request().flags() & QgsFeatureRequest::NoGeometry );

  // but if we are requesting only visible features, then geometry must be fetched...

  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowVisible ) );
  QVERIFY( dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( !( dlg->mMainView->masterModel()->request().flags() & QgsFeatureRequest::NoGeometry ) );

  // try changing existing dialog to no geometry mode
  dlg->mFeatureFilterWidget->filterShowAll();
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( dlg->mMainView->masterModel()->request().flags() & QgsFeatureRequest::NoGeometry );

  // and back to a geometry mode
  dlg->mFeatureFilterWidget->filterVisible();
  QVERIFY( dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( !( dlg->mMainView->masterModel()->request().flags() & QgsFeatureRequest::NoGeometry ) );

}

void TestQgsAttributeTable::testVisibleTemporal()
{
  // test attribute table opening in show feature visible mode
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:date" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsPolylineXY line;
  line << QgsPointXY( 0, 0 ) << QgsPointXY( 1, 1 );
  QgsGeometry geometry = QgsGeometry::fromPolylineXY( line ) ;
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setGeometry( geometry );
  f1.setAttributes( QgsAttributes() << 1 << QDate( 2020, 1, 1 ) );
  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setGeometry( geometry );
  f2.setAttributes( QgsAttributes() << 2 << QDate( 2020, 3, 1 ) );
  QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  line.clear();
  line << QgsPointXY( -3, -3 ) << QgsPointXY( -2, -2 );
  geometry = QgsGeometry::fromPolylineXY( line );
  f3.setGeometry( geometry );
  f3.setAttributes( QgsAttributes() << 3 << QDate( 2020, 1, 1 ) );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  QgsVectorLayerTemporalProperties *temporalProperties = qobject_cast< QgsVectorLayerTemporalProperties *>( tempLayer->temporalProperties() );
  temporalProperties->setIsActive( true );
  temporalProperties->setMode( Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields );
  temporalProperties->setStartField( QStringLiteral( "col1" ) );

  mQgisApp->mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  mQgisApp->mapCanvas()->resize( 500, 500 );
  mQgisApp->mapCanvas()->setLayers( QList< QgsMapLayer *>() << tempLayer.get() );
  mQgisApp->mapCanvas()->setExtent( QgsRectangle( -1, -1, 1, 1 ) );
  mQgisApp->mapCanvas()->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 2, 1 ), QTime( 0, 0, 0 ) ) ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowVisible ) );

  // feature id 2 is filtered out due to being out of temporal range
  // feature id 3 is filtered out due to being out of visible extent
  QCOMPARE( dlg->mMainView->filteredFeatures(), QgsFeatureIds() << 1 );
}

void TestQgsAttributeTable::testSelected()
{
  // test attribute table opening in show selected mode
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  const QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  const QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  const QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowSelected ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  //should be nothing - because no selection!
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );

  // make a selection
  tempLayer->selectByIds( QgsFeatureIds() << 1 << 3 );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );

  // another test - start with selection when dialog created
  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowSelected ) );
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );
  // remove selection
  tempLayer->removeSelection();
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );
}

void TestQgsAttributeTable::testEdited()
{
  // test attribute table opening in edited features mode
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  const QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  const QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  const QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowEdited ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  //should be nothing - because no edited features!
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );

  // make some edits
  tempLayer->startEditing();
  QVERIFY( tempLayer->changeAttributeValue( 1, 1, 5.5 ) );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 );
  QgsGeometry geom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 1 1)" ) );
  QVERIFY( tempLayer->changeGeometry( 3, geom ) );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );

  // another test - start with edited features when dialog created
  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowEdited ) );
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );
  // remove edits
  tempLayer->rollBack();
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), QgsFeatureRequest::FilterFids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );
}

void TestQgsAttributeTable::testSelectedOnTop()
{
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 3.2 );
  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 1.8 );
  QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( 0, 3 );
  f3.setAttribute( 1, 5.0 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  dlg->mMainView->setSortExpression( "pk" );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 3 ) );

  tempLayer->selectByIds( QgsFeatureIds() << 2 );
  dlg->mMainView->setSelectedOnTop( true );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 3 ) );

  dlg->mMainView->setSelectedOnTop( false );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 3 ) );

  tempLayer->selectByIds( QgsFeatureIds() << 3 );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 3 ) );

  dlg->mMainView->setSelectedOnTop( true );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 3 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( QgsAttributeTableModel::FeatureIdRole ), QVariant( 2 ) );

}

void TestQgsAttributeTable::testSortByDisplayExpression()
{
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 3.2 );
  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 1.8 );
  QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( 0, 3 );
  f3.setAttribute( 1, 5.0 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  dlg->mMainView->mFeatureListView->setDisplayExpression( "pk" );
  QgsFeatureListModel *listModel = dlg->mMainView->mFeatureListModel;
  QCOMPARE( listModel->rowCount(), 3 );

  QCOMPARE( listModel->index( 0, 0 ).data( Qt::DisplayRole ), QVariant( 1 ) );
  QCOMPARE( listModel->index( 1, 0 ).data( Qt::DisplayRole ), QVariant( 2 ) );
  QCOMPARE( listModel->index( 2, 0 ).data( Qt::DisplayRole ), QVariant( 3 ) );

  dlg->mMainView->mFeatureListView->setDisplayExpression( "col1" );
  QCOMPARE( listModel->index( 0, 0 ).data( Qt::DisplayRole ), QVariant( 1.8 ) );
  QCOMPARE( listModel->index( 1, 0 ).data( Qt::DisplayRole ), QVariant( 3.2 ) );
  QCOMPARE( listModel->index( 2, 0 ).data( Qt::DisplayRole ), QVariant( 5.0 ) );
}

void TestQgsAttributeTable::testSortNumbers()
{

  QLocale::setDefault( QLocale::Italian );

  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 2.001 );
  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 1001 );
  QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( 0, 3 );
  f3.setAttribute( 1, 10.0001 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  QgsAttributeTableConfig cfg;
  cfg.setSortExpression( QStringLiteral( R"("col1")" ) );
  cfg.setSortOrder( Qt::SortOrder::DescendingOrder );
  QgsAttributeTableConfig::ColumnConfig cfg1;
  QgsAttributeTableConfig::ColumnConfig cfg2;
  cfg1.name = QStringLiteral( "pk" );
  cfg2.name = QStringLiteral( "col1" );
  cfg.setColumns( {{ cfg1, cfg2 }} );

  dlg->mMainView->setAttributeTableConfig( cfg );

  auto model { dlg->mMainView->mFilterModel };

  QCOMPARE( model->data( model->index( 2, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "2,00100" ) );
  QCOMPARE( model->data( model->index( 1, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "10,00010" ) );
  QCOMPARE( model->data( model->index( 0, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "1.001,00000" ) );

  QCOMPARE( model->data( model->index( 2, 2 ), QgsAttributeTableModel::Role::SortRole ).toDouble(), 2.001 );
  QCOMPARE( model->data( model->index( 1, 2 ), QgsAttributeTableModel::Role::SortRole ).toDouble(), 10.0001 );
  QCOMPARE( model->data( model->index( 0, 2 ), QgsAttributeTableModel::Role::SortRole ).toDouble(), 1001.0 );

  QCOMPARE( dlg->mMainView->mTableView->horizontalHeader()->sortIndicatorSection(), 1 );
  QCOMPARE( dlg->mMainView->mTableView->horizontalHeader()->sortIndicatorOrder(), Qt::SortOrder::DescendingOrder );
  QVERIFY( dlg->mMainView->mTableView->horizontalHeader()->isSortIndicatorShown() );

}

void TestQgsAttributeTable::testRegression15974()
{
  // Test duplicated rows in attribute table + two crashes.
  const QString path = QDir::tempPath() + "/testshp15974.shp";
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "polygon?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = QStringLiteral( "system" );
  saveOptions.driverName = QStringLiteral( "ESRI Shapefile" );
  QgsVectorFileWriter::writeAsVectorFormatV3( tempLayer.get(), path, tempLayer->transformContext(), saveOptions );
  std::unique_ptr< QgsVectorLayer> shpLayer( new QgsVectorLayer( path, QStringLiteral( "test" ),  QStringLiteral( "ogr" ) ) );
  QgsFeature f1( shpLayer->dataProvider()->fields(), 1 );
  QgsGeometry geom;
  geom = QgsGeometry::fromWkt( QStringLiteral( "polygon((0 0, 0 1, 1 1, 1 0, 0 0))" ) );
  QVERIFY( geom.isGeosValid() );
  f1.setGeometry( geom );
  QgsFeature f2( shpLayer->dataProvider()->fields(), 2 );
  f2.setGeometry( geom );
  QgsFeature f3( shpLayer->dataProvider()->fields(), 3 );
  f3.setGeometry( geom );
  QVERIFY( shpLayer->startEditing() );
  QVERIFY( shpLayer->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );
  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( shpLayer.get() ) );
  QCOMPARE( shpLayer->featureCount(), 3L );
  mQgisApp->saveEdits( shpLayer.get() );
  QCOMPARE( shpLayer->featureCount(), 3L );
  QCOMPARE( dlg->mMainView->masterModel()->rowCount(), 3 );
  QCOMPARE( dlg->mMainView->mLayerCache->cachedFeatureIds().count(), 3 );
  QCOMPARE( dlg->mMainView->featureCount(), 3 );
  // All the following instructions made the test pass, before the connections to invalidate()
  // were introduced in QgsDualView::initModels
  // dlg->mMainView->mFilterModel->setSourceModel( dlg->mMainView->masterModel() );
  // dlg->mMainView->mFilterModel->invalidate();
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 3 );
}

void TestQgsAttributeTable::testOrderColumn()
{
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 13 );
  f1.setAttribute( 2, 7 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  // Issue https://github.com/qgis/QGIS/issues/28493
  // When we reorder column (last column becomes first column), and we select an entire row
  // the currentIndex is no longer the first column, and consequently it breaks edition

  QgsAttributeTableConfig config = QgsAttributeTableConfig();
  config.update( tempLayer->dataProvider()->fields() );
  QVector<QgsAttributeTableConfig::ColumnConfig> columns = config.columns();

  // move last column in first position
  columns.move( 2, 0 );
  config.setColumns( columns );

  dlg->mMainView->setAttributeTableConfig( config );

  QgsAttributeTableFilterModel *filterModel = static_cast<QgsAttributeTableFilterModel *>( dlg->mMainView->mTableView->model() );
  filterModel->sort( 0, Qt::AscendingOrder );

  QModelIndex index = filterModel->mapToSource( filterModel->sourceModel()->index( 0, 0 ) );
  QCOMPARE( index.row(), 0 );
  QCOMPARE( index.column(), 2 );

  index = filterModel->mapFromSource( filterModel->sourceModel()->index( 0, 0 ) );
  QCOMPARE( index.row(), 0 );
  QCOMPARE( index.column(), 1 );

  qDebug() << filterModel->mapFromSource( filterModel->sourceModel()->index( 0, 0 ) );

  // column 0 is indeed column 2 since we move it
  QCOMPARE( filterModel->sortColumn(), 2 );
}

void TestQgsAttributeTable::testFilteredFeatures()
{
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 2 );
  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 4 );
  QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  f3.setAttribute( 0, 3 );
  f3.setAttribute( 1, 6 );
  QgsFeature f4( tempLayer->dataProvider()->fields(), 4 );
  f4.setAttribute( 0, 4 );
  f4.setAttribute( 1, 8 );

  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

  QEventLoop loop;
  connect( qobject_cast<QgsAttributeTableFilterModel *>( dlg->mMainView->mFilterModel ), &QgsAttributeTableFilterModel::featuresFiltered, &loop, &QEventLoop::quit );

  // show all (three features)
  dlg->mFeatureFilterWidget->filterShowAll();
  QCOMPARE( dlg->mMainView->featureCount(), 3 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 3 );

  // add a feature
  tempLayer->startEditing();
  QVERIFY( tempLayer->addFeatures( QgsFeatureList() << f4 ) );
  //still show all (four features)
  QCOMPARE( tempLayer->featureCount(), 4L );
  QCOMPARE( dlg->mMainView->featureCount(), 4 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 4 );

  // bigger 5 (two of four features)
  dlg->mFeatureFilterWidget->setFilterExpression( QStringLiteral( "col1>5" ), QgsAttributeForm::ReplaceFilter, true );
  QCOMPARE( dlg->mMainView->featureCount(), 4 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 2 );
  // bigger 7 (one of four features)
  dlg->mFeatureFilterWidget->setFilterExpression( QStringLiteral( "col1>7" ), QgsAttributeForm::ReplaceFilter, true );
  QCOMPARE( dlg->mMainView->featureCount(), 4 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 1 );
  // bigger 9 (no of four features)
  dlg->mFeatureFilterWidget->setFilterExpression( QStringLiteral( "col1>9" ), QgsAttributeForm::ReplaceFilter, true );
  QCOMPARE( dlg->mMainView->featureCount(), 4 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 0 );

  //add two features
  QgsFeature f5( tempLayer->dataProvider()->fields(), 5 );
  f5.setAttribute( 0, 5 );
  f5.setAttribute( 1, 10 );
  QgsFeature f6( tempLayer->dataProvider()->fields(), 6 );
  f6.setAttribute( 0, 6 );
  f6.setAttribute( 1, 12 );
  QVERIFY( tempLayer->addFeatures( QgsFeatureList() << f5 << f6 ) );
  tempLayer->commitChanges();
  loop.exec();
  //no filter change -> now two of six features
  QCOMPARE( dlg->mMainView->featureCount(), 6 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 2 );

  //remove a feature not affecting the filter
  tempLayer->startEditing();
  QVERIFY( tempLayer->deleteFeature( f2.id() ) );
  //no filter change -> now two of five features
  QCOMPARE( dlg->mMainView->featureCount(), 5 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 2 );

  //remove a feature affecting the filter
  QVERIFY( tempLayer->deleteFeature( f5.id() ) );
  //no filter change -> now one of four features
  QCOMPARE( dlg->mMainView->featureCount(), 4 );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 1 );

  // smaller 11 (three of four features)
  dlg->mFeatureFilterWidget->setFilterExpression( QStringLiteral( "col1<11" ), QgsAttributeForm::ReplaceFilter, true );
  QCOMPARE( dlg->mMainView->filteredFeatureCount(), 3 );
}

void TestQgsAttributeTable::testCopySelectedRows()
{
  std::unique_ptr< QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 2 );

  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 4 );

  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  std::unique_ptr< QgsAttributeTableDialog > dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

  tempLayer->selectByIds( QgsFeatureIds() << 1 << 2 );

  dlg->mActionCopySelectedRows_triggered();

  QgsClipboard *clipboard = QgisApp::instance()->clipboard();
  QVERIFY( clipboard );
  QVERIFY( !clipboard->isEmpty() );
  QCOMPARE( clipboard->fields().names(), QStringList() << "pk" << "col1" << "col2" );

  const QgsFeatureList features = clipboard->copyOf();
  QCOMPARE( features.count(), 2 );
  QCOMPARE( features.at( 0 ).attribute( 0 ), 1 );
  QCOMPARE( features.at( 0 ).attribute( "col1" ), 2 );
  QCOMPARE( features.at( 0 ).attribute( "col2" ), QVariant() );
  QCOMPARE( features.at( 1 ).attribute( "pk" ), 2 );
  QCOMPARE( features.at( 1 ).attribute( "col1" ), 4 );
  QCOMPARE( features.at( 1 ).attribute( 2 ), QVariant() );

  QCOMPARE( clipboard->crs().authid(), "EPSG:3111" );
}

QGSTEST_MAIN( TestQgsAttributeTable )
#include "testqgsattributetable.moc"
