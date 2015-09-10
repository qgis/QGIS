/***************************************************************************
     testqgsrectangle.cpp
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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QObject>
//header for class being tested
#include <qgsdatasourceuri.h>

Q_DECLARE_METATYPE( QGis::WkbType )
Q_DECLARE_METATYPE( QgsDataSourceURI::SSLmode )

class TestQgsDataSourceUri: public QObject
{
    Q_OBJECT
  private slots:
    void checkparser();
    void checkparser_data();
};

void TestQgsDataSourceUri::checkparser_data()
{
  QTest::addColumn<QString>( "uri" );
  QTest::addColumn<QString>( "table" );
  QTest::addColumn<QString>( "geometrycolumn" );
  QTest::addColumn<QString>( "key" );
  QTest::addColumn<bool>( "estimatedmetadata" );
  QTest::addColumn<QString>( "srid" );
  QTest::addColumn<QGis::WkbType>( "type" );
  QTest::addColumn<bool>( "selectatid" );
  QTest::addColumn<QString>( "service" );
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn<QString>( "dbname" );
  QTest::addColumn<QString>( "host" );
  QTest::addColumn<QString>( "port" );
  QTest::addColumn<QgsDataSourceURI::SSLmode>( "sslmode" );
  QTest::addColumn<QString>( "sql" );
  QTest::addColumn<QString>( "myparam" );

  QTest::newRow( "oci" )
  << "host=myhost port=1234 user='myname' password='mypasswd' estimatedmetadata=true srid=1000003007 table=\"myschema\".\"mytable\" (GEOM) myparam='myvalue' sql="
  << "mytable" // table
  << "GEOM" // geometrycolumn
  << "" // key
  << true // estimatedmetadata
  << "1000003007" // srid
  << QGis::WKBUnknown // type
  << false // selectatid
  << "" // service
  << "myname" // user
  << "mypasswd" // password
  << "" // dbname
  << "myhost" // host
  << "1234" // port
  << QgsDataSourceURI::SSLprefer // sslmode
  << "" // sql
  << "myvalue" // myparam
  ;

  QTest::newRow( "pgrast" )
  << "PG: dbname=mydb host=myhost user=myname password=mypasswd port=5432 mode=2 schema=public column=geom table=mytable"
  << "mytable" // table
  << "" // geometrycolumn
  << "" // key
  << false // estimatedmetadata
  << "" // srid
  << QGis::WKBUnknown // type
  << false // selectatid
  << "" // service
  << "myname" // user
  << "mypasswd" // password
  << "mydb" // dbname
  << "myhost" // host
  << "5432" // port
  << QgsDataSourceURI::SSLprefer // sslmode
  << "" // sql
  << "" // myparam
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
  QFETCH( QGis::WkbType, type );
  QFETCH( bool, selectatid );
  QFETCH( QString, service );
  QFETCH( QString, user );
  QFETCH( QString, password );
  QFETCH( QString, dbname );
  QFETCH( QString, host );
  QFETCH( QString, port );
  QFETCH( QgsDataSourceURI::SSLmode, sslmode );
  QFETCH( QString, sql );
  QFETCH( QString, myparam );

  QgsDataSourceURI ds( uri );
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
  QCOMPARE( ds.sslMode(), sslmode );
  QCOMPARE( ds.sql(), sql );
  QCOMPARE( ds.param( "myparam" ), myparam );
}


QTEST_MAIN( TestQgsDataSourceUri )
#include "testqgsdatasourceuri.moc"

