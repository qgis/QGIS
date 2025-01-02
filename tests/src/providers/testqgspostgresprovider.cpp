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
#include "qgsconfig.h" // for ENABLE_PGTEST
#include <QObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include <qgspostgresprovider.h>
#include <qgspostgresconn.h>
#include <qgsfields.h>

class TestQgsPostgresProvider : public QObject
{
    Q_OBJECT

  private:
#ifdef ENABLE_PGTEST
    QgsPostgresConn *_connection;


    QgsPostgresConn *getConnection()
    {
      if ( !_connection )
      {
        const char *connstring = getenv( "QGIS_PGTEST_DB" );
        if ( !connstring )
          connstring = "service=qgis_test";
        _connection = QgsPostgresConn::connectDb( connstring, true );
      }
      return _connection;
    }
#endif

  private slots:

    void initTestCase() // will be called before the first testfunction is executed.
    {
#ifdef ENABLE_PGTEST
      this->_connection = 0;
#endif
    }
    void cleanupTestCase() // will be called after the last testfunction was executed.
    {
#ifdef ENABLE_PGTEST
      if ( this->_connection )
        this->_connection->unref();
#endif
    }

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
#ifdef ENABLE_PGTEST
    void testEwktInOut();
#endif
};


void TestQgsPostgresProvider::decodeHstore()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "\"1\"=>\"2\", \"a\"=>\"b, \\\"c'\", \"backslash\"=>\"\\\\\"" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantMap );

  QVariantMap expected;
  expected[QStringLiteral( "1" )] = "2";
  expected[QStringLiteral( "a" )] = "b, \"c'";
  expected[QStringLiteral( "backslash" )] = "\\";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeHstoreNoQuote()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "1=>2, a=>b c" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantMap );

  QVariantMap expected;
  expected[QStringLiteral( "1" )] = "2";
  expected[QStringLiteral( "a" )] = "b c";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeArray2StringList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QStringList, QMetaType::Type::QString, QStringLiteral( "{\"1\",\"2\", \"a\\\\1\" , \"\\\\\",\"b, \\\"c'\"}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QStringList );

  QStringList expected;
  expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a\\1" ) << QStringLiteral( "\\" ) << QStringLiteral( "b, \"c'" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toStringList(), expected );
}

void TestQgsPostgresProvider::decodeArray2StringListNoQuote()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QStringList, QMetaType::Type::QString, QStringLiteral( "{1,2, a ,b, c}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QStringList );

  QStringList expected;
  expected << QStringLiteral( "1" ) << QStringLiteral( "2" ) << QStringLiteral( "a" ) << QStringLiteral( "b" ) << QStringLiteral( "c" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toStringList(), expected );
}

void TestQgsPostgresProvider::decodeArray2IntList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QStringList, QMetaType::Type::QString, QStringLiteral( "{1, 2, 3,-5,10}" ), QStringLiteral( "hstore" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QStringList );

  QVariantList expected;
  expected << QVariant( 1 ) << QVariant( 2 ) << QVariant( 3 ) << QVariant( -5 ) << QVariant( 10 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decode2DimensionArray()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QStringList, QMetaType::Type::QString, QStringLiteral( "{{foo,\"escape bracket \\}\"},{\"escape bracket and backslash \\\\\\}\",\"hello bar\"}}" ), QStringLiteral( "_text" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QStringList );

  QVariantList expected;
  expected << QVariant( "{foo,\"escape bracket \\}\"}" ) << QVariant( "{\"escape bracket and backslash \\\\\\}\",\"hello bar\"}" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decode3DimensionArray()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QStringList, QMetaType::Type::QString, QStringLiteral( "{{{0,1},{1,2}},{{3,4},{5,6}}}" ), QStringLiteral( "_integer" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QStringList );

  QVariantList expected;
  expected << QVariant( "{{0,1},{1,2}}" ) << QVariant( "{{3,4},{5,6}}" );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "[1,2,3]" ), QStringLiteral( "json" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantList );

  QVariantList expected;
  expected.append( 1 );
  expected.append( 2 );
  expected.append( 3 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonbList()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "[1,2,3]" ), QStringLiteral( "jsonb" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantList );

  QVariantList expected;
  expected.append( 1 );
  expected.append( 2 );
  expected.append( 3 );
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toList(), expected );
}

void TestQgsPostgresProvider::decodeJsonMap()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "json" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantMap );

  QVariantMap expected;
  expected[QStringLiteral( "a" )] = "1";
  expected[QStringLiteral( "b" )] = "2";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::decodeJsonbMap()
{
  const QVariant decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QVariantMap, QMetaType::Type::QString, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "jsonb" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QVariantMap );

  QVariantMap expected;
  expected[QStringLiteral( "a" )] = "1";
  expected[QStringLiteral( "b" )] = "2";
  qDebug() << "actual: " << decoded;
  QCOMPARE( decoded.toMap(), expected );
}

void TestQgsPostgresProvider::testDecodeDateTimes()
{
  QVariant decoded;

  decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QDateTime, QMetaType::Type::UnknownType, QStringLiteral( "2020-06-08 18:30:35.496438+02" ), QStringLiteral( "timestamptz" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QDateTime );

  decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QTime, QMetaType::Type::UnknownType, QStringLiteral( "18:29:27.569401+02" ), QStringLiteral( "timetz" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QTime );

  decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QDate, QMetaType::Type::UnknownType, QStringLiteral( "2020-06-08" ), QStringLiteral( "date" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QDate );

  decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QDateTime, QMetaType::Type::UnknownType, QStringLiteral( "2020-06-08 18:30:35.496438" ), QStringLiteral( "timestamp" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QDateTime );

  decoded = QgsPostgresProvider::convertValue( QMetaType::Type::QTime, QMetaType::Type::UnknownType, QStringLiteral( "18:29:27.569401" ), QStringLiteral( "time" ), nullptr );
  QCOMPARE( static_cast<QMetaType::Type>( decoded.userType() ), QMetaType::Type::QTime );
}

void TestQgsPostgresProvider::testQuotedValueBigInt()
{
  QgsFields fields;
  QList<int> pkAttrs;
  QVariantList vlst;

  const std::shared_ptr<QgsPostgresSharedData> sdata( new QgsPostgresSharedData() );

  QgsField f0, f1, f2, f3;

  // 4 byte integer
  f0.setName( "fld_integer" );
  f0.setType( QMetaType::Type::Int );
  f0.setTypeName( "int4" );

  fields.append( f0 );
  pkAttrs.append( 0 );
  vlst.append( 42 );

  // for positive integers, fid  == the value, there is no map.
  sdata->insertFid( 42, vlst );

  QCOMPARE( QgsPostgresUtils::whereClause( 42, fields, NULL, QgsPostgresPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QString( "\"fld_integer\"=42" ) );

  // 8 byte integer
  f1.setName( "fld_bigint" );
  f1.setType( QMetaType::Type::LongLong );
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
  f2.setType( QMetaType::Type::Double );
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
  f3.setType( QMetaType::Type::QString );
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

  const std::shared_ptr<QgsPostgresSharedData> sdata( new QgsPostgresSharedData() );

  QgsField f0, f1, f2, f3;

  // need regular expression to check IN/OR because QgsFeatureIds is a set and ids could come
  // in various order

#define CHECK_IN_CLAUSE( whereClause, expectedValues )                 \
  {                                                                    \
    QRegularExpression inRe( "\\\"fld\\\" IN \\(([^,]*),([^,]*)\\)" ); \
    QVERIFY( inRe.isValid() );                                         \
    QRegularExpressionMatch match = inRe.match( whereClause );         \
    QVERIFY( match.hasMatch() );                                       \
    QStringList values;                                                \
    values << match.captured( 1 );                                     \
    values << match.captured( 2 );                                     \
    std::sort( values.begin(), values.end() );                         \
    QCOMPARE( values, expectedValues );                                \
  }

  // QRegularExpression inOr("\\(\\\"fld\\\"=([^,]*) OR \\\"fld\\\"=([^,]*)\\)");

#define CHECK_OR_CLAUSE( whereClause, expectedValues )         \
  {                                                            \
    QRegularExpression inOr( "\\((.*) OR (.*)\\)" );           \
    QVERIFY( inOr.isValid() );                                 \
    QRegularExpressionMatch match = inOr.match( whereClause ); \
    QVERIFY( match.hasMatch() );                               \
    QStringList values;                                        \
    values << match.captured( 1 );                             \
    values << match.captured( 2 );                             \
    std::sort( values.begin(), values.end() );                 \
    QCOMPARE( values, expectedValues );                        \
  }

  // 4 byte integer -> IN clause
  f0.setName( "fld" );
  f0.setType( QMetaType::Type::Int );
  f0.setTypeName( "int4" );

  // for positive integers, fid  == the value, there is no map.
  fields.append( f0 );
  pkAttrs.append( 0 );

  sdata->insertFid( 42, QVariantList() << 42 );
  sdata->insertFid( 43, QVariantList() << 43 );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 42 << 43, fields, NULL, QgsPostgresPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QStringList() << "42" << "43" );

  // 8 byte integer -> IN clause
  f1.setName( "fld" );
  f1.setType( QMetaType::Type::LongLong );
  f1.setTypeName( "int8" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f1 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << -9223372036854775800LL ); // way outside int4 range
  sdata->insertFid( 2LL, QVariantList() << -9223372036854775801LL );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktInt64, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QStringList() << "-9223372036854775800" << "-9223372036854775801" );

  // double -> OR clause
  f2.setName( "fld" );
  f2.setType( QMetaType::Type::Double );
  f2.setTypeName( "float8" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f2 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << 3.141592741 );
  sdata->insertFid( 2LL, QVariantList() << 6.141592741 );

  CHECK_OR_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QStringList() << "\"fld\"='3.141592741'" << "\"fld\"='6.141592741'" );

  // text -> IN clause
  f3.setName( "fld" );
  f3.setType( QMetaType::Type::QString );
  f3.setTypeName( "text" );

  fields.clear();
  pkAttrs.clear();

  fields.append( f3 );
  pkAttrs.append( 0 );

  sdata->clear();
  sdata->insertFid( 1LL, QVariantList() << QString( "QGIS 'Rocks'!" ) );
  sdata->insertFid( 2LL, QVariantList() << QString( "PostGIS too!" ) );

  CHECK_IN_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QStringList() << "'PostGIS too!'" << "'QGIS ''Rocks''!'" );

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

  CHECK_OR_CLAUSE( QgsPostgresUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( sdata ) ), QStringList() << "\"fld_int\"=42 AND \"fld\"::text='QGIS ''Rocks''!'"
                                                                                                                                                                                                               << "\"fld_int\"=43 AND \"fld\"::text='PostGIS too!'" );
}

#ifdef ENABLE_PGTEST
void TestQgsPostgresProvider::testEwktInOut()
{
  QGSTEST_NEED_PGTEST_DB();

  QgsPostgresConn *conn = getConnection();
  QVERIFY( conn != nullptr );
  QgsReferencedGeometry g;
  QString ewkt_obtained;

  g = QgsPostgresProvider::fromEwkt( "SRID=4326;LINESTRING(0 0,-5 2)", conn );
  QVERIFY( !g.isNull() );
  QCOMPARE( g.crs().authid(), "EPSG:4326" );
  ewkt_obtained = QgsPostgresProvider::toEwkt( g, conn );
  QCOMPARE( ewkt_obtained, "SRID=4326;LineString (0 0, -5 2)" );

  // Test for srid-less geometry
  // See https://github.com/qgis/QGIS/issues/49380#issuecomment-1282913470
  g = QgsPostgresProvider::fromEwkt( "POINT(0 0)", conn );
  QVERIFY( !g.isNull() );
  ewkt_obtained = QgsPostgresProvider::toEwkt( g, conn );
  QVERIFY( !g.crs().isValid() ); // is unknown
  QCOMPARE( ewkt_obtained, QString( "SRID=0;Point (0 0)" ) );
}
#endif // ENABLE_PGTEST

QGSTEST_MAIN( TestQgsPostgresProvider )
#include "testqgspostgresprovider.moc"
