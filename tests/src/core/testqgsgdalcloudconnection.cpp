/***************************************************************************
     testqgsgdalcloudconnection.cpp
     --------------------
    Date                 : June 2024
    Copyright            : (C) 2024 by Nyall Dawson
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
#include "qgsgdalcloudconnection.h"
#include "qgssettings.h"

class TestQgsGdalCloudConnection : public QObject
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


void TestQgsGdalCloudConnection::initTestCase()
{
  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  QgsApplication::init();
  QgsApplication::initQgis();

  QgsSettings().clear();
}

void TestQgsGdalCloudConnection::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsGdalCloudConnection::encodeDecode()
{
  QgsGdalCloudProviderConnection::Data data;
  data.vsiHandler = QStringLiteral( "vsis3" );
  data.container = QStringLiteral( "my_container" );
  data.rootPath = QStringLiteral( "some/path" );
  data.credentialOptions = QVariantMap { { "pw", QStringLiteral( "xxxx" ) }, { "key", QStringLiteral( "yyy" ) } };

  QCOMPARE( QgsGdalCloudProviderConnection::encodedUri( data ), QStringLiteral( "container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some/path" ) );

  const QgsGdalCloudProviderConnection::Data data2 = QgsGdalCloudProviderConnection::decodedUri( QStringLiteral( "container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some/path" ) );
  QCOMPARE( data2.vsiHandler, QStringLiteral( "vsis3" ) );
  QCOMPARE( data2.container, QStringLiteral( "my_container" ) );
  QCOMPARE( data2.rootPath, QStringLiteral( "some/path" ) );
  QCOMPARE( data2.credentialOptions.value( QStringLiteral( "pw" ) ).toString(), QStringLiteral( "xxxx" ) );
  QCOMPARE( data2.credentialOptions.value( QStringLiteral( "key" ) ).toString(), QStringLiteral( "yyy" ) );
}

void TestQgsGdalCloudConnection::testConnections()
{
  QVERIFY( QgsGdalCloudProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "does not exist" ) ).container, QString() );

  QgsGdalCloudProviderConnection conn = QgsGdalCloudProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QString() );

  QgsGdalCloudProviderConnection::Data data;
  data.vsiHandler = QStringLiteral( "vsis3" );
  data.container = QStringLiteral( "my_container" );
  data.rootPath = QStringLiteral( "some/path" );
  data.credentialOptions = QVariantMap { { "pw", QStringLiteral( "xxxx" ) }, { "key", QStringLiteral( "yyy" ) } };

  QgsGdalCloudProviderConnection::addConnection( QStringLiteral( "my connection" ), data );
  QCOMPARE( QgsGdalCloudProviderConnection::connectionList(), { QStringLiteral( "my connection" ) } );

  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).vsiHandler, QStringLiteral( "vsis3" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).container, QStringLiteral( "my_container" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).rootPath, QStringLiteral( "some/path" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).credentialOptions.value( QStringLiteral( "pw" ) ).toString(), QStringLiteral( "xxxx" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).credentialOptions.value( QStringLiteral( "key" ) ).toString(), QStringLiteral( "yyy" ) );

  // retrieve stored connection
  conn = QgsGdalCloudProviderConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( conn.uri(), QStringLiteral( "container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some/path" ) );

  // add a second connection
  QgsGdalCloudProviderConnection::Data data2;
  data2.vsiHandler = QStringLiteral( "vsiaz" );
  data2.container = QStringLiteral( "some_container" );
  data2.rootPath = QStringLiteral( "path" );
  data2.credentialOptions = QVariantMap { { "pw", QStringLiteral( "zzz" ) } };

  QgsGdalCloudProviderConnection conn2( QgsGdalCloudProviderConnection::encodedUri( data2 ), {} );
  QCOMPARE( conn2.uri(), QStringLiteral( "container=some_container&credentialOptions=pw%3Dzzz&handler=vsiaz&rootPath=path" ) );
  conn2.store( QStringLiteral( "second connection" ) );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsGdalCloudProviderConnection::connectionList() ), qgis::listToSet( QStringList() << QStringLiteral( "my connection" ) << QStringLiteral( "second connection" ) ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).vsiHandler, QStringLiteral( "vsis3" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).container, QStringLiteral( "my_container" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).rootPath, QStringLiteral( "some/path" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).credentialOptions.value( QStringLiteral( "pw" ) ).toString(), QStringLiteral( "xxxx" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "my connection" ) ).credentialOptions.value( QStringLiteral( "key" ) ).toString(), QStringLiteral( "yyy" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "second connection" ) ).vsiHandler, QStringLiteral( "vsiaz" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "second connection" ) ).container, QStringLiteral( "some_container" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "second connection" ) ).rootPath, QStringLiteral( "path" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( QStringLiteral( "second connection" ) ).credentialOptions.value( QStringLiteral( "pw" ) ).toString(), QStringLiteral( "zzz" ) );

  QgsGdalCloudProviderConnection::setSelectedConnection( QStringLiteral( "second connection" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::selectedConnection(), QStringLiteral( "second connection" ) );
  QgsGdalCloudProviderConnection::setSelectedConnection( QStringLiteral( "my connection" ) );
  QCOMPARE( QgsGdalCloudProviderConnection::selectedConnection(), QStringLiteral( "my connection" ) );
}


QGSTEST_MAIN( TestQgsGdalCloudConnection )
#include "testqgsgdalcloudconnection.moc"
