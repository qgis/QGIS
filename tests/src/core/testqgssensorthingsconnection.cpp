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
#include "qgssensorthingsconnection.h"
#include "qgssettings.h"
#include "qgstest.h"

#include <QObject>
#include <QString>
#include <QtConcurrent>

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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

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
  data.url = u"http://testurl"_s;
  data.authCfg = u"my_auth"_s;
  data.username = u"my_user"_s;
  data.password = u"my_pw"_s;
  data.httpHeaders.insert( u"my_header"_s, u"value"_s );

  QCOMPARE( QgsSensorThingsProviderConnection::encodedUri( data ), u"url=http%3A%2F%2Ftesturl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );
  QCOMPARE( QgsSensorThingsProviderConnection::encodedLayerUri( data ), u"user='my_user' password='my_pw' authcfg=my_auth url='http://testurl' http-header:my_header='value'"_s );

  const QgsSensorThingsProviderConnection::Data data2 = QgsSensorThingsProviderConnection::decodedUri( u"url=http://testurl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );
  QCOMPARE( data2.url, u"http://testurl"_s );
  QCOMPARE( data2.authCfg, u"my_auth"_s );
  QCOMPARE( data2.username, u"my_user"_s );
  QCOMPARE( data2.password, u"my_pw"_s );
  QCOMPARE( data2.httpHeaders.headers().value( u"my_header"_s ).toString(), u"value"_s );
}

void TestQgsSensorThingsConnection::testConnections()
{
  QVERIFY( QgsSensorThingsProviderConnection::connectionList().isEmpty() );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( u"does not exist"_s ).url, QString() );

  QgsSensorThingsProviderConnection conn = QgsSensorThingsProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), QString() );

  QgsSensorThingsProviderConnection::Data data;
  data.url = u"http://testurl"_s;
  data.authCfg = u"my_auth"_s;
  data.username = u"my_user"_s;
  data.password = u"my_pw"_s;
  data.httpHeaders.insert( u"my_header"_s, u"value"_s );

  QgsSensorThingsProviderConnection::addConnection( u"my connection"_s, data );
  QCOMPARE( QgsSensorThingsProviderConnection::connectionList(), { u"my connection"_s } );

  QCOMPARE( QgsSensorThingsProviderConnection::connection( u"my connection"_s ).url, u"http://testurl"_s );

  // retrieve stored connection
  conn = QgsSensorThingsProviderConnection( u"my connection"_s );
  QCOMPARE( conn.uri(), u"url=http%3A%2F%2Ftesturl&username=my_user&password=my_pw&authcfg=my_auth&http-header:my_header=value"_s );

  // add a second connection
  QgsSensorThingsProviderConnection::Data data2;
  data2.url = u"http://testurl2"_s;
  data2.authCfg = u"my_auth2"_s;
  data2.username = u"my_user2"_s;
  data2.password = u"my_pw2"_s;
  data2.httpHeaders.insert( u"my_header"_s, u"value2"_s );
  // construct connection using encoded uri
  QgsSensorThingsProviderConnection conn2( QgsSensorThingsProviderConnection::encodedUri( data2 ), {} );
  QCOMPARE( conn2.uri(), u"url=http%3A%2F%2Ftesturl2&username=my_user2&password=my_pw2&authcfg=my_auth2&http-header:my_header=value2"_s );
  conn2.store( u"second connection"_s );

  // retrieve stored connections
  QCOMPARE( qgis::listToSet( QgsSensorThingsProviderConnection::connectionList() ), qgis::listToSet( QStringList() << u"my connection"_s << u"second connection"_s ) );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( u"my connection"_s ).url, u"http://testurl"_s );
  QCOMPARE( QgsSensorThingsProviderConnection::connection( u"second connection"_s ).url, u"http://testurl2"_s );

  QgsSensorThingsProviderConnection::setSelectedConnection( u"second connection"_s );
  QCOMPARE( QgsSensorThingsProviderConnection::selectedConnection(), u"second connection"_s );
  QgsSensorThingsProviderConnection::setSelectedConnection( u"my connection"_s );
  QCOMPARE( QgsSensorThingsProviderConnection::selectedConnection(), u"my connection"_s );
}


QGSTEST_MAIN( TestQgsSensorThingsConnection )
#include "testqgssensorthingsconnection.moc"
