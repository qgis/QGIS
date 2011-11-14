/***************************************************************************
     testqgsrenderers.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim @ linfiniti.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
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
//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the different renderers for vector layers.
 */
class TestQgsRenderers: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void singleSymbol();
    void uniqueValue();
    void graduatedSymbol();
    void continuousSymbol();
    void checkClassificationFieldMismatch();
  private:
    bool mTestHasError;
    bool setQml( QString theType ); //uniquevalue / continuous / single /
    bool imageCheck( QString theType ); //as above
    QgsMapRenderer * mpMapRenderer;
    QgsMapLayer * mpPointsLayer;
    QgsMapLayer * mpLinesLayer;
    QgsMapLayer * mpPolysLayer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsRenderers::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath();
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::showSettings();
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance( QgsApplication::pluginPath() );

  //create some objects that will be used in all tests...

  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

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
  myLayers << mpPointsLayer->id();
  myLayers << mpPolysLayer->id();
  myLayers << mpLinesLayer->id();
  mpMapRenderer->setLayerSet( myLayers );
  mReport += "<h1>Vector Renderer Tests</h1>\n";
}
void TestQgsRenderers::cleanupTestCase()
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

}

void TestQgsRenderers::singleSymbol()
{
  mReport += "<h2>Single symbol renderer test</h2>\n";
  QVERIFY( setQml( "single" ) );
  QVERIFY( imageCheck( "single" ) );
}

void TestQgsRenderers::uniqueValue()
{
  mReport += "<h2>Unique value symbol renderer test</h2>\n";
  QVERIFY( setQml( "uniquevalue" ) );
  QVERIFY( imageCheck( "uniquevalue" ) );
}

void TestQgsRenderers::graduatedSymbol()
{
  mReport += "<h2>Graduated symbol renderer test</h2>\n";
  QVERIFY( setQml( "graduated" ) );
  QVERIFY( imageCheck( "graduated" ) );
}

void TestQgsRenderers::continuousSymbol()
{
  mReport += "<h2>Continuous symbol renderer test</h2>\n";
  QVERIFY( setQml( "continuous" ) );
  QVERIFY( imageCheck( "continuous" ) );
}

void TestQgsRenderers::checkClassificationFieldMismatch()
{
  mReport += "<h2>Classification field mismatch test</h2>\n";
  // Here we test to see that a qml created for one layer
  // will raise an error properly if the
  // We will do this by trying to apply the points qml to the polys shpfile
  // it should fail and raise an error
  QString myFileName = mTestDataDir + "points_continuous_symbol.qml";
  bool myStyleFlag = false;
  mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  QVERIFY( !myStyleFlag ); //we are expecting this to have raised an error
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRenderers::setQml( QString theType )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  if ( ! mpPointsLayer->isValid() )
  {
    return false;
  }
  QString myFileName = mTestDataDir + "points_" + theType + "_symbol.qml";
  bool myStyleFlag = false;
  QString error = mpPointsLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
    return false;
  }
  else
  {
    myStyleFlag = false; //ready for next test
  }
  myFileName = mTestDataDir + "polys_" + theType + "_symbol.qml";
  mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    return false;
  }
  else
  {
    myStyleFlag = false; //ready for next test
  }
  myFileName = mTestDataDir + "lines_" + theType + "_symbol.qml";
  mpLinesLayer->loadNamedStyle( myFileName, myStyleFlag );
  return myStyleFlag;
}

bool TestQgsRenderers::imageCheck( QString theTestType )
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

QTEST_MAIN( TestQgsRenderers )
#include "moc_testqgsrenderers.cxx"

