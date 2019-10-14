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

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( QStringLiteral( "jvalue" ) ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast< QLineEdit * >( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "[1,2,3]" ) );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "{\"a\":1,\"b\":2}" ) );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "[2]" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );

  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );

  // test invalid JSON
  widget->setText( QString( "{\"body\";\"text\"}" ) );
  QVERIFY( w_json.value().isValid() );
  //invalid JSON will be parsed as text - it is up to the Postgres provider to reject it
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value().toString(), QStringLiteral( "{\"body\";\"text\"}" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );

  // test with bare integer (without container) which is valid JSON
  widget->setText( QString( "2" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value(), QVariant( 2 ) );
  QVERIFY( w_json.value().userType() == QMetaType::Int );

  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( w_json.value().isValid() ) ;
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value().toString(), QStringLiteral( "abc" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );
}

void TestQgsTextEditWrapper::testWithJsonBInPostgres()
{
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

  QgsTextEditWrapper w_json( vl_json, vl_json->fields().indexOf( QStringLiteral( "jbvalue" ) ), nullptr, nullptr );
  QLineEdit *widget = qobject_cast< QLineEdit * >( w_json.widget() );
  w_json.setEnabled( true );

  // check text output from DB
  w_json.setFeature( vl_json->getFeature( 1 ) );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "[4,5,6]" ) );
  w_json.setFeature( vl_json->getFeature( 2 ) );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "{\"c\":4,\"d\":5}" ) );

  // check input into widget
  // test array
  widget->setText( QString( "[2]" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QListVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "[2]" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantList );

  //test object
  widget->setText( QString( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "{\"foo\":\"bar\"}" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QVariantMap );

  //test empty
  widget->setText( QString( "" ) );
  QVERIFY( w_json.value().isValid() );
  // w_json value is QMapVariant
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );

  // test invalid JSON
  widget->setText( QString( "{\"body\";\"text\"}" ) );
  QVERIFY( w_json.value().isValid() );
  //invalid JSON will be parsed as text - it is up to the Postgres provider to reject it
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value().toString(), QStringLiteral( "{\"body\";\"text\"}" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );

  // test with bare integer (without container) which is valid JSON
  widget->setText( QString( "2" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value(), QVariant( 2 ) );
  QVERIFY( w_json.value().userType() == QMetaType::Int );

  // test with bare string (not valid JSON)
  widget->setText( QString( "abc" ) );
  QVERIFY( w_json.value().isValid() );
  QCOMPARE( QString::fromUtf8( QJsonDocument::fromVariant( w_json.value() ).toJson( QJsonDocument::Compact ).data() ), QStringLiteral( "" ) );
  QCOMPARE( w_json.value().toString(), QStringLiteral( "abc" ) );
  QVERIFY( w_json.value().userType() == QMetaType::QString );
}

QGSTEST_MAIN( TestQgsTextEditWrapper )
#include "testqgstexteditwrapper.moc"
