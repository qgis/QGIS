/***************************************************************************
     testqgsvectorfilewriter.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:22:54 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
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
#include <QPainter>
#include <QPixmap>
#include <QByteArray>
#include <QBuffer>
#include <QStringList>
#include <QObject>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

#include <iostream>
//qgis includes...
#include <qgsmaprender.h> 
#include <qgsmaplayer.h> 
#include <qgsvectorlayer.h> 
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>

/** \ingroup UnitTests
 * This is a unit test for the different renderers for vector layers.
 */
class TestQgsRenderers: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(){};// will be called before each testfunction is executed.
    void cleanup(){};// will be called after every testfunction.

    void singleSymbol();
    void uniqueValue();
    void graduatedSymbol();
    void continuousSymbol();
  private:
    bool setQml (QString theType); //uniquevalue / continuous / single / 
    bool imageCheck(QString theType); //as above
    QgsMapRender * mpMapRenderer;
    QgsMapLayer * mpPointsLayer;
    QgsMapLayer * mpLinesLayer;
    QgsMapLayer * mpPolysLayer;
    QString mTestDataDir;
    QString mReport;
};

void TestQgsRenderers::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(qgisPath, TRUE);
#ifdef Q_OS_LINUX
  QgsApplication::setPkgDataPath(qgisPath + "/../share/qgis");
  QgsApplication::setPluginPath(qgisPath + "/../lib/qgis");
#endif
  // Instantiate the plugin directory so that providers are loaded
  QgsProviderRegistry::instance(QgsApplication::pluginPath());

  //create some objects that will be used in all tests...

  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

  //
  //create a point layer that will be used in all tests...
  //
  QString myDataDir (TEST_DATA_DIR); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + QDir::separator();
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo ( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer ( myPointFileInfo.filePath(),
            myPointFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpPointsLayer);

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo ( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer ( myPolyFileInfo.filePath(),
            myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpPolysLayer);
  
  //
  // Create a line layer that will be used in all tests...
  //
  QString myLinesFileName = mTestDataDir + "lines.shp";
  QFileInfo myLineFileInfo ( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer ( myLineFileInfo.filePath(),
            myLineFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpLinesLayer);
  //
  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mpMapRenderer = new QgsMapRender();
  QStringList myLayers;
  myLayers << mpPointsLayer->getLayerID();
  myLayers << mpPolysLayer->getLayerID();
  myLayers << mpLinesLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
}
void TestQgsRenderers::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "renderertest.html";
  QFile myFile ( myReportFile);
  if ( myFile.open ( QIODevice::WriteOnly ) )
  {
    QTextStream myQTextStream ( &myFile );
    myQTextStream << mReport;
    myFile.close();
    QDesktopServices::openUrl("file://"+myReportFile);
  }
  
}

void TestQgsRenderers::singleSymbol()
{
  mReport+= "<h1>Single symbol renderer test</h1>\n";
  QVERIFY ( setQml("single") );
  QVERIFY ( imageCheck("single"));
}

void TestQgsRenderers::uniqueValue()
{
  mReport+= "<h1>Unique value symbol renderer test</h1>\n";
  QVERIFY ( setQml("uniquevalue") );
  QVERIFY ( imageCheck("uniquevalue"));
}

void TestQgsRenderers::graduatedSymbol()
{
  mReport+= "<h1>Graduated symbol renderer test</h1>\n";
  QVERIFY ( setQml("graduated") );
  QVERIFY ( imageCheck("graduated"));
}

void TestQgsRenderers::continuousSymbol()
{
  mReport+= "<h1>Continuous symbol renderer test</h1>\n";
  QVERIFY ( setQml("continuous") );
  QVERIFY ( imageCheck("continuous"));
}
//
// Private helper functions not called directly by CTest
//

bool TestQgsRenderers::setQml (QString theType)
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  if (! mpPointsLayer->isValid() )
  {
    return false;
  }
  QString myFileName = mTestDataDir + "points_" + theType + "_symbol.qml";
  bool myStyleFlag=false;
  mpPointsLayer->loadNamedStyle ( myFileName , myStyleFlag );
  if (!myStyleFlag)
  {
    return false;
  }
  else
  {
    myStyleFlag=false; //ready for next test
  }
  myFileName = mTestDataDir + "polys_" + theType + "_symbol.qml";
  mpPolysLayer->loadNamedStyle ( myFileName , myStyleFlag );
  if (!myStyleFlag)
  {
    return false;
  }
  else
  {
    myStyleFlag=false; //ready for next test
  }
  myFileName = mTestDataDir + "lines_" + theType + "_symbol.qml";
  mpLinesLayer->loadNamedStyle ( myFileName , myStyleFlag );
  return myStyleFlag;
}

bool TestQgsRenderers::imageCheck(QString theTestType)
{
  //
  // Now render our layers onto a pixmap 
  //
  QPixmap myPixmap( 800,800 );
  myPixmap.fill ( QColor ( "#98dbf9" ) );
  QPainter myPainter( &myPixmap );
  mpMapRenderer->setOutputSize( QSize ( 800,800 ),72 ); 
  mpMapRenderer->setExtent(mpPointsLayer->extent());
  mpMapRenderer->render( &myPainter );
  myPainter.end();
  //
  // Save the pixmap to disk so the user can make a 
  // visual assessment if needed
  //
  myPixmap.save (QDir::tempPath() + QDir::separator() + theTestType + ".png");
  //
  // Load the expected result pixmap
  //
  QPixmap myExpectedPixmap (mTestDataDir + "expected_" + theTestType + ".png");
  mReport+= "<table>"
    "<tr><td>Test Result:</td><td>Expected Result:</td></tr>\n"
    "<tr><td><img src=\"" +
    QDir::tempPath() + QDir::separator() + theTestType + ".png" +
    "\"></td>\n<td><img src=\"" +
    mTestDataDir + "expected_" + theTestType + ".png" +
    "\"></td></tr></table>\n";
  //
  // Now load the renderered image and the expected image
  // each into a byte array, and then iterate through them
  // counting how many dissimilar pixel values there are
  //
  QByteArray myResultBytes;
  QBuffer myResultBuffer(&myResultBytes);
  myResultBuffer.open(QIODevice::WriteOnly);
  myPixmap.save(&myResultBuffer, "PNG"); // writes pixmap into bytes in PNG format 

  QByteArray myExpectedBytes;
  QBuffer myExpectedBuffer(&myExpectedBytes);
  myExpectedBuffer.open(QIODevice::WriteOnly);
  myExpectedPixmap.save(&myExpectedBuffer, "PNG"); // writes pixmap into bytes in PNG format 

  if (myExpectedBytes.size() != myResultBytes.size())
  {
    qDebug ("Test image and result image for " + theTestType + " are different - FAILING!");
    return false;
  }
  int myMismatchCount = 0;
  for (int i = 0; i < myExpectedBytes.size(); ++i) 
  {
    if (myExpectedBytes.at(i) != myResultBytes.at(i))
    {
      ++myMismatchCount;
    }
  }
  qDebug (QString::number(myMismatchCount).toLocal8Bit() + "/" +
          QString::number(myExpectedBytes.size()).toLocal8Bit() + 
    " bytes mismatched");; 

  if ( myMismatchCount==0 )
  {
    return true;
  }
  else
  {
    return false;
  }
}

QTEST_MAIN(TestQgsRenderers)
#include "moc_testqgsrenderers.cxx"

