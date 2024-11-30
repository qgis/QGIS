/***************************************************************************
     testqgsjsonutils.cpp
     --------------------------------------
    Date                 : September 2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include <qgsjsonutils.h>
#include <nlohmann/json.hpp>

#include "qgsvectorlayer.h"
#include "qgsfeature.h"

class TestQgsJsonUtils : public QObject
{
  public:
    enum JsonAlgs
    {
      Json,
      String
    };
    Q_ENUM( JsonAlgs )

    Q_OBJECT

  private slots:
    void testStringList();
    void testJsonArray();
    void testJsonToVariant();
    void testParseJson();
    void testIntList();
    void testDoubleList();
    void testExportAttributesJson_data();
    void testExportAttributesJson();
    void testExportFeatureJson();
    void testExportFeatureJsonCrs();
    void testExportGeomToJson();
    void testParseNumbers();
    void testParseNumbers_data();
};


void TestQgsJsonUtils::testStringList()
{
  QStringList list;

  {
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[]" ) );
    const QVariant back = QgsJsonUtils::parseArray( json, QMetaType::Type::QString );
    QCOMPARE( back.toStringList(), list );
  }

  {
    list << QStringLiteral( "one" ) << QStringLiteral( "<',\"\\>" ) << QStringLiteral( "two" );
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[\"one\",\"<',\\\"\\\\>\",\"two\"]" ) );
    const QVariant back = QgsJsonUtils::parseArray( json, QMetaType::Type::QString );
    QCOMPARE( back.toStringList(), list );
  }
}

void TestQgsJsonUtils::testJsonArray()
{
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])", QMetaType::Type::Int ), QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])" ), QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])", QMetaType::Type::Double ), QVariantList() << 1.0 << 2.0 << 3.0 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1.0,2.0,3.0])" ), QVariantList() << 1.0 << 2.0 << 3.0 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1.234567,2.00003e+4,-3.01234e-02])" ), QVariantList() << 1.234567 << 2.00003e+4 << -3.01234e-2 );
  // Strings
  QCOMPARE( QgsJsonUtils::parseArray( R"(["one", "two", "three"])" ), QVariantList() << "one" << "two" << "three" );
  // VC++ doesn't like \" in raw strings
  QCOMPARE( QgsJsonUtils::parseArray( "[\"one,comma\", \"two[]brackets\", \"three\\\"escaped\"]" ), QVariantList() << "one,comma" << "two[]brackets" << "three\"escaped" );
  // Nested (not implemented: discard deeper levels)
  //QCOMPARE( QgsJsonUtils::parseArray( R"([1.0,[2.0,5.0],3.0])" ), QVariantList() << 1.0 << 3.0 );
  // Mixed types
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,"a",2.0])" ), QVariantList() << 1 << "a" << 2.0 );
  // discarded ...
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,"a",2.0])", QMetaType::Type::Int ), QVariantList() << 1 << 2.0 );
  // Try invalid JSON
  QCOMPARE( QgsJsonUtils::parseArray( R"(not valid json here)" ), QVariantList() );
  QCOMPARE( QgsJsonUtils::parseArray( R"(not valid json here)", QMetaType::Type::Int ), QVariantList() );
  // Empty
  QCOMPARE( QgsJsonUtils::parseArray( R"([])", QMetaType::Type::Int ), QVariantList() );
  QCOMPARE( QgsJsonUtils::parseArray( "", QMetaType::Type::Int ), QVariantList() );
  // Booleans
  QCOMPARE( QgsJsonUtils::parseArray( "[true,false]", QMetaType::Type::Bool ), QVariantList() << true << false );
  // Nulls
  for ( const QVariant &value : QgsJsonUtils::parseArray( R"([null, null])" ) )
  {
    QVERIFY( value.isNull() );
    QVERIFY( value.isValid() );
    QCOMPARE( value, QgsVariantUtils::createNullVariant( QMetaType::Type::Int ) );
  }
  for ( const QVariant &value : QgsJsonUtils::parseArray( R"([null, null])", QMetaType::Type::Double ) )
  {
    QVERIFY( value.isNull() );
    QVERIFY( value.isValid() );
    QCOMPARE( value, QVariant( QVariant::Type::Double ) );
  }
}

void TestQgsJsonUtils::testJsonToVariant()
{
  const json value = json::parse( "{\"_bool\":true,\"_double\":1234.45,\"_int\":123,\"_list\":[1,2,3.4,null],\"_null\":null,\"_object\":{\"int\":123}}" );
  const QVariant variant = QgsJsonUtils::jsonToVariant( value );
  QCOMPARE( static_cast<QMetaType::Type>( variant.userType() ), QMetaType::Type::QVariantMap );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_bool" ) ), true );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_double" ) ), 1234.45 );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_int" ) ), 123 );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_list" ) ), QVariantList( { 1, 2, 3.4, QVariant() } ) );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_null" ) ), QVariant() );
  QCOMPARE( variant.toMap().value( QStringLiteral( "_object" ) ), QVariantMap( { { QStringLiteral( "int" ), 123 } } ) );
}

void TestQgsJsonUtils::testParseJson()
{
  const QStringList tests { {
    "null",
    "false",
    "true",
    "123",
    "123.45",
    "4294967295",
    "-9223372036854775807",
    "9223372036854775807",
    R"j("a string")j",
    "[1,2,3.4,null]",
    R"j({"_bool":true,"_double":1234.45,"_int":123,"_list":[1,2,3.4,null],"_null":null,"_object":{"int":123}})j",
  } };

  for ( const auto &testJson : tests )
  {
    const auto parsed = QgsJsonUtils::parseJson( testJson );
    QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( parsed ).dump() ), testJson );
  }

  // Test empty string: null
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QgsJsonUtils::parseJson( QString() ) ).dump() ), QString( "null" ) );
  // invalid json -> null
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QgsJsonUtils::parseJson( QStringLiteral( "invalid json" ) ) ).dump() ), QString( "null" ) );
  // String lists
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QStringList() << QStringLiteral( "A string" ) << QStringLiteral( "Another string" ) ).dump() ), QString( R"raw(["A string","Another string"])raw" ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QStringList() << QStringLiteral( "A string" ) ).dump() ), QString( R"raw(["A string"])raw" ) );
}

void TestQgsJsonUtils::testIntList()
{
  QVariantList list;

  {
    list << 1 << -2;
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[1,-2]" ) );
    const QVariantList back = QgsJsonUtils::parseArray( json, QMetaType::Type::Int );
    QCOMPARE( back, list );
    QCOMPARE( static_cast<QMetaType::Type>( back.at( 0 ).userType() ), QMetaType::Type::Int );
  }

  {
    // check invalid entries are ignored
    const QVariantList back = QgsJsonUtils::parseArray( QStringLiteral( "[1,\"a\",-2]" ), QMetaType::Type::Int );
    QCOMPARE( back, list );
  }
}

void TestQgsJsonUtils::testDoubleList()
{
  QVariantList list;

  list << 1.0 << -2.2456;
  const QString json = QgsJsonUtils::encodeValue( list );
  QCOMPARE( json, QString( "[1,-2.2456]" ) );
  const QVariantList back = QgsJsonUtils::parseArray( json, QMetaType::Type::Double );
  QCOMPARE( back, list );
  QCOMPARE( static_cast<QMetaType::Type>( back.at( 0 ).userType() ), QMetaType::Type::Double );
}

void TestQgsJsonUtils::testExportAttributesJson_data()
{
  QTest::addColumn<JsonAlgs>( "jsonAlg" );
  QTest::newRow( "Use json" ) << JsonAlgs::Json;
  QTest::newRow( "Use old string concat" ) << JsonAlgs::String;
}

void TestQgsJsonUtils::testExportAttributesJson()
{
  QFETCH( enum JsonAlgs, jsonAlg );

  QgsVectorLayer vl { QStringLiteral( "Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
  QgsFeature feature { vl.fields() };
  feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

  if ( jsonAlg == JsonAlgs::Json ) // 0.0022
  {
    QBENCHMARK
    {
      const json j( QgsJsonUtils::exportAttributesToJsonObject( feature, &vl ) );
      QCOMPARE( QString::fromStdString( j.dump() ), QStringLiteral( R"raw({"flddbl":2.0,"fldint":1,"fldtxt":"a value"})raw" ) );
    }
  }
  else // 0.0032
  {
    QBENCHMARK
    {
      const auto json = QgsJsonUtils::exportAttributes( feature, &vl );
      QCOMPARE( json, QStringLiteral( "{\"fldtxt\":\"a value\",\n\"fldint\":1,\n\"flddbl\":2}" ) );
    }
  }
}

void TestQgsJsonUtils::testExportFeatureJson()
{
  QgsVectorLayer vl { QStringLiteral( "Polygon?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
  QgsFeature feature { vl.fields() };
  feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((1.12 1.34,5.45 1.12,5.34 5.33,1.56 5.2,1.12 1.34),(2 2, 3 2, 3 3, 2 3,2 2))" ) ) );
  feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

  const QgsJsonExporter exporter { &vl };

  const auto expectedJson { QStringLiteral( "{\"bbox\":[1.12,1.12,5.45,5.33],\"geometry\":{\"coordinates\":"
                                            "[[[1.12,1.34],[5.45,1.12],[5.34,5.33],[1.56,5.2],[1.12,1.34]],"
                                            "[[2.0,2.0],[3.0,2.0],[3.0,3.0],[2.0,3.0],[2.0,2.0]]],\"type\":\"Polygon\"}"
                                            ",\"id\":null,\"properties\":{\"flddbl\":2.0,\"fldint\":1,\"fldtxt\":\"a value\"}"
                                            ",\"type\":\"Feature\"}" ) };

  const auto j( exporter.exportFeatureToJsonObject( feature ) );
  QCOMPARE( QString::fromStdString( j.dump() ), expectedJson );
  const auto json = exporter.exportFeature( feature );
  QCOMPARE( json, expectedJson );

  const QgsJsonExporter exporterPrecision { &vl, 1 };


  const auto expectedJsonPrecision { QStringLiteral( "{\"bbox\":[1.1,1.1,5.5,5.3],\"geometry\":{\"coordinates\":"
                                                     "[[[1.1,1.3],[5.5,1.1],[5.3,5.3],[1.6,5.2],[1.1,1.3]],"
                                                     "[[2.0,2.0],[3.0,2.0],[3.0,3.0],[2.0,3.0],[2.0,2.0]]],\"type\":\"Polygon\"}"
                                                     ",\"id\":123,\"properties\":{\"flddbl\":2.0,\"fldint\":1,\"fldtxt\":\"a value\"}"
                                                     ",\"type\":\"Feature\"}" ) };

  feature.setId( 123 );
  const auto jPrecision( exporterPrecision.exportFeatureToJsonObject( feature ) );
  QCOMPARE( QString::fromStdString( jPrecision.dump() ), expectedJsonPrecision );
  const auto jsonPrecision { exporterPrecision.exportFeature( feature ) };
  QCOMPARE( jsonPrecision, expectedJsonPrecision );
}

void TestQgsJsonUtils::testExportFeatureJsonCrs()
{
  QgsVectorLayer vl { QStringLiteral( "Polygon?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
  QgsFeature feature { vl.fields() };
  feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((1.12 1.34,5.45 1.12,5.34 5.33,1.56 5.2,1.12 1.34),(2 2, 3 2, 3 3, 2 3,2 2))" ) ) );
  feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

  QgsJsonExporter exporterPrecision { &vl, 1 };
  exporterPrecision.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );


  const auto expectedJsonPrecision { QStringLiteral( "{\"bbox\":[124677.8,124685.8,606691.2,594190.5],\"geometry\":"
                                                     "{\"coordinates\":[[[124677.8,149181.7],[606691.2,124685.8],[594446.1,594190.5],[173658.4,579657.7],"
                                                     "[124677.8,149181.7]],[[222639.0,222684.2],[333958.5,222684.2],[333958.5,334111.2],[222639.0,334111.2],"
                                                     "[222639.0,222684.2]]],\"type\":\"Polygon\"},\"id\":123,\"properties\":{\"flddbl\":2.0,\"fldint\":1,"
                                                     "\"fldtxt\":\"a value\"},\"type\":\"Feature\"}" ) };

  feature.setId( 123 );
  const auto jPrecision( exporterPrecision.exportFeatureToJsonObject( feature ) );
  qDebug() << QString::fromStdString( jPrecision.dump() );
  QCOMPARE( QString::fromStdString( jPrecision.dump() ), expectedJsonPrecision );
  const auto jsonPrecision { exporterPrecision.exportFeature( feature ) };
  QCOMPARE( jsonPrecision, expectedJsonPrecision );
}

void TestQgsJsonUtils::testExportGeomToJson()
{
  const QMap<QString, QString> testWkts {
    {
      { QStringLiteral( "LINESTRING(-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932)" ),
        QStringLiteral( R"json({"coordinates":[[-71.16,42.259],[-71.161,42.259],[-71.161,42.259]],"type":"LineString"})json" )
      },
      { QStringLiteral( "MULTILINESTRING((-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932), (-70 43.56, -67 44.68))" ),
        QStringLiteral( R"json({"coordinates":[[[-71.16,42.259],[-71.161,42.259],[-71.161,42.259]],[[-70.0,43.56],[-67.0,44.68]]],"type":"MultiLineString"})json" )
      },
      { QStringLiteral( "POINT(-71.064544 42.28787)" ), QStringLiteral( R"json({"coordinates":[-71.065,42.288],"type":"Point"})json" ) },
      { QStringLiteral( "MULTIPOINT(-71.064544 42.28787, -71.1776585052917 42.3902909739571)" ), QStringLiteral( R"json({"coordinates":[[-71.065,42.288],[-71.178,42.39]],"type":"MultiPoint"})json" ) },
      { QStringLiteral( "POLYGON((-71.1776585052917 42.3902909739571,-71.1776820268866 42.3903701743239,"
                        "-71.1776063012595 42.3903825660754,-71.1775826583081 42.3903033653531,-71.1776585052917 42.3902909739571))" ),
        QStringLiteral( R"json({"coordinates":[[[-71.178,42.39],[-71.178,42.39],[-71.178,42.39],[-71.178,42.39],[-71.178,42.39]]],"type":"Polygon"})json" )
      },
      { QStringLiteral( "MULTIPOLYGON(((1 1,5 1,5 5,1 5,1 1),(2 2, 3 2, 3 3, 2 3,2 2)),((3 3,6 2,6 4,3 3)))" ),
        QStringLiteral( R"json({"coordinates":[[[[1.0,1.0],[5.0,1.0],[5.0,5.0],[1.0,5.0],[1.0,1.0]],[[2.0,2.0],[3.0,2.0],[3.0,3.0],[2.0,3.0],[2.0,2.0]]],[[[3.0,3.0],[6.0,2.0],[6.0,4.0],[3.0,3.0]]]],"type":"MultiPolygon"})json" )
      },
      // Note: CIRCULARSTRING json is very long, we will check first three vertices only
      { QStringLiteral( "CIRCULARSTRING(220268 150415,220227 150505,220227 150406)" ), QStringLiteral( R"json({"coordinates":[[220268.0,150415.0],[220268.7,150415.535],[220269.391,150416.081])json" ) },
    }
  };

  for ( const auto &w : testWkts.toStdMap() )
  {
    const auto g { QgsGeometry::fromWkt( w.first ) };
    QVERIFY( !g.isNull() );
    if ( w.first.startsWith( QLatin1String( "CIRCULARSTRING" ) ) )
    {
      QVERIFY( g.asJson( 3 ).startsWith( w.second ) );
      QCOMPARE( QString::fromStdString( g.asJsonObject( 3 )["type"].dump() ), QStringLiteral( R"("LineString")" ) );
    }
    else
    {
      QCOMPARE( g.asJson( 3 ), w.second );
    }
  }
}

void TestQgsJsonUtils::testParseNumbers()
{
  QFETCH( QString, number );
  QFETCH( int, type );

  qDebug() << number << QgsJsonUtils::parseJson( number ) << static_cast<QMetaType::Type>( QgsJsonUtils::parseJson( number ).userType() ) << type;
  QCOMPARE( static_cast<QMetaType::Type>( QgsJsonUtils::parseJson( number ).userType() ), type );
}

void TestQgsJsonUtils::testParseNumbers_data()
{
  QTest::addColumn<QString>( "number" );
  QTest::addColumn<int>( "type" );

  QTest::newRow( "zero" ) << "0" << static_cast<int>( QMetaType::Type::Int );
  QTest::newRow( "int max" ) << QString::number( std::numeric_limits<int>::max() ) << static_cast<int>( QMetaType::Type::Int );
  QTest::newRow( "int min" ) << QString::number( std::numeric_limits<int>::lowest() ) << static_cast<int>( QMetaType::Type::Int );
  QTest::newRow( "uint max" ) << QString::number( std::numeric_limits<uint>::max() ) << static_cast<int>( QMetaType::Type::LongLong );
  QTest::newRow( "ulong max" ) << QString::number( std::numeric_limits<qulonglong>::max() ) << static_cast<int>( QMetaType::Type::ULongLong );
  QTest::newRow( "longlong max" ) << QString::number( std::numeric_limits<qlonglong>::max() ) << static_cast<int>( QMetaType::Type::LongLong );
  QTest::newRow( "longlong min" ) << QString::number( std::numeric_limits<qlonglong>::lowest() ) << static_cast<int>( QMetaType::Type::LongLong );
}


QGSTEST_MAIN( TestQgsJsonUtils )
#include "testqgsjsonutils.moc"
