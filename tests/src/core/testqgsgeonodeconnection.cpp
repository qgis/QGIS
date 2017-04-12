/***************************************************************************
 testqgsgeonodeconnection.cpp
 --------------------------------------
  Date : Saturday, 25 March 2017
  Copyright: (C) 2017
  Email: ismail@kartoza.com
 ***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgstest.h"
#include <QtTest/QtTest>

#include <QtTest/QSignalSpy>
#include <QString>
#include <QMultiMap>

//#include "qgis_core.h"
#include "qgsgeonodeconnection.h"
#include "qgssettings.h"

/** \ingroup UnitTests
 * This is a unit test for the QgsGeoConnection class.
 */

class TestQgsGeoNodeConnection: public QObject
{
    Q_OBJECT

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase();
    // will be called after the last testfunction was executed.
    void cleanupTestCase()
    {
      QgsGeoNodeConnection::deleteConnection( mGeoNodeConnectionName );
      QgsGeoNodeConnection::deleteConnection( "Demo GeoNode" );
    }
    // will be called before each testfunction is executed.
    void init() {}
    // will be called after every testfunction.
    void cleanup() {}

    // Check if we can create geonode connection from database.
    void testCreation();
    void testGetLayers();
    void testGetMaps();
    void testGetWMSUrl();
  private:
    QString mGeoNodeConnectionName;
    QString mGeoNodeConnectionURL;
    QString mDemoGeoNodeName;
    bool mSkipRemoteTest;
};

// Runs before all unit tests
void TestQgsGeoNodeConnection::initTestCase()
{
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  mGeoNodeConnectionName = "ThisIsAGeoNodeConnection";
  mGeoNodeConnectionURL = "www.thisisageonodeurl.com";
  mDemoGeoNodeName = "Demo GeoNode";
  // Change it to skip remote testing
  mSkipRemoteTest = true;

  // Add Demo GeoNode Connection
  QgsSettings settings;

  // Testing real server, demo.geonode.org. Need to be changed later.
  settings.setValue( QgsGeoNodeConnection::pathGeoNodeConnection + QStringLiteral( "/%1/url" ).arg( mDemoGeoNodeName ), "demo.geonode.org" );
}

// Test the creation of geonode connection
void TestQgsGeoNodeConnection::testCreation()
{
  QStringList connectionList = QgsGeoNodeConnection::connectionList();
  int numberOfConnection = connectionList.count();
  // Verify if the demo.geonode.org is created properly
  QVERIFY( connectionList.contains( mDemoGeoNodeName ) );
  QVERIFY( !connectionList.contains( mGeoNodeConnectionName ) );

  // Add new GeoNode Connection
  QgsSettings settings;

  settings.setValue( QgsGeoNodeConnection::pathGeoNodeConnection + QStringLiteral( "/%1/url" ).arg( mGeoNodeConnectionName ), mGeoNodeConnectionURL );

  QStringList newConnectionList = QgsGeoNodeConnection::connectionList();
  int newNumberOfConnection = newConnectionList.count();

  // Check the number is increased by 1
  QCOMPARE( numberOfConnection + 1, newNumberOfConnection );

  // Verify if the new connection is created properly
  QVERIFY( newConnectionList.contains( mGeoNodeConnectionName ) );
}

// Test retrieving layers
void TestQgsGeoNodeConnection::testGetLayers()
{
  if ( !mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeConnection geonodeConnection( mDemoGeoNodeName );

  QVariantList layers = geonodeConnection.getLayers();

  QVERIFY( layers.count() > 0 );
  // Example how to access it
  QVariantMap layer1 = layers[0].toMap();  // Need to convert to map
  QList<QString> keys = layer1.keys();
  QVERIFY( keys.indexOf( "title" ) != -1 ); // Check if title is in the keys
  QVERIFY( keys.indexOf( "name" ) != -1 ); // Check if title is in the keys
  QVERIFY( !layer1["name"].toString().contains( "geonode%3A" ) ); // Check if there is not geonode prefix
}

// Test retrieving maps
void TestQgsGeoNodeConnection::testGetMaps()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }
  QgsGeoNodeConnection geonodeConnection( mDemoGeoNodeName );

  QVariantList layers = geonodeConnection.getMaps();

  QVERIFY( layers.count() > 0 );
  // Example how to access it
  QVariantMap layer1 = layers[0].toMap();  // Need to convert to map
  QList<QString> keys = layer1.keys();
  QVERIFY( keys.indexOf( "title" ) != -1 ); // Check if title is in the keys
}

// Test retrieving WMS Url
void TestQgsGeoNodeConnection::testGetWMSUrl()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }
  QgsGeoNodeConnection geonodeConnection( mDemoGeoNodeName );

  QString layerID = "1c863918-f9e8-11e6-ab35-0e23392a5c01";
  QString WMSUrl = geonodeConnection.serviceUrl( layerID, QString( "WMS" ) );
  std::cout << WMSUrl.toStdString();
  QVERIFY( WMSUrl == "http://demo.geonode.org/geoserver/geonode/wms" );
}

QGSTEST_MAIN( TestQgsGeoNodeConnection )
#include "testqgsgeonodeconnection.moc"
