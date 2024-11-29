/***************************************************************************
     testqgstiledsceneconnection.cpp
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
#include "qgstiledsceneconnection.h"
#include "qgssettings.h"

class TestQgsTiledSceneConnection : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.
    void encodeDecode();
    void testConnections();
};


void TestQgsTiledSceneConnection::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();
}

void TestQgsTiledSceneConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsTiledSceneConnection::encodeDecode()
{
  QgsTiledSceneProviderConnection::Data data;
  data.provider = QStringLiteral( "test_provider" );
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QCOMPARE( QgsTiledSceneProviderConnection::encodedUri( data ), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::encodedLayerUri( data ), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );

  const QgsTiledSceneProviderConnection::Data data2 = QgsTiledSceneProviderConnection::decodedUri( QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( data2.url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( data2.authCfg, QStringLiteral( "my_auth" ) );
  QCOMPARE( data2.username, QStringLiteral( "my_user" ) );
  QCOMPARE( data2.password, QStringLiteral( "my_pw" ) );
  QCOMPARE( data2.httpHeaders.headers().value( QStringLiteral( "my_header" ) ).toString(), QStringLiteral( "value" ) );
}

void TestQgsTiledSceneConnection::testConnections()
{
  QVERIFY( QgsTiledSceneProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "does not exist" ) ).url, QString() );

  QgsTiledSceneProviderConnection conn = QgsTiledSceneProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QString() );


  QgsTiledSceneProviderConnection::Data data;
  data.provider = QStringLiteral( "test_provider" );
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QgsTiledSceneProviderConnection::addConnection( QStringLiteral( "my connection" ), data );
  QCOMPARE( QgsTiledSceneProviderConnection::connectionList(), { QStringLiteral( "my connection" ) } );

  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "my connection" ) ).provider, QStringLiteral( "test_provider" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );

  // retrieve stored connection
  conn = QgsTiledSceneProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( qgis::down_cast<QgsTiledSceneProviderConnection *>( &conn )->providerKey(), QStringLiteral( "test_provider" ) );

  // add a second connection
  QgsTiledSceneProviderConnection::Data data2;
  data2.provider = QStringLiteral( "test_provider2" );
  data2.url = QStringLiteral( "http://testurl2" );
  data2.authCfg = QStringLiteral( "my_auth2" );
  data2.username = QStringLiteral( "my_user2" );
  data2.password = QStringLiteral( "my_pw2" );
  data2.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value2" ) );
  // construct connection using encoded uri
  QgsTiledSceneProviderConnection conn2( QgsTiledSceneProviderConnection::encodedUri( data2 ), QStringLiteral( "test_provider2" ), {} );
  QCOMPARE( conn2.uri(), QStringLiteral( "url=http://testurl2&username=my_user2&password=my_pw2&authcfg=my_auth2&http-header:my_header=value2" ) );
  QCOMPARE( qgis::down_cast<QgsTiledSceneProviderConnection *>( &conn2 )->providerKey(), QStringLiteral( "test_provider2" ) );
  conn2.store( QStringLiteral( "second connection" ) );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsTiledSceneProviderConnection::connectionList() ), qgis::listToSet( QStringList() << QStringLiteral( "my connection" ) << QStringLiteral( "second connection" ) ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "my connection" ) ).provider, QStringLiteral( "test_provider" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "second connection" ) ).provider, QStringLiteral( "test_provider2" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( QStringLiteral( "second connection" ) ).url, QStringLiteral( "http://testurl2" ) );

  QgsTiledSceneProviderConnection::setSelectedConnection( QStringLiteral( "second connection" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::selectedConnection(), QStringLiteral( "second connection" ) );
  QgsTiledSceneProviderConnection::setSelectedConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( QgsTiledSceneProviderConnection::selectedConnection(), QStringLiteral( "my connection" ) );
}


QGSTEST_MAIN( TestQgsTiledSceneConnection )
#include "testqgstiledsceneconnection.moc"
