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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"

class TestQgsOgrUtils: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void ogrGeometryToQgsGeometry();
    void ogrGeometryToQgsGeometry2_data();
    void ogrGeometryToQgsGeometry2();
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
  QVERIFY( QgsOgrUtils::ogrGeometryToQgsGeometry( nullptr ).isNull() );

  // get a geometry from line file, test
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );
  OGRGeometryH ogrGeom = OGR_F_GetGeometryRef( oFeat );
  QVERIFY( ogrGeom );

  QgsGeometry geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QVERIFY( !geom.isNull() );
  QCOMPARE( geom.constGet()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( geom.constGet()->nCoordinates(), 71 );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );

  ogrGeom = nullptr;
  QByteArray wkt( "point( 1.1 2.2)" );
  char *wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "Point (1.1 2.2)" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "point z ( 1.1 2.2 3)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointZ (1.1 2.2 3)" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "point m ( 1.1 2.2 3)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointM (1.1 2.2 3)" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "point zm ( 1.1 2.2 3 4)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointZM (1.1 2.2 3 4)" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "multipoint( 1.1 2.2, 3.3 4.4)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPoint ((1.1 2.2),(3.3 4.4))" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "multipoint z ((1.1 2.2 3), (3.3 4.4 4))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointZ ((1.1 2.2 3),(3.3 4.4 4))" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "multipoint m ((1.1 2.2 3), (3.3 4.4 4))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointM ((1.1 2.2 3),(3.3 4.4 4))" ) );

  ogrGeom = nullptr;
  wkt = QByteArray( "multipoint zm ((1.1 2.2 3 4), (3.3 4.4 4 5))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointZM ((1.1 2.2 3 4),(3.3 4.4 4 5))" ) );
}

void TestQgsOgrUtils::ogrGeometryToQgsGeometry2_data()
{
  QTest::addColumn<QString>( "wkt" );
  QTest::addColumn<int>( "type" );

  QTest::newRow( "point" ) << QStringLiteral( "Point (1.1 2.2)" ) << static_cast< int >( QgsWkbTypes::Point );
  QTest::newRow( "pointz" ) << QStringLiteral( "PointZ (1.1 2.2 3.3)" ) <<  static_cast< int >( QgsWkbTypes::Point25D ); // ogr uses 25d for z
  QTest::newRow( "pointm" ) << QStringLiteral( "PointM (1.1 2.2 3.3)" ) <<  static_cast< int >( QgsWkbTypes::PointM );
  QTest::newRow( "pointzm" ) << QStringLiteral( "PointZM (1.1 2.2 3.3 4.4)" ) <<  static_cast< int >( QgsWkbTypes::PointZM );
  QTest::newRow( "point25d" ) << QStringLiteral( "Point25D (1.1 2.2 3.3)" ) <<  static_cast< int >( QgsWkbTypes::Point25D );

  QTest::newRow( "linestring" ) << QStringLiteral( "LineString (1.1 2.2, 3.3 4.4)" ) << static_cast< int >( QgsWkbTypes::LineString );
  QTest::newRow( "linestringz" ) << QStringLiteral( "LineStringZ (1.1 2.2 3.3, 4.4 5.5 6.6)" ) <<  static_cast< int >( QgsWkbTypes::LineString25D ); // ogr uses 25d for z
  QTest::newRow( "linestringm" ) << QStringLiteral( "LineStringM (1.1 2.2 3.3, 4.4 5.5 6.6)" ) <<  static_cast< int >( QgsWkbTypes::LineStringM );
  QTest::newRow( "linestringzm" ) << QStringLiteral( "LineStringZM (1.1 2.2 3.3 4.4, 5.5 6.6 7.7 8.8)" ) <<  static_cast< int >( QgsWkbTypes::LineStringZM );
  QTest::newRow( "linestring25d" ) << QStringLiteral( "LineString25D (1.1 2.2 3.3, 4.4 5.5 6.6)" ) <<  static_cast< int >( QgsWkbTypes::LineString25D );

  QTest::newRow( "linestring" ) << QStringLiteral( "MultiLineString ((1.1 2.2, 3.3 4.4))" ) << static_cast< int >( QgsWkbTypes::MultiLineString );
  QTest::newRow( "linestring" ) << QStringLiteral( "MultiLineString ((1.1 2.2, 3.3 4.4),(5 5, 6 6))" ) << static_cast< int >( QgsWkbTypes::MultiLineString );
  QTest::newRow( "linestring" ) << QStringLiteral( "MultiLineStringZ ((1.1 2.2 3, 3.3 4.4 6),(5 5 3, 6 6 1))" ) << static_cast< int >( QgsWkbTypes::MultiLineStringZ );
  QTest::newRow( "linestring" ) << QStringLiteral( "MultiLineStringM ((1.1 2.2 4, 3.3 4.4 7),(5 5 4, 6 6 2))" ) << static_cast< int >( QgsWkbTypes::MultiLineStringM );
  QTest::newRow( "linestring" ) << QStringLiteral( "MultiLineStringZM ((1.1 2.2 4 5, 3.3 4.4 8 9),(5 5 7 1, 6 6 2 3))" ) << static_cast< int >( QgsWkbTypes::MultiLineStringZM );
}

void TestQgsOgrUtils::ogrGeometryToQgsGeometry2()
{
  QFETCH( QString, wkt );
  QFETCH( int, type );

  QgsGeometry input = QgsGeometry::fromWkt( wkt );
  QVERIFY( !input.isNull() );

  // to OGR Geometry
  QByteArray wkb( input.asWkb() );
  OGRGeometryH ogrGeom = nullptr;

  QCOMPARE( OGR_G_CreateFromWkb( reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ), nullptr, &ogrGeom, wkb.length() ), OGRERR_NONE );

  // back again!
  QgsGeometry geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( static_cast< int >( geom.wkbType() ), type );

  // bit of trickiness here - QGIS wkt conversion changes 25D -> Z, so account for that
  wkt.replace( QStringLiteral( "25D" ), QStringLiteral( "Z" ) );
  QCOMPARE( geom.asWkt( 3 ), wkt );
}

void TestQgsOgrUtils::readOgrFeatureGeometry()
{
  QgsFeature f;

  // null geometry
  QgsOgrUtils::readOgrFeatureGeometry( nullptr, f );
  QVERIFY( !f.hasGeometry() );

  //real geometry
  // get a geometry from line file, test
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  QgsOgrUtils::readOgrFeatureGeometry( oFeat, f );
  QVERIFY( f.hasGeometry() );
  QCOMPARE( f.geometry().constGet()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( f.geometry().constGet()->nCoordinates(), 71 );

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
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( QStringLiteral( "int_field" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "dbl_field" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "date_field" ), QVariant::Date ) );
  fields.append( QgsField( QStringLiteral( "time_field" ), QVariant::Time ) );
  fields.append( QgsField( QStringLiteral( "datetime_field" ), QVariant::DateTime ) );
  fields.append( QgsField( QStringLiteral( "string_field" ), QVariant::String ) );

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
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( QStringLiteral( "int_field" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "dbl_field" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "date_field" ), QVariant::Date ) );
  fields.append( QgsField( QStringLiteral( "time_field" ), QVariant::Time ) );
  fields.append( QgsField( QStringLiteral( "datetime_field" ), QVariant::DateTime ) );
  fields.append( QgsField( QStringLiteral( "string_field" ), QVariant::String ) );

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
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );

  fields.append( QgsField( QStringLiteral( "int_field" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "dbl_field" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "date_field" ), QVariant::Date ) );
  fields.append( QgsField( QStringLiteral( "time_field" ), QVariant::Time ) );
  fields.append( QgsField( QStringLiteral( "datetime_field" ), QVariant::DateTime ) );
  fields.append( QgsField( QStringLiteral( "string_field" ), QVariant::String ) );

  f = QgsOgrUtils::readOgrFeature( oFeat, fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( f.isValid() );
  QCOMPARE( f.id(), 1LL );
  QCOMPARE( f.attribute( "int_field" ), QVariant( 5 ) );
  QCOMPARE( f.attribute( "dbl_field" ), QVariant( 8.9 ) );
  QCOMPARE( f.attribute( "date_field" ), QVariant( QDate( 2005, 01, 05 ) ) );
  QCOMPARE( f.attribute( "time_field" ), QVariant( QTime( 8, 11, 01 ) ) );
  QCOMPARE( f.attribute( "datetime_field" ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0 ) ) ) );
  QCOMPARE( f.attribute( "string_field" ), QVariant( "a string" ) );
  QVERIFY( f.hasGeometry() );
  QCOMPARE( f.geometry().constGet()->wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( f.geometry().constGet()->nCoordinates(), 71 );

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
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
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
  fields.append( QgsField( QStringLiteral( "name" ), QVariant::String ) );

  //empty string
  QgsFeatureList features = QgsOgrUtils::stringToFeatureList( QString(), fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( features.isEmpty() );
  // bad string
  features = QgsOgrUtils::stringToFeatureList( QStringLiteral( "asdasdas" ), fields, QTextCodec::codecForName( "System" ) );
  QVERIFY( features.isEmpty() );

  // geojson string with 1 feature
  features = QgsOgrUtils::stringToFeatureList( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}}" ), fields, QTextCodec::codecForName( "System" ) );
  QCOMPARE( features.length(), 1 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  QgsGeometry featureGeom = features.at( 0 ).geometry();
  const QgsPoint *point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );

  // geojson string with 2 features
  features = QgsOgrUtils::stringToFeatureList( "{ \"type\": \"FeatureCollection\",\"features\":[{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\"}},"
             " {\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [110, 20]},\"properties\": {\"name\": \"Henry Gale Island\"}}]}", fields, QTextCodec::codecForName( "System" ) );
  QCOMPARE( features.length(), 2 );
  QVERIFY( features.at( 0 ).hasGeometry() && !features.at( 0 ).geometry().isNull() );
  QCOMPARE( features.at( 0 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  featureGeom = features.at( 0 ).geometry();
  point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 125.0 );
  QCOMPARE( point->y(), 10.0 );
  QCOMPARE( features.at( 0 ).attribute( "name" ).toString(), QString( "Dinagat Islands" ) );
  QVERIFY( features.at( 1 ).hasGeometry() && !features.at( 1 ).geometry().isNull() );
  QCOMPARE( features.at( 1 ).geometry().constGet()->wkbType(), QgsWkbTypes::Point );
  featureGeom = features.at( 1 ).geometry();
  point = dynamic_cast< const QgsPoint * >( featureGeom.constGet() );
  QCOMPARE( point->x(), 110.0 );
  QCOMPARE( point->y(), 20.0 );
  QCOMPARE( features.at( 1 ).attribute( "name" ).toString(), QString( "Henry Gale Island" ) );
}

void TestQgsOgrUtils::stringToFields()
{
  //empty string
  QgsFields fields = QgsOgrUtils::stringToFields( QString(), QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 0 );
  // bad string
  fields = QgsOgrUtils::stringToFields( QStringLiteral( "asdasdas" ), QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 0 );

  // geojson string
  fields = QgsOgrUtils::stringToFields( QStringLiteral( "{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}" ), QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QCOMPARE( fields.at( 1 ).name(), QString( "height" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Double );
}



QGSTEST_MAIN( TestQgsOgrUtils )
#include "testqgsogrutils.moc"
