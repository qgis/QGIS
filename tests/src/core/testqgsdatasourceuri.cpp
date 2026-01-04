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

Q_DECLARE_METATYPE( Qgis::WkbType )
Q_DECLARE_METATYPE( QgsDataSourceUri::SslMode )

class TestQgsDataSourceUri : public QObject
{
    Q_OBJECT
  private slots:
    void equality();
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
    void checkRemovePassword();
    void checkUnicodeUri();
    void checkUriInUri();
};

void TestQgsDataSourceUri::checkparser_data()
{
  QTest::addColumn<QString>( "uri" );
  QTest::addColumn<QString>( "table" );
  QTest::addColumn<QString>( "geometrycolumn" );
  QTest::addColumn<QString>( "key" );
  QTest::addColumn<bool>( "estimatedmetadata" );
  QTest::addColumn<QString>( "srid" );
  QTest::addColumn<Qgis::WkbType>( "type" );
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
    << "mytable"                   // table
    << "GEOM"                      // geometrycolumn
    << ""                          // key
    << true                        // estimatedmetadata
    << "1000003007"                // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << "myname"                    // user
    << "mypasswd"                  // password
    << ""                          // authcfg
    << ""                          // dbname
    << "myhost"                    // host
    << "1234"                      // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << "myvalue"                   // myparam
    << "myschema";

  QTest::newRow( "pgrast" )
    << R"(PG: dbname='qgis_tests' host=localhost port=5432 user='myname' sslmode=disable estimatedmetadata=true srid=3067 table="public"."basic_map_tiled" (rast))"
    << "basic_map_tiled"            // table
    << "rast"                       // geometrycolumn
    << ""                           // key
    << true                         // estimatedmetadata
    << "3067"                       // srid
    << Qgis::WkbType::Unknown       // type
    << false                        // selectatid
    << ""                           // service
    << "myname"                     // user
    << ""                           // password
    << ""                           // authcfg
    << "qgis_tests"                 // dbname
    << "localhost"                  // host
    << "5432"                       // port
    << ""                           // driver
    << QgsDataSourceUri::SslDisable // sslmode
    << ""                           // sql
    << ""                           // myparam
    << "public"                     // schema
    ;

  QTest::newRow( "pg_notable" )
    << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=myschema "
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << "myname"                    // user
    << "mypasswd"                  // password
    << ""                          // authcfg
    << "mydb"                      // dbname
    << "myhost"                    // host
    << "5432"                      // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << ""                          // myparam
    << "public"                    // schema
    ;

  QTest::newRow( "pg_notable_quoted" )
    << "dbname='mydb' host='myhost' user='myname' password='mypasswd' port='5432' mode='2' schema=myschema"
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << "myname"                    // user
    << "mypasswd"                  // password
    << ""                          // authcfg
    << "mydb"                      // dbname
    << "myhost"                    // host
    << "5432"                      // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << ""                          // myparam
    << "public"                    // schema
    ;

  QTest::newRow( "pg_notable_authcfg" )
    << "PG: dbname=mydb host=myhost authcfg=myauthcfg port=5432 mode=2 schema=myschema "
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << ""                          // user
    << ""                          // password
    << "myauthcfg"                 // authcfg
    << "mydb"                      // dbname
    << "myhost"                    // host
    << "5432"                      // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << ""                          // myparam
    << "public"                    // schema
    ;


  QTest::newRow( "pgmlsz" )
    << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=public column=geom table=mytable type=MultiLineStringZ"
    << "mytable"                       // table
    << ""                              // geometrycolumn
    << ""                              // key
    << false                           // estimatedmetadata
    << ""                              // srid
    << Qgis::WkbType::MultiLineStringZ // type
    << false                           // selectatid
    << ""                              // service
    << "myname"                        // user
    << "mypasswd"                      // password
    << ""                              // authcfg
    << "mydb"                          // dbname
    << "myhost"                        // host
    << "5432"                          // port
    << ""                              // driver
    << QgsDataSourceUri::SslPrefer     // sslmode
    << ""                              // sql
    << ""                              // myparam
    << "public"                        // schema
    ;

  QTest::newRow( "arcgis rest sql" )
    << "crs='EPSG:2154' filter='' url='https://carto.isogeo.net/server/rest/services/scan_services_1/EMS_EFS_WMS_WFS/FeatureServer/2' table='' sql=abc='def'"
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << ""                          // user
    << ""                          // password
    << ""                          // authcfg
    << ""                          // dbname
    << ""                          // host
    << ""                          // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << "abc='def'"                 // sql
    << ""                          // myparam
    << "public"                    // schema
    ;

  QTest::newRow( "arcgis rest empty sql" )
    << "crs='EPSG:2154' filter='' url='https://carto.isogeo.net/server/rest/services/scan_services_1/EMS_EFS_WMS_WFS/FeatureServer/2' table='' sql=''"
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << ""                          // user
    << ""                          // password
    << ""                          // authcfg
    << ""                          // dbname
    << ""                          // host
    << ""                          // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << ""                          // myparam
    << "public"                    // schema
    ;

  QTest::newRow( "arcgis rest empty sql 2" )
    << "crs='EPSG:2154' filter='' url='https://carto.isogeo.net/server/rest/services/scan_services_1/EMS_EFS_WMS_WFS/FeatureServer/2' table='' sql=\"\""
    << ""                          // table
    << ""                          // geometrycolumn
    << ""                          // key
    << false                       // estimatedmetadata
    << ""                          // srid
    << Qgis::WkbType::Unknown      // type
    << false                       // selectatid
    << ""                          // service
    << ""                          // user
    << ""                          // password
    << ""                          // authcfg
    << ""                          // dbname
    << ""                          // host
    << ""                          // port
    << ""                          // driver
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // sql
    << ""                          // myparam
    << "public"                    // schema
    ;
}

void TestQgsDataSourceUri::equality()
{
  QgsDataSourceUri uri1;
  QgsDataSourceUri uri2;

  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setHost( u"localhost"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setHost( u"remote"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setHost( u"localhost"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setPort( u"5432"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setPort( u"5433"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setPort( u"5432"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setDriver( u"QPSQL"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setDriver( u"mssql"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setDriver( u"QPSQL"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setService( u"service"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setService( u"service2"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setService( u"service"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setDatabase( u"mydb"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setDatabase( u"mydb2"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setDatabase( u"mydb"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setUsername( u"user"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setUsername( u"user2"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setUsername( u"user"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setPassword( u"pass"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setPassword( u"pass2"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setPassword( u"pass"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setSchema( u"public"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSchema( u"other"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSchema( u"public"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setTable( u"mytable"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setTable( u"othertable"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setTable( u"mytable"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setGeometryColumn( u"geom"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setGeometryColumn( u"geometry"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setGeometryColumn( u"geom"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setKeyColumn( u"id"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setKeyColumn( u"id"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setSql( u"WHERE id > 10"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSql( u"WHERE id < 10"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSql( u"WHERE id > 10"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setAuthConfigId( u"abc123"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setAuthConfigId( u"def123"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setAuthConfigId( u"abc123"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setSslMode( QgsDataSourceUri::SslAllow );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSslMode( QgsDataSourceUri::SslAllow );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setKeyColumn( u"pk"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setKeyColumn( u"id"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setKeyColumn( u"pk"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setUseEstimatedMetadata( true );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setUseEstimatedMetadata( true );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.disableSelectAtId( true );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.disableSelectAtId( true );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setWkbType( Qgis::WkbType::Point );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setWkbType( Qgis::WkbType::Point );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setSrid( u"4326"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSrid( u"3111"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setSrid( u"4326"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  uri1.setParam( u"param1"_s, u"value1"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.setParam( u"param1"_s, u"value2"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  // params are a multi-map!
  uri2.setParam( u"param1"_s, u"value1"_s );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  uri2.removeParam( u"param1"_s );
  uri2.setParam( u"param1"_s, u"value1"_s );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );

  QgsHttpHeaders headers1;
  headers1.insert( u"Authorization"_s, u"Bearer token123"_s );
  uri1.setHttpHeaders( headers1 );
  QVERIFY( uri1 != uri2 );
  QVERIFY( !( uri1 == uri2 ) );
  QgsHttpHeaders headers2;
  headers2.insert( u"Authorization"_s, u"Bearer token123"_s );
  uri2.setHttpHeaders( headers2 );
  QVERIFY( uri1 == uri2 );
  QVERIFY( !( uri1 != uri2 ) );
}

void TestQgsDataSourceUri::checkparser()
{
  QFETCH( QString, uri );
  QFETCH( QString, table );
  QFETCH( QString, geometrycolumn );
  QFETCH( QString, key );
  QFETCH( bool, estimatedmetadata );
  QFETCH( QString, srid );
  QFETCH( Qgis::WkbType, type );
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
    << "myhost"                    // host
    << "5432"                      // port
    << "mydb"                      // dbname
    << "myname"                    // user
    << "mypasswd"                  // password
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // authcfg
    ;

  QTest::newRow( "authcfg" )
    << "myhost"                    // host
    << "5432"                      // port
    << ""                          // dbname
    << ""                          // user
    << "mypasswd"                  // password
    << QgsDataSourceUri::SslPrefer // sslmode
    << "myauthcfg"                 // authcfg
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
    << "myservice"                 // service
    << "mydb"                      // dbname
    << "myname"                    // user
    << "mypasswd"                  // password
    << QgsDataSourceUri::SslPrefer // sslmode
    << ""                          // authcfg
    ;

  QTest::newRow( "authcfg" )
    << "myservice"                 // service
    << ""                          // dbname
    << ""                          // user
    << "mypasswd"                  // password
    << QgsDataSourceUri::SslPrefer // sslmode
    << "myauthcfg"                 // authcfg
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

  ds.setParam( u"extraparam"_s, u"extravalue"_s );
  QCOMPARE( ds.connectionInfo(), conninfo );
  QCOMPARE( ds.connectionInfo( true ), conninfo );
  QCOMPARE( ds.connectionInfo( false ), conninfo );
}

void TestQgsDataSourceUri::checkAuthParams()
{
  // some providers rely on the QgsDataSourceUri params for storing and retrieving username, password and authentication.
  // Test here that the direct setters and getters for username/password/authcfg are compatible with providers which utilize the parameter system

  QgsDataSourceUri uri;
  QVERIFY( uri.param( u"username"_s ).isEmpty() );
  QVERIFY( uri.param( u"password"_s ).isEmpty() );
  QVERIFY( uri.param( u"authcfg"_s ).isEmpty() );

  uri.setUsername( u"kaladin"_s );
  uri.setPassword( u"stormblessed"_s );
  uri.setAuthConfigId( u"syl"_s );

  QCOMPARE( uri.param( u"username"_s ), u"kaladin"_s );
  QCOMPARE( uri.param( u"password"_s ), u"stormblessed"_s );
  QCOMPARE( uri.param( u"authcfg"_s ), u"syl"_s );

  // round trip through encodedUri should not lose username/password/authcfg
  const QByteArray encoded = uri.encodedUri();
  QgsDataSourceUri uri2;
  uri2.setEncodedUri( encoded );

  QCOMPARE( uri2.param( u"username"_s ), u"kaladin"_s );
  QCOMPARE( uri2.username(), u"kaladin"_s );
  QCOMPARE( uri2.param( u"password"_s ), u"stormblessed"_s );
  QCOMPARE( uri2.password(), u"stormblessed"_s );
  QCOMPARE( uri2.param( u"authcfg"_s ), u"syl"_s );
  QCOMPARE( uri2.authConfigId(), u"syl"_s );

  QgsDataSourceUri uri3;
  uri3.setParam( u"username"_s, u"kaladin"_s );
  uri3.setParam( u"password"_s, u"stormblessed"_s );
  uri3.setParam( u"authcfg"_s, u"syl"_s );
  QCOMPARE( uri3.param( u"username"_s ), u"kaladin"_s );
  QCOMPARE( uri3.params( u"username"_s ), QStringList() << u"kaladin"_s );
  QCOMPARE( uri3.username(), u"kaladin"_s );
  QCOMPARE( uri3.param( u"password"_s ), u"stormblessed"_s );
  QCOMPARE( uri3.params( u"password"_s ), QStringList() << u"stormblessed"_s );
  QCOMPARE( uri3.password(), u"stormblessed"_s );
  QCOMPARE( uri3.param( u"authcfg"_s ), u"syl"_s );
  QCOMPARE( uri3.params( u"authcfg"_s ), QStringList() << u"syl"_s );
  QCOMPARE( uri3.authConfigId(), u"syl"_s );

  QVERIFY( uri.hasParam( u"username"_s ) );
  uri.removeParam( u"username"_s );
  QVERIFY( !uri.hasParam( u"username"_s ) );
  QVERIFY( uri.param( u"username"_s ).isEmpty() );
  QVERIFY( uri.username().isEmpty() );
  QVERIFY( uri.hasParam( u"password"_s ) );
  uri.removeParam( u"password"_s );
  QVERIFY( !uri.hasParam( u"password"_s ) );
  QVERIFY( uri.param( u"password"_s ).isEmpty() );
  QVERIFY( uri.password().isEmpty() );
  QVERIFY( uri.hasParam( u"authcfg"_s ) );
  uri.removeParam( u"authcfg"_s );
  QVERIFY( !uri.hasParam( u"authcfg"_s ) );
  QVERIFY( uri.param( u"authcfg"_s ).isEmpty() );
  QVERIFY( uri.authConfigId().isEmpty() );

  // issue GH #39243
  QgsDataSourceUri uri4;
  uri4.setEncodedUri( u"dpiMode=7&url=http://localhost:8000/ows/?MAP%3D/home/bug.qgs&username=username&password=pa%25%25word"_s );

  QCOMPARE( uri4.param( u"username"_s ), u"username"_s );
  QCOMPARE( uri4.username(), u"username"_s );
  QCOMPARE( uri4.param( u"password"_s ), u"pa%%word"_s );
  QCOMPARE( uri4.password(), u"pa%%word"_s );

  // issue GH #42405
  uri4.setEncodedUri( u"dpiMode=7&url=http://localhost:8000/ows/?MAP%3D/home/bug.qgs&username=username&password=qgis%C3%A8%C3%A9"_s );
  QCOMPARE( uri4.param( u"username"_s ), u"username"_s );
  QCOMPARE( uri4.username(), u"username"_s );
  QCOMPARE( uri4.param( u"password"_s ), u"qgisèé"_s );
  QCOMPARE( uri4.password(), u"qgisèé"_s );

  uri4.setEncodedUri( u"dpiMode=7&url=http://localhost:8000/&username=username&password=%1"_s.arg( QString( QUrl::toPercentEncoding( u"😁😂😍"_s ) ) ) );
  QCOMPARE( uri4.param( u"username"_s ), u"username"_s );
  QCOMPARE( uri4.username(), u"username"_s );
  QCOMPARE( uri4.param( u"password"_s ), u"😁😂😍"_s );
  QCOMPARE( uri4.password(), u"😁😂😍"_s );

  // issue GH #53654
  QgsDataSourceUri uri5;
  uri5.setEncodedUri( u"zmax=14&zmin=0&styleUrl=http://localhost:8000/&f=application%2Fvnd.geoserver.mbstyle%2Bjson"_s );
  QCOMPARE( uri5.param( u"f"_s ), u"application/vnd.geoserver.mbstyle+json"_s );

  uri5.setEncodedUri( u"zmax=14&zmin=0&styleUrl=http://localhost:8000/&f=application/vnd.geoserver.mbstyle+json"_s );
  QCOMPARE( uri5.param( u"f"_s ), u"application/vnd.geoserver.mbstyle+json"_s );

  // round trip through encodedUri/setEncodedUri should not lose "%2B" or "+"
  QgsDataSourceUri uri6;
  uri6.setParam( u"percent"_s, u"application%2Fvnd.geoserver.mbstyle%2Bjson"_s );
  uri6.setParam( u"explicit"_s, u"application/vnd.geoserver.mbstyle+json"_s );
  QCOMPARE( uri6.param( u"percent"_s ), u"application%2Fvnd.geoserver.mbstyle%2Bjson"_s );
  QCOMPARE( uri6.param( u"explicit"_s ), u"application/vnd.geoserver.mbstyle+json"_s );

  const QByteArray encodedTwo = uri6.encodedUri();

  QgsDataSourceUri uri7;
  uri7.setEncodedUri( encodedTwo );
  QCOMPARE( uri7.param( u"percent"_s ), u"application%2Fvnd.geoserver.mbstyle%2Bjson"_s );
  QCOMPARE( uri7.param( u"explicit"_s ), u"application/vnd.geoserver.mbstyle+json"_s );
}

void TestQgsDataSourceUri::checkParameterKeys()
{
  QgsDataSourceUri uri( "dbname='foo' bar='baz'"_L1 );
  QCOMPARE( uri.parameterKeys().size(), 2 );
  QVERIFY( uri.parameterKeys().contains( "dbname"_L1 ) );
  QVERIFY( uri.parameterKeys().contains( "bar"_L1 ) );
}

void TestQgsDataSourceUri::checkRemovePassword()
{
  const QString uri0 = QgsDataSourceUri::removePassword( u"postgresql://user:password@127.0.0.1:5432?dbname=test"_s );
  QCOMPARE( uri0, u"postgresql://user@127.0.0.1:5432?dbname=test"_s );

  const QString uri1 = QgsDataSourceUri::removePassword( u"postgresql://user:password@127.0.0.1:5432?dbname=test"_s, true );
  QCOMPARE( uri1, u"postgresql://user:XXXXXXXX@127.0.0.1:5432?dbname=test"_s );

  const QString uri2 = QgsDataSourceUri::removePassword( u"postgresql://user@127.0.0.1:5432?dbname=test"_s );
  QCOMPARE( uri2, u"postgresql://user@127.0.0.1:5432?dbname=test"_s );
}

void TestQgsDataSourceUri::checkUnicodeUri()
{
  QgsDataSourceUri uri;
  uri.setEncodedUri( u"url=file:///directory/テスト.mbtiles&type=mbtiles"_s );
  QCOMPARE( uri.param( u"url"_s ), u"file:///directory/テスト.mbtiles"_s );
}

void TestQgsDataSourceUri::checkUriInUri()
{
  QString dataUri = u"dpiMode=7&url=%1&SERVICE=WMS&REQUEST=GetCapabilities&username=username&password=qgis%C3%A8%C3%A9"_s;

  // If the 'url' field references a QGIS server then the 'MAP' parameter can contain an url to the project file.
  // When the project is saved in a postgresql db, the connection url will also contains '&' and '='.
  {
    QgsDataSourceUri uri;
    // here the project url is encoded but the whole serverUrl is not encoded.
    // The OGC server will receive a call with this url: http://localhost:8000/ows/?MAP=postgresql://?service=qgis_test&dbname&schema=project&project=luxembourg&SERVICE=WMS&REQUEST=GetCapabilities
    // from the OGC server POV the 'schema' and 'project' keys will be parsed as main query parameters for 'http://localhost:8000/ows/?'
    // and not associated to the project file uri.
    QString project = "postgresql://?service=qgis_test&dbname&schema=project&project=luxembourg";
    QString projectEnc = QUrl::toPercentEncoding( project );
    QString serverUrl = QString( "http://localhost:8000/ows/?MAP=%1" );
    uri.setEncodedUri( dataUri.arg( serverUrl.arg( projectEnc ) ) );
    QCOMPARE( uri.param( u"username"_s ), u"username"_s );
    QCOMPARE( uri.username(), u"username"_s );
    QCOMPARE( uri.param( u"password"_s ), u"qgisèé"_s );
    QCOMPARE( uri.password(), u"qgisèé"_s );
    QCOMPARE( uri.param( u"SERVICE"_s ), u"WMS"_s );
    QCOMPARE( uri.param( u"REQUEST"_s ), u"GetCapabilities"_s );
    // not enough encoded at the beginning ==> bad encoding at the end
    QCOMPARE( uri.param( u"url"_s ), serverUrl.arg( project ) );

    QgsDataSourceUri uri2;
    // here the project url is encoded and the whole serverUrl is also encoded.
    // The OGC server will receive a call with this url: http://localhost:8000/ows/?MAP=postgresql%3A%2F%2F%3Fservice%3Dqgis_test%26dbname%26schema%3Dproject%26project%3Dluxembourg&SERVICE=WMS&REQUEST=GetCapabilities
    // and will be able to decode all parameters
    QString serverUrlEnc = QUrl::toPercentEncoding( serverUrl.arg( projectEnc ) );
    uri2.setEncodedUri( dataUri.arg( serverUrlEnc ) );
    QCOMPARE( uri2.param( u"username"_s ), u"username"_s );
    QCOMPARE( uri2.username(), u"username"_s );
    QCOMPARE( uri2.param( u"password"_s ), u"qgisèé"_s );
    QCOMPARE( uri2.password(), u"qgisèé"_s );
    QCOMPARE( uri2.param( u"SERVICE"_s ), u"WMS"_s );
    QCOMPARE( uri2.param( u"REQUEST"_s ), u"GetCapabilities"_s );
    QCOMPARE( uri2.param( u"url"_s ), serverUrl.arg( projectEnc ) );
  }

  // same as above but with extra param at the end of the
  {
    QgsDataSourceUri uri;
    // here the project url is encoded but the whole serverUrl is not encoded.
    // The OGC server will receive a call with this url: https://titiler.xyz/cog/tiles/WebMercatorQuad/16/34060/23336@1x?url=https://data.geo.admin.ch/ch.swisstopo.swissalti3d/swissalti3d_2019_2573-1085/swissalti3d_2019_2573-1085_0.5_2056_5728.tif&bidx=1&rescale=1600%2C2100&colormap_name=gist_earth
    // from the OGC server POV the 'rescale' and 'colormap_name' keys could be parsed as sub query parameters for 'https://data.geo.admin.ch/'
    QString project = "https://data.geo.admin.ch/ch.swisstopo.swissalti3d/swissalti3d_2019_2573-1085/swissalti3d_2019_2573-1085_0.5_2056_5728.tif";
    QString projectEnc = QUrl::toPercentEncoding( project );
    QString extraParam = "&bidx=1&rescale=1600%2C2100&colormap_name=gist_earth";
    QString serverUrl = QString( "https://titiler.xyz/cog/tiles/WebMercatorQuad/16/34060/23336@1x?url=%1" );

    uri.setEncodedUri( dataUri.arg( serverUrl.arg( projectEnc ) + extraParam ) );
    QCOMPARE( uri.param( u"username"_s ), u"username"_s );
    QCOMPARE( uri.username(), u"username"_s );
    QCOMPARE( uri.param( u"password"_s ), u"qgisèé"_s );
    QCOMPARE( uri.password(), u"qgisèé"_s );
    QCOMPARE( uri.param( u"SERVICE"_s ), u"WMS"_s );
    QCOMPARE( uri.param( u"REQUEST"_s ), u"GetCapabilities"_s );
    // not enough encoded at the beginning ==> bad encoding at the end
    QCOMPARE( uri.param( u"url"_s ), serverUrl.arg( project ) );

    QgsDataSourceUri uri2;
    // here the project url is encoded and the whole serverUrl is also encoded.
    // The OGC server will receive a call with this url: https://titiler.xyz/cog/tiles/WebMercatorQuad/16/34060/23336@1x?url=https%3A%2F%2Fdata.geo.admin.ch%2Fch.swisstopo.swissalti3d%2Fswissalti3d_2019_2573-1085%2Fswissalti3d_2019_2573-1085_0.5_2056_5728.tif&bidx=1&rescale=1600%2C2100&colormap_name=gist_earth
    // and will be able to decode all parameters
    QString serverUrlEnc = QUrl::toPercentEncoding( serverUrl.arg( projectEnc ) + extraParam );
    uri2.setEncodedUri( dataUri.arg( serverUrlEnc ) );
    QCOMPARE( uri2.param( u"username"_s ), u"username"_s );
    QCOMPARE( uri2.username(), u"username"_s );
    QCOMPARE( uri2.param( u"password"_s ), u"qgisèé"_s );
    QCOMPARE( uri2.password(), u"qgisèé"_s );
    QCOMPARE( uri2.param( u"SERVICE"_s ), u"WMS"_s );
    QCOMPARE( uri2.param( u"REQUEST"_s ), u"GetCapabilities"_s );
    QCOMPARE( uri2.param( u"url"_s ), serverUrl.arg( projectEnc ) + extraParam );
  }
}


QGSTEST_MAIN( TestQgsDataSourceUri )
#include "testqgsdatasourceuri.moc"
