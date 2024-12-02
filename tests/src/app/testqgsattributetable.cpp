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
#include "qgssettings.h"
#include "qgsvectorfilewriter.h"
#include "qgsfeaturelistmodel.h"
#include "qgsclipboard.h"
#include "qgsvectorlayercache.h"
#include "qgsgui.h"
#include "qgseditorwidgetregistry.h"

#include <QSignalSpy>

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

    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();
    // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

  public slots:

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
    void testOpenWithFilterExpression();
    void testVisibleTemporal();
    void testCopySelectedRows();
    void testSortNumbers();
    void testStartMultiEditNoChanges();
    void testMultiEditMakeUncommittedChanges();
    void testInvalidView();
    void testEnsureEditSelection();
  private slots:
    void testFetchAllAttributes();

  private:
    QgisApp *mQgisApp = nullptr;
};

TestQgsAttributeTable::TestQgsAttributeTable() = default;

//runs before all tests
void TestQgsAttributeTable::initTestCase()
{
  qDebug() << "TestQgsAttributeTable::initTestCase()";
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
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
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), 0.0 );
  QgsPolylineXY line3111;
  line3111 << QgsPointXY( 2484588, 2425722 ) << QgsPointXY( 2482767, 2398853 );
  const QgsGeometry line3111G = QgsGeometry::fromPolylineXY( line3111 );
  f1.setGeometry( line3111G );
  tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 );

  // set project CRS and ellipsoid
  const QgsCoordinateReferenceSystem srs( QStringLiteral( "EPSG:3111" ) );
  QgsProject::instance()->setCrs( srs );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Meters );

  // run length calculation
  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );
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
  QgsProject::instance()->setDistanceUnits( Qgis::DistanceUnit::Feet );
  std::unique_ptr<QgsAttributeTableDialog> dlg2( new QgsAttributeTableDialog( tempLayer.get() ) );
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
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "Polygon?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
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
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMeters );

  // run area calculation
  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );
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
  QgsProject::instance()->setAreaUnits( Qgis::AreaUnit::SquareMiles );
  std::unique_ptr<QgsAttributeTableDialog> dlg2( new QgsAttributeTableDialog( tempLayer.get() ) );
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
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( dlg->mMainView->masterModel()->request().flags() & Qgis::FeatureRequestFlag::NoGeometry );

  // but if we are requesting only visible features, then geometry must be fetched...

  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowVisible ) );
  QVERIFY( dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( !( dlg->mMainView->masterModel()->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );

  // try changing existing dialog to no geometry mode
  dlg->mFeatureFilterWidget->filterShowAll();
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( dlg->mMainView->masterModel()->request().flags() & Qgis::FeatureRequestFlag::NoGeometry );

  // and back to a geometry mode
  dlg->mFeatureFilterWidget->filterVisible();
  QVERIFY( dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QVERIFY( !( dlg->mMainView->masterModel()->request().flags() & Qgis::FeatureRequestFlag::NoGeometry ) );
}

void TestQgsAttributeTable::testVisibleTemporal()
{
  // test attribute table opening in show feature visible mode
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:date" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsPolylineXY line;
  line << QgsPointXY( 0, 0 ) << QgsPointXY( 1, 1 );
  QgsGeometry geometry = QgsGeometry::fromPolylineXY( line );
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

  QgsVectorLayerTemporalProperties *temporalProperties = qobject_cast<QgsVectorLayerTemporalProperties *>( tempLayer->temporalProperties() );
  temporalProperties->setIsActive( true );
  temporalProperties->setMode( Qgis::VectorTemporalMode::FeatureDateTimeStartAndEndFromFields );
  temporalProperties->setStartField( QStringLiteral( "col1" ) );

  mQgisApp->mapCanvas()->setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:4326" ) );
  mQgisApp->mapCanvas()->resize( 500, 500 );
  mQgisApp->mapCanvas()->setLayers( QList<QgsMapLayer *>() << tempLayer.get() );
  mQgisApp->mapCanvas()->setExtent( QgsRectangle( -1, -1, 1, 1 ) );
  mQgisApp->mapCanvas()->setTemporalRange( QgsDateTimeRange( QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2020, 2, 1 ), QTime( 0, 0, 0 ) ) ) );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowVisible ) );

  // feature id 2 is filtered out due to being out of temporal range
  // feature id 3 is filtered out due to being out of visible extent
  QCOMPARE( dlg->mMainView->filteredFeatures(), QgsFeatureIds() << 1 );
}

void TestQgsAttributeTable::testSelected()
{
  // test attribute table opening in show selected mode
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  const QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  const QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  const QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowSelected ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  //should be nothing - because no selection!
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );

  // make a selection
  tempLayer->selectByIds( QgsFeatureIds() << 1 << 3 );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );

  // another test - start with selection when dialog created
  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowSelected ) );
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );
  // remove selection
  tempLayer->removeSelection();
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );
}

void TestQgsAttributeTable::testEdited()
{
  // test attribute table opening in edited features mode
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  const QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  const QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  const QgsFeature f3( tempLayer->dataProvider()->fields(), 3 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowEdited ) );

  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  //should be nothing - because no edited features!
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );

  // make some edits
  tempLayer->startEditing();
  QVERIFY( tempLayer->changeAttributeValue( 1, 1, 5.5 ) );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 );
  QgsGeometry geom = QgsGeometry::fromWkt( QStringLiteral( "LineString(0 0, 1 1)" ) );
  QVERIFY( tempLayer->changeGeometry( 3, geom ) );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );

  // another test - start with edited features when dialog created
  dlg.reset( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowEdited ) );
  QVERIFY( !dlg->mMainView->masterModel()->layerCache()->cacheGeometry() );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QCOMPARE( dlg->mMainView->masterModel()->request().filterFids(), QgsFeatureIds() << 1 << 3 );
  // remove edits
  tempLayer->rollBack();
  QCOMPARE( dlg->mMainView->masterModel()->request().filterType(), Qgis::FeatureRequestFilterType::Fids );
  QVERIFY( dlg->mMainView->masterModel()->request().filterFids().isEmpty() );
}

void TestQgsAttributeTable::testSelectedOnTop()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
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

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  dlg->mMainView->setSortExpression( "pk" );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );

  tempLayer->selectByIds( QgsFeatureIds() << 2 );
  dlg->mMainView->setSelectedOnTop( true );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );

  dlg->mMainView->setSelectedOnTop( false );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );

  tempLayer->selectByIds( QgsFeatureIds() << 3 );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );

  dlg->mMainView->setSelectedOnTop( true );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );

  dlg->mMainView->setSelectedOnTop( false );
  dlg->mMainView->tableView()->sortByColumn( 1, Qt::DescendingOrder );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );

  tempLayer->selectByIds( QgsFeatureIds() << 2 );
  dlg->mMainView->setSelectedOnTop( true );

  QCOMPARE( dlg->mMainView->mFilterModel->index( 0, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 2 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 1, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 3 ) );
  QCOMPARE( dlg->mMainView->mFilterModel->index( 2, 0 ).data( static_cast<int>( QgsAttributeTableModel::CustomRole::FeatureId ) ), QVariant( 1 ) );
}

void TestQgsAttributeTable::testSortByDisplayExpression()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
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

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

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

  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:double" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
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

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

  QgsAttributeTableConfig cfg;
  cfg.setSortExpression( QStringLiteral( R"("col1")" ) );
  cfg.setSortOrder( Qt::SortOrder::DescendingOrder );
  QgsAttributeTableConfig::ColumnConfig cfg1;
  QgsAttributeTableConfig::ColumnConfig cfg2;
  cfg1.name = QStringLiteral( "pk" );
  cfg2.name = QStringLiteral( "col1" );
  cfg.setColumns( { { cfg1, cfg2 } } );

  dlg->mMainView->setAttributeTableConfig( cfg );

  auto model { dlg->mMainView->mFilterModel };

  QCOMPARE( model->data( model->index( 2, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "2,00100" ) );
  QCOMPARE( model->data( model->index( 1, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "10,00010" ) );
  QCOMPARE( model->data( model->index( 0, 1 ), Qt::ItemDataRole::DisplayRole ).toString(), QString( "1.001,00000" ) );

  QCOMPARE( model->data( model->index( 2, 2 ), static_cast<int>( QgsAttributeTableModel::CustomRole::Sort ) ).toDouble(), 2.001 );
  QCOMPARE( model->data( model->index( 1, 2 ), static_cast<int>( QgsAttributeTableModel::CustomRole::Sort ) ).toDouble(), 10.0001 );
  QCOMPARE( model->data( model->index( 0, 2 ), static_cast<int>( QgsAttributeTableModel::CustomRole::Sort ) ).toDouble(), 1001.0 );

  QCOMPARE( dlg->mMainView->mTableView->horizontalHeader()->sortIndicatorSection(), 1 );
  QCOMPARE( dlg->mMainView->mTableView->horizontalHeader()->sortIndicatorOrder(), Qt::SortOrder::DescendingOrder );
  QVERIFY( dlg->mMainView->mTableView->horizontalHeader()->isSortIndicatorShown() );
}

void TestQgsAttributeTable::testStartMultiEditNoChanges()
{
  std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=col0:integer&field=col1:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  QgsFeature ft1( layer->dataProvider()->fields() );
  ft1.setAttributes( QgsAttributes() << 1 << 2 );
  layer->dataProvider()->addFeature( ft1 );
  QgsFeature ft2( layer->dataProvider()->fields() );
  ft2.setAttributes( QgsAttributes() << 3 << 4 );
  layer->dataProvider()->addFeature( ft2 );

  layer->selectAll();

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( layer.get() ) );

  for ( int i = 0; i < 10; ++i )
  {
    dlg->mMainView->setCurrentEditSelection( { ft2.id() } );
    layer->startEditing();
    dlg->mMainView->setMultiEditEnabled( true );

    // nothing should change until the user actually makes a change!
    // see https://github.com/qgis/QGIS/issues/46306
    QgsFeature fNew1 = layer->getFeature( ft1.id() );
    QCOMPARE( fNew1.attributes().at( 0 ).toInt(), 1 );
    QCOMPARE( fNew1.attributes().at( 1 ).toInt(), 2 );
    QgsFeature fNew2 = layer->getFeature( ft2.id() );
    QCOMPARE( fNew2.attributes().at( 0 ).toInt(), 3 );
    QCOMPARE( fNew2.attributes().at( 1 ).toInt(), 4 );

    layer->rollBack();
    dlg->mMainView->setCurrentEditSelection( { ft1.id() } );
    layer->startEditing();
    dlg->mMainView->setMultiEditEnabled( true );

    // nothing should change until the user actually makes a change!
    fNew1 = layer->getFeature( ft1.id() );
    QCOMPARE( fNew1.attributes().at( 0 ).toInt(), 1 );
    QCOMPARE( fNew1.attributes().at( 1 ).toInt(), 2 );
    fNew2 = layer->getFeature( ft2.id() );
    QCOMPARE( fNew2.attributes().at( 0 ).toInt(), 3 );
    QCOMPARE( fNew2.attributes().at( 1 ).toInt(), 4 );
    layer->rollBack();
  }
}

void TestQgsAttributeTable::testMultiEditMakeUncommittedChanges()
{
  std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=col0:integer&field=col1:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  QgsFeature ft1( layer->dataProvider()->fields() );
  ft1.setAttributes( QgsAttributes() << 1 << 2 );
  layer->dataProvider()->addFeature( ft1 );
  QgsFeature ft2( layer->dataProvider()->fields() );
  ft2.setAttributes( QgsAttributes() << 3 << 4 );
  layer->dataProvider()->addFeature( ft2 );

  layer->selectAll();

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( layer.get() ) );

  dlg->mMainView->setCurrentEditSelection( { ft2.id() } );
  layer->startEditing();
  dlg->mMainView->setMultiEditEnabled( true );

  dlg->mMainView->mAttributeForm->changeAttribute( QStringLiteral( "col0" ), 99 );

  // nothing should change until the multiedit changes are manually applied
  QgsFeature fNew1 = layer->getFeature( ft1.id() );
  QCOMPARE( fNew1.attributes().at( 0 ).toInt(), 1 );
  QCOMPARE( fNew1.attributes().at( 1 ).toInt(), 2 );
  QgsFeature fNew2 = layer->getFeature( ft2.id() );
  QCOMPARE( fNew2.attributes().at( 0 ).toInt(), 3 );
  QCOMPARE( fNew2.attributes().at( 1 ).toInt(), 4 );

  layer->rollBack();
}

void TestQgsAttributeTable::testRegression15974()
{
  // Test duplicated rows in attribute table + two crashes.
  const QString path = QDir::tempPath() + "/testshp15974.shp";
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "polygon?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );
  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.fileEncoding = QStringLiteral( "system" );
  saveOptions.driverName = QStringLiteral( "ESRI Shapefile" );
  QgsVectorFileWriter::writeAsVectorFormatV3( tempLayer.get(), path, tempLayer->transformContext(), saveOptions );
  std::unique_ptr<QgsVectorLayer> shpLayer( new QgsVectorLayer( path, QStringLiteral( "test" ), QStringLiteral( "ogr" ) ) );
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
  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( shpLayer.get() ) );
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
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 13 );
  f1.setAttribute( 2, 7 );
  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 ) );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get() ) );

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

  // Assume an action column at the index 0,3
  // When we request the source index, it should be invalid (because there is no source of this column)
  index = filterModel->mapToSource( filterModel->sourceModel()->index( 0, 3 ) );
  QVERIFY( index.isValid() );
  // "hen we request the source index by mapToMaster, there should be returned the source index of the first column
  // that's done to provide the feature
  index = filterModel->mapToMaster( filterModel->sourceModel()->index( 0, 3 ) );
  QCOMPARE( index.column(), 0 );
}

void TestQgsAttributeTable::testFilteredFeatures()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
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

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

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
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=col1:int&field=col2:int" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsFeature f1( tempLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( 0, 1 );
  f1.setAttribute( 1, 2 );

  QgsFeature f2( tempLayer->dataProvider()->fields(), 2 );
  f2.setAttribute( 0, 2 );
  f2.setAttribute( 1, 4 );

  QVERIFY( tempLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll ) );

  tempLayer->selectByIds( QgsFeatureIds() << 1 << 2 );

  dlg->mActionCopySelectedRows_triggered();

  QgsClipboard *clipboard = QgisApp::instance()->clipboard();
  QVERIFY( clipboard );
  QVERIFY( !clipboard->isEmpty() );
  QCOMPARE( clipboard->fields().names(), QStringList() << "pk" << "col1" << "col2" );

  const QgsFeatureList features = clipboard->copyOf();
  QCOMPARE( features.count(), 2 );

  const int feature1Index = features.at( 0 ).attribute( 0 ).toInt() == 1 ? 0 : 1;
  const int feature2Index = feature1Index == 0 ? 1 : 0;

  QCOMPARE( features.at( feature1Index ).attribute( 0 ), 1 );
  QCOMPARE( features.at( feature1Index ).attribute( "col1" ), 2 );
  QCOMPARE( features.at( feature1Index ).attribute( "col2" ), QVariant() );
  QCOMPARE( features.at( feature2Index ).attribute( "pk" ), 2 );
  QCOMPARE( features.at( feature2Index ).attribute( "col1" ), 4 );
  QCOMPARE( features.at( feature2Index ).attribute( 2 ), QVariant() );

  QCOMPARE( clipboard->crs().authid(), "EPSG:3111" );
}

void TestQgsAttributeTable::testOpenWithFilterExpression()
{
  // test attribute table opening in show feature visible mode
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:date" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsPolylineXY line;
  line << QgsPointXY( 0, 0 ) << QgsPointXY( 1, 1 );
  QgsGeometry geometry = QgsGeometry::fromPolylineXY( line );
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

  const QString filterExpression = QStringLiteral( "col1 < to_date('2020-02-03')" );
  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowFilteredList, nullptr, Qt::Window, nullptr, filterExpression ) );

  // feature id 2 is filtered out due not matching the provided filter expression
  QCOMPARE( dlg->mMainView->filteredFeatures(), QgsFeatureIds() << 1 << 3 );
}

void TestQgsAttributeTable::testInvalidView()
{
  std::unique_ptr<QgsVectorLayer> tempLayer( new QgsVectorLayer( QStringLiteral( "LineString?crs=epsg:4326&field=pk:int&field=col1:date" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) ) );
  QVERIFY( tempLayer->isValid() );

  QgsPolylineXY line;
  line << QgsPointXY( 0, 0 ) << QgsPointXY( 1, 1 );
  QgsGeometry geometry = QgsGeometry::fromPolylineXY( line );
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

  const QString filterExpression = QStringLiteral( "col1 >= to_date('2020-02-03')" );
  tempLayer->setConstraintExpression( 1, QStringLiteral( "col1 >= to_date('2020-02-03')" ) );
  tempLayer->setFieldConstraint( 1, QgsFieldConstraints::ConstraintExpression, QgsFieldConstraints::ConstraintStrengthHard );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( tempLayer.get(), QgsAttributeTableFilterModel::ShowAll, nullptr, Qt::Window, nullptr, filterExpression ) );
  dlg->mFeatureFilterWidget->filterInvalid();

  // feature id 2 is filtered out due not matching the provided filter expression
  QCOMPARE( dlg->mMainView->filteredFeatures(), QgsFeatureIds() << 1 << 3 );
}

void TestQgsAttributeTable::testEnsureEditSelection()
{
  std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( QStringLiteral( "Point?field=col0:integer&field=col1:integer" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );
  QVERIFY( layer->isValid() );

  QgsFeature ft1( layer->dataProvider()->fields(), 1 );
  ft1.setAttributes( QgsAttributes() << 1 << 2 );
  layer->dataProvider()->addFeature( ft1 );
  QgsFeature ft2( layer->dataProvider()->fields(), 2 );
  ft2.setAttributes( QgsAttributes() << 3 << 4 );
  layer->dataProvider()->addFeature( ft2 );
  QgsFeature ft3( layer->dataProvider()->fields(), 3 );
  ft3.setAttributes( QgsAttributes() << 5 << 6 );
  layer->dataProvider()->addFeature( ft3 );
  QgsFeature ft4( layer->dataProvider()->fields(), 4 );
  ft4.setAttributes( QgsAttributes() << 7 << 8 );
  layer->dataProvider()->addFeature( ft4 );

  layer->removeSelection();

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( layer.get() ) );

  //since the update is done by timer, we have to wait (at least one millisecond) or until the current edit selection changed
  qRegisterMetaType<QgsFeature>( "QgsFeature&" );
  QSignalSpy spy( dlg->mMainView->mFeatureListView, &QgsFeatureListView::currentEditSelectionChanged );

  // we set the index to ft3
  dlg->mMainView->setCurrentEditSelection( { ft3.id() } );
  // ... and the currentEditSelection is on ft3
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 3 ) );

  // we make a featureselection on ft1, ft2 and ft3
  layer->selectByIds( QgsFeatureIds() << 1 << 2 << 3 );
  spy.wait( 1 );
  // ... and the currentEditSelection stays on ft3 (since it's in the featureselection)
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 3 ) );

  // we release the featureselection
  layer->removeSelection();
  spy.wait( 1 );
  // ... and the currentEditSelection persists on 3 (since it does not make an update)
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 3 ) );

  // we make afeatureselection on ft4
  layer->selectByIds( QgsFeatureIds() << 4 );
  spy.wait( 1 );
  // ... and the currentEditSelection goes to ft4
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 4 ) );

  // we make afeatureselection on ft2 and ft3
  layer->selectByIds( QgsFeatureIds() << 2 << 3 );
  spy.wait( 1 );
  // ... and the currentEditSelection goes to the first one of the featureselection (means ft2)
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 2 ) );

  // we reload the layer
  layer->reload();
  spy.wait( 1 );
  // ... and the currentEditSelection stays on 2 (since lastEditSelectionFid is persisted)
  QVERIFY( dlg->mMainView->mFeatureListView->currentEditSelection().contains( 2 ) );
}

void TestQgsAttributeTable::testFetchAllAttributes()
{
  QString pointFileName = TEST_DATA_DIR + QStringLiteral( "/points.shp" );
  std::unique_ptr<QgsVectorLayer> layer = std::make_unique<QgsVectorLayer>( pointFileName );
  QVERIFY( layer->isValid() );

  QgsAttributeTableConfig config { layer->attributeTableConfig() };
  config.setColumnHidden( 1, true );
  layer->setAttributeTableConfig( config );

  std::unique_ptr<QgsAttributeTableDialog> dlg( new QgsAttributeTableDialog( layer.get() ) );

  QCOMPARE( dlg->mMainView->masterModel()->data( dlg->mMainView->masterModel()->index( 0, 0 ), Qt::DisplayRole ).toString(), "Jet" );
  QCOMPARE( dlg->mMainView->masterModel()->data( dlg->mMainView->masterModel()->index( 0, 1 ), Qt::DisplayRole ).toString(), "90" );
  QCOMPARE( dlg->mMainView->masterModel()->data( dlg->mMainView->masterModel()->index( 0, 2 ), Qt::DisplayRole ).toString(), "3.000" );
}

QGSTEST_MAIN( TestQgsAttributeTable )
#include "testqgsattributetable.moc"
