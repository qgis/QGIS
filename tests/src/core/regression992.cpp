/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Frida  Nov 23  2007
    Copyright            : (C) 2007 by Tim Sutton
    Email                : tim@linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <iostream>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QTime>
#include <QDesktopServices>

//qgis includes...
#include <qgsrasterlayer.h>
#include <qgsrasterbandstats.h>
#include <qgsmaplayerregistry.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmapsettings.h>

//qgis unit test includes
#include <qgsrenderchecker.h>


/** \ingroup UnitTests
 * This is a regression test for ticket #992.
 */
class Regression992: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void regression992();
  private:
    bool render( QString theFileName );
    QString mTestDataDir;
    QgsRasterLayer *mpRasterLayer;
    QString mReport;
};

//runs before all tests
void Regression992::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  // QgsApplication::skipGdalDriver( "JP2ECW" );
  // QgsApplication::skipGdalDriver( "JP2MrSID" );

  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QString( TEST_DATA_DIR ) + QDir::separator(); //defined in CMakeLists.txt
  QString myFileName = mTestDataDir + "rgbwcmyk01_YeGeo.jp2";
  QFileInfo myRasterFileInfo( myFileName );
  mpRasterLayer = new QgsRasterLayer( myRasterFileInfo.filePath(),
                                      myRasterFileInfo.completeBaseName() );
  if ( ! mpRasterLayer->isValid() )
  {
    QSKIP( "This test requires the JPEG2000 GDAL driver", SkipAll );
  }
  else
  {
    qDebug() << mpRasterLayer->dataProvider()->metadata();
  }

  // Register the layer with the registry
  QList<QgsMapLayer *> myList;
  myList << mpRasterLayer;
  QgsMapLayerRegistry::instance()->addMapLayers( myList );
  // add the test layer to the maprender
  mReport += "<h1>Regression 992 Test</h1>\n";
  mReport += "<p>See <a href=\"http://hub.qgis.org/issues/992\">"
             "redmine ticket 992</a> for more details.</p>";
}
//runs after all tests
void Regression992::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }

  QgsApplication::exitQgis();
}

void Regression992::regression992()
{
  QgsMapSettings settings;
  settings.setExtent( mpRasterLayer->extent() );
  settings.setLayers( QStringList() << mpRasterLayer->id() );
  QgsRenderChecker myChecker;
  myChecker.setMapSettings( settings );
  myChecker.setControlName( "expected_rgbwcmyk01_YeGeo.jp2" );
  // allow up to 300 mismatched pixels
  bool myResultFlag = myChecker.runTest( "regression992", 400 );
  mReport += "\n\n\n" + myChecker.report();
  QVERIFY( myResultFlag );
}

QTEST_MAIN( Regression992 )
#include "regression992.moc"
