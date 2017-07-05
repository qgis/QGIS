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
      QgsGeoNodeConnection::deleteConnection( mDemoGeoNodeName );
      QgsGeoNodeConnection::deleteConnection( mKartozaGeoNodeQGISServerName );
    }
    // will be called before each testfunction is executed.
    void init() {}
    // will be called after every testfunction.
    void cleanup() {}

    // Check if we can create geonode connection from database.
    void testCreation();
    void testGetLayersOldGeoNode();
    void testGetLayersQGISServer();
    void testGetLayersGeoServer();
    void testGetMaps();
    void testGetWMSUrl();
    void testGetGeoNodeUrl();
  private:
    QString mGeoNodeConnectionName;
    QString mGeoNodeConnectionURL;

    QString mDemoGeoNodeName;
    QString mDemoGeoNodeURL;

    QString mKartozaGeoNodeQGISServerName;
    QString mKartozaGeoNodeQGISServerURL;

    QString mKartozaGeoNodeGeoServerName;
    QString mKartozaGeoNodeGeoServerURL;

    bool mSkipRemoteTest;
};

// Runs before all unit tests
void TestQgsGeoNodeConnection::initTestCase()
{
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  mGeoNodeConnectionName = QStringLiteral( "ThisIsAGeoNodeConnection" );
  mGeoNodeConnectionURL = QStringLiteral( "www.thisisageonodeurl.com" );
  mDemoGeoNodeName = QStringLiteral( "Demo GeoNode" );
  mDemoGeoNodeURL = QStringLiteral( "demo.geonode.org" );
  mKartozaGeoNodeQGISServerName = QStringLiteral( "Staging Kartoza GeoNode QGIS Server" );
  mKartozaGeoNodeQGISServerURL = QStringLiteral( "staging.geonode.kartoza.com" );
  mKartozaGeoNodeGeoServerName = QStringLiteral( "Staging Kartoza GeoNode GeoServer" );
  mKartozaGeoNodeGeoServerURL = QStringLiteral( "staginggs.geonode.kartoza.com" );

  // Change it to skip remote testing
  mSkipRemoteTest = true;

  // Add Demo GeoNode Connection
  QgsSettings settings;

  // Testing real server, demo.geonode.org. Need to be changed later.
  settings.setValue( QgsGeoNodeConnection::pathGeoNodeConnection + QStringLiteral( "/%1/url" ).arg( mDemoGeoNodeName ), mDemoGeoNodeURL );
  // Testing real server, staging.geonode.kartoza.com. Need to be changed later.
  settings.setValue( QgsGeoNodeConnection::pathGeoNodeConnection + QStringLiteral( "/%1/url" ).arg( mKartozaGeoNodeQGISServerName ), mKartozaGeoNodeQGISServerURL );
  // Testing real server, staginggs.geonode.kartoza.com. Need to be changed later.
  settings.setValue( QgsGeoNodeConnection::pathGeoNodeConnection + QStringLiteral( "/%1/url" ).arg( mKartozaGeoNodeGeoServerName ), mKartozaGeoNodeGeoServerURL );
}

// Test the creation of geonode connection
void TestQgsGeoNodeConnection::testCreation()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

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

// Test retrieving layers for old GeoNode (v2.6), only GeoServer backend available
void TestQgsGeoNodeConnection::testGetLayersOldGeoNode()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeConnection geonodeConnection( mDemoGeoNodeName );

  QList<LayerStruct> layers = geonodeConnection.getLayers();

  QVERIFY( layers.count() > 0 );
  // Example how to access it
  LayerStruct layer1 = layers[0];
  QVERIFY( !layer1.typeName.contains( "%3A" ) ); // Check if there is not geonode prefix
  QVERIFY( layer1.wmsURL.length() > 0 );
  QVERIFY( layer1.wfsURL.length() > 0 );
}

// Test retrieving layers for QGIS Server Backend
void TestQgsGeoNodeConnection::testGetLayersQGISServer()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeConnection geonodeConnection( mKartozaGeoNodeQGISServerName );

  QList<LayerStruct> layers = geonodeConnection.getLayers();

  QVERIFY( layers.count() > 0 );
  // Example how to access it
  LayerStruct layer1 = layers[0];
  QVERIFY( !layer1.name.contains( "geonode%3A" ) ); // Check if there is not geonode prefix
  QVERIFY( layer1.xyzURL.length() > 0 );
}

// Test retrieving layers for GeoNode v2.7 with GeoServer backend
void TestQgsGeoNodeConnection::testGetLayersGeoServer()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeConnection geonodeConnection( mKartozaGeoNodeGeoServerName );

  QList<LayerStruct> layers = geonodeConnection.getLayers();

  QVERIFY( layers.count() > 0 );
  // Example how to access it
  LayerStruct layer1 = layers[0];
  QVERIFY( !layer1.name.contains( "geonode%3A" ) ); // Check if there is not geonode prefix
  QVERIFY( layer1.wmsURL.length() > 0 );
  QVERIFY( layer1.wfsURL.length() > 0 );
  QVERIFY( layer1.xyzURL.length() > 0 );
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
  QgsGeoNodeConnection geonodeConnection( mKartozaGeoNodeGeoServerName );

  QStringList WMSUrls = geonodeConnection.serviceUrl( QString( "WMS" ) );
  QVERIFY( WMSUrls.count() > 0 );
}

// Test retrieving WMS Url
void TestQgsGeoNodeConnection::testGetGeoNodeUrl()
{
  QSKIP( "The method no longer used." );

  QgsGeoNodeConnection geonodeConnection( mDemoGeoNodeName );

  QString WMSUrl = geonodeConnection.serviceUrl( QStringLiteral( "WMS" ) )[0];
  qDebug() << WMSUrl;
  QVERIFY( WMSUrl == "http://demo.geonode.org/geoserver/geonode/wms" );

  QString WFSUrl = geonodeConnection.serviceUrl( QStringLiteral( "WFS" ) )[0];
  qDebug() << WFSUrl;
  QVERIFY( WFSUrl == "http://demo.geonode.org/geoserver/geonode/wfs" );

  QgsGeoNodeConnection kartozaGeoNodeConnection( mKartozaGeoNodeQGISServerName );

  QString XYZUrl = kartozaGeoNodeConnection.serviceUrl( QStringLiteral( "XYZ" ) )[0];
  qDebug() << XYZUrl;
  QVERIFY( XYZUrl == "http://geonode.kartoza.com/qgis-server/tiles/LAYERNAME/{z}/{x}/{y}.png" );
}

QGSTEST_MAIN( TestQgsGeoNodeConnection )
#include "testqgsgeonodeconnection.moc"
