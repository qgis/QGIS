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
#include "qgssettings.h"
#include "qgstest.h"
#include "qgstiledsceneconnection.h"

#include <QObject>
#include <QString>
#include <QtConcurrent>

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
  data.provider = u"test_provider"_s;
  data.url = u"http://testurl"_s;
  data.authCfg = u"my_auth"_s;
  data.username = u"my_user"_s;
  data.password = u"my_pw"_s;
  data.httpHeaders.insert( u"my_header"_s, u"value"_s );

  QCOMPARE( QgsTiledSceneProviderConnection::encodedUri( data ), u"url=http%3A%2F%2Ftesturl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::encodedLayerUri( data ), u"url=http%3A%2F%2Ftesturl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );

  const QgsTiledSceneProviderConnection::Data data2 = QgsTiledSceneProviderConnection::decodedUri( u"url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );
  QCOMPARE( data2.url, u"http://testurl"_s );
  QCOMPARE( data2.authCfg, u"my_auth"_s );
  QCOMPARE( data2.username, u"my_user"_s );
  QCOMPARE( data2.password, u"my_pw"_s );
  QCOMPARE( data2.httpHeaders.headers().value( u"my_header"_s ).toString(), u"value"_s );
}

void TestQgsTiledSceneConnection::testConnections()
{
  QVERIFY( QgsTiledSceneProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"does not exist"_s ).url, QString() );

  QgsTiledSceneProviderConnection conn = QgsTiledSceneProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), QString() );


  QgsTiledSceneProviderConnection::Data data;
  data.provider = u"test_provider"_s;
  data.url = u"http://testurl"_s;
  data.authCfg = u"my_auth"_s;
  data.username = u"my_user"_s;
  data.password = u"my_pw"_s;
  data.httpHeaders.insert( u"my_header"_s, u"value"_s );

  QgsTiledSceneProviderConnection::addConnection( u"my connection"_s, data );
  QCOMPARE( QgsTiledSceneProviderConnection::connectionList(), { u"my connection"_s } );

  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"my connection"_s ).provider, u"test_provider"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"my connection"_s ).url, u"http://testurl"_s );

  // retrieve stored connection
  conn = QgsTiledSceneProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), u"url=http%3A%2F%2Ftesturl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );
  QCOMPARE( qgis::down_cast<QgsTiledSceneProviderConnection *>( &conn )->providerKey(), u"test_provider"_s );

  // add a second connection
  QgsTiledSceneProviderConnection::Data data2;
  data2.provider = u"test_provider2"_s;
  data2.url = u"http://testurl2"_s;
  data2.authCfg = u"my_auth2"_s;
  data2.username = u"my_user2"_s;
  data2.password = u"my_pw2"_s;
  data2.httpHeaders.insert( u"my_header"_s, u"value2"_s );
  // construct connection using encoded uri
  QgsTiledSceneProviderConnection conn2( QgsTiledSceneProviderConnection::encodedUri( data2 ), u"test_provider2"_s, {} );
  QCOMPARE( conn2.uri(), u"url=http%3A%2F%2Ftesturl2&username=my_user2&password=my_pw2&authcfg=my_auth2&http-header:my_header=value2"_s );
  QCOMPARE( qgis::down_cast<QgsTiledSceneProviderConnection *>( &conn2 )->providerKey(), u"test_provider2"_s );
  conn2.store( u"second connection"_s );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsTiledSceneProviderConnection::connectionList() ), qgis::listToSet( QStringList() << u"my connection"_s << u"second connection"_s ) );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"my connection"_s ).provider, u"test_provider"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"my connection"_s ).url, u"http://testurl"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"second connection"_s ).provider, u"test_provider2"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::connection( u"second connection"_s ).url, u"http://testurl2"_s );

  QgsTiledSceneProviderConnection::setSelectedConnection( u"second connection"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::selectedConnection(), u"second connection"_s );
  QgsTiledSceneProviderConnection::setSelectedConnection( u"my connection"_s );
  QCOMPARE( QgsTiledSceneProviderConnection::selectedConnection(), u"my connection"_s );
}


QGSTEST_MAIN( TestQgsTiledSceneConnection )
#include "testqgstiledsceneconnection.moc"
