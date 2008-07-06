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
#include <QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QObject>
#include <iostream>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QPainter>
#include <QSettings>
#include <QTime>
#include <QDesktopServices>


//qgis includes...
#include <qgsrasterlayer.h> 
#include <qgsrasterbandstats.h> 
#include <qgsmaplayerregistry.h> 
#include <qgsapplication.h>
#include <qgsmaprender.h> 

//qgis unit test includes
#include <qgsrenderchecker.h>


/** \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsRasterLayer: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(){};// will be called before each testfunction is executed.
    void cleanup(){};// will be called after every testfunction.

    void isValid();
    void pseudoColor();
    void landsatBasic();
    void landsatBasic875Qml();
    void checkDimensions(); 
  private:
    bool render(QString theFileName);
    bool setQml (QString theType);
    QString mTestDataDir;
    QgsRasterLayer * mpRasterLayer;
    QgsRasterLayer * mpLandsatRasterLayer;
    QgsMapRender * mpMapRenderer;
    QString mReport;
};

//runs before all tests
void TestQgsRasterLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(INSTALL_PREFIX, true);
  QgsApplication::showSettings();
  //create some objects that will be used in all tests...
  //create a raster layer that will be used in all tests...
  mTestDataDir = QString(TEST_DATA_DIR) + QDir::separator(); //defined in CmakeLists.txt
  QString myFileName = mTestDataDir + "tenbytenraster.asc";
  QString myLandsatFileName = mTestDataDir + "landsat.tif";
  QFileInfo myRasterFileInfo ( myFileName );
  mpRasterLayer = new QgsRasterLayer ( myRasterFileInfo.filePath(),
            myRasterFileInfo.completeBaseName() );
  QFileInfo myLandsatRasterFileInfo ( myLandsatFileName );
  mpLandsatRasterLayer = new QgsRasterLayer ( myLandsatRasterFileInfo.filePath(),
            myLandsatRasterFileInfo.completeBaseName() );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpRasterLayer);
  QgsMapLayerRegistry::instance()->addMapLayer(mpLandsatRasterLayer);
  // add the test layer to the maprender
  mpMapRenderer = new QgsMapRender();
  QStringList myLayers;
  myLayers << mpRasterLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
  mReport += "<h1>Raster Layer Tests</h1>\n";
}
//runs after all tests
void TestQgsRasterLayer::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "rastertest.html";
  QFile myFile ( myReportFile);
  if ( myFile.open ( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream ( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl("file://"+myReportFile);
  }
  
}

void TestQgsRasterLayer::isValid()
{
  QVERIFY ( mpRasterLayer->isValid() );
  mpMapRenderer->setExtent(mpRasterLayer->extent());
  QVERIFY ( render("raster") );
}

void TestQgsRasterLayer::pseudoColor()
{
  mpRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
  mpRasterLayer->setColorShadingAlgorithm(QgsRasterLayer::PSEUDO_COLOR);  
  mpRasterLayer->setContrastEnhancementAlgorithm(
      QgsContrastEnhancement::STRETCH_TO_MINMAX, false);
  mpRasterLayer->setMinimumValue(mpRasterLayer->getGrayBandName(),0.0, false);
  mpRasterLayer->setMaximumValue(mpRasterLayer->getGrayBandName(),10.0);
  mpMapRenderer->setExtent(mpRasterLayer->extent());
  QVERIFY(render("raster_pseudo"));
}

void TestQgsRasterLayer::landsatBasic()
{
  QStringList myLayers;
  myLayers << mpLandsatRasterLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
  mpMapRenderer->setExtent(mpLandsatRasterLayer->extent());
  QVERIFY(render("landsat_basic"));
}
void TestQgsRasterLayer::landsatBasic875Qml()
{
  //a qml that orders the rgb bands as 8,7,5
  QStringList myLayers;
  myLayers << mpLandsatRasterLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
  mpMapRenderer->setExtent(mpLandsatRasterLayer->extent());
  QVERIFY(setQml("875"));
  QVERIFY(render("landsat_875"));
}
void TestQgsRasterLayer::checkDimensions()
{
   QVERIFY ( mpRasterLayer->getRasterXDim() == 10 );
   QVERIFY ( mpRasterLayer->getRasterYDim() == 10 );
   // regression check for ticket #832
   // note getRasterBandStats call is base 1
   QVERIFY ( mpRasterLayer->getRasterBandStats(1).elementCount == 100 );
}

bool TestQgsRasterLayer::render(QString theTestType)
{
  mReport += "<h2>" + theTestType + "</h2>\n";
  QString myDataDir (TEST_DATA_DIR); //defined in CmakeLists.txt
  QString myTestDataDir = myDataDir + QDir::separator();
  QgsRenderChecker myChecker;
  myChecker.setExpectedImage ( myTestDataDir + "expected_" + theTestType + ".png" );
  myChecker.setMapRenderer ( mpMapRenderer );
  bool myResultFlag = myChecker.runTest(theTestType);
  mReport += "\n\n\n" + myChecker.report();
  return myResultFlag;
}

bool TestQgsRasterLayer::setQml (QString theType)
{
  //load a qml style and apply to our layer
  // huh? this is failing but shouldnt!
  //if (! mpLandsatRasterLayer->isValid() )
  //{
  //  qDebug(" **** setQml -> mpLandsatRasterLayer is invalid");
  //  return false;
  //}
  QString myFileName = mTestDataDir + "landsat_" + theType + ".qml";
  bool myStyleFlag=false;
  mpLandsatRasterLayer->loadNamedStyle ( myFileName , myStyleFlag );
  if (!myStyleFlag)
  {
    qDebug(" **** setQml -> mpLandsatRasterLayer is invalid");
    qDebug("Qml File :" +  myFileName.toLocal8Bit());
  }
  return myStyleFlag;
}

QTEST_MAIN(TestQgsRasterLayer)
#include "moc_testqgsrasterlayer.cxx"

