/***************************************************************************
     testqgsogrutils.cpp
     -------------------
    Date                 : February 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QSharedPointer>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspointv2.h"

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

class TestQgsOgrUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void ogrGeometryToQgsGeometry();
    void readOgrFeatureGeometry();
    void getOgrFeatureAttribute();
    void readOgrFeatureAttributes();
    void readOgrFeature();
    void readOgrFields();
    void stringToFeatureList();
    void stringToFields();

  private:

    QString mTestDataDir;
    QString mTestFile;
};

void TestQgsOgrUtils::initTestCase()
{
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  mTestFile = mTestDataDir + "ogr_types.tab";

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::registerOgrDrivers();
}

void TestQgsOgrUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsOgrUtils::init()
{

}

void TestQgsOgrUtils::cleanup()
{

}

void TestQgsOgrUtils::ogrGeometryToQgsGeometry()
{
  // test with null geometry
  QVERIFY( !QgsOgrUtils::ogrGeometryToQgsGeometry( nullptr ) );

  // get a geometry from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );
  OGRGeometryH ogrGeom = OGR_F_GetGeometryRef( oFeat );
  QVERIFY( ogrGeom );

  QScopedPointer< QgsGeometry > geom( QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom ) );
  QVERIFY( geom.data() );
  QCOMPARE( geom->geometry()->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( geom->geometry()->nCoordinates(), 71 );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::readOgrFeatureGeometry()
{
  QgsFeature f;

  // null geometry
  QgsOgrUtils::readOgrFeatureGeometry( nullptr, f );
  QVERIFY( !f.constGeometry() );

  //real geometry
  // get a geometry from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  QgsOgrUtils::readOgrFeatureGeometry( oFeat, f );
  QVERIFY( f.constGeometry() );
  QCOMPARE( f.constGeometry()->geometry()->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( f.constGeometry()->geometry()->nCoordinates(), 71 );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::getOgrFeatureAttribute()
{
  QgsFeature f;
  QgsFields fields;

  // null feature
  bool ok = false;
  QVariant val = QgsOgrUtils::getOgrFeatureAttribute( nullptr, fields, 0, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( !ok );
  QVERIFY( !val.isValid() );

  //real feature
  //get a feature from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( "int_field", QVariant::Int ) );
  fields.append( QgsField( "dbl_field", QVariant::Double ) );
  fields.append( QgsField( "date_field", QVariant::Date ) );
  fields.append( QgsField( "time_field", QVariant::Time ) );
  fields.append( QgsField( "datetime_field", QVariant::DateTime ) );
  fields.append( QgsField( "string_field", QVariant::String ) );

  // attribute index out of range
  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, -1, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( !ok );
  QVERIFY( !val.isValid() );
  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 100, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( !ok );
  QVERIFY( !val.isValid() );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 0, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( 5 ) );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 1, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( 8.9 ) );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 2, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( QDate( 2005, 01, 05 ) ) );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 3, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( QTime( 8, 11, 01 ) ) );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 4, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0 ) ) ) );

  val = QgsOgrUtils::getOgrFeatureAttribute( oFeat, fields, 5, QTextCodec::codecForName( "System" ), &ok );
  QVERIFY( ok );
  QVERIFY( val.isValid() );
  QCOMPARE( val, QVariant( "a string" ) );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::readOgrFeatureAttributes()
{
  QgsFeature f;
  QgsFields fields;

  // null feature
  QVERIFY( !QgsOgrUtils::readOgrFeatureAttributes( nullptr, fields, f, QTextCodec::codecForName( "System" ) ) );

  //real feature
  //get a feature from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( "int_field", QVariant::Int ) );
  fields.append( QgsField( "dbl_field", QVariant::Double ) );
  fields.append( QgsField( "date_field", QVariant::Date ) );
  fields.append( QgsField( "time_field", QVariant::Time ) );
  fields.append( QgsField( "datetime_field", QVariant::DateTime ) );
  fields.append( QgsField( "string_field", QVariant::String ) );

  QVERIFY( QgsOgrUtils::readOgrFeatureAttributes( oFeat, fields, f, QTextCodec::codecForName( "System" ) ) );
  QCOMPARE( f.attribute( "int_field" ), QVariant( 5 ) );
  QCOMPARE( f.attribute( "dbl_field" ), QVariant( 8.9 ) );
  QCOMPARE( f.attribute( "date_field" ), QVariant( QDate( 2005, 01, 05 ) ) );
  QCOMPARE( f.attribute( "time_field" ), QVariant( QTime( 8, 11, 01 ) ) );
  QCOMPARE( f.attribute( "datetime_field" ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0 ) ) ) );
  QCOMPARE( f.attribute( "string_field" ), QVariant( "a string" ) );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::readOgrFeature()
{
  QgsFields fields;

  // null feature
  QgsFeature f = QgsOgrUtils::readOgrFeature( nullptr, fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( !f.isValid() );

  //real feature
  //get a feature from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( "int_field", QVariant::Int ) );
  fields.append( QgsField( "dbl_field", QVariant::Double ) );
  fields.append( QgsField( "date_field", QVariant::Date ) );
  fields.append( QgsField( "time_field", QVariant::Time ) );
  fields.append( QgsField( "datetime_field", QVariant::DateTime ) );
  fields.append( QgsField( "string_field", QVariant::String ) );

  f = QgsOgrUtils::readOgrFeature( oFeat, fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( f.isValid() );
  QCOMPARE( f.id(), 1LL );
  QCOMPARE( f.attribute( "int_field" ), QVariant( 5 ) );
  QCOMPARE( f.attribute( "dbl_field" ), QVariant( 8.9 ) );
  QCOMPARE( f.attribute( "date_field" ), QVariant( QDate( 2005, 01, 05 ) ) );
  QCOMPARE( f.attribute( "time_field" ), QVariant( QTime( 8, 11, 01 ) ) );
  QCOMPARE( f.attribute( "datetime_field" ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0 ) ) ) );
  QCOMPARE( f.attribute( "string_field" ), QVariant( "a string" ) );
  QVERIFY( f.constGeometry() );
  QCOMPARE( f.constGeometry()->geometry()->wkbType(), QgsWKBTypes::LineString );
  QCOMPARE( f.constGeometry()->geometry()->nCoordinates(), 71 );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::readOgrFields()
{
  // null feature
  QgsFields f = QgsOgrUtils::readOgrFields( nullptr, QTextCodec::codecForName( "System" ) );
  QCOMPARE( f.count(), 0 );

  //real feature
  //get a feature from line file, test
  OGRDataSourceH hDS = OGROpen( TO8F( mTestFile ), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  f = QgsOgrUtils::readOgrFields( oFeat, QTextCodec::codecForName( "System" ) );
  QCOMPARE( f.count(), 6 );
  QCOMPARE( f.at( 0 ).name(), QString( "int_field" ) );
  QCOMPARE( f.at( 0 ).type(), QVariant::Int );
  QCOMPARE( f.at( 1 ).name(), QString( "dbl_field" ) );
  QCOMPARE( f.at( 1 ).type(), QVariant::Double );
  QCOMPARE( f.at( 2 ).name(), QString( "date_field" ) );
  QCOMPARE( f.at( 2 ).type(), QVariant::Date );
  QCOMPARE( f.at( 3 ).name(), QString( "time_field" ) );
  QCOMPARE( f.at( 3 ).type(), QVariant::Time );
  QCOMPARE( f.at( 4 ).name(), QString( "datetime_field" ) );
  QCOMPARE( f.at( 4 ).type(), QVariant::DateTime );
  QCOMPARE( f.at( 5 ).name(), QString( "string_field" ) );
  QCOMPARE( f.at( 5 ).type(), QVariant::String );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::stringToFeatureList()
{
  QgsFields fields;
  fields.append( QgsField( "name", QVariant::String ) );

  //empty string
  QgsFeatureList features = QgsOgrUtils::stringToFeatureList( "", fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( features.isEmpty() );
  // bad string
  features = QgsOgrUtils::stringToFeatureList( "asdasdas", fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( features.isEmpty() );

  // geojson string with 1 feature
  features = QgsOgrUtils::stringToFeatureList( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}", fields, QTextCodec::codecForName( "System" ) );
  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).constGeometry() && !features.at( 0 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 0 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  const QgsPointV2* point = dynamic_cast< QgsPointV2* >( features.at( 0 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );

  // geojson string with 2 features
  features = QgsOgrUtils::stringToFeatureList( "{ \"type\": \"FeatureCollection\",\"features\":[{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}},"
             " {\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [110, 20]},\"properties\": {\"name\": \"Henry Gale Island\"}}]}", fields, QTextCodec::codecForName( "System" ) );
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).constGeometry() && !features.at( 0 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 0 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  point = dynamic_cast< QgsPointV2* >( features.at( 0 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
  QVERIFY( features.at( 1 ).constGeometry() && !features.at( 1 ).constGeometry()->isEmpty() );
  QCOMPARE( features.at( 1 ).constGeometry()->geometry()->wkbType(), QgsWKBTypes::Point );
  point = dynamic_cast< QgsPointV2* >( features.at( 1 ).constGeometry()->geometry() );
  QCOMPARE( point->x(), 110.0 );
  QCOMPARE( point->y(), 20.0 );
  QCOMPARE( features.at( 1 ).attribute( "name" ).toString(), QString( "Henry Gale Island" ) );
}

void TestQgsOgrUtils::stringToFields()
{
  //empty string
  QgsFields fields = QgsOgrUtils::stringToFields( "", QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 0 );
  // bad string
  fields = QgsOgrUtils::stringToFields( "asdasdas", QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 0 );

  // geojson string
  fields = QgsOgrUtils::stringToFields( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}", QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QCOMPARE( fields.at( 1 ).name(), QString( "height" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Double );
}



QTEST_MAIN( TestQgsOgrUtils )
#include "testqgsogrutils.moc"
