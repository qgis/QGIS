/***************************************************************************

   test_qgsserver_interface.cpp
     --------------------------------------
    Date                 : Jan 20 2023
    Copyright            : (C) 2023 by Paul Blottiere
    Email                : blottiere.paul@gmail.com
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

#include "qgsserverplugins.h"
#include "qgsserverinterfaceimpl.h"

/**
 * \ingroup UnitTests
 * Unit tests for the server interface
 */
class TestQgsServerInterface : public QObject
{
    Q_OBJECT

  public:
    TestQgsServerInterface() = default;

  private slots:
    // will be called before the first testfunction is executed.
    void initTestCase();

    // will be called after the last testfunction was executed.
    void cleanupTestCase();

    // will be called before each testfunction is executed
    void init();

    // will be called after every testfunction.
    void cleanup();

    void testPluginsList();
};


void TestQgsServerInterface::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsServerInterface::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsServerInterface::init()
{
}

void TestQgsServerInterface::cleanup()
{
}

void TestQgsServerInterface::testPluginsList()
{
  qputenv( "QGIS_PLUGINPATH", QByteArray( TEST_DATA_DIR ) + "/test_plugin_path" );

  QgsCapabilitiesCache cache;
  QgsServiceRegistry registry;
  QgsServerSettings settings;
  QgsServerInterfaceImpl interface( &cache, &registry, &settings );

  QgsServerPlugins::initPlugins( &interface );
  const QStringList plugins = interface.plugins();

  QCOMPARE( plugins.size(), 2 );
  QCOMPARE( plugins[0], QString( "server_plugin_1" ) );
  QCOMPARE( plugins[1], QString( "server_plugin_2" ) );
}


QGSTEST_MAIN( TestQgsServerInterface )
#include "test_qgsserver_interface.moc"
