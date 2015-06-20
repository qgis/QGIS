/***************************************************************************
     testqgsquickprint.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim  @  linfiniti.com
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
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgsquickprint.h>
//qgis test includes
#include <qgsrenderchecker.h>

/** \ingroup UnitTests
 * This is a unit test for the different renderers for vector layers.
 */
class TestQgsQuickPrint : public QObject
{
    Q_OBJECT
  public:
    TestQgsQuickPrint()
        : mpMapRenderer( 0 )
        , mpPointsLayer( 0 )
        , mpLinesLayer( 0 )
        , mpPolysLayer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void basicMapTest();
  private:
    bool imageCheck( QString theType ); //as above
    QgsMapRenderer * mpMapRenderer;
    QgsMapLayer * mpPointsLayer;
    QgsMapLayer * mpLinesLayer;
    QgsMapLayer * mpPolysLayer;
    QString mTestDataDir;
    QString mReport;
};

void TestQgsQuickPrint::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication.init();
  QgsApplication.initQgis();
  QgsApplication::showSettings();

  //
  //create a point layer that will be used in all tests...
  //
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + QDir::separator();
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer( mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer( mpPolysLayer );

  //
  // Create a line layer that will be used in all tests...
  //
  QString myLinesFileName = mTestDataDir + "lines.shp";
  QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer( mpLinesLayer );
  //
  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mpMapRenderer = new QgsMapRenderer();
  QStringList myLayers;
  myLayers << mpPointsLayer->getLayerID();
  myLayers << mpPolysLayer->getLayerID();
  myLayers << mpLinesLayer->getLayerID();
  mpMapRenderer->setLayerSet( myLayers );
  mpMapRenderer->setExtent( mpPointsLayer->extent() );
  mReport += "<h1>QuickPrint Tests</h1>\n";
}
void TestQgsQuickPrint::cleanupTestCase()
{
  /*
  QString myReportFile = QDir::tempPath() + QDir::separator() + "quickprinttest.html";
  QFile myFile ( myReportFile);
  if ( myFile.open ( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream ( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl("file://"+myReportFile);
  }
  */

}

void TestQgsQuickPrint::basicMapTest()
{
  //make the legends really long so we can test
  //word wrapping
  mpPointsLayer->setLayerName( "This is a very very very long name it should word wrap" );
  mpPolysLayer->setLayerName( "This is a very very very long name it should also word wrap" );
  mpLinesLayer->setLayerName( "This is a very very very very long name it should word wrap" );

  //now print the map
  QgsQuickPrint myQuickPrint;
  myQuickPrint.setMapBackgroundColor( Qt::cyan );
  myQuickPrint.setOutputPdf( QDir::tempPath() + QDir::separator() + "quickprinttest.pdf" );
  myQuickPrint.setMapRenderer( mpMapRenderer );
  myQuickPrint.setTitle( "Map Title" );
  myQuickPrint.setName( "Map Name" );
  myQuickPrint.setCopyright( "Copyright Text" );
  //void setNorthArrow(QString theFileName);
  //void setLogo1(QString theFileName);
  //void setLogo2(QString theFileName);
  myQuickPrint.printMap();
}


//
// Helper functions below
//

bool TestQgsQuickPrint::imageCheck( QString theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mpMapRenderer->setExtent( mpPointsLayer->extent() );
  QString myExpectedImage = mTestDataDir + "expected_" + theTestType + ".png";
  QgsRenderChecker myChecker;
  myChecker.setExpectedImage( myExpectedImage );
  myChecker.setMapRenderer( mpMapRenderer );
  bool myResultFlag = myChecker.runTest( theTestType );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsQuickPrint )
#include "testqgsquickprint.moc"
