/***************************************************************************
    testqgstexteditwidgetwrapper.cpp
     --------------------------------------
    Date                 : 30 09 2019
    Copyright            : (C) 2019 Stephen Knox
    Email                : stephenknox73 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <editorwidgets/core/qgseditorwidgetregistry.h>
#include "qgsattributeform.h"
#include <qgsapplication.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include "qgseditorwidgetwrapper.h"
#include "qgsattributeformeditorwidget.h"
#include <editorwidgets/qgstexteditwrapper.h>
#include <QTableWidget>
#include "qgsgui.h"
#include <nlohmann/json.hpp>
#include "qgsjsonutils.h"

class TestQgsTextEditWrapper : public QObject
{
    Q_OBJECT
  public:
    TestQgsTextEditWrapper() = default;

  private:
    QTemporaryDir tempDir;

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testWithJsonInPostgres();
    void testWithJsonBInPostgres();
};

void TestQgsTextEditWrapper::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsGui::editorWidgetRegistry()->initEditors();
  testWithJsonInPostgres();
}

void TestQgsTextEditWrapper::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTextEditWrapper::init()
{
}

void TestQgsTextEditWrapper::cleanup()
{
}

void TestQgsTextEditWrapper::testWithJsonInPostgres()
{
#ifdef ENABLE_PGTEST
  // create pg layers
  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsVectorLayer *vl_json = new QgsVectorLayer( QStringLiteral( "%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"json\" sql=" ).arg( dbConn ), QStringLiteral( "json" ), QStringLiteral( "postgres" ) );
  QVERIFY( vl_json->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QCOMPARE( vl_json->fields().at( 1 ).type(), QVariant::Map );

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( QLatin1String( "jvalue" ) ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast< QLineEdit * >( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "[1,2,3]" ) );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "{\"a\":1,\"b\":2}" ) );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_array() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "[2]" ) );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_object() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "{\"foo\":\"bar\"}" ) );

  //test complex object
  widget->setText( QString( "{\"foo\":\"bar\",\"baz\":[1,2,3]}" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  json complexJson =  QgsJsonUtils::jsonFromVariant( w_json.value() );
  QVERIFY( complexJson.is_object() );
  const json jsonArr = complexJson.at( "baz" );
  QCOMPARE( QString::fromStdString( jsonArr.dump() ), QStringLiteral( "[1,2,3]" ) );

  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "\"\"" ) );

  //test quoted empty
  widget->setText( QString( "\"\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "\"\"" ) );

  // test invalid JSON
  widget->setText( QString( "{\"body\";\"text\"}" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with primitive integer (without container) which is valid JSON
  widget->setText( QString( "2" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( w_json.value(), QVariant( 2 ) );
  QVERIFY( w_json.value().userType() == QMetaType::Int );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_number_integer() );
  const int n = QgsJsonUtils::jsonFromVariant( w_json.value() ).front();
  QCOMPARE( QVariant( n ), QVariant( 2 ) );

  // test with primitive boolean
  widget->setText( QString( "true" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( w_json.value(), QVariant( true ) );
  QVERIFY( w_json.value().userType() == QMetaType::Bool );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_boolean() );
  const bool x = QgsJsonUtils::jsonFromVariant( w_json.value() ).front();
  QCOMPARE( QVariant( x ), QVariant( true ) );

  // test with primitive null
  widget->setText( QString( "null" ) );
  QVERIFY( w_json.value().isNull() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_null() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "null" ) );

  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with quoted string (valid JSON)
  widget->setText( QString( "\"abc\"" ) );
  QVERIFY( w_json.value().isValid() ) ;
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  // avoid dumping as strings are quoted, so would be double quoted
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "abc" ) );
#endif
}

void TestQgsTextEditWrapper::testWithJsonBInPostgres()
{
#ifdef ENABLE_PGTEST
  //create pg layers
  QString dbConn = getenv( "QGIS_PGTEST_DB" );
  if ( dbConn.isEmpty() )
  {
    dbConn = "service=\"qgis_test\"";
  }
  QgsVectorLayer *vl_json = new QgsVectorLayer( QStringLiteral( "%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"json\" sql=" ).arg( dbConn ), QStringLiteral( "json" ), QStringLiteral( "postgres" ) );
  QVERIFY( vl_json->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QCOMPARE( vl_json->fields().at( 1 ).type(), QVariant::Map );

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( QLatin1String( "jbvalue" ) ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast< QLineEdit * >( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "[4,5,6]" ) );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "{\"c\":4,\"d\":5}" ) );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_array() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "[2]" ) );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVaJsonDocriant
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_object() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "{\"foo\":\"bar\"}" ) );

  //test complex object
  widget->setText( QString( "{\"foo\":\"bar\",\"baz\":[1,2,3]}" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  json complexJson =  QgsJsonUtils::jsonFromVariant( w_json.value() );
  QVERIFY( complexJson.is_object() );
  const json jsonArr = complexJson.at( "baz" );
  QCOMPARE( QString::fromStdString( jsonArr.dump() ), QStringLiteral( "[1,2,3]" ) );


  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "\"\"" ) );


  //test quoted empty
  widget->setText( QString( "\"\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "\"\"" ) );

  // test invalid JSON
  widget->setText( QString( "{\"body\";\"text\"}" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with primitive integer (without container) which is valid JSON
  widget->setText( QString( "2" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( w_json.value(), QVariant( 2 ) );
  QVERIFY( w_json.value().userType() == QMetaType::Int );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_number_integer() );
  const int n = QgsJsonUtils::jsonFromVariant( w_json.value() ).front();
  QCOMPARE( QVariant( n ), QVariant( 2 ) );

  // test with primitive boolean
  widget->setText( QString( "true" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( w_json.value(), QVariant( true ) );
  QVERIFY( w_json.value().userType() == QMetaType::Bool );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_boolean() );
  const bool x = QgsJsonUtils::jsonFromVariant( w_json.value() ).front();
  QCOMPARE( QVariant( x ), QVariant( true ) );

  // test with primitive null
  widget->setText( QString( "null" ) );
  QVERIFY( w_json.value().isNull() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_null() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), QStringLiteral( "null" ) );


  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with quoted string (valid JSON)
  widget->setText( QString( "\"abc\"" ) );
  QVERIFY( w_json.value().isValid() ) ;
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  // avoid dumping as strings are quoted, so would be double quoted
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front( ) ), QStringLiteral( "abc" ) );
#endif
}

QGSTEST_MAIN( TestQgsTextEditWrapper )
#include "testqgstexteditwrapper.moc"
