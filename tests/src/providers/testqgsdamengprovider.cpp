/***************************************************************************
    testqgsdamengprovider.cpp
    ---------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
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

#include "qgsdamengexpressioncompiler.h"
#include "qgsdamengfeatureiterator.h"
#include <qgsdamengprovider.h>
#include <qgsdamengconn.h>
#include <qgsfields.h>

//The only purpose of this class is to set geomColumn and srid
class QgsTestDamengExpressionCompiler : public QgsDamengExpressionCompiler
{
  public:
    QgsTestDamengExpressionCompiler( QgsDamengFeatureSource *source, const QString &srid, const QString &geometryColumn )
      : QgsDamengExpressionCompiler( source )
    {
      mDetectedSrid = srid;
      mGeometryColumn = geometryColumn;
    }
};

class TestQgsDamengProvider : public QObject
{
    Q_OBJECT
  private:
    QgsDamengConn *_connection;

    QgsDamengConn *getConnection()
    {
      if ( !_connection )
      {
        const char *connstring = getenv( "QGIS_DMTEST_DB" );
        if ( NULL == connstring )
          connstring = "host=127.0.0.1 port=5236 user='SYSDBA' password='Test1234'";
        _connection = QgsDamengConn::connectDb( connstring, true );
        assert( _connection );
      }
      return _connection;
    }

  private slots:
    void initTestCase() // will be called before the first testfunction is executed.
    {
      this->_connection = 0;
    }
    void cleanupTestCase() // will be called after the last testfunction was executed.
    {
      if ( this->_connection )
        this->_connection->unref();
    }

    void quotedValueString()
    {
      QCOMPARE( QgsDamengConn::quotedValue( "b" ), QString( "'b'" ) );
      QCOMPARE( QgsDamengConn::quotedValue( "b's" ), QString( "'b''s'" ) );
      QCOMPARE( QgsDamengConn::quotedValue( "b \"c' \\x" ), QString( "E'b \"c'' \\\\x'" ) );
    }

    void quotedValueDatetime()
    {
      QCOMPARE( QgsDamengConn::quotedValue( QDateTime::fromString( "2020-05-07 17:56:00", "yyyy-MM-dd HH:mm:ss" ) ), QString( "'2020-05-07T17:56:00.000'" ) );

      QgsFields fields;
      QgsField f;
      f.setName( "ts" );
      f.setType( QVariant::DateTime );
      f.setTypeName( "timestamp" );
      fields.append( f );
      QgsField f2;
      f2.setName( "pk" );
      f2.setType( QVariant::LongLong );
      f2.setTypeName( "bigint" );
      fields.append( f2 );

      QList<int> pkAttrs;
      pkAttrs.append( 0 );
      pkAttrs.append( 1 );

      QgsDamengSharedData *shared = new QgsDamengSharedData;
      QVariantList qvlist;
      qvlist.append( QDateTime::fromString( "2020-05-07 17:56:00", "yyyy-MM-dd HH:mm:ss" ) );
      qvlist.append( 123LL );
      shared->insertFid( 1LL, qvlist );

      QCOMPARE( QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( shared ) ), QString( "\"ts\"='2020-05-07T17:56:00.000' AND \"pk\"=123" ) );
    }

    void supportedLayers()
    {
      QgsDamengConn *conn = getConnection();
      QVERIFY( conn != 0 );
      QVector<QgsDamengLayerProperty> layers;
      QMap<QString, QgsDamengLayerProperty> layersMap;

      const bool success = conn->supportedLayers(
        layers,
        false,   // searchSyedbaOnly
        false,   // allowGeometrylessTables
        "SYSDBA" // schema
      );
      QVERIFY( success );

      // Test no duplicates are reported by supportedLayers
      for ( const auto &l : layers )
      {
        const QString key = QString( "%1.%2.%3" ).arg( l.schemaName, l.tableName, l.geometryColName );
        const auto i = layersMap.find( key );
        if ( i != layersMap.end() )
        {
          QFAIL(
            QString(
              "Layer %1 returned multiple times by supportedLayers"
            )
              .arg( key )
              .toUtf8()
              .data()
          );
        }
        layersMap.insert( key, l );
      }

      // Test SYSDBA.TOPO_NORMALTABLE.tgcol
      const QString key = QString( "SYSDBA.TOPO_NORMALTABLE.tgcol" );
      const auto lit = layersMap.find( key );
      QVERIFY2(
        lit != layersMap.end(),
        "Layer SYSDBA.TOPO_NORMALTABLE.tgcol not returned by supportedLayers"
      );
      QCOMPARE( lit->geometryColName, "tgcol" );
      QCOMPARE( lit->geometryColType, SctTopoGeometry );
      // TODO: add more tests
    }

    //provider
    void decodeJsonMap()
    {
      const QVariant decoded = QgsDamengProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "json" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "a" )] = "1";
      expected[QStringLiteral( "b" )] = "2";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeJsonbMap()
    {
      const QVariant decoded = QgsDamengProvider::convertValue( QVariant::Map, QVariant::String, QStringLiteral( "{\"a\":1,\"b\":2}" ), QStringLiteral( "jsonb" ) );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected[QStringLiteral( "a" )] = "1";
      expected[QStringLiteral( "b" )] = "2";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void testDecodeDateTimes()
    {
      QVariant decoded;

      decoded = QgsDamengProvider::convertValue( QVariant::DateTime, QVariant::Invalid, QStringLiteral( "2020-06-08 18:30:35.496438+02" ), QStringLiteral( "TIMESTAMP WITH TIME ZONE" ) );
      QCOMPARE( decoded.type(), QVariant::DateTime );

      decoded = QgsDamengProvider::convertValue( QVariant::Time, QVariant::Invalid, QStringLiteral( "18:29:27.569401+02" ), QStringLiteral( "TIME WITH TIME ZONE" ) );
      QCOMPARE( decoded.type(), QVariant::Time );

      decoded = QgsDamengProvider::convertValue( QVariant::Date, QVariant::Invalid, QStringLiteral( "2020-06-08" ), QStringLiteral( "date" ) );
      QCOMPARE( decoded.type(), QVariant::Date );

      decoded = QgsDamengProvider::convertValue( QVariant::DateTime, QVariant::Invalid, QStringLiteral( "2020-06-08 18:30:35.496438" ), QStringLiteral( "timestamp" ) );
      QCOMPARE( decoded.type(), QVariant::DateTime );

      decoded = QgsDamengProvider::convertValue( QVariant::Time, QVariant::Invalid, QStringLiteral( "18:29:27.569401" ), QStringLiteral( "time" ) );
      QCOMPARE( decoded.type(), QVariant::Time );
    }

    void testQuotedValueBigInt()
    {
      QgsFields fields;
      QList<int> pkAttrs;
      QVariantList vlst;

      const std::shared_ptr< QgsDamengSharedData > sdata( new QgsDamengSharedData() );

      QgsField f0, f1, f2, f3;

      // 4 byte integer
      f0.setName( "fld_integer" );
      f0.setType( QVariant::Int );
      f0.setTypeName( "int" );

      fields.append( f0 );
      pkAttrs.append( 0 );
      vlst.append( 42 );

      // for positive integers, fid  == the value, there is no map.
      sdata->insertFid( 42, vlst );

      QCOMPARE( QgsDamengUtils::whereClause( 42, fields, NULL, QgsDamengPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QString( "\"fld_integer\"=42" ) );

      // 8 byte integer
      f1.setName( "fld_bigint" );
      f1.setType( QVariant::LongLong );
      f1.setTypeName( "bigint" );

      fields.clear();
      pkAttrs.clear();
      vlst.clear();

      fields.append( f1 );
      pkAttrs.append( 0 );
      vlst.append( -9223372036854775800LL ); // way outside bigint range

      sdata->clear();
      sdata->insertFid( 1LL, vlst );

      QCOMPARE( QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktInt64, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QString( "\"fld_bigint\"=-9223372036854775800" ) );

      // double
      f2.setName( "fld_double" );
      f2.setType( QVariant::Double );
      f2.setTypeName( "double" );

      fields.clear();
      pkAttrs.clear();
      vlst.clear();

      fields.append( f2 );
      pkAttrs.append( 0 );
      vlst.append( 3.141592741 );

      sdata->clear();
      sdata->insertFid( 1LL, vlst );

      QCOMPARE( QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QString( "\"fld_double\"='3.141592741'" ) );

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

      QString str = QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) );
      QCOMPARE( QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QString( "\"fld_text\"='QGIS ''Rocks''!'" ) );

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

      str = QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) );
      QCOMPARE( QgsDamengUtils::whereClause( 1LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QString( "\"fld_bigint\"=-9223372036854775800 AND \"fld_text\"='QGIS ''Rocks''!' AND \"fld_integer\"=42" ) );
    }

    void testWhereClauseFids()
    {
      // test the returned where clause according to given feature ids and primary key

      QgsFields fields;
      QList<int> pkAttrs;
      const QString clause;

      const std::shared_ptr< QgsDamengSharedData > sdata( new QgsDamengSharedData() );

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
      f0.setType( QVariant::Int );
      f0.setTypeName( "int" );

      // for positive integers, fid  == the value, there is no map.
      fields.append( f0 );
      pkAttrs.append( 0 );

      sdata->insertFid( 42, QVariantList() << 42 );
      sdata->insertFid( 43, QVariantList() << 43 );

      CHECK_IN_CLAUSE( QgsDamengUtils::whereClause( QgsFeatureIds() << 42 << 43, fields, NULL, QgsDamengPrimaryKeyType::PktInt, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QStringList() << "42" << "43" );

      // 8 byte integer -> IN clause
      f1.setName( "fld" );
      f1.setType( QVariant::LongLong );
      f1.setTypeName( "bigint" );

      fields.clear();
      pkAttrs.clear();

      fields.append( f1 );
      pkAttrs.append( 0 );

      sdata->clear();
      sdata->insertFid( 1LL, QVariantList() << -9223372036854775800LL ); // way outside bigint range
      sdata->insertFid( 2LL, QVariantList() << -9223372036854775801LL );

      CHECK_IN_CLAUSE( QgsDamengUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsDamengPrimaryKeyType::PktInt64, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QStringList() << "-9223372036854775800" << "-9223372036854775801" );

      // double -> OR clause
      f2.setName( "fld" );
      f2.setType( QVariant::Double );
      f2.setTypeName( "double" );

      fields.clear();
      pkAttrs.clear();

      fields.append( f2 );
      pkAttrs.append( 0 );

      sdata->clear();
      sdata->insertFid( 1LL, QVariantList() << 3.141592741 );
      sdata->insertFid( 2LL, QVariantList() << 6.141592741 );

      CHECK_OR_CLAUSE( QgsDamengUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QStringList() << "\"fld\"='3.141592741'" << "\"fld\"='6.141592741'" );

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
      sdata->insertFid( 2LL, QVariantList() << QString( "Dameng too!" ) );

      CHECK_IN_CLAUSE( QgsDamengUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QStringList() << "'Dameng too!'" << "'QGIS ''Rocks''!'" );

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
      sdata->insertFid( 2LL, QVariantList() << 43 << QString( "Dameng too!" ) );

      CHECK_OR_CLAUSE( QgsDamengUtils::whereClause( QgsFeatureIds() << 1LL << 2LL, fields, NULL, QgsDamengPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsDamengSharedData>( sdata ) ), QStringList() << "\"fld_int\"=42 AND \"fld\"='QGIS ''Rocks''!'"
                                                                                                                                                                                                             << "\"fld_int\"=43 AND \"fld\"='Dameng too!'" );
    }

    //ExpressionCompiler
    void testGeometryFromWkt()
    {
      const QgsDamengProvider p( QStringLiteral( "" ), QgsDataProvider::ProviderOptions() );
      QgsDamengFeatureSource featureSource( &p );
      QgsTestDamengExpressionCompiler compiler( &featureSource, QStringLiteral( "4326" ), QStringLiteral( "geom" ) );
      QgsExpression exp( QStringLiteral( "intersects($geometry,geom_from_wkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))" ) );
      const QgsExpressionContext expContext;
      exp.prepare( &expContext );
      const QgsSqlExpressionCompiler::Result r = compiler.compile( &exp );
      QCOMPARE( r, QgsSqlExpressionCompiler::Complete );
      const QString sql = compiler.result();
      QCOMPARE( sql, QStringLiteral( "DMGEO2.ST_Intersects(\"geom\",DMGEO2.ST_GeomFromText('Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))',4326))" ) );
    }
};


QGSTEST_MAIN( TestQgsDamengProvider )
#include "testqgsdamengprovider.moc"
