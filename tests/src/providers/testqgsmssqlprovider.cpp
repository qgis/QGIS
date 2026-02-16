/***************************************************************************
  testqgsmssqlprovider.cpp
 ---------------------
 begin                : 16.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <QRandomGenerator>
#include <QSqlError>
#include <QString>
#include <qtconcurrentrun.h>

using namespace Qt::StringLiterals;

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmssqltransaction.h"
#include "qgsmssqlconnection.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmssqlgeomcolumntypethread.h"
#include "qgsmssqldatabase.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsmssqlutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsMssqlProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void openLayer();

    void projectTransaction();

    void transactionTwoLayers();
    void transactionUndoRedo();
    void testGeomTypeResolutionValid();
    void testGeomTypeResolutionValidNoWorkaround();
    void testGeomTypeResolutionInvalid();
    void testGeomTypeResolutionInvalidNoWorkaround();
    void testFieldsForTable();
    void testFieldsForQuery();
    void testEmptyLayer();
    void testColumnDefinitionForField_data();
    void testColumnDefinitionForField();

  private:
    QString mDbConn;

    QStringList mSomeDataWktGeom;
    QStringList mSomeDataPolyWktGeom;
    QList<QVariantList> mSomeDataAttributes;
};

//runs before all tests
void TestQgsMssqlProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mDbConn = qEnvironmentVariable( "QGIS_MSSQLTEST_DB", "service='testsqlserver' user=sa password='QGIStestSQLServer1234' " );

  mSomeDataWktGeom << u"Point (-70.33199999999999363 66.32999999999999829)"_s
                   << u"Point (-68.20000000000000284 70.79999999999999716)"_s
                   << QString()
                   << u"Point (-65.31999999999999318 78.29999999999999716)"_s
                   << u"Point (-71.12300000000000466 78.23000000000000398)"_s;

  QVariantList varList;
  varList << 1ll << 100 << "Orange" << "oranGe" << "1" << QDateTime( QDate( 2020, 05, 03 ), QTime( 12, 13, 14 ) ) << QDate( 2020, 05, 03 ) << QTime( 12, 13, 14 );
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 2ll << 200 << "Apple" << "Apple" << "2" << QDateTime( QDate( 2020, 05, 04 ), QTime( 12, 14, 14 ) ) << QDate( 2020, 05, 04 ) << QTime( 12, 14, 14 );
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 3ll << 300 << "Pear" << "PEaR" << "3" << QDateTime() << QDate() << QTime();
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 4ll << 400 << "Honey" << "Honey" << "4" << QDateTime( QDate( 2021, 05, 04 ), QTime( 13, 13, 14 ) ) << QDate( 2021, 05, 04 ) << QTime( 13, 13, 14 );
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 5ll << -200 << "" << "NuLl" << "5" << QDateTime( QDate( 2020, 05, 04 ), QTime( 12, 13, 14 ) ) << QDate( 2020, 05, 02 ) << QTime( 12, 13, 1 );
  mSomeDataAttributes << varList;


  mSomeDataPolyWktGeom << u"Polygon ((-69 81.40000000000000568, -69 80.20000000000000284, -73.70000000000000284 80.20000000000000284, -73.70000000000000284 76.29999999999999716, -74.90000000000000568 76.29999999999999716, -74.90000000000000568 81.40000000000000568, -69 81.40000000000000568))"_s
                       << u"Polygon ((-67.59999999999999432 81.20000000000000284, -66.29999999999999716 81.20000000000000284, -66.29999999999999716 76.90000000000000568, -67.59999999999999432 76.90000000000000568, -67.59999999999999432 81.20000000000000284))"_s
                       << u"Polygon ((-68.40000000000000568 75.79999999999999716, -67.5 72.59999999999999432, -68.59999999999999432 73.70000000000000284, -70.20000000000000284 72.90000000000000568, -68.40000000000000568 75.79999999999999716))"_s
                       << QString();
}


//runs after all tests
void TestQgsMssqlProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMssqlProvider::openLayer()
{
  QString uri( mDbConn + u" key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql="_s );

  QgsVectorLayer vl( uri, u"point_layer"_s, u"mssql"_s );

  QVERIFY( vl.isValid() );
  QCOMPARE( vl.featureCount(), 5 );

  QVERIFY( vl.isValid() );
}


void TestQgsMssqlProvider::projectTransaction()
{
  QString uriPoint( mDbConn + u" key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql="_s );
  QString uriPolygon( mDbConn + u" key = \"pk\" srid=4326 type=POLYGON schema=\"qgis_test\" table=\"some_poly_data\" (geom) sql="_s );

  QgsProject project;

  QgsVectorLayer *vectorLayerPoint = new QgsVectorLayer( uriPoint, u"point_layer"_s, u"mssql"_s );
  QgsVectorLayer *vectorLayerPoly = new QgsVectorLayer( uriPolygon, u"poly_layer"_s, u"mssql"_s );
  project.addMapLayer( vectorLayerPoint );
  project.addMapLayer( vectorLayerPoly );

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint->startEditing() );
  QVERIFY( vectorLayerPoint->isEditable() );
  QVERIFY( vectorLayerPoly->isEditable() );

  // test geometries and attributes of exisintg layer
  QgsFeatureIterator featIt = vectorLayerPoint->getFeatures();
  QgsFeature feat;
  QList<QVariantList> attributes;
  QStringList geoms;
  while ( featIt.nextFeature( feat ) )
  {
    attributes.append( feat.attributes().toList() );
    geoms.append( feat.geometry().asWkt() );
  }
  QCOMPARE( attributes.size(), mSomeDataAttributes.size() );
  QVERIFY( attributes == mSomeDataAttributes );
  QVERIFY( geoms == mSomeDataWktGeom );

  featIt = vectorLayerPoly->getFeatures();
  geoms.clear();
  while ( featIt.nextFeature( feat ) )
    geoms.append( feat.geometry().asWkt() );

  QVERIFY( geoms == mSomeDataPolyWktGeom );

  // add a point to the layer point
  feat = QgsFeature();
  feat.setFields( vectorLayerPoint->fields(), true );
  feat.setAttribute( 0, 10 );
  feat.setGeometry( QgsGeometry( new QgsPoint( -71, 67 ) ) );

  QStringList newGeoms = mSomeDataWktGeom;
  newGeoms << u"Point (-71 67)"_s;
  QList<QVariantList> newAttributes = mSomeDataAttributes;
  QVariantList attrList;
  attrList << 10ll << 0 << "" << "" << "" << QDateTime() << QDate() << QTime();
  newAttributes.append( attrList );

  vectorLayerPoint->addFeature( feat );

  QCOMPARE( vectorLayerPoint->featureCount(), 6 );

  featIt = vectorLayerPoint->getFeatures();
  attributes.clear();
  geoms.clear();
  while ( featIt.nextFeature( feat ) )
  {
    attributes.append( feat.attributes().toList() );
    geoms.append( feat.geometry().asWkt() );
  }
  QCOMPARE( attributes.size(), newAttributes.size() );
  QVERIFY( attributes == newAttributes );
  QVERIFY( geoms == newGeoms );

  vectorLayerPoint->rollBack();

  QCOMPARE( vectorLayerPoint->featureCount(), 5 );
}

void TestQgsMssqlProvider::transactionTwoLayers()
{
  QString uriPoint( mDbConn + u" key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql="_s );
  QgsVectorLayer *vectorLayerPoint1 = new QgsVectorLayer( uriPoint, u"point_layer_1"_s, u"mssql"_s );
  QgsVectorLayer *vectorLayerPoint2 = new QgsVectorLayer( uriPoint, u"point_layer_2"_s, u"mssql"_s );

  QgsProject project;
  project.addMapLayer( vectorLayerPoint1 );
  project.addMapLayer( vectorLayerPoint2 );

  // point to be added to the point layer
  QgsFeature feat;
  feat.setFields( vectorLayerPoint1->fields(), true );
  feat.setAttribute( 0, 123 );
  feat.setGeometry( QgsGeometry( new QgsPoint( -123, 123 ) ) );

  // 1. without transaction, try to add a feature to the first layer -> the other layer is not affected

  QVERIFY( vectorLayerPoint1->startEditing() );

  vectorLayerPoint1->addFeature( feat );

  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 5 );

  vectorLayerPoint1->rollBack();

  // 2. with transaction, try to add a feature to the first layer -> both layers are affected

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint1->startEditing() );

  vectorLayerPoint1->addFeature( feat );

  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 6 );

  vectorLayerPoint1->rollBack();

  QCOMPARE( vectorLayerPoint1->featureCount(), 5 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 5 );
}

void TestQgsMssqlProvider::transactionUndoRedo()
{
  QString uriPoint( mDbConn + u" key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql="_s );
  QgsVectorLayer *vectorLayerPoint1 = new QgsVectorLayer( uriPoint, u"point_layer_1"_s, u"mssql"_s );
  //QgsVectorLayer *vectorLayerPoint2 = new QgsVectorLayer( uriPoint, u"point_layer_2"_s, u"mssql"_s );

  QgsProject project;
  project.addMapLayer( vectorLayerPoint1 );
  //project.addMapLayer( vectorLayerPoint2 );

  // point to be added to the point layer
  QgsFeature feat1;
  feat1.setFields( vectorLayerPoint1->fields(), true );
  feat1.setAttribute( 0, 111 );
  feat1.setGeometry( QgsGeometry( new QgsPoint( -111, 111 ) ) );

  QgsFeature feat2;
  feat2.setFields( vectorLayerPoint1->fields(), true );
  feat2.setAttribute( 0, 222 );
  feat2.setGeometry( QgsGeometry( new QgsPoint( -222, 222 ) ) );

  // 1. without transaction

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint1->startEditing() );

  QCOMPARE( vectorLayerPoint1->undoStack()->count(), 0 );

  vectorLayerPoint1->addFeature( feat1 );

  vectorLayerPoint1->addFeature( feat2 );

  QCOMPARE( vectorLayerPoint1->undoStack()->count(), 2 );

  QCOMPARE( vectorLayerPoint1->featureCount(), 7 );
  vectorLayerPoint1->undoStack()->undo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  vectorLayerPoint1->undoStack()->undo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 5 );
  vectorLayerPoint1->undoStack()->redo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  vectorLayerPoint1->undoStack()->redo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 7 );

  vectorLayerPoint1->rollBack();

  // 2. with transaction, try to add a feature to the first layer -> both layers are affected
}

void TestQgsMssqlProvider::testGeomTypeResolutionValid()
{
  QgsDataSourceUri uri( mDbConn );

  // with invalid geometry handling workaround:
  QgsMssqlGeomColumnTypeThread thread( uri.service(), uri.host(), uri.database(), uri.username(), uri.password(), false, false );

  QgsMssqlLayerProperty layerProperty;
  layerProperty.schemaName = u"qgis_test"_s;
  layerProperty.tableName = u"someData"_s;
  layerProperty.geometryColName = u"geom"_s;
  thread.addGeometryColumn( layerProperty );

  thread.run();
  const QList<QgsMssqlLayerProperty> results = thread.results();
  QCOMPARE( results.size(), 1 );
  const QgsMssqlLayerProperty result = results.at( 0 );
  QCOMPARE( result.type, u"POINT"_s );
  QCOMPARE( result.schemaName, u"qgis_test"_s );
  QCOMPARE( result.tableName, u"someData"_s );
  QCOMPARE( result.geometryColName, u"geom"_s );
  QCOMPARE( result.pkCols.size(), 0 );
  QCOMPARE( result.srid, u"4326"_s );
  QCOMPARE( result.isGeography, false );
  QCOMPARE( result.sql, QString() );
  QCOMPARE( result.isView, false );
}

void TestQgsMssqlProvider::testGeomTypeResolutionValidNoWorkaround()
{
  QgsDataSourceUri uri( mDbConn );

  // WITHOUT invalid geometry handling workaround:
  QgsMssqlGeomColumnTypeThread thread( uri.service(), uri.host(), uri.database(), uri.username(), uri.password(), false, true );

  QgsMssqlLayerProperty layerProperty;
  layerProperty.schemaName = u"qgis_test"_s;
  layerProperty.tableName = u"someData"_s;
  layerProperty.geometryColName = u"geom"_s;
  thread.addGeometryColumn( layerProperty );

  thread.run();
  const QList<QgsMssqlLayerProperty> results = thread.results();
  QCOMPARE( results.size(), 1 );
  const QgsMssqlLayerProperty result = results.at( 0 );
  QCOMPARE( result.type, u"POINT"_s );
  QCOMPARE( result.schemaName, u"qgis_test"_s );
  QCOMPARE( result.tableName, u"someData"_s );
  QCOMPARE( result.geometryColName, u"geom"_s );
  QCOMPARE( result.pkCols.size(), 0 );
  QCOMPARE( result.srid, u"4326"_s );
  QCOMPARE( result.isGeography, false );
  QCOMPARE( result.sql, QString() );
  QCOMPARE( result.isView, false );
}


void TestQgsMssqlProvider::testGeomTypeResolutionInvalid()
{
  QgsDataSourceUri uri( mDbConn );

  // with invalid geometry handling workaround:
  QgsMssqlGeomColumnTypeThread thread( uri.service(), uri.host(), uri.database(), uri.username(), uri.password(), false, false );

  QgsMssqlLayerProperty layerProperty;
  layerProperty.schemaName = u"qgis_test"_s;
  layerProperty.tableName = u"invalid_polys"_s;
  layerProperty.geometryColName = u"ogr_geometry"_s;
  thread.addGeometryColumn( layerProperty );

  thread.run();
  const QList<QgsMssqlLayerProperty> results = thread.results();
  QCOMPARE( results.size(), 1 );
  const QgsMssqlLayerProperty result = results.at( 0 );
  QCOMPARE( result.type, u"MULTIPOLYGON,POLYGON"_s );
  QCOMPARE( result.schemaName, u"qgis_test"_s );
  QCOMPARE( result.tableName, u"invalid_polys"_s );
  QCOMPARE( result.geometryColName, u"ogr_geometry"_s );
  QCOMPARE( result.pkCols.size(), 0 );
  QCOMPARE( result.srid, u"4167,4167"_s );
  QCOMPARE( result.isGeography, false );
  QCOMPARE( result.sql, QString() );
  QCOMPARE( result.isView, false );
}

void TestQgsMssqlProvider::testGeomTypeResolutionInvalidNoWorkaround()
{
  QgsDataSourceUri uri( mDbConn );

  // WITHOUT invalid geometry handling workaround.
  // We expect this to fail
  QgsMssqlGeomColumnTypeThread thread( uri.service(), uri.host(), uri.database(), uri.username(), uri.password(), false, true );

  QgsMssqlLayerProperty layerProperty;
  layerProperty.schemaName = u"qgis_test"_s;
  layerProperty.tableName = u"invalid_polys"_s;
  layerProperty.geometryColName = u"ogr_geometry"_s;
  thread.addGeometryColumn( layerProperty );

  thread.run();
  const QList<QgsMssqlLayerProperty> results = thread.results();
  QCOMPARE( results.size(), 1 );
  const QgsMssqlLayerProperty result = results.at( 0 );
  // geometry type resolution will fail because of unhandled exception raised by SQL Server
  QCOMPARE( result.type, QString() );
  QCOMPARE( result.schemaName, u"qgis_test"_s );
  QCOMPARE( result.tableName, u"invalid_polys"_s );
  QCOMPARE( result.geometryColName, u"ogr_geometry"_s );
  QCOMPARE( result.pkCols.size(), 0 );
  QCOMPARE( result.srid, QString() );
  QCOMPARE( result.isGeography, false );
  QCOMPARE( result.sql, QString() );
  QCOMPARE( result.isView, false );
}

void TestQgsMssqlProvider::testFieldsForTable()
{
  QgsDataSourceUri uri( mDbConn );

  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( uri );

  QgsMssqlDatabase::FieldDetails details;
  QString error;
  QVERIFY( db->loadFields( details, u"qgis_test"_s, u"someData"_s, error ) );
  QCOMPARE( error, QString() );
  QCOMPARE( details.attributeFields.size(), 8 );
  QCOMPARE( details.attributeFields.at( 0 ).name(), u"pk"_s );
  QCOMPARE( details.attributeFields.at( 0 ).type(), QVariant::Int );
  QCOMPARE( details.attributeFields.at( 0 ).typeName(), u"int"_s );
  QCOMPARE( details.attributeFields.at( 1 ).name(), u"cnt"_s );
  QCOMPARE( details.attributeFields.at( 1 ).type(), QVariant::Int );
  QCOMPARE( details.attributeFields.at( 1 ).typeName(), u"int"_s );
  QCOMPARE( details.attributeFields.at( 2 ).name(), u"name"_s );
  QCOMPARE( details.attributeFields.at( 2 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 2 ).typeName(), u"ntext"_s );
  QCOMPARE( details.attributeFields.at( 3 ).name(), u"name2"_s );
  QCOMPARE( details.attributeFields.at( 3 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 3 ).typeName(), u"ntext"_s );
  QCOMPARE( details.attributeFields.at( 4 ).name(), u"num_char"_s );
  QCOMPARE( details.attributeFields.at( 4 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 4 ).typeName(), u"ntext"_s );
  QCOMPARE( details.attributeFields.at( 5 ).name(), u"dt"_s );
  QCOMPARE( details.attributeFields.at( 5 ).type(), QVariant::DateTime );
  QCOMPARE( details.attributeFields.at( 5 ).typeName(), u"datetime"_s );
  QCOMPARE( details.attributeFields.at( 6 ).name(), u"date"_s );
  QCOMPARE( details.attributeFields.at( 6 ).type(), QVariant::Date );
  QCOMPARE( details.attributeFields.at( 6 ).typeName(), u"date"_s );
  QCOMPARE( details.attributeFields.at( 7 ).name(), u"time"_s );
  QCOMPARE( details.attributeFields.at( 7 ).type(), QVariant::Time );
  QCOMPARE( details.attributeFields.at( 7 ).typeName(), u"time"_s );
}

void TestQgsMssqlProvider::testFieldsForQuery()
{
  QgsDataSourceUri uri( mDbConn );

  std::shared_ptr<QgsMssqlDatabase> db = QgsMssqlDatabase::connectDb( uri );

  QgsMssqlDatabase::FieldDetails details;
  QString error;
  QVERIFY( db->loadQueryFields( details, u"SELECT ROW_NUMBER() OVER(ORDER BY (SELECT NULL)) AS _uid1_, concat('a', cnt ) as b, cast(cnt as numeric)/100 as c, * FROM [qgis_test].[someData]"_s, error ) );
  QCOMPARE( error, QString() );
  QCOMPARE( details.attributeFields.size(), 11 );

  QCOMPARE( details.attributeFields.at( 0 ).name(), u"_uid1_"_s );
  QCOMPARE( details.attributeFields.at( 0 ).type(), QVariant::LongLong );
  QCOMPARE( details.attributeFields.at( 0 ).typeName(), u"bigint"_s );
  QCOMPARE( details.attributeFields.at( 1 ).name(), u"b"_s );
  QCOMPARE( details.attributeFields.at( 1 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 1 ).typeName(), u"varchar(13)"_s );
  QCOMPARE( details.attributeFields.at( 2 ).name(), u"c"_s );
  QCOMPARE( details.attributeFields.at( 2 ).type(), QVariant::Double );
  QCOMPARE( details.attributeFields.at( 2 ).typeName(), u"numeric(24,6)"_s );
  QCOMPARE( details.attributeFields.at( 2 ).length(), 24 );
  QCOMPARE( details.attributeFields.at( 3 ).name(), u"pk"_s );
  QCOMPARE( details.attributeFields.at( 3 ).type(), QVariant::Int );
  QCOMPARE( details.attributeFields.at( 3 ).typeName(), u"int"_s );
  QCOMPARE( details.attributeFields.at( 4 ).name(), u"cnt"_s );
  QCOMPARE( details.attributeFields.at( 4 ).type(), QVariant::Int );
  QCOMPARE( details.attributeFields.at( 4 ).typeName(), u"int"_s );
  QCOMPARE( details.attributeFields.at( 5 ).name(), u"name"_s );
  QCOMPARE( details.attributeFields.at( 5 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 5 ).typeName(), u"nvarchar(max)"_s );
  QCOMPARE( details.attributeFields.at( 6 ).name(), u"name2"_s );
  QCOMPARE( details.attributeFields.at( 6 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 6 ).typeName(), u"nvarchar(max)"_s );
  QCOMPARE( details.attributeFields.at( 7 ).name(), u"num_char"_s );
  QCOMPARE( details.attributeFields.at( 7 ).type(), QVariant::String );
  QCOMPARE( details.attributeFields.at( 7 ).typeName(), u"nvarchar(max)"_s );
  QCOMPARE( details.attributeFields.at( 8 ).name(), u"dt"_s );
  QCOMPARE( details.attributeFields.at( 8 ).type(), QVariant::DateTime );
  QCOMPARE( details.attributeFields.at( 8 ).typeName(), u"datetime"_s );
  QCOMPARE( details.attributeFields.at( 9 ).name(), u"date"_s );
  QCOMPARE( details.attributeFields.at( 9 ).type(), QVariant::Date );
  QCOMPARE( details.attributeFields.at( 9 ).typeName(), u"date"_s );
  QCOMPARE( details.attributeFields.at( 10 ).name(), u"time"_s );
  QCOMPARE( details.attributeFields.at( 10 ).type(), QVariant::Time );
  QCOMPARE( details.attributeFields.at( 10 ).typeName(), u"time(7)"_s );

  QCOMPARE( details.geometryColumnName, u"geom"_s );
  QCOMPARE( details.geometryColumnType, u"geometry"_s );
  QVERIFY( !details.isGeography );
}

void TestQgsMssqlProvider::testEmptyLayer()
{
  QgsDataSourceUri uri( mDbConn );
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( "mssql" );
  QVERIFY( metadata );

  metadata->createConnection( mDbConn, {} );
  std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn;
  conn.reset( static_cast<QgsAbstractDatabaseProviderConnection *>( metadata->createConnection( uri.uri(), QVariantMap() ) ) );
  conn->executeSql( u"DROP TABLE IF EXISTS qgis_test.empty_layer"_s );

  uri.setTable( u"empty_layer"_s );
  uri.setSchema( u"qgis_test"_s );
  QgsFields fields;
  QMap<int, int> oldToNewAttrIdxMap;
  QString errorMessage;
  QString createdUri;

  // with explicit primary key
  fields.append( QgsField( "some_string", QMetaType::Type::QString ) );
  fields.append( QgsField( "my_pk", QMetaType::Type::LongLong ) );
  fields.append( QgsField( "some_real", QMetaType::Type::Double ) );

  uri.setKeyColumn( u"my_pk"_s );

  Qgis::VectorExportResult res = metadata->createEmptyLayer( uri.uri(), fields, Qgis::WkbType::Point, QgsCoordinateReferenceSystem( "EPSG:3111" ), true, oldToNewAttrIdxMap, errorMessage, {}, createdUri );
  if ( !errorMessage.isEmpty() )
  {
    QgsDebugError( errorMessage );
  }
  QCOMPARE( res, Qgis::VectorExportResult::Success );

  auto vl = std::make_unique< QgsVectorLayer >( createdUri, "test", "mssql" );
  QVERIFY( vl->isValid() );
  QCOMPARE( vl->crs().authid(), u"EPSG:3111"_s );
  QCOMPARE( vl->wkbType(), Qgis::WkbType::Point );
  QCOMPARE( vl->fields().size(), 3 );
  // primary key will always be first
  QCOMPARE( vl->fields().at( 0 ).name(), u"my_pk"_s );
  // currently primary key is always an int (it's created as serial type)
  QCOMPARE( static_cast< int >( vl->fields().at( 0 ).type() ), static_cast< int >( QMetaType::Type::Int ) );
  QCOMPARE( vl->fields().at( 1 ).name(), u"some_string"_s );
  QCOMPARE( static_cast< int >( vl->fields().at( 1 ).type() ), static_cast< int >( QMetaType::Type::QString ) );
  QCOMPARE( vl->fields().at( 2 ).name(), u"some_real"_s );
  QCOMPARE( static_cast< int >( vl->fields().at( 2 ).type() ), static_cast< int >( QMetaType::Type::Double ) );

  QCOMPARE( oldToNewAttrIdxMap.size(), 3 );
  QCOMPARE( oldToNewAttrIdxMap.value( 0 ), 1 );
  QCOMPARE( oldToNewAttrIdxMap.value( 1 ), 0 );
  QCOMPARE( oldToNewAttrIdxMap.value( 2 ), 2 );

  // creating a brand new primary key
  uri.setKeyColumn( u"my_new_pk"_s );

  res = metadata->createEmptyLayer( uri.uri(), fields, Qgis::WkbType::Point, QgsCoordinateReferenceSystem( "EPSG:3111" ), true, oldToNewAttrIdxMap, errorMessage, {}, createdUri );
  if ( !errorMessage.isEmpty() )
  {
    QgsDebugError( errorMessage );
  }
  QCOMPARE( res, Qgis::VectorExportResult::Success );

  vl = std::make_unique< QgsVectorLayer >( createdUri, "test", "mssql" );
  QVERIFY( vl->isValid() );
  QCOMPARE( vl->crs().authid(), u"EPSG:3111"_s );
  QCOMPARE( vl->wkbType(), Qgis::WkbType::Point );
  QCOMPARE( vl->fields().size(), 4 );
  // primary key will always be first
  QCOMPARE( vl->fields().at( 0 ).name(), u"my_new_pk"_s );
  // currently primary key is always an int (it's created as serial type)
  QCOMPARE( static_cast< int >( vl->fields().at( 0 ).type() ), static_cast< int >( QMetaType::Type::Int ) );
  QCOMPARE( vl->fields().at( 1 ).name(), u"some_string"_s );
  QCOMPARE( static_cast< int >( vl->fields().at( 1 ).type() ), static_cast< int >( QMetaType::Type::QString ) );
  QCOMPARE( vl->fields().at( 2 ).name(), u"my_pk"_s );
  QCOMPARE( static_cast< int >( vl->fields().at( 2 ).type() ), static_cast< int >( QMetaType::Type::LongLong ) );
  QCOMPARE( vl->fields().at( 3 ).name(), u"some_real"_s );
  QCOMPARE( static_cast< int >( vl->fields().at( 3 ).type() ), static_cast< int >( QMetaType::Type::Double ) );

  QCOMPARE( oldToNewAttrIdxMap.size(), 3 );
  QCOMPARE( oldToNewAttrIdxMap.value( 0 ), 1 );
  QCOMPARE( oldToNewAttrIdxMap.value( 1 ), 2 );
  QCOMPARE( oldToNewAttrIdxMap.value( 2 ), 3 );
}

void TestQgsMssqlProvider::testColumnDefinitionForField_data()
{
  QTest::addColumn<QgsField>( "field" );
  QTest::addColumn<bool>( "ignoreTypeString" );
  QTest::addColumn<QString>( "definition" );

  // using automatic type mapping (i.e. no explicit SQL server column type)
  QTest::newRow( "auto_bigint" )
    << QgsField( "id", QMetaType::Type::LongLong )
    << false
    << "[id] bigint";
  QTest::newRow( "auto_int" )
    << QgsField( "age", QMetaType::Type::Int )
    << false
    << "[age] int";
  QTest::newRow( "auto_numeric" )
    << QgsField( "score", QMetaType::Type::Double )
    << false
    << "[score] float";
  QTest::newRow( "auto_numeric_with_precision" )
    << QgsField( "score", QMetaType::Type::Double, QString(), 20, 8 )
    << false
    << "[score] numeric(20,8)";
  QTest::newRow( "auto_date" )
    << QgsField( "dob", QMetaType::Type::QDate )
    << false
    << "[dob] date";
  QTest::newRow( "auto_time" )
    << QgsField( "alarm", QMetaType::Type::QTime )
    << false
    << "[alarm] time";
  QTest::newRow( "auto_datetime" )
    << QgsField( "created_at", QMetaType::Type::QDateTime )
    << false
    << "[created_at] datetime";
  QTest::newRow( "auto_string_with_length" )
    << QgsField( "name", QMetaType::Type::QString, QString(), 50 )
    << false
    << "[name] nvarchar(50)";
  QTest::newRow( "auto_string_unlimited" )
    << QgsField( "notes", QMetaType::Type::QString, QString(), 0 )
    << false
    << "[notes] nvarchar(max)";

  // with explicit SQL Server column type names
  QTest::newRow( "explicit_int" )
    << QgsField( "count", QMetaType::Type::Int, "int" )
    << false
    << "[count] int";
  QTest::newRow( "fmt_varchar_len" )
    << QgsField( "code", QMetaType::Type::QString, "varchar", 10 )
    << false
    << "[code] varchar(10)";
  QTest::newRow( "fmt_nvarchar_len" )
    << QgsField( "label", QMetaType::Type::QString, "nvarchar", 255 )
    << false
    << "[label] nvarchar(255)";
  QTest::newRow( "fmt_char_len" )
    << QgsField( "flag", QMetaType::Type::QString, "char", 1 )
    << false
    << "[flag] char(1)";
  QTest::newRow( "fmt_varchar_no_len" )
    << QgsField( "raw", QMetaType::Type::QString, "varchar", 0 )
    << false
    << "[raw] varchar";

  QTest::newRow( "fmt_numeric_full" )
    << QgsField( "cost", QMetaType::Type::Double, "numeric", 10, 2 )
    << false
    << "[cost] numeric(10,2)";
  QTest::newRow( "fmt_decimal_full" )
    << QgsField( "ratio", QMetaType::Type::Double, "decimal", 18, 6 )
    << false
    << "[ratio] decimal(18,6)";
  QTest::newRow( "fmt_numeric_no_prec" )
    << QgsField( "val", QMetaType::Type::Double, "numeric", 10, 0 )
    << false
    << "[val] numeric";
  QTest::newRow( "fmt_decimal_no_len" )
    << QgsField( "val2", QMetaType::Type::Double, "decimal", 0, 5 )
    << false
    << "[val2] decimal";

  QTest::newRow( "decimal_with_scale" )
    << QgsField( "rate", QMetaType::Type::Double, "decimal", 18, 6 )
    << false
    << "[rate] decimal(18,6)";


  // with explicit SQL Server column type names, but ignoreTypeString set
  QTest::newRow( "explicit_int_ignore_type_string" )
    << QgsField( "count", QMetaType::Type::Int, "int" )
    << true
    << "[count] int";
  QTest::newRow( "fmt_varchar_len_ignore_type_string" )
    << QgsField( "code", QMetaType::Type::QString, "varchar", 10 )
    << true
    << "[code] nvarchar(10)";
  QTest::newRow( "fmt_nvarchar_len_ignore_type_string" )
    << QgsField( "label", QMetaType::Type::QString, "nvarchar", 255 )
    << true
    << "[label] nvarchar(255)";
  QTest::newRow( "fmt_char_len_ignore_type_string" )
    << QgsField( "flag", QMetaType::Type::QString, "char", 1 )
    << true
    << "[flag] nvarchar(1)";
  QTest::newRow( "fmt_varchar_no_len_ignore_type_string" )
    << QgsField( "raw", QMetaType::Type::QString, "varchar", 0 )
    << true
    << "[raw] nvarchar(max)";
  QTest::newRow( "fmt_numeric_full_ignore_type_string" )
    << QgsField( "cost", QMetaType::Type::Double, "numeric", 10, 2 )
    << true
    << "[cost] numeric(10,2)";
  QTest::newRow( "fmt_decimal_full_ignore_type_string" )
    << QgsField( "ratio", QMetaType::Type::Double, "decimal", 18, 6 )
    << true
    << "[ratio] numeric(18,6)";
  QTest::newRow( "fmt_numeric_no_prec_ignore_type_string" )
    << QgsField( "val", QMetaType::Type::Double, "numeric", 10, 0 )
    << true
    << "[val] float";
  QTest::newRow( "fmt_decimal_no_len_ignore_type_string" )
    << QgsField( "val2", QMetaType::Type::Double, "decimal", 0, 5 )
    << true
    << "[val2] float";
  QTest::newRow( "decimal_with_scale_ignore_type_string" )
    << QgsField( "rate", QMetaType::Type::Double, "decimal", 18, 6 )
    << true
    << "[rate] numeric(18,6)";

  // unhandled variant type
  QTest::newRow( "unhandled_variant" )
    << QgsField( "bad_col", QMetaType::Type::QVariantMap )
    << false
    << QString();

  // field name escaping
  QTest::newRow( "name_escaping" )
    << QgsField( "complex[field name]", QMetaType::Type::QString )
    << false
    << "[complex[field name]]] nvarchar(max)";
}

void TestQgsMssqlProvider::testColumnDefinitionForField()
{
  QFETCH( QgsField, field );
  QFETCH( bool, ignoreTypeString );
  QFETCH( QString, definition );

  QCOMPARE( QgsMssqlUtils::columnDefinitionForField( field, ignoreTypeString ), definition );
}

QGSTEST_MAIN( TestQgsMssqlProvider )
#include "testqgsmssqlprovider.moc"
