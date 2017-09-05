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

#include "qgsgeonodeconnection.h"
#include "qgssettings.h"
#include "qgsgeonoderequest.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

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
      QgsGeoNodeConnectionUtils::deleteConnection( mGeoNodeConnectionName );
      QgsGeoNodeConnectionUtils::deleteConnection( mDemoGeoNodeName );
      QgsGeoNodeConnectionUtils::deleteConnection( mKartozaGeoNodeQGISServerName );
    }
    // will be called before each testfunction is executed.
    void init() {}
    // will be called after every testfunction.
    void cleanup() {}

    // Check if we can create geonode connection from database.
    void testCreation();

    // Test Layer API
    void testLayerAPI();

    // Test Style API
    void testStyleAPI();

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
  mGeoNodeConnectionURL = QStringLiteral( "http://www.thisisageonodeurl.com" );
  mDemoGeoNodeName = QStringLiteral( "Demo GeoNode" );
  mDemoGeoNodeURL = QStringLiteral( "http://demo.geonode.org" );
  mKartozaGeoNodeQGISServerName = QStringLiteral( "Staging Kartoza GeoNode QGIS Server" );
  mKartozaGeoNodeQGISServerURL = QStringLiteral( "http://staging.geonode.kartoza.com" );
  mKartozaGeoNodeGeoServerName = QStringLiteral( "Staging Kartoza GeoNode GeoServer" );
  mKartozaGeoNodeGeoServerURL = QStringLiteral( "http://staginggs.geonode.kartoza.com" );

  // Change it to skip remote testing
  mSkipRemoteTest = true;

  // Add Demo GeoNode Connection
  QgsSettings settings;

  // Testing real server, demo.geonode.org. Need to be changed later.
  settings.setValue( QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + QStringLiteral( "/%1/url" ).arg( mDemoGeoNodeName ), mDemoGeoNodeURL, QgsSettings::Providers );
  // Testing real server, staging.geonode.kartoza.com. Need to be changed later.
  settings.setValue( QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + QStringLiteral( "/%1/url" ).arg( mKartozaGeoNodeQGISServerName ), mKartozaGeoNodeQGISServerURL, QgsSettings::Providers );
  // Testing real server, staginggs.geonode.kartoza.com. Need to be changed later.
  settings.setValue( QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + QStringLiteral( "/%1/url" ).arg( mKartozaGeoNodeGeoServerName ), mKartozaGeoNodeGeoServerURL, QgsSettings::Providers );
}

// Test the creation of geonode connection
void TestQgsGeoNodeConnection::testCreation()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QStringList connectionList = QgsGeoNodeConnectionUtils::connectionList();
  int numberOfConnection = connectionList.count();
  // Verify if the demo.geonode.org is created properly
  QVERIFY( connectionList.contains( mDemoGeoNodeName ) );
  QVERIFY( !connectionList.contains( mGeoNodeConnectionName ) );

  // Add new GeoNode Connection
  QgsSettings settings;

  settings.setValue( QgsGeoNodeConnectionUtils::pathGeoNodeConnection() + QStringLiteral( "/%1/url" ).arg( mGeoNodeConnectionName ), mGeoNodeConnectionURL, QgsSettings::Providers );

  QStringList newConnectionList = QgsGeoNodeConnectionUtils::connectionList();
  int newNumberOfConnection = newConnectionList.count();

  // Check the number is increased by 1
  QCOMPARE( numberOfConnection + 1, newNumberOfConnection );

  // Verify if the new connection is created properly
  QVERIFY( newConnectionList.contains( mGeoNodeConnectionName ) );
}

// Test Layer API
void TestQgsGeoNodeConnection::testLayerAPI()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeRequest geonodeRequest( mKartozaGeoNodeQGISServerURL, true );
  QList<QgsServiceLayerDetail> layers = geonodeRequest.getLayers();
  QString msg = QStringLiteral( "Number of layers: %1" ).arg( layers.count() );
  QgsDebugMsg( msg );
  QVERIFY( layers.count() > 0 );
}

// Test Style API
void TestQgsGeoNodeConnection::testStyleAPI()
{
  if ( mSkipRemoteTest )
  {
    QSKIP( "Skip remote test for faster testing" );
  }

  QgsGeoNodeRequest geonodeRequest( mKartozaGeoNodeQGISServerURL, true );
  QgsGeoNodeStyle defaultStyle = geonodeRequest.getDefaultStyle( QStringLiteral( "airports" ) );
  QVERIFY( !defaultStyle.name.isEmpty() );
  QVERIFY( defaultStyle.body.toString().startsWith( QStringLiteral( "<qgis" ) ) );
  QVERIFY( defaultStyle.body.toString().contains( QStringLiteral( "</qgis>" ) ) );

  QgsGeoNodeStyle geoNodeStyle = geonodeRequest.getStyle( "76" );
  QVERIFY( !geoNodeStyle.name.isEmpty() );
  QVERIFY( geoNodeStyle.body.toString().startsWith( QStringLiteral( "<qgis" ) ) );
  QVERIFY( geoNodeStyle.body.toString().contains( QStringLiteral( "</qgis>" ) ) );

  QList<QgsGeoNodeStyle> geoNodeStyles = geonodeRequest.getStyles( QStringLiteral( "airports" ) );
  QgsDebugMsg( geoNodeStyles.count() );
  QVERIFY( geoNodeStyles.count() == 2 );

}

QGSTEST_MAIN( TestQgsGeoNodeConnection )
#include "testqgsgeonodeconnection.moc"
