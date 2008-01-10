/***************************************************************************
  testqgsapplication.cpp
  --------------------------------------
Date                 : Sun Sep 16 12:22:49 AKDT 2007
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
#include <iostream>

#include <QPixmap>

#include <qgsapplication.h>

//header for class being tested
#include <qgsapplication.h>

class TestQgsApplication: public QObject
{
  Q_OBJECT;
  private slots:
    void getPaths();
    void checkTheme();
  private:
    QString getQgisPath();
};

QString TestQgsApplication::getQgisPath()
{
#ifdef Q_OS_LINUX 
  QString qgisPath = QCoreApplication::applicationDirPath () + "/../";
#else //mac and win
  QString qgisPath = QCoreApplication::applicationDirPath () ;
#endif
  return qgisPath;
}

void TestQgsApplication::getPaths()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::setPrefixPath(getQgisPath(), TRUE);
#ifdef Q_OS_LINUX
  QgsApplication::setPkgDataPath(getQgisPath() + "/../share/qgis");
  QgsApplication::setPluginPath(getQgisPath() + "/../lib/qgis");
#endif
  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;

};

void TestQgsApplication::checkTheme()
{
  QgsApplication::setPrefixPath(getQgisPath(), TRUE);
#ifdef Q_OS_LINUX
  QgsApplication::setPkgDataPath(getQgisPath() + "/../share/qgis");
  QgsApplication::setPluginPath(getQgisPath() + "/../lib/qgis");
#endif
  std::cout << "Prefix  PATH: " << QgsApplication::prefixPath().toLocal8Bit().data() << std::endl;
  std::cout << "Plugin  PATH: " << QgsApplication::pluginPath().toLocal8Bit().data() << std::endl;
  std::cout << "PkgData PATH: " << QgsApplication::pkgDataPath().toLocal8Bit().data() << std::endl;
  std::cout << "User DB PATH: " << QgsApplication::qgisUserDbFilePath().toLocal8Bit().data() << std::endl;
  QString myIconPath = QgsApplication::themePath();
  QPixmap myPixmap;
  myPixmap.load(myIconPath+"/mIconProjectionDisabled.png");
  qDebug("Checking if a theme icon exists:");
  qDebug(myIconPath.toLocal8Bit()+"/mIconProjectionDisabled.png");
  QVERIFY(!myPixmap.isNull());

};


QTEST_MAIN(TestQgsApplication)
#include "moc_testqgsapplication.cxx"

