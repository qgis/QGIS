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

    void testV2ExportAttributes_data()
    {
      QTest::addColumn<bool>( "useQJson" );
      QTest::newRow( "Use V2 (QJson)" ) << true;
      QTest::newRow( "Use old string concat" ) << false;
    }

    void testV2ExportAttributes()
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

    void testV2ExportFeature_data()
    {
      QTest::addColumn<bool>( "useQJson" );
      QTest::newRow( "Use V2 (QJson)" ) << true;
      QTest::newRow( "Use old string concat" ) << false;
    }

    void testV2ExportFeature()
    {

      QFETCH( bool, useQJson );

      QgsVectorLayer vl { QStringLiteral( "Polygon?field=fldtxt:string&field=fldint:integer&field=flddbl:double" ), QStringLiteral( "mem" ), QStringLiteral( "memory" ) };
      QgsFeature feature { vl.fields() };
      feature.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "POLYGON((1.12 1.34,5.45 1.12,5.34 5.33,1.56 5.2,1.12 1.34),(2 2, 3 2, 3 3, 2 3,2 2))" ) ) );
      feature.setAttributes( QgsAttributes() << QStringLiteral( "a value" ) << 1 << 2.0 );

      QgsJsonExporter exporter { &vl };

      if ( useQJson )  // average: 0.038 msecs per iteration
      {
        QBENCHMARK
        {
          const auto json { exporter.exportFeatureToJsonObject( feature ) };
          QCOMPARE( QJsonDocument( json ).toJson( QJsonDocument::JsonFormat::Compact ),
                    QStringLiteral( "{\"bbox\":[1.12,1.12,5.45,5.33],\"geometry\":{\"coordinates\":[[[1.12,1.34]"
                                    ",[5.45,1.12],[5.34,5.33],[1.56,5.2],[1.12,1.34]],[[2,2],[3,2],[3,3],[2,3],[2,2]]],"
                                    "\"type\":\"Point\"},\"id\":0,\"properties\":{\"flddbl\":2,\"fldint\":1,\"fldtxt\":\"a value\"},\"type\":\"Feature\"}"
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
};

QGSTEST_MAIN( TestQgsJsonUtils )
#include "testqgsjsonutils.moc"
