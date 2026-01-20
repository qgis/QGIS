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
#include "qgsgdalcloudconnection.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QtConcurrent>

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
  data.vsiHandler = u"vsis3"_s;
  data.container = u"my_container"_s;
  data.rootPath = u"some/path"_s;
  data.credentialOptions = QVariantMap { { "pw", u"xxxx"_s }, { "key", u"yyy"_s } };

  QCOMPARE( QgsGdalCloudProviderConnection::encodedUri( data ), u"container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some%2Fpath"_s );

  const QgsGdalCloudProviderConnection::Data data2 = QgsGdalCloudProviderConnection::decodedUri( u"container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some/path"_s );
  QCOMPARE( data2.vsiHandler, u"vsis3"_s );
  QCOMPARE( data2.container, u"my_container"_s );
  QCOMPARE( data2.rootPath, u"some/path"_s );
  QCOMPARE( data2.credentialOptions.value( u"pw"_s ).toString(), u"xxxx"_s );
  QCOMPARE( data2.credentialOptions.value( u"key"_s ).toString(), u"yyy"_s );
}

void TestQgsGdalCloudConnection::testConnections()
{
  QVERIFY( QgsGdalCloudProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"does not exist"_s ).container, QString() );

  QgsGdalCloudProviderConnection conn = QgsGdalCloudProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), QString() );

  QgsGdalCloudProviderConnection::Data data;
  data.vsiHandler = u"vsis3"_s;
  data.container = u"my_container"_s;
  data.rootPath = u"some/path"_s;
  data.credentialOptions = QVariantMap { { "pw", u"xxxx"_s }, { "key", u"yyy"_s } };

  QgsGdalCloudProviderConnection::addConnection( u"my connection"_s, data );
  QCOMPARE( QgsGdalCloudProviderConnection::connectionList(), { u"my connection"_s } );

  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).vsiHandler, u"vsis3"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).container, u"my_container"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).rootPath, u"some/path"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).credentialOptions.value( u"pw"_s ).toString(), u"xxxx"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).credentialOptions.value( u"key"_s ).toString(), u"yyy"_s );

  // retrieve stored connection
  conn = QgsGdalCloudProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), u"container=my_container&credentialOptions=key%3Dyyy%7Cpw%3Dxxxx&handler=vsis3&rootPath=some%2Fpath"_s );

  // add a second connection
  QgsGdalCloudProviderConnection::Data data2;
  data2.vsiHandler = u"vsiaz"_s;
  data2.container = u"some_container"_s;
  data2.rootPath = u"path"_s;
  data2.credentialOptions = QVariantMap { { "pw", u"zzz"_s } };

  QgsGdalCloudProviderConnection conn2( QgsGdalCloudProviderConnection::encodedUri( data2 ), {} );
  QCOMPARE( conn2.uri(), u"container=some_container&credentialOptions=pw%3Dzzz&handler=vsiaz&rootPath=path"_s );
  conn2.store( u"second connection"_s );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsGdalCloudProviderConnection::connectionList() ), qgis::listToSet( QStringList() << u"my connection"_s << u"second connection"_s ) );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).vsiHandler, u"vsis3"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).container, u"my_container"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).rootPath, u"some/path"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).credentialOptions.value( u"pw"_s ).toString(), u"xxxx"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"my connection"_s ).credentialOptions.value( u"key"_s ).toString(), u"yyy"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"second connection"_s ).vsiHandler, u"vsiaz"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"second connection"_s ).container, u"some_container"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"second connection"_s ).rootPath, u"path"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::connection( u"second connection"_s ).credentialOptions.value( u"pw"_s ).toString(), u"zzz"_s );

  QgsGdalCloudProviderConnection::setSelectedConnection( u"second connection"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::selectedConnection(), u"second connection"_s );
  QgsGdalCloudProviderConnection::setSelectedConnection( u"my connection"_s );
  QCOMPARE( QgsGdalCloudProviderConnection::selectedConnection(), u"my connection"_s );
}


QGSTEST_MAIN( TestQgsGdalCloudConnection )
#include "testqgsgdalcloudconnection.moc"
