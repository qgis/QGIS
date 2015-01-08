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
#include <QtTest/QtTest>
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
#include "qgsmultirenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for the different renderers for vector layers.
 */
class TestQgsRenderers : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void singleSymbol();
//    void uniqueValue();
//    void graduatedSymbol();
//    void continuousSymbol();
  private:
    bool mTestHasError;
    bool setQml( QString theType ); //uniquevalue / continuous / single /
    bool imageCheck( QString theType ); //as above
    QgsMapSettings mMapSettings;
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
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...


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
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );


  //
  // Create a line layer that will be used in all tests...
  //
  QString myLinesFileName = mTestDataDir + "lines.shp";
  QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer );
  //
  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers(
    QStringList() << mpPointsLayer->id() << mpPolysLayer->id() << mpLinesLayer->id() );
  mReport += "<h1>Vector Renderer Tests</h1>\n";
}
void TestQgsRenderers::cleanupTestCase()
{
  QgsApplication::exitQgis();

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

// TODO: update tests and enable
/*
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
*/
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

  // mpPointsLayer->extent() was giving wrong extent in QGIS 2.0 (xmin shifted,
  // the same wrong value is reported by ogrinfo). Since QGIS 2.1, the provider
  // gives correct extent. Forced to fixed extend however to avoid problems in future.
  QgsRectangle extent( -118.8888888888887720, 22.8002070393376783, -83.3333333333331581, 46.8719806763287536 );
  mMapSettings.setExtent( extent );
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 15 );
  bool myResultFlag = myChecker.runTest( theTestType, 200 );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsRenderers )
#include "testqgsrenderers.moc"
