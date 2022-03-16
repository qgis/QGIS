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
#include <QDate>
#include <QTime>
#include <QDateTime>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"
#include <ogr_srs_api.h>
#include <gdal.h>

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"
#include "qgsogrproxytextcodec.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfontutils.h"
#include "qgssymbol.h"
#include "qgsfielddomain.h"

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
    void textCodec();
    void parseStyleString_data();
    void parseStyleString();
    void convertStyleString();
    void ogrCrsConversion();
    void ogrFieldToVariant();
    void variantToOgrField();
    void testOgrFieldTypeToQVariantType_data();
    void testOgrFieldTypeToQVariantType();
    void testVariantTypeToOgrFieldType_data();
    void testVariantTypeToOgrFieldType();
    void testOgrStringToVariant_data();
    void testOgrStringToVariant();

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
    void testConvertFieldDomain();
    void testConvertToFieldDomain();
#endif

  private:

    QString mTestDataDir;
    QString mTestFile;
};

void TestQgsOgrUtils::initTestCase()
{
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
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
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "point z ( 1.1 2.2 3)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointZ (1.1 2.2 3)" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "point m ( 1.1 2.2 3)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointM (1.1 2.2 3)" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "point zm ( 1.1 2.2 3 4)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "PointZM (1.1 2.2 3 4)" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "multipoint( 1.1 2.2, 3.3 4.4)" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPoint ((1.1 2.2),(3.3 4.4))" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "multipoint z ((1.1 2.2 3), (3.3 4.4 4))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointZ ((1.1 2.2 3),(3.3 4.4 4))" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "multipoint m ((1.1 2.2 3), (3.3 4.4 4))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointM ((1.1 2.2 3),(3.3 4.4 4))" ) );
  OGR_G_DestroyGeometry( ogrGeom );
  ogrGeom = nullptr;

  wkt = QByteArray( "multipoint zm ((1.1 2.2 3 4), (3.3 4.4 4 5))" );
  wktChar = wkt.data();
  OGR_G_CreateFromWkt( &wktChar, nullptr, &ogrGeom );
  geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( geom.asWkt( 3 ), QStringLiteral( "MultiPointZM ((1.1 2.2 3 4),(3.3 4.4 4 5))" ) );
  OGR_G_DestroyGeometry( ogrGeom );
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

  const QgsGeometry input = QgsGeometry::fromWkt( wkt );
  QVERIFY( !input.isNull() );

  // to OGR Geometry
  const QByteArray wkb( input.asWkb() );
  OGRGeometryH ogrGeom = nullptr;

  QCOMPARE( OGR_G_CreateFromWkb( reinterpret_cast<unsigned char *>( const_cast<char *>( wkb.constData() ) ), nullptr, &ogrGeom, wkb.length() ), OGRERR_NONE );

  // back again!
  const QgsGeometry geom = QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom );
  QCOMPARE( static_cast< int >( geom.wkbType() ), type );
  OGR_G_DestroyGeometry( ogrGeom );

  // bit of trickiness here - QGIS wkt conversion changes 25D -> Z, so account for that
  wkt.replace( QLatin1String( "25D" ), QLatin1String( "Z" ) );
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
  const QgsFeature f;
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
  QCOMPARE( val, QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0, 123 ) ) ) );

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
  QCOMPARE( f.attribute( "datetime_field" ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0, 123 ) ) ) );
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
  QCOMPARE( f.attribute( "datetime_field" ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0, 123 ) ) ) );
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

  // geojson string with 2 features
  fields = QgsOgrUtils::stringToFields( QStringLiteral( "{ \"type\": \"FeatureCollection\",\"features\":[{\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [125, 10]},\"properties\": {\"name\": \"Dinagat Islands\",\"height\":5.5}}, {\n\"type\": \"Feature\",\"geometry\": {\"type\": \"Point\",\"coordinates\": [110, 20]},\"properties\": {\"name\": \"Henry Gale Island\",\"height\":6.5}}]}" ), QTextCodec::codecForName( "System" ) );
  QCOMPARE( fields.count(), 2 );
  QCOMPARE( fields.at( 0 ).name(), QString( "name" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::String );
  QCOMPARE( fields.at( 1 ).name(), QString( "height" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Double );
}

void TestQgsOgrUtils::textCodec()
{
  QVERIFY( QgsOgrProxyTextCodec::supportedCodecs().contains( QStringLiteral( "CP852" ) ) );
  QVERIFY( !QgsOgrProxyTextCodec::supportedCodecs().contains( QStringLiteral( "xxx" ) ) );

  // The QTextCodec should always be constructed on the heap. Qt takes ownership and will delete it when the application terminates.
  QgsOgrProxyTextCodec *codec = new QgsOgrProxyTextCodec( "CP852" );
  QCOMPARE( codec->toUnicode( codec->fromUnicode( "abcŐ" ) ), QStringLiteral( "abcŐ" ) );
  QCOMPARE( codec->toUnicode( codec->fromUnicode( "" ) ), QString() );
  // cppcheck-suppress memleak
}

void TestQgsOgrUtils::parseStyleString_data()
{
  QTest::addColumn<QString>( "string" );
  QTest::addColumn<QVariantMap>( "expected" );

  QTest::newRow( "symbol" ) << QStringLiteral( R"""(SYMBOL(a:0,c:#000000,s:12pt,id:"mapinfo-sym-35,ogr-sym-10"))""" ) << QVariantMap{ { "symbol", QVariantMap{ { "a", "0"},
        {"c", "#000000"},
        {"s", "12pt"},
        {"id", "mapinfo-sym-35,ogr-sym-10"},
      }
    } };

  QTest::newRow( "pen" ) << QStringLiteral( R"""(PEN(w:2px,c:#ffb060,id:"mapinfo-pen-14,ogr-pen-6",p:"8 2 1 2px"))""" ) << QVariantMap{ { "pen", QVariantMap{ { "w", "2px"},
        {"c", "#ffb060"},
        {"id", "mapinfo-pen-14,ogr-pen-6"},
        {"p", "8 2 1 2px"},
      }
    } };

  QTest::newRow( "brush and pen" ) << QStringLiteral( R"""(BRUSH(FC:#ff8000,bc:#f0f000,id:"mapinfo-brush-6,ogr-brush-4");pen(W:3px,c:#e00000,id:"mapinfo-pen-2,ogr-pen-0"))""" )
  << QVariantMap{ { "brush", QVariantMap{ { "fc", "#ff8000"},
        {"bc", "#f0f000"},
        {"id", "mapinfo-brush-6,ogr-brush-4"}
      }
    },
    {
      "pen", QVariantMap{   { "w", "3px"},
        {"c", "#e00000"},
        {"id", "mapinfo-pen-2,ogr-pen-0"}
      }
    }
  };
}

void TestQgsOgrUtils::parseStyleString()
{
  QFETCH( QString, string );
  QFETCH( QVariantMap, expected );

  const QVariantMap res = QgsOgrUtils::parseStyleString( string );
  QCOMPARE( expected, res );
}

void TestQgsOgrUtils::convertStyleString()
{
  std::unique_ptr<QgsSymbol> symbol( QgsOgrUtils::symbolFromStyleString( QStringLiteral( "xxx" ), Qgis::SymbolType::Line ) );
  QVERIFY( !symbol );
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(PEN(w:7px,c:#0040c0,id:"mapinfo-pen-5,ogr-pen-3",p:"3 1px"))""" ), Qgis::SymbolType::Line );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( dynamic_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#0040c0" ) );
  // px sizes should be converted to pts
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->width(), 5.25 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->widthUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->penCapStyle(), Qt::RoundCap );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->penJoinStyle(), Qt::RoundJoin );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->customDashVector().at( 0 ), 21.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->customDashVector().at( 1 ), 10.5 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->customDashPatternUnit(), QgsUnitTypes::RenderPoints );
  QVERIFY( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->useCustomDashPattern() );

  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(PEN(c:#00000087,w:10.500000cm,cap:p,j:b))""" ), Qgis::SymbolType::Line );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#000000" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 135 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->width(), 105.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->widthUnit(), QgsUnitTypes::RenderMillimeters );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->penCapStyle(), Qt::SquareCap );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->penJoinStyle(), Qt::BevelJoin );

  // both brush and pen, but requesting a line symbol only
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(PEN(c:#FFFF007F,w:4.000000pt);BRUSH(fc:#00FF007F))""" ), Qgis::SymbolType::Line );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#ffff00" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 127 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->width(), 4.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer * >( symbol->symbolLayer( 0 ) )->widthUnit(), QgsUnitTypes::RenderPoints );

  // brush
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(BRUSH(fc:#00FF007F))""" ), Qgis::SymbolType::Fill );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 127 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->brushStyle(), Qt::SolidPattern );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::NoPen );

  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(BRUSH(fc:#00FF007F,bc:#00000087,id:ogr-brush-6))""" ), Qgis::SymbolType::Fill );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 2 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#000000" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 135 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->brushStyle(), Qt::SolidPattern );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::NoPen );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 1 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 1 ) )->color().alpha(), 127 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 1 ) )->brushStyle(), Qt::CrossPattern );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 1 ) )->strokeStyle(), Qt::NoPen );

  // brush with pen
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(PEN(c:#FFFF007F,w:4.000000pt);BRUSH(fc:#00FF007F))""" ), Qgis::SymbolType::Fill );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 127 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->brushStyle(), Qt::SolidPattern );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::SolidLine );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeColor().name(), QStringLiteral( "#ffff00" ) );

  // no brush, but need fill symbol
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(PEN(c:#FFFF007F,w:4.000000pt))""" ), Qgis::SymbolType::Fill );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->brushStyle(), Qt::NoBrush );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::SolidLine );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeColor().name(), QStringLiteral( "#ffff00" ) );

  // symbol
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(a:0,c:#5050ff,s:36pt,id:"ogr-sym-5"))""" ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#5050ff" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->shape(), Qgis::MarkerShape::Square );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 36.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), 0.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::NoPen );
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(a:0,c:#5050ff,s:36pt,id:"ogr-sym-6"))""" ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().alpha(), 0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeColor().name(), QStringLiteral( "#5050ff" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->shape(), Qgis::MarkerShape::Triangle );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 36.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), 0.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::SolidLine );
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(a:20,c:#5050ff,s:36pt,id:"ogr-sym-5"))""" ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#5050ff" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->shape(), Qgis::MarkerShape::Square );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 36.0 );
  // OGR symbol angles are opposite direction to qgis marker angles
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), -20.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::NoPen );
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(c:#5050ff,o:#3030ff,s:36pt,id:"ogr-sym-5"))""" ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#5050ff" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->shape(), Qgis::MarkerShape::Square );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 36.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), 0.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::SolidLine );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeColor().name(), QStringLiteral( "#3030ff" ) );

  // font symbol
  const QFont f = QgsFontUtils::getStandardTestFont();
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(c:#00FF00,s:12pt,id:"font-sym-75,ogr-sym-9",f:"%1"))""" ).arg( f.family() ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->character(), QStringLiteral( "K" ) );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 12.0 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), 0.0 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeWidth(), 0 );

  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(a:20,c:#00FF00,o:#3030ff,s:12pt,id:"font-sym-75,ogr-sym-9",f:"%1"))""" ).arg( f.family() ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->character(), QStringLiteral( "K" ) );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 12.0 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), -20.0 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeWidth(), 1 );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeWidthUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsFontMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeColor().name(), QStringLiteral( "#3030ff" ) );

  // bad font name, should fallback to ogr symbol id
  symbol = QgsOgrUtils::symbolFromStyleString( QStringLiteral( R"""(SYMBOL(c:#00FF00,s:12pt,id:"font-sym-75,ogr-sym-9",f:"xxxxxx"))""" ), Qgis::SymbolType::Marker );
  QVERIFY( symbol );
  QCOMPARE( symbol->symbolLayerCount(), 1 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->color().name(), QStringLiteral( "#00ff00" ) );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->shape(), Qgis::MarkerShape::Star );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->size(), 12.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->angle(), 0.0 );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->sizeUnit(), QgsUnitTypes::RenderPoints );
  QCOMPARE( qgis::down_cast<QgsSimpleMarkerSymbolLayer * >( symbol->symbolLayer( 0 ) )->strokeStyle(), Qt::NoPen );
}

void TestQgsOgrUtils::ogrCrsConversion()
{
  // test conversion utilities for OGR srs objects
  {
    const QgsCoordinateReferenceSystem crs1( QStringLiteral( "EPSG:3111" ) );
    OGRSpatialReferenceH srs = QgsOgrUtils::crsToOGRSpatialReference( crs1 );
    QVERIFY( srs );

    // Check that OGRSpatialReferenceH object built has all information preserved
    const char *authName = OSRGetAuthorityName( srs, "DATUM" );
    QVERIFY( authName );
    QCOMPARE( QString( authName ), "EPSG" );
    const char *authCode = OSRGetAuthorityCode( srs, "DATUM" );
    QVERIFY( authCode );
    QCOMPARE( QString( authCode ), "6283" );

    const QgsCoordinateReferenceSystem crs2( QgsOgrUtils::OGRSpatialReferenceToCrs( srs ) );
    // round trip should be lossless
    QCOMPARE( crs1, crs2 );
    OSRRelease( srs );

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
    QVERIFY( std::isnan( crs2.coordinateEpoch() ) );
#endif
  }

  {
    OGRSpatialReferenceH srs = OSRNewSpatialReference( "PROJCS[\"GDA94 / Vicgrid\",GEOGCS[\"GDA94\",DATUM[\"Geocentric_Datum_of_Australia_1994\",SPHEROID[\"GRS 1980\",6378137,298.257222101,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6283\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4283\"]],PROJECTION[\"Lambert_Conformal_Conic_2SP\"],PARAMETER[\"latitude_of_origin\",-37],PARAMETER[\"central_meridian\",145],PARAMETER[\"standard_parallel_1\",-36],PARAMETER[\"standard_parallel_2\",-38],PARAMETER[\"false_easting\",2500000],PARAMETER[\"false_northing\",2500000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"3111\"]]" );
    // Check that we used EPSG:3111 to instantiate the CRS, and thus get the
    // extent from PROJ
    const QgsCoordinateReferenceSystem crs( QgsOgrUtils::OGRSpatialReferenceToCrs( srs ) );
    OSRRelease( srs );
    QVERIFY( !crs.bounds().isEmpty() );
  }

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,4,0)
  {
    // test conversion with a coordinate epoch, should work on GDAL 3.4+
    QgsCoordinateReferenceSystem crs1( QStringLiteral( "EPSG:4326" ) );
    crs1.setCoordinateEpoch( 2020.7 );
    OGRSpatialReferenceH srs = QgsOgrUtils::crsToOGRSpatialReference( crs1 );
    QVERIFY( srs );
    const QgsCoordinateReferenceSystem crs2( QgsOgrUtils::OGRSpatialReferenceToCrs( srs ) );
    // round trip should be lossless
    QCOMPARE( crs1, crs2 );
    QCOMPARE( crs2.coordinateEpoch(), 2020.7 );
    OSRRelease( srs );
  }
#endif
}

void TestQgsOgrUtils::ogrFieldToVariant()
{
  OGRDataSourceH hDS = OGROpen( mTestFile.toUtf8().constData(), false, nullptr );
  QVERIFY( hDS );
  OGRLayerH ogrLayer = OGR_DS_GetLayer( hDS, 0 );
  QVERIFY( ogrLayer );
  OGRFeatureH oFeat;
  oFeat = OGR_L_GetNextFeature( ogrLayer );
  QVERIFY( oFeat );
  OGRField oFieldInt, oFieldDbl, oFieldDate, oFieldTime, oFieldDatetime, oFieldString;
  oFieldInt = *OGR_F_GetRawFieldRef( oFeat, 0 );
  oFieldDbl = *OGR_F_GetRawFieldRef( oFeat, 1 );
  oFieldDate = *OGR_F_GetRawFieldRef( oFeat, 2 );
  oFieldTime = *OGR_F_GetRawFieldRef( oFeat, 3 );
  oFieldDatetime = *OGR_F_GetRawFieldRef( oFeat, 4 );
  oFieldString = *OGR_F_GetRawFieldRef( oFeat, 5 );

  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldInt, OGRFieldType::OFTInteger ), QVariant( 5 ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldDbl, OGRFieldType::OFTReal ), QVariant( 8.9 ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldDate, OGRFieldType::OFTDate ), QVariant( QDate( 2005, 01, 05 ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldTime, OGRFieldType::OFTTime ), QVariant( QTime( 8, 11, 01 ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldDatetime, OGRFieldType::OFTDateTime ), QVariant( QDateTime( QDate( 2005, 3, 5 ), QTime( 6, 45, 0, 123 ) ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( &oFieldString, OGRFieldType::OFTString ), QVariant( "a string" ) );

  OGR_F_Destroy( oFeat );
  OGR_DS_Destroy( hDS );
}

void TestQgsOgrUtils::variantToOgrField()
{
  std::unique_ptr<OGRField> field( QgsOgrUtils::variantToOGRField( QVariant() ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTInteger ), QVariant() );

  field = QgsOgrUtils::variantToOGRField( QVariant( true ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTInteger ), QVariant( 1 ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( false ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTInteger ), QVariant( 0 ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( 11 ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTInteger ), QVariant( 11 ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( 11LL ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTInteger64 ), QVariant( 11LL ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( 5.5 ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTReal ), QVariant( 5.5 ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( QStringLiteral( "abc" ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTString ), QVariant( QStringLiteral( "abc" ) ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( QDate( 2021, 2, 3 ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTDate ), QVariant( QDate( 2021, 2, 3 ) ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( QTime( 12, 13, 14, 50 ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTTime ), QVariant( QTime( 12, 13, 14, 50 ) ) );
  field = QgsOgrUtils::variantToOGRField( QVariant( QDateTime( QDate( 2021, 2, 3 ), QTime( 12, 13, 14, 50 ) ) ) );
  QCOMPARE( QgsOgrUtils::OGRFieldtoVariant( field.get(), OFTDateTime ), QVariant( QDateTime( QDate( 2021, 2, 3 ), QTime( 12, 13, 14, 50 ) ) ) );
}

void TestQgsOgrUtils::testOgrFieldTypeToQVariantType_data()
{
  QTest::addColumn<int>( "ogrType" );
  QTest::addColumn<int>( "ogrSubType" );
  QTest::addColumn<int>( "expectedType" );
  QTest::addColumn<int>( "expectedSubType" );

  QTest::newRow( "OFTInteger" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::Int ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTIntegerList" ) << static_cast< int >( OFTIntegerList ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::List ) << static_cast< int >( QVariant::Int );

  QTest::newRow( "OFSTBoolean" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTBoolean ) << static_cast< int >( QVariant::Bool ) << static_cast< int >( QVariant::Invalid );

  QTest::newRow( "OFTReal" ) << static_cast< int >( OFTReal ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::Double ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTRealList" ) << static_cast< int >( OFTRealList ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::List ) << static_cast< int >( QVariant::Double );

  QTest::newRow( "OFTString" ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::String ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTStringList" ) << static_cast< int >( OFTStringList ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::StringList ) << static_cast< int >( QVariant::String );
  QTest::newRow( "OFTWideString" ) << static_cast< int >( OFTWideString ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::String ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTWideStringList" ) << static_cast< int >( OFTWideStringList ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::StringList ) << static_cast< int >( QVariant::String );

  QTest::newRow( "OFTString OFSTJSON" ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTJSON ) << static_cast< int >( QVariant::Map ) << static_cast< int >( QVariant::String );
  QTest::newRow( "OFTWideString OFSTJSON" ) << static_cast< int >( OFTWideString ) << static_cast< int >( OFSTJSON ) << static_cast< int >( QVariant::Map ) << static_cast< int >( QVariant::String );

  QTest::newRow( "OFTInteger64" ) << static_cast< int >( OFTInteger64 ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::LongLong ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTInteger64List" ) << static_cast< int >( OFTInteger64List ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::List ) << static_cast< int >( QVariant::LongLong );

  QTest::newRow( "OFTBinary" ) << static_cast< int >( OFTBinary ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::ByteArray ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTDate" ) << static_cast< int >( OFTDate ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::Date ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTTime" ) << static_cast< int >( OFTTime ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::Time ) << static_cast< int >( QVariant::Invalid );
  QTest::newRow( "OFTDateTime" ) << static_cast< int >( OFTDateTime ) << static_cast< int >( OFSTNone ) << static_cast< int >( QVariant::DateTime ) << static_cast< int >( QVariant::Invalid );
}

void TestQgsOgrUtils::testOgrFieldTypeToQVariantType()
{
  QFETCH( int, ogrType );
  QFETCH( int, ogrSubType );
  QFETCH( int, expectedType );
  QFETCH( int, expectedSubType );

  QVariant::Type variantType;
  QVariant::Type variantSubType;
  QgsOgrUtils::ogrFieldTypeToQVariantType( static_cast<OGRFieldType>( ogrType ),
      static_cast<OGRFieldSubType>( ogrSubType ),
      variantType, variantSubType );
  QCOMPARE( static_cast< int >( variantType ), expectedType );
  QCOMPARE( static_cast< int >( variantSubType ), expectedSubType );
}

void TestQgsOgrUtils::testVariantTypeToOgrFieldType_data()
{
  QTest::addColumn<int>( "variantType" );
  QTest::addColumn<int>( "expectedType" );
  QTest::addColumn<int>( "expectedSubType" );

  QTest::newRow( "Bool" ) << static_cast< int >( QVariant::Type::Bool ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTBoolean );
  QTest::newRow( "Int" ) << static_cast< int >( QVariant::Type::Int ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone );
  QTest::newRow( "LongLong" ) << static_cast< int >( QVariant::Type::LongLong ) << static_cast< int >( OFTInteger64 ) << static_cast< int >( OFSTNone );
  QTest::newRow( "Double" ) << static_cast< int >( QVariant::Type::Double ) << static_cast< int >( OFTReal ) << static_cast< int >( OFSTNone );
  QTest::newRow( "Char" ) << static_cast< int >( QVariant::Type::Char ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTNone );
  QTest::newRow( "String" ) << static_cast< int >( QVariant::Type::String ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTNone );
  QTest::newRow( "StringList" ) << static_cast< int >( QVariant::Type::StringList ) << static_cast< int >( OFTStringList ) << static_cast< int >( OFSTNone );
  QTest::newRow( "ByteArray" ) << static_cast< int >( QVariant::Type::ByteArray ) << static_cast< int >( OFTBinary ) << static_cast< int >( OFSTNone );
  QTest::newRow( "Date" ) << static_cast< int >( QVariant::Type::Date ) << static_cast< int >( OFTDate ) << static_cast< int >( OFSTNone );
  QTest::newRow( "Time" ) << static_cast< int >( QVariant::Type::Time ) << static_cast< int >( OFTTime ) << static_cast< int >( OFSTNone );
  QTest::newRow( "DateTime" ) << static_cast< int >( QVariant::Type::DateTime ) << static_cast< int >( OFTDateTime ) << static_cast< int >( OFSTNone );
}

void TestQgsOgrUtils::testVariantTypeToOgrFieldType()
{
  QFETCH( int, variantType );
  QFETCH( int, expectedType );
  QFETCH( int, expectedSubType );

  OGRFieldType type;
  OGRFieldSubType subType;
  QgsOgrUtils::variantTypeToOgrFieldType( static_cast<QVariant::Type>( variantType ),
                                          type, subType );
  QCOMPARE( static_cast< int >( type ), expectedType );
  QCOMPARE( static_cast< int >( subType ), expectedSubType );
}

void TestQgsOgrUtils::testOgrStringToVariant_data()
{
  QTest::addColumn<int>( "ogrType" );
  QTest::addColumn<int>( "ogrSubType" );
  QTest::addColumn<QString>( "string" );
  QTest::addColumn<QVariant>( "expected" );

  QTest::newRow( "OFTInteger null" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTInteger 5" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone ) << QStringLiteral( "5" ) << QVariant( 5 );

  QTest::newRow( "OFTInteger64 null" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTInteger64 5" ) << static_cast< int >( OFTInteger ) << static_cast< int >( OFSTNone ) << QStringLiteral( "5" ) << QVariant( 5LL );

  QTest::newRow( "OFTReal null" ) << static_cast< int >( OFTReal ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTReal 5.5" ) << static_cast< int >( OFTReal ) << static_cast< int >( OFSTNone ) << QStringLiteral( "5.5" ) << QVariant( 5.5 );
  QTest::newRow( "OFTReal -5.5" ) << static_cast< int >( OFTReal ) << static_cast< int >( OFSTNone ) << QStringLiteral( "-5.5" ) << QVariant( -5.5 );

  QTest::newRow( "OFTString null" ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTString aaaa" ) << static_cast< int >( OFTString ) << static_cast< int >( OFSTNone ) << QStringLiteral( "aaaa" ) << QVariant( QStringLiteral( "aaaa" ) );

  QTest::newRow( "OFTWideString null" ) << static_cast< int >( OFTWideString ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTWideString aaaa" ) << static_cast< int >( OFTWideString ) << static_cast< int >( OFSTNone ) << QStringLiteral( "aaaa" ) << QVariant( QStringLiteral( "aaaa" ) );

  QTest::newRow( "OFTDate null" ) << static_cast< int >( OFTDate ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTDate 2021-03-04" ) << static_cast< int >( OFTDate ) << static_cast< int >( OFSTNone ) << QStringLiteral( "2021-03-04" ) << QVariant( QDate( 2021, 3, 4 ) );

  QTest::newRow( "OFTTime null" ) << static_cast< int >( OFTTime ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTTime aaaa" ) << static_cast< int >( OFTTime ) << static_cast< int >( OFSTNone ) << QStringLiteral( "13:14:15" ) << QVariant( QTime( 13, 14, 15 ) );

  QTest::newRow( "OFTDateTime null" ) << static_cast< int >( OFTDateTime ) << static_cast< int >( OFSTNone ) << QString( "" ) << QVariant();
  QTest::newRow( "OFTDateTime aaaa" ) << static_cast< int >( OFTDateTime ) << static_cast< int >( OFSTNone ) << QStringLiteral( "2021-03-04 13:14:15" ) << QVariant( QDateTime( QDate( 2021, 3, 4 ), QTime( 13, 14, 15 ) ) );
}

void TestQgsOgrUtils::testOgrStringToVariant()
{
  QFETCH( int, ogrType );
  QFETCH( int, ogrSubType );
  QFETCH( QString, string );
  QFETCH( QVariant, expected );

  const QVariant res = QgsOgrUtils::stringToVariant( static_cast<OGRFieldType>( ogrType ),
                       static_cast<OGRFieldSubType>( ogrSubType ),
                       string );
  QCOMPARE( res, expected );
}

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,3,0)
void TestQgsOgrUtils::testConvertFieldDomain()
{
  OGRCodedValue v1;
  v1.pszCode = const_cast< char *>( "1" );
  v1.pszValue = const_cast< char *>( "val1" );
  OGRCodedValue v2;
  v2.pszCode = const_cast< char *>( "2" );
  v2.pszValue = const_cast< char *>( "val2" );
  OGRCodedValue v3;
  v3.pszCode = nullptr;
  v3.pszValue = nullptr;
  OGRCodedValue values[] =
  {
    v1,
    v2,
    v3
  };
  OGRFieldDomainH domain = OGR_CodedFldDomain_Create( "name", "desc", OFTInteger, OFSTNone, values );

  std::unique_ptr< QgsFieldDomain > res = QgsOgrUtils::convertFieldDomain( domain );
  QgsCodedFieldDomain *codedFieldDomain = dynamic_cast< QgsCodedFieldDomain *>( res.get() );
  QVERIFY( codedFieldDomain );
  QCOMPARE( codedFieldDomain->name(), QStringLiteral( "name" ) );
  QCOMPARE( codedFieldDomain->description(), QStringLiteral( "desc" ) );
  QCOMPARE( codedFieldDomain->fieldType(), QVariant::Int );
  QCOMPARE( codedFieldDomain->values().size(), 2 );
  QCOMPARE( codedFieldDomain->values().at( 0 ).code(), QVariant( 1 ) );
  QCOMPARE( codedFieldDomain->values().at( 0 ).value(), QStringLiteral( "val1" ) );
  QCOMPARE( codedFieldDomain->values().at( 1 ).code(), QVariant( 2 ) );
  QCOMPARE( codedFieldDomain->values().at( 1 ).value(), QStringLiteral( "val2" ) );

  OGR_FldDomain_SetSplitPolicy( domain, OFDSP_DEFAULT_VALUE );
  OGR_FldDomain_SetMergePolicy( domain, OFDMP_DEFAULT_VALUE );
  res = QgsOgrUtils::convertFieldDomain( domain );

  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::DefaultValue );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::DefaultValue );

  OGR_FldDomain_SetSplitPolicy( domain, OFDSP_DUPLICATE );
  OGR_FldDomain_SetMergePolicy( domain, OFDMP_SUM );
  res = QgsOgrUtils::convertFieldDomain( domain );

  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::Duplicate );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::Sum );

  OGR_FldDomain_SetSplitPolicy( domain, OFDSP_GEOMETRY_RATIO );
  OGR_FldDomain_SetMergePolicy( domain, OFDMP_GEOMETRY_WEIGHTED );
  res = QgsOgrUtils::convertFieldDomain( domain );

  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::GeometryRatio );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::GeometryWeighted );

  OGR_FldDomain_Destroy( domain );

  OGRField min;
  min.Integer = 5;
  OGRField max;
  max.Integer = 15;
  domain = OGR_RangeFldDomain_Create( "name", "desc", OFTInteger, OFSTNone, &min, true, &max, false );
  res = QgsOgrUtils::convertFieldDomain( domain );
  QgsRangeFieldDomain *rangeDomain = dynamic_cast< QgsRangeFieldDomain *>( res.get() );
  QVERIFY( rangeDomain );
  QCOMPARE( rangeDomain->name(), QStringLiteral( "name" ) );
  QCOMPARE( rangeDomain->description(), QStringLiteral( "desc" ) );
  QCOMPARE( rangeDomain->fieldType(), QVariant::Int );
  QCOMPARE( rangeDomain->minimum(), QVariant( 5 ) );
  QCOMPARE( rangeDomain->maximum(), QVariant( 15 ) );
  QVERIFY( rangeDomain->minimumIsInclusive() );
  QVERIFY( !rangeDomain->maximumIsInclusive() );
  OGR_FldDomain_Destroy( domain );
  domain = OGR_RangeFldDomain_Create( "name", "desc", OFTInteger, OFSTNone, &min, false, &max, true );
  res = QgsOgrUtils::convertFieldDomain( domain );
  rangeDomain = dynamic_cast< QgsRangeFieldDomain *>( res.get() );
  QVERIFY( !rangeDomain->minimumIsInclusive() );
  QVERIFY( rangeDomain->maximumIsInclusive() );
  OGR_FldDomain_Destroy( domain );

  domain = OGR_GlobFldDomain_Create( "name", "desc", OFTString, OFSTNone, "*a*" );
  res = QgsOgrUtils::convertFieldDomain( domain );
  QgsGlobFieldDomain *globDomain = dynamic_cast< QgsGlobFieldDomain *>( res.get() );
  QVERIFY( globDomain );
  QCOMPARE( globDomain->name(), QStringLiteral( "name" ) );
  QCOMPARE( globDomain->description(), QStringLiteral( "desc" ) );
  QCOMPARE( globDomain->fieldType(), QVariant::String );
  OGR_FldDomain_Destroy( domain );
}

void TestQgsOgrUtils::testConvertToFieldDomain()
{
  // test converting QgsFieldDomain to OGR field domain
  QgsGlobFieldDomain globDomain( QStringLiteral( "name" ), QStringLiteral( "desc" ), QVariant::String, QStringLiteral( "*a*" ) );
  OGRFieldDomainH domain = QgsOgrUtils::convertFieldDomain( &globDomain );

  std::unique_ptr< QgsFieldDomain > res = QgsOgrUtils::convertFieldDomain( domain );
  QCOMPARE( res->name(), QStringLiteral( "name" ) );
  QCOMPARE( res->description(), QStringLiteral( "desc" ) );
  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::DefaultValue );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::DefaultValue );
  QCOMPARE( dynamic_cast< QgsGlobFieldDomain * >( res.get() )->glob(), QStringLiteral( "*a*" ) );
  OGR_FldDomain_Destroy( domain );

  globDomain.setSplitPolicy( Qgis::FieldDomainSplitPolicy::Duplicate );
  globDomain.setMergePolicy( Qgis::FieldDomainMergePolicy::Sum );
  domain = QgsOgrUtils::convertFieldDomain( &globDomain );
  res = QgsOgrUtils::convertFieldDomain( domain );
  OGR_FldDomain_Destroy( domain );
  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::Duplicate );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::Sum );

  globDomain.setSplitPolicy( Qgis::FieldDomainSplitPolicy::GeometryRatio );
  globDomain.setMergePolicy( Qgis::FieldDomainMergePolicy::GeometryWeighted );
  domain = QgsOgrUtils::convertFieldDomain( &globDomain );
  res = QgsOgrUtils::convertFieldDomain( domain );
  OGR_FldDomain_Destroy( domain );
  QCOMPARE( res->splitPolicy(), Qgis::FieldDomainSplitPolicy::GeometryRatio );
  QCOMPARE( res->mergePolicy(), Qgis::FieldDomainMergePolicy::GeometryWeighted );

  // range

  QgsRangeFieldDomain rangeDomain( QStringLiteral( "name" ), QStringLiteral( "desc" ), QVariant::Int,
                                   1, true, 5, false );
  domain = QgsOgrUtils::convertFieldDomain( &rangeDomain );
  res = QgsOgrUtils::convertFieldDomain( domain );
  OGR_FldDomain_Destroy( domain );
  QCOMPARE( res->name(), QStringLiteral( "name" ) );
  QCOMPARE( res->description(), QStringLiteral( "desc" ) );
  QCOMPARE( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->minimum(), QVariant( 1 ) );
  QVERIFY( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->minimumIsInclusive() );
  QCOMPARE( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->maximum(), QVariant( 5 ) );
  QVERIFY( !dynamic_cast< QgsRangeFieldDomain * >( res.get() )->maximumIsInclusive() );

  rangeDomain.setFieldType( QVariant::Double );
  rangeDomain.setMinimum( 5.5 );
  rangeDomain.setMaximum( 12.1 );
  rangeDomain.setMinimumIsInclusive( false );
  rangeDomain.setMaximumIsInclusive( true );
  domain = QgsOgrUtils::convertFieldDomain( &rangeDomain );
  res = QgsOgrUtils::convertFieldDomain( domain );
  OGR_FldDomain_Destroy( domain );
  QCOMPARE( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->minimum(), QVariant( 5.5 ) );
  QVERIFY( !dynamic_cast< QgsRangeFieldDomain * >( res.get() )->minimumIsInclusive() );
  QCOMPARE( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->maximum(), QVariant( 12.1 ) );
  QVERIFY( dynamic_cast< QgsRangeFieldDomain * >( res.get() )->maximumIsInclusive() );

  // coded
  QgsCodedFieldDomain codedDomain( QStringLiteral( "name" ), QStringLiteral( "desc" ), QVariant::String,
  {
    QgsCodedValue( "aa", "aaaa" ),
    QgsCodedValue( "bb", "bbbb" ),
  } );
  domain = QgsOgrUtils::convertFieldDomain( &codedDomain );
  res = QgsOgrUtils::convertFieldDomain( domain );
  OGR_FldDomain_Destroy( domain );
  QCOMPARE( res->name(), QStringLiteral( "name" ) );
  QCOMPARE( res->description(), QStringLiteral( "desc" ) );
  QList< QgsCodedValue > resValues = dynamic_cast< QgsCodedFieldDomain * >( res.get() )->values();
  QCOMPARE( resValues.size(), 2 );
  QCOMPARE( resValues.at( 0 ).code(), QVariant( "aa" ) );
  QCOMPARE( resValues.at( 0 ).value(), QStringLiteral( "aaaa" ) );
  QCOMPARE( resValues.at( 1 ).code(), QVariant( "bb" ) );
  QCOMPARE( resValues.at( 1 ).value(), QStringLiteral( "bbbb" ) );

}
#endif

QGSTEST_MAIN( TestQgsOgrUtils )
#include "testqgsogrutils.moc"
