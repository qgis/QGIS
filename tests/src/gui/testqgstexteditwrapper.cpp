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


#include <nlohmann/json.hpp>

#include "editorwidgets/core/qgseditorwidgetregistry.h"
#include "editorwidgets/qgstexteditwrapper.h"
#include "qgsapplication.h"
#include "qgsattributeform.h"
#include "qgsattributeformeditorwidget.h"
#include "qgseditorwidgetwrapper.h"
#include "qgsgui.h"
#include "qgsjsonutils.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QTableWidget>

class TestQgsTextEditWrapper : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsTextEditWrapper()
      : QgsTest( u"Text Edit Wrapper"_s ) {};

  private:
    QTemporaryDir tempDir;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void defaultValueClause();
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

void TestQgsTextEditWrapper::defaultValueClause()
{
  QgsVectorLayer vl( copyTestData( u"points_gpkg.gpkg"_s ) + u"|layername=points_gpkg"_s );
  QVERIFY( vl.isValid() );

  QgsTextEditWrapper wrapper( &vl, vl.fields().indexOf( "fid"_L1 ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast<QLineEdit *>( wrapper.widget() );
  wrapper.setEnabled( true );
  QCOMPARE( wrapper.defaultValue().toString(), u"Autogenerate"_s );
  QCOMPARE( widget->placeholderText(), u"Autogenerate"_s );

  wrapper.setValues( QgsUnsetAttributeValue( u"Autogenerate"_s ), {} );
  QCOMPARE( widget->text(), u"Autogenerate"_s );
  QVERIFY( QgsVariantUtils::isUnsetAttributeValue( wrapper.value() ) );

  // set explicit text
  widget->setText( u"11"_s );
  QCOMPARE( wrapper.value().userType(), qMetaTypeId< long long >() );
  QCOMPARE( wrapper.value().toInt(), 11 );

  // reset to unset value (this time without the default value clause, should still work)
  wrapper.setValues( QgsUnsetAttributeValue(), {} );
  QCOMPARE( widget->text(), u"Autogenerate"_s );
  QVERIFY( QgsVariantUtils::isUnsetAttributeValue( wrapper.value() ) );

  // set to null
  widget->clear();
  QVERIFY( QgsVariantUtils::isNull( wrapper.value() ) );

  // reset to unset value (this time without the default value clause, should still work)
  wrapper.setValues( QgsUnsetAttributeValue(), {} );
  QCOMPARE( widget->text(), u"Autogenerate"_s );
  QVERIFY( QgsVariantUtils::isUnsetAttributeValue( wrapper.value() ) );

  // null -> valid value
  widget->clear();
  QVERIFY( QgsVariantUtils::isNull( wrapper.value() ) );
  widget->setText( u"11"_s );
  QCOMPARE( wrapper.value().userType(), qMetaTypeId< long long >() );
  QCOMPARE( wrapper.value().toInt(), 11 );
  // back to null
  widget->clear();
  QVERIFY( QgsVariantUtils::isNull( wrapper.value() ) );
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
  QgsVectorLayer *vl_json = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"json\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );
  QVERIFY( vl_json->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QCOMPARE( vl_json->fields().at( 1 ).type(), QVariant::Map );

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( "jvalue"_L1 ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast<QLineEdit *>( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"[1,2,3]"_s );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"{\"a\":1,\"b\":2}"_s );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_array() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"[2]"_s );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_object() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"{\"foo\":\"bar\"}"_s );

  //test complex object
  widget->setText( QString( "{\"foo\":\"bar\",\"baz\":[1,2,3]}" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  json complexJson = QgsJsonUtils::jsonFromVariant( w_json.value() );
  QVERIFY( complexJson.is_object() );
  const json jsonArr = complexJson.at( "baz" );
  QCOMPARE( QString::fromStdString( jsonArr.dump() ), u"[1,2,3]"_s );

  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"\"\""_s );

  //test quoted empty
  widget->setText( QString( "\"\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"\"\""_s );

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
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"null"_s );

  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with quoted string (valid JSON)
  widget->setText( QString( "\"abc\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  // avoid dumping as strings are quoted, so would be double quoted
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"abc"_s );
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
  QgsVectorLayer *vl_json = new QgsVectorLayer( u"%1 sslmode=disable key=\"pk\" table=\"qgis_test\".\"json\" sql="_s.arg( dbConn ), u"json"_s, u"postgres"_s );
  QVERIFY( vl_json->isValid() );

  QgsProject::instance()->addMapLayer( vl_json, false, false );
  QCOMPARE( vl_json->fields().at( 1 ).type(), QVariant::Map );

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( "jbvalue"_L1 ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast<QLineEdit *>( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"[4,5,6]"_s );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"{\"c\":4,\"d\":5}"_s );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_array() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"[2]"_s );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVaJsonDocriant
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_object() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"{\"foo\":\"bar\"}"_s );

  //test complex object
  widget->setText( QString( "{\"foo\":\"bar\",\"baz\":[1,2,3]}" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );
  json complexJson = QgsJsonUtils::jsonFromVariant( w_json.value() );
  QVERIFY( complexJson.is_object() );
  const json jsonArr = complexJson.at( "baz" );
  QCOMPARE( QString::fromStdString( jsonArr.dump() ), u"[1,2,3]"_s );


  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"\"\""_s );


  //test quoted empty
  widget->setText( QString( "\"\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"\"\""_s );

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
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).dump() ), u"null"_s );


  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( !w_json.value().isValid() );

  // test with quoted string (valid JSON)
  widget->setText( QString( "\"abc\"" ) );
  QVERIFY( w_json.value().isValid() );
  QVERIFY( QgsJsonUtils::jsonFromVariant( w_json.value() ).is_string() );
  // avoid dumping as strings are quoted, so would be double quoted
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( w_json.value() ).front() ), u"abc"_s );
#endif
}

QGSTEST_MAIN( TestQgsTextEditWrapper )
#include "testqgstexteditwrapper.moc"
