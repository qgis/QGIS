/***************************************************************************
     testqgstiledmeshconnection.cpp
     --------------------
    Date                 : June 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include <QtConcurrent>
#include "qgstiledmeshconnection.h"
#include "qgssettings.h"

class TestQgsTiledMeshConnection : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.
    void encodeDecode();
    void testConnections();

};


void TestQgsTiledMeshConnection::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();
}

void TestQgsTiledMeshConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTiledMeshConnection::encodeDecode()
{
  QgsTiledMeshProviderConnection::Data data;
  data.provider = QStringLiteral( "test_provider" );
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QCOMPARE( QgsTiledMeshProviderConnection::encodedUri( data ), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::encodedLayerUri( data ), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );

  const QgsTiledMeshProviderConnection::Data data2 = QgsTiledMeshProviderConnection::decodedUri( QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( data2.url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( data2.authCfg, QStringLiteral( "my_auth" ) );
  QCOMPARE( data2.username, QStringLiteral( "my_user" ) );
  QCOMPARE( data2.password, QStringLiteral( "my_pw" ) );
  QCOMPARE( data2.httpHeaders.headers().value( QStringLiteral( "my_header" ) ).toString(), QStringLiteral( "value" ) );
}

void TestQgsTiledMeshConnection::testConnections()
{
  QVERIFY( QgsTiledMeshProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "does not exist" ) ).url, QString() );

  QgsTiledMeshProviderConnection conn = QgsTiledMeshProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QString() );


  QgsTiledMeshProviderConnection::Data data;
  data.provider = QStringLiteral( "test_provider" );
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QgsTiledMeshProviderConnection::addConnection( QStringLiteral( "my connection" ), data );
  QCOMPARE( QgsTiledMeshProviderConnection::connectionList(), {QStringLiteral( "my connection" )} );

  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "my connection" ) ).provider, QStringLiteral( "test_provider" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );

  // retrieve stored connection
  conn = QgsTiledMeshProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( qgis::down_cast< QgsTiledMeshProviderConnection * >( &conn )->providerKey(), QStringLiteral( "test_provider" ) );

  // add a second connection
  QgsTiledMeshProviderConnection::Data data2;
  data2.provider = QStringLiteral( "test_provider2" );
  data2.url = QStringLiteral( "http://testurl2" );
  data2.authCfg = QStringLiteral( "my_auth2" );
  data2.username = QStringLiteral( "my_user2" );
  data2.password = QStringLiteral( "my_pw2" );
  data2.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value2" ) );
  // construct connection using encoded uri
  QgsTiledMeshProviderConnection conn2( QgsTiledMeshProviderConnection::encodedUri( data2 ), QStringLiteral( "test_provider2" ), {} );
  QCOMPARE( conn2.uri(), QStringLiteral( "url=http://testurl2&username=my_user2&password=my_pw2&authcfg=my_auth2&http-header:my_header=value2" ) );
  QCOMPARE( qgis::down_cast< QgsTiledMeshProviderConnection * >( &conn2 )->providerKey(), QStringLiteral( "test_provider2" ) );
  conn2.store( QStringLiteral( "second connection" ) );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsTiledMeshProviderConnection::connectionList() ), qgis::listToSet( QStringList() << QStringLiteral( "my connection" ) << QStringLiteral( "second connection" ) ) );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "my connection" ) ).provider, QStringLiteral( "test_provider" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "second connection" ) ).provider, QStringLiteral( "test_provider2" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::connection( QStringLiteral( "second connection" ) ).url, QStringLiteral( "http://testurl2" ) );

  QgsTiledMeshProviderConnection::setSelectedConnection( QStringLiteral( "second connection" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::selectedConnection(), QStringLiteral( "second connection" ) );
  QgsTiledMeshProviderConnection::setSelectedConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( QgsTiledMeshProviderConnection::selectedConnection(), QStringLiteral( "my connection" ) );
}


QGSTEST_MAIN( TestQgsTiledMeshConnection )
#include "testqgstiledmeshconnection.moc"
