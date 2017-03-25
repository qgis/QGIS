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
#include <QString>

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
    void cleanupTestCase() {}
    // will be called before each testfunction is executed.
    void init() {}
    // will be called after every testfunction.
    void cleanup()
    {
      QgsGeoNodeConnection::deleteConnection( mGeoNodeConnectionName );
    }

    // Check if we can create geonode connection from database.
    void testCreation();
  private:
    QString mGeoNodeConnectionName;
    QString mGeoNodeConnectionURL;
};

// Runs before all unit tests
void TestQgsGeoNodeConnection::initTestCase()
{
  std::cout << "CTEST_FULL_OUTPUT" << std::endl;
  mGeoNodeConnectionName = "ThisIsAGeoNodeConnection";
  mGeoNodeConnectionURL = "www.thisisageonodeurl.com";
}

// Test the creation of geonode connection
void TestQgsGeoNodeConnection::testCreation()
{
  QStringList connectionList = QgsGeoNodeConnection::connectionList();
  int numberOfConnection = connectionList.count();

  // Add new GeoNode Connection
  QgsSettings settings;
  settings.setValue( QStringLiteral( "qgis/connections-geonode/%1/url" ).arg( mGeoNodeConnectionName ), mGeoNodeConnectionURL );

  QStringList newConnectionList = QgsGeoNodeConnection::connectionList();
  int newNumberOfConnection = newConnectionList.count();

  // Check the number is increased by 1
  QCOMPARE( numberOfConnection + 1, newNumberOfConnection );

  // Verify if the new connection is created properly
  QVERIFY( newConnectionList.contains( mGeoNodeConnectionName ) );
}

QGSTEST_MAIN( TestQgsGeoNodeConnection )
#include "testqgsgeonodeconnection.moc"
