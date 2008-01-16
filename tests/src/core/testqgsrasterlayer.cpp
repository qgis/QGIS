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


//qgis includes...
#include <qgsrasterlayer.h> 
#include <qgsrasterbandstats.h> 
#include <qgsmaplayerregistry.h> 
#include <qgsapplication.h>
#include <qgsmaprender.h> 

/** \ingroup UnitTests
 * This is a unit test for the QgsRasterLayer class.
 */
class TestQgsRasterLayer: public QObject
{
  Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase(){};// will be called after the last testfunction was executed.
    void init(){};// will be called before each testfunction is executed.
    void cleanup(){};// will be called after every testfunction.

    void isValid();
    void pseudoColor();
    void checkDimensions(); 
  private:
    void render(QString theFileName);
    QgsRasterLayer * mpRasterLayer;
    QgsMapRender * mpMapRenderer;
};

void TestQgsRasterLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QString qgisPath = QCoreApplication::applicationDirPath ();
  QgsApplication::setPrefixPath(qgisPath, TRUE);
#ifdef Q_OS_LINUX
  QgsApplication::setPkgDataPath(qgisPath + "/../share/qgis");
#endif
  //create some objects that will be used in all tests...

  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

  //create a raster layer that will be used in all tests...
  QString myFileName (TEST_DATA_DIR); //defined in CmakeLists.txt
  myFileName = myFileName + QDir::separator() + "tenbytenraster.asc";
  QFileInfo myRasterFileInfo ( myFileName );
  mpRasterLayer = new QgsRasterLayer ( myRasterFileInfo.filePath(),
            myRasterFileInfo.completeBaseName() );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayer(mpRasterLayer);
  // add the test layer to the maprender
  mpMapRenderer = new QgsMapRender();
  QStringList myLayers;
  myLayers << mpRasterLayer->getLayerID();
  mpMapRenderer->setLayerSet(myLayers);
}

void TestQgsRasterLayer::isValid()
{
  QVERIFY ( mpRasterLayer->isValid() );
  render("raster_test.png");
}

void TestQgsRasterLayer::pseudoColor()
{
   mpRasterLayer->setDrawingStyle(QgsRasterLayer::SINGLE_BAND_PSEUDO_COLOR);
   mpRasterLayer->setColorShadingAlgorithm(QgsRasterLayer::PSEUDO_COLOR);  
   mpRasterLayer->setContrastEnhancementAlgorithm(
          QgsContrastEnhancement::STRETCH_TO_MINMAX, false);
   mpRasterLayer->setMinimumValue(mpRasterLayer->getGrayBandName(),0.0, false);
   mpRasterLayer->setMaximumValue(mpRasterLayer->getGrayBandName(),0.0);
   render("raster_pseudo_test.png");
}

void TestQgsRasterLayer::checkDimensions()
{
   QVERIFY ( mpRasterLayer->getRasterXDim() == 10 );
   QVERIFY ( mpRasterLayer->getRasterYDim() == 10 );
   // regression check for ticket #832
   // note getRasterBandStats call is base 1
   QVERIFY ( mpRasterLayer->getRasterBandStats(1).elementCount == 100 );
}

void TestQgsRasterLayer::render(QString theFileName)
{

  //
  // Now render our layers onto a pixmap 
  //
  QPixmap myPixmap( 100,100 );
  myPixmap.fill ( QColor ( "#98dbf9" ) );
  QPainter myPainter( &myPixmap );
  mpMapRenderer->setOutputSize( QSize ( 100,100 ),72 ); 
  mpMapRenderer->setExtent(mpRasterLayer->extent());
  qDebug ("Extents set to:");
  qDebug (mpRasterLayer->extent().stringRep());
  QTime myTime;
  myTime.start();
  mpMapRenderer->render( &myPainter );
  qDebug ("Elapsed time in ms for render job: " + 
      QString::number ( myTime.elapsed() ).toLocal8Bit()); 
  myPainter.end();
  //
  // Save the pixmap to disk so the user can make a 
  // visual assessment if needed
  //
  myPixmap.save (QDir::tempPath() + QDir::separator() + theFileName);
}

QTEST_MAIN(TestQgsRasterLayer)
#include "moc_testqgsrasterlayer.cxx"

