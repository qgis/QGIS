/***************************************************************************
    testqgspostgresconn.cpp
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

#include <qgspostgresconn.h>
#include <qgsfields.h>
#include <qgspostgresprovider.h>

// Helper function for QCOMPARE
char *toString( const QgsPostgresGeometryColumnType &t )
{
  const char *ptr;
  switch ( t )
  {
    case SctNone:
      ptr = "None";
      break;
    case SctGeometry:
      ptr = "Geometry";
      break;
    case SctGeography:
      ptr = "Geography";
      break;
    case SctTopoGeometry:
      ptr = "TopoGeometry";
      break;
    case SctPcPatch:
      ptr = "PcPatch";
      break;
    case SctRaster:
      ptr = "Raster";
      break;
    default:
      ptr = "Unknown";
      break;
  }
  char *dst = new char[16];
  return qstrcpy( dst, ptr );
}

class TestQgsPostgresConn: public QObject
{
    Q_OBJECT

  private:
    QgsPostgresConn *_connection;

    QgsPostgresConn *getConnection()
    {
      if ( ! _connection )
      {
        const char *connstring = getenv( "QGIS_PGTEST_DB" );
        if ( NULL == connstring ) connstring = "service=qgis_test";
        _connection = QgsPostgresConn::connectDb( connstring, true );
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
      if ( this->_connection ) this->_connection->unref();
    }

    void quotedValueHstore()
    {
      QVariantMap map;
      map[QStringLiteral( "1" )] = "2";
      map[QStringLiteral( "a" )] = "b \"c' \\x";

      const QString actual = QgsPostgresConn::quotedValue( map );
      QCOMPARE( actual, QString( "E'\"1\"=>\"2\",\"a\"=>\"b \\\\\"c\\' \\\\\\\\x\"'::hstore" ) );
    }

    void quotedValueString()
    {
      QCOMPARE( QgsPostgresConn::quotedValue( "b" ), QString( "'b'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b's" ), QString( "'b''s'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b \"c' \\x" ), QString( "E'b \"c'' \\\\x'" ) );
    }

    void quotedValueDatetime()
    {
      QCOMPARE( QgsPostgresConn::quotedValue( QDateTime::fromString( "2020-05-07 17:56:00", "yyyy-MM-dd HH:mm:ss" ) ), QString( "'2020-05-07T17:56:00.000'" ) );

      QgsFields fields;
      QgsField f;
      f.setName( "ts" );
      f.setType( QVariant::DateTime );
      f.setTypeName( "timestamp" );
      fields.append( f );
      QgsField f2;
      f2.setName( "pk" );
      f2.setType( QVariant::LongLong );
      f2.setTypeName( "serial8" );
      fields.append( f2 );

      QList<int> pkAttrs;
      pkAttrs.append( 0 );
      pkAttrs.append( 1 );

      QgsPostgresSharedData *shared = new QgsPostgresSharedData;
      QVariantList qvlist;
      qvlist.append( QDateTime::fromString( "2020-05-07 17:56:00", "yyyy-MM-dd HH:mm:ss" ) );
      qvlist.append( 123LL );
      shared->insertFid( 1LL, qvlist );

      QCOMPARE( QgsPostgresUtils::whereClause( 1LL, fields, NULL, QgsPostgresPrimaryKeyType::PktFidMap, pkAttrs, std::shared_ptr<QgsPostgresSharedData>( shared ) ), QString( "\"ts\"='2020-05-07T17:56:00.000' AND \"pk\"=123" ) );
    }

    void quotedValueStringArray()
    {
      QStringList list;
      list << QStringLiteral( "a" ) << QStringLiteral( "b \"c' \\x" );
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{\"a\",\"b \\\\\"c\\' \\\\\\\\x\"}'" ) );
    }

    void quotedValueIntArray()
    {
      QVariantList list;
      list << 1 << -5;
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{1,-5}'" ) );
    }

    void quotedValue2DimArray()
    {
      QStringList list;
      list << QStringLiteral( "{\"hello foo\",b}" ) << QStringLiteral( "{c,\"hello bar\"}" );
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{{\"hello foo\",b},{c,\"hello bar\"}}'" ) );
    }

    void supportedLayers()
    {
      QgsPostgresConn *conn = getConnection();
      QVERIFY( conn != 0 );
      QVector<QgsPostgresLayerProperty> layers;
      QMap<QString, QgsPostgresLayerProperty> layersMap;

      const bool success = conn->supportedLayers(
                             layers,
                             false, // searchGeometryColumnsOnly
                             false, // searchPublicOnly
                             false, // allowGeometrylessTables
                             "qgis_test" // schema
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
            ).arg( key ).toUtf8().data()
          );
        }
        layersMap.insert( key, l );
      }

      // Test qgis_test.TopoLayer1.topogeom
      const QString key = QString( "qgis_test.TopoLayer1.topogeom" );
      const auto lit = layersMap.find( key );
      QVERIFY2(
        lit != layersMap.end(),
        "Layer qgis_test.TopoLayer1.topogeom not returned by supportedLayers"
      );
      QCOMPARE( lit->geometryColName, "topogeom" );
      QCOMPARE( lit->geometryColType, SctTopoGeometry );
      // TODO: add more tests

    }

};

QGSTEST_MAIN( TestQgsPostgresConn )
#include "testqgspostgresconn.moc"
