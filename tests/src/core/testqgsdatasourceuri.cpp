/***************************************************************************
     testqgsdatasourceuri.cpp
     --------------------------------------
    Date                 : Thu Apr 16 2015
    Copyright            : (C) 2015 by Sandro Mani
    Email                : manisandro@gmail.com
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
//header for class being tested
#include <qgsdatasourceuri.h>

Q_DECLARE_METATYPE( QgsWkbTypes::Type )
Q_DECLARE_METATYPE( QgsDataSourceUri::SslMode )

class TestQgsDataSourceUri: public QObject
{
    Q_OBJECT
  private slots:
    void checkparser();
    void checkparser_data();
    void checkAuthParams();
};

void TestQgsDataSourceUri::checkparser_data()
{
  QTest::addColumn<QString>( "uri" );
  QTest::addColumn<QString>( "table" );
  QTest::addColumn<QString>( "geometrycolumn" );
  QTest::addColumn<QString>( "key" );
  QTest::addColumn<bool>( "estimatedmetadata" );
  QTest::addColumn<QString>( "srid" );
  QTest::addColumn<QgsWkbTypes::Type>( "type" );
  QTest::addColumn<bool>( "selectatid" );
  QTest::addColumn<QString>( "service" );
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn<QString>( "dbname" );
  QTest::addColumn<QString>( "host" );
  QTest::addColumn<QString>( "port" );
  QTest::addColumn<QString>( "driver" );
  QTest::addColumn<QgsDataSourceUri::SslMode>( "sslmode" );
  QTest::addColumn<QString>( "sql" );
  QTest::addColumn<QString>( "myparam" );
  QTest::addColumn<QString>( "schema" );


  QTest::newRow( "oci" )
      << "host=myhost port=1234 user='myname' password='mypasswd' estimatedmetadata=true srid=1000003007 table=\"myschema\".\"mytable\" (GEOM) myparam='myvalue' sql="
      << "mytable" // table
      << "GEOM" // geometrycolumn
      << "" // key
      << true // estimatedmetadata
      << "1000003007" // srid
      << QgsWkbTypes::Unknown // type
      << false // selectatid
      << "" // service
      << "myname" // user
      << "mypasswd" // password
      << "" // dbname
      << "myhost" // host
      << "1234" // port
      << "" // driver
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // sql
      << "myvalue" // myparam
      << "myschema"
      ;

  QTest::newRow( "pgrast" )
      << R"(PG: dbname='qgis_tests' host=localhost port=5432 user='myname' sslmode=disable estimatedmetadata=true srid=3067 table="public"."basic_map_tiled" (rast))"
      << "basic_map_tiled" // table
      << "rast" // geometrycolumn
      << "" // key
      << true // estimatedmetadata
      << "3067" // srid
      << QgsWkbTypes::Unknown // type
      << false // selectatid
      << "" // service
      << "myname" // user
      << "" // password
      << "qgis_tests" // dbname
      << "localhost" // host
      << "5432" // port
      << "" // driver
      << QgsDataSourceUri::SslDisable // sslmode
      << "" // sql
      << "" // myparam
      << "public" // schema
      ;

  QTest::newRow( "pg_notable" )
      << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=myschema "
      << "" // table
      << "" // geometrycolumn
      << "" // key
      << false // estimatedmetadata
      << "" // srid
      << QgsWkbTypes::Unknown // type
      << false // selectatid
      << "" // service
      << "myname" // user
      << "mypasswd" // password
      << "mydb" // dbname
      << "myhost" // host
      << "5432" // port
      << "" // driver
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // sql
      << "" // myparam
      << "public" // schema
      ;

  QTest::newRow( "pg_notable_quoted" )
      << "dbname='mydb' host='myhost' user='myname' password='mypasswd' port='5432' mode='2' schema=myschema"
      << "" // table
      << "" // geometrycolumn
      << "" // key
      << false // estimatedmetadata
      << "" // srid
      << QgsWkbTypes::Unknown // type
      << false // selectatid
      << "" // service
      << "myname" // user
      << "mypasswd" // password
      << "mydb" // dbname
      << "myhost" // host
      << "5432" // port
      << "" // driver
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // sql
      << "" // myparam
      << "public" // schema
      ;



  QTest::newRow( "pgmlsz" )
      << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=public column=geom table=mytable type=MultiLineStringZ"
      << "mytable" // table
      << "" // geometrycolumn
      << "" // key
      << false // estimatedmetadata
      << "" // srid
      << QgsWkbTypes::MultiLineStringZ // type
      << false // selectatid
      << "" // service
      << "myname" // user
      << "mypasswd" // password
      << "mydb" // dbname
      << "myhost" // host
      << "5432" // port
      << "" // driver
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // sql
      << "" // myparam
      << "public" // schema
      ;
}

void TestQgsDataSourceUri::checkparser()
{
  QFETCH( QString, uri );
  QFETCH( QString, table );
  QFETCH( QString, geometrycolumn );
  QFETCH( QString, key );
  QFETCH( bool, estimatedmetadata );
  QFETCH( QString, srid );
  QFETCH( QgsWkbTypes::Type, type );
  QFETCH( bool, selectatid );
  QFETCH( QString, service );
  QFETCH( QString, user );
  QFETCH( QString, password );
  QFETCH( QString, dbname );
  QFETCH( QString, host );
  QFETCH( QString, port );
  QFETCH( QString, driver );
  QFETCH( QgsDataSourceUri::SslMode, sslmode );
  QFETCH( QString, sql );
  QFETCH( QString, myparam );
  QFETCH( QString, schema );

  const QgsDataSourceUri ds( uri );
  QCOMPARE( ds.table(), table );
  QCOMPARE( ds.geometryColumn(), geometrycolumn );
  QCOMPARE( ds.keyColumn(), key );
  QCOMPARE( ds.useEstimatedMetadata(), estimatedmetadata );
  QCOMPARE( ds.srid(), srid );
  QCOMPARE( ds.wkbType(), type );
  QCOMPARE( ds.selectAtIdDisabled(), selectatid );
  QCOMPARE( ds.service(), service );
  QCOMPARE( ds.username(), user );
  QCOMPARE( ds.password(), password );
  QCOMPARE( ds.database(), dbname );
  QCOMPARE( ds.host(), host );
  QCOMPARE( ds.port(), port );
  QCOMPARE( ds.driver(), driver );
  QCOMPARE( ds.sslMode(), sslmode );
  QCOMPARE( ds.sql(), sql );
  QCOMPARE( ds.param( "myparam" ), myparam );
}

void TestQgsDataSourceUri::checkAuthParams()
{
  // some providers rely on the QgsDataSourceUri params for storing and retrieving username, password and authentication.
  // Test here that the direct setters and getters for username/password/authcfg are compatible with providers which utilize the parameter system

  QgsDataSourceUri uri;
  QVERIFY( uri.param( QStringLiteral( "username" ) ).isEmpty() );
  QVERIFY( uri.param( QStringLiteral( "password" ) ).isEmpty() );
  QVERIFY( uri.param( QStringLiteral( "authcfg" ) ).isEmpty() );

  uri.setUsername( QStringLiteral( "kaladin" ) );
  uri.setPassword( QStringLiteral( "stormblessed" ) );
  uri.setAuthConfigId( QStringLiteral( "syl" ) );

  QCOMPARE( uri.param( QStringLiteral( "username" ) ), QStringLiteral( "kaladin" ) );
  QCOMPARE( uri.param( QStringLiteral( "password" ) ), QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri.param( QStringLiteral( "authcfg" ) ), QStringLiteral( "syl" ) );

  // round trip through encodedUri should not lose username/password/authcfg
  const QByteArray encoded = uri.encodedUri();
  QgsDataSourceUri uri2;
  uri2.setEncodedUri( encoded );

  QCOMPARE( uri2.param( QStringLiteral( "username" ) ), QStringLiteral( "kaladin" ) );
  QCOMPARE( uri2.username(), QStringLiteral( "kaladin" ) );
  QCOMPARE( uri2.param( QStringLiteral( "password" ) ), QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri2.password(), QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri2.param( QStringLiteral( "authcfg" ) ), QStringLiteral( "syl" ) );
  QCOMPARE( uri2.authConfigId(), QStringLiteral( "syl" ) );

  QgsDataSourceUri uri3;
  uri3.setParam( QStringLiteral( "username" ), QStringLiteral( "kaladin" ) );
  uri3.setParam( QStringLiteral( "password" ), QStringLiteral( "stormblessed" ) );
  uri3.setParam( QStringLiteral( "authcfg" ), QStringLiteral( "syl" ) );
  QCOMPARE( uri3.param( QStringLiteral( "username" ) ), QStringLiteral( "kaladin" ) );
  QCOMPARE( uri3.params( QStringLiteral( "username" ) ), QStringList() << QStringLiteral( "kaladin" ) );
  QCOMPARE( uri3.username(), QStringLiteral( "kaladin" ) );
  QCOMPARE( uri3.param( QStringLiteral( "password" ) ), QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri3.params( QStringLiteral( "password" ) ), QStringList() << QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri3.password(), QStringLiteral( "stormblessed" ) );
  QCOMPARE( uri3.param( QStringLiteral( "authcfg" ) ), QStringLiteral( "syl" ) );
  QCOMPARE( uri3.params( QStringLiteral( "authcfg" ) ), QStringList() << QStringLiteral( "syl" ) );
  QCOMPARE( uri3.authConfigId(), QStringLiteral( "syl" ) );

  QVERIFY( uri.hasParam( QStringLiteral( "username" ) ) );
  uri.removeParam( QStringLiteral( "username" ) );
  QVERIFY( !uri.hasParam( QStringLiteral( "username" ) ) );
  QVERIFY( uri.param( QStringLiteral( "username" ) ).isEmpty() );
  QVERIFY( uri.username().isEmpty() );
  QVERIFY( uri.hasParam( QStringLiteral( "password" ) ) );
  uri.removeParam( QStringLiteral( "password" ) );
  QVERIFY( !uri.hasParam( QStringLiteral( "password" ) ) );
  QVERIFY( uri.param( QStringLiteral( "password" ) ).isEmpty() );
  QVERIFY( uri.password().isEmpty() );
  QVERIFY( uri.hasParam( QStringLiteral( "authcfg" ) ) );
  uri.removeParam( QStringLiteral( "authcfg" ) );
  QVERIFY( !uri.hasParam( QStringLiteral( "authcfg" ) ) );
  QVERIFY( uri.param( QStringLiteral( "authcfg" ) ).isEmpty() );
  QVERIFY( uri.authConfigId().isEmpty() );

  // issue GH #39243
  QgsDataSourceUri uri4;
  uri4.setEncodedUri( QStringLiteral( "dpiMode=7&url=http://localhost:8000/ows/?MAP%3D/home/bug.qgs&username=username&password=pa%25%25word" ) );

  QCOMPARE( uri4.param( QStringLiteral( "username" ) ), QStringLiteral( "username" ) );
  QCOMPARE( uri4.username(), QStringLiteral( "username" ) );
  QCOMPARE( uri4.param( QStringLiteral( "password" ) ), QStringLiteral( "pa%%word" ) );
  QCOMPARE( uri4.password(), QStringLiteral( "pa%%word" ) );

  // issue GH #42405
  uri4.setEncodedUri( QStringLiteral( "dpiMode=7&url=http://localhost:8000/ows/?MAP%3D/home/bug.qgs&username=username&password=qgis%C3%A8%C3%A9" ) );
  QCOMPARE( uri4.param( QStringLiteral( "username" ) ), QStringLiteral( "username" ) );
  QCOMPARE( uri4.username(), QStringLiteral( "username" ) );
  QCOMPARE( uri4.param( QStringLiteral( "password" ) ), QStringLiteral( "qgisèé" ) );
  QCOMPARE( uri4.password(), QStringLiteral( "qgisèé" ) );

  uri4.setEncodedUri( QStringLiteral( "dpiMode=7&url=http://localhost:8000/&username=username&password=%1" ).arg( QString( QUrl::toPercentEncoding( QStringLiteral( "😁😂😍" ) ) ) ) );
  QCOMPARE( uri4.param( QStringLiteral( "username" ) ), QStringLiteral( "username" ) );
  QCOMPARE( uri4.username(), QStringLiteral( "username" ) );
  QCOMPARE( uri4.param( QStringLiteral( "password" ) ), QStringLiteral( "😁😂😍" ) );
  QCOMPARE( uri4.password(), QStringLiteral( "😁😂😍" ) );

}


QGSTEST_MAIN( TestQgsDataSourceUri )
#include "testqgsdatasourceuri.moc"
