/***************************************************************************
     testqgssensorthingsconnection.cpp
     --------------------
    Date                 : December 2023
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
#include "qgssensorthingsconnection.h"
#include "qgssettings.h"

class TestQgsSensorThingsConnection : public QObject
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


void TestQgsSensorThingsConnection::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();
}

void TestQgsSensorThingsConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSensorThingsConnection::encodeDecode()
{
  QgsSensorThingsProviderConnection::Data data;
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QCOMPARE( QgsSensorThingsProviderConnection::encodedUri( data ), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( QgsSensorThingsProviderConnection::encodedLayerUri( data ), QStringLiteral( "user='my_user' password='my_pw' authcfg=my_auth url='http://testurl' http-header:my_header='value'" ) );

  const QgsSensorThingsProviderConnection::Data data2 = QgsSensorThingsProviderConnection::decodedUri( QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );
  QCOMPARE( data2.url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( data2.authCfg, QStringLiteral( "my_auth" ) );
  QCOMPARE( data2.username, QStringLiteral( "my_user" ) );
  QCOMPARE( data2.password, QStringLiteral( "my_pw" ) );
  QCOMPARE( data2.httpHeaders.headers().value( QStringLiteral( "my_header" ) ).toString(), QStringLiteral( "value" ) );
}

void TestQgsSensorThingsConnection::testConnections()
{
  QVERIFY( QgsSensorThingsProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( QStringLiteral( "does not exist" ) ).url, QString() );

  QgsSensorThingsProviderConnection conn = QgsSensorThingsProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QString() );

  QgsSensorThingsProviderConnection::Data data;
  data.url = QStringLiteral( "http://testurl" );
  data.authCfg = QStringLiteral( "my_auth" );
  data.username = QStringLiteral( "my_user" );
  data.password = QStringLiteral( "my_pw" );
  data.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value" ) );

  QgsSensorThingsProviderConnection::addConnection( QStringLiteral( "my connection" ), data );
  QCOMPARE( QgsSensorThingsProviderConnection::connectionList(), { QStringLiteral( "my connection" ) } );

  QCOMPARE( QgsSensorThingsProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );

  // retrieve stored connection
  conn = QgsSensorThingsProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QStringLiteral( "url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value" ) );

  // add a second connection
  QgsSensorThingsProviderConnection::Data data2;
  data2.url = QStringLiteral( "http://testurl2" );
  data2.authCfg = QStringLiteral( "my_auth2" );
  data2.username = QStringLiteral( "my_user2" );
  data2.password = QStringLiteral( "my_pw2" );
  data2.httpHeaders.insert( QStringLiteral( "my_header" ), QStringLiteral( "value2" ) );
  // construct connection using encoded uri
  QgsSensorThingsProviderConnection conn2( QgsSensorThingsProviderConnection::encodedUri( data2 ), {} );
  QCOMPARE( conn2.uri(), QStringLiteral( "url=http://testurl2&username=my_user2&password=my_pw2&authcfg=my_auth2&http-header:my_header=value2" ) );
  conn2.store( QStringLiteral( "second connection" ) );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsSensorThingsProviderConnection::connectionList() ), qgis::listToSet( QStringList() << QStringLiteral( "my connection" ) << QStringLiteral( "second connection" ) ) );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( QStringLiteral( "my connection" ) ).url, QStringLiteral( "http://testurl" ) );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( QStringLiteral( "second connection" ) ).url, QStringLiteral( "http://testurl2" ) );

  QgsSensorThingsProviderConnection::setSelectedConnection( QStringLiteral( "second connection" ) );
  QCOMPARE( QgsSensorThingsProviderConnection::selectedConnection(), QStringLiteral( "second connection" ) );
  QgsSensorThingsProviderConnection::setSelectedConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( QgsSensorThingsProviderConnection::selectedConnection(), QStringLiteral( "my connection" ) );
}


QGSTEST_MAIN( TestQgsSensorThingsConnection )
#include "testqgssensorthingsconnection.moc"
