/***************************************************************************
    testqgsvaluerelationwidgetwrapper.cpp
     --------------------------------------
    Date                 : 21 07 2017
    Copyright            : (C) 2017 Paul Blottiere
    Email                : paul dot blottiere at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include <QScrollBar>
#include <QSignalSpy>

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include "qgseditorwidgetwrapper.h"
#include <editorwidgets/qgsvaluerelationwidgetwrapper.h>
#include <QTableWidget>
#include <QComboBox>
#include "qgsgui.h"
#include <gdal_version.h>
#include <nlohmann/json.hpp>

class TestQgsValueRelationWidgetWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsValueRelationWidgetWrapper() = default;

  private:
    QTemporaryDir tempDir;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testScrollBarUnlocked();
    void testDrillDown();
    void testDrillDownMulti();
    //! Checks that a related value of 0 is not interpreted as a NULL
    void testZeroIndexInRelatedTable();
    void testWithJsonInPostgres();
    void testWithJsonInGPKG();
    void testWithTextInGPKG();
    void testWithTextInGPKGTextFk();
    void testWithTextInGPKGWeirdTextFk();
    void testWithJsonInSpatialite();
    void testWithJsonInSpatialiteTextFk();
    void testMatchLayerName();
    //! Check that setFeature works correctly after regression #42003
    void testRegressionGH42003();
};

void TestQgsValueRelationWidgetWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
}

void TestQgsValueRelationWidgetWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsValueRelationWidgetWrapper::init()
{
}

void TestQgsValueRelationWidgetWrapper::cleanup()
{

}

void TestQgsValueRelationWidgetWrapper::testScrollBarUnlocked()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "LineString?crs=epsg:3111&field=pk:int&field=fk|:int" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );

  // build a value relation widget wrapper
  QgsValueRelationWidgetWrapper w( &vl1, 0, nullptr, nullptr );

  QVariantMap config;
  config.insert( QStringLiteral( "AllowMulti" ), true );
  w.setConfig( config );
  w.widget();
  w.setEnabled( true );

  // add an item virtually
  QTableWidgetItem item;
  item.setText( QStringLiteral( "MyText" ) );
  w.mTableWidget->setItem( 0, 0, &item );

  QCOMPARE( w.mTableWidget->item( 0, 0 )->text(), QString( "MyText" ) );

  // when the widget wrapper is enabled, the container should be enabled
  // as well as items
  w.setEnabled( true );

  QCOMPARE( w.widget()->isEnabled(), true );

  bool itemEnabled = w.mTableWidget->item( 0, 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, true );

  // when the widget wrapper is disabled, the container should still be enabled
  // to keep the scrollbar available but items should be disabled to avoid
  // edition
  w.setEnabled( false );

  itemEnabled = w.mTableWidget->item( 0, 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, false );

  QCOMPARE( w.widget()->isEnabled(), true );

  // recheck after re-enabled
  w.setEnabled( true );

  QCOMPARE( w.widget()->isEnabled(), true );
  itemEnabled = w.mTableWidget->item( 0, 0 )->flags() & Qt::ItemIsEnabled;
  QCOMPARE( itemEnabled, true );
}

void TestQgsValueRelationWidgetWrapper::testDrillDown()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // insert some features
  QgsFeature f1( vl1.fields() );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "province" ), 123 );
  f1.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Some Place By The River" ) );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  QVERIFY( f1.isValid() );
  QgsFeature f2( vl1.fields() );
  f2.setAttribute( QStringLiteral( "pk" ), 2 );
  f2.setAttribute( QStringLiteral( "province" ), 245 );
  f2.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Dreamland By The Clouds" ) );
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 1 0, 1 1, 2 1, 2 0, 1 0 ))" ) ) );
  QVERIFY( f2.isValid() );
  QVERIFY( vl1.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  QgsFeature f3( vl2.fields() );
  f3.setAttribute( QStringLiteral( "fk_province" ), 123 );
  f3.setAttribute( QStringLiteral( "fk_municipality" ), 1 );
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 0.5 0.5)" ) ) );
  QVERIFY( f3.isValid() );
  QVERIFY( f3.geometry().isGeosValid() );
  QVERIFY( vl2.dataProvider()->addFeature( f3 ) );

  // build a value relation widget wrapper for municipality
  QgsValueRelationWidgetWrapper w_municipality( &vl2, vl2.fields().indexOf( QLatin1String( "fk_municipality" ) ), nullptr, nullptr );
  QVariantMap cfg_municipality;
  cfg_municipality.insert( QStringLiteral( "Layer" ), vl1.id() );
  cfg_municipality.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_municipality.insert( QStringLiteral( "Value" ), QStringLiteral( "municipality" ) );
  cfg_municipality.insert( QStringLiteral( "AllowMulti" ), false );
  cfg_municipality.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_municipality.insert( QStringLiteral( "AllowNull" ), false );
  cfg_municipality.insert( QStringLiteral( "OrderByValue" ), true );
  cfg_municipality.insert( QStringLiteral( "FilterExpression" ), QStringLiteral( "\"province\" = current_value('fk_province')" ) );
  cfg_municipality.insert( QStringLiteral( "UseCompleter" ), false );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.widget();
  w_municipality.setEnabled( true );

  QCOMPARE( w_municipality.mCache.size(), 2 );
  QCOMPARE( w_municipality.mComboBox->count(), 2 );

  // Set a feature
  w_municipality.setFeature( vl2.getFeature( 1 ) );
  QCOMPARE( w_municipality.mCache.size(), 1 );
  QCOMPARE( w_municipality.mComboBox->count(), 1 );

  // check that valueChanged signal is correctly triggered
  const QSignalSpy spy( &w_municipality, &QgsEditorWidgetWrapper::valuesChanged );

  w_municipality.setFeature( f3 );
  QCOMPARE( spy.count(), 1 );
  QCOMPARE( w_municipality.mCache.size(), 1 );

  // Check first is selected
  QCOMPARE( w_municipality.mComboBox->count(), 1 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.value().toString(), QStringLiteral( "1" ) );

  // Filter by geometry
  cfg_municipality[ QStringLiteral( "FilterExpression" ) ] = QStringLiteral( "contains(buffer(@current_geometry, 1 ), $geometry)" );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mComboBox->count(), 1 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Some Place By The River" ) );

  // Move the point to 1.5 0.5
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 1.5 0.5)" ) ) );
  w_municipality.setFeature( f3 );
  // this shouldn't force change the existing value, but rather show it as a "invalid" value surrounded by (...)
  QCOMPARE( w_municipality.value().toInt(), 1 );
  QCOMPARE( w_municipality.mComboBox->count(), 2 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mComboBox->itemText( 1 ), QStringLiteral( "(1)" ) );
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 1 );

  // Enlarge the buffer
  cfg_municipality[ QStringLiteral( "FilterExpression" ) ] = QStringLiteral( "contains(buffer(@current_geometry, 3 ), $geometry)" );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mComboBox->count(), 2 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mComboBox->itemText( 1 ), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.value().toInt(), 1 );
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 1 );

  // Check with allow null
  cfg_municipality[QStringLiteral( "AllowNull" )] = true;
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( QgsFeature() );

  // Check null is selected
  QCOMPARE( w_municipality.mComboBox->count(), 3 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "(no selection)" ) );
  QVERIFY( w_municipality.value().isNull() );
  QCOMPARE( w_municipality.value().toString(), QString() );

  // Check order by value false
  cfg_municipality[QStringLiteral( "AllowNull" )] = false;
  cfg_municipality[QStringLiteral( "OrderByValue" )] = false;
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mComboBox->itemText( 1 ), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Some Place By The River" ) );

}

void TestQgsValueRelationWidgetWrapper::testDrillDownMulti()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:string" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // insert some features
  QgsFeature f1( vl1.fields() );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "province" ), 123 );
  f1.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Some Place By The River" ) );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  QVERIFY( f1.isValid() );
  QgsFeature f2( vl1.fields() );
  f2.setAttribute( QStringLiteral( "pk" ), 2 );
  f2.setAttribute( QStringLiteral( "province" ), 245 );
  f2.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Dreamland By The Clouds" ) );
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 1 0, 1 1, 2 1, 2 0, 1 0 ))" ) ) );
  QVERIFY( f2.isValid() );
  QVERIFY( vl1.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  QgsFeature f3( vl2.fields() );
  f3.setAttribute( QStringLiteral( "fk_province" ), 123 );
  f3.setAttribute( QStringLiteral( "fk_municipality" ), QStringLiteral( "{1}" ) );
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 0.5 0.5)" ) ) );
  QVERIFY( f3.isValid() );
  QVERIFY( f3.geometry().isGeosValid() );
  QVERIFY( vl2.dataProvider()->addFeature( f3 ) );

  // build a value relation widget wrapper for municipality
  QgsValueRelationWidgetWrapper w_municipality( &vl2, vl2.fields().indexOf( QLatin1String( "fk_municipality" ) ), nullptr, nullptr );
  QVariantMap cfg_municipality;
  cfg_municipality.insert( QStringLiteral( "Layer" ), vl1.id() );
  cfg_municipality.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_municipality.insert( QStringLiteral( "Value" ), QStringLiteral( "municipality" ) );
  cfg_municipality.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_municipality.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_municipality.insert( QStringLiteral( "AllowNull" ), false );
  cfg_municipality.insert( QStringLiteral( "OrderByValue" ), true );
  cfg_municipality.insert( QStringLiteral( "FilterExpression" ), QStringLiteral( "\"province\" =  current_value('fk_province')" ) );
  cfg_municipality.insert( QStringLiteral( "UseCompleter" ), false );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.widget();
  w_municipality.setEnabled( true );

  QCOMPARE( w_municipality.mCache.size(), 2 );
  QCOMPARE( w_municipality.mTableWidget->rowCount(), 2 );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mCache.size(), 1 );

  QCOMPARE( w_municipality.mTableWidget->rowCount(), 1 );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.value(), QVariant( QStringLiteral( "{1}" ) ) );

  // Filter by geometry
  cfg_municipality[ QStringLiteral( "FilterExpression" ) ] = QStringLiteral( "contains(buffer(@current_geometry, 1 ), $geometry)" );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mTableWidget->rowCount(), 1 );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Some Place By The River" ) );

  // Move the point to 1.5 0.5
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 1.5 0.5)" ) ) );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mTableWidget->rowCount(), 1 );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Dreamland By The Clouds" ) );

  // Enlarge the buffer
  cfg_municipality[ QStringLiteral( "FilterExpression" ) ] = QStringLiteral( "contains(buffer(@current_geometry, 3 ), $geometry)" );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mTableWidget->rowCount(), 2 );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_municipality.value(), QVariant( QStringLiteral( "{1}" ) ) );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  w_municipality.setValues( QStringLiteral( "{1,2}" ), QVariantList() );
  QCOMPARE( w_municipality.value(), QVariant( QStringLiteral( "{2,1}" ) ) );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );

  // Check with passing a variant list
  w_municipality.setValues( QVariantList( {1, 2} ), QVariantList() );
  QCOMPARE( w_municipality.value(), QVariant( QStringLiteral( "{2,1}" ) ) );

  // Check values are checked
  f3.setAttribute( QStringLiteral( "fk_municipality" ), QStringLiteral( "{1,2}" ) );
  w_municipality.setFeature( f3 );
  QCOMPARE( w_municipality.mTableWidget->rowCount(), 2 );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_municipality.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_municipality.value(), QVariant( QStringLiteral( "{2,1}" ) ) );
}

void TestQgsValueRelationWidgetWrapper::testZeroIndexInRelatedTable()
{
  // findData fails to tell a 0 from a NULL
  // See: "Value relation, value 0 = NULL" - https://github.com/qgis/QGIS/issues/27803

  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // insert some features
  QgsFeature f1( vl1.fields() );
  f1.setAttribute( QStringLiteral( "pk" ), 0 );  // !!! Notice: pk 0
  f1.setAttribute( QStringLiteral( "province" ), 123 );
  f1.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Some Place By The River" ) );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  QVERIFY( f1.isValid() );
  QgsFeature f2( vl1.fields() );
  f2.setAttribute( QStringLiteral( "pk" ), 2 );
  f2.setAttribute( QStringLiteral( "province" ), 245 );
  f2.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Dreamland By The Clouds" ) );
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 1 0, 1 1, 2 1, 2 0, 1 0 ))" ) ) );
  QVERIFY( f2.isValid() );
  QVERIFY( vl1.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  QgsFeature f3( vl2.fields() );
  f3.setAttribute( QStringLiteral( "fk_province" ), 123 );
  f3.setAttribute( QStringLiteral( "fk_municipality" ), QStringLiteral( "0" ) );
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 0.5 0.5)" ) ) );
  QVERIFY( f3.isValid() );
  QVERIFY( f3.geometry().isGeosValid() );
  QVERIFY( vl2.dataProvider()->addFeature( f3 ) );

  // build a value relation widget wrapper for municipality
  QgsValueRelationWidgetWrapper w_municipality( &vl2, vl2.fields().indexOf( QLatin1String( "fk_municipality" ) ), nullptr, nullptr );
  QVariantMap cfg_municipality;
  cfg_municipality.insert( QStringLiteral( "Layer" ), vl1.id() );
  cfg_municipality.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_municipality.insert( QStringLiteral( "Value" ), QStringLiteral( "municipality" ) );
  cfg_municipality.insert( QStringLiteral( "AllowMulti" ), false );
  cfg_municipality.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_municipality.insert( QStringLiteral( "AllowNull" ), true );
  cfg_municipality.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_municipality.insert( QStringLiteral( "UseCompleter" ), false );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.widget();
  w_municipality.setEnabled( true );

  w_municipality.setValues( 0, QVariantList() );
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 1 );
  QCOMPARE( w_municipality.mComboBox->currentText(), QStringLiteral( "Some Place By The River" ) );
}

void TestQgsValueRelationWidgetWrapper::testWithJsonInPostgres()
{
#ifdef ENABLE_PGTEST
  //this is only reading

  // create pg layers
  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsVectorLayer *vl_json = new QgsVectorLayer( QStringLiteral( "%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"json\" sql=" ).arg( dbConn ), QStringLiteral( "json" ), QStringLiteral( "postgres" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( QStringLiteral( "%1 sslmode=disable key='pk' table=\"qgis_test\".\"authors\" sql=" ).arg( dbConn ), QStringLiteral( "authors" ), QStringLiteral( "postgres" ) );
  QVERIFY( vl_json->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );

  QCOMPARE( vl_json->fields().at( 1 ).type(), QVariant::Map );

  // build a value relation widget wrapper for json field
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_json, vl_json->fields().indexOf( QLatin1String( "jvalue" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "name" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 7 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6" ) );

  //check if first feature checked correctly (should be 1,2,3 and the rest is not)
  w_favoriteauthors.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  // build a value relation widget wrapper for jsonb field
  QgsValueRelationWidgetWrapper w_favoriteauthors_b( vl_json, vl_json->fields().indexOf( QLatin1String( "jbvalue" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors_b;
  cfg_favoriteauthors_b.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors_b.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_favoriteauthors_b.insert( QStringLiteral( "Value" ), QStringLiteral( "name" ) );
  cfg_favoriteauthors_b.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors_b.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors_b.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors_b.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors_b.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors_b.setConfig( cfg_favoriteauthors_b );
  w_favoriteauthors_b.widget();
  w_favoriteauthors_b.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors_b.mTableWidget->rowCount(), 7 );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6" ) );

  //check if second feature checked correctly (should be 4,5,6 and the rest is not)
  w_favoriteauthors_b.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 3, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_b.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );

  // check value from widget wrapper
  QCOMPARE( w_favoriteauthors_b.value().toStringList(), QStringList() << "4" << "5" << "6" );
#endif
}

void TestQgsValueRelationWidgetWrapper::testWithJsonInGPKG()
{
  // create ogr gpkg layers
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  QFile::copy( myFileName + "/provider/test_json.gpkg", myTempDirName + "/test_json.gpkg" );
  const QString myTempFileName = myTempDirName + "/test_json.gpkg";
  const QFileInfo myMapFileInfo( myTempFileName );
  QgsVectorLayer *vl_json = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=author", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_json->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_json->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_json, vl_json->fields().indexOf( QLatin1String( "json_content" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "fid" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "NAME" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 6 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6" ) );

  w_favoriteauthors.setFeature( vl_json->getFeature( 1 ) );

  //check if first feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //check other authors
  w_favoriteauthors.mTableWidget->item( 0, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 2, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly 0, 2, 4 (means the fids 1, 3, 5)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //we do jump over the part with QgsAttributeForm::saveEdits
  vl_json->changeAttributeValue( 1, 4, w_favoriteauthors.value() );

  w_favoriteauthors.setFeature( vl_json->getFeature( 2 ) );
  //check if second feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  w_favoriteauthors.setFeature( vl_json->getFeature( 1 ) );
  //check if first feature checked correctly 0, 2, 4
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  // check if stored correctly
  vl_json->commitChanges();
  QVariantList expected_vl;
  expected_vl << "1" << "3" << "5";

  const QgsFeature f = vl_json->getFeature( 1 );
  const QVariant attribute = f.attribute( QStringLiteral( "json_content" ) );
  const QList<QVariant> value = attribute.toList();
  QCOMPARE( value, expected_vl );
}

// Same test procedure like in testWithJsonInGPKG to check the non-json way of storing multi-selections into formatted strings
void TestQgsValueRelationWidgetWrapper::testWithTextInGPKG()
{
  // create ogr gpkg layers
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  QFile::copy( myFileName + "/provider/test_json.gpkg", myTempDirName + "/test_json.gpkg" );
  const QString myTempFileName = myTempDirName + "/test_json.gpkg";
  const QFileInfo myMapFileInfo( myTempFileName );
  QgsVectorLayer *vl_text = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=author", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_text, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_text->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_text, vl_text->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "fid" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "NAME" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 6 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6" ) );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );

  //check if first feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //check other authors
  w_favoriteauthors.mTableWidget->item( 0, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 2, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly 0, 2, 4 (means the fids 1, 3, 5)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //we do jump over the part with QgsAttributeForm::saveEdits
  vl_text->changeAttributeValue( 1, 3, w_favoriteauthors.value() );

  w_favoriteauthors.setFeature( vl_text->getFeature( 2 ) );
  //check if second feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );
  //check if first feature checked correctly 0, 2, 4
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  // check if stored correctly

  vl_text->commitChanges();
  const QString expected_string QStringLiteral( "{1,3,5}" );

  const QgsFeature f = vl_text->getFeature( 1 );
  const QVariant attribute = f.attribute( QStringLiteral( "PRFEDEA" ) );
  const QString value = attribute.toString();
  QCOMPARE( value, expected_string );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );

  //check if first feature checked correctly 0, 2, 4 (means the fids 1, 3, 5) after the commit
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //reread completely
  QgsVectorLayer *vl_text_reread = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text_reread->isValid() );

  QgsProject::instance()->addMapLayer( vl_text_reread, false, false );
  vl_text_reread->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors_reread( vl_text_reread, vl_text->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  w_favoriteauthors_reread.setConfig( cfg_favoriteauthors );
  w_favoriteauthors_reread.widget();
  w_favoriteauthors_reread.setEnabled( true );

  w_favoriteauthors_reread.setFeature( vl_text_reread->getFeature( 1 ) );

  //check if first feature on new layer checked correctly 0, 2, 4 (means the fids 1, 3, 5) after the reread
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
}

// Storing of strings as key and handle the quotes
void TestQgsValueRelationWidgetWrapper::testWithTextInGPKGTextFk()
{
  // create ogr gpkg layers
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  QFile::copy( myFileName + "/provider/test_json.gpkg", myTempDirName + "/test_json.gpkg" );
  const QString myTempFileName = myTempDirName + "/test_json.gpkg";
  const QFileInfo myMapFileInfo( myTempFileName );
  QgsVectorLayer *vl_text = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=author", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_text, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_text->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_text, vl_text->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "NAME" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "NAME" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 6 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "Richard Helm" ) );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );

  //check if first feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //check other authors 0, 2, 4 (means the names "Douglas Adams", "John Vliessides", "Ralph Johnson")
  w_favoriteauthors.mTableWidget->item( 0, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 2, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly 0, 2, 4 (means the names "Douglas Adams", "John Vliessides", "Ralph Johnson")
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //we do jump over the part with QgsAttributeForm::saveEdits
  vl_text->changeAttributeValue( 1, 3, w_favoriteauthors.value() );

  w_favoriteauthors.setFeature( vl_text->getFeature( 2 ) );
  //check if second feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );
  //check if first feature checked correctly 0, 2, 4
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  // check if stored correctly
  vl_text->commitChanges();
  QString expected_string = QStringLiteral( "{\"Douglas Adams\",\"John Vlissides\",\"Ralph Johnson\"}" );

  QgsFeature f = vl_text->getFeature( 1 );
  QVariant attribute = f.attribute( QStringLiteral( "PRFEDEA" ) );
  QString value = attribute.toString();
  QCOMPARE( value, expected_string );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );

  //check if first feature checked correctly 0, 2, 4 (means the names "Douglas Adams", "John Vliessides", "Ralph Johnson") after the commit
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //reread completely
  QgsVectorLayer *vl_text_reread = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text_reread->isValid() );

  QgsProject::instance()->addMapLayer( vl_text_reread, false, false );
  vl_text_reread->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors_reread( vl_text_reread, vl_text_reread->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  w_favoriteauthors_reread.setConfig( cfg_favoriteauthors );
  w_favoriteauthors_reread.widget();
  w_favoriteauthors_reread.setEnabled( true );

  w_favoriteauthors_reread.setFeature( vl_text_reread->getFeature( 1 ) );

  //check if first feature on new layer checked correctly 0, 2, 4(means the names "Douglas Adams", "John Vliessides", "Ralph Johnson") after the reread
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //we store data wrongly (like it has possibly been on legacy systems)
  vl_text_reread->changeAttributeValue( 1, 3, "{Douglas Adams,John Vlissides,Ralph Johnson}" );

  // check if stored correctly
  vl_text_reread->commitChanges();
  expected_string = QStringLiteral( "{Douglas Adams,John Vlissides,Ralph Johnson}" );

  f = vl_text_reread->getFeature( 1 );
  attribute = f.attribute( QStringLiteral( "PRFEDEA" ) );
  value = attribute.toString();
  QCOMPARE( value, expected_string );

  w_favoriteauthors.setFeature( vl_text_reread->getFeature( 1 ) );

  //check if first feature checked correctly 0, 2, 4 (means the names "Douglas Adams", "John Vlissides", "Ralph Johnson") after the commit
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_reread.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //reread completely
  QgsVectorLayer *vl_text_reread2 = new QgsVectorLayer( myMapFileInfo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text_reread2->isValid() );

  QgsProject::instance()->addMapLayer( vl_text_reread2, false, false );
  vl_text_reread2->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors_reread2( vl_text_reread2, vl_text_reread2->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  w_favoriteauthors_reread2.setConfig( cfg_favoriteauthors );
  w_favoriteauthors_reread2.widget();
  w_favoriteauthors_reread2.setEnabled( true );

  w_favoriteauthors_reread2.setFeature( vl_text_reread2->getFeature( 1 ) );

  //check if first feature on new layer checked correctly 0, 2, 4(means the names "Douglas Adams", "John Vliessides", "Ralph Johnson") after the reread
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors_reread2.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
}

// Storing of strings as key and handle the quotes
void TestQgsValueRelationWidgetWrapper::testWithTextInGPKGWeirdTextFk()
{
  // create ogr gpkg layer for foo (vl_text)
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  QString myTempFileName = myTempDirName + "/test_json.gpkg";
  QFile::copy( myFileName + "/provider/test_json.gpkg", myTempFileName );
  const QFileInfo myMapFileInfoFoo( myTempFileName );
  QgsVectorLayer *vl_text = new QgsVectorLayer( myMapFileInfoFoo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text->isValid() );

  // create ogr spatialite layer for authors with weird signs (vl_authors)
  myTempFileName = myTempDirName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" );
  QFile::copy( myFileName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" ),
               myTempFileName );
  const QFileInfo myMapFileInfoAuthor( myTempFileName );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( QStringLiteral( R"(dbname='%1' table="%2")" )
      .arg( myMapFileInfoAuthor.filePath() ).arg( QLatin1String( "authors" ) ),
      QStringLiteral( "test" ),
      QStringLiteral( "spatialite" ) );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_text, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_text->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_text, vl_text->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk_text" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "name" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 7 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2helm,comma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3johnson\"quote" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5adams'singlequote" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6follett{}" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "7garc%1a][" ).arg( QChar( 0x00EC ) ) );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );

  //check if first feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  //check authors 1,2,4,5,6 means all the super weird ones: "2helm,comma", "3johnson\"quote", "5adams'singlequote", "6follett{}", "7garcìa]["
  w_favoriteauthors.mTableWidget->item( 1, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 2, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 5, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 6, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly 1,2,4,5,6
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );

  //we do jump over the part with QgsAttributeForm::saveEdits
  vl_text->changeAttributeValue( 1, 3, w_favoriteauthors.value() );

  //check if everything set correctly
  QCOMPARE( w_favoriteauthors.value(), QVariant( QStringLiteral( "{\"2helm,comma\",\"3johnson\\\"quote\",\"5adams'singlequote\",\"6follett{}\",\"7garc%1a][\"}" ).arg( QChar( 0x00EC ) ) ) );

  w_favoriteauthors.setFeature( vl_text->getFeature( 2 ) );
  //check if second feature checked correctly (none)
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );

  w_favoriteauthors.setFeature( vl_text->getFeature( 1 ) );
  //check if first feature checked correctly 1,2,4,5,6
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );

  // check if stored correctly
  vl_text->commitChanges();
  const QString expected_string = QStringLiteral( "{\"2helm,comma\",\"3johnson\\\"quote\",\"5adams'singlequote\",\"6follett{}\",\"7garc%1a][\"}" ).arg( QChar( 0x00EC ) );

  const QgsFeature f = vl_text->getFeature( 1 );
  const QVariant attribute = f.attribute( QStringLiteral( "PRFEDEA" ) );
  const QString value = attribute.toString();
  QCOMPARE( value, expected_string );

  //reread completely
  QgsVectorLayer *vl_text_reread = new QgsVectorLayer( myMapFileInfoFoo.filePath() + "|layername=foo", "test", QStringLiteral( "ogr" ) );
  QVERIFY( vl_text_reread->isValid() );

  QgsProject::instance()->addMapLayer( vl_text_reread, false, false );
  vl_text_reread->startEditing();

  // build a value relation widget wrapper for authors
  QgsValueRelationWidgetWrapper w_favoriteauthors_reread( vl_text_reread, vl_text_reread->fields().indexOf( QLatin1String( "PRFEDEA" ) ), nullptr, nullptr );
  w_favoriteauthors_reread.setConfig( cfg_favoriteauthors );
  w_favoriteauthors_reread.widget();
  w_favoriteauthors_reread.setEnabled( true );

  w_favoriteauthors_reread.setFeature( vl_text_reread->getFeature( 1 ) );

  //check if first feature on new layer checked correctly 1,2,4,5,6 after reread
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Checked );
}

void TestQgsValueRelationWidgetWrapper::testWithJsonInSpatialite()
{
  const auto fk_field { QStringLiteral( "json_content" ) };
  // create ogr gpkg layers
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  const QString myTempFileName = myTempDirName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" );
  QFile::copy( myFileName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" ),
               myTempFileName );
  const QFileInfo myMapFileInfo( myTempFileName );
  QgsVectorLayer *vl_json = new QgsVectorLayer( QStringLiteral( R"(dbname='%1' table="%2")" )
      .arg( myMapFileInfo.filePath() ).arg( QLatin1String( "json" ) ),
      QStringLiteral( "test" ),
      QStringLiteral( "spatialite" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( QStringLiteral( R"(dbname='%1' table="%2")" )
      .arg( myMapFileInfo.filePath() ).arg( QLatin1String( "authors" ) ),
      QStringLiteral( "test" ),
      QStringLiteral( "spatialite" ) );
  const auto fk_field_idx { vl_json->fields().indexOf( fk_field ) };

  QVERIFY( vl_json->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_json->startEditing();

  // build a value relation widget wrapper for authors
  // fk_field is a json array type
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_json, fk_field_idx, nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "name" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 7 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  /* Test data:
  pk: 1 [1,3]
  pk: 2 [2,5]
  pk: 3 [4,6,7]
  pk: 4 NULL
  pk: 5 blank
  */

  // FEATURE 1
  w_favoriteauthors.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( { 1, 3 } ) ) );
  //check if first feature checked correctly (1,3)                                          pk
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );   // 1
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked ); // 2
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );   // 3
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked ); // 4
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked ); // 5
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked ); // 6
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked ); // 7

  //check other authors
  w_favoriteauthors.mTableWidget->item( 1, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly (1,2,3,5)
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( {1, 2, 3, 5} ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  vl_json->changeAttributeValue( 1, fk_field_idx, w_favoriteauthors.value() );
  // check if stored correctly
  vl_json->commitChanges();
  QVariantList expected_vl;
  expected_vl << "1" << "2" << "3" << "5";
  const QgsFeature f = vl_json->getFeature( 1 );
  const QVariant attribute = f.attribute( fk_field );
  const QList<QVariant> value = attribute.toList();
  QCOMPARE( value, expected_vl );

  // FEATURE 2
  w_favoriteauthors.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( {2, 5} ) ) );
  //check if second feature checked correctly
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  // FEATURE 4
  w_favoriteauthors.setFeature( vl_json->getFeature( 4 ) );
  // Because allowNull is false we have a NULL variant here
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariant::Type::List ) );
  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = true;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  //check if first feature checked correctly (empty list)
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList() ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );
  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = false;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );

  // FEATURE 5
  w_favoriteauthors.setFeature( vl_json->getFeature( 5 ) );
  // Because allowNull is false we have a NULL variant here
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariant::Type::List ) );

  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = true;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  //check if first feature checked correctly (empty list)
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );
}


void TestQgsValueRelationWidgetWrapper::testWithJsonInSpatialiteTextFk()
{
  const auto fk_field { QStringLiteral( "json_content_text" ) };
  // create ogr gpkg layers
  const QString myFileName( TEST_DATA_DIR ); //defined in CmakeLists.txt
  const QString myTempDirName = tempDir.path();
  const QString myTempFileName = myTempDirName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" );
  QFile::copy( myFileName + QStringLiteral( "/valuerelation_widget_wrapper_test.spatialite.sqlite" ),
               myTempFileName );
  const QFileInfo myMapFileInfo( myTempFileName );
  QgsVectorLayer *vl_json = new QgsVectorLayer( QStringLiteral( R"(dbname='%1' table="%2")" )
      .arg( myMapFileInfo.filePath() ).arg( QLatin1String( "json" ) ),
      QStringLiteral( "test" ),
      QStringLiteral( "spatialite" ) );
  QgsVectorLayer *vl_authors = new QgsVectorLayer( QStringLiteral( R"(dbname='%1' table="%2")" )
      .arg( myMapFileInfo.filePath() ).arg( QLatin1String( "authors" ) ),
      QStringLiteral( "test" ),
      QStringLiteral( "spatialite" ) );
  const auto fk_field_idx { vl_json->fields().indexOf( fk_field ) };

  QVERIFY( vl_json->isValid() );
  QVERIFY( vl_authors->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QgsProject::instance()->addMapLayer( vl_authors, false, false );
  vl_json->startEditing();

  // build a value relation widget wrapper for authors
  // fk_field is a json array type
  QgsValueRelationWidgetWrapper w_favoriteauthors( vl_json, fk_field_idx, nullptr, nullptr );
  QVariantMap cfg_favoriteauthors;
  cfg_favoriteauthors.insert( QStringLiteral( "Layer" ), vl_authors->id() );
  cfg_favoriteauthors.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk_text" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "Value" ), QStringLiteral( "name" ) );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowMulti" ), true );
  cfg_favoriteauthors.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_favoriteauthors.insert( QStringLiteral( "AllowNull" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_favoriteauthors.insert( QStringLiteral( "UseCompleter" ), false );
  w_favoriteauthors.setConfig( cfg_favoriteauthors );
  w_favoriteauthors.widget();
  w_favoriteauthors.setEnabled( true );

  //check if set up nice
  QCOMPARE( w_favoriteauthors.mTableWidget->rowCount(), 7 );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->text(), QStringLiteral( "Erich Gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "1gamma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->text(), QStringLiteral( "Richard Helm" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "2helm,comma" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->text(), QStringLiteral( "Ralph Johnson" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "3johnson\"quote" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->text(), QStringLiteral( "John Vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "4vlissides" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->text(), QStringLiteral( "Douglas Adams" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "5adams'singlequote" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->text(), QStringLiteral( "Ken Follett" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "6follett{}" ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->text(), QStringLiteral( "Gabriel Garc%1a M%2rquez" ).arg( QChar( 0x00ED ) ).arg( QChar( 0x00E1 ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->data( Qt::UserRole ).toString(), QStringLiteral( "7garc%1a][" ).arg( QChar( 0x00EC ) ) );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  /* Test data:

    Keys:
    "1gamma"
    "2helm,comma"
    "3johnson""quote"
    "4vlissides"
    "5adams'singlequote"
    "6follett{}"
    "7garcìa]["

    Data:
    1 "[1,3]" "[""1gamma"", ""3johnson\""quote""]
    2 "[2,5]" "[""2helm,comma"", ""5adams'singlequote""]"
    3 "[4,6,7]" "[""4vlissides"", ""6follett{}"" , ""7garca][""]"
    5
    7 ""  ""

  */

  // FEATURE 1
  w_favoriteauthors.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( { "1gamma", "3johnson\"quote" } ) ) );

  //check if first feature checked correctly (1,3)                                          pk
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );   // 1
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked ); // 2
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );   // 3
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked ); // 4
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked ); // 5
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked ); // 6
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked ); // 7

  //check other authors
  w_favoriteauthors.mTableWidget->item( 1, 0 )->setCheckState( Qt::Checked );
  w_favoriteauthors.mTableWidget->item( 4, 0 )->setCheckState( Qt::Checked );

  //check if first feature checked correctly (1,2,3,5) ) );
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( { "1gamma", "2helm,comma", "3johnson\"quote", "5adams'singlequote" } ) ) );

  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  vl_json->changeAttributeValue( 1, fk_field_idx, w_favoriteauthors.value() );
  // check if stored correctly
  vl_json->commitChanges();
  const QgsFeature f = vl_json->getFeature( 1 );
  const QVariant attribute = f.attribute( fk_field );
  const QVariantList value = attribute.toList();

  QCOMPARE( value, QVariantList( { "1gamma", "2helm,comma", "3johnson\"quote", "5adams'singlequote" } ) );

  // FEATURE 2
  w_favoriteauthors.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( { "2helm,comma", "5adams'singlequote" } ) ) );

  //check if second feature checked correctly
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Checked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );

  // FEATURE 4
  w_favoriteauthors.setFeature( vl_json->getFeature( 4 ) );

  // Because allowNull is false we have a NULL variant here
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariant::Type::List ) );
  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = true;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );

  //check if first feature checked correctly (NULL)
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList( ) ) );

  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );
  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = false;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );

  // FEATURE 5
  w_favoriteauthors.setFeature( vl_json->getFeature( 5 ) );

  // Because allowNull is false we have a NULL variant here
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariant::Type::List ) );
  cfg_favoriteauthors[ QStringLiteral( "AllowNull" ) ] = true;
  w_favoriteauthors.setConfig( cfg_favoriteauthors );

  //check if first feature checked correctly (empty list)
  QCOMPARE( w_favoriteauthors.value(), QVariant( QVariantList() ) );

  QCOMPARE( w_favoriteauthors.mTableWidget->item( 0, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 1, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 2, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 3, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 4, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 5, 0 )->checkState(), Qt::Unchecked );
  QCOMPARE( w_favoriteauthors.mTableWidget->item( 6, 0 )->checkState(), Qt::Unchecked );
}

void TestQgsValueRelationWidgetWrapper::testMatchLayerName()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // insert some features
  QgsFeature f1( vl1.fields() );
  f1.setAttribute( QStringLiteral( "pk" ), 0 );  // !!! Notice: pk 0
  f1.setAttribute( QStringLiteral( "province" ), 123 );
  f1.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Some Place By The River" ) );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  QVERIFY( f1.isValid() );
  QgsFeature f2( vl1.fields() );
  f2.setAttribute( QStringLiteral( "pk" ), 2 );
  f2.setAttribute( QStringLiteral( "province" ), 245 );
  f2.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Dreamland By The Clouds" ) );
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 1 0, 1 1, 2 1, 2 0, 1 0 ))" ) ) );
  QVERIFY( f2.isValid() );
  QVERIFY( vl1.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  QgsFeature f3( vl2.fields() );
  f3.setAttribute( QStringLiteral( "fk_province" ), 123 );
  f3.setAttribute( QStringLiteral( "fk_municipality" ), QStringLiteral( "0" ) );
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 0.5 0.5)" ) ) );
  QVERIFY( f3.isValid() );
  QVERIFY( f3.geometry().isGeosValid() );
  QVERIFY( vl2.dataProvider()->addFeature( f3 ) );

  // build a value relation widget wrapper for municipality
  QgsValueRelationWidgetWrapper w_municipality( &vl2, vl2.fields().indexOf( QLatin1String( "fk_municipality" ) ), nullptr, nullptr );
  QVariantMap cfg_municipality;
  cfg_municipality.insert( QStringLiteral( "Layer" ), QStringLiteral( "wrong_id_here_hope_name_is_good" ) );
  cfg_municipality.insert( QStringLiteral( "LayerName" ), vl1.name() );
  cfg_municipality.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_municipality.insert( QStringLiteral( "Value" ), QStringLiteral( "municipality" ) );
  cfg_municipality.insert( QStringLiteral( "AllowMulti" ), false );
  cfg_municipality.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_municipality.insert( QStringLiteral( "AllowNull" ), true );
  cfg_municipality.insert( QStringLiteral( "OrderByValue" ), false );
  cfg_municipality.insert( QStringLiteral( "UseCompleter" ), false );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.widget();
  w_municipality.setEnabled( true );

  w_municipality.setValues( 0, QVariantList() );
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 1 );
  QCOMPARE( w_municipality.mComboBox->currentText(), QStringLiteral( "Some Place By The River" ) );
}

void TestQgsValueRelationWidgetWrapper::testRegressionGH42003()
{
  // create a vector layer
  QgsVectorLayer vl1( QStringLiteral( "Polygon?crs=epsg:4326&field=pk:int&field=province:int&field=municipality:string" ), QStringLiteral( "vl1" ), QStringLiteral( "memory" ) );
  QgsVectorLayer vl2( QStringLiteral( "Point?crs=epsg:4326&field=pk:int&field=fk_province:int&field=fk_municipality:int" ), QStringLiteral( "vl2" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( &vl1, false, false );
  QgsProject::instance()->addMapLayer( &vl2, false, false );

  // insert some features
  QgsFeature f1( vl1.fields() );
  f1.setAttribute( QStringLiteral( "pk" ), 1 );
  f1.setAttribute( QStringLiteral( "province" ), 123 );
  f1.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Some Place By The River" ) );
  f1.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 0 0, 0 1, 1 1, 1 0, 0 0 ))" ) ) );
  QVERIFY( f1.isValid() );
  QgsFeature f2( vl1.fields() );
  f2.setAttribute( QStringLiteral( "pk" ), 2 );
  f2.setAttribute( QStringLiteral( "province" ), 245 );
  f2.setAttribute( QStringLiteral( "municipality" ), QStringLiteral( "Dreamland By The Clouds" ) );
  f2.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON(( 1 0, 1 1, 2 1, 2 0, 1 0 ))" ) ) );
  QVERIFY( f2.isValid() );
  QVERIFY( vl1.dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 ) );

  QgsFeature f3( vl2.fields() );
  f3.setAttribute( QStringLiteral( "fk_province" ), 123 );
  f3.setAttribute( QStringLiteral( "fk_municipality" ), 1 );
  f3.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POINT( 0.5 0.5)" ) ) );
  QVERIFY( f3.isValid() );
  QVERIFY( f3.geometry().isGeosValid() );
  QVERIFY( vl2.dataProvider()->addFeature( f3 ) );

  // build a value relation widget wrapper for municipality
  QgsValueRelationWidgetWrapper w_municipality( &vl2, vl2.fields().indexOf( QLatin1String( "fk_municipality" ) ), nullptr, nullptr );
  QVariantMap cfg_municipality;
  cfg_municipality.insert( QStringLiteral( "Layer" ), vl1.id() );
  cfg_municipality.insert( QStringLiteral( "Key" ),  QStringLiteral( "pk" ) );
  cfg_municipality.insert( QStringLiteral( "Value" ), QStringLiteral( "municipality" ) );
  cfg_municipality.insert( QStringLiteral( "AllowMulti" ), false );
  cfg_municipality.insert( QStringLiteral( "NofColumns" ), 1 );
  cfg_municipality.insert( QStringLiteral( "AllowNull" ), false );
  cfg_municipality.insert( QStringLiteral( "OrderByValue" ), true );
  cfg_municipality.insert( QStringLiteral( "UseCompleter" ), false );
  w_municipality.setConfig( cfg_municipality );
  w_municipality.widget();
  w_municipality.setEnabled( true );

  w_municipality.setFeature( QgsFeature( vl2.fields() ) );
  while ( w_municipality.mComboBox->currentIndex() != 0 )
    QCoreApplication::processEvents();

  // Check first is selected (fid 2 because of OrderByValue)
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 0 );
  QCOMPARE( w_municipality.mComboBox->count(), 2 );
  QCOMPARE( w_municipality.mComboBox->itemText( 0 ), QStringLiteral( "Dreamland By The Clouds" ) );
  QCOMPARE( w_municipality.mComboBox->itemText( 1 ), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.value().toInt(), 2 );

  // Simulate what happens in the attribute form initialization
  w_municipality.setFeature( QgsFeature( vl2.fields() ) );
  w_municipality.setFeature( vl2.getFeature( 1 ) );
  while ( w_municipality.mComboBox->currentIndex() != 1 )
    QCoreApplication::processEvents();

  // Check fid 1 is selected
  QCOMPARE( w_municipality.mComboBox->currentIndex(), 1 );
  QCOMPARE( w_municipality.mComboBox->currentText(), QStringLiteral( "Some Place By The River" ) );
  QCOMPARE( w_municipality.value().toString(), QStringLiteral( "1" ) );

}

QGSTEST_MAIN( TestQgsValueRelationWidgetWrapper )
#include "testqgsvaluerelationwidgetwrapper.moc"
