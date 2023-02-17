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
    void checkSetConnection();
    void checkSetConnection_data();
    void checkSetConnectionService();
    void checkSetConnectionService_data();
    void checkConnectionInfo();
    void checkConnectionInfo_data();
    void checkAuthParams();
    void checkParameterKeys();
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
  QTest::addColumn<QString>( "authcfg" );
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
      << "" // authcfg
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
      << "" // authcfg
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
      << "" // authcfg
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
      << "" // authcfg
      << "mydb" // dbname
      << "myhost" // host
      << "5432" // port
      << "" // driver
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // sql
      << "" // myparam
      << "public" // schema
      ;

  QTest::newRow( "pg_notable_authcfg" )
      << "PG: dbname=mydb host=myhost authcfg=myauthcfg port=5432 mode=2 schema=myschema "
      << "" // table
      << "" // geometrycolumn
      << "" // key
      << false // estimatedmetadata
      << "" // srid
      << QgsWkbTypes::Unknown // type
      << false // selectatid
      << "" // service
      << "" // user
      << "" // password
      << "myauthcfg" // authcfg
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
      << "" // authcfg
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
  QFETCH( QString, authcfg );
  QFETCH( QString, dbname );
  QFETCH( QString, host );
  QFETCH( QString, port );
  QFETCH( QString, driver );
  QFETCH( QgsDataSourceUri::SslMode, sslmode );
  QFETCH( QString, sql );
  QFETCH( QString, myparam );

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
  QCOMPARE( ds.authConfigId(), authcfg );
  QCOMPARE( ds.database(), dbname );
  QCOMPARE( ds.host(), host );
  QCOMPARE( ds.port(), port );
  QCOMPARE( ds.driver(), driver );
  QCOMPARE( ds.sslMode(), sslmode );
  QCOMPARE( ds.sql(), sql );
  QCOMPARE( ds.param( "myparam" ), myparam );
}

void TestQgsDataSourceUri::checkSetConnection_data()
{
  QTest::addColumn<QString>( "host" );
  QTest::addColumn<QString>( "port" );
  QTest::addColumn<QString>( "dbname" );
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn<QgsDataSourceUri::SslMode>( "sslmode" );
  QTest::addColumn<QString>( "authcfg" );

  QTest::newRow( "simple" )
      << "myhost" // host
      << "5432" // port
      << "mydb" // dbname
      << "myname" // user
      << "mypasswd" // password
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // authcfg
      ;

  QTest::newRow( "authcfg" )
      << "myhost" // host
      << "5432" // port
      << "" // dbname
      << "" // user
      << "mypasswd" // password
      << QgsDataSourceUri::SslPrefer // sslmode
      << "myauthcfg" // authcfg
      ;
}

void TestQgsDataSourceUri::checkSetConnection()
{
  QFETCH( QString, host );
  QFETCH( QString, port );
  QFETCH( QString, dbname );
  QFETCH( QString, user );
  QFETCH( QString, password );
  QFETCH( QgsDataSourceUri::SslMode, sslmode );
  QFETCH( QString, authcfg );

  QgsDataSourceUri uri;
  if ( authcfg.isEmpty() )
    uri.setConnection( host, port, dbname, user, password, sslmode );
  else
    uri.setConnection( host, port, dbname, user, password, sslmode, authcfg );

  QCOMPARE( uri.host(), host );
  QCOMPARE( uri.port(), port );
  QCOMPARE( uri.database(), dbname );
  QCOMPARE( uri.username(), user );
  QCOMPARE( uri.password(), password );
  QCOMPARE( uri.sslMode(), sslmode );
  QCOMPARE( uri.authConfigId(), authcfg );
}

void TestQgsDataSourceUri::checkSetConnectionService_data()
{
  QTest::addColumn<QString>( "service" );
  QTest::addColumn<QString>( "dbname" );
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn<QgsDataSourceUri::SslMode>( "sslmode" );
  QTest::addColumn<QString>( "authcfg" );

  QTest::newRow( "simple" )
      << "myservice" // service
      << "mydb" // dbname
      << "myname" // user
      << "mypasswd" // password
      << QgsDataSourceUri::SslPrefer // sslmode
      << "" // authcfg
      ;

  QTest::newRow( "authcfg" )
      << "myservice" // service
      << "" // dbname
      << "" // user
      << "mypasswd" // password
      << QgsDataSourceUri::SslPrefer // sslmode
      << "myauthcfg" // authcfg
      ;
}

void TestQgsDataSourceUri::checkSetConnectionService()
{
  QFETCH( QString, service );
  QFETCH( QString, dbname );
  QFETCH( QString, user );
  QFETCH( QString, password );
  QFETCH( QgsDataSourceUri::SslMode, sslmode );
  QFETCH( QString, authcfg );

  QgsDataSourceUri uri;
  if ( authcfg.isEmpty() )
    uri.setConnection( service, dbname, user, password, sslmode );
  else
    uri.setConnection( service, dbname, user, password, sslmode, authcfg );

  QCOMPARE( uri.service(), service );
  QCOMPARE( uri.database(), dbname );
  QCOMPARE( uri.username(), user );
  QCOMPARE( uri.password(), password );
  QCOMPARE( uri.sslMode(), sslmode );
  QCOMPARE( uri.authConfigId(), authcfg );
}

void TestQgsDataSourceUri::checkConnectionInfo_data()
{
  QTest::addColumn<QString>( "uri" );
  QTest::addColumn<QString>( "conninfo" );


  QTest::newRow( "service" )
      << "service='qgis_test'"
      << "service='qgis_test'" // conninfo
      ;

  QTest::newRow( "db_host_port_user_pw" )
      << "dbname='qgis_test' host=postgres port=5432 user='qgis_test_user' password='qgis_test_user_password'"
      << "dbname='qgis_test' host=postgres port=5432 user='qgis_test_user' password='qgis_test_user_password'" // conninfo
      ;

  QTest::newRow( "oci" )
      << "host=myhost port=1234 user='myname' password='mypasswd' estimatedmetadata=true srid=1000003007 table=\"myschema\".\"mytable\" (GEOM) myparam='myvalue' sql="
      << "host=myhost port=1234 user='myname' password='mypasswd'" // conninfo
      ;

  QTest::newRow( "pgrast" )
      << R"(PG: dbname='qgis_tests' host=localhost port=5432 user='myname' sslmode=disable estimatedmetadata=true srid=3067 table="public"."basic_map_tiled" (rast))"
      << "dbname='qgis_tests' host=localhost port=5432 user='myname' sslmode=disable" // conninfo
      ;

  QTest::newRow( "pg_notable" )
      << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=myschema "
      << "dbname='mydb' host=myhost port=5432 user='myname' password='mypasswd'" // conninfo
      ;

  QTest::newRow( "pg_notable_quoted" )
      << "dbname='mydb' host='myhost' user='myname' password='mypasswd' port='5432' mode='2' schema=myschema"
      << "dbname='mydb' host=myhost port=5432 user='myname' password='mypasswd'" // conninfo
      ;

  QTest::newRow( "pgmlsz" )
      << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=public column=geom table=mytable type=MultiLineStringZ"
      << "dbname='mydb' host=myhost port=5432 user='myname' password='mypasswd'" // conninfo
      ;
}

void TestQgsDataSourceUri::checkConnectionInfo()
{
  QFETCH( QString, uri );
  QFETCH( QString, conninfo );

  QgsDataSourceUri ds( uri );
  QCOMPARE( ds.connectionInfo(), conninfo );
  QCOMPARE( ds.connectionInfo( true ), conninfo );
  QCOMPARE( ds.connectionInfo( false ), conninfo );

  ds.setParam( QStringLiteral( "extraparam" ), QStringLiteral( "extravalue" ) );
  QCOMPARE( ds.connectionInfo(), conninfo );
  QCOMPARE( ds.connectionInfo( true ), conninfo );
  QCOMPARE( ds.connectionInfo( false ), conninfo );
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

void TestQgsDataSourceUri::checkParameterKeys()
{
  QgsDataSourceUri uri( QLatin1String( "dbname='foo' bar='baz'" ) );
  QCOMPARE( uri.parameterKeys().size(), 2 );
  QVERIFY( uri.parameterKeys().contains( QLatin1String( "dbname" ) ) );
  QVERIFY( uri.parameterKeys().contains( QLatin1String( "bar" ) ) );
}

QGSTEST_MAIN( TestQgsDataSourceUri )
#include "testqgsdatasourceuri.moc"
