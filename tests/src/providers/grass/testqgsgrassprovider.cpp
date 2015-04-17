/***************************************************************************
     testqgsgrassprovider.cpp
     --------------------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>

#include <QApplication>
#include <QObject>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include <qgsapplication.h>
#include <qgsgrass.h>
#include <qgsproviderregistry.h>
#include <qgsvectordataprovider.h>

extern "C"
{
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <grass/version.h>
}

#define TINY_VALUE  std::numeric_limits<double>::epsilon() * 20

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsGrassProvider: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void vectorLayers();
  private:
    void reportRow( QString message );
    QString mGisdbase;
    QString mLocation;
    QString mReport;
};


void TestQgsGrassProvider::reportRow( QString message )
{
  mReport += message + "<br>";
}

//runs before all tests
void TestQgsGrassProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  // QgsApplication::initQgis() calls QgsProviderRegistry::instance() which registers providers.
  // Because providers are linked in build directory with rpath, it will also try to load GRASS providers
  // in version different form which we are testing here so it loads also GRASS libs in different version
  // and it results in segfault when __do_global_dtors_aux() is called.
  // => we must not call QgsApplication::initQgis()
  //QgsApplication::initQgis();
  QString mySettings = QgsApplication::showSettings();
  mySettings = mySettings.replace( "\n", "<br />" );
  mReport += QString( "<h1>GRASS %1 provider tests</h1>\n" ).arg( GRASS_BUILD_VERSION );
  mReport += "<p>" + mySettings + "</p>";

  QgsGrass::init();

  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mGisdbase = QString( TEST_DATA_DIR ) + "/grass";
  mLocation = "wgs84";
  reportRow( "mGisdbase: " + mGisdbase );
  reportRow( "mLocation: " + mLocation );
  qDebug() << "mGisdbase = " << mGisdbase << " mLocation = " << mLocation;
}

//runs after all tests
void TestQgsGrassProvider::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  //QgsApplication::exitQgis();
}

void TestQgsGrassProvider::vectorLayers()
{
  QString mapset = QString( "test%1" ).arg( GRASS_BUILD_VERSION );
  QString mapName = "test";
  QStringList expectedLayers;
  expectedLayers << "1_point" << "2_line" << "3_polygon";

  reportRow( "" );
  reportRow( "QgsGrass::vectorLayers test" );
  reportRow( "mapset: " + mapset );
  reportRow( "mapName: " + mapName );
  reportRow( "expectedLayers: " + expectedLayers.join( ", " ) );

  bool ok = true;
  G_TRY
  {
    QStringList layers = QgsGrass::vectorLayers( mGisdbase, mLocation, mapset, mapName );
    reportRow( "layers:" + layers.join( ", " ) );

    foreach ( QString expectedLayer, expectedLayers )
    {
      if ( !layers.contains( expectedLayer ) )
      {
        ok = false;
        reportRow( "ERROR: expected layer '" + expectedLayer + "' missing" );
      }
    }
  }
  G_CATCH( QgsGrass::Exception &e )
  {
    ok = false;
    reportRow( QString( "ERROR: %1" ).arg( e.what() ) );
  }

  QVERIFY( ok );
  reportRow( "OK" );
}

QTEST_MAIN( TestQgsGrassProvider )
#include "testqgsgrassprovider.moc"
