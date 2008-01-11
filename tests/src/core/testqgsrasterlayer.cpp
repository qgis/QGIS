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

//qgis includes...
#include <qgsrasterlayer.h> 
#include <qgsrasterbandstats.h> 
#include <qgsapplication.h>

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
    void checkDimensions(); 
  private:
    QgsRasterLayer * mpLayer;
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
  mpLayer = new QgsRasterLayer ( myRasterFileInfo.filePath(),
            myRasterFileInfo.completeBaseName() );
}

void TestQgsRasterLayer::isValid()
{
  QVERIFY ( mpLayer->isValid() );
}
void TestQgsRasterLayer::checkDimensions()
{
   QVERIFY ( mpLayer->getRasterXDim() == 10 );
   QVERIFY ( mpLayer->getRasterYDim() == 10 );
   // regression check for ticket #832
   // note getRasterBandStats call is base 1
   QVERIFY ( mpLayer->getRasterBandStats(1).elementCount == 100 );
}

QTEST_MAIN(TestQgsRasterLayer)
#include "moc_testqgsrasterlayer.cxx"

