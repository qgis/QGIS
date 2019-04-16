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
#include "qgsvectorlayer.h"
#include "qgsfeature.h"


class TestQgsJsonUtils : public QObject
{
    Q_OBJECT
  private slots:
    void testStringList()
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

    void testIntList()
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

    void testDoubleList()
    {
      QVariantList list;

      list << 1.0 << -2.2456;
      const QString json = QgsJsonUtils::encodeValue( list );
      QCOMPARE( json, QString( "[1,-2.2456]" ) );
      const QVariantList back = QgsJsonUtils::parseArray( json, QVariant::Double );
      QCOMPARE( back, list );
      QCOMPARE( back.at( 0 ).type(), QVariant::Double );
    }

    void testExportAttributesQJson_data()
    {
      QTest::addColumn<bool>( "useQJson" );
      QTest::newRow( "Use V2 (QJson)" ) << true;
      QTest::newRow( "Use old string concat" ) << false;
    }

    void testExportAttributesQJson()
    {

      QFETCH( bool, useQJson );

      QgsVectorLayer vl { QStringLiteral( "Point?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
      QgsFeature feature { vl.fields() };
      feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

      if ( useQJson )  // average: 0.0048 msecs per iteration
      {
        QBENCHMARK
        {
          const auto json { QgsJsonUtils::exportAttributesToJsonObject( feature, &vl ) };
          QCOMPARE( QJsonDocument( json ).toJson( QJsonDocument::JsonFormat::Compact ), QStringLiteral( "{\"flddbl\":2,\"fldint\":1,\"fldtxt\":\"a value\"}" ) );
        }
      }
      else // average: 0.0070 msecs per iteration
      {
        QBENCHMARK
        {
          const auto json { QgsJsonUtils::exportAttributes( feature, &vl ) };
          QCOMPARE( json, QStringLiteral( "{\"fldtxt\":\"a value\",\n\"fldint\":1,\n\"flddbl\":2}" ) );
        }
      }
    }

    void testExportFeatureQJson_data()
    {
      QTest::addColumn<bool>( "useQJson" );
      QTest::newRow( "Use V2 (QJson)" ) << true;
      QTest::newRow( "Use old string concat" ) << false;
    }

    void testExportFeatureQJson()
    {

      QFETCH( bool, useQJson );

      QgsVectorLayer vl { QStringLiteral( "Polygon?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
      QgsFeature feature { vl.fields() };
      feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((1.12 1.34,5.45 1.12,5.34 5.33,1.56 5.2,1.12 1.34),(2 2, 3 2, 3 3, 2 3,2 2))" ) ) );
      feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

      QgsJsonExporter exporter { &vl };

      if ( useQJson )  // average: 0.041 msecs per iteration
      {
        QBENCHMARK
        {
          const auto json { exporter.exportFeatureToJsonObject( feature ) };
          QCOMPARE( QJsonDocument( json ).toJson( QJsonDocument::JsonFormat::Compact ),
                    QStringLiteral( "{\"bbox\":[1.12,1.12,5.45,5.33],\"geometry\":{\"coordinates\":[[[1.12,1.34]"
                                    ",[5.45,1.12],[5.34,5.33],[1.56,5.2],[1.12,1.34]],[[2,2],[3,2],[3,3],[2,3],[2,2]]],"
                                    "\"type\":\"Polygon\"},\"id\":0,\"properties\":{\"flddbl\":2,\"fldint\":1,\"fldtxt\":\"a value\"},\"type\":\"Feature\"}"
                                  ) );
        }
      }
      else // average: 0.047 msecs per iteration
      {
        QBENCHMARK
        {
          const auto json { exporter.exportFeature( feature ) };
          QCOMPARE( json, QStringLiteral( "{\n   \"type\":\"Feature\",\n   \"id\":0,\n   \"bbox\":[1.12, 1.12, 5.45, 5.33],\n   \"geometry\":\n   "
                                          "{\"type\": \"Polygon\", \"coordinates\": [[ [1.12, 1.34], [5.45, 1.12], [5.34, 5.33], [1.56, 5.2], [1.12, 1.34]], "
                                          "[ [2, 2], [3, 2], [3, 3], [2, 3], [2, 2]]] },\n   "
                                          "\"properties\":{\n      \"fldtxt\":\"a value\",\n      \"fldint\":1,\n      \"flddbl\":2\n   }\n}" ) );
        }
      }
    }

    void testExportGeomToQJson()
    {
      const QStringList testWkts
      {
        {
          QStringLiteral( "LINESTRING(-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932)" ),
          QStringLiteral( "MULTILINESTRING((-71.160281 42.258729,-71.160837 42.259113,-71.161144 42.25932))" ),
          QStringLiteral( "POINT(-71.064544 42.28787)" ),
          QStringLiteral( "MULTIPOINT(-71.064544 42.28787, -71.1776585052917 42.3902909739571)" ),
          QStringLiteral( "POLYGON((-71.1776585052917 42.3902909739571,-71.1776820268866 42.3903701743239,"
                          "-71.1776063012595 42.3903825660754,-71.1775826583081 42.3903033653531,-71.1776585052917 42.3902909739571))" ),
          QStringLiteral( "MULTIPOLYGON(((1 1,5 1,5 5,1 5,1 1),(2 2, 3 2, 3 3, 2 3,2 2)),((3 3,6 2,6 4,3 3)))" ),
          QStringLiteral( "CIRCULARSTRING(220268 150415,220227 150505,220227 150406)" ),
        }
      };

      for ( const auto &w : testWkts )
      {
        const auto g { QgsGeometry::fromWkt( w ) };
        QVERIFY( !g.isNull( ) );
        QCOMPARE( QJsonDocument( g.asJsonObject( 3 ) ).toJson( QJsonDocument::JsonFormat::Compact ),
                  QJsonDocument::fromJson( g.asJson( 3 ).toUtf8() ).toJson( QJsonDocument::JsonFormat::Compact ) );
      }
    }
};

QGSTEST_MAIN( TestQgsJsonUtils )
#include "testqgsjsonutils.moc"
