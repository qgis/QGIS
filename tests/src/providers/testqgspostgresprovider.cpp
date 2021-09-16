/***************************************************************************
    testqgspostgresprovider.cpp
    ---------------------
    begin                : August 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
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
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <qgspostgresprovider.h>
#include <qgspostgresconn.h>
#include <qgsfields.h>


class TestQgsPostgresProvider: public QObject
{
    Q_OBJECT
  private slots:

    void decodeHstore();
    void decodeHstoreNoQuote();
    void decodeArray2StringList();
    void decodeArray2StringListNoQuote();
    void decodeArray2IntList();
    void decode2DimensionArray();
    void decode3DimensionArray();
    void decodeJsonList();
    void decodeJsonbList();
    void decodeJsonMap();
    void decodeJsonbMap();
    void testDecodeDateTimes();
    void testQuotedValueBigInt();
    void testWhereClauseFids();
};



void TestQgsPostgresProvider::decodeHstore()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "\"1\"=>\"2\", \"a\"=>\"b, \\\"c'\", \"backslash\"=>\"\\\\\"" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Map );

  QVariantMap expected;
  expected[QStringLiteral( "1" )] = "2";
  expected[QStringLiteral( "a" )] = "b, \"c'";
  expected[QStringLiteral( "backslash" )] = "\\";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeHstoreNoQuote()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "1=>2, a=>b c" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Map );

  QVariantMap expected;
  expected[QStringLiteral( "1" )] = "2";
  expected[QStringLiteral( "a" )] = "b c";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeArray2StringList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{\"1\",\"2\", \"a\\\\1\" , \"\\\\\",\"b, \\\"c'\"}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::StringList );

  QStringList expected;
  expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a\\1" ) << QStringLiteral( "\\" ) << QStringLiteral( "b, \"c'" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toStringList(), expected );
}

void TestQgsPostgresProvider::decodeArray2StringListNoQuote()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1,2, a ,b, c}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::StringList );

  QStringList expected;
  expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toStringList(), expected );
}

void TestQgsPostgresProvider::decodeArray2IntList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{1, 2, 3,-5,10}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::StringList );

  QVariantList expected;
  expected << QVariant( 1 ) << QVariant( 2 ) << QVariant( 3 ) << QVariant( -5 ) << QVariant( 10 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decode2DimensionArray()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{{foo,\"escape bracket \\}\"},{\"escape bracket and backslash \\\\\\}\",\"hello bar\"}}" ), QStringLiteral( "_text" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::StringList );

  QVariantList expected;
  expected << QVariant( "{foo,\"escape bracket \\}\"}" ) << QVariant( "{\"escape bracket and backslash \\\\\\}\",\"hello bar\"}" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decode3DimensionArray()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::StringList, QVariant::String, QStringLiteral( "{{{0,1},{1,2}},{{3,4},{5,6}}}" ), QStringLiteral( "_integer" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::StringList );

  QVariantList expected;
  expected << QVariant( "{{0,1},{1,2}}" ) << QVariant( "{{3,4},{5,6}}" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "[1,2,3]" ), QStringLiteral( "json" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::List );

  QVariantList expected;
  expected.append( 1 );
  expected.append( 2 );
  expected.append( 3 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonbList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "[1,2,3]" ), QStringLiteral( "jsonb" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::List );

  QVariantList expected;
  expected.append( 1 );
  expected.append( 2 );
  expected.append( 3 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonMap()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "json" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Map );

  QVariantMap expected;
  expected[QStringLiteral( "a" )] = "1";
  expected[QStringLiteral( "b" )] = "2";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeJsonbMap()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "jsonb" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Map );

  QVariantMap expected;
  expected[QStringLiteral( "a" )] = "1";
  expected[QStringLiteral( "b" )] = "2";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::testDecodeDateTimes()
{

  QVariant decoded;

  decoded = QgsPostgresProvider::convertValue( QVariant::DateTime, QVariant::Invalid, QStringLiteral( "2020-06-08 18:30:35.496438+02" ), QStringLiteral( "timestamptz" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::DateTime );

  decoded = QgsPostgresProvider::convertValue( QVariant::Time, QVariant::Invalid, QStringLiteral( "18:29:27.569401+02" ), QStringLiteral( "timetz" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Time );

  decoded = QgsPostgresProvider::convertValue( QVariant::Date, QVariant::Invalid, QStringLiteral( "2020-06-08" ), QStringLiteral( "date" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Date );

  decoded = QgsPostgresProvider::convertValue( QVariant::DateTime, QVariant::Invalid, QStringLiteral( "2020-06-08 18:30:35.496438" ), QStringLiteral( "timestamp" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::DateTime );

  decoded = QgsPostgresProvider::convertValue( QVariant::Time, QVariant::Invalid, QStringLiteral( "18:29:27.569401" ), QStringLiteral( "time" ), nullptr );
  QCOMPARE( decoded.type(), QVariant::Time );

}

void TestQgsPostgresProvider::testQuotedValueBigInt()
{
  QgsFields fields;
  QList<int> pkAttrs;
  QVariantList vlst;

  const std::shared_ptr< QgsPostgresSharedData > sdata( new QgsPostgresSharedData() );

  QgsField f0, f1, f2, f3;

  // 4 byte integer
  f0.setName( "fld_integer" );
  f0.setType( QVariant::Int );
  f0.setTypeName( "int4" );

  fields.append( f0 );
  pkAttrs.append( 0 );
  vlst.append( 42 );

  // for positive integers, fid  == the value, there is no map.
  sdata->insertFid( 42, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 42, fields, NULL, QgsPostgresPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_integer\"=42" ) );

  // 8 byte integer
  f1.setName( "fld_bigint" );
  f1.setType( QVariant::LongLong );
  f1.setTypeName( "int8" );

  fields.clear();
  pkAttrs.clear();
  vlst.clear();

  fields.append( f1 );
  pkAttrs.append( 0 );
  vlst.append( -9223372036854775800LL ); // way outside int4 range

  sdata->clear();
  sdata->insertFid( 1LL, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 1LL, fields, NULL, QgsPostgresPrimaryKeyType::PktInt64, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_bigint\"=-9223372036854775800" ) );

  // double
  f2.setName( "fld_double" );
  f2.setType( QVariant::Double );
  f2.setTypeName( "float8" );

  fields.clear();
  pkAttrs.clear();
  vlst.clear();

  fields.append( f2 );
  pkAttrs.append( 0 );
  vlst.append( 3.141592741 );

  sdata->clear();
  sdata->insertFid( 1LL, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 1LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_double\"='3.141592741'" ) );

  // text
  f3.setName( "fld_text" );
  f3.setType( QVariant::String );
  f3.setTypeName( "text" );

  fields.clear();
  pkAttrs.clear();
  vlst.clear();

  fields.append( f3 );
  pkAttrs.append( 0 );
  vlst.append( QString( "QGIS 'Rocks'!" ) );

  sdata->clear();
  sdata->insertFid( 1LL, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 1LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_text\"::text='QGIS ''Rocks''!'" ) );

  // Composite bigint + text + int
  pkAttrs.clear();

  pkAttrs.append( 0 );
  pkAttrs.append( 1 );
  pkAttrs.append( 2 );

  vlst.clear();
  vlst.append( -9223372036854775800LL );
  vlst.append( QString( "QGIS 'Rocks'!" ) );
  vlst.append( 42 );

  fields.clear();
  fields.append( f1 );
  fields.append( f3 );
  fields.append( f0 );

  sdata->clear();
  sdata->insertFid( 1LL, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 1LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_bigint\"=-9223372036854775800 AND \"fld_text\"::text='QGIS ''Rocks''!' AND \"fld_integer\"=42" ) );
}

void TestQgsPostgresProvider::testWhereClauseFids()
{
  // test the returned where clause according to given feature ids and primary key

  QgsFields fields;
  QList<int> pkAttrs;

  const std::shared_ptr< QgsPostgresSharedData > sdata( new QgsPostgresSharedData() );

  QgsField f0, f1, f2, f3;

  // need regular expression to check IN/OR because QgsFeatureIds is a set and ids could come
  // in various order

#define CHECK_IN_CLAUSE(whereClause,expectedValues)                     \
  {                                                                     \
    QRegularExpression inRe("\\\"fld\\\" IN \\(([^,]*),([^,]*)\\)");    \
    QVERIFY(inRe.isValid());                                            \
    QRegularExpressionMatch match = inRe.match( whereClause );          \
    QVERIFY( match.hasMatch() );                                        \
    QStringList values;                                                 \
    values << match.captured(1);                                        \
    values << match.captured(2);                                        \
    std::sort( values.begin(), values.end() );                          \
    QCOMPARE( values, expectedValues );                                 \
  }

  // QRegularExpression inOr("\\(\\\"fld\\\"=([^,]*) OR \\\"fld\\\"=([^,]*)\\)");

#define CHECK_OR_CLAUSE(whereClause,expectedValues)                     \
  {                                                                     \
    QRegularExpression inOr("\\((.*) OR (.*)\\)");                      \
    QVERIFY(inOr.isValid());                                            \
    QRegularExpressionMatch match = inOr.match( whereClause );          \
    QVERIFY( match.hasMatch() );                                        \
    QStringList values;                                                 \
    values << match.captured(1);                                        \
    values << match.captured(2);                                        \
    std::sort( values.begin(), values.end() );                          \
    QCOMPARE( values, expectedValues );                                 \
  }

  // 4 byte integer -> IN clause
  f0.setName( "fld" );
  f0.setType( QVariant::Int );
  f0.setTypeName( "int4" );

  // for positive integers, fid  == the value, there is no map.
  fields.append( f0 );
  pkAttrs.append( 0 );

  sdata->insertFid( 42, QVariantList() << 42 );
  sdata->insertFid( 43, QVariantList() << 43 );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 42 << 43, fields, NULL, QgsPostgresPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ),
                   QStringList() << "42" << "43" );

  // 8 byte integer -> IN clause
  f1.setName( "fld" );
  f1.setType( QVariant::LongLong );
  f1.setTypeName( "int8" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f1 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << -9223372036854775800LL ); // way outside int4 range
  sdata->insertFid( 2LL, QVariantList() << -9223372036854775801LL );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktInt64, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ),
                   QStringList() << "-9223372036854775800" << "-9223372036854775801" );

  // double -> OR clause
  f2.setName( "fld" );
  f2.setType( QVariant::Double );
  f2.setTypeName( "float8" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f2 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << 3.141592741 );
  sdata->insertFid( 2LL, QVariantList() << 6.141592741 );

  CHECK_OR_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ),
                   QStringList() << "\"fld\"='3.141592741'" << "\"fld\"='6.141592741'" );

  // text -> IN clause
  f3.setName( "fld" );
  f3.setType( QVariant::String );
  f3.setTypeName( "text" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f3 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << QString( "QGIS 'Rocks'!" ) );
  sdata->insertFid( 2LL, QVariantList() << QString( "PostGIS too!" ) );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ),
                   QStringList() << "'PostGIS too!'" << "'QGIS ''Rocks''!'" );

  // Composite text + int -> OR clause
  f0.setName( "fld_int" );
  pkAttrs.clear();
  pkAttrs.append( 0 );
  pkAttrs.append( 1 );

  fields.clear();
  fields.append( f0 );
  fields.append( f3 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << 42 << QString( "QGIS 'Rocks'!" ) );
  sdata->insertFid( 2LL, QVariantList() << 43 << QString( "PostGIS too!" ) );

  CHECK_OR_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ),
                   QStringList() << "\"fld_int\"=42 AND \"fld\"::text='QGIS ''Rocks''!'"
                   << "\"fld_int\"=43 AND \"fld\"::text='PostGIS too!'" );
}

QGSTEST_MAIN( TestQgsPostgresProvider )
#include "testqgspostgresprovider.moc"
