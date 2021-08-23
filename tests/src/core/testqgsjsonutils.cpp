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
    void testParseJson();
    void testIntList();
    void testDoubleList();
    void testExportAttributesJson_data();
    void testExportAttributesJson();
    void testExportFeatureJson();
    void testExportGeomToJson();
};



void TestQgsJsonUtils::testStringList()
{
  QStringList list;

  {
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[]" ) );
    const QVariant back = QgsJsonUtils::parseArray( json, QVariant::String );
    QCOMPARE( back.toStringList(), list );
  }

  {
    list << QStringLiteral( "one" ) << QStringLiteral( "<',\"\\>" ) << QStringLiteral( "two" );
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[\"one\",\"<',\\\"\\\\>\",\"two\"]" ) );
    const QVariant back = QgsJsonUtils::parseArray( json, QVariant::String );
    QCOMPARE( back.toStringList(), list );
  }
}

void TestQgsJsonUtils::testJsonArray()
{
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])", QVariant::Int ), QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])" ), QVariantList() << 1 << 2 << 3 );
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,2,3])", QVariant::Double ), QVariantList() << 1.0 << 2.0 << 3.0 );
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
  QCOMPARE( QgsJsonUtils::parseArray( R"([1,"a",2.0])", QVariant::Int ), QVariantList() << 1 << 2.0 );
  // Try invalid JSON
  QCOMPARE( QgsJsonUtils::parseArray( R"(not valid json here)" ), QVariantList() );
  QCOMPARE( QgsJsonUtils::parseArray( R"(not valid json here)", QVariant::Int ), QVariantList() );
  // Empty
  QCOMPARE( QgsJsonUtils::parseArray( R"([])", QVariant::Int ), QVariantList() );
  QCOMPARE( QgsJsonUtils::parseArray( "", QVariant::Int ), QVariantList() );
  // Booleans
  QCOMPARE( QgsJsonUtils::parseArray( "[true,false]", QVariant::Bool ), QVariantList() << true << false );
  // Nulls
  for ( const QVariant &value : QgsJsonUtils::parseArray( R"([null, null])" ) )
  {
    QVERIFY( value.isNull() );
    QVERIFY( value.isValid() );
    QCOMPARE( value, QVariant( QVariant::Type::Int ) );
  }
  for ( const QVariant &value : QgsJsonUtils::parseArray( R"([null, null])", QVariant::Type::Double ) )
  {
    QVERIFY( value.isNull() );
    QVERIFY( value.isValid() );
    QCOMPARE( value, QVariant( QVariant::Type::Double ) );
  }
}

void TestQgsJsonUtils::testParseJson()
{
  const QStringList tests {{
      "null",
      "false",
      "true",
      "123",
      "123.45",
      R"j("a string")j",
      "[1,2,3.4,null]",
      R"j({"_bool":true,"_double":1234.45,"_int":123,"_list":[1,2,3.4,null],"_null":null,"_object":{"int":123}})j",
    }};

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
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QStringList()
                                    << QStringLiteral( "A string" )
                                    << QStringLiteral( "Another string" ) ).dump() ),
            QString( R"raw(["A string","Another string"])raw" ) );
  QCOMPARE( QString::fromStdString( QgsJsonUtils::jsonFromVariant( QStringList()
                                    << QStringLiteral( "A string" ) ).dump() ),
            QString( R"raw(["A string"])raw" ) );

}

void TestQgsJsonUtils::testIntList()
{
  QVariantList list;

  {
    list << 1 << -2;
    const QString json = QgsJsonUtils::encodeValue( list );
    QCOMPARE( json, QString( "[1,-2]" ) );
    const QVariantList back = QgsJsonUtils::parseArray( json, QVariant::Int );
    QCOMPARE( back, list );
    QCOMPARE( back.at( 0 ).type(), QVariant::Int );
  }

  {
    // check invalid entries are ignored
    const QVariantList back = QgsJsonUtils::parseArray( QStringLiteral( "[1,\"a\",-2]" ), QVariant::Int );
    QCOMPARE( back, list );
  }
}

void TestQgsJsonUtils::testDoubleList()
{
  QVariantList list;

  list << 1.0 << -2.2456;
  const QString json = QgsJsonUtils::encodeValue( list );
  QCOMPARE( json, QString( "[1,-2.2456]" ) );
  const QVariantList back = QgsJsonUtils::parseArray( json, QVariant::Double );
  QCOMPARE( back, list );
  QCOMPARE( back.at( 0 ).type(), QVariant::Double );
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

  if ( jsonAlg == JsonAlgs::Json )  // 0.0022
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
  QCOMPARE( QString::fromStdString( j.dump() ),  expectedJson );
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
  QCOMPARE( QString::fromStdString( jPrecision.dump() ),  expectedJsonPrecision );
  const auto jsonPrecision { exporterPrecision.exportFeature( feature ) };
  QCOMPARE( jsonPrecision, expectedJsonPrecision );

}

void TestQgsJsonUtils::testExportGeomToJson()
{
  const QMap<QString, QString> testWkts
  {
    {
      {
        QStringLiteral( "LINESTRING(-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932)" ),
        QStringLiteral( R"json({"coordinates":[[-71.16,42.259],[-71.161,42.259],[-71.161,42.259]],"type":"LineString"})json" )
      },
      {
        QStringLiteral( "MULTILINESTRING((-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932), (-70 43.56, -67 44.68))" ),
        QStringLiteral( R"json({"coordinates":[[[-71.16,42.259],[-71.161,42.259],[-71.161,42.259]],[[-70.0,43.56],[-67.0,44.68]]],"type":"MultiLineString"})json" )
      },
      { QStringLiteral( "POINT(-71.064544 42.28787)" ), QStringLiteral( R"json({"coordinates":[-71.065,42.288],"type":"Point"})json" ) },
      { QStringLiteral( "MULTIPOINT(-71.064544 42.28787, -71.1776585052917 42.3902909739571)" ), QStringLiteral( R"json({"coordinates":[[-71.065,42.288],[-71.178,42.39]],"type":"MultiPoint"})json" ) },
      {
        QStringLiteral( "POLYGON((-71.1776585052917 42.3902909739571,-71.1776820268866 42.3903701743239,"
                        "-71.1776063012595 42.3903825660754,-71.1775826583081 42.3903033653531,-71.1776585052917 42.3902909739571))" ),
        QStringLiteral( R"json({"coordinates":[[[-71.178,42.39],[-71.178,42.39],[-71.178,42.39],[-71.178,42.39],[-71.178,42.39]]],"type":"Polygon"})json" )
      },
      {
        QStringLiteral( "MULTIPOLYGON(((1 1,5 1,5 5,1 5,1 1),(2 2, 3 2, 3 3, 2 3,2 2)),((3 3,6 2,6 4,3 3)))" ),
        QStringLiteral( R"json({"coordinates":[[[[1.0,1.0],[5.0,1.0],[5.0,5.0],[1.0,5.0],[1.0,1.0]],[[2.0,2.0],[3.0,2.0],[3.0,3.0],[2.0,3.0],[2.0,2.0]]],[[[3.0,3.0],[6.0,2.0],[6.0,4.0],[3.0,3.0]]]],"type":"MultiPolygon"})json" )
      },
      // Note: CIRCULARSTRING json is very long, we will check first three vertices only
      { QStringLiteral( "CIRCULARSTRING(220268 150415,220227 150505,220227 150406)" ), QStringLiteral( R"json({"coordinates":[[220268.0,150415.0],[220268.7,150415.535],[220269.391,150416.081])json" ) },
    }
  };

  for ( const auto &w : testWkts.toStdMap() )
  {
    const auto g { QgsGeometry::fromWkt( w.first ) };
    QVERIFY( !g.isNull( ) );
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

QGSTEST_MAIN( TestQgsJsonUtils )
#include "testqgsjsonutils.moc"
